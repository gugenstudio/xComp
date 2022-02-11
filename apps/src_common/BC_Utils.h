//==================================================================
/// BC_Utils.h
///
/// Created by Davide Pasca - 2020/01/17
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef BC_UTILS_H
#define BC_UTILS_H

#include "DBase.h"

DStr BCUT_FindAssetsDir();
DStr BCUT_FindFont(const DStr &name );
DStr BCUT_FindIcon(const DStr &name );
DStr BCUT_MakeAppBaseFName( const char *pArgv0 );
DStr BCUT_MakeAppBaseWorkDir( const DStr &profileBaseDir, const char *pArgv0 );
DStr BCUT_MakeProfileNameFromArgs( int argc, const char *argv[], bool forDisplay=false );
DStr BCUT_MakeProfileFolderNameFromArgs( int argc, const char *argv[] );
DStr BCUT_MakeFNameCompatibleString( DStr str );
DStr BCUT_MakeAppUserProfileDir( int argc, const char *argv[] );
DStr BCUT_MakeAppUserProfileDisplay( int argc, const char *argv[] );

void BCUT_BacktraceHandler( int sig );

#endif

