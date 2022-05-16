//==================================================================
/// DContainers.h
///
/// Created by Davide Pasca - 2008/12/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DCONTAINERS_H
#define DCONTAINERS_H

#include "DBase.h"
#include "DExceptions.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <array>

template <typename K, typename V> using DUNORD_MAP = std::unordered_map<K, V>;
template <typename T>             using DUNORD_SET = std::unordered_set<T>;

#include <string>

#include <vector>
#include <algorithm>
#include <numeric>

template <typename T> using DVec = std::vector<T>;

#include <functional>

template <typename T> using DFun = std::function<T>; // simple shortcut

//
#define D_OP_EQ_BLOCK( TYP )      friend bool operator ==( const TYP &l, const TYP &r )
#define D_OP_EQ_FIELD( A )        && l.A == r.A
#define D_OP_NEQ_IMPLEMENT( TYP ) friend bool operator !=( const TYP &l, const TYP &r )\
                                  { return !(l == r); }

//
template <class T>
void Dpush_back_unique( DVec<T> &vec, const T &val ) {
    if ( std::find( vec.begin(), vec.end(), val ) == vec.end() )
		vec.push_back( val );
}

template <typename T>
inline typename DVec<T>::iterator Dfind( DVec<T> &v, const T &val ) {
    return std::find( v.begin(), v.end(), val );
}
template <typename T>
inline typename DVec<T>::const_iterator Dfind( const DVec<T> &v, const T &val ) {
    return std::find( v.begin(), v.end(), val );
}

//==================================================================
template <class T>
size_t Dsize_bytes( const DVec<T> &v ) {
    return v.size() * sizeof(T);
}
//==================================================================
template <typename T>
inline void Dresize_loose( DVec<T> &vec, size_t newSize )
{
    if ( newSize > vec.capacity() )
    {
        auto locmax = []( auto a, auto b ) { return a > b ? a : b; };
        vec.reserve( locmax( newSize, vec.capacity()/2*3 ) );
    }
    vec.resize( newSize );
}

template <typename T>
inline void Dresize_loose( DVec<T> &vec, size_t newSize, const T &defVal )
{
    if ( newSize > vec.capacity() )
    {
        auto locmax = []( auto a, auto b ) { return a > b ? a : b; };
        vec.reserve( locmax( newSize, vec.capacity()/2*3 ) );
    }
    vec.resize( newSize, defVal );
}

//==================================================================
template <typename T>
T *Dgrow( DVec<T> &vec, size_t growN )
{
    size_t n = vec.size();
    Dresize_loose<T>( vec, n + growN );
    return vec.data() + n;
}

//==================================================================
template <typename T>
T &Dgrow( DVec<T> &vec )
{
    return *Dgrow( vec, 1 );
}

//==================================================================
template <typename T, typename _V>
void Dfind_and_erase( T &cont, const _V &val )
{
    auto it = cont.find(val);
    if ( it != cont.end() )
        cont.erase( it );
}

//==================================================================
template <size_t _NUM>
class DVecBits
{
    U8  mData[_NUM];

public:
    DVecBits()
    {
        clear();
    }

    void clear()                { memset( &mData[0], 0, sizeof(mData) ); }
    void Set( size_t i )        { DASSERT( i < _NUM ); mData[ i >> 3 ] |= 1 << (i & 7); }
    void Reset( size_t i )      { DASSERT( i < _NUM ); mData[ i >> 3 ] &= ~(1 << (i & 7)); }
    bool IsSet( size_t i ) const{ DASSERT( i < _NUM ); return !!(mData[ i >> 3 ] & (1 << (i & 7))); }
};

//==================================================================
template <typename T>
DFORCEINLINE bool DIsBitSet( const DVec<T> &v, size_t idx )
{
    static const size_t BITS_N = sizeof(T) * 8;

    const auto msk = (T)(1 << (idx & ((BITS_N)-1)));

    return !!(v[ idx / BITS_N ] & msk);
}

template <typename T>
DFORCEINLINE void DSetBit( DVec<T> &v, size_t idx, bool onOff )
{
    static const size_t BITS_N = sizeof(T) * 8;

    const auto msk = (T)(1 << (idx & ((BITS_N)-1)));

    if ( onOff )
        v[ idx / BITS_N ] |=  msk;
    else
        v[ idx / BITS_N ] &= ~msk;
}

//
#include "DataVec.h"
#include "DArray.h"
#include "DVecOwn.h"

//#define DVECVIEW_CHECK_RANGES

//==================================================================
template <typename T>
class DVecView
{
    const DVec<T> *mpRefVec {};

public:
    typedef T value_type;
    typedef const T *const_iterator;

private:
    size_t  mI1 = 0;
    size_t  mI2 = 0;

public:
    DVecView()
    {}

    DVecView( const DVec<T> &refVec )
        : mpRefVec(&refVec)
        , mI1(0)
        , mI2(refVec.size())
    {}

    DVecView(
        const DVec<T> &refVec,
        typename DVec<T>::const_iterator i1,
        typename DVec<T>::const_iterator i2 )
        : mpRefVec(&refVec)
        , mI1((size_t)(i1 - refVec.begin()))
        , mI2((size_t)(i2 - refVec.begin()))
    {
    }

    DVecView(
        const DVecView<T> &refVecView,
        const_iterator i1,
        const_iterator i2 )
        : mpRefVec(refVecView.mpRefVec)
        , mI1((size_t)(i1 - refVecView.mpRefVec->data()))
        , mI2((size_t)(i2 - refVecView.mpRefVec->data()))
    {
    }

    DVecView MakeSubViewFromEnd( ptrdiff_t offI ) const
    {
        return
        {
            *this,
            begin() +
                std::min(
                    (size_t)(std::max( (ptrdiff_t)size(), -offI ) + offI),
                    size() ),
            end()
        };
    }

public:
    size_t size() const { return mI2 - mI1; }
    bool empty() const { return size() == 0; }

          T *data()       { return mpRefVec ? mpRefVec->data() + mI1 : nullptr; }
    const T *data() const { return mpRefVec ? mpRefVec->data() + mI1 : nullptr; }

    T *data_safe( size_t i1, size_t i2 )
    {
        if (c_auto siz = size(); i1 > i2 || i1 >= siz || i2 > siz )
            DEX_OUT_OF_RANGE( "DVecView bad range for data_safe()" );

        return data();
    }

    const T *data_safe( size_t i1, size_t i2 ) const
    {
        if (c_auto siz = size(); i1 > i2 || i1 >= siz || i2 > siz )
            DEX_OUT_OF_RANGE( "DVecView bad range for data_safe()" );

        return data();
    }

    T *data_safe( ptrdiff_t i1, ptrdiff_t i2 )
    {
        if (c_auto siz = (ptrdiff_t)size(); i1 < 0 || i1 > i2 || i1 >= siz || i2 > siz )
            DEX_OUT_OF_RANGE( "DVecView bad range for data_safe()" );

        return data();
    }

    const T *data_safe( ptrdiff_t i1, ptrdiff_t i2 ) const
    {
        if (c_auto siz = (ptrdiff_t)size(); i1 < 0 || i1 > i2 || i1 >= siz || i2 > siz )
            DEX_OUT_OF_RANGE( "DVecView bad range for data_safe()" );

        return data();
    }

    const T *get_outside_value_by_index( ptrdiff_t idx ) const
    {
        c_auto refIdx = (ptrdiff_t)mI1 + idx;
        if ( refIdx < 0 || refIdx >= (ptrdiff_t)mpRefVec->size() )
            return nullptr;

        return &(*mpRefVec)[ refIdx ];
    }

    const_iterator  begin()  const  { return mpRefVec ? mpRefVec->data() + mI1 : nullptr; }
    const_iterator  end()    const  { return mpRefVec ? mpRefVec->data() + mI2 : nullptr; }
    const_iterator  cbegin() const  { return mpRefVec ? mpRefVec->data() + mI1 : nullptr; }
    const_iterator  cend()   const  { return mpRefVec ? mpRefVec->data() + mI2 : nullptr; }

    const T &front() const { return (*mpRefVec)[mI1  ]; }
    const T &back() const  { return (*mpRefVec)[mI2-1]; }

    const T &operator[]( size_t idx ) const
    {
        DASSERT(idx < size());
#ifdef DVECVIEW_CHECK_RANGES
        if NOT( idx < size() )
            DEX_OUT_OF_RANGE( "Out of range access in DVecView<> %zu of %zu", idx, size() );
#endif
        return (*mpRefVec)[ mI1 + idx ];
    }

    friend bool operator ==(const DVecView<T> &l, const DVecView<T> &r)
    {
        return
            l.mI1 == r.mI1 &&
            l.mI2 == r.mI2 &&
            l.mpRefVec == r.mpRefVec;
    }

    friend bool operator !=(const DVecView<T> &l, const DVecView<T> &r) { return !(l == r); }
};

//==================================================================
template <typename T, size_t SIZE>
class RingBuffer
{
    std::array<T,SIZE> mVals;
    size_t          mValsN = 0;

public:
    void push_back( const T &val )
    {
        mVals[ mValsN % SIZE ] = val;
        mValsN += 1;
    }

    constexpr size_t size() const { return mValsN < SIZE ? mValsN : SIZE; }

    constexpr size_t capacity() const { return SIZE; }

    void clear()    { mValsN = 0; }

    bool empty() const { return size() == 0; }

          auto &back()       { return (*this)[ size()-1 ]; }
    const auto &back() const { return (*this)[ size()-1 ]; }

    const T &operator[](size_t idx) const
    { return mVals[ mValsN < SIZE ? idx : (idx + mValsN) % SIZE ];}

          T &operator[](size_t idx)
    { return mVals[ mValsN < SIZE ? idx : (idx + mValsN) % SIZE ];}
};

//==================================================================
template <typename T, size_t SIZE>
class RingBufferH
{
    DVec<T>         mVals;
    size_t          mValsN = 0;

public:
    void push_back( const T &val )
    {
        if ( mVals.size() < SIZE )
            mVals.push_back( val );
        else
            mVals[ mValsN % SIZE ] = val;

        mValsN += 1;
    }

    constexpr size_t size() const { return mVals.size(); }

    constexpr size_t capacity() const { return SIZE; }

    void clear()    { mVals.clear(); }

    bool empty() const { return size() == 0; }

          auto &back()       { return (*this)[ size()-1 ]; }
    const auto &back() const { return (*this)[ size()-1 ]; }

    const T &operator[](size_t idx) const
    { return mVals[ mValsN < SIZE ? idx : (idx + mValsN) % SIZE ];}

          T &operator[](size_t idx)
    { return mVals[ mValsN < SIZE ? idx : (idx + mValsN) % SIZE ];}
};

//==================================================================
inline void Dreorder_vec(
                DVec<size_t> &vOrder,
                const DFun<void (size_t, size_t)> &swapFn )
{
    // for all elements to put in place
    for(size_t i = 0; i < vOrder.size(); ++i)
    {
        // while vOrder[i] is not yet in place
        // every swap places at least one element in it's proper place
        while( vOrder[i] != vOrder[vOrder[i]] )
        {
            swapFn( vOrder[i], vOrder[vOrder[i]] );
            std::swap( vOrder[i], vOrder[vOrder[i]] );
        }
    }
}

//==================================================================
inline DVec<size_t> Dmake_sorted_indices(
                        size_t n,
                        const DFun<bool (size_t,size_t)> &cmpFn )
{
    DVec<size_t> idx( n );
    std::iota( idx.begin(), idx.end(), 0 );
    std::sort( idx.begin(), idx.end(), cmpFn );

    return idx;
}

#endif

