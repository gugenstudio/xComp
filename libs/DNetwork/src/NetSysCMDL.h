//==================================================================
/// NetSysCMDL.h
///
/// Created by Davide Pasca - 2020/11/07
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef NETSYSCMDL_H
#define NETSYSCMDL_H

#include "DBase.h"
#include "DContainers.h"

//==================================================================
inline void NSCMDL_AddCommand( DStr &cmdList, const DStr &cmd, const DStr &val )
{
    cmdList += cmd + '\n';
    cmdList += val + '\n';
}

inline void NSCMDL_AddCommand( DStr &cmdList, const DStr &cmd, const char *pVal )
{
    cmdList += cmd        + '\n';
    cmdList += DStr(pVal) + '\n';
}

inline void NSCMDL_AddCommand( DStr &cmdList, const DStr &cmd, int val )
{
    NSCMDL_AddCommand( cmdList, cmd, std::to_string( (unsigned)val ) );
}

inline void NSCMDL_AddCommand( DStr &cmdList, const DStr &cmd, bool val )
{
    NSCMDL_AddCommand( cmdList, cmd, (int)val );
}

//
void NSCMDL_LogOutList( const DStr &cmdList );
DStr NSCMDL_MakePrintStr( const DStr &cmdList );

//
bool NSCMDL_ParseCommands(
                const DStr &cmdList,
                const DFun<void (const DStr&, const DStr&)> &onCmdFn );


#endif

