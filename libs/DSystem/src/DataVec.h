//==================================================================
/// DataVec.h
///
/// Created by Davide Pasca - 2018/03/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DATAVEC_H
#define DATAVEC_H

#include <vector>
#include <stdint.h>
#include "DAssert.h"

//==================================================================
class DataVec
{
    size_t               mTypeSize = 0;
    size_t               mSize     = 0;
    std::vector<uint8_t> mData;

    size_t   mSubCnt = 0;
    u_int    mNativeType = 0;
    bool     mIsNormalized = false;

public:
    DataVec() {}

    // copy
    DataVec( const DataVec &from )
    {
        *this = from;
    }
    DataVec &operator=( const DataVec &from )
    {
        mTypeSize = from.mTypeSize;
        mSize     = from.mSize;
        mData     = from.mData;

        mSubCnt       = from.mSubCnt      ;
        mNativeType   = from.mNativeType  ;
        mIsNormalized = from.mIsNormalized;

        return *this;
    }

    // move
    DataVec( DataVec &&from ) noexcept
    {
        *this = std::move( from );
    }
    DataVec &operator=( DataVec &&from ) noexcept
    {
        CopyFormatDV( from );

        mData = std::move( from.mData );

        mSize = from.mSize;
        from.mSize = 0;

        return *this;
    }

    void InitDV(
            size_t typeSize    ,
            size_t subCnt      ,
            u_int  nativeType  ,
            bool   isNormalized )
    {
        DASSERT( mData.size() == 0 );
        DASSERT( mTypeSize == 0 );
        mTypeSize = typeSize;

        mSubCnt       = subCnt      ;
        mNativeType   = nativeType  ;
        mIsNormalized = isNormalized;
    }

    void CopyFormatDV( const DataVec &from )
    {
        mTypeSize     = from.mTypeSize    ;

        mSubCnt       = from.mSubCnt      ;
        mNativeType   = from.mNativeType  ;
        mIsNormalized = from.mIsNormalized;
    }

    void resize( size_t size )
    {
        mData.resize( size * mTypeSize );
        mSize = size;
    }

    void reserve( size_t size )
    {
        mData.reserve( size * mTypeSize );
    }

    void clear() { resize(0); }

    void shrink_to_fit() { mData.shrink_to_fit(); }

    size_t size() const { return mSize; }
    size_t type_size() const { return mTypeSize; }

    size_t GetSubCnt() const       { return  mSubCnt        ; }
    u_int  GetNativeType() const   { return  mNativeType    ; }
    bool   GetIsNormalized() const { return  mIsNormalized  ; }

    bool empty() const { return size() == 0; }

          uint8_t *data() { return mData.data(); }
    const uint8_t *data() const { return mData.data(); }

    template <typename T>
    T *dataT()
    {
        DASSERT( sizeof(T) == mTypeSize );
        return (T*)mData.data();
    }

    template <typename T>
    const T *dataT() const
    {
        DASSERT( sizeof(T) == mTypeSize );
        return (const T*)mData.data();
    }

    template <typename T>
    void push_back( const T &val )
    {
        DASSERT( sizeof(T) == mTypeSize );

        Dgrow( mData, mTypeSize );
        *((T *)data() + mSize) = val;
        mSize += 1;
    }

    const uint8_t *get_elem_ptr( size_t idx ) const
    {
        DASSERT( idx < mSize );
        return data() + idx * mTypeSize;
    }
    uint8_t *get_elem_ptr( size_t idx )
    {
        DASSERT( idx < mSize );
        return data() + idx * mTypeSize;
    }

    template <typename T>
    const T &get_by_idx( size_t idx ) const
    {
        DASSERT( sizeof(T) == mTypeSize && idx < mSize );
        return *(const T *)(data() + idx * sizeof(T));
    }

    template <typename T>
    void set_by_idx( const T &val, size_t idx )
    {
        DASSERT( sizeof(T) == mTypeSize && idx < mSize );

        auto *pDes = (T *)(data() + idx * sizeof(T));
        *pDes = val;
    }

    void append( const void *pFromData, size_t n )
    {
        size_t totBytesN = mTypeSize * n;
        auto *pDes = Dgrow( mData, totBytesN );
        memcpy( pDes, pFromData, totBytesN );
        mSize += n;
    }

    void push_back( const DataVec &fromVec, size_t fromIdx )
    {
        DASSERT( mTypeSize == fromVec.type_size() );
        DASSERT( fromIdx < fromVec.size() );

        const auto *pSrc = fromVec.get_elem_ptr( fromIdx );
        auto *pDes = Dgrow( mData, mTypeSize );
        memcpy( pDes, pSrc, mTypeSize );
        mSize += 1;
    }

    void copy_to_idx( size_t toIdx, const DataVec &fromVec, size_t fromIdx )
    {
        DASSERT( mTypeSize == fromVec.type_size() );
        DASSERT( fromIdx < fromVec.size() );

        const auto *pSrc = fromVec.get_elem_ptr( fromIdx );
        auto *pDes = get_elem_ptr( toIdx );
        memcpy( pDes, pSrc, mTypeSize );
    }

    void copy_to_idx( size_t toIdx, size_t fromIdx )
    {
        DASSERT( toIdx < size() && fromIdx < size() );
        const auto *pSrc = get_elem_ptr( fromIdx );
        auto *pDes = get_elem_ptr( toIdx );
        memcpy( pDes, pSrc, mTypeSize );
    }
};

//==================================================================
template <typename T>
T *Dgrow( DataVec &vec, size_t growN=1 )
{
    size_t n = vec.size();

    vec.resize( n + growN );
    return (T *)vec.data() + n;
}

#endif

