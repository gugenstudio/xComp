//==================================================================
/// @author Davide Pasca - davide@oyatsukai.com
/// @author Copyright(C) 2011 oyatsukai.com. All rights reserved.
/// @since 2011/5/25
/// @file image_DLS.h
//==================================================================

#ifndef IMAGE_DLS_H
#define IMAGE_DLS_H

#include "Image.h"
#include "DUT_MemFile.h"

//==================================================================
namespace ImgDLS
{

//==================================================================
static const U8	ENC_FMT_LOSSLESS1	= 1;
static const U8	ENC_FMT_LOSSY1		= 2;

static const size_t MAX_CHANS = 4;

//==================================================================
#pragma pack(push,1)
class ImgDLS_Head1
{
public:
	U8		depth;
	U8		chans;
	U8		encodeFmt;
	U8		reserved1;
	U16		width;
	U16		height;
	U32		bprow;
	U32		imgFlags;
    U32     chansAvgs[MAX_CHANS];
};
#pragma pack(pop)

//==================================================================
void SaveDLS(
        const image &img,
        DUT::MemWriterDynamic &mw,
        u_int quality=100,
        DStr *pOutHeadStr=nullptr );

void QuickLoadDLS(
        image &img,
        u_int imgFlagsAdd,
        u_int imgFlagsRem,
        const ImgDLS_Head1 &head1 );

void LoadDLS(
        image &img,
        u_int imgFlagsAdd,
        u_int imgFlagsRem,
        u_int scalePow2Div,
        DUT::MemReader &mr,
        ImgDLS_Head1 *pOut_Head );

const size_t MIN_QUICKLOAD_SIZE = 4 + sizeof(ImgDLS_Head1);

//==================================================================
DStr MakeStrFromHead( const ImgDLS_Head1 &head );
bool MakeHeadFromStr( ImgDLS_Head1 &out_head, const DStr &fname );
bool MakeHeadFromFName( ImgDLS_Head1 &out_head, const DStr &fname );

//==================================================================
}

#endif

