/* File: zborg7.h */

/* Purpose: Header file for "borg7.c" -BEN- */

#ifndef INCLUDED_BORG7_H
#define INCLUDED_BORG7_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg7.c".
 */

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"


/*
 * Determine if an item is "icky"
 */
extern bool borg_item_icky(borg_item *item);

/*
 * Various functions
 */
extern bool borg_use_things(void);
extern bool borg_check_lite(void);
extern bool borg_check_lite_only(void);

extern bool borg_enchanting(void);
extern bool borg_recharging(void);
extern bool borg_crush_junk(void);
extern bool borg_crush_hole(void);
extern bool borg_crush_slow(void);
extern bool borg_pseudoid_stuff(void);
extern bool borg_test_stuff(bool star_id);
extern bool borg_takeoff_stuff(void);
extern bool borg_swap_rings(void);
extern bool borg_wear_rings(void);
extern bool borg_wear_stuff(void);
extern bool borg_wear_swap(void);
extern bool borg_best_stuff(void);
extern bool borg_best_combo(bool shops_too);
extern bool borg_play_magic(bool bored);
extern bool borg_remove_stuff(void);
extern bool borg_wear_recharge(void);
extern int borg_count_sell(void);
extern bool borg_eat_magic(bool cursed_only, int fail_allowed);
extern bool borg_starid_item(borg_item *item);

/*
 * Attempt to leave the level
 */
extern bool borg_leave_level(bool bored);


/*
 * Initialize this file
 */
extern void borg_init_7(void);


#endif

#endif

