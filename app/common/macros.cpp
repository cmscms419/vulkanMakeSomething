#include "macros.h"

const cString UserName = []()
{
    cChar username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);

#if DEBUG_

    if (cString(username) != DEBUG_USER_NAME_0) 
    {
        username[0] = '\0';
    }

#else

    username[0] = '\0';

#endif // DEBUG_

    return cString(username);
}();