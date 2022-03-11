//==================================================================
/// ImageSystemOCIO.h
///
/// Created by Davide Pasca - 2022/03/01
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGESYSTEMOCIO_H
#define IMAGESYSTEMOCIO_H

#ifdef ENABLE_OCIO

class image;

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

//==================================================================
class ImageSystemOCIO
{
    OCIO::ConfigRcPtr       msDefaultCfg;
    DStr                    mUseCfgFName;
    OCIO::ConstConfigRcPtr  msUseCfg;

    DVec<DStr>                          mDispNames;
    std::unordered_map<DStr,DVec<DStr>> mViewNames;
    DVec<DStr>                          mLookNames;

public:
    ImageSystemOCIO();

    void ApplyOCIO(
        const image &srcImg,
        const DStr &cfgFName,
        const DStr &dispName,
        const DStr &viewName,
        const DStr &lookName );

    void UpdateConfigOCIO( const DStr &cfgFName );

    const DVec<DStr> &GetDisps() const { return mDispNames; }
    const DVec<DStr> &GetViews( const DStr &disp ) const;
    const DVec<DStr> &GetLooks() const { return mLookNames; }

    bool HasView( const DStr &disp, const DStr &view ) const;
    const char *GetDefView( const DStr &disp ) const;
};

#endif

#endif

