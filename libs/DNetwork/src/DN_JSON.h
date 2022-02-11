//==================================================================
/// DN_JSON.h
///
/// Created by Davide Pasca - 2018/03/09
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DN_JSON_H
#define DN_JSON_H

#include <memory>
#include <functional>
#include "DExceptions.h"
#include "rapidjson/document.h"

using JSVal = rapidjson::Value;
using JSDoc = rapidjson::Document;
using JSRes = std::shared_ptr<rapidjson::Document>;

inline const JSVal &GetJSONObj( const JSRes &res )             { return *res; }
inline const JSVal &GetJSONObj( const rapidjson::Document &d ) { return d; }

inline bool         AsJSONBool( const JSVal& val )   { return val.GetBool(); }
inline int          AsJSONInt( const JSVal& val )    { return val.GetInt(); }
inline unsigned     AsJSONUInt( const JSVal& val )   { return val.GetUint(); }
inline int64_t      AsJSONInt64( const JSVal& val )  { return val.GetInt64(); }
inline uint64_t     AsJSONUInt64( const JSVal& val ) { return val.GetUint64(); }
inline double       AsJSONDouble( const JSVal& val ) { return val.GetDouble(); }
inline std::string  AsJSONString( const JSVal& val ) { return val.GetString(); }

inline size_t GetJSONArrSize( const JSVal& val )                 { return val.Size(); }
inline const JSVal &GetJSONArrIdx( const JSVal& val, size_t idx ){
    return val[ (rapidjson::SizeType)idx ]; }

inline bool IsJSONArray( const JSVal& val )    { return val.IsArray(); }
inline bool IsJSONObject( const JSVal& val )   { return val.IsObject(); }
inline bool IsJSONString( const JSVal& val )   { return val.IsString(); }
inline bool IsJSONBool( const JSVal& val )     { return val.IsBool(); }
inline bool IsJSONInt( const JSVal& val )      { return val.IsInt(); }

//
bool HasJSONField( const JSVal& val, const char *pName );

inline const JSVal& GetJSONValue( const JSVal& val, const char *pName )
{
    auto it = val.FindMember( pName );
    if ( it == val.MemberEnd() )
        DEX_RUNTIME_ERROR( "Could not find JSON value '%s'", pName );

    return it->value;
}

inline std::string GetJSONString( const JSVal& val, const char *pName )
{
    return GetJSONValue( val, pName ).GetString();
}

inline bool GetJSONBool( const JSVal& val, const char *pName ) {
    return GetJSONValue( val, pName ).GetBool(); }

inline int GetJSONInt( const JSVal& val, const char *pName ) {
    return AsJSONInt( GetJSONValue( val, pName ) ); }

inline int64_t GetJSONInt64( const JSVal& val, const char *pName ) {
    return AsJSONInt64( GetJSONValue( val, pName ) ); }

inline double GetJSONDouble( const JSVal& val, const char *pName ) {
    return AsJSONDouble( GetJSONValue( val, pName ) ); }

inline const JSVal *GetJSONValuePtr( const JSVal& val, const char *pName )
{
    if (auto it = val.FindMember( pName ); it != val.MemberEnd())
        return &it->value;

    return nullptr;
}

inline auto GetJSONStringSafe = []( const auto &obj, const auto &name )
{
    const auto *pVal = GetJSONValuePtr( obj, name );
    return pVal && IsJSONString( *pVal ) ? AsJSONString( *pVal ) : std::string();
};

inline void ForEachJSONObject(
                const JSVal& obj,
                const std::function<void (const char *,const JSVal &)> &fn )
{
    for (auto m=obj.MemberBegin(); m != obj.MemberEnd(); ++m)
        fn( m->name.GetString(), m->value );
}

//
std::string SerializeJSON( const JSVal& val );

#endif

