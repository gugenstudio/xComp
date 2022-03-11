//==================================================================
/// ImageSystem.h
///
/// Created by Davide Pasca - 2022/01/18
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMAGESYSTEM_H
#define IMAGESYSTEM_H

#include <map>
#include "Image.h"
#include "Image_EXR.h"

#ifdef ENABLE_OCIO
class ImageSystemOCIO;
#endif

class SerialJS;
class DeserialJS;

//==================================================================
struct ImageEntry
{
    friend class ImageSystem;

    DStr            mImagePathFName;
    bool            mIsImageEnabled { true };
    DStr            mCurLayerForImage;
    uptr<image>     moStdImage;
#ifdef ENABLE_OPENEXR
    uptr<ImageEXR>  moEXRImage;
#endif
private:
    uptr<image>     moStdImageScaled;
public:
    ImageEntry() {}
    ImageEntry( const DStr &pathFName );

private:
    void loadStdImage();
    void loadEXRImage();
};

//==================================================================
class IMSConfig
{
public:
    bool    imsc_useBilinear        { true };
    bool    imsc_ccorRGBOnly        { true };
    bool    imsc_ccorSRGB           { true };
    DStr    imsc_ccorXform          { "none" };
    DStr    imsc_ccorOCIOCfgFName   {};
    DStr    imsc_ccorOCIOCSpace     {};

    friend bool operator==(const IMSConfig &l, const IMSConfig &r)
    {
        return
            l.imsc_useBilinear      == r.imsc_useBilinear       &&
            l.imsc_ccorRGBOnly      == r.imsc_ccorRGBOnly       &&
            l.imsc_ccorSRGB         == r.imsc_ccorSRGB          &&
            l.imsc_ccorXform        == r.imsc_ccorXform         &&
            l.imsc_ccorOCIOCfgFName == r.imsc_ccorOCIOCfgFName  &&
            l.imsc_ccorOCIOCSpace   == r.imsc_ccorOCIOCSpace    &&
            true;
    }

    friend bool operator!=(const IMSConfig &l, const IMSConfig &r) { return !(l == r); }

    void Serialize( SerialJS &v_ ) const;
    void Deserialize( DeserialJS &v_ );

    friend void Serialize( SerialJS &v_, const IMSConfig &o_ ){ o_.Serialize(v_);   }
    friend void Deserialize( DeserialJS &v_,   IMSConfig &o_ ){ o_.Deserialize(v_); }
};

//==================================================================
class ImageSystem
{
public:
    inline static DStr          DUMMY_LAYER_NAME { "__default__" };
    std::map<DStr,ImageEntry>   mEntries;
    uptr<image>                 moComposite;
    DStr                        mCurSelPathFName;
    IMSConfig                   mIMSCfg;
#ifdef ENABLE_OCIO
    uptr<ImageSystemOCIO>       moIS_OCIO;
#endif

    DStr                        mCurLayerName;

    bool                        mHasRebuildReq = false;

private:
    std::unordered_set<DStr>    mNotifiedBadPaths;

public:
    ImageSystem( const IMSConfig &initCfg={} );
    ~ImageSystem();

    void OnNewScanDir( const DStr &path, const DStr &selPathFName );

    void SaveComposite( const DStr &path ) const;
    bool IncCurSel( int step );
    void SetFirstCurSel();
    void SetLastCurSel();

    void ReqRebuildComposite();
    void ReqRebuildComposite( const IMSConfig &cfg );

    void AnimateIMS();

    bool IsRebuildingComposite() const;

private:
    void makeDummyComposite();
    void rebuildComposite();
    void makeComposite( DVec<ImageEntry *> pEntries, size_t n );
};


#endif

