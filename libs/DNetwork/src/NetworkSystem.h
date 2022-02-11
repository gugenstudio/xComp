//==================================================================
/// NetworkSystem.h
///
/// Created by Davide Pasca - 2012/4/28
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef NETWORKSYSTEM_H
#define NETWORKSYSTEM_H

#include <set>
#include "NetSysBase.h"
#include "DNET_Listener.h"
#include "DNET_Connecter.h"
#include "DNET_PacketManager.h"
#include "DUT_MemFile.h"
#include "NetSysChan.h"

//==================================================================
class NetSysRecver
{
    friend class NetSys;

    DNET::Packet        mPacket;
    NetSysChan          *mpChan {};
public:
    NetSysRecver() {}

    NetSysRecver(
        DNET::Packet        &packet,
        DNET::PacketManager &packMgr,
        NetSysChan          *pChan )
        : mPacket( std::move(packet) )
        , mpChan(pChan)
    {
    }

    NetSysRecver( NetSysRecver &&from ) noexcept
    {
        *this = std::move( from );
    }

    NetSysRecver &operator=( NetSysRecver &&from ) noexcept
    {
        mPacket     = std::move( from.mPacket  );
        mpChan      = std::move( from.mpChan   );
        return *this;
    }

    ~NetSysRecver()
    {
    }

    bool IsValid() const { return mPacket.IsPacketValid(); }

    u_int GetChanRemoteGlobalID() const { return mpChan->mRemoteGlobalID; }

	      NetSysChan *GetChan()       { return mpChan; }
    const NetSysChan *GetChan() const { return mpChan; }

    // NOTE: may become invalid if Packet is moved
    DUT::MemReader MakeMemReader() const
    {
        constexpr auto HEAD_SIZE = sizeof(NetSysPackID);

        c_auto [pData, size] = mPacket.GetPacketDataPtrAndSize();

        return { pData + HEAD_SIZE, size - HEAD_SIZE };
    }

    std::pair<NetSysPackID, DUT::MemReader> MakePackIDAndMemReader() const
    {
        c_auto [pData, size] = mPacket.GetPacketDataPtrAndSize();

        return
        {
            *(const NetSysPackID *)pData,
            DUT::MemReader{
                pData + sizeof(NetSysPackID),
                size - sizeof(NetSysPackID)
            }
        };
    }
};

//==================================================================
namespace NetSysP
{
	enum {
		MSGID_HELO,
		MSGID_PING,
		MSGID_PONG,
		MSGID_CHAT,
		MSGID_CUSTOM0,
	};

    struct MsgHelo
    {
        NetSysGlobID mSrvGlobalID = 0;
        NetSysGlobID mCliGlobalID = 0;
    };
};

//==================================================================
class NetSys
{
    DNET::Listener          mListener;
    TimedEvent              mListenerCheckTE { TimeUS::ONE_SECOND() / 5 };

    static const u_int LOCAL_ID_COUNT_BASE = 100;
    u_int                   mLocalIDCounter = LOCAL_ID_COUNT_BASE;
public:
    NetSysGlobID            mMyGlobalID = 0;

	VecOwn<NetSysChan*>		moChannels;

private:
    std::set<NetSysChan*>   mpChangedStateChannels;
    std::set<NetSysChan*>   mpNewAckChannels;
	DVec<u_int>				mConnChansIDs;

public:
    //DUT::CharScreen         mCharScreen;

public:
	NetSys();

	void AnimateNetSys( const TimeUS curTimeUS, const DFun<void ()> &processFn={} );

    struct DiagItem
    {
        DStr    di_name;
        DStr    di_val;
        int     di_status {};
    };

    DVec<DiagItem> GetNetSysDiags() const;

    struct ChanDiagRow
    {
        NetSysGlobID    mRemoteGID {};
        DVec<DStr>      mVals;
    };

    DVec<ChanDiagRow> GetNetSysChanDiags() const;

	bool StartListeningNS( u_short port );
	void StopListeningNS();
	bool IsListeningNS() const;
	u_short GetListenPort() const { return mListener.GetListenPort(); }
	bool ConnectToIP( const char *pIPName, u_short port );

	void SetChanBadProtocol( NetSysChan &chan );
	void SetChanGenericError( NetSysChan &chan );
    void SetChanWaitingDisconnect( NetSysChan &chan );

	//------------------------------------------------------------------
	bool HasAnyChans() const;
	const DVec<u_int> &GetConnChansIDs() const { return mConnChansIDs; }

    const NetSysChan *GetChanByID( u_int remoteGlobalID ) const;
          NetSysChan *GetChanByID( u_int remoteGlobalID );

private:
	void flushTerminatedChans();
	void flushNotConnectedChans();

	//------------------------------------------------------------------
	NetSysRecver GetPacket( NetSysChan &chan, NetSysPackID matchID )
	{
		const U8 ids[] = { matchID };

		if (auto pack = chan.moPackMgr->GetNextPacketMatchID8( ids, 1 ); pack.IsPacketValid())
			return NetSysRecver( pack, *chan.moPackMgr, &chan );

        return {};
	}

    NetSysRecver GetCustomPacket( NetSysChan &chan )
    {
        if (auto pack = chan.moPackMgr->GetNextCustomPacket( NetSysP::MSGID_CUSTOM0 );
                 pack.IsPacketValid())
        {
            return NetSysRecver( pack, *chan.moPackMgr, &chan );
        }

        return {};
    }

	//------------------------------------------------------------------
public:
	void SendIDAndDataPtr(
                NetSysChan &chan,
                NetSysPackID id,
                const U8 *pData,
                const size_t dataSize );

	template <typename T>
	void SendIDAndData( NetSysChan &chan, NetSysPackID id, const T &data )
	{
        SendIDAndDataPtr( chan, id, (const U8 *)&data, sizeof(data) );
	}

	void SendIDAndWriter(
                NetSysChan &chan,
                NetSysPackID id,
                const DUT::MemWriterDynamic &srcMW )
	{
        SendIDAndDataPtr( chan, id, srcMW.GetDataBegin(), srcMW.GetCurSize()  );
	}

	NetSysRecver GetPacketAllChans( NetSysPackID matchID )
	{
		for (auto *pChan : moChannels)
		{
			if ( pChan->IsConnected() )
                if (auto recvr = GetPacket( *pChan, matchID ); recvr.IsValid())
                    return recvr;
		}

		return {};
	}

	NetSysRecver GetCustomPacketAllChans()
	{
		for (auto *pChan : moChannels)
		{
			if ( pChan->IsConnected() )
                if (auto recvr = GetCustomPacket( *pChan ); recvr.IsValid())
                    return recvr;
		}

		return {};
	}

	//------------------------------------------------------------------
	template <typename T>
	NetSysChan *GetPacketValueAllChans( NetSysPackID matchID, T &io_data )
	{
		for (auto *pChan : moChannels)
		{
			if ( pChan->IsConnected() )
				if ( pChan->GetPacketData( matchID, &io_data, sizeof(T) ) )
					return pChan;
		}

		return nullptr;
	}

	void SendEmptyPacket( NetSysChan &chan, NetSysPackID id );
	void SendEmptyPacket( u_int remoteGlobalID, NetSysPackID id );

	bool GetEmptyPacket( NetSysChan &chan, NetSysPackID matchID );

	NetSysChan *GetEmptyPacket( NetSysPackID matchID );

	const std::set<NetSysChan*> &GetChangedChannels() { return mpChangedStateChannels; }
	const std::set<NetSysChan*> &GetNewAckChannels() { return mpNewAckChannels; }

    size_t FlushPendingCustomPackets( const DFun<void (const DNET::Packet&)> &onPacketFn );

private:
    bool animate_Listen( TimeUS curTimeUS );
	void animate_General( NetSysChan &chan, TimeUS curTimeUS );
	void animate_PingPong( NetSysChan &chan, TimeUS curTimeUS );
	void advanceLocalIDCounter();
};

#endif
