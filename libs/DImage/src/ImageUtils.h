//==================================================================
/// ImageConv.h
///
/// Created by Davide Pasca - 2014/9/4
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

//==================================================================
namespace ImageUtils
{

//==================================================================
template <class _T, size_t CHANS_N>
void WrapMap( _T *pMap, u_int dimL2, u_int wrapHDim )
{
    DASSERT( wrapHDim >= 1 && wrapHDim <= ((1U << dimL2)/2) );

    auto cosLerpCoe = []( float a )
    {
        return (1.0f - cosf(a * FM_PI)) * 0.5f;
    };

	int	dim = 1 << dimL2;

	for (int i=0; i < (int)wrapHDim; ++i)
	{
		float t1 = cosLerpCoe( 0.5f + 0.5f * (float)i / wrapHDim );
		float t2 = cosLerpCoe( 0.5f + 0.5f * (float)(i+1) / (wrapHDim + 1) );

		int	i1 = i;
		int	i2 = dim-1 - i;

		// rows
		int	row1 = i1 << dimL2;
		int	row2 = i2 << dimL2;
		for (int j=0; j < dim; ++j)
		{
			int	j1 = j + row1;
			int	j2 = j + row2;
			for (int k=0; k < (int)CHANS_N; ++k)
			{
				int	jj1 = j1 * CHANS_N + k;
				int	jj2 = j2 * CHANS_N + k;
				_T val1 = pMap[ jj1 ];
				_T val2 = pMap[ jj2 ];
				pMap[ jj1 ] = (_T)DLerp( val2, val1, t1 );
				pMap[ jj2 ] = (_T)DLerp( val1, val2, t2 );
			}
		}
	}

	for (int i=0; i < (int)wrapHDim; ++i)
	{
		float t1 = cosLerpCoe( 0.5f + 0.5f * (float)i / wrapHDim );
		float t2 = cosLerpCoe( 0.5f + 0.5f * (float)(i+1) / (wrapHDim + 1) );

		int	i1 = i;
		int	i2 = dim-1 - i;

		// cols
		for (int j=0; j < dim; ++j)
		{
			int	j1 = i1 + (j << dimL2);
			int	j2 = i2 + (j << dimL2);
			for (int k=0; k < (int)CHANS_N; ++k)
			{
				int	jj1 = j1 * CHANS_N + k;
				int	jj2 = j2 * CHANS_N + k;
				_T val1 = pMap[ jj1 ];
				_T val2 = pMap[ jj2 ];
				pMap[ jj1 ] = (_T)DLerp( val2, val1, t1 );
				pMap[ jj2 ] = (_T)DLerp( val1, val2, t2 );
			}
		}
	}
}

//==================================================================
}

#endif

