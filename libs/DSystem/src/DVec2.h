//==================================================================
/// DVec2.h
///
/// Created by Davide Pasca - 2019/09/14
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DVEC2_H
#define DVEC2_H

#include <memory>
#include <stdlib.h>

//==================================================================
template <typename T>
class DVec2
{
    T       *mpData {};
    size_t  mSize = 0;
    size_t  mCapacity = 0;

public:
    typedef const T    *const_iterator;
    typedef T          *iterator;

    class const_reverse_iterator
    {
        const T *pPtr {};
    public:
        //using IT = const_reverse_iterator;
#define IT const_reverse_iterator

        IT( const T *p={} ) : pPtr(p) {}
        IT& operator ++() const   { pPtr -= 1; return *this; }
        IT operator ++(int) const { auto t=*this; ++*this; return t; }

        friend bool operator ==(const IT &l, const IT &r) { return l.pPtr == r.pPtr; }
        friend bool operator !=(const IT &l, const IT &r) { return l.pPtr != r.pPtr; }

#undef IT
    };

    class reverse_iterator
    {
        T *pPtr {};
    public:
        //using IT = reverse_iterator;
#define IT reverse_iterator

        IT( T *p={} ) : pPtr(p) {}
        IT& operator ++() const   { pPtr -= 1; return *this; }
        IT operator ++(int) const {auto t=*this; ++*this; return t;}

        friend bool operator ==(const IT &l, const IT &r) { return l.pPtr == r.pPtr; }
        friend bool operator !=(const IT &l, const IT &r) { return l.pPtr != r.pPtr; }

#undef IT
    };

public:
    DVec2() {}
    explicit DVec2( size_t n )               { resize( n ); }
    explicit DVec2( size_t n, const T &val ) { resize( n ); for (auto &x : *this) x=val; }
    DVec2( const DVec2 &from ) { *this = from; }
    DVec2( DVec2 &&from )      { *this = std::move( from ); }

    DVec2( std::initializer_list<T> l )
    {
        for (auto it=l.begin(); it != l.end(); ++it)
            push_back( *it );
    }

    ~DVec2() { clear(); }

    DVec2 &operator=(const DVec2& from)
    {
        resize( from.size() );

        for (size_t i=0; i != from.mSize; ++i)
            (*this)[i] = from[i];

        return *this;
    }

    DVec2 &operator=(DVec2&& from)
    {
        resize( from.size() );

        for (size_t i=0; i != from.mSize; ++i)
            (*this)[i] = std::move( from[i] );

        from.resize( 0 );

        return *this;
    }

public:
    size_t size() const { return mSize; }
    size_t size_bytes() const { return mSize * sizeof(T); }

    size_t capacity() const { return mCapacity; }

    void clear()    { return resize( 0 ); }

    bool empty() const { return size() == 0; }
    bool is_full() const { return size() == capacity(); }

    void shrink_to_fit() { changeCapacity( mSize ); }

          T *data()       { return mpData; }
    const T *data() const { return mpData; }

    iterator        begin()         { return data();    }
    const_iterator  begin() const   { return data();    }

    iterator        end()           { return data() + mSize;    }
    const_iterator  end()   const   { return data() + mSize;    }

    reverse_iterator        rbegin()         { return data() + mSize - 1; }
    const_reverse_iterator  rbegin() const   { return data() + mSize - 1; }

    reverse_iterator        rend()           { return data() - 1; }
    const_reverse_iterator  rend()   const   { return data() - 1; }

    void reserve( size_t newSize )
    {
        if ( newSize > mCapacity )
            changeCapacity( newSize );
    }

    void resize( size_t newSize, const T &defVal={} )
    {
        if ( newSize == mSize )
            return;

        if ( newSize > mCapacity )
            changeCapacity( newSize );
        //expandGenerous( newSize );

        if ( newSize < mSize )
        {
            for (size_t i=newSize; i < mSize; ++i)
                (*this)[i].~T();
        }
        else
        {
            for (size_t i=mSize; i < newSize; ++i)
                *(new (data() + i) T) = defVal;
        }

        mSize = newSize;
    }

    T *grow( size_t n=1 )
    {
        size_t fromIdx = size();
        resize( fromIdx + n );
        return data() + fromIdx;
    }

    iterator erase( iterator f, iterator l )
    {
        if ( f < begin() || f >= end() || l <= f )
            DEX_OUT_OF_RANGE( "Out of bounds !" );

        const auto idxF = f - begin();
        const auto idxL = l - begin();

        for (auto i=idxF; i != idxL; ++i)
            (*this)[i].~T();

        const auto moveN = mSize - idxL;
        for (auto i=0u; i != moveN; ++i)
            (*this)[i + idxF] = std::move( (*this)[i + idxL] );

        mSize -= idxL - idxF;

        return begin() + idxF;
    }

    iterator erase( iterator it )
    {
        return erase( it, it + 1 );
    }

    void insert( iterator itBefore, T &val )
    {
        if ( mSize == 0 ) // being stl-like.. I hope !
        {
            resize( 1 );
            (*this)[0] = val;
            return;
        }
        else
        if ( itBefore == end() )
        {
            push_back( val );
            return;
        }

        if NOT( itBefore >= begin() && itBefore < end() )
            DEX_OUT_OF_RANGE( "Out of bounds !" );

        size_t  idx = itBefore - data();

        resize( mSize + 1 );

        for (size_t i=mSize; i > (idx+1); --i)
            (*this)[i-1] = std::move( (*this)[i-2] );

        (*this)[idx] = val;
    }

    void push_front( const T &val )
    {
        grow();
        for (size_t i=mSize; i > 1; --i)
            (*this)[i-1] = std::move( (*this)[i-2] );

        (*this)[0] = val;
    }

    void push_back( const T &val )
    {
        *grow() = val;
    }

    void pop_back()
    {
        DASSERT( mSize >= 1 );
        resize( mSize - 1 );
    }

    template< class... Args >
    void emplace_back( Args&&... args )
    {
        expandGenerous( mSize + 1 );

        new (data() + mSize) T( std::forward<Args>(args)... );

        mSize += 1;
    }

    iterator find( const T &val )
    {
        for (size_t i=0; i < mSize; ++i)
            if ( (*this)[i] == val )
                return data() + i;

        return end();
    }

    const   T &front() const { return (*this)[0]; }
            T &front()       { return (*this)[0]; }

    const   T &back() const  { return (*this)[mSize-1]; }
            T &back()        { return (*this)[mSize-1]; }

    const T &operator[](size_t idx) const { DASSERT(idx < mSize); return data()[idx]; }
          T &operator[](size_t idx)       { DASSERT(idx < mSize); return data()[idx]; }

    friend bool operator ==(const DVec2<T> &l, const DVec2<T> &r)
    {
        if ( l.size() != r.size() )
            return false;

        for (size_t i=0; i != l.size(); ++i)
            if ( l[i] != r[i] )
                return false;

        return true;
    }

    friend bool operator !=(const DVec2<T> &l, const DVec2<T> &r)
    {
        return !(l == r);
    }

    // see: https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare
    friend bool operator <(const DVec2<T> &l, const DVec2<T> &r)
    {
        return std::lexicographical_compare( l.begin(), l.end(),
                                             r.begin(), r.end() );
    }

private:
    void changeCapacity( size_t newCapacity )
    {
        DASSERT( newCapacity != mCapacity );

        c_auto newP = (T *)realloc( mpData, newCapacity * sizeof(T) );
        if NOT( newP )
            DEX_BAD_ALLOC( "Could not reallocate %zu elements.", newCapacity );

        mpData = newP;
        mCapacity = newCapacity;
    }

    void expandGenerous( size_t newSize )
    {
        if ( newSize > mCapacity )
            changeCapacity( newSize );
            //changeCapacity( std::max( newSize, newSize / 2 * 3 ) );
    }
};

//==================================================================
template <typename T>
inline void Dresize_loose( DVec2<T> &vec, size_t newSize )
{
    if ( newSize > vec.capacity() )
    {
        auto locmax = []( auto a, auto b ) { return a > b ? a : b; };
        vec.reserve( locmax( newSize, vec.capacity()/2*3 ) );
    }
    vec.resize( newSize );
}

template <typename T>
inline void Dresize_loose( DVec2<T> &vec, size_t newSize, const T &defVal )
{
    if ( newSize > vec.capacity() )
    {
        auto locmax = []( auto a, auto b ) { return a > b ? a : b; };
        vec.reserve( locmax( newSize, vec.capacity()/2*3 ) );
    }
    vec.resize( newSize, defVal );
}

#endif

