//==================================================================
/// DGLUtils.h
///
/// Created by Davide Pasca - 2011/6/25
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DGLUTILS_H
#define DGLUTILS_H

#include "DBase.h"

#ifdef ENABLE_OPENGL
static const unsigned int INVALID_OGL_OBJ_ID = (unsigned int)-1;

//==================================================================
namespace DGLUT
{

bool MasterCheckGLErr( const char *pFileName, int line );
bool CheckGLErr( const char *pFileName, int line, bool doPrint=true );
void FlushGLErr();

const char *GetFBStatusStr( u_int status );

bool SetupErrorIntercept();

}

//==================================================================
#if defined(DEBUG) || defined(_DEBUG)
# define MASTERCHECKGLERR DGLUT::MasterCheckGLErr(__FILE__,__LINE__)
# define CHECKGLERR DGLUT::CheckGLErr(__FILE__,__LINE__)
# define FLUSHGLERR DGLUT::FlushGLErr()
#else
# define MASTERCHECKGLERR
# define CHECKGLERR
# define FLUSHGLERR
#endif

#endif

#endif

