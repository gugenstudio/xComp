//==================================================================
/// DNETUT.h
///
/// Created by Davide Pasca - 2015/7/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNETUT_H
#define DNETUT_H

#include "DBase.h"

//==================================================================
namespace DNETUT
{

bool GetPortNumber( const char *pStr, u_short &out_val );

std::pair<DStr,DStr> SplitURIBaseAndQuery( const DStr &uriFull );

}

#endif

