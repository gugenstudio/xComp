//==================================================================
/// Graphics.h
///
/// Created by Davide Pasca - 2018/02/14
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "DBase.h"
#include "DContainers.h"
#include "DVector.h"
#include "DMatrix44.h"
#include "ColorF.h"
#include "Image.h"
#include "GWindowScoped.h"

class GShaderProg;

//==================================================================
class Graphics
{
    struct VtxPC
    {
        Float3  pos;
        ColorF  col;
    };
    struct VtxPCT
    {
        Float3  pos;
        ColorF  col;
        Float2  tc0;
    };
    DVec<VtxPC>     mVtxPC;
    DVec<VtxPCT>    mVtxPCT;

    enum
    {
        BM_NONE,
        BM_ADD,
        BM_ALPHA,
    };

    int             mCurBlendMode = BM_NONE;

    enum : u_int
    {
        FLG_LINES = 1 << 0,
        FLG_TEX   = 1 << 1,
    };
    u_int           mModeFlags = 0;

    u_int           mCurTexID = 0;

    Float2          mVP_Siz {0,0};
    bool            mCurScissOnOff = false;
    Float4          mCurScissRC {0,0,0,0};

    Matrix44D       mCurXForm {true};
    float           mCurColSca {1};

    ColorF          mBGCol {0,0,0};

    Double2         mCurPixSize {1,1};

    bool            mUseShaders {};

    DVec<uptr<GShaderProg>>   moShaProgs;

    u_int           mCurShaderProgram {};

    u_int           mVAO = 0;

    u_int           mVBO = 0;
    size_t          mLastVBOSize {};

    double          mContentSca = 1;

public:
    Graphics( int majorVer, bool isSWRendering );
    ~Graphics();

    void SetConentScale( float sca ) { mContentSca = sca; }
    void SetVPSize( float w, float h ) { mVP_Siz = {w, h}; updateCurPixSize(); }
    Float2 GetVPSize() const { return mVP_Siz; }

    void SetScissor( bool onOff, const Float4 &rc={0,0,0,0} );
    void SetScissorXForm( bool onOff, double x1, double y1, double x2, double y2 );

    std::pair<bool,Float4> GetScissor() const
    {
        return {mCurScissOnOff, mCurScissRC};
    }

    bool IsPointInScissor( const Double2 &pos ) const;

    const Matrix44D &GetXForm() const { return mCurXForm; }

    void SetXForm( const Matrix44D &xform );

    const Double2 &GetCurPixSize() const { return mCurPixSize; }

    static Double2 CalcLogPosFromPhy( const Matrix44D &xform, const Double2 &phyPos )
    {
        const auto xf = xform.GetAffineInverse();
        const auto p3 = DXFormPos( Double3( phyPos[0], phyPos[1], 0.0 ), xf );
        return { p3[0], p3[1] };
    }

    Double2 CalcLogPosFromPhy( const Double2 &phyPos ) const
    {
        return CalcLogPosFromPhy( GetXForm(), phyPos );
    }

    void SetColorSca( float sca ) { mCurColSca = sca; }

    void SetBGColor( const ColorF &bgCol ) { mBGCol = bgCol; }
    ColorF GetBGColor() const { return mBGCol; }

    void SetBlendNone();
    void SetBlendAdd();
    void SetBlendAlpha();

    static void UploadImageTexture( image &img );

    void SetTextureFromImage( const image &img );
    void SetTexture( u_int texID );
    void SetNoTexture() { SetTexture( 0 ); }

    float GetRatioWoH() const { return mVP_Siz[0] / mVP_Siz[1]; }

    void DrawLine( const Double2 &p1, const Double2 &p2, const ColorF &col );
    void DrawLine( const Double2 &p1, const Double2 &p2, const ColorF &col1, const ColorF &col2 );
    void DrawLine( double x1, double y1, double x2, double y2, const ColorF &col )
    {
        DrawLine( {x1,y1}, {x2,y2}, col );
    }

    DVec<VtxPC> &BeginRawLines();

    void DrawRectFill( const Double2 &pos, const Double2 &siz, const std::array<ColorF,4> &cols );
    void DrawRectFill( const Double2 &pos, const Double2 &siz, const ColorF &col );
    void DrawRectFill( double x, double y, double w, double h, const ColorF &col )
    {
        DrawRectFill( {x,y}, {w,h}, col );
    }
    void DrawRectFill( const Double4 &rc, const ColorF &col )
    {
        DrawRectFill( {rc[0],rc[1]}, {rc[2],rc[3]}, col );
    }

    void DrawRectFrame( const Double2 &pos, const Double2 &siz, const ColorF &col );
    void DrawRectFrame( double x, double y, double w, double h, const ColorF &col )
    {
        DrawRectFrame( {x,y}, {w,h}, col );
    }
    void DrawRectFrame( const Double4 &rc, const ColorF &col )
    {
        DrawRectFrame( {rc[0],rc[1]}, {rc[2],rc[3]}, col );
    }

    void DrawRectTex(
            const Double2 &pos,
            const Double2 &siz,
            const ColorF &col,
            const Float4 &txcBox );

    void DrawQuad( const std::array<Double2,4> &poses, const ColorF &col );
    void DrawTri( const std::array<Double2,3> &poses, const ColorF &col );

    void DrawThickLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col,
        const double thick );

    void ResetStates();
    void FlushPrims();

    image GrabDisplayImage(
                const u_int x=0,
                const u_int y=0,
                const u_int w=0,
                const u_int h=0 ) const;

    Float3 MakeXformedP( double x, double y ) const { return DXFormPos( Double3( x, y, 0.0 ), mCurXForm ); }
    Float3 MakeXformedP( const Double2 &pos ) const { return DXFormPos( Double3( pos, 0.0 ), mCurXForm ); }

    ColorF MakeXformedC( const ColorF &col ) const { return col * mCurColSca; }

private:
    inline void updateCurPixSize()
    {
        mCurPixSize = {mContentSca / (mCurXForm.mij(0,0) * (double)mVP_Siz[0]),
                       mContentSca / (mCurXForm.mij(1,1) * (double)mVP_Siz[1]) };
    }

    std::array<Float3,4> makeRectVtxPos(
                                const Double2 &pos,
                                const Double2 &siz ) const;

    void switchModeFlags( u_int flags );

    //==================================================================
    template <typename D, typename S>
    static void setQuadStripAsTrigsP(
            D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
    {
        out_des[0].pos = v0;
        out_des[1].pos = v1;
        out_des[2].pos = v2;
        out_des[3].pos = v3;
        out_des[4].pos = v2;
        out_des[5].pos = v1;
    }
    template <typename D, typename S>
    static void setQuadStripAsTrigsC(
            D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
    {
        out_des[0].col = v0;
        out_des[1].col = v1;
        out_des[2].col = v2;
        out_des[3].col = v3;
        out_des[4].col = v2;
        out_des[5].col = v1;
    }
    template <typename D, typename S>
    static void setQuadStripAsTrigsT(
            D out_des[6], const S &v0, const S &v1, const S &v2, const S &v3 )
    {
        out_des[0].tc0 = v0;
        out_des[1].tc0 = v1;
        out_des[2].tc0 = v2;
        out_des[3].tc0 = v3;
        out_des[4].tc0 = v2;
        out_des[5].tc0 = v1;
    }
};

//==================================================================
// Inlined
//==================================================================
inline void Graphics::DrawTri( const std::array<Double2,3> &vps, const ColorF &col )
{
    switchModeFlags( 0 );

    auto *pVtx = Dgrow( mVtxPC, 3 );
    pVtx[0].pos = MakeXformedP( vps[0] );
    pVtx[1].pos = MakeXformedP( vps[1] );
    pVtx[2].pos = MakeXformedP( vps[2] );
    pVtx[0].col = col;
    pVtx[1].col = col;
    pVtx[2].col = col;
}

//==================================================================
inline void Graphics::DrawQuad( const std::array<Double2,4> &vps, const ColorF &col )
{
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
}

//==================================================================
inline void Graphics::DrawLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col )
{
    switchModeFlags( FLG_LINES );

    auto *pVtx = Dgrow( mVtxPC, 2 );
    pVtx[0].pos = MakeXformedP( p1 );
    pVtx[1].pos = MakeXformedP( p2 );

    pVtx[0].col =
    pVtx[1].col = MakeXformedC( col );
}


//==================================================================
inline void Graphics::DrawLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col1,
        const ColorF &col2 )
{
    switchModeFlags( FLG_LINES );

    auto *pVtx = Dgrow( mVtxPC, 2 );
    pVtx[0].pos = MakeXformedP( p1 );
    pVtx[1].pos = MakeXformedP( p2 );

    pVtx[0].col = MakeXformedC( col1 );
    pVtx[1].col = MakeXformedC( col2 );
}

//==================================================================
inline void Graphics::DrawThickLine(
        const Double2 &p1,
        const Double2 &p2,
        const ColorF &col,
        const double thick )
{
    switchModeFlags( 0 );

    // high precision akternative to DNormalize()
    auto norm = []( const Double2 &v )
    {
        return v / sqrt( v[0]*v[0] + v[1]*v[1] );
    };

    c_auto dir = norm( p2 - p1 );
    c_auto u = Double2( dir[1], -dir[0] );

    c_auto dirh = dir * 0.5;

    c_auto &pixSiz = GetCurPixSize();
    c_auto sca = pixSiz * thick;

    c_auto off0 = (dirh - u) * sca[0];
    c_auto off1 = (dirh + u) * sca[1];

    c_auto v0t = MakeXformedP( p1 - off1 );
    c_auto v1t = MakeXformedP( p1 - off0 );
    c_auto v2t = MakeXformedP( p2 + off0 );
    c_auto v3t = MakeXformedP( p2 + off1 );

    auto *pVtx = Dgrow( mVtxPC, 6 );

    setQuadStripAsTrigsP( pVtx, v0t, v1t, v2t, v3t );

    pVtx[0].col =
    pVtx[1].col =
    pVtx[2].col =
    pVtx[3].col =
    pVtx[4].col =
    pVtx[5].col = MakeXformedC( col );
}

#endif

