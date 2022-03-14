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
        const DStr &dispName,
        const DStr &viewName,
        const DStr &lookName )
{
    auto getName = []( c_auto &str ) -> const char*
    {
        return str.empty() ? nullptr : str == "None" ? "" : str.c_str();
    };

    c_auto *pDisp = msUseCfg->getDefaultDisplay();
    c_auto *pView = msUseCfg->getDefaultView( pDisp );
    c_auto *pLook = msUseCfg->getDisplayViewLooks( pDisp, pView );

    if (c_auto *p = getName( dispName )) pDisp = p;
    if (c_auto *p = getName( viewName )) pView = p;
    if (c_auto *p = getName( lookName )) pLook = p;

    try
    {
        // see if we have to load a new config
        UpdateConfigOCIO( cfgFName );

        OCIO::PackedImageDesc ocioImg(
                (void *)srcImg.GetPixelPtr(0,0),
                srcImg.mW,
                srcImg.mH,
                3 );

        auto dvt = OCIO::DisplayViewTransform::Create();
        dvt->setSrc( OCIO::ROLE_SCENE_LINEAR );
        dvt->setDisplay( pDisp );
        dvt->setView( pView );

        auto lvp = OCIO::LegacyViewingPipeline::Create();
        lvp->setDisplayViewTransform( dvt );
        lvp->setLooksOverrideEnabled( true );
        lvp->setLooksOverride( pLook );

        auto proc = lvp->getProcessor( msUseCfg, msUseCfg->getCurrentContext() );

        proc->getDefaultCPUProcessor()->apply( ocioImg );
    }
    catch ( OCIO::Exception &ec )
    {
        LogOut( LOG_ERR, "OpenColorIO Error: %s, while applying"
                         " disp:%s, view:%s, look:%s",
                         ec.what(),
                         pDisp, pView, pLook );
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

    // collect the names of displays
    mDispNames.clear();
    for (int i=0; i < msUseCfg->getNumDisplays(); ++i)
        mDispNames.push_back( msUseCfg->getDisplay( i ) );

    // collect the names of color spaces
    mViewNames.clear();
    for (c_auto &disp : mDispNames)
    {
        c_auto *pDisp = disp.c_str();
        for (int i=0; i < msUseCfg->getNumViews( pDisp ); ++i)
            mViewNames[disp].push_back( msUseCfg->getView( pDisp, i ) );
    }

    // collect the names of the looks
    mLookNames.clear();
    for (int i=0; i < msUseCfg->getNumLooks(); ++i)
        mLookNames.push_back( msUseCfg->getLookNameByIndex( i ) );
}

//==================================================================
const DVec<DStr> &ImageSystemOCIO::GetViews( const DStr &disp ) const
{
    if (auto it = mViewNames.find( disp ); it != mViewNames.end())
        return it->second;

    static DVec<DStr> sDummy;
    return sDummy;
}

//==================================================================
bool ImageSystemOCIO::HasView( const DStr &disp, const DStr &view ) const
{
    auto itView = mViewNames.find( disp );
    if ( itView == mViewNames.end() )
        return false;

    c_auto &v = itView->second;

    return std::find( v.begin(), v.end(), view ) != v.end();
}

//==================================================================
const char *ImageSystemOCIO::GetDefView( const DStr &disp ) const
{
    return msUseCfg->getDefaultView( disp.c_str() );
}

#endif

