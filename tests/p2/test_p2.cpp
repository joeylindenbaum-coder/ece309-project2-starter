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

int main() {
    // TODO: write your tests here.

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
            //if the right error happens we know the bounds check worked
            threw = true;
        }
        //this fails if at(0) did not throw the error
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




        //check the assistant message at the end
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
    //the messages and size should be the same
    assert(copy.size() == original.size());

    assert(copy.at(0).content() == "first message");
    assert(copy.at(1).content() == "second message");


    //different starting addresses prove they do not share the same array


    assert(copy.begin() != original.begin());
}


//test 4 checks copy assignment and assigning a conversation to itself
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
    copy = copy;

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


    //the new conversation should have the exact same array address
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


    //moved already exists so this calls the move assignment operator
    moved = std::move(original);


    //the old array address should now belong to moved
    assert(moved.begin() == old_address);

    assert(moved.size() == 1);
    assert(moved.at(0).role() == Role::User);
    assert(moved.at(0).content() == "new message");


    //the source should be empty after giving up its array
    assert(original.size() == 0);
    assert(original.begin() == original.end());
}





//test 7 checks the doubling pattern and makes sure no messages are lost
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



    //make sure every message survived the array changes
    assert(conversation.size() == 5);

    assert(conversation.at(0).content() == "one");
    assert(conversation.at(1).content() == "two");
    assert(conversation.at(2).content() == "three");
    assert(conversation.at(3).content() == "four");
    assert(conversation.at(4).content() == "five");
}





//test 8 checks normal text that does not contain the sentinel
{
    SentinelScanner scanner("<|end_conversation|>");


    auto first = scanner.feed("ordinary message");

    //flush releases the text that was still being held
    auto last = scanner.flush();


    assert(!first.sentinel_found);
    assert(!last.sentinel_found);


    //put both returned pieces together and make sure nothing was lost
    assert(first.safe_text + last.safe_text == "ordinary message");
}





//test 9 checks the full sentinel in one chunk
{
    SentinelScanner scanner("<|end_conversation|>");


    auto result = scanner.feed("goodbye.<|end_conversation|>");


    //the normal text should come back but the sentinel should not
    assert(result.safe_text == "goodbye.");
    assert(result.sentinel_found);
}





//test 10 checks every place where the sentinel could be split
{
    const std::string sentinel = "<|end_conversation|>";
    const std::string text = "goodbye." + sentinel;


    //each loop cuts the same text at a different position
    for (std::size_t split = 0; split <= text.size(); split++) {

        SentinelScanner scanner(sentinel);


        auto first = scanner.feed(text.substr(0, split));
        auto second = scanner.feed(text.substr(split));


        //one of the two feeds must find the complete sentinel
        assert(first.sentinel_found || second.sentinel_found);

        //only the normal text should be returned
        assert(first.safe_text + second.safe_text == "goodbye.");
    }
}





//test 11 checks text that looks similar but is not the sentinel
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
    SentinelScanner scanner("<|end_conversation|>");


    const std::size_t total_characters = 4 * 1024 * 1024;
    std::size_t returned_characters = 0;


    //this checks that the scanner can handle a long stream without saving all of it
    for (std::size_t i = 0; i < total_characters; i++) {

        auto result = scanner.feed("x");

        assert(!result.sentinel_found);
        returned_characters += result.safe_text.size();
    }


    //add the final characters that were still waiting
    auto last = scanner.flush();
    returned_characters += last.safe_text.size();


    assert(returned_characters == total_characters);
}


    return 0;
}
