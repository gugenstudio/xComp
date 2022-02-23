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
#include "ImageConv.h"
#include "Image_PNG.h"
#include "Image_EXR.h"
#include "ImageSystem.h"

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

    moStdImage = std::make_unique<image>( par );

    if ( (moStdImage->mChans != 3 &&
          moStdImage->mChans != 4) ||
         ((int)moStdImage->mDepth / (int)moStdImage->mChans) != 8 )
    {
        LogOut( LOG_ERR, "Unsupported format for %s. Should be RGB 24 or RGBA 32",
                    mImagePathFName.c_str() );

        image::Params newPar;
        newPar.width    = moStdImage->mW;
        newPar.height   = moStdImage->mH;
        newPar.depth    = 32;
        newPar.chans    = 4;
        moStdImage = std::make_unique<image>( newPar );
        moStdImage->Clear();
    }

    //Graphics::UploadImageTexture( *moStdImage );
}

//==================================================================
void ImageEntry::loadEXRImage()
{
#ifdef ENABLE_OPENEXR
    moEXRImage = ImageEXR_Load( mImagePathFName, ImageSystem::DUMMY_LAYER_NAME );
#endif
}

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
void ImageSystem::OnNewScanDir( const DStr &path, const DStr &selPathFName )
{
    std::unordered_set<DStr>  newNames;

    {
        auto it = mNotifiedBadPaths.find( path );
        if NOT( FU_DirectoryExists( path ) )
        {
            if ( it == mNotifiedBadPaths.end() )
            {
                mNotifiedBadPaths.insert( path );
                LogOut( LOG_WRN, "Folder %s is unreachable", path.c_str() );
            }
            return;
        }
        else
        {
            if ( it != mNotifiedBadPaths.end() )
                mNotifiedBadPaths.erase( it );
        }
    }

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
    }

#if 0
    LogOut( LOG_DBG, "----" );
    for (c_auto &[k, e] : mEntries)
        LogOut( LOG_DBG, "F: %s", k.c_str() );
#endif
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
template <typename T>
static auto copyRow = []( auto *pDes, c_auto *pSrc, size_t w, size_t chN )
{
    if ( chN >= 4 )
    {
        for (size_t x=0; x < w; ++x)
        {
            //if ( pSrc[3] )
            {
                float a;
                if ( std::is_integral<T>::value )
                    a = (float)pSrc[3] * (1.0f / 255);
                else
                    a = DClamp( (float)pSrc[3], 0.f, 1.f );

                pDes[0] = (T)DLerp( (float)pDes[0], (float)pSrc[0], a );
                pDes[1] = (T)DLerp( (float)pDes[1], (float)pSrc[1], a );
                pDes[2] = (T)DLerp( (float)pDes[2], (float)pSrc[2], a );
            }

            pDes += 3;
            pSrc += 4;
        }
    }
    else
    {
        if ( chN >= 3 )
        {
            for (size_t x=0; x < w; ++x)
            {
                pDes[0] = pSrc[0];
                pDes[1] = pSrc[1];
                pDes[2] = pSrc[2];
                pDes += 3;
                pSrc += 3;
            }
        }
        else
        if ( chN >= 2 )
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
    c_auto mainW = pEntries[n-1]->moStdImage->mW;
    c_auto mainH = pEntries[n-1]->moStdImage->mH;

    {
        image::Params par;
        par.width   = mainW;
        par.height  = mainH;
        par.chans   = 3;
        par.flags   = mUseBilinear ? image::FLG_USE_BILINEAR : 0;
        if ( pEntries[n-1]->moStdImage->IsFloat32() )
        {
            par.depth = 32 * 3;
            par.flags = image::FLG_IS_FLOAT32;
        }
        else
        {
            par.depth = 8 * 3;
        }

        moComposite = std::make_unique<image>( par );
        moComposite->Clear();
    }

    for (size_t i=0; i < n; ++i)
    {
        auto &e = *pEntries[i];

        const image *pUseSrcImg {};

        if ( e.moStdImage->mW == mainW && e.moStdImage->mH == mainH )
        {
            pUseSrcImg = e.moStdImage.get();
        }
        else
        {
            if ( !e.moStdImageScaled ||
                 e.moStdImageScaled->mW != mainW ||
                 e.moStdImageScaled->mH != mainH )
            {
                image::Params par;
                par.width   = mainW;
                par.height  = mainH;
                par.depth   = e.moStdImage->mDepth;
                par.chans   = e.moStdImage->mChans;
                par.flags   = e.moStdImage->mFlags; // for "float"

                e.moStdImageScaled = std::make_unique<image>( par );

                ImageConv::BlitStretch(
                    *e.moStdImage,       0, 0, e.moStdImage->mW, e.moStdImage->mH,
                    *e.moStdImageScaled, 0, 0, mainW,            mainH      );
            }

            pUseSrcImg = e.moStdImageScaled.get();
        }

        c_auto srcChansN = pUseSrcImg->mChans;

        if ( moComposite->IsFloat32() )
        {
            if ( pUseSrcImg->IsFloat32() )
            {
                for (u_int y=0; y < mainH; ++y)
                {
                    c_auto *pSrc = (const float *)pUseSrcImg->GetPixelPtr( 0, y );
                      auto *pDes = (float *)moComposite->GetPixelPtr( 0, y );

                    copyRow<float>( pDes, pSrc, mainW, srcChansN );
                }
            }
            else
            {
                c_auto sampsN = mainW * srcChansN;
                DVec<float> tmpRow( sampsN );
                for (u_int y=0; y < mainH; ++y)
                {
                    c_auto *pSrc = (const uint8_t *)pUseSrcImg->GetPixelPtr( 0, y );

                    for (size_t j=0; j < sampsN; ++j)
                        tmpRow[j] = (float)pSrc[j] * (1.f/255);

                    auto *pDes = (float *)moComposite->GetPixelPtr( 0, y );

                    copyRow<float>( pDes, tmpRow.data(), mainW, srcChansN );
                }
            }
        }
        else
        {
            if ( pUseSrcImg->IsFloat32() )
            {
                c_auto sampsN = mainW * srcChansN;
                DVec<uint8_t> tmpRow( sampsN );
                for (u_int y=0; y < mainH; ++y)
                {
                    c_auto *pSrc = (const float *)pUseSrcImg->GetPixelPtr( 0, y );

                    for (size_t j=0; j < sampsN; ++j)
                        tmpRow[j] = (uint8_t)DClamp( pSrc[j] * 255.f, 0.f, 255.f );

                    auto *pDes = (uint8_t *)moComposite->GetPixelPtr( 0, y );

                    copyRow<uint8_t>( pDes, tmpRow.data(), mainW, srcChansN );
                }
            }
            else
            {
                for (u_int y=0; y < mainH; ++y)
                {
                    c_auto *pSrc = (const uint8_t *)pUseSrcImg->GetPixelPtr( 0, y );
                      auto *pDes = (uint8_t *)moComposite->GetPixelPtr( 0, y );

                    copyRow<uint8_t>( pDes, pSrc, mainW, srcChansN );
                }
            }
        }
    }

    if ( mToneMapping == "filmic" && moComposite->IsFloat32() )
    {
        applyFilmic( *moComposite );
    }

    if ( mConvOutToSRGB && moComposite->IsFloat32() )
    {
        applySRGB( *moComposite );
    }
}

//==================================================================
void ImageSystem::rebuildComposite()
{
#ifdef ENABLE_OPENEXR
    if NOT( mCurLayerName.empty() )
    {
        for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
        {
            auto &ie = it->second;

            if ( !ie.moEXRImage || !ie.mIsImageEnabled )
                continue;

            auto *pLayer = ie.moEXRImage->FindLayerByName( mCurLayerName );
            if NOT( pLayer )
                continue;

            bool didLoad = false;
            if NOT( pLayer->IsLayerDataLoaded() )
            {
                didLoad = true;
                ImageEXR_LoadLayer( *ie.moEXRImage, mCurLayerName );
            }

            if ( ie.mCurLayerForImage != mCurLayerName || didLoad )
            {
                ie.mCurLayerForImage = mCurLayerName;
                ie.moStdImage = ImageEXR_MakeImageFromLayer( *pLayer, *ie.moEXRImage );
            }
        }
    }
#endif

    DVec<ImageEntry *> pEntries;

    size_t curSelIdx = DNPOS;

    for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
    {
        auto &e = it->second;
        if ( e.moStdImage && e.mIsImageEnabled )
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

    makeComposite( pEntries, curSelIdx+1 );

    Graphics::UploadImageTexture( *moComposite );
}

//==================================================================
void ImageSystem::ReqRebuildComposite()
{
    mHasRebuildReq = true;
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

