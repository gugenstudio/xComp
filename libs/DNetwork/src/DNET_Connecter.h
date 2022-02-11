//==================================================================
/// DNET_Connecter.h
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNET_CONNECTER_H
#define DNET_CONNECTER_H

#include "DNET_Base.h"

//==================================================================
namespace DNET
{

//==================================================================
class Connecter
{
public:
	enum StartRet
	{
		STARTRET_OK,
		STARTRET_INVALID_ADDR,
		STARTRET_GENERIC_ERROR
	};

	enum IdleRet
	{
		IDLERET_WAITING,
		IDLERET_CONNECTED,
		IDLERET_ERROR
	};

private:
    SOCKET      mSock = INVALID_SOCKET;
    bool        mIsConnected = false;

public:
	//==================================================================
	Connecter( const char *pIPName=NULL, u_short port=0 );
	~Connecter();

	StartRet StartConnect(
            const char *pIPName,
            u_short port );

	bool IsConnected() const { return mIsConnected;	}

	std::pair<IdleRet,DStr> IdleConnect( double timeoutS=0 );

	SOCKET GetSocket();
};


//==================================================================
}

#endif
