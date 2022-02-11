//==================================================================
/// DBase.h
///
/// Created by Davide Pasca - 2018/3/15
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DBASE_H
#define DBASE_H

#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <assert.h>
#include <string.h>
#include <memory>
#include <stdint.h>
#include <stddef.h>
#include "DSafeCrt.h"

using U8  = uint8_t;
using U16 = uint16_t;
using I32 =  int32_t;
using U32 = uint32_t;
using I64 =  int64_t;
using U64 = uint64_t;

using u_char  = unsigned char;
using u_short = unsigned short;
using u_int   = unsigned int;

static constexpr size_t DNPOS = (size_t)-1;
static constexpr U32    DNPOS32 = (U32)-1;
static constexpr u_int  DNPOSUI = (u_int)-1;

template <typename T> using uptr = std::unique_ptr<T>;
template <typename T> using sptr = std::shared_ptr<T>;
template <typename T> using wptr = std::weak_ptr<T>;

#define c_auto const auto

#define NOT(_X_) (!(_X_))

#ifndef _countof
#define _countof(_X_)   (sizeof(_X_)/sizeof((_X_)[0]))
#endif

#define DTOKENPASTE(X,Y)    X##Y
#define DTOKENPASTE2(X,Y)   DTOKENPASTE(X,Y)

#define DUNREFPARAM(_P_)    (void)(_P_)

#define D_UNALIGNED_MEM_ACCESS

//===============================================================
#if defined(_MSC_VER)
# define DFORCEINLINE __forceinline
#else
# if defined(DEBUG)
#  define DFORCEINLINE inline
# else
#  define DFORCEINLINE inline __attribute__((always_inline))
# endif
#endif

//===============================================================
using DStr = std::string;

#ifdef _MSC_VER
inline int strcasecmp( const char *a, const char *b )
{
	return _stricmp( a, b );
}

inline int strncasecmp( const char *a, const char *b, size_t len )
{
	return _strnicmp( a, b, len );
}

# define strtok_r	strtok_s

#endif

//==================================================================
class DStrRef
{
    const char *mpStr {};

public:
    constexpr DStrRef() = default;
    constexpr DStrRef( const char *pStr ) : mpStr(pStr) {}
              DStrRef( const DStr &str )  : mpStr(str.c_str()) {}

    operator DStr() const        { return mpStr; }

    constexpr const char *c_str() const { return mpStr; }

    constexpr bool empty() const { return !mpStr || !mpStr[0]; }
};

inline int strcasecmp( DStrRef a, DStrRef b )
{
	return strcasecmp( a.c_str(), b.c_str() );
}

//===============================================================
#if defined(_MSC_VER)
# define SIZE_T_FMT	"%Id"
#else
# define SIZE_T_FMT	"%zd"
#endif

//
#define DNEW               new
#define DSAFE_DELETE(_X_)  { if ( _X_ ) { delete (_X_); (_X_) = 0; } }

//
#include "DAssert.h"

#endif

