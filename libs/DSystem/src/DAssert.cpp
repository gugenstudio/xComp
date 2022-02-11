//==================================================================
/// DAssert.cpp
///
/// Created by Davide Pasca - 2018/03/15
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(_DEBUG) || defined(DEBUG)
# if defined(_MSC_VER)
#  include <Windows.h>
# endif
#endif

#include "DBase.h"
#include "DSafeCrt.h"
#include "DAssert.h"

static constexpr size_t MAX_BUFF_LEN = 2048;

//===============================================================
void DAssert( bool ok, const char *pFile, int line, const char *msg )
{
    if ( ok )
        return;

    if NOT( msg )
        msg = "";

    char buff[MAX_BUFF_LEN];
    std::vector<char> altBuff;

    int len = snprintf(
                buff, sizeof(buff),
                "ASSERT: %s:%i '%s'\n", pFile, line, msg );

    char *pUseBuff;

    if ( len < (int)sizeof(buff) )
    {
        pUseBuff = buff;
    }
    else
    {
        altBuff.resize( (size_t)(len + 1) );
        pUseBuff = &altBuff[0];

        len = snprintf(
                    &altBuff[0], altBuff.size(),
                    "ASSERT: %s:%i '%s'\n", pFile, line, msg );

        // len should be good here.. can't assert otherwise !
        (void)len;
    }

#if defined(_DEBUG) || defined(DEBUG)

# if defined(_MSC_VER)
    OutputDebugString( pUseBuff );
    DebugBreak();
# else
    printf( "%s", pUseBuff );
    abort();
# endif

#else
    printf( "%s", pUseBuff );
#endif
}


