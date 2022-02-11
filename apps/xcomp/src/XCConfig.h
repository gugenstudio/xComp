//==================================================================
/// XCConfig.h
///
/// Created by Davide Pasca - 2022/01/21
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef XCCONFIG_H
#define XCCONFIG_H

#include "SerializeJS.h"

//==================================================================
class XCConfig
{
public:
    DStr                cfg_savedVer            ;
    DStr                cfg_profileBaseDir      ;

    DStr                cfg_scanDir             {};
    DStr                cfg_saveDir             {};
    bool                cfg_dispUseBilinear     { true };
    bool                cfg_dispConvToSRGB      { true };
    DStr                cfg_dispToneMapping     { "filmic" };
    bool                cfg_dispAutoFit         { true };
    DStr                cfg_ctrlPanButton       { "left" };

public:
    void CopyConfigVals( const XCConfig &from )
    {
        cfg_scanDir            =    from.cfg_scanDir            ;
        cfg_saveDir            =    from.cfg_saveDir            ;
        cfg_dispUseBilinear    =    from.cfg_dispUseBilinear    ;
        cfg_dispConvToSRGB     =    from.cfg_dispConvToSRGB     ;
        cfg_dispToneMapping    =    from.cfg_dispToneMapping    ;
        cfg_dispAutoFit        =    from.cfg_dispAutoFit        ;
        cfg_ctrlPanButton      =    from.cfg_ctrlPanButton      ;
    }

    static bool CheckValsChange( const XCConfig &l, const XCConfig &r )
    {
        return
            l.cfg_scanDir            !=   r.cfg_scanDir            ||
            l.cfg_saveDir            !=   r.cfg_saveDir            ||
            l.cfg_dispUseBilinear    !=   r.cfg_dispUseBilinear    ||
            l.cfg_dispConvToSRGB     !=   r.cfg_dispConvToSRGB     ||
            l.cfg_dispToneMapping    !=   r.cfg_dispToneMapping    ||
            l.cfg_dispAutoFit        !=   r.cfg_dispAutoFit        ||
            l.cfg_ctrlPanButton      !=   r.cfg_ctrlPanButton      ||
            true;
    }

    void Serialize( SerialJS &v_ ) const;
    void Deserialize( DeserialJS &v_ );

    friend void Serialize( SerialJS &v_, const XCConfig &o_ ){ o_.Serialize(v_);   }
    friend void Deserialize( DeserialJS &v_,   XCConfig &o_ ){ o_.Deserialize(v_); }
};

#endif

