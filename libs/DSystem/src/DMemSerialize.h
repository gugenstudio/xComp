//==================================================================
/// DMemSerialize.h
///
/// Created by Davide Pasca - 2019/4/13
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DMEMSERIALIZE_H
#define DMEMSERIALIZE_H

#include <algorithm>
#include <unordered_map>
#include "DUT_MemFile.h"
#include "DSerialBase.h"

//==================================================================
class DMemSerial : public DSerialBase
{
public:
    DUT::MemWriterDynamic mW;

    DMemSerial() : DSerialBase(DSERIAL_TYPE_DMEM) {}

    void MSerialize( const DStr &v ) override { mW.WritePStr32( v ); }
    void MSerialize(      size_t v ) override { mW.WriteValue( v );  }
    void MSerialize(        bool v ) override { mW.WriteValue( v );  }
    void MSerialize(         int v ) override { mW.WriteValue( v );  }
    void MSerialize(    uint32_t v ) override { mW.WriteValue( v );  }
    void MSerialize(    uint16_t v ) override { mW.WriteValue( v );  }
    void MSerialize(     int64_t v ) override { mW.WriteValue( v );  }
    void MSerialize(      double v ) override { mW.WriteValue( v );  }
    void MSerialize(       float v ) override { mW.WriteValue( v );  }
    void MSerialize(const Double2 &v) override { mW.WriteValue( v );  }
};

//==================================================================
class DMemDeserial : public DDeserialBase
{
public:
    DUT::MemReader mR;

    DMemDeserial() : DDeserialBase(DSERIAL_TYPE_DMEM) {}

    DMemDeserial( DUT::MemReader &&src )
        : DDeserialBase(DSERIAL_TYPE_DMEM)
    {
        mR = std::move( src );
    }

    bool MDeserializeIsEOF()         override { return mR.IsEOF();  }

    void MDeserialize(     DStr &v ) override { mR.ReadPStr32( v ); }
    void MDeserialize(   size_t &v ) override { mR.ReadValue( v );  }
    void MDeserialize(     bool &v ) override { mR.ReadValue( v );  }
    void MDeserialize(      int &v ) override { mR.ReadValue( v );  }
    void MDeserialize( uint32_t &v ) override { mR.ReadValue( v );  }
    void MDeserialize( uint16_t &v ) override { mR.ReadValue( v );  }
    void MDeserialize(  int64_t &v ) override { mR.ReadValue( v );  }
    void MDeserialize(   double &v ) override { mR.ReadValue( v );  }
    void MDeserialize(    float &v ) override { mR.ReadValue( v );  }
    void MDeserialize(  Double2 &v ) override { mR.ReadValue( v );  }
};

#endif

