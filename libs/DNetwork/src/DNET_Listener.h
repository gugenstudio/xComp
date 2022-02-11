//==================================================================
/// DNET_Listener.h
///
/// Created by Davide Pasca - 2009/8/1
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNET_LISTENER_H
#define DNET_LISTENER_H

#include "DBase.h"
#include "DNET_Base.h"

//==================================================================
namespace DNET
{

//==================================================================
class Listener
{
	u_int	mPort = 0;
	SOCKET	mListenSock = INVALID_SOCKET;

public:
    enum IdleRetType
    {
        IRT_NOTHING,
        IRT_CONNECTED,
        IRT_ERROR,
        IRT_N
    };

    struct RetInfo
    {
        enum Message
        {
            RIMSG_NOTHING,
            RIMSG_CONNECTED,
            RIMSG_ERROR,
            RIMSG_N
        };

        int     ri_message = RIMSG_NOTHING;
        SOCKET  ri_acceptedSock = INVALID_SOCKET;
        DStr    ri_IP;
        u_int   ri_port {};
    };

public:
	Listener();
	~Listener();

    bool    StartListener( u_short port );
    void    StopListener();
    bool    IsListening() const;
    u_short GetListenPort() const { return (u_short)mPort; }
    RetInfo IdleListener( double waitS=0 );
};

//==================================================================
}

#endif
