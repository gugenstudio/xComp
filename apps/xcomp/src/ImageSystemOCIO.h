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

    DVec<DStr>              mColSpaceNames;

public:
    ImageSystemOCIO();

    void ApplyOCIO(
        const image &srcImg,
        const DStr &cfgFName,
        const DStr &cspaceName );

    void UpdateConfigOCIO( const DStr &cfgFName );

    c_auto &GetColorSpaces() const { return mColSpaceNames; }
};

#endif

#endif

