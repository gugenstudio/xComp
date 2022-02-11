//==================================================================
/// Image_DLS_LSS.h
///
/// Created by Davide Pasca - 2012/5/31
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGE_DLS_LSS_H
#define IMAGE_DLS_LSS_H

#include "DUT_MemFile.h"

//==================================================================
namespace ImgDLS
{

class ImgDLS_Head1;

//==================================================================
void LSS_LoadDLS( image &img, DUT::MemReader &mr, const ImgDLS_Head1 &head1 );
void LSS_SaveDLS( const image &img, DUT::MemWriterDynamic &mw );

//==================================================================
}

#endif
