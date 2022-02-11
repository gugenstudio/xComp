//==================================================================
/// RenderTarget.cpp
///
/// Created by Davide Pasca - 2010/12/4
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OPENGL

#include "GL/glew.h"
#include "Image.h"
#include "RenderTarget.h"

#define MASTERCHECKGLERR

#define RGB_8_REND_INTERNAL  GL_RGB
#define RGBA_8_REND_INTERNAL GL_RGBA

//==================================================================
static GLuint setRenderbufferStorage(
                        GLuint samples,
                        GLenum rendIntFmd,
                        GLuint w,
                        GLuint h )
{
    MASTERCHECKGLERR;
#if defined(GL_INTERNALFORMAT_SUPPORTED)
    if ( samples )
    {
        GLint maxSamples[] = { 0 };
        if NOT( glGetInternalformativ ) // overly shitty driver ?
            maxSamples[0] = 4;
        else
            glGetInternalformativ( GL_RENDERBUFFER, rendIntFmd, GL_SAMPLES, 1, maxSamples );

        if ( (int)samples > maxSamples[0] )
        {
            //DPRINT( "Clamping samples count from %i to %i", (int)samples, (int)maxSamples[0] );
            samples = (GLuint)maxSamples[0];
        }
    }

    if ( samples )
    {
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, rendIntFmd, w, h );
        MASTERCHECKGLERR;
        return samples;
    }
#endif

	glRenderbufferStorage( GL_RENDERBUFFER, rendIntFmd, w, h );
    MASTERCHECKGLERR;
    return samples;
}

//==================================================================
static u_int makeTexture(
        GLenum target,
        GLenum texIntFmt,
        GLenum format,
        GLenum type,
        GLuint w,
        GLuint h,
        GLuint samplesN,
        bool useLinear )
{
    MASTERCHECKGLERR;
    u_int texID;
    glGenTextures( 1, &texID );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( target, texID );
    MASTERCHECKGLERR;

#if defined(GL_TEXTURE_2D_MULTISAMPLE)
    if ( target != GL_TEXTURE_2D_MULTISAMPLE )
#endif
    {
        u_int filter_gl = (useLinear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri( target, GL_TEXTURE_MAG_FILTER, filter_gl );
        glTexParameteri( target, GL_TEXTURE_MIN_FILTER, filter_gl );
        glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        MASTERCHECKGLERR;
    }

#if defined(GL_TEXTURE_2D_MULTISAMPLE)
    if ( samplesN )
        glTexImage2DMultisample(
                target,
                samplesN,
                texIntFmt,
                w,
                h,
                GL_FALSE );
    else
#endif
        glTexImage2D(
                target,
                0,
                texIntFmt,
                w,
                h,
                0,
                format,
                type,
                nullptr );

    return texID;
}

//==================================================================
static void determineIntColFmdAndDataType(
        u_int depth,
        u_int chansN,
        bool useHDR,
        GLenum &out_texIntColFmt,
        GLenum &out_rendIntColFmt,
        GLenum &out_colorDataType )
{
    if ( chansN == 3 )
        out_texIntColFmt = GL_RGB;
    else
        out_texIntColFmt = GL_RGBA;

    if ( useHDR )
    {
        switch ( depth )
        {
#if defined(GL_RGB10_A2)
        case 32:
            DASSERT( chansN == 4 );
            out_rendIntColFmt = GL_RGB10_A2;
            out_colorDataType = GL_UNSIGNED_INT_2_10_10_10_REV;
            out_texIntColFmt = GL_RGBA;
            break;
#endif

#if defined(GL_RGB16F)
        case 48:
            DASSERT( chansN == 3 );
            out_texIntColFmt = GL_RGB16F;
            out_rendIntColFmt = GL_RGB16F;
            out_colorDataType = GL_HALF_FLOAT;
            break;

        case 64:
            DASSERT( chansN == 4 );
            out_texIntColFmt = GL_RGBA16F;
            out_rendIntColFmt = GL_RGBA16F;
            out_colorDataType = GL_HALF_FLOAT;
            break;
#endif

#if defined(GL_RGB32F)
        case 96:
            DASSERT( chansN == 3 );
            out_texIntColFmt = GL_RGB32F;
            out_rendIntColFmt = GL_RGB32F;
            out_colorDataType = GL_FLOAT;
            break;

        case 128:
            DASSERT( chansN == 4 );
            out_texIntColFmt = GL_RGBA32F;
            out_rendIntColFmt = GL_RGBA32F;
            out_colorDataType = GL_FLOAT;
            break;
#endif

        default: // unsupported
            DASSERT(0);
            out_rendIntColFmt = RGB_8_REND_INTERNAL;
            out_colorDataType = GL_UNSIGNED_BYTE;
            break;
        }
    }
    else
    {
        out_colorDataType = GL_UNSIGNED_BYTE;
        out_rendIntColFmt = RGB_8_REND_INTERNAL;

        switch ( depth )
        {
        case 16:
            DASSERT( chansN == 3 ); // may want to try 4 as well
            out_rendIntColFmt = GL_RGB565;
            out_colorDataType = GL_UNSIGNED_SHORT_5_6_5;
            break;

        case 24: DASSERT( chansN == 3 ); break;

        case 32: DASSERT( chansN == 4 ); break;

        default: DASSERT(0); break;
        }
    }
}

//==================================================================
RenderTarget::RenderTarget( const Params &par ) : mPar(par)
{
    if ( par.colorChansN == 0 )
    {
        if NOT( par.hasDepth )
            DEX_RUNTIME_ERROR( "Requested RT has no color channels nor any depth" );

        if ( mPar.useColTex )
            DEX_RUNTIME_ERROR( "Requested RT has no color channels but requests color texture" );
    }

    // internal color format and data type
    GLenum colorDataType = GL_NONE;

    if ( par.colorChansN )
    {
        if ( mPar.colorDepth == 0 )
        {
            if ( mPar.useHDR )
                mUsedColorDepth = (mPar.colorChansN * 16); // half-float
            else
                mUsedColorDepth = (mPar.colorChansN * 8);
        }
        else
            mUsedColorDepth = mPar.colorDepth;

        determineIntColFmdAndDataType(
                mUsedColorDepth,
                mPar.colorChansN,
                mPar.useHDR,
                mTexIntColFmd,
                mRendIntColFmd,
                colorDataType );
    }

    u_int intDepthFmtForTexture;
    u_int dataTypeForDepthTexture;

    // internal depth format
    mIntDepFmt = GL_DEPTH_COMPONENT;

    // assumes 24/32
    intDepthFmtForTexture = GL_DEPTH_COMPONENT;
    dataTypeForDepthTexture = GL_UNSIGNED_INT;

    // save currently bound render anf frame buffers
    GLint savedFB {}; glGetIntegerv( GL_FRAMEBUFFER_BINDING, &savedFB );
    GLint savedRB {}; glGetIntegerv( GL_RENDERBUFFER_BINDING, &savedRB );

    //
    glGenFramebuffers( 1, &mFBID );
	glBindFramebuffer( GL_FRAMEBUFFER, mFBID );

    if ( par.colorChansN )
    {
        glGenRenderbuffers( 1, &mCol_RBID );
        glBindRenderbuffer( GL_RENDERBUFFER, mCol_RBID );

        if ( mPar.makeColRBuffStorageFn )
        {
            mPar.makeColRBuffStorageFn( *this );
            MASTERCHECKGLERR;
        }
        else
        {
            mPar.samplesN = setRenderbufferStorage(
                    mPar.samplesN,
                    mRendIntColFmd,
                    mPar.width,
                    mPar.height );
        }

        MASTERCHECKGLERR;
        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_RENDERBUFFER,
                mCol_RBID );
    }
    else
    {
        const GLenum none = GL_NONE;

        glDrawBuffers( 1, &none );
    }

	if ( mPar.hasDepth )
    {
        glGenRenderbuffers( 1, &mDep_RBID );
        glBindRenderbuffer( GL_RENDERBUFFER, mDep_RBID );
        mPar.samplesN = setRenderbufferStorage(
                mPar.samplesN,
                mIntDepFmt,
                mPar.width,
                mPar.height );

        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER,
                mDep_RBID );
    }

	if ( mPar.useColTex )
    {
        if ( mPar.samplesN )
            buildNonMultisampleFB();

        DASSERT( mPar.colorChansN == 3 || mPar.colorChansN == 4 );

        mColTex = makeTexture(
                    GL_TEXTURE_2D,
                    mTexIntColFmd,
                    mPar.colorChansN == 3 ? GL_RGB : GL_RGBA,
                    colorDataType,
                    mPar.width,
                    mPar.height,
                    0, // 0 samples !
                    mPar.useLinearFilter );

        glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                mColTex,
                0 );

		glBindTexture( GL_TEXTURE_2D, 0 );
        MASTERCHECKGLERR;

        if ( mPar.samplesN )
        {
        	glBindFramebuffer( GL_FRAMEBUFFER, mFBID );
            glBindRenderbuffer( GL_RENDERBUFFER, mCol_RBID );
            MASTERCHECKGLERR;
        }
    }

	if ( mPar.useDepthTex )
    {
        DASSERT( mPar.samplesN == 0 );

        bool useLinear  = false;
#if defined(GL_TEXTURE_COMPARE_FUNC)
        if ( mPar.useShadowMapCompare )
            useLinear = true;
#endif

        mDepthTex = makeTexture(
                        GL_TEXTURE_2D,
                        intDepthFmtForTexture,
                        GL_DEPTH_COMPONENT,
                        dataTypeForDepthTexture,
                        mPar.width,
                        mPar.height,
                        mPar.samplesN,
                        useLinear );

#if defined(GL_TEXTURE_COMPARE_FUNC)
        if ( mPar.useShadowMapCompare )
        {
            // set up the depth compare function to check the shadow depth in hardware
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        }
#endif
		glBindTexture( GL_TEXTURE_2D, 0 );

        glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D,
                    mDepthTex,
                    0 );

        MASTERCHECKGLERR;
    }

	auto status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	// switch back to window-system-provided framebuffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    if ( status != GL_FRAMEBUFFER_COMPLETE )
    {
        // manually delete because dtor won't be called
        deleteResources();

        // bring back the old bound render anf frame buffers
        glBindRenderbuffer( GL_RENDERBUFFER, savedRB );
        glBindFramebuffer( GL_FRAMEBUFFER, savedFB );

        DEX_RUNTIME_ERROR(
            "Could not create the frame buffer object ! Status: %i",
                (int)status );
    }

    // bring back the old bound render anf frame buffers
    glBindRenderbuffer( GL_RENDERBUFFER, savedRB );
    glBindFramebuffer( GL_FRAMEBUFFER, savedFB );
}

//==================================================================
void RenderTarget::deleteResources()
{
    if (mColTex       ){glDeleteTextures(1, &mColTex            );mColTex       =0;}
    if (mDepthTex     ){glDeleteTextures(1, &mDepthTex          );mDepthTex     =0;}
    if (mFBID         ){glDeleteFramebuffers(1, &mFBID          );mFBID         =0;}
    if (mCol_RBID     ){glDeleteRenderbuffers(1, &mCol_RBID     );mCol_RBID     =0;}
    if (mDep_RBID     ){glDeleteRenderbuffers(1, &mDep_RBID     );mDep_RBID     =0;}
    if (mNoMS_FBID    ){glDeleteFramebuffers(1, &mNoMS_FBID     );mNoMS_FBID    =0;}
    if (mNoMS_Col_RBID){glDeleteRenderbuffers(1, &mNoMS_Col_RBID);mNoMS_Col_RBID=0;}
}

//==================================================================
RenderTarget::~RenderTarget()
{
    deleteResources();
}

//==================================================================
void RenderTarget::buildNonMultisampleFB()
{
#if defined(GL_TEXTURE_2D_MULTISAMPLE)
    glGenFramebuffers( 1, &mNoMS_FBID );
    glBindFramebuffer( GL_FRAMEBUFFER, mNoMS_FBID );

    glGenRenderbuffers( 1, &mNoMS_Col_RBID );
    glBindRenderbuffer( GL_RENDERBUFFER, mNoMS_Col_RBID );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB8, mPar.width, mPar.height );

    glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER,
            mNoMS_Col_RBID );

    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    //glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    if ( status != GL_FRAMEBUFFER_COMPLETE )
    {
        DEX_RUNTIME_ERROR(
            "Could not create the frame buffer object ! Status: %i",
                status );
    }
#endif
}

//==================================================================
void RenderTarget::ResolveMSFBO()
{
#if defined(GL_TEXTURE_2D_MULTISAMPLE)
    // nothing to do ?
    if NOT( IsMultisample() )
        return;

    GLint oldRead {}; glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &oldRead );
    GLint oldDraw {}; glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &oldDraw );

    if ( mNoMS_FBID == 0 )
        buildNonMultisampleFB();

    // bind the MS FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBID);
    // bind the standard FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mNoMS_FBID);

    // blit
    glBlitFramebuffer(
            0, 0, mPar.width, mPar.height,
            0, 0, mPar.width, mPar.height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST );

    glBindFramebuffer( GL_READ_FRAMEBUFFER, oldRead );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, oldDraw );
#endif
}

//==================================================================
void RenderTarget::imageInfoFromGLColFmt_s(
        GLenum texIntColFmt,
        GLenum &out_dataType,
        u_int &out_chansN,
        u_int &out_depth )
{
    switch ( texIntColFmt )
    {
    case GL_RGB16F:
        out_dataType = GL_HALF_FLOAT;
        out_depth = 16 * 3;
        out_chansN = 3;
        //par.flags = image::FLG_IS_FLOAT16;
        break;
    case GL_RGBA16F:
        out_dataType = GL_HALF_FLOAT;
        out_depth = 16 * 4;
        out_chansN = 4;
        //par.flags = image::FLG_IS_FLOAT16;
        break;
    case GL_RGB32F:
        out_dataType = GL_FLOAT;
        out_depth = 32 * 3;
        out_chansN = 3;
        //par.flags = image::FLG_IS_FLOAT32;
        break;
    case GL_RGBA32F:
        out_dataType = GL_FLOAT;
        out_depth = 32 * 4;
        out_chansN = 4;
        //par.flags = image::FLG_IS_FLOAT32;
        break;
    default:
        out_dataType = GL_UNSIGNED_BYTE;
        out_depth = 8 * 3;
        out_chansN = 3;
        break;
    }
}

//==================================================================
size_t RenderTarget::CalcSizeBytesN( const Int4 *pRect ) const
{
    GLenum dataType;
    u_int chansN;
    u_int depth;
    imageInfoFromGLColFmt_s( mTexIntColFmd, dataType, chansN, depth );

    size_t useW;
    size_t useH;
    if ( pRect )
    {
        useW = (size_t)(*pRect)[2];
        useH = (size_t)(*pRect)[3];
    }
    else
    {
        useW = (size_t)mPar.width;
        useH = (size_t)mPar.height;
    }

    // make sure it's aligned to 4, like for OpenGL
    size_t bytesPerRow = (useW * (depth/8) + 3) & ~3;
    size_t sizeBytesN = bytesPerRow * useH;

    return sizeBytesN;
}

//==================================================================
void RenderTarget::SetupDesImageFromRT(
                    image &desImg,
                    bool force8Bit,
                    u_int useW,
                    u_int useH ) const
{
    image::Params par;
    par.width       = (useW != (u_int)-1) ? useW : mPar.width;
    par.height      = (useH != (u_int)-1) ? useH : mPar.height;

    // make sure it's aligned to 4, like for OpenGL
    par.autoRowPitchAlign = 4;

    par.chans = 3;
    par.depth = 24;

    // ignore alpha, but honor float (16/32)
    //GLenum dataType = GL_UNSIGNED_BYTE;
    if NOT( force8Bit )
    {
        switch ( mTexIntColFmd )
        {
        case GL_RGB16F:
            //dataType = GL_HALF_FLOAT;
            par.depth = 16 * 3;
            par.flags = image::FLG_IS_FLOAT16;
            break;
        case GL_RGBA16F:
            //dataType = GL_HALF_FLOAT;
            par.depth = 16 * 4;
            par.flags = image::FLG_IS_FLOAT16;
            break;
        case GL_RGB32F:
            //dataType = GL_FLOAT;
            par.depth = 32 * 3;
            par.flags = image::FLG_IS_FLOAT32;
            break;
        case GL_RGBA32F:
            //dataType = GL_FLOAT;
            par.depth = 32 * 4;
            par.flags = image::FLG_IS_FLOAT32;
            break;
        default:
            break;
        }
    }

    desImg.Setup( par );
}

//==================================================================
static GLenum getDataTypeFromImage( const image &img )
{
    if ((img.mFlags & image::FLG_IS_FLOAT16)) return GL_HALF_FLOAT; else
    if ((img.mFlags & image::FLG_IS_FLOAT32)) return GL_FLOAT;

    return GL_UNSIGNED_BYTE;
}

//==================================================================
void RenderTarget::CopyToImage( image &desImg, bool force8Bit )
{
    desImg = image();
    SetupDesImageFromRT( desImg, force8Bit );

	desImg.Clear();

    //
    GLint oldReadFB {}; glGetIntegerv( GL_FRAMEBUFFER_BINDING, &oldReadFB );

    if ( IsMultisample() )
    {
        ResolveMSFBO();

        glBindFramebuffer( GL_READ_FRAMEBUFFER, GetNoMSFBID() );
    }
    else
        glBindFramebuffer( GL_READ_FRAMEBUFFER, GetFBID() );

    auto dataType = getDataTypeFromImage( desImg );

    glReadPixels(
        0,
        0,
        desImg.mW,
        desImg.mH,
        GL_RGB,
        dataType,
        (void *)desImg.GetPixelPtr(0,0) );

    MASTERCHECKGLERR;
}

#endif

