//==================================================================
/// DSPUtils.h
///
/// Created by Davide Pasca - 2014/11/26
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSPUTILS_H
#define DSPUTILS_H

//==================================================================
namespace DSPUtils
{

//==================================================================
// see: http://mathworks.com/help/vision/ref/psnr.html
template <typename _T>
inline double CalcPSNR(
        const _T *pSrc1,
        const _T *pSrc2,
        size_t len,
        double signalRange )
{
    if ( len == 0 )
        return 0;

    double mse = 0.0;
    for (size_t i=0; i != len; ++i)
    {
        auto diff = (double)(pSrc1[i] - pSrc2[i]);
        mse += diff * diff;
    }
    mse /= (double)len;

    auto psnr =
        10.0 * log10( signalRange * signalRange / mse );

    return psnr;
}

//==================================================================
template <typename _T>
inline double CalcPSNRIntegral(
        const void *pSrc1,
        const void *pSrc2,
        size_t len )
{
    const auto typeBits = sizeof(_T) * 8;
    const double signalRange = (double)((1 << typeBits) - 1);

    return
        CalcPSNR(
            (const _T *)pSrc1,
            (const _T *)pSrc2,
            len,
            signalRange );
}

//==================================================================
}

#endif

