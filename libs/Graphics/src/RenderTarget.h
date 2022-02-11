//==================================================================
/// RenderTarget.h
///
/// Created by Davide Pasca - 2010/12/4
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#ifdef ENABLE_OPENGL

#include "GL/glew.h"

#include <functional>
#include "DBase.h"

//==================================================================
class RenderTarget
{
    GLuint          mColTex = 0;
    GLuint          mDepthTex = 0;
    GLuint          mFBID = 0;
    GLuint          mCol_RBID = 0;
    GLuint          mDep_RBID = 0;
    GLuint          mNoMS_FBID = 0;
    GLuint          mNoMS_Col_RBID = 0;
    u_int           mUsedColorDepth = 0;

    GLenum  mTexIntColFmd = 0;
    GLenum  mRendIntColFmd = 0;
    GLenum  mIntDepFmt = 0;

public:
    class Params
    {
    public:
        u_int   width       = 0;
        u_int   height      = 0;
        u_int   colorDepth  = 0; // can be 0
        u_int   colorChansN = 0; // ca be 0 only if hasDepth is true
        bool    useHDR      = false;
        bool    useColTex   = false;
        bool    useDepthTex = false;
        bool    useShadowMapCompare = false;
        bool    hasDepth    = false;
        bool    useLinearFilter = false;
        u_int   samplesN    = 0;

        std::function<void (RenderTarget &rt)> makeColRBuffStorageFn;

        friend bool operator==(const Params &p1, const Params &p2)
        {
            return
                   p1.width                  == p2.width
                && p1.height                 == p2.height
                && p1.colorDepth             == p2.colorDepth
                && p1.colorChansN            == p2.colorChansN
                && p1.useHDR                 == p2.useHDR
                && p1.useColTex              == p2.useColTex
                && p1.useDepthTex            == p2.useDepthTex
                && p1.useShadowMapCompare    == p2.useShadowMapCompare
                && p1.hasDepth               == p2.hasDepth
                && p1.useLinearFilter        == p2.useLinearFilter
                && p1.samplesN               == p2.samplesN
                // assume different if there's any callback in play
                && !p1.makeColRBuffStorageFn
                && !p2.makeColRBuffStorageFn;
        }
    };

private:
	Params	mPar;
	
public:
    RenderTarget( const Params &par );
    ~RenderTarget();

    bool HasSameParams( const Params &par ) const { return mPar == par; }

	u_int GetSize( size_t dimIdx ) const
	{
		DASSERT( dimIdx == 0 || dimIdx == 1 );
		return dimIdx == 0 ? mPar.width : mPar.height;
	}

    Int2 GetSize() const { return Int2( mPar.width, mPar.height ); }

    u_int GetColorChansN() const { return mPar.colorChansN; }

    bool IsMultisample() const { return mPar.samplesN != 0; }

    GLenum GetTexGLTarget() const { return GL_TEXTURE_2D; }

    u_int GetColTex() const { return mColTex; }
    u_int GetDepthTex() const { return mDepthTex; }

    u_int GetFBID() const { return mFBID; }
    u_int GetNoMSFBID() const { return mNoMS_FBID; }
    u_int GetColRBID() const { return mCol_RBID; }

    bool HasColTex() const { return !!mColTex; }
    bool HasDepthTex() const { return !!mDepthTex; }
    bool HasDepth() const { return mPar.hasDepth; }
    bool IsHDR() const { return mPar.useHDR; }

    void SetupDesImageFromRT(
                    image &desImg,
                    bool force8Bit,
                    u_int useW=(u_int)-1,
                    u_int useH=(u_int)-1 ) const;

	void CopyToImage( image &desImg, bool force8Bit=false );

    size_t CalcSizeBytesN( const Int4 *pRect=nullptr ) const;

	void ResolveMSFBO();

private:
    void deleteResources();

    static void imageInfoFromGLColFmt_s(
        GLenum intColFmt,
        GLenum &out_dataType,
        u_int &out_chansN,
        u_int &out_depth );

    void buildNonMultisampleFB();
};

#endif

#endif
