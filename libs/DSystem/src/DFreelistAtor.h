//==================================================================
/// DFreelistAtor.h
///
/// Created by Davide Pasca - 2012/12/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DFREELISTATOR_H
#define DFREELISTATOR_H

#include <mutex>
#include "DMemPoolGrow.h"

//==================================================================
template <typename _T>
class DFreelistAtor
{
    DMemPoolGrow mMemPool;
    DVec<U8 *>   mpFreeList;

public:
    DFreelistAtor( size_t chunkN )
        : mMemPool(chunkN*sizeof(_T))
    {
        mpFreeList.reserve( chunkN );
        for (size_t i=0; i != chunkN; ++i)
            mpFreeList.push_back( (U8 *)mMemPool.Alloc(sizeof(_T)) );
    }

    _T *Alloc()
    {
        if ( mpFreeList.size() ) {
            _T *pRet = (_T *)mpFreeList.back();
            mpFreeList.pop_back();
            return pRet;
        }
        else
            return (_T *)mMemPool.Alloc(sizeof(_T));
    }

    template <typename... Args>
    _T *NewDFA(Args&&... args)
    {
        auto *p = Alloc();

        new (p) _T(std::forward<Args>(args)...);

        return p;
    }

    void Free( void *p )
    {
        mpFreeList.push_back( (U8 *)p );
    }
};

//==================================================================
// Thread-Safe edition of the free-list
template <typename _T>
class DFreelistAtorTS
{
    DMemPoolGrow mMemPool;
    DVec<U8 *>   mpFreeList;
    std::mutex   mMutex;

public:
    DFreelistAtorTS( size_t chunkN )
        : mMemPool(chunkN*sizeof(_T))
    {
        mpFreeList.reserve( chunkN );
        for (size_t i=0; i != chunkN; ++i)
            mpFreeList.push_back( (U8 *)mMemPool.Alloc(sizeof(_T)) );
    }

    _T *Alloc()
    {
        std::lock_guard<std::mutex> lock( mMutex );

        if ( mpFreeList.size() ) {
            _T *pRet = (_T *)mpFreeList.back();
            mpFreeList.pop_back();
            return pRet;
        }
        else
            return (_T *)mMemPool.Alloc(sizeof(_T));
    }

    void Free( void *p )
    {
        std::lock_guard<std::mutex> lock( mMutex );

        mpFreeList.push_back( (U8 *)p );
    }
};

#endif

