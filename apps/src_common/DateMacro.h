//==================================================================
/// DateMacro.h
///
/// Created by Davide Pasca - 2015/5/27
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DATEMACRO_H
#define DATEMACRO_H

#define DM_YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define DM_MONTH (__DATE__ [1] == 'a' && __DATE__ [2] == 'n' ? 1 \
    : __DATE__ [2] == 'b' ? 2 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
    : __DATE__ [2] == 'y' ? 5 \
    : __DATE__ [2] == 'n' ? 6 \
    : __DATE__ [2] == 'l' ? 7 \
    : __DATE__ [2] == 'g' ? 8 \
    : __DATE__ [2] == 'p' ? 9 \
    : __DATE__ [2] == 't' ? 10 \
    : __DATE__ [2] == 'v' ? 11 : 12)

#define DM_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))

#define DATE_AS_INT (((DM_YEAR - 2000) * 12 + DM_MONTH) * 31 + DM_DAY)

#define DM_HOURS ((__TIME__[0]-'0') * 10 + (__TIME__[1]-'0'))
#define DM_MINS  ((__TIME__[3]-'0') * 10 + (__TIME__[4]-'0'))

#endif

