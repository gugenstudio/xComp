//==================================================================
/// GWindowScoped.cpp
///
/// Created by Davide Pasca - 2018/02/15
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "Graphics.h"
#include "GWindowScoped.h"

//==================================================================
GWindowScoped::GWindowScoped(
        Graphics &g,
        const Double4 &phyRC,
        const Double4 &logRC )
    : mG(g)
    , mPhyRC(phyRC)
    , mLogRC(logRC)
    , mOldXForm(g.GetXForm())
    , mOldScissor(g.GetScissor())
{
    const auto sx = (float)(phyRC[2] / (logRC[2] ? logRC[2] : 1.0));
    const auto sy = (float)(phyRC[3] / (logRC[3] ? logRC[3] : 1.0));

    const auto xform =
        Matrix44D::Translate({ -logRC[0], -logRC[1], 0 }) *
        Matrix44D::Scale({ sx, sy, 1 }) *
        Matrix44D::Translate({  phyRC[0],  phyRC[1], 0 });

    mG.SetXForm( xform );

    {
        c_auto vpSiz = mG.GetVPSize();

        c_auto x = (float)(phyRC[0] * vpSiz[0]);
        c_auto y = (float)(phyRC[1] * vpSiz[1]);
        c_auto w = (float)(phyRC[2] * vpSiz[0]);
        c_auto h = (float)(phyRC[3] * vpSiz[1]);

        mG.SetScissor( true, {x,y,w,h} );
    }
}

//==================================================================
GWindowScoped::~GWindowScoped()
{
    mG.SetXForm( mOldXForm );

    mG.SetScissor( mOldScissor.first, mOldScissor.second );
}

//==================================================================
bool GWindowScoped::IsPhysicalPointInside( const Double2 &pos ) const
{
    return
        pos[0] >= mPhyRC[0] &&
        pos[1] >= mPhyRC[1] &&
        pos[0] <= (mPhyRC[0]+mPhyRC[2]) &&
        pos[1] <= (mPhyRC[1]+mPhyRC[3]);
}

