//==================================================================
/// DNET_HTTPFile.h
///
/// Created by Davide Pasca - 2013/7/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNET_HTTPFILE_H
#define DNET_HTTPFILE_H

#include <functional>
#include "TimeUtils.h"
#include "DContainers.h"
#include "DNET_Connecter.h"

//==================================================================
namespace DNET {

//==================================================================
class HTTPFile
{
    int         mStatus = 0;
    Connecter   mConnecter;
    SOCKET      mSocket = INVALID_SOCKET;
    DStr        mReqStr;

    TimeUS      mStartTimeEUS;

    int         mExpectedDataSize = 0;
    size_t      mDataStartIdx = DNPOS;

    size_t      mSentTotSize = 0;

    int         mHTTPRetCode = 0;
    DVec<U8>    mInData;

public:

    enum Error : int
    {
        SUCCESS = 0,
        ERROR_CONNECTION,
        ERROR_DATA,
        ERROR_GENERIC,
    };
    typedef std::function<void (Error, int, const DVec<U8> &)> OnResultFnT;

private:
    OnResultFnT mOnResultFn;

public:
    HTTPFile();
    ~HTTPFile();

    void RequestFile(
        const char *pIPName,
        const char *pURL,
        u_short port,
        const OnResultFnT &onResultFn );

    void Idle();
    size_t GetPercentageComplete() const;
    bool IsHTTPFileDone() const;

private:
    void onError( Error err );
    void handleConnecting();
    void handleSending();
    void handleReceiving();
    int searchHeader();
};

//==================================================================
}

#endif
