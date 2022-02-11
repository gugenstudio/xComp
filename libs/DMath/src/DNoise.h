//==================================================================
/// DNoise.h
///
/// Created by Davide Pasca - 2012/12/8
/// Perlin Noise implementation by Ken Perlin.
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DNOISE_H
#define DNOISE_H

#include "DMathBase.h"
#include "DVector.h"

// From: http://mrl.nyu.edu/~perlin/noise/ImprovedNoise4D.java

//==================================================================
namespace DNoise
{

extern int _p[512];

DMT_FINLINE float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
DMT_FINLINE float lerp(float t, float a, float b) { return a + t * (b - a); }
DMT_FINLINE float grad(int hash, float x, float y, float z)
{
    int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
         v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

DMT_FINLINE float grad(int hash, float x, float y, float z, float w)
{
    int h = hash & 31; // CONVERT LO 5 BITS OF HASH TO 32 GRAD DIRECTIONS.
    float a=y,b=z,c=w;            // X,Y,Z
    switch (h >> 3) {          // OR, DEPENDING ON HIGH ORDER 2 BITS:
    case 1: a=w;b=x;c=y;break;     // W,X,Y
    case 2: a=z;b=w;c=x;break;     // Z,W,X
    case 3: a=y;b=z;c=w;break;     // Y,Z,W
    }
    return ((h&4)==0 ? -a:a) + ((h&2)==0 ? -b:b) + ((h&1)==0 ? -c:c);
}

DMT_FINLINE float noise3(float x, float y, float z)
{
    float xfl = floorf(x);
    float yfl = floorf(y);
    float zfl = floorf(z);
    int X = (int)xfl & 255;                  // FIND UNIT CUBE THAT
    int Y = (int)yfl & 255;                  // CONTAINS POINT.
    int Z = (int)zfl & 255;
    x -= xfl;                                // FIND RELATIVE X,Y,Z
    y -= yfl;                                // OF POINT IN CUBE.
    z -= zfl;
    float u = fade(x);                                // COMPUTE FADE CURVES
    float v = fade(y);                                // FOR EACH OF X,Y,Z.
    float w = fade(z);
    int A  = _p[X  ]+Y;
    int AA = _p[A  ]+Z;
    int AB = _p[A+1]+Z;      // HASH COORDINATES OF
    int B  = _p[X+1]+Y;
    int BA = _p[B  ]+Z;
    int BB = _p[B+1]+Z;      // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad(_p[AA  ], x  , y  , z   ),  // AND ADD
                                   grad(_p[BA  ], x-1, y  , z   )), // BLENDED
                           lerp(u, grad(_p[AB  ], x  , y-1, z   ),  // RESULTS
                                   grad(_p[BB  ], x-1, y-1, z   ))),// FROM  8
                   lerp(v, lerp(u, grad(_p[AA+1], x  , y  , z-1 ),  // CORNERS
                                   grad(_p[BA+1], x-1, y  , z-1 )), // OF CUBE
                           lerp(u, grad(_p[AB+1], x  , y-1, z-1 ),
                                   grad(_p[BB+1], x-1, y-1, z-1 ))));
}

DMT_FINLINE float noise4(float x, float y, float z, float w)
{
    float xfl = floorf(x);
    float yfl = floorf(y);
    float zfl = floorf(z);
    float wfl = floorf(w);
    int X = (int)xfl & 255;                  // FIND UNIT HYPERCUBE
    int Y = (int)yfl & 255;                  // THAT CONTAINS POINT.
    int Z = (int)zfl & 255;
    int W = (int)wfl & 255;
    x -= xfl;                                // FIND RELATIVE X,Y,Z,W
    y -= yfl;                                // OF POINT IN CUBE.
    z -= zfl;
    w -= wfl;
    float a = fade(x);                             // COMPUTE FADE CURVES
    float b = fade(y);                             // FOR EACH OF X,Y,Z,W.
    float c = fade(z);
    float d = fade(w);

    int A   = _p[X  ]+Y; int AA = _p[A]+Z, AB = _p[A+1]+Z;   // HASH COORDINATES OF
    int B   = _p[X+1]+Y; int BA = _p[B]+Z, BB = _p[B+1]+Z;   // THE 16 CORNERS OF
    int AAA = _p[AA]+W;  int AAB = _p[AA+1]+W;               // THE HYPERCUBE.
    int ABA = _p[AB]+W;  int ABB = _p[AB+1]+W;
    int BAA = _p[BA]+W;  int BAB = _p[BA+1]+W;
    int BBA = _p[BB]+W;  int BBB = _p[BB+1]+W;

    return lerp(d,                                     // INTERPOLATE DOWN.
               lerp(c,lerp(b,lerp(a,grad(_p[AAA  ], x  , y  , z  , w),
                                    grad(_p[BAA  ], x-1, y  , z  , w)),
                             lerp(a,grad(_p[ABA  ], x  , y-1, z  , w),
                                    grad(_p[BBA  ], x-1, y-1, z  , w))),

                      lerp(b,lerp(a,grad(_p[AAB  ], x  , y  , z-1, w),
                                    grad(_p[BAB  ], x-1, y  , z-1, w)),
                             lerp(a,grad(_p[ABB  ], x  , y-1, z-1, w),
                                    grad(_p[BBB  ], x-1, y-1, z-1, w)))),

               lerp(c,lerp(b,lerp(a,grad(_p[AAA+1], x  , y  , z  , w-1),
                                    grad(_p[BAA+1], x-1, y  , z  , w-1)),
                             lerp(a,grad(_p[ABA+1], x  , y-1, z  , w-1),
                                    grad(_p[BBA+1], x-1, y-1, z  , w-1))),

                      lerp(b,lerp(a,grad(_p[AAB+1], x  , y  , z-1, w-1),
                                    grad(_p[BAB+1], x-1, y  , z-1, w-1)),
                             lerp(a,grad(_p[ABB+1], x  , y-1, z-1, w-1),
                                    grad(_p[BBB+1], x-1, y-1, z-1, w-1)))));
}

DMT_FINLINE float noise1( float x )         { return noise3( x, 0, 0 ); }
DMT_FINLINE float noise2( const float v[2] ){ return noise3( v[0], v[1], 0 ); }
DMT_FINLINE float noise3( const float v[3] ){ return noise3( v[0], v[1], v[2] ); }
DMT_FINLINE float noise2( const Float2 &v ) { return noise3( v[0], v[1], 0 ); }
DMT_FINLINE float noise3( const Float3 &v ) { return noise3( v[0], v[1], v[2] ); }
DMT_FINLINE float noise4( const Float4 &v ) { return noise4(v[0], v[1], v[2], v[3] ); }

// overloads
DMT_FINLINE float noise( float x )         { return noise3( x, 0, 0 ); }
DMT_FINLINE float noise( const Float2 &x ) { return noise3( x[0], x[1], 0 ); }
DMT_FINLINE float noise( const Float3 &x ) { return noise3( x[0], x[1], x[2] ); }
DMT_FINLINE float noise( const Float4 &x ) { return noise4( x[0], x[1], x[2], x[3] ); }

template<typename _T>
float fBm( _T point, float H, float lacu, float octaves )
{
    float val = 0;

    const int octaves_int = (int)octaves;

    for (int i=0; i < octaves_int; ++i)
    {
        val += noise( point ) * powf( lacu, -H*i );
        point *= lacu;
    }

    const float rem = octaves - (float)octaves_int;
    if ( rem )
        val += rem * noise( point ) * powf( lacu, -H*octaves_int );

    return val;
}

};

#endif

