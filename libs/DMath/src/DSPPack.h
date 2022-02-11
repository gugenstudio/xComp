//==================================================================
/// DSPPack.h
///
/// Created by Davide Pasca - 2014/11/26
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSPPACK_H
#define DSPPACK_H

#include <type_traits>
#include <limits>
#include "DUT_MemFile.h"

//==================================================================
namespace DSPPack
{

template <typename _T> using make_S = typename std::make_signed<_T>::type;
template <typename _T> using make_U = typename std::make_unsigned<_T>::type;
template <bool _COND, typename _T> using en_if = typename std::enable_if<_COND,_T>::type;

template <typename _T> using is_S = std::is_signed<_T>;
template <typename _T> using is_U = std::is_unsigned<_T>;

//=============================================================================
template <typename _T>
DFORCEINLINE make_U<_T> EncodeSign( const _T &val )
{
    static_assert( is_S<_T>::value, "Expecting a signed type" );

    // mask is the sign spread to all bits
    auto signMask = val >> (sizeof(_T)*8-1);

    return ((val ^ signMask) << 1) | (signMask & 1);
}

//=============================================================================
template <typename _T>
DFORCEINLINE make_S<_T> DecodeSign( const _T &val )
{
    static_assert( is_U<_T>::value, "Expecting an unsigned type" );

    // mask is the sign spread to all bits
    auto signMask = (make_S<_T>)((val & 1) ^ 1) - 1;

    return (val >> 1) ^ signMask;
}

// NOTE: "Silent" version only encodes/decodes if necessary
//=============================================================================
template <typename _T> DFORCEINLINE
    en_if< is_S<_T>::value, make_U<_T> > EncodeSignSilent( const _T &val )
{
    return EncodeSign( val );
}
//=============================================================================
template <typename _T> DFORCEINLINE
    en_if< is_U<_T>::value, make_U<_T> > EncodeSignSilent( const _T &val )
{
    return val;
}

//=============================================================================
template <typename _T> DFORCEINLINE
    en_if< is_U<_T>::value, make_S<_T> > DecodeSignSilent( const _T &val )
{
    return DecodeSign( val );
}
//=============================================================================
template <typename _T> DFORCEINLINE
    en_if< is_S<_T>::value, make_S<_T> > DecodeSignSilent( const _T &val )
{
    return val;
}

//==================================================================
template <class _T>
inline void EncodeBlockMinMax(
        DUT::MemWriterDynamic &mwFlags,
        DUT::MemWriterDynamic &mwBase,
        DUT::MemWriterDynamic &mwBlock,
        const _T *pData,
        size_t pitchCol,
        size_t pitchRow,
        size_t wd,
        size_t he )
{
    using _TU = typename std::make_unsigned<_T>::type;

    // calc base (min) value
    _TU   minVal = std::numeric_limits<_TU>::max();
    _TU   maxVal = 0;
    const _T *pDataRow = pData;
    for (size_t y=0; y != he; ++y)
    {
        const _T *pDataCol = pDataRow;

        for (size_t x = 0; x != wd; ++x, pDataCol += pitchCol)
        {
            _TU uval = EncodeSignSilent( pDataCol[0] );
            minVal = DMin( minVal, uval );
            maxVal = DMax( maxVal, uval );
        }

        pDataRow += pitchRow;
    }

    // write the base value
    mwBase.WriteValue( minVal );
    if ( minVal == maxVal )
    {
        //mwFlags.WriteBits( 1, 1 );
        mwFlags.WriteValue( (U8)1 );
        return;
    }

    pDataRow = pData;

    U8 flags = 0;
    if ( (maxVal - minVal) <= 255 )
    {
        for (size_t y=0; y != he; ++y)
        {
            const _T *pDataCol = pDataRow;

            for (size_t x=0; x != wd; ++x, pDataCol += pitchCol)
            {
                _TU uval = EncodeSignSilent( pDataCol[0] );
                mwBlock.WriteValue<U8>( (U8)(uval - minVal) );
            }

            pDataRow += pitchRow;
        }
    }
    else
    {
        flags |= 2;
        for (size_t y=0; y != he; ++y)
        {
            const _T *pDataCol = pDataRow;

            for (size_t x=0; x != wd; ++x, pDataCol += pitchCol)
            {
                _TU uval = EncodeSignSilent( pDataCol[0] );
                mwBlock.WriteValue<_T>( uval - minVal );
            }

            pDataRow += pitchRow;
        }
    }

    //mwFlags.WriteBits( 0, 1 );
    mwFlags.WriteValue( flags );
}

//==================================================================
template <class _T>
inline void DecodeBlockMinMax(
        DUT::MemReader &mrFlags,
        DUT::MemReader &mrBase,
        DUT::MemReader &mrBlock,
        _T *pData,
        size_t pitchCol,
        size_t pitchRow,
        size_t wd,
        size_t he )
{
    using _TU = typename std::make_unsigned<_T>::type;

    U8 flags = mrFlags.ReadValue<U8>();

    _TU minVal = mrBase.ReadValue<_TU>();

    _T *pDataRow = pData;
    if ( flags & 1 )
    {
        for (size_t y=0; y != he; ++y)
        {
            _TU *pDataCol = (_TU *)pDataRow;
            for (size_t x=0; x != wd; ++x, pDataCol += pitchCol)
                pDataCol[0] = minVal;
            pDataRow += pitchRow;
        }
    }
    else
    if ( flags & 2 )
    {
        for (size_t y=0; y != he; ++y)
        {
            _T *pDataCol = pDataRow;
            for (size_t x=0; x != wd; ++x, pDataCol += pitchCol)
                pDataCol[0] = DecodeSignSilent( minVal + mrBlock.ReadValue<_TU>() );
            pDataRow += pitchRow;
        }
    }
    else
    {
        for (size_t y=0; y != he; ++y)
        {
            _T *pDataCol = pDataRow;
            for (size_t x=0; x != wd; ++x, pDataCol += pitchCol)
                pDataCol[0] = DecodeSignSilent( minVal + mrBlock.ReadValue<U8>() );
            pDataRow += pitchRow;
        }
    }
}

//==================================================================
}

#endif

