//==================================================================
/// DN_JSON.cpp
///
/// Created by Davide Pasca - 2018/03/09
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DBase.h"
#include "DExceptions.h"
#include "DN_JSON.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

//==================================================================
bool HasJSONField( const JSVal& val, const char *pName )
{
    return val.FindMember( pName ) != val.MemberEnd();
}

//==================================================================
std::string SerializeJSON( const JSVal& val )
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w( sb );
    val.Accept( w );
    return { sb.GetString(), sb.GetSize() };
}

