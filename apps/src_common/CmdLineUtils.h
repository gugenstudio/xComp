//==================================================================
/// CmdLineUtils.h
///
/// Created by Davide Pasca - 2019/04/04
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef CMDLINEUTILS_H
#define CMDLINEUTILS_H

#include <map>
#include "DBase.h"

//==================================================================
static std::map<DStr,DStr> CLU_ParseMacros( int argc, const char *argv[] )
{
    std::map<DStr,DStr> macros;

    DStr logSubDir;
    for (int i=1; i < argc; ++i)
    {
        if ( argv[i] == strstr( argv[i], "-D" ) )
        {
            char buff[128] {};
            if ( strlen( argv[i] ) >= sizeof(buff) )
            {
                printf( "Bad macro definition '%s'\n", argv[i] );
                exit( -1 );
            }
            strcpy_s( buff, argv[i] + 2 );

            DVec<char *> pSplits;

            DUT::StrSplitLine( buff, pSplits, "=", false );

            if ( pSplits.size() != 2 )
            {
                printf( "Bad macro definition '%s'\n", argv[i] );
                exit( -1 );
            }

            macros.emplace( StrMakeLower(pSplits[0]), StrMakeLower(pSplits[1]) );
            macros.emplace( StrMakeUpper(pSplits[0]), StrMakeUpper(pSplits[1]) );
        }
    }

    return macros;
}


#endif

