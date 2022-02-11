//==================================================================
/// DNETUT.cpp
///
/// Created by Davide Pasca - 2015/7/24
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <cctype>
#include "StringUtils.h"
#include "DNETUT.h"

//==================================================================
namespace DNETUT
{

//==================================================================
bool GetPortNumber( const char *pStr, u_short &out_val )
{
    out_val = 0;

    DStr goodStr;
    char ch;
    while ( (ch = *pStr++) )
    {
        if ( isspace( ch ) )
            continue;

        if ( ch < '0' || ch > '9' )
            return false;

        goodStr += ch;
    }

    int val = atoi( goodStr.c_str() );

    if ( val < 0 || val > 65535 )
        return false;

    out_val = (u_short)val;
    return true;
}

//==================================================================
std::pair<DStr,DStr> SplitURIBaseAndQuery( const DStr &uriFull )
{
    if ( !StrStartsWithI( uriFull.c_str(), "http://"  ) &&
         !StrStartsWithI( uriFull.c_str(), "https://" ) )
        return {};

    DStr base;
    DStr query;
    size_t  slashCnt = 0;
    for (c_auto ch : uriFull)
    {
        if ( ch == '/' )
        {
            slashCnt += 1;

            // skip the slah right before the the query section
            if ( slashCnt == 3 )
                continue;
        }

        if ( slashCnt >= 3 )
            query.push_back( ch );
        else
            base.push_back( ch );
    }

    return { base, query };
}

}

