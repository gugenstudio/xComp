//==================================================================
/// GWindowScoped.h
///
/// Created by Davide Pasca - 2018/02/15
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GWINDOWSCOPED_H
#define GWINDOWSCOPED_H

#include "DMatrix44.h"

class Graphics;

//==================================================================
class GWindowScoped
{
    Graphics &mG;
    const Matrix44D  mOldXForm;

    const std::pair<bool,Float4> mOldScissor;

    const Double4    mPhyRC;
    const Double4    mLogRC;

public:
    GWindowScoped(
            Graphics &g,
            const Double4 &phyRC,
            const Double4 &logRC );

    ~GWindowScoped();

          Graphics &GetGfx()       { return mG; }
    const Graphics &GetGfx() const { return mG; }

    const Double4 &GetLogRC() const { return mLogRC; }

    bool IsPhysicalPointInside( const Double2 &pos ) const;
};

#endif

