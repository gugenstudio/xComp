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

public:
    ImageSystemOCIO();

    void ApplyOCIO( const image &srcImg, const DStr &cfgFName );

private:
    void updateConfigOCIO( const DStr &cfgFName );
};

#endif

#endif

