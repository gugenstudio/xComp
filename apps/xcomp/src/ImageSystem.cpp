//==================================================================
/// ImageSystem.cpp
///
/// Created by Davide Pasca - 2022/01/19
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DLogOut.h"
#include "Graphics.h"
#include "TimeUtils.h"
#include "FileUtils.h"
#include "SerializeJS.h"
#include "ImageConv.h"
#include "Image_PNG.h"
#include "Image_EXR.h"
#include "ImageConv.h"
#include "ImageSystemOCIO.h"
#include "ImageSystem.h"

//==================================================================
void IMSConfig::Serialize( SerialJS &v_ ) const
{
    v_.MSerializeObjectStart();
    SERIALIZE_THIS_MEMBER( v_, imsc_useBilinear             );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorRGBOnly             );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorSRGB                );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorXform               );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOCfgFName        );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOCfgFNameHist    );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIODisp            );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOView            );
    SERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOLook            );
    v_.MSerializeObjectEnd();
}

void IMSConfig::Deserialize( DeserialJS &v_ )
{
    DESERIALIZE_THIS_MEMBER( v_, imsc_useBilinear           );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorRGBOnly           );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorSRGB              );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorXform             );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOCfgFName      );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOCfgFNameHist  );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIODisp          );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOView          );
    DESERIALIZE_THIS_MEMBER( v_, imsc_ccorOCIOLook          );

    // remove empty or unreachable files
    auto &h = imsc_ccorOCIOCfgFNameHist;
    for (auto it=h.begin(); it != h.end();)
    {
        if ( it->empty() || !FU_FileExists( *it ) )
            it = h.erase( it );
        else
            ++it;
    }
}

//==================================================================
void IMSConfig::SetOCIOFName( const DStr &fname )
{
    imsc_ccorOCIOCfgFName = fname;
    addOCIOFNameToHistory();
}

//==================================================================
bool IMSConfig::addOCIOFNameToHistory()
{
    auto &v = imsc_ccorOCIOCfgFNameHist;
    if ( !imsc_ccorOCIOCfgFName.empty() &&
         v.end() == std::find( v.begin(), v.end(), imsc_ccorOCIOCfgFName ) )
    {
        // erase the oldest, if there are too many entries
        if ( v.size() >= 10 )
            v.erase( v.begin() );

        // append the new one
        v.push_back( imsc_ccorOCIOCfgFName );
        return true;
    }

    return false;
}

//==================================================================
ImageEntry::ImageEntry( const DStr &pathFName )
    : mImagePathFName(pathFName)
{
    LogOut( 0, "Loading %s", mImagePathFName.c_str() );

    try {
        if ( StrEndsWithI( pathFName, ".exr" ) )
            loadEXRImage();
        else
            loadStdImage();
    } catch (...)
    {
        LogOut( LOG_ERR, "Failed to load %s", mImagePathFName.c_str() );
    }
}

//==================================================================
void ImageEntry::loadStdImage()
{
    image::LoadParams par;
    par.mLP_FName = mImagePathFName;

    // load
    auto oImg = std::make_unique<image>( par );

    // verify that we support the format, or replace with a dummy
    if ( (oImg->mChans != 3 &&
          oImg->mChans != 4) ||
         ((int)oImg->mDepth / (int)oImg->mChans) != 8 )
    {
        LogOut( LOG_ERR, "Unsupported format for %s. Should be RGB 24 or RGBA 32",
                    mImagePathFName.c_str() );

        image::Params newPar;
        newPar.width    = oImg->mW;
        newPar.height   = oImg->mH;
        newPar.depth    = 32;
        newPar.chans    = 4;
        oImg = std::make_unique<image>( newPar );
        oImg->Clear();
    }

    // convert to float
    {
        image::Params newPar;
        newPar.width    = oImg->mW;
        newPar.height   = oImg->mH;
        newPar.depth    = oImg->mChans * sizeof(float) * 8;
        newPar.chans    = oImg->mChans;
        newPar.flags    = oImg->mFlags | image::FLG_IS_FLOAT32;
        moBaseImage = std::make_unique<image>( newPar );

        ImageConv::BlitProcess<uint8_t,float>( *oImg, *moBaseImage,
            [chans=oImg->mChans](const uint8_t *pSrc, float *pDes)
            {
                for (int i=0; i != chans; ++i)
                    pDes[i] = (float)pSrc[i] * (1.f/255);
            });
    }
}

//==================================================================
void ImageEntry::loadEXRImage()
{
#ifdef ENABLE_OPENEXR
    moEXRImage = ImageEXR_Load( mImagePathFName, ImageSystem::DUMMY_LAYER_NAME );
#endif
}

//==================================================================
//==================================================================
ImageSystem::ImageSystem( const IMSConfig &initCfg )
    : mIMSCfg(initCfg)
{
#ifdef ENABLE_OCIO
    moIS_OCIO = std::make_unique<ImageSystemOCIO>();
#endif
}

//
ImageSystem::~ImageSystem() = default;

//==================================================================
void ImageSystem::SaveComposite( const DStr &path ) const
{
    auto pathFName = FU_JPath( path, "xComp_out.png" );

    try {
        LogOut( 0, "Saving %s", pathFName.c_str() );

        if ( moComposite->IsFloat32() )
        {
            image::Params par;
            par.width   = moComposite->mW;
            par.height  = moComposite->mH;
            par.depth   = 24;
            par.chans   = 3;
            image tmp( par );

            for (u_int y=0; y < tmp.mH; ++y)
            {
                c_auto *pSrc = (const float *)moComposite->GetPixelPtr( 0, y );
                  auto *pDes = (uint8_t *)tmp.GetPixelPtr( 0, y );

                for (u_int x=0; x < tmp.mW; ++x)
                {
                    pDes[x*3+0] = (uint8_t)DClamp( pSrc[x*3+0] * 255.f, 0.f, 255.f );
                    pDes[x*3+1] = (uint8_t)DClamp( pSrc[x*3+1] * 255.f, 0.f, 255.f );
                    pDes[x*3+2] = (uint8_t)DClamp( pSrc[x*3+2] * 255.f, 0.f, 255.f );
                }
            }

            Image_PNGSave( tmp, pathFName.c_str(), false );
        }
        else
        {
            Image_PNGSave( *moComposite, pathFName.c_str(), false );
        }
    } catch (...)
    {
        LogOut( LOG_ERR, "Failed to save %s", pathFName.c_str() );
    }
}

//==================================================================
bool ImageSystem::OnNewScanDir( const DStr &path, const DStr &selPathFName )
{
    std::unordered_set<DStr>  newNames;

    // fail silently
    if NOT( FU_DirectoryExists( path ) )
        return false;

    for (auto it  = fs::recursive_directory_iterator( path );
              it != fs::recursive_directory_iterator(); ++it)
    {
        if ( it.depth() > 0 )
            continue;

        c_auto &p = *it;

        if NOT( fs::is_regular_file( p ) )
            continue;

        // skip if it's our own generated file
        if ( StrStartsWithI( p.path().filename().string().c_str(), "mt_compar" ) ||
             StrStartsWithI( p.path().filename().string().c_str(), "xComp" ) )
            continue;

        c_auto pathFName = p.path().string();

        if NOT( StrEndsWithI( pathFName, ".png" ) ||
                StrEndsWithI( pathFName, ".exr" )
              )
        //if NOT( StrEndsWithI( pathFName, ".png" ) ||
        //        StrEndsWithI( pathFName, ".jpg" ) ||
        //        StrEndsWithI( pathFName, ".jpeg" ) )
            continue;

        newNames.insert( pathFName );
    }

    // remove what's not longer here
    std::erase_if( mEntries, [&](c_auto &x) {
        return newNames.find( x.second.mImagePathFName ) == newNames.end(); } );

    c_auto prevN = mEntries.size();

    for (c_auto &nn : newNames)
    {
        if ( mEntries.find( nn ) == mEntries.end() )
            mEntries[nn] = { nn };
    }

    //
    if ( prevN != mEntries.size() )
    {
        // has a specific file name to select ?
        if NOT( selPathFName.empty() )
        {
            // select this
            mCurSelPathFName = selPathFName;

            // enable only what's selected
            for (auto &[k, e] : mEntries)
                e.mIsImageEnabled = (e.mImagePathFName == selPathFName);
        }
        else
        if ( !mEntries.empty() )
        {
            // select the new last image
            auto it = mEntries.end();
            std::advance( it, -1 );
            mCurSelPathFName = it->first;
        }

        ReqRebuildComposite();
        return true;
    }
    else
    if ( !selPathFName.empty() && mCurSelPathFName != selPathFName )
    {
        // select this
        mCurSelPathFName = selPathFName;

        // enable only what's selected
        for (auto &[k, e] : mEntries)
            e.mIsImageEnabled = (e.mImagePathFName == selPathFName);

        ReqRebuildComposite();
        return true;
    }

#if 0
    LogOut( LOG_DBG, "----" );
    for (c_auto &[k, e] : mEntries)
        LogOut( LOG_DBG, "F: %s", k.c_str() );
#endif

    return false;
}

//==================================================================
bool ImageSystem::IncCurSel( int step )
{
    if ( mEntries.empty() || !step )
    {
        mCurSelPathFName = {};
        return false;
    }

    auto it = mEntries.find( mCurSelPathFName );
    if ( it == mEntries.end() )
        return false;

    if ( step < 0 )
    {
        if ( it == mEntries.begin() )
            return false;

        for (--it; ; --it)
        {
            if ( it->second.mIsImageEnabled )
            {
                if ( ++step == 0 )
                {
                    mCurSelPathFName = it->first;
                    ReqRebuildComposite();
                    return true;
                }
            }

            if ( it == mEntries.begin() )
                break;
        }
    }
    else
    {
        for (++it; it != mEntries.end(); ++it)
        {
            if ( it->second.mIsImageEnabled )
            {
                if ( --step == 0 )
                {
                    mCurSelPathFName = it->first;
                    ReqRebuildComposite();
                    return true;
                }
            }
        }
    }

    return false;
}

//==================================================================
void ImageSystem::SetFirstCurSel()
{
    while ( IncCurSel( -1 ) )
    {
    }
    ReqRebuildComposite();
}

//==================================================================
void ImageSystem::SetLastCurSel()
{
    while ( IncCurSel( 1 ) )
    {
    }
    ReqRebuildComposite();
}

//==================================================================
inline auto copyRow = []( auto *pDes, c_auto *pSrc, size_t w, size_t chN )
{
    if ( chN >= 4 )
    {
        for (size_t x=0; x < w; ++x)
        {
            c_auto a = DClamp( pSrc[3], 0.f, 1.f );
            pDes[0] = DLerp( pDes[0], pSrc[0], a );
            pDes[1] = DLerp( pDes[1], pSrc[1], a );
            pDes[2] = DLerp( pDes[2], pSrc[2], a );
            pDes += 3;
            pSrc += chN;
        }
    }
    else
    if ( chN == 3 )
    {
        for (size_t x=0; x < w; ++x)
        {
            pDes[0] = pSrc[0];
            pDes[1] = pSrc[1];
            pDes[2] = pSrc[2];
            pDes += 3;
            pSrc += chN;
        }
    }
    else
    if ( chN == 2 )
    {
        for (size_t x=0; x < w; ++x)
        {
            pDes[0] = pSrc[0];
            pDes[1] = pSrc[1];
            pDes[2] = 0;
            pDes += 3;
            pSrc += 2;
        }
    }
    else
    {
        for (size_t x=0; x < w; ++x)
        {
            pDes[0] = pSrc[0];
            pDes[1] = pSrc[0];
            pDes[2] = pSrc[0];
            pDes += 3;
            pSrc += 1;
        }
    }
};

inline auto copyRowA = []( auto *pDes, c_auto *pSrc, c_auto *pASrc, size_t w, size_t chN )
{
    if ( chN >= 3 )
    {
        for (size_t x=0; x < w; ++x)
        {
            c_auto a = DClamp( pASrc[0], 0.f, 1.f );
            pASrc += 1;
            pDes[0] = DLerp( pDes[0], pSrc[0], a );
            pDes[1] = DLerp( pDes[1], pSrc[1], a );
            pDes[2] = DLerp( pDes[2], pSrc[2], a );
            pDes += 3;
            pSrc += chN;
        }
    }
    else
    if ( chN >= 2 )
    {
        for (size_t x=0; x < w; ++x)
        {
            c_auto a = DClamp( pASrc[0], 0.f, 1.f );
            pASrc += 1;
            pDes[0] = DLerp( pDes[0], pSrc[0], a );
            pDes[1] = DLerp( pDes[1], pSrc[1], a );
            pDes[2] = 0;
            pDes += 3;
            pSrc += 2;
        }
    }
    else
    {
        for (size_t x=0; x < w; ++x)
        {
            c_auto a = DClamp( pASrc[0], 0.f, 1.f );
            pASrc += 1;
            pDes[0] =
            pDes[1] =
            pDes[2] = DLerp( pDes[0], pSrc[0], a );
            pDes += 3;
            pSrc += 1;
        }
    }
};

//==================================================================
static auto applyFilmic = []( const image &img )
{
    // from http://filmicworlds.com/blog/filmic-tonemapping-operators/
    auto uncharted2_tonemap_partial = []( const Float3 &x )
    {
        float A = 0.15f;
        float B = 0.50f;
        float C = 0.10f;
        float D = 0.20f;
        float E = 0.02f;
        float F = 0.30f;
        return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
    };

    auto uncharted2_filmic = [&]( const Float3 &v )
    {
        float exposure_bias = 2.0f;
        Float3 curr = uncharted2_tonemap_partial(v * exposure_bias);

        Float3 W = Float3(11.2f);
        Float3 white_scale = Float3(1.0f) / uncharted2_tonemap_partial(W);
        return curr * white_scale;
    };

    for (u_int y=0; y < img.mH; ++y)
    {
        auto *pData = (Float3 *)img.GetPixelPtr( 0, y );

        for (u_int x=0; x < img.mW; ++x)
        {
            pData[x] = uncharted2_filmic( pData[x] );
        }
    }
};

//==================================================================
static auto applySRGB = []( const image &img )
{
    auto s2lin = []( auto x )
    {
        c_auto a = 0.055f;
        //return x <= 0.04045f
        //            ? x * (1.0f / 12.92f)
        //            : pow((x + a) * (1.0f / (1 + a)), 2.4f);
        return DPow((x + a) * (1.0f / (1 + a)), 2.2f);
    };

    auto lin2s = []( auto x )
    {
        c_auto a = 0.055f;
        //return x <= 0.0031308f
        //            ? x * 12.92f,
        //            : (1 + a) * pow(x, 1 / 2.4f) - a;
        return (1 + a) * DPow(x, 1 / 2.2f) - a;
    };

    for (u_int y=0; y < img.mH; ++y)
    {
        auto *pData = (Float3 *)img.GetPixelPtr( 0, y );

        for (u_int x=0; x < img.mW; ++x)
            pData[x] = lin2s( pData[x] );
    }
};

//==================================================================
void ImageSystem::makeDummyComposite()
{
    image::Params par;
    par.width   = 320;
    par.height  = 240;
    par.chans   = 3;
    par.flags   = 0;
    par.depth = 8 * 3;

    moComposite = std::make_unique<image>( par );
    moComposite->Clear();
}

//==================================================================
void ImageSystem::makeComposite( DVec<ImageEntry *> pEntries, size_t n )
{
    c_auto mainW = pEntries[n-1]->moBaseImage->mW;
    c_auto mainH = pEntries[n-1]->moBaseImage->mH;

    {
        image::Params par;
        par.width   = mainW;
        par.height  = mainH;
        par.chans   = 3;
        par.depth   = 3 * sizeof(float) * 8;
        par.flags   = image::FLG_IS_FLOAT32 |
                        (mIMSCfg.imsc_useBilinear ? image::FLG_USE_BILINEAR : 0);

        moComposite = std::make_unique<image>( par );
        moComposite->Clear();
    }

    for (size_t i=0; i < n; ++i)
    {
        auto &e = *pEntries[i];

        const image *pUseBSrcImg {};
        const image *pUseASrcImg {};

        if ( e.moBaseImage->mW == mainW && e.moBaseImage->mH == mainH )
        {
            pUseBSrcImg = e.moBaseImage.get();
            pUseASrcImg = e.moAlphaImage.get();
        }
        else
        {
            if ( !e.moBaseImageScaled ||
                 e.moBaseImageScaled->mW != mainW ||
                 e.moBaseImageScaled->mH != mainH )
            {
                c_auto &simg = e.moBaseImage;

                image::Params par;
                par.width   = mainW;
                par.height  = mainH;
                par.depth   = simg->mDepth;
                par.chans   = simg->mChans;
                par.flags   = simg->mFlags; // for "float"

                e.moBaseImageScaled = std::make_unique<image>( par );

                ImageConv::BlitStretch(
                    *simg,               0, 0, simg->mW, simg->mH,
                    *e.moBaseImageScaled, 0, 0, mainW,    mainH     );

                if (c_auto &aimg = e.moAlphaImage)
                {
                    par.depth   = aimg->mDepth;
                    par.chans   = aimg->mChans;
                    par.flags   = aimg->mFlags; // for "float"

                    e.moAlphaImageScaled = std::make_unique<image>( par );

                    ImageConv::BlitStretch(
                        *aimg,                 0, 0, aimg->mW, aimg->mH,
                        *e.moAlphaImageScaled, 0, 0, mainW,    mainH     );
                }
            }

            pUseBSrcImg = e.moBaseImageScaled.get();
            pUseASrcImg = e.moAlphaImageScaled.get();
        }

        c_auto srcChansN = (size_t)pUseBSrcImg->mChans;

        for (u_int y=0; y < mainH; ++y)
        {
            c_auto *pSrc = (const float *)pUseBSrcImg->GetPixelPtr( 0, y );
              auto *pDes = (      float *)moComposite->GetPixelPtr( 0, y );

            if ( pUseASrcImg )
            {
                c_auto *pASrc = (const float *)pUseASrcImg->GetPixelPtr( 0, y );
                copyRowA( pDes, pSrc, pASrc, mainW, srcChansN );
            }
            else
            {
                copyRow( pDes, pSrc, mainW, srcChansN );
            }
        }
    }
}

//==================================================================
void ImageSystem::rebuildComposite()
{
    auto doApplyColorCorr = false;

#ifdef ENABLE_OPENEXR
    if NOT( mCurLayerName.empty() )
    {
        bool hasNonRGBAChans = false;
        bool hasEXRImage = false;
        for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
        {
            auto &ie = it->second;

            if ( !ie.moEXRImage || !ie.mIsImageEnabled )
                continue;

            hasEXRImage = true;

            auto *pLayer = ie.moEXRImage->FindLayerByName( mCurLayerName );
            if NOT( pLayer )
                continue;

            // if color correction is for layers with RGBA channels only...
            if ( mIMSCfg.imsc_ccorRGBOnly )
            {
                for (c_auto &ch : pLayer->iel_chans)
                {
                    // if none of the channels is R,G,B or A...
                    if (c_auto name = StrMakeUpper( ch.GetChanNameOnly() );
                                name != "R" &&
                                name != "G" &&
                                name != "B" &&
                                name != "A" )
                    {
                        hasNonRGBAChans = true;
                        break;
                    }
                }
            }

            //
            bool didLoad = false;
            if NOT( pLayer->IsLayerDataLoaded() )
            {
                didLoad = true;
                ImageEXR_LoadLayer( *ie.moEXRImage, mCurLayerName );
            }

            if ( ie.mBaseImageCurLayer != mCurLayerName || didLoad )
            {
                ie.mBaseImageCurLayer = mCurLayerName;
                ie.moBaseImage = ImageEXR_MakeImageFromLayer( *pLayer, *ie.moEXRImage );
            }
        }

        //
        doApplyColorCorr = hasEXRImage && !hasNonRGBAChans;
    }

    if NOT( mCurLayerAlphaName.empty() )
    {
        for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
        {
            auto &ie = it->second;

            if ( !ie.moEXRImage || !ie.mIsImageEnabled )
                continue;

            auto *pLayer = ie.moEXRImage->FindLayerByName( mCurLayerAlphaName );
            if NOT( pLayer )
                continue;

            //
            bool didLoad = false;
            if NOT( pLayer->IsLayerDataLoaded() )
            {
                didLoad = true;
                ImageEXR_LoadLayer( *ie.moEXRImage, mCurLayerAlphaName );
            }

            if ( ie.mAlphaImageCurlayer != mCurLayerAlphaName || didLoad )
            {
                ie.mAlphaImageCurlayer = mCurLayerAlphaName;
                ie.moAlphaImage = ImageEXR_MakeAlphaImageFromLayer( *pLayer, *ie.moEXRImage );
            }
        }
    }
#endif

    DVec<ImageEntry *> pEntries;

    size_t curSelIdx = DNPOS;

    for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
    {
        auto &e = it->second;
        if ( e.moBaseImage && e.mIsImageEnabled )
        {
#ifdef ENABLE_OPENEXR
            if ( (e.moEXRImage && e.moEXRImage->FindLayerByName( mCurLayerName )) ||
                (!e.moEXRImage && mCurLayerName == DUMMY_LAYER_NAME) )
#else
            if ( mCurLayerName == DUMMY_LAYER_NAME )
#endif
            {
                pEntries.push_back( &e );
            }
        }

        if ( mCurSelPathFName == e.mImagePathFName && pEntries.size() )
            curSelIdx = pEntries.size() - 1;
    }

    if ( pEntries.empty() || curSelIdx == DNPOS )
    {
        makeDummyComposite();
        return;
    }

    // make the composite
    makeComposite( pEntries, curSelIdx+1 );

    // apply the color correction, if necessary
    if ( doApplyColorCorr )
    {
        if ( mIMSCfg.imsc_ccorXform == "filmic" )
            applyFilmic( *moComposite );
#ifdef ENABLE_OCIO
        else
        if ( mIMSCfg.imsc_ccorXform == "ocio" )
            moIS_OCIO->ApplyOCIO(
                            *moComposite,
                            mIMSCfg.imsc_ccorOCIOCfgFName,
                            mIMSCfg.imsc_ccorOCIODisp,
                            mIMSCfg.imsc_ccorOCIOView,
                            mIMSCfg.imsc_ccorOCIOLook );
#endif
        // we ignore this sRGB conversion in case of OCIO
        if ( mIMSCfg.imsc_ccorSRGB && mIMSCfg.imsc_ccorXform != "ocio" )
            applySRGB( *moComposite );
    }

    // upload to the texture object
    Graphics::UploadImageTexture( *moComposite );
}

//==================================================================
void ImageSystem::ReqRebuildComposite()
{
    mHasRebuildReq = true;
}

void ImageSystem::ReqRebuildComposite( const IMSConfig &cfg )
{
    mIMSCfg = cfg;
    ReqRebuildComposite();
}

//==================================================================
void ImageSystem::AnimateIMS()
{
    if ( mHasRebuildReq )
    {
        mHasRebuildReq = false;
        rebuildComposite();
    }
}

//==================================================================
bool ImageSystem::IsRebuildingComposite() const
{
    return mHasRebuildReq;
}

