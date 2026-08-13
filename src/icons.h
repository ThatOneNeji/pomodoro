/**
 * @file icons.h
 * @brief Declares the Icon instances whose bitmap data is generated into src/icons/icons.cpp by scripts/gen_assets.py.
 */
#ifndef ICONS_H
#define ICONS_H

#include "icon.h"

extern Icon icon_email;       ///< Emails preset icon (normal set).
extern Icon icon_warning;     ///< Warning/alert icon.
extern Icon icon_focus;       ///< Focus preset icon (normal set).
extern Icon icon_coffee;      ///< Coffee/break icon.
extern Icon icon_lpetantrum;  ///< Emails preset icon ("LPE" set).
extern Icon icon_lpehacker;   ///< Coding preset icon ("LPE" set); also the fallback icon.
extern Icon icon_checkmark;   ///< Checkmark, used to render a checked Checkbox.
extern Icon icon_lpenote;     ///< Note icon ("LPE" set).
extern Icon icon_lpethink;    ///< Focus preset icon ("LPE" set).
extern Icon icon_lpesip;      ///< Sip/drink icon ("LPE" set).
extern Icon icon_coding;      ///< Coding preset icon (normal set).

#endif  // ICONS_H
