//==================================================================
/// DNET_Base.cpp
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(WIN32)
# include <WinSock2.h>
# pragma comment(lib, "Ws2_32.lib")
#endif

#include "DNET_Base.h"

//==================================================================
namespace DNET
{

//==================================================================
bool InitializeSocket()
{
	static bool	initialized;

	if ( initialized )
		return true;

#if defined(WIN32)
	WSAData	data;

	if ( WSAStartup( MAKEWORD(2,2), &data ) != 0 )
	{
		return false;
	}
#endif

	initialized = true;

	return true;
}

//==================================================================
int LastSockErr()
{
#if defined(WIN32)
	return WSAGetLastError();
#else
	return errno;
#endif
}

//==================================================================
bool SetNonBlocking( SOCKET sock )
{
#if defined(WIN32)
	u_long	blockflg = 1;
	if ( -1 == ioctlsocket( sock, FIONBIO, &blockflg ) )
		return false;

#else
	int flags = fcntl( sock, F_GETFL, 0 );
    if ( flags < 0 )
        return false;

    flags |= O_NONBLOCK;
	if ( -1 == fcntl( sock, F_SETFL, flags ) )
		return false;
#endif

	return true;
}

//==================================================================
const char *GetSockErrStr( int err )
{
	switch ( err )
	{
	case EWOULDBLOCK    : return "EWOULDBLOCK";
	case EINPROGRESS    : return "EINPROGRESS";
	case EALREADY       : return "EALREADY";
	case ENOTSOCK       : return "ENOTSOCK";
	case EDESTADDRREQ   : return "EDESTADDRREQ";
	case EMSGSIZE       : return "EMSGSIZE";
	case EPROTOTYPE     : return "EPROTOTYPE";
	case ENOPROTOOPT    : return "ENOPROTOOPT";
	case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
	case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
	case EOPNOTSUPP     : return "EOPNOTSUPP";
	case EPFNOSUPPORT   : return "EPFNOSUPPORT";
	case EAFNOSUPPORT   : return "EAFNOSUPPORT";
	case EADDRINUSE     : return "EADDRINUSE";
	case EADDRNOTAVAIL  : return "EADDRNOTAVAIL";
	case ENETDOWN       : return "ENETDOWN";
	case ENETUNREACH    : return "ENETUNREACH";
	case ENETRESET      : return "ENETRESET";
	case ECONNABORTED   : return "ECONNABORTED";
	case ECONNRESET     : return "ECONNRESET";
	case ENOBUFS        : return "ENOBUFS";
	case EISCONN        : return "EISCONN";
	case ENOTCONN       : return "ENOTCONN";
	case ESHUTDOWN      : return "ESHUTDOWN";
	case ETOOMANYREFS   : return "ETOOMANYREFS";
	case ETIMEDOUT      : return "ETIMEDOUT";
	case ECONNREFUSED   : return "ECONNREFUSED";
	case ELOOP          : return "ELOOP";
	case ENAMETOOLONG   : return "ENAMETOOLONG";
	case EHOSTDOWN      : return "EHOSTDOWN";
	case EHOSTUNREACH   : return "EHOSTUNREACH";
	case ENOTEMPTY      : return "ENOTEMPTY";
// TODO fix at some point
#if defined(WIN32)
	case EPROCLIM       : return "EPROCLIM";
#endif
	case EUSERS         : return "EUSERS";
	case EDQUOT         : return "EDQUOT";
	case ESTALE         : return "ESTALE";
	case EREMOTE        : return "EREMOTE";
	default				: return "UNKNOWN_ERROR";
	}
}

//==================================================================
}
