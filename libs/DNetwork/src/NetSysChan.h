//==================================================================
/// NetSysChan.h
///
/// Created by Davide Pasca - 2012/4/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef NETSYSCHAN_H
#define NETSYSCHAN_H

#include <optional>
#include "NetSysBase.h"
#include "NetSysSock.h"
#include "DNET_Connecter.h"
#include "DNET_PacketManager.h"
#include "TimeUtils.h"

//==================================================================
class NetSysChan
{
public:
	enum State
	{
		STATE_CREATED               ,
		STATE_CONNECTING            ,
		STATE_CONNECTED             ,
		STATE_DISCONNECTED          ,
		STATE_WAITING_DISCONNECT    ,

		STATE_INVALID_ADDR_ERROR    ,
		STATE_CONNECT_ERROR         ,
		STATE_GENERIC_ERROR         ,
		STATE_BAD_PROTOCOL_ERROR    ,
		STATE_UNRESPONSIVE_ERROR    ,

        FIRST_ERROR = STATE_INVALID_ADDR_ERROR,
        LAST_ERROR  = STATE_UNRESPONSIVE_ERROR,

		STATE_N
	};

	State						mState = STATE_N;
    bool                        mIsExpectingDisconnect {};
    NetSysGlobID                mRemoteGlobalID = 0;
private:
	DStr						mIPName;
	u_short						mPort = 0;
public:
    NetSysSock                  moSock;
    bool                        mIsConnAcceptor = false;
	uptr<DNET::Connecter>		moConnecter;
	uptr<DNET::PacketManager>	moPackMgr;
    TimeUS                      mNextPingSendTimeUS;
    TimeUS                      mPingReqTimeUS;
    TimeUS                      mCurLatencyUS;
private:
    TimeUS                      mCurTimeUS;
    TimeUS                      mStartConnTimeUS;

public:
    static NetSysChan *CreateClientSelf( const char *pIPName, u_short port )
    {
        return new NetSysChan( pIPName, port );
    }

    static NetSysChan *CreateClientRemove(
                                const char *pIPName,
                                u_short port,
                                u_int remoteGlobalID,
                                SOCKET sock )
    {
        return new NetSysChan( pIPName, port, remoteGlobalID, sock );
    }

    static NetSysChan *CreateServer( u_int remoteGlobalID )
    {
        return new NetSysChan( remoteGlobalID );
    }

private:
    NetSysChan( const char *pIPName, u_short port );

    NetSysChan( const char *pIPName,
                u_short port,
                u_int remoteGlobalID,
                SOCKET sock );

    NetSysChan( u_int remoteGlobalID );

public:
    c_auto &GetIPName() const { return mIPName; }
    c_auto GetPort() const    { return mPort; }

	void IdleChan( TimeUS curTimeUS );

	bool IsTerminated() const;
	bool IsConnected() const { return mState == STATE_CONNECTED; }
	bool IsConnecting() const { return mState == STATE_CONNECTING; }

    bool IsConnAcceptor() const;

    void SetExpectedConnBreakNSC();

	// for debugging purposes
	static const DStr &GetStateStr( State st );

	static bool IsStateError( State st ) { return st >= FIRST_ERROR && st <= LAST_ERROR; }

	const DStr &GetCurStateStr() const { return GetStateStr( mState ); }

	bool GetPacketData( NetSysPackID matchID, void *pOut_data, size_t dataSize );

    template <typename T>
	std::optional<T> GetPacketValue( NetSysPackID matchID )
    {
        T val;
        return GetPacketData( matchID, &val, sizeof(val) )
            ? std::optional<T>{val}
            : std::nullopt;
    }

private:
    void logNSC( u_int logFlags, const DStr &msg ) const;
    void setStateNSC( State st, const DStr &errMsg={} );
	void idleConnect( TimeUS curTimeUS );
};


#endif
