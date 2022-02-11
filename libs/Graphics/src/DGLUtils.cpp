//==================================================================
/// DGLUtils.cpp
///
/// Created by Davide Pasca - 2011/6/25
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OPENGL

#include "GL/glew.h"
#include "DGLUtils.h"
#include "DLogOut.h"

//#define USE_LAZY_FAST_ERR_CHECK

#if defined(GL_ARB_debug_output) && !defined(__linux__)
static constexpr bool INTERCEPT_LOW = false;
static constexpr bool INTERCEPT_MED = false;
static constexpr bool INTERCEPT_NOT = false;
#endif

//==================================================================
namespace DGLUT
{

#ifdef USE_LAZY_FAST_ERR_CHECK
static bool _sMasterCheck;
#endif

//==================================================================
static const char *getErrStr( GLenum err )
{
    switch ( err )
    {
    case GL_NO_ERROR:           return "GL_NO_ERROR";
    case GL_INVALID_ENUM:       return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:      return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:  return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:      return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
        {
            static DStr unkStr;
            unkStr = SSPrintFS( "#x%04x", err );
            return unkStr.c_str();
        }
    }
}

//==================================================================
const char *GetFBStatusStr( u_int status )
{
    switch ( status )
    {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         : return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT : return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_UNSUPPORTED                   : return "GL_FRAMEBUFFER_UNSUPPORTED";

#ifdef GL_FRAMEBUFFER_UNDEFINED
    case GL_FRAMEBUFFER_UNDEFINED                     : return "GL_FRAMEBUFFER_UNDEFINED";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER        : return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER        : return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE        : return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS      : return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
#endif

    default:
        {
            static DStr unkStr;
            unkStr = SSPrintFS( "#x%04x", status );
            return unkStr.c_str();
        }
    }
}

//==================================================================
bool MasterCheckGLErr( const char *pFileName, int line )
{
#if defined(USE_LAZY_FAST_ERR_CHECK)
    bool didErr = false;
    DVec<GLenum> errList;
	GLenum err = glGetError();
	while (err != GL_NO_ERROR)
	{
        didErr = true;
        errList.push_back( err );
        err = glGetError();
    }

    if ( errList.size() )
    {
        if NOT( _sMasterCheck )
        {
            _sMasterCheck = true;

            LogOut( LOG_ERR,
                "Master Error Check found %u errors. Activating detailed check.",
                (u_int)errList.size() );
        }

        DStr errStrList;
        for (auto err : errList)
        {
            errStrList += getErrStr( err );
            errStrList += ' ';
        }
        LogOut( LOG_ERR, "[%s:%i] %s", pFileName, line, errStrList.c_str() );
    }
    else
    {
        _sMasterCheck = false;
    }

    return didErr;
#else
    return CheckGLErr( pFileName, line );
#endif
}

//==================================================================
bool CheckGLErr( const char *pFileName, int line, bool doPrint )
{
#if defined(USE_LAZY_FAST_ERR_CHECK)
    if NOT( _sMasterCheck )
        return false;
#endif

    bool didErr = false;
	GLenum err = glGetError();
	while (err != GL_NO_ERROR)
	{
        didErr = true;
        const char *pErrStr = getErrStr( err );

        if ( pErrStr )
        {
            LogOut( LOG_ERR, "GL error: %s at %s : %i", pErrStr, pFileName, line );
        }
        else
        {
            LogOut( LOG_ERR, "Unknown error: %d 0x%x at %s : %i", err, err, pFileName, line );
        }

		err = glGetError();
	}

    return didErr;
}

//==================================================================
void FlushGLErr()
{
	while ( glGetError() != GL_NO_ERROR )
    {
    }
}

#if defined(GL_ARB_debug_output) && !defined(__linux__)
//==================================================================
static void GLAPIENTRY errorCallbackARB(
                GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *pMessage,
                const void *pUserParam )
{
    const char *pSource = "";
    switch ( source )
    {
    case GL_DEBUG_SOURCE_API            : pSource = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM  : pSource = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: pSource = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY    : pSource = "3rd Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION    : pSource = "App"; break;
    case GL_DEBUG_SOURCE_OTHER          : pSource = "Other"; break;
    default: pSource = "Unknown"; break;
    }

    const char *pType = "";
    switch ( type )
    {
    case GL_DEBUG_TYPE_ERROR              : pType = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: pType = "Deprecated"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : pType = "Undefined"; break;
    case GL_DEBUG_TYPE_PORTABILITY        : pType = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE        : pType = "Performance"; break;
    case GL_DEBUG_TYPE_OTHER              : pType = "Other"; break;
    default: pType = "Unknown"; break;
    }

    const char *pSeverity = "";
    switch ( severity )
    {
    case GL_DEBUG_SEVERITY_HIGH  : pSeverity = "H"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: pSeverity = "M"; break;
    case GL_DEBUG_SEVERITY_LOW   : pSeverity = "L"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: pSeverity = "Not"; break;
    default: pSeverity = "Unk"; break;
    }

    (void)pSource;
    (void)id;
    LogOut( LOG_ERR, "* GLERR: %s(%s) -- %s", pType, pSeverity, pMessage );
}
#endif

//==================================================================
bool SetupErrorIntercept()
{
#if defined(GL_ARB_debug_output) && !defined(__linux__)
    if ( glDebugMessageCallback )
    {
        glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
        glDebugMessageCallback( errorCallbackARB, NULL );

        auto enableServerity = []( auto sev, bool onOff )
        {
            glDebugMessageControl(
                GL_DONT_CARE,
                GL_DONT_CARE,
                sev,
                0,
                nullptr,
                onOff ? GL_TRUE : GL_FALSE );
        };

        enableServerity( GL_DONT_CARE, true );
        enableServerity( GL_DEBUG_SEVERITY_LOW, INTERCEPT_LOW );
        enableServerity( GL_DEBUG_SEVERITY_MEDIUM, INTERCEPT_MED );
        enableServerity( GL_DEBUG_SEVERITY_NOTIFICATION, INTERCEPT_NOT );

        return true;
    }
#endif

    return false;
}

//==================================================================
}

#endif

