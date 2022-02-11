//==================================================================
/// GShaderProg.h
///
/// Created by Davide Pasca - 2021/05/30
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GSHADERPROG_H
#define GSHADERPROG_H

#include "DBase.h"

//==================================================================
class GShaderProg
{
public:
    u_int   mShaderVertex   {};
    u_int   mShaderFragment {};
    u_int   mShaderProgram  {};

    u_int   mTexLoc {};

    GShaderProg( bool useTex );
    ~GShaderProg();

    c_auto GetProgramID() const { return mShaderProgram; }
};


#endif

