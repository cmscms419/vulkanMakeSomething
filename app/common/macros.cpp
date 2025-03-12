#include "macros.h"

const std::string UserName = []()
{
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);

#if DEBUG_

    if (std::string(username) != DEBUG_USER_NAME_0) 
    {
        username[0] = '\0';
    }

#else

    username[0] = '\0';

#endif // DEBUG_

    return std::string(username);
}();