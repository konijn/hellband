/* File: zborg8.h */

/* Purpose: Header file for "borg8.c" -BEN- */

#ifndef INCLUDED_BORG8_H
#define INCLUDED_BORG8_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg8.c".
 */

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg6.h"
#include "zborg7.h"


/*
 * Think about the stores
 */
extern bool borg_think_store(void);
extern bool borg_think_shop_restoration(void);
extern bool borg_think_shop_thieves(void);
extern bool borg_think_shop_library(void);
extern bool borg_think_shop_sorcery(void);
extern bool borg_think_shop_temple(void);
extern bool borg_think_shop_bow(void);
extern bool borg_think_shop_fighter(void);
extern bool borg_think_shop_paladin(void);
extern bool borg_think_shop_mutation(void);
extern bool borg_think_shop_inn(void);
extern bool borg_think_shop_trump(void);

extern bool borg_caution_phase(int, int);
extern bool borg_lite_beam(bool simulation);
/*
 * Think about the dungeon
 */
extern bool borg_think_dungeon(void);


/*
 * Initialize this file
 */
extern void borg_init_8(void);


#endif

#endif

