//==================================================================
/// AppBaseConfig.cpp
///
/// Created by Davide Pasca - 2020/01/06
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DN_JSON.h"
#include "SerializeJS.h"
#include "AppBaseConfig.h"

//==================================================================
void ABConfigData::Serialize( SerialJS &v_ ) const
{
    v_.MSerializeObjectStart();
    SERIALIZE_THIS_MEMBER( v_, sys_win_pos_x            );
    SERIALIZE_THIS_MEMBER( v_, sys_win_pos_y            );
    SERIALIZE_THIS_MEMBER( v_, sys_win_siz_w            );
    SERIALIZE_THIS_MEMBER( v_, sys_win_siz_h            );
    SERIALIZE_THIS_MEMBER( v_, color_scheme             );
    SERIALIZE_THIS_MEMBER( v_, save_win_layout          );
    SERIALIZE_THIS_MEMBER( v_, content_scale            );
    SERIALIZE_THIS_MEMBER( v_, enable_multi_viewports   );
    v_.MSerializeObjectEnd();
}

void ABConfigData::Deserialize( DeserialJS &v_ )
{
    DESERIALIZE_THIS_MEMBER( v_, sys_win_pos_x          );
    DESERIALIZE_THIS_MEMBER( v_, sys_win_pos_y          );
    DESERIALIZE_THIS_MEMBER( v_, sys_win_siz_w          );
    DESERIALIZE_THIS_MEMBER( v_, sys_win_siz_h          );
    DESERIALIZE_THIS_MEMBER( v_, color_scheme           );
    DESERIALIZE_THIS_MEMBER( v_, save_win_layout        );
    DESERIALIZE_THIS_MEMBER( v_, content_scale          );
    DESERIALIZE_THIS_MEMBER( v_, enable_multi_viewports );
}

//==================================================================
AppBaseConfig::AppBaseConfig(
            const DStr &layoutPathFName,
            const DStr &pathFNmae )
    : mLayoutSavePathFName(layoutPathFName)
    , mPathFName(pathFNmae)
{
    if ( !pathFNmae.empty() && FU_FileExists( pathFNmae ) )
    {
        try {
            DeserializeFromFile( pathFNmae, mData );
        }
        catch (...)
        {
            // ignore
        }
    }
}

//==================================================================
void AppBaseConfig::SaveAppBaseConfig()
{
    SerializeToFile( mPathFName, mData );
}

