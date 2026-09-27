// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// this provides the user input we want the harness to receive
class FakeUserInput : public InputSource {
public:
    explicit FakeUserInput(std::vector<std::string> input_lines) : input_lines_(std::move(input_lines)) {
    }

    std::string read_line() override {


        //there are no more user messages left

        if (next_line_ == input_lines_.size()) {
            reached_end_ = true;
            return "";
        }

        return input_lines_[next_line_++];
    }


    bool is_eof() const override {
        return reached_end_;
    }



private:
    std::vector<std::string> input_lines_;
    std::size_t next_line_ = 0;
    bool reached_end_ = false;
};



//this saves everything the harness tries to print


class SavedProgramOutput : public OutputSink {
public:

    void write(std::string_view new_text) override {
        saved_text += new_text;
    }


    std::string saved_text;
};



int main() {
    // write your tests here.

//test 1 makes sure a new conversation starts empty
    {
        Conversation conversation;
        //there should not be any messages yet
        assert(conversation.size() == 0);

        //begin and end should match when there is nothing stored
        assert(conversation.begin() == conversation.end());

        bool threw = false;
        try {
            //position 0 should not exist in an empty conversation
            conversation.at(0);
        }
        catch (const std::out_of_range&) {
            // if the right error happens we know the bounds check worked
            threw = true;
        }
        //  this fails if at(0) did not throw the error
        assert(threw);
    }

    //test 2 makes sure the messages stay in the order we added them
    {

        Conversation conversation;

        //the system message should be added first and stay at the front

        conversation.append(Message(Role::System, "system instructions"));

        conversation.append(Message(Role::User, "hello"));

        conversation.append(Message(Role::Assistant, "hi"));

        //we added three messages so the size should now be three
        assert(conversation.size() == 3);

        //check the system message at the front
        assert(conversation.at(0).role() == Role::System);
        assert(conversation.at(0).content() == "system instructions");


        //check the user message in the middle
        assert(conversation.at(1).role() == Role::User);
        assert(conversation.at(1).content() == "hello");




        // check the assistant message at the end
        assert(conversation.at(2).role() == Role::Assistant);
        assert(conversation.at(2).content() == "hi");
    }


    //test 3 makes sure the copy constructor creates its own array
{
    Conversation original;

    original.append(Message(Role::User, "first message"));
    original.append(Message(Role::Assistant, "second message"));



    //this creates a new conversation from the original
    Conversation copy(original);
    // the messages and size should be the same
    assert(copy.size() == original.size());

    assert(copy.at(0).content() == "first message");
    assert(copy.at(1).content() == "second message");


    //different starting addresses prove they do not share the same array


    assert(copy.begin() != original.begin());
}


// test 4 checks copy assignment and assigning a conversation to itself
{
    Conversation original;
    original.append(Message(Role::User, "copied message"));


    Conversation copy;
    copy.append(Message(Role::Assistant, "old message"));


    // copy already exists here so this uses the copy assignment operator
    copy = original;



    //the old message should be replaced with the copied message
    assert(copy.size() == 1);
    assert(copy.at(0).role() == Role::User);
    assert(copy.at(0).content() == "copied message");


    //the two conversations should still own different arrays
    assert(copy.begin() != original.begin());



    //this checks that self assignment does not damage anything
    Conversation* same_conversation = &copy;
    copy = *same_conversation;

    assert(copy.size() == 1);
    assert(copy.at(0).content() == "copied message");

}


//test 5 makes sure the move constructor takes the original array
{
    Conversation original;
    original.append(Message(Role::User, "move this message"));



    //save the address so we can check that the array was moved
    const Message* old_address = original.begin();


    //moved is being created here so this calls the move constructor

    Conversation moved(std::move(original));


    //  the new conversation should have the exact same array address
    assert(moved.begin() == old_address);

    assert(moved.size() == 1);
    assert(moved.at(0).content() == "move this message");




    //the source should be valid but empty after the move

    assert(original.size() == 0);
    assert(original.begin() == original.end());
}





//test 6 checks moving into a conversation that already exists
{

    Conversation original;
    original.append(Message(Role::User, "new message"));

    const Message* old_address = original.begin();


    Conversation moved;
    moved.append(Message(Role::Assistant, "old message"));


    // moved already exists so this calls the move assignment operator
    moved = std::move(original);


    //the old array address should now belong to moved
    assert(moved.begin() == old_address);

    assert(moved.size() == 1);
    assert(moved.at(0).role() == Role::User);
    assert(moved.at(0).content() == "new message");



    //  the source should be empty after giving up its array
    assert(original.size() == 0);
    assert(original.begin() == original.end());
}





// test 7 checks the doubling pattern and makes sure no messages are getting lost
{
    Conversation conversation;


    conversation.append(Message(Role::User, "one"));
    const Message* capacity_one = conversation.begin();




    conversation.append(Message(Role::User, "two"));
    const Message* capacity_two = conversation.begin();

    //going from one space to two should create a new array
    assert(capacity_two != capacity_one);


    conversation.append(Message(Role::User, "three"));
    const Message* capacity_four = conversation.begin();



    //going from two spaces to four should create another array
    assert(capacity_four != capacity_two);


    conversation.append(Message(Role::User, "four"));


    //the fourth message still fits so the address should not change
    assert(conversation.begin() == capacity_four);


    conversation.append(Message(Role::User, "five"));

    //the fifth message needs a larger array
    assert(conversation.begin() != capacity_four);



    // make sure every message survived the array changes

    assert(conversation.size() == 5);

    assert(conversation.at(0).content() == "one");
    assert(conversation.at(1).content() == "two");
    assert(conversation.at(2).content() == "three");
    assert(conversation.at(3).content() == "four");
    assert(conversation.at(4).content() == "five");


}





 // test 8 checks normal text that does not contain the sentinel

{

    SentinelScanner scanner("<|end_conversation|>");


    auto first = scanner.feed("ordinary message");


    //then flush releases the text that was still being held

    auto last = scanner.flush();



    assert(!first.sentinel_found);
    assert(!last.sentinel_found);


    // put both returned pieces together and make sure nothing was lost

    assert(first.safe_text + last.safe_text == "ordinary message");

}





//test 9 just checks the full sentinel in one chunk
{



    SentinelScanner scanner("<|end_conversation|>");


    auto result = scanner.feed("goodbye.<|end_conversation|>");




    //  the normal text should come back but the sentinel should not


    assert(result.safe_text == "goodbye.");

    assert(result.sentinel_found);
}





// test 10 checks every place where the sentinel could be split
{



    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "goodbye." + sentinel;


    //each loop cuts the same text at a different position


    for (std::size_t split = 0; split <= text.size(); split++) {

        SentinelScanner scanner(sentinel);


        auto first = scanner.feed(text.substr(0, split));
        auto second = scanner.feed(text.substr(split));


        // one of the two feeds must find the complete sentinel
        assert(first.sentinel_found || second.sentinel_found);

        // only the normal text should be returned
        assert(first.safe_text + second.safe_text == "goodbye.");
    }
}





// test 11 checks text that looks similar but is not the sentinel

{

    SentinelScanner scanner("<|end_conversation|>");

    const std::string text = "hello <|end_world|>";



    auto first = scanner.feed(text);
    auto last = scanner.flush();


    //a similar message should not stop the conversation
    assert(!first.sentinel_found);
    assert(!last.sentinel_found);


    //all of the original text should still come back
    assert(first.safe_text + last.safe_text == text);
}







//test 12 feeds a large stream one character at a time
{

    const std::string sentinel = "<|end_conversation|>";

    SentinelScanner scanner(sentinel);




    const std::size_t total_characters = 4 * 1024 * 1024;

    std::size_t characters_sent = 0;
    std::size_t characters_returned = 0;



    // the scanner should never hold more than sentinel length minus one
    for (std::size_t i = 0; i < total_characters; i++) {
        auto result = scanner.feed("x");

        characters_sent++;
        characters_returned += result.safe_text.size();

        assert(!result.sentinel_found);


        //anything not returned yet is still being held by the scanner
        std::size_t characters_held = characters_sent - characters_returned;
        assert(characters_held <= sentinel.size() - 1);


    }


    //release the final characters still being held

    auto last = scanner.flush();
    characters_returned += last.safe_text.size();



    assert(!last.sentinel_found);
    assert(characters_returned == total_characters);

}







// test 13 makes sure the harness stops when it reaches the turn limit
{
    const std::string script_path = "turn_limit_test.script";


    // we create two normal model replies
    {
        std::ofstream script_file(script_path);


        script_file << "role: assistant\n";
        script_file << "first reply\n";
        script_file << "---\n";
        script_file << "role: assistant\n";
        script_file << "second reply\n";

    }


    auto model = std::make_unique<ScriptedModelClient>(script_path);



    HarnessConfig settings;
    settings.max_turns = 2;
    // 2 max turns


    Harness harness(std::move(model), settings);


    FakeUserInput user_input({"first question", "second question"});
    SavedProgramOutput program_output;


    StopReason result = harness.run(user_input, program_output);


    //both turns were used so the turn limit should stop the harness
    assert(result.kind == StopReason::Kind::TurnLimit);


    //each turn adds one user message and one assistant message
    assert(harness.conversation().size() == 4);


    std::remove(script_path.c_str());
}






//test 14 makes sure the harness stops when the sentinel is found
{
    const std::string script_path = "sentinel_test.script";



    //chunk size one sends the reply one character at a time
    {


        std::ofstream script_file(script_path);

        script_file << "chunk: 1\n";
        script_file << "role: assistant\n";
        script_file << "goodbye.<|end_conversation|>\n";
    }



    auto model = std::make_unique<ScriptedModelClient>(script_path);



    HarnessConfig settings;

    Harness harness(std::move(model), settings);


    FakeUserInput user_input({"bye"});
    SavedProgramOutput program_output;

    StopReason result = harness.run(user_input, program_output);


    //the sentinel should stop the harness after the first reply
    assert(result.kind == StopReason::Kind::Sentinel);



    //the reply should be printed without showing the sentinel

    assert(program_output.saved_text.find("goodbye.") != std::string::npos);
    assert(program_output.saved_text.find("<|end_conversation|>") == std::string::npos);



    //the stored assistant message still needs the sentinel for replay

    assert(harness.conversation().size() == 2);
    assert(harness.conversation().at(1).content() == "goodbye.<|end_conversation|>");


    std::remove(script_path.c_str());
}





// test 15 makes sure a transcript replays the same assistant messages
{

    const std::string transcript_path = "replay_test.txt";


    //create a small transcript with two assistant replies
    {
        std::ofstream transcript_file(transcript_path);

        transcript_file << "role: user\n";
        transcript_file << "hello\n";
        transcript_file << "---\n";

        transcript_file << "role: assistant\n";
        transcript_file << "hi there\n";
        transcript_file << "---\n";

        transcript_file << "role: user\n";
        transcript_file << "bye\n";
        transcript_file << "---\n";

        transcript_file << "role: assistant\n";
        transcript_file << "goodbye.<|end_conversation|>\n";
    }


    ReplayModelClient replay(transcript_path);
    Conversation conversation;


    Message first_reply = replay.generate(conversation);

    Message second_reply = replay.generate(conversation);



    // the replayed replies should match the transcript

    assert(first_reply.role() == Role::Assistant);
    assert(first_reply.content() == "hi there");

    assert(second_reply.role() == Role::Assistant);
    assert(second_reply.content() == "goodbye.<|end_conversation|>");


    std::remove(transcript_path.c_str());
}






//test 16 makes sure eof ends the conversation cleanly
{
    const std::string script_path = "eof_test.script";


    // the model will not be called but it still needs a valid script
    {
        std::ofstream script_file(script_path);

        script_file << "role: assistant\n";
        script_file << "unused reply\n";
    }


    auto model = std::make_unique<ScriptedModelClient>(script_path);

    HarnessConfig settings;
    Harness harness(std::move(model), settings);


    // the no input lines makes the input reach eof immediately
    FakeUserInput user_input({});
    SavedProgramOutput program_output;

    StopReason result = harness.run(user_input, program_output);


    assert(result.kind == StopReason::Kind::UserExit);
    assert(harness.conversation().size() == 0);


    std::remove(script_path.c_str());
}







    return 0;
}
