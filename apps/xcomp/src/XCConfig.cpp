//==================================================================
/// XCConfig.cpp
///
/// Created by Davide Pasca - 2022/01/21
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "GTVersions.h"
#include "XCConfig.h"

//==================================================================
void XCConfig::Serialize( SerialJS &v_ ) const
{
    v_.MSerializeObjectStart();
    SerializeMember( v_, "cfg_savedVer", DStr(GTV_SUITE_VERSION) );
    SERIALIZE_THIS_MEMBER( v_, cfg_scanDir              );
    SERIALIZE_THIS_MEMBER( v_, cfg_scanDirHist          );
    SERIALIZE_THIS_MEMBER( v_, cfg_saveDir              );
    SERIALIZE_THIS_MEMBER( v_, cfg_ctrlPanButton        );
    SERIALIZE_THIS_MEMBER( v_, cfg_dispAutoFit          );
    SERIALIZE_THIS_MEMBER( v_, cfg_imsConfig           );
    v_.MSerializeObjectEnd();
}

void XCConfig::Deserialize( DeserialJS &v_ )
{
    DESERIALIZE_THIS_MEMBER( v_, cfg_savedVer           );
    DESERIALIZE_THIS_MEMBER( v_, cfg_scanDir            );
    DESERIALIZE_THIS_MEMBER( v_, cfg_scanDirHist        );
    DESERIALIZE_THIS_MEMBER( v_, cfg_saveDir            );
    DESERIALIZE_THIS_MEMBER( v_, cfg_ctrlPanButton      );
    DESERIALIZE_THIS_MEMBER( v_, cfg_dispAutoFit        );
    DESERIALIZE_THIS_MEMBER( v_, cfg_imsConfig          );

    // remove empty or unreachable directories
    for (auto it=cfg_scanDirHist.begin(); it != cfg_scanDirHist.end();)
    {
        if ( it->empty() || !FU_DirectoryExists( *it ) )
            it = cfg_scanDirHist.erase( it );
        else
            ++it;
    }
}

//==================================================================
bool XCConfig::AddScanDirToHistory()
{
    auto &v = cfg_scanDirHist;
    if ( !cfg_scanDir.empty() &&
         v.end() == std::find( v.begin(), v.end(), cfg_scanDir ) )
    {
        // erase the oldest, if there are too many entries
        if ( v.size() > 10 )
            v.erase( v.begin() );

        // append the new one
        v.push_back( cfg_scanDir );
        return true;
    }

    return false;
}
