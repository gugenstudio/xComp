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
    SERIALIZE_THIS_MEMBER( v_, cfg_saveDir              );
    SERIALIZE_THIS_MEMBER( v_, cfg_dispUseBilinear      );
    SERIALIZE_THIS_MEMBER( v_, cfg_dispConvToSRGB       );
    SERIALIZE_THIS_MEMBER( v_, cfg_dispToneMapping      );
    SERIALIZE_THIS_MEMBER( v_, cfg_dispAutoFit          );
    SERIALIZE_THIS_MEMBER( v_, cfg_ctrlPanButton        );
    v_.MSerializeObjectEnd();
}

void XCConfig::Deserialize( DeserialJS &v_ )
{
    DESERIALIZE_THIS_MEMBER( v_, cfg_savedVer           );
    DESERIALIZE_THIS_MEMBER( v_, cfg_scanDir            );
    DESERIALIZE_THIS_MEMBER( v_, cfg_saveDir            );
    DESERIALIZE_THIS_MEMBER( v_, cfg_dispUseBilinear    );
    DESERIALIZE_THIS_MEMBER( v_, cfg_dispConvToSRGB     );
    DESERIALIZE_THIS_MEMBER( v_, cfg_dispToneMapping    );
    DESERIALIZE_THIS_MEMBER( v_, cfg_dispAutoFit        );
    DESERIALIZE_THIS_MEMBER( v_, cfg_ctrlPanButton      );

    DStr oldScanDir;
    DeserializeMember( v_, "bls_sourcePath", oldScanDir );
    if ( cfg_scanDir.empty() )
        cfg_scanDir = oldScanDir;
}
