//==================================================================
/// DNET_Listener.cpp
///
/// Created by Davide Pasca - 2009/8/1
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(WIN32)
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#include "DLogOut.h"
#include "DNET.h"
#include "DNET_Listener.h"

#undef LOGSTD
#undef LOGERR

#define LOGERR(_FMT_,...) \
    LogOut(LOG_ERR, "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ )

//#define LOGSTD(_FMT_,...) \
//    LogOut(LOG_DBG, "[%s:%i] " _FMT_, __DSHORT_FILE__, __LINE__, ##__VA_ARGS__ )

//#define LOGERR(_FMT_,...)
#define LOGSTD(_FMT_,...)

//==================================================================
namespace DNET
{

//==================================================================
Listener::Listener() = default;

Listener::~Listener()
{
	StopListener();
}

//==================================================================
bool Listener::IsListening() const
{
	return mListenSock != INVALID_SOCKET;
}

//==================================================================
bool Listener::StartListener( u_short port )
{
	if ( mListenSock != INVALID_SOCKET )
	{
		DASSERT( 0 );
		return false;
	}

	struct sockaddr_in	in_sa;

	mListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if ( mListenSock == INVALID_SOCKET )
		return false;

	memset( &in_sa, 0, sizeof(in_sa) );
	in_sa.sin_family = AF_INET;
	in_sa.sin_addr.s_addr = htonl( INADDR_ANY );
	in_sa.sin_port        = htons( port );

    //
    auto onFail = [&]( c_auto *pMsg )
    {
        LOGERR( "%s failed: %s", pMsg, GetSockErrStr( LastSockErr() ) );
		closesocket( mListenSock );
		mListenSock = INVALID_SOCKET;
    };

    // necessary for a quick reconnect
    // https://stackoverflow.com/a/577905/1507238
#ifndef WIN32
    {
        int val = 1;
        setsockopt( mListenSock, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(val) );
    }
#endif

	if ( -1 == bind( mListenSock, (struct sockaddr *)&in_sa, sizeof(in_sa) ) )
	{
        onFail( "bind()" );
		return false;
	}

    if NOT( port )
    {
        struct sockaddr_in sa {};
        socklen_t sa_len = sizeof(sa);

        if ( -1 == getsockname( mListenSock, (struct sockaddr *)&sa, &sa_len ) )
        {
            onFail( "getsockname()" );
            return false;
        }

        mPort = htons( sa.sin_port );

        LOGSTD( "Get port %u assigned.", (unsigned)mPort );
    }
    else
    {
        mPort = port;
    }

	if ( -1 == listen( mListenSock, SOMAXCONN ) )
	{
        onFail( "listen()" );
		return false;
	}

	if NOT( SetNonBlocking( mListenSock ) )
	{
        onFail( "SetNonBlocking()" );
		return false;
	}

    LOGSTD( "Successfully started listening" );
	return true;
}

//==================================================================
void Listener::StopListener()
{
	if ( mListenSock != INVALID_SOCKET )
	{
		closesocket( mListenSock );
		mListenSock = INVALID_SOCKET;
	}
}

//==================================================================
// https://stackoverflow.com/a/2372149/1507238
// get port, IPv4 or IPv6:
static u_int get_in_port( const struct sockaddr *sa )
{
	if (sa->sa_family == AF_INET)
		return ntohs( ((const struct sockaddr_in*)sa)->sin_port );

	return ntohs( ((const struct sockaddr_in6*)sa)->sin6_port );
}

//==================================================================
Listener::RetInfo Listener::IdleListener( double waitS )
{
    RetInfo ret;

	struct timeval	tv {};
	fd_set			rdset {};

	if ( mListenSock == INVALID_SOCKET )
    {
        ret.ri_message = RetInfo::RIMSG_NOTHING;
		return ret;
    }

	FD_ZERO( &rdset );
	FD_SET( mListenSock, &rdset );

    if ( waitS )
    {
        tv.tv_sec = (int)waitS;
        tv.tv_usec = (long)((waitS - (double)tv.tv_sec) * 1000000);
    }

    // pick the highest socket numer (easy, it's just this one)
    c_auto highSock = mListenSock;

    // run the select()
	c_auto selN = select( (int)highSock+1, &rdset, NULL, 0, &tv );

    // ignore if got nothing
    if ( selN == 0 )
    {
        ret.ri_message = RetInfo::RIMSG_NOTHING;
        return ret;
    }

    //  handle error
	if ( selN == -1 )
	{
        LOGERR( "select() failed: %s", GetSockErrStr( LastSockErr() ) );
		StopListener();
		ret.ri_message = RetInfo::RIMSG_ERROR;
        return ret;
	}

    // see if there's something to accept if we have a count od sockets
	if ( FD_ISSET( mListenSock, &rdset ) )
	{
		sockaddr client_addr {};
		socklen_t clen = sizeof(client_addr);

		ret.ri_acceptedSock = accept(mListenSock, (struct sockaddr *)&client_addr, &clen);

		if ( ret.ri_acceptedSock != INVALID_SOCKET )
		{
            LOGSTD( "Connection accepted listen:%i accepted:%i",
                    mListenSock,
                    ret.ri_acceptedSock );

            // get the IP and port
// https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
            {
                c_auto *pV4Addr = (struct sockaddr_in*)&client_addr;
                c_auto ipAddr = pV4Addr->sin_addr;

                char ipAddrStr[INET_ADDRSTRLEN] {};
                inet_ntop(AF_INET, &ipAddr, ipAddrStr, INET_ADDRSTRLEN);

                ret.ri_IP   = ipAddrStr;
                ret.ri_port = get_in_port(&client_addr);

                //LogOut( LOG_DBG, "Accepted from %s:%u", ipAddrStr, ret.ri_port  );
            }

			// make non-blocking
			if NOT( SetNonBlocking( ret.ri_acceptedSock ) )
			{
                LOGERR( "Failed to set non-blocking" );
				closesocket( ret.ri_acceptedSock );
				ret.ri_acceptedSock = INVALID_SOCKET;
				ret.ri_message = RetInfo::RIMSG_ERROR;
                return ret;
			}

            LOGSTD( "Listener did connect !" );
			ret.ri_message = RetInfo::RIMSG_CONNECTED;
            return ret;
		}
	}

	ret.ri_message = RetInfo::RIMSG_NOTHING;
    return ret;
}

//==================================================================
}
