//==================================================================
/// DMemPoolGrow.h
///
/// Created by Davide Pasca - 2013/4/8
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMEMPOOLGROW_H
#define DMEMPOOLGROW_H

#include "DBase.h"
#include "DContainers.h"

//==================================================================
class DMemPoolGrow
{
    size_t    mMaxChunkSize = 0;
    DVec<U8*> mpChunks;
    size_t    mCurChunkSize = 0;
    size_t    mUsedSize = 0;
public:
    DMemPoolGrow( size_t chunkSize ) : mMaxChunkSize(chunkSize) {}

    //DMemPoolGrow( const DMemPoolGrow  &from ) { *this = from; }
    DMemPoolGrow(       DMemPoolGrow &&from ) { *this = std::move( from ); }

    //DMemPoolGrow &operator=( const DMemPoolGrow &from )
    //{
    //    mMaxChunkSize   = from.mMaxChunkSize;
    //    mpChunks        = from.mpChunks;
    //    mCurChunkSize   = from.mCurChunkSize;
    //    mUsedSize       = from.mUsedSize;
    //    return *this;
    //}
    DMemPoolGrow &operator=( DMemPoolGrow &&from )
    {
        mMaxChunkSize   = std::move( from.mMaxChunkSize );
        mpChunks        = std::move( from.mpChunks );
        mCurChunkSize   = std::move( from.mCurChunkSize );
        mUsedSize       = std::move( from.mUsedSize );
        return *this;
    }

    ~DMemPoolGrow()
    {
        for (auto &c : mpChunks)
            delete [] c;
    }

    void *Alloc( size_t siz )
    {
        DASSERT( siz <= mMaxChunkSize );

        if ( !mpChunks.size() || (mCurChunkSize + siz) >= mMaxChunkSize )
        {
            mpChunks.push_back( new U8 [ mMaxChunkSize ] );
            mCurChunkSize = 0;
        }

        mUsedSize += siz;

        auto *p = mpChunks.back() + mCurChunkSize;
        mCurChunkSize += siz;

        return p;
    }

    size_t size_used_bytes() const { return mUsedSize; }

private:
    DMemPoolGrow( const DMemPoolGrow & ) {}
    DMemPoolGrow &operator=( const DMemPoolGrow & ) { return *this; }
};

#endif

