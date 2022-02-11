//==================================================================
/// DAssert.h
///
/// Created by Davide Pasca - 2018/03/15
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DASSERT_H
#define DASSERT_H

//==================================================================
#if defined(_DEBUG) || defined(DEBUG)

# if defined(_MSC_VER)
#  define DANALYSIS_ASSUME(_X_) __analysis_assume(_X_)
#  define DANALYZER_NORETURN
# elif !defined(ANDROID) && !defined(__linux__)
#  define DANALYSIS_ASSUME(_X_) ((void)0)
#  define DANALYZER_NORETURN __attribute__((analyzer_noreturn))
# else
#  define DANALYSIS_ASSUME(_X_) ((void)0)
#  define DANALYZER_NORETURN
# endif

# define DASSERT(_X_)                                       \
    {                                                       \
        bool ok=(bool)(_X_);                                \
        DANALYSIS_ASSUME( ok );                             \
        if ( !ok )                                          \
            DAssert( ok, __FILE__, __LINE__, #_X_ );   \
    }

# define DASSERT_LOC(_X_,_F_,_L_)                           \
    {                                                       \
        DANALYSIS_ASSUME( _X_ );                            \
        bool ok=(_X_);                                      \
        if ( !ok )                                          \
            DAssert( ok, (_F_), (_L_), #_X_ );         \
    }

#else

# define DASSERT(_X_) ((void)0)
# define DASSERT_LOC(_X_,_F_,_L_) ((void)0)
# define DANALYZER_NORETURN

#endif

void DAssert( bool ok, const char *pFile, int line, const char *msg = 0 ) DANALYZER_NORETURN;

#endif

