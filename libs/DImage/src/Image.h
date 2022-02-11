//==================================================================
/// image.h
///
/// Created by Davide Pasca - 2009/10/5
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_H
#define IMAGE_H

#include "DBase.h"
#include "DContainers.h"
#include "DUT_Files.h"
#include "DVector.h"

// forward declaration
namespace ImgDLS
{
    class ImgDLS_Head1;
};

//==================================================================
//==================================================================
class image
{
    friend class ImageUploader;
    friend class VolatileTextureCache;
    friend class Graphics;

    static DFun<void (image *)> msOnImageDestructFn;
    static DFun<u_int (const image *)> msOnImageGetTempTexIDFn;

private:
    // OpenGL texture-related stuff
    mutable u_int mImageTexID0 = (u_int)-1;
public:
    u_int       mTexWd = 0;
    u_int       mTexHe = 0;

public:
    float       mTexS2 = 0;
    float       mTexT2 = 0;
    float       mXtoS = 0;
    float       mYtoT = 0;

public:
    static const u_int  MAX_CHANS = 4;

	static const u_int	FLG_USE_BILINEAR = 1 << 0;
	static const u_int	FLG_USE_MIPMAPS	 = 1 << 1;

	static const u_int	FLG_REPEAT_X	 = 1 << 3;
	static const u_int	FLG_REPEAT_Y	 = 1 << 4;

	static const u_int	FLG_FAVORCOLOR	 = 1 << 6;
	static const u_int	FLG_HARD_TRANSP	 = 1 << 7;
	static const u_int	FLG_IS_LUMINANCE = 1 << 8;
    static const u_int  FLG_IS_CUBEMAP   = 1 << 9;
    static const u_int  FLG_HAS_GAMMA22  = 1 << 10;
    static const u_int  FLG_IS_FLOAT16   = 1 << 11;
    static const u_int  FLG_IS_FLOAT32   = 1 << 12;
	static const u_int	FLG_IS_ALPHA     = 1 << 13;
	static const u_int	FLG_IS_NORMAL    = 1 << 14;
	static const u_int	FLG_IS_YUV411    = 1 << 15;
	static const u_int	FLG_IS_YUV444    = 1 << 16;

public:
    u_int       mW = 0;
    u_int       mH = 0;

//private:
    u_int       mDepth = 0;
    u_int       mChans = 0;

    u_int       mBytesPerPixel = 0;
    u_int       mBytesPerRow = 0;

    u_int       mFlags = 0;

private:
    DVec<U8>    mPixels;
    U8          *mpData = 0;

public:
    uptr<image> moSubImage;

    image *GetOrMakeSubImage()
    {
        if NOT( moSubImage )
            moSubImage = std::make_unique<image>();

        return moSubImage.get();
    }

public:
	//==================================================================
	class Params
	{
	public:
		const U8	*pSrcData   = nullptr;
		U8			*pUseData   = nullptr;
		const image	*pSrcImg    = nullptr;
        bool        doCopySrcImgPixels = false;
		u_int		width       = 0;
		u_int		height      = 0;
		u_int		depth       = 0;
		u_int		chans       = 0;
		u_int		rowPitch    = 0;
		u_int		flags       = 0;
        u_int       autoRowPitchAlign = 4; // must be pow of 2
        u_int       subImgWidth = 0;
        u_int       subImgHeight = 0;

        void SetFormatRGB16F()
        {
            chans = 3;
            depth = chans * sizeof(U16) * 8;
            flags |= FLG_IS_FLOAT16;
        }
        void SetFormatRGB32F()
        {
            chans = 3;
            depth = chans * sizeof(float) * 8;
            flags |= FLG_IS_FLOAT32;
        }

        void FormatFromImage( const image &img )
        {
            depth = img.mDepth;
            chans = img.mChans;
            flags = img.mFlags;
        }
	};

	//==================================================================
	class LoadParams
	{
	public:
        DStr        mLP_FName;
        DVec<U8>    mLP_DataSrc;
        u_int       mLP_FlagsAdd = 0;
        u_int       mLP_FlagsRem = 0;
        bool        mLP_QuickLoad = false;
        //u_int       mLP_AutoRowPitchAlign = 4; // must be pow of 2 (unused ?)
        u_int       mLP_ScalePow2Div = 0;

        LoadParams() {}
        LoadParams( const DStr &fname ) : mLP_FName(fname) {}
	};
private:
    LoadParams    mLoadParams;

public:
	//==================================================================
	image();
	image( const image &from );
	image( const Params &par );
	image( const LoadParams &par, ImgDLS::ImgDLS_Head1 *pOut_Head=nullptr );
	image( const char *pFName, u_int flags );

    ~image();

	image &operator=( const image &from );
    image &operator=( image &&from ) noexcept;

    const DStr &GetFileName() const { return mLoadParams.mLP_FName; }

private:
	void borrowData( const image &from );
    void moveData( image &from );

public:
	void Setup( const Params &par );
	void Setup( const LoadParams &par, ImgDLS::ImgDLS_Head1 *pOut_Head );
	void Clear();
    void FreeImageData();

	void CopyImageDefinition( const image &from );
	void CopyImageData( const image &from );

    void CopyDefTranserData( image &from );

    bool IsFloat16() const { return !!(mFlags & FLG_IS_FLOAT16); }
    bool IsFloat32() const { return !!(mFlags & FLG_IS_FLOAT32); }
    bool IsYUV411() const  { return !!(mFlags & FLG_IS_YUV411); }
    bool IsYUV444() const  { return !!(mFlags & FLG_IS_YUV444); }

    DFORCEINLINE U8 *GetCubePixelPtr( u_int face, u_int x, u_int y ) {
		DASSERT( x < mH && y < mH );
		return &mpData[ y * mBytesPerRow + (face*mH + x) * mBytesPerPixel ];
    }

    DFORCEINLINE const U8 *GetCubePixelPtr( u_int face, u_int x, u_int y ) const {
		DASSERT( x < mH && y < mH );
		return &mpData[ y * mBytesPerRow + (face*mH + x) * mBytesPerPixel ];
    }

	DFORCEINLINE U8  *GetPixelPtr( u_int x, u_int y ) {
		DASSERT( x < mW && y < mH );
		return &mpData[ y * mBytesPerRow + x * mBytesPerPixel ];
	}

	DFORCEINLINE const U8  *GetPixelPtr( u_int x, u_int y ) const {
		DASSERT( x < mW && y < mH );
		return &mpData[ y * mBytesPerRow + x * mBytesPerPixel ];
	}

	DFORCEINLINE const U8  *GetPixelPtrCheck( int x, int y ) const {
		if ( (u_int)x < mW && (u_int)y < mH )
			return &mpData[ y * mBytesPerRow + x * mBytesPerPixel ];
		else
			return NULL;
	}

	DFORCEINLINE void SetPixelPtr( u_int x, u_int y, const U8 *pData ) {
		DASSERT( x < mW && y < mH );

		U8 *pDes = &mpData[ y * mBytesPerRow + x * mBytesPerPixel ];

		for (u_int i=0; i < mBytesPerPixel; ++i)
			pDes[i] = pData[i];
	}

    DFORCEINLINE static void GetPixelUnpacked_s(
            const U8 *pPix,
            u_int depth,
            u_int chans,
            size_t bppix,
            u_int imgFlags,
            u_int dest[MAX_CHANS] );

    DFORCEINLINE static void SetPixelUnpacked_s(
            U8 *pPix,
            u_int depth,
            u_int chans,
            size_t bppix,
            u_int imgFlags,
            const u_int src[MAX_CHANS] );

    void GetPixelUnpacked( const U8 *pPix, u_int dest[MAX_CHANS] ) const
    {
        GetPixelUnpacked_s(
                pPix,
                mDepth,
                mChans,
                mBytesPerPixel,
                mFlags,
                dest );
    }
    void SetPixelUnpacked( U8 *pPix, const u_int src[MAX_CHANS] )
    {
        SetPixelUnpacked_s(
                pPix,
                mDepth,
                mChans,
                mBytesPerPixel,
                mFlags,
                src );
    }

	u_int GetTextureID() const;
	bool IsTextureIDValid() const;

	u_int GetSubTextureID() const;
	bool IsSubTextureIDValid() const;

    //
	DFORCEINLINE void GetTextureStripCoords(
					float x0,
					float y0,
					Float2 out_coords[4] ) const;

	DFORCEINLINE void GetTextureStripCoords(
					float x0,
					float y0,
					float x1,
					float y1,
					Float2 out_coords[4] ) const;

	void GetTextureStripCoords( Float2 out_coords[4] ) const {
		GetTextureStripCoords( 0, 0, out_coords );
	}

    //
	DFORCEINLINE void GetTextureTrigCoords(
					float x0,
					float y0,
					Float2 out_coords[6] ) const;

	DFORCEINLINE void GetTextureTrigCoords(
					float x0,
					float y0,
					float x1,
					float y1,
					Float2 out_coords[6] ) const;

	void GetTextureTrigCoords( Float2 out_coords[6] ) const {
		GetTextureTrigCoords( 0, 0, out_coords );
	}

    static DFORCEINLINE u_int GetNextPow2( u_int val );

	void MakeTexVars(
        u_int useW,
        u_int useH,
        u_int &out_texW,
        u_int &out_texH,
	    float &out_XtoS,
	    float &out_YtoT,
	    float &out_texS2,
	    float &out_texT2 ) const;

    void ScaleQuarter();

private:
	bool isInitialized() const;
    void reloadImage( ImgDLS::ImgDLS_Head1 *pOut_Head );
    void reload_loadFileData(
        int         fxSize,
        const U8*   &out_pData,
        size_t      &out_dataSize,
        DFileData   &out_fileData ) const;
    void reload_DLS( ImgDLS::ImgDLS_Head1 *pOut_Head );
    void reload_Other( const char *pExt );
};

//==================================================================
DFORCEINLINE void image::GetTextureStripCoords(
                float x0,
                float y0,
                Float2 out_coords[4] ) const
{
    auto s0 = x0 * mXtoS;
    auto t0 = y0 * mYtoT;
    out_coords[0] = { s0,     t0     };
    out_coords[1] = { mTexS2, t0     };
    out_coords[2] = { s0,     mTexT2 };
    out_coords[3] = { mTexS2, mTexT2 };
}

//==================================================================
DFORCEINLINE void image::GetTextureStripCoords(
                float x0,
                float y0,
                float x1,
                float y1,
                Float2 out_coords[4] ) const
{
    auto s0 = x0 * mXtoS;
    auto t0 = y0 * mYtoT;
    auto s1 = x1 * mXtoS;
    auto t1 = y1 * mYtoT;
    out_coords[0] = { s0, t0 };
    out_coords[1] = { s1, t0 };
    out_coords[2] = { s0, t1 };
    out_coords[3] = { s1, t1 };
}

//==================================================================
DFORCEINLINE void image::GetTextureTrigCoords(
                float x0,
                float y0,
                Float2 out_coords[6] ) const
{
    auto s0 = x0 * mXtoS;
    auto t0 = y0 * mYtoT;
    out_coords[0] = { s0,     t0     };
    out_coords[1] = { mTexS2, t0     };
    out_coords[2] = { s0,     mTexT2 };
    out_coords[3] = { mTexS2, mTexT2 };
    out_coords[4] = out_coords[2];
    out_coords[5] = out_coords[1];
}

//==================================================================
DFORCEINLINE void image::GetTextureTrigCoords(
                float x0,
                float y0,
                float x1,
                float y1,
                Float2 out_coords[6] ) const
{
    auto s0 = x0 * mXtoS;
    auto t0 = y0 * mYtoT;
    auto s1 = x1 * mXtoS;
    auto t1 = y1 * mYtoT;
    out_coords[0] = { s0, t0 };
    out_coords[1] = { s1, t0 };
    out_coords[2] = { s0, t1 };
    out_coords[3] = { s1, t1 };
    out_coords[4] = out_coords[2];
    out_coords[5] = out_coords[1];
}

//==================================================================
DFORCEINLINE u_int image::GetNextPow2( u_int val )
{
    if NOT( val )
        return 0;

	for (u_int i=0; i < 32; ++i)
		if ( val <= (u_int)(1 << i) )
			return (u_int)(1 << i);

	return 0;
}

//==================================================================
DFORCEINLINE void image::GetPixelUnpacked_s(
            const U8 *pPix,
            u_int depth,
            u_int chans,
            size_t bppix,
            u_int imgFlags,
            u_int dest[MAX_CHANS] )
{
    if ( depth == 8 )
    {
        dest[0] = pPix[0];
    }
    else
    if ( depth == 16 && !(imgFlags & FLG_IS_FLOAT16) )
    {
        U16 val = *(const U16 *)pPix;
        switch ( chans )
        {
        case 1:
        	dest[0] = val;
            break;
        case 2:
        	dest[0] = (val >> 0) & 255;
        	dest[1] = (val >> 8) & 255;
            break;
        case 3: // 565
        	dest[0] = (val >> 11) & 31;
        	dest[1] = (val >>  5) & 63;
        	dest[2] = (val >>  0) & 31;
            break;
        case 4:
            if ( !!(imgFlags & image::FLG_FAVORCOLOR) ) { // 5551
            	dest[0] = (val >> 10) & 31;
            	dest[1] = (val >>  5) & 31;
            	dest[2] = (val >>  0) & 31;
            	dest[3] = (val >> 15) & 1;
            }
            else { // 4444
            	dest[0] = (val >>  8) & 15;
            	dest[1] = (val >>  4) & 15;
            	dest[2] = (val >>  0) & 15;
            	dest[3] = (val >> 12) & 15;
            }
            break;

        default: DASSERT(0); break;
        }
    }
    else
    if ( depth == 24 && chans == 3 )
    {
        dest[0] = pPix[0];
        dest[1] = pPix[1];
        dest[2] = pPix[2];
    }
    else
    if ( depth == 32 && chans == 4 )
    {
        dest[0] = pPix[0];
        dest[1] = pPix[1];
        dest[2] = pPix[2];
        dest[3] = pPix[3];
    }
    else
    {
        u_int bytesPerChan = (u_int)bppix / chans;

        DASSERT( bytesPerChan <= sizeof(dest[0]) );

        switch ( bytesPerChan )
        {
        case 1: for (u_int i=0; i != chans; ++i) dest[i] = ((const U8 *)pPix)[i]; break;
        case 2: for (u_int i=0; i != chans; ++i) dest[i] = ((const U16 *)pPix)[i]; break;
        case 4: for (u_int i=0; i != chans; ++i) dest[i] = ((const U32 *)pPix)[i]; break;
        default: DASSERT( 0 ); break;
        }
    }
}

//==================================================================
DFORCEINLINE void image::SetPixelUnpacked_s(
            U8 *pPix,
            u_int depth,
            u_int chans,
            size_t bppix,
            u_int imgFlags,
            const u_int src[MAX_CHANS] )
{
    if ( depth == 8 )
    {
         pPix[0] = (U8)src[0];
    }
    else
    if ( depth == 16 && !(imgFlags & FLG_IS_FLOAT16) )
    {
        U16 &pixU16 = *(U16 *)pPix;
        switch ( chans )
        {
        case 1:
        	pixU16 = (U16)src[0];
            break;
        case 2:
        	pixU16 = (U16)(
                    ((src[0] & 255) << 8) |
                    ((src[1] & 255) << 0) );
            break;
        case 3: // 565
        	pixU16 = (U16)(
                    ((src[0] & 31) << 11) |
                    ((src[1] & 63) <<  5) |
                    ((src[2] & 31) <<  0) );
            break;
        case 4:
            if ( !!(imgFlags & FLG_FAVORCOLOR) ) { // 5551
            	pixU16 = (U16)(
                        ((src[0] & 31) << 10) |
                        ((src[1] & 31) <<  5) |
                        ((src[2] & 31) <<  0) |
                        ((src[3] &  1) << 15) );
            }
            else { // 4444
            	pixU16 = (U16)(
                        ((src[0] & 15) <<  8) |
                        ((src[1] & 15) <<  4) |
                        ((src[2] & 15) <<  0) |
                        ((src[3] & 15) << 12) );
            }
            break;
        default:
            DASSERT( 0 );
            break;
        }
    }
    else
    if ( depth == 24 && chans == 3 )
    {
        pPix[0] = (U8)src[0];
        pPix[1] = (U8)src[1];
        pPix[2] = (U8)src[2];
    }
    else
    if ( depth == 32 && chans == 4 )
    {
        pPix[0] = (U8)src[0];
        pPix[1] = (U8)src[1];
        pPix[2] = (U8)src[2];
        pPix[3] = (U8)src[3];
    }
    else
    {
        auto bytesPerChan = (int)bppix / (int)chans;

        DASSERT( bytesPerChan <= (int)sizeof(src[0]) );

        switch ( bytesPerChan )
        {
        case 1: for (u_int i=0; i != chans; ++i) ((U8 *)pPix)[i]  = (U8)src[i]; break;
        case 2: for (u_int i=0; i != chans; ++i) ((U16 *)pPix)[i] = (U16)src[i]; break;
        case 4: for (u_int i=0; i != chans; ++i) ((U32 *)pPix)[i] = (U32)src[i]; break;
        default: DASSERT( 0 ); break;
        }
    }
}

//==================================================================
#include "ImageConv.h"

#endif

