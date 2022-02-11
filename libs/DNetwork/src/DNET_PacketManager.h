//==================================================================
/// DNET_PacketManager.h
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNET_PACKETMANAGER_H
#define DNET_PACKETMANAGER_H

#include <atomic>
#include <thread>
#include <memory.h>
#include "DFreelistAtor.h"
#include "DNET_Base.h"
#include "DContainers.h"

//#define DNET_PACKET_PROF

//==================================================================
namespace DNET
{

#ifdef DNET_PACKET_PROF
inline volatile ptrdiff_t _stacnt;
inline volatile ptrdiff_t _dyncnt;
inline volatile ptrdiff_t _totsiz;
inline volatile ptrdiff_t _stacnt_f;
inline volatile ptrdiff_t _dyncnt_f;
inline volatile ptrdiff_t _totsiz_f;
#endif

//==================================================================
/// Packet
//==================================================================
class Packet
{
public:
    using SIZET = U32;

    static constexpr size_t HEAD_SIZE = sizeof(SIZET);

private:
    // use a Thread-Safe free-list (NOTE: should see about performance)
    //static DFreelistAtorTS<Packet>  msFListAtor;

    size_t            mSizeFull = 0;
    std::array<U8,92> mStaticBuff {};

    //DVec<U8>      mDynBuff;
    U8            *mpDataBuff = nullptr;
public:

    Packet()
    {
    }

	Packet( size_t size, const void *pSrcData )
        : mSizeFull(HEAD_SIZE + size )
    {
        initData( size, pSrcData );
    }

	Packet( const DVec<U8> &dataBuff )
        : mSizeFull(HEAD_SIZE + dataBuff.size())
    {
        initData( dataBuff.size(), dataBuff.data() );
    }

	Packet( const Packet & ) = delete;
	void operator =( const Packet & ) = delete;

    Packet( Packet &&from ) noexcept
    {
        *this = std::move( from );
    }

    Packet &operator=( Packet &&from ) noexcept
    {
        // just in case...
        if ( &from == this )
            return *this;

        mSizeFull    = std::move( from.mSizeFull   );

        freePacketData();

        if ( from.mpDataBuff == from.mStaticBuff.data() )
        {
            mStaticBuff = std::move( from.mStaticBuff );
            mpDataBuff = mStaticBuff.data();
        }
        else
        {
            mpDataBuff = std::move( from.mpDataBuff );
        }

        from.mpDataBuff = nullptr;

        return *this;
    }

private:
    void initData( size_t size, const void *pSrcData )
    {
#ifdef DNET_PACKET_PROF
        _totsiz += (ptrdiff_t)mSizeFull;
        _totsiz_f += (ptrdiff_t)mSizeFull;
#endif

        if ( mSizeFull <= mStaticBuff.size() )
        {
            mpDataBuff = mStaticBuff.data();
#ifdef DNET_PACKET_PROF
            _stacnt += 1;
            _stacnt_f += 1;
#endif
        }
        else
        {
            //mDynBuff.resize( mSizeFull );
            //mpDataBuff = mDynBuff.data();
            mpDataBuff = new U8[ mSizeFull ];
#ifdef DNET_PACKET_PROF
            _dyncnt += 1;
            _dyncnt_f += 1;
#endif
        }

        // make header
        ((SIZET*)mpDataBuff)[0] = (SIZET)size;

        // copy data, if any
        if ( pSrcData )
            memcpy( mpDataBuff + HEAD_SIZE, pSrcData, size );
    }

public:
    ~Packet()
    {
        freePacketData();
    }
private:
    void freePacketData()
    {
#ifdef DNET_PACKET_PROF
        if ( mpDataBuff )
        {
            _totsiz_f -= (ptrdiff_t)mSizeFull;
            if ( mpDataBuff != mStaticBuff.data() )
                _dyncnt_f -= 1;
            else
                _stacnt_f -= 1;
        }
#endif
        if ( mpDataBuff && mpDataBuff != mStaticBuff.data() )
            delete [] mpDataBuff;
    }
public:
    bool IsPacketValid() const { return mpDataBuff != nullptr; }

    //void *operator new( size_t siz ) { (void)siz; return msFListAtor.Alloc(); }
    //void operator delete( void *p ) { msFListAtor.Free( p ); }

    std::pair<const U8*, size_t> GetPackedPtrAndSizeWithHead()
    {
        return { mpDataBuff, mSizeFull };
    }

    std::pair<const U8*, size_t> GetPacketDataPtrAndSize() const
    {
        return { mpDataBuff + HEAD_SIZE, mSizeFull - HEAD_SIZE };
    }

    std::pair<U8*, size_t> GetPacketDataPtrAndSizeW()
    {
        return { mpDataBuff + HEAD_SIZE, mSizeFull - HEAD_SIZE };
    }

    size_t GetPacketDataSize() const
    {
        return mSizeFull - HEAD_SIZE;
    }
};

//===============================================================
///
//===============================================================
class PacketManager
{
    friend class    PMMaster;

    class Queue
    {
    public:
        DVec<Packet>  mInQueue;
        std::mutex    mInQueueCS;
        DVec<Packet>  mQueue;

        //==================================================================
        void FlushInQueue()
        {
            if NOT( mInQueue.size() )
                return;

            std::lock_guard<std::mutex> lock( mInQueueCS );

            for (size_t i=0; i < mInQueue.size(); ++i)
                mQueue.push_back( std::move( mInQueue[i] ) );

            mInQueue.clear();
        }
    };

    bool               mIsDirectMode = false;

    SOCKET             mSocket = INVALID_SOCKET;
    Queue              mSendList;

    std::mutex         mRecvOutQueueCS;
    DVec<Packet>       mRecvOutQueue;

    bool               mConnError = false;
    std::atomic<bool>  mIsExpectingConnBreak { false };
    bool               mProtoError = false;

    std::thread        mThread;
    volatile bool      mThreadQuitMsg = false;

    size_t             mDbg_SentN { 0 };
    size_t             mDbg_RcvdN { 0 };

public:
    //===============================================================
    PacketManager( SOCKET socket );
    PacketManager() : mIsDirectMode(true) {}
    ~PacketManager();

    bool IsConnectedAndHealthy() const;
    bool HasConnError() const { return mConnError; }
    bool HasProtoError() const { return mProtoError; }

    void SetExpectedConnBreak() { mIsExpectingConnBreak = true; }

    c_auto Dbg_GetSendQ1N() const { return mSendList.mInQueue.size(); }
    c_auto Dbg_GetSendQ2N() const { return mSendList.mQueue.size(); }
    c_auto Dbg_GetRecvQN() const  { return mRecvOutQueue.size(); }

    c_auto Dbg_GetSentN() const  { return mDbg_SentN; }
    c_auto Dbg_GetRcvdN() const  { return mDbg_RcvdN; }

    void SendData( const void *pData, size_t dataSize );

    template<class T>
    void SendValue( const T &val )
    {
        SendData( &val, sizeof(val) );
    }

    void SendPacket( Packet &packet );

    Packet GetNextPacket();
    Packet GetNextCustomPacket( const U8 firstCustomID );

    Packet GetNextPacketMatchID32(
        const U32 matchArray[],
        size_t matchArrayN );

    Packet GetNextPacketMatchID8(
        const U8 matchArray[],
        size_t matchArrayN );

private:
    struct AnimateIOState
    {
        size_t  curRecvPackDoneHeadSize = 0;
        size_t  curRecvPackDataSize = 0;
        size_t  curRecvPackDoneDataSize = 0;
        size_t  curSendPackDataSize = 0;

        DVec<U8> recvBuff;
    } mAIOS;

    void AnimateIO();

    bool animIO_HandleSendList();
    bool animIO_HandleReceive();
};

//==================================================================
}

#endif
