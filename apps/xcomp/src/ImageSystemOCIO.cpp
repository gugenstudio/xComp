//==================================================================
/// ImageSystemOCIO.cpp
///
/// Created by Davide Pasca - 2022/03/07
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OCIO

#include "DLogOut.h"
#include "FileUtils.h"
#include "image.h"
#include "ImageSystemOCIO.h"

//==================================================================
ImageSystemOCIO::ImageSystemOCIO()
{
    msDefaultCfg = OCIO::Config::Create();
    msUseCfg = msDefaultCfg;
}

//==================================================================
void ImageSystemOCIO::ApplyOCIO(
        const image &srcImg,
        const DStr &cfgFName,
        const DStr &cspaceName )
{
    try
    {
        // see if we have to load a new config
        UpdateConfigOCIO( cfgFName );

        c_auto *pSrcCS = OCIO::ROLE_SCENE_LINEAR;
        c_auto *pDesCS = cspaceName.empty()
                            ? OCIO::ROLE_SCENE_LINEAR
                            : cspaceName.c_str();

        //auto config = OCIO::GetCurrentConfig();
        auto processor = msUseCfg->getProcessor( pSrcCS, pDesCS );

        auto cpu = processor->getDefaultCPUProcessor();

        OCIO::PackedImageDesc ocioImg(
                (void *)srcImg.GetPixelPtr(0,0),
                srcImg.mW,
                srcImg.mH,
                3 );

        cpu->apply( ocioImg );
    }
    catch ( OCIO::Exception &ec )
    {
        LogOut( LOG_ERR, "OpenColorIO Error: %s", ec.what() );
    }
}

//==================================================================
void ImageSystemOCIO::UpdateConfigOCIO( const DStr &cfgFName )
{
    if ( cfgFName == mUseCfgFName )
        return;

    mUseCfgFName = {};
    msUseCfg = {};

    mUseCfgFName = cfgFName;

    if NOT( mUseCfgFName.empty() )
    {
        if ( FU_FileExists( mUseCfgFName ) )
        {
            msUseCfg = OCIO::Config::CreateFromFile( mUseCfgFName.c_str() );
            mUseCfgFName = mUseCfgFName;
        }
        else
        {
            LogOut( LOG_ERR, "Could not find %s", mUseCfgFName.c_str() );
            // settle for the default
            msUseCfg = msDefaultCfg;
        }
    }
    else
    {
        // settle for the default
        msUseCfg = msDefaultCfg;
    }

    // collect the names of color spaces
    mColSpaceNames.clear();
    const int csN = msUseCfg->getNumColorSpaces();
    for (int i=0; i < csN; ++i)
        mColSpaceNames.push_back( msUseCfg->getColorSpaceNameByIndex( i ) );
}

#endif

