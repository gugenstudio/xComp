//==================================================================
/// DMT_Gradient1D.h
///
/// Created by Davide Pasca - 2011/10/27
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMT_GRADIENT1D_H
#define DMT_GRADIENT1D_H

#include "DContainers.h"
#include "DMT_Spline.h"

//==================================================================
template<class _T>
class Gradient1D
{
	DVec<_T>	mVals;
    int         mValsN = 0;

public:
	Gradient1D( const _T *pVals, size_t n, float sca=1.f )
	{
		mVals.reserve( n );
		// fill the array by linearly interpolating batches of equal values
		// ..do this because when inputting values manually, it comes handy
		// to repeat the same value.
		for (size_t i=0; i < n; ++i)
		{
			const _T &a = pVals[i];
			mVals.push_back( a * sca );
			for (size_t j=i+1; j < n; ++j)
			{
				if ( pVals[j] == a )
					continue;

				size_t	fillLen = j - i - 1;
				if ( fillLen )
				{
					_T b = pVals[j];

					float	dt = 1.0f / ((float)fillLen + 1);
					float	t = dt;
					for (size_t k=i+1; k < j; ++k, t += dt)
					{
						mVals.push_back( DLerp( a, b, t ) * sca );
					}

					i += fillLen;
				}
				break;
			}

		}

        // keep this for convenience instead of calling .size() every time
		mValsN = (int)n;
	}

	//==================================================================
	DMT_FINLINE _T SampleBSpline( float u ) const
	{
        // NOTE: assumes that u is >= 0
        auto ui0 = (float)mValsN * u;
        auto ui0_int = std::min( (int)ui0, mValsN-1 );
        auto t_sub = ui0 - (float)ui0_int;

        auto ui1_int = ui0_int+1;
        if ( ui1_int >= mValsN )
            return mVals[ui0_int];

        int uiN1_int = std::max( ui0_int-1, 0 );
        int ui2_int = std::min( ui1_int+1, mValsN-1 );

		return
			DMT::Spline(
				t_sub,
				DMT::BSplineBasis,
				mVals[uiN1_int],
				mVals[ui0_int],
				mVals[ui1_int],
				mVals[ui2_int]
				);
	}

	DMT_FINLINE _T SampleNearest( float u ) const
	{
		int	ui = std::min( (int)((float)mValsN * u), mValsN-1 );

		return mVals[ ui ];
	}

	DMT_FINLINE _T Sample( float u ) const
	{
		// NOTE: assumes that u is >= 0
        auto ui0 = (float)mValsN * u;
        auto ui0_int = std::min( (int)ui0, mValsN-1 );
        auto t_sub = ui0 - (float)ui0_int;

        auto ui1_int = ui0_int+1;
        if ( ui1_int >= mValsN )
            return mVals[ui0_int];

		_T val = DLerp( mVals[ui0_int], mVals[ui1_int], t_sub );
		return val;
	}

	DMT_FINLINE _T SampleClamp( float u ) const
	{
        u = DClamp( u, 0.f, 1.f );

        auto ui0 = (float)mValsN * u;
        auto ui0_int = std::min( (int)ui0, mValsN-1 );
        auto t_sub = ui0 - (float)ui0_int;

        auto ui1_int = ui0_int+1;
        if ( ui1_int >= mValsN )
            return mVals[ui0_int];
		
		_T val = DLerp( mVals[ui0_int], mVals[ui1_int], t_sub );
		return val;
	}
};


#endif
