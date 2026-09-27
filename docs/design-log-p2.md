# Design Log — Project 2

(500–800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost

I chose to double the capacity of the array whenever it becomes full. The capacity begins at zero, changes to one after the first append, and then grows from 1 to 2, 4, 8, and so on. When the array needs to grow, a new array is created with twice the capacity. The existing messages are moved into the new array, and the old array is deleted.

Most calls to append() do not require a new array. They only place the message in the next open position, which takes O(1) time. The more expensive calls happen when the array is full because every existing message must be moved. However, doubling leaves enough open space for many more messages before another reallocation is needed.

For example, reallocations move 1, 2, 4, 8, and then 16 messages. This forms a geometric series. For n total appends, the total number of messages moved is less than 2n. This means all n appends together take O(n) time. Dividing that total work across the n calls gives an amortized cost of O(1) for each append.

## Rule of Five evidence

The Conversation class manages its own array, so it needs all five Rule of Five functions. The destructor deletes the array when the conversation is no longer being used. The copy constructor creates a separate array and copies every stored message into it. This means changing or destroying one conversation will not affect the other. The copy assignment operator does the same thing for two conversations that already exist. It creates a temporary copy, swaps the information, and lets the temporary object delete the old array. It also checks for self-assignment first.

The move constructor transfers the array from one conversation to another instead of copying every message. The move assignment operator also transfers the array, but it first deletes the array that the receiving conversation already owns. After either move, the original conversation is reset by setting its pointer to nullptr and its size and capacity to zero. The original is left empty and safe to destroy, and only the new conversation owns the array.

## Sentinel scanner: bounded pending_ proof

I used pending_ to save the characters that could still be the beginning of the sentinel. When a new chunk arrives, the scanner combines it with the characters already in pending_ and searches the combined text. If it finds the complete sentinel, it returns the normal text before it and reports that the sentinel was found.

If the sentinel is not found, the scanner returns everything except the final sentinel_.size() - 1 characters. Those final characters are kept because the next chunk could complete the sentinel. There is no reason to keep more than that because a full sentinel would have already been found. As a result, pending_ can never hold more than sentinel_.size() - 1 characters. This limit stays the same even if the reply is very large or arrives one character at a time.

## What I would change differently

If I designed the project again, I would move the array-growing code out of append() and place it in a separate private function. That function would create the larger array, move the messages, delete the old array, and update the capacity. This would make append() shorter and separate adding a message from managing the array. I kept the code inside append() because the project is small and the array only grows in one place. However, a separate function would make the code easier to read and reuse if more features were added later.