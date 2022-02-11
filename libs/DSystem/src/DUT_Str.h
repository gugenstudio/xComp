//==================================================================
/// DUT_Str.h
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DUT_STR_H
#define DUT_STR_H

#include "DContainers.h"

//==================================================================
namespace DUT
{

//==================================================================
inline bool IsWhite( char ch )
{
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f';
}

//==================================================================
void StrStripBeginEndWhite( char *pStr );
const char *StrStrI( const char *pStr, const char *pSearch );
bool StrStartsWithI( const char *pStr, const char *pSearch );
bool StrEndsWithI( const char *pStr, const char *pSearch );
void StrToUpper( DStr &str );
void StrToUpper( char *pStr );
void StrToLower( DStr &str );
void StrToLower( char *pStr );
inline DStr StrMakeLower( DStr str ) { StrToLower( str ); return str; }
inline DStr StrMakeUpper( DStr str ) { StrToUpper( str ); return str; }
const char *StrFindFirstOf( const char *pStr, char searchCh );
const char *StrFindLastOf( const char *pStr, char searchCh );
const char *StrFindLastOfMulti( const char *pStr, const char *pChars );

//==================================================================
inline DStr StrReplaceSubStr(
        const DStr &src,
        const DStr &findSub,
        const DStr &replSub )
{
    const auto pos = src.find( findSub );
    if ( pos == std::string::npos )
        return DStr();

    DStr des = src;
    des.replace( pos, findSub.size(), replSub );
    return des;
}

char *StrTok_StrQuot(
        char *pSrcStr,
        char **ppContext,
        bool *pOut_HadQuots=nullptr,
        const char *pSeps=nullptr );

DVec<char *> VecStrTok_StrQuot(
        char *pSrcStr,
        const char *pSeps=nullptr );

void StrConvertCEscape( char *pStr );
void StrSplitLine( char *pLine, DVec<char *> &out_pPtrs, const char *pSeps, bool sepMakesEmptyStr );
DStr StrGetDirPath( const char *pStr );
const char *StrSeekToFilename( const char *pStr );

inline bool CharIsWhite( char ch )
{
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f';
}

}

#endif

