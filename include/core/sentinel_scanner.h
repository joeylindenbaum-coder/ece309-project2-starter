#pragma once


#include <string>
#include <string_view>



//checks the model's reply for the stop message
class SentinelScanner {
public:



    //stores the sentinel we are looking for
    explicit SentinelScanner(std::string sentinel);




    struct Out {
        std::string safe_text;
        bool sentinel_found;
    };
    //holds the safe text and tells us if the sentinel was found


    //takes in the next piece of text from the model
    Out feed(std::string_view chunk);

    //releases anything still waiting after the stream ends
    Out flush();



private:

    //the full stop message we are searching for
    std::string sentinel_;



    //holds the ending text that could still be part of the sentinel
    std::string pending_;
};