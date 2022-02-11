//==================================================================
/// GShaderProg.cpp
///
/// Created by Davide Pasca - 2021/05/30
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifdef ENABLE_OPENGL
# include "GL/glew.h"
# include "DGLUtils.h"
#endif
#include "DContainers.h"
#include "DExceptions.h"
#include "GShaderProg.h"

//==================================================================
#ifdef ENABLE_OPENGL
static void check_shader_compilation( GLuint oid, bool isLink )
{
    GLint n {};
    if ( isLink )
        glGetProgramiv( oid, GL_LINK_STATUS, &n );
    else
        glGetShaderiv( oid, GL_COMPILE_STATUS, &n );

    if NOT( n )
    {
        if ( isLink )
            glGetProgramiv( oid, GL_INFO_LOG_LENGTH, &n );
        else
            glGetShaderiv( oid, GL_INFO_LOG_LENGTH, &n );

        DVec<GLchar>  info_log( n );

        if ( isLink )
            glGetProgramInfoLog( oid, n, &n, info_log.data() );
        else
            glGetShaderInfoLog( oid, n, &n, info_log.data() );

        DEX_RUNTIME_ERROR(
                "%s %s failed: %*s",
                isLink ? "Program" : "Shader",
                isLink ? "linking" : "compilation",
                n,
                info_log.data() );
    }
}
#endif

//==================================================================
GShaderProg::GShaderProg( bool useTex )
{
#ifdef ENABLE_OPENGL
static const DStr vtxSrouce[2] = {
R"RAW(
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_col;

out vec4 v_col;

void main()
{
   v_col = a_col;

   gl_Position = vec4( a_pos * 2.0 - 1.0, 1.0 );
}
)RAW",
R"RAW(
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_col;
layout (location = 2) in vec2 a_tc0;

out vec4 v_col;
out vec2 v_tc0;

void main()
{
   v_col = a_col;
   v_tc0 = a_tc0;

   gl_Position = vec4( a_pos * 2.0 - 1.0, 1.0 );
}
)RAW"};

static const DStr frgSource[2] = {
R"RAW(
in vec4 v_col;

out vec4 o_col;

void main()
{
   o_col = v_col;
}
)RAW",
R"RAW(
uniform sampler2D s_tex;

in vec4 v_col;
in vec2 v_tc0;

out vec4 o_col;

void main()
{
   o_col = v_col * texture( s_tex, v_tc0 );
}
)RAW"};

    c_auto srcIdx = useTex ? 1 : 0;

    auto makeShader = []( c_auto type, const DStr &src )
    {
        c_auto obj = glCreateShader( type );

        c_auto fullStr = "#version 330\n" + src;

        const GLchar *ppsrcs[2] = { fullStr.c_str(), 0 };

        glShaderSource( obj, 1, &ppsrcs[0], nullptr ); CHECKGLERR;

        glCompileShader( obj );
        CHECKGLERR;

        check_shader_compilation( obj, false );
        return obj;
    };

    mShaderVertex   = makeShader( GL_VERTEX_SHADER  , vtxSrouce[srcIdx] );
    mShaderFragment = makeShader( GL_FRAGMENT_SHADER, frgSource[srcIdx] );

    mShaderProgram = glCreateProgram();

    glAttachShader( mShaderProgram, mShaderVertex );
    glAttachShader( mShaderProgram, mShaderFragment );
    glLinkProgram( mShaderProgram );

    check_shader_compilation( mShaderProgram, true );

    // Always detach shaders after a successful link.
    glDetachShader( mShaderProgram, mShaderVertex );
    glDetachShader( mShaderProgram, mShaderFragment );

    //
    if ( useTex )
    {
        mTexLoc = glGetUniformLocation( mShaderProgram, "s_tex" );
        glUseProgram( mShaderProgram );
        glUniform1i( mTexLoc, 0 );
    }

    // for consistency
    glUseProgram( 0 );
#endif
}

//==================================================================
GShaderProg::~GShaderProg()
{
#ifdef ENABLE_OPENGL
    if ( mShaderProgram )
        glDeleteProgram( mShaderProgram );

    if ( mShaderVertex )
        glDeleteShader( mShaderVertex );

    if ( mShaderVertex )
        glDeleteShader( mShaderFragment );
#endif
}


