//==================================================================
/// AppBaseConfig.h
///
/// Created by Davide Pasca - 2020/01/06
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef APPBASECONFIG_H
#define APPBASECONFIG_H

#include <optional>
#include "DBase.h"
#include "SerializeJS.h"

//==================================================================
struct ABConfigData
{
    static constexpr int BAD_POS_VAL = -599999;

    static constexpr double DEF_CONT_SCALE = 1.0;

    int     sys_win_pos_x           {BAD_POS_VAL};
    int     sys_win_pos_y           {BAD_POS_VAL};
    int     sys_win_siz_w           {BAD_POS_VAL};
    int     sys_win_siz_h           {BAD_POS_VAL};
    DStr    color_scheme            {};
    bool    save_win_layout         {};
    double  content_scale           {DEF_CONT_SCALE};
    bool    enable_multi_viewports  {};

    void Serialize( SerialJS &v_ ) const;

    void Deserialize( DeserialJS &v_ );

    friend void Serialize( SerialJS &v_, const ABConfigData &o_ ){ o_.Serialize(v_);   }
    friend void Deserialize( DeserialJS &v_,   ABConfigData &o_ ){ o_.Deserialize(v_); }
};

//==================================================================
class AppBaseConfig
{
    static constexpr int BAD_POS_VAL = -599999;

    DStr    mLayoutSavePathFName;
    DStr    mPathFName;

public:
    ABConfigData    mData;

public:
    AppBaseConfig(
            const DStr &layoutPathFName={},
            const DStr &pathFNmae={} );

    void SaveAppBaseConfig();

    c_auto &GetLayoutSavePathFName() const { return mLayoutSavePathFName; }

    std::optional<std::array<int,2>> GetWinPosABC() const
    {
        if ( mData.sys_win_pos_x == BAD_POS_VAL || mData.sys_win_pos_y == BAD_POS_VAL )
            return {};

        return { {mData.sys_win_pos_x, mData.sys_win_pos_y} };
    }

    std::optional<std::array<int,2>> GetWinSizABC() const
    {
        if ( mData.sys_win_siz_w == BAD_POS_VAL || mData.sys_win_siz_h == BAD_POS_VAL )
            return {};

        return { {mData.sys_win_siz_w, mData.sys_win_siz_h} };
    }
};

#endif

