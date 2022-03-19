//==================================================================
/// XCConfig.h
///
/// Created by Davide Pasca - 2022/01/21
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef XCCONFIG_H
#define XCCONFIG_H

#include "ImageSystem.h"
#include "SerializeJS.h"

//==================================================================
class XCConfig
{
public:
    DStr                cfg_savedVer            ;
    DStr                cfg_profileBaseDir      ;

    DStr                cfg_scanDir             {};
    DVec<DStr>          cfg_scanDirHist         {};
    DStr                cfg_saveDir             {};
    DStr                cfg_ctrlPanButton       { "left" };
    bool                cfg_dispAutoFit         { true };

    IMSConfig           cfg_imsConfig           {};

public:
#if 0
    void CopyConfigVals( const XCConfig &from )
    {
        cfg_scanDir            =    from.cfg_scanDir            ;
        cfg_scanDirHist        =    from.cfg_scanDirHist        ;
        cfg_saveDir            =    from.cfg_saveDir            ;
        cfg_ctrlPanButton      =    from.cfg_ctrlPanButton      ;
        cfg_dispAutoFit        =    from.cfg_dispAutoFit        ;
        cfg_imsConfig          =    from.cfg_imsConfig          ;
    }
#endif
    static bool CheckValsChange( const XCConfig &l, const XCConfig &r )
    {
        return
            l.cfg_scanDir            !=   r.cfg_scanDir            ||
            l.cfg_scanDirHist        !=   r.cfg_scanDirHist        ||
            l.cfg_saveDir            !=   r.cfg_saveDir            ||
            l.cfg_ctrlPanButton      !=   r.cfg_ctrlPanButton      ||
            l.cfg_dispAutoFit        !=   r.cfg_dispAutoFit        ||
            l.cfg_imsConfig          !=   r.cfg_imsConfig          ||
            false;
    }

    bool AddScanDirToHistory();

    void Serialize( SerialJS &v_ ) const;
    void Deserialize( DeserialJS &v_ );

    friend void Serialize( SerialJS &v_, const XCConfig &o_ ){ o_.Serialize(v_);   }
    friend void Deserialize( DeserialJS &v_,   XCConfig &o_ ){ o_.Deserialize(v_); }
};

#endif

