//==================================================================
/// Image_DLS_LOS.h
///
/// Created by Davide Pasca - 2012/5/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_DLS_LOS_H
#define IMAGE_DLS_LOS_H

#include "DUT_MemFile.h"

//==================================================================
namespace ImgDLS
{

class ImgDLS_Head1;

static const int MAX_SCALE_POW2_DIV = 3;

void LOS_LoadDLS(
        image &img,
        DUT::MemReader &mr,
        const ImgDLS_Head1 &head1,
        int scalePow2Div=-1 );

void LOS_SaveDLS( const image &img, DUT::MemWriterDynamic &mw, u_int quality );

//==================================================================
}

#endif
