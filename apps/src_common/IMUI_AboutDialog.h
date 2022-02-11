//==================================================================
/// IMUI_AboutDialog.h
///
/// Created by Davide Pasca - 2019/12/29
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMUI_ABOUTDIALOG_H
#define IMUI_ABOUTDIALOG_H

#include "DBase.h"

//==================================================================
struct IMUI_AboutDialogParams
{
    DStr        appName     ;
    DStr        appLongName ;
    DStr        appVersion  ;
    DStr        dispURL     ;
    DStr        fullURL     ;
    bool        *pOpen {};
    DStr        copyrightText;
    uint32_t    val_Year  {};
    uint32_t    val_Month {};
    uint32_t    val_Day   {};
    uint32_t    val_Hours {};
    uint32_t    val_Mins  {};
    bool        showSmall = false;
};

void IMUI_AboutDialog( const IMUI_AboutDialogParams &par );

#endif

