//==================================================================
/// IMUI_InputTextNumFields.h
///
/// Created by Davide Pasca - 2020/01/19
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMUI_INPUTTEXTNUMFIELDS_H
#define IMUI_INPUTTEXTNUMFIELDS_H

#include <map>
#include "DBase.h"

//==================================================================
class IMUI_InputTextNumFields
{
    bool                    mEmptyStringForZero {};

    std::map<int*   , DStr> mMapInt;
    std::map<double*, DStr> mMapDbl;

public:
    IMUI_InputTextNumFields( bool emptyStringForZero )
        : mEmptyStringForZero(emptyStringForZero)
    {
    }

public:
    DStr *AllocField( int    *pVal ) { return allocFieldT( pVal, mMapInt ); }
    DStr *AllocField( double *pVal ) { return allocFieldT( pVal, mMapDbl ); }

    void SetValue( int    *pVal, int    newVal ) { setValueT( pVal, newVal, mMapInt ); }
    void SetValue( double *pVal, double newVal ) { setValueT( pVal, newVal, mMapDbl ); }

    void UpdateFields()
    {
        for (auto &[k, v] : mMapInt)
            *k = atoi( v.c_str() );

        for (auto &[k, v] : mMapDbl)
            *k = atof( v.c_str() );
    }

private:
    template <typename T>
    DStr *allocFieldT( T *pVal, std::map<T*,DStr> &m )
    {
        auto [it, success] = m.emplace( pVal, DStr() );
        if ( success )
            it->second = makeString( *pVal );

        return &it->second;
    }

    template <typename T>
    void setValueT( T *pVal, T newVal, std::map<T*,DStr> &m )
    {
        auto it = m.find( pVal );
        if ( it == m.end() )
            DEX_RUNTIME_ERROR( "Bad value mapping." );

        *it->first = newVal;
        it->second = makeString( newVal );
    }

    DStr makeString( double val ) const
    {
        if ( val == 0 && mEmptyStringForZero )
            return {};

        return StrFromRealCompact( val );
    }
    DStr makeString( int val ) const
    {
        if ( val == 0 && mEmptyStringForZero )
            return {};

        return std::to_string( val );
    }
};

#endif

