//==================================================================
/// NetSysSock.h
///
/// Created by Davide Pasca - 2015/7/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef NETSYSSOCK_H
#define NETSYSSOCK_H

#include "DNET_Base.h"

//==================================================================
template <typename _T, _T RESET_VAL, void (*delCB)(_T)>
class DUniqueBase
{
    _T mVal = RESET_VAL;

public:
    DUniqueBase()                     {}
    DUniqueBase( _T id ) : mVal(id)   {}
    DUniqueBase( DUniqueBase &&from ) { *this = std::move( from ); }

    ~DUniqueBase() { reset(); }

    DUniqueBase &operator=( DUniqueBase &&from )
    {
        mVal = from.release();
        return *this;
    }

    _T get() const { return mVal; }

    void reset( _T newVal = RESET_VAL )
    {
        if ( mVal != RESET_VAL )
            delCB( mVal );

        mVal = newVal;
    }

    _T release()
    {
        auto old = mVal;
        mVal = RESET_VAL;
        return old;
    }

private:
    DUniqueBase( const DUniqueBase &from ) { DASSERT(0); }
    DUniqueBase &operator=( const DUniqueBase &from ) { DASSERT(0); }
};

//==================================================================
void NetSysSock_deleter( SOCKET val );

using NetSysSock = DUniqueBase<
                        SOCKET,
                        INVALID_SOCKET,
                        NetSysSock_deleter>;

#endif

