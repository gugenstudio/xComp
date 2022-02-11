//==================================================================
/// DThreads.h
///
/// Created by Davide Pasca - 2018/11/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DTHREADS_H
#define DTHREADS_H

#include <future>
#include "DContainers.h"
#include "DLogOut.h"

//==================================================================
template <typename T>
inline bool DT_IsFutureReady( const std::future<T> &f )
{
    constexpr auto ZERO_MS = std::chrono::milliseconds(0);
    return f.wait_for(ZERO_MS) == std::future_status::ready;
}

// https://stackoverflow.com/a/45360400/1507238
namespace nostd
{

template<class T>
inline std::future<std::decay_t<T>> make_ready_future( T&& t ) {
  std::promise<std::decay_t<T>> pr;
  auto r = pr.get_future();
  pr.set_value(std::forward<T>(t));
  return r;
}

}

//==================================================================
class DT_QuickThreadPool
{
    const size_t            mTheadsN;
    DVec<std::future<void>> mFutures;

public:
    DT_QuickThreadPool( size_t threadsN )
        : mTheadsN(threadsN)
    {
        mFutures.reserve( threadsN );
    }

    ~DT_QuickThreadPool()
    {
        //JoinTheads();
    }

    void JoinTheads()
    {
        try {
            for (auto &f : mFutures)
                f.get();
        }
        catch(const std::exception& ex)
        {
            LogOut( LOG_ERR, "Uncaught Exception ! '" + DStr(ex.what()) + "'" );
            throw;
        }	
    }

    void AddThread( DFun<void ()> fn )
    {
        // flush what's done
        mFutures.erase(
            std::remove_if(
                mFutures.begin(), mFutures.end(),
                [&]( const auto &a ){ return DT_IsFutureReady( a ); } ),
            mFutures.end() );

        // force wait if we're full
        while ( mFutures.size() >= mTheadsN )
        {
            mFutures[0].get();
            mFutures.erase( mFutures.begin() );
        }

        mFutures.push_back( std::async( std::launch::async, fn ) );
    }
};

#endif

