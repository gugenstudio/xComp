//==================================================================
/// DMT_Debug.h
///
/// Created by Davide Pasca - 2015/8/9
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMT_DEBUG_H
#define DMT_DEBUG_H

#include "DMatrix33.h"
#include "DMatrix44.h"
#include "DQuat.h"

//==================================================================
namespace DMTD
{

void PrintNiceMatrix( const Matrix33 &m );
void PrintNiceMatrix( const Matrix44 &m );
void PrintNiceBool( bool val, const char *pTitle=nullptr );
void PrintNiceNormal( const Float3 &n, const char *pTitle=nullptr );
void PrintNiceFloat( const float n, const char *pTitle=nullptr, int places=6, int dec=3);
void PrintNiceVec2(const Float2 &n, const char *pTitle=nullptr, int places=6, int dec=3);
void PrintNiceVec3(const Float3 &n, const char *pTitle=nullptr, int places=6, int dec=3);
void PrintNiceVec4(const Float4 &n, const char *pTitle=nullptr, int places=6, int dec=3);
void PrintNiceQuat( const Quat &n, const char *pTitle=nullptr );
void PrintSep();
void PrintNL();

//==================================================================
}

#endif

