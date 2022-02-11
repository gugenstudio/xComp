//==================================================================
/// DNET_PacketManager.cpp
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#if defined(WIN32)
# include <WS2tcpip.h>
# include <Windows.h>
#else
# include <poll.h>
#endif

#include "TimeUtils.h"
#include "DLogOut.h"
#include "DNET.h"
#include "DNET_PacketManager.h"

#define USE_POLL
#define USE_PMMASTER
//#define DBGLOG_PMMASTER LogOut
#define DBGLOG_PMMASTER(_FLG_,_FMT_,...)

#undef LOGSTD

#define LOGSTD printf
//#define LOGSTD (void)

//==================================================================
namespace DNET
{

//==================================================================
class PMMaster
{
    std::thread             mThread;
    volatile bool           mThreadQuitMsg = false;

    std::mutex              mManagersMutex;
    DVec<PacketManager *>   mpManagers;

public:
    PMMaster()
    {
        mThread = std::thread( [this]()
        {
            DVec<PacketManager *> pUsePMs;
            DVec<pollfd> pfds;

            while NOT( mThreadQuitMsg )
            {
                if ( animThread( pUsePMs, pfds ) )
                    SleepSecs( 1.0 / 1000 );
                else
                    SleepSecs( 10.0 / 1000 );
            }
        });
    }

    //
    ~PMMaster()
    {
        try {
            if ( mThread.joinable() )
            {
                mThreadQuitMsg = true;
                mThread.join();
            }
        }
        catch ( ... )
        {
            // neutralize any pending exceptions
            LogOut( LOG_ERR, "Could not shutdown PMMaster" );
        }
    }

    //
    bool animThread( DVec<PacketManager *> &pUsePMs, DVec<pollfd> &pfds )
    {
        pUsePMs.clear();
        pfds.clear();

        std::lock_guard<std::mutex> lock( mManagersMutex );

        for (auto *pPM : mpManagers)
        {
            if ( pPM->mConnError || pPM->mProtoError || pPM->mSocket == INVALID_SOCKET )
                continue;

            pPM->mSendList.FlushInQueue();

            pUsePMs.push_back( pPM );

            pollfd pfd;
            pfd.fd = pPM->mSocket;
            pfd.events = POLLIN | (pPM->mSendList.mQueue.size() ? POLLOUT : 0);
            pfds.push_back( pfd );
        }

        if ( pfds.empty() )
            return false;

        // NOTE: we're assuming that RLIMIT_NOFILE is comfortably larger than this
        constexpr size_t CHUNK_N = 128;

        bool gotPollError = false;

        for (size_t ci=0; ci < pfds.size(); ci += CHUNK_N)
        {
            // make the range for this chunk
            c_auto i1 = ci;
            c_auto i2 = std::min( ci + CHUNK_N, pfds.size() );

            // wait only for the first chunk
            c_auto timeoutMS = (ci == 0 ? 5 : 0);
#if defined(WIN32)
            c_auto ret = WSAPoll( pfds.data() + i1, (ULONG)(i2 - i1), timeoutMS );
#else
            c_auto ret = poll( pfds.data() + i1, i2 - i1, timeoutMS );
#endif
            // debug log in case of error or if we have something
            if ( ret != 0 )
            {
                DBGLOG_PMMASTER( LOG_DBG,
                    "PMM: poll( %zi, %zu ) -> %i", (int64_t)pfds[i1].fd, pfds.size(), ret );
            }

            // continue to the next chunk in case of error..
            if ( ret == -1 )
            {
                gotPollError = true;
                continue;
            }

            // parse the tasks in this chunk
            for (size_t i=i1; i < i2; ++i)
            {
                auto *pPM = pUsePMs[i];
                c_auto &pfd = pfds[i];

                if ( (pfd.revents & POLLHUP) )
                {
                    DBGLOG_PMMASTER( LOG_DBG,
                        "PMM: PM:%p POLLHUP expected:%i", pPM, (int)pPM->mIsExpectingConnBreak );

                    if NOT( pPM->mIsExpectingConnBreak )
                        pPM->mConnError = true;

                    continue;
                }

                if ( (pfd.revents & POLLERR) )
                {
                    DBGLOG_PMMASTER( LOG_DBG | LOG_ERR,
                        "PMM: PM:%p POLLERR expected:%i", pPM, (int)pPM->mIsExpectingConnBreak );

                    if NOT( pPM->mIsExpectingConnBreak )
                        pPM->mConnError = true;

                    continue;
                }

                if ( (pfd.revents & POLLOUT) )
                {
                    DBGLOG_PMMASTER( LOG_DBG, "PMM: PM:%p POLLOUT", pPM );
                    (void)pPM->animIO_HandleSendList();
                }

                if ( (pfd.revents & POLLIN) )
                {
                    DBGLOG_PMMASTER( LOG_DBG, "PMM: PM:%p POLLIN", pPM );
                    (void)pPM->animIO_HandleReceive();
                }
            }
        }

        return !gotPollError;
    }

    //
    void AddPacketManager( PacketManager *pPM )
    {
        std::lock_guard<std::mutex> lock( mManagersMutex );
        mpManagers.push_back( pPM );
    }

    //
    void RemovePacketManager( const PacketManager *pPM )
    {
        std::lock_guard<std::mutex> lock( mManagersMutex );

        if (auto it  = std::find( mpManagers.begin(), mpManagers.end(), pPM );
                 it != mpManagers.end())
        {
            mpManagers.erase( it );
        }
    }
};

//
static PMMaster _gsPMMaster;

// pre-allock 100 packets.. should be enough (?)
//DFreelistAtorTS<Packet> Packet::msFListAtor( 100 );

//==================================================================
PacketManager::PacketManager( SOCKET socket ) : mSocket(socket)
{
#ifdef USE_PMMASTER
    _gsPMMaster.AddPacketManager( this );
#else
    mThread = std::thread( [this]()
    {
        while NOT( mThreadQuitMsg )
        {
            if ( mConnError || mProtoError )
                SleepSecs( 10.0 / 1000 );

            AnimateIO();
        }
    });
#endif
}

//==================================================================
PacketManager::~PacketManager()
{
    //DUT_STATICPROF;

#ifdef USE_PMMASTER
    _gsPMMaster.RemovePacketManager( this );
#else
    try {
        if ( mThread.joinable() )
        {
            mThreadQuitMsg = true;
            mThread.join();
        }
    }
    catch ( ... )
    {
        // neutralize any pending exceptions
    }
#endif
}

//==================================================================
bool PacketManager::IsConnectedAndHealthy() const
{
	return mSocket != INVALID_SOCKET && !mConnError && !mProtoError;
}

//==================================================================
void PacketManager::SendPacket( Packet &packet )
{
    if ( mIsDirectMode )
    {
        mRecvOutQueue.push_back( std::move(packet) );
    }
    else
    {
        std::lock_guard<std::mutex> lock( mSendList.mInQueueCS );
        mSendList.mInQueue.push_back( std::move(packet) );
    }
}

//==================================================================
void PacketManager::SendData( const void *pData, size_t dataSize )
{
    Packet pack { dataSize, pData };
    SendPacket( pack );
}

//==================================================================
Packet PacketManager::GetNextPacket()
{
	if NOT( mRecvOutQueue.size() )
		return {};

    std::lock_guard<std::mutex> lock( mRecvOutQueueCS );

	if NOT( mRecvOutQueue.size() )
		return {};

	auto outEntry = std::move( mRecvOutQueue[0] );

    mRecvOutQueue.erase( mRecvOutQueue.begin() );

	return outEntry;
}

//==================================================================
Packet PacketManager::GetNextCustomPacket( const U8 firstCustomID )
{
	if NOT( mRecvOutQueue.size() )
		return {};

    std::lock_guard<std::mutex> lock( mRecvOutQueueCS );

	if NOT( mRecvOutQueue.size() )
		return {};

	for (size_t i=0; i < mRecvOutQueue.size(); ++i)
	{
		c_auto [pPackData, packSize] = mRecvOutQueue[i].GetPacketDataPtrAndSize();
        if ( packSize < sizeof(U8) )
            continue;

		c_auto packID = *(const U8 *)pPackData;

        if ( packID >= firstCustomID )
        {
            auto tmp = std::move( mRecvOutQueue[i] );
            mRecvOutQueue.erase( mRecvOutQueue.begin()+i );

            return tmp;
        }
	}

	return {};
}
//==================================================================
Packet PacketManager::GetNextPacketMatchID32(
							const U32 matchArray[],
							size_t matchArrayN )
{
	if NOT( mRecvOutQueue.size() )
		return {};

    std::lock_guard<std::mutex> lock( mRecvOutQueueCS );

	if NOT( mRecvOutQueue.size() )
		return {};

	for (size_t i=0; i < mRecvOutQueue.size(); ++i)
	{
		c_auto [pPackData, packSize] = mRecvOutQueue[i].GetPacketDataPtrAndSize();
        if ( packSize < sizeof(U32) )
            continue;

		c_auto packID = *(const U32 *)pPackData;

		for (size_t j=0; j < matchArrayN; ++j)
		{
			if ( packID == matchArray[j] )
			{
                auto tmp = std::move( mRecvOutQueue[i] );
                mRecvOutQueue.erase( mRecvOutQueue.begin()+i );

				return tmp;
			}
		}
	}

	return {};
}

//==================================================================
Packet PacketManager::GetNextPacketMatchID8(
							const U8 matchArray[],
							size_t matchArrayN )
{
	if NOT( mRecvOutQueue.size() )
		return {};

    std::lock_guard<std::mutex> lock( mRecvOutQueueCS );

	if NOT( mRecvOutQueue.size() )
		return {};

	for (size_t i=0; i < mRecvOutQueue.size(); ++i)
	{
		c_auto [pPackData, packSize] = mRecvOutQueue[i].GetPacketDataPtrAndSize();
        if ( packSize < sizeof(U8) )
            continue;

		c_auto packID = *(const U8 *)pPackData;

		for (size_t j=0; j < matchArrayN; ++j)
		{
			if ( packID == matchArray[j] )
            {
                auto tmp = std::move( mRecvOutQueue[i] );
                mRecvOutQueue.erase( mRecvOutQueue.begin()+i );

                return tmp;
            }
		}
	}

	return {};
}

//==================================================================
static int niceSend( SOCKET sock, const U8 *pData, size_t dataSize, size_t &curSize )
{
#if defined(WIN32) || defined(__APPLE__)
    const int flags = 0;
#else
    const int flags = MSG_NOSIGNAL;
#endif
    // send what's possible
    c_auto doneSize = send(
                        sock,
                        (const char *)&pData[ curSize ],
                        (int)(dataSize - curSize),
                        flags );

    // got an error ?
    if ( doneSize == SOCKET_ERROR )
    {
        int err = LastSockErr();
        if ( err == EWOULDBLOCK )
            return 1;
        else
        {
            LOGSTD( "niceSend: Fatal socket error: %s (%i)\n", GetSockErrStr( err ), err );
            return -1;
        }
    }

    // update the sent size
    curSize += (size_t)doneSize;

    if ( curSize == dataSize )
        return 2;
    else
        return 0;
}

//==================================================================
static int niceRecv( SOCKET sock, DVec<U8> &data, size_t &curSize )
{
    // send what's possible
    c_auto doneSize = recv(
                    sock,
                    (char *)&data[ curSize ],
                    (int)(data.size() - curSize),
                    0 );

    // got an error ?
    if ( doneSize == SOCKET_ERROR )
    {
        int err = LastSockErr();
        if ( err == EWOULDBLOCK )
            return 1;
        else
        {
            LOGSTD( "niceRecv: Fatal socket error: %s (%i)\n", GetSockErrStr( err ), err );
            return -1;
        }
    }

    // update the sent size
    curSize += (size_t)doneSize;

    if ( curSize == data.size() )
        return 2;
    else
        return 0;
}

//==================================================================
bool PacketManager::animIO_HandleSendList()
{
    if ( mSendList.mQueue.empty() )
        return true;

    std::lock_guard<std::mutex> lock( mSendList.mInQueueCS );

    // as long as we have entries to send..
    while ( mSendList.mQueue.size() )
    {
        c_auto it = mSendList.mQueue.begin();

        c_auto [pPackPtr, packSiz] = it->GetPackedPtrAndSizeWithHead();

        if NOT( pPackPtr )
        {
            LogOut( LOG_ERR | LOG_DBG, "No data in the packed ! %s:%i",
                __DSHORT_FILE__, __LINE__ );

            mProtoError = true;
        }

        c_auto retVal = niceSend( mSocket, pPackPtr, packSiz, mAIOS.curSendPackDataSize );

        switch ( retVal )
        {
        case 1:     // would block
            return true;

        case -1:    // error !!
            if ( mIsExpectingConnBreak )
            {
                mIsExpectingConnBreak = false;
            }
            else
            {
                mConnError = true;
            }
            return false;

        case 2:     // done with this
            // remove it from queue
            mSendList.mQueue.erase( it );
            mAIOS.curSendPackDataSize = 0;

            mDbg_SentN += 1;
            break;
        }
    }

    return true;
}

//==================================================================
bool PacketManager::animIO_HandleReceive()
{
    auto &recvBuff                  = mAIOS.recvBuff;
    auto &curRecvPackDoneHeadSize   = mAIOS.curRecvPackDoneHeadSize;
    auto &curRecvPackDataSize       = mAIOS.curRecvPackDataSize    ;
    auto &curRecvPackDoneDataSize   = mAIOS.curRecvPackDoneDataSize;

    // ----- read the head
    if ( curRecvPackDoneHeadSize < sizeof(U32) )
    {
        // receive what's possible
        const ptrdiff_t doneSize = recv(
                        mSocket,
                        (char *)&curRecvPackDataSize,
                        (int)(sizeof(U32) - curRecvPackDoneHeadSize),
                        0 );

        // got an error ?
        if ( doneSize == SOCKET_ERROR )
        {
            int err = LastSockErr();
            if ( err != EWOULDBLOCK )
            {
                if ( mIsExpectingConnBreak )
                {
                    mIsExpectingConnBreak = false;
                }
                else
                {
                    LOGSTD( "PacketManager: Fatal socket error on receive: %s (%i)\n",
                         GetSockErrStr( err ), err );

                    mConnError = true;
                }
                return false;
            }
        }
        else
        {
            curRecvPackDoneHeadSize += (size_t)doneSize;

            // if the head is done, then set the size of the buffer for receiving
            if ( curRecvPackDoneHeadSize == sizeof(U32) )
            {
                recvBuff.resize( curRecvPackDataSize );
                if NOT( curRecvPackDataSize )
                {
                    LogOut( LOG_ERR | LOG_DBG, "PacketManager received an empty packet" );

                    // reset the head of this packet.. ready for next
                    recvBuff.clear();
                    curRecvPackDoneHeadSize = 0;
                    curRecvPackDataSize = 0;
                    curRecvPackDoneDataSize = 0;

                    // continues, but with a protocol error...
                    mProtoError = true;

                    return true;
                }
            }
        }
    }

    // ----- read the body if the head has been set
    if ( curRecvPackDoneHeadSize == sizeof(U32) )
    {
        DASSERT( curRecvPackDoneDataSize < recvBuff.size() );

        // receive what's possible
        c_auto retVal = niceRecv( mSocket, recvBuff, curRecvPackDoneDataSize );

        switch ( retVal )
        {
        case 1:     // would block
            break;

        case -1:    // error !!
            if ( mIsExpectingConnBreak )
            {
                mIsExpectingConnBreak = false;
            }
            else
            {
                mConnError = true;
            }
            return false;

        case 2:     // done with this
            {
            std::lock_guard<std::mutex> lock( mRecvOutQueueCS );

                mRecvOutQueue.emplace_back( recvBuff );
            }

            recvBuff.clear();

            // reset the head of this packet.. ready for next
            curRecvPackDoneHeadSize = 0;
            curRecvPackDataSize = 0;
            curRecvPackDoneDataSize = 0;

            mDbg_RcvdN += 1;
            break;
        }
    }

    return true;
}

//==================================================================
void PacketManager::AnimateIO()
{
    // zombie state
    if ( mConnError || mProtoError )
        return;

    mSendList.FlushInQueue();

#ifdef USE_POLL
    struct pollfd pfds[1] {};
    pfds[0].fd = mSocket;
    pfds[0].events = POLLIN | (mSendList.mQueue.size() ? POLLOUT : 0);

#if defined(WIN32)
    c_auto ret = WSAPoll( pfds, (ULONG)std::size(pfds), 5 );
#else
    c_auto ret = poll( pfds, std::size(pfds), 5 );
#endif
    if ( ret == -1 )
    {
        mConnError = true;
        return;
    }

    if ( ret == 0 )
        return;

    // handle sending
    if ( (pfds[0].revents & POLLOUT) )
        (void)animIO_HandleSendList();

    // handle receiving
    if ( (pfds[0].revents & POLLIN) )
        (void)animIO_HandleReceive();
#else
    fd_set readSocks;
    fd_set writeSocks;
    FD_ZERO( &readSocks );
    FD_ZERO( &writeSocks );

    FD_SET( mSocket, &readSocks );

    if ( mSendList.mQueue.size() )
        FD_SET( mSocket, &writeSocks );

    struct timeval  tv;
    tv.tv_sec   = 0;
    tv.tv_usec  = 5000;
    // ok to typecast to (int) in windows, since it's ignored
    int selRet = select( (int)mSocket+1, &readSocks, &writeSocks, NULL, &tv );

    if ( selRet == -1 )
    {
        mConnError = true;
        return;
    }

    // handle sending
    if ( FD_ISSET( mSocket, &writeSocks ) )
    {
        (void)animIO_HandleSendList();
    }

    // handle receiving
    if ( FD_ISSET( mSocket, &readSocks ) )
    {
        (void)animIO_HandleReceive();
    }
#endif
}

//==================================================================
}

