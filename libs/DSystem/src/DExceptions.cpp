//==================================================================
/// DExceptions.cpp
///
/// Created by Davide Pasca - 2016/9/28
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#include <future>
#include "StringUtils.h"
#include "DExceptions.h"


//==================================================================
std::tuple<std::string,int,std::string> DEX_ParseExceptString( const std::string &src )
{
    // minimum req for definition: "[a:1] "
    if ( src.size() >= 6 && src[0] == '[' )
    {
        if (c_auto colonPos = src.find_first_of( ':' );
                   colonPos != std::string::npos)
        {
            if (c_auto closePos = src.find_first_of( ']' );
                       closePos != std::string::npos &&
                       closePos > colonPos)
            {
                c_auto lineStr = src.substr(
                                        colonPos+1,
                                        closePos - (colonPos+1) );

                c_auto lineVal = (int)strtol( lineStr.c_str(), nullptr, 10 );
                // confirm that the line is a number
                if ( lineVal > 0 && lineStr == std::to_string( lineVal ) )
                {
                    return
                    {
                          // after [
                          src.substr( 1, colonPos - 1 )
                        , lineVal
                          // after the space, after the closing ]
                        , src.substr( closePos+2, src.size() - (closePos+2) )
                    };
                }
            }
        }
    }

    return { {}, 0, src };
}

//==================================================================
std::string DEX_CatchExceptionString( std::function<void ()> fn )
{
    try
    {
        fn();
        return DStr();
    }
    catch ( const std::system_error &error )
    {
        return SSPrintFS(
                    "* SYSTEM ERROR: %s\n",
                    error.code().message().c_str() );
    }
    catch ( std::runtime_error &e )
    {
        return SSPrintFS( "%s", e.what() );
    }
    catch ( std::out_of_range &e )
    {
        return SSPrintFS( "* OUT_OF_RANGE: %s", e.what() );
    }
    catch ( std::length_error &e )
    {
        return SSPrintFS( "* LENGTH_ERROR: %s", e.what() );
    }
#if !defined(ANDROID)
    catch ( std::bad_alloc &e )
    {
        return SSPrintFS( "* BAD_ALLOC: %s", e.what() );
    }
#endif
    catch ( std::bad_function_call &e )
    {
        return SSPrintFS( "* BAD_FUNCTION_CALL: %s", e.what() );
    }
    catch ( std::future_error &e )
    {
        return SSPrintFS( "* FUTURE_ERROR: %s", e.what() );
    }
    catch ( ... )
    {
        return "* UNKNOWN_EXCEPTION";
    }

    return DStr();
}

//==================================================================
void DEX_PrintExceptionRethrow( std::function<void ()> fn )
{
    try
    {
        fn();
    }
    catch ( const std::system_error &error )
    {
        fprintf_s( stderr, "* SYSTEM ERROR: %s\n", error.code().message().c_str() );
        throw;
    }
    catch ( std::runtime_error &e )
    {
        fprintf_s( stderr, "%s\n", e.what() );
        throw;
    }
    catch ( std::out_of_range &e )
    {
        fprintf_s( stderr, "* OUT_OF_RANGE: %s\n", e.what() );
        throw;
    }
    catch ( std::length_error &e )
    {
        fprintf_s( stderr, "* LENGTH_ERROR: %s\n", e.what() );
        throw;
    }
#if !defined(ANDROID)
    catch ( std::bad_alloc &e )
    {
        fprintf_s( stderr, "* BAD_ALLOC: %s\n", e.what() );
        throw;
    }
#endif
    catch ( std::bad_function_call &e )
    {
        fprintf_s( stderr, "* BAD_FUNCTION_CALL: %s\n", e.what() );
        throw;
    }
    catch ( std::future_error &e )
    {
        fprintf_s( stderr, "* FUTURE_ERROR: %s\n", e.what() );
        throw;
    }
    catch ( ... )
    {
        fprintf_s( stderr, "* UNKNOWN_EXCEPTION\n" );
        throw;
    }
}

