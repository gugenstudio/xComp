//==================================================================
/// NetSysCMDL.cpp
///
/// Created by Davide Pasca - 2020/11/07
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include "DLogOut.h"
#include "NetSysCMDL.h"

//==================================================================
void NSCMDL_LogOutList( const DStr &cmdList )
{
    LogOut( LOG_DBG, "CMDL Full ```%s```", cmdList.c_str() );
    if (c_auto lines = StrSplitLines( cmdList ); !lines.empty())
    {
        for (size_t i=0; i < lines.size(); ++i)
            LogOut( LOG_DBG, "CMDL %zu) %s", i, lines[i].c_str() );
    }
    else
    {
        LogOut( LOG_DBG, "CMDL Empty" );
    }
}

//==================================================================
DStr NSCMDL_MakePrintStr( const DStr &cmdList )
{
    DStr str;
    str = "CMDL: `";

    c_auto lines = StrSplitLines( cmdList );
    for (size_t i=0; i < lines.size(); ++i)
        str += SSPrintFS( "%zu)%s", i, lines[i].c_str() )
                    + (lines.size() == (i+1) ? "" : " ");

    str += "`";
    return str;
}

//==================================================================
bool NSCMDL_ParseCommands(
                const DStr &cmdList,
                const DFun<void (const DStr&, const DStr&)> &onCmdFn )
{
    if (c_auto lines = StrSplitLines( cmdList ); !lines.empty())
    {
        // must have pairs of lines
        if ( lines.size() & 1 )
        {
            //NSCMDL_LogOutList( cmdList );
            return false;
        }

        for (size_t i=0; i < lines.size(); i += 2)
            onCmdFn( lines[i], lines[i+1] );
    }

    return true;
}

