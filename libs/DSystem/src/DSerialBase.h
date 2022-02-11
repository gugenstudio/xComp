//==================================================================
/// DSerialBase.h
///
/// Created by Davide Pasca - 2019/11/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSERIALBASE_H
#define DSERIALBASE_H

#include "DContainers.h"
#include <array>
#include <map>
#include <unordered_map>
#include "DVector.h"

enum DSerialType
{
    DSERIAL_TYPE_DMEM,
    DSERIAL_TYPE_BS,
    DSERIAL_TYPE_JS,
};

//==================================================================
class DSerialBase
{
    const DSerialType mDSType;

public:
    DSerialBase( DSerialType type )
        : mDSType(type)
    {}

    DSerialType GetDSType() const { return mDSType; }

    virtual void MSerialize( const DStr & ) {}
    virtual void MSerialize(      size_t  ) {}
    virtual void MSerialize(        bool  ) {}
    virtual void MSerialize(         int  ) {}
#if defined(__clang__)
    virtual void MSerialize(    uint64_t  ) {}
#endif
    virtual void MSerialize(    uint32_t  ) {}
    virtual void MSerialize(    uint16_t  ) {}
    virtual void MSerialize(     int64_t  ) {}
    virtual void MSerialize(      double  ) {}
    virtual void MSerialize(       float  ) {}
    virtual void MSerialize(const Double2&){}
    virtual void MSerializeEndStruct()       {}
    virtual void MSerializeObjectStart()     {}
    virtual void MSerializeObjectEnd()       {}
    virtual void MSerializeArrayStart()      {}
    virtual void MSerializeArrayEnd()        {}

    friend void Serialize( DSerialBase &ser,       DStr  v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,      size_t v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,        bool v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,         int v ){ser.MSerialize( v );}
#if defined(__clang__)
    friend void Serialize( DSerialBase &ser,    uint64_t v ){ser.MSerialize( v );}
#endif
    friend void Serialize( DSerialBase &ser,    uint32_t v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,    uint16_t v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,     int64_t v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,      double v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,       float v ){ser.MSerialize( v );}
    friend void Serialize( DSerialBase &ser,     Double2 v ){ser.MSerialize( v );}
    friend void SerializeEndStruct( DSerialBase &ser )      {ser.MSerializeEndStruct();}

    template <typename T>
    void MSerialize( const DVec<T> &v )
    {
        MSerialize( v.size() );
        for (const auto &x : v)
            Serialize( *this, x );
    }

    template <typename T>
    void MSerialize( const DVecView<T> &v )
    {
        MSerialize( v.size() );
        for (const auto &x : v)
            Serialize( *this, x );
    }

    template <typename T, size_t N>
    void MSerialize( const T (&in_v)[N] )
    {
        for (size_t i=0; i != N; ++i)
            Serialize( *this, in_v[i] );
    }

    template <typename T, size_t N>
    void MSerialize( const std::array<T,N> &in_v )
    {
        for (size_t i=0; i != N; ++i)
            Serialize( *this, in_v[i] );
    }

    template <typename K, typename V>
    void MSerialize( const std::map<K,V> &m )
    {
        MSerialize( m.size() );
        for (const auto &[k, v] : m)
        {
            Serialize( *this, k );
            Serialize( *this, v );
        }
    }

    template <typename K, typename V>
    void MSerialize( const std::unordered_map<K,V> &m )
    {
        MSerialize( m.size() );
        for (const auto &[k, v] : m)
        {
            Serialize( *this, k );
            Serialize( *this, v );
        }
    }

    template <typename T>
    friend void Serialize( DSerialBase &ser, const DVec<T> &v ) { ser.MSerialize( v ); }

    template <typename T>
    friend void Serialize( DSerialBase &ser, const DVecView<T> &v ) { ser.MSerialize( v ); }

    template <typename T, size_t N>
    friend void Serialize( DSerialBase &ser, const T (&in_v)[N] ) { ser.MSerialize( in_v ); }

    template <typename T, size_t N>
    friend void Serialize( DSerialBase &ser, const std::array<T,N> &in_v )
    {
        ser.MSerialize( in_v );
    }

    template <typename K, typename V>
    friend void Serialize( DSerialBase &ser, const std::map<K,V> &m )
    {
        ser.MSerialize( m );
    }

    template <typename K, typename V>
    friend void Serialize( DSerialBase &ser, const std::unordered_map<K,V> &m )
    {
        ser.MSerialize( m );
    }
};

//==================================================================
class DDeserialBase
{
    const DSerialType mDSType;

public:
    DDeserialBase( DSerialType type )
        : mDSType(type)
    {}

    DSerialType GetDSType() const { return mDSType; }

    virtual bool MDeserializeIsEOF()        { return false; }

    virtual void MDeserialize(     DStr & ) {}
    virtual void MDeserialize(   size_t & ) {}
    virtual void MDeserialize(     bool & ) {}
    virtual void MDeserialize(      int & ) {}
#if defined(__clang__)
    virtual void MDeserialize( uint64_t & ) {}
#endif
    virtual void MDeserialize( uint32_t & ) {}
    virtual void MDeserialize( uint16_t & ) {}
    virtual void MDeserialize(  int64_t & ) {}
    virtual void MDeserialize(   double & ) {}
    virtual void MDeserialize(    float & ) {}
    virtual void MDeserialize(  Double2 & ) {}

    friend void Deserialize( DDeserialBase &des,     DStr &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,   size_t &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,     bool &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,      int &v ) { des.MDeserialize( v ); }
#if defined(__clang__)
    friend void Deserialize( DDeserialBase &des, uint64_t &v ) { des.MDeserialize( v ); }
#endif
    friend void Deserialize( DDeserialBase &des, uint32_t &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des, uint16_t &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,  int64_t &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,   double &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,    float &v ) { des.MDeserialize( v ); }
    friend void Deserialize( DDeserialBase &des,  Double2 &v ) { des.MDeserialize( v ); }

    template <typename T>
    void MDeserialize( DVec<T> &v )
    {
        size_t n = 0;
        MDeserialize( n );
        v.resize( n );

        for (size_t i=0; i != n; ++i)
            Deserialize( *this, v[i] );
    }

    template <typename T, size_t N>
    void MDeserialize( T (&out_v)[N] )
    {
        for (size_t i=0; i != N; ++i)
            Deserialize( *this, out_v[i] );
    }

    template <typename T, size_t N>
    void MDeserialize( std::array<T,N> &out_v )
    {
        for (size_t i=0; i != N; ++i)
            Deserialize( *this, out_v[i] );
    }

    template <typename K, typename V>
    void MDeserialize( std::unordered_map<K,V> &m )
    {
        size_t n = 0;
        MDeserialize( n );

        m.clear();

        for (size_t i=0; i != n; ++i)
        {
            K k;
            V v;
            Deserialize( *this, k );
            Deserialize( *this, v );
            m[k] = v;
        }
    }

    template <typename K, typename V>
    void MDeserialize( std::map<K,V> &m )
    {
        size_t n = 0;
        MDeserialize( n );

        m.clear();

        for (size_t i=0; i != n; ++i)
        {
            K k;
            V v;
            Deserialize( *this, k );
            Deserialize( *this, v );
            m[k] = v;
        }
    }

    //
    template <typename T>
    friend void Deserialize( DDeserialBase &des, DVec<T> &v ) { des.MDeserialize( v ); }

    template <typename T, size_t N>
    friend void Deserialize( DDeserialBase &des, T (&out_v)[N] ) { des.MDeserialize(out_v); }

    template <typename T, size_t N>
    friend void Deserialize( DDeserialBase &des, std::array<T,N> &out_v )
    {
        des.MDeserialize( out_v );
    }

    template <typename K, typename V>
    friend void Deserialize( DDeserialBase &des, std::unordered_map<K,V> &m )
    {
        des.MDeserialize( m );
    }

    template <typename K, typename V>
    friend void Deserialize( DDeserialBase &des, std::map<K,V> &m )
    {
        des.MDeserialize( m );
    }
};

#endif

