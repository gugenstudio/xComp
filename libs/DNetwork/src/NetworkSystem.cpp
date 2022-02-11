//==================================================================
/// NetworkSystem.cpp
///
/// Created by Davide Pasca - 2012/4/28
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DLogOut.h"
#include "DNET_PacketManager.h"
#include "NetworkSystem.h"

#if defined(WIN32)
# include <WinSock2.h>
#endif

//#define LOG LogOut
#define LOG(FLG,FMT,...)

//==================================================================
static const TimeUS PING_INTERVAL_TIME_US = TimeUS::ONE_SECOND() * 15;
static const TimeUS MAX_TIME_FOR_UNRESPONSIVE = PING_INTERVAL_TIME_US * 8;

//==================================================================
NetSys::NetSys()
{
	DASSERT( sizeof(NetSysPackID) == 1 );

	DNET::InitializeSocket();
}

//==================================================================
void NetSys::advanceLocalIDCounter()
{
	mLocalIDCounter += 1;

	// special case ..0 is for no channel
	if ( mLocalIDCounter == 0 )
		mLocalIDCounter = LOCAL_ID_COUNT_BASE;
}

//==================================================================
bool NetSys::StartListeningNS( u_short port )
{
	DASSERT( !mListener.IsListening() );

	return mListener.StartListener( port );
}

//==================================================================
void NetSys::StopListeningNS()
{
	DASSERT( mListener.IsListening() );

	mListener.StopListener();
}

//==================================================================
bool NetSys::IsListeningNS() const
{
	return mListener.IsListening();
}

//==================================================================
bool NetSys::HasAnyChans() const
{
	return moChannels.size() != 0;
}

//==================================================================
const NetSysChan *NetSys::GetChanByID( u_int remoteGlobalID ) const
{
    for (const auto *p : moChannels)
        if ( p->mRemoteGlobalID == remoteGlobalID )
            return p;

    return nullptr;
}

//==================================================================
NetSysChan *NetSys::GetChanByID( u_int remoteGlobalID )
{
    for (auto *p : moChannels)
        if ( p->mRemoteGlobalID == remoteGlobalID )
            return p;

    return nullptr;
}

//==================================================================
DVec<NetSys::ChanDiagRow> NetSys::GetNetSysChanDiags() const
{
    DVec<ChanDiagRow> mtx;

    mtx.resize( moChannels.size()+1 );

    mtx[0].mVals.resize( 7 );

    {
        auto &head = mtx[0].mVals;
        head[0] = "Address";
        head[1] = "State";
        head[2] = "PM S Q1";
        head[3] = "PM S Q2";
        head[4] = "PM R Q";
        head[5] = "PM Snd";
        head[6] = "PM Rcv";
    }

    for (size_t i=0; i < moChannels.size(); ++i)
    {
        c_auto &chan = *moChannels[i];

        mtx[ i+1 ].mRemoteGID = chan.mRemoteGlobalID;

        auto &row = mtx[ i+1 ].mVals;

        row.resize( 7 );

        row[0] = chan.GetIPName() + ":" + DToString( chan.GetPort() );
        row[1] = chan.GetCurStateStr();

        if (c_auto *pPMan = chan.moPackMgr.get())
        {
            row[2] = DToString(pPMan->Dbg_GetSendQ1N());
            row[3] = DToString(pPMan->Dbg_GetSendQ2N());
            row[4] = DToString(pPMan->Dbg_GetRecvQN());
            row[5] = DToString(pPMan->Dbg_GetSentN());
            row[6] = DToString(pPMan->Dbg_GetRcvdN());
        }
    }

    return mtx;
}

//==================================================================
DVec<NetSys::DiagItem> NetSys::GetNetSysDiags() const
{
    DVec<DiagItem>  items;

    items.push_back({ "Channels #", DToString( moChannels.size() ), 0 });

    for (size_t i=0; i < moChannels.size(); ++i)
    {
        c_auto &chan = *moChannels[i];

        c_auto prefix = "C" + DToString( i ) + ") ";

        items.push_back({
                prefix + "Address",
                chan.GetIPName() + ":" + DToString( chan.GetPort() ),
                0 });

        items.push_back({
                prefix + "State",
                chan.GetCurStateStr(),
                0 });

        if (c_auto *pPMan = chan.moPackMgr.get())
        {
            items.push_back({ prefix + "PKM Send Q1", DToString(pPMan->Dbg_GetSendQ1N()), 0 });
            items.push_back({ prefix + "PKM Send Q2", DToString(pPMan->Dbg_GetSendQ2N()), 0 });

            items.push_back({ prefix + "PKM Recv Q", DToString(pPMan->Dbg_GetRecvQN()), 0 });

            items.push_back({ prefix + "PKM Sent/Recvd",
                    DToString(pPMan->Dbg_GetSentN()) + " / " +
                    DToString(pPMan->Dbg_GetRcvdN()),
                    0 });
        }
    }

    return items;
}

//==================================================================
bool NetSys::animate_Listen( TimeUS curTimeUS )
{
    auto retInfo = mListener.IdleListener();

    switch ( retInfo.ri_message )
    {
    case DNET::Listener::IRT_CONNECTED:

        LOG( 0, "NetSys: Accepted connection from %s:%u", retInfo.ri_IP.c_str(), retInfo.ri_port );

        // on a first connection accepted, we get a global id !
        if ( mMyGlobalID == 0 )
        {
            // assing the global ID
            mMyGlobalID = mLocalIDCounter;

            // create a direct channel to communicate with self
            moChannels.push_back( NetSysChan::CreateServer( mMyGlobalID ) );
        }

        // increase the counter, to assign to the new channel
        advanceLocalIDCounter();

        moChannels.push_back(
            NetSysChan::CreateClientRemove(
                    retInfo.ri_IP.c_str(),
                    retInfo.ri_port,
                    mLocalIDCounter,
                    retInfo.ri_acceptedSock ) );

        // send the global ID to the client !
        // NOTE: global and local ID match from the server point of view !
        {
        NetSysP::MsgHelo helo;
        helo.mSrvGlobalID = mMyGlobalID;
        helo.mCliGlobalID = mLocalIDCounter;
        SendIDAndData( *moChannels.back(), NetSysP::MSGID_HELO, helo );
        }

        mpNewAckChannels.insert( moChannels.back() );

        mpChangedStateChannels.insert( moChannels.back() );

        return true;

    case DNET::Listener::IRT_ERROR:
        LogOut( LOG_ERR, "NetSys: Error while listening !" );
        return false;

    default:
        return false;
    }
}

//==================================================================
void NetSys::AnimateNetSys( const TimeUS curTimeUS, const DFun<void ()> &processFn )
{
    // listen task at a slower rate
    if ( mListenerCheckTE.CheckTimedEvent( curTimeUS ) )
    {
        // accept up to 50 pending connections
        for (auto i=0; i < 50; ++i)
            if NOT( animate_Listen( curTimeUS ) )
                break;
    }

    //
	mConnChansIDs.clear();

    //
	for (size_t i=0; i < moChannels.size(); ++i)
	{
        auto &chan = *moChannels[i];

		// if bad, then simply skip it
		//if ( chan.IsBad() )
		//	continue;

		NetSysChan::State oldState = chan.mState;

		chan.IdleChan( curTimeUS );

		if ( chan.mState == NetSysChan::STATE_CONNECTED )
		{
			animate_General( chan, curTimeUS );
			animate_PingPong( chan, curTimeUS );
		}

		if ( chan.mState == NetSysChan::STATE_CONNECTED )
		{
			mConnChansIDs.push_back( chan.mRemoteGlobalID );
		}

		if ( oldState != chan.mState )
			mpChangedStateChannels.insert( moChannels[i] );
	}

    //
    if ( processFn )
        processFn();

    // flush all, at last
    mpNewAckChannels.clear();
	mpChangedStateChannels.clear();
	flushTerminatedChans();
}

//==================================================================
void NetSys::flushTerminatedChans()
{
	for (size_t i=0; i < moChannels.size();)
	{
        c_auto &chan = *moChannels[i];

		if ( chan.IsTerminated() )
		{
			moChannels.erase( i );
			continue;
		}

		++i;
	}
}

//==================================================================
void NetSys::flushNotConnectedChans()
{
	for (size_t i=0; i < moChannels.size();)
	{
        c_auto &chan = *moChannels[i];

		if ( chan.mState != NetSysChan::STATE_CONNECTED )
		{
			moChannels.erase( i );
			continue;
		}

		++i;
	}
}

//==================================================================
void NetSys::SendIDAndDataPtr(
            NetSysChan &chan,
            NetSysPackID id,
            const U8 *pData,
            const size_t dataSize )
{
    DNET::Packet  pack( sizeof(NetSysPackID) + dataSize, nullptr );

    c_auto [pPackData, packSize] = pack.GetPacketDataPtrAndSizeW();

    DUT::MemWriterStatic mw { pPackData, packSize };

    mw.WriteValue( id );                   // id
    mw.WriteArray<U8>( pData, dataSize );  // data
    chan.moPackMgr->SendPacket( pack );    // send
}

//==================================================================
void NetSys::animate_General( NetSysChan &chan, TimeUS curTimeUS )
{
    // if not server...
    if NOT( chan.IsConnAcceptor() )
    {
        NetSysP::MsgHelo helo;
		if ( chan.GetPacketData( NetSysP::MSGID_HELO, &helo, sizeof(helo) ) )
        {
            // server global ID
            chan.mRemoteGlobalID = helo.mSrvGlobalID;
            // client (us) global ID
            mMyGlobalID = helo.mCliGlobalID;

            mpNewAckChannels.insert( moChannels.back() );
        }
    }
}

//==================================================================
void NetSys::animate_PingPong( NetSysChan &chan, TimeUS curTimeUS )
{
	if ( !chan.mPingReqTimeUS && curTimeUS > chan.mNextPingSendTimeUS )
	{
		SendIDAndData( chan, NetSysP::MSGID_PING, curTimeUS );

        chan.mPingReqTimeUS = curTimeUS;
	}

	// get ping, send pong
    if (auto recver = GetPacket( chan, NetSysP::MSGID_PING ); recver.IsValid())
    {
        c_auto pingTotTimeS = recver.MakeMemReader().ReadValue<TimeUS>();

        SendIDAndData( chan, NetSysP::MSGID_PONG, pingTotTimeS );
    }

	// get pong, update the latency
    if (auto recver = GetPacket( chan, NetSysP::MSGID_PONG ); recver.IsValid())
    {
        c_auto pongTotTimeS = recver.MakeMemReader().ReadValue<TimeUS>();

        chan.mCurLatencyUS = curTimeUS - pongTotTimeS;

        chan.mNextPingSendTimeUS = curTimeUS + PING_INTERVAL_TIME_US;
        chan.mPingReqTimeUS = {};
    }

	if ( chan.mPingReqTimeUS )
    {
		if ( (curTimeUS - chan.mPingReqTimeUS) > MAX_TIME_FOR_UNRESPONSIVE )
            chan.mState = NetSysChan::STATE_UNRESPONSIVE_ERROR;
    }
}

//==================================================================
bool NetSys::ConnectToIP( const char *pIPName, u_short port )
{
	// don't allow connections if in a session
#if 0
	if ( mInSession )
	{
		//DASSERT( 0 );
		return false;
	}
#endif

	for (size_t i=0; i < moChannels.size(); ++i)
	{
		if ( 0 == strcasecmp( moChannels[i]->GetIPName().c_str(), pIPName ) )
		{
			if ( moChannels[i]->GetPort() == port )
			{
				LOG( 0, "NetSys: Already connected to %s:%u", pIPName, (unsigned)port );
				//DASSERT( 0 );
				return false;
			}
		}
	}

	moChannels.push_back( NetSysChan::CreateClientSelf( pIPName, port ) );

    return true;
}

//==================================================================
void NetSys::SendEmptyPacket( NetSysChan &chan, NetSysPackID id )
{
	chan.moPackMgr->SendValue( id );
}

//==================================================================
void NetSys::SendEmptyPacket( u_int remoteGlobalID, NetSysPackID id )
{
	for (size_t i=0; i < moChannels.size(); ++i)
	{
		if ( moChannels[i]->mRemoteGlobalID == remoteGlobalID )
		{
			SendEmptyPacket( *moChannels[i], id );
			return;
		}
	}

	DASSERT( 0 );
}

//==================================================================
bool NetSys::GetEmptyPacket( NetSysChan &chan, NetSysPackID matchID )
{
	U8 ids[] = { matchID };

	auto pack = chan.moPackMgr->GetNextPacketMatchID8( ids, 1 );

    return pack.IsPacketValid();
}

//==================================================================
NetSysChan *NetSys::GetEmptyPacket( NetSysPackID matchID )
{
	for (size_t i=0; i < moChannels.size(); ++i)
	{
		if ( moChannels[i]->IsConnected() )
		{
			if ( GetEmptyPacket( *moChannels[i], matchID ) )
			{
				return moChannels[i];
			}
		}
	}

	return nullptr;
}

//==================================================================
size_t NetSys::FlushPendingCustomPackets( const DFun<void (const DNET::Packet&)> &onPacketFn )
{
    size_t flushedN = 0;

    for (auto *pChan : moChannels)
    {
        if ( pChan->IsConnected() )
        {
            if (c_auto pack = pChan->moPackMgr->GetNextCustomPacket( NetSysP::MSGID_CUSTOM0 );
                       pack.IsPacketValid())
            {
                onPacketFn( pack );
                ++flushedN;
            }
        }
    }

    return flushedN;
}

//==================================================================
void NetSys::SetChanBadProtocol( NetSysChan &chan )
{
    chan.mState = NetSysChan::STATE_BAD_PROTOCOL_ERROR;
    mpChangedStateChannels.insert( &chan );
}

//==================================================================
void NetSys::SetChanGenericError( NetSysChan &chan )
{
    chan.mState = NetSysChan::STATE_GENERIC_ERROR;
    mpChangedStateChannels.insert( &chan );
}

//==================================================================
void NetSys::SetChanWaitingDisconnect( NetSysChan &chan )
{
    chan.SetExpectedConnBreakNSC();
    //chan.mState = NetSysChan::STATE_WAITING_DISCONNECT;
    //mpChangedStateChannels.insert( &chan );
    chan.mIsExpectingDisconnect = true;
}

