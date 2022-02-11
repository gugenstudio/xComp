//==================================================================
/// DVecOwn.h
///
/// Created by Davide Pasca - 2018/03/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DVECOWN_H
#define DVECOWN_H

#include <algorithm>

//==================================================================
template <typename T>
class VecOwn
{
	DVec<T>	mVec;

public:
    typedef typename DVec<T>::const_iterator const_iterator;
    typedef typename DVec<T>::iterator       iterator;

    typedef typename DVec<T>::const_reverse_iterator const_reverse_iterator;
    typedef typename DVec<T>::reverse_iterator       reverse_iterator;

public:
    VecOwn() {}
	~VecOwn() { clear(); }

    // move constructor
    VecOwn( VecOwn &&other ) { mVec = std::move( other.mVec ); }

    // move assignment operator
    VecOwn &operator=( VecOwn &&other )
    {
        // delete the existing objects
        for (auto *pObj : mVec)
        {
            if ( pObj )
                delete pObj;
        }

        // move the pointers
        mVec = std::move( other.mVec );

        return *this;
    }

    size_t size() const { return mVec.size(); }

	void resize( size_t newSize ) { mVec.resize( newSize ); }
	void reserve( size_t reserveSize ) { mVec.reserve( reserveSize ); }

    iterator        begin()         { return mVec.begin();  }
    const_iterator  begin() const   { return mVec.begin();  }

    iterator        end()           { return mVec.end();    }
    const_iterator  end()   const   { return mVec.end();    }

    reverse_iterator       rbegin()         { return mVec.rbegin();  }
    const_reverse_iterator rbegin() const   { return mVec.rbegin();  }

    reverse_iterator       rend()           { return mVec.rend();    }
    const_reverse_iterator rend()   const   { return mVec.rend();    }

	void clear()
	{
		for (size_t i=0; i < size(); ++i)
		{
			delete (*this)[i];
		}

		mVec.clear();
	}

    bool empty() const { return size() == 0; }

    void shrink_to_fit() { mVec.shrink_to_fit(); }

    iterator erase( iterator it )
    {
        size_t idx = it - mVec.begin();
        delete (*this)[idx];
        return mVec.erase( it );
    }

    iterator erase( iterator it, iterator itEnd )
    {
        size_t idx    = it    - mVec.begin();
        size_t idxEnd = itEnd - mVec.begin();

        for (size_t i=idx; i != idxEnd; ++i)
            delete (*this)[ i ];

        return mVec.erase( mVec.begin() + idx, mVec.begin() + idxEnd );
    }

    void erase( size_t idx )
    {
        delete (*this)[idx];
        mVec.erase( mVec.begin() + (ptrdiff_t)idx );
    }

	void erase( T p )
	{
		for (size_t i=0; i != size(); ++i)
        {
            if ( (*this)[i] == p )
            {
                erase( i );
                return;
            }
        }
        DASSERT( 0 );
	}

    void EraseIf( const std::function<bool (const T)> &fn )
    {
        for (auto it=begin(); it != end();)
        {
            if ( fn( *it ) )
                it = erase( it );
            else
                ++it;
        }
    }

    void push_back( const T &val ) { mVec.push_back( val ); }

    template <typename... Args>
    T &emplace_back( Args&&... args )
    {
        using _TBASE = typename std::remove_pointer<T>::type;
        mVec.push_back( new _TBASE(std::forward<Args>(args)...) );

        return back();
    }

    void pop_back()
    {
		delete (*this)[mVec.size()-1];

		mVec.erase( mVec.begin() + (mVec.size()-1) );
    }

	const	T &back() const	{ return mVec.back(); }
			T &back()			{ return mVec.back(); }

    const T &operator[]( size_t idx ) const { return mVec[ idx ]; }
          T &operator[]( size_t idx )       { return mVec[ idx ]; }

private:
    VecOwn( const VecOwn &from ) {}
    VecOwn& operator=( const VecOwn &from ) { return *this; }
};

//
template <typename T>
inline typename VecOwn<T>::iterator Dfind( VecOwn<T> &v, const T &val )
{
    return std::find( v.begin(), v.end(), val );
}


#endif

