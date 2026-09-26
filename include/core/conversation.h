#pragma once
#include "core/message.h"
#include <cstddef>

class Conversation {
public:
    // starts empty
    Conversation();
    ~Conversation();

    // copy functions
    Conversation(const Conversation& other);
    Conversation& operator=(const Conversation& other);


    // move functions
    Conversation(Conversation&& other) noexcept;

    Conversation& operator=(Conversation&& other) noexcept;


    // adds a new message
    void append(Message m);

    std::size_t size() const noexcept;

    // checks the position before returning it
    const Message& at(std::size_t i) const;


    const Message* begin() const noexcept;
    const Message* end() const noexcept;




private:



    Message* data_ = nullptr;

    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};