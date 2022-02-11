//==================================================================
/// SerializeJS.h
///
/// Created by Davide Pasca - 2018/02/26
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef SERIALIZEJS_H
#define SERIALIZEJS_H

#include <array>
#include "DN_JSON.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"
#include "DExceptions.h"
#include "DContainers.h"
#include "FileUtils.h"
#include "DSerialBase.h"
#include "DVector.h"

//==================================================================
class SerialJS : public DSerialBase
{
    rapidjson::PrettyWriter<rapidjson::StringBuffer>    mWP;
    rapidjson::Writer<rapidjson::StringBuffer>          mWS;

    bool mUseP = true;

public:
    SerialJS()
		: DSerialBase(DSERIAL_TYPE_JS)
	{}

    SerialJS( rapidjson::StringBuffer &sb, bool usePretty=true )
        : DSerialBase(DSERIAL_TYPE_JS)
        , mWP(sb)
        , mWS(sb)
        , mUseP(usePretty)
    {}

    void MSerialize( const DStr &v ) override { if (mUseP) mWP.String( v.c_str() ); else
                                                           mWS.String( v.c_str() ); }
    void MSerialize(      size_t v ) override { if (mUseP) mWP.Uint64( v );         else
                                                           mWS.Uint64( v );         }
    void MSerialize(        bool v ) override { if (mUseP) mWP.Bool( v );           else
                                                           mWS.Bool( v );           }
    void MSerialize(         int v ) override { if (mUseP) mWP.Int( (int)v );       else
                                                           mWS.Int( (int)v );       }
#if defined(__clang__)
    void MSerialize(    uint64_t v ) override { if (mUseP) mWP.Uint64( (uint64_t)v ); else
                                                           mWS.Uint64( (uint64_t)v ); }
#endif
    void MSerialize(    uint32_t v ) override { if (mUseP) mWP.Uint( (uint32_t)v ); else
                                                           mWS.Uint( (uint32_t)v ); }
    void MSerialize(    uint16_t v ) override { if (mUseP) mWP.Uint( (uint16_t)v ); else
                                                           mWS.Uint( (uint16_t)v ); }
    void MSerialize(     int64_t v ) override { if (mUseP) mWP.Int64( v );          else
                                                           mWS.Int64( v );          }
    void MSerialize(      double v ) override { if (mUseP) mWP.Double( v );         else
                                                           mWS.Double( v );         }
    void MSerialize(       float v ) override { if (mUseP) mWP.Double( v );         else
                                                           mWS.Double( v );         }

    void MSerialize( const Double2 &v ) override
    {
        MSerializeArrayStart();
        if (mUseP) { mWP.Double(v[0]); mWP.Double(v[1]); } else
                   { mWS.Double(v[0]); mWS.Double(v[1]); }
        MSerializeArrayEnd();
    }

    void MSerializeObjectStart()     override { if (mUseP) mWP.StartObject();       else
                                                           mWS.StartObject();       }
    void MSerializeObjectEnd()       override { if (mUseP) mWP.EndObject();         else
                                                           mWS.EndObject();         }
    void MSerializeArrayStart()      override { if (mUseP) mWP.StartArray();        else
                                                           mWS.StartArray();        }
    void MSerializeArrayEnd()        override { if (mUseP) mWP.EndArray();          else
                                                           mWS.EndArray();          }
};

//==================================================================
class DeserialJS : public DDeserialBase
{
public:
    const JSVal &mR;

    DeserialJS( const JSVal &jsval )
        : DDeserialBase(DSERIAL_TYPE_JS)
        , mR(jsval)
    {}

    void MDeserialize(     DStr &v ) override { v = AsJSONString(mR); }
    void MDeserialize(   size_t &v ) override { v = (size_t)mR.GetUint64(); }
    void MDeserialize(     bool &v ) override { v = AsJSONBool(mR); }
    void MDeserialize(      int &v ) override { v = AsJSONInt(mR); }
#if defined(__clang__)
    void MDeserialize( uint64_t &v ) override { v = (uint64_t)AsJSONUInt64(mR); }
#endif
    void MDeserialize( uint32_t &v ) override { v = (uint32_t)AsJSONUInt(mR); }
    void MDeserialize( uint16_t &v ) override { v = (uint16_t)AsJSONUInt(mR); }
    void MDeserialize(  int64_t &v ) override { v = AsJSONInt64(mR); }
    void MDeserialize(   double &v ) override { v = AsJSONDouble(mR); }
    void MDeserialize(    float &v ) override { v = (float)AsJSONDouble(mR); }
    void MDeserialize(  Double2 &v ) override
    {
        DASSERT( mR.IsArray() );
        for (size_t i=0; i != 2; ++i)
        {
            DeserialJS subDes( mR[(rapidjson::SizeType)i] );
            auto tmp = static_cast<double>(v[i]);
            Deserialize( subDes, tmp );
            v[i] = tmp;
        }
    }

    const JSVal &GetJSVal() const { return mR; }

    friend bool HasJSONField( const DeserialJS &des, const char *pName )
    {
        return HasJSONField( des.mR, pName );
    }

    friend std::string GetJSONString( const DeserialJS &des, const char *pName )
    {
        return GetJSONString( des.mR, pName );
    }
};

//==================================================================
template <typename T>
inline void Serialize( SerialJS &ser, const DVec<T> &v )
{
    ser.MSerializeArrayStart();
    for (const auto &x : v)
        Serialize( ser, x );
    ser.MSerializeArrayEnd();
}

template <typename T>
inline void Serialize( SerialJS &ser, const DVec<uptr<T>> &v )
{
    ser.MSerializeArrayStart();
    for (const auto &x : v)
        Serialize( ser, *x );
    ser.MSerializeArrayEnd();
}

template <typename T>
inline void Serialize( SerialJS &ser, const DVec<sptr<T>> &v )
{
    ser.MSerializeArrayStart();
    for (const auto &x : v)
        Serialize( ser, *x );
    ser.MSerializeArrayEnd();
}

template <typename T, size_t N>
inline void Serialize( SerialJS &ser, const std::array<T,N> &v )
{
    ser.MSerializeArrayStart();
    for (const auto &x : v)
        Serialize( ser, x );
    ser.MSerializeArrayEnd();
}

template <typename T>
inline void SerializeMember( SerialJS &ser, const DStr &name, const T &v )
{
    ser.MSerialize( name );
    Serialize( ser, v );
}

template <typename T>
inline void SerializeMember( SerialJS &ser, const DStr &name, const DVec<T> &v )
{
    ser.MSerialize( name );
    Serialize( ser, v );
}

#if 0
template <typename T>
inline void SerializeMember( SerialJS &ser, const DStr &name, const DVec<uptr<T>> &v )
{
    ser.MSerialize( name );
    Serialize( ser, v );
}
#endif

template <typename T, size_t N>
inline void SerializeMember( SerialJS &ser, const DStr &name, const std::array<T,N> &v )
{
    ser.MSerialize( name );
    Serialize( ser, v );
}

//==================================================================
template <typename T>
inline void Deserialize( const DeserialJS &des, DVec<T> &v )
{
    // just ignore if it's null
    if ( des.mR.IsNull() )
        return;

    DASSERT( des.mR.IsArray() );//&& v.empty() );

    v.resize( des.mR.Size() );
    for (size_t i=0; i != v.size(); ++i)
	{
		DeserialJS subDes( des.mR[(rapidjson::SizeType)i] );
#if 0
        Deserialize( subDes, *(T *)&v[i] );
#else
        //auto tmp = static_cast<T>(v[i]);
        Deserialize( subDes, v[i] );
        //v[i] = tmp;
#endif
	}
}

//==================================================================
template <typename T>
inline void Deserialize( const DeserialJS &des, DVec<sptr<T>> &v )
{
    // just ignore if it's null
    if ( des.mR.IsNull() )
        return;

    DASSERT( des.mR.IsArray() );//&& v.empty() );

    v.resize( des.mR.Size() );
    for (size_t i=0; i != v.size(); ++i)
	{
		DeserialJS subDes( des.mR[(rapidjson::SizeType)i] );
        v[i] = std::make_shared<T>();
        Deserialize( subDes, *v[i] );
	}
}

template <typename T, size_t N>
inline void Deserialize( const DeserialJS &des, std::array<T,N> &v )
{
    DVec<T> tmp;
    Deserialize( des, tmp );
    if ( tmp.size() != N )
        DEX_RUNTIME_ERROR( "Bad array size. expected:%zu, got:%zu", N, tmp.size() );

    for (size_t i=0; i < std::min(N, tmp.size()); ++i)
        v[i] = tmp[i];
}

template <typename T>
inline void DeserializeMember( const DeserialJS &des, const DStr &name, T &out_v )
{
    // see if we have the member, otherwise ignore it
    c_auto it = des.mR.FindMember( name.c_str() );
    if ( it == des.mR.MemberEnd() )
        return;

    c_auto &jv = it->value;
	DeserialJS subDes( jv );
    Deserialize( subDes, out_v );
}

template <typename T>
inline void DeserializeMember( const DeserialJS &des, const DStr &name, DVec<T> &out_v )
{
    // see if we have the member, otherwise ignore it
    c_auto it = des.mR.FindMember( name.c_str() );
    if ( it == des.mR.MemberEnd() )
        return;

    c_auto &jv = it->value;
    DeserialJS subDes( jv );
    Deserialize( subDes, out_v );
}

template <typename T, size_t N>
inline void DeserializeMember( const DeserialJS &des, const DStr &name, std::array<T,N> &out_v )
{
    // see if we have the member, otherwise ignore it
    c_auto it = des.mR.FindMember( name.c_str() );
    if ( it == des.mR.MemberEnd() )
        return;

    c_auto &jv = it->value;
    DeserialJS subDes( jv );
    Deserialize( subDes, out_v );
}

//==================================================================
template <typename T>
DStr SerializeToString( const T &obj )
{
    rapidjson::StringBuffer sb;
    SerialJS ser( sb, false );
    Serialize( ser, obj );

    return { sb.GetString() };
}

//==================================================================
template <typename T>
T &DeserializeFromString( const DStr &str, T &obj )
{
    rapidjson::Document d;
    d.Parse( str.data() );
    if ( d.HasParseError() )
    {
        DEX_RUNTIME_ERROR(
            "Error while parsing '%s' (error code #%u)",
            str.c_str(),
            (unsigned)d.GetParseError() );
    }

	DeserialJS subDes( GetJSONObj(d) );
    Deserialize( subDes, obj );
    return obj;
}

//==================================================================
template <typename T>
void SerializeToFile( const DStr &fname, const T &obj )
{
    rapidjson::StringBuffer sb;
    SerialJS ser( sb );
    Serialize( ser, obj );
    FU_WriteStringToFile( fname, DStr(sb.GetString()) );
}

//==================================================================
template <typename T>
void DeserializeFromFile( const DStr &fname, T &obj )
{
    DeserializeFromString( FU_ReadStringFromFile(fname), obj );
}

//==================================================================
#define SERIALIZE_MEMBER( _W_, _S_, _F_ )   SerializeMember( _W_, #_F_, _S_._F_ )
#define DESERIALIZE_MEMBER( _R_, _S_, _F_ ) DeserializeMember( _R_, #_F_, _S_._F_ )

#define SERIALIZE_THIS_MEMBER( _W_, _F_ )   SerializeMember( _W_, #_F_, _F_ )
#define DESERIALIZE_THIS_MEMBER( _R_, _F_ ) DeserializeMember( _R_, #_F_, _F_ )

#endif

