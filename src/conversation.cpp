#include "core/conversation.h"

#include <stdexcept>
#include <utility>




// starts it as an empty conversation
Conversation::Conversation() : data_(nullptr), size_(0), capacity_(0) {
}


// this clears the message array

Conversation::~Conversation() {
    delete[] data_;
}





// makes a separate copy of the array
// for example when we do Conversation second(first); or Conversation second = first;
Conversation::Conversation(const Conversation& other) : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
//we copy the other conversation's size and capacity

    if (capacity_ > 0) {
        data_ = new Message[capacity_];
//here we create a new array at a different location in memory

        for (std::size_t i = 0; i < size_; i++)
            data_[i] = other.data_[i];
    }
}
//loop just copies over the messages (does not copy unused array positions)

//now we have a deep copy






//copy assignment operator
Conversation& Conversation::operator=(const Conversation& other) {


    if (this != &other) { //check convo not assigned to itself (this points to conversation being changed, &other to convo being copied)
        // make the new copy before replacing the old one
        Conversation copy(other);
        //create temporary copy


        std::swap(data_, copy.data_);
        std::swap(size_, copy.size_);
        std::swap(capacity_, copy.capacity_);
    }

    return *this;
}
// copy goes out of scope after this function
//copy and swap creates new copy before removing old data



// move constructor takes the array without copying each message

Conversation::Conversation(Conversation&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
// copy over pointer and numbers



    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    //empty the source
}
//r uns when new convo takes ownership from another convo, like Conversation second(std::move(first));






// when both conversations already exist, so old array must be released first
Conversation& Conversation::operator=(Conversation&& other) noexcept {
    if (this != &other) {
        delete[] data_; //release old array

        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        //give second the source's information




        // leaves the old conversation empty
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    return *this;
}


//grows the array when there is no room left
void Conversation::append(Message m) {
    if (size_ == capacity_) {
        std::size_t new_capacity;



        //if there is already unused space the entire growth section can be skipped
        if (capacity_ == 0) // the first append has to change the capacity from 0 to 1
            new_capacity = 1;
        else
            new_capacity = capacity_ * 2;

        Message* new_data = new Message[new_capacity];
        //capacity will growth from 0, to 1, to 2, to 4, to 8, to 16, etc.



        for (std::size_t i = 0; i < size_; i++)
            new_data[i] = std::move(data_[i]);
        //loop through and transfer each stored message into the new array


        delete[] data_; //the old array is no longed needed

        data_ = new_data;
        capacity_ = new_capacity;
    }


    data_[size_] = std::move(m);
    size_++;

}

// function to return the number of messages currently stored
std::size_t Conversation::size() const noexcept {

    return size_;


}



// function checks the position before using the array
const Message& Conversation::at(std::size_t i) const {

    if (i >= size_)
        throw std::out_of_range("conversation index is out of range");


    return data_[i];


}


//data points to the first message in the array, which is also the beginning of the convo
const Message* Conversation::begin() const noexcept {
    return data_;
}


// end points one position after the final stored message
const Message* Conversation::end() const noexcept {
    if (data_ == nullptr)
    
        return nullptr;

    return data_ + size_;
}