//==================================================================
/// DNET_Connecter.cpp
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <memory.h>
#include <math.h>
#include "DLogOut.h"
#include "DExceptions.h"
#include "DNET_Connecter.h"

#if defined(WIN32)
# include <WinSock2.h>
#endif

//#define DLOG LogOut
#define DLOG(_FLG_,_FMT_,...)

//==================================================================
namespace DNET
{

//==================================================================
Connecter::Connecter( const char *pIPName, u_short port )
{
    if NOT( pIPName )
        return;

    switch ( StartConnect( pIPName, port ) )
    {
    case STARTRET_OK:
        break;

    case STARTRET_INVALID_ADDR:
        DEX_CONNECTION_ERROR( "Invalid network address '%s'", pIPName );
        break;

    default:
    case STARTRET_GENERIC_ERROR:
        DEX_CONNECTION_ERROR( "Failed to start connection to %s'", pIPName );
        break;
    }
}

//==================================================================
Connecter::StartRet Connecter::StartConnect(
            const char *pIPName,
            u_short port )
{
	sockaddr_in	sockAddr;

	memset( &sockAddr, 0, sizeof(sockAddr) );

	u_long	addr;		// in_addr_t ?

	addr = inet_addr( pIPName );
	if ( addr == INADDR_NONE )
	{
		struct hostent	*pHostent;

		if NOT( pHostent = gethostbyname( pIPName ) )
		{
			DLOG( LOG_DBG, "Invalid network address '%s'", pIPName );
			return STARTRET_INVALID_ADDR;
		}

		addr = *(long *)pHostent->h_addr;
	}

	memcpy( &sockAddr.sin_addr, &addr, sizeof(long) );

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons( port );

	mSock = socket(AF_INET, SOCK_STREAM, 0);
	if ( mSock == INVALID_SOCKET )
	{
		DLOG( LOG_DBG, "Failed to create the socket" );
		return STARTRET_GENERIC_ERROR;
	}

	if NOT( SetNonBlocking( mSock ) )
	{
		DLOG( LOG_DBG, "Failed to set the socket as non-blocking" );
		closesocket( mSock );
		mSock = INVALID_SOCKET;
		return STARTRET_GENERIC_ERROR;
	}

	int connectRet = connect( mSock, (struct sockaddr *)&sockAddr, sizeof(sockAddr) );

	if ( connectRet == INVALID_SOCKET )
	{
		int	err = LastSockErr();

		if ( err == EWOULDBLOCK || err == EINPROGRESS )
		{
			// fine.. we can wait..
		}
		else
		if ( err == EISCONN || err == EALREADY )
		{
			// i guess it's ok too
		}
		else
		{
			DLOG( LOG_DBG, "Failed to connect" );
			closesocket( mSock );
			mSock = INVALID_SOCKET;
			return STARTRET_GENERIC_ERROR;
		}
	}
    else
    if ( connectRet == 0 )
    {
        // immediate connection.. no need to wait
        mIsConnected = true;
    }

	return STARTRET_OK;
}

//==================================================================
Connecter::~Connecter()
{
	if ( mSock != INVALID_SOCKET )
		closesocket( mSock );
}

//==================================================================
static DStr makeErrorStr( int err, const DStr &extraMsg )
{
    return DStr("Net Error ") +
            GetSockErrStr( err ) +
            " #" + std::to_string( err ) +
            ", " + extraMsg;
}

//==================================================================
std::pair<Connecter::IdleRet,DStr> Connecter::IdleConnect( double timeoutS )
{
	if ( mIsConnected )
		return {IDLERET_CONNECTED, {}};

	fd_set socks;
	FD_ZERO( &socks );
	FD_SET( mSock, &socks );

	SOCKET	highSock = mSock;

    long secs  = (long)floor( timeoutS );
    long usecs = (long)((timeoutS - (double)secs) * 1000000);

	struct timeval tv;
	tv.tv_sec	= secs;
	tv.tv_usec	= (uint32_t)usecs;

    // first param is ignored for windows, so, (int) cast is safe
	int goodSocks = select( (int)highSock+1, NULL, &socks, NULL, &tv );

	if ( goodSocks == -1 )
	{
        c_auto err = LastSockErr();
		return {IDLERET_ERROR, makeErrorStr( err, "select()" ) };
	}
	if ( goodSocks > 0 )	// or rather, "1" in this case..
	{
#ifndef WIN32
        // see: https://stackoverflow.com/a/10194883/1507238
        int err {};
        socklen_t result_len = sizeof(err);

        if ( getsockopt( mSock, SOL_SOCKET, SO_ERROR, &err, &result_len ) < 0 )
        {
            c_auto err = LastSockErr();

            // error, fail somehow, close socket
			closesocket( mSock );
			mSock = INVALID_SOCKET;
            return {IDLERET_ERROR, makeErrorStr( err, "Failed to get SO_ERROR" ) };
        }

        if ( err )
        {
            // connection failed; error code is in 'err'
			closesocket( mSock );
			mSock = INVALID_SOCKET;
            return {IDLERET_ERROR, makeErrorStr( err, "" )};
        }

        // socket is ready for read()/write()
#endif
		mIsConnected = true;

		return {IDLERET_CONNECTED, {}};
	}
	else
		return {IDLERET_WAITING, {}};
}

//==================================================================
SOCKET Connecter::GetSocket()
{
	if ( mIsConnected )
	{
		SOCKET	sock = mSock;
		mSock = INVALID_SOCKET;

		return sock;
	}
	else
		return INVALID_SOCKET;
}

//==================================================================
}
