//==================================================================
/// DNET_HTTPFile.cpp
///
/// Created by Davide Pasca - 2013/7/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StringUtils.h"
#include "DNET_HTTPFile.h"

#if defined(WIN32)
# include <WinSock2.h>
#else
# include <sys/socket.h>
# include <netinet/tcp.h>
#endif

//==================================================================
namespace DNET {

//==================================================================
enum
{
    STATE_CREATED,
    STATE_CONNECTING,
    STATE_SENDING,
    STATE_RECEIVING,
    STATE_ERROR,
    STATE_SUCCESS
};

//==================================================================
HTTPFile::HTTPFile()
    : mStatus(STATE_CREATED)
{
}

//==================================================================
HTTPFile::~HTTPFile()
{
    if ( mSocket != INVALID_SOCKET )
        closesocket( mSocket );
}

//==================================================================
void HTTPFile::RequestFile(
        const char *pIPName,
        const char *pURL,
        u_short port,
        const OnResultFnT &onResultFn )
{
    mOnResultFn = onResultFn;

    if (   mStatus == STATE_CONNECTING
        || mStatus == STATE_SENDING
        || mStatus == STATE_RECEIVING )
    {
        onError( ERROR_GENERIC );
        return;
    }

    mStartTimeEUS = GetEpochTimeUS();

    if ( mSocket == INVALID_SOCKET )
    {
        auto retVal = mConnecter.StartConnect( pIPName, port );
        if ( retVal != Connecter::STARTRET_OK )
        {
            onError( ERROR_CONNECTION );
            return;
        }

        mStatus = STATE_CONNECTING;
    }
    else
        mStatus = STATE_SENDING;

    mReqStr = SSPrintFS(
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Accept: */*\r\n"
                "\r\n", pURL, pIPName );

    Idle();
}

//==================================================================
void HTTPFile::onError( Error err )
{
    mStatus = STATE_ERROR;
    mOnResultFn( err, mHTTPRetCode, mInData );
}

//==================================================================
void HTTPFile::handleConnecting()
{
    switch ( c_auto &ret = mConnecter.IdleConnect( 0 ); ret.first )
    {
    case Connecter::IdleRet::IDLERET_WAITING:
        break;

    case Connecter::IdleRet::IDLERET_CONNECTED:
        mSocket = mConnecter.GetSocket();

        // set socket non-blocking
        if NOT( DNET::SetNonBlocking( mSocket ) )
        {
            onError( ERROR_CONNECTION );
            return;
        }

#ifdef __APPLE__
        {
            int val = 1;
            setsockopt( mSocket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&val, sizeof(int));
        }
#endif

        // start sending
        mStatus = STATE_SENDING;
        handleSending();
        break;

    case Connecter::IdleRet::IDLERET_ERROR:
        onError( ERROR_CONNECTION );
        break;
    }
}

//==================================================================
void HTTPFile::handleSending()
{
    struct timeval tv = { 0, 0 };

    fd_set wtset;
    FD_ZERO( &wtset );
    FD_SET( mSocket, &wtset );

    // ok to typecast to (int) in windows, since it's ignored
    if ( select( (int)mSocket+1, nullptr, &wtset, 0, &tv ) == -1 )
    {
        onError( ERROR_GENERIC );
        return;
    }

    // not ready to write ?
    if NOT( FD_ISSET( mSocket, &wtset ) )
        return;

    // total size
    size_t totSize = mReqStr.size()+1;
    // what's left to send
    size_t leftToSend = totSize - mSentTotSize;

    // try to send
#if defined(WIN32) || defined(__APPLE__)
    const int flags = 0;
#else
    const int flags = MSG_NOSIGNAL;
#endif
    auto sentSize = send( mSocket, mReqStr.c_str(), (int)leftToSend, flags );
    if ( sentSize <= 0 )
    {
        // error or just no bandwidth ?
        int err = LastSockErr();
        if ( err == EWOULDBLOCK )
        {
            // graceful disconnect ?
            if ( sentSize == 0 )
            {
                if ( mSentTotSize != totSize )
                {
                    // shouldn't happen really !
                    DASSERT( 0 );
                    onError( ERROR_DATA );
                }
            }
        }
        else
        {
            onError( ERROR_DATA );
        }
    }
    else
    {
        // increase the total sent size
        mSentTotSize += (size_t)sentSize;

        // sent everything ?
        if ( mSentTotSize == totSize )
        {
            mStatus = STATE_RECEIVING;
            mInData.reserve( 512 ); // for good measure
            handleReceiving();
        }
    }
}

//==================================================================
static size_t findStrIEnd( const DVec<U8> &vec, size_t fromidx, const char *pSearch )
{
    size_t progress = 0;

    for (size_t i=fromidx; i != vec.size(); ++i)
    {
        char ch = (char)vec[i];

        // if not a match, reset the search and continue
        if ( tolower(ch) != tolower(pSearch[ progress ]) )
        {
            progress = 0;
            continue;
        }

        // advance to the next search char
        progress += 1;

        // found the whole string, if it was the last char in pSearch
        if ( pSearch[ progress ] == 0 )
        {
            return i + 1;
        }
    }

    return DNPOS;
}

//==================================================================
int HTTPFile::searchHeader()
{
    DASSERT( mDataStartIdx == DNPOS );

    // determine the HTTP return code
    {
        auto retCodeSta = findStrIEnd( mInData, 0, " " );
        if ( retCodeSta == DNPOS )
            return -1;

        auto retCodeEnd = findStrIEnd( mInData, retCodeSta, " " );
        if ( retCodeEnd == DNPOS )
            return -1;

        const auto retCodeLen = (int)retCodeEnd - (int)retCodeSta - 1;
        if ( retCodeLen >= 8 && retCodeLen <= 0 )
            return -1;

        char buff[32];
        for (int i=0; i != retCodeLen; ++i)
            buff[i] = (char)mInData[i + retCodeSta];
        buff[retCodeLen] = 0;

        mHTTPRetCode = atoi( buff );
    }

    mDataStartIdx = findStrIEnd( mInData, 0,  "\r\n\r\n" );

    // still nothing ?
    if ( mDataStartIdx == DNPOS )
        return 0;

    size_t valSta = findStrIEnd( mInData, 0, "Content-Length:" );
    // content len should be provided !
    if ( valSta == DNPOS )
        return -1;

    size_t valEnd = findStrIEnd( mInData, valSta, "\r\n" );
    // malformed !
    if ( valEnd == DNPOS )
        return -1;

    int valLen = (int)valEnd-2 - (int)valSta;

    // malformed !
    if ( valLen <= 0 || valLen > 20 )
        return -1;

    // make a nice string
    char buff[32];
    memcpy( buff, &mInData[valSta], valLen );
    buff[valLen] = 0;

    int val = atoi( buff );
    // bad size ! (should also add a max limit perhaps)
    if ( val < 0 )
        return -1;

    // set the expected data size
    mExpectedDataSize = (int)val;

    // reserve for good (hopefully !)
    mInData.reserve( mDataStartIdx + mExpectedDataSize );

    return 0;
}

//==================================================================
void HTTPFile::handleReceiving()
{
    struct timeval tv = { 0, 0 };

    fd_set rdset;
    FD_ZERO( &rdset );
    FD_SET( mSocket, &rdset );

    // ok to typecast to (int) in windows, since it's ignored
    if ( select( (int)mSocket+1, &rdset, nullptr, 0, &tv ) == -1 )
    {
        onError( ERROR_GENERIC );
        return;
    }

    // not ready to read ?
    if NOT( FD_ISSET( mSocket, &rdset ) )
        return;

    while ( true )
    {
        U8 buff[4*1024];
        auto recvSize = recv( mSocket, (char *)buff, sizeof(buff), 0 );

        // add and continue
        if ( recvSize > 0 )
        {
            mInData.insert( mInData.end(), buff, buff + recvSize );

            // have we read the header yet ?
            if ( mDataStartIdx == DNPOS )
            {
                if ( searchHeader() )
                {
                    onError( ERROR_DATA );
                    return;
                }

                // do we have it right away ?
                if ( mInData.size() == (mDataStartIdx + mExpectedDataSize) )
                {
                    // it's done, trim the header
                    mInData.erase( mInData.begin(), mInData.begin() + mDataStartIdx );

                    mStatus = STATE_SUCCESS;

                    mOnResultFn( SUCCESS, mHTTPRetCode, mInData );
                    return;
                }
            }

            continue;
        }

        // is it done ?
        if ( recvSize == 0 )
        {
            // verify with expected size
            if ( mInData.size() != (mDataStartIdx + mExpectedDataSize) )
            {
                onError( ERROR_DATA );
                return;
            }

            // it's done, trim the header
            mInData.erase( mInData.begin(), mInData.begin() + mDataStartIdx );

            mStatus = STATE_SUCCESS;

            mOnResultFn( SUCCESS, mHTTPRetCode, mInData );
            return;
        }

        // error or simply lack of data for now ?
        int err = LastSockErr();
        switch ( err )
        {
        // no data ready, no sweat
        case EWOULDBLOCK:
            return;

        // connection error.. this is bad
        case ECONNRESET:
        default:
            onError( ERROR_CONNECTION );
            return;
        }
    }
}

//==================================================================
void HTTPFile::Idle()
{
    // abort after 20 seconds
    if ( mStatus != STATE_ERROR )
        if ( (GetEpochTimeUS() - mStartTimeEUS) >= (TimeUS::ONE_SECOND()*30) )
            onError( ERROR_CONNECTION );

    switch ( mStatus )
    {
    case STATE_CONNECTING:
        handleConnecting();
        break;
    case STATE_SENDING:
        handleSending();
        break;
    case STATE_RECEIVING:
        handleReceiving();
        break;
    default:
        break;
    }
}

//==================================================================
bool HTTPFile::IsHTTPFileDone() const
{
    return mStatus == STATE_SUCCESS || mStatus == STATE_ERROR;
}

//==================================================================
size_t HTTPFile::GetPercentageComplete() const
{
    if ( mExpectedDataSize > 0 )
        return (mInData.size()-mDataStartIdx) * 100 / mExpectedDataSize;

    return 0;
}

//==================================================================
}
