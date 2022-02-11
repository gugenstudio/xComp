//==================================================================
/// DN_URIBuild.cpp
///
/// Created by Davide Pasca - 2018/03/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DN_URIBuild.h"

//==================================================================
URIBuild::URIBuild( const DStr &str, bool genJSONParams )
    : mURIBase(str)
    , mGenJSONParams(genJSONParams)
{
}

URIBuild::URIBuild( const char *pStr, bool genJSONParams )
    : mURIBase(pStr)
    , mGenJSONParams(genJSONParams)
{
}

//==================================================================
URIBuild::~URIBuild()
{
    //delete mpBaseURIB;
}

//==================================================================
URIBuild &URIBuild::add_prefix( const DStr &str )
{
    mURIBase = str + mURIBase;
    return *this;
}

//==================================================================
URIBuild &URIBuild::append_query(
        const DStr &name, const DStr &val, bool doEnc, bool isString )
{
    mURIParams += (mURIParams.empty() ? DStr() : "&") + name + "=" + val;

    if ( mGenJSONParams )
    {
        c_auto valQuot = (isString ? DStr("\"") : DStr());

        mJSONString +=
            (mJSONString.empty() ? DStr() : ",")
            + '"' + name + '"' + ":"
            + valQuot + val + valQuot;
    }

    return *this;
}

//==================================================================
URIBuild &URIBuild::append_query( const DStr &name, int val, bool doEnc )
{
    return append_query( name, std::to_string( val ), doEnc, false );
}

//==================================================================
URIBuild &URIBuild::append_query( const DStr &name, int64_t val, bool doEnc )
{
    return append_query( name, std::to_string( val ), doEnc, false );
}

//==================================================================
URIBuild &URIBuild::append_query( const DStr &name, double val, bool doEnc )
{
    return append_query( name, std::to_string( val ), doEnc, false );
}

//==================================================================
URIBuild &URIBuild::append_query( const DStr &name, bool val, bool doEnc )
{
    return append_query( name, (val ? "true" : "false"), doEnc, false );
}

#if 0
//==================================================================
URIBuild &URIBuild::append_path( const DStr &path, bool doEnc )
{
    if ( path.empty() || path == "/" )
        return *this;

    mURIParams += (!mURIParams.empty() && mURIParams.back() != '/' ? "/" : "")
                    + (path[0] == '/' ? path.substr( 1 ) : path);
    return *this;
}
#endif

//==================================================================
DStr URIBuild::to_string() const
{
    return mURIBase + (mURIParams.empty() ? DStr() : ('?' + mURIParams));
}

