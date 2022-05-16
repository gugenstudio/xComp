//==================================================================
/// DUT_Str.cpp
///
/// Created by Davide Pasca - 2018/3/16
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <ctype.h>
#include "DUT_Str.h"

//==================================================================
namespace DUT
{

//==================================================================
void StrStripBeginEndWhite( char *pStr )
{
	size_t	len = strlen( pStr );

	if NOT( len )
		return;

	int	newLen = (int)len;
	for (int i=(int)len-1; i >= 0; --i)
	{
		char ch = pStr[i];
		if ( IsWhite( ch ) )
			pStr[i] = 0;
		else
		{
			newLen = i + 1;
			break;
		}
	}

	size_t	di = 0;
	bool	foundNonWhite = false;
	for (int si=0; si < newLen; ++si)
	{
		char ch = pStr[si];

		if ( foundNonWhite || !IsWhite( ch ) )
		{
			pStr[di++] = pStr[si];
			foundNonWhite = true;
		}
	}
	pStr[di] = 0;
}

//==================================================================
const char *StrStrI( const char *pStr, const char *pSearch )
{
	// ANSI strstr behavior: it's a match if search is empty
	if NOT( pSearch[0] )
		return pStr;

	for (size_t i=0; pStr[i]; ++i)
	{
		if ( tolower(pSearch[0]) == tolower(pStr[i]) )
		{
			size_t	j = 0;
			for (; pSearch[j]; ++j)
				if ( tolower(pStr[i+j]) != tolower(pSearch[j]) )
					break;

			// reached the end of the search string ?
			if ( pSearch[j] == 0 )
				return pStr + i;	// it's a match
		}
	}

	return NULL;
}

//==================================================================
bool StrStartsWithI( const char *pStr, const char *pSearch )
{
	return pStr == StrStrI( pStr, pSearch );
}

//==================================================================
bool StrEndsWithI( const char *pStr, const char *pSearch )
{
	size_t	strLen = strlen( pStr );
	size_t	searchLen = strlen( pSearch );

	if ( searchLen > strLen )
		return false;

	return 0 == strcasecmp( pStr + strLen - searchLen, pSearch );
}

//==================================================================
void StrToUpper( DStr &str )
{
	for (size_t i=0; i < str.size(); ++i)
		str[i] = toupper( str[i] );
}
//==================================================================
void StrToUpper( char *pStr )
{
	for (size_t i=0; pStr[i]; ++i)
		pStr[i] = toupper( pStr[i] );
}

//==================================================================
void StrToLower( DStr &str )
{
	for (size_t i=0; i < str.size(); ++i)
		str[i] = tolower( str[i] );
}
//==================================================================
void StrToLower( char *pStr )
{
    for (size_t i=0; pStr[i]; ++i)
        pStr[i] = tolower( pStr[i] );
}

//==================================================================
const char *StrFindFirstOf( const char *pStr, char searchCh )
{
	for (; ; ++pStr)
	{
		if ( *pStr == 0 )
            return NULL;
        else
		if ( *pStr == searchCh )
			return pStr;
	}
}

//==================================================================
const char *StrFindLastOf( const char *pStr, char searchCh )
{
	size_t	len = strlen( pStr );

	for (size_t i=0; i < len; ++i)
	{
		if ( pStr[ len-1 - i ] == searchCh )
			return &pStr[ len-1 - i];
	}

	return NULL;
}

//==================================================================
const char *StrFindLastOfMulti( const char *pStr, const char *pChars )
{
	size_t	len = strlen( pStr );
	size_t	findN = strlen( pChars );

	for (size_t i=0; i < len; ++i)
	{
		for (size_t j=0; j < findN; ++j)
		{
			if ( pStr[ len-1 - i ] == pChars[j] )
				return &pStr[ len-1 - i];
		}
	}

	return NULL;
}

//==================================================================
static char *find_first_sep( char *pTxt, const char *pSeps )
{
	size_t  sepN = strlen( pSeps );

	while ( pTxt[0] )
	{
		for (size_t i=0; i < sepN; ++i)
			if ( pTxt[0] == pSeps[i] )
				return pTxt;

		++pTxt;
	}

	return NULL;
}

//==================================================================
static char *find_first_nonsep( char *pTxt, const char *pSeps )
{
	size_t	sepN = strlen( pSeps );

	while ( pTxt[0] )
	{
		size_t i = 0;
		for (; i < sepN; ++i)
			if ( pTxt[0] == pSeps[i] )
				break;

		if ( i == sepN )
			return pTxt;

		++pTxt;
	}

	return NULL;
}

//==================================================================
void StrSplitLine( char *pLine, DVec<char *> &out_pPtrs, const char *pSeps, bool sepMakesEmptyStr )
{
    out_pPtrs.clear();

	if ( sepMakesEmptyStr )
	{
		char *pTok = pLine;

        while ( true )
        {
    		out_pPtrs.push_back( pTok );

            pTok = find_first_sep( pTok, pSeps );
    		if NOT( pTok )
    			break;

            *pTok++ = 0;
        }
	}
    else
    {
    	char	*pLine2 = pLine;

    	while ( pLine2[0] )
    	{
    		char *pTok = find_first_nonsep( pLine2, pSeps );
    		if NOT( pTok )
    			break;

			out_pPtrs.push_back( pTok );

    		char *pTokEnd = find_first_sep( pTok, pSeps );
    		if NOT( pTokEnd )
    		{
    			break;
    		}
    		else
    		{
    			pTokEnd[0] = 0;
    			pLine2 = pTokEnd;
    		}

    		++pLine2;
    	}
    }
}

//==================================================================
DStr StrGetDirPath( const char *pStr )
{
	const char *pLastSlash = StrFindLastOf( pStr, '/' );
	const char *pLastBSlash = StrFindLastOf( pStr, '\\' );
	const char *pLastColon = StrFindLastOf( pStr, ':' );

	size_t	n = 0;

	if ( pLastBSlash )
		n = std::max( n, (size_t)(pLastBSlash - pStr + 1) );

	if ( pLastSlash )
		n = std::max( n, (size_t)(pLastSlash - pStr + 1) );

	if ( pLastColon )
		n = std::max( n, (size_t)(pLastColon - pStr + 1) );

	if ( n )
		return DStr( pStr, n );
	else
		return DStr();
}

//==================================================================
const char *StrSeekToFilename( const char *pStr )
{
	const char *pLastSlash = StrFindLastOf( pStr, '/' );
	const char *pLastBSlash = StrFindLastOf( pStr, '\\' );
	const char *pLastColon = StrFindLastOf( pStr, ':' );

	size_t	n = 0;

	if ( pLastBSlash )
		n = std::max( n, (size_t)(pLastBSlash - pStr + 1) );

	if ( pLastSlash )
		n = std::max( n, (size_t)(pLastSlash - pStr + 1) );

	if ( pLastColon )
		n = std::max( n, (size_t)(pLastColon - pStr + 1) );

	return pStr + n;
}

//==================================================================
char *StrTok_StrQuot(
        char *pSrcStr,
        char **ppContext,
        bool *pOut_HadQuots,
        const char *pSeps )
{
    if NOT( pSeps )
    {
        static const char *spWhiteSeps = " \t\n\r\f";
        pSeps = spWhiteSeps;
    }

    auto findSepChar = []( char ch, const char *pSepStr )
    {
        while ( pSepStr[0] )
        {
            if ( ch == *pSepStr++ )
                return true;
        }

        return false;
    };


	if ( pSrcStr )
	{
		*ppContext = pSrcStr;
	}

	char *p = *ppContext;

	if NOT( p )
		return NULL;

	size_t i = 0;
	for (; ; ++i)
	{
		if NOT( p[i] )
			return NULL;

		if NOT( findSepChar( p[i], pSeps ) )
			break;
	}

	bool isInStr;

	if ( p[i] == '"' )
	{
		isInStr = true;
		++i;

        if ( pOut_HadQuots )
            *pOut_HadQuots = true;
	}
	else
    {
		isInStr = false;

        if ( pOut_HadQuots )
            *pOut_HadQuots = false;
    }

	size_t	start = i;

	for (; p[i]; ++i)
	{
        bool isQuotedTokenEnd =
                (isInStr &&
                    p[i] == '"' && (i == 0 || p[i-1] != '\\'));

		if ( isQuotedTokenEnd ||
			 (!isInStr && findSepChar( p[i], pSeps )) )
		{
			p[i] = 0;
			*ppContext = p + i + 1;
			return p + start;
		}
	}

	*ppContext = NULL;
	return p + start;
}

//==================================================================
DVec<char *> VecStrTok_StrQuot(
        char *pSrcStr,
        const char *pSeps )
{
    DVec<char *> vec;

    char *pContext {};

    char *pTok = StrTok_StrQuot( pSrcStr, &pContext, nullptr, pSeps );
    if NOT( pTok )
        return vec;

    do {
        vec.push_back( pTok );
        pTok = StrTok_StrQuot( nullptr, &pContext, nullptr, pSeps );
    } while ( pTok );

    return vec;
}

//==================================================================
void StrConvertCEscape( char *pStr )
{
	if NOT( pStr[0] )
		return;

	size_t len = strlen( pStr );

	bool isEsc = false;
	size_t j = 0;
	for (size_t i=0; i < len; ++i)
	{
		if ( pStr[i] == '\\' )
		{
			if ( isEsc )
				isEsc = false;
			else
			{
				isEsc = true;
				continue;
			}
		}

		if ( isEsc )
		{
			isEsc = false;

			if ( pStr[i] == 'n' )
				pStr[j++] = '\n';
		}
		else
		{
			pStr[j++] = pStr[i];
		}
	}
	pStr[j] = 0;
}

//==================================================================
}

