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
    DStr                cfg_saveDir             {};
    DStr                cfg_ctrlPanButton       { "left" };
    bool                cfg_dispAutoFit         { true };

    IMSConfig           cfg_imsConfig           {};

public:
    void CopyConfigVals( const XCConfig &from )
    {
        cfg_scanDir            =    from.cfg_scanDir            ;
        cfg_saveDir            =    from.cfg_saveDir            ;
        cfg_ctrlPanButton      =    from.cfg_ctrlPanButton      ;
        cfg_dispAutoFit        =    from.cfg_dispAutoFit        ;
        cfg_imsConfig          =    from.cfg_imsConfig          ;
    }

    static bool CheckValsChange( const XCConfig &l, const XCConfig &r )
    {
        return
            l.cfg_scanDir            !=   r.cfg_scanDir            ||
            l.cfg_saveDir            !=   r.cfg_saveDir            ||
            l.cfg_ctrlPanButton      !=   r.cfg_ctrlPanButton      ||
            l.cfg_dispAutoFit        !=   r.cfg_dispAutoFit        ||
            l.cfg_imsConfig          !=   r.cfg_imsConfig          ||
            false;
    }

    void Serialize( SerialJS &v_ ) const;
    void Deserialize( DeserialJS &v_ );

    friend void Serialize( SerialJS &v_, const XCConfig &o_ ){ o_.Serialize(v_);   }
    friend void Deserialize( DeserialJS &v_,   XCConfig &o_ ){ o_.Deserialize(v_); }
};

#endif

