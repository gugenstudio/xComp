//==================================================================
/// DUT_MemFile.h
///
/// Created by Davide Pasca - 2009/8/2
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DUT_MEMFILE_H
#define DUT_MEMFILE_H

#include <memory.h>
#include "DContainers.h"
#include "DExceptions.h"

//==================================================================
namespace DUT
{

//==================================================================
class MemWriterStatic
{
    U8* const       mpDest;
    const size_t    mMaxSize;

    size_t          mIdx = 0;

public:
	MemWriterStatic( void *pDest, size_t maxSize )
		: mpDest((U8 *)pDest)
		, mMaxSize(maxSize)
	{
        (void)mMaxSize;
	}

	template <class T>
	void WriteValue( const T &from )
	{
        DASSERT( mIdx + sizeof(T) <= mMaxSize );
		*((T *)(mpDest + mIdx)) = from;
		mIdx += sizeof(T);
	}

	template <class T>
	void WriteArray( const T *pFrom, size_t cnt )
	{
        DASSERT( mIdx + cnt*sizeof(T) <= mMaxSize );
		for (size_t i=0; i < cnt; ++i)
		{
			*((T *)(mpDest + mIdx)) = pFrom[i];
			mIdx += sizeof(T);
		}
	}

	U8 *Grow( size_t cnt )
	{
        DASSERT( mIdx + cnt <= mMaxSize );

		U8	*ptr = mpDest + mIdx;

		mIdx += cnt;

		return ptr;
	}

	const U8 *GetDataBegin() const { return mpDest; }

	size_t GetCurSize() const { return mIdx; }
};

//==================================================================
class MemWriterDynamic
{
	friend class MemFile;
	friend class MemReader;

    DVec<U8>    mDest;
    U32         mBits = 0;
    int         mBitsCnt = 0;

public:
    MemWriterDynamic() {}

    MemWriterDynamic( const MemWriterDynamic &from )
        : mDest   (from.mDest   )
        , mBits   (from.mBits   )
        , mBitsCnt(from.mBitsCnt)
    {
    }

    MemWriterDynamic( MemWriterDynamic &&from ) noexcept
    {
        *this = std::move( from );
    }

    MemWriterDynamic &operator=( MemWriterDynamic &&from ) noexcept
    {
        mDest    = std::move( from.mDest    );
        mBits    = std::move( from.mBits    );
        mBitsCnt = std::move( from.mBitsCnt );
        return *this;
    }

	void Resize( size_t size )
	{
        if ( size > mDest.size() )
            DEX_OUT_OF_RANGE(
                "MemWriteDynamic::Resize() can only shorten" );

		mBits = 0;
		mBitsCnt = 0;
		mDest.resize( size );
	}

	template <class T>
	void WriteValue( const T &from )
	{
		size_t	idx = mDest.size();
		mDest.resize( idx + sizeof(T) );
		memcpy( &mDest[idx], &from, sizeof(T) );
	}

	template <class T>
	void WriteArray( const T *pFrom, size_t cnt )
	{
        if NOT( cnt )
            return;

		size_t	idx = mDest.size();
		mDest.resize( idx + sizeof(T)*cnt );
		memcpy( &mDest[idx], pFrom, sizeof(T)*cnt );
	}

	void WriteBits( U16 bits, size_t cnt )
	{
		// max 16 bits at once
		DASSERT( cnt <= 16 );

		// push the new bits at the back
		mBits <<= cnt;
		// mask the bits to make sure that
		// the output doesn't go beyond cnt-bits
		mBits |= bits & ((1 << cnt) - 1);
		// increase the count of bits
		mBitsCnt += (int)cnt;

		while ( mBitsCnt >= 8 )
		{
			// write out the topmost 8 bits
			WriteValue( (U8)(mBits >> (mBitsCnt - 8)) );
			// update the number of bits stored
			mBitsCnt -= 8;
		}
	}

	void WriteBitsEnd()
	{
		// WriteBits shouldn't have let this happen
		DASSERT( mBitsCnt < 8 );

		// flush out the remaining bits with zeros
		if ( mBitsCnt )
			WriteBits( 0x00, 8 - mBitsCnt );

		DASSERT( mBitsCnt == 0 );
	}

    void WritePadAlign( size_t alignN )
    {
        while ( GetCurSize() % alignN )
            WriteValue<U8>( 0 );
    }

	void WriteString( const DStr &str );
	void WriteString( const char *pStr );
	void WritePStr32( const DStr &str );

	void PrintF( const char *pFmt, ... );

	U8 *Grow( size_t cnt )
	{
		return Dgrow( mDest, cnt );
	}

	void Reserve( size_t siz ) // actually prealloc
	{
        size_t oldSiz = mDest.size();
		mDest.resize( siz );
		mDest.resize( oldSiz );
	}

	const U8 *GetDataBegin() const { return mDest.size() ? &mDest[0] : NULL; }

	size_t GetCurSize() const { return mDest.size(); }
};

//==================================================================
using MemWriter = MemWriterDynamic;

//==================================================================
class MemReader
{
    DVec<U8>    mOwnedData;
    const U8    *mpSrc   = nullptr;
    size_t      mIdx     = 0;
    size_t      mMaxSize = 0;
    U32         mBits    = 0;
    int         mBitsCnt = 0;

public:
    MemReader()
    {}

    MemReader( const void *pDest, size_t maxSize )
        : mpSrc((const U8 *)pDest)
        , mMaxSize(maxSize)
    {}

    MemReader( const DVec<U8> &vec )
        : mpSrc((const U8 *)&vec[0])
        , mMaxSize(vec.size())
    {}

    MemReader( DVec<U8> &vec )
    {
        InitOwnVec( vec );
    }

    MemReader( MemReader &&from ) noexcept
    {
        *this = std::move( from );
    }

    MemReader &operator=( MemReader &&from ) noexcept
    {
        mOwnedData = std::move( from.mOwnedData );
        mpSrc      = std::move( from.mpSrc      );
        mIdx       = std::move( from.mIdx       );
        mMaxSize   = std::move( from.mMaxSize   );
        mBits      = std::move( from.mBits      );
        mBitsCnt   = std::move( from.mBitsCnt   );
        return *this;
    }

    void InitOwnVec( DVec<U8> &vec )
    {
        mOwnedData = std::move( vec );
        mpSrc = (const U8 *)mOwnedData.data();
        mMaxSize = mOwnedData.size();
    }

    void InitOwnMemWriterDyn( MemWriterDynamic &mw )
    {
        mOwnedData = std::move( mw.mDest );
        mpSrc = (const U8 *)mOwnedData.data();
        mMaxSize = mOwnedData.size();
    }

    template <typename _T>
        using MAKE_NOREF_T = typename std::remove_reference<_T>::type;

    template <typename _T>
    auto PeekValue() const -> MAKE_NOREF_T<_T>
    {
        using _NRT = MAKE_NOREF_T<_T>;

        if ( (mIdx + sizeof(_T)) > mMaxSize )
            DEX_OUT_OF_RANGE(
                    "ReadValue mIdx:%zu + sizeof(_T):%zu > mMaxSize:%zu",
                    mIdx, sizeof(_T), mMaxSize );

    #if defined(D_UNALIGNED_MEM_ACCESS)
        return *((const _NRT *)(mpSrc + mIdx));
    #else
        // make a buffer long enough AND aligned
        U64 tmp[ (sizeof(_T) + sizeof(U64)-1) / sizeof(U64) ];
        // copy into the temporary buffer
        memcpy( tmp, mpSrc + mIdx, sizeof(_T) );

        return *(const _NRT *)(void *)tmp;
    #endif
    }

    template <typename _T>
    auto ReadValue() -> MAKE_NOREF_T<_T>
    {
        auto val = PeekValue<_T>();
        mIdx += sizeof(_T);

        return val;
    }

    template <typename _T>
    void ReadValue( _T &out_val )
    {
        out_val = ReadValue<_T>();
    }

    template <class T>
    void ReadArray( T *pDest, size_t cnt )
    {
        if ( (mIdx+cnt) > mMaxSize )
            DEX_OUT_OF_RANGE(
                    "ReadValue mIdx:%zu + cnt:%zu > mMaxSize:%zu",
                    mIdx, cnt, mMaxSize );

    #if defined(D_UNALIGNED_MEM_ACCESS)
        for (size_t i=0; i < cnt; ++i)
        {
            pDest[i] = *((T *)(mpSrc + mIdx));
            mIdx += sizeof(T);
        }
    #else
        memcpy( pDest, mpSrc + mIdx, cnt * sizeof(T) );
        mIdx += sizeof(T) * cnt;
    #endif
    }

    DStr ReadPStr32()
    {
        size_t len = ReadValue<U32>();

        if NOT( len )
            return DStr();

        const U8 *pData = GetDataPtr( len );

        return DStr( (const char *)pData, len );
    }

    void ReadPStr32( DStr &str ) { str = ReadPStr32(); }

    bool ReadTextLine( char *pDestStr, size_t destStrMaxSize );

    U32 ReadBits( size_t cnt )
    {
        U32 bits = 0;

        // does it have enough bits ?
        while ( mBitsCnt < (int)cnt )
        {
            // ..if not, get a new byte
            U8 val = ReadValue<U8>();

            // add the new byte at the back of the bits stream
            mBits <<= 8;
            mBits |= val;
            mBitsCnt += 8;
        }

        bits = mBits;
        bits >>= mBitsCnt - cnt;
        bits &= (1 << cnt) - 1;

        mBitsCnt -= (int)cnt;

        return bits;
    }

    void ReadBitsEnd()
    {
        mBits       = 0;
        mBitsCnt    = 0;
    }

    void SkipPadAlign( size_t alignN )
    {
        size_t pos = GetCurPos();
        size_t leftN = GetSize() - pos;
        size_t skipN = (((pos + alignN-1) / alignN) * alignN) - pos;
        skipN = (skipN < leftN ? skipN : leftN);
        SkipBytes( skipN );
    }

    const U8 *GetDataPtr( size_t cnt )
    {
        if ( (mIdx+cnt) > mMaxSize )
            DEX_OUT_OF_RANGE( "GetDataPtr(): (%zu+%zu) > %zu", (mIdx+cnt), mMaxSize );

        size_t  idx = mIdx;
        mIdx += cnt;

        return (const U8 *)(mpSrc + idx);
    }

    void SkipBytes( size_t cnt )
    {
        if ( (mIdx+cnt) > mMaxSize )
            DEX_OUT_OF_RANGE( "SkipBytes(): (%zu+%zu) > %zu", (mIdx+cnt), mMaxSize );

        mIdx += cnt;
    }

    void SeekFromStart( size_t offset )
    {
        if ( offset > mMaxSize )
            DEX_OUT_OF_RANGE( "SeekFromStart" );

        mIdx = offset;
    }

    void SeekFromCurPos( ptrdiff_t offset )
    {
        if ( (mIdx+offset) > mMaxSize )
            DEX_OUT_OF_RANGE( "SeekFromCurPos" );

        mIdx += offset;
    }

    size_t GetCurPos() const { return mIdx; }
    size_t GetSize() const { return mMaxSize; }

    bool IsEOF() const
    {
        return mIdx >= mMaxSize;
    }
};

//==================================================================
class MemFile
{
    DVec<U8>    mOwnData;
    const U8    *mpData = 0;
    size_t      mDataSize = 0;
    size_t      mReadPos = 0;

public:
	MemFile() {}
	MemFile( const void *pDataSrc, size_t dataSize );
	MemFile( const char *pFileName, bool prefs = false );
	MemFile( MemWriterDynamic &mw );

	void Init( const void *pDataSrc, size_t dataSize );
	void Init( const char *pFileName, bool prefs = false );
	bool InitNoThrow( const char *pFileName, bool prefs = false );
	void InitExclusiveOwenership( DVec<U8> &fromData );
	void InitExclusiveOwenership( MemWriterDynamic &mw );

	const U8 *GetData() const	{	return mpData;		}
	size_t GetDataSize() const	{	return mDataSize;	}

	bool ReadTextLine( char *pDestStr, size_t destStrMaxSize );
	void ReadData( void *pDest, size_t readSize );
	const void *ReadDataPtr( size_t readSize );

	void SeekSet( size_t pos );
	void SeekFromCur( ptrdiff_t offset );
	void SeekFromEnd( ptrdiff_t offset );

	size_t GetCurPos() const { return mReadPos; }
};

//==================================================================
}

#endif
