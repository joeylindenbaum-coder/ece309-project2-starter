#include "core/sentinel_scanner.h"

#include <algorithm>
#include <utility>



//this stored the stop message we are looking for
SentinelScanner::SentinelScanner(std::string sentinel) : sentinel_(std::move(sentinel)) {
}





//takes in the next piece from the  model text
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {

    //add the new chunk to anything we were already holding

    std::string combined = pending_;
    combined.append(chunk);



    //look for the full sentinel in the combined text

    std::size_t position = combined.find(sentinel_);



    if (position != std::string::npos) {

        //everything before the sentinel is safe to show

        std::string safe_text = combined.substr(0, position);

        pending_.clear();

        return {safe_text, true};


    }





    //keep enough ending characters to catch a split sentinel
    std::size_t keep = std::min(combined.size(), sentinel_.size() - 1);

    std::size_t safe_size = combined.size() - keep;



    //the front cannot be part of a future sentinel


    std::string safe_text = combined.substr(0, safe_size);

    pending_ = combined.substr(safe_size);


    return {safe_text, false};
}



//releases the text left over when the stream ends



SentinelScanner::Out SentinelScanner::flush() {

    std::string safe_text = pending_;

    pending_.clear();

    return {safe_text, false};

    
}