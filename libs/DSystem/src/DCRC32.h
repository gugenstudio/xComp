//==================================================================
/// DCRC32.h
///
/// Created by Davide Pasca - 2011/2/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
///
/// Originally from http://www.cplusplus.com/forum/lounge/27570/
//==================================================================

#ifndef DCRC32_H
#define DCRC32_H

#include "DBase.h"

//==================================================================
class DCRC32
{
public:
    //=========================================
    DCRC32()								{ Reset();                  }
    DCRC32(const U8 *buf, size_t siz)		{ Reset(); Hash(buf,siz);   }

    //=========================================
    // implicit cast, so that you can do something like foo = CRC(dat,siz);
    operator U32 () const                    { return Get();             }

    //=========================================
    // getting the crc
    U32          Get() const                 { return ~mCrc;             }

    //=========================================
    // HashBase stuff
    void        Reset()
	{
	    if NOT( mTableBuilt )
	        BuildTable();

		mCrc = (U32)~0;
	}

    DFORCEINLINE DCRC32 &Hash( const U8 *buf, size_t siz )
    {
        for(size_t i=0; i < siz; ++i)
            mCrc = (mCrc >> 8) ^ mTable[ (mCrc & 0xFF) ^ buf[i] ];
        return *this;
    }

    DCRC32 &HashCStr( const char *pStr );
    DCRC32 &HashStr( const DStr &str );

	template <class _T>
    DCRC32 &HashVal( const _T &val )   { return Hash( (const U8 *)&val, sizeof(val) ); }

    DCRC32 &HashVal( const DStr &str ) { return HashStr( str ); }

private:
    U32         mCrc;
    static bool mTableBuilt;
    static U32  mTable[0x100];

    static const U32        POLYNOMIAL = 0x04C11DB7;

private:
    //=========================================
    // internal support
    static void         BuildTable();
    static U32          Reflect(U32 v,int bits);
};

#endif
