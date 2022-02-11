//==================================================================
/// IMUI_EditableListBox.h
///
/// Created by Davide Pasca - 2021/09/06
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef IMUI_EDITABLELISTBOX_H
#define IMUI_EDITABLELISTBOX_H

#ifdef ENABLE_IMGUI

#include "DBase.h"
#include "DContainers.h"

//==================================================================
void IMUI_EditableListBox(
        const char *pTitle,
        DStr &edit, DVec<DStr> &vals,
        const DFun<bool (DStr&)> &onEditFn={} );

#endif

#endif

