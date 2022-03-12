//==================================================================
/// GTVersions.h
///
/// Created by Davide Pasca - 2022/02/08
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef GTVERSIONS_H
#define GTVERSIONS_H

#define GTV_XCOMP_NAME      "xComp"
#define GTV_XCOMP_LONGNAME  "xComp - Sequential image comparer"

#define GTV_SUITE_VERSION "1.1.1"
#define GTV_SUITE_DISPURL "www.gugenstudio.co.jp"
#define GTV_SUITE_FULLURL "http://www.gugenstudio.co.jp"

#include <array>
#include <string>
#include <stdio.h>

inline std::array<int,3> BV_DecodeSuiteVersion( const std::string &verStr )
{
    if (const auto p1 = verStr.find_first_of( '.' ); p1 != std::string::npos)
        if (const auto p2 = verStr.find_first_of( '.', p1+1 ); p2 != std::string::npos)
        {
            const auto p3 = verStr.size();

            const auto s1 = verStr.substr( 0, p1 );
            const auto s2 = verStr.substr( p1+1, p2 - (p1+1) );
            const auto s3 = verStr.substr( p2+1, p3 - (p2+1) );

            if ( !s1.empty() && !s2.empty() && !s3.empty() )
            {
                return {
                    std::stoi( s1 ),
                    std::stoi( s2 ),
                    std::stoi( s3 )
                };
            }
        }

    return {0,0,0};
}

#endif

