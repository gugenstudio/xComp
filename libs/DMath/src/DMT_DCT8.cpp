//==================================================================
/// DMT_DCT8.cpp
///
/// Created by Davide Pasca - 2012/5/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DMathBase.h"
#include "DMT_DCT8.h"

//==================================================================
static const int	DCTSIZE = 8;
//static const int    DCTSIZE2 = DCTSIZE * DCTSIZE;

//==================================================================
/// DDCT8Fwd
//==================================================================
// Warning ! Output is scaled by 8 !
void DDCT8Fwd( float *pOutBlock, const float *pInBlock )
{
	float z1, z2, z3, z4, z5, z11, z13;

	// Pass 1: process rows
	for (int ctr = 0; ctr < DCTSIZE; ++ctr)
	{
		const	float *pSrcRow = &pInBlock[ ctr * DCTSIZE ];
				float *pDesRow = &pOutBlock[ ctr * DCTSIZE ];

		float tmp0 = pSrcRow[0] + pSrcRow[7];
		float tmp7 = pSrcRow[0] - pSrcRow[7];
		float tmp1 = pSrcRow[1] + pSrcRow[6];
		float tmp6 = pSrcRow[1] - pSrcRow[6];
		float tmp2 = pSrcRow[2] + pSrcRow[5];
		float tmp5 = pSrcRow[2] - pSrcRow[5];
		float tmp3 = pSrcRow[3] + pSrcRow[4];
		float tmp4 = pSrcRow[3] - pSrcRow[4];

		// Even part

		float tmp10 = tmp0 + tmp3;	// phase 2
		float tmp13 = tmp0 - tmp3;
		float tmp11 = tmp1 + tmp2;
		float tmp12 = tmp1 - tmp2;

		pDesRow[0] = tmp10 + tmp11; // phase 3
		pDesRow[4] = tmp10 - tmp11;

		z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
		pDesRow[2] = tmp13 + z1;	// phase 5
		pDesRow[6] = tmp13 - z1;

		// Odd part

		tmp10 = tmp4 + tmp5;	// phase 2
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		// The rotator is modified from fig 4-8 to avoid extra negations.
		z5 = (tmp10 - tmp12) * ((float) 0.382683433);	// c6
		z2 = ((float) 0.541196100) * tmp10 + z5;		// c2-c6
		z4 = ((float) 1.306562965) * tmp12 + z5;		// c2+c6
		z3 = tmp11 * ((float) 0.707106781);				// c4

		z11 = tmp7 + z3;		// phase 5
		z13 = tmp7 - z3;

		pDesRow[5] = z13 + z2;	// phase 6
		pDesRow[3] = z13 - z2;
		pDesRow[1] = z11 + z4;
		pDesRow[7] = z11 - z4;
	}

	// Pass 2: process columns
	for (int ctr = 0; ctr < DCTSIZE; ++ctr)
	{
		const	float *pSrc = &pOutBlock[ ctr ];
				float *pDes = &pOutBlock[ ctr ];

		float tmp0 = pSrc[DCTSIZE*0] + pSrc[DCTSIZE*7];
		float tmp7 = pSrc[DCTSIZE*0] - pSrc[DCTSIZE*7];
		float tmp1 = pSrc[DCTSIZE*1] + pSrc[DCTSIZE*6];
		float tmp6 = pSrc[DCTSIZE*1] - pSrc[DCTSIZE*6];
		float tmp2 = pSrc[DCTSIZE*2] + pSrc[DCTSIZE*5];
		float tmp5 = pSrc[DCTSIZE*2] - pSrc[DCTSIZE*5];
		float tmp3 = pSrc[DCTSIZE*3] + pSrc[DCTSIZE*4];
		float tmp4 = pSrc[DCTSIZE*3] - pSrc[DCTSIZE*4];

		// Even part

		float tmp10 = tmp0 + tmp3;	// phase 2
		float tmp13 = tmp0 - tmp3;
		float tmp11 = tmp1 + tmp2;
		float tmp12 = tmp1 - tmp2;

		pDes[DCTSIZE*0] = tmp10 + tmp11; // phase 3
		pDes[DCTSIZE*4] = tmp10 - tmp11;

		z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
		pDes[DCTSIZE*2] = tmp13 + z1; // phase 5
		pDes[DCTSIZE*6] = tmp13 - z1;

		// Odd part

		tmp10 = tmp4 + tmp5;	// phase 2
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;

		// The rotator is modified from fig 4-8 to avoid extra negations.
		z5 = (tmp10 - tmp12) * ((float) 0.382683433); // c6
		z2 = ((float) 0.541196100) * tmp10 + z5; // c2-c6
		z4 = ((float) 1.306562965) * tmp12 + z5; // c2+c6
		z3 = tmp11 * ((float) 0.707106781); // c4

		z11 = tmp7 + z3;		// phase 5
		z13 = tmp7 - z3;

		pDes[DCTSIZE*5] = z13 + z2; // phase 6
		pDes[DCTSIZE*3] = z13 - z2;
		pDes[DCTSIZE*1] = z11 + z4;
		pDes[DCTSIZE*7] = z11 - z4;
	}
}

//==================================================================
/// DDCT8Inv
//==================================================================
// Warning ! Output is scaled by 8 !
void DDCT8Inv( float *pInOutBlock )
{
	float z5, z10, z11, z12, z13;

	const float	*pInBlock	= pInOutBlock;
	float		*pOutBlock	= pInOutBlock;

	// Pass 1: process columns from input, store into work array.
	for (int ctr = 0; ctr < DCTSIZE; ++ctr)
	{
		float tmp00 = pInBlock[DCTSIZE*0];
		float tmp01 = pInBlock[DCTSIZE*1];
		float tmp02 = pInBlock[DCTSIZE*2];
		float tmp03 = pInBlock[DCTSIZE*3];
		float tmp04 = pInBlock[DCTSIZE*4];
		float tmp05 = pInBlock[DCTSIZE*5];
		float tmp06 = pInBlock[DCTSIZE*6];
		float tmp07 = pInBlock[DCTSIZE*7];

/*
		if (tmp01 == 0 &&
			tmp02 == 0 && tmp03 == 0 && tmp04 == 0 &&
			tmp05 == 0 && tmp06 == 0 && tmp07 == 0 )
		{
			// AC terms all zero
			float dcval = tmp00;

			pOutBlock[DCTSIZE*0] = dcval;
			pOutBlock[DCTSIZE*1] = dcval;
			pOutBlock[DCTSIZE*2] = dcval;
			pOutBlock[DCTSIZE*3] = dcval;
			pOutBlock[DCTSIZE*4] = dcval;
			pOutBlock[DCTSIZE*5] = dcval;
			pOutBlock[DCTSIZE*6] = dcval;
			pOutBlock[DCTSIZE*7] = dcval;

			pInBlock++;			// advance pointers to next column
			pOutBlock++;		// advance pointers to next column
			continue;
		}
*/

		// Even part
		float tmp10 = tmp00 + tmp04;	// phase 3
		float tmp11 = tmp00 - tmp04;

		float tmp13 = tmp02 + tmp06;	// phases 5-3
		float tmp12 = (tmp02 - tmp06) * ((float) 1.414213562) - tmp13; // 2*c4

		tmp00 = tmp10 + tmp13;	// phase 2
		tmp06 = tmp10 - tmp13;
		tmp02 = tmp11 + tmp12;
		tmp04 = tmp11 - tmp12;

		// Odd part

		z13 = tmp05 + tmp03;		// phase 6
		z10 = tmp05 - tmp03;
		z11 = tmp01 + tmp07;
		z12 = tmp01 - tmp07;

		tmp07 = z11 + z13;		// phase 5
		tmp11 = (z11 - z13) * ((float) 1.414213562); // 2*c4

		z5 = (z10 + z12) * ((float) 1.847759065); // 2*c2
		tmp10 = ((float) 1.082392200) * z12 - z5; // 2*(c2-c6)
		tmp12 = ((float) -2.613125930) * z10 + z5; // -2*(c2+c6)

		tmp05 = tmp12 - tmp07;	// phase 2
		tmp03 = tmp11 - tmp05;
		tmp01 = tmp10 + tmp03;

		pOutBlock[DCTSIZE*0] = tmp00 + tmp07;
		pOutBlock[DCTSIZE*7] = tmp00 - tmp07;
		pOutBlock[DCTSIZE*1] = tmp02 + tmp05;
		pOutBlock[DCTSIZE*6] = tmp02 - tmp05;
		pOutBlock[DCTSIZE*2] = tmp04 + tmp03;
		pOutBlock[DCTSIZE*5] = tmp04 - tmp03;
		pOutBlock[DCTSIZE*4] = tmp06 + tmp01;
		pOutBlock[DCTSIZE*3] = tmp06 - tmp01;

		pInBlock++;			// advance pointers to next column
		pOutBlock++;		// advance pointers to next column
	}

	// Pass 2: process rows from work array, store into output array.
	// Note that we must descale the results by a factor of 8 == 2**3.
	pInBlock	= pInOutBlock;
	pOutBlock	= pInOutBlock;

	for (int ctr = 0; ctr < DCTSIZE; ++ctr)
	{
		// Even part

		float tmp10 = pInBlock[0] + pInBlock[4];
		float tmp11 = pInBlock[0] - pInBlock[4];

		float tmp13 = pInBlock[2] + pInBlock[6];
		float tmp12 = (pInBlock[2] - pInBlock[6]) * ((float) 1.414213562) - tmp13;

		float tmp0 = tmp10 + tmp13;
		float tmp3 = tmp10 - tmp13;
		float tmp1 = tmp11 + tmp12;
		float tmp2 = tmp11 - tmp12;

		// Odd part

		z13 = pInBlock[5] + pInBlock[3];
		z10 = pInBlock[5] - pInBlock[3];
		z11 = pInBlock[1] + pInBlock[7];
		z12 = pInBlock[1] - pInBlock[7];

		float tmp7 = z11 + z13;
		tmp11 = (z11 - z13) * ((float) 1.414213562);

		z5 = (z10 + z12) * ((float) 1.847759065);	// 2*c2
		tmp10 = ((float) 1.082392200) * z12 - z5;	// 2*(c2-c6)
		tmp12 = ((float) -2.613125930) * z10 + z5;	// -2*(c2+c6)

		float tmp6 = tmp12 - tmp7;
		float tmp5 = tmp11 - tmp6;
		float tmp4 = tmp10 + tmp5;

		// Final output stage: scale down by a factor of 8
		pOutBlock[0] = tmp0 + tmp7;
		pOutBlock[7] = tmp0 - tmp7;
		pOutBlock[1] = tmp1 + tmp6;
		pOutBlock[6] = tmp1 - tmp6;
		pOutBlock[2] = tmp2 + tmp5;
		pOutBlock[5] = tmp2 - tmp5;
		pOutBlock[4] = tmp3 + tmp4;
		pOutBlock[3] = tmp3 - tmp4;

		pInBlock	+= DCTSIZE;		// advance pointer to next row
		pOutBlock	+= DCTSIZE;		// advance pointer to next row
	}
}
