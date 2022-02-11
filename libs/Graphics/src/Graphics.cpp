//==================================================================
/// Graphics.cpp
///
/// Created by Davide Pasca - 2018/02/14
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OPENGL
# include "GL/glew.h"
# include "DGLUtils.h"
#endif
#include "GShaderProg.h"
#include "Graphics.h"

#define USE_SOFT_MIPMAPS
//#define USE_HARD_MIPMAPS

//==================================================================
template <typename D, typename S>
inline void setQuadStripAsTrigsP( D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
{
    out_des[0].pos = v0;
    out_des[1].pos = v1;
    out_des[2].pos = v2;
    out_des[3].pos = v3;
    out_des[4].pos = v2;
    out_des[5].pos = v1;
}
template <typename D, typename S>
inline void setQuadStripAsTrigsC( D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
{
    out_des[0].col = v0;
    out_des[1].col = v1;
    out_des[2].col = v2;
    out_des[3].col = v3;
    out_des[4].col = v2;
    out_des[5].col = v1;
}
template <typename D, typename S>
inline void setQuadStripAsTrigsT( D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
{
    out_des[0].tc0 = v0;
    out_des[1].tc0 = v1;
    out_des[2].tc0 = v2;
    out_des[3].tc0 = v3;
    out_des[4].tc0 = v2;
    out_des[5].tc0 = v1;
}

//==================================================================
Graphics::Graphics( int majorVer, bool isSWRendering )
{
#ifdef ENABLE_OPENGL
    mUseShaders = (majorVer >= 4 && !isSWRendering);

    if ( mUseShaders )
    {
        FLUSHGLERR;

        moShaProgs.push_back( std::make_unique<GShaderProg>( false ) );
        moShaProgs.push_back( std::make_unique<GShaderProg>( true ) );

        //
        glGenBuffers( 1, &mVBO );
        CHECKGLERR;

        //
        glGenVertexArrays( 1, &mVAO );
        CHECKGLERR;
    }
#endif
}

//==================================================================
Graphics::~Graphics()
{
}

//==================================================================
void Graphics::SetScissorXForm( bool onOff, double x1, double y1, double x2, double y2 )
{
    c_auto p1Xfmed = DXFormPos( Double3( x1, y1, 0.0 ), mCurXForm );
    c_auto p2Xfmed = DXFormPos( Double3( x2, y2, 0.0 ), mCurXForm );

    c_auto vpSiz = GetVPSize();
    c_auto x1Xfmed = std::min( p1Xfmed[0], p2Xfmed[0] ) * vpSiz[0];
    c_auto x2Xfmed = std::max( p1Xfmed[0], p2Xfmed[0] ) * vpSiz[0];
    c_auto y1Xfmed = std::min( p1Xfmed[1], p2Xfmed[1] ) * vpSiz[1];
    c_auto y2Xfmed = std::max( p1Xfmed[1], p2Xfmed[1] ) * vpSiz[1];

    SetScissor(
        onOff,
        Float4( (float)(x1Xfmed          ),
                (float)(y1Xfmed          ),
                (float)(x2Xfmed - x1Xfmed),
                (float)(y2Xfmed - y1Xfmed) ) );
}

//==================================================================
void Graphics::SetScissor( bool onOff, const Float4 &rc )
{
    if ( mCurScissOnOff == onOff && mCurScissRC == rc )
        return;

    FlushPrims();

    mCurScissOnOff = onOff;
    mCurScissRC = rc;

    DASSERT( rc[2] >= 0 && rc[3] >= 0 );

#ifdef ENABLE_OPENGL
    if ( onOff )
    {
        glEnable( GL_SCISSOR_TEST );
        glScissor(
                (int)rc[0],
                (int)rc[1],
                (int)rc[2],
                (int)rc[3] );
    }
    else
    {
        glDisable( GL_SCISSOR_TEST );
    }
#endif
}

//==================================================================
bool Graphics::IsPointInScissor( const Double2 &pos ) const
{
    if NOT( mCurScissOnOff )
        return true;

    c_auto posXForm = MakeXformedP( pos );
    c_auto vpSiz = GetVPSize();
    c_auto x = (float)(posXForm[0] * vpSiz[0]);
    c_auto y = (float)(posXForm[1] * vpSiz[1]);

    return x >= mCurScissRC[0] && x <= (mCurScissRC[0] + mCurScissRC[2]) &&
           y >= mCurScissRC[1] && y <= (mCurScissRC[1] + mCurScissRC[3]);
}

//==================================================================
void Graphics::SetXForm( const Matrix44D &xform )
{
    mCurXForm = xform;

    updateCurPixSize();
}

//==================================================================
void Graphics::SetBlendNone()
{
#ifdef ENABLE_OPENGL
    if ( mCurBlendMode == BM_NONE )
        return;

    mCurBlendMode = BM_NONE;
    FlushPrims();

    glDisable( GL_BLEND );
#endif
}

//==================================================================
void Graphics::SetBlendAdd()
{
#ifdef ENABLE_OPENGL
    if ( mCurBlendMode == BM_ADD )
        return;

    mCurBlendMode = BM_ADD;
    FlushPrims();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
#endif
}

//==================================================================
void Graphics::SetBlendAlpha()
{
#ifdef ENABLE_OPENGL
    if ( mCurBlendMode == BM_ALPHA )
        return;

    mCurBlendMode = BM_ALPHA;
    FlushPrims();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#endif
}

//==================================================================
#ifdef ENABLE_OPENGL
static void checkGLErr( const char *pFile, int line )
{
    while (auto err = glGetError())
    {
        printf(
            "%s:%i -- OpenGL Error # %u\n",
            pFile,
            line,
            err );
    }
}
#endif

//==================================================================
void Graphics::UploadImageTexture( image &img )
{
#ifdef ENABLE_OPENGL
    CHECKGLERR;

#if defined(USE_SOFT_MIPMAPS) || defined(USE_HARD_MIPMAPS)
    c_auto usingMips = !!(img.mFlags & image::FLG_USE_MIPMAPS);
#else
    c_auto usingMips = false;
#endif

    if NOT( img.IsTextureIDValid() )
    {
        glGenTextures( 1, &img.mImageTexID0 );
        glBindTexture( GL_TEXTURE_2D, img.mImageTexID0 );

        auto setParamI = [&]( auto a, auto b )
        {
            glTexParameteri( GL_TEXTURE_2D, a, b );
        };

        c_auto usingLinear = !!(img.mFlags & image::FLG_USE_BILINEAR);

        setParamI( GL_TEXTURE_MAG_FILTER,
                    usingLinear
                        ? GL_LINEAR
                        : GL_NEAREST );

        setParamI( GL_TEXTURE_MIN_FILTER,
                    usingLinear
                        ? (usingMips ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST)
                        : (usingMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) );

        setParamI( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        setParamI( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        CHECKGLERR;
    }

    u_int fmt = GL_RGBA;

    if ( img.mChans == 3 )
        fmt = GL_RGB;
    else
    if ( img.mChans == 4 )
        fmt = GL_RGBA;
    else
    {
        DASSERT( 0 );
    }

    glBindTexture( GL_TEXTURE_2D, img.mImageTexID0 );

    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            fmt, //GL_RGBA8,
            img.mW,
            img.mH,
            0,
            fmt,
            img.IsFloat32() ? GL_FLOAT : GL_UNSIGNED_BYTE,
            img.GetPixelPtr(0,0) );

    CHECKGLERR;

#ifdef USE_SOFT_MIPMAPS
    if ( usingMips )
    {
        auto tmp = img;
        int level = 1;
        while ( tmp.mW >= 4 && tmp.mH >= 4 )
        {
            tmp.ScaleQuarter();

            glTexImage2D(
                    GL_TEXTURE_2D,
                    level++,
                    fmt, //GL_RGBA8,
                    tmp.mW,
                    tmp.mH,
                    0,
                    fmt,
                    img.IsFloat32() ? GL_FLOAT : GL_UNSIGNED_BYTE,
                    tmp.GetPixelPtr(0,0) );

            CHECKGLERR;
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level-1 );
    }
#endif

#ifdef USE_HARD_MIPMAPS
    if ( usingMips )
    {
        glGenerateMipmap( GL_TEXTURE_2D );
        CHECKGLERR;
    }
#endif

    glBindTexture( GL_TEXTURE_2D, 0 );
#endif
}

//==================================================================
void Graphics::SetTextureFromImage( const image &img )
{
#ifdef ENABLE_OPENGL
    c_auto texID = img.GetTextureID();
    SetTexture( texID );
#endif
}

//==================================================================
void Graphics::SetTexture( u_int texID )
{
#ifdef ENABLE_OPENGL
    if ( mCurTexID == texID )
        return;

    FlushPrims();
    mCurTexID = texID;
#endif
}

//==================================================================
void Graphics::switchModeFlags( u_int flags )
{
#ifdef ENABLE_OPENGL
    if ( mModeFlags == flags )
        return;

    FlushPrims();
    mModeFlags = flags;
#endif
}

//==================================================================
inline std::array<Float3,4> Graphics::makeRectVtxPos(
                                const Double2 &pos,
                                const Double2 &siz ) const
{
    return
    {
        MakeXformedP( pos[0]+siz[0]*0, pos[1]+siz[1]*0 ),
        MakeXformedP( pos[0]+siz[0]*1, pos[1]+siz[1]*0 ),
        MakeXformedP( pos[0]+siz[0]*0, pos[1]+siz[1]*1 ),
        MakeXformedP( pos[0]+siz[0]*1, pos[1]+siz[1]*1 )
    };
}

//==================================================================
void Graphics::DrawRectFill(
            const Double2 &pos,
            const Double2 &siz,
            const std::array<ColorF,4> &cols )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( 0 );

    auto *pVtx = Dgrow( mVtxPC, 6 );
    c_auto vps = makeRectVtxPos( pos, siz );
    setQuadStripAsTrigsP<VtxPC>( pVtx, vps[0], vps[1], vps[2], vps[3] );

    const auto colxf0 = MakeXformedC( cols[0] );
    const auto colxf1 = MakeXformedC( cols[1] );
    const auto colxf2 = MakeXformedC( cols[2] );
    const auto colxf3 = MakeXformedC( cols[3] );
    setQuadStripAsTrigsC( pVtx, colxf0, colxf1, colxf2, colxf3 );
#endif
}

//==================================================================
void Graphics::DrawRectFill(
            const Double2 &pos,
            const Double2 &siz,
            const ColorF &col )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( 0 );

    auto *pVtx = Dgrow( mVtxPC, 6 );
    c_auto vps = makeRectVtxPos( pos, siz );
    setQuadStripAsTrigsP( pVtx, vps[0], vps[1], vps[2], vps[3] );

    c_auto colxf = MakeXformedC( col );
    setQuadStripAsTrigsC( pVtx, colxf, colxf, colxf, colxf );
#endif
}

//==================================================================
void Graphics::DrawRectFrame(
            const Double2 &pos,
            const Double2 &siz,
            const ColorF &col )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( FLG_LINES );

    auto *pVtx = Dgrow( mVtxPC, 8 );
    c_auto vps = makeRectVtxPos( pos, siz );

    pVtx[0].pos = vps[0];   pVtx[1].pos = vps[1];
    pVtx[2].pos = vps[1];   pVtx[3].pos = vps[3];
    pVtx[4].pos = vps[2];   pVtx[5].pos = vps[3];
    pVtx[6].pos = vps[2];   pVtx[7].pos = vps[0];

    c_auto colxf = MakeXformedC( col );
    for (size_t i=0; i != 8; ++i)
        pVtx[i].col = colxf;
#endif
}

//==================================================================
void Graphics::DrawRectTex(
        const Double2 &pos,
        const Double2 &siz,
        const ColorF &col,
        const Float4 &txcBox )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( FLG_TEX );

    auto *pVtx = Dgrow( mVtxPCT, 6 );
    c_auto vps = makeRectVtxPos( pos, siz );
    setQuadStripAsTrigsP( pVtx, vps[0], vps[1], vps[2], vps[3] );

    c_auto colxf = MakeXformedC( col );
    setQuadStripAsTrigsC( pVtx, colxf, colxf, colxf, colxf );

    c_auto tc0 = Float2( txcBox[0], txcBox[1] );
    c_auto tc1 = Float2( txcBox[2], txcBox[1] );
    c_auto tc2 = Float2( txcBox[0], txcBox[3] );
    c_auto tc3 = Float2( txcBox[2], txcBox[3] );
    setQuadStripAsTrigsT( pVtx, tc0, tc1, tc2, tc3 );
#endif
}

//==================================================================
void Graphics::DrawQuad( const std::array<Double2,4> &vps, const ColorF &col )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( 0 );

    auto *pVtx = Dgrow( mVtxPC, 6 );

    setQuadStripAsTrigsP(
            pVtx,
            MakeXformedP( vps[0] ),
            MakeXformedP( vps[1] ),
            MakeXformedP( vps[2] ),
            MakeXformedP( vps[3] ) );

    c_auto colxf = MakeXformedC( col );
    setQuadStripAsTrigsC( pVtx, colxf, colxf, colxf, colxf );
#endif
}

//==================================================================
void Graphics::DrawLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( FLG_LINES );

    auto *pVtx = Dgrow( mVtxPC, 2 );
    pVtx[0].pos = MakeXformedP( p1 );
    pVtx[1].pos = MakeXformedP( p2 );

    pVtx[0].col =
    pVtx[1].col = MakeXformedC( col );
#endif
}

//==================================================================
void Graphics::DrawLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col1,
        const ColorF &col2 )
{
#ifdef ENABLE_OPENGL
    switchModeFlags( FLG_LINES );

    auto *pVtx = Dgrow( mVtxPC, 2 );
    pVtx[0].pos = MakeXformedP( p1 );
    pVtx[1].pos = MakeXformedP( p2 );

    pVtx[0].col = MakeXformedC( col1 );
    pVtx[1].col = MakeXformedC( col2 );
#endif
}

//==================================================================
DVec<Graphics::VtxPC> &Graphics::BeginRawLines()
{
#ifdef ENABLE_OPENGL
    switchModeFlags( FLG_LINES );
#endif
    return mVtxPC;
}

//==================================================================
void Graphics::ResetStates()
{
#ifdef ENABLE_OPENGL
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    mCurBlendMode = BM_NONE;
    glDisable( GL_BLEND );

    mModeFlags = 0;

    glDisable( GL_TEXTURE_2D );
    mCurTexID = (u_int)0;
    glBindTexture( GL_TEXTURE_2D, mCurTexID );

    mVP_Siz = {1,1};

    mCurScissOnOff = false;
    glDisable( GL_SCISSOR_TEST );

    mCurScissRC = {0,0,0,0};
    glScissor( 0, 0, 0, 0 );

    //
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    //
    if ( mUseShaders )
    {
        mCurShaderProgram = 0;
        glUseProgram( 0 );

        //
        glBindVertexArray( 0 );
    }
#endif
}

//==================================================================
void Graphics::FlushPrims()
{
#ifdef ENABLE_OPENGL
    c_auto n = (GLsizei)((mModeFlags & FLG_TEX) ? mVtxPCT.size() : mVtxPC.size());
    if NOT( n )
        return;

    if ( mUseShaders )
    {
        FLUSHGLERR;
        c_auto newVBOSize  = (mModeFlags & FLG_TEX)
                            ? mVtxPCT.size() * sizeof(mVtxPCT[0])
                            : mVtxPC.size()  * sizeof(mVtxPC[0]);

        glBindBuffer( GL_ARRAY_BUFFER, mVBO );
        CHECKGLERR;

        // expand as necessary
        if ( mLastVBOSize != newVBOSize )
        {
            mLastVBOSize = newVBOSize;
            glBufferData( GL_ARRAY_BUFFER, newVBOSize, 0, GL_DYNAMIC_DRAW );
        }
        CHECKGLERR;

        if ( (mModeFlags & FLG_TEX) )
            glBufferSubData( GL_ARRAY_BUFFER, 0, newVBOSize, mVtxPCT.data() );
        else
            glBufferSubData( GL_ARRAY_BUFFER, 0, newVBOSize, mVtxPC.data() );
        CHECKGLERR;

        //
        glBindVertexArray( mVAO );

        glEnableVertexAttribArray( 0 );
        CHECKGLERR;
        glEnableVertexAttribArray( 1 );
        CHECKGLERR;
        if ( (mModeFlags & FLG_TEX) )
        {
            auto vp = []( c_auto idx, c_auto cnt, c_auto off )
            {
                glVertexAttribPointer( idx, cnt, GL_FLOAT, GL_FALSE, sizeof(VtxPCT), (const void *)off );
                CHECKGLERR;
            };
            vp( 0, 3, offsetof(VtxPCT,pos) );
            vp( 1, 4, offsetof(VtxPCT,col) );

            glEnableVertexAttribArray( 2 );
            CHECKGLERR;
            vp( 2, 2, offsetof(VtxPCT,tc0) );

            //
            glActiveTexture( GL_TEXTURE0 );
            CHECKGLERR;
            glBindTexture( GL_TEXTURE_2D, mCurTexID );
            CHECKGLERR;
        }
        else
        {
            auto vp = []( c_auto idx, c_auto cnt, c_auto off )
            {
                glVertexAttribPointer( idx, cnt, GL_FLOAT, GL_FALSE, sizeof(VtxPC), (const void *)off );
                CHECKGLERR;
            };
            vp( 0, 3, offsetof(VtxPC,pos) );
            vp( 1, 4, offsetof(VtxPC,col) );

            glDisableVertexAttribArray( 2 );
            CHECKGLERR;
        }

        //
        if (c_auto progID = moShaProgs[(mModeFlags & FLG_TEX) ? 1 : 0]->GetProgramID();
                   progID != mCurShaderProgram )
        {
            mCurShaderProgram = progID;
            glUseProgram( progID );
            CHECKGLERR;
        }
    }
    else
    {
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );

        if ( (mModeFlags & FLG_TEX) )
        {
            glVertexPointer( 3, GL_FLOAT, sizeof(VtxPCT), &mVtxPCT[0].pos );
            glColorPointer( 4, GL_FLOAT, sizeof(VtxPCT), &mVtxPCT[0].col );

            DASSERT( mCurTexID );

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, sizeof(VtxPCT), &mVtxPCT[0].tc0 );

            //
            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, mCurTexID );
        }
        else
        {
            glVertexPointer( 3, GL_FLOAT, sizeof(VtxPC), &mVtxPC[0].pos );
            glColorPointer( 4, GL_FLOAT, sizeof(VtxPC), &mVtxPC[0].col );

            glDisableClientState( GL_TEXTURE_COORD_ARRAY );

            //
            glDisable( GL_TEXTURE_2D );
        }
    }

    //
    glDrawArrays( (mModeFlags & FLG_LINES) ? GL_LINES : GL_TRIANGLES, 0, n );
    CHECKGLERR;

    if ( mUseShaders )
    {
        //glUseProgram( 0 );
        glBindVertexArray( 0 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        CHECKGLERR;
    }

    mVtxPC.clear();
    mVtxPCT.clear();
#endif
}

//==================================================================
image Graphics::GrabDisplayImage(
                const u_int x,
                const u_int y,
                const u_int w,
                const u_int h ) const
{
    auto par = image::Params();
    par.width       = (int)w;
    par.height      = (int)h;
    par.rowPitch    = (((int)w * 3) + 3) & ~3;
    par.depth       = 24;
    par.chans       = 3;
    image img( par );
    img.Clear();

#ifdef ENABLE_OPENGL
    glReadPixels(
        (int)x,
        (int)y,
        img.mW,
        img.mH,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        (void *)img.GetPixelPtr(0,0) );
#endif

    return img;
}

