//==================================================================
/// NetSysChan.cpp
///
/// Created by Davide Pasca - 2012/4/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "NetSysChan.h"
#include "DLogOut.h"

#define DLOG logNSC
//#define DLOG (void)

//==================================================================
NetSysChan::NetSysChan( const char *pIPName, u_short port )
    : mState(STATE_CREATED)
    , mIPName(pIPName)
    , mPort(port)
    , moConnecter(std::make_unique<DNET::Connecter>())
{
}

//==================================================================
NetSysChan::NetSysChan(
                const char *pIPName,
                u_short port,
                u_int remoteGlobalID,
                SOCKET sock )
    : mState(STATE_CONNECTED)
    , mRemoteGlobalID(remoteGlobalID)
    , mIPName(pIPName)
    , mPort(port)
    , moSock(sock)
    , mIsConnAcceptor(true)
    , moPackMgr(std::make_unique<DNET::PacketManager>( sock ))
{
	DASSERT( remoteGlobalID != 0 );
}

//==================================================================
NetSysChan::NetSysChan( u_int remoteGlobalID )
    : mState(STATE_CONNECTED)
    , mRemoteGlobalID(remoteGlobalID)
    , moPackMgr(std::make_unique<DNET::PacketManager>())
{
	DASSERT( remoteGlobalID != 0 );
}

//==================================================================
void NetSysChan::logNSC( u_int logFlags, const DStr &msg ) const
{
    if ( mIPName == "127.0.0.1" )
        LogOut( logFlags, "NSChan (:%u) %s", (unsigned)mPort, msg.c_str() );
    else
        LogOut( logFlags, "NSChan (%s:%u) %s", mIPName.c_str(), (unsigned)mPort, msg.c_str() );
}

//==================================================================
void NetSysChan::setStateNSC( State st, const DStr &errMsg )
{
    if ( st != mState )
    {
        DLOG(
            LOG_DBG | (IsStateError( st ) ? LOG_ERR : 0),
            "state change:"
                + GetStateStr( mState )
                + " -> "
                + GetStateStr( st )
            + (errMsg.empty() ? DStr() : " ErrMsg: " + errMsg) );

        mState = st;
    }
}

//==================================================================
void NetSysChan::idleConnect( TimeUS curTimeUS )
{
	switch ( mState )
	{
	case STATE_CREATED:
		mStartConnTimeUS = mCurTimeUS;
		switch( moConnecter->StartConnect( mIPName.c_str(), mPort ) )
		{
		case DNET::Connecter::STARTRET_OK:
			setStateNSC( STATE_CONNECTING );
			break;

		case DNET::Connecter::STARTRET_INVALID_ADDR:
			moConnecter = nullptr;
			setStateNSC( STATE_INVALID_ADDR_ERROR );
			break;

		default:
			moConnecter = nullptr;
			setStateNSC( STATE_GENERIC_ERROR, "StartConnect()" );
			break;
		}
		break;

	case STATE_CONNECTING:
		switch ( c_auto &ret = moConnecter->IdleConnect(); ret.first )
		{
		case DNET::Connecter::IDLERET_WAITING:
			if ( (mCurTimeUS - mStartConnTimeUS) > (TimeUS::ONE_SECOND()*30) )
			{
				moConnecter = nullptr;
				setStateNSC( STATE_CONNECT_ERROR, "Connection attempt timeout." );
			}
			break;

		case DNET::Connecter::IDLERET_CONNECTED:
            // reset here so that it doesn't exist before moSock is destroyed
            moPackMgr = nullptr;

            moSock.reset( moConnecter->GetSocket() );
			moConnecter = nullptr;

            moPackMgr = std::make_unique<DNET::PacketManager>( moSock.get() );

			setStateNSC( STATE_CONNECTED );
			break;

		case DNET::Connecter::IDLERET_ERROR:
			moConnecter = nullptr;
			setStateNSC( STATE_CONNECT_ERROR, ret.second );
			break;

		default:
			DASSERT( 0 );
			break;
		}
		break;

	default:
		break;
	}
}

//==================================================================
void NetSysChan::IdleChan( TimeUS curTimeUS )
{
	mCurTimeUS = curTimeUS;

	// if connecting
	if ( moConnecter.get() )
		idleConnect( curTimeUS );

    if ( moPackMgr )
    {
        if ( moPackMgr->HasProtoError() )
            setStateNSC( STATE_BAD_PROTOCOL_ERROR, "PacketManager Protocol Error" );
        else
        if ( moPackMgr->HasConnError() )
        {
            if ( mIsExpectingDisconnect )
            {
                mIsExpectingDisconnect = false;
                setStateNSC( STATE_DISCONNECTED );
            }
            else
                setStateNSC( STATE_GENERIC_ERROR, "PacketManager Generic Error" );
        }
    }
}

//==================================================================
const DStr &NetSysChan::GetStateStr( State st )
{
	if ( (u_int)st >= STATE_N )
        DEX_RUNTIME_ERROR( "NSChan unknown state %u", (u_int)st );

#define MAKE_STR( _F_ ) DStr(#_F_)
	static const DStr strs[] =
	{
        MAKE_STR( STATE_CREATED            ),
        MAKE_STR( STATE_CONNECTING         ),
        MAKE_STR( STATE_CONNECTED          ),
        MAKE_STR( STATE_DISCONNECTED       ),
        MAKE_STR( STATE_WAITING_DISCONNECT ),
        MAKE_STR( STATE_INVALID_ADDR_ERROR ),
        MAKE_STR( STATE_CONNECT_ERROR      ),
        MAKE_STR( STATE_GENERIC_ERROR      ),
        MAKE_STR( STATE_BAD_PROTOCOL_ERROR ),
        MAKE_STR( STATE_UNRESPONSIVE_ERROR ),
	};
#undef MAKE_STR

	return strs[ st ];
}

//==================================================================
bool NetSysChan::IsTerminated() const
{
	switch ( mState )
	{
	case STATE_DISCONNECTED:
	case STATE_WAITING_DISCONNECT:
	case STATE_INVALID_ADDR_ERROR:
	case STATE_CONNECT_ERROR:
	case STATE_GENERIC_ERROR:
	case STATE_BAD_PROTOCOL_ERROR:
	case STATE_UNRESPONSIVE_ERROR:
		return true;

	default:
		return false;
	}
}

//==================================================================
bool NetSysChan::IsConnAcceptor() const
{
    return mIsConnAcceptor;
}

//==================================================================
void NetSysChan::SetExpectedConnBreakNSC()
{
    if ( moPackMgr )
        moPackMgr->SetExpectedConnBreak();
}

//==================================================================
bool NetSysChan::GetPacketData( NetSysPackID matchID, void *pOut_data, size_t dataSize )
{
	U8 ids[] = { matchID };

    auto pack = moPackMgr->GetNextPacketMatchID8( ids, 1 );
	if NOT( pack.IsPacketValid() )
		return false;

    c_auto totSize = sizeof(NetSysPackID) + dataSize;

    c_auto [pPackData, packSize] = pack.GetPacketDataPtrAndSize();

    if ( packSize != totSize )
    {
        setStateNSC( NetSysChan::STATE_BAD_PROTOCOL_ERROR, "Packet size" );
        return false;
    }

    memcpy( pOut_data, pPackData + sizeof(NetSysPackID), dataSize );

    return true;
}

