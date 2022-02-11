//==================================================================
/// NetSysSock.cpp
///
/// Created by Davide Pasca - 2015/7/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "NetSysSock.h"

#if defined(WIN32)
# include <WinSock2.h>
#endif

//==================================================================
void NetSysSock_deleter( SOCKET val )
{
    closesocket( val );
}
