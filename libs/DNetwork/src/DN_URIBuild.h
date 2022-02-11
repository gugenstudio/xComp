//==================================================================
/// DURIBuild.h
///
/// Created by Davide Pasca - 2018/03/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DN_URIBUILD_H
#define DN_URIBUILD_H

#include "DBase.h"

//==================================================================
class URIBuild
{
    bool            mGenJSONParams {};

    mutable DStr    mURIBase;
    mutable DStr    mURIParams;
    mutable DStr    mJSONString;

public:
    URIBuild() {}
    URIBuild( const DStr &str, bool genJSONParams=false );
    URIBuild( const char *pStr, bool genJSONParams=false );
    ~URIBuild();

    URIBuild &add_prefix( const DStr &str );

    URIBuild &append_query( const DStr &name, const DStr &val, bool doEnc=true, bool isString=true );
    URIBuild &append_query( const DStr &name, const char *pVal, bool doEnc=true )
    {
        return append_query( name, DStr(pVal), doEnc );
    }

    URIBuild &append_query( const DStr &name, int val, bool doEnc=true );
    URIBuild &append_query( const DStr &name, int64_t val, bool doEnc=true );
    URIBuild &append_query( const DStr &name, double val, bool doEnc=true );
    URIBuild &append_query( const DStr &name, bool val, bool doEnc=true );

    //URIBuild &append_path( const DStr &path, bool doEnc=false );

    DStr to_string() const;

    const DStr &base_to_string()   const { return mURIBase; };
    const DStr &params_to_string() const { return mURIParams; };
          DStr  params_to_json()   const { return '{' + mJSONString + '}'; };
};

#endif

