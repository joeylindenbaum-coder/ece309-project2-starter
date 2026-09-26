#include "core/message.h"




// creates an empty system message
Message::Message()
    : role_(Role::System),
      content_("")
{
}




// stores the given role and text
Message::Message(Role role, std::string content)
    : role_(role),
      content_(content)
{
}



// returns who sent the message
Role Message::role() const noexcept
{
    return role_;
}



// returns the message text
const std::string& Message::content() const noexcept
{
    return content_;
}