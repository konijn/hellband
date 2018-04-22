/* File: zborg6.h */

/* Purpose: Header file for "borg6.c" -BEN- */

#ifndef INCLUDED_BORG6_H
#define INCLUDED_BORG6_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg6.c".
 */

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg7.h"

/*
 * Possible values of "goal"
 */
#define GOAL_KILL    1  /* Monsters */
#define GOAL_TAKE    2  /* Objects */
#define GOAL_MISC    3  /* Stores */
#define GOAL_DARK    4  /* Exploring */
#define GOAL_XTRA    5  /* Searching */
#define GOAL_BORE    6  /* Leaving */
#define GOAL_FLEE    7  /* Fleeing */
#define GOAL_TOWN    8  /* Town Special Grid */
#define GOAL_VAULT   9  /* Vaults */
#define GOAL_RECOVER 10 /* Safe grid for me to rest on */
#define GOAL_DIGGING 11 /* Anti-summon Corridor */
#define GOAL_UNREACH 12 /* Move to a safer grid so monsters cant hit him */
#define GOAL_LAST    13 /* All max goal checks should use this */

extern cptr goal_descriptions[GOAL_LAST+1];

/*
 * Minimum "harmless" food
 */

#define SV_FOOD_MIN_OKAY    SV_FOOD_CURE_POISON

/*
 * Attempt to induce "word of recall"
 */
extern bool borg_recall(void);

/*
 * Low level goals
 */
extern bool borg_caution(void);
extern int borg_attack(bool boosted_bravery, bool inflate, int specific, bool full_simulate);  /* Inflating the value adds the danger of the monster to the result */
extern bool borg_recover(void);
extern bool borg_surrounded(void);
extern bool borg_surrounded_breeder(void);

extern bool borg_eat_food_any(void);
extern bool borg_eat_vamp(void);
extern bool borg_offset_ball(void);
extern bool borg_defend(int p);
extern bool borg_perma_spell(void);

extern bool borg_check_rest(int y, int x);
extern bool borg_on_safe_grid(int y, int x);
extern int borg_near_monster_type(int dist);
extern bool borg_happy_grid_bold(int y, int x);

/*
 * Twitchy goals
 */
extern bool borg_charge_kill(void);
extern bool borg_charge_take(void);
extern bool borg_twitchy(void);

/*
 * Continue a high level goal
 */
extern bool borg_flow_old(int why);

/*
 * Flow to stairs
 */
extern bool borg_flow_stair_both(int why, bool sneak, bool prep_check);
extern bool borg_flow_stair_both_dim(/*int why*/);
extern bool borg_flow_stair_less(int why, bool sneak);
extern bool borg_flow_stair_less_dim(/*int why*/);
extern bool borg_flow_stair_more(int why, bool sneak, bool prep_check);
extern bool borg_flow_stair_more_dim(/*int why*/);
extern bool borg_flow_town_exit(int why);
extern bool borg_flow_demenager(int town_num, int why);
extern bool borg_flow_glyph(/*int why*/);
extern bool borg_flow_light(int why);
extern bool borg_flow_recover(/*bool viewable,*/ int dist);
extern bool borg_flow_passwall(void);
extern bool borg_check_lite_only(void);
extern bool borg_backup_swap(int p);

/*
 * Flow to shops
 */
extern bool borg_flow_shop_visit(void);
extern bool borg_flow_shop_entry(int n);
extern void borg_flow_direct(int y, int x);
extern bool borg_flow_shop_special(bool first);
extern int borg_net_mutations(void);
extern bool borg_flow_shop_inn(void);
extern bool borg_flow_shop_trump(void);

/*
 * Flow towards monsters/objects
 */
extern bool borg_flow_kill(bool viewable, int nearness);
extern bool borg_flow_kill_aim(bool viewable);
extern bool borg_flow_kill_corridor_1(/*bool viewable*/);
extern bool borg_flow_kill_corridor_2(/*bool viewable*/);
extern bool borg_flow_kill_unreachable(int nearness);
extern bool borg_flow_kill_direct(/*bool viewable*/);
extern bool borg_flow_take(bool viewable, int nearness);
extern bool borg_flow_take_lunal(bool viewable, int nearness);
extern bool borg_flow_vein(bool viewable, int nearness);
extern void borg_flow_direct_dig(int m_y, int m_x);
extern bool borg_flow_take_scum(bool viewable, int nearness);
extern bool borg_flow_vault(int nearness);
extern bool borg_excavate_vault(int nearness);
extern bool borg_excavate_region(int nearness);
extern bool borg_lite_beam(bool simulation);

/*
 * Flow towards "interesting" grids
 */
extern bool borg_flow_dark(bool neer);
extern bool borg_flow_dark_interesting(int y, int x);
extern bool borg_flow_dark_reachable(int y, int x);

/*
 * Search for secret doors
 */
extern bool borg_flow_spastic(bool bored);

extern bool borg_target(int y, int x);
extern int borg_launch_damage_one(int i, int dam, int typ, bool inflate, int ammo_location);
extern int borg_attack_aux_thrust(bool inflate, int specific);
extern int borg_thrust_damage_one(int i, bool inflate);
extern bool borg_heal(int danger);
extern bool borg_munchkin_magic(void);
extern bool borg_munchkin_melee(void);
/* extern bool borg_munchkin_mutation(void); */

extern void borg_log_battle(bool);
extern bool borg_target_unknown_wall(int g_y,int g_x);

/*
 * Initialize this file
 */
extern void borg_init_6(void);

#endif

#endif
