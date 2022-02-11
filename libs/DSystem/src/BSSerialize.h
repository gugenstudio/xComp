//==================================================================
/// BSSerialize.h
///
/// Created by Davide Pasca - 2018/02/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef BSSERIALIZE_H
#define BSSERIALIZE_H

#include <algorithm>
#include "DBase.h"
#include "DContainers.h"
#include "StringUtils.h"
#include "DSerialBase.h"

//==================================================================
class BSSerial : public DSerialBase
{
public:
    DVec<char>  mData;

    BSSerial()
        : DSerialBase(DSERIAL_TYPE_BS)
    {}

    void MSerialize( const DStr &v ) override { AppendString( v ); }
    void MSerialize(      size_t v ) override { AppendNumber( v ); }
    void MSerialize(        bool v ) override { AppendNumber( (int)v ); }
    void MSerialize(         int v ) override { AppendNumber( v ); }
    void MSerialize(    uint32_t v ) override { AppendNumber( v ); }
    void MSerialize(    uint16_t v ) override { AppendNumber( v ); }
    void MSerialize(     int64_t v ) override { AppendNumber( v ); }
    void MSerialize(      double v ) override { AppendNumber( v ); }
    void MSerialize(       float v ) override { AppendNumber( v ); }
    void MSerialize( const Double2 & ) override { DASSERT(0); }
    void MSerializeEndStruct()       override { AppendEndStructSep(); }

    void AppendEndStructSep( char sep='\n' )
    {
        mData.insert( mData.end(), sep );
    }

    void AppendString( const DStr &str, char sep=' ' )
    {
        mData.insert( mData.end(), str.begin(), str.end() );
        mData.insert( mData.end(), sep );
    }

    template <typename T>
    void AppendNumber( const T val, char sep=' ' )
    {
        AppendString( std::to_string( val ), sep );
    }
};

//==================================================================
class BSDeserial : public DDeserialBase
{
public:
    DVec<char>  mData;
    size_t      mCurIdx = 0;

    BSDeserial( DVec<char> v )
        : DDeserialBase(DSERIAL_TYPE_BS)
        , mData( std::move(v) )
    {}

    void MDeserialize(     DStr &v ){ v = NextString(); }
    void MDeserialize(   size_t &v ){ StrToNumber(v, NextString());}
    void MDeserialize(     bool &v ){ StrToNumber(v, NextString());}
    void MDeserialize(      int &v ){ StrToNumber(v, NextString());}
    void MDeserialize( uint32_t &v ){ StrToNumber(v, NextString());}
    void MDeserialize( uint16_t &v ){ StrToNumber(v, NextString());}
    void MDeserialize(  int64_t &v ){ StrToNumber(v, NextString());}
    void MDeserialize(   double &v ){ StrToNumber(v, NextString());}
    void MDeserialize(    float &v ){ StrToNumber(v, NextString());}
    void MDeserialize(  Double2 &  ){ DASSERT(0);}

    DStr NextString( char sep=' ' )
    {
        const auto itSep = std::find( mData.begin() + mCurIdx, mData.end(), sep );

        if ( itSep == mData.end() )
            return {};

        const auto idxSep = itSep - mData.begin();

        auto subStr = DStr( mData.data() + mCurIdx, idxSep - mCurIdx );

        // update the index
        mCurIdx = idxSep + 1;

        return subStr;
    }
};

#endif

