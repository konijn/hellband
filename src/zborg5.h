/* File: zborg5.h */
/* Purpose: Header file for "borg5.c" -BEN- */

#ifndef INCLUDED_BORG5_H
#define INCLUDED_BORG5_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg5.c".
 */

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg6.h"

/*
 * Update state based on current "map"
 */
extern void borg_update(void);


/*
 * React to various "important" messages
 */
extern void borg_react(cptr msg, cptr buf);
extern void borg_delete_kill(int i);
extern void borg_relocate_kill(int i);


/*
 * Initialize this file
 */
extern void borg_init_5(void);




#endif

#endif

