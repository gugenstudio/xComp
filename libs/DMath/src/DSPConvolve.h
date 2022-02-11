//==================================================================
/// DSPConvolve.h
///
/// Created by Davide Pasca - 2013/10/10
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DSPCONVOLVE_H
#define DSPCONVOLVE_H

//==================================================================
template <int _FSIZ>
inline void DSPMakeCosFilter( float (&out_weights)[_FSIZ] )
{
    static const int  FSIZH = _FSIZ/2;

    float wsum = 0;
    for (int i=0; i != _FSIZ; ++i)
    {
        // t : -1..1
        float t = (i - FSIZH) * (float)(1.f / FSIZH);
        float ang = FM_PI_2 * t;
        float c = cos( ang );
        out_weights[i] = c;
        wsum += c;
    }
    // normalize wieghts
    for (int i=0; i != _FSIZ; ++i)
        out_weights[i] /= wsum;
}

//==================================================================
// Sample filters
/*
static float DSPSobelFilter3H[3] =
{
    -1, 0, 1
};
static float DSPSobelFilter3V[3] =
{
    1,
    2,
    1
};
*/

//==================================================================
template <int _FSIZ>
class DSPConvolve
{
    friend class const_iterator;

    static const int  FSIZH = _FSIZ/2;

    DVec<float> mData;
    size_t      mSiz;
    size_t      mPitch;

public:
	class const_iterator
    {
        friend class DSPConvolve;

        const DSPConvolve *pParent;
        size_t          x;
        const float     *pRow;

    public:
        const_iterator() {}
        const_iterator( const DSPConvolve &c )
            : pParent(&c)
            , x(0)
            , pRow(c.getData00())
        {}

        float operator*() const {
            return pRow[x];
        }
        const_iterator &operator++()
        {
            DASSERT( *this != pParent->cend() );
            if ( (x+1) >= pParent->mSiz ) {
                x = 0;
                pRow += pParent->mPitch;
            }
            else
                ++x;

            return *this;
        }
        const_iterator operator++(int) {
            const_iterator saved( *this );
            ++(*this);
            return saved;
        }
        bool operator==(const const_iterator &other) const {
            DASSERT( pParent == other.pParent );
            return x == other.x && pRow == other.pRow;
        }
        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }
    };
private:
    const_iterator mBaseConstIterEnd;

public:
    const_iterator cbegin() const {
        return const_iterator( *this );
    }

    const const_iterator &cend() const {
        return mBaseConstIterEnd;
    }

private:
    void convolveSetup(
        std::function<float (int,int)> getValFn,
        std::function<float (int,int)> getValClampFn,
        int siz )
    {
        size_t desPitch = siz + FSIZH*2;

        // used to iterate later on !
        mSiz   = (size_t)siz;
        mPitch = (size_t)desPitch;

        // alloc the data
        mData.resize( desPitch * desPitch );
        for (auto &val : mData)
            val = 0;

        // add border
        float *pDes = getData00();

        // fill the center
        float *pDes2 = pDes;
        for (int y=0; y != siz; ++y)
        {
            for (int x=0; x != siz; ++x)
                pDes2[x] = getValFn( x, y );

            pDes2 += desPitch;
        }

        // get the edges
        // left and right
        pDes2 = pDes;
        for (int y=0; y != siz; ++y)
        {
            for (int i=-FSIZH; i < 0; ++i)
                pDes2[i] = getValClampFn( i, y );

            for (int i=siz; i < (siz+FSIZH); ++i)
                pDes2[i] = getValClampFn( i, y );

            pDes2 += desPitch;
        }

        // top
        pDes2 = pDes + (-FSIZH) * desPitch;
        for (int y=-FSIZH; y < 0; ++y)
        {
            for (int i=0; i != siz; ++i)
                pDes2[i] = getValClampFn( i, y );
            pDes2 += desPitch;
        }
        // bottom
        pDes2 = pDes + siz * desPitch;
        for (int y=siz; y < (siz+FSIZH); ++y)
        {
            for (int i=0; i != siz; ++i)
                pDes2[i] = getValClampFn( i, y );
            pDes2 += desPitch;
        }

        mBaseConstIterEnd.pParent = this;
        mBaseConstIterEnd.x = mSiz;
        mBaseConstIterEnd.pRow = pDes + (siz-1) * desPitch;
    }

public:
    // 1D filter
    DSPConvolve(
        std::function<float (int,int)> getValFn,
        std::function<float (int,int)> getValClampFn,
        int siz,
        const float filterWeightsH[_FSIZ],
        const float filterWeightsV[_FSIZ] )
    {
        static_assert(
                (_FSIZ & 1) == 1 && _FSIZ >= 3,
                "Filter size must be an odd number >= 3");

        convolveSetup(
                getValFn,
                getValClampFn,
                siz );

        DVec<float> scratch( mSiz + FSIZH*2 );
        applyFilter1DH( scratch, filterWeightsH );
        applyFilter1DV( scratch, filterWeightsV );
    }

private:
    //
    void applyFilter1DH( DVec<float> &scratch, const float filterWeights[_FSIZ] )
    {
        float *pScratch00 = &scratch[0];

        size_t fullLineLen = mSiz+FSIZH*2;

        // horizontal filter
        float *pData = getData00();
        for (size_t i=0; i != mSiz; ++i)
        {
            memcpy( pScratch00, pData-FSIZH, fullLineLen*sizeof(*pData) );

            for (size_t x=0; x != mSiz; ++x)
            {
                float val = 0;
                for (int j=0; j != _FSIZ; ++j)
                {
                    val += filterWeights[j] * pScratch00[x + j];
                }
                pData[x] = val;
            }

            pData += mPitch;
        }
    }

    void applyFilter1DV( DVec<float> &scratch, const float filterWeights[_FSIZ] )
    {
        float *pScratch00 = &scratch[0];

        size_t fullLineLen = mSiz+FSIZH*2;

        // vertical filter
        for (size_t i=0; i != mSiz; ++i)
        {
            float *pData = getData00() + i + (-FSIZH * mPitch);

            float *pData2 = pData;
            for (size_t j=0; j != fullLineLen; ++j)
            {
                pScratch00[j] = pData2[0];
                pData2 += mPitch;
            }

            pData2 = pData;
            for (size_t x=0; x != mSiz; ++x)
            {
                float val = 0;
                for (int j=0; j != _FSIZ; ++j)
                {
                    val += filterWeights[j] * pScratch00[x + j];
                }
                pData2[0] = val;
                pData2 += mPitch;
            }
        }
    }

public:
    float GetSample( u_int ix, u_int iy ) const
    {
        DASSERT( ix < (u_int)mSiz && iy < (u_int)mSiz );

        return mData[ ix + FSIZH + (iy + FSIZH) * mPitch ];
    }

private:
    float *getData00() { return &mData[ FSIZH + FSIZH * mPitch ]; }
    const float *getData00() const { return &mData[ FSIZH + FSIZH * mPitch ]; }
};

#endif

