/* File: borg6.c */
/* Purpose: Medium level stuff for the Borg -BEN-
 * Fighting, Defending, Moving through the dungeon
 */

#include "angband.h"

#ifdef BORG_TK
#include "tnb.h"
#endif /* BORG_TK */

#ifdef ALLOW_BORG

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg4.h"
#include "zborg5.h"
#include "zborg6.h"

static bool borg_desperate = FALSE;

/*
 * This file is responsible for the low level dungeon goals.
 *
 * This includes calculating the danger from monsters, determining
 * how and when to attack monsters, and calculating "flow" paths
 * from place to place for various reasons.
 *
 * Notes:
 *   We assume that invisible/offscreen monsters are dangerous
 *   We consider physical attacks, missile attacks, spell attacks,
 *     wand attacks, etc, as variations on a single theme.
 *   We take account of monster resistances and susceptibilities
 *   We try not to wake up sleeping monsters by throwing things
 *
 *
 * Bugs:
 *   Currently the "twitchy()" function is not very smart
 *   We get "twitchy" when we are afraid of the monsters
 *   Annoyance and Danger are very different things (!)
 */

/*
 * New method for handling attacks, missiles, and spells
 *
 * Every turn, we evaluate every known method of causing damage
 * to monsters, and evaluate the "reward" inherent in each of
 * the known methods which is usable at that time, and then
 * we actually use whichever method, if any, scores highest.
 *
 * For each attack, we need a function which will determine the best
 * possible result of using that attack, and return its value.  Also,
 * if requested, the function should actually perform the action.
 *
 * Note that the functions should return zero if the action is not
 * usable, or if the action is not useful.
 *
 * These functions need to apply some form of "cost" evaluation, to
 * prevent the use of expensive spells with minimal reward.  Also,
 * we should always prefer attacking by hand to using spells if the
 * damage difference is "small", since there is no "cost" in making
 * a physical attack.
 *
 * We should take account of "spell failure", as well as "missile
 * missing" and "blow missing" probabilities.
 *
 * Note that the functions may store local state information when
 * doing a "simulation" and then they can use this information if
 * they are asked to implement their strategy.
 *
 * There are several types of damage inducers:
 *
 *   Attacking physically
 *   Launching missiles
 *   Throwing objects
 *   Casting spells
 *   Praying prayers
 *   Using wands
 *   Using rods
 *   Using staffs
 *   Using scrolls
 *   Activating Artifacts
 *   Activate Dragon Armour
 *   etc.
 */
enum {
	BF_THRUST,
	BF_REST,
	BF_OBJECT,

	BF_LAUNCH_NORMAL,
	BF_LAUNCH_SEEKER,
	BF_LAUNCH_EGO,

	BF_LIFE_CALL_LIGHT,
	BF_LIFE_HOLY_ORB,
	BF_LIFE_EXORCISM,
	BF_LIFE_DISP_UNDEAD,
	BF_LIFE_DISP_EVIL,
	BF_LIFE_HOLY_WORD,
	BF_LIFE_DIVINE_INT,
	BF_LIFE_DAY_DOVE,

	BF_ARCANE_ZAP,
	BF_ARCANE_ZAP_RESERVE,
	BF_ARCANE_LAREA,
	BF_ARCANE_STONEMUD,
	BF_ARCANE_LBEAM,
	BF_ARCANE_ELEM_BALL,

	BF_SORC_LAREA,
	BF_SORC_CONF_MON,
	BF_SORC_SLEEP_I,
	BF_SORC_SLOW_MON,
	BF_SORC_SLEEP_III,
	BF_SORC_STASIS,

	BF_NATURE_DAYL,
	BF_NATURE_STONEMUD,
	BF_NATURE_ELECBOLT,
	BF_NATURE_FROSTBOLT,
	BF_NATURE_SUNL,
	BF_NATURE_TAME,
	BF_NATURE_ENTANGLE,
	BF_NATURE_WHIRLWIND,
	BF_NATURE_BLIZZARD,
	BF_NATURE_ELECSTORM,
	BF_NATURE_WHIRLPOOL,
	BF_NATURE_SUNLIGHT,
	BF_NATURE_NATWRATH,

	BF_TRUMP_MINDBLAST,
	BF_TRUMP_MINDBLAST_RESERVE,

	BF_CHAOS_MMISSILE,
	BF_CHAOS_MMISSILE_RESERVE,
	BF_CHAOS_FLASHLIGHT,
	BF_CHAOS_MANABURST,
	BF_CHAOS_FIREBOLT,
	BF_CHAOS_FISTFORCE,
	BF_CHAOS_CHAOSBOLT,
	BF_CHAOS_SONICBOOM,
	BF_CHAOS_DOOMBOLT,
	BF_CHAOS_FIREBALL,
	BF_CHAOS_INVOKELOGRUS,
	BF_CHAOS_POLYMORPH,
	BF_CHAOS_CHAINLIGHT,
	BF_CHAOS_DISINTEG,
	BF_CHAOS_GRAVITY,
	BF_CHAOS_METEORSWARM,
	BF_CHAOS_FLAMESTRIKE,
	BF_CHAOS_ROCKET,
	BF_CHAOS_MANASTORM,
	BF_CHAOS_BRLOGRUS,
	BF_CHAOS_CALLVOID,

	BF_DEATH_MALEDICTION,
	BF_DEATH_MALEDICTION_RESERVE,
	BF_DEATH_STINKCLOUD,
	BF_DEATH_SLEEP_I,
	BF_DEATH_HORRIFY,
	BF_DEATH_ENSLAVE_UNDEAD,
	BF_DEATH_ENTROPY,
	BF_DEATH_NETHERBOLT,
	BF_DEATH_TERROR,
	BF_DEATH_VAMPDRAIN,
	BF_DEATH_DISPELGOOD,
	BF_DEATH_DARKBOLT,
	BF_DEATH_VAMPIRISM,
	BF_DEATH_DARKNESS,
	BF_DEATH_DEATHRAY,
	BF_DEATH_WORDOFDEATH,
	BF_DEATH_EVOCATION,
	BF_DEATH_HELLFIRE,

	BF_MIND_NEURAL,
	BF_MIND_DOMINATION,
	BF_MIND_PULVERISE,
	BF_MIND_MINDWAVE,
	BF_MIND_PSYCH_DRAIN,
	BF_MIND_TELE_WAVE,

	BF_ROD_ELEC_BOLT,
	BF_ROD_COLD_BOLT,
	BF_ROD_ACID_BOLT,
	BF_ROD_FIRE_BOLT,
	BF_ROD_LITE_BEAM,
	BF_ROD_DRAIN_LIFE,
	BF_ROD_ELEC_BALL,
	BF_ROD_COLD_BALL,
	BF_ROD_ACID_BALL,
	BF_ROD_FIRE_BALL,
	BF_ROD_SLOW_MONSTER,
	BF_ROD_SLEEP_MONSTER,
	BF_ROD_UNKNOWN,

	BF_STAFF_SLEEP_MONSTERS,
	BF_STAFF_SLOW_MONSTERS,
	BF_STAFF_DISPEL_EVIL,
	BF_STAFF_POWER,
	BF_STAFF_HOLINESS,

	BF_WAND_UNKNOWN,
	BF_WAND_MAGIC_MISSILE,
	BF_WAND_CHARM_MONSTER,
	BF_WAND_COLD_BOLT,
	BF_WAND_ACID_BOLT,
	BF_WAND_FIRE_BOLT,
	BF_WAND_SLOW_MONSTER,
	BF_WAND_SLEEP_MONSTER,
	BF_WAND_CONFUSE_MONSTER,
	BF_WAND_FEAR_MONSTER,
	BF_WAND_ANNIHILATION,
	BF_WAND_DRAIN_LIFE,
	BF_WAND_LITE_BEAM,
	BF_WAND_STINKING_CLOUD,
	BF_WAND_ELEC_BALL,
	BF_WAND_COLD_BALL,
	BF_WAND_ACID_BALL,
	BF_WAND_FIRE_BALL,
	BF_WAND_WONDER,
	BF_WAND_DRAGON_COLD,
	BF_WAND_DRAGON_FIRE,
	BF_WAND_ROCKETS,

	BF_ART_DAGGER_INFERNO,
	BF_ART_COCYTUS,
	BF_ART_DAGGER_FURCIFER,
	BF_ART_KINSLAYER,
	BF_ART_FROST,
	BF_ART_ARUNRUTH,
	BF_ART_MICHAEL,
	BF_ART_RASHAVERAK,
	BF_ART_ELIGOR,
	/*BF_ART_AEGLOS,*/
	BF_ART_PHENEX,
	BF_ART_MORNINGSTAR,
	BF_ART_JUSTICE,
	BF_ART_BAPHOMET,
	BF_ART_ASMODAI,
	BF_ART_LIGHT,
	BF_ART_IRONFIST,
	BF_ART_GHOULS,
	BF_ART_SIMPLE,
	BF_ART_FURFICER,
	BF_ART_DEAD,
	/*BF_ART_FINGOLFIN,*/
	BF_ART_AMULET_MICHAEL,
	BF_ART_AMULET_RAPHAEL,
	BF_ART_RING_RAPHAEL,
	BF_ART_RING_MICHAEL,
	BF_ART_EMMANUEL,

	BF_DRAGON_BLUE,
	BF_DRAGON_WHITE,
	BF_DRAGON_BLACK,
	BF_DRAGON_GREEN,
	BF_DRAGON_RED,
	BF_DRAGON_MULTIHUED,
	BF_DRAGON_BRONZE,
	BF_DRAGON_GOLD,
	BF_DRAGON_CHAOS,
	BF_DRAGON_LAW,
	BF_DRAGON_BALANCE,
	BF_DRAGON_SHINING,
	BF_DRAGON_POWER,

	BF_RACIAL_VAMP,
	/*BF_RACIAL_CYCLOPS,*/
	BF_RACIAL_DARK_ELF,
	BF_RACIAL_DRACONIAN,
	BF_RACIAL_IMP,
	BF_RACIAL_KLACKON,
	BF_RACIAL_KOBOLD,
	BF_RACIAL_MINDFLAYER,
	BF_RACIAL_SPRITE,
	/*BF_RACIAL_YEEK,*/
	BF_RACIAL_HALFGIANT,

	BF_COR1_SPIT_ACID,
	BF_COR1_BR_FIRE,
	BF_COR1_HYPN_GAZE,
	BF_COR1_MIND_BLST,
	/*BF_COR1_RADIATION,*/
	BF_COR1_VAMPIRISM,
	BF_COR1_SHRIEK,
	BF_COR1_ILLUMINE,
	BF_COR1_PANIC_HIT,
	BF_COR1_DAZZLE,
	/*	BF_COR1_LASER_EYE,*/
	BF_COR1_BANISH, /* Evil non-unique, non-questor */
	BF_COR1_COLD_TOUCH,

	BF_RING_FLAMES,
	BF_RING_ICE,
	BF_RING_ACID,

	BF_ACT_SUNLIGHT,
	BF_ACT_BO_MISS_1,
	BF_ACT_BA_POIS_1,
	BF_ACT_BO_ELEC_1,
	BF_ACT_BO_ACID_1,
	BF_ACT_BO_COLD_1,
	BF_ACT_BO_FIRE_1,
	BF_ACT_BA_COLD_1,
	BF_ACT_BA_FIRE_1,
	BF_ACT_DRAIN_1,
	BF_ACT_BA_COLD_2,
	BF_ACT_BA_ELEC_2,
	BF_ACT_DRAIN_2,
	BF_ACT_VAMPIRE_1,
	BF_ACT_BO_MISS_2,
	BF_ACT_BA_FIRE_2,
	BF_ACT_BA_COLD_3,
	BF_ACT_BA_ELEC_3,
	BF_ACT_WHIRLWIND,
	BF_ACT_VAMPIRE_2,
	/* BF_ACT_CALL_CHAOS, */
	/*BF_ACT_ROCKET,*/
	BF_ACT_DISP_EVIL,
	BF_ACT_BA_MISS_3,
	BF_ACT_DISP_GOOD,
	BF_ACT_CONFUSE,
	BF_ACT_SLEEP,
	BF_ACT_CHARM_ANIMAL,
	BF_ACT_CHARM_UNDEAD,
	BF_ACT_CHARM_OTHER,
	BF_ACT_CHARM_ANIMALS,
	BF_ACT_CHARM_OTHERS,
	BF_ACT_SUMMON_ANIMAL,
	BF_ACT_SUMMON_PHANTOM,
	BF_ACT_SUMMON_ELEMENTAL,
	BF_ACT_SUMMON_DEMON,
	BF_ACT_SUMMON_UNDEAD,

	BF_MAX
};

/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * Note that we use "diagonal" motion whenever possible.
 *
 * We return "5" if no motion is needed.
 */
static int borg_extract_dir(int y1, int x1, int y2, int x2) {
	/* No movement required */
	if ((y1 == y2) && (x1 == x2))
		return (5);

	/* South or North */
	if (x1 == x2)
		return ((y1 < y2) ? 2 : 8);

	/* East or West */
	if (y1 == y2)
		return ((x1 < x2) ? 6 : 4);

	/* South-east or South-west */
	if (y1 < y2)
		return ((x1 < x2) ? 3 : 1);

	/* North-east or North-west */
	if (y1 > y2)
		return ((x1 < x2) ? 9 : 7);

	/* Paranoia */
	return (5);
}

/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * We prefer "non-diagonal" motion, which allows us to save the
 * "diagonal" moves for avoiding pillars and other obstacles.
 *
 * If no "obvious" path is available, we use "borg_extract_dir()".
 *
 * We return "5" if no motion is needed.
 */
static int borg_goto_dir(int y1, int x1, int y2, int x2) {
	int d, e;

	int ay = (y2 > y1) ? (y2 - y1) : (y1 - y2);
	int ax = (x2 > x1) ? (x2 - x1) : (x1 - x2);

	/* If Passwalling, use the other version of the function */
	if (borg_skill[BI_PASSWALL])
		return (borg_extract_dir(y1, x1, y2, x2));

	/* Default direction */
	e = borg_extract_dir(y1, x1, y2, x2);

	/* Adjacent location, use default */
	if ((ay <= 1) && (ax <= 1))
		return (e);

	/* Try south/north (primary) */
	if (ay > ax) {
		d = (y1 < y2) ? 2 : 8;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Try east/west (primary) */
	if (ay < ax) {
		d = (x1 < x2) ? 6 : 4;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Try diagonal */
	d = borg_extract_dir(y1, x1, y2, x2);

	/* Check for walls */
	if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
		return (d);

	/* Try south/north (secondary) */
	if (ay <= ax) {
		d = (y1 < y2) ? 2 : 8;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Try east/west (secondary) */
	if (ay >= ax) {
		d = (x1 < x2) ? 6 : 4;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Circle obstacles */
	if (!ay) {
		/* Circle to the south */
		d = (x1 < x2) ? 3 : 1;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);

		/* Circle to the north */
		d = (x1 < x2) ? 9 : 7;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Circle obstacles */
	if (!ax) {
		/* Circle to the east */
		d = (y1 < y2) ? 3 : 9;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);

		/* Circle to the west */
		d = (y1 < y2) ? 1 : 7;
		if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
			return (d);
	}

	/* Oops */
	return (e);
}

static int borg_passwall_dir(int y1, int x1, int y2, int x2) {
	int d, e;

	int ay = (y2 > y1) ? (y2 - y1) : (y1 - y2);
	int ax = (x2 > x1) ? (x2 - x1) : (x1 - x2);

	/* Default direction */
	e = borg_extract_dir(y1, x1, y2, x2);

	/* Adjacent location, use default */
	if ((ay <= 1) && (ax <= 1) && !borg_grids[y1 + ddy[e]][x1 + ddx[e]].kill &&
		 borg_grids[y1 + ddy[e]][x1 + ddx[e]].feat < FEAT_PERM_INNER)
		return (e);

	/* Try south/north (primary) */
	if (ay > ax) {
		d = (y1 < y2) ? 2 : 8;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Try east/west (primary) */
	if (ay < ax) {
		d = (x1 < x2) ? 6 : 4;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Try diagonal */
	d = borg_extract_dir(y1, x1, y2, x2);

	/* Check for walls */
	if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
		 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
		return (d);

	/* Try south/north (secondary) */
	if (ay <= ax) {
		d = (y1 < y2) ? 2 : 8;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Try east/west (secondary) */
	if (ay >= ax) {
		d = (x1 < x2) ? 6 : 4;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Circle obstacles */
	if (!ay) {
		/* Circle to the south */
		d = (x1 < x2) ? 3 : 1;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);

		/* Circle to the north */
		d = (x1 < x2) ? 9 : 7;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Circle obstacles */
	if (!ax) {
		/* Circle to the east */
		d = (y1 < y2) ? 3 : 9;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);

		/* Circle to the west */
		d = (y1 < y2) ? 1 : 7;
		if (!borg_grids[y1 + ddy[d]][x1 + ddx[d]].kill &&
			 borg_grids[y1 + ddy[d]][x1 + ddx[d]].feat < FEAT_PERM_INNER)
			return (d);
	}

	/* Oops */
	return (e);
}

/*
 * Clear the "flow" information
 *
 * This function was once a major bottleneck, so we now use several
 * slightly bizarre, but highly optimized, memory copying methods.
 */
static void borg_flow_clear(void) {
	/* Reset the "cost" fields */
	COPY(borg_data_cost, borg_data_hard, borg_data);

	/* Wipe costs and danger */
	if (borg_danger_wipe) {
		/* Wipe the "know" flags */
		WIPE(borg_data_know, borg_data);

		/* Wipe the "icky" flags */
		WIPE(borg_data_icky, borg_data);

		/* Wipe complete */
		borg_danger_wipe = FALSE;
	}

	/* Start over */
	borg_flow_head = 0;
	borg_flow_tail = 0;
}

/*
 * Clear the "flow" information for monsters
 *
 */
static void borg_flow_clear_m(void) {
	/* Reset the "cost" fields */
	COPY(borg_data_cost_m, borg_data_hard_m, borg_data);

	/* Start over */
	borg_flow_head = 0;
	borg_flow_tail = 0;
}

/*
 * Spread a "flow" from the "destination" grids outwards
 *
 * We fill in the "cost" field of every grid that the player can
 * "reach" with the number of steps needed to reach that grid,
 * if the grid is "reachable", and otherwise, with "255", which
 * is the largest possible value that can be stored in a byte.
 *
 * Thus, certain grids which are actually "reachable" but only by
 * a path which is at least 255 steps in length will thus appear
 * to be "unreachable", but this is not a major concern.
 *
 * We use the "flow" array as a "circular queue", and thus we must
 * be careful not to allow the "queue" to "overflow".  This could
 * only happen with a large number of distinct destination points,
 * each several units away from every other destination point, and
 * in a dungeon with no walls and no dangerous monsters.  But this
 * is technically possible, so we must check for it just in case.
 *
 * We do not need a "priority queue" because the cost from grid to
 * grid is always "one" and we process them in order.  If we did
 * use a priority queue, this function might become unusably slow,
 * unless we reactivated the "room building" code.
 *
 * We handle both "walls" and "danger" by marking every grid which
 * is "impassible", due to either walls, or danger, as "ICKY", and
 * marking every grid which has been "checked" as "KNOW", allowing
 * us to only check the wall/danger status of any grid once.  This
 * provides some important optimization, since many "flows" can be
 * done before the "ICKY" and "KNOW" flags must be reset.
 *
 * Note that the "borg_enqueue_grid()" function should refuse to
 * enqueue "dangeous" destination grids, but does not need to set
 * the "KNOW" or "ICKY" flags, since having a "cost" field of zero
 * means that these grids will never be queued again.  In fact,
 * the "borg_enqueue_grid()" function can be used to enqueue grids
 * which are "walls", such as "doors" or "rubble".
 *
 * This function is extremely expensive, and is a major bottleneck
 * in the code, due more to internal processing than to the use of
 * the "borg_danger()" function, especially now that the use of the
 * "borg_danger()" function has been optimized several times.
 *
 * The "optimize" flag allows this function to stop as soon as it
 * finds any path which reaches the player, since in general we are
 * looking for paths to destination grids which the player can take,
 * and we can stop this function as soon as we find any usable path,
 * since it will always be as short a path as possible.
 *
 * We queue the "children" in reverse order, to allow any "diagonal"
 * neighbors to be processed first, since this may boost efficiency.
 *
 * Note that we should recalculate "danger", and reset all "flows"
 * if we notice that a wall has disappeared, and if one appears, we
 * must give it a maximal cost, and mark it as "icky", in case it
 * was currently included in any flow.
 *
 * If a "depth" is given, then the flow will only be spread to that
 * depth, note that the maximum legal value of "depth" is 250.
 *
 * "Avoid" flag means the borg will not move onto unknown grids,
 * nor to Monster grids if borg_desperate or borg_lunal_mode are
 * set.
 *
 * "Sneak" will have the borg avoid grids which are adjacent to a monster.
 *
 */
static void borg_flow_spread(int depth, bool optimize, bool avoid,
									  bool tunneling, int stair_idx, bool sneak) {
	int i;
	int n, nn = 0;
	int x1, y1;
	int x, y;
	int origin_y, origin_x;

	int ii, yy, xx;
	/*int fear = 0;*/
	bool bad_sneak = FALSE;
	bool twitchy = FALSE;
	bool use_dimdoor = FALSE;

	/* Default starting points */
	origin_y = c_y;
	origin_x = c_x;

	/* Is the borg moving under boosted bravery? */
	if (avoidance > borg_skill[BI_CURHP])
		twitchy = TRUE;

	/* Is the borg a starving vampire and able to use DimDoor to jump to a
	 * creature? */
	if (borg_skill[BI_ISWEAK] && borg_skill[BI_VAMPIRE] &&
		 borg_skill[BI_ADIMDOOR])
		use_dimdoor = TRUE;

	/* Use the closest stair for calculation distance (cost) from the stair to
	 * the goal */
	if (stair_idx >= 0 && borg_skill[BI_CLEVEL] < 15) {
		origin_y = track_less_y[stair_idx];
		origin_x = track_less_x[stair_idx];
		optimize = FALSE;
	}

	/* Set a goal grid, to be used elsewhere */
	borg_goal_y = borg_flow_y[borg_flow_tail];
	borg_goal_x = borg_flow_x[borg_flow_tail];

	/* Special case for Starving Vamps */
	if (use_dimdoor) {
		borg_kill *kill;
		monster_race *r_ptr;
		borg_grid *ag;

		y = borg_goal_y;
		x = borg_goal_x;

		/* Distance to the destination grid */
		n = distance(y, x, origin_y, origin_x);

		/* Skip non-warm blooded monsters */
		ag = &borg_grids[y][x];
		kill = &borg_kills[ag->kill];
		r_ptr = &r_info[kill->r_idx];

		/* Only do so if monster is warm blooded and within reach */
		if (ag->kill && monster_living(r_ptr) && borg_skill[BI_CLEVEL] >= n) {
			bool bad_spot = FALSE;

			/* Scan grids near the goal grid */
			for (i = 0; i < 8; i++) {
				/* Neighbor grid */
				x = borg_goal_x + ddx_ddd[i];
				y = borg_goal_y + ddy_ddd[i];

				/* only on legal grids (the monster i'm trying to drain */
				if (!in_bounds(y, x))
					continue;

				/**
				 * The borg will target the monster.  But the game will place the
				 *borg on a nearby suitable grid
				 * We need to check to make sure there is a suitable grid for us
				 **/

				/* Access the grid */
				ag = &borg_grids[y][x];

				/* Skip Icky Grids (vault type) */
				if (cave[y][x].info & (CAVE_ICKY))
					continue;

				/* Only certain ground types are unacceptable.  We permit the
				 * FEAT_NONE since warm blooded
				 * monsters would not be passwalling and by the time we can DimDoor,
				 * we would have Detect Walls. */
				if (ag->feat != FEAT_FLOOR && /*ag->feat != FEAT_SHAL_LAVA && */
					 ag->feat != FEAT_WATER && /* ag->feat != FEAT_GRASS &&*/
					 /*ag->feat != FEAT_DIRT &&*/ ag->feat != FEAT_NONE)
					continue;

				/* Skip monsters */
				if (ag->kill || cave[y][x].m_idx)
					continue;

				/* Skip items */
				if (ag->take)
					continue;

				/* Track the bad landing zones.
				 * I may have tried to jump onto this grid before and failed.
				 * Maybe there is an object, feature, or unknown monster on it.
				 */
				bad_spot = FALSE;
				for (ii = 0; ii < track_land_num; ii++) {
					bad_spot = FALSE;

					if (y == track_land_y[ii] && x == track_land_x[ii] &&
						 borg_t - track_land_when[ii] < 50)
						bad_spot = TRUE;
				}
				if (bad_spot == TRUE)
					continue;

				/* Save the flow cost as the distance to the grid */
				borg_data_cost->data[c_y][c_x] = n;

				/* All done here */
				borg_goal_y = y;
				borg_goal_x = x;
				return;

			} /* adjacent grids to target monster */
		}	 /* Living and close */
	}		  /* Use Dimdoor */

	/* Now process the queue starting with the goal and moving 1 grid away from
	 * the goal */
	while (borg_flow_head != borg_flow_tail) {
		/* Extract the next entry */
		x1 = borg_flow_x[borg_flow_tail];
		y1 = borg_flow_y[borg_flow_tail];

		/* Circular queue -- dequeue the next entry */
		if (++borg_flow_tail == AUTO_FLOW_MAX)
			borg_flow_tail = 0;

		/* Cost (one per movement grid) */
		n = borg_data_cost->data[y1][x1] + 1;

		/* New depth */
		if (n > nn) {
			/* Optimize (if requested) */
			if (optimize && (n > borg_data_cost->data[origin_y][origin_x]))
				break;

			/* Limit depth */
			if (n > depth)
				break;

			/* Save */
			nn = n;
		}

		/* Queue the "children" */
		for (i = 0; i < 8; i++) {
			int old_head;

			borg_grid *ag;

			/* reset bad_sneak */
			bad_sneak = FALSE;

			/* Neighbor grid */
			x = x1 + ddx_ddd[i];
			y = y1 + ddy_ddd[i];

			/* only on legal grids */
			if (!in_bounds(y, x))
				continue;

			/* Skip "reached" grids (Leash included)*/
			if (borg_data_cost->data[y][x] <= n)
				continue;

			/* Access the grid */
			ag = &borg_grids[y][x];

			/* No need to check the sneak on my own grid, I will be leaving this
			 * one */
			if (sneak && y != c_y && x != c_x) {
				/* Scan the neighbors */
				for (ii = 0; ii < 8; ii++) {
					/* Neighbor grid */
					xx = x + ddx_ddd[ii];
					yy = y + ddy_ddd[ii];

					/* only on legal grids */
					if (!in_bounds(yy, xx))
						continue;

					/* Make sure no monster is on this grid, which is
					 * adjacent to the grid on which I am thinking about stepping.
					 */
					if (borg_grids[yy][xx].kill &&
						 !borg_kills[borg_grids[yy][xx].kill].ally) {
						bad_sneak = TRUE;
						break;
					}
				}
			}

			/*
			 * Some of these grid checks are repeated in borg_flow_old.  If they
			 * get changed here,
			 * change them in borg_flow_old as well.
			 *
			 */

			/*
			 * Check monster movement:
			 * 1.  over certain terrains,
			 * 2.  randomness of movement,
			 * 3.  animals dont follow
			 * 4.  monster afraid (not done yet since it is a m_ptr, not r_ptr)
			 * 5.  strong player
			 */
			/* The grid I am thinking about is adjacent to a monster */
			if (sneak && bad_sneak && !borg_desperate && !twitchy)
				continue;

			/* Avoid "wall" grids (not doors) unless tunneling or a fleeing spectre
			 */
			if (!tunneling) {
				/* Fleeing Spectres */
				if ((goal == GOAL_FLEE || goal == GOAL_BORE || twitchy || sneak ||
					  (borg_grids[c_y][c_x].feat >= FEAT_DOOR_HEAD &&
						borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)) &&
					 borg_skill[BI_PASSWALL] && borg_skill[BI_CDEPTH] >= 1 &&
					 goal != GOAL_RECOVER) {
					/*** No PassWalling if monsters are going to harm the borg. ***/

					if (borg_skill[BI_CURHP] <= 0) {
						/* If the grid onto which I will move has an adjacent monster
							Allowed to flow through walls */
					}
				} else {
					if (borg_skill[BI_CLEVEL] < 10 && ag->feat >= FEAT_SECRET &&
						 ag->feat <= FEAT_WALL_SOLID)
						continue;
					if (borg_skill[BI_CLEVEL] >= 10 && ag->feat > FEAT_RUBBLE &&
						 ag->feat <= FEAT_WALL_SOLID)
						continue;
				}
			}

			/* Avoid difficult tunneling */
			if (tunneling) {
				if (borg_skill[BI_ASTONE2MUD] <= 0)
					if (borg_items[weapon_swap].tval != TV_DIGGING &&
						 borg_skill[BI_DIG] < BORG_DIG)
						if (borg_skill[BI_DIG] < BORG_DIG + 20)
							/* if (goal != GOAL_VAULT) */
							continue;
			}

			/* Avoid "perma-wall" grids */
			if (ag->feat >= FEAT_PERM_EXTRA && ag->feat <= FEAT_PERM_SOLID)
				continue;

			/* Avoid Lava
			if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
				continue;
			if ((ag->feat == FEAT_SHAL_LAVA) && !borg_skill[BI_IFIRE] &&
				 !borg_skill[BI_FEATH])
				continue;
			*/

			/* Avoid Water if dangerous */
			if (ag->feat == FEAT_WATER &&
				 (borg_skill[BI_ENCUMBERD] && !borg_skill[BI_FEATH]))
				continue;

			/* Note that if the borg is surrounded by water or lava and taking
			 * damage,
				* he can either tport/phase out or attempt to run out, taking a few
			 * hits
				* in the process.
				*/

			/* Avoid Mountains
			if (ag->feat == FEAT_MOUNTAIN)
				continue;
			if (ag->feat == FEAT_DARK_PIT && !borg_skill[BI_FEATH])
				continue;
			*/

			/* Avoid some other Zang Terrains */

			/* Avoid town doors unless seeking them directly */
			if (borg_skill[BI_CDEPTH] == 0 && (ag->feat >= FEAT_DOOR_HEAD) &&
				 (ag->feat <= FEAT_DOOR_HEAD + 0x07))
				continue;

			/* Avoid unknown grids (if requested or retreating) */
			if ((avoid || borg_desperate) && (ag->feat == FEAT_NONE))
				continue;

			/* Avoid Monsters if Desperate, lunal */
			if ((ag->kill && !borg_kills[ag->kill].ally) &&
				 (borg_desperate || borg_lunal_mode || borg_munchkin_mode))
				continue;

			/* Carefull when trying to flow through a pet, I will strike them. */
			if (ag->kill &&
				 (borg_kills[ag->kill].ally || borg_kills[ag->kill].neutral) &&
				 (borg_skill[BI_ISSTUN] || borg_skill[BI_ISIMAGE] ||
				  borg_skill[BI_ISCONFUSED] || borg_berserk))
				continue;

			/* Avoid Monsters if low level, unless twitchy */
			if ((ag->kill && !borg_kills[ag->kill].ally &&
				  !borg_kills[ag->kill].neutral) &&
				 avoidance <= borg_skill[BI_MAXHP] && borg_skill[BI_FOOD] >= 2 &&
				 borg_skill[BI_MAXCLEVEL] < 5)
				continue;

			/* Avoid Monsters if requested */
			if ((ag->kill && !borg_kills[ag->kill].ally &&
				  !borg_kills[ag->kill].neutral) &&
				 avoid)
				continue;

			/* Avoid Traps if low level-- unless brave or scaryguy. */
			if (ag->feat >= FEAT_TRAP_TRAPDOOR && ag->feat <= FEAT_TRAP_SLEEP &&
				 avoidance <= borg_skill[BI_CURHP] && !(borg_depth & DEPTH_SCARY)) {
				/* Do not disarm when you could end up dead */
				if (borg_skill[BI_CURHP] < 60)
					continue;

				/* Do not disarm when clumsy */
				if (borg_skill[BI_DIS] < 30 && borg_skill[BI_CLEVEL] < 20)
					continue;
				if (borg_skill[BI_DIS] < 45 && borg_skill[BI_CLEVEL] < 10)
					continue;
			}

			/* Ignore "icky" grids */
			if (borg_data_icky->data[y][x])
				continue;

			/* Analyze every grid once */
			if (!borg_data_know->data[y][x]) {
				int p;

				/* Mark as known */
				borg_data_know->data[y][x] = TRUE;

				if (!borg_desperate && !borg_lunal_mode && !borg_munchkin_mode &&
					 !borg_digging) {
					/* Get the danger */
					p = borg_danger(y, x, 1, TRUE);

					/* Dangerous grid */
					if (p > avoidance / 3) {
						/* Mark as icky */
						borg_data_icky->data[y][x] = TRUE;

						/* Ignore this grid */
						continue;
					}
				}
			}

			/* Save the flow cost */
			borg_data_cost->data[y][x] = n;

			/* Enqueue that entry */
			borg_flow_x[borg_flow_head] = x;
			borg_flow_y[borg_flow_head] = y;

			/* Circular queue -- memorize head */
			old_head = borg_flow_head;

			/* Circular queue -- insert with wrap */
			if (++borg_flow_head == AUTO_FLOW_MAX)
				borg_flow_head = 0;

			/* Circular queue -- handle overflow (badly) */
			if (borg_flow_head == borg_flow_tail)
				borg_flow_head = old_head;
		} /* Children Queue */
	}	 /* wend */

	/* Forget the flow info */
	borg_flow_head = borg_flow_tail = 0;
}

/*
 * Choose "logical" directions for monster movement
 */
static bool borg_get_moves_m(int m_idx) {

	int y, ay, x, ax;
	int move_val = 0;
	int mm[8], i, d;

	borg_kill *kill = &borg_kills[m_idx];
	monster_race *r_ptr = &r_info[kill->r_idx];
	borg_grid *ag = &borg_grids[kill->y][kill->x];
	/*cave_type *c_ptr = &cave[kill->y][kill->x];*/
	/*monster_type *m_ptr = &m_list[c_ptr->m_idx];*/

	/* Some monsters won't flow to player */
	if (!kill->awake)
		return (FALSE);
	if (kill->afraid)
		return (FALSE);
	if (r_ptr->flags1 & RF1_RAND_50)
		return (FALSE);
	if (r_ptr->flags3 & RF3_ANIMAL)
		return (FALSE);
	if (r_ptr->flags1 & RF1_NEVER_MOVE)
		return (FALSE);
	if (r_ptr->flags1 & RF1_FRIENDS)
		return (FALSE);
	if (r_ptr->level + 25 < borg_skill[BI_CLEVEL])
		return (FALSE);
#ifdef MONSTER_FLOW
	/* If flow_by_sound and _smell are off, then the borg has to be very close to
	 * the monsters before they chase him */
	if (!flow_by_sound && (kill->dist > r_ptr->aaf))
		return (FALSE);
#endif

	/* Hack -- Assume no movement */
	mm[0] = mm[1] = mm[2] = mm[3] = 0;
	mm[4] = mm[5] = mm[6] = mm[7] = 0;

	/* Extract the "pseudo-direction." Players walks 'this many' to get to
	 * monster */
	y = kill->y - c_y;
	x = kill->x - c_x;

	/* Extract the "absolute distances" */
	ax = ABS(x);
	ay = ABS(y);

	/* Do something weird */
	if (y < 0)
		move_val += 8;
	if (x > 0)
		move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1)) {
		move_val++;
		move_val++;
	} else if (ax > (ay << 1)) {
		move_val++;
	}

	/* Extract some directions */
	switch (move_val) {
	case 0:
		mm[0] = 9;
		if (ay > ax) {
			mm[1] = 8;
			mm[2] = 6;
			mm[3] = 7;
			mm[4] = 3;
		} else {
			mm[1] = 6;
			mm[2] = 8;
			mm[3] = 3;
			mm[4] = 7;
		}
		break;
	case 1:
	case 9:
		mm[0] = 6;
		if (y < 0) {
			mm[1] = 3;
			mm[2] = 9;
			mm[3] = 2;
			mm[4] = 8;
		} else {
			mm[1] = 9;
			mm[2] = 3;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 2:
	case 6:
		mm[0] = 8;
		if (x < 0) {
			mm[1] = 9;
			mm[2] = 7;
			mm[3] = 6;
			mm[4] = 4;
		} else {
			mm[1] = 7;
			mm[2] = 9;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 4:
		mm[0] = 7;
		if (ay > ax) {
			mm[1] = 8;
			mm[2] = 4;
			mm[3] = 9;
			mm[4] = 1;
		} else {
			mm[1] = 4;
			mm[2] = 8;
			mm[3] = 1;
			mm[4] = 9;
		}
		break;
	case 5:
	case 13:
		mm[0] = 4;
		if (y < 0) {
			mm[1] = 1;
			mm[2] = 7;
			mm[3] = 2;
			mm[4] = 8;
		} else {
			mm[1] = 7;
			mm[2] = 1;
			mm[3] = 8;
			mm[4] = 2;
		}
		break;
	case 8:
		mm[0] = 3;
		if (ay > ax) {
			mm[1] = 2;
			mm[2] = 6;
			mm[3] = 1;
			mm[4] = 9;
		} else {
			mm[1] = 6;
			mm[2] = 2;
			mm[3] = 9;
			mm[4] = 1;
		}
		break;
	case 10:
	case 14:
		mm[0] = 2;
		if (x < 0) {
			mm[1] = 3;
			mm[2] = 1;
			mm[3] = 6;
			mm[4] = 4;
		} else {
			mm[1] = 1;
			mm[2] = 3;
			mm[3] = 4;
			mm[4] = 6;
		}
		break;
	case 12:
		mm[0] = 1;
		if (ay > ax) {
			mm[1] = 2;
			mm[2] = 4;
			mm[3] = 3;
			mm[4] = 7;
		} else {
			mm[1] = 4;
			mm[2] = 2;
			mm[3] = 7;
			mm[4] = 3;
		}
		break;
	}

	/** Now check these grids to see if moves are valid **/
	for (i = 0; mm[i]; i++) {
		/* Get the direction */
		d = mm[i];

		/* Hack -- allow "randomized" motion */
		if (d == 5)
			d = ddd[rand_int(8)];

		/* Get the destination */
		y = kill->y + ddy[d];
		x = kill->x + ddx[d];

		/* Access that cave grid */
		ag = &borg_grids[y][x];

		/*
		* Check monster movement: (from melee2.c: get_moves())
		* 1.  over certain terrains,
		* 2.  randomness of movement,
		* 3.  animals dont follow (just like RF1_FRIENDS)
		* 4.  monster afraid (not done yet since it is a m_ptr, not r_ptr)
		* 5.  strong player
		* 6.  Can the monster push through another monster. (but not the origin
		* monster)
		* 7.  Flow is turned off.
		* 8.  Critters that are friends like to ambush so they tend to not be
		* considered.
		*/
		/*if (!monster_can_cross_terrain(ag->feat, r_ptr))
			continue;*/
		if (!can_go_monster(y, x, kill->r_idx))
			continue;
		if ((ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL) &&
			 !(r_ptr->flags2 & RF2_OPEN_DOOR) && !(r_ptr->flags2 & RF2_BASH_DOOR))
			continue;
		if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM_SOLID)
			continue;
		if ((ag->kill && y != kill->y && x != kill->x) &&
			 !(r_ptr->flags2 & RF2_MOVE_BODY) && !(r_ptr->flags2 & RF2_KILL_BODY))
			continue;
		if (ag->feat == FEAT_GLYPH)
			continue;
		if (ag->feat == FEAT_MINOR_GLYPH && !(r_ptr->flags1 & RF1_NEVER_BLOW)) {
			/* reasonable chance Break the ward */
			if (BREAK_MINOR_GLYPH > r_ptr->level * 2)
				continue;
		}

		/* Wants to move... */
		return (TRUE);
	}

	/* unable to make first move */
	return (FALSE);
}

/* Determine the flow for monsters, pretty close to the function for the borg.
 *
 * NON-FLOW Monster movement is based on get_moves() from melee2.c
 * FLOW Monster movement is based on get_moves_aux() from melee2.c
 *
 * In get_moves() each monster is given a list of 5 directions in mm[].  The
 * game
 * attempts to have the monster move in those directions.  If mm[0] is not
 * valid,
 * mm[1] is selected, and so on until the last mm[4] is reached.  If no movement
 * is
 * valid, then the monster tries spells (process_monster()).
 *
 * In get_moves_aux(), there is a basic flow structure enabled.  But it is
 * conditional
 * on flow_by_sound.  If flow_by_sound is off, then no flow will happen.
 * If flow_by_sound and flow_by_smell are both on, then the monsters are very
 * good
 * at tracking the player down.  The distance-to-player must be less than
 * r_ptr->aaf.
 * The flow adds about 16 grids to the players location, simulating the player
 * being
 * further away.  This allows the mm[] array greater flexibility when it will be
 * populated in gets_moves() which is called immediately afterwards.  This
 * allows
 * monsters to navigate better and find the player a little easier.
 *
 * This flow routine will be divided into two parts.  The first will check to
 * see if
 * the game is doing the full flow or the short flow.  Then it will simulate
 * monster
 * movement similar to the way the borg does it.
 */
static void borg_flow_spread_m(int depth, int i, s16b race) {
	int n, nn = 0;
	int x1, y1;
	int x, y;
	/*int origin_x, origin_y;*/

	int ii /*, yy, xx*/;
	/*int fear = 0;*/
	bool flow_short = TRUE;
	bool flow_long = FALSE;

	borg_kill *kill = &borg_kills[i];
	monster_race *r_ptr = &r_info[race];

/*origin_x = borg_kills[i].x;
origin_y = borg_kills[i].y;*/

/* is the game doing short or long flow? */

#ifdef MONSTER_FLOW
	if (flow_by_sound) {
		flow_long = TRUE;
		flow_short = FALSE;
	} else {
		flow_long = FALSE;
		flow_short = TRUE;
	}
#else
	flow_long = FALSE;
	flow_short = TRUE;
#endif

	/* Short flow, test for the first grid mm[] to see if valid.
	 * If none are valid, then the monster wont flow to the borg.
	 */
	if (flow_short && !flow_long)
		flow_long = borg_get_moves_m(i);

#ifndef MONSTER_FLOW
	flow_long = FALSE;
	if (!flow_by_sound)
		flow_long = FALSE;
#endif

	/* Now process the queue starting with the goal and moving 1 grid away from
	 * the goal */
	while (flow_long && flow_short && borg_flow_head != borg_flow_tail) {
		/* Extract the next entry */
		x1 = borg_mflow_x[borg_flow_tail];
		y1 = borg_mflow_y[borg_flow_tail];

		/* Circular queue -- dequeue the next entry */
		if (++borg_flow_tail == AUTO_FLOW_MAX)
			borg_flow_tail = 0;

		/* Cost (one per movement grid) */
		n = borg_data_cost_m->data[y1][x1] + 1;

		/* New depth */
		if (n > nn) {
			/* Optimize (if requested) */
			/* if (n > borg_data_cost_m->data[origin_y][origin_x]) break; */

			/* Limit depth */
			if (n > depth)
				break;

			/* Save */
			nn = n;
		}

		/* Queue the "children" */
		for (ii = 0; ii < 8; ii++) {
			int old_head;

			borg_grid *ag;

			/* Neighbor grid */
			x = x1 + ddx_ddd[ii];
			y = y1 + ddy_ddd[ii];

			/* only on legal grids */
			if (!in_bounds(y, x))
				continue;

			/* Skip "reached" grids (Leash included)*/
			/* if (borg_data_cost_m->data[y][x] > n) continue; */

			/* Access the grid */
			ag = &borg_grids[y][x];

			/*
			 * Check monster movement: (from melee2.c: get_moves())
			 * 1.  over certain terrains,
			 * 2.  randomness of movement,
			 * 3.  animals dont follow (just like RF1_FRIENDS)
			 * 4.  monster afraid (not done yet since it is a m_ptr, not r_ptr)
			 * 5.  strong player
			 * 6.  Can the monster push through another monster. (but not the
			 * origin monster)
			 * 7.  Flow is turned off.
			 * 8.  Critters that are friends like to ambush so they tend to not be
			 * considered.
			 */
			if (!kill->awake)
				continue;
			/*if (!monster_can_cross_terrain(ag->feat, r_ptr))
				continue;*/
			if (!can_go_monster(y, x, kill->r_idx))
				continue;
			if ((ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL) &&
				 !(r_ptr->flags2 & RF2_OPEN_DOOR) &&
				 !(r_ptr->flags2 & RF2_BASH_DOOR))
				continue;
			if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM_SOLID)
				continue;
			if (r_ptr->flags1 & RF1_RAND_50)
				continue;
			if (r_ptr->flags3 & RF3_ANIMAL)
				continue;
			if (r_ptr->level + 25 < borg_skill[BI_CLEVEL])
				continue;
			if (r_ptr->flags1 & RF1_NEVER_MOVE)
				continue;
			if ((ag->kill && y != kill->y && x != kill->x) &&
				 !(r_ptr->flags2 & RF2_MOVE_BODY) &&
				 !(r_ptr->flags2 & RF2_KILL_BODY))
				continue;
#ifdef MONSTER_FLOW
			/* If flow_by_sound and _smell are off, then the borg has to be very
			 * close to the monsters before they chase him */
			if (!flow_by_sound && !flow_by_smell && n > BORG_MON_FLOW)
				continue;
#endif
			if (r_ptr->flags1 & RF1_FRIENDS)
				continue;
			/* Most of the time, the monster has some difficutly crossing the
			 * glyph.
			 * But if the borg is in the Sea of Runes, then we must 'allow' the
			 * monster
			 * the chance to pass through the glyph in order to correctly calculate
			 * the value
			 * the borg_defence_aux_tplucifer()
			 */
			if (ag->feat == FEAT_GLYPH && !(borg_position & POSITION_SEA)) {
				/* Real Glyph or Minor Glyph?  Reasonable chance Break the ward */
				if (cave[y][x].feat == FEAT_MINOR_GLYPH &&
					 !(r_ptr->flags1 & RF1_NEVER_BLOW)) {
					if (BREAK_MINOR_GLYPH > r_ptr->level * 2)
						continue;
				} else if (BREAK_GLYPH > r_ptr->level * 2)
					continue;
			}

			/* only check each grid once */
			if (borg_data_cost_m->data[y][x] != 0)
				continue;

			/* Save the flow cost */
			borg_data_cost_m->data[y][x] = n;

			/* Enqueue that entry */
			borg_mflow_x[borg_flow_head] = x;
			borg_mflow_y[borg_flow_head] = y;

			/* Circular queue -- memorize head */
			old_head = borg_flow_head;

			/* Circular queue -- insert with wrap */
			if (++borg_flow_head == AUTO_FLOW_MAX)
				borg_flow_head = 0;

			/* Circular queue -- handle overflow (badly) */
			if (borg_flow_head == borg_flow_tail)
				borg_flow_head = old_head;
		} /* Queue children */
	}	 /* Wend */

	/* Forget the flow info */
	borg_flow_head = borg_flow_tail = 0;
}

/*
 * Enqueue a fresh (legal) starting grid, if it is safe
 */
static void borg_flow_enqueue_grid(int y, int x) {
	int old_head;
	int fear;
	int p;

	/* Avoid icky grids */
	if (borg_data_icky->data[y][x])
		return;

	/* Unknown */
	if (!borg_data_know->data[y][x]) {
		/* Mark as known */
		borg_data_know->data[y][x] = TRUE;

		/** Mark dangerous grids as icky **/

		/* Get the danger */
		p = borg_danger(y, x, 1, TRUE);

		/* Increase bravery */
		if (borg_skill[BI_MAXCLEVEL] == 50)
			fear = avoidance * 5 / 10;
		if (borg_skill[BI_MAXCLEVEL] != 50)
			fear = avoidance * 3 / 10;
		if ((borg_depth & DEPTH_SCARY))
			fear = avoidance * 2;
		if (unique_on_level && (borg_depth & DEPTH_VAULT) &&
			 borg_skill[BI_MAXCLEVEL] == 50)
			fear = avoidance * 3;
		if ((borg_depth & DEPTH_SCARY) && borg_skill[BI_CLEVEL] <= 5)
			fear = avoidance * 3;
		if (goal_ignoring)
			fear = avoidance * 5;
		if (borg_t - borg_began > 5000)
			fear = avoidance * 25;
		if (borg_skill[BI_FOOD] == 0)
			fear = avoidance * 100;

		/* Normal in town */
		if (borg_skill[BI_CLEVEL] == 0)
			fear = avoidance * 3 / 10;

		/* Dangerous grid */
		if ((p > fear) && !borg_desperate && !borg_lunal_mode &&
			 !borg_munchkin_mode && !borg_digging) {
			/* Icky */
			borg_data_icky->data[y][x] = TRUE;

			/* Avoid */
			return;
		}
	}

	/* Only enqueue a grid once */
	if (!borg_data_cost->data[y][x])
		return;

	/* Save the flow cost (zero) */
	borg_data_cost->data[y][x] = 0;

	/* Enqueue that entry */
	borg_flow_y[borg_flow_head] = y;
	borg_flow_x[borg_flow_head] = x;

	/* Circular queue -- memorize head */
	old_head = borg_flow_head;

	/* Circular queue -- insert with wrap */
	if (++borg_flow_head == AUTO_FLOW_MAX)
		borg_flow_head = 0;

	/* Circular queue -- handle overflow */
	if (borg_flow_head == borg_flow_tail)
		borg_flow_head = old_head;
}

static void borg_flow_enqueue_grid_m(int y, int x) {
	int old_head;
	/*int fear;*/
	/*int p;*/

	/* Save the flow cost */
	borg_data_cost_m->data[y][x] = 0;

	/* Enqueue that entry */
	borg_mflow_y[borg_flow_head] = y;
	borg_mflow_x[borg_flow_head] = x;

	/* Circular queue -- memorize head */
	old_head = borg_flow_head;

	/* Circular queue -- insert with wrap */
	if (++borg_flow_head == AUTO_FLOW_MAX)
		borg_flow_head = 0;

	/* Circular queue -- handle overflow */
	if (borg_flow_head == borg_flow_tail)
		borg_flow_head = old_head;
}

/*
 * Do a "reverse" flow from the player outwards
 */
static void borg_flow_reverse(void) {
	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue the player's grid */
	borg_flow_enqueue_grid(c_y, c_x);

	/* Spread, but do NOT optimize */
	borg_flow_spread(250, FALSE, FALSE, FALSE, -1, FALSE);
}

/* Do a Stair-Flow.  Look at how far away this grid is to my closest stair */
static int borg_flow_cost_stair(int y, int x, int b_stair) {
	int cost = 255;

	/* Clear the flow codes */
	borg_flow_clear();

	/* Paranoid */
	if (b_stair == -1)
		return (0);

	/* Enqueue the player's grid */
	borg_flow_enqueue_grid(track_less_y[b_stair], track_less_x[b_stair]);

	/* Spread, but do NOT optimize */
	borg_flow_spread(250, FALSE, FALSE, FALSE, b_stair, FALSE);

	/* Distance from the grid to the stair */
	cost = borg_data_cost->data[y][x];

	return (cost);
}

/*
 * Commit the current "flow"
 */
static bool borg_flow_commit(cptr who, int why, bool kill) {
	int cost;
	/*int x, y;*/

	/* Cost of current grid */
	cost = borg_data_cost->data[c_y][c_x];

	/* A Starving vampire with DimDoor should be allowed to jump to the target */

	/* Verify the total "cost" */
	if (cost >= 250)
		return (FALSE);

	/* Message */
	if (who)
		borg_note(format("# Flowing toward %s at cost %d", who, cost));

	/* Obtain the "flow" information */
	COPY(borg_data_flow, borg_data_cost, borg_data);

	/* Save the goal type */
	if (!kill)
		goal = why;

	/* Success */
	return (TRUE);
}

static bool borg_flow_commit_m(int y, int x) {
	int cost;

	/* Cost of current grid */
	cost = borg_data_cost_m->data[y][x];

	/* Verify the total "cost" */
	if (cost == 0)
		return (FALSE);

	/* Monster is able to flow to the grid */
	return (TRUE);
}

/*
 * We want the borg to be able to fight summoning monster while both are in
 * a hallway.  This will reduce the summoners ability to summon.
 * We check to see if the borg is in a good hallway and if a monster is able
 * to flow into the hallway.
 *
 * This is called from borg_defend() which is called up in borg_caution() which
 * is called before any borg_flow_xxx() are called.  There is a
 * borg_flow_kill_corridor_1() will have the borg dig an anti-summon corridor.
 * It is always called after this funciton, if at all.  Since that AS-corridor
 * is far safer than just resting in a hallway.  We want to make sure this
 * routine will fail if we beleive the other one is a better option for us.
 * So the major difference is whether or not the hallway will be suitable for a
 * AS-corridor.  If it is not, then we should consider this an a good
 * alternative.
 *
 * The arrays below are used to determine the pattern of the walls near us.
 * The arrays need to match exactly to the ones in borg_flow_corridor_1()
 * Look at wall array to see if it is acceptable
 * We want to find this in the array:
 *
 * #####  ..@..  ####.  .####
 * ##.##  ##.##	 ##.#.  .#.##
 * #.#.#  #.#.#  #.#.@  @.#.#
 * ##.##  ##.##  ##.#.  .#.##
 * ..@..  #####  ####.  .####
 *
 * NORTH  SOUTH  WEST   East
 *
 */
static bool borg_lure_monster(void) {
	bool hallway = FALSE;
	int i;
	int o_y = 0;
	int o_x = 0;
	int m_x = 0;
	int m_y = 0;
	/*int b_y = 0, b_x = 0;*/
	/*int b_distance = 99;*/

	/*bool b_n = FALSE;
	bool b_s = FALSE;
	bool b_e = FALSE;
	bool b_w = FALSE;*/

	int n_array[25] = {1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
							 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1};
	int ny[25] = {-4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -2, -2, -2,
					  -2, -2, -1, -1, -1, -1, -1, 0,  0,  0,  0,  0};
	int nx[25] = {-2, -1, 0,  1,  2, -2, -1, 0,  1,  2, -2, -1, 0,
					  1,  2,  -2, -1, 0, 1,  2,  -2, -1, 0, 1,  2};

	int s_array[25] = {1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0,
							 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1};
	int sy[25] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2,
					  2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4};
	int sx[25] = {-2, -1, 0,  1,  2, -2, -1, 0,  1,  2, -2, -1, 0,
					  1,  2,  -2, -1, 0, 1,  2,  -2, -1, 0, 1,  2};

	int e_array[25] = {1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0,
							 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1};
	int ey[25] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0,
					  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2, 2};
	int ex[25] = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2,
					  3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4};

	int w_array[25] = {1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0,
							 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1};
	int wy[25] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0,
					  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2, 2};
	int wx[25] = {-4, -3, -2, -1, 0,  -4, -3, -2, -1, 0,  -4, -3, -2,
					  -1, 0,  -4, -3, -2, -1, 0,  -4, -3, -2, -1, 0};

	int wall_north = 0;
	int wall_south = 0;
	int wall_east = 0;
	int wall_west = 0;
	/*int q_x; int q_y;*/
	/*borg_kill *kill;*/

	borg_digging = FALSE;

	/* Don't do it near the edge of the screen */
	if (!in_bounds(c_y + 2, c_x + 2) || !in_bounds(c_y - 2, c_x - 2))
		return (FALSE);

	/* Is the borg in a hallway?
	  * check for 'in a hall' x axis
	  * This check is for this:
	  *
	  *      x
	  *    ..@..
	  *      x
	  *
	  * 'x' being 'not a floor' and '.' being a floor.
	  *
	  */

	/* Check for 'in a hall' x axis. */
	if ((borg_grids[c_y + 1][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y + 2][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y - 2][c_x].feat == FEAT_FLOOR) &&
		 (borg_grids[c_y][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x + 2].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x + 2].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x - 2].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x - 2].feat != FEAT_FLOOR))
		hallway = TRUE;

	/* check for 'in a hall' y axis.
	 */
	if ((borg_grids[c_y][c_x + 1].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x + 2].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 1].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 2].feat == FEAT_FLOOR) &&
		 (borg_grids[c_y + 1][c_x].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 1][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 2][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 2][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y + 2][c_x - 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 2][c_x - 1].feat != FEAT_FLOOR))
		hallway = TRUE;

	/* Resting in this hallway might be a good idea.  */
	if (hallway == TRUE) {
		int q_x;
		int q_y;

		/* Scan the monster list */
		for (i = 1; i < borg_kills_nxt; i++) {
			borg_kill *kill;

			/* Monster */
			kill = &borg_kills[i];

			/* Skip dead monsters */
			if (!kill->r_idx)
				continue;

			/* Can anyone see me and attack? */
			if (kill->los && kill->ranged_attack)
				continue;

			/* Is one standing right next to me? */
			if (kill->dist == 1)
				continue;

			/* Ignore monsters who can pass through walls or never move  */
			/* if (r_info[kill->r_idx].flags2 & RF2_PASS_WALL) continue; */
			/* if (r_info[kill->r_idx].flags2 & RF2_KILL_WALL) continue; */
			if (r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE)
				continue;
			if (!kill->awake)
				continue;

			/* Skip friendly */
			if (kill->ally)
				continue;

			/* Can the monster flow to me? */
			/* Extract panel */
			q_x = w_x / PANEL_WID;
			q_y = w_y / PANEL_HGT;

			if (borg_detect_wall[q_y + 0][q_x + 0] == TRUE &&
				 borg_detect_wall[q_y + 0][q_x + 1] == TRUE &&
				 borg_detect_wall[q_y + 1][q_x + 0] == TRUE &&
				 borg_detect_wall[q_y + 1][q_x + 1] == TRUE) {
				borg_flow_clear_m();
				borg_digging = TRUE;
				borg_flow_enqueue_grid_m(c_y, c_x);
				borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
				if (!borg_flow_commit_m(kill->y, kill->x))
					continue;
			} else {
				borg_flow_clear_m();
				borg_digging = TRUE;
				borg_flow_enqueue_grid_m(c_y, c_x);
				borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
				if (!borg_flow_commit_m(kill->y, kill->x))
					continue;
			}

			/* Check to see if this hallway would be better used as an AS-Corridor.
			 * Must be able to excavate since digging takes time
			 */
			if (borg_skill[BI_ASTONE2MUD]) {
				/* NORTH -- Consider each area near the borg, looking for a good
				 * spot to hide */
				for (o_y = -2; o_y < 1; o_y++) {
					/* Resest Wall count */
					wall_north = 0;

					/* No E-W offset when looking North-South */
					o_x = 0;

					for (i = 0; i < 25; i++) {
						borg_grid *ag;

						/* Check grids near borg */
						m_y = c_y + o_y + ny[i];
						m_x = c_x + o_x + nx[i];

						/* avoid screen edgeds */
						if (!in_bounds(m_y, m_x))
							continue;

						/* grid the grid */
						ag = &borg_grids[m_y][m_x];

						/* Certain grids must not be floor types */
						if (n_array[i] == 0 &&
							 ((ag->feat == FEAT_NONE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* This is a good grid */
							wall_north++;
						}
						if (n_array[i] == 1 &&
							 ((ag->feat <= FEAT_MORE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* A good wall would score 25. */
							wall_north++;
						}
						/* But position 11 and 13 cant both be floor grids */
						if (i == 13 && ag->feat >= FEAT_FLOOR &&
							 ag->feat <= FEAT_MORE) {
							/* get grid #7 */
							ag = &borg_grids[c_y + o_y + ny[11]][c_x + o_x + nx[11]];

							if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
								/* Alter the count because #11 is a floor grid */
								wall_north = 100;
							}
						}
					}

					/* If I found 25 grids, then that spot will work well for a
					 * as-corridor. */
					if (wall_north == 25) {
						return (FALSE);
					}
				}

				/* SOUTH -- Consider each area near the borg, looking for a good
				 * spot to hide */
				for (o_y = -1; o_y < 2; o_y++) {
					/* Resest Wall count */
					wall_south = 0;

					for (i = 0; i < 25; i++) {
						borg_grid *ag;

						/* No lateral offset on South check */
						o_x = 0;

						/* Check grids near borg */
						m_y = c_y + o_y + sy[i];
						m_x = c_x + o_x + sx[i];

						/* avoid screen edgeds */
						if (!in_bounds(m_y, m_x))
							continue;

						/* grid the grid */
						ag = &borg_grids[m_y][m_x];

						/* Certain grids must not be floor types */
						if (s_array[i] == 0 &&
							 ((ag->feat == FEAT_NONE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* This is a good grid */
							wall_south++;
						}
						if (s_array[i] == 1 &&
							 ((ag->feat <= FEAT_MORE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* A good wall would score 25. */
							wall_south++;
						}
						/* But position 11 and 13 cant both be floor grids */
						if (i == 13 && ag->feat >= FEAT_FLOOR &&
							 ag->feat <= FEAT_MORE) {
							/* get grid # 11 */
							ag = &borg_grids[c_y + o_y + sy[11]][c_x + o_x + sx[11]];

							if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
								/* Alter the count because #11 is a floor grid */
								wall_south = 100;
							}
						}
					}

					/* If I found 25 grids, then that spot will work well */
					if (wall_south == 25) {
						return (FALSE);
					}
				}

				/* EAST -- Consider each area near the borg, looking for a good spot
				 * to hide */
				for (o_x = -1; o_x < 2; o_x++) {
					/* Resest Wall count */
					wall_east = 0;

					/* No N-S offset check when looking E-W */
					o_y = 0;

					for (i = 0; i < 25; i++) {
						borg_grid *ag;

						/* Check grids near borg */
						m_y = c_y + o_y + ey[i];
						m_x = c_x + o_x + ex[i];

						/* avoid screen edgeds */
						if (!in_bounds(m_y, m_x))
							continue;

						/* grid the grid */
						ag = &borg_grids[m_y][m_x];

						/* Certain grids must not be floor types */
						if (e_array[i] == 0 &&
							 ((ag->feat == FEAT_NONE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* This is a good grid */
							wall_east++;
						}
						if (e_array[i] == 1 &&
							 ((ag->feat <= FEAT_MORE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* A good wall would score 25. */
							wall_east++;
						}

						/* But position 17 and 7 cant both be floor grids */
						if (i == 17 && ag->feat >= FEAT_FLOOR &&
							 ag->feat <= FEAT_MORE) {
							/* get grid # 7 */
							ag = &borg_grids[c_y + o_y + ey[7]][c_x + o_x + ex[7]];

							if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
								/* Alter the count because #7 is a floor grid */
								wall_east = 100;
							}
						}
					}

					/* If I found 25 grids, then that spot will work well */
					if (wall_east == 25) {
						return (FALSE);
					}
				}

				/* WEST -- Consider each area near the borg, looking for a good spot
				 * to hide */
				for (o_x = -2; o_x < 1; o_x++) {
					/* Resest Wall count */
					wall_west = 0;

					/* No N-S offset check when looking E-W */
					o_y = 0;

					for (i = 0; i < 25; i++) {
						borg_grid *ag;

						/* Check grids near borg */
						m_y = c_y + o_y + wy[i];
						m_x = c_x + o_x + wx[i];

						/* avoid screen edgeds */
						if (!in_bounds(m_y, m_x))
							continue;

						/* grid the grid */
						ag = &borg_grids[m_y][m_x];

						/* Certain grids must not be floor types */
						if (w_array[i] == 0 &&
							 ((ag->feat == FEAT_NONE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* This is a good grid */
							wall_west++;
						}
						if (w_array[i] == 1 &&
							 ((ag->feat <= FEAT_MORE) ||
							  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
							  (ag->feat >= FEAT_WALL_EXTRA &&
								ag->feat <= FEAT_WALL_SOLID))) {
							/* A good wall would score 25. */
							wall_west++;
						}

						/* But position 7 and 17 cant both be floor grids */
						if (i == 17 && ag->feat >= FEAT_FLOOR &&
							 ag->feat <= FEAT_MORE) {
							/* get grid #7 */
							ag = &borg_grids[c_y + o_y + wy[7]][c_x + o_x + wx[7]];

							if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
								/* Alter the count because #7 is a floor grid */
								wall_west = 100;
							}
						}
					}

					/* If I found 25 grids, then that spot will work well */
					if (wall_west == 25) {
						return (FALSE);
					}
				}
			} /* Checking for Stone to Mud Spell */

			/* Everything check out, this is a good place to lure the summoner */
			return (TRUE);

		} /* Scan Monster List */
	}
	/* Not good */
	return (FALSE);
}

/*
 * Simulate/Apply the optimal result of making a racial physical attack.
 */
extern int borg_attack_aux_racial_thrust(int level, int dam, int fail,
													  bool inflate, int specific, int cost) {
	int p, dir;

	int i, b_i = -1;
	int d, b_d = -1;
	/*int dd = 0;*/

	borg_grid *ag;

	borg_kill *kill;

	monster_race *r_ptr;

	/* Too afraid to attack */
	if (borg_skill[BI_ISAFRAID])
		return (0);

	/* not when confused or afraid */
	if (borg_skill[BI_ISAFRAID] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* must be right race or right mutation */
	if (!borg_skill[BI_VAMPIRE])
		return (0);

	/* must be right level */
	if (borg_skill[BI_CLEVEL] < level)
		return (0);

	/* Not if terribly weak */
	if (borg_skill[BI_ISFULL] &&
		 borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 4 / 10)
		return (0);
	if (!borg_skill[BI_ISFULL] && !borg_skill[BI_ISWEAK] &&
		 borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 2 / 10)
		return (0);

	/* Check for Mutation check */
	if (borg_race != RACE_VAMPIRE && (p_ptr->muta1 & COR1_VAMPIRISM)) {
		if (!borg_mutation(COR1_VAMPIRISM, TRUE, 25, FALSE))
			return (0);
	}

	/* Examine possible destinations */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Attacking a specific monster? */
		if (specific >= 0) {
			x = borg_temp_x[specific];
			y = borg_temp_y[specific];
		}

		/* Require "adjacent" */
		if (distance(c_y, c_x, y, x) > 1)
			continue;

		/* Acquire grid */
		ag = &borg_grids[y][x];

		/* Obtain the monster */
		kill = &borg_kills[ag->kill];

		/* monster race */
		r_ptr = &r_info[kill->r_idx];

		/* Dont attack our buddies */
		if (!borg_skill[BI_ISWEAK] && (kill->ally))
			continue;

		/* Can't really attack those who are invuln */
		if (kill->invulner)
			continue;

		/* Drain gives food */
		if (!monster_living(r_ptr))
			continue;

		/* Base Dam */
		d = dam;

		/* Bonus in hunger states */
		if (!borg_skill[BI_ISFULL])
			d = d * 15 / 10;
		if (borg_skill[BI_ISHUNGRY])
			d = d * 15 / 10;

		/* Bonus if down on HP */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] && !borg_skill[BI_ISFULL])
			d = d * 3;

		/*
		 * Enhance the preceived damage on Uniques.  This way we target them
		 * Keep in mind that he should hit the uniques but if he has a
		 * x5 great bane of dragons, he will tend attack the dragon since the
		 * precieved (and actual) damage is higher.  But don't select
		 * the town uniques (maggot does no damage)
		 *
		 * These are copied from the thrust damage routine.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] >= 1)
			d += (d * 5);

		/* Hack -- ignore Maggot until later.  Player will chase Maggot
		 * down all accross the screen waking up all the monsters.  Then
		 * he is stuck in a comprimised situation.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] == 0) {
			d = d * 2 / 3;

			/* Dont hunt maggot until later */
			if (borg_skill[BI_CLEVEL] < 5)
				d = 0;
		}

		/* give a small bonus for whacking a breeder */
		if (r_ptr->flags2 & RF2_MULTIPLY)
			d = (d * 3 / 2);

		/* Enhance the preceived damgage to summoner in order to influence the
		 * choice of targets.
		 */
		if (kill->summoner || kill->questor)
			d += ((d * 3) / 2);

		/* Hack -- avoid waking most "hard" sleeping monsters */
		if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode &&
			 borg_skill[BI_CLEVEL] < 50) {
			/* Calculate danger */
			borg_full_damage = TRUE;
			p = borg_danger_aux(y, x, 1, ag->kill, TRUE, FALSE);
			borg_full_damage = FALSE;

			if (p > avoidance / 2)
				continue;
		}

		/* Hack -- ignore sleeping town monsters */
		if (!borg_skill[BI_CDEPTH] && !kill->awake && !borg_skill[BI_VAMPIRE])
			continue;

		/* Calculate "danger" to player */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 2, ag->kill, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Reduce "bonus" of partial kills */
		if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15)
			p = p / 10;

		/* Add the danger to the damage */
		d += p;

		/* Ignore lower damage */
		if ((b_i >= 0) && (d < b_d))
			continue;

		/* Save the info */
		b_i = i;
		b_d = d;
		if (specific >= 0)
			b_i = specific;
	}

	/* Nothing to attack */
	if (b_i < 0)
		return (0);

	/* Reduce the damage by the Activation Failure % unless its a really good
	 * idea to use it */
	if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] || borg_skill[BI_ISFULL])
		b_d = b_d * (100 - fail) / 100;

	/* Bonus if I'm hungry */
	if (borg_skill[BI_ISHUNGRY])
		b_d = b_d * 15 / 10;

	/* Another Bonus if I'm weak from hunger */
	if (borg_skill[BI_ISWEAK])
		b_d = b_d * 15 / 10;

	/* Reduce the value by the cost of mana */
	b_d = b_d - cost;

	/* Simulation */
	if (borg_simulate && !inflate)
		return (dam);
	if (borg_simulate)
		return (b_d);

	/* Save the location */
	g_x = borg_temp_x[b_i];
	g_y = borg_temp_y[b_i];

	ag = &borg_grids[g_y][g_x];
	kill = &borg_kills[ag->kill];

	/* Racial Attack */
	if (borg_race == RACE_VAMPIRE) {
		/* Note */
		borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.",
							  (r_name + r_info[kill->r_idx].name), g_y, g_x,
							  kill->power));
		borg_note(format("# Attacking with Racial Attack.  Value:'%d'", b_d));

		/* Get a direction for attacking */
		dir = borg_extract_dir(c_y, c_x, g_y, g_x);

		/* Activate */
		borg_keypress('U');

		/* Racial is always 'a' */
		borg_keypress('a');
	} else if (p_ptr->muta1 & COR1_VAMPIRISM) {
		/* Note */
		borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.",
							  (r_name + r_info[kill->r_idx].name), g_y, g_x,
							  kill->power));
		borg_note(format("# Attacking with Mutation Attack.  Value:'%d'", b_d));

		/* Get a direction for attacking */
		dir = borg_extract_dir(c_y, c_x, g_y, g_x);
		(void)borg_mutation(COR1_VAMPIRISM, FALSE, 25, FALSE);
	}
	/* Attack the grid */
	borg_keypress(I2D(dir));

	/* Success */
	return (b_d);
}

/*
 * Attempt to induce "word of recall"
 * artifact activations added throughout this code
 */
bool borg_recall(void) {

	/* Vampires dont like to Recall up in Daytime */
	if (borg_skill[BI_FEAR_LITE] && borg_skill[BI_CDEPTH] >= 1) {
		/* If day time, Recall could be bad */
		if ((borg_skill[BI_HRTIME] >= 5) && (borg_skill[BI_HRTIME] <= 18)) {
			/* Do not Recall */
			/*return (FALSE); This is safe now, because vampires will recall into
			 * the dungeon while standing on the town stairs. */
		}
	}

	/* Multiple "recall" fails */
	if (!goal_recalling) {
		/* Try to "recall" */
		if (borg_zap_rod(SV_ROD_RECALL) ||
			 borg_activate_artifact(ART_AZRAEL, FALSE) ||
			 /*borg_activate_artifact(ART_THRAIN, TRUE) ||*/
			 borg_spell_fail(REALM_SORCERY, 2, 7, 60) ||
			 borg_spell_fail(REALM_ARCANE, 3, 6, 60) ||
			 borg_spell_fail(REALM_TRUMP, 1, 6, 60) ||
			 borg_mutation(COR1_RECALL, FALSE, 60, FALSE) ||
			 borg_activate_activation(ACT_RECALL, FALSE) ||
			 borg_read_scroll(SV_SCROLL_WORD_OF_RECALL)) {
			/* reset recall depth in dungeon? */
			if (borg_skill[BI_CDEPTH] < borg_skill[BI_MAXDEPTH] &&
				 borg_skill[BI_CDEPTH] != 0) {
				if (goal_fleeing_munchkin == TRUE) {
					/* Do reset Depth */
					borg_note("# Resetting recall depth during munchkin mode.");
					borg_keypress('y');
				} else if (borg_skill[BI_MAXDEPTH] < 99 &&
							  borg_skill[BI_CDEPTH] > 40 &&
							  ((cptr)NULL !=
								borg_prepared[borg_skill[BI_CDEPTH] - 10])) {
					/* Do reset Depth */
					borg_note("# Resetting recall depth; too deep for prep.");
					borg_keypress('y');
				} else {
					/* Do not reset Depth */
					borg_note("# Not resetting recall depth.");
					borg_keypress('n');
				}
			}

			/* Success */
			return (TRUE);
		}
	}

	/* Nothing */
	return (FALSE);
}

/*
 * Vampire type borgs need to suck blood
 * If they can't vamp_drain, then try to create a creature to consume.
 */
extern bool borg_eat_vamp(void) {
	int dam =
		 borg_skill[BI_CLEVEL] + ((borg_skill[BI_CLEVEL] / 2) *
										  MAX(1, borg_skill[BI_CLEVEL] / 10)); /* Dmg */
	int cost = 1 + (borg_skill[BI_CLEVEL] / 3);
	int fail = 100 - racial_chance(2, A_CON, 9);
	int success = 0;
	int i;
	int y, x;

	/* Reset monster list */
	borg_temp_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {

		borg_kill *kill;
		borg_grid *ag;

		/* Monster */
		kill = &borg_kills[i];

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Check the distance */
		if (kill->dist > 1)
			continue;

		/* Sometimes the borg can lose a monster index in the grid if there are
		 * lots of monsters
		 * on screen.  If he does lose one, reinject the index here.
		 */
		if (!ag->kill)
			borg_grids[kill->y][kill->x].kill = i;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Sufficient mana to activate the ability and a resonable target */
	borg_simulate = FALSE;
	success = (borg_attack_aux_racial_thrust(2, dam, fail, FALSE, -1, cost));
	borg_simulate = TRUE;

	/* Try to create a living thing to consume next round */
	if (success <= 0) {
		/* Attempt to cast a summon */
		if (borg_spell(REALM_NATURE, 1, 6) || borg_spell(REALM_TRUMP, 1, 1) ||
			 borg_spell(REALM_TRUMP, 1, 3) || borg_spell(REALM_TRUMP, 2, 1) ||
			 borg_spell(REALM_TRUMP, 2, 2) || borg_spell(REALM_TRUMP, 2, 3) ||
			 borg_read_scroll(SV_SCROLL_SUMMON_MONSTER)) {
			borg_note("# Attempting to Summon Animal to consume next round.");

			return (TRUE);
		}
	}

	/* Not successful */
	if (!success)
		return (FALSE);
	else
		return (TRUE);
}

/*
 * Prevent starvation by any means possible
 */
extern bool borg_eat_food_any(void) {
	int i;

	/* Scan the inventory for "normal" food */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip unknown food */
		if (!item->kind)
			continue;

		/* Skip non-food */
		if (item->tval != TV_FOOD)
			continue;

		/* Skip "flavored" food */
		if (item->sval < SV_FOOD_MIN_FOOD)
			continue;

		/* Eat something of that type */
		if (borg_eat_food(item->sval))
			return (TRUE);
	}

	/* Scan the inventory for "okay" food */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip unknown food */
		if (!item->kind)
			continue;

		/* Skip non-food */
		if (item->tval != TV_FOOD)
			continue;

		/* Skip "icky" food */
		if (item->sval < SV_FOOD_MIN_OKAY)
			continue;

		/* Eat something of that type */
		if (borg_eat_food(item->sval))
			return (TRUE);
	}

	/* Scan the inventory for "potions" food */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip unknown food */
		if (!item->kind)
			continue;

		/* Skip non-food */
		if (item->tval != TV_POTION)
			continue;

		/* Consume in order, when hurting */
		if ((borg_skill[BI_CURHP] < 4 ||
			  borg_skill[BI_CURHP] < borg_skill[BI_MAXHP]) &&
			 (borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			  borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
			  borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
			  borg_quaff_potion(SV_POTION_CURING) ||
			  borg_quaff_potion(SV_POTION_RESTORE_MANA) ||
			  borg_quaff_potion(SV_POTION_HEALING) ||
			  borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_LIFE))) {
			return (TRUE);
		}
	}

	/* Nothing */
	return (FALSE);
}
/*
 * Hack -- evaluate the likelihood of the borg getting surrounded
 * by a bunch of monsters.  This is called from borg_danger() when
 * he looking for a strategic retreat.  It is hopeful that the borg
 * will see that several monsters are approaching him and he may
 * become surrouned then die.  This routine looks at near by monsters
 * as determines the likelyhood of him getting surrouned.
 */
bool borg_surrounded(void) {
	borg_kill *kill;
	/*monster_race *r_ptr;*/

	int safe_grids = 8;
	int non_safe_grids = 0;
	int monsters = 0;
	int adjacent_monsters = 0;

	int x9, y9, ax, ay, d;
	int i;

	/* Evaluate the local monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip Friendly */
		if (kill->ally)
			continue;

		x9 = kill->x;
		y9 = kill->y;

		/* Distance components */
		ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
		ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

		/* Distance */
		d = MAX(ax, ay);

		/* if the monster is too far then skip it. */
		if (d > 3)
			continue;

		/* if he cant see me then forget it.*/
		if (!(borg_grids[y9][x9].info & BORG_VIEW))
			continue;

		/* if asleep, don't consider this one */
		if (!kill->awake)
			continue;

		/* keep track of monsters touching me */
		if (d == 1)
			adjacent_monsters++;

		/* Save the location used in borg_caution() during strategic retreat) */
		borg_temp_x[borg_temp_n] = x9;
		borg_temp_y[borg_temp_n] = y9;
		borg_temp_n++;

		/* Add them up. */
		monsters++;
	}

	/* Evaluate the Non Safe Grids, (walls, closed doors, traps, monsters) */
	for (i = 0; i < 8; i++) {
		int x = c_x + ddx_ddd[i];
		int y = c_y + ddy_ddd[i];

		borg_grid *ag;

		/* check for bounds */
		if (y > AUTO_MAX_Y || y < 0)
			continue;
		if (x > AUTO_MAX_X || x < 0)
			continue;

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Non Safe grids (non-floor grids) */
		if (!borg_cave_floor_grid(ag)) {
			switch (ag->feat) {
			case FEAT_PERM_EXTRA:
			case FEAT_PERM_INNER:
			case FEAT_PERM_OUTER:
			case FEAT_PERM_SOLID:
				non_safe_grids++;
				break;
			default:
				/* Wall grids */
				if (!borg_skill[BI_PASSWALL])
					non_safe_grids++;
			}
		}

		/* Floor'ish grids */
		else if (ag->feat == FEAT_WATER && borg_skill[BI_ENCUMBERD] &&
					!borg_skill[BI_FEATH])
			non_safe_grids++;
		/*
		else if (ag->feat == FEAT_SHAL_LAVA && !borg_skill[BI_IFIRE] &&
					!borg_skill[BI_FEATH])
			non_safe_grids++;
		else if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
			non_safe_grids++;
		*/
		/* Skip unknown grids */
		else if (ag->feat == FEAT_NONE)
			non_safe_grids++;

		/* Skip monster grids */
		else if (ag->kill)
			non_safe_grids++;

		/* Mega-Hack -- skip stores XXX XXX XXX */
		else if ((ag->feat >= FEAT_SHOP_HEAD) && (ag->feat <= FEAT_SHOP_TAIL))
			non_safe_grids++;
		else if ((ag->feat >= FEAT_BLDG_HEAD) && (ag->feat <= FEAT_BLDG_TAIL))
			non_safe_grids++;

		/* Mega-Hack -- skip traps XXX XXX XXX */
		else if ((ag->feat >= FEAT_TRAP_TRAPDOOR) &&
					(ag->feat <= FEAT_TRAP_SLEEP))
			non_safe_grids++;
	}

	/* Safe grids are decreased */
	safe_grids = safe_grids - non_safe_grids;

	/* Am I in hallway? If so don't worry about it */
	if (safe_grids == 1 && adjacent_monsters == 1)
		return (FALSE);

	/* I am likely to get surrouned */
	if (monsters > safe_grids) {
		borg_note(format("# Possibility of being surrounded (%d/%d)", monsters,
							  safe_grids));

		/* The borg can get trapped by breeders by continueing to flee
		 * into a dead-end.  So he needs to be able to trump this
		 * routine.
		 */
		if (goal_ignoring)
			return (FALSE);
		else
			return (TRUE);
	}

	/* Probably will not be surrouned */
	return (FALSE);
}

/*
 * Surrounded by breeders.
 */
bool borg_surrounded_breeder(void) {
	borg_kill *kill;
	monster_race *r_ptr;

	int safe_grids = 8;
	int non_safe_grids = 0;
	int monsters = 0;
	int adjacent_monsters = 0;

	int x9, y9, ax, ay, d;
	int i;

	/* Evaluate the local monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* skip non-breeders */
		if (!(r_ptr->flags2 & RF2_MULTIPLY))
			continue;

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip Friendly */
		if (kill->ally)
			continue;

		x9 = kill->x;
		y9 = kill->y;

		/* Distance components */
		ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
		ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

		/* Distance */
		d = MAX(ax, ay);

		/* if the monster is too far then skip it. */
		if (d > 3)
			continue;

		/* if he cant see me then forget it.*/
		if (!(borg_grids[y9][x9].info & BORG_VIEW))
			continue;

		/* if asleep, don't consider this one */
		if (!kill->awake)
			continue;

		/* Monsters with Pass Wall are dangerous, no escape from them */
		if (r_ptr->flags2 & RF2_PASS_WALL)
			continue;
		if (r_ptr->flags2 & RF2_KILL_WALL)
			continue;

		/* Monsters who never move cant surround */
		if (r_ptr->flags1 & RF1_NEVER_MOVE)
			continue;

		/* keep track of monsters touching me */
		if (d == 1)
			adjacent_monsters++;

		/* Save the location used in borg_caution() during strategic retreat) */
		borg_temp_x[borg_temp_n] = x9;
		borg_temp_y[borg_temp_n] = y9;
		borg_temp_n++;

		/* Add them up. */
		monsters++;
	}

	/* Evaluate the Non Safe Grids, (walls, closed doors, traps, monsters) */
	for (i = 0; i < 8; i++) {
		int x = c_x + ddx_ddd[i];
		int y = c_y + ddy_ddd[i];

		borg_grid *ag;

		/* check for bounds */
		if (y > AUTO_MAX_Y || y < 0)
			continue;
		if (x > AUTO_MAX_X || x < 0)
			continue;

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Non Safe grids */
		if (!borg_cave_floor_grid(ag)) {
			switch (ag->feat) {
			default:
				non_safe_grids++;
			}
		}

		/* Floor'ish grids */
		if (ag->feat == FEAT_WATER && borg_skill[BI_ENCUMBERD] &&
			 !borg_skill[BI_FEATH])
			non_safe_grids++;
		/*
		if (ag->feat == FEAT_SHAL_LAVA && !borg_skill[BI_IFIRE] &&
			 !borg_skill[BI_FEATH])
			non_safe_grids++;
		if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
			non_safe_grids++;
		*/
		/* Skip unknown grids */
		if (ag->feat == FEAT_NONE)
			non_safe_grids++;

		/* Skip monster grids */
		if (ag->kill)
			non_safe_grids++;

		/* Mega-Hack -- skip stores XXX XXX XXX */
		if ((ag->feat >= FEAT_SHOP_HEAD) && (ag->feat <= FEAT_SHOP_TAIL))
			non_safe_grids++;
		if ((ag->feat >= FEAT_BLDG_HEAD) && (ag->feat <= FEAT_BLDG_TAIL))
			non_safe_grids++;

		/* Mega-Hack -- skip traps XXX XXX XXX */
		if ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP))
			non_safe_grids++;
	}

	/* Safe grids are decreased */
	safe_grids = safe_grids - non_safe_grids;

	/* Am I in hallway? If so don't worry about it */
	if (safe_grids == 1 && adjacent_monsters == 1)
		return (FALSE);

	/* I am likely to get surrouned */
	if (monsters > safe_grids) {
		borg_note(format("# Possibility of being surrounded by breeders (%d/%d)",
							  monsters, safe_grids));

		return (TRUE);
	}

	/* Probably will not be surrouned */
	return (FALSE);
}

/*
 * Mega-Hack -- evaluate the "freedom" of the given location
 *
 * The theory is that often, two grids will have equal "danger",
 * but one will be "safer" than the other, perhaps because it
 * is closer to stairs, or because it is in a corridor, or has
 * some other characteristic that makes it "safer".
 *
 * Then, if the Borg is in danger, say, from a normal speed monster
 * which is standing next to him, he will know that walking away from
 * the monster is "pointless", because the monster will follow him,
 * but if the resulting location is "safer" for some reason, then
 * he will consider it.  This will allow him to flee towards stairs
 * in the town, and perhaps towards corridors in the dungeon.
 *
 * This method is used in town to chase the stairs.
 *
 * XXX XXX XXX We should attempt to walk "around" buildings.
 */
static int borg_freedom(int y, int x) {
	int d, f = 0;

	/* Hack -- chase down stairs in town */
	if (!borg_skill[BI_CDEPTH] && track_more_num) {
		/* Love the stairs! */
		d = double_distance(y, x, track_more_y[0], track_more_x[0]);

		/* Proximity is good */
		f += (1000 - d);

		/* Close proximity is great */
		if (d < 4)
			f += (2000 - (d * 500));
	}

	/* Hack -- chase Up Stairs in dungeon */
	if (borg_skill[BI_CDEPTH] && track_less_num) {
		/* Love the stairs! */
		d = double_distance(y, x, track_less_y[0], track_less_x[0]);

		/* Proximity is good */
		f += (1000 - d);

		/* Close proximity is great */
		if (d < 4)
			f += (2000 - (d * 500));
	}

	/* Freedom */
	return (f);
}

/*
 * Check a floor grid for "happy" status
 *
 * These grids are floor grids which contain stairs, or which
 * are non-corners in corridors, or which are directly adjacent
 * to pillars, or grids which we have stepped on before.
 *  Stairs are good because they can be used to leave
 * the level.  Corridors are good because you can back into them
 * to avoid groups of monsters and because they can be used for
 * escaping.  Pillars are good because while standing next to a
 * pillar, you can walk "around" it in two different directions,
 * allowing you to retreat from a single normal monster forever.
 * Stepped on grids are good because they likely stem from an area
 * which has been cleared of monsters.
 */
extern bool borg_happy_grid_bold(int y, int x) {
	int i;

	borg_grid *ag = &borg_grids[y][x];

	/* Accept stairs */
	if (ag->feat == FEAT_LESS)
		return (TRUE);
	if (ag->feat == FEAT_MORE)
		return (TRUE);
	if (ag->feat == FEAT_MINOR_GLYPH)
		return (TRUE);
	if (ag->feat == FEAT_GLYPH)
		return (TRUE);

	/* Quick Bounds check for borg.
	 * This may screw him up
	 */
	if (y - 2 < 0 || x - 2 < 0 || y + 2 > AUTO_MAX_Y || x + 2 > AUTO_MAX_X)
		return (FALSE);

	/* Hack -- weak/dark is very unhappy */
	if (borg_skill[BI_ISWEAK] || borg_skill[BI_CUR_LITE] == 0)
		return (FALSE);

	/* Case 1a: north-south corridor */
	if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 1b: east-west corridor */
	if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 1aa: north-south doorway */
	if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 1ba: east-west doorway */
	if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 2a: north pillar */
	if (!borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y - 1, x - 1) &&
		 borg_cave_floor_bold(y - 1, x + 1) && borg_cave_floor_bold(y - 2, x)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 2b: south pillar */
	if (!borg_cave_floor_bold(y + 1, x) && borg_cave_floor_bold(y + 1, x - 1) &&
		 borg_cave_floor_bold(y + 1, x + 1) && borg_cave_floor_bold(y + 2, x)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 2c: east pillar */
	if (!borg_cave_floor_bold(y, x + 1) && borg_cave_floor_bold(y - 1, x + 1) &&
		 borg_cave_floor_bold(y + 1, x + 1) && borg_cave_floor_bold(y, x + 2)) {
		/* Happy */
		return (TRUE);
	}

	/* Case 2d: west pillar */
	if (!borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y - 1, x - 1) &&
		 borg_cave_floor_bold(y + 1, x - 1) && borg_cave_floor_bold(y, x - 2)) {
		/* Happy */
		return (TRUE);
	}

	/* check for grids that have been stepped on before */
	for (i = 0; i < track_step_num; i++) {
		/* Enqueue the grid */
		if ((track_step_y[i] == y) && (track_step_x[i] == x)) {
			/* Recent step is good */
			if (i < 45) {
				return (TRUE);
			}
		}
	}

	/* Not happy */
	return (FALSE);
}

/* This will look down a hallway and possibly light it up using
 * the Light Beam mage spell.  This spell is mostly used when
 * the borg is moving through the dungeon under boosted bravery.
 * This will allow him to "see" if anyone is there.
 *
 * It might also come in handy if he's in a hallway and gets shot, or
 * if resting in a hallway.  He may want to cast it to make
 * sure no previously unknown monsters are in the hall.
 * NOTE:  ESP will alter the value of this spell.
 *
 * Borg has a problem when not on map centering mode and casting the beam
 * repeatedly, down or up when at the edge of a panel.
 */
bool borg_lite_beam(bool simulation) {
	int dir = 5;
	bool spell_ok = FALSE;
	int i;
	bool blocked = FALSE;

	borg_grid *ag = &borg_grids[c_y][c_x];

	/* Hack -- weak/dark is very unhappy */
	if (borg_skill[BI_ISWEAK] || borg_skill[BI_CUR_LITE] == 0)
		return (FALSE);

	/* Apply a control effect so that he does not get stuck in a loop */
	if ((borg_t - borg_began) >= 2000)
		return (FALSE);

	/* Require the abilitdy */
	if (borg_spell_okay_fail(REALM_NATURE, 1, 4, 20) ||
		 borg_spell_okay_fail(REALM_ARCANE, 2, 5, 20) ||
		 (-1 != borg_slot(TV_WAND, SV_WAND_LITE) &&
		  borg_items[borg_slot(TV_WAND, SV_WAND_LITE)].pval) ||
		 borg_equips_rod(SV_ROD_LITE) ||
		 borg_equips_activation(ACT_SUNLIGHT, TRUE))
		spell_ok = TRUE;

	/*** North Direction Test***/

	/* Quick Boundary check */
	if (c_y - borg_skill[BI_CUR_LITE] - 1 > 0) {
		/* Look just beyond my light */
		ag = &borg_grids[c_y - borg_skill[BI_CUR_LITE] - 1][c_x];

		/* Must be on the panel */
		if (panel_contains(c_y - borg_skill[BI_CUR_LITE] - 2, c_x)) {
			/* Check each grid in our light radius along the course */
			for (i = 0; i <= borg_skill[BI_CUR_LITE]; i++) {
				if (borg_cave_floor_bold(c_y - i, c_x) &&
					 !borg_cave_floor_bold(c_y - borg_skill[BI_CUR_LITE] - 1, c_x) &&
					 ag->feat < FEAT_DOOR_HEAD && blocked == FALSE) {
					/* note the direction */
					dir = 8;
				} else {
					dir = 5;
					blocked = TRUE;
				}
			}
		}
	}

	/*** South Direction Test***/

	/* Quick Boundary check */
	if (c_y + borg_skill[BI_CUR_LITE] + 1 < AUTO_MAX_Y && dir == 5) {
		/* Look just beyond my light */
		ag = &borg_grids[c_y + borg_skill[BI_CUR_LITE] + 1][c_x];

		/* Must be on the panel */
		if (panel_contains(c_y + borg_skill[BI_CUR_LITE] + 2, c_x)) {
			/* Check each grid in our light radius along the course */
			for (i = 0; i <= borg_skill[BI_CUR_LITE]; i++) {
				if (borg_cave_floor_bold(c_y + i, c_x) && /* all floors */
					 !borg_cave_floor_bold(c_y + borg_skill[BI_CUR_LITE] + 1, c_x) &&
					 ag->feat < FEAT_DOOR_HEAD && blocked == FALSE) {
					/* note the direction */
					dir = 2;
				} else {
					dir = 5;
					blocked = TRUE;
				}
			}
		}
	}

	/*** East Direction Test***/

	/* Quick Boundary check */
	if (c_x + borg_skill[BI_CUR_LITE] + 1 < AUTO_MAX_X && dir == 5) {
		/* Look just beyond my light */
		ag = &borg_grids[c_y][c_x + borg_skill[BI_CUR_LITE] + 1];

		/* Must be on the panel */
		if (panel_contains(c_y, c_x + borg_skill[BI_CUR_LITE] + 2)) {
			/* Check each grid in our light radius along the course */
			for (i = 0; i <= borg_skill[BI_CUR_LITE]; i++) {
				if (borg_cave_floor_bold(c_y, c_x + i) && /* all floors */
					 !borg_cave_floor_bold(c_y, c_x + borg_skill[BI_CUR_LITE] + 1) &&
					 ag->feat < FEAT_DOOR_HEAD && blocked == FALSE) {
					/* note the direction */
					dir = 6;
				} else {
					dir = 5;
					blocked = TRUE;
				}
			}
		}
	}

	/*** West Direction Test***/

	/* Quick Boundary check */
	if (c_x - borg_skill[BI_CUR_LITE] - 1 > 0 && dir == 5) {
		/* Look just beyond my light */
		ag = &borg_grids[c_y][c_x - borg_skill[BI_CUR_LITE] - 1];

		/* Must be on the panel */
		if (panel_contains(c_y, c_x - borg_skill[BI_CUR_LITE] - 2)) {
			/* Check each grid in our light radius along the course */
			for (i = 1; i <= borg_skill[BI_CUR_LITE]; i++) {
				/* Verify that there are no blockers in my light radius and
				 * the 1st grid beyond my light is not a floor nor a blocker
				 */
				if (borg_cave_floor_bold(c_y, c_x - i) && /* all see through */
					 !borg_cave_floor_bold(c_y, c_x - borg_skill[BI_CUR_LITE] - 1) &&
					 ag->feat < FEAT_DOOR_HEAD && blocked == FALSE) {
					/* note the direction */
					dir = 4;
				} else {
					dir = 5;
					blocked = TRUE;
				}
			}
		}
	}

	/* Dont do it if: */
	if (dir == 5 || spell_ok == FALSE || blocked == TRUE ||
		 (dir == 2 && (c_y == 18 || c_y == 19 || c_y == 29 || c_y == 30 ||
							c_y == 40 || c_y == 41 || c_y == 51 || c_y == 52)) ||
		 (dir == 8 && (c_y == 13 || c_y == 14 || c_y == 24 || c_y == 25 ||
							c_y == 35 || c_y == 36 || c_y == 46 || c_y == 47)))
		return (FALSE);

	/* simulation */
	if (simulation)
		return (TRUE);

	/* cast the light beam */
	if (borg_spell_fail(REALM_NATURE, 1, 4, 20) ||
		 borg_spell_fail(REALM_ARCANE, 2, 5, 20) || borg_zap_rod(SV_ROD_LITE) ||
		 borg_activate_activation(ACT_SUNLIGHT, FALSE) ||
		 borg_aim_wand(SV_WAND_LITE)) { /* apply the direction */
		borg_keypress(I2D(dir));
		borg_note("# Illuminating this hallway");
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
		return (TRUE);
	}

	/* cant do it */
	return (FALSE);
}

/*
 * Scan the monster lists for certain types of monster that we
 * should be concerned over.
 * This only works for monsters we know about.  If one of the
 * monsters around is misidentified then it may be a unique
 * and we wouldn't know.  Special consideration is given to Lucifer
 */
int borg_near_monster_type(int dist) {
	borg_kill *kill;
	monster_race *r_ptr;

	int x9, y9, ax, ay, d;
	int i;
	int breeder_count = 0;
	int total_danger = 0;

	/* reset the borg flags */
	borg_fighting_summoner = FALSE;
	borg_fighting_unique = 0;
	borg_fighting_evil_unique = FALSE;
	borg_kills_summoner = -1;
	borg_fighting_questor = FALSE;
	borg_fighting_dragon = FALSE;
	borg_fighting_demon = FALSE;
	borg_fighting_tunneler = FALSE;
	borg_fighting_tele_to = FALSE;
	borg_fighting_chaser = FALSE;

	/* Scan the monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip friendly */
		if (kill->ally)
			continue;

		/* Skip sleeping ones */
		if (!kill->awake && !borg_munchkin_mode)
			continue;

		x9 = kill->x;
		y9 = kill->y;

		/* Distance components */
		ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
		ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

		/* Distance */
		d = MAX(ax, ay);

		/* if the guy is too far then skip it. */
		if (d > dist)
			continue;

		/* Count Breeder types */
		if (r_ptr->flags2 & RF2_MULTIPLY)
			breeder_count++;

		/* Keep a total of how much danger they pose */
		total_danger += borg_danger(kill->y, kill->x, 1, TRUE);

		/*** Scan for Uniques ***/

		/* this is a unique. */
		if (kill->unique && borg_skill[BI_CLEVEL] >= 35) {
			/* Set a flag for use with certain types of spells */
			unique_on_level = kill->r_idx;

			/* regular unique */
			borg_fighting_unique = kill->r_idx;

			/* Note that fighting a Questor */
			if (r_ptr->flags3 & RF3_EVIL)
				borg_fighting_evil_unique = TRUE;
		}

		/* Is it a level quest monster ? */
		if (kill->questor && kill->los) {
			borg_fighting_questor = TRUE;
		}

		/* Is it a dragon?  Used in selecting swap weapon and kill brand */
		if ((r_ptr->flags3 & RF3_DRAGON) && kill->los) {
			borg_fighting_dragon = TRUE;
		}

		/* Is it a demon?  Used in selecting swap weapon and kill brand */
		if ((r_ptr->flags3 & RF3_DEMON) && kill->los) {
			borg_fighting_demon = TRUE;
		}

		/* Can it bore holes in the dungeon? */
		if (((r_ptr->flags2 & RF2_KILL_WALL) ||
			  (r_ptr->flags2 & RF2_PASS_WALL)) &&
			 kill->dist < 12) {
			borg_fighting_tunneler = TRUE;
		}

		/* Can it cast Teleport_to, which is moving the player */
		if ((r_ptr->flags6 & RF6_TELE_TO) && kill->los) {
			borg_fighting_tele_to = TRUE;
		}

		/* Can it cast Teleport, which can follow the player during his teleport
		 */
		if ((r_ptr->flags6 & RF6_TPORT) && kill->los &&
			 (kill->dist >= 2 && kill->dist <= 3)) {
			borg_fighting_chaser = TRUE;
		}

		/*** Scan for Scary Guys ***/

		/* run from certain scaries */
		if (borg_skill[BI_CLEVEL] < 3 &&
			 (strstr(r_name + r_ptr->name, "Squint") ||
			  strstr(r_name + r_ptr->name, "Fruit") ||
			  (strstr(r_name + r_ptr->name, "Insect") &&
				breeder_count > borg_skill[BI_CLEVEL])))
			borg_depth |= DEPTH_SCARY;

		/* run from certain dungeon scaries */
		if (borg_skill[BI_CLEVEL] <= 5 &&
			 (strstr(r_name + r_ptr->name, "Grip") ||
			  strstr(r_name + r_ptr->name, "Agent") ||
			  strstr(r_name + r_ptr->name, "Wolf,") ||
			  strstr(r_name + r_ptr->name, "Fang")))
			borg_depth |= DEPTH_SCARY;

		/* run from certain scaries */
		if (borg_skill[BI_CLEVEL] <= 6 &&
			 (strstr(r_name + r_ptr->name, "Small kobold") ||
			  strstr(r_name + r_ptr->name, "Kobold") ||
			  strstr(r_name + r_ptr->name, "Shrieker") ||
			  strstr(r_name + r_ptr->name, "Jackal") ||
			  strstr(r_name + r_ptr->name, "Filthy street urchin") ||
			  strstr(r_name + r_ptr->name, "Battle scarred veteran") ||
			  strstr(r_name + r_ptr->name, "Mean looking mercenary")))
			borg_depth |= DEPTH_SCARY;

		/* Run from fast guys, most of the time */
		if (borg_skill[BI_CLEVEL] <= 7 && kill->speed > borg_skill[BI_SPEED] &&
			 ((r_ptr->flags1 & RF1_RAND_50) || (r_ptr->flags1 & RF1_RAND_25)) &&
			 (!strstr(r_name + r_ptr->name, "Blubbering") &&
			  !strstr(r_name + r_ptr->name, "Pitiful") &&
			  !strstr(r_name + r_ptr->name, "Raving") &&
			  !strstr(r_name + r_ptr->name, "Aimless") &&
			  !strstr(r_name + r_ptr->name, "Hobo") &&
			  !strstr(r_name + r_ptr->name, "Singing")))
			borg_depth |= DEPTH_SCARY;

		if (borg_skill[BI_CLEVEL] <= 8 &&
			 (strstr(r_name + r_ptr->name, "Freesia") ||
			  ((strstr(r_name + r_ptr->name, "Giant white mouse") ||
				 strstr(r_name + r_ptr->name, "White worm mass") ||
				 strstr(r_name + r_ptr->name, "Grid bug") ||
				 strstr(r_name + r_ptr->name, "Green worm mass")) &&
				breeder_count > borg_skill[BI_CLEVEL])))
			borg_depth |= DEPTH_SCARY;

		if (borg_skill[BI_CLEVEL] <= 10 &&
			 (strstr(r_name + r_ptr->name, "Cave spider") ||
			  (strstr(r_name + r_ptr->name, "Yellow worm mass") &&
				breeder_count > borg_skill[BI_CLEVEL]) ||
			  strstr(r_name + r_ptr->name, "Pink naga") ||
			  strstr(r_name + r_ptr->name, "Giant pink frog") ||
			  strstr(r_name + r_ptr->name, "Radiation eye")))
			borg_depth |= DEPTH_SCARY;

		if (borg_skill[BI_CLEVEL] <= 25 &&
			 (strstr(r_name + r_ptr->name, "Phantom warrior")))
			borg_depth |= DEPTH_SCARY;

		if (borg_skill[BI_CLEVEL] <= 40 &&
			 (strstr(r_name + r_ptr->name, "Greater hell")))
			borg_depth |= DEPTH_SCARY;

		/* Nether breath is bad */
		if ((!borg_skill[BI_SRNTHR] && !borg_skill[BI_AXGOI]) &&
			 (strstr(r_name + r_ptr->name, "Azriel") ||
			  strstr(r_name + r_ptr->name, "Dracolich") ||
			  strstr(r_name + r_ptr->name, "Dracolisk")))
			borg_depth |= DEPTH_SCARY;

		/* Blindness is really bad */
		if ((!borg_skill[BI_SRBLIND]) &&
			 ((strstr(r_name + r_ptr->name, "Light hound") &&
				!borg_skill[BI_SRLITE]) ||
			  (strstr(r_name + r_ptr->name, "Dark hound") &&
				!borg_skill[BI_SRDARK])))
			borg_depth |= DEPTH_SCARY;

		/* Chaos and Confusion are really bad */
		if ((!borg_skill[BI_SRKAOS] && !borg_skill[BI_SRCONF]) &&
			 (strstr(r_name + r_ptr->name, "Chaos")))
			borg_depth |= DEPTH_SCARY;

		/* Cyberdemons are are really bad */
		if ((!borg_skill[BI_SRSHRD]) && (strstr(r_name + r_ptr->name, "Cyber")))
			borg_depth |= DEPTH_SCARY;

		/* Tarrasque kills with Disenchant */
		if (!borg_skill[BI_RDIS] && /* Note the RDisn not SRDisn */
			 (strstr(r_name + r_ptr->name, "Tarrasque")))
			borg_depth |= DEPTH_SCARY;

		/* Try to improve the borg's ability to attack the questor monsters while
		 * in munchkin mode */
		if (borg_munchkin_mode && borg_fighting_questor &&
			 (borg_depth & DEPTH_SCARY))
			borg_depth &= ~DEPTH_SCARY;

		/*** Scan for Summoners ***/

		if (kill->summoner) {
			/* mark the flag */
			borg_fighting_summoner = TRUE;

			/* Check if the summoner can flow to the borg. */
			borg_flow_clear_m();
			borg_flow_enqueue_grid_m(c_y, c_x);
			borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
			if (borg_flow_commit_m(kill->y, kill->x)) {
				/* recheck the distance to see if close and mark the index for
				 * as-corridor */
				if (borg_data_cost_m->data[kill->y][kill->x] < BORG_MON_FLOW)
					borg_kills_summoner = i;
			}
		}
	}

	return (total_danger);
}

/*
 * Help determine if "phase door" seems like a good idea
 */
bool borg_caution_phase(int emergency, int turns) {
	int n, k, i, d, x, y, p;

	int dis = 10;
	int min;

	borg_grid *ag = &borg_grids[c_y][c_x];

	/* Define minimal distance */
	min = dis / 2;

	/* Simulate 100 attempts */
	for (n = k = 0; k < 100; k++) {
		/* Pick a location */
		for (i = 0; i < 100; i++) {
			/* Pick a (possibly illegal) location */
			while (1) {
				y = rand_spread(c_y, dis);
				x = rand_spread(c_x, dis);
				d = distance(c_y, c_x, y, x);
				if ((d >= min) && (d <= dis))
					break;
			}

			/* Ignore illegal locations */
			if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 1))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* If low level, unknown squares are scary */
			if (ag->feat == FEAT_NONE && borg_skill[BI_MAXHP] < 30)
				break;

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip walls */
			if (!borg_cave_floor_bold(y, x))
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Stop looking */
			break;
		}

		/* If low level, unknown squares are scary */
		if (ag->feat == FEAT_NONE && borg_skill[BI_MAXHP] < 30) {
			n++;
			continue;
		}

		/* No location */
		/* in the real code it would keep trying but here we should */
		/* assume that there is unknown spots that you would be able */
		/* to go but may be dangerious. */
		if (i >= 100) {
			n++;
			continue;
		}

		/* Examine */
		p = borg_danger(y, x, turns, TRUE);

		/* if *very* scary, do not allow jumps at all */
		if (p > borg_skill[BI_CURHP])
			n++;
	}

	/* Too much danger */
	/* in an emergency try with extra danger allowed */
	if (n > emergency) {
		borg_note(format("# No Phase. scary squares: %d", n));
		return (FALSE);
	} else
		borg_note(format("# Safe to Phase. scary squares: %d", n));

	/* Okay */
	return (TRUE);
}

/* Attempt to cast Dimension Door to land on a particular spot */
bool borg_dim_door_to(int b_y, int b_x) {
	int y1, x1, y2, x2;
	int d, i;
	bool force_it = FALSE;

	/* Cheat for vault floor detection */
	/*cave_type *c_ptr;*/

	borg_grid *ag = &borg_grids[b_y][b_x];

	/* Legal grid */
	if (!in_bounds(b_y, b_x))
		return (FALSE);

	/* Is spell available */
	if (!borg_spell_okay_fail(REALM_SORCERY, 2, 3, 30) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 0, 5, 30) &&
		 !borg_mindcr_okay_fail(MIND_MINOR_DISP, 40, 30) &&
		 !borg_activate_activation(ACT_DIM_DOOR, FALSE))
		return (FALSE);

	/* Too far, or too close. out of Dim_door range */
	d = distance(c_y, c_x, b_y, b_x);
	if (d > borg_skill[BI_CLEVEL] + 2)
		return (FALSE);

	/* Sometimes we need to land on an occupied or unknown grid */
	if (borg_skill[BI_ISWEAK] && borg_skill[BI_VAMPIRE] && goal == GOAL_KILL)
		force_it = TRUE;

	/* Not in a vault, not occupied by monster */
	if (cave[b_y][b_x].info & (CAVE_ICKY))
		return (FALSE);
	if (borg_grids[b_y][b_x].kill)
		return (FALSE);

	/* Certain ground types are acceptable */
	if (ag->feat != FEAT_FLOOR && /* ag->feat != FEAT_SHAL_LAVA && */
		 ag->feat != FEAT_WATER && /*ag->feat != FEAT_GRASS &&*/
		 /*ag->feat != FEAT_DIRT && */ ag->feat != FEAT_GLYPH &&
		 (ag->feat != FEAT_NONE && force_it))
		return (FALSE);

	/* Track the bad landing zones.
	* I may have tried to jump onto this grid before and failed.
	* Maybe there is an object, feature, or unknown monster on it.
	*/
	for (i = 0; i < track_land_num; i++) {
		if (b_y == track_land_y[i] && b_x == track_land_x[i] &&
			 borg_t - track_land_when[i] < 50)
			return (FALSE);
	}

	/* do it */
	if (borg_spell(REALM_SORCERY, 2, 3) || borg_spell(REALM_TRUMP, 0, 5) ||
		 borg_mindcr(MIND_MINOR_DISP, 40) ||
		 borg_activate_activation(ACT_DIM_DOOR, FALSE)) {
		borg_note(format("# Dim Door: Starting: (%d, %d) Landing: (%d, %d)", c_y,
							  c_x, b_y, b_x));

		/** Select my landing zone **/
		/* Determine "path" */
		x1 = c_x;
		y1 = c_y;
		x2 = b_x;
		y2 = b_y;

		/* Move to the location (diagonals) */
		for (; (y1 < y2) && (x1 < x2); y1++, x1++)
			borg_keypress('3');
		for (; (y1 < y2) && (x1 > x2); y1++, x1--)
			borg_keypress('1');
		for (; (y1 > y2) && (x1 < x2); y1--, x1++)
			borg_keypress('9');
		for (; (y1 > y2) && (x1 > x2); y1--, x1--)
			borg_keypress('7');

		/* Move to the location */
		for (; y1 < y2; y1++)
			borg_keypress('2');
		for (; y1 > y2; y1--)
			borg_keypress('8');
		for (; x1 < x2; x1++)
			borg_keypress('6');
		for (; x1 > x2; x1--)
			borg_keypress('4');

		/* Select the target */
		borg_keypress(' ');

		/* Note the landing grid, in case we fail */
		if (track_land_num < track_land_size) {
			track_land_x[track_land_num] = b_x;
			track_land_y[track_land_num] = b_y;
			track_land_when[track_land_num] = borg_t;
			/* the fail message will be caught in borg_parse() and the counter will
			 * tick */
		}

		/* Success */
		return (TRUE);
	}

	/* Guess not */
	return (FALSE);
}

/*
 * Help determine if "dimension door" seems like a good idea
 */
bool borg_dim_door(int emergency, int p1) {
	int x, y, p;
	int b_y = -1, b_x = -1, b_p = p1;
	int dis = borg_skill[BI_CLEVEL] - 1;
	/*int x1, y1, x2, y2;*/
	int dx, b_dx = -1;

	borg_grid *ag = &borg_grids[c_y][c_x];

	/* Do we have Dim Door spell ? */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* Scan every grid in landing zone */
	for (y = c_y - dis; y < c_y + dis; y++) {
		/* Pick a location */
		for (x = c_x - dis; x < c_x + dis; x++) {

			/* Ignore illegal locations */
			if (y < w_y + 1 || y > w_y + SCREEN_HGT - 1)
				continue;
			if (x < w_x + 1 || x > w_x + SCREEN_WID - 1)
				continue;
			if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 1))
				continue;
			if ((x == c_x) && (y == c_y))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Verify distance again */
			if (distance(y, x, c_y, c_x) > borg_skill[BI_CLEVEL] + 2)
				continue;

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip Icky Grids (vault type) */
			if (cave[y][x].info & (CAVE_ICKY))
				continue;

			/* Certain ground types are acceptable */
			if (ag->feat != FEAT_FLOOR && /* ag->feat != FEAT_SHAL_LAVA && */
				 ag->feat != FEAT_WATER		/* ag->feat != FEAT_GRASS && */
				 /*ag->feat != FEAT_DIRT*/)
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Skip items */
			if (ag->take)
				continue;

			/* Examine */
			p = borg_danger(y, x, 1, TRUE);

			/* if *very* scary, do not allow jumps at all */
			if (!emergency && p > borg_skill[BI_CURHP])
				continue;

			/* Track the distance so we select a spot which is far away, and low
			 * danger */
			dx = distance(y, x, c_y, c_x);

			/* Track the grid with the least danger and further away */
			if (p > b_p)
				continue;
			if (dx < b_dx)
				continue;

			/* We should have a preference for grids surrounded by walls since they
			 * provide some LOS cover. */

			/* note good landing zones */
			b_p = p;
			b_y = y;
			b_x = x;
			b_dx = dx;
		}
	}

	/* Dimension Door report */
	borg_note(format("# Dim Door: Safest grid: (%d, %d) with %d Danger", b_y,
						  b_x, b_p));

	/* No good landing zone */
	if (b_p >= p1 * 7 / 10)
		return (FALSE);

	/* do it */
	if (borg_dim_door_to(b_y, b_x))
		return (TRUE);

	/* Okay */
	return (FALSE);
}

/*
 * Help determine if "phase door" with Shoot N Scoot seems like
 * a good idea.
 * Good Idea on two levels:
 * 1.  We are the right class, we got some good ranged weapons
 * 2.  The possible landing grids are ok.
 * Almost a copy of the borg_caution_phase above.
 * The emergency is the number of dangerous grids out of 100
 * that we tolerate.  If we have 80, then we accept the risk
 * of landing on a grid that is 80% likely to be bad.  A low
 * number, like 20, means that we are less like to risk the
 * phase door and we require more of the possible grids to be
 * safe.
 *
 * The pattern of ShootN'Scoot works like this:
 * 1. Shoot monster that is far away.
 * 2. Monsters walks closer and closer each turn
 * 3. Borg shoots monster each step it takes as it approaches.
 * 4. Monster gets within 1 grid of the borg.
 * 5. Borg phases away.
 * 6. Go back to #1
 */
bool borg_shoot_scoot_safe(int emergency, int turns, int b_p) {
	int n, k, i, d, x, y, p, u;

	int dis = 10;
	int allow_fail = 20;
	int min = dis / 2;

	bool adjacent_monster = FALSE;

	borg_grid *ag;
	borg_kill *kill;
	monster_race *r_ptr;

	/* no need if high level in town */
	if (borg_skill[BI_CLEVEL] >= 8 && borg_skill[BI_CDEPTH] == 0)
		return (FALSE);

	/* Not if fighting a unique */
	if (borg_fighting_unique)
		return (FALSE);

	/* not if fighting a summoner; finish them faster */
	if (borg_fighting_summoner)
		return (FALSE);

	/* Not if in a good position */
	if ((borg_position & POSITION_SUMM) && borg_fighting_summoner &&
		 !borg_fighting_chaser)
		return (FALSE);

	/* Not if too deep */
	if (borg_skill[BI_CDEPTH] > 70 && !borg_fighting_chaser)
		return (FALSE);

	/* must have phase ability, */
	if (!borg_spell_okay_fail(REALM_ARCANE, 0, 4, allow_fail) &&
		 !borg_spell_okay_fail(REALM_SORCERY, 0, 1, allow_fail) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 0, 0, allow_fail) &&
		 !borg_mindcr_okay_fail(MIND_MINOR_DISP, 3, allow_fail) &&
		 !borg_mutation(COR1_BLINK, TRUE, allow_fail, FALSE) &&
		 !borg_equips_artifact(ART_DRAEBOR) && !borg_equips_artifact(ART_NYNAULD))
		return (FALSE);
	/* And not have Dim Door spell */
	if (borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* Cheat the floor grid */
	/* Not if in a vault since it throws us out of the vault */
	if (cave[c_y][c_x].info & (CAVE_ICKY))
		return (FALSE);

	/*** Need Missiles or cheap spells ***/

	/* Mage Priest */
	if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST ||
		 borg_class == CLASS_ORPHIC || borg_class == CLASS_HIGH_MAGE) {
		/* Low mana */
		if (borg_skill[BI_CLEVEL] >= 45 && borg_skill[BI_CURSP] < 15)
			return (FALSE);

		/* Low mana, low level, generally OK */
		if (borg_skill[BI_CLEVEL] < 45 && borg_skill[BI_CURSP] < 5)
			return (FALSE);
	} else /* Other classes need some missiles */
	{
		if (borg_skill[BI_AMISSILES] < 5 || borg_skill[BI_CLEVEL] >= 45)
			return (FALSE);
	}

	/* Not if I am in a safe spot for killing special monsters */
	if ((borg_position & (POSITION_SEA | POSITION_SUMM | POSITION_BORE)))
		return (FALSE);

	/* scan the adjacent grids for an awake monster */
	for (i = 0; i < 8; i++) {
		/* Grid in that direction */
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Obtain the monster */
		kill = &borg_kills[ag->kill];
		r_ptr = &r_info[kill->r_idx];

		/* If a qualifying monster is adjacent to me. */
		if ((ag->kill && kill->awake && kill->killer) &&
			 !(r_ptr->flags1 & RF1_NEVER_MOVE) &&
			 !(r_ptr->flags2 & RF2_PASS_WALL) &&
			 !(r_ptr->flags2 & RF2_KILL_WALL) &&
			 (kill->power >= borg_skill[BI_CLEVEL])) {
			/* Spell casters shoot at everything */
			if (borg_class == CLASS_PRIEST || borg_class == CLASS_MAGE ||
				 borg_class == CLASS_HIGH_MAGE || borg_class == CLASS_ORPHIC) {
				adjacent_monster = TRUE;
			}

			/* All other borgs need to make sure he would shoot.
			 * In an effort to conserve missiles, the borg will
			 * not shoot at certain types of monsters.  That list
			 * is defined in borg_launch_damage_one().
			 *
			 * We need this aforementioned list to match the one
			 * following.  Otherwise Rogues and Warriors will
			 * burn up Phases as he scoots away but never fire
			 * the missiles.  That totally defeats the purpose
			 * of this routine.
			 *
			 * The following criteria are exactly the same as the
			 * list in borg_launch_damage_one()
			 */
			else if ((borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE) >
						 avoidance * 3 / 10) ||
						((r_ptr->flags1 & RF1_FRIENDS) /* monster has friends*/ &&
						 kill->level >=
							  borg_skill[BI_CLEVEL] - 5 /* close levels */) ||
						(kill->ranged_attack /* monster has a ranged attack */) ||
						(kill->unique) || (r_ptr->flags2 & RF2_MULTIPLY) ||
						(borg_skill[BI_CLEVEL] <= 5 /* stil very weak */)) {
				adjacent_monster = TRUE;
			}
		}
	}

	/* if No Adjacent_monster no need for it */
	if (adjacent_monster == FALSE && !borg_fighting_chaser)
		return (FALSE);

	/* Simulate 100 attempts */
	for (n = k = 0; k < 100; k++) {
		/* Pick a location */
		for (i = 0; i < 100; i++) {
			/* Pick a (possibly illegal) location */
			while (1) {
				y = rand_spread(c_y, dis);
				x = rand_spread(c_x, dis);
				d = distance(c_y, c_x, y, x);
				if ((d >= min) && (d <= dis))
					break;
			}

			/* Ignore illegal locations */
			if ((y <= 0) || (y >= AUTO_MAX_Y - 2))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 2))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip walls */
			if (!borg_cave_floor_bold(y, x))
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Stop looking.  Really, the game would keep
			 * looking for a grid.  The borg could check
			 * all the known grids but I dont think that
			 * is not a good idea, especially if the area is
			 * not fully explored.
			 */
			break;
		}

		/* No location */
		/* In the real code it would keep trying but here we should */
		/* assume that there is unknown spots that you would be able */
		/* to go but we define it as dangerous. */
		if (i >= 100) {
			n++;
			continue;
		}

		/* Examine danger of that grid */
		p = borg_danger(y, x, turns, TRUE);

		/* if more scary than my current one, do not allow jumps at all */
		if (p > b_p) {
			n++;
			continue;
		}

		/* Should not land next to a monster either.
		  * Scan the adjacent grids for a monster.
		  * Reuse the adjacent_monster variable.
		  */
		for (u = 0; u < 8; u++) {
			/* Access the grid */
			ag = &borg_grids[y + ddy_ddd[u]][x + ddx_ddd[u]];

			/* Obtain the monster */
			kill = &borg_kills[ag->kill];

			/* If monster adjacent to that grid...
			 */
			if (ag->kill && kill->awake)
				n++;
		}
	}

	/* Too much danger */
	/* in an emergency try with extra danger allowed */
	if (n > emergency) {
		borg_note(format("# No Shoot'N'Scoot. scary squares: %d/100", n));
		return (FALSE);
	} else
		borg_note(format("# Safe to Shoot'N'Scoot. scary squares: %d/100", n));

	/* Okay */
	return (TRUE);
}
/*
 * Help determine if shoot and scoot with "dimension door" seems like a good
 * idea
 */
bool borg_shoot_scoot_dim(int p1) {
	int x, y, p;
	int b_y = -1, b_x = -1, b_p = p1;
	int dis = borg_skill[BI_CLEVEL] + 2;
	/*int x1, y1, x2, y2;*/
	int i, b_i;

	borg_grid *ag = &borg_grids[c_y][c_x];
	borg_kill *kill;
	monster_race *r_ptr;

	/* No adjacent monster index yet */
	b_i = -1;

	/* Too expensive on mana early on */
	if (borg_skill[BI_CURSP] < 25)
		return (FALSE);

	/* Do we have Dim Door spell ? */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* Not if in a good position */
	if ((borg_position & POSITION_SUMM) && borg_fighting_summoner)
		return (FALSE);

	/* scan the adjacent grids for an awake monster */
	for (i = 0; i < 8; i++) {
		/* Grid in that direction */
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Obtain the monster */
		kill = &borg_kills[ag->kill];
		r_ptr = &r_info[kill->r_idx];

		/* Skip non-monster grids */
		if (!ag->kill)
			continue;

		/* If a qualifying monster is adjacent to me. */
		if ((kill->awake && kill->killer) && !(r_ptr->flags1 & RF1_NEVER_MOVE) &&
			 !(r_ptr->flags2 & RF2_PASS_WALL) &&
			 !(r_ptr->flags2 & RF2_KILL_WALL) /* &&
		     (kill->power >= borg_skill[BI_CLEVEL]) */) {
			/* Spell casters shoot at everything */
			if (borg_class == CLASS_PRIEST || borg_class == CLASS_MAGE ||
				 borg_class == CLASS_HIGH_MAGE || borg_class == CLASS_ORPHIC) {
				/* Index of an adjacent monster */
				b_i = ag->kill;
			}

			/* All other borgs need to make sure he would shoot.
			 * In an effort to conserve missiles, the borg will
			 * not shoot at certain types of monsters.  That list
			 * is defined in borg_launch_damage_one().
			 *
			 * We need this aforementioned list to match the one
			 * following.  Otherwise Rogues and Warriors will
			 * burn up Phases as he scoots away but never fire
			 * the missiles.  That totally defeats the purpose
			 * of this routine.
			 *
			 * The following criteria are exactly the same as the
			 * list in borg_launch_damage_one()
			 */
			else if ((borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE) >
						 avoidance * 3 / 10) ||
						((r_ptr->flags1 & RF1_FRIENDS) /* monster has friends*/ &&
						 kill->level >=
							  borg_skill[BI_CLEVEL] - 5 /* close levels */) ||
						(kill->ranged_attack /* monster has a ranged attack */) ||
						(kill->unique) || (r_ptr->flags2 & RF2_MULTIPLY) ||
						(borg_skill[BI_CLEVEL] <= 5 /* stil very weak */)) {
				/* Index of an adjacent monster */
				b_i = ag->kill;
			}
		}
	}

	/* No need if no monster is adjacent */
	if (b_i == -1)
		return (FALSE);

	/* Scan every grid in landing zone */
	for (y = c_y - dis; y < c_y + dis; y++) {
		/* Pick a location */
		for (x = c_x - dis; x < c_x + dis; x++) {

			/* Ignore illegal locations */
			if (y < w_y + 1 || y > w_y + SCREEN_HGT - 1)
				continue;
			if (x < w_x + 1 || x > w_x + SCREEN_WID - 1)
				continue;
			if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 1))
				continue;
			if ((x == c_x) && (y == c_y))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Verify distance again */
			if (distance(y, x, c_y, c_x) > borg_skill[BI_CLEVEL])
				continue;
			if (borg_kills[b_i].speed > borg_skill[BI_SPEED] &&
				 distance(y, x, c_y, c_x) < 7)
				continue;
			if (borg_kills[b_i].speed <= borg_skill[BI_SPEED] &&
				 distance(y, x, c_y, c_x) < 5)
				continue;
			if (distance(y, x, borg_kills[b_i].y, borg_kills[b_i].x) < 5)
				continue;

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip Icky Grids (vault type) */
			if (cave[y][x].info & (CAVE_ICKY))
				continue;

			/* Certain ground types are not acceptable */
			if (ag->feat != FEAT_FLOOR && /* ag->feat != FEAT_SHAL_LAVA && */
				 ag->feat != FEAT_WATER		/*ag->feat != FEAT_GRASS && */
				 /*ag->feat != FEAT_DIRT*/)
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Skip items */
			if (ag->take)
				continue;

			/* Skip grids that do not allow me to stay LOS */
			if (!borg_los(borg_kills[b_i].y, borg_kills[b_i].x, y, x))
				continue;

			/* Examine */
			p = borg_danger(y, x, 1, TRUE);

			/* Track the grid with the least danger */
			if (p > b_p)
				continue;

			/* note good landing zones */
			b_p = p;
			b_y = y;
			b_x = x;
		}
	}

	/* No good landing zone */
	if (b_p >= p1 * 7 / 10)
		return (FALSE);

	/* do it */
	if (borg_dim_door_to(b_y, b_x)) {
		borg_note(format(
			 "# Dim Door: ShootN'Scoot. Starting: (%d, %d) Landing: (%d, %d)", c_y,
			 c_x, b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* Okay */
	return (FALSE);
}

/*
 * Help determine if mutation "Swap Position" seems like a good idea.
 */
bool borg_swap_position(int p1) {
	int /*x, y, */ p;

	int i;
	/*int b_i = -1;*/
	int b_y = -1, b_x = -1, b_p = p1;

	borg_grid *ag;
	/*borg_kill *kill;*/
	monster_race *r_ptr;

	/* Do I have the mutation ? */
	if (!borg_mutation(COR1_SWAP_POS, TRUE, 25, FALSE))
		return (FALSE);

	/* Scan every monster, looking to swap places with */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Monster race */
		r_ptr = &r_info[kill->r_idx];

		/* Ignore illegal locations */
		if ((kill->y <= 0) || (kill->y >= AUTO_MAX_Y - 1))
			continue;
		if ((kill->x <= 0) || (kill->x >= AUTO_MAX_X - 1))
			continue;
		if ((kill->x == c_x) && (kill->y == c_y))
			continue;

		/* Make sure grid is on the panel */
		if (kill->y < w_y || kill->y > w_y + SCREEN_HGT)
			continue;
		if (kill->x < w_x || kill->x > w_x + SCREEN_WID)
			continue;

		/* Access grid */
		ag = &borg_grids[kill->y][kill->x];

		/* Skip unknown grids */
		if (ag->feat == FEAT_NONE)
			continue;

		/* Some monsters resist */
		if (r_ptr->flags3 & RF3_RES_TELE)
			continue;

		/* Skip weird grids */
		if (ag->feat == FEAT_INVIS)
			continue;

		/* Remove this monster from the list in order to see what the danger will
		 * be with it moved. */
		borg_swap_pos_index = ag->kill;
		borg_swap_pos_y = kill->y;
		borg_swap_pos_x = kill->x;

		/* Examine the landing zone */
		borg_swap_pos = TRUE;
		p = borg_danger(kill->y, kill->x, 1, TRUE);
		borg_swap_pos = FALSE;

		/* Track the grid with the least danger */
		if (p > b_p)
			continue;

		/* note good landing zones */
		b_p = p;
		b_y = kill->y;
		b_x = kill->x;
		/*b_i = i;*/
	}

	/* No good landing zone (needs to be worth it) */
	if (b_p >= p1 * 7 / 10)
		return (FALSE);

	/* Target the good landing zone */
	borg_note(
		 format("# Swap Position target: Safest grid: (%d, %d) with %d Danger",
				  b_y, b_x, b_p));
	borg_target(b_y, b_x);

	/* Execute the spell */
	if (borg_mutation(COR1_SWAP_POS, FALSE, 30, FALSE)) {
		/* Get the targetted spot */
		borg_keypress('5');
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Check to see if it is a good idea to use DimDoor to jump into a pit then
 * blast the pit with a dispel spell.  In the end, type of jump will save more
 * mana and actually be much safer for the borg.
 *
 * Several considerations must be met.
 * 1. Does the borg have Dim Door, or the ability to swap positions with a
 *monster
 * 2. Does the borg have sufficient Dispel attacks? and mana to cast a couple of
 *them
 * 3. Is he already in a choice spot
 * 4. Is there a good spot to do such a thing
 *    - Lots of monsters which have LOS
 *	  - Monsters need to qualify and be suseptable to the attack type.
 *	  - Spot must be available for a landing position
 *    - Swap Position might work too
 *
 * Other sources of Dim Door include the Trumps and the Sorcs realms.  But those
 *guys need
 * really good dispel attacks as well.
 */
static bool borg_jump_into_pit(void) {
	int range = borg_skill[BI_CLEVEL];
	int b_y = c_y;
	int b_x = c_x;
	int b_n = -1;
	int y, x, i;
	/*int y1, y2;
	int x1, x2;*/
	int n;
	bool bad_spot = FALSE;

	borg_grid *ag;
	/*borg_kill *kill;
	monster_race *r_ptr;*/

	/* Does the borg have DimDoor */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* or does the borg have Swap Position */

	/* Does the borg have sufficient Dispel Attacks, including lots of mana */
	if (!borg_mindcr_okay_fail(MIND_MIND_WAVE, 25, 20))
		return (FALSE);
	if (borg_minds[MIND_MIND_WAVE].power * 3 > borg_skill[BI_CURSP])
		return (FALSE);

	/* Not if too wounded */
	if (borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)
		return (FALSE);

	/* First obtain a base count of how many monsters have LOS to the borg */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];
		monster_race *r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Seen it recently? */
		if (borg_t - kill->when > 5)
			continue;

		/* Does this monster have LOS to this grid */
		if (!kill->los)
			continue;

		/* Is this monster suseptable to my attack type */
		/* Right now it is only mindwave, but we need to add support for
		 * dispel evil, undead
		 */
		if (r_ptr->flags2 & RF2_EMPTY_MIND)
			continue;
		if (r_ptr->flags3 & RF3_DEMON)
			continue;
		if (r_ptr->flags3 & RF3_UNDEAD)
			continue;
		if (kill->level >= borg_skill[BI_CLEVEL])
			continue;

		/* Tally the number of monsters */
		b_n++;
	}

	/* Scan every grid to which I could teleport */
	for (y = c_y - range; y < c_y + range; y++) {
		for (x = c_x - range; x < c_x + range; x++) {
			/* Ignore illegal locations */
			if (y < w_y + 1 || y > w_y + SCREEN_HGT - 1)
				continue;
			if (x < w_x + 1 || x > w_x + SCREEN_WID - 1)
				continue;
			if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 1))
				continue;
			if ((x == c_x) && (y == c_y))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip Icky Grids (vault type) */
			if (cave[y][x].info & (CAVE_ICKY))
				continue;

			/* Certain ground types are unacceptable */
			if (ag->feat != FEAT_FLOOR && /*ag->feat != FEAT_SHAL_LAVA && */
				 ag->feat != FEAT_WATER		/* ag->feat != FEAT_GRASS && */
				 /*ag->feat != FEAT_DIRT*/)
				continue;

			/* Skip monsters */
			if (ag->kill || cave[y][x].m_idx)
				continue;

			/* Skip items */
			if (ag->take)
				continue;

			/* Track the bad landing zones.
			 * I may have tried to jump onto this grid before and failed.
			 * Maybe there is an object, feature, or unknown monster on it.
			 */
			bad_spot = FALSE;
			for (i = 0; i < track_land_num; i++) {
				bad_spot = FALSE;

				if (y == track_land_y[i] && x == track_land_x[i] &&
					 borg_t - track_land_when[i] < 50)
					bad_spot = TRUE;
			}
			if (bad_spot == TRUE)
				continue;

			/* Reset the tally for the number of monsters who can see this grid */
			n = 0;

			/* Scan the monster list */
			for (i = 0; i < borg_kills_nxt; i++) {
				borg_kill *kill = &borg_kills[i];
				monster_race *r_ptr = &r_info[kill->r_idx];

				/* Skip dead monsters */
				if (!kill->r_idx)
					continue;

				/* Does this monster have LOS to this grid */
				if (!borg_los(y, x, kill->y, kill->x))
					continue;

				/* Is this monster a deep summoner? if so, do not attempt */
				if (kill->level >= 50 && kill->summoner)
					return (FALSE);

				/* Backup check to make sure a monster is not on this grid */
				if (kill->y == y && kill->x == x) {
					break;
				}

				/* Is this monster suseptable to my attack type */
				/* Right now it is only mindwave, but we need to add support for
				 * dispel evil, undead
				 */
				if (r_ptr->flags2 & RF2_EMPTY_MIND)
					continue;
				if (r_ptr->flags3 & RF3_DEMON)
					continue;
				if (r_ptr->flags3 & RF3_UNDEAD)
					continue;
				if (kill->level >= borg_skill[BI_CLEVEL])
					continue;

				/* need to have seen it recently */
				if (borg_t - kill->when > 5)
					continue;

				/* Skip friendlies */
				if (kill->ally)
					continue;

				/* Tally the number of monsters */
				n++;

				/* is there a threshhold met */
				if (n < 5)
					continue;

				/* is it much better than my existing place */
				if (n - 5 <= b_n)
					continue;

				/* Note the grid */
				b_n = n;
				b_y = y;
				b_x = x;
			}
		}
	}

	/* Is it a good idea? */
	if (b_n < BORG_PIT_JUMP)
		return (FALSE);
	if (b_y == c_y && b_x == c_x)
		return (FALSE);

	/* Perform the jump */
	if (borg_dim_door_to(b_y, b_x)) {
		borg_note(
			 format("# Dim Door: Jump into pit. Count: %d. Landing: (%d, %d)", b_n,
					  b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* I guess I cant do it */
	return (FALSE);
}

/*
 * Check to see if it is a good idea to use DimDoor to jump into a hallway.
 *
 * Several considerations must be met.
 * 1. Does the borg have Dim Door, or the ability to swap positions with a
 * monster
 * 2. Is he fighting a summoner out in the open.
 * 3. Is he already in a choice spot
 * 4. Is there a good spot to do such a thing
 */
static bool borg_jump_into_hall(void) {
	int range = borg_skill[BI_CLEVEL];
	int b_y = c_y;
	int b_x = c_x;
	int b_n = 0;
	int y, x, i;
	/*int y1, y2;*/
	/*int x1, x2;*/
	/*int n;*/
	bool bad_spot = FALSE;
	int adjacent_floor;
	int d, b_d = range;

	borg_grid *ag;
	/*borg_kill *kill;
	monster_race *r_ptr;*/

	/* Does the borg have DimDoor */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* or does the borg have Swap Position */

	/* Am I currently in a hall way? */
	adjacent_floor = 0;
	for (i = 0; i < 8; i++) {
		/* check for bounds */
		if (!in_bounds(c_y + ddy_ddd[i], c_x + ddx_ddd[i]))
			continue;

		/* check for floor grid */
		if (borg_grids[c_y + ddy_ddd[i]][c_x + ddx_ddd[i]].feat == FEAT_FLOOR)
			adjacent_floor++;
	}

	/* Currently in a hallway or tucked into a corner */
	if (adjacent_floor <= 2)
		return (FALSE);

	/* First obtain a base count of how many summoners have LOS to the borg */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];
		/*monster_race *r_ptr = &r_info[kill->r_idx];*/

		/* Skip dead and asleep monsters*/
		if (!kill->r_idx)
			continue;
		if (!kill->awake)
			continue;

		/* Seen it recently? */
		if (borg_t - kill->when > 5)
			continue;

		/* Only summoners */
		if (!kill->summoner)
			continue;

		/* Does this monster have LOS to this grid */
		if (!kill->los)
			continue;

		/* If currently fighting a special type of monster, dont do this at all */
		if ((r_info[kill->r_idx].flags2 & RF2_KILL_WALL) &&
			 (kill->unique || kill->questor))
			return (FALSE);

		/* We skip the boreres */
		if (r_info[kill->r_idx].flags2 & RF2_PASS_WALL)
			continue;
		if (r_info[kill->r_idx].flags2 & RF2_KILL_WALL)
			continue;

		/* Tally the number of monsters */
		b_n++;
	}

	/* No Summoner LOS to me */
	if (b_n <= 0)
		return (FALSE);

	/* Scan every grid to which I could teleport */
	for (y = c_y - range; y < c_y + range; y++) {
		for (x = c_x - range; x < c_x + range; x++) {
			/* Ignore illegal locations */
			if (y < w_y + 1 || y > w_y + SCREEN_HGT - 1)
				continue;
			if (x < w_x + 1 || x > w_x + SCREEN_WID - 1)
				continue;
			if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
				continue;
			if ((x <= 0) || (x >= AUTO_MAX_X - 1))
				continue;
			if ((x == c_x) && (y == c_y))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip Icky Grids (vault type) */
			if (cave[y][x].info & (CAVE_ICKY))
				continue;

			/* Certain ground types are unacceptable */
			if (ag->feat != FEAT_FLOOR && /* ag->feat != FEAT_SHAL_LAVA && */
				 ag->feat != FEAT_WATER		/* ag->feat != FEAT_GRASS && */
				 /*ag->feat != FEAT_DIRT*/)
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Skip items */
			if (ag->take)
				continue;

			/* check to see if this grid is in a hall, or is sufficiently secluded
			 */
			adjacent_floor = 0;
			for (i = 0; i < 8; i++) {
				/* check for bounds */
				if (!in_bounds(y + ddy_ddd[i], x + ddx_ddd[i]))
					continue;

				/* check for floor grid */
				if (borg_grids[y + ddy_ddd[i]][x + ddx_ddd[i]].feat == FEAT_FLOOR)
					adjacent_floor++;
			}
			/* Try to select a spot that is in a hallway or tucked into a corner */
			if (adjacent_floor > 2)
				continue;

			/* Track the bad landing zones.
			 * I may have tried to jump onto this grid before and failed.
			 * Maybe there is an object, feature, or unknown monster on it.
			 */
			bad_spot = FALSE;
			for (i = 0; i < track_land_num; i++) {
				bad_spot = FALSE;

				if (y == track_land_y[i] && x == track_land_x[i] &&
					 borg_t - track_land_when[i] < 50)
					bad_spot = TRUE;
			}
			if (bad_spot == TRUE)
				continue;

			/* Jump to the closer ones. but don't jump 1 grid (waste of mana) */
			d = distance(c_y, c_x, y, x);
			if (d > b_d || d == 1)
				continue;

			/* The grid meets our criteria.  Remember it. */
			b_y = y;
			b_x = x;
			b_d = d;
		}
	}

	/* Is it a good idea? */
	if (b_y == c_y && b_x == c_x)
		return (FALSE);

	/* Perform the jump */
	if (borg_dim_door_to(b_y, b_x)) {
		borg_note(
			 format("# Dim Door: Jump into hall. Count: %d Landing: (%d, %d)", b_n,
					  b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* I guess I cant do it */
	return (FALSE);
}

/*
 * Hack -- If the borg is standing on a stair and is in some danger, just leave
 * the level.
 * No need to hang around on that level, try conserving the teleport scrolls
 */
static bool borg_escape_stair(void) {
	/* Current grid */
	borg_grid *ag = &borg_grids[c_y][c_x];

	/* Usable stairs */
	if (ag->feat == FEAT_LESS) {
		if ((borg_skill[BI_MAXDEPTH] - 4) > borg_skill[BI_CDEPTH] &&
			 borg_skill[BI_MAXCLEVEL] >= 35) {
			borg_note("scumming");
			auto_scum = TRUE;
		}

		/* Take the stairs */
		if (dungeon_stair)
			borg_on_dnstairs = TRUE;
		borg_note("# Escaping level via stairs.");
		borg_keypress('<');

		/* Success */
		return (TRUE);
	}

	return (FALSE);
}

/*
 * Try to phase door or teleport
 * b_q is the danger of the least dangerious square around us.
 */
bool borg_escape(int b_q) {

	bool nasties = FALSE;
	int risky_boost = 0;
	int q_regional = 0;
	int i, b_i = -1;
	bool special_position = FALSE;

	/* only escape with spell if fail is low */
	int allow_fail = 25;
	int sv_mana;
	borg_grid *ag = &borg_grids[c_y][c_x];

	/* Not if locked down */
	if (borg_skill[BI_CRSNOTELE])
		return (FALSE);

	/* Not in munchkin mode */
	if (borg_munchkin_mode)
		return (FALSE);

	/* Reduce the fear by the regional amount */
	q_regional = borg_fear_region[c_y / 11][c_x / 11];

	/* if very healthy, allow extra fail */
	if (((borg_skill[BI_CURHP] * 100) / borg_skill[BI_MAXHP]) > 70)
		allow_fail = 10;

	/* comprimised, get out of the fight */
	if (borg_skill[BI_ISHEAVYSTUN])
		allow_fail = 35;

	/* Is borg is a sea of runes or anti-summon corridor */
	special_position =
		 ((borg_position & (POSITION_SEA | POSITION_SUMM | POSITION_BORE)));

	/* Find the number of nasties in order of nastiness */
	for (i = 0; i < borg_nasties_num; i++) {
		/* Skipping the hounds */
		if (borg_nasties_count[i] >= borg_nasties_limit[i] &&
			 borg_nasties[i] != 'Z') {
			b_i = i;
			nasties = TRUE;
		}
	}

	/* for emergencies */
	sv_mana = borg_skill[BI_CURSP];

	/* Borgs who are bleeding to death or dying of poison may sometimes
	 * phase around the last two hit points right before they enter a
	 * shop.  He knows to make a bee-line for the temple but the danger
	 * trips this routine.  So we must bypass this routine for some
	 * particular circumstances.
	 */
	if (!borg_skill[BI_CDEPTH] &&
		 (borg_skill[BI_ISPOISONED] || borg_skill[BI_ISWEAK] ||
		  borg_skill[BI_ISCUT]))
		return (FALSE);

	/* Hack -- If the borg is weak (no food, starving) on depth 1 and he has no
	 * idea where the stairs
	 * may be, run the risk of diving deeper against the benefit of rising to
	 * town.
	 */
	if (borg_skill[BI_ISWEAK] && borg_skill[BI_CDEPTH] == 1) {
		if (borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL)) {
			borg_note("# Attempting to get to town immediately");
			return (TRUE);
		}
	}

	/* Borgs with GOI should not escape until the GOI falls */
	if (borg_goi || borg_wraith)
		return (FALSE);

	/* Risky borgs are more likely to stay in a fight */
	if (borg_plays_risky)
		risky_boost = 3;

	/* 1. really scary, I'm about to die */
	/* Try an emergency teleport, or phase door as last resort */
	if (borg_skill[BI_ISHEAVYSTUN] ||
		 (b_q > avoidance * (45 + risky_boost) / 10) ||
		 ((b_q > avoidance * (40 + risky_boost) / 10) && borg_fighting_unique &&
		  borg_skill[BI_CDEPTH] == 100 && borg_skill[BI_CURHP] < 600) ||
		 ((b_q > avoidance * (30 + risky_boost) / 10) && borg_fighting_unique &&
		  borg_skill[BI_CDEPTH] == 99 && borg_skill[BI_CURHP] < 600) ||
		 ((b_q > avoidance * (25 + risky_boost) / 10) && borg_fighting_unique &&
		  borg_fighting_unique <= 8 && borg_skill[BI_CDEPTH] >= 95 &&
		  borg_skill[BI_CURHP] < 550) ||
		 ((b_q > avoidance * (20 + risky_boost) / 10) && borg_fighting_unique &&
		  borg_skill[BI_CDEPTH] < 95) ||
		 ((b_q > avoidance * (15 + risky_boost) / 10) && !borg_fighting_unique)) {

		int allow_fail = 15;

		if (borg_escape_stair() ||
			 /* borg_dim_door(TRUE, b_q) || */
			 (borg_skill[BI_CDEPTH] == 0 &&
			  borg_read_scroll(SV_SCROLL_PHASE_DOOR)) ||
			 (borg_count_summoners >= 10 && !borg_fighting_unique &&
			  (borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL) ||
				borg_racial(NEPHILIM, 1))) ||
			 borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail - 10) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail - 10) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail - 10) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail - 10) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail - 10) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail - 10, FALSE) ||
			 borg_swap_position(b_q) || borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_activate_planar(allow_fail) ||
			 /* revisit spells, increased fail rate */
			 borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail + 9) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail + 9) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail + 9) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail + 9) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail + 9) ||
			 borg_racial(RACE_GNOME, 1) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail + 10, FALSE) ||
			 /* Attempt Teleport Level */
			 borg_spell_fail(REALM_SORCERY, 2, 6, allow_fail + 9) ||
			 borg_spell_fail(REALM_TRUMP, 1, 5, allow_fail + 9) ||
			 borg_spell_fail(REALM_ARCANE, 3, 1, allow_fail + 9) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL) ||
			 borg_racial(NEPHILIM, 1) ||
			 borg_activate_artifact(ART_ANDROMALIUS, FALSE) ||
			 borg_activate_planar(allow_fail + 9) ||
			 /* revisit teleport, increased fail rate */
			 borg_use_staff(SV_STAFF_TELEPORTATION) ||
			 /* try phase -- except if the fear is due to regional */
			 (q_regional < b_q / 2 &&
			  (borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail)))) {
			/* Flee! */
			borg_note("# Danger Level 1.");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			return (TRUE);
		}

		borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

		/* try to teleport, get far away from here */
		if (borg_spell(REALM_ARCANE, 2, 3) || borg_spell(REALM_TRUMP, 0, 4) ||
			 borg_spell(REALM_CHAOS, 0, 7) || borg_spell(REALM_SORCERY, 0, 5)) {
			/* verify use of spell */
			/* borg_keypress('y');  */

			/* Flee! */
			borg_note("# Danger Level 1.1  Critical Attempt");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			return (TRUE);
		}

		/* emergency phase spell */
		if (q_regional < b_q / 2) {
			if (borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				 (borg_caution_phase(80, 5) &&
				  (borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
				/* Flee! */
				borg_escapes--; /* a phase isn't really an escape */
				borg_note("# Danger Level 1.2  Critical Phase");
				/* Reset timer if borg was in a anti-summon corridor */
				if (borg_t - borg_t_position < 50)
					borg_t_position = 0;
				return (TRUE);
			}

			/* emergency phase spell */
			if (borg_caution_phase(80, 5) &&
				 (borg_spell_fail(REALM_ARCANE, 0, 4, 15) ||
				  borg_spell_fail(REALM_SORCERY, 0, 1, 15) ||
				  borg_spell_fail(REALM_TRUMP, 0, 0, 15))) {
				/* verify use of spell */
				borg_keypress('y');

				/* Flee! */
				borg_note("# Danger Level 1.3  Critical Attempt");
				/* Reset timer if borg was in a anti-summon corridor */
				if (borg_t - borg_t_position < 50)
					borg_t_position = 0;
				return (TRUE);
			}
			borg_skill[BI_CURSP] = sv_mana;
		}
	}

	/* If fighting a unique and at the end of the game try to stay and
	 * finish the fight.  Only bail out in extreme danger as above.
	 */
	if ((b_q < avoidance * (25 + risky_boost) / 10 &&
		  borg_fighting_unique >= 1 && borg_fighting_unique <= 3 &&
		  borg_skill[BI_CDEPTH] >= 97) ||
		 (borg_skill[BI_CURHP] > 550 && !(borg_depth & DEPTH_BREEDER) &&
		  borg_count_summoners < 10 && !nasties))
		return (FALSE);

	/* 2 - a bit more scary */
	/* Attempt to teleport (usually) */
	/* do not escape from uniques so quick */
	if (borg_skill[BI_ISHEAVYSTUN] ||
		 ((b_q > avoidance * (15 + risky_boost) / 10) && borg_fighting_unique &&
		  borg_skill[BI_CDEPTH] != 99 && !special_position) ||
		 ((b_q > avoidance * (13 + risky_boost) / 10) && !borg_fighting_unique)) {

		/* Try teleportation */
		if (borg_escape_stair() ||
			 /* try Dimension Door */
			 borg_dim_door(TRUE, b_q) ||
			 (borg_skill[BI_CDEPTH] == 0 &&
			  borg_read_scroll(SV_SCROLL_PHASE_DOOR)) ||
			 borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail - 12) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail - 12) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail - 12) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail - 12) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail - 12) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail - 12, FALSE) ||
			 borg_swap_position(b_q) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_activate_artifact(ART_ANDROMALIUS, FALSE) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_activate_planar(allow_fail) ||
			 borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_racial(RACE_GNOME, 1) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 /* try Dimension Door */
			 borg_dim_door(TRUE, b_q)) {
			/* Flee! */
			borg_note("# Danger Level 2.1");

			/* Success */
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			return (TRUE);
		}
		/* Phase door, if useful */
		if (q_regional < b_q / 2 && borg_caution_phase(50, 2) &&
			 borg_t - borg_t_position > 50 &&
			 (borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
			  borg_spell(REALM_ARCANE, 0, 4) || borg_spell(REALM_SORCERY, 0, 1) ||
			  borg_spell(REALM_TRUMP, 0, 0) || borg_mindcr(MIND_MINOR_DISP, 3) ||
			  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			  borg_activate_artifact(ART_NYNAULD, FALSE) ||
			  borg_mutation(COR1_BLINK, FALSE, allow_fail + 20, FALSE))) {
			/* Flee! */
			borg_note("# Danger Level 2.2");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}
	}

	/* 3- not too bad */
	/* also run if stunned or it is scary here */
	if (borg_skill[BI_ISHEAVYSTUN] ||
		 ((b_q > avoidance * (13 + risky_boost) / 10) && borg_fighting_unique &&
		  !special_position) ||
		 ((b_q > avoidance * (10 + risky_boost) / 10) && !borg_fighting_unique &&
		  !borg_fighting_questor) ||
		 ((b_q > avoidance * (10 + risky_boost) / 10) &&
		  borg_skill[BI_ISAFRAID] &&
		  (borg_skill[BI_AMISSILES] <= 0 && borg_class == CLASS_WARRIOR))) {
		/* try Dimension Door */
		if (borg_escape_stair() || borg_dim_door(TRUE, b_q) ||
			 /* Phase door, if useful */
			 (q_regional < b_q / 2 && borg_caution_phase(25, 2) &&
			  borg_t - borg_t_position > 50 &&
			  (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
				borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_activate_artifact(ART_NYNAULD, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
			/* Flee! */
			borg_escapes--; /* a phase isn't really an escape */
			borg_note("# Danger Level 3.1");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}

		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_racial(RACE_GNOME, 1) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 borg_swap_position(b_q) || borg_activate_planar(allow_fail) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT)) {
			/* Flee! */
			borg_note("# Danger Level 3.2");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
		/* Phase door, if useful */
		if (q_regional < b_q / 2 && borg_caution_phase(65, 2) &&
			 borg_t - borg_t_position > 50 &&
			 (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			  borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			  borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			  borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			  borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
			  borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
			  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			  borg_activate_artifact(ART_NYNAULD, FALSE) ||
			  borg_read_scroll(SV_SCROLL_PHASE_DOOR))) {
			/* Flee! */
			borg_escapes--; /* a phase isn't really an escape */
			borg_note("# Danger Level 3.3");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}

		/* if we got this far we tried to escape but couldn't... */
		/* time to flee */
		if (!goal_fleeing &&
			 (!borg_fighting_unique || borg_skill[BI_CLEVEL] < 35) &&
			 !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT)) &&
			 !borg_skill[BI_PASSWALL]) {
			/* Note */
			borg_note("# Fleeing (failed to teleport)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}

		/* Flee now */
		if (!goal_leaving &&
			 (!borg_fighting_unique || borg_skill[BI_CLEVEL] < 35) &&
			 !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT)) &&
			 !borg_skill[BI_PASSWALL]) {
			/* Flee! */
			borg_note("# Leaving (failed to teleport)");

			/* Start leaving */
			goal_leaving = TRUE;
		}
	}
	/* 4- not too scary but I'm comprimized */
	if (!special_position &&
		 ((b_q > avoidance * (8 + risky_boost) / 10 &&
			(borg_skill[BI_CLEVEL] < 35 ||
			 borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)) ||
		  ((b_q > avoidance * (12 + risky_boost) / 10) && borg_fighting_unique &&
			(borg_skill[BI_CLEVEL] < 35 ||
			 borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)) ||
		  ((b_q > avoidance * (6 + risky_boost) / 10) &&
			borg_skill[BI_CLEVEL] <= 20 && !borg_fighting_unique) ||
		  ((b_q > avoidance * (6 + risky_boost) / 10) &&
			borg_skill[BI_NO_MELEE] && !borg_fighting_unique))) {
		/* Phase door, if useful */
		if (borg_escape_stair() || borg_dim_door(TRUE, b_q) ||
			 (q_regional < b_q / 2 && borg_caution_phase(20, 2) &&
			  borg_t - borg_t_position > 50 &&
			  (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
				borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_activate_artifact(ART_NYNAULD, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
			/* Flee! */
			borg_escapes--; /* a phase isn't really an escape */
			borg_note("# Danger Level 4.1");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}

		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) || borg_swap_position(b_q) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION) ||
			 borg_racial(RACE_GNOME, 1) || borg_activate_planar(allow_fail)) {
			/* Flee! */
			borg_note("# Danger Level 4.2");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}

		/* if we got this far we tried to escape but couldn't... */
		/* time to flee */
		if (!goal_fleeing && !borg_fighting_unique &&
			 borg_skill[BI_CLEVEL] < 25 &&
			 !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT))) {
			/* Note */
			borg_note("# Fleeing (failed to teleport)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}

		/* Flee now */
		if (!goal_leaving && !borg_fighting_unique &&
			 !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT)) &&
			 !borg_skill[BI_PASSWALL]) {
			/* Flee! */
			borg_note("# Leaving (failed to teleport)");

			/* Start leaving */
			goal_leaving = TRUE;
		}
		/* Emergency Phase door if a weak mage */
		if ((borg_skill[BI_NO_MELEE]) && q_regional < b_q / 2 &&
			 borg_caution_phase(65, 2) && borg_t - borg_t_position > 50 &&
			 (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
			  borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
			  borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
			  borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
			  borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
			  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			  borg_activate_artifact(ART_NYNAULD, FALSE) ||
			  borg_read_scroll(SV_SCROLL_PHASE_DOOR))) {
			/* Flee! */
			borg_escapes--; /* a phase isn't really an escape */
			borg_note("# Danger Level 4.3");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}
	}

	/* 5- not too scary but I'm very low level  */
	if (borg_skill[BI_CLEVEL] < 10 &&
		 (b_q > avoidance * (6 + risky_boost) / 10 ||
		  (b_q > avoidance * (8 + risky_boost) / 10 && borg_fighting_unique))) {
		/* Dimension Door, if useful */
		if (borg_escape_stair() || borg_dim_door(TRUE, b_q) ||
			 /* Phase Door */
			 (q_regional < b_q / 2 && borg_caution_phase(20, 2) &&
			  borg_t - borg_t_position > 50 &&
			  (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
				borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_activate_artifact(ART_NYNAULD, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
			/* Flee! */
			borg_note("# Danger Level 5.1");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}

		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_swap_position(b_q) || borg_racial(RACE_GNOME, 1) ||
			 borg_activate_planar(allow_fail) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION)) {
			/* Flee! */
			borg_note("# Danger Level 5.2");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}

		/* if we got this far we tried to escape but couldn't... */
		/* time to flee */
		if (!goal_fleeing && !borg_fighting_unique) {
			/* Note */
			borg_note("# Fleeing (failed to teleport)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}

		/* Flee now */
		if (!goal_leaving && !borg_fighting_unique) {
			/* Flee! */
			borg_note("# Leaving (failed to teleport)");

			/* Start leaving */
			goal_leaving = TRUE;
		}
		/* Emergency Phase door if a weak mage */
		if (borg_skill[BI_NO_MELEE] && q_regional < b_q / 2 &&
			 borg_caution_phase(65, 2) && borg_t - borg_t_position > 50 &&
			 (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
			  borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
			  borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
			  borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
			  borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
			  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			  borg_activate_artifact(ART_NYNAULD, FALSE) ||
			  borg_read_scroll(SV_SCROLL_PHASE_DOOR))) {
			/* Flee! */
			borg_escapes--; /* a phase isn't really an escape */
			borg_note("# Danger Level 5.3");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}
	}

	/* 6- not too scary but I'm out of mana  */
	if (!special_position &&
		 (borg_skill[BI_NO_MELEE] || borg_class == CLASS_PRIEST) &&
		 (b_q > avoidance * (5 + risky_boost) / 10 ||
		  (b_q > avoidance * (7 + risky_boost) / 10 && borg_fighting_unique)) &&
		 (borg_skill[BI_CURSP] <= (borg_skill[BI_MAXSP] * 1 / 10) &&
		  borg_skill[BI_MAXSP] >= 100)) {
		/* Dimension Door, if useful */
		if (borg_escape_stair() || borg_dim_door(TRUE, b_q) ||
			 /* Phase Door */
			 (q_regional < b_q / 2 && borg_caution_phase(20, 2) &&
			  borg_t - borg_t_position > 50 &&
			  (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_activate_artifact(ART_NYNAULD, FALSE) ||
				borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
			/* Flee! */
			borg_note("# Danger Level 6.1");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;
			/* Success */
			return (TRUE);
		}

		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_swap_position(b_q) || borg_racial(RACE_GNOME, 1) ||
			 borg_activate_planar(allow_fail) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION)) {
			/* Flee! */
			borg_note("# Danger Level 6.2");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
	}

	/* 7- not scary but I'm in a bad grid (lava or water) */
	if (borg_on_safe_grid(c_y, c_x) == FALSE && ag->feat == FEAT_WATER) {
		/* Dimension Door, if useful */
		if (borg_escape_stair() || borg_dim_door(TRUE, b_q) ||
			 /* Phase Door */
			 (borg_caution_phase(20, 2) && borg_t - borg_t_position > 50 &&
			  (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail + 25) ||
				borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail + 25) ||
				borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail + 25) ||
				borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail + 25) ||
				borg_mutation(COR1_BLINK, FALSE, allow_fail + 25, FALSE) ||
				borg_activate_artifact(ART_DRAEBOR, FALSE) ||
				borg_activate_artifact(ART_NYNAULD, FALSE) ||
				borg_read_scroll(SV_SCROLL_PHASE_DOOR)))) {
			/* Flee! */
			borg_note("# Danger Level 7.1");
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}

		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail + 25) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail + 25) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail + 25) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail + 25) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail + 25) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail + 25, FALSE) ||
			 borg_swap_position(b_q) || borg_racial(RACE_GNOME, 1) ||
			 borg_activate_planar(allow_fail) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION)) {
			/* Flee! */
			borg_note("# Danger Level 7.2");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
	}
	/* 8- Shoot N Scoot */
	if ((borg_spell_okay_fail(REALM_ARCANE, 0, 4, allow_fail) ||
		  borg_spell_okay_fail(REALM_SORCERY, 0, 1, allow_fail) ||
		  borg_spell_okay_fail(REALM_TRUMP, 0, 0, allow_fail) ||
		  borg_mindcr_okay_fail(MIND_MINOR_DISP, 3, allow_fail) ||
		  borg_mutation(COR1_BLINK, TRUE, allow_fail, FALSE)) &&
		 borg_shoot_scoot_safe(20, 2, b_q)) {
		/* Phase door */
		if (borg_spell_fail(REALM_ARCANE, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 1, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 0, allow_fail) ||
			 borg_mindcr_fail(MIND_MINOR_DISP, 3, allow_fail) ||
			 borg_mutation(COR1_BLINK, FALSE, allow_fail, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_read_scroll(SV_SCROLL_PHASE_DOOR)) {
			/* Flee! */
			borg_note("# Shoot N Scoot. (Danger Level 8.1)");
			borg_escapes--; /* a phase isn't really an escape */

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
	}

	/* 8.2- Shoot N Scoot with Dim Door */
	if (borg_skill[BI_ADIMDOOR] && borg_shoot_scoot_dim(p1)) {
		/* Reset timer if borg was in a anti-summon corridor */
		if (borg_t - borg_t_position < 50)
			borg_t_position = 0;

		/* Use Dim door to scoot */
		borg_note("# Shoot N Scoot. (Danger Level 8.2)");
		borg_escapes--; /* a phase isn't really an escape */
		return (TRUE);
	}

	/* 9.  Teleport out if surrounded by lots of breeders.  */
	if ((borg_depth & DEPTH_BREEDER) && borg_surrounded_breeder() &&
		 !special_position) {
		/* Teleport via spell */
		if (borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
			 borg_spell_fail(REALM_TRUMP, 0, 4, allow_fail) ||
			 borg_spell_fail(REALM_CHAOS, 0, 7, allow_fail) ||
			 borg_spell_fail(REALM_SORCERY, 0, 5, allow_fail) ||
			 borg_mindcr_fail(MIND_MAJOR_DISP, 7, allow_fail) ||
			 borg_mutation(COR1_VTELEPORT, FALSE, allow_fail, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_activation(ACT_TELEPORT, FALSE) ||
			 borg_swap_position(b_q) || borg_racial(RACE_GNOME, 1) ||
			 borg_activate_planar(allow_fail) ||
			 borg_read_scroll(SV_SCROLL_TELEPORT) ||
			 borg_use_staff_fail(SV_STAFF_TELEPORTATION)) {
			/* Flee! */
			borg_note("# Danger Level 9");

			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
	}

	/* 10.  Teleport Level or ditch the level if there are too many monsters  */
	if ((!borg_fighting_unique && !special_position) &&
		 !borg_skill[BI_AGENOCIDE] && (borg_count_summoners >= 10 || nasties)) {

		/* Flee! */
		if (b_i > -1)
			borg_note(format("# Danger Level 10 due to '%c' (qty:%d).",
								  borg_nasties[b_i], borg_nasties_count[b_i]));
		else
			borg_note("# Danger Level 10 due to summoner count.");

		/* In case the spells don't work, I can ditch this level */
		if (borg_skill[BI_CDEPTH] >= 15)
			borg_depth |= DEPTH_SCARY;

		/* Teleport via spell */
		if ((borg_count_summoners >= 10 || b_i > -1) &&
			 (borg_spell_fail(REALM_SORCERY, 2, 6, allow_fail + 9) ||
			  borg_spell_fail(REALM_TRUMP, 1, 5, allow_fail + 9) ||
			  borg_spell_fail(REALM_ARCANE, 3, 1, allow_fail + 9) ||
			  borg_racial(NEPHILIM, 1) ||
			  borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL) ||
			  borg_activate_planar(allow_fail + 9) ||
			  borg_activate_artifact(ART_ANDROMALIUS, FALSE))) {
			/* Reset timer if borg was in a anti-summon corridor */
			if (borg_t - borg_t_position < 50)
				borg_t_position = 0;

			/* Success */
			return (TRUE);
		}
	}

	return (FALSE);
}

/*
 * ** Try healing **
 * this function tries to heal the borg before trying to flee.
 * The ez_heal items (*Heal* and Life) are reserved for Lucifer and Lilith.
 * In severe emergencies the borg can drink an ez_heal item but that is
 * checked in borg_caution().  He should bail out of the fight before
 * using an ez_heal.
 */
bool borg_heal(int danger) {
	int hp_down;
	int allow_fail = 20;
	int chance;

	int stats_needing_fix = 0;

	hp_down = borg_skill[BI_MAXHP] - borg_skill[BI_CURHP];

	/* when fighting Lucifer and Lilith, we want the borg to use Life potion to
	 * fix his
	 * stats.  So we need to add up the ones that are dropped.
	 */
	if (borg_skill[BI_ISFIXSTR])
		stats_needing_fix++;
	if (borg_skill[BI_ISFIXINT])
		stats_needing_fix++;
	if (borg_skill[BI_ISFIXWIS])
		stats_needing_fix++;
	if (borg_skill[BI_ISFIXDEX])
		stats_needing_fix++;
	if (borg_skill[BI_ISFIXCON])
		stats_needing_fix++;

	/* Special cases get a second vote */
	if ((borg_class == CLASS_MAGE || borg_class == CLASS_HIGH_MAGE) &&
		 borg_skill[BI_ISFIXINT])
		stats_needing_fix++;
	if ((borg_class == CLASS_PRIEST || borg_class == CLASS_ORPHIC) &&
		 borg_skill[BI_ISFIXWIS])
		stats_needing_fix++;
	if (borg_class == CLASS_WARRIOR && borg_skill[BI_ISFIXCON])
		stats_needing_fix++;
	if (borg_skill[BI_MAXHP] <= 850 && borg_skill[BI_ISFIXCON])
		stats_needing_fix++;
	if (borg_skill[BI_MAXHP] <= 700 && borg_skill[BI_ISFIXCON])
		stats_needing_fix += 3;
	if (borg_class == CLASS_PRIEST && borg_skill[BI_MAXSP] < 100 &&
		 borg_skill[BI_ISFIXWIS])
		stats_needing_fix += 5;
	if (borg_skill[BI_NO_MELEE] && borg_skill[BI_MAXSP] < 100 &&
		 borg_skill[BI_ISFIXINT])
		stats_needing_fix += 5;

	/*  Hack -- heal when confused. This is deadly.*/
	/* This is checked twice, once, here, to see if he is in low danger
	 * and again at the end of borg_caution, when all other avenues have failed
	 */
	if ((borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISHEAVYSTUN]) &&
		 (rand_int(100) < 85)) {
		if ((hp_down >= 300) && danger - 300 < borg_skill[BI_CURHP] &&
			 borg_quaff_potion(SV_POTION_HEALING)) {
			borg_note("# Fixing Confusion. Level 1");
			return (TRUE);
		}
		if (danger - 20 < borg_skill[BI_CURHP] &&
			 ((borg_skill[BI_ISCONFUSED] &&
				borg_eat_food(SV_FOOD_CURE_CONFUSION)) ||
			  borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
			  borg_quaff_crit(FALSE) || borg_quaff_potion(SV_POTION_HEALING) ||
			  borg_use_staff_fail(SV_STAFF_HEALING) ||
			  borg_use_staff_fail(SV_STAFF_CURING))) {
			if (borg_skill[BI_ISCONFUSED])
				borg_note("# Fixing Confusion. Level 2");
			if (borg_skill[BI_ISHEAVYSTUN])
				borg_note("# Fixing Stun. Level 2");
			return (TRUE);
		}

		/* If my ability to use a teleport staff is really
		 * bad, then I should heal up then use the staff.
		 */
		/* Check for a charged teleport staff */
		if (borg_equips_staff_fail(SV_STAFF_TELEPORTATION)) {
			/* check my skill, drink a potion */
			if ((borg_skill[BI_DEV] -
						borg_items[borg_slot(TV_STAFF, SV_STAFF_TELEPORTATION)]
							 .level >
				  7) &&
				 (danger < (avoidance + 35) * 15 / 10) &&
				 (borg_quaff_crit(FALSE) || borg_quaff_potion(SV_POTION_HEALING))) {
				borg_note("# Fixing Confusion. Level 3");
				return (TRUE);
			}
			/* However, if I am in really big trouble and there is no way I am
			 * going to be able to
			 * survive another round, take my chances on the staff.
			 */
			else if (danger > avoidance * 15 / 10) {
				borg_note("# Too scary to fix Confusion. Level 4");
				return (FALSE);
			}
		}
	}
	/*  Hack -- heal when blind. This is deadly.*/
	if (borg_skill[BI_ISBLIND] && borg_class != CLASS_ORPHIC &&
		 (rand_int(100) < 85)) {
		/* if in extreme danger, use teleport then fix the
		 * blindness later.
		 */
		if (danger > avoidance * 25 / 10) {
			/* Check for a charged teleport staff */
			if (borg_equips_staff_fail(SV_STAFF_TELEPORTATION))
				return (0);
		}
		if ((hp_down >= 300) && borg_quaff_potion(SV_POTION_HEALING)) {
			return (TRUE);
		}
		/* Warriors with ESP won't need it so quickly */
		if (!(borg_class == CLASS_WARRIOR &&
				borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 4 &&
				borg_skill[BI_ESP])) {
			if (borg_eat_food(SV_FOOD_CURE_BLINDNESS) ||
				 borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
				 borg_quaff_crit(TRUE) || borg_quaff_potion(SV_POTION_HEALING) ||
				 borg_use_staff_fail(SV_STAFF_HEALING) ||
				 borg_use_staff_fail(SV_STAFF_CURING) || borg_racial(NEPHILIM, 2)) {
				borg_note("# Fixing Blindness.");
				return (TRUE);
			}
		}
	}

	/* We generally try to conserve ez-heal pots */
	if ((borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) &&
		 ((hp_down >= 400) ||
		  (danger > borg_skill[BI_CURHP] * 5 && hp_down > 100)) &&
		 borg_quaff_potion(SV_POTION_STAR_HEALING)) {
		borg_note("# Fixing Confusion/Blind.");
		return (TRUE);
	}

#if 0
	/* Get some Mana.  Consume a junky staff or Rod */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 3 / 10))
    {
        if (borg_eat_magic(TRUE, 25))
        {
            borg_note("# Use Eat Magic (from junk)");
            return (TRUE);
        }
    }

	/* Get some Mana Consume a staff or Rod */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 1 / 10))
    {
        if (borg_eat_magic(FALSE, 20))
        {
            borg_note("# Use Eat Magic");
            return (TRUE);
        }
    }

	/*  Hack -- rest until healed */
    if ( (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCUT] && !borg_goi && !borg_wraith &&
          !borg_see_inv && (!borg_shield && borg_skill[BI_CDEPTH] != 100) &&
          !borg_skill[BI_ISWEAK] && !borg_skill[BI_ISHUNGRY] && danger < avoidance/5) &&
         (borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE] || borg_skill[BI_ISAFRAID] || borg_skill[BI_ISSTUN] || borg_skill[BI_ISHEAVYSTUN] ||
          borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] || borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 6 / 10)  &&
         borg_check_rest(c_y, c_x) && !(borg_depth & DEPTH_SCARY) &&
         danger <= borg_fear_region[c_y/11][c_x/11] &&
         !goal_fleeing && goal != GOAL_RECOVER)
    {
        /* check for then call lite in dark room before resting */
        if (!borg_check_lite_only())
        {
            /* Take note */
            borg_note(format("# Resting to restore HP/SP...(heal)"));

            /* Rest until done */
            borg_keypress('R');
            borg_keypress('&');
            borg_keypress('\n');

            /* Reset our panel clock, we need to be here */
            time_this_panel =0;

            /* reset the inviso clock to avoid loops */
            need_see_inviso = borg_t - 50;

            /* Done */
            return (TRUE);
        }
        else
        {
            /* Must have been a dark room */
            borg_note(format("# Lighted the darkened room instead of resting."));
            return (TRUE);
        }
     }
#endif

	/* Healing and fighting Lilith and Lucifer. */
	if (borg_fighting_unique == RACE_LUCIFER ||
		 borg_fighting_unique == RACE_LILITH) {
		if (borg_skill[BI_CURHP] <= 625 &&
			 ((borg_skill[BI_CURHP] > 250 &&
				borg_spell_fail(REALM_LIFE, 2, 6, 14)) || /* Holy Word */
			  borg_use_staff_fail(SV_STAFF_HOLINESS) ||
			  /* Choose Life over *Healing* to fix stats*/
			  (stats_needing_fix >= 5 && borg_quaff_potion(SV_POTION_LIFE)) ||
			  /* Choose Life over Healing if way down on pts*/
			  (hp_down > 500 &&
				-1 == borg_slot(TV_POTION, SV_POTION_STAR_HEALING) &&
				borg_quaff_potion(SV_POTION_LIFE)) ||
			  borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_HEALING) ||
			  (borg_skill[BI_CURHP] < 250 &&
				borg_spell_fail(REALM_LIFE, 2, 6, 5)) || /* Holy Word */
			  (borg_skill[BI_CURHP] > 550 &&
				borg_spell_fail(REALM_LIFE, 2, 6, 15)) || /* Holy Word */
			  borg_spell_fail(REALM_LIFE, 3, 4, 15) ||	/* 2000 pts */
			  borg_spell_fail(REALM_NATURE, 1, 7, allow_fail + 9) ||
			  borg_spell_fail(REALM_LIFE, 1, 7, 15) || /* 300 pts */
			  borg_zap_rod(SV_ROD_HEALING) || borg_quaff_potion(SV_POTION_LIFE))) {
			borg_note("# Healing in Questor Combat.");
			return (TRUE);
		}
	}

	/* restore Mana */
	/* note, blow the staff charges easy because the staff will not last. */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] / 5) &&
		 (rand_int(100) < 50)) {
		if (borg_use_staff_fail(SV_STAFF_THE_MAGI)) {
			borg_note("# Use Magi Staff");
			return (TRUE);
		}
	}

	/* blowing potions is harder */
	/* NOTE: must have enough mana to keep up GOI or do a HEAL */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] / 10) ||
		 ((borg_skill[BI_CURSP] < 70 && borg_skill[BI_MAXSP] > 200) &&
		  (borg_goi + borg_wraith <= borg_game_ratio * 3))) {
		/*  use the potion if battling a unique and not too dangerous */
		if ((borg_fighting_unique && danger < avoidance * 2) ||
			 (borg_skill[BI_ATELEPORT] == 0 && danger > avoidance)) {
			if (borg_use_staff_fail(SV_STAFF_THE_MAGI) ||
				 borg_quaff_potion(SV_POTION_RESTORE_MANA)) {
				borg_note("# Restored My Mana");
				return (TRUE);
			}
		}
	}

	/* if unhurt no healing needed */
	if (hp_down == 0)
		return FALSE;

	/* Don't bother healing if not in danger */
	if (danger == 0 && borg_skill[BI_CURHP] >= 1 && !borg_skill[BI_ISPOISONED] &&
		 !borg_skill[BI_ISCUT])
		return (FALSE);

	/* Restoring while fighting the last 2 chars */
	if ((stats_needing_fix >= 5 && (borg_fighting_unique == RACE_LUCIFER ||
											  borg_fighting_unique == RACE_LILITH) &&
		  borg_skill[BI_CURHP] > 650) &&
		 (borg_eat_food(SV_FOOD_RESTORING) ||
		  borg_activate_activation(ACT_REST_ALL, FALSE))) {
		borg_note("# Trying to fix stats in combat.");
		return (TRUE);
	}

	/* No further Healing considerations if fighting Questors */
	if (borg_fighting_unique == RACE_LUCIFER ||
		 borg_fighting_unique == RACE_LILITH) {
		/* No further healing considerations right now */
		return (FALSE);
	}

	/* Hack -- heal when wounded a percent of the time */
	/* down 4/5 hp 0%                      */
	/* 3/4 hp 2%                           */
	/* 2/3 hp 20%                          */
	/* 1/2 hp 50%                          */
	/* 1/3 hp 75%                          */
	/* 1/4 hp 100%                         */

	chance = rand_int(100);

	/* if we are fighting a unique increase the odds of healing */
	if (borg_fighting_unique)
		chance += 10;

	/* if danger is close to the hp and healing will help, do it */
	if (danger >= borg_skill[BI_CURHP] && danger < borg_skill[BI_MAXHP])
		chance += 25;
	else {
		if (borg_class == CLASS_PRIEST || borg_class == CLASS_PALADIN ||
			 borg_skill[BI_REALM1] == REALM_LIFE)
			chance += 25;
	}

	/* Risky Borgs are less likely to heal in the fight */
	if (borg_plays_risky)
		chance -= 10;

	/* Chance to abate the heal attempt.  The lower the chance value, the greater
	 * the likelihood of non-heal */
	if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 4 / 5) {
		if (chance < 95)
			return (FALSE);
	} else if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 3 / 4) {
		if (chance < 75)
			return (FALSE);
	} else if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 2 / 3) {
		if (chance < 50)
			return (FALSE);
	} else if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] / 2) {
		if (chance < 25)
			return (FALSE);
	} else if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] / 3) {
		if (chance < 10)
			return (FALSE);
	}

	/* Heal step one (200hp) */
	if (hp_down > 150 && danger < 250 &&
		 borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2 &&
		 (borg_spell_fail(REALM_LIFE, 1, 6, allow_fail) ||
		  borg_use_staff_fail(SV_STAFF_HEALING) || borg_zap_rod(SV_ROD_HEALING) ||
		  borg_activate_activation(ACT_CURE_700, FALSE))) {
		borg_note("# Healing Level 4.");
		return (TRUE);
	}

	/* Cure Critical Wounds (6d10) */
	if (hp_down > 50 && hp_down < 75 && ((danger) < 60) &&
		 (borg_spell_fail(REALM_LIFE, 1, 2, allow_fail) ||
		  (!borg_speed && !borg_hero && !borg_berserk &&
			borg_mindcr_fail(MIND_ADRENALINE, 35, allow_fail)) ||
		  borg_quaff_crit(FALSE))) {
		borg_note("# Healing Level 3.");
		return (TRUE);
	}

	/* Cure Serious Wounds (4d10) */
	if (hp_down > 20 && hp_down < 40 && ((danger) < 40) &&
		 (borg_spell_fail(REALM_LIFE, 0, 6, allow_fail) ||
		  borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
		  (!borg_speed && !borg_hero && !borg_berserk &&
			borg_mindcr_fail(MIND_ADRENALINE, 23, allow_fail)) ||
		  borg_activate_activation(ACT_CURE_MW, FALSE) ||
		  borg_quaff_potion(SV_POTION_CURE_SERIOUS))) {
		borg_note("# Healing Level 2.");
		return (TRUE);
	}

	/* Cure light Wounds (2d10) */
	if (hp_down > 10 && hp_down < 20 && ((danger) < 20) &&
		 (borg_spell_fail(REALM_LIFE, 0, 1, allow_fail) ||
		  borg_spell_fail(REALM_ARCANE, 0, 7, allow_fail) ||
		  borg_spell_fail(REALM_NATURE, 0, 1, allow_fail) ||
		  borg_activate_activation(ACT_CURE_LW, FALSE) ||
		  borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
		  borg_activate_artifact(ART_RONOVE, FALSE))) {
		borg_note("# Healing Level 1.");
		return (TRUE);
	}

	/* If in danger try  one more Cure Critical if it will help */
	if (danger >= borg_skill[BI_CURHP] && danger < borg_skill[BI_MAXHP] &&
		 borg_skill[BI_CURHP] < 20 && danger < 50 && (borg_quaff_crit(TRUE))) {
		borg_note("# Healing Level 5.");
		return (TRUE);
	}

	/* Generally continue to heal.  But if we are preparing for the end
	 * game uniques, then bail out here in order to save our heal pots.
	 * (unless lucifer is dead)
	 * Priests wont need to bail, they have good heal spells.
	 */
	if (borg_skill[BI_MAXDEPTH] >= 98 && !borg_skill[BI_KING] &&
		 !borg_fighting_unique && borg_class != CLASS_PRIEST) {
		/* Bail out to save the heal pots for Lucifer & Lilith*/
		return (FALSE);
	}

	/* Heal step one (200hp) */
	if ((hp_down > 200 && danger < 200 &&
		  borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) &&
		 (borg_use_staff_fail(SV_STAFF_HEALING) || borg_zap_rod(SV_ROD_HEALING) ||
		  borg_spell_fail(REALM_LIFE, 1, 6, allow_fail))) {
		borg_note("# Healing Level 6.");
		return (TRUE);
	}

	/* Heal step two (300hp) */
	if (hp_down > 250 && danger < 300 &&
		 borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2 &&
		 (borg_use_staff_fail(SV_STAFF_HEALING) ||
		  (borg_fighting_evil_unique &&
			borg_spell_fail(REALM_LIFE, 2, 6, allow_fail)) || /* holy word */
																			  /* Vamp Drain ? */
		  borg_use_staff_fail(SV_STAFF_HOLINESS) ||
		  borg_spell_fail(REALM_LIFE, 1, 6, allow_fail) ||
		  borg_quaff_potion(SV_POTION_HEALING))) {
		borg_note("# Healing Level 7.");
		return (TRUE);
	}

	/* Heal step three (500hp) */
	if (hp_down > 450 && danger < 500 &&
		 (borg_zap_rod(SV_ROD_HEALING) ||
		  borg_activate_artifact(ART_RING_RAPHAEL, FALSE) ||
		  borg_activate_artifact(ART_EMMANUEL, FALSE) ||
		  borg_activate_artifact(ART_ROBE_MICHAEL, FALSE) ||
		  borg_activate_artifact(ART_CORSON, FALSE) ||
		  borg_activate_artifact(ART_SUN, FALSE) ||
		  borg_activate_activation(ACT_CURE_700, FALSE) ||
		  borg_activate_activation(ACT_CURE_1000, FALSE))) {
		borg_note("# Healing Level 7.1");
		return (TRUE);
	}

	/* Healing step three (650hp).  */
	if (hp_down > 400 && danger < 650 &&
		 ((borg_fighting_evil_unique &&
			borg_spell_fail(REALM_LIFE, 2, 6, allow_fail)) || /* holy word */
		  (borg_spell_fail(REALM_LIFE, 1, 6, allow_fail) ||
			borg_spell_fail(REALM_NATURE, 1, 7, allow_fail) ||
			borg_use_staff_fail(SV_STAFF_HOLINESS) ||
			borg_use_staff_fail(SV_STAFF_HEALING) ||
			borg_zap_rod(SV_ROD_HEALING) ||
			borg_activate_activation(ACT_CURE_700, FALSE) ||
			borg_activate_activation(ACT_CURE_1000, FALSE) ||
			borg_quaff_potion(SV_POTION_HEALING) ||
			borg_activate_artifact(ART_RING_RAPHAEL, FALSE) ||
			borg_activate_artifact(ART_EMMANUEL, FALSE) ||
			borg_activate_artifact(ART_ROBE_MICHAEL, FALSE) ||
			borg_activate_artifact(ART_CORSON, FALSE) ||
			borg_activate_artifact(ART_SUN, FALSE)))) {
		borg_note("# Healing Level 8.");
		return (TRUE);
	}

	/* Healing final check (regular check).  Note that *heal*
	 * and Life potions are not
	 * wasted.  They are saved for Lucifer, Lilith, and emergencies.  The
	 * Emergency check is at the end of borg_caution().
	 */
	if (hp_down >= 450 && (danger < 500) &&
		 ((borg_fighting_evil_unique &&
			borg_spell_fail(REALM_LIFE, 2, 6, allow_fail)) || /* holy word */
																			  /* Vamp Drain ? */
		  borg_spell_fail(REALM_LIFE, 3, 4, allow_fail) ||	/* 2000 */
		  borg_spell_fail(REALM_NATURE, 1, 7, allow_fail) || /* 1000 */
		  borg_use_staff_fail(SV_STAFF_HOLINESS) ||
		  borg_use_staff_fail(SV_STAFF_HEALING) ||
		  (borg_zap_rod(SV_ROD_HEALING) || borg_quaff_potion(SV_POTION_HEALING) ||
			borg_activate_artifact(ART_RING_RAPHAEL, FALSE) ||
			borg_activate_artifact(ART_EMMANUEL, FALSE) ||
			borg_activate_artifact(ART_ROBE_MICHAEL, FALSE) ||
			borg_activate_artifact(ART_CORSON, FALSE) ||
			borg_activate_activation(ACT_CURE_700, FALSE) ||
			borg_activate_activation(ACT_CURE_1000, FALSE) ||
			(borg_fighting_unique && (borg_quaff_potion(SV_POTION_STAR_HEALING) ||
											  borg_quaff_potion(SV_POTION_HEALING) ||
											  borg_quaff_potion(SV_POTION_LIFE)))))) {
		borg_note("# Healing Level 9.");
		return (TRUE);
	}

	/*** Cures ***/

	/* Dont do these in the middle of a fight, teleport out then try it */
	if (danger > avoidance * 2 / 10)
		return (FALSE);

	/* Hack -- cure poison when poisoned
	 * This was moved from borg_caution.
	 */
	if (borg_skill[BI_ISPOISONED] &&
		 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)) {
		if (borg_spell_fail(REALM_LIFE, 1, 1, 60) ||
			 borg_spell_fail(REALM_ARCANE, 1, 5, 60) ||
			 borg_spell_fail(REALM_NATURE, 0, 7, 60) ||
			 borg_quaff_potion(SV_POTION_CURE_POISON) ||
			 borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_use_staff(SV_STAFF_CURING) ||
			 borg_eat_food(SV_FOOD_CURE_POISON) ||
			 borg_quaff_potion(SV_POTION_CURING) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 /* buy time */
			 borg_quaff_crit(TRUE) || borg_spell_fail(REALM_LIFE, 0, 6, 40) ||
			 borg_spell_fail(REALM_LIFE, 0, 1, 40) ||
			 borg_spell_fail(REALM_ARCANE, 0, 7, 40) ||
			 borg_spell_fail(REALM_NATURE, 0, 1, 40) ||
			 borg_use_staff_fail(SV_STAFF_HEALING) ||
			 borg_activate_activation(ACT_CURE_MW, FALSE) ||
			 borg_activate_activation(ACT_CURE_LW, FALSE)) {
			borg_note("# Curing.");
			return (TRUE);
		}

		/* attempt to fix mana then poison on next round */
		if ((borg_spell_legal(REALM_LIFE, 1, 1) ||
			  borg_spell_legal(REALM_ARCANE, 1, 5) ||
			  borg_spell_legal(REALM_NATURE, 0, 7)) &&
			 (borg_quaff_potion(SV_POTION_RESTORE_MANA))) {
			borg_note("# Curing next round.");
			return (TRUE);
		}
	}

	/* Hack -- cure poison when poisoned CRITICAL CHECK
	 */
	if (borg_skill[BI_ISPOISONED] &&
		 (borg_skill[BI_CURHP] < 2 ||
		  borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20)) {
		int sv_mana = borg_skill[BI_CURSP];

		borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

		if (borg_spell(REALM_LIFE, 1, 1) || borg_spell(REALM_ARCANE, 1, 5) ||
			 borg_spell(REALM_NATURE, 0, 7)) {
			/* verify use of spell */
			/* borg_keypress('y'); */

			/* Flee! */
			borg_note("# Emergency Cure Poison! Gasp!!!....");

			return (TRUE);
		}
		borg_skill[BI_CURSP] = sv_mana;

		/* Quaff healing pots to buy some time- in this emergency.  */
		if (borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS))
			return (TRUE);

		/* Try to Restore Mana */
		if (borg_quaff_potion(SV_POTION_RESTORE_MANA))
			return (TRUE);

		/* Emergency check on healing.  Borg_heal has already been checked but
		 * but we did not use our ez_heal potions.  All other attempts to save
		 * ourself have failed.  Use the ez_heal if I have it.
		 */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20 &&
			 (borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_LIFE) ||
			  borg_quaff_potion(SV_POTION_HEALING))) {
			return (TRUE);
		}

		/* Quaff unknown potions in this emergency.  We might get luck */
		if (borg_quaff_unknown())
			return (TRUE);

		/* Eat unknown mushroom in this emergency.  We might get luck */
		if (borg_eat_unknown())
			return (TRUE);

		/* Use unknown Staff in this emergency.  We might get luck */
		if (borg_use_unknown())
			return (TRUE);
	}

	/* Hack -- cure wounds when bleeding, also critical check */
	if (borg_skill[BI_ISCUT] &&
		 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3 ||
		  rand_int(100) < 20)) {
		if (borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
			 borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			 borg_quaff_crit(borg_skill[BI_CURHP] < 10) ||
			 borg_activate_activation(ACT_CURE_MW, FALSE) ||
			 borg_activate_activation(ACT_CURE_LW, FALSE) ||
			 borg_spell_fail(REALM_LIFE, 1, 1, 100) ||
			 borg_spell_fail(REALM_LIFE, 0, 6, 100) ||
			 borg_spell_fail(REALM_LIFE, 0, 1, 100) ||
			 borg_spell_fail(REALM_ARCANE, 2, 2, 100) ||
			 borg_spell_fail(REALM_ARCANE, 0, 7, 100) ||
			 borg_spell_fail(REALM_NATURE, 0, 7, 100) ||
			 borg_spell_fail(REALM_NATURE, 0, 1, 100)) {
			return (TRUE);
		}
	}
	/* bleeding and about to die CRITICAL CHECK*/
	if (borg_skill[BI_ISCUT] &&
		 ((borg_skill[BI_CURHP] < 2) ||
		  borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20)) {
		int sv_mana = borg_skill[BI_CURSP];

		/* Quaff healing pots to buy some time- in this emergency.  */
		if (borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS))
			return (TRUE);

		/* Try to Restore Mana */
		if (borg_quaff_potion(SV_POTION_RESTORE_MANA))
			return (TRUE);

		/* Emergency check on healing.  Borg_heal has already been checked but
		 * but we did not use our ez_heal potions.  All other attempts to save
		 * ourself have failed.  Use the ez_heal if I have it.
		 */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20 &&
			 (borg_quaff_potion(SV_POTION_HEALING) ||
			  borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_LIFE))) {
			return (TRUE);
		}

		borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

		/* Emergency use of spell */
		if (borg_spell_fail(REALM_LIFE, 1, 1, 100) ||
			 borg_spell_fail(REALM_LIFE, 0, 6, 100) ||
			 borg_spell_fail(REALM_LIFE, 0, 1, 100) ||
			 borg_spell_fail(REALM_ARCANE, 2, 2, 100) ||
			 borg_spell_fail(REALM_ARCANE, 0, 7, 100) ||
			 borg_spell_fail(REALM_NATURE, 0, 7, 100) ||
			 borg_spell_fail(REALM_NATURE, 0, 1, 100))

		{
			/* verify use of spell */
			/* borg_keypress('y'); */

			/* Flee! */
			borg_note("# Emergency Wound Patch! Gasp!!!....");

			return (TRUE);
		}
		borg_skill[BI_CURSP] = sv_mana;

		/* Quaff unknown potions in this emergency.  We might get luck */
		if (borg_quaff_unknown())
			return (TRUE);

		/* Eat unknown mushroom in this emergency.  We might get luck */
		if (borg_eat_unknown())
			return (TRUE);

		/* Use unknown Staff in this emergency.  We might get luck */
		if (borg_use_unknown())
			return (TRUE);
	}

	/* nothing to do */
	return (FALSE);
}

/*
 * Be "cautious" and attempt to prevent death or dishonor.
 *
 * Strategy:
 *
 *   (1) Caution
 *   (1a) Analyze the situation
 *   (1a1) try to heal
 *   (1a2) try a defence
 *   (1b) Teleport from danger
 *   (1c) Handle critical stuff
 *   (1d) Retreat to happy grids
 *   (1e) Back away from danger
 *   (1f) Heal various conditions
 *
 *   (2) Attack
 *   (2a) Simulate possible attacks
 *   (2b) Perform optimal attack
 *
 *   (3) Recover
 *   (3a) Recover by spells/prayers
 *   (3b) Recover by items/etc
 *   (3c) Recover by resting
 *
 * XXX XXX XXX
 * In certain situations, the "proper" course of action is to simply
 * attack a nearby monster, since often most of the danger is due to
 * a single monster which can sometimes be killed in a single blow.
 *
 * Actually, both "borg_caution()" and "borg_recover()" need to
 * be more intelligent, and should probably take into account
 * such things as nearby monsters, and/or the relative advantage
 * of simply pummeling nearby monsters instead of recovering.
 *
 * Note that invisible/offscreen monsters contribute to the danger
 * of an extended "region" surrounding the observation, so we will
 * no longer rest near invisible monsters if they are dangerous.
 *
 * XXX XXX XXX
 * We should perhaps reduce the "fear" values of each region over
 * time, to take account of obsolete invisible monsters.
 *
 * Note that walking away from a fast monster is counter-productive,
 * since the monster will often just follow us, so we use a special
 * method which allows us to factor in the speed of the monster and
 * predict the state of the world after we move one step.  Of course,
 * walking away from a spell casting monster is even worse, since the
 * monster will just get to use the spell attack multiple times.  But,
 * if we are trying to get to known safety, then fleeing in such a way
 * might make sense.  Actually, this has been done too well, note that
 * it makes sense to flee some monsters, if they "stumble", or if we
 * are trying to get to stairs.  XXX XXX XXX
 *
 * Note that the "flow" routines attempt to avoid entering into
 * situations that are dangerous, but sometimes we do not see the
 * danger coming, and then we must attempt to survive by any means.
 *
 * We will attempt to "teleport" if the danger in the current situation,
 * as well as that resulting from attempting to "back away" from danger,
 * are sufficient to kill us in one or two blows.  This allows us to
 * avoid teleportation in situations where simply backing away is the
 * proper course of action, for example, when standing next to a nasty
 * stationary monster, but also to teleport when backing away will not
 * reduce the danger sufficiently.
 *
 * But note that in "nasty" situations (when we are running out of light,
 * or when we are starving, blind, confused, or hallucinating), we will
 * ignore the possibility of "backing away" from danger, when considering
 * the possibility of using "teleport" to escape.  But if the teleport
 * fails, we will still attempt to "retreat" or "back away" if possible.
 *
 * XXX XXX XXX Note that it should be possible to do some kind of nasty
 * "flow" algorithm which would use a priority queue, or some reasonably
 * efficient normal queue stuff, to determine the path which incurs the
 * smallest "cumulative danger", and minimizes the total path length.
 * It may even be sufficient to treat each step as having a cost equal
 * to the danger of the destination grid, plus one for the actual step.
 * This would allow the Borg to prefer a ten step path passing through
 * one grid with danger 10, to a five step path, where each step has
 * danger 9.  Currently, he often chooses paths of constant danger over
 * paths with small amounts of high danger.  However, the current method
 * is very fast, which is certainly a point in its favor...
 *
 * When in danger, attempt to "flee" by "teleport" or "recall", and if
 * this is not possible, attempt to "heal" damage, if needed, and else
 * attempt to "flee" by "running".
 *
 * XXX XXX XXX Both "borg_caution()" and "borg_recover()" should only
 * perform the "healing" tasks if they will cure more "damage"/"stuff"
 * than may be re-applied in the next turn, this should prevent using
 * wimpy healing spells next to dangerous monsters, and resting to regain
 * mana near a mana-drainer.
 *
 * Whenever we are in a situation in which, even when fully healed, we
 * could die in a single round, we set the "goal_fleeing" flag, and if
 * we could die in two rounds, we set the "goal_leaving" flag.
 *
 * In town, whenever we could die in two rounds if we were to stay still,
 * we set the "goal_leaving" flag.  In combination with the "retreat" and
 * the "back away" code, this should allow us to leave town before getting
 * into situations which might be fatal.
 *
 * Flag "goal_fleeing" means get off this level right now, using recall
 * if possible when we get a chance, and otherwise, take stairs, even if
 * it is very dangerous to do so.
 *
 * Flag "goal_leaving" means get off this level when possible, using
 * stairs if possible when we get a chance.
 *
 * We will also take stairs if we happen to be standing on them, and we
 * could die in two rounds.  This is often "safer" than teleportation,
 * and allows the "retreat" code to retreat towards stairs, knowing that
 * once there, we will leave the level.
 *
 * If we can, we should try to hit a monster with an offset  spell.
 * A Druj can not move but they are really dangerous.  So we should retreat
 * to a happy grid (meaning we have los and it does not), we should target
 * one space away from the bad guy then blast away with ball spells.
 */
bool borg_caution(void) {
	int j, p;
	bool borg_surround = FALSE;
	/*bool borg_surround_breeder = FALSE;*/
	bool nasty = FALSE;
	bool ok_to_retreat = FALSE;

	/*** Notice "nasty" situations ***/

	/* About to run out of light is extremely nasty */
	if (!borg_skill[BI_LITE] && borg_items[INVEN_LITE].pval < 250)
		nasty = TRUE;

	/* Starvation is nasty */
	if (borg_skill[BI_ISWEAK])
		nasty = TRUE;

	/* Blind-ness is nasty */
	if (borg_skill[BI_ISBLIND])
		nasty = TRUE;

	/* Confusion is nasty */
	if (borg_skill[BI_ISCONFUSED])
		nasty = TRUE;

	/* Hallucination is nasty */
	if (borg_skill[BI_ISIMAGE])
		nasty = TRUE;

	/* if on level 100 and not ready for Lucifer, run */
	if (borg_skill[BI_CDEPTH] == LUCIFER_DEPTH && borg_t - borg_began < 10) {
		if (borg_ready_lucifer == 0 && !borg_skill[BI_KING]) {
			/* teleport level up to 99 to finish uniques */
			if (borg_spell_fail(REALM_SORCERY, 2, 6, 100) ||
				 borg_spell_fail(REALM_TRUMP, 1, 5, 100) ||
				 borg_spell_fail(REALM_ARCANE, 3, 1, 100) ||
				 borg_read_scroll(SV_SCROLL_TELEPORT_LEVEL)) {
				borg_note("# Rising one dlevel (Not ready for Lucifer)");
				return (TRUE);
			}

			/* Start leaving */
			if (!goal_leaving) {
				/* Note */
				borg_note("# Leaving (Not ready for Lucifer now)");

				/* Start leaving */
				goal_leaving = TRUE;
			}
		}
	}

	/*** Evaluate local danger ***/

	/* Monsters all around me, blocking my movement? */
	borg_surround = borg_surrounded();
	/*if ((borg_depth & DEPTH_BREEDER))
		borg_surround_breeder = borg_surrounded_breeder();*/

	/* Only allow three 'escapes' per level unless heading for morogoth
		or fighting a unique, then allow 85. */
	if ((borg_escapes > 3 && !unique_on_level && !borg_ready_lucifer &&
		  !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT))) ||
		 borg_escapes > 10) {
		/* No leaving if going after questors */
		if (borg_skill[BI_CDEPTH] <= 98) {
			/* Start leaving */
			if (!goal_leaving) {
				/* Note */
				borg_note("# Leaving (Too many escapes)");

				/* Start leaving */
				goal_leaving = TRUE;
			}

			/* Start fleeing */
			if (!goal_fleeing && borg_escapes > 3) {
				/* Note */
				borg_note("# Fleeing (Too many escapes)");

				/* Start fleeing */
				goal_fleeing = TRUE;
			}
		}
	}

	/* No hanging around if nasty here. */
	if ((borg_depth & DEPTH_SCARY)) {
		/* Start leaving */
		if (!goal_leaving) {
			/* Note */
			borg_note("# Leaving (Scary guy on level)");

			/* Start leaving */
			goal_leaving = TRUE;
		}

		/* Start fleeing */
		if (!goal_fleeing) {
			/* Note */
			borg_note("# Fleeing (Scary guy on level)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}

		/* Return to town quickly after leaving town */
		if (borg_skill[BI_CDEPTH] == 0)
			borg_fleeing_town = TRUE;
	}

	/* Look around */
	p = borg_danger(c_y, c_x, 1, TRUE);

	/* Describe (briefly) the current situation */
	/* Danger (ignore stupid "fear" danger) */
	if (borg_goi || borg_wraith ||
		 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 4) ||
		 (p > avoidance / 10) || borg_fear_region[c_y / 11][c_x / 11] ||
		 (borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		  borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)) {
		/* Describe (briefly) the current situation */
		borg_note(format(
			 "# Loc:%d,%d Dep:%d Lev:%d HP:%d/%d SP:%d/%d Danger:p=%d", c_y, c_x,
			 borg_skill[BI_CDEPTH], borg_skill[BI_CLEVEL], borg_skill[BI_CURHP],
			 borg_skill[BI_MAXHP], borg_skill[BI_CURSP], borg_skill[BI_MAXSP], p));
		if (borg_goi) {
			borg_note(format("# Protected by GOI (borg turns:%d; game turns:%d)",
								  borg_goi / borg_game_ratio, p_ptr->invuln));
		}
		if (borg_wraith) {
			borg_note(
				 format("# Protected by Wraithform (borg turns:%d; game turns:%d)",
						  borg_wraith / borg_game_ratio, p_ptr->wraith_form));
		}
		if (borg_shield) {
			borg_note(format("# Protected by Mystic Shield"));
		}
		if (borg_prot_from_evil) {
			borg_note(format("# Protected by PFE"));
		}
		if ((borg_position & POSITION_SUMM)) {
			borg_note("# Protected by anti-summon corridor.");
		}
		if ((borg_position & (POSITION_SEA | POSITION_BORE))) {
			borg_note("# Protected by Sea of Runes.");
		}
	}

	/* Comment on glyph */
	if (track_glyph_num) {
		int i;
		for (i = 0; i < track_glyph_num; i++) {
			/* Enqueue the grid */
			if ((track_glyph_y[i] == c_y) && (track_glyph_x[i] == c_x)) {
				/* if standing on one */
				borg_note(format("# Standing on Glyph"));
			}
		}
	}
	/* Comment on stair */
	if (track_less_num) {
		int i;
		for (i = 0; i < track_less_num; i++) {
			/* Enqueue the grid */
			if ((track_less_y[i] == c_y) && (track_less_x[i] == c_x)) {
				/* if standing on one */
				borg_note(format("# Standing on up-stairs"));

				/* If I had been returning to one, I succeeded. */
				if (goal_less)
					goal_less = FALSE;
			}
		}
	}
	/* Comment on stair */
	if (track_more_num) {
		int i;
		for (i = 0; i < track_more_num; i++) {
			/* Enqueue the grid */
			if ((track_more_y[i] == c_y) && (track_more_x[i] == c_x)) {
				/* if standing on one */
				borg_note(format("# Standing on dn-stairs"));
			}
		}
	}

	if (borg_skill[BI_NO_MELEE] &&
		 !(borg_position & (POSITION_SEA | POSITION_SUMM | POSITION_BORE)) &&
		 !borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCUT] &&
		 !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCONFUSED]) {
		/* do some defence before running away */
		if (borg_defend(p))
			return TRUE;

		/* try healing before running away */
		if (borg_heal(p))
			return TRUE;
	} else {
		/* try healing before running away */
		if (borg_heal(p))
			return TRUE;

		/* do some defence before running away! */
		if (borg_defend(p))
			return TRUE;
	}

	if (borg_uses_swaps) {
		/* do some swapping before running away! */
		if (p > (avoidance / 3) || borg_goi || borg_wraith) {
			if (borg_backup_swap(p))
				return TRUE;
		}
	}

	/* If I am waiting for recall,  & safe, then stay put. */
	if (goal_recalling && borg_check_rest(c_y, c_x) && borg_skill[BI_CDEPTH] &&
		 borg_danger(c_y, c_x, 1, TRUE) <= borg_skill[BI_CURHP] / 20) {
		/* note the resting */
		borg_note("# Resting here, waiting for Recall.");

		/* rest here until lift off */
		borg_keypress('R');
		borg_keypress('5');
		borg_keypress('0');
		borg_keypress('0');
		borg_keypress('\n');

		return (TRUE);
	}

	/* If I am waiting for recall in town */
	if (goal_recalling && goal_recalling <= (borg_game_ratio * 2) &&
		 !borg_skill[BI_CDEPTH]) {
		/* Cast GOI just before returning to dungeon */
		if (!borg_goi && !borg_wraith &&
			 (borg_spell_fail(REALM_LIFE, 3, 7, 15) ||
			  borg_spell_fail(REALM_SORCERY, 3, 7, 15) ||
			  borg_activate_activation(ACT_INVULN, FALSE) ||
			  borg_spell_fail(REALM_DEATH, 3, 7, 15) ||
			  borg_activate_activation(ACT_WRAITH, FALSE))) {
			borg_note("# Casting GOI before Recall activates.");
			borg_no_rest_prep = 10000;
			return (TRUE);
		}

		/* Cast PFE just before returning to dungeon */
		if (!borg_prot_from_evil &&
			 (borg_spell_fail(REALM_LIFE, 1, 5, 15) ||
			  borg_activate_activation(ACT_PROT_EVIL, FALSE))) {
			borg_note("# Casting PFE before Recall activates.");
			borg_no_rest_prep = 10000;
			return (TRUE);
		}

		/* Cast other good prep things */
		if ((!borg_speed && borg_spell_fail(REALM_SORCERY, 1, 5, 15)) ||
			 (borg_skill[BI_TRFIRE] + borg_skill[BI_TRCOLD] +
						 borg_skill[BI_TRACID] + borg_skill[BI_TRELEC] +
						 borg_skill[BI_TRPOIS] <
					3 &&
			  (borg_spell_fail(REALM_NATURE, 2, 3, 15) ||
				borg_spell_fail(REALM_NATURE, 0, 6, 15))) ||
			 (borg_skill[BI_TRFIRE] + borg_skill[BI_TRCOLD] +
						 borg_skill[BI_TRELEC] <
					2 &&
			  borg_spell_fail(REALM_NATURE, 0, 6, 15)) ||
			 (!borg_shield && !borg_goi && !borg_wraith &&
			  (borg_spell_fail(REALM_NATURE, 2, 2, 15) ||
				borg_racial(RACE_GOLEM, 1))) ||
			 (!borg_hero && borg_spell_fail(REALM_SORCERY, 7, 0, 15)) ||
			 (!borg_berserk && borg_spell_fail(REALM_DEATH, 2, 0, 15)) ||
			 (!borg_berserk && borg_mutation(COR1_BERSERK, FALSE, 15, FALSE)) ||
			 (!borg_bless && borg_spell_fail(REALM_LIFE, 0, 2, 15)) ||
			 (!borg_speed && borg_mindcr_fail(MIND_ADRENALINE, 35, 15))) {
			borg_note("# Casting preparatory spell before Recall activates.");
			borg_no_rest_prep = 10000;
			return (TRUE);
		}
	}

	/*** Danger ***/

	/* Impending doom */
	/* Don't take off in the middle of a fight */
	/* just to restock and it is useless to restock */
	/* if you have just left town. */
	if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH]) &&
		 (!borg_fighting_unique &&
		  !strstr(borg_restock(borg_skill[BI_CDEPTH]), "teleport")) &&
		 (borg_time_town + (borg_t - borg_began)) > 200) {
		/* Start leaving */
		if (!goal_leaving) {
			/* Note */
			borg_note(format("# Leaving (restock) %s",
								  borg_restock(borg_skill[BI_CDEPTH])));

			/* Start leaving */
			goal_leaving = TRUE;
		}
		/* Start fleeing */
		if (!goal_fleeing && borg_skill[BI_ACCW] < 2) {
			/* Flee */
			borg_note(format("# Fleeing (restock) %s",
								  borg_restock(borg_skill[BI_CDEPTH])));

			/* Start fleeing */
			goal_fleeing = TRUE;
		}
	}
	/* Excessive danger */
	else if (p > (borg_skill[BI_CURHP] * 2)) {
		/* Start fleeing */
		/* do not flee level if going after Lucifer or fighting a unique or
		 * passwalling */
		if (!goal_fleeing && !borg_fighting_unique &&
			 (borg_skill[BI_CLEVEL] < 50) && !borg_skill[BI_PASSWALL] &&
			 !(borg_depth & (DEPTH_QUEST | DEPTH_VAULT)) &&
			 (borg_skill[BI_CDEPTH] < 100 && borg_ready_lucifer == 1)) {
			/* Note */
			borg_note("# Fleeing (excessive danger)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}
	}
	/* Potential danger (near death) in town */
	else if (!borg_skill[BI_CDEPTH] && (p > borg_skill[BI_CURHP]) &&
				(borg_skill[BI_CLEVEL] < 50)) {
		/* Flee now */
		if (!goal_leaving) {
			/* Flee! */
			borg_note("# Leaving (potential danger)");

			/* Start leaving */
			goal_leaving = TRUE;
		}
	}

	/*** Stairs ***/

	/* Leaving or Fleeing, take stairs */
	if (goal_leaving || goal_fleeing || (borg_depth & DEPTH_SCARY) ||
		 goal_fleeing_lunal || goal_fleeing_munchkin ||
		 ((p > avoidance || (borg_skill[BI_CLEVEL] < 15 && p > avoidance / 2)) &&
		  borg_skill[BI_CLEVEL] <= 49 &&
		  borg_grids[c_y][c_x].feat ==
				FEAT_LESS)) /* danger and standing on stair */
	{
		int i;

		/* Take next stairs */
		stair_less = TRUE;

		if (borg_ready_lucifer == 0 && !borg_skill[BI_KING]) {
			stair_less = TRUE;
			if ((borg_depth & DEPTH_SCARY))
				borg_note("# Fleeing and leaving the level. (scaryguy)");
			if (goal_fleeing_lunal)
				borg_note("# Fleeing and leaving the level. (fleeing_lunal)");
			if (goal_fleeing_munchkin)
				borg_note("# Fleeing and leaving the level. (fleeing munchkin)");
			if (p > avoidance && borg_skill[BI_CLEVEL] <= 49 &&
				 borg_grids[c_y][c_x].feat == FEAT_LESS)
				borg_note("# Leaving level,  Some danger but I'm on a stair.");
		}

		if ((borg_depth & DEPTH_SCARY))
			stair_less = TRUE;

		/* Only go down if fleeing or prepared, but not when starving.
		 * or lacking on food */
		if (goal_fleeing == TRUE || goal_fleeing_lunal == TRUE ||
			 goal_fleeing_munchkin == TRUE)
			stair_more = TRUE;
		if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1])
			stair_more = TRUE;
		if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH]))
			stair_more = FALSE;

		/* If I need to sell crap, then don't go down */
		if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 &&
			 borg_gold < 25000 && borg_count_sell() >= 12)
			stair_more = FALSE;

		/* Its ok to go one level deep if evading scary guy */
		if ((borg_depth & DEPTH_SCARY))
			stair_more = TRUE;

		/* Not if hungry */
		if (borg_skill[BI_CUR_LITE] == 0 ||
			 (borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
			 borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] < 2)
			stair_more = FALSE;

		/* Make sure no Quest Item is adjacent to me. */
		for (i = 0; i < borg_takes_nxt; i++) {
			if (borg_takes[i].quest == TRUE &&
				 distance(c_y, c_x, borg_takes[i].y, borg_takes[i].x) <= 2) {
				stair_more = FALSE;
				stair_less = FALSE;
			}
		}

		/* if fleeing town, then dive */
		if (!borg_skill[BI_CDEPTH])
			stair_more = TRUE;
	}

	/* Take stairs up */
	if (stair_less) {
		int i;

		/* Current grid */
		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Make sure no Quest Item is adjacent to me. */
		for (i = 0; i < borg_takes_nxt; i++) {
			if (borg_takes[i].quest == TRUE &&
				 distance(c_y, c_x, borg_takes[i].y, borg_takes[i].x) <= 2) {
				stair_less = FALSE;
				stair_more = FALSE;
			}
		}

		/* Usable stairs */
		if (ag->feat == FEAT_LESS && stair_less) {
			if ((borg_skill[BI_MAXDEPTH] - 4) > borg_skill[BI_CDEPTH] &&
				 borg_skill[BI_MAXCLEVEL] >= 35) {
				borg_note("scumming");
				auto_scum = TRUE;
			}

			/* Take the stairs */
			borg_on_dnstairs = TRUE;
			borg_note("# Taking Stairs, slightly dangerous on this level.");
			borg_keypress('<');

			/* Success */
			return (TRUE);
		}
	}

	/* Take stairs down */
	if (stair_more && !goal_recalling) {
		int i;

		/* Current grid */
		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Make sure no Quest Item is adjacent to me. */
		for (i = 0; i < borg_takes_nxt; i++) {
			if (borg_takes[i].quest == TRUE &&
				 distance(c_y, c_x, borg_takes[i].y, borg_takes[i].x) <= 2) {
				stair_more = FALSE;
				stair_less = FALSE;
			}
		}

		/* Usable stairs */
		if (ag->feat == FEAT_MORE && stair_more) {
			if ((borg_skill[BI_MAXDEPTH] - 5) > borg_skill[BI_CDEPTH] &&
				 borg_skill[BI_MAXCLEVEL] >= 35) {
				borg_note("scumming");
				auto_scum = TRUE;
			}

			if (!goal_fleeing_lunal && !goal_fleeing_munchkin) {

				/* Cast GOI just before returning to dungeon */
				if (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] * 6 / 10 &&
					 !borg_goi && !borg_wraith &&
					 (borg_spell_fail(REALM_LIFE, 3, 7, 15) ||
					  borg_spell_fail(REALM_SORCERY, 3, 7, 15) ||
					  borg_spell_fail(REALM_DEATH, 3, 7, 15) ||
					  borg_activate_activation(ACT_WRAITH, FALSE) ||
					  borg_activate_activation(ACT_INVULN, FALSE))) {
					borg_note("# Casting GOI before taking stairs.");
					borg_no_rest_prep = 3000;

					return (TRUE);
				}

				/* Cast PFE just before returning to dungeon */
				if (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] * 6 / 10 &&
					 !borg_prot_from_evil && borg_spell_fail(REALM_LIFE, 1, 5, 15)) {
					borg_note("# Casting PFE before taking stairs.");
					borg_no_rest_prep = 3000;
					return (TRUE);
				}

				/* Cast other good prep things */
				if ((borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] * 6 / 10) &&
					 ((!borg_speed && borg_spell_fail(REALM_SORCERY, 1, 5, 15)) ||
					  (borg_skill[BI_TRFIRE] + borg_skill[BI_TRCOLD] +
								  borg_skill[BI_TRACID] + borg_skill[BI_TRELEC] +
								  borg_skill[BI_TRPOIS] <
							 3 &&
						(borg_mutation(COR1_RESIST, FALSE, 15, FALSE) ||
						 borg_spell_fail(REALM_NATURE, 2, 3, 15) ||
						 borg_spell_fail(REALM_NATURE, 0, 6, 15))) ||
					  (borg_skill[BI_TRFIRE] + borg_skill[BI_TRCOLD] +
								  borg_skill[BI_TRELEC] <
							 2 &&
						(borg_mutation(COR1_RESIST, FALSE, 15, FALSE) ||
						 borg_spell_fail(REALM_NATURE, 0, 6, 15))) ||
					  (!borg_shield && !borg_goi && !borg_wraith &&
						(borg_spell_fail(REALM_NATURE, 2, 2, 15) ||
						 borg_racial(RACE_GOLEM, 1))) ||
					  (!borg_hero && borg_spell_fail(REALM_SORCERY, 7, 0, 15)) ||
					  (!borg_berserk && borg_spell_fail(REALM_DEATH, 2, 0, 15)) ||
					  (!borg_bless && borg_spell_fail(REALM_LIFE, 0, 2, 15)) ||
					  (!borg_speed && borg_mindcr_fail(MIND_ADRENALINE, 35, 15)) ||
					  (!borg_hero &&
						borg_mutation(COR1_BERSERK, FALSE, 25, FALSE)))) {
					borg_note("# Casting preparatory spell before taking stairs.");
					borg_no_rest_prep = 3000;
					return (TRUE);
				}
			}

			/* Special Handling for Vampires
			 * Characters that fear light are concerned about recalling out of the
			 * dungeon
			 * because they may end up in the middle of town during daylight.
			 * These characters
			 * will need to recall into the dungeon while standing on the stairs in
			 * order to
			 * land on the stairs when they recall back from the dungeon.
			 */
			if (borg_skill[BI_FEAR_LITE] && borg_skill[BI_CDEPTH] == 0 &&
				 borg_skill[BI_RECALL] >= 3 && borg_skill[BI_MAXDEPTH] >= 8 &&
				 (cptr)NULL == borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10] &&
				 ((borg_skill[BI_HRTIME] >= 18 && borg_skill[BI_MNTIME] <= 35) ||
				  (borg_skill[BI_HRTIME] <= 6 && borg_skill[BI_MNTIME] <= 25))) {
				borg_note("# Recalling while on a stair because I fear daylight.");
				if (borg_recall())
					return (TRUE);
			}

			/* Take the stairs */
			borg_on_upstairs = TRUE;
			borg_note("# Stair More, not recalling.");
			borg_keypress('>');

			/* Success */
			return (TRUE);
		}
	}

	/*** Deal with critical situations ***/

	/* Hack -- require light */
	if (!borg_skill[BI_LITE]) {
		borg_item *item = &borg_items[INVEN_LITE];

		/* Must have light -- Refuel current torch */
		if ((item->tval == TV_LITE) && (item->sval == SV_LITE_TORCH)) {
			/* Try to refuel the torch */
			if ((item->pval < 500) && borg_refuel_torch())
				return (TRUE);
		}

		/* Must have light -- Refuel current lantern */
		if ((item->tval == TV_LITE) && (item->sval == SV_LITE_LANTERN)) {
			/* Try to refill the lantern */
			if ((item->pval < 1000) && borg_refuel_lantern())
				return (TRUE);
		}

		/* Flee for fuel */
		if (borg_skill[BI_CDEPTH] && (item->pval < 250)) {
			/* Start leaving */
			if (!goal_leaving) {
				/* Flee */
				borg_note("# Leaving (need fuel)");

				/* Start leaving */
				goal_leaving = TRUE;
			}
		}
	}

	/* Hack -- prevent starvation */
	if (borg_skill[BI_ISWEAK]) {
		/* Attempt to satisfy hunger */
		if (borg_eat_food_any() || borg_spell_fail(REALM_LIFE, 0, 7, 45) ||
			 borg_spell_fail(REALM_ARCANE, 2, 6, 45) ||
			 borg_spell_fail(REALM_NATURE, 0, 3, 45) ||
			 borg_activate_activation(ACT_SATIATE, FALSE) || borg_eat_vamp() ||
			 borg_eat_unknown() || borg_quaff_unknown()) {
			/* Success */
			return (TRUE);
		}

		/* Try to restore mana then cast the spell next round */
		if (borg_quaff_potion(SV_POTION_RESTORE_MANA))
			return (TRUE);

		/* Flee for food */
		if (borg_skill[BI_CDEPTH]) {
			/* Start leaving */
			if (!goal_leaving) {
				/* Flee */
				borg_note("# Leaving (need food)");

				/* Start leaving */
				goal_leaving = TRUE;
			}

			/* Start fleeing */
			if (!goal_fleeing) {
				/* Flee */
				borg_note("# Fleeing (need food)");

				/* Start fleeing */
				goal_fleeing = TRUE;
			}
		}
	}

	/* Prevent breeder explosions when low level */
	if ((borg_depth & DEPTH_BREEDER) && borg_skill[BI_CLEVEL] < 15) {
		/* Start leaving */
		if (!goal_leaving) {
			/* Flee */
			borg_note("# Leaving (breeder level)");

			/* Start leaving */
			goal_leaving = TRUE;
		}
	}

	/*** Flee on foot ***/

	/* Desperation Head for stairs */
	/* If you are low level and near the stairs and you can */
	/* hop onto them in very few steps, try to head to them */
	/* out of desperation */
	if ((track_less_num || track_more_num) &&
		 (goal_fleeing || (p > avoidance && borg_skill[BI_CLEVEL] < 35))) {
		int y, x, i;
		int b_j = -1;
		int m;
		int b_m = -1;
		bool safe = TRUE;

		borg_grid *ag;

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			ag = &borg_grids[y][x];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* Skip stairs if a monster is on the stair */
			if (ag->kill && !borg_kills[ag->kill].ally)
				continue;

			/* skip the closer ones */
			if (b_j >= j)
				continue;

			/* track it */
			b_j = j;
		}

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			x = track_more_x[i];
			y = track_more_y[i];

			ag = &borg_grids[y][x];

			/* How far is the nearest up stairs */
			m = distance(c_y, c_x, y, x);

			/* Skip stairs if a monster is on the stair */
			if (ag->kill && !borg_kills[ag->kill].ally)
				continue;

			/* skip the closer ones */
			if (b_m >= m)
				continue;

			/* track it */
			b_m = m;
		}

		/* If you are within a few (3) steps of the stairs */
		/* and you can take some damage to get there */
		/* go for it */
		if (b_j < 3 && b_j != -1 && p < borg_skill[BI_CURHP]) {
			borg_desperate = TRUE;
			if (borg_flow_stair_less(GOAL_FLEE, FALSE)) {
				/* Note */
				borg_note("# Desperate for Stairs (one)");

				borg_desperate = FALSE;
				return (TRUE);
			}
			borg_desperate = FALSE;
		}

		/* If you are next to steps of the stairs go for it */
		if (b_j <= 2 && b_j != -1) {
			borg_desperate = TRUE;
			if (borg_flow_stair_less(GOAL_FLEE, FALSE)) {
				/* Note */
				borg_note("# Desperate for Stairs (two)");

				borg_desperate = FALSE;
				return (TRUE);
			}
			borg_desperate = FALSE;
		}

		/* Low level guys tend to waste money reading the recall scrolls */
		if (b_j < 20 && b_j != -1 && (borg_depth & DEPTH_SCARY) &&
			 borg_skill[BI_CLEVEL] < 20) {
			/* do not attempt it if an adjacent monster is faster than me */
			for (i = 0; i < 8; i++) {
				x = c_x + ddx_ddd[i];
				y = c_y + ddy_ddd[i];

				/* check for bounds */
				if (!in_bounds(y, x))
					continue;

				/* Monster there ? */
				if (!borg_grids[y][x].kill)
					continue;

				/* skip allies */
				if (borg_kills[borg_grids[y][x].kill].ally)
					continue;

				/* skip neutrals */
				if (borg_kills[borg_grids[y][x].kill].neutral)
					continue;

				/* Access the monster and check it's speed */
				if (borg_kills[borg_grids[y][x].kill].speed > borg_skill[BI_SPEED])
					safe = FALSE;
			}

			/* Dont run from Grip or Fang or if its not safe */
			/* TODO MUST Hellband has outrunnable uniques at that level*/
			if ((borg_skill[BI_CDEPTH] <= 5 && borg_skill[BI_CDEPTH] != 0 &&
				  borg_fighting_unique) ||
				 safe != TRUE) {
				/* try to take them on, you cant outrun them */
			} else {
				borg_desperate = TRUE;
				if (borg_flow_stair_less(GOAL_FLEE, FALSE)) {
					/* Note */
					borg_note("# Desperate for Stairs (three)");

					borg_desperate = FALSE;
					return (TRUE);
				}
				borg_desperate = FALSE;
			}
		}

		/* If you are next to steps of the down stairs go for it */
		if (b_m <= 2 && b_m != -1) {
			borg_desperate = TRUE;
			if (borg_flow_stair_more(GOAL_FLEE, FALSE, FALSE)) {
				/* Note */
				borg_note("# Desperate for Stairs (four)");

				borg_desperate = FALSE;
				return (TRUE);
			}
			borg_desperate = FALSE;
		}
	}

	/* Strategic retreat */
	/* Do not retreat if */
	/* 1) we are icky (poisoned, blind, confused etc */
	/* 2) we are boosting our avoidance because we are stuck */
	ok_to_retreat = FALSE;
	if (p > avoidance / 3)
		ok_to_retreat = TRUE;
	if (borg_class == CLASS_HIGH_MAGE &&
		 borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] / 10)
		ok_to_retreat = TRUE;
	if (borg_surround && p != 0)
		ok_to_retreat = TRUE;
	if (ok_to_retreat &&
		 (nasty || borg_no_retreat || borg_skill[BI_SPEED] <= 100))
		ok_to_retreat = FALSE;
	if (ok_to_retreat &&
		 (borg_position & (POSITION_SEA | POSITION_BORE | POSITION_SUMM)))
		ok_to_retreat = FALSE;
	if (ok_to_retreat) {
		int d, b_d = -1;
		int r, b_r = -1;
		int /*b_p, */ p1 = -1;
		int b_x = c_x;
		int b_y = c_y;
		int ii;

		/* Scan the useful viewable grids */
		for (j = 1; j < borg_view_n; j++) {
			int x1 = c_x;
			int y1 = c_y;

			int x2 = borg_view_x[j];
			int y2 = borg_view_y[j];

			/* Cant if confused: no way to predict motion */
			if (borg_skill[BI_ISCONFUSED])
				continue;

			/* Require "floor" grids */
			if (!borg_cave_floor_bold(y2, x2))
				continue;

			/* Try to avoid pillar dancing if at good health */
			if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
				 /*Make sure we actually do have that many pas steps..*/
				 track_step_num > 2 && ((track_step_y[track_step_num - 2] == y2 &&
												 track_step_x[track_step_num - 2] == x2 &&
												 track_step_y[track_step_num - 3] == c_y &&
												 track_step_x[track_step_num - 3] == c_x) ||
												time_this_panel >= 300))
				continue;

			/* XXX -- Borgs in an unexplored hall (& with only a torch)
			 * will always return FALSE for Happy Grids:
			 *
			 *  222222      Where 2 = unknown grid.  Borg has a torch.
			 *  2221.#      Borg will consider both the . and the 1
			 *     #@#      for a retreat from the C. But the . will be
			 *     #C#      false d/t adjacent wall to the east.  1 will
			 *     #'#      will be false d/t unknown grid to the west.
			 *              So he makes no attempt to retreat.
			 * However, the next function (backing away), allows him
			 * to back up to 1 safely.
			 *
			 * To play safer, the borg should not retreat to grids where
			 * he has not previously been.  This tends to run him into
			 * more monsters.  It is better for him to retreat to grids
			 * previously travelled, where the monsters are most likely
			 * dead, and the path is clear.  However, there is not (yet)
			 * tag for those grids.  Something like BORG_BEEN would work.
			 */

			/* Require "happy" grids (most of the time)*/
			if (!borg_happy_grid_bold(y2, x2))
				continue;

			/* Track "nearest" grid */
			if (b_r >= 0) {
				int ay = ((y2 > y1) ? (y2 - y1) : (y1 - y2));
				int ax = ((x2 > x1) ? (x2 - x1) : (x1 - x2));

				/* Ignore "distant" locations */
				if ((ax > b_r) || (ay > b_r))
					continue;
			}

			/* Reset */
			r = 0;

			/* Simulate movement */
			while (1) {
				borg_grid *ag;

				/* Obtain direction */
				d = borg_goto_dir(y1, x1, y2, x2);

				/* Verify direction */
				if ((d == 0) || (d == 5))
					break;

				/* Track distance */
				r++;

				/* Simulate the step */
				y1 += ddy[d];
				x1 += ddx[d];

				/* Obtain the grid */
				ag = &borg_grids[y1][x1];

				/* Lets make one more check that we are not bouncing */
				if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
					 track_step_num > 2 &&
					 ((track_step_y[track_step_num - 2] == y1 &&
						track_step_x[track_step_num - 2] == x1 &&
						track_step_y[track_step_num - 3] == c_y &&
						track_step_x[track_step_num - 3] == c_x) ||
					  time_this_panel >= 300))
					break;

				/* Require floor */
				if (!borg_cave_floor_grid(ag))
					break;

				/* Require it to be somewhat close */
				if (r >= 10)
					break;

				/* Check danger of that spot */
				p1 = borg_danger(y1, x1, 1, TRUE);
				if (p1 >= p)
					break;

				/* make sure it is not dangerous to take the first step; unless
				 * surrounded. */
				if (r == 1) {
					/* Not surrounded */
					if (!borg_surround || (borg_surround && goal_ignoring)) {
						if (p1 >= borg_skill[BI_CURHP] * 5 / 10)
							break;

						/* Ought to be worth it */;
						if (p1 > p * 5 / 10)
							break;
					} else
					/* Surrounded, try to back-up */
					{
						if (borg_skill[BI_CLEVEL] >= 20) {
							if (p1 >= (b_r <= 5 ? borg_skill[BI_CURHP] * 15 / 10
													  : borg_skill[BI_CURHP]))
								break;
						} else {
							if (p1 >= borg_skill[BI_CURHP])
								break;
						}
					}

					/*
					 * Skip this grid if it is adjacent to a monster.  He will just
					 * hit me
					 * when I land on that grid (unless I am surrounded).
					 */
					for (ii = 1; ii < borg_kills_nxt; ii++) {
						borg_kill *kill;

						/* Monster */
						kill = &borg_kills[ii];

						/* Skip dead monsters */
						if (!kill->r_idx)
							continue;

						/* Skip friendly */
						if (kill->ally)
							continue;

						/* Require current knowledge */
						if (kill->when < borg_t - 2)
							continue;

						/* Check distance -- 1 grid away */
						if (distance(kill->y, kill->x, y1, x1) <= 1 &&
							 kill->speed > borg_skill[BI_SPEED] && !borg_surround)
							break;
					}
				}

				/* Skip monsters */
				if (ag->kill && !borg_kills[ag->kill].ally)
					break;

				/* Skip traps */
				if ((ag->feat >= FEAT_TRAP_TRAPDOOR) &&
					 (ag->feat <= FEAT_TRAP_SLEEP))
					break;

				/* Safe arrival */
				if ((x1 == x2) && (y1 == y2)) {
					/* Save distance */
					b_r = r;
					/*b_p = p1;*/

					/* Save location */
					b_x = x2;
					b_y = y2;

					/* Done */
					break;
				}
			}
		}

		/* Make sure the adjacent monsters are not faster than me */
		for (ii = 1; ii < borg_kills_nxt; ii++) {
			borg_kill *kill;

			/* Monster */
			kill = &borg_kills[ii];

			/* Skip non-threat monsters */
			if (!kill->r_idx)
				continue;
			if (!kill->awake)
				continue;
			if (!kill->killer)
				continue;

			/* Require current knowledge */
			if (kill->when < borg_t - 2)
				continue;

			/* Check distance -- 1 grid away */
			if (kill->speed > borg_skill[BI_SPEED] &&
				 kill->dist < ((kill->speed - (borg_skill[BI_SPEED]) + 10) / 10) &&
				 !borg_surround)
				b_r = -1;
		}

		/* Retreat */
		if (b_r >= 0 && goal != GOAL_TAKE) {
			/* Save direction */
			b_d = borg_goto_dir(c_y, c_x, b_y, b_x);

			/* Hack -- set goal */
			g_x = c_x + ddx[b_d];
			g_y = c_y + ddy[b_d];

			/* Note */
			borg_note(format(
				 "# Retreating to %d,%d (distance %d) via %d,%d (%d > %d)", b_y,
				 b_x, b_r, g_y, g_x, p, borg_danger(g_y, g_x, 1, TRUE)));

			/* Strategic retreat */
			borg_keypress(I2D(b_d));

			/* Reset my Movement and Flow Goals */
			goal = 0;

			/* Success */
			return (TRUE);
		}
	}

	/* Strategic retreat into a wall
	 * Only retreat if surrounded, dangerous and recalling
	*/
	if (borg_skill[BI_PASSWALL] && borg_surround && !borg_skill[BI_ISCONFUSED] &&
		 borg_skill[BI_RECALL] &&
		 (amt_phase + borg_skill[BI_ATELEPORT] <= 0 || goal_recalling) &&
		 (p > avoidance / 3 ||
		  (borg_class == CLASS_HIGH_MAGE &&
			borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] / 10))) {
		int /*d, */ b_d = -1;
		/*int r, b_r = -1;*/
		/*int b_p, p1 = -1;*/
		int b_x = c_x;
		int b_y = c_y;
		int /*ii, */ i;
		int walls = 0;
		int locked = 0;

		/* Scan the useful viewable grids */
		for (j = 0; j < 24; j++) {
			/*int x1 = c_x;
			int y1 = c_y;*/

			int x2 = c_x + borg_ddx_ddd[j];
			int y2 = c_y + borg_ddy_ddd[j];

			/* Require non "floor" grids */
			if (borg_cave_floor_bold(y2, x2))
				continue;

			/* count the walls, i might be in a good spot */
			if (j < 9)
				walls++;
			if (walls >= 8) {
				/* Sit tight and wait for recall */
				if (goal_recalling && borg_danger(c_y, c_x, 1, TRUE) == 0) {
					/* note the resting */
					borg_note("# Resting here, waiting for Recall (1).");

					/* rest here until lift off */
					borg_keypress('R');
					borg_keypress('5');
					borg_keypress('0');
					borg_keypress('0');
					borg_keypress('\n');

					return (TRUE);
				}

				/* Recall if possible */
				if (borg_recall())
					return (TRUE);

				/* Continue if you cant do anything */
				continue;
			}

			/* Dont check the adjacent grids; we are interested in the others */
			if (j <= 9)
				continue;

			/* Make sure this particular grid is locked in */
			locked = 0;
			for (i = 0; i < 8; i++) {
				/* Non-floor grid? */
				if (borg_cave_floor_bold(y2 + borg_ddy_ddd[i],
												 x2 + borg_ddx_ddd[i]))
					continue;

				/* Not adjacent to a monster */
				if (borg_grids[y2 + borg_ddy_ddd[i]][x2 + borg_ddx_ddd[i]].kill)
					continue;

				/* Check danger of that spot */
				if (borg_danger(y2 + borg_ddy_ddd[i], x2 + borg_ddx_ddd[i], 1,
									 TRUE) > p)
					continue;

				/* looking good so far */
				locked++;
			}

			/* Save location */
			if (locked >= 8) {
				b_x = x2;
				b_y = y2;
			}
		}

		/* Retreat */
		if (b_x != c_x && c_y != b_y) {
			/* Save direction */
			b_d = borg_passwall_dir(c_y, c_x, b_y, b_x);

			/* Hack -- set goal */
			g_x = c_x + ddx[b_d];
			g_y = c_y + ddy[b_d];

			/* Note */
			borg_note(format("# Retreating into wall at %d,%d via %d,%d (%d > %d)",
								  b_y, b_x, g_y, g_x, p,
								  borg_danger(g_y, g_x, 1, TRUE)));

			/* Strategic retreat */
			borg_keypress(I2D(b_d));

			/* Reset my Movement and Flow Goals */
			goal = 0;

			/* Success */
			return (TRUE);
		}
	}

	/* Hiding in the walls, danger lurks.  Consider using Recall */
	if (borg_skill[BI_PASSWALL] && borg_skill[BI_RECALL] &&
		 (amt_phase + borg_skill[BI_ATELEPORT] <= 0 || goal_recalling)) {
		int j;
		int walls = 0;

		/* Scan the useful viewable grids */
		for (j = 0; j < 24; j++) {
			/*int x1 = c_x;
			int y1 = c_y;*/

			int x2 = c_x + borg_ddx_ddd[j];
			int y2 = c_y + borg_ddy_ddd[j];

			/* boundary check */
			if (!in_bounds(y2, x2))
				continue;

			/* Count the walls. I might be in a good spot */
			if (j < 9 && !borg_cave_floor_bold(y2, x2))
				walls++;

			/* Proteted by 8 walls */
			if (walls >= 8 && goal_recalling &&
				 borg_danger(c_y, c_x, 1, TRUE) == 0) {
				/* note the resting */
				borg_note("# Resting here, waiting for Recall (2).");

				/* rest here until lift off */
				borg_keypress('R');
				borg_keypress('5');
				borg_keypress('0');
				borg_keypress('0');
				borg_keypress('\n');
				return (TRUE);
			}

			/* Dont do further checks on the adjacent grids.  We are interested in
			 * the outiside ones */
			if (j <= 9 || walls < 8)
				continue;

			/* Dangerous outside grids */
			if (borg_danger(y2, x2, 1, TRUE) >= avoidance / 2) {
				/* Recall if possible */
				if (borg_recall())
					return (TRUE);

				/* Continue if you cant do anything */
				continue;
			}
		}
	}

	/*** Escape if possible ***/

	/* Attempt to escape via spells */
	if (borg_escape(p)) {
		/* increment the escapes this level counter */
		borg_escapes++;

		/* Success */
		return (TRUE);
	}

	/*** Back away ***/
	/* Do not back up if */
	/* 1) we are icky (poisoned, blind, confused etc */
	/* 2) we are boosting our avoidance because we are stuck */
	ok_to_retreat = FALSE;
	if (p > avoidance / 5)
		ok_to_retreat = TRUE;
	if (borg_class == CLASS_HIGH_MAGE &&
		 borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] / 10)
		ok_to_retreat = TRUE;
	if (borg_surround && p != 0)
		ok_to_retreat = TRUE;
	if (ok_to_retreat &&
		 (nasty || borg_no_retreat || borg_skill[BI_SPEED] <= 100 || borg_goi ||
		  borg_wraith))
		ok_to_retreat = FALSE;
	if (ok_to_retreat && avoidance > borg_skill[BI_MAXHP] * 7 / 10)
		ok_to_retreat = FALSE;
	if (ok_to_retreat &&
		 (borg_position & (POSITION_SEA | POSITION_BORE | POSITION_SUMM)))
		ok_to_retreat = FALSE;
	if (ok_to_retreat) {
		int i = -1, b_i = -1;
		int k = -1, b_k = -1;
		int f = -1, b_f = -1;
		int g_k = 0;
		int ii;
		bool adjacent_monster = FALSE;

		/* Current danger */
		b_k = p;

		/* Fake the danger down if surounded so that he can move. */
		if (borg_surround)
			b_k = (b_k * 6 / 10);

		/* Check the freedom */
		b_f = borg_freedom(c_y, c_x);

		/* Attempt to find a better grid */
		for (i = 0; i < 8; i++) {
			int x = c_x + ddx_ddd[i];
			int y = c_y + ddy_ddd[i];

			/* Access the grid */
			borg_grid *ag = &borg_grids[y][x];

			/* Cant if confused: no way to predict motion */
			if (borg_skill[BI_ISCONFUSED])
				continue;

			/* Skip walls/doors */
			if (!borg_cave_floor_grid(ag))
				continue;

			/* Skip monster grids */
			if (ag->kill && borg_kills[ag->kill].killer)
				continue;

			/* Mega-Hack -- skip stores XXX XXX XXX */
			if ((ag->feat >= FEAT_SHOP_HEAD) && (ag->feat <= FEAT_SHOP_TAIL))
				continue;
			if ((ag->feat >= FEAT_BLDG_HEAD) && (ag->feat <= FEAT_BLDG_TAIL))
				continue;

			/* Mega-Hack -- skip traps XXX XXX XXX */
			if ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP))
				continue;

			/* If i was here last round and 3 rounds ago, suggesting a "bounce" */
			if (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
				 track_step_num > 2 && ((track_step_y[track_step_num - 2] == y &&
												 track_step_x[track_step_num - 2] == x &&
												 track_step_y[track_step_num - 3] == c_y &&
												 track_step_x[track_step_num - 3] == c_x) ||
												time_this_panel >= 300))
				continue;

			/*
			 * Skip this grid if it is adjacent to a monster.  He will just hit me
			 * when I land on that grid.
			 */
			for (ii = 1; ii < borg_kills_nxt; ii++) {
				borg_kill *kill;

				/* Monster */
				kill = &borg_kills[ii];

				/* Skip non-threat monsters */
				if (!kill->r_idx)
					continue;
				if (!kill->awake)
					continue;
				if (!kill->killer)
					continue;

				/* Require current knowledge */
				if (kill->when < borg_t - 2)
					continue;

				/* Check distance -- 1 grid away */
				if (distance(kill->y, kill->x, y, x) <= 1 && !borg_surround)
					adjacent_monster = TRUE;

				/* Check distance -- 2 grids away and he is faster than me */
				if (kill->speed > borg_skill[BI_SPEED] &&
					 kill->dist <
						  ((kill->speed - (borg_skill[BI_SPEED]) + 10) / 10) &&
					 !borg_surround)
					adjacent_monster = TRUE;
			}

			/* Skip this grid consideration because it is next to a monster */
			if (adjacent_monster == TRUE)
				continue;

			/* Extract the danger there */
			k = borg_danger(y, x, 1, TRUE);

			/* if the grid has a pet in it, I can switch places with it so reduce
			 * danger a lot. */
			if (borg_grids[y][x].kill && borg_kills[borg_grids[y][x].kill].ally)
				k /= 4;

			/* Skip this grid if danger is higher than my HP.
			 * Take my chances with fighting.
			 */
			if (k > avoidance)
				continue;

			/* Skip this grid if it is not really worth backing up.  Look for a 40%
			 * reduction in the danger if higher level.  If the danger of the new
			 * grid
			 * is close to the danger of my current grid, I'll stay and fight.
			 */
			if (borg_skill[BI_MAXCLEVEL] >= 35 && k > b_k * 6 / 10)
				continue;

			/* Skip this grid if it is not really worth backing up.  If the danger
			 * of the new grid
			 * is close to the danger of my current grid, I'll stay and fight
			 * unless I am low
			 * level and there is an adjacent monster.
			 */
			if (borg_skill[BI_MAXCLEVEL] < 35 && adjacent_monster == FALSE &&
				 k > b_k * 8 / 10)
				continue;

			/* Skip higher danger */
			/*     note: if surrounded, then b_k has been adjusted to a higher
		 * number to make his current
		 *     grid seem more dangerous.  This will encourage him to Back-Up.
		 */
			if (k > b_k)
				continue;

			/* Record the danger of this prefered grid */
			g_k = k;

			/* Check the freedom there */
			f = borg_freedom(y, x);

			/* Danger is the same */
			if (b_k == k) {
				/* If I am low level, reward backing-up if safe */
				if (borg_skill[BI_CLEVEL] <= 10 && borg_skill[BI_CDEPTH] &&
					 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] ||
					  borg_skill[BI_CURSP] < borg_skill[BI_MAXSP])) {
					/* do consider the retreat */
				} else {
					/* Freedom of my grid is better than the next grid
					 * so stay put and fight.
					 */
					if (b_f > f || borg_skill[BI_CDEPTH] >= 85)
						continue;
				}
			}

			/* Save the info */
			b_i = i;
			b_k = k;
			b_f = f;
		}

		/* Back away */
		if (b_i >= 0) {
			/* Hack -- set goal */
			g_x = c_x + ddx_ddd[b_i];
			g_y = c_y + ddy_ddd[b_i];

			/* Note */
			borg_note(format("# Backing up to %d,%d (%d > %d)", g_x, g_y, p, g_k));

			/* Back away from danger */
			borg_keypress(I2D(ddd[b_i]));

			/* Reset my Movement and Flow Goals */
			goal = 0;

			/* Set a flag that the borg is  not allowed to retreat for 5 rounds */
			borg_no_retreat = 3;

			/* Success */
			return (TRUE);
		}
	}

	/*** Exit the walls ***/
	/* The borg is using passwall and has come to the edge of a wall.  He is
	 * fighting a monster, but still sitting
	 * in the wall.  He will continue to take damage while sitting in the wall.
	 */
	if (borg_skill[BI_PASSWALL] &&
		 ((borg_grids[c_y][c_x].feat >= FEAT_DOOR_HEAD &&
			borg_grids[c_y][c_x].feat <= FEAT_DOOR_TAIL) ||
		  (borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
			borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)) &&
		 p > 1 && !borg_no_retreat) {
		int i = -1, b_i = -1;
		int k = -1, b_k = -1;
		/*int f = -1, b_f = -1;*/
		int g_k = 0;
		/*int ii;
		bool adjacent_monster = FALSE;*/

		/* Current danger plus the damage I will suffer for sitting here. */
		b_k = p + (borg_skill[BI_CLEVEL] / 5);

		/* Fake the danger down if surounded so that he can move. */
		if (borg_surround)
			b_k = (b_k * 6 / 10);

		/* Check the freedom */
		/*b_f = borg_freedom(c_y, c_x);*/

		/* Attempt to find a better grid */
		for (i = 0; i < 8; i++) {
			int x = c_x + ddx_ddd[i];
			int y = c_y + ddy_ddd[i];

			/* Access the grid */
			borg_grid *ag = &borg_grids[y][x];

			/* Cant if confused: no way to predict motion */
			if (borg_skill[BI_ISCONFUSED])
				continue;

			/* Skip walls/doors */
			if (!borg_cave_floor_grid(ag))
				continue;

			/* Skip monster grids */
			if (ag->kill && borg_kills[ag->kill].killer)
				continue;

			/* Mega-Hack -- skip stores XXX XXX XXX */
			if ((ag->feat >= FEAT_SHOP_HEAD) && (ag->feat <= FEAT_SHOP_TAIL))
				continue;
			if ((ag->feat >= FEAT_BLDG_HEAD) && (ag->feat <= FEAT_BLDG_TAIL))
				continue;

			/* Mega-Hack -- skip traps XXX XXX XXX */
			if ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP))
				continue;

			/* If i was here recently, suggesting a "bounce" */
			if (track_step_num > 3) {
				if ((track_step_y[track_step_num - 2] == y &&
					  track_step_x[track_step_num - 2] == x) ||
					 (track_step_y[track_step_num - 3] == y &&
					  track_step_x[track_step_num - 3] == x) ||
					 (track_step_y[track_step_num - 4] == y &&
					  track_step_x[track_step_num - 4] == x)) {
					continue;
				}
			}

			/* Extract the danger there */
			k = borg_danger(y, x, 1, TRUE);

			/* Skip this grid if danger is higher than my HP.
			 * Take my chances with fighting.
			 */
			if (k > avoidance)
				continue;

			/* Skip higher danger */
			/*     note: if surrounded, then b_k has been adjusted to a higher
		 * number to make his current
		 *     grid seem more dangerous.  This will encourage him to Back-Up.
		 */
			if (k > b_k)
				continue;

			/* Record the danger of this prefered grid */
			g_k = k;

			/* Save the info */
			b_i = i;
			b_k = k;
		}

		/* Move out of wall */
		if (b_i >= 0) {
			/* Hack -- set goal */
			g_x = c_x + ddx_ddd[b_i];
			g_y = c_y + ddy_ddd[b_i];

			/* Note */
			borg_note(
				 format("# Exiting wall to %d,%d (%d > %d)", g_x, g_y, p, g_k));

			/* Exit wall */
			borg_keypress(I2D(ddd[b_i]));

			/* Reset my Movement and Flow Goals */
			goal = 0;

			/* Set a flag that the borg is  not allowed to retreat for 5 rounds */
			borg_no_retreat = 3;

			/* Success */
			return (TRUE);
		}
	}

	/*** Special jump maneuver.
	 * If the borg has the DimDoor spell and a great attack spell that can do
	 * massive damage
	 * to a large group of people, then he might want to consicder jumping into
	 * the middle.
	 * I envision him use DimDoor to jump into a pit of Trolls and using Mind
	 * Wave or Dispel
	 * Evil on the whole lot.  The use of this technique would save a lot of
	 * mana.
	 */
	if (!borg_fighting_unique && borg_jump_into_pit())
		return (TRUE);

	/* If LOS to a summoner, it might be a good idea to dim-door into a hallway
	 */
	if (borg_jump_into_hall())
		return (TRUE);

	/*** Cures ***/

	/* cure confusion, second check, first (slightly different) in borg_heal() */
	if (borg_skill[BI_ISCONFUSED]) {
		if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 300 &&
			 (borg_quaff_potion(SV_POTION_HEALING) ||
			  borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_LIFE))) {
			return (TRUE);
		}
		if (borg_eat_food(SV_FOOD_CURE_CONFUSION) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS) || borg_quaff_crit(FALSE) ||
			 borg_quaff_potion(SV_POTION_HEALING) ||
			 borg_use_staff_fail(SV_STAFF_HEALING)) {
			return (TRUE);
		}
	}

	/* Hack -- cure fear when afraid */
	if (borg_skill[BI_ISAFRAID] && !borg_skill[BI_NO_MELEE]) {
		if (borg_spell_fail(REALM_LIFE, 0, 3, 100) ||
			 borg_spell_fail(REALM_DEATH, 2, 0, 100) ||
			 borg_mindcr_fail(MIND_ADRENALINE, 23, 100) ||
			 borg_mutation(COR1_BERSERK, FALSE, 75, FALSE) ||
			 borg_quaff_potion(SV_POTION_BOLDNESS) ||
			 borg_quaff_potion(SV_POTION_HEROISM) ||
			 borg_quaff_potion(SV_POTION_BESERK_STRENGTH) ||
			 borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 /*borg_racial(RACE_HALF_ORC, 1) || */ borg_racial(RACE_HALF_TROLL,
																				1)) {
			return (TRUE);
		}
	}

	/* Heal Passwall damage prior to leaving the walls. */
	if (borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID &&
		 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 5 ||
		  borg_skill[BI_CURHP] < 10)) {
		int walls = 0;
		int i;
		int y, x;
		int hp_down = borg_skill[BI_MAXHP] - borg_skill[BI_CURHP];
		int allow_fail = 15;

		/*
		 * Because the borg takes damage in the walls, we want him to heal up
		 * a bit prior to exiting the walls and stepping onto a floor grid.
		 * The borg will first look to see if he is surrounded by walls.
		 * Then he will try to estimate where he will step in two turns.
		 * If that grid is a non-wall grid, then we will heal up.
		 *
		 *
		 */

		/* Scan all adjacent grids to make sure I am encased in the walls */
		for (i = 0; i < 8; i++) {
			x = c_x + ddx_ddd[i];
			y = c_y + ddy_ddd[i];

			/* check for bounds */
			if (!in_bounds(y, x))
				continue;

			/* Monster there ? */
			if (borg_grids[y][x].feat >= FEAT_MAGMA &&
				 borg_grids[y][x].feat <= FEAT_PERM_SOLID)
				walls++;
		}

		/* Borg is covered by walls on all sides */
		if (walls >= 8) {
			/* Try to guess where the borg will step */
			int o = 0;
			int false_y, false_x;
			borg_grid *ag;

			false_y = c_y;
			false_x = c_x;

			/* Continue */
			while (o <= 1) {
				int b_n = 0;

				int i, b_i = -1;

				int c, b_c;

				/* Flow cost of current grid */
				b_c = borg_data_flow->data[c_y][c_x] * 10;

				/* Prevent loops */
				b_c = b_c - 5;

				/* Look around */
				for (i = 0; i < 8; i++) {
					/* Grid in that direction */
					x = false_x + ddx_ddd[i];
					y = false_y + ddy_ddd[i];

					/* Access the grid */
					ag = &borg_grids[y][x];

					/* Flow cost at that grid */
					c = borg_data_flow->data[y][x] * 10;

					/* Penalty for choosing to flow through rocks */
					if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_WALL_SOLID)
						c += 2;

					/* Never backtrack */
					if (c > b_c)
						continue;

					/* Notice new best value */
					if (c < b_c)
						b_n = 0;

					/* Apply the randomizer to equivalent values */
					if ((++b_n >= 2) && (rand_int(b_n) != 0))
						continue;

					/* Track it */
					b_i = i;
					b_c = c;
				}

				/* Found a likely grid I would step on */
				if (b_i >= 0) {
					/* Access the location */
					x = false_x + ddx_ddd[b_i];
					y = false_y + ddy_ddd[b_i];

					/* Simulate motion */
					false_y = y;
					false_x = x;
				}

				/* Next step */
				o++;
			}

			/* Found a grid I would likely step on in 2 steps.  See if it is a
			 * floor. */
			x = false_x;
			y = false_y;

			/* floor(ish)? */
			if ((borg_grids[y][x].feat >= FEAT_FLOOR &&
				  borg_grids[y][x].feat <= FEAT_MORE) ||
				 /*(borg_grids[y][x].feat >= FEAT_DEEP_WATER &&
				  borg_grids[y][x].feat <= FEAT_SHAL_LAVA) || */
				 (borg_grids[y][x].feat == FEAT_NONE &&
				  borg_detect_wall[(w_y / PANEL_HGT) + 0][(w_x / PANEL_WID) + 0] ==
						TRUE)) {
				/* Attempt to heal some of the passwall damage */
				if (hp_down < 20 &&
					 (borg_spell_fail(REALM_LIFE, 0, 1, allow_fail) ||
					  borg_spell_fail(REALM_ARCANE, 0, 7, allow_fail) ||
					  borg_spell_fail(REALM_NATURE, 0, 1, allow_fail) ||
					  borg_activate_artifact(ART_RONOVE, FALSE) ||
					  borg_activate_activation(ACT_CURE_MW, FALSE) ||
					  borg_activate_activation(ACT_CURE_LW, FALSE))) {
					borg_note("# Healing Level 5.1");
					return (TRUE);
				}

				/* Cure Serious Wounds (4d10) */
				if (hp_down < 40 &&
					 (borg_spell_fail(REALM_LIFE, 0, 6, allow_fail) ||
					  borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail) ||
					  borg_spell_fail(REALM_LIFE, 0, 1, allow_fail) ||
					  (!borg_speed && !borg_hero && !borg_berserk &&
						borg_mindcr_fail(MIND_ADRENALINE, 23, allow_fail)))) {
					borg_note("# Healing Level 5.2");
					return (TRUE);
				}

				/* Cure Critical Wounds (6d10) */
				if (hp_down < 60 &&
					 (borg_spell_fail(REALM_LIFE, 1, 2, allow_fail) ||
					  borg_spell_fail(REALM_LIFE, 0, 6, allow_fail) ||
					  (!borg_speed && !borg_hero && !borg_berserk &&
						borg_mindcr_fail(MIND_ADRENALINE, 35, allow_fail)))) {
					borg_note("# Healing Level 5.3");
					return (TRUE);
				}

				/* Heal step one (200hp) */
				if (hp_down >= 60 &&
					 (borg_zap_rod(SV_ROD_HEALING) ||
					  borg_spell_fail(REALM_LIFE, 1, 6, allow_fail) ||
					  borg_spell_fail(REALM_LIFE, 1, 2, allow_fail) ||
					  borg_spell_fail(REALM_LIFE, 0, 6, allow_fail) ||
					  borg_spell_fail(REALM_ARCANE, 2, 3, allow_fail))) {
					borg_note("# Healing Level 5.4");
					return (TRUE);
				}
			} /* Floor(ish) */
		}	 /* Surrounded by walls on all sides */
	}		  /* Healing Passwall damage */

	/*** Note impending death XXX XXX XXX ***/

	/* Flee from low hit-points */
	if (!borg_skill[BI_PASSWALL] &&
		 ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3) ||
		  ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) &&
			borg_skill[BI_CURHP] < (borg_skill[BI_CLEVEL] * 3))) &&
		 (borg_skill[BI_ACCW] < 3) && (borg_skill[BI_AHEAL] < 1)) {
		/* Flee from low hit-points */
		if (borg_skill[BI_CDEPTH] && (rand_int(100) < 25)) {
			/* Start leaving */
			if (!goal_leaving) {
				/* Flee */
				borg_note("# Leaving (low hit-points)");

				/* Start leaving */
				goal_leaving = TRUE;
			}
			/* Start fleeing */
			if (!goal_fleeing) {
				/* Flee */
				borg_note("# Fleeing (low hit-points)");

				/* Start fleeing */
				goal_fleeing = TRUE;
			}
		}
	}

	/* Flee from bleeding wounds or poison and no heals */
	if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) &&
		 (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)) {
		/* Flee from bleeding wounds */
		if (borg_skill[BI_CDEPTH] && (rand_int(100) < 25)) {
			/* Start leaving */
			if (!goal_leaving) {
				/* Flee */
				borg_note("# Leaving (bleeding/posion)");

				/* Start leaving */
				goal_leaving = TRUE;
			}

			/* Start fleeing */
			if (!goal_fleeing) {
				/* Flee */
				borg_note("# Fleeing (bleeding/poison)");

				/* Start fleeing */
				goal_fleeing = TRUE;
			}
		}
	}

	/* Emergency check on healing.  Borg_heal has already been checked but
	 * but we did not use our ez_heal potions.  All other attempts to save
	 * ourself have failed.  Use the ez_heal if I have it.
	 */
	if ((borg_skill[BI_CURHP] <
				borg_skill[BI_MAXHP] / 10 || /* dangerously low HP */
		  (p > borg_skill[BI_CURHP] * 2 && /* extreme danger -AND-*/
			(borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] == 0 &&
			 borg_skill[BI_CURHP] <
				  borg_skill[BI_MAXHP] / 4)) || /* low on escapes */
		  (p > borg_skill[BI_CURHP] && borg_skill[BI_AEZHEAL] > 5 &&
			borg_skill[BI_CURHP] <
				 borg_skill[BI_MAXHP] / 4) || /* moderate danger, lots of heals */
		  (p > borg_skill[BI_CURHP] * 12 / 10 &&
			borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 400 &&
			borg_fighting_unique &&
			borg_skill[BI_CDEPTH] >= 85)) && /* moderate danger, unique, deep */
		 (borg_quaff_potion(SV_POTION_STAR_HEALING) ||
		  borg_quaff_potion(SV_POTION_HEALING) ||
		  borg_quaff_potion(SV_POTION_LIFE))) {
		borg_note("# Using reserve EZ_Heal.");
		return (TRUE);
	}

	/* Hack -- use "recall" to flee if possible */
	if (goal_fleeing && borg_skill[BI_CDEPTH] && !goal_fleeing_munchkin &&
		 !borg_fleeing_town && (borg_recall())) {
		/* Note */
		borg_note("# Fleeing the level (recall)");

		/* Success */
		return (TRUE);
	}

	/* If I am waiting for recall,and in danger, buy time with
	 * phase and cure_anythings.
	 */
	if (goal_recalling && (p > avoidance * 2)) {
		if (!borg_skill[BI_ISCONFUSED] && !borg_skill[BI_ISBLIND] &&
			 borg_skill[BI_MAXSP] > 60 &&
			 borg_skill[BI_CURSP] < (borg_skill[BI_CURSP] / 4) &&
			 borg_quaff_potion(SV_POTION_RESTORE_MANA)) {
			borg_note("# Buying time waiting for Recall.  Step 1.");
			return (TRUE);
		}

		if (borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
			 borg_spell_fail(REALM_ARCANE, 0, 4, 25) ||
			 borg_spell_fail(REALM_SORCERY, 0, 1, 25) ||
			 borg_spell_fail(REALM_TRUMP, 0, 0, 25) ||
			 borg_mindcr_fail(MIND_MINOR_DISP, 3, 35) ||
			 borg_activate_artifact(ART_DRAEBOR, FALSE) ||
			 borg_activate_artifact(ART_NYNAULD, FALSE) ||
			 borg_zap_rod(SV_ROD_HEALING)) {
			borg_note("# Buying time waiting for Recall.  Step 2.");
			return (TRUE);
		}

		if ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] < 100) &&
			 (borg_quaff_crit(TRUE) || borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
			  borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			  borg_activate_activation(ACT_CURE_MW, FALSE) ||
			  borg_activate_activation(ACT_CURE_LW, FALSE))) {
			borg_note("# Buying time waiting for Recall.  Step 3.");
			return (TRUE);
		}
		if ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] > 150) &&
			 (borg_quaff_potion(SV_POTION_HEALING) ||
			  borg_quaff_potion(SV_POTION_STAR_HEALING) ||
			  borg_quaff_potion(SV_POTION_LIFE) || borg_quaff_crit(TRUE) ||
			  borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
			  borg_quaff_potion(SV_POTION_CURE_LIGHT))) {
			borg_note("# Buying time waiting for Recall.  Step 4.");
			return (TRUE);
		}
	}

	/* if I am gonna die next round, and I have no way to escape
	 * use the unknown stuff (if I am low level).
	 */
	if (p > (borg_skill[BI_CURHP] * 4) && borg_skill[BI_CLEVEL] < 20 &&
		 !borg_skill[BI_MAXSP]) {
		if (borg_use_unknown() || borg_read_unknown() || borg_quaff_unknown() ||
			 borg_eat_unknown())
			return (TRUE);
	}

	/* Nothing */
	return (FALSE);
}

/*
 * Guess how much damage a physical attack will do to a monster
 */
extern int borg_thrust_damage_one(int i, bool inflate) {
	int dam;
	int mult;
	int sleeping_bonus = 0;
	int terrain_bonus = 0;
	bool special_target = FALSE;
	borg_kill *kill;

	monster_race *r_ptr;

	borg_item *item;

	int chance;

	/* Examine current weapon */
	item = &borg_items[INVEN_WIELD];

	/* Monster record */
	kill = &borg_kills[i];

	/* Monster race */
	r_ptr = &r_info[kill->r_idx];

	/* Is it a unique monster or a questor? */
	if (kill->unique || kill->questor || kill->summoner)
		special_target = TRUE;

	/* Damage */
	dam = (item->dd * (item->ds + 1) / 2);

	/* Minimal damage */
	if (dam <= 0)
		dam = 1;

	/* here is the place for slays and such */
	mult = 1;

	if (((borg_skill[BI_WS_ANIMAL]) && (r_ptr->flags3 & RF3_ANIMAL)) ||
		 ((borg_skill[BI_WS_EVIL]) && (r_ptr->flags3 & RF3_EVIL)))
		mult = 2;
	if (((borg_skill[BI_WS_UNDEAD]) && (r_ptr->flags3 & RF3_ANIMAL)) ||
		 ((borg_skill[BI_WS_DEMON]) && (r_ptr->flags3 & RF3_DEMON)) ||
		 ((borg_skill[BI_WS_GIANT]) && (r_ptr->flags3 & RF3_GIANT)) ||
		 ((borg_skill[BI_WS_DRAGON]) && (r_ptr->flags3 & RF3_DRAGON)) ||
		 ((borg_skill[BI_WB_ACID]) && !(r_ptr->flags3 & RF3_IM_ACID)) ||
		 ((borg_skill[BI_WB_FIRE]) && !(r_ptr->flags3 & RF3_IM_FIRE)) ||
		 ((borg_skill[BI_WB_COLD]) && !(r_ptr->flags3 & RF3_IM_COLD)) ||
		 ((borg_skill[BI_WB_ELEC]) && !(r_ptr->flags3 & RF3_IM_ELEC)))
		mult = 3;
	if ((borg_skill[BI_WK_DRAGON]) && (r_ptr->flags3 & RF3_DRAGON))
		mult = 5;

	/* add the multiplier */
	dam *= mult;

	/* add weapon bonuses --- Not exactly accurate for Z, but close enough */
	dam += item->to_d;

	/* add player bonuses */
	dam += borg_skill[BI_TODAM];

	/* multiply the damage for the whole round of attacks */
	dam *= borg_skill[BI_BLOWS];

	/* Rogues get a bonus for sneaking up on sleeping guys */
	if (borg_class == CLASS_ROGUE) {
		if (!kill->awake) {
			/* Can't backstab creatures that we can't see, right? */
			sleeping_bonus = 10 + 2 * borg_skill[BI_CLEVEL] / 5;
		} else if (kill->afraid) {
			sleeping_bonus = 5 + borg_skill[BI_CLEVEL] / 5;
		}
	}

	/* Monsters in rubble can take advantage of cover. -LM- */
	if (borg_grids[kill->y][kill->x].feat == FEAT_RUBBLE) {
		terrain_bonus = r_ptr->ac / 7 + 5;
	}

	/*
	 * Monsters in trees can take advantage of cover,
	 * except from rangers.
	else if ((borg_grids[kill->y][kill->x].feat == FEAT_TREES) &&
				(borg_class != CLASS_RANGER)) {
		terrain_bonus = r_ptr->ac / 7 + 5;
	}
	*/
	/* Monsters in water are vulnerable. -LM- */
	else if (borg_grids[kill->y][kill->x].feat == FEAT_WATER &&
				!water_ok(kill->r_idx)) {
		/*TODO make this true ;)*/
		terrain_bonus -= r_ptr->ac / 5;
	}

	/* reduce for % chance to hit (AC) */
	chance = (borg_skill[BI_THN] +
				 ((borg_skill[BI_TOHIT] + item->to_h) * BTH_PLUS_ADJ));
	chance = chance + sleeping_bonus;

	/* 5% automatic success/fail */
	if (chance < 5)
		chance = 5;

	/* Do I stand at least 0% chance of hitting hit or is it a guy we should
	 * focus on? */
	if (!special_target && chance < (r_ptr->ac + terrain_bonus))
		dam = 0;

	/* Sometimes we want the actual likely damage */
	if (!inflate)
		dam = (dam * chance) / 100;

	/* Sometimes we want just the real damage with no inflation */
	if (inflate) {

		/* add 20% to chance to give a bit more wieght to weapons */
		if (borg_skill[BI_CLEVEL] > 15 && !borg_skill[BI_NO_MELEE])
			chance += 20;

		/* Average out the damage */
		if (dam >= 3)
			dam = (dam * chance) / 100;

		/* Limit damage to twice maximal hitpoints */
		if (!(kill->unique) && dam > kill->power * 2 && kill->power > 30)
			dam = kill->power * 2;
		else if (!(kill->questor) && dam > kill->power * 2 && kill->power > 30)
			dam = kill->power * 2;

		/* Reduce the damage if a mage, they should not melee if they can avoid it
		 */
		if (borg_skill[BI_NO_MELEE] && dam >= 3)
			dam = dam * 8 / 10;

		/* Slight boost if a vampiric weapon since it can boost our health a bit
		 */
		if (borg_skill[BI_WB_VAMPIRIC] && !(r_ptr->flags1 & RF3_UNDEAD))
			dam = dam * 13 / 10;

		/*
		 * Enhance the preceived damage on Uniques.  This way we target them
		 * Keep in mind that he should hit the uniques but if he has a
		 * x5 great bane of dragons, he will tend attack the dragon since the
		 * precieved (and actual) damage is higher.  But don't select
		 * the town uniques (maggot does no damage)
		 *
		 */
		if (special_target && borg_skill[BI_CDEPTH] >= 1)
			dam = (dam * 5);
		if (special_target && borg_skill[BI_CDEPTH] >= 1 && kill->injury > 80)
			dam += (dam * 99);

		/* Hack -- ignore Maggot until later.  Player will chase Maggot
		 * down all accross the screen waking up all the monsters.  Then
		 * he is stuck in a comprimised situation.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] == 0) {
			dam = dam * 2 / 3;

			/* Dont hunt maggot until later */
			if (borg_skill[BI_CLEVEL] < 5)
				dam = 0;
		}

		/* give a small bonus for whacking a breeder */
		if (r_ptr->flags2 & RF2_MULTIPLY)
			dam = (dam * 3 / 2);

		/* To conserve mana, for keeping GOI up, increase the value of melee */
		if ((borg_goi || borg_wraith) && !borg_skill[BI_NO_MELEE]) {
			dam += (dam * 15 / 10);
		}
	}

	/* Do not hit our pets.  Reduce damage by the pet's Hit Points */
	if ((kill->ally) && dam > 0) {
		dam -= kill->power * 2;
	}

	/* Invuln monsters take no dam */
	if (kill->invulner)
		dam = 0;

	/* Damage */
	return (dam);
}

/*
 * Simulate/Apply the optimal result of making a physical attack
 */
extern int borg_attack_aux_thrust(bool inflate, int specific) {
	int p, dir;

	int i, b_i = -1;
	int d, b_d = -1;

	borg_grid *ag;

	borg_kill *kill;

	/* Too afraid to attack */
	if (borg_skill[BI_ISAFRAID])
		return (0);

	/* Examine possible destinations */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Is there is specific monster that we should be targeting? */
		if (specific >= 1) {
			x = borg_temp_x[specific];
			y = borg_temp_y[specific];
		}

		/* Require "adjacent" */
		if (distance(c_y, c_x, y, x) > 1)
			continue;

		/* Acquire grid */
		ag = &borg_grids[y][x];

		/* Calculate "average" damage */
		d = borg_thrust_damage_one(ag->kill, inflate);

		/* No damage */
		if (d <= 0)
			continue;

		/* Obtain the monster */
		kill = &borg_kills[ag->kill];

		/* Hack -- avoid waking most "hard" sleeping monsters */
		if (!kill->awake && borg_class != CLASS_ROGUE &&
			 borg_skill[BI_CLEVEL] < 50 && (d <= kill->power) &&
			 !borg_munchkin_mode) {
			/* Calculate danger */
			borg_full_damage = TRUE;
			p = borg_danger_aux(y, x, 1, ag->kill, TRUE, FALSE);
			borg_full_damage = FALSE;

			if (p > avoidance / 2)
				continue;
		}

		/* Calculate "danger" to player */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 2, ag->kill, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Reduce "bonus" of partial kills */
		if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15)
			p = p / 10;

		/* Add the danger to the damage */
		if (inflate)
			d += p;

		/* Ignore lower damage */
		if ((b_i >= 0) && (d < b_d))
			continue;

		/* Save the info */
		b_i = i;
		b_d = d;
		if (specific != -1)
			b_i = specific;
	}

	/* Nothing to attack */
	if (b_i < 0)
		return (0);

	/* Save the location */
	borg_target_x = borg_temp_x[b_i];
	borg_target_y = borg_temp_y[b_i];

	/* Simulation */
	if (borg_simulate)
		return (b_d);

	/* Save the location */
	g_x = borg_temp_x[b_i];
	g_y = borg_temp_y[b_i];

	ag = &borg_grids[g_y][g_x];
	kill = &borg_kills[ag->kill];

	/* Note */
	borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.",
						  (r_name + r_info[kill->r_idx].name), g_y, g_x,
						  kill->power));
	borg_note(
		 format("# Attacking with weapon '%s'", borg_items[INVEN_WIELD].desc));

	/* Get a direction for attacking */
	dir = borg_extract_dir(c_y, c_x, g_y, g_x);

	/* Attack the grid */
	if (!borg_skill[BI_ISCONFUSED])
		borg_keypress('+');
	borg_keypress(I2D(dir));

	/* Success */
	return (b_d);
}

/*
 * Simulate/Apply the optimal result of making a mutation physical attack (Cold
 * Touch)
 */
extern int borg_attack_aux_mutation_thrust(int mutation, bool inflate,
														 int specific, int cost) {
	int p, dir;
	int p1;

	int i, b_i = -1;
	int d, b_d = -1;
	bool surrounded;

	borg_grid *ag;
	monster_race *r_ptr;
	borg_kill *kill;

	surrounded = borg_surrounded();

	/* Too afraid to attack */
	if (borg_skill[BI_ISAFRAID])
		return (0);

	/** Special checks for each mutation **/
	p1 = borg_danger(c_y, c_x, 1, TRUE);

	/* Panic Hit */
	if (mutation == BF_COR1_PANIC_HIT &&
		 (surrounded || (p1 < avoidance * 4 / 10 && !surrounded)))
		return (0);
	if (mutation == BF_COR1_PANIC_HIT &&
		 !borg_mutation(COR1_PANIC_HIT, TRUE, 25, FALSE))
		return (0);

	/* Banish */
	if (mutation == BF_COR1_BANISH && p1 < avoidance * 5 / 10)
		return (0);
	if (mutation == BF_COR1_BANISH && surrounded)
		return (0);
	if (mutation == BF_COR1_BANISH &&
		 !borg_mutation(COR1_BANISH, TRUE, 25, FALSE))
		return (0);

	/* Cold Touch */
	if (mutation == BF_COR1_COLD_TOUCH &&
		 !borg_mutation(COR1_COLD_TOUCH, TRUE, 25, FALSE))
		return (0);

	/* Examine possible destinations */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Is there is specific monster that we should be targeting? */
		if (specific >= 1) {
			x = borg_temp_x[specific];
			y = borg_temp_y[specific];
		}

		/* Require "adjacent" */
		if (distance(c_y, c_x, y, x) > 1)
			continue;

		/* Acquire grid and monster */
		ag = &borg_grids[y][x];
		kill = &borg_kills[ag->kill];
		r_ptr = &r_info[kill->r_idx];

		/* Does not work on certain types of guys */
		if (mutation == BF_COR1_BANISH &&
			 (!(r_ptr->flags3 & RF3_EVIL) || (kill->questor || kill->unique)))
			continue;

		/* Calculate "average" damage */
		d = borg_thrust_damage_one(ag->kill, inflate);

		/* Cold Touch */
		if (mutation == BF_COR1_COLD_TOUCH) {
			d = borg_skill[BI_CLEVEL] * 2;
			/* Immunes take little damage */
			if (r_ptr->flags3 & RF3_IM_COLD)
				d = 0;
		}

		/* Banish-- consider extra damage since it gets tported away */
		if (mutation == BF_COR1_BANISH)
			d *= 9;

		/* Increase the value for the Panic Hit so he chooses it over regular hit
		 */
		if (mutation == BF_COR1_PANIC_HIT)
			d *= 3;
		if (mutation == BF_COR1_PANIC_HIT &&
			 (kill->afraid || (r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE)))
			d = 0;

		/* Some penalty for the mana cost */
		d -= cost;

		/* No damage */
		if (d <= 0)
			continue;

		/* Hack -- avoid waking most "hard" sleeping monsters */
		if (!kill->awake && (d <= kill->power) && mutation != BF_COR1_BANISH) {
			/* Calculate danger */
			borg_full_damage = TRUE;
			p = borg_danger_aux(y, x, 1, ag->kill, TRUE, FALSE);
			borg_full_damage = FALSE;

			if (p > avoidance / 2)
				continue;
		}

		/* Calculate "danger" to player */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 2, ag->kill, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Reduce "bonus" of partial kills */
		if (!inflate && d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15)
			p = p / 10;

		/* Add the danger to the damage */
		if (inflate)
			d += p;

		/* Ignore lower damage */
		if ((b_i >= 0) && (d < b_d))
			continue;

		/* Save the info */
		b_i = i;
		b_d = d;
		if (specific >= 1)
			b_i = specific;
	}

	/* Nothing to attack */
	if (b_i < 0)
		return (0);

	/* Save the location */
	borg_target_x = borg_temp_x[b_i];
	borg_target_y = borg_temp_y[b_i];

	/* Simulation */
	if (borg_simulate)
		return (b_d);

	/* Save the location */
	g_x = borg_temp_x[b_i];
	g_y = borg_temp_y[b_i];
	borg_target(g_y, g_x);

	ag = &borg_grids[g_y][g_x];
	kill = &borg_kills[ag->kill];

	/* Note */
	borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.",
						  (r_name + r_info[kill->r_idx].name), g_y, g_x,
						  kill->power));
	borg_note(format("# Attacking with Mutation and '%s'",
						  borg_items[INVEN_WIELD].desc));

	/* Get a direction for attacking */
	dir = borg_extract_dir(c_y, c_x, g_y, g_x);

	/* Using mutation */
	switch (mutation) {
	case BF_COR1_PANIC_HIT:
		(void)(borg_mutation(COR1_PANIC_HIT, FALSE, 30, FALSE));
		break;

	case BF_COR1_COLD_TOUCH:
		(void)(borg_mutation(COR1_COLD_TOUCH, FALSE, 30, FALSE));
		break;

	case BF_COR1_BANISH:
		(void)(borg_mutation(COR1_BANISH, FALSE, 30, FALSE));
		break;
	}
	/* Attack the grid */
	borg_keypress('+');
	borg_keypress(I2D(dir));

	/* Success */
	return (b_d);
}

/*
 * Target a location.  Can be used alone or at "Direction?" prompt.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target(int y, int x) {
	int x1, y1, x2, y2;

	borg_grid *ag;
	borg_kill *kill;

	ag = &borg_grids[y][x];
	kill = &borg_kills[ag->kill];

	/* Log */
	/* Report a little bit */
	if (ag->kill) {
		borg_note(format("# Targeting %s who has %d HP  (%d%% health) (%d,%d).",
							  (r_name + r_info[kill->r_idx].name), kill->power,
							  100 - kill->injury, y, x));
	} else {
		borg_note(format("# Targetting location (%d,%d)", y, x));
	}

	/* Target mode */
	borg_keypress('*');

	/* Target a location */
	borg_keypress('p');

	/* Determine "path" */
	x1 = c_x;
	y1 = c_y;
	x2 = x;
	y2 = y;

	/* Move to the location (diagonals) */
	for (; (y1 < y2) && (x1 < x2); y1++, x1++)
		borg_keypress('3');
	for (; (y1 < y2) && (x1 > x2); y1++, x1--)
		borg_keypress('1');
	for (; (y1 > y2) && (x1 < x2); y1--, x1++)
		borg_keypress('9');
	for (; (y1 > y2) && (x1 > x2); y1--, x1--)
		borg_keypress('7');

	/* Move to the location */
	for (; y1 < y2; y1++)
		borg_keypress('2');
	for (; y1 > y2; y1--)
		borg_keypress('8');
	for (; x1 < x2; x1++)
		borg_keypress('6');
	for (; x1 > x2; x1--)
		borg_keypress('4');

	/* Select the target */
	borg_keypress('5');

	/* Carry these variables to be used on reporting spell
	 * pathway
	 */
	borg_target_y = y;
	borg_target_x = x;

	/* Success */
	return (TRUE);
}

/*
 * Mark spot along the target path a wall.
 * This will mark the unknown squares as a wall.  This might not be
 * the wall we ran into but also might be.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target_unknown_wall(int y, int x) {
	int n_x, n_y;
	bool found = FALSE;
	bool y_hall = FALSE;
	bool x_hall = FALSE;
	monster_race *r_ptr;

	if (y == 0 && x == 0)
		return (TRUE);

	borg_note(format("# Perhaps wall near targetted location (%d,%d)", y, x));

	/* Determine "path" */
	n_x = c_x;
	n_y = c_y;

	/*Boundary Check */
	if (!in_bounds(c_y - 2, c_x - 2) || !in_bounds(c_y + 2, c_x + 2))
		return (FALSE);

	/* check for 'in a hall' x axis */
	/* This check is for this: */
	/*
	 *      x
	 *    ..@..
	 *      x
	 *
	 * 'x' being 'not a floor' and '.' being a floor.
	 */

	if ((borg_grids[c_y + 1][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y + 2][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x].feat == FEAT_FLOOR &&
		  borg_grids[c_y - 2][c_x].feat == FEAT_FLOOR) &&
		 (borg_grids[c_y][c_x + 1].feat != FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 1].feat != FEAT_FLOOR))
		x_hall = TRUE;

	/* check for 'in a hall' y axis */
	if ((borg_grids[c_y][c_x + 1].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x + 2].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 1].feat == FEAT_FLOOR &&
		  borg_grids[c_y][c_x - 2].feat == FEAT_FLOOR) &&
		 (borg_grids[c_y + 1][c_x].feat != FEAT_FLOOR &&
		  borg_grids[c_y - 1][c_x].feat != FEAT_FLOOR))
		y_hall = TRUE;

	while (1) {
		r_ptr = &r_info[borg_kills[borg_grids[n_y][n_x].kill].r_idx];

		if (r_ptr->flags2 & RF2_PASS_WALL) {
			borg_note(
				 format("# Guessing wall (%d,%d) under ghostly target (%d,%d)", n_y,
						  n_x, n_y, n_x));
			borg_grids[n_y][n_x].feat = FEAT_WALL_SOLID;
			found = TRUE;
			return (found); /* not sure... should we return here? */
		}

		if (borg_grids[n_y][n_x].feat == FEAT_NONE && ((n_y != c_y) || !y_hall) &&
			 ((n_x != c_x) || !x_hall)) {
			borg_note(format("# Guessing wall (%d,%d) near target (%d,%d)", n_y,
								  n_x, y, x));
			borg_grids[n_y][n_x].feat = FEAT_WALL_SOLID;
			found = TRUE;
			return found; /* not sure... should we return here?
								  maybe should mark ALL unknowns in path... */
		}
		/* Pathway found the target. */
		if (n_x == x && n_y == y) {
			/* end of the pathway */
			borgmove2(&n_y, &n_x, y, x, c_y, c_x);
			borg_note(format("# Guessing wall (%d,%d) near target (%d,%d)", n_y,
								  n_x, y, x));
			if (borg_grids[n_y][n_x].feat == FEAT_NONE ||
				 borg_grids[n_y][n_x].feat == FEAT_FLOOR)
				borg_grids[n_y][n_x].feat = FEAT_WALL_SOLID;
			return (found);
		}

		/* Calculate the new location */
		borgmove2(&n_y, &n_x, c_y, c_x, y, x);
	}

	return found;
}

/*
 * Guess how much damage a spell attack will do to a monster
 *
 * We only handle the "standard" damage types.
 *
 * We are paranoid about monster resistances
 *
 * He tends to waste all of his arrows on a monsters immediately adjacent
 * to him.  Then he has no arrows for the rest of the level.  We will
 * decrease the damage if the monster is adjacent and we are getting low
 * on missiles.
 *
 * We will also decrease the value of the missile attack on breeders or
 * high clevel borgs town scumming.
 */
int borg_launch_damage_one(int i, int dam, int typ, bool inflate,
									int ammo_location) {
	int p1, p2 = 0;
	bool borg_use_missile = FALSE;
	int ii;

	int j;
	int vault_grids = 0;
	int x, y;
	/*int k;*/
	/*bool gold_eater = FALSE;*/
	bool special_target = FALSE;

	int sleeping_bonus = 0;
	int terrain_bonus = 0;
	int chance = 0;
	int bonus = 0;
	int cur_dis = 0;
	int armor = 0;
	int skill = 0;

	borg_grid *ag;
	borg_kill *kill;
	monster_race *r_ptr;
	borg_item *item;

	/* Monster record */
	kill = &borg_kills[i];

	/* Ammo record */
	item = &borg_items[ammo_location];

	/* all danger checks are with maximal damage */
	borg_full_damage = TRUE;

	/* Monster race */
	r_ptr = &r_info[kill->r_idx];

	/* How far away is the target? */
	cur_dis = kill->dist;

	/* Is it a unique monster or a questor? */
	if (kill->unique || kill->questor || kill->summoner)
		special_target = TRUE;

	/* Calculation our chance of hitting.  Player bonuses, Bow bonuses, Ammo
	 * Bonuses */
	bonus = (borg_skill[BI_TOHIT] + borg_items[INVEN_BOW].to_h + item->to_h);
	if (!kill->awake && borg_class == CLASS_ROGUE)
		sleeping_bonus = 5 + borg_skill[BI_CLEVEL] / 5;
	if (kill->afraid && borg_class == CLASS_ROGUE)
		sleeping_bonus = 5 + borg_skill[BI_CLEVEL] / 5;
	if (borg_grids[kill->y][kill->x].feat == FEAT_RUBBLE)
		terrain_bonus = r_ptr->ac / 5 + 5;
	/*if (borg_grids[kill->y][kill->x].feat == FEAT_TREES && borg_class !=
		CLASS_RANGER)
		terrain_bonus = r_ptr->ac / 5 + 5;*/
	if (borg_grids[kill->y][kill->x].feat == FEAT_RUBBLE)
		terrain_bonus = r_ptr->ac / 5 + 5;
	/* Not entirely sure this is true in Hellband, TODO SHOULD verify */
	if (borg_grids[kill->y][kill->x].feat == FEAT_WATER)
		terrain_bonus -= r_ptr->ac / 4;
	chance =
		 (borg_skill[BI_THB] + (bonus * BTH_PLUS_ADJ) - cur_dis + sleeping_bonus);
	armor = r_ptr->ac + terrain_bonus;

	/* Determine the skill level for certain wand activations */
	if (borg_skill[BI_CLEVEL] < 13) {
		skill = borg_skill[BI_CLEVEL];
	} else {
		skill = (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10;
	}

	/* Analyze the damage type */
	switch (typ) {
	/* Magic Missile */
	case GF_MISSILE:
		break;

	/* Standard Arrow */
	case GF_ARROW:
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly?*/
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Seeker Arrow/Bolt (Heavy ammo) */
	case GF_ARROW_SEEKER:
		if (!special_target)
			dam /= 10;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly?*/
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of Hurt Animal*/
	case GF_ARROW_ANIMAL:
		if (r_ptr->flags3 & RF3_ANIMAL)
			dam *= 2;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of hurt evil */
	case GF_ARROW_EVIL:
		if (r_ptr->flags3 & RF3_EVIL)
			dam *= 2;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of slay dragon*/
	case GF_ARROW_DRAGON:
		if (r_ptr->flags3 & RF3_DRAGON)
			dam *= 3;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of Wounding-- has boosted +th +td*/
	case GF_ARROW_WOUNDING:
	case GF_ARROW_SLAYING:
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of Lightning*/
	case GF_ARROW_ELEC:
		if (!(r_ptr->flags3 & RF3_IM_ELEC))
			dam *= 3;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of Flame*/
	case GF_ARROW_FLAME:
		if (!(r_ptr->flags3 & RF3_IM_FIRE))
			dam *= 3;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Arrow of Frost*/
	case GF_ARROW_FROST:
		if (!(r_ptr->flags3 & RF3_IM_COLD))
			dam *= 3;
		if (cur_dis == 1 && !borg_skill[BI_ISAFRAID] && !special_target)
			dam /= 5;
		/* Do I hit regularly? */
		if (!special_target && chance / 2 < armor * 8 / 10)
			dam = 0;
		break;

	/* Pure damage */
	case GF_MANA:
		/* only use mana storm against uniques... this */
		/* should cut down on some mana use. */
		if (!borg_fighting_unique || borg_has[POTION_RES_MANA] < 3)
			dam /= 2;
		if (borg_fighting_unique && borg_has[POTION_RES_MANA] > 7)
			dam *= 2;
		break;

	/* Acid */
	case GF_ACID:
		if (r_ptr->flags3 & RF3_IM_ACID)
			dam /= 9;
		break;

	/* Electricity */
	case GF_ELEC:
		if (r_ptr->flags3 & RF3_IM_ELEC)
			dam /= 9;
		break;

	/* Fire damage */
	case GF_FIRE:
		if (r_ptr->flags3 & RF3_IM_FIRE)
			dam /= 9;
		break;

	/* Cold */
	case GF_COLD:
		if (r_ptr->flags3 & RF3_IM_COLD)
			dam /= 9;
		break;

	/* Hack -- Equal chance of all elements to be cast */
	case GF_ELEMENTS:
		if (r_ptr->flags3 & RF3_IM_COLD)
			dam /= 4;
		if (r_ptr->flags3 & RF3_IM_ELEC)
			dam /= 4;
		if (r_ptr->flags3 & RF3_IM_FIRE)
			dam /= 4;
		if (r_ptr->flags3 & RF3_IM_ACID)
			dam /= 4;
		break;

	/* Poison */
	case GF_POIS:
		if (r_ptr->flags3 & RF3_IM_POIS)
			dam /= 9;
		break;

	/* Nuke
	case GF_NUKE:
		if (r_ptr->flags3 & RF3_IM_POIS)
			dam = (dam * 3) / 9;
		break;
	*/

	/* Ice */
	case GF_ICE:
		if (r_ptr->flags3 & RF3_IM_COLD)
			dam /= 9;
		break;

	/* Holy Orb */
	case GF_HELL_FIRE:
		if (r_ptr->flags3 & RF3_EVIL)
			dam *= 2;
		break;

	/* Holy Orb */
	case GF_HOLY_FIRE:
		if (r_ptr->flags3 & RF3_GOOD)
			dam = 0;
		else if (r_ptr->flags3 & RF3_EVIL)
			dam *= 2;
		else
			dam = (dam * 3) / 9;
		break;

	/* dispel undead */
	case GF_DISP_UNDEAD:
		if (!(r_ptr->flags3 & RF3_UNDEAD))
			dam = 0;
		break;

	/* Dispel Demon */
	case GF_DISP_DEMON:
		if (!(r_ptr->flags3 & RF3_DEMON))
			dam = 0;
		break;

	/* Dispel Demons and Undead (Exorcism Spell) */
	case GF_DISP_UNDEAD_DEMON:
		if (!(r_ptr->flags3 & RF3_UNDEAD))
			dam = 0;
		if (!(r_ptr->flags3 & RF3_DEMON))
			dam = 0;
		break;

	/*  Dispel Evil */
	case GF_DISP_EVIL:
		if (!(r_ptr->flags3 & RF3_EVIL))
			dam = 0;
		break;

	/*  Holy Word */
	case GF_HOLY_WORD:
		if (!(r_ptr->flags3 & RF3_EVIL))
			dam = 0;
		break;

	/* Weak Lite */
	case GF_LITE_WEAK:
		if (!(r_ptr->flags3 & RF3_HURT_LITE))
			dam = 0;
		if (borg_skill[BI_FEAR_LITE])
			dam = 0;
		break;

	/* Regular Lite */
	case GF_LITE:
		if (borg_skill[BI_FEAR_LITE])
			dam = 0;
		break;

	/* Drain Life / Psi / Vamp. */
	case GF_OLD_DRAIN:
	case GF_DEATH_RAY:
		if (!monster_living(r_ptr)) {
			dam = 0;
		}
		/* unique is only hurt 1:888 */
		if (typ == GF_DEATH_RAY && kill->unique)
			dam = 0;
		break;

	case GF_PSI:
	case GF_PSI_DRAIN:
		if (!kill->los) {
			dam = 0;
		}
		if (r_ptr->flags2 & RF2_EMPTY_MIND) {
			dam = 0;
		}
		if ((r_ptr->flags2 & RF2_STUPID) || (r_ptr->flags2 & RF2_WEIRD_MIND) ||
			 (r_ptr->flags3 & RF3_ANIMAL) || (r_ptr->level > (3 * dam / 2)) ||
			 /* These can backlash the attack-- but I can attempt a save against
				 the effect */
			 (((r_ptr->flags3 & RF3_UNDEAD) || (r_ptr->flags3 & RF3_DEMON)) &&
			  !special_target && r_ptr->level > borg_skill[BI_CLEVEL] + 20)) {
			dam /= 5;
		} else if (special_target && typ == GF_PSI) {
			dam *= 2;
		}

		/* Boost damage perception if low on mana since PSI_DRAIN gives us mana
		 * back */
		if (typ == GF_PSI_DRAIN) {
			/* A little low on mana */
			if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 7 / 10)
				dam *= 2;

			/* A lot low on mana */
			if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 5 / 10)
				dam *= 5;
			if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 3 / 10)
				dam *= 20;
		}
		break;

	/* Stone to Mud */
	case GF_KILL_WALL:
		if (!(r_ptr->flags3 & RF3_HURT_ROCK))
			dam = 0;
		break;

	/* New mage spell */
	case GF_NETHER: {
		if (r_ptr->flags3 & RF3_UNDEAD) {
			dam = 0;
		} else if (r_ptr->flags4 & RF4_BR_NETH) {
			dam *= 3;
			dam /= 9;
		} else if (r_ptr->flags3 & RF3_EVIL) {
			dam /= 2;
		}
	} break;

	/* New mage spell */
	case GF_CHAOS:
		if ((r_ptr->flags4 & RF4_BR_CHAO) || (r_ptr->flags3 & RF3_DEMON)) {
			dam *= 3;
			dam /= 9;
		}
		break;

	/* New mage spell */
	case GF_GRAVITY:
		if (r_ptr->flags4 & RF4_BR_GRAV) {
			dam *= 2;
			dam /= 9;
		}
		break;

	/* New mage spell */
	case GF_SHARDS:
		if (r_ptr->flags4 & RF4_BR_SHAR) {
			dam *= 3;
			dam /= 9;
		}
		break;

	/* Rockets
	case GF_ROCKET:
		if (r_ptr->flags4 & RF4_BR_SHAR) {
			dam /= 2;
		}
		break;
	*/

	/* New mage spell */
	case GF_SOUND:
		if (r_ptr->flags4 & RF4_BR_SOUN) {
			dam *= 2;
			dam /= 9;
		}

		/* Bonus for Stunning effect (stun paralyzes the monster, very cool) */
		dam = dam * 12 / 10;
		if (kill->summoner || kill->questor || kill->unique)
			dam *= 10;

		break;

	/* Plasma */
	case GF_PLASMA:
		if ((r_ptr->flags4 & RF4_BR_PLAS) || (r_ptr->flags3 & RF3_RES_PLAS)) {
			dam *= 2;
			dam /= 9;
		}
		break;

	/* Force */
	case GF_FORCE:
		if (r_ptr->flags4 & RF4_BR_WALL) {
			dam *= 2;
			dam /= 9;
		}
		break;

	case GF_DARK:
		if (r_ptr->flags4 & RF4_BR_DARK) {
			dam *= 2;
			dam /= 9;
		}
		break;

	case GF_WATER:
		if (r_ptr->flags3 & RF3_RES_WATE) {
			dam *= 2;
			dam /= 9;
		}
		break;

	case GF_DISINTEGRATE:
		if (r_ptr->flags3 & RF3_RES_DISE) {
			dam *= 2;
			dam /= 9;
		}
		break;

	case GF_TELEKINESIS:
		/* Should do a little more damage if a monster is adjacent to me since it
		 * throws them away from me */
		/* Scan grids adjacent to monster */
		for (ii = 0; ii < 8; ii++) {
			x = c_x + ddx_ddd[ii];
			y = c_y + ddy_ddd[ii];

			/* Access the grid */
			ag = &borg_grids[y][x];

			/* Skip non-monster bearing grids (important) */
			if (!ag->kill)
				continue;

			/* Monster adjacent to me, up the damage by 20% per monster. */
			dam = dam * 11 / 10;
		}

		/* Big bonus if I am injured and inside a vault.  This will
		 * Kick the monster out of the vault.  Of course, we try
		 * to not do this to uniques
		 */
		if ((cave[c_y][c_x].info & CAVE_ICKY) && !kill->unique &&
			 !kill->questor &&
			 borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] * 6 / 10)
			dam *= 99;

		break;

	/* Tough Magic Missile */
	case GF_METEOR:
		break;

	/* Dispel Good */
	case GF_DISP_GOOD:
		if (!(r_ptr->flags3 & RF3_GOOD))
			dam = 0;
		break;

	/* Dispel Good */
	case GF_DISP_LIVING:
		if (!monster_living(r_ptr))
			dam = 0;
		break;

	/* Weird attacks */
	case GF_CONFUSION:
	case GF_DISENCHANT:
	case GF_NEXUS:
	case GF_INERTIA:
	case GF_TIME:
		dam /= 2;
		break;

	/* Really weird attacks */
	case GF_DOMINATION:
	case GF_STUN:
	case GF_CHARM:
	case GF_CONTROL_UNDEAD:
	case GF_CONTROL_ANIMAL:
		/* Some can resist */
		if (kill->unique || r_ptr->level > dam)
			dam = 0;

		/* Resist_Confusion resists, but undead have innate resist confusion */
		if (typ != GF_CONTROL_UNDEAD && (r_ptr->flags3 & RF3_NO_CONF))
			dam = 0;

		/* Only stun is effective on questors */
		if (typ != GF_STUN && kill->questor)
			dam = 0;

		/* Some can cause a backlash effect */
		if (typ == GF_DOMINATION && (r_ptr->flags3 & (RF3_UNDEAD | RF3_DEMON)) &&
			 r_ptr->level > borg_skill[BI_CLEVEL] / 2)
			dam = 0;

		/* Type specific checks */
		if (typ == GF_CONTROL_UNDEAD && !(r_ptr->flags3 & RF3_UNDEAD))
			dam = 0;
		if (typ == GF_CONTROL_ANIMAL && !(r_ptr->flags3 & RF3_ANIMAL))
			dam = 0;

		/* Already affected */
		if (kill->confused || kill->stunned || !kill->killer || kill->afraid)
			dam = 0;

		/* No use if aggrevation */
		if (borg_skill[BI_CRSAGRV])
			dam = 0;

		/* If successful, this immediately eliminates the threat */
		if (dam) {
			dam += borg_danger_aux(c_y, c_x, 2, i, TRUE, FALSE);

			/* Looks at neighboring grids. */
			for (j = 0; j < 48; j++) {
				int y2 = kill->y + borg_ddy_ddd[j];
				int x2 = kill->x + borg_ddx_ddd[j];

				/* Get the grid */
				if (!in_bounds(y2, x2))
					continue;
				ag = &borg_grids[y2][x2];

				/* bonus if the to-be-charmed guy is close to a monster */
				if (ag->kill &&
					 (!borg_kills[ag->kill].ally && !borg_kills[ag->kill].ally)) {
					/* Add bonus */
					dam += (borg_danger_aux(y2, x2, 1, i, TRUE, TRUE) * 3);
				}
			}
		}
		break;

	/* Various */
	case GF_OLD_HEAL:
	case GF_OLD_CLONE:
	case GF_OLD_SPEED:
	case GF_DARK_WEAK:
	case GF_KILL_DOOR:
	case GF_KILL_TRAP:
	case GF_MAKE_WALL:
	case GF_MAKE_DOOR:
	case GF_MAKE_TRAP:
	/* Teleport Away the Undead XXX */
	case GF_AWAY_UNDEAD:
		dam = 0;
		break;

	/* These spells which put the monster out of commission, we
	 * look at the danger of the monster prior to and after being
	 * put out of commission.  The difference is the damage.
	 * The following factors are considered when we
	 * consider the spell:
	 *
	 * 1. Is it already comprised by that spell?
	 * 2. Is it comprimised by another spell?
	 * 3. Does it resist the modality?
	 * 4. Will it make it's savings throw better than half the time?
	 * 5. We generally ignore these spells for breeders.
	 *
	 * The spell sleep II and sanctuary have a special consideration
	 * since the monsters must be adjacent to the player.
	 */

	case GF_AWAY_ALL:
		/* Teleport Other works differently.  Basically the borg
		 * will keep a list of all the monsters in the line of
		 * fire.  Then when he checks the danger, he will not
		 * include those monsters.
		 */

		/* Unique and it resists teleport */
		if (kill->unique && (r_ptr->flags3 & (RF3_RES_TELE))) {
			/* We cannot move these types */
			dam = -999;
			break;
		}

		/* uniques and does not resist teleport. */
		if (kill->unique && !(r_ptr->flags3 & (RF3_RES_TELE))) {
			/* This unique is low on HP, finish it off */
			if (kill->injury >= 60) {
				dam = -9999;
			}

			/* I am sitting pretty in an AS-Corridor */
			if ((borg_position & POSITION_SUMM))
				dam = -9999;

			/* If this unique is causing the danger, get rid of it */
			if (dam > avoidance * 13 / 10 && borg_skill[BI_CDEPTH] <= 98) {
				/* get rid of this unique by storing his info */
				borg_tp_other_index[borg_tp_other_n] = i;
				borg_tp_other_y[borg_tp_other_n] = kill->y;
				borg_tp_other_x[borg_tp_other_n] = kill->x;
				borg_tp_other_n++;
			}

			/* If fighting multiple uniques, get rid of one */

			/* Unique is adjacent to Borg */
			else if (borg_class == CLASS_MAGE && kill->dist <= 2) {
				/* get rid of unique next to me */
				borg_tp_other_index[borg_tp_other_n] = i;
				borg_tp_other_y[borg_tp_other_n] = kill->y;
				borg_tp_other_x[borg_tp_other_n] = kill->x;
				borg_tp_other_n++;

			}
			/* Unique in a vault, get rid of it, clean vault */
			else if ((borg_depth & DEPTH_VAULT)) {
				/* Scan grids adjacent to monster */
				for (ii = 0; ii < 8; ii++) {
					x = kill->x + ddx_ddd[ii];
					y = kill->y + ddy_ddd[ii];

					/* Access the grid */
					ag = &borg_grids[y][x];

					/* Skip unknown grids (important) */
					if (ag->feat == FEAT_NONE)
						continue;

					/* Count adjacent Permas */
					if (ag->feat == FEAT_PERM_INNER)
						vault_grids++;
				}

				/* Near enough perma grids? */
				if (vault_grids >= 2) {
					/* get rid of unique next to perma grids */
					borg_tp_other_index[borg_tp_other_n] = i;
					borg_tp_other_y[borg_tp_other_n] = kill->y;
					borg_tp_other_x[borg_tp_other_n] = kill->x;
					borg_tp_other_n++;
				}

			} else
				dam = -999;
			break;
		}
		/* Not a unique, but it does resist Teleport a bit */
		if (!kill->unique && (r_ptr->flags3 & (RF3_RES_TELE))) {
			/* get rid of this non-unique by storing his info */
			borg_tp_other_index[borg_tp_other_n] = i;
			borg_tp_other_y[borg_tp_other_n] = kill->y;
			borg_tp_other_x[borg_tp_other_n] = kill->x;
			borg_tp_other_n++;
			break;
		}

		/* Not a unique, and it does not resist Teleport */
		if (!kill->unique && !(r_ptr->flags3 & (RF3_RES_TELE))) {
			/* get rid of this non-unique by storing his info */
			borg_tp_other_index[borg_tp_other_n] = i;
			borg_tp_other_y[borg_tp_other_n] = kill->y;
			borg_tp_other_x[borg_tp_other_n] = kill->x;
			borg_tp_other_n++;
			break;
		}

	/* This teleport away is used to teleport away all monsters
	 * as the borg goes through his special attacks.
	 */
	case GF_AWAY_ALL_LUCIFER:
		/* Mostly no damage */
		dam = 0;

		/* Unique and it resists teleport */
		if (kill->unique && (r_ptr->flags3 & (RF3_RES_TELE))) {
			/* We cannot move these types */
			dam = -999;
			break;
		} else {
			/* If its touching a glyph grid, nail it. */
			for (j = 0; j < 8; j++) {
				int y2 = kill->y + ddy_ddd[j];
				int x2 = kill->x + ddx_ddd[j];

				/* Get the grid */
				ag = &borg_grids[y2][x2];

				/* If its touching a glyph grid, nail it. */
				if (ag->feat == FEAT_GLYPH) {
					/* get rid of this one by storing his info */
					borg_tp_other_index[borg_tp_other_n] = i;
					borg_tp_other_y[borg_tp_other_n] = kill->y;
					borg_tp_other_x[borg_tp_other_n] = kill->x;
					borg_tp_other_n++;
					dam = 3000;
				}
			}

			/* if it is too close */
			if (kill->dist <= 2)
				dam = 2000;

			/* If the borg is not in a good position, do it */
			if (((borg_depth & (DEPTH_SUMMONER & DEPTH_BORER)) &&
				  !(borg_position & POSITION_SEA)) ||
				 ((borg_depth & (DEPTH_BORER)) &&
				  !(borg_position & (POSITION_BORE | POSITION_SEA)))) {
				/* get rid of this one by storing his info */
				borg_tp_other_index[borg_tp_other_n] = i;
				borg_tp_other_y[borg_tp_other_n] = kill->y;
				borg_tp_other_x[borg_tp_other_n] = kill->x;
				borg_tp_other_n++;
				dam = 1000;
			}

			/* If the borg does not have enough Mana to attack this
			 * round and cast Teleport Away next round, then do it now.
			 */
			if (borg_skill[BI_CURSP] <= 35 && !borg_skill[BI_NSRANGED]) {
				/* get rid of this unique by storing his info */
				borg_tp_other_index[borg_tp_other_n] = i;
				borg_tp_other_y[borg_tp_other_n] = kill->y;
				borg_tp_other_x[borg_tp_other_n] = kill->x;
				borg_tp_other_n++;
				dam = 1500;
			}
		} /* Non uniques and non-tport resistors */
		break;

	/* In Z this does hurt Uniques but not in V */
	case GF_DISP_ALL:
		break;

	case GF_OLD_CONF:
		dam = 0;
		if (r_ptr->flags3 & RF3_NO_CONF)
			break;
		if (r_ptr->flags2 & RF2_MULTIPLY)
			break;
		if (kill->speed < r_ptr->speed - 5)
			break;
		if (kill->afraid)
			break;
		if (kill->confused)
			break;
		if (!kill->awake)
			break;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		borg_confuse_spell = FALSE;
		if (kill->dist >= 4)
			p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p1 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_confuse_spell = TRUE;
		if (kill->dist >= 4)
			p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_confuse_spell = FALSE;
		dam = (p1 - p2);
		break;

	case GF_TURN_ALL:
		dam = 0;
		if (kill->speed < r_ptr->speed - 5)
			break;
		if (r_ptr->flags3 & RF3_NO_FEAR)
			break;
		if (kill->afraid)
			break;
		if (kill->confused)
			break;
		if (!kill->awake)
			break;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		borg_fear_mon_spell = FALSE;
		p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_fear_mon_spell = TRUE;
		if (kill->dist >= 4)
			p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_fear_mon_spell = FALSE;
		dam = (p1 - p2);
		break;

	case GF_TURN_EVIL:
		dam = 0;
		if (!(r_ptr->flags3 & RF3_EVIL))
			break;
		if (kill->speed < r_ptr->speed - 5)
			break;
		if (r_ptr->flags3 & RF3_NO_FEAR)
			break;
		if (kill->afraid)
			break;
		if (kill->confused)
			break;
		if (!kill->awake)
			break;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		borg_fear_mon_spell = FALSE;
		p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_fear_mon_spell = TRUE;
		if (kill->dist >= 4)
			p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_fear_mon_spell = FALSE;
		dam = (p1 - p2);
		break;

	case GF_OLD_SLOW:
		dam = 0;
		if (kill->speed < r_ptr->speed - 5)
			break;
		if (kill->afraid)
			break;
		if (kill->confused)
			break;
		if (!kill->awake)
			break;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		borg_slow_spell = FALSE;
		p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_slow_spell = TRUE;
		if (kill->dist >= 4)
			p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_slow_spell = FALSE;
		dam = (p1 - p2);
		break;

	case GF_OLD_SLEEP:
	case GF_STASIS:
		dam = 0;
		if (typ == GF_OLD_SLEEP && r_ptr->flags3 & RF3_NO_SLEEP)
			break;
		if (kill->speed < r_ptr->speed - 5)
			break;
		if (kill->afraid)
			break;
		if (kill->confused)
			break;
		if (!kill->awake)
			break;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		borg_sleep_spell = FALSE;
		p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_sleep_spell = TRUE;
		if (kill->dist >= 4)
			p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		else
			p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
		borg_sleep_spell = FALSE;
		dam = (p1 - p2);
		break;

	case GF_OLD_POLY:
		dam = 0;
		if (kill->level >= skill)
			break;
		dam = -999;
		if (kill->unique)
			break;
		dam = borg_danger_aux(c_y, c_x, 2, i, TRUE, FALSE);
		/* dont bother unless he is a scary monster */
		if (dam < avoidance * 2)
			dam = 0;
		break;

	case GF_TURN_UNDEAD:
		if (r_ptr->flags3 & RF3_UNDEAD) {
			dam = 0;
			if (kill->confused)
				break;
			if (kill->afraid)
				break;
			if (kill->speed < r_ptr->speed - 5)
				break;
			if (!kill->awake)
				break;
			if (kill->level > borg_skill[BI_CLEVEL] - 5)
				break;
			borg_fear_mon_spell = FALSE;
			p1 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
			borg_fear_mon_spell = TRUE;
			if (kill->dist >= 4)
				p2 = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
			else
				p2 = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);
			borg_fear_mon_spell = FALSE;
			dam = (p1 - p2);
		} else {
			dam = 0;
		}
		break;

	/* Banishment-- cast when in extreme danger (checked in borg_defense). */
	case GF_AWAY_EVIL:
		if (r_ptr->flags3 & RF3_EVIL) {
			/* try not teleport away uniques. */
			if (r_ptr->flags1 & RF1_UNIQUE) {
				/* Banish ones with escorts */
				if (r_ptr->flags1 & RF1_ESCORT) {
					dam = 0;
				} else {
					/* try not Banish non escorted uniques */
					dam = -500;
				}

			} else {
				/* damage is the danger of the baddie */
				dam = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
			}
		} else {
			dam = 0;
		}
		break;
	}

	/* use Missiles on certain types of monsters */
	if ((borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE) >
		  avoidance * 3 / 10) ||
		 (r_ptr->flags1 & RF1_FRIENDS /* monster has friends*/ &&
		  kill->level >= borg_skill[BI_CLEVEL] - 5 /* close levels */) ||
		 (kill->ranged_attack /* monster has a ranged attack */) ||
		 special_target || (r_ptr->flags2 & RF2_MULTIPLY) ||
		 (borg_skill[BI_CLEVEL] <= 25 /* stil very weak */)) {
		borg_use_missile = TRUE;
	}

	/* Restore normal calcs of danger */
	borg_full_damage = FALSE;

	/* dont hurt friends or pets */
	if (kill->ally)
		dam = -500;

	/* Invuln monsters take no dam */
	if (kill->invulner && typ != GF_PSI_DRAIN)
		dam = 0;

	/* Return Damage as pure danger of the monster */
	if (typ == GF_AWAY_ALL || typ == GF_AWAY_EVIL)
		return (dam);

	/* Return non inflated score, if desired */
	if (!inflate)
		return (dam);

	/* Do not use super weak attacks against strong enemies.
	 * a creature with 2800 HP would need at least 12 damage
	 * to pass this test.
	 * Essentially, it will take 250 strikes to fell the monster.
	 */
	if (dam < kill->power / 250)
		dam = 0;

	/* If damage is zero, no further */
	if (dam <= 0)
		return (dam);

	/* Limit damage to twice maximal hitpoints */
	if ((!special_target && typ != GF_PSI_DRAIN) && dam > kill->power * 2)
		dam = kill->power * 2;

	/* give a small bonus for whacking a unique */
	/* this should be just enough to give prefrence to wacking uniques */
	if (special_target && borg_skill[BI_CDEPTH] >= 1)
		dam = (dam * 5);
	if (special_target && borg_skill[BI_CDEPTH] >= 1 && kill->injury > 80)
		dam += (dam * 99);

	/* Hack -- ignore Maggot until later.  Player will chase Maggot
	 * down all accross the screen waking up all the monsters.  Then
	 * he is stuck in a comprimised situation.
	 */
	if (kill->unique && borg_skill[BI_CDEPTH] == 0) {
		dam = dam * 2 / 3;

		/* Dont hunt maggot until later */
		if (borg_skill[BI_CLEVEL] < 5)
			dam = 0;
	}

	/* give a small bonus for whacking a breeder */
	if (r_ptr->flags2 & RF2_MULTIPLY)
		dam = (dam * 3 / 2);

	/*  Try to conserve missiles.
	 */
	if (typ == GF_ARROW ||
		 (typ >= GF_ARROW_SEEKER && typ <= GF_ARROW_WOUNDING)) {
		if (!borg_use_missile)
			/* set damage to zero, force borg to melee attack */
			dam = 0;
	}

	/* Damage */
	return (dam);
}
/*
 * Simulate / Invoke the launching of a bolt at a monster
 */
static int borg_launch_bolt_aux_hack(int i, int dam, int typ, bool inflate,
												 int ammo_location, int specific) {
	int d, p, x, y;
	int o_y = 0;
	int o_x = 0;
	int walls = 0;
	int unknown = 0;

	borg_grid *ag;

	borg_kill *kill;

	monster_race *r_ptr;

	/* Monster */
	kill = &borg_kills[i];

	/* monster race */
	r_ptr = &r_info[kill->r_idx];

	/* Skip dead monsters */
	if (!kill->r_idx)
		return (0);

	/* Require current knowledge */
	if (kill->when < borg_t - 2)
		return (0);

	/* Acquire location */
	x = kill->x;
	y = kill->y;

	/* Acquire the grid */
	ag = &borg_grids[y][x];

	/* Testing damage against a specific monster */
	if (specific >= 0)
		return (borg_launch_damage_one(i, dam, typ, inflate, ammo_location));

	/* Never shoot missiles into walls/doors */
	if (!borg_cave_floor_grid(ag) && !ag->kill &&
		 (typ == GF_ARROW ||
		  (typ >= GF_ARROW_SEEKER && typ <= GF_ARROW_WOUNDING)))
		return (0);

#if 0
    /* Hack -- Unknown grids should be avoided some of the time */
    if ((ag->feat == FEAT_NONE) && ((borg_t % 8) == 0)) return (0);

    /* Hack -- Weird grids should be avoided some of the time */
    if ((ag->feat == FEAT_INVIS) && ((borg_t % 8) == 0)) return (0);
#endif

	/* dont shoot at ghosts in walls, not perfect.  Its ok to cast spells into
	 * the walls though. */
	if ((r_ptr->flags2 & RF2_PASS_WALL) &&
		 (typ == GF_ARROW ||
		  (typ >= GF_ARROW_SEEKER && typ <= GF_ARROW_WOUNDING))) {
		/* if 2 walls and 1 unknown skip this monster */
		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		for (o_x = -1; o_x <= 1; o_x++) {
			for (o_y = -1; o_y <= 1; o_y++) {
				/* Acquire location */
				x = kill->x + o_x;
				y = kill->y + o_y;

				ag = &borg_grids[y][x];

				if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM_SOLID)
					walls++;
				if (ag->feat & FEAT_INVIS)
					unknown++;
			}
		}
		/* Is the ghost likely in a wall? */
		if (walls >= 2 && unknown >= 1)
			return (0);
	}

	/* Calculate damage */
	d = borg_launch_damage_one(i, dam, typ, inflate, ammo_location);

	/* Do not hit our pets.  Reduce damage by the pet's Hit Points */
	if (kill->ally) {
		d -= kill->power;
	}

	/* Calculate danger */
	borg_full_damage = TRUE;
	p = borg_danger_aux(y, x, 1, i, TRUE, FALSE);
	borg_full_damage = FALSE;

	/* Return Damage as pure danger of the monster */
	if ((typ == GF_AWAY_ALL || typ == GF_AWAY_EVIL) && d >= 1)
		return (p);

	/* Return the exact real damage; no inflation from danger */
	if (!inflate)
		return (d);

	/* Return 0 if the true damge (w/o the danger bonus) is 0 */
	if (d <= 0)
		return (d);

	/* Hack -- avoid waking most "hard" sleeping monsters */
	if (!kill->awake && (p > avoidance / 2) && borg_skill[BI_CLEVEL] < 50 &&
		 (d < kill->power) && !borg_munchkin_mode) {
		return (-999);
	}

	/* Hack -- ignore sleeping town monsters */
	if (!borg_skill[BI_CDEPTH] && !kill->awake) {
		return (0);
	}

	/* Calculate "danger" to player */
	borg_full_damage = TRUE;
	p = borg_danger_aux(c_y, c_x, 2, i, TRUE, FALSE);
	borg_full_damage = FALSE;

	/* Reduce "bonus" of partial kills */
	if (d < kill->power)
		p = p / 10;

	/* Add in power */
	d += p;

	/* Result */
	return (d);
}

/*
 * Determine the "reward" of launching a beam/bolt/ball at a location
 *
 * An "unreachable" location always has zero reward.
 *
 * Basically, we sum the "rewards" of doing the appropriate amount of
 * damage to each of the "affected" monsters.
 *
 * We will attempt to apply the offset-ball attack here
 */
static int borg_launch_bolt_aux(int y, int x, int rad, int dam, int typ,
										  int max, bool inflate, int ammo_location,
										  int specific, bool rzb) {
	/*int i;*/

	int x1, y1;
	int x2, y2;

	/*int dist;*/

	int r, n;
	int ry, rx;

	/* Number of grids in the "path" */
	int path_n = 0;
	int path_step = 1; /* current step on the pathway from target to origin */
	/*int ny;
	int nx;*/
	coord path_g[512];
	int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_THRU;
	/*int mon_count = 0;*/
	bool thru = FALSE;
	borg_grid *ag_path;
	monster_race *r_ptr_path;
	borg_kill *kill_path;

	borg_grid *ag;
	/*monster_race *r_ptr;*/
	/*borg_kill *kill;*/

	int q_x, q_y;

	/* Does the attack act like a beam and pass through monsters */
	if (rad == -1 || rad >= 10 || typ == GF_METEOR)
		thru = TRUE;

	/* check for legality */
	if (!in_bounds(y, x))
		return (0);

	/* Testing for damage against a specific monster */
	if (specific >= 0) {
		ag = &borg_grids[y][x];
		/*kill = &borg_kills[ag->kill];*/
		return (borg_launch_bolt_aux_hack(ag->kill, dam, typ, inflate,
													 ammo_location, specific));
	}

	/* Extract panel */
	q_x = w_x / 33;
	q_y = w_y / 11;

	/* Reset damage */
	n = 0;

	/* Initial location */
	x1 = c_x;
	y1 = c_y;

	/* Final location for beams */
	if (rad == -1 && rzb == FALSE) {
		path_n = borg_project_path(path_g, MAX_RANGE, c_y, c_x, y, x, flg);

		/* Redefine the target to the be end of the projection beam */
		y2 = path_g[path_n - 1].y;
		x2 = path_g[path_n - 1].x;
		max = path_n;
	}
	/* Final location for bolts */
	else if (rad == 0 && rzb == FALSE) {
		flg = PROJECT_STOP | PROJECT_KILL;
		path_n = borg_project_path(path_g, max, c_y, c_x, y, x, flg);

		/* Redefine the target to the be end of the projection beam */
		y2 = path_g[path_n - 1].y;
		x2 = path_g[path_n - 1].x;
		max = path_n;
	}
	/* ball spells */
	else if (rzb == TRUE || (rad != 0 && rad != -1)) {
		/* Final location for balls */
		flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		path_n = borg_project_path(path_g, MAX_RANGE, c_y, c_x, y, x, flg);

		/* Redefine the target to the be end of the projection beam */
		if (y != c_y || x != c_x) {
			y2 = path_g[path_n - 1].y;
			x2 = path_g[path_n - 1].x;
			max = path_n;
		} else /* ball centered on player */
		{
			y2 = c_y;
			x2 = c_x;
			max = 0;
		}
	}

	/* Start over */
	x = x1;
	y = y1;

	/* Simulate the spell/missile path */
	for (path_step = 0; path_step < max; path_step++) {
		/* Where on the pathstep are we? */
		y2 = path_g[path_step].y;
		x2 = path_g[path_step].x;

		/* Get the grid */
		ag = &borg_grids[y2][x2];
		/*if (ag->kill)
			kill = &borg_kills[ag->kill];*/
		/*if (ag->kill)
			r_ptr = &r_info[kill->r_idx];*/

		/* Get the grid of the pathway monster, if any */
		ag_path = &borg_grids[y2][x2];
		kill_path = &borg_kills[ag_path->kill];
		r_ptr_path = &r_info[kill_path->r_idx];

		/* Stop at walls */
		/* note: beams end at walls.  */
		if (path_step) {
			/* Stop at walls, trees, rubble, and such */
			/* note if beam, this is the end of the beam */
			/* dispel spells act like beams (sort of) */
			/* We are allowed to damage a monster in the wall */
			if (!borg_cave_floor_grid(ag) && !ag->kill) {
				if (thru)
					return (n);
				else
					return (0);
			}
		}

		/* Collect damage (bolts/beams) not balls except radius-0 balls. */
		if (rad <= 0)
			n += borg_launch_bolt_aux_hack(ag->kill, dam, typ, inflate,
													 ammo_location, -1);

		/* Check for arrival at "final target" */
		/* except beams and dispel effects, which keep going. */
		if ((!thru || (thru && typ == GF_METEOR)) && x == x2 && y == y2)
			break;

		/* Stop bolts at monsters  */
		if (!rad && ag->kill)
			return (n);

		/* The missile path can be complicated.  There are several checks
		 * which need to be made.  First we assume that we targetting
		 * a monster.  That monster could be known from either sight or
		 * ESP.  If the entire pathway from us to the monster is known,
		 * then there is no concern.  But if the borg is shooting through
		 * unknown grids, then there is a concern when he has ESP; without
		 * ESP he would not see that monster if the unknown grids
		 * contained walls or closed doors.
		 *
		 * 1.  ESP Inactive
		 *   A.  No Infravision
		 *       -Then the monster must be in a lit grid.
		 *   B.  Yes Infravision
		 *       -Then the monster must be projectable()
		 * 2.  ESP Active
		 *   A. No Infravision
		 *       -Then the monster could be in a lit grid.
		 *       -Or I detect it with ESP and it's not projectable().
		 *   B.  Yes Infravision
		 *       -Then the monster could be projectable()
		 *       -Or I detect it with ESP and it's not projectable().
		 *   -In the cases of ESP Active, the borg will test fire a missile.
		 *    Then wait for a 'painful ouch' from the monster.
		 */

		/* dont do the check if esp */
		if (!borg_skill[BI_ESP] && !borg_esp) {
			/* Check the missile path--no Infra, no HAS_LITE */
			if (path_step && (borg_skill[BI_INFRA] <= 0)
#ifdef MONSTER_LITE
				 && !(r_ptr->flags2 & RF2_HAS_LITE)
#endif /* has_lite */
					  ) {
				/* Stop at unknown grids (see above) */
				/* note if beam, dispel, this is the end of the beam */
				if (ag->feat == FEAT_NONE) {
					if (!thru)
						return (0);
					else
						return (n);
				}
				/* Stop at weird grids (see above) */
				/* note if beam, this is the end of the beam */
				if (ag->feat == FEAT_INVIS ||
					 (r_ptr_path->flags2 & RF2_PASS_WALL)) {
					if (!thru)
						return (0);
					else
						return (n);
				}
				/* Stop at unseen walls */
				/* We just shot and missed, this is our next shot */
				if (successful_target < 0) {
					/* When throwing things, it is common to just 'miss' */
					/* Skip only one round in this case */
					if (successful_target == -12)
						successful_target = 0;
					if (!thru)
						return (0);
					else
						return (n);
				}
			} else /* I do have infravision or it's a lite monster */
			{
				/* Stop at unseen walls */
				/* We just shot and missed, this is our next shot */
				if (successful_target < 0) {
					/* When throwing things, it is common to just 'miss' */
					/* Skip only one round in this case */
					if (successful_target == -12)
						successful_target = 0;
					if (!thru)
						return (0);
					else
						return (n);
				}
			}
		} else /* I do have ESP */
		{
			/* Check the missile path */
			if (path_step) {
				/* if this area has been magic mapped,
				* ok to shoot in the dark.
		  * Also ok, to shoot into the dark if there is regional fear, which is
		  * probably due to a non-LOS monster.
				*/
				if (!borg_detect_wall[q_y + 0][q_x + 0] &&
					 !borg_detect_wall[q_y + 0][q_x + 1] &&
					 !borg_detect_wall[q_y + 1][q_x + 0] &&
					 !borg_detect_wall[q_y + 1][q_x + 1] &&
					 borg_fear_region[c_y / 11][c_x / 11] < avoidance / 20) {

					/* Stop at unknown grids (see above) */
					/* note if beam, dispel, this is the end of the beam */
					if (ag->feat == FEAT_NONE) {
						if (!thru)
							return (0);
						else
							return (n);
					}
					/* Stop at unseen walls */
					/* We just shot and missed, this is our next shot */
					if (successful_target <= 1) {
						/* When throwing things, it is common to just 'miss' */
						/* Skip only one round in this case */
						if (successful_target == -12)
							successful_target = 0;
						if (!thru)
							return (0);
						else
							return (n);
					}
				}

				/* Stop at weird grids (see above) */
				/* note if beam, this is the end of the beam */
				if (ag->feat == FEAT_INVIS ||
					 (r_ptr_path->flags2 & RF2_PASS_WALL)) {
					if (!thru)
						return (0);
					else
						return (n);
				}
				/* Stop at unseen walls */
				/* We just shot and missed, this is our next shot */
				if (successful_target <= 1) {
					/* When throwing things, it is common to just 'miss' */
					/* Skip only one round in this case */
					if (successful_target == -12)
						successful_target = 0;

					if (!thru)
						return (0);
					else
						return (n);
				}
			}
		}
	}

	/* Bolt/Beam attack */
	if (rad <= 0)
		return (n);

	/* Excessive distance */
	if (path_step >= MAX_RANGE)
		return (0);

	/* Check monsters in blast radius, including the center.
	* And apply Dispel type
	 * spells or spells which center on the player with a
	 * particular radius
	 */
	/* Check monsters and objects in blast radius */
	for (ry = y2 - rad; ry <= y2 + rad; ry++) {
		for (rx = x2 - rad; rx <= x2 + rad; rx++) {

			/* Bounds check */
			if (!in_bounds(ry, rx))
				continue;

			/* Get the grid */
			ag = &borg_grids[ry][rx];

			/* Check distance */
			r = distance(y2, x2, ry, rx);

			/* Maximal distance */
			if (r > rad)
				continue;

			/* Never pass through walls*/
			if (!borg_los(y2, x2, ry, rx))
				continue;

			/*  dispel spells should hurt the same no matter the rad: make r= y
			 * and x */
			if (rad == MAX_SIGHT)
				r = 0;

			/*  dispel spells should hurt the same no matter the rad: make r= y
			 * and x */
			if (rad == 10)
				r = 0;

			/* Collect damage, lowered by distance */
			n += borg_launch_bolt_aux_hack(ag->kill, dam / (r + 1), typ, inflate,
													 ammo_location, -1);

			/* probable damage int was just changed by b_l_b_a_h*/

			/* check destroyed stuff. */
			if (ag->take) {
				borg_take *take = &borg_takes[ag->take];
				object_kind *k_ptr = &k_info[take->k_idx];

				switch (typ) {
				case GF_ACID: {
					/* rings/boots cost extra (might be speed!) */
					if (k_ptr->tval == TV_BOOTS && !k_ptr->aware) {
						n -= 20;
					}
					break;
				}
				case GF_ELEC: {
					/* rings/boots cost extra (might be speed!) */
					if (k_ptr->tval == TV_RING && !k_ptr->aware) {
						n -= 20;
					}
					if (k_ptr->tval == TV_RING && k_ptr->sval == SV_RING_SPEED) {
						n -= 2000;
					}
					break;
				}

				case GF_FIRE: {
					/* rings/boots cost extra (might be speed!) */
					if (k_ptr->tval == TV_BOOTS && !k_ptr->aware) {
						n -= 20;
					}
					break;
				}
				case GF_COLD:
				case GF_SOUND: {
					if (k_ptr->tval == TV_POTION) {
						if (k_ptr->sval == SV_POTION_CURE_CRITICAL)
							n -= 20;

						/* Extra penalty for cool potions */
						if (!k_ptr->aware || k_ptr->sval == SV_POTION_HEALING ||
							 k_ptr->sval == SV_POTION_STAR_HEALING ||
							 k_ptr->sval == SV_POTION_LIFE ||
							 (k_ptr->sval == SV_POTION_INC_STR &&
							  amt_add_stat[A_STR] >= 1000) ||
							 (k_ptr->sval == SV_POTION_INC_INT &&
							  amt_add_stat[A_INT] >= 1000) ||
							 (k_ptr->sval == SV_POTION_INC_WIS &&
							  amt_add_stat[A_WIS] >= 1000) ||
							 (k_ptr->sval == SV_POTION_INC_DEX &&
							  amt_add_stat[A_DEX] >= 1000) ||
							 (k_ptr->sval == SV_POTION_INC_CON &&
							  amt_add_stat[A_CON] >= 1000))
							n -= 2000;
					}
					break;
				}
				case GF_MANA: {
					/* Used against uniques, allow the stuff to burn */
					break;
				}
				}
			}
		}
	}
	/* Result */
	return (n);
}

/*
 * Simulate/Apply the optimal result of launching a beam/bolt/ball
 *
 * Note that "beams" have a "rad" of "-1", "bolts" have a "rad" of "0",
 * and "balls" have a "rad" of "2" or "3", depending on "blast radius".
 * Some spells are a radius-0 ball spell, affecting only 1 creature.
 */
static int borg_launch_bolt(int rad, int dam, int typ, int max, bool inflate,
									 int ammo_location, int specific, bool offset,
									 bool rzb) {
	/*int num = 0;*/

	int i, b_i = -1;
	int n = 0, b_n = 0;
	int b_o_y = 0, b_o_x = 0;
	int o_y = 0, o_x = 0;
	int d, b_d = MAX_RANGE;
	int x, y;
	int b_borg_tp_other_n = -1;

	/** Examine possible destinations **/

	/* Dispel Spells and Special Ball Spells centered on Player */
	if (rad >= 10) {
		/* some Dispel-Type Spells have limted range */
		max = rad - 10;

		/* When looking at a specific monster, lets consider his spot */
		if (specific >= 0) {
			x = borg_kills[specific].x;
			y = borg_kills[specific].y;
			return (borg_launch_bolt_aux(y, x, rad - 10, dam, typ, max, inflate,
												  ammo_location, specific, rzb));
		}
		/* Consider it centered on Player */
		return (borg_launch_bolt_aux(c_y, c_x, rad - 10, dam, typ, max, inflate,
											  ammo_location, specific, rzb));
	}

	/* When looking at a specific monster, lets consider his spot */
	if (specific >= 0) {
		x = borg_kills[specific].x;
		y = borg_kills[specific].y;
		return (borg_launch_bolt_aux(y, x, rad, dam, typ, max, inflate,
											  ammo_location, specific, rzb));
	}

	/* This will allow the borg to target places adjacent to a monster
	 * in order to exploit and abuse a feature of the game.  Whereas,
	 * the borg, while targeting a monster will not score d/t walls, he
	 * could land a successful hit by targeting adjacent to the monster.
	 * For example:
	 * ######################
	 * #####@..........######
	 * ############P..x......
	 * ######################
	 * In order to hit the P, the borg must target the x and not the P.
	 *
	 */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Certain borgs should avoid using spell attacks when the monster is
		 * immediately adjacent */
		if (distance(y, x, c_y, c_x) == 1 && !borg_skill[BI_NO_MELEE] &&
			 (typ != GF_ARROW && typ != GF_STASIS && typ != GF_CONTROL_ANIMAL &&
			  typ != GF_CONTROL_UNDEAD && typ != GF_OLD_SLOW &&
			  typ != GF_OLD_SLEEP && typ != GF_OLD_DRAIN && typ != GF_CONFUSION &&
			  typ != GF_TURN_ALL && typ != GF_DOMINATION && typ != GF_CHARM &&
			  typ != GF_PSI_DRAIN && typ != GF_STUN &&
			  !(typ >= GF_ARROW_ANIMAL && typ <= GF_ARROW_WOUNDING)))
			continue;

		/* Consider each adjacent/neighboring spot to and on top of the monster*/
		for (o_x = -2; o_x <= 2; o_x++) {
			for (o_y = -2; o_y <= 2; o_y++) {
				/* Acquire location */
				x = borg_temp_x[i] + o_x;
				y = borg_temp_y[i] + o_y;

				/* Reset N */
				n = 0;

				/* Skip the borg's grid (if monster is adjacent */
				if (y == c_y && x == c_x)
					continue;

				/* Some spells are radius-0 ball effects */
				if ((offset == FALSE || rzb == TRUE) && (o_x != 0 || o_y != 0))
					continue;

				/* Reset Teleport Other variables */
				borg_tp_other_n = 0;

				/* track the distance to the monster */
				d = distance(c_y, c_x, borg_temp_y[i], borg_temp_x[i]);

				/* The game forbids targetting the outside walls (bounds)*/
				if (x <= 0 || y <= 0 || x >= MAX_WID - 1 || y >= MAX_HGT - 1)
					continue;

				/* Consider it if its a ball spell (not not making a sea of runes in
				 * order to save mana) or right on top of it */
				if (rad >= 2) {
					/* right on top of the guy */
					if (y == borg_temp_y[i] && x == borg_temp_x[i])
						n = borg_launch_bolt_aux(y, x, rad, dam, typ, max, inflate,
														 ammo_location, specific, rzb);
					else if ((borg_depth & (DEPTH_SUMMONER & DEPTH_BORER)) &&
								(borg_position & POSITION_SEA))
						n = borg_launch_bolt_aux(y, x, rad, dam, typ, max, inflate,
														 ammo_location, specific, rzb);
					else if ((borg_depth & (DEPTH_BORER)) &&
								(borg_position & (POSITION_BORE | POSITION_SEA)))
						n = borg_launch_bolt_aux(y, x, rad, dam, typ, max, inflate,
														 ammo_location, specific, rzb);
				} else
				/* not a ball spell */
				{
					n = borg_launch_bolt_aux(y, x, rad, dam, typ, max, inflate,
													 ammo_location, specific, rzb);
				}

				/* Skip useless attacks */
				if (n <= 0)
					continue;

				/* Skip offset attack it its the same value as the direct hit */
				if (o_x != 0 && o_y != 0 && n <= b_n)
					continue;

				/* Collect best attack */
				if ((b_i >= 0) && (n < b_n))
					continue;

				/* Skip attacking farther monster if rewards are equal. */
				if (n == b_n && d > b_d)
					continue;

				/* Track it */
				b_i = i;
				b_n = n;
				b_o_y = o_y;
				b_o_x = o_x;
				b_d = d;
				b_borg_tp_other_n = borg_tp_other_n;
			}
		}
	}

	/* Reset Teleport Other variables */
	borg_tp_other_n = b_borg_tp_other_n;

	/* Save the location */
	g_x = borg_temp_x[b_i] + b_o_x;
	g_y = borg_temp_y[b_i] + b_o_y;

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Target the location */
	(void)borg_target(g_y, g_x);

	/* Result */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of launching a dispel attack
 *
 */
static int borg_launch_dispel(int rad, int dam, int typ, int max, bool inflate,
										int ammo_location, int specific) {
	/*int num = 0;*/

	int i = 0;
	int n = 0;
	int dispel_dam = dam;
	int dispel_distance = 0;

	/* Examine the monsters */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Acquire location */
		x = borg_temp_x[i];
		y = borg_temp_y[i];
		if (specific >= 0) {
			x = borg_temp_x[specific];
			y = borg_temp_y[specific];
			return (borg_launch_bolt_aux_hack(borg_grids[y][x].kill, dispel_dam,
														 typ, inflate, ammo_location,
														 specific));
		}

		/* Bounds check */
		if (!in_bounds(y, x))
			continue;

		/* Skip places that are out of range */
		dispel_distance = distance(c_y, c_x, y, x);
		if (dispel_distance > max)
			continue;
		if (dispel_distance > rad)
			continue;

		/* Can it be seen/hit.  Verified both ways to avoid 'knight movement'
		 * errors */
		if (!borg_projectable(c_y, c_x, y, x, TRUE, TRUE))
			continue;
		if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE))
			continue;

		/* Reduce the damage based on the Dispel effect.  Decreased damage with
		 * distance */
		dispel_dam = (dam + dispel_distance) / (dispel_distance + 1);

		/* Consider it  (look at visability here as well) */
		n += borg_launch_bolt_aux_hack(borg_grids[y][x].kill, dispel_dam, typ,
												 inflate, ammo_location, -1);
	}

	/* Reset Teleport Other variables */
	/* borg_tp_other_n = 0; */

	/* Simulation */
	if (borg_simulate)
		return (n);

	/* Result */
	return (n);
}

/*
 * Simulate/Apply the optimal result of launching a normal missile
 *
 * First, pick the "optimal" ammo, then pick the optimal target
 */
static int borg_attack_aux_launch(bool inflate, int specific) {
	int b_n = 0;

	int k, b_k = -1;
	int d, b_d = -1;

	borg_item *bow = &borg_items[INVEN_BOW];

	/* Scan the pack */
	for (k = 0; k < INVEN_PACK; k++) {

		borg_item *item = &borg_items[k];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip Ego branded items--they are looked at later */
		if (item->name2)
			continue;

		/* Skip bad missiles */
		if (item->tval != my_ammo_tval)
			continue;

		/* Skip worthless missiles */
		if (item->value <= 0)
			continue;

		/* Skip un-identified, non-average, missiles */
		if (!item->ident && !strstr(item->desc, "{average") &&
			 !strstr(item->desc, "{good") && !strstr(item->desc, "{excellent"))
			continue;

		/* Determine average damage */
		d = (item->dd * (item->ds + 1) / 2);
		d = d + item->to_d + bow->to_d;
		d = d * my_ammo_power * borg_skill[BI_SHOTS];

		/* Paranoia */
		if (d <= 0)
			continue;

		if ((b_k >= 0) && (d <= b_d))
			continue;

		b_k = k;
		b_d = d;
	}

	/* Nothing to use */
	if (b_k < 0)
		return (0);

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Choose optimal type of bolt */
	b_n = borg_launch_bolt(0, b_d, GF_ARROW, 5 + 5 * my_ammo_power, inflate, b_k,
								  specific, TRUE, FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Do it */
	borg_note(format("# Firing standard missile '%s'", borg_items[b_k].desc));

	/* Fire */
	borg_keypress('f');

	/* Use the missile */
	borg_keypress(I2A(b_k));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -2;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of launching a SEEKER missile
 *
 * First, pick the "optimal" ammo, then pick the optimal target
 */
static int borg_attack_aux_launch_seeker(bool inflate, int specific) {
	int b_n = 0;

	int k, b_k = -1;
	int d, b_d = -1;

	borg_item *bow = &borg_items[INVEN_BOW];

	/* Scan the pack */
	for (k = 0; k < INVEN_PACK; k++) {

		borg_item *item = &borg_items[k];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip non-seekers items--they are looked at later */
		if (item->sval != SV_AMMO_HEAVY)
			continue;

		/* Skip bad missiles */
		if (item->tval != my_ammo_tval)
			continue;

		/* Skip worthless missiles */
		if (item->value <= 0)
			continue;

		/* Skip un-identified, non-average, missiles */
		if (!item->ident && !strstr(item->desc, "{average"))
			continue;

		/* Determine average damage */
		d = (item->dd * (item->ds + 1) / 2);
		d = d + item->to_d + bow->to_d;
		d = d * my_ammo_power * borg_skill[BI_SHOTS];

		/* Paranoia */
		if (d <= 0)
			continue;

		if ((b_k >= 0) && (d <= b_d))
			continue;

		b_k = k;
		b_d = d;
	}

	/* Nothing to use */
	if (b_k < 0)
		return (0);

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Choose optimal type of bolt */
	b_n = borg_launch_bolt(0, b_d, GF_ARROW_SEEKER, 5 + 5 * my_ammo_power,
								  inflate, b_k, specific, TRUE, FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Do it */
	borg_note(format("# Firing heavy missile '%s'", borg_items[b_k].desc));

	/* Fire */
	borg_keypress('f');

	/* Use the missile */
	borg_keypress(I2A(b_k));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -2;

	/* Value */
	return (b_n);
}
/*
 * Simulate/Apply the optimal result of launching a branded missile
 *
 * First, pick the "optimal" ammo, then pick the optimal target
 */
static int borg_attack_aux_launch_ego(bool inflate, int specific) {
	int n, b_n = -1;

	int k, b_k = -1;
	int d;
	int brand = GF_ARROW;

	borg_item *bow = &borg_items[INVEN_BOW];

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Scan the pack */
	for (k = 0; k < INVEN_PACK; k++) {
		borg_item *item = &borg_items[k];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* skip seeker ammo */
		if (item->sval == SV_AMMO_HEAVY)
			continue;

		/* Must be an ego type */
		if (!item->name2)
			continue;

		/* Skip bad missiles */
		if (item->tval != my_ammo_tval)
			continue;

		/* Skip worthless missiles */
		if (item->value <= 0)
			continue;

		/* Skip un-identified, non-average, missiles */
		if (!item->ident && !strstr(item->desc, "{average"))
			continue;

		/* Determine average damage */
		d = (item->dd * (item->ds + 1) / 2);
		d = d + item->to_d + bow->to_d;
		d = d * my_ammo_power * borg_skill[BI_SHOTS];

		/* Paranoia */
		if (d <= 0)
			continue;

		/* Determine the ego branding */
		switch (item->name2) {
		case EGO_HURT_ANIMAL:
			brand = GF_ARROW_ANIMAL;
			break;
		case EGO_HURT_EVIL:
			brand = GF_ARROW_EVIL;
			break;
		case EGO_HURT_DRAGON:
			brand = GF_ARROW_DRAGON;
			break;
		case EGO_SLAYING:
			brand = GF_ARROW_SLAYING;
			break;
		case EGO_LIGHTNING_BOLT:
			brand = GF_ARROW_ELEC;
			break;
		case EGO_FLAME:
			brand = GF_ARROW_FLAME;
			break;
		case EGO_FROST:
			brand = GF_ARROW_FROST;
			break;
		case EGO_WOUNDING:
			brand = GF_ARROW_WOUNDING;
			break;
		default:
			brand = GF_ARROW;
			break;
		}

		/* Choose optimal type of bolt */
		n = borg_launch_bolt(0, d, brand, 5 + 5 * my_ammo_power, inflate, b_k,
									specific, TRUE, FALSE);

		if ((b_n >= 0) && (n <= b_n))
			continue;
		b_k = k;
		b_n = n;
	}

	/* Nothing to use */
	if (b_k < 0)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Do it */
	borg_note(format("# Firing ego branded missile '%s'", borg_items[b_k].desc));

	/* Fire */
	borg_keypress('f');

	/* Use the missile */
	borg_keypress(I2A(b_k));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/* Attempt to rest on the grid to allow the monster to approach me.
 * Make sure the monster does not have a ranged attack and that I am
 * inclined to attack him.
 */
static int borg_attack_aux_rest(void) {
	int i;
	int resting_is_good = -1;
	bool los = FALSE;
	int b_i = -1;

	int my_danger = borg_danger(c_y, c_x, 1, FALSE);

	/* Examine all the monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];
		/*monster_race *r_ptr = &r_info[kill->r_idx];*/

		int x9 = kill->x;
		int y9 = kill->y;
		int ax, ay, d;
		/*int my_scan;
		int mon_scan;*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip friendlies */
		if (kill->ally)
			continue;

		/* Distance components */
		ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
		ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

		/* Distance */
		d = MAX(ax, ay);

		/* Minimal and maximal distance */
		if (d == 1) {
			resting_is_good = 0;
			continue;
		}
		if (d != 2)
			continue;

		/* Check LOS */
		los = kill->los;

		/* Ranged Attacks, don't rest. */
		if (kill->ranged_attack && los) {
			resting_is_good = 0;
			continue;
		}

		/* Skip the sleeping ones */
		if (!kill->awake)
			continue;

		/* need to have seen it recently */
		if (borg_t - kill->when > 10)
			continue;

		/* Skip monsters that dont chase */
		if (r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE)
			continue;

		/* Monster better not be faster than me */
		if (kill->speed - borg_skill[BI_SPEED] >= 5 && los)
			continue;

		/* Should be flowing towards the monster */
		if (goal != GOAL_KILL && los)
			continue;

		/* Might be a little dangerous to just wait here */
		if (my_danger > borg_skill[BI_CURHP])
			continue;

		/** Monster must be allowed to get to me
		 **/
		borg_flow_clear_m();
		borg_flow_enqueue_grid_m(c_y, c_x);
		borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
		if (borg_flow_commit_m(kill->y, kill->x)) {
			resting_is_good = 1;
			b_i = i;
		} else {
			continue;
		}

		/* Should be a good idea to wait for monster here. */
		if (resting_is_good == -1) {
			resting_is_good = 1;
			b_i = i;
		}
	}

	/* Resting while in a sea of runes.  Either building one or completed one. */
	if (resting_is_good != 0 &&
		 ((glyph_x == c_x &&
			glyph_y == c_y) || /* off center, probably building it */
		  (glyph_y_center == c_y &&
			glyph_x_center == c_x) || /* center, but not comleted building */
		  ((borg_position &
			 (POSITION_SEA | POSITION_BORE))))) /* completed, centered, waiting */
	{
		/* Do I need to rest while building it? */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] ||
			 (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] / 3 &&
			  borg_skill[BI_MAXSP] > 25)) {
			/* Don't wait forever, they need to have been seen recently */
			if (borg_depth & (DEPTH_SUMMONER | DEPTH_BORER))
				resting_is_good = 1;
		}

	}
	/* Monster might not want to flow to the borg */
	else if (time_this_panel >= 300)
		return (FALSE);

	/* Not a good idea */
	if (resting_is_good != 1) {
		return (0);
	}

	/* Return some value for this rest */
	if (borg_simulate)
		return (1);

	/* Rest */
	borg_keypress(',');
	if (b_i >= 1)
		borg_note(
			 format("# Resting on grid (%d, %d), waiting for %s to approach.", c_y,
					  c_x, (r_name + r_info[borg_kills[b_i].r_idx].name)));
	else
		borg_note(format("# Resting on grid (%d, %d) seems like a good idea.",
							  c_y, c_x));

	/* All done */
	return (1);
}

/*
 * Simulate/Apply the optimal result of throwing an object
 *
 * First choose the "best" object to throw, then check targets.
 */
static int borg_attack_aux_object(bool inflate, int specific) {
	int b_n;

	int b_r = 0;

	int k, b_k = -1;
	int d, b_d = -1;

	int div, mul;

	/* Scan the pack */
	for (k = 0; k < INVEN_PACK; k++) {
		borg_item *item = &borg_items[k];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip un-identified, non-average, objects */
		/* if (!item->aware && !strstr(item->desc, "{average")) continue; */

		/* Skip my spell/prayer book */
		if (item->tval == REALM1_BOOK || item->tval == REALM2_BOOK)
			continue;

		/* Skip stuff that is valuable */
		if (item->value >= 25 /* && item->kind != K_FIGURINE*/)
			continue;

		/* Dont throw wands or rods */
		if (item->tval == TV_ROD || item->tval == TV_ROD)
			continue;

		/* Skip "equipment" items (not ammo) */
		if (borg_wield_slot(item) >= 0)
			continue;

		/* Don't throw stones if I am wielding a sling */
		if (item->tval == TV_SHOT && borg_items[INVEN_BOW].sval == SV_SLING)
			continue;

		/* Determine average damage from object */
		d = (k_info[item->kind].dd * (k_info[item->kind].ds + 1) / 2);

		/* Magical figurines are cool because they summon a pet
		if (item->kind == K_FIGURINE)
			d += item->level * 2;
		if (item->kind == K_FIGURINE && borg_fighting_unique)
			d *= 3;
		*/

		/* some potions can explode and do some damage or effect (sleep, blind),
		 * some heal the monsters */
		if (item->tval == TV_POTION) {
			/* If unID'd assume it can do some extra damage */
			if (!item->aware)
				d = d * 5;

			switch (item->sval) {
			case SV_POTION_SLOWNESS:
			case SV_POTION_POISON:
			case SV_POTION_BLINDNESS:
			case SV_POTION_CONFUSION: /* Booze */
			case SV_POTION_SLEEP:
				d = d * 5;
				break;
			case SV_POTION_CURE_LIGHT:
			case SV_POTION_CURE_SERIOUS:
			case SV_POTION_CURE_CRITICAL:
			case SV_POTION_CURING:
			case SV_POTION_HEALING:
			case SV_POTION_SPEED:
				d = -500;
				break;
			}
		}

		/* Skip useless stuff */
		if (d <= 0)
			continue;

		/* Dont throw certain items in town */
		if (d < item->value && borg_skill[BI_CDEPTH] == 0)
			continue;

		/* Skip "expensive" stuff */
		if (d < item->value && borg_skill[BI_CLEVEL] > 5 /*&&
			 item->kind != K_FIGURINE*/)
			continue;

		/* Hack -- Save last flasks for fuel, if needed */
		if (item->tval == TV_FLASK && !borg_fighting_unique &&
			 borg_skill[BI_CURHP] >= 5)
			continue;

		/* Ignore worse damage */
		if ((b_k >= 0) && (d <= b_d))
			continue;

		/* Track */
		b_k = k;
		b_d = d;

		/* Extract a "distance multiplier" */
		mul = 10;

		/* Enforce a minimum "weight" of one pound */
		div = ((item->weight > 10) ? item->weight : 10);

		/* Hack -- Distance -- Reward strength, penalize weight */
		b_r = (adj_str_blow[my_stat_ind[A_STR]] + 20) * mul / div;

		/* Max distance of 10 */
		if (b_r > 10)
			b_r = 10;
	}

	/* Nothing to use */
	if (b_k < 0)
		return (0);

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(0, b_d, GF_ARROW, b_r, inflate, b_k, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Do it */
	borg_note(format("# Throwing painful object '%s'", borg_items[b_k].desc));

	/* Fire */
	borg_keypress('v');

	/* Use the object */
	borg_keypress(I2A(b_k));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -2;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 *
 * Take into account the failure rate of spells/objects/etc.  XXX XXX XXX
 */
static int borg_attack_aux_spell_bolt(int realm, int book, int what, int rad,
												  int dam, int typ, bool inflate,
												  int specific) {
	int b_n;
	int penalty = 0;
	int vamp = 0;
	int fail = 25;
	bool offset = TRUE;
	int cost = 0;
	bool rzb = FALSE;

	borg_magic *as = &borg_magics[realm][book][what];

	/* Modify Fail Rate */
	if (borg_munchkin_mode)
		fail = 70;
	if (borg_fighting_unique)
		fail = 40;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* make sure I am powerfull enough to do another goi if this one falls */
	if ((borg_goi || borg_wraith) && ((borg_skill[BI_CURSP] - as->power) < 70))
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 3))
		return (0);

	/* Require ability (right now) */
	if (!borg_spell_okay_fail(realm, book, what, fail))
		return (0);

	/* Some spells are radius-0 ball effects */
	if ((realm == REALM_DEATH && book == 0 && what == 1)) {
		offset = FALSE;
		rzb = TRUE;
	}

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific,
								  offset, rzb);

	/* ratio of mana: damage */
	if (b_n >= 1)
		cost = (b_n * 100) / (as->power * 100);

	/* weaklings need that spell, they dont get penalized */
	if (book == 0 && borg_skill[BI_MAXCLEVEL] <= 30) {
		if (borg_simulate)
			return (b_n);
	}

	/* If Death: Vampirism True, it heals us nicely */
	if (realm == REALM_DEATH && book == 2 && what == 4 && b_n >= 50 &&
		 (borg_skill[BI_MAXHP] - 250 >= borg_skill[BI_CURHP]))
		b_n = b_n * 3;

	/* If Death: Vampiric Drain, it heals us a little */
	vamp = (borg_skill[BI_CLEVEL] +
			  (borg_skill[BI_CLEVEL] / 2 * (MAX(1, borg_skill[BI_CLEVEL] / 10))));
	if (realm == REALM_DEATH && book == 1 && what == 3 && b_n >= 25 &&
		 (borg_skill[BI_MAXHP] - vamp >= borg_skill[BI_CURHP]))
		b_n = b_n * 3;

	/* Vampirism is nutritious */
	if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) &&
		 realm == REALM_DEATH &&
		 ((book == 2 && what == 4) || (book == 1 && what == 3)))
		b_n = b_n * (3 + borg_skill[BI_ISWEAK]);

	/* Penalize mana usage */
	if (as->power > b_n && borg_class != CLASS_HIGH_MAGE)
		b_n = b_n - as->power;

	/* Penalize use of reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 3 &&
		 borg_class != CLASS_HIGH_MAGE)
		b_n = b_n - (as->power * 2);

	/* Penalize use of deep reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 4 &&
		 borg_class != CLASS_HIGH_MAGE)
		b_n = b_n - (as->power * 5);

	/* Really penalize use of mana needed for final teleport */
	if (borg_class == CLASS_MAGE)
		penalty = 6;
	if (borg_class == CLASS_RANGER)
		penalty = 22;
	if (borg_class == CLASS_ROGUE)
		penalty = 20;
	if (borg_class == CLASS_PRIEST)
		penalty = 8;
	if (borg_class == CLASS_PALADIN)
		penalty = 20;
	if ((borg_skill[BI_MAXSP] > 30) &&
		 (borg_skill[BI_CURSP] - as->power < penalty))
		b_n = b_n - (as->power * 750);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the spell */
	(void)borg_spell(realm, book, what);

	/* Report on the value of the spell in terms for mana for damage */
	borg_note(format("# Mana:Damage Ratio %d:%d (%d)", as->power, b_n, cost));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/* This routine is the same as the one above only in an emergency case.
 * The borg will enter negative mana casting this
 */
static int borg_attack_aux_spell_bolt_reserve(int realm, int book, int what,
															 int rad, int dam, int typ,
															 bool inflate, int specific) {
	int b_n;
	int i;
	bool offset = TRUE;
	bool rzb = FALSE;

	/* Fake our Mana */
	int sv_mana = borg_skill[BI_CURSP];

	/* Only Weak guys should try this */
	if (borg_skill[BI_CLEVEL] >= 15)
		return (0);

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISWEAK])
		return (0);

	/* Must not have enough mana right now */
	if (borg_spell_okay_fail(realm, book, what, 25))
		return (0);

	/* If there is more than one close monster, don't risk fainting */
	if (borg_temp_n > 1)
		return (0);

	/* Must be dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) < avoidance * 2)
		return (0);

	/* Find the monster */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* check the location */
		if (borg_temp_x[0] != kill->x || borg_temp_y[0] != kill->y)
			continue;

		/* If it has too many hp to be taken out with this */
		/* spell, don't bother trying */
		/* NOTE: the +4 is because the damage is toned down
					as an 'average damage' */
		if (kill->power > (dam + 4))
			return (0);

		break;
	}

	/* Require ability (with faked mana) */
	borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];
	if (!borg_spell_okay_fail(realm, book, what, 25)) {
		/* Restore Mana */
		borg_skill[BI_CURSP] = sv_mana;
		return (0);
	}

	/* Some spells are radius-0 ball effects */
	if ((realm == REALM_DEATH && book == 0 && what == 1)) {
		offset = FALSE;
		rzb = TRUE;
	}

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific,
								  offset, rzb);

	/* return the value */
	if (borg_simulate) {
		/* Restore Mana */
		borg_skill[BI_CURSP] = sv_mana;
		return (b_n);
	}

	/* Cast the spell with fake mana */
	borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];
	if (borg_spell_fail(realm, book, what, 25)) {
		/* Note the use of the emergency spell */
		borg_note("# Emergency use of an Attack Spell.");

		/* verify use of spell */
		/* borg_keypress('y'); */
	}

	/* Use target */
	/* borg_keypress('5'); */
	borg_confirm_target = TRUE;

	/* Set our shooting flag */
	successful_target = -1;

	/* restore true mana */
	borg_skill[BI_CURSP] = 0;

	/* Value */
	return (b_n);
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" attack prayer
 */
static int borg_attack_aux_spell_dispel(int realm, int book, int what, int rad,
													 int dam, int typ, bool inflate,
													 int specific) {
	int b_n;
	int penalty = 0;

	borg_magic *as = &borg_magics[realm][book][what];

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISWEAK])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Require ability */
	if (!borg_spell_okay_fail(realm, book, what, 25))
		return (0);

	/* Choose optimal location--*/
	b_n = borg_launch_dispel(rad, dam, typ, MAX_RANGE, inflate, 0, specific);

	/* Penalize mana usage */
	b_n = b_n - as->power;

	/* Penalize use of reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 2)
		b_n = b_n - (as->power * 2);

	/* Penalize use of deep reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 3)
		b_n = b_n - (as->power * 5);

	/* Really penalize use of mana needed for final teleport */
	if (borg_class == CLASS_MAGE)
		penalty = 6;
	if (borg_class == CLASS_RANGER)
		penalty = 22;
	if (borg_class == CLASS_ROGUE)
		penalty = 20;
	if (borg_class == CLASS_PRIEST)
		penalty = 8;
	if (borg_class == CLASS_PALADIN)
		penalty = 20;
	if ((borg_skill[BI_MAXSP] > 30) &&
		 (borg_skill[BI_CURSP] - as->power < penalty))
		b_n = b_n - (as->power * 750);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the prayer */
	if (borg_spell(realm, book, what)) {
		/* Value */
		return (b_n);
	} else {
		borg_note(format("# Failed to cast spell (%d, %d, %d) when I wanted to",
							  realm, book, what));

		return (0);
	}
}

/*
 *  Simulate/Apply the optimal result of using a meteor swarm attack.
 *  The game code was broken, but I fixed it with my release.
 *  The game will fire 10-20 meteors up to 5 grids away.  Each meteor
 *  is a rad 2 ball effect.
 */
static int borg_attack_aux_spell_meteorswarm(int realm, int book, int what,
															int rad, int dam, int typ,
															bool inflate, int specific) {
	int b_n = 0;
	int penalty = 0;
	int y = c_y;
	int x = c_x;
	int i;

	borg_magic *as = &borg_magics[realm][book][what];

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISWEAK])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Require ability */
	if (!borg_spell_okay_fail(realm, book, what, 25))
		return (0);

	/* Meteor Swarm will fire 10-20 radius 3 or 4 balls to random locations that
	 * are randspread(-5, 5)
	 * from the player.  The borg will need to find some random spots and add up
	 * the damage.
	 */

	/** Simulate 15 meteors -- Pick a location */
	for (i = 0; i < 15; i++) {
		int count = 0;
		bool redo = FALSE;

		/* Pick a location */
		do {
			redo = FALSE;
			count++;
			if (count > 250)
				break;
			x = c_x - 5 + randint(10);
			y = c_y - 5 + randint(10);

			/* checks for loop */
			if (!in_bounds(y, x))
				redo = TRUE;
			else if (!(borg_grids[y][x].info & BORG_VIEW))
				redo = TRUE;
			else if (distance(c_y, c_x, y, x) > 6)
				redo = TRUE;
		} while (redo);

		/* Test this possible location--*/
		b_n += borg_launch_bolt_aux(y, x, rad, dam, typ, MAX_RANGE, inflate, 0,
											 specific, FALSE);
	}

	/* Penalize mana usage */
	b_n = b_n - as->power;

	/* Penalize use of reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 2)
		b_n = b_n - (as->power * 2);

	/* Penalize use of deep reserve mana */
	if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 3)
		b_n = b_n - (as->power * 5);

	/* Really penalize use of mana needed for final teleport */
	if (borg_class == CLASS_MAGE)
		penalty = 6;
	if (borg_class == CLASS_RANGER)
		penalty = 22;
	if (borg_class == CLASS_ROGUE)
		penalty = 20;
	if (borg_class == CLASS_PRIEST)
		penalty = 8;
	if (borg_class == CLASS_PALADIN)
		penalty = 20;
	if ((borg_skill[BI_MAXSP] > 30) &&
		 (borg_skill[BI_CURSP] - as->power < penalty))
		b_n = b_n - (as->power * 750);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the prayer */
	if (borg_spell(realm, book, what)) {
		/* Value */
		return (b_n);
	} else {
		borg_note(format("# Failed to cast spell (%d, %d, %d) when I wanted to",
							  realm, book, what));

		return (0);
	}
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 *
 * Take into account the failure rate of spells/objects/etc.  XXX XXX XXX
 */
static int borg_attack_aux_mind_bolt(int spell, int level, int rad, int dam,
												 int typ, bool inflate, int specific) {
	int b_n = -1;
	int penalty = 0;
	bool offset = TRUE;
	bool rzb = FALSE;

	borg_mind *as = &borg_minds[spell];

	/* No firing while confused */
	if (borg_skill[BI_ISCONFUSED])
		return (0);

	/* Paranoia */
	/* if (borg_simulate && specific == -1 && (rand_int(100) < 5)) return (0); */

	/* Require ability (right now) */
	if (!borg_mindcr_okay_fail(spell, level, (borg_fighting_unique ? 40 : 30)))
		return (0);

	/* Some spells are radius 0 ball effects */
	if ((spell == MIND_PULVERISE && rad == 0) || spell == MIND_PSYCHIC_DR ||
		 spell == MIND_NEURAL_BL) {
		offset = FALSE;
		rzb = TRUE;
	}

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific,
								  offset, rzb);

	/* weaklings need that spell, they dont get penalized */
	if (spell == MIND_NEURAL_BL && borg_skill[BI_MAXCLEVEL] <= 30 && b_n > 0) {
		if (borg_simulate)
			return (b_n + 5);
	}

	/* When not low level ... */
	if (borg_skill[BI_CLEVEL] > 35 && spell != MIND_PSYCHIC_DR) {
		/* Penalize mana usage */
		b_n = b_n - as->power;

		/* Penalize use of reserve mana */
		if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 2)
			b_n = b_n - (as->power * 2);

		/* Penalize use of deep reserve mana */
		if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 3)
			b_n = b_n - (as->power * 5);

		/* Really penalize use of mana needed for final teleport */
		penalty = 6;
		if ((borg_skill[BI_MAXSP] > 30) &&
			 (borg_skill[BI_CURSP] - as->power) < penalty)
			b_n = b_n - (as->power * 750);
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the spell */
	(void)borg_mindcr(spell, level);

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 *
 * Take into account the failure rate of spells/objects/etc.  XXX XXX XXX
 */
static int borg_attack_aux_mind_dispel(int spell, int level, int rad, int dam,
													int typ, bool inflate, int specific) {
	int b_n = -1;
	int penalty = 0;

	borg_mind *as = &borg_minds[spell];

	/* No firing while confused */
	if (borg_skill[BI_ISCONFUSED])
		return (0);

	/* Paranoia */
	/* if (borg_simulate && specific == -1 && (rand_int(100) < 5)) return (0); */

	/* Require ability (right now) */
	if (!borg_mindcr_okay_fail(spell, level, (borg_fighting_unique ? 40 : 30)))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_dispel(rad, dam, typ, MAX_RANGE, inflate, 0, specific);

	/* When not low level ... */
	if (borg_skill[BI_CLEVEL] > 35 && spell != MIND_PSYCHIC_DR) {
		/* Penalize mana usage */
		b_n = b_n - as->power;

		/* Penalize use of reserve mana */
		if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 2)
			b_n = b_n - (as->power * 2);

		/* Penalize use of deep reserve mana */
		if (borg_skill[BI_CURSP] - as->power < borg_skill[BI_MAXSP] / 3)
			b_n = b_n - (as->power * 5);

		/* Really penalize use of mana needed for final teleport */
		penalty = 6;
		if ((borg_skill[BI_MAXSP] > 30) &&
			 (borg_skill[BI_CURSP] - as->power) < penalty)
			b_n = b_n - (as->power * 750);
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the spell */
	(void)borg_mindcr(spell, level);

	/* Value */
	return (b_n);
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" staff
 * Which would be dispel evil, power, holiness.  Genocide handeled later.
 */
static int borg_attack_aux_staff_dispel(int sval, /*int rad,*/ int dam, int typ,
													 bool inflate, int specific) {
	int /*i, */ b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISWEAK])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* look for the staff */
	if (!borg_equips_staff_fail(sval))
		return (0);
	/*i = borg_slot(TV_STAFF, sval);*/

	/* Choose optimal location--radius defined as 10 */
	b_n =
		 borg_launch_dispel(MAX_SIGHT, dam, typ, MAX_RANGE, inflate, 0, specific);

	/* Big Penalize charge usage */
	b_n = b_n - 15;

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Cast the prayer */
	(void)borg_use_staff(sval);

	/* Value */
	return (b_n);
}

/* Draconian races breathe different types based on their level and class.
 * Early on,
 * its mostly GF_FIRE, but later it varies.
 * TO SHOULD, make this a common function with the main code base
 * In fact, we could add this as part of the class table
 */
static int borg_draco_type(void) {
	int Type = GF_FIRE;

	/* most of the time */
	if (borg_skill[BI_CLEVEL] < 35)
		return (Type);

	switch (p_ptr->pclass) {
	case CLASS_WARRIOR:
	case CLASS_RANGER:
		Type = GF_SHARDS;
		break;
	case CLASS_MAGE:
	case CLASS_WARRIOR_MAGE:
	case CLASS_HIGH_MAGE:
		Type = GF_DISENCHANT;
		break;
	case CLASS_CHAOS_WARRIOR:
	case CLASS_MONK:
	case CLASS_ORPHIC:
		Type = GF_CONFUSION;
		break;
	case CLASS_PRIEST:
	case CLASS_PALADIN:
		Type = GF_HOLY_FIRE;
		break;
	case CLASS_ROGUE:
		Type = GF_POIS;
		break;
	}
	return (Type);
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack rod
 */
static int borg_attack_aux_rod_bolt(int sval, int rad, int dam, int typ,
												bool inflate, int specific) {
	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Look for that rod */
	if (!borg_equips_rod(sval))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Zap the rod */
	(void)borg_zap_rod(sval);

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack rod
 */
static int borg_attack_aux_rod_unknown(int rad, int dam, int typ, bool inflate,
													int specific) {
	int b_n;
	int i;
	int b_i = -1;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Look for an un-id'd wand */
	for (i = 0; i < INVEN_PACK; i++) {
		if (borg_items[i].tval != TV_ROD)
			continue;

		/* known */
		if (borg_items[i].kind)
			continue;

		/* No charges */
		if (!borg_items[i].pval)
			continue;

		/* Tried, but effect not known */
		if (strstr(borg_items[i].desc, "tried"))
			continue;

		/* Select this wand */
		b_i = i;
	}

	/* None available */
	if (b_i < 0)
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Log the message */
	borg_note(format("# Zapping unknown rod '%s.'", borg_items[b_i].desc));

	/* Perform the action */
	borg_keypress('z');
	borg_keypress(I2A(b_i));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack wand
 */
static int borg_attack_aux_wand_bolt(int sval, int rad, int dam, int typ,
												 bool inflate, int specific) {
	int i;

	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Look for that wand */
	i = borg_slot(TV_WAND, sval);

	/* None available */
	if (i < 0)
		return (0);

	/* No charges */
	if (!borg_items[i].pval)
		return (0);

	/* Try to conserve the wand charges for the dungeon in order to maximize XP
	 */
	if ((borg_no_retreat >= 1 || goal_recalling) &&
		 borg_skill[BI_CLEVEL] <= 15 && borg_skill[BI_CDEPTH] == 0)
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Wands of wonder are used in last ditch efforts.  They behave
	 * randomly, so the best use of them is an emergency.  I have seen
	 * borgs die from hill orcs with fully charged wonder wands.  Odds
	 * are he could have taken the orcs with the wand.  So use them in
	 * an emergency after all the borg_caution() steps have failed.
	* But we do not use them as the 'strong-offense' defense.
	 */
	if (sval == SV_WAND_WONDER && inflate && !borg_munchkin_mode) {
		/* check the danger */
		if (b_n > 0 && borg_danger(c_y, c_x, 1, TRUE) >= avoidance) {
			/* make the wand appear deadly */
			b_n = 999;

			/* note the use of the wand in the emergency */
			borg_note(format("# Emergency use of a Wand of Wonder."));
		} else {
			b_n = 0;
		}
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Aim the wand */
	(void)borg_aim_wand(sval);

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of using an un-id'd wand
 */
static int borg_attack_aux_wand_bolt_unknown(int dam, int typ, bool inflate,
															int specific) {
	int i;
	int b_i = -1;
	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Look for an un-id'd wand */
	for (i = 0; i < INVEN_PACK; i++) {
		if (borg_items[i].tval != TV_WAND)
			continue;

		/* known */
		if (borg_items[i].kind)
			continue;

		/* No charges */
		if (!borg_items[i].pval)
			continue;

		/* Select this wand */
		b_i = i;
	}

	/* None available */
	if (b_i < 0)
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(0, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Log the message */
	borg_note(format("# Aiming unknown wand '%s.'", borg_items[b_i].desc));

	/* Perform the action */
	borg_keypress('a');
	borg_keypress(I2A(b_i));

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an attack artifact
 *
 */
static int borg_attack_aux_artifact(int art_name, int rad, int dam, int typ,
												bool inflate, int specific) {
	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && (rand_int(100) < 5))
		return (0);

	/* Look for that artifact and to see if it is charged */
	if (!borg_equips_artifact(art_name))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Activate the artifact */
	(void)borg_activate_artifact(art_name, FALSE);

	/* Use target, unless we dont need a target */
	if (art_name != ART_AMULET_RAPHAEL && art_name != ART_BAPHOMET &&
		 art_name == ART_ASMODAI) {
		borg_keypress('5');

		/* Set our shooting flag */
		successful_target = -1;
	}

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING a DRAGON ARMOUR
 *
 */
static int borg_attack_aux_dragon(int sval, int rad, int dam, int typ,
											 bool inflate, int specific) {
	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && (rand_int(100) < 5))
		return (0);

	/* Look for that scale mail and charged*/
	if (!borg_equips_dragon(sval))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Activate the scale mail */
	(void)borg_activate_dragon(sval);

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an equipped item
 *
 */
static int borg_attack_aux_equip(int tval, int sval, int rad, int dam, int typ,
											bool inflate, int specific) {
	int b_n;

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		return (0);

	/* Paranoia */
	if (borg_simulate && specific == -1 && (rand_int(100) < 5))
		return (0);

	/* Look for that item, and if charged*/
	if (!borg_equips_item(tval, sval))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Activate the scale mail */
	(void)borg_activate_item(tval, sval, FALSE);

	/* Use target */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Value */
	return (b_n);
}

/* Whirlwind--
 * Attacks adjacent monsters
 */
static int borg_attack_aux_nature_whirlwind(bool inflate /*, int specific*/) {
	int y = 0, x = 0;
	int dir;

	int dam = 0;

	borg_grid *ag;
	borg_kill *kill;

	if (!borg_spell_okay_fail(REALM_NATURE, 3, 1, 20) &&
		 !borg_equips_activation(ACT_WHIRLWIND, TRUE))
		return 0;

	/* Scan neighboring grids */
	for (dir = 0; dir <= 9; dir++) {
		y = c_y + ddy[dir];
		x = c_x + ddx[dir];

		ag = &borg_grids[y][x];
		kill = &borg_kills[ag->kill];

		/* is there a kill next to me */
		if (ag->kill) {
			/* Calculate "average" damage */
			dam += borg_thrust_damage_one(ag->kill, inflate);

			/* No good, if the adjacent monster is a friendly */
			if (!kill->killer)
				return (0);
		}
	}

	/* Return the damage for consideration */
	if (borg_simulate)
		return (dam);

	/* Cast the spell, perform the attack */
	if (borg_spell_fail(REALM_NATURE, 3, 1, 20) ||
		 borg_activate_activation(ACT_WHIRLWIND, FALSE))
		return (dam);

	/* */
	return (0);
}
/* Call Void--
 * Multiple Attacks, careful for walls
 */
static int borg_attack_aux_spell_callvoid(bool inflate) {
	int y = 0, x = 0;
	int dir;

	int dam = 0;

	borg_grid *ag;

	if (!borg_spell_okay_fail(REALM_CHAOS, 3, 7, 20))
		return 0;
	/* Scan neighboring grids */
	for (dir = 0; dir <= 9; dir++) {
		y = c_y + ddy[dir];
		x = c_x + ddx[dir];

		ag = &borg_grids[y][x];

		/* is there a wall next to me */
		if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM_SOLID) {
			return (0);

		} else {
			/* Calculate "average" damage */
			dam += borg_launch_bolt(10 + 2, 175, GF_SHARDS, MAX_RANGE, inflate, 0,
											-1, TRUE, FALSE);
			dam += borg_launch_bolt(10 + 2, 175, GF_MANA, MAX_RANGE, inflate, 0,
											-1, TRUE, FALSE);
			/* TODO SHOULD make sure call the void does not have a third new damage
			 * type*/
			/*dam += borg_launch_bolt(10 + 4, 175, GF_NUKE, MAX_RANGE, inflate, 0,
			 * -1, TRUE, FALSE);*/
		}
	}

	/* Return the damage for consideration */
	if (borg_simulate)
		return (dam);

	/* Cast the spell, perform the attack */
	if (borg_spell_fail(REALM_CHAOS, 3, 7, 20)) {
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
		return (dam);
	}

	/* */
	return (0);
}

/* Holcolleth -- sleep adjacent monsters */
static int borg_attack_aux_artifact_holcolleth(/*bool inflate*/) {
	int p1 = 0;
	int p2 = 0;
	int d = 0;

	/* Obtain initial danger */
	borg_sleep_spell = FALSE;
	p1 = borg_danger(c_y, c_x, 4, TRUE);

	if (!borg_equips_artifact(ART_SIMPLE) &&
		 !borg_equips_activation(ACT_SLEEP, TRUE))
		return (0);

	/* What effect is there? */
	borg_sleep_spell_ii = TRUE;
	p2 = borg_danger(c_y, c_x, 4, TRUE);
	borg_sleep_spell_ii = FALSE;

	/* value is d, enhance the value for rogues and rangers so that
	 * they can use their critical hits.
	 */
	d = (p1 - p2);

	/* Simulation */
	if (borg_simulate)
		return (d);

	/* Cast the spell */
	if (borg_activate_artifact(ART_SIMPLE, FALSE) ||
		 borg_activate_activation(ACT_SLEEP, FALSE)) {
		return (d);
	}

	return (0);
}

/*
 * Simulate/Apply the optimal result of making a racial attack.
 */
static int borg_attack_aux_racial_bolt(int race, int rad, int dam, int typ,
													int fail, bool inflate, int specific,
													int cost) {
	int b_n = -1;

	/* Cost of ability, affordable */

	/* Not if terribly weak */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 4 / 10)
		return (0);

	/* Check for ability most of the time */
	if (!borg_racial_check(race, fail))
		return (FALSE);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Reduce the damage by the Activation Failure % */
	b_n = b_n * (100 - fail) / 100;

	/* Reduce the value by the cost of mana */
	b_n = b_n - cost;

	/* Penalize the cost if mana is involved */
	if (borg_skill[BI_MAXSP] >= 100) {
		/* Minor penalty if at 40% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 7 / 10;
	} else
	/* Penalize the HP cost */
	{
		/* Minor penalty if at 50% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 5 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 3 / 10)
			b_n = b_n * 7 / 10;
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Note */
	borg_note(format("# Racial Attack -- for '%d'", b_n));

	/* Activate */
	borg_keypress('U');

	/* Racial is always 'a' */
	borg_keypress('a');

	/* Attack the previously targeted grid */
	borg_keypress('5');

	/* Set our shooting flag */
	successful_target = -1;

	/* Success */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of making a racial attack.
 */
static int borg_attack_aux_racial_dispel(int race, int rad, int dam, int typ,
													  int fail, bool inflate, int specific,
													  int cost) {
	int b_n = -1;

	/* Cost of ability, affordable */

	/* Not if terribly weak */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 4 / 10)
		return (0);

	/* Check for ability most of the time */
	if (!borg_racial_check(race, fail))
		return (FALSE);

	/* Choose optimal location */
	b_n = borg_launch_dispel(rad, dam, typ, MAX_RANGE, inflate, 0, specific);

	/* Reduce the damage by the Activation Failure % */
	b_n = b_n * (100 - fail) / 100;

	/* Reduce the value by the cost of mana */
	b_n = b_n - cost;

	/* Penalize the cost if mana is involved */
	if (borg_skill[BI_MAXSP] >= 100) {
		/* Minor penalty if at 40% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 7 / 10;
	} else
	/* Penalize the HP cost */
	{
		/* Minor penalty if at 50% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 5 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 3 / 10)
			b_n = b_n * 7 / 10;
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Note */
	borg_note(format("# Racial Attack -- for '%d'", b_n));

	/* Activate */
	borg_keypress('U');

	/* Racial is always 'a' */
	borg_keypress('a');

	/* Set our shooting flag */
	successful_target = -1;

	/* Success */
	return (b_n);
}

/*
 * Simulate/Apply the optimal result of making a Mutation attack.
 */
static int borg_attack_aux_mutation_bolt(int mutation, int rad, int dam,
													  int typ, bool inflate, bool dispel,
													  int specific, int cost) {
	int b_n = -1;

	/* Cost of ability, affordable */

	/* Not if terribly weak */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 4 / 10)
		return (0);

	/* Check for ability with a 20% fail rate */
	if (!borg_mutation(mutation, TRUE, 20, FALSE))
		return (0);

	/* Choose optimal location */
	if (dispel)
		b_n = borg_launch_dispel(rad, dam, typ, MAX_RANGE, inflate, 0, specific);
	else
		b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific,
									  TRUE, FALSE);

	/* Reduce the value by the cost of mana */
	b_n = b_n - cost;

	/* Penalize the cost if mana is involved */
	if (borg_skill[BI_MAXSP] >= 100) {
		/* Minor penalty if at 40% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% mana */
		if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 4 / 10)
			b_n = b_n * 7 / 10;
	} else
	/* Penalize the HP cost */
	{
		/* Minor penalty if at 50% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 5 / 10)
			b_n = b_n * 8 / 10;

		/* Greater penalty if at 30% health */
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 3 / 10)
			b_n = b_n * 7 / 10;
	}

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Note */
	borg_note(format("# Mutation Attack -- for '%d'", b_n));

	/* Activate */
	if (borg_mutation(mutation, FALSE, 20, FALSE)) {

		/* bolt or dispel? */
		if (dispel == FALSE) {
			/* Attack the previously targeted grid */
			borg_keypress('5');

			/* Set our shooting flag */
			successful_target = -1;
		}

		/* Success */
		return (b_n);
	}

	return (0);
}

/*
 * Simulate/Apply the optimal result of making a randart activation.
 */
static int borg_attack_aux_act_bolt(int activation, int rad, int dam, int typ,
												bool inflate, int specific) {
	int b_n = -1;

	/* Cost of ability, affordable */

	/* Check for ability with a 20% fail rate */
	if (!borg_equips_activation(activation, TRUE))
		return (0);

	/* Choose optimal location */
	b_n = borg_launch_bolt(rad, dam, typ, MAX_RANGE, inflate, 0, specific, TRUE,
								  FALSE);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Note */
	borg_note(format("# Randart Attack -- for '%d'", b_n));

	/* Activate */
	if (borg_activate_activation(activation, TRUE)) {

		/* Set our shooting flag */
		successful_target = -1;

		/* Success */
		return (b_n);
	}

	return (0);
}

/*
 * Simulate/Apply the optimal result of making a randart vamp activation.
 */
extern int borg_attack_aux_act_vamp(int activation, int dam, bool inflate,
												int specific) {
	int p /*, dir*/;

	int i, b_i = -1;
	int d, b_d = -1;
	/*int dd = 0;*/

	borg_grid *ag;

	borg_kill *kill;

	monster_race *r_ptr;

	/* not when confused or afraid */
	if (borg_skill[BI_ISCONFUSED])
		return (0);

	/* Check for ability */
	if (!borg_equips_activation(activation, TRUE))
		return (0);

	/* Examine possible destinations */
	for (i = 0; i < borg_temp_n; i++) {
		int x = borg_temp_x[i];
		int y = borg_temp_y[i];

		/* Attacking a specific monster? */
		if (specific >= 0) {
			x = borg_temp_x[specific];
			y = borg_temp_y[specific];
		}

		/* Require nearness and projectable */
		if (distance(c_y, c_x, y, x) > MAX_RANGE)
			continue;
		if (!borg_projectable(c_y, c_x, y, x, TRUE, TRUE))
			continue;

		/* Acquire grid */
		ag = &borg_grids[y][x];

		/* Obtain the monster */
		kill = &borg_kills[ag->kill];

		/* monster race */
		r_ptr = &r_info[kill->r_idx];

		/* Dont attack our buddies */
		if (kill->ally)
			continue;

		/* Can't really attack those who are invuln */
		if (kill->invulner)
			continue;

		/* Drain gives food */
		if (!monster_living(r_ptr))
			continue;

		/* Base Dam */
		d = dam;

		/* Bonus if down on HP */
		if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= dam) {
			d = dam + (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]);
		}

		/*
		 * Enhance the preceived damage on Uniques.  This way we target them
		 * Keep in mind that he should hit the uniques but if he has a
		 * x5 great bane of dragons, he will tend attack the dragon since the
		 * precieved (and actual) damage is higher.  But don't select
		 * the town uniques (maggot does no damage)
		 *
		 * These are copied from the thrust damage routine.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] >= 1)
			d += (d * 5);

		/* Hack -- ignore Maggot until later.  Player will chase Maggot
		 * down all accross the screen waking up all the monsters.  Then
		 * he is stuck in a comprimised situation.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] == 0) {
			d = d * 2 / 3;

			/* Dont hunt maggot until later */
			if (borg_skill[BI_CLEVEL] < 5)
				d = 0;
		}

		/* give a small bonus for whacking a breeder */
		if (r_ptr->flags2 & RF2_MULTIPLY)
			d = (d * 3 / 2);

		/* Enhance the preceived damgage to summoner in order to influence the
		 * choice of targets.
		 */
		if (kill->summoner || kill->questor)
			d += ((d * 3) / 2);

		/* Hack -- avoid waking most "hard" sleeping monsters */
		if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode &&
			 borg_skill[BI_CLEVEL] < 50) {
			/* Calculate danger */
			borg_full_damage = TRUE;
			p = borg_danger_aux(y, x, 1, ag->kill, TRUE, FALSE);
			borg_full_damage = FALSE;

			if (p > avoidance / 2)
				continue;
		}

		/* Hack -- ignore sleeping town monsters */
		if (!borg_skill[BI_CDEPTH] && !kill->awake && !borg_skill[BI_VAMPIRE])
			continue;

		/* Calculate "danger" to player */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 2, ag->kill, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Reduce "bonus" of partial kills */
		if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15)
			p = p / 10;

		/* Add the danger to the damage */
		d += p;

		/* Ignore lower damage */
		if ((b_i >= 0) && (d < b_d))
			continue;

		/* Save the info */
		b_i = i;
		b_d = d;
		if (specific >= 0)
			b_i = specific;
	}

	/* Nothing to attack */
	if (b_i < 0)
		return (0);

	/* Simulation */
	if (borg_simulate && !inflate)
		return (dam);
	if (borg_simulate)
		return (b_d);

	/* Save the location */
	g_x = borg_temp_x[b_i];
	g_y = borg_temp_y[b_i];

	ag = &borg_grids[g_y][g_x];
	kill = &borg_kills[ag->kill];

	/* Note */
	borg_note(format("# Targetting %s at (%d,%d) who has %d Hit Points.",
						  (r_name + r_info[kill->r_idx].name), g_y, g_x,
						  kill->power));
	borg_note(format("# Attacking with Activation Attack.  Value:'%d'", b_d));

	/* Get a target location for attacking */
	borg_target(g_y, g_x);
	if (borg_activate_activation(activation, TRUE)) {
		/* Set our shooting flag */
		successful_target = -1;
	}

	/* Success */
	return (b_d);
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" attack activation
 */
static int borg_attack_aux_act_dispel(int activation, int rad, int dam, int typ,
												  bool inflate, int specific) {
	int b_n;
	/*int penalty = 0;*/

	/* No firing while blind, confused, or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISWEAK])
		return (0);

	/* Require ability */
	if (!borg_equips_activation(activation, TRUE))
		return (0);

	/* Choose optimal location--*/
	b_n = borg_launch_dispel(rad, dam, typ, MAX_RANGE, inflate, 0, specific);

	/* Simulation */
	if (borg_simulate)
		return (b_n);

	/* Activate the randart */
	if (borg_activate_activation(activation, FALSE)) {
		/* Value */
		return (b_n);
	} else
		return (0);
}

/*
 * Simulate/Apply the optimal result of using the given "type" of attack
 */
static int borg_attack_aux(int what, bool inflate, int specific) {
	int dam = 0, chance, rad = 0;
	int fail = 0;
	int cost = 0;

	/* Analyze */
	switch (what) {
	/* Wait on grid for monster to approach me */
	case BF_REST:
		return (borg_attack_aux_rest());

	/* Physical attack */
	case BF_THRUST:
		return (borg_attack_aux_thrust(inflate, specific));

	/* Object attack */
	case BF_OBJECT:
		return (borg_attack_aux_object(inflate, specific));

	/* Missile attack */
	case BF_LAUNCH_NORMAL:
		return (borg_attack_aux_launch(inflate, specific));

	/* Missile attack */
	case BF_LAUNCH_SEEKER:
		return (borg_attack_aux_launch_seeker(inflate, specific));

	/* Missile attack */
	case BF_LAUNCH_EGO:
		return (borg_attack_aux_launch_ego(inflate, specific));

	/* Spell -- Call Light */
	case BF_LIFE_CALL_LIGHT:
		dam = (2 * borg_skill[BI_CLEVEL]) / 2;
		rad = (borg_skill[BI_CLEVEL] / 10) + 1;
		return (borg_attack_aux_spell_dispel(REALM_LIFE, 0, 4, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Spell -- Holy Orb */
	case BF_LIFE_HOLY_ORB:
		dam = ((3 * 3) + borg_skill[BI_CLEVEL] + (borg_skill[BI_CLEVEL] / 3));
		rad = 2;
		return (borg_attack_aux_spell_bolt(REALM_LIFE, 1, 4, rad, dam,
													  GF_HOLY_FIRE, inflate, specific));

	/* Spell -- Exorcism */
	case BF_LIFE_EXORCISM:
		dam = borg_skill[BI_CLEVEL];
		rad = MAX_SIGHT;
		return (borg_attack_aux_spell_dispel(
			 REALM_LIFE, 2, 0, rad, dam, GF_DISP_UNDEAD_DEMON, inflate, specific));

	/* Spell -- Disp Undead & Demon */
	case BF_LIFE_DISP_UNDEAD:
		dam = (borg_skill[BI_CLEVEL] * 3);
		rad = MAX_SIGHT;
		return (borg_attack_aux_spell_dispel(
			 REALM_LIFE, 2, 2, rad, dam, GF_DISP_UNDEAD_DEMON, inflate, specific));

	/* Spell -- Disp Evil */
	case BF_LIFE_DISP_EVIL:
		dam = (borg_skill[BI_CLEVEL] * 4);
		rad = MAX_SIGHT;
		return (borg_attack_aux_spell_dispel(REALM_LIFE, 2, 4, rad, dam,
														 GF_DISP_EVIL, inflate, specific));

	/* Holy Word also has heal effect and is considered in borg_heal */
	case BF_LIFE_HOLY_WORD:
		rad = MAX_SIGHT;
		if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 300)
		/*  force him to think the spell is more deadly to get him to
		 * cast it.  This will provide some healing for him.
		 */
		{
			dam = ((borg_skill[BI_CLEVEL] * 10));
			return (borg_attack_aux_spell_dispel(REALM_LIFE, 2, 6, rad, dam,
															 GF_DISP_EVIL, inflate, specific));
		} else /* If he is not wounded dont cast this, use Disp Evil instead. */
		{
			dam = ((borg_skill[BI_CLEVEL] * 3)) - 50;
			return (borg_attack_aux_spell_dispel(REALM_LIFE, 2, 6, rad, dam,
															 GF_DISP_EVIL, inflate, specific));
		}

	/* Spell -- Divine Intervention */
	case BF_LIFE_DIVINE_INT:
		dam = borg_skill[BI_CLEVEL] * 4;
		rad = MAX_SIGHT;
		/* if hurting, add bonus */
		if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 200)
			dam = (dam * 12) / 10;
		/* if no speedy, add bonus */
		if (!borg_speed)
			dam = (dam * 13) / 10;
		/* bonus for refreshing the speedy */
		if (borg_speed)
			dam = (dam * 11) / 10;
		return (borg_attack_aux_spell_dispel(REALM_LIFE, 3, 6, rad, dam,
														 GF_HOLY_FIRE, inflate, specific));

	/* Life -- Day of the Dove (Charm Monster) */
	case BF_LIFE_DAY_DOVE:
		dam = borg_skill[BI_CLEVEL] * 2;
		rad = MAX_SIGHT;
		return (borg_attack_aux_spell_dispel(REALM_LIFE, 2, 3, rad, dam, GF_CHARM,
														 inflate, specific));

	/* Spell -- Zap */
	case BF_ARCANE_ZAP:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) / 2;
		return (borg_attack_aux_spell_bolt(REALM_ARCANE, 0, 0, rad, dam, GF_ELEC,
													  inflate, specific));

	/* Emergency cast of Zap */
	case BF_ARCANE_ZAP_RESERVE:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) / 2;
		return (borg_attack_aux_spell_bolt_reserve(REALM_ARCANE, 0, 0, rad, dam,
																 GF_ELEC, inflate, specific));

	/* Spell -- Light Area */
	case BF_ARCANE_LAREA:
		dam = (2 * (borg_skill[BI_CLEVEL]) / 2) / 2;
		rad = borg_skill[BI_CLEVEL] / 10 + 1;
		return (borg_attack_aux_spell_dispel(REALM_ARCANE, 0, 5, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Spell -- Stone to Mud */
	case BF_ARCANE_STONEMUD:
		dam = 45;
		return (borg_attack_aux_spell_bolt(REALM_ARCANE, 2, 4, rad, dam,
													  GF_KILL_WALL, inflate, specific));

	/* Spell -- Light Beam */
	case BF_ARCANE_LBEAM:
		dam = 30;
		rad = -1;
		return (borg_attack_aux_spell_bolt(REALM_ARCANE, 2, 5, rad, dam,
													  GF_LITE_WEAK, inflate, specific));

	/* Spell -- Elem Ball */
	case BF_ARCANE_ELEM_BALL:
		dam = (75 + borg_skill[BI_CLEVEL]);
		rad = 2;
		return (borg_attack_aux_spell_bolt(REALM_ARCANE, 3, 4, rad, dam,
													  GF_ELEMENTS, inflate, specific));

	/* Spell -- Light area */
	case BF_SORC_LAREA:
		dam = (2 * (borg_skill[BI_CLEVEL]) / 2) / 2;
		rad = borg_skill[BI_CLEVEL] / 10 + 1;
		return (borg_attack_aux_spell_dispel(REALM_SORCERY, 0, 3, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Spell -- Confuse Monster */
	case BF_SORC_CONF_MON:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_SORCERY, 0, 4, rad, dam,
													  GF_OLD_CONF, inflate, specific));

	/* Spell -- Sleep I */
	case BF_SORC_SLEEP_I:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_SORCERY, 0, 6, rad, dam,
													  GF_OLD_SLEEP, inflate, specific));

	/* Spell -- Slow I */
	case BF_SORC_SLOW_MON:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_SORCERY, 1, 2, rad, dam,
													  GF_OLD_SLOW, inflate, specific));

	/* Spell -- Sleep III */
	case BF_SORC_SLEEP_III:
		rad = MAX_SIGHT;
		dam = 10;
		return (borg_attack_aux_spell_dispel(REALM_SORCERY, 1, 3, rad, dam,
														 GF_OLD_SLEEP, inflate, specific));

	/* Spell -- Stasis */
	case BF_SORC_STASIS:
		dam = 30;
		return (borg_attack_aux_spell_bolt(REALM_SORCERY, 3, 0, rad, dam,
													  GF_STASIS, inflate, specific));

	/* Spell -- Mind Blast */
	case BF_TRUMP_MINDBLAST:
		dam = 3 + ((borg_skill[BI_CLEVEL] - 1) / 5);
		return (borg_attack_aux_spell_bolt(REALM_TRUMP, 0, 1, rad, dam, GF_PSI,
													  inflate, specific));

	/* Spell -- Mind Blast Reserve*/
	case BF_TRUMP_MINDBLAST_RESERVE:
		dam = 3 + ((borg_skill[BI_CLEVEL] - 1) / 5);
		return (borg_attack_aux_spell_bolt_reserve(REALM_TRUMP, 0, 1, rad, dam,
																 GF_PSI, inflate, specific));

	/* Spell -- Day Light */
	case BF_NATURE_DAYL:
		rad = (borg_skill[BI_CLEVEL] / 10) + 1;
		dam = (borg_skill[BI_CLEVEL] / 2);
		return (borg_attack_aux_spell_dispel(REALM_NATURE, 0, 4, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Spell -- Animal Taming */
	case BF_NATURE_TAME:
		rad = 0;
		dam = borg_skill[BI_CLEVEL];
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 0, 5, rad, dam,
													  GF_CONTROL_ANIMAL, inflate, specific));

	/* Spell -- Stone to Mud */
	case BF_NATURE_STONEMUD:
		dam = (45);
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 1, 0, rad, dam,
													  GF_KILL_WALL, inflate, specific));

	/* Spell -- lightning bolt */
	case BF_NATURE_ELECBOLT:
		dam = 3 + ((borg_skill[BI_CLEVEL] - 5) / 4);
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 1, 1, rad, dam, GF_ELEC,
													  inflate, specific));

	/* Spell -- frost bolt */
	case BF_NATURE_FROSTBOLT:
		dam = 5 + ((borg_skill[BI_CLEVEL] - 5) / 4);
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 1, 3, rad, dam, GF_COLD,
													  inflate, specific));

	/* Spell -- Sunlight */
	case BF_NATURE_SUNL:
		dam = (6 * 8) / 2;
		rad = -1;
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 1, 4, rad, dam,
													  GF_LITE_WEAK, inflate, specific));

	/* Spell -- Entangle */
	case BF_NATURE_ENTANGLE:
		rad = MAX_SIGHT;
		dam = (10);
		return (borg_attack_aux_spell_dispel(REALM_NATURE, 1, 5, rad, dam,
														 GF_OLD_SLOW, inflate, specific));

	/* Whirlwind */
	case BF_NATURE_WHIRLWIND:
		return (borg_attack_aux_nature_whirlwind(inflate /*, specific*/));

	/* Blizzard */
	case BF_NATURE_BLIZZARD:
		dam = 70 + borg_skill[BI_CLEVEL];
		rad = (borg_skill[BI_CLEVEL] / 12) + 1;
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 3, 2, rad, dam,
													  GF_OLD_CONF, inflate, specific));

	/* Elec Storm */
	case BF_NATURE_ELECSTORM:
		dam = 90 + borg_skill[BI_CLEVEL];
		rad = (borg_skill[BI_CLEVEL] / 12) + 1;
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 3, 3, rad, dam, GF_ELEC,
													  inflate, specific));

	/* Whirlpool */
	case BF_NATURE_WHIRLPOOL:
		dam = 100 + borg_skill[BI_CLEVEL];
		rad = (borg_skill[BI_CLEVEL] / 12) + 1;
		return (borg_attack_aux_spell_bolt(REALM_NATURE, 3, 4, rad, dam, GF_WATER,
													  inflate, specific));

	/* Call Sunlight */
	case BF_NATURE_SUNLIGHT:
		dam = 150;
		rad = 8;
		if (borg_skill[BI_FEAR_LITE] && borg_skill[BI_CURHP] < 150)
			dam = 0;
		return (borg_attack_aux_spell_dispel(REALM_NATURE, 3, 5, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Natures Wrath */
	case BF_NATURE_NATWRATH:
		dam = ((borg_skill[BI_CLEVEL] + 100));
		rad = (borg_skill[BI_CLEVEL] / 12) + 1;
		return (borg_attack_aux_spell_dispel(REALM_NATURE, 3, 7, rad, dam,
														 GF_DISINTEGRATE, inflate, specific));

	/* Magic Missile */
	case BF_CHAOS_MMISSILE:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) * 2;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 0, rad, dam,
													  GF_MISSILE, inflate, specific));

	/* Magic Missile */
	case BF_CHAOS_MMISSILE_RESERVE:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) * 2;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 0, rad, dam,
													  GF_MISSILE, inflate, specific));

	/* Flash of Light */
	case BF_CHAOS_FLASHLIGHT:
		rad = (borg_skill[BI_CLEVEL] / 10) + 1;
		dam = (borg_skill[BI_CLEVEL] / 2);
		return (borg_attack_aux_spell_dispel(REALM_CHAOS, 0, 2, rad, dam,
														 GF_LITE_WEAK, inflate, specific));

	/* Mana Burst */
	case BF_CHAOS_MANABURST:
		rad = 2;
		dam = (8 + borg_skill[BI_CLEVEL] + (borg_skill[BI_CLEVEL] / 3));
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 4, rad, dam, GF_MANA,
													  inflate, specific));

	/* Fire Bolt */
	case BF_CHAOS_FIREBOLT:
		dam = (8 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 4;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 5, rad, dam, GF_FIRE,
													  inflate, specific));

	/* Fist of Force */
	case BF_CHAOS_FISTFORCE:
		dam = (8 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 4;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 6, rad, dam,
													  GF_DISINTEGRATE, inflate, specific));

	/* Chaos Bolt */
	case BF_CHAOS_CHAOSBOLT:
		dam = (10 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 4;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 1, 1, rad, dam, GF_CHAOS,
													  inflate, specific));

	/* Sonic Boom */
	case BF_CHAOS_SONICBOOM:
		dam = (borg_skill[BI_CLEVEL] + 45);
		rad = (borg_skill[BI_CLEVEL] / 10) + 2;
		return (borg_attack_aux_spell_dispel(REALM_CHAOS, 1, 2, rad, dam,
														 GF_SOUND, inflate, specific));

	/* Doom Bolt */
	case BF_CHAOS_DOOMBOLT:
		rad = -1;
		dam = (11 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 4;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 1, 3, rad, dam, GF_MANA,
													  inflate, specific));

	/* Fire ball */
	case BF_CHAOS_FIREBALL:
		rad = 2;
		dam = (borg_skill[BI_CLEVEL] + 55);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 1, 4, rad, dam, GF_FIRE,
													  inflate, specific));

	/* Invoke Lorgrus */
	case BF_CHAOS_INVOKELOGRUS:
		rad = (borg_skill[BI_CLEVEL] / 5);
		dam = (borg_skill[BI_CLEVEL] + 66);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 1, 7, rad, dam, GF_CHAOS,
													  inflate, specific));

	/* Polymorph */
	case BF_CHAOS_POLYMORPH:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 2, 0, rad, dam,
													  GF_OLD_POLY, inflate, specific));

	/* Chain Lightning */
	case BF_CHAOS_CHAINLIGHT:
		rad = 13;
		dam = (5 + (borg_skill[BI_CLEVEL] / 10)) * 4;
		return (borg_attack_aux_spell_dispel(REALM_CHAOS, 2, 1, rad, dam, GF_ELEC,
														 inflate, specific));

	/* Disintegration */
	case BF_CHAOS_DISINTEG:
		rad = 3;
		dam = (borg_skill[BI_CLEVEL] + 80);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 2, 3, rad, dam,
													  GF_DISINTEGRATE, inflate, specific));

	/* Gravity */
	case BF_CHAOS_GRAVITY:
		rad = -1;
		dam = (9 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 4;
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 3, 0, rad, dam,
													  GF_GRAVITY, inflate, specific));

	/* Meteor Swarm */
	case BF_CHAOS_METEORSWARM:
		rad = 2;
		dam = (borg_skill[BI_CLEVEL] + 65);
		return (borg_attack_aux_spell_meteorswarm(REALM_CHAOS, 3, 1, rad, dam,
																GF_METEOR, inflate, specific));

	/* Flamestrike */
	case BF_CHAOS_FLAMESTRIKE:
		rad = 8;
		dam = 150 + (borg_skill[BI_CLEVEL] * 2);
		return (borg_attack_aux_spell_dispel(REALM_CHAOS, 3, 2, rad, dam, GF_FIRE,
														 inflate, specific));

	/* Rocket */
	case BF_CHAOS_ROCKET:
		rad = 2;
		dam = 120 + (borg_skill[BI_CLEVEL]);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 3, 4, rad, dam, GF_SHARDS,
													  inflate, specific));

	/* Mana Storm */
	case BF_CHAOS_MANASTORM:
		rad = 4;
		dam = 300 + (borg_skill[BI_CLEVEL] * 2);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 3, 5, rad, dam, GF_MANA,
													  inflate, specific));

	/* Breath Logrus */
	case BF_CHAOS_BRLOGRUS:
		rad = 2;
		dam = (borg_skill[BI_CURHP]);
		return (borg_attack_aux_spell_bolt(REALM_CHAOS, 3, 6, rad, dam, GF_CHAOS,
													  inflate, specific));

	/* Call Void */
	case BF_CHAOS_CALLVOID:
		return (borg_attack_aux_spell_callvoid(inflate));

	/* Malediction */
	case BF_DEATH_MALEDICTION:
		rad = 0;
		dam = ((3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) * 3) / 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 1, rad, dam,
													  GF_HELL_FIRE, inflate, specific));

	/* Malediction RESERVE*/
	case BF_DEATH_MALEDICTION_RESERVE:
		rad = 0;
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5)) / 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 1, rad, dam,
													  GF_HELL_FIRE, inflate, specific));

	/* Poison Ball */
	case BF_DEATH_STINKCLOUD:
		rad = 2;
		dam = (borg_skill[BI_CLEVEL] / 2);
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 3, rad, dam, GF_POIS,
													  inflate, specific));

	/* Black Sleep */
	case BF_DEATH_SLEEP_I:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 4, rad, dam,
													  GF_OLD_SLEEP, inflate, specific));

	/* Horrify */
	case BF_DEATH_HORRIFY:
		dam = 10;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 6, rad, dam,
													  GF_TURN_ALL, inflate, specific));

	/* Enslave Undead */
	case BF_DEATH_ENSLAVE_UNDEAD:
		dam = borg_skill[BI_CLEVEL];
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 0, 7, rad, dam,
													  GF_CONTROL_UNDEAD, inflate, specific));

	/* Entropy */
	case BF_DEATH_ENTROPY:
		rad = 2;
		dam = 10 + borg_skill[BI_CLEVEL] + ((borg_skill[BI_CLEVEL] / 4));
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 1, 0, rad, dam,
													  GF_OLD_DRAIN, inflate, specific));

	/* Nether Bolt */
	case BF_DEATH_NETHERBOLT:
		rad = 0;
		dam = ((6 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 8) / 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 1, 1, rad, dam, GF_NETHER,
													  inflate, specific));

	/* Terror */
	case BF_DEATH_TERROR:
		dam = borg_skill[BI_CLEVEL] + 30;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 1, 2, rad, dam,
													  GF_TURN_ALL, inflate, specific));

	/* Vamp Drain */
	case BF_DEATH_VAMPDRAIN:
		dam = (borg_skill[BI_CLEVEL] + (borg_skill[BI_CLEVEL] / 2 *
												  (MAX(1, borg_skill[BI_CLEVEL] / 10))));
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 7 / 10)
			dam *= 2;
		if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
			dam *= 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 1, 3, rad, dam,
													  GF_OLD_DRAIN, inflate, specific));

	/* Dispel Good */
	case BF_DEATH_DISPELGOOD:
		rad = MAX_SIGHT;
		dam = (borg_skill[BI_CLEVEL] * 4) / 2;
		return (borg_attack_aux_spell_dispel(REALM_DEATH, 1, 5, rad, dam,
														 GF_DISP_GOOD, inflate, specific));

	/* Dark Bolt */
	case BF_DEATH_DARKBOLT:
		dam = ((4 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 8) / 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 2, 2, rad, dam, GF_DARK,
													  inflate, specific));

	/* Vampirism True, this one is very nice since it gives us 300 HP */
	case BF_DEATH_VAMPIRISM:
		dam = 300;
		if ((borg_class == CLASS_PALADIN || borg_class == CLASS_RANGER ||
			  borg_class == CLASS_ROGUE) &&
			 (borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] - 100))
			dam = 0;
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] - 200)
			dam *= 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 2, 4, rad, dam,
													  GF_OLD_DRAIN, inflate, specific));

	/* DarknessStorm */
	case BF_DEATH_DARKNESS:
		dam = 120;
		rad = 4;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 2, 6, rad, dam, GF_DARK,
													  inflate, specific));

	/* Death Ray */
	case BF_DEATH_DEATHRAY:
		dam = borg_skill[BI_CLEVEL];
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 3, 0, rad, dam,
													  GF_DEATH_RAY, inflate, specific));

	/* Word of Death */
	case BF_DEATH_WORDOFDEATH:
		rad = MAX_SIGHT;
		dam = (3 * borg_skill[BI_CLEVEL]) / 2;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 3, 3, rad, dam,
													  GF_DISP_LIVING, inflate, specific));

	/* Evocation */
	case BF_DEATH_EVOCATION:
		dam = borg_skill[BI_CLEVEL] * 4;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 3, 4, rad, dam,
													  GF_DISP_ALL, inflate, specific));

	/* HellFire */
	case BF_DEATH_HELLFIRE:
		dam = 666;
		if (borg_skill[BI_CURHP] < 200)
			dam = 0;
		return (borg_attack_aux_spell_bolt(REALM_DEATH, 3, 5, rad, dam,
													  GF_HELL_FIRE, inflate, specific));

	/* Mind-- Neural Blast */
	case BF_MIND_NEURAL:
		dam = 3 +
				((borg_skill[BI_CLEVEL] - 1) / 4) *
					 (3 + (borg_skill[BI_CLEVEL] / 15)) / 2;
		rad = -1;
		return (borg_attack_aux_mind_bolt(MIND_NEURAL_BL, 2, rad, dam, GF_PSI,
													 inflate, specific));

	/* Mind -- Domination */
	case BF_MIND_DOMINATION:
		if (borg_skill[BI_CLEVEL] < 30) {
			rad = 1;
			dam = borg_skill[BI_CLEVEL];
			return (borg_attack_aux_mind_bolt(MIND_DOMINATION, 9, rad, dam,
														 GF_DOMINATION, inflate, specific));
		} else {
			rad = 10;
			dam = borg_skill[BI_CLEVEL] * 2;
			return (borg_attack_aux_mind_bolt(MIND_DOMINATION, 30, rad, dam,
														 GF_CHARM, inflate, specific));
		}
		break;

	/* Mind-- Pulverise */
	case BF_MIND_PULVERISE:
		dam = ((8 + ((borg_skill[BI_CLEVEL] - 5) / 4)) * 8) / 2;
		rad = (borg_skill[BI_CLEVEL] > 20 ? (borg_skill[BI_CLEVEL] - 20) / 8 + 1
													 : 0);
		return (borg_attack_aux_mind_bolt(MIND_PULVERISE, 11, rad, dam, GF_SOUND,
													 inflate, specific));

	/* Mind-- Mind Wave */
	case BF_MIND_MINDWAVE:
		if (borg_skill[BI_CLEVEL] < 25) {
			dam = (borg_skill[BI_CLEVEL] * 3) / 2;
			rad = (2 + borg_skill[BI_CLEVEL] / 10);
		} else {
			dam = borg_skill[BI_CLEVEL] * ((borg_skill[BI_CLEVEL] - 5) / 10 + 1);
			rad = MAX_SIGHT;
		}
		return (borg_attack_aux_mind_dispel(MIND_MIND_WAVE, 18, rad, dam, GF_PSI,
														inflate, specific));

	/* Mind-- Psychic Drain */
	case BF_MIND_PSYCH_DRAIN:
		dam = ((borg_skill[BI_CLEVEL] / 2) * 6) / 2;
		rad = 0;
		return (borg_attack_aux_mind_bolt(MIND_PSYCHIC_DR, 25, rad, dam,
													 GF_PSI_DRAIN, inflate, specific));

	/* Mind-- Telekinetic Wave */
	case BF_MIND_TELE_WAVE:
		if (borg_skill[BI_CLEVEL] < 40) {
			dam = (borg_skill[BI_CLEVEL] * 3);
			rad = 2 + (borg_skill[BI_CLEVEL] / 10);
		} else {
			dam = borg_skill[BI_CLEVEL] * 4;
			rad = 2 + (borg_skill[BI_CLEVEL] / 10);
		}
		return (borg_attack_aux_mind_dispel(MIND_TELE_WAVE, 28, rad, dam,
														GF_TELEKINESIS, inflate, specific));

	/* ROD -- slow monster */
	case BF_ROD_SLOW_MONSTER:
		dam = 10;
		return (borg_attack_aux_rod_bolt(SV_ROD_SLOW_MONSTER, rad, dam,
													GF_OLD_SLOW, inflate, specific));

	/* ROD -- sleep monster */
	case BF_ROD_SLEEP_MONSTER:
		dam = 10;
		return (borg_attack_aux_rod_bolt(SV_ROD_SLEEP_MONSTER, rad, dam,
													GF_OLD_SLEEP, inflate, specific));

	/* Rod -- elec bolt */
	case BF_ROD_ELEC_BOLT:
		dam = 3 * (8 + 1) / 2;
		return (borg_attack_aux_rod_bolt(SV_ROD_ELEC_BOLT, rad, dam, GF_ELEC,
													inflate, specific));

	/* Rod -- cold bolt */
	case BF_ROD_COLD_BOLT:
		dam = 5 * (8 + 1) / 2;
		return (borg_attack_aux_rod_bolt(SV_ROD_COLD_BOLT, rad, dam, GF_COLD,
													inflate, specific));

	/* Rod -- acid bolt */
	case BF_ROD_ACID_BOLT:
		dam = 6 * (8 + 1) / 2;
		return (borg_attack_aux_rod_bolt(SV_ROD_ACID_BOLT, rad, dam, GF_ACID,
													inflate, specific));

	/* Rod -- fire bolt */
	case BF_ROD_FIRE_BOLT:
		dam = 8 * (8 + 1) / 2;
		return (borg_attack_aux_rod_bolt(SV_ROD_FIRE_BOLT, rad, dam, GF_FIRE,
													inflate, specific));

	/* Spell -- light beam */
	case BF_ROD_LITE_BEAM:
		rad = -1;
		dam = (6 * (8 + 1) / 2);
		return (borg_attack_aux_rod_bolt(SV_ROD_LITE, rad, dam, GF_LITE_WEAK,
													inflate, specific));

	/* Spell -- drain life */
	case BF_ROD_DRAIN_LIFE:
		dam = (75);
		return (borg_attack_aux_rod_bolt(SV_ROD_DRAIN_LIFE, rad, dam,
													GF_OLD_DRAIN, inflate, specific));

	/* Rod -- elec ball */
	case BF_ROD_ELEC_BALL:
		rad = 2;
		dam = 32;
		return (borg_attack_aux_rod_bolt(SV_ROD_ELEC_BALL, rad, dam, GF_ELEC,
													inflate, specific));

	/* Rod -- acid ball */
	case BF_ROD_COLD_BALL:
		rad = 2;
		dam = 48;
		return (borg_attack_aux_rod_bolt(SV_ROD_COLD_BALL, rad, dam, GF_COLD,
													inflate, specific));

	/* Rod -- acid ball */
	case BF_ROD_ACID_BALL:
		rad = 2;
		dam = 60;
		return (borg_attack_aux_rod_bolt(SV_ROD_ACID_BALL, rad, dam, GF_ACID,
													inflate, specific));

	/* Rod -- fire ball */
	case BF_ROD_FIRE_BALL:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_rod_bolt(SV_ROD_FIRE_BALL, rad, dam, GF_FIRE,
													inflate, specific));

	/* Rod -- Pesticide
	case BF_ROD_PESTICIDE:
		rad = 3;
		dam = 8;
		return (borg_attack_aux_rod_bolt(SV_ROD_PESTICIDE, rad, dam, GF_POIS,
													inflate, specific));
	*/
	/* Rod -- Unknown */
	case BF_ROD_UNKNOWN:
		rad = -1;
		dam = 50;
		return (
			 borg_attack_aux_rod_unknown(rad, dam, GF_MISSILE, inflate, specific));

	/* Wand -- unknown wands */
	case BF_WAND_UNKNOWN:
		dam = 5;
		return (borg_attack_aux_wand_bolt_unknown(dam, GF_MISSILE, inflate,
																specific));

	/* Wand -- Rocket
	case BF_WAND_ROCKETS:
		dam = 250;
		return (borg_attack_aux_wand_bolt(SV_WAND_ROCKETS, rad, dam, GF_ROCKET,
										 inflate, specific));
  */

	/* Wand -- magic missile */
	case BF_WAND_MAGIC_MISSILE:
		dam = 2 * (6 + 1) / 2;
		if (borg_skill[BI_CLEVEL] < 5)
			dam *= 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_MAGIC_MISSILE, rad, dam,
													 GF_MISSILE, inflate, specific));

	/* Wand -- slow monster */
	case BF_WAND_SLOW_MONSTER:
		dam = 10;
		return (borg_attack_aux_wand_bolt(SV_WAND_SLOW_MONSTER, rad, dam,
													 GF_OLD_SLOW, inflate, specific));

	/* Wand -- sleep monster */
	case BF_WAND_SLEEP_MONSTER:
		dam = 10;
		return (borg_attack_aux_wand_bolt(SV_WAND_SLEEP_MONSTER, rad, dam,
													 GF_OLD_SLEEP, inflate, specific));

	/* Wand -- fear monster */
	case BF_WAND_FEAR_MONSTER:
		dam = 2 * (6 + 1) / 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_FEAR_MONSTER, rad, dam,
													 GF_TURN_ALL, inflate, specific));

	/* Wand -- conf monster */
	case BF_WAND_CONFUSE_MONSTER:
		dam = 2 * (6 + 1) / 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_CONFUSE_MONSTER, rad, dam,
													 GF_OLD_CONF, inflate, specific));

	/* Wand -- Charm Monsters */
	case BF_WAND_CHARM_MONSTER:
		dam = 45;
		rad = 0;
		return (borg_attack_aux_wand_bolt(SV_WAND_CHARM_MONSTER, rad, dam,
													 GF_CHARM, inflate, specific));

	/* Wand -- cold bolt */
	case BF_WAND_COLD_BOLT:
		dam = 3 * (8 + 1) / 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_COLD_BOLT, rad, dam, GF_COLD,
													 inflate, specific));

	/* Wand -- acid bolt */
	case BF_WAND_ACID_BOLT:
		dam = 5 * (8 + 1) / 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_ACID_BOLT, rad, dam, GF_ACID,
													 inflate, specific));

	/* Wand -- fire bolt */
	case BF_WAND_FIRE_BOLT:
		dam = 6 * (8 + 1) / 2;
		return (borg_attack_aux_wand_bolt(SV_WAND_FIRE_BOLT, rad, dam, GF_FIRE,
													 inflate, specific));

	/* Spell -- light beam */
	case BF_WAND_LITE_BEAM:
		rad = -1;
		dam = (6 * (8 + 1) / 2);
		return (borg_attack_aux_wand_bolt(SV_WAND_LITE, rad, dam, GF_LITE_WEAK,
													 inflate, specific));

	/* Wand -- stinking cloud */
	case BF_WAND_STINKING_CLOUD:
		rad = 2;
		dam = 12;
		return (borg_attack_aux_wand_bolt(SV_WAND_STINKING_CLOUD, rad, dam,
													 GF_POIS, inflate, specific));

	/* Wand -- elec ball */
	case BF_WAND_ELEC_BALL:
		rad = 2;
		dam = 32;
		return (borg_attack_aux_wand_bolt(SV_WAND_ELEC_BALL, rad, dam, GF_ELEC,
													 inflate, specific));

	/* Wand -- acid ball */
	case BF_WAND_COLD_BALL:
		rad = 2;
		dam = 48;
		return (borg_attack_aux_wand_bolt(SV_WAND_COLD_BALL, rad, dam, GF_COLD,
													 inflate, specific));

	/* Wand -- acid ball */
	case BF_WAND_ACID_BALL:
		rad = 2;
		dam = 60;
		return (borg_attack_aux_wand_bolt(SV_WAND_ACID_BALL, rad, dam, GF_ACID,
													 inflate, specific));

	/* Wand -- fire ball */
	case BF_WAND_FIRE_BALL:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_wand_bolt(SV_WAND_FIRE_BALL, rad, dam, GF_FIRE,
													 inflate, specific));

	/* Wand -- dragon cold */
	case BF_WAND_DRAGON_COLD:
		rad = 3;
		dam = 80;
		return (borg_attack_aux_wand_bolt(SV_WAND_DRAGON_COLD, rad, dam, GF_COLD,
													 inflate, specific));

	/* Wand -- dragon fire */
	case BF_WAND_DRAGON_FIRE:
		rad = 3;
		dam = 100;
		return (borg_attack_aux_wand_bolt(SV_WAND_DRAGON_FIRE, rad, dam, GF_FIRE,
													 inflate, specific));

	/* Wand -- annihilation */
	case BF_WAND_ANNIHILATION:
		dam = 125;
		return (borg_attack_aux_wand_bolt(SV_WAND_ANNIHILATION, rad, dam,
													 GF_OLD_DRAIN, inflate, specific));

	/* Wand -- drain life */
	case BF_WAND_DRAIN_LIFE:
		dam = 75;
		return (borg_attack_aux_wand_bolt(SV_WAND_DRAIN_LIFE, rad, dam,
													 GF_OLD_DRAIN, inflate, specific));

	/* Wand -- wand of wonder */
	case BF_WAND_WONDER:
		dam = 35;
		/* do not attempt this if using the "strong offense" defense since we cant
		 * predict the spell.*/
		if (inflate && !borg_munchkin_mode)
			return (0);
		return (borg_attack_aux_wand_bolt(SV_WAND_WONDER, rad, dam, GF_MISSILE,
													 inflate, specific));

	/* Staff -- Sleep Monsters */
	case BF_STAFF_SLEEP_MONSTERS:
		dam = 60;
		rad = MAX_SIGHT;
		return (borg_attack_aux_staff_dispel(SV_STAFF_SLEEP_MONSTERS,
														 /*rad,*/ dam, GF_OLD_SLEEP, inflate,
														 specific));

	/* Staff -- Slow Monsters */
	case BF_STAFF_SLOW_MONSTERS:
		dam = 60;
		rad = MAX_SIGHT;
		return (borg_attack_aux_staff_dispel(SV_STAFF_SLOW_MONSTERS, /*rad,*/ dam,
														 GF_OLD_SLOW, inflate, specific));

	/* Staff -- Dispel Evil */
	case BF_STAFF_DISPEL_EVIL:
		dam = 60;
		rad = MAX_SIGHT;
		return (borg_attack_aux_staff_dispel(SV_STAFF_DISPEL_EVIL, /*rad,*/ dam,
														 GF_DISP_EVIL, inflate, specific));

	/* Staff -- Power */
	case BF_STAFF_POWER:
		dam = 120;
		rad = MAX_SIGHT;
		return (borg_attack_aux_staff_dispel(SV_STAFF_POWER, /*rad,*/ dam,
														 GF_TURN_ALL, inflate, specific));

	/* Staff -- holiness */
	case BF_STAFF_HOLINESS:
		rad = MAX_SIGHT;
		if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
			dam = 500;
		else
			dam = 120;
		return (borg_attack_aux_staff_dispel(SV_STAFF_HOLINESS, /*rad,*/ dam,
														 GF_DISP_EVIL, inflate, specific));

	/* Artifact -- Narthanc- fire bolt 9d8*/
	case BF_ART_DAGGER_INFERNO:
		rad = 0;
		dam = (9 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_DAGGER_INFERNO, rad, dam, GF_FIRE,
													inflate, specific));

	/* Artifact -- Nimthanc- frost bolt 6d8*/
	case BF_ART_COCYTUS:
		rad = 0;
		dam = (6 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_COCYTUS, rad, dam, GF_COLD, inflate,
													specific));

	/* Artifact -- Dethanc- electric bolt 4d8*/
	case BF_ART_DAGGER_FURCIFER:
		rad = 0;
		dam = (4 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_DAGGER_FURCIFER, rad, dam, GF_ELEC,
													inflate, specific));

	/* Artifact -- Rilia- poison gas 12*/
	case BF_ART_KINSLAYER:
		rad = 2;
		dam = 12;
		return (borg_attack_aux_artifact(ART_KINSLAYER, rad, dam, GF_POIS,
													inflate, specific));

	/* Artifact -- Belangil- frost ball 48*/
	case BF_ART_FROST:
		rad = 2;
		dam = 48;
		return (borg_attack_aux_artifact(ART_FROST, rad, dam, GF_COLD, inflate,
													specific));

	/* Artifact -- Arunruth- frost bolt 12d8*/

	/* Artifact -- Ringil- frost ball 100*/
	case BF_ART_MICHAEL:
		rad = 2;
		dam = 100;
		return (borg_attack_aux_artifact(ART_MICHAEL, rad, dam, GF_COLD, inflate,
													specific));

	/* Artifact -- Anduril- fire ball 72*/
	case BF_ART_RASHAVERAK:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_artifact(ART_RASHAVERAK, rad, dam, GF_FIRE,
													inflate, specific));

	/* Artifact -- Theoden- drain Life 120*/
	case BF_ART_ELIGOR:
		rad = 0;
		dam = 120;
		return (borg_attack_aux_artifact(ART_ELIGOR, rad, dam, GF_OLD_DRAIN,
													inflate, specific));

	/* Artifact -- Aeglos- frost ball 100
	case BF_ART_AEGLOS:
		rad = 2;
		dam = 100;
		return (borg_attack_aux_artifact(ART_AEGLOS, INVEN_WIELD, rad, dam,
													GF_COLD, inflate, specific));
	*/
	/* Artifact -- Totila- confustion */
	case BF_ART_PHENEX:
		rad = 0;
		dam = 10;
		return (borg_attack_aux_artifact(ART_PHENEX, rad, dam, GF_OLD_CONF,
													inflate, specific));

	/* Artifact -- Holcolleth -- sleep ii and sanctuary */
	case BF_ART_SIMPLE:
		dam = 10;
		return (borg_attack_aux_artifact_holcolleth(/*inflate*/));

	/* Artifact -- Firestar- fire ball 72 */
	case BF_ART_MORNINGSTAR:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_artifact(ART_MORNINGSTAR, rad, dam, GF_FIRE,
													inflate, specific));

	/* Artifact -- TURMIL- drain life 90 */
	case BF_ART_JUSTICE:
		rad = 0;
		dam = 90;
		return (borg_attack_aux_artifact(ART_JUSTICE, rad, dam, GF_OLD_DRAIN,
													inflate, specific));

	/* Artifact -- Razorback- spikes 150 */
	case BF_ART_BAPHOMET:
		rad = 2;
		dam = 150;
		return (borg_attack_aux_artifact(ART_BAPHOMET, rad, dam, GF_MISSILE,
													inflate, specific));

	/* Artifact -- Razorback- spikes 150 */
	case BF_ART_ASMODAI:
		rad = 2;
		dam = 150;
		return (borg_attack_aux_artifact(ART_ASMODAI, rad, dam, GF_MISSILE,
													inflate, specific));

	/* Artifact -- Cammithrim- Magic Missile 2d6 */
	case BF_ART_LIGHT:
		rad = 0;
		dam = (2 * (6 + 1) / 2);
		return (borg_attack_aux_artifact(ART_LIGHT, rad, dam, GF_MISSILE, inflate,
													specific));

	/* Artifact -- Paurhach- fire bolt 9d8 */
	case BF_ART_IRONFIST:
		rad = 0;
		dam = (9 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_IRONFIST, rad, dam, GF_FIRE, inflate,
													specific));

	/* Artifact -- Paurnimmen- frost bolt 6d8 */
	case BF_ART_GHOULS:
		rad = 0;
		dam = (4 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_GHOULS, rad, dam, GF_ELEC, inflate,
													specific));

	/* Artifact -- Pauraegen- lightning bolt 4d8 */
	case BF_ART_FURFICER:
		rad = 0;
		dam = (4 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_FURFICER, rad, dam, GF_ELEC, inflate,
													specific));

	/* Artifact -- PaurNEN- ACID bolt 5d8 */
	case BF_ART_DEAD:
		rad = 0;
		dam = (5 * (8 + 1) / 2);
		return (borg_attack_aux_artifact(ART_DEAD, rad, dam, GF_ACID, inflate,
													specific));

	/* Artifact -- FINGOLFIN- MISSILE 150
	case BF_ART_FINGOLFIN:
		rad = 0;
		dam = 150;
		return (borg_attack_aux_artifact(ART_FINGOLFIN, INVEN_HANDS, rad, dam,
													GF_MISSILE, inflate, specific));
	*/

	/* Artifact -- INGWE- DISPEL EVIL X5 */
	case BF_ART_AMULET_RAPHAEL:
		rad = 10;
		dam = (10 + (borg_skill[BI_CLEVEL] * 5) / 2);
		return (borg_attack_aux_artifact(ART_AMULET_RAPHAEL, rad, dam,
													GF_DISP_EVIL, inflate, specific));

	/* Artifact -- INGWE- DISPEL EVIL X5 */
	case BF_ART_AMULET_MICHAEL:
		rad = 10;
		dam = (10 + (borg_skill[BI_CLEVEL] * 5) / 2);
		return (borg_attack_aux_artifact(ART_AMULET_MICHAEL, rad, dam,
													GF_DISP_EVIL, inflate, specific));

	/* Artifact -- NARYA- FIRE BALL 120 */
	case BF_ART_RING_RAPHAEL:
		rad = 2;
		dam = 120;
		return (borg_attack_aux_artifact(ART_RING_RAPHAEL, rad, dam, GF_FIRE,
													inflate, specific));

	/* Artifact -- NENYA- COLD BALL 200 */
	case BF_ART_RING_MICHAEL:
		rad = 2;
		dam = 200;
		return (borg_attack_aux_artifact(ART_RING_MICHAEL, rad, dam, GF_COLD,
													inflate, specific));

	/* Artifact -- VILYA- ELEC BALL 250 */
	case BF_ART_EMMANUEL:
		rad = 2;
		dam = 250;
		return (borg_attack_aux_artifact(ART_EMMANUEL, rad, dam, GF_ELEC, inflate,
													specific));

	/* Hack -- Dragon Scale Mail can be activated as well */
	case BF_DRAGON_BLUE:
		rad = 2;
		dam = 100;
		return (borg_attack_aux_dragon(SV_DRAGON_BLUE, rad, dam, GF_ELEC, inflate,
												 specific));

	case BF_DRAGON_WHITE:
		rad = 2;
		dam = 110;
		return (borg_attack_aux_dragon(SV_DRAGON_WHITE, rad, dam, GF_COLD,
												 inflate, specific));

	case BF_DRAGON_BLACK:
		rad = 2;
		dam = 130;
		return (borg_attack_aux_dragon(SV_DRAGON_BLACK, rad, dam, GF_ACID,
												 inflate, specific));

	case BF_DRAGON_GREEN:
		rad = 2;
		dam = 150;
		return (borg_attack_aux_dragon(SV_DRAGON_GREEN, rad, dam, GF_POIS,
												 inflate, specific));

	case BF_DRAGON_RED:
		rad = 2;
		dam = 200;
		return (borg_attack_aux_dragon(SV_DRAGON_RED, rad, dam, GF_FIRE, inflate,
												 specific));

	case BF_DRAGON_MULTIHUED:
		chance = rand_int(5);
		rad = 2;
		dam = 200;
		return (borg_attack_aux_dragon(
			 SV_DRAGON_MULTIHUED, rad, dam,
			 (((chance == 1)
					 ? GF_ELEC
					 : ((chance == 2)
							  ? GF_COLD
							  : ((chance == 3)
										? GF_ACID
										: ((chance == 4) ? GF_POIS : GF_FIRE))))),
			 inflate, specific));

	case BF_DRAGON_BRONZE:
		rad = 2;
		dam = 120;
		return (borg_attack_aux_dragon(SV_DRAGON_BRONZE, rad, dam, GF_CONFUSION,
												 inflate, specific));

	case BF_DRAGON_GOLD:
		rad = 2;
		dam = 150;
		return (borg_attack_aux_dragon(SV_DRAGON_GOLD, rad, dam, GF_SOUND,
												 inflate, specific));

	case BF_DRAGON_CHAOS:
		chance = rand_int(2);
		rad = 2;
		dam = 220;
		return (borg_attack_aux_dragon(SV_DRAGON_CHAOS, rad, dam,
												 (chance == 1 ? GF_CHAOS : GF_DISENCHANT),
												 inflate, specific));

	case BF_DRAGON_LAW:
		chance = rand_int(2);
		rad = 2;
		dam = 230;
		return (borg_attack_aux_dragon(SV_DRAGON_LAW, rad, dam,
												 (chance == 1 ? GF_SOUND : GF_SHARDS),
												 inflate, specific));

	case BF_DRAGON_BALANCE:
		chance = rand_int(4);
		rad = 2;
		dam = 230;
		return (borg_attack_aux_dragon(
			 SV_DRAGON_BALANCE, rad, dam,
			 (((chance == 1)
					 ? GF_CHAOS
					 : ((chance == 2) ? GF_DISENCHANT
											: ((chance == 3) ? GF_SOUND : GF_SHARDS)))),
			 inflate, specific));

	case BF_DRAGON_SHINING:
		chance = rand_int(2);
		rad = 2;
		dam = 200;
		return (borg_attack_aux_dragon(SV_DRAGON_SHINING, rad, dam,
												 (chance == 0 ? GF_LITE : GF_DARK), inflate,
												 specific));

	case BF_DRAGON_POWER:
		rad = 2;
		dam = 300;
		return (borg_attack_aux_dragon(SV_DRAGON_POWER, rad, dam, GF_MISSILE,
												 inflate, specific));

	/** Racial Attacks **/

	/* Suck Blood */
	case BF_RACIAL_VAMP:
		dam = borg_skill[BI_CLEVEL] +
				((borg_skill[BI_CLEVEL] / 2) *
				 MAX(1, borg_skill[BI_CLEVEL] / 10)); /* Dmg */
		fail = 100 - racial_chance(2, A_CON, 9);
		cost = 1 + (borg_skill[BI_CLEVEL] / 3);
		return (
			 borg_attack_aux_racial_thrust(2, dam, fail, inflate, specific, cost));

	/* Throw Boulder
	case BF_RACIAL_CYCLOPS:
		dam = borg_skill[BI_CLEVEL] * 3 / 2;
		fail = 100 - racial_chance(20, A_STR, 12);
		cost = 15;
		return (borg_attack_aux_racial_bolt(RACE_CYCLOPS, rad, dam, GF_MISSILE,
														fail, inflate, specific, cost));
	*/
	/* Magic Missile */
	case BF_RACIAL_DARK_ELF:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1)) / 4) * 5;
		fail = 100 - racial_chance(2, A_INT, 9);
		cost = 2;
		return (borg_attack_aux_racial_bolt(RACE_DARK_ELF, rad, dam, GF_MISSILE,
														fail, inflate, specific, cost));

	/* Draconian Breath */
	case BF_RACIAL_DRACONIAN:
		dam = 2 * borg_skill[BI_CLEVEL];
		rad = 1 + borg_skill[BI_CLEVEL] / 15;
		fail = 100 - racial_chance(1, A_CON, 12);
		cost = borg_skill[BI_CLEVEL];
		int damage_type = borg_draco_type();
		return (borg_attack_aux_racial_bolt(SIGN_HEAD + SIGN_DRACO, rad, dam,
														damage_type, fail, inflate, specific,
														cost));

	/* Fireball */
	case BF_RACIAL_IMP:
		dam = borg_skill[BI_CLEVEL];
		rad = (borg_skill[BI_CLEVEL] >= 30) ? 2 : 0;
		fail = 100 - racial_chance(9, A_WIS, 15);
		cost = 15;
		return (borg_attack_aux_racial_bolt(RACE_IMP, rad, dam, GF_FIRE, fail,
														inflate, specific, cost));

	/* Acidball */
	case BF_RACIAL_KLACKON:
		dam = borg_skill[BI_CLEVEL];
		rad = (borg_skill[BI_CLEVEL] >= 25) ? 2 : 0;
		fail = 100 - racial_chance(9, A_DEX, 14);
		cost = 9;
		return (borg_attack_aux_racial_bolt(SIGN_HEAD + SIGN_MORUI, rad, dam,
														GF_ACID, fail, inflate, specific,
														cost));

	/* Poison bolt */
	case BF_RACIAL_KOBOLD:
		dam = borg_skill[BI_CLEVEL];
		fail = 100 - racial_chance(12, A_DEX, 14);
		cost = 8;
		return (borg_attack_aux_racial_bolt(RACE_KOBOLD, rad, dam, GF_POIS, fail,
														inflate, specific, cost));

	/* Mindblast bolt */
	case BF_RACIAL_MINDFLAYER:
		dam = borg_skill[BI_CLEVEL];
		fail = 100 - racial_chance(15, A_INT, 14);
		cost = 12;
		return (borg_attack_aux_racial_bolt(RACE_MIND_FLAYER, rad, dam, GF_PSI,
														fail, inflate, specific, cost));

	/* Sleep III */
	case BF_RACIAL_SPRITE:
		dam = borg_skill[BI_CLEVEL];
		rad = MAX_RANGE;
		fail = 100 - racial_chance(12, A_INT, 15);
		cost = 12;
		return (borg_attack_aux_racial_dispel(RACE_SPRITE, rad, dam, GF_OLD_SLEEP,
														  fail, inflate, specific, cost));

	/* Scare Mon
	case BF_RACIAL_YEEK:
		dam = borg_skill[BI_CLEVEL];
		fail = 100 - racial_chance(15, A_WIS, 10);
		cost = 15;
		return (borg_attack_aux_racial_bolt(RACE_YEEK, rad, dam, GF_OLD_SLEEP,
														fail, inflate, specific, cost));
  */
	/* Stone to Mud */
	case BF_RACIAL_HALFGIANT:
		dam = 35;
		fail = 100 - racial_chance(20, A_STR, 12);
		cost = 35;
		return (borg_attack_aux_racial_bolt(RACE_HALF_GIANT, rad, dam,
														GF_KILL_WALL, fail, inflate, specific,
														cost));

	/* Mutations */
	case BF_COR1_SPIT_ACID:
		dam = borg_skill[BI_CLEVEL];
		rad = 1 + (borg_skill[BI_CLEVEL] / 30);
		cost = 9;
		return (borg_attack_aux_mutation_bolt(COR1_SPIT_ACID, rad, dam, GF_ACID,
														  inflate, FALSE, specific, cost));

	case BF_COR1_BR_FIRE:
		dam = borg_skill[BI_CLEVEL] * 2;
		rad = 1 + (borg_skill[BI_CLEVEL] / 20);
		cost = borg_skill[BI_CLEVEL];
		return (borg_attack_aux_mutation_bolt(COR1_BR_FIRE, rad, dam, GF_FIRE,
														  inflate, FALSE, specific, cost));

	case BF_COR1_HYPN_GAZE:
		dam = borg_skill[BI_CLEVEL];
		rad = 0;
		cost = 12;
		return (borg_attack_aux_mutation_bolt(COR1_HYPN_GAZE, rad, dam, GF_CHARM,
														  inflate, FALSE, specific, cost));

	case BF_COR1_MIND_BLST:
		dam = (3 + ((borg_skill[BI_CLEVEL] - 1) / 5) * 3) / 2;
		rad = 0;
		cost = 3;
		return (borg_attack_aux_mutation_bolt(COR1_MIND_BLST, rad, dam, GF_PSI,
														  inflate, FALSE, specific, cost));
	/*
		case BF_COR1_RADIATION:
			dam = (borg_skill[BI_CLEVEL] * 2);
			rad = 3 + (borg_skill[BI_CLEVEL] / 20);
			cost = 15;
			return (borg_attack_aux_mutation_bolt(COR1_RADIATION, rad, dam,
		GF_NUKE,
															  inflate, TRUE, specific, cost));
	*/
	case BF_COR1_VAMPIRISM:
		dam = borg_skill[BI_CLEVEL] +
				((borg_skill[BI_CLEVEL] / 2) *
				 MAX(1, borg_skill[BI_CLEVEL] / 10)); /* Dmg */
		fail = 100 - racial_chance(2, A_CON, 9);
		cost = (1 + (borg_skill[BI_CLEVEL] / 3));
		return (
			 borg_attack_aux_racial_thrust(2, dam, fail, inflate, specific, cost));

	case BF_COR1_SHRIEK:
		dam = (borg_skill[BI_CLEVEL] * 2);
		rad = 8;
		cost = 14;
		return (borg_attack_aux_mutation_bolt(COR1_SHRIEK, rad, dam, GF_SOUND,
														  inflate, TRUE, specific, cost));

	case BF_COR1_PANIC_HIT:
		cost = 14;
		return (borg_attack_aux_mutation_thrust(BF_COR1_PANIC_HIT, inflate,
															 specific, cost));

	case BF_COR1_DAZZLE:
		cost = 15 / 3;
		dam += (borg_attack_aux_mutation_bolt(COR1_DAZZLE, 10,
														  borg_skill[BI_CLEVEL] * 4, GF_STUN,
														  inflate, TRUE, specific, cost));
		dam += (borg_attack_aux_mutation_bolt(
			 COR1_DAZZLE, 10, borg_skill[BI_CLEVEL] * 4, GF_OLD_CONF, inflate,
			 TRUE, specific, cost));
		dam += (borg_attack_aux_mutation_bolt(
			 COR1_DAZZLE, 10, borg_skill[BI_CLEVEL] * 4, GF_TURN_ALL, inflate,
			 TRUE, specific, cost));
		return (dam);
	/*
		case BF_COR1_LASER_EYE:
			dam = (borg_skill[BI_CLEVEL] * 2);
			rad = -1;
			cost = 15;
			return (borg_attack_aux_mutation_bolt(COR1_LASER_EYE, rad, dam,
		GF_LITE,
															  inflate, FALSE, specific, cost));
	*/
	case BF_COR1_BANISH:
		cost = 25;
		return (borg_attack_aux_mutation_thrust(BF_COR1_BANISH, inflate, specific,
															 cost));

	case BF_COR1_COLD_TOUCH:
		cost = 2;
		return (borg_attack_aux_mutation_thrust(BF_COR1_COLD_TOUCH, inflate,
															 specific, cost));

	case BF_RING_FLAMES:
		rad = 2;
		dam = 50;
		return (borg_attack_aux_equip(TV_RING, SV_RING_FLAMES, rad, dam, GF_FIRE,
												inflate, specific));

	case BF_RING_ICE:
		rad = 2;
		dam = 50;
		return (borg_attack_aux_equip(TV_RING, SV_RING_ICE, rad, dam, GF_COLD,
												inflate, specific));

	case BF_RING_ACID:
		rad = 2;
		dam = 50;
		return (borg_attack_aux_equip(TV_RING, SV_RING_ACID, rad, dam, GF_ACID,
												inflate, specific));

	case BF_ACT_SUNLIGHT:
		rad = -1;
		dam = (6 * 8) / 2;
		return (borg_attack_aux_act_bolt(ACT_SUNLIGHT, rad, dam, GF_LITE_WEAK,
													inflate, specific));

	case BF_ACT_BO_MISS_1:
		rad = 0;
		dam = (2 * 6) / 2;
		return (borg_attack_aux_act_bolt(ACT_BO_MISS_1, rad, dam, GF_MISSILE,
													inflate, specific));

	case BF_ACT_BA_POIS_1:
		rad = 3;
		dam = 12;
		return (borg_attack_aux_act_bolt(ACT_BA_POIS_1, rad, dam, GF_POIS,
													inflate, specific));

	case BF_ACT_BO_ELEC_1:
		rad = 0;
		dam = (4 * 8) / 2;
		return (borg_attack_aux_act_bolt(ACT_BO_ELEC_1, rad, dam, GF_ELEC,
													inflate, specific));

	case BF_ACT_BO_ACID_1:
		rad = 0;
		dam = (5 * 8) / 2;
		return (borg_attack_aux_act_bolt(ACT_BO_ACID_1, rad, dam, GF_ACID,
													inflate, specific));

	case BF_ACT_BO_COLD_1:
		rad = 0;
		dam = (6 * 8) / 2;
		return (borg_attack_aux_act_bolt(ACT_BO_COLD_1, rad, dam, GF_COLD,
													inflate, specific));

	case BF_ACT_BO_FIRE_1:
		rad = 0;
		dam = (9 * 8) / 2;
		return (borg_attack_aux_act_bolt(ACT_BO_FIRE_1, rad, dam, GF_FIRE,
													inflate, specific));

	case BF_ACT_BA_COLD_1:
		rad = 2;
		dam = 48;
		return (borg_attack_aux_act_bolt(ACT_BA_COLD_1, rad, dam, GF_COLD,
													inflate, specific));

	case BF_ACT_BA_FIRE_1:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_act_bolt(ACT_BA_FIRE_1, rad, dam, GF_FIRE,
													inflate, specific));

	case BF_ACT_DRAIN_1:
		rad = 0;
		dam = 100;
		return (borg_attack_aux_act_bolt(ACT_DRAIN_1, rad, dam, GF_OLD_DRAIN,
													inflate, specific));

	case BF_ACT_BA_COLD_2:
		rad = 2;
		dam = 100;
		return (borg_attack_aux_act_bolt(ACT_BA_COLD_2, rad, dam, GF_COLD,
													inflate, specific));

	case BF_ACT_BA_ELEC_2:
		rad = 2;
		dam = 72;
		return (borg_attack_aux_act_bolt(ACT_BA_ELEC_2, rad, dam, GF_ELEC,
													inflate, specific));

	case BF_ACT_DRAIN_2:
		rad = 0;
		dam = 120;
		return (borg_attack_aux_act_bolt(ACT_DRAIN_2, rad, dam, GF_OLD_DRAIN,
													inflate, specific));

	case BF_ACT_VAMPIRE_1:
		dam = 150;
		return (borg_attack_aux_act_vamp(ACT_VAMPIRE_1, dam, inflate, specific));

	case BF_ACT_BO_MISS_2:
		rad = 0;
		dam = 150;
		return (borg_attack_aux_act_bolt(ACT_BO_MISS_2, rad, dam, GF_MISSILE,
													inflate, specific));

	case BF_ACT_BA_FIRE_2:
		rad = 3;
		dam = 120;
		return (borg_attack_aux_act_bolt(ACT_BA_FIRE_2, rad, dam, GF_FIRE,
													inflate, specific));

	case BF_ACT_BA_COLD_3:
		rad = 3;
		dam = 200;
		return (borg_attack_aux_act_bolt(ACT_BA_COLD_3, rad, dam, GF_COLD,
													inflate, specific));

	case BF_ACT_BA_ELEC_3:
		rad = 3;
		dam = 250;
		return (borg_attack_aux_act_bolt(ACT_BA_ELEC_3, rad, dam, GF_ELEC,
													inflate, specific));

	case BF_ACT_WHIRLWIND:
		return (borg_attack_aux_nature_whirlwind(inflate /*, specific*/));

	case BF_ACT_VAMPIRE_2:
		dam = 300;
		return (borg_attack_aux_act_vamp(ACT_VAMPIRE_2, dam, inflate, specific));

	/* case BF_ACT_CALL_CHAOS:
	 * rad = 3;
	 * dam = 250;
	 * return (borg_attack_aux_act_dispel(ACT_CALL_CHAOS, rad, dam, GF_MISSILE,
	 * inflate, specific));
	 */
	/*
		case BF_ACT_ROCKET:
			rad = 2;
			dam = 120 + borg_skill[BI_CLEVEL];
			return (borg_attack_aux_act_bolt(ACT_ROCKET, rad, dam, GF_ROCKET,
		inflate,
																specific));
	*/
	case BF_ACT_DISP_EVIL:
		dam = (borg_skill[BI_CLEVEL] * 5);
		rad = MAX_SIGHT;
		return (borg_attack_aux_act_dispel(ACT_DISP_EVIL, rad, dam, GF_DISP_EVIL,
													  inflate, specific));

	case BF_ACT_BA_MISS_3:
		rad = 4;
		dam = 300;
		return (borg_attack_aux_act_bolt(ACT_BA_MISS_3, rad, dam, GF_MISSILE,
													inflate, specific));

	case BF_ACT_DISP_GOOD:
		dam = (borg_skill[BI_CLEVEL] * 5);
		rad = MAX_SIGHT;
		return (borg_attack_aux_act_dispel(ACT_DISP_GOOD, rad, dam, GF_DISP_GOOD,
													  inflate, specific));

	case BF_ACT_CONFUSE:
		rad = 0;
		dam = 20;
		return (borg_attack_aux_act_bolt(ACT_CONFUSE, rad, dam, GF_CONFUSION,
													inflate, specific));

	case BF_ACT_SLEEP:
		return (borg_attack_aux_artifact_holcolleth(/*inflate*/));

	case BF_ACT_CHARM_ANIMAL:
		rad = 0;
		dam = borg_skill[BI_MAXCLEVEL];
		return (borg_attack_aux_act_bolt(ACT_CHARM_ANIMAL, rad, dam,
													GF_CONTROL_ANIMAL, inflate, specific));

	case BF_ACT_CHARM_UNDEAD:
		rad = 0;
		dam = borg_skill[BI_MAXCLEVEL];
		return (borg_attack_aux_act_bolt(ACT_CHARM_UNDEAD, rad, dam,
													GF_CONTROL_UNDEAD, inflate, specific));

	case BF_ACT_CHARM_OTHER:
		rad = 0;
		dam = borg_skill[BI_MAXCLEVEL];
		return (borg_attack_aux_act_bolt(ACT_CHARM_OTHER, rad, dam, GF_CHARM,
													inflate, specific));

	case BF_ACT_CHARM_ANIMALS:
		rad = 10;
		dam = borg_skill[BI_MAXCLEVEL];
		return (borg_attack_aux_act_dispel(ACT_CHARM_ANIMALS, rad, dam,
													  GF_CONTROL_ANIMAL, inflate, specific));

	case BF_ACT_CHARM_OTHERS:
		rad = 10;
		dam = borg_skill[BI_MAXCLEVEL];
		return (borg_attack_aux_act_dispel(ACT_CHARM_OTHERS, rad, dam, GF_CHARM,
													  inflate, specific));

	default:
		/* Oops */
		return (0);
	}

	return (0);
}

/*
 * Attack nearby monsters, in the best possible way, if any.
 *
 * We consider a variety of possible attacks, including physical attacks
 * on adjacent monsters, missile attacks on nearby monsters, spell/prayer
 * attacks on nearby monsters, and wand/rod attacks on nearby monsters.
 *
 * Basically, for each of the known "types" of attack, we "simulate" the
 * "optimal" result of using that attack, and then we "apply" the "type"
 * of attack which appears to have the "optimal" result.
 *
 * When calculating the "result" of using an attack, we only consider the
 * effect of the attack on visible, on-screen, known monsters, which are
 * within 16 grids of the player.  This prevents most "spurious" attacks,
 * but we can still be fooled by situations like creeping coins which die
 * while out of sight, leaving behind a pile of coins, which we then find
 * again, and attack with distance attacks, which have no effect.  Perhaps
 * we should "expect" certain results, and take note of failure to observe
 * those effects.  XXX XXX XXX
 *
 * See above for the "semantics" of each "type" of attack.
 * "Inflate" adds the danger of the monster to the result.
 * "Specific" refers to the borg testing his attacks against a specific monster
 * as opposed to sorting through each one.
 */
int borg_attack(bool boosted_bravery, bool inflate, int specific,
					 bool full_simulate) {
	int i, x, y;

	int n, b_n = 0;
	int g, b_g = -1;
	/*int a_y, a_x;*/

	bool adjacent_monster = FALSE;

	borg_grid *ag;
	monster_race *r_ptr;

	/* Nobody around */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Set the attacking flag so that danger is boosted for monsters */
	/* we want to attack first. */
	borg_attacking = TRUE;

	/* Reset list */
	borg_temp_n = 0;

	/* When estimating the damage to a specific monster */
	if (specific >= 0) {
		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = borg_kills[specific].x;
		borg_temp_y[borg_temp_n] = borg_kills[specific].y;
		borg_temp_n++;
	} else {
		/* Find "nearby" monsters */
		for (i = 1; i < borg_kills_nxt; i++) {

			borg_kill *kill;

			/* Monster */
			kill = &borg_kills[i];
			r_ptr = &r_info[kill->r_idx];

			/* Require current knowledge */
			if ((r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE) &&
				 kill->when < borg_t - 30)
				continue;
			if (!(r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE) &&
				 kill->when < borg_t - 2)
				continue;

			/* Ignore multiplying monsters and when fleeing from scaries*/
			if (goal_ignoring && !borg_skill[BI_ISAFRAID] &&
				 (r_info[kill->r_idx].flags2 & RF2_MULTIPLY))
				continue;

			/* Skip friendly */
			if (kill->ally)
				continue;

			/* Skip the Greater Hell Beast */
			if (strstr(r_name + r_ptr->name, "Greater hell-"))
				continue;

			/* Acquire location */
			/*a_x = kill->x; a_y = kill->y;*/

			/* Check if there is a monster adjacent to me or he's close and fast.
			 */
			if ((kill->speed > borg_skill[BI_SPEED] && kill->dist <= 2) ||
				 kill->dist <= 1)
				adjacent_monster = TRUE;

			/* no attacking most scaryguys, try to get off the level */
			if ((borg_depth & DEPTH_SCARY)) {
				/* probably Grip or Fang. */
				if (strstr(r_name + r_ptr->name, "Grip") ||
					 strstr(r_name + r_ptr->name, "Fang")) {
					/* Try to fight Grip and Fang. */
				} else if (borg_skill[BI_CDEPTH] <= 5 &&
							  borg_skill[BI_CDEPTH] != 0 &&
							  (r_info[kill->r_idx].flags2 & RF2_MULTIPLY)) {
					/* Try to fight single worms and mice. */
				} else if (borg_t - borg_began >= 2000 ||
							  borg_time_town + (borg_t - borg_began) >= 3000) {
					/* Try to fight been there too long. */
				} else if (boosted_bravery || borg_no_retreat >= 1 ||
							  goal_recalling) {
					/* Try to fight if being Boosted or recall engaged. */
					borg_note("# Bored, or recalling and fighting a monster on "
								 "Scaryguy Level.");
				} else if (borg_skill[BI_CDEPTH] * 4 <= borg_skill[BI_CLEVEL] &&
							  borg_skill[BI_CLEVEL] > 10) {
					/* Try to fight anyway. */
					borg_note("# High clevel fighting monster on Scaryguy Level.");
				} else if (adjacent_monster) {
					/* Try to fight if there is a monster next to me */
					borg_note("# Adjacent to monster on Scaryguy Level.");
				} else {
					/* Flee from other scary guys */
					continue;
				}
			}

			/* Low level mages need to conserve the mana in town. These guys don't
			 * fight back */
			if (borg_skill[BI_MAXCLEVEL] < 10 && borg_skill[BI_CDEPTH] == 0 &&
				 (strstr(r_name + r_ptr->name, "Raving") ||
				  strstr(r_name + r_ptr->name, "Blubbering") ||
				  strstr(r_name + r_ptr->name, "Boil") ||
				  strstr(r_name + r_ptr->name, "Village") ||
				  strstr(r_name + r_ptr->name, "Pitiful") ||
				  strstr(r_name + r_ptr->name, "Mangy"))) {
				/* No one nearby */
				if (!adjacent_monster) {
					continue;
				}
			}

			/* Acquire location */
			x = kill->x;
			y = kill->y;

			/* Get grid */
			ag = &borg_grids[y][x];

			/* Never shoot off-screen */
			if (!(ag->info & BORG_OKAY))
				continue;

			/* Never shoot through walls */
			/* if (!(ag->info & BORG_VIEW)) continue; */

			/* Check the distance XXX XXX XXX */
			if (kill->dist > MAX_RANGE)
				continue;

			/* Sometimes the borg can lose a monster index in the grid if there are
			 * lots of monsters
			 * on screen.  If he does lose one, reinject the index here. */
			if (!ag->kill)
				borg_grids[kill->y][kill->x].kill = i;

			/* Save the location (careful) */
			borg_temp_x[borg_temp_n] = x;
			borg_temp_y[borg_temp_n] = y;
			borg_temp_n++;
		}
	}

	/* No destinations */
	if (!borg_temp_n && (!(borg_position & (POSITION_SEA | POSITION_BORE)))) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Simulate */
	borg_simulate = TRUE;

	/* Analyze the possible attacks */
	for (g = 0; g < BF_MAX; g++) {
		/* Simulate */
		n = borg_attack_aux(g, inflate, specific);

		/* Force a starving vampire to only value a bite attack */
		if (borg_skill[BI_VAMPIRE] && borg_skill[BI_ISWEAK] &&
			 borg_skill[BI_CDEPTH] == 0) {
			if (borg_race == RACE_VAMPIRE) {
				if (g != BF_RACIAL_VAMP)
					n = 0;
			} else /* Mutation */
			{
				if (g != BF_COR1_VAMPIRISM)
					n = 0;
			}
		}

		/* Track "best" attack  <= */
		if (n <= b_n)
			continue;

		/* Track best */
		b_g = g;
		b_n = n;
	}

	/* Nothing good */
	if (b_n <= 0) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Return the test value */
	if (full_simulate)
		return (b_n);

	/* Note */
	borg_note(format("# Performing attack type %d with value %d.", b_g, b_n));

	/* Instantiate */
	borg_simulate = FALSE;

	/* Instantiate */
	(void)borg_attack_aux(b_g, inflate, specific);

	borg_attacking = FALSE;

	/* Success */
	return (b_n);
}

/* Munchkin Attack - Mages
 *
 * The early mages have a very difficult time surviving until they level up
 * some.
 * This routine will allow the mage to do some very limited attacking while he
 * is
 * doing the munchking start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate mana, then use MM to attack
 * some
 * easy to kill monsters.  If the monster gets too close, he will flee via the
 * stairs.
 * He hope to be able to kill the monster in two shots from the MM.  A perfect
 * scenario
 * would be a mold which does not move, then he could rest/shoot/rest.
 */

bool borg_munchkin_magic(void) {

	int i, x, y;
	/*int a_y, a_x;*/
	/*int attack = 0;*/
	int dam = -1;
	int b_dam = -1;
	int b_n = -1;

	borg_grid *ag;
	/*monster_race *r_ptr;*/

	/* Nobody around */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Must not be too dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) > avoidance * 7 / 10)
		return (FALSE);
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3)
		return (FALSE);
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Must be standing on a stair */
	if (borg_grids[c_y][c_x].feat != FEAT_MORE &&
		 borg_grids[c_y][c_x].feat != FEAT_LESS)
		return (FALSE);

	/* Set the attacking flag so that danger is boosted for monsters */
	borg_attacking = TRUE;

	/* Reset list */
	borg_temp_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip friendly */
		if (kill->ally)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Acquire location
		a_x = kill->x;
		a_y = kill->y;
		*/

		/* Not in town.  This should not be reached, but just in case we add it */
		if (borg_skill[BI_CDEPTH] == 0)
			continue;

/* Check if there is a monster adjacent to me or he's close and fast. */
#if 0
		if ((kill->speed > borg_skill[BI_SPEED] && distance(c_y, c_x, a_y, a_x) <= 2 &&
			!(r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE)) ||
			distance(c_y, c_x, a_y, a_x) <= 1) return (FALSE);
#endif

		/* no attacking most scaryguys, try to get off the level */
		if ((borg_depth & DEPTH_SCARY))
			return (FALSE);

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;

		/* Check the distance XXX XXX XXX */
		if (kill->dist > MAX_RANGE)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No destinations */
	if (!borg_temp_n) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Simulate */
	borg_simulate = TRUE;

	/* Simulated */
	for (i = 0; i <= BF_MAX; i++) {
		/* Skip certain types of attacks */
		if (i < BF_LAUNCH_NORMAL)
			continue;

		/* Perform the simulated attack and return a value */
		dam = borg_attack_aux(i, TRUE, -1);

		/* Track the best attack method */
		if (dam >= b_dam && dam > 0) {
			b_dam = dam;
			b_n = i;
		}
	}

	/* Nothing good */
	if (b_n <= 0 || b_dam <= 0) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Note */
	borg_note(format("# Performing munchkin ranged attack (%d) with value %d.",
						  b_n, b_dam));

	/* Instantiate */
	borg_simulate = FALSE;

	/* Instantiate */
	(void)borg_attack_aux(b_n, TRUE, -1);

	borg_attacking = FALSE;

	/* Success */
	return (TRUE);
}

/* Munchkin Attack - Melee
 *
 * The early borgs have a very difficult time surviving until they level up
 * some.
 * This routine will allow the borg to do some very limited attacking while he
 * is
 * doing the munchking start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate HP, then use melee to attack
 * some
 * easy to kill adjacent monsters.
 */

bool borg_munchkin_melee(void) {

	int i, x, y;
	/*int a_y, a_x;*/
	int attack = BF_THRUST;
	int n2 = 0;
	int b_n = -1;

	borg_grid *ag;
	/*monster_race *r_ptr;*/

	/* No Mages for now */
	if (borg_skill[BI_NO_MELEE])
		return (FALSE);

	/* Must be standing on a stair */
	if (borg_grids[c_y][c_x].feat != FEAT_MORE &&
		 borg_grids[c_y][c_x].feat != FEAT_LESS)
		return (FALSE);

	/* Nobody around */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Must not be too dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) > avoidance)
		return (FALSE);
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2 &&
		 borg_danger(c_y, c_x, 1, TRUE) > avoidance / 2)
		return (FALSE);
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Set the attacking flag so that danger is boosted for monsters */
	/* we want to attack first. */
	borg_attacking = TRUE;

	/* Reset list */
	borg_temp_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Skip friendly */
		if (kill->ally)
			continue;

		/* Acquire location
		a_x = kill->x;
		a_y = kill->y;
		*/

		/* Not in town.  This should not be reached, but just in case we add it */
		if (borg_skill[BI_CDEPTH] == 0)
			continue;

		/* no attacking most scaryguys, try to get off the level */
		if ((borg_depth & DEPTH_SCARY))
			return (FALSE);

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;

		/* Check the distance XXX XXX XXX */
		if (kill->dist != 1)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No destinations */
	if (!borg_temp_n) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Simulate */
	borg_simulate = TRUE;

	/* Simulated */
	b_n = borg_attack_aux(BF_THRUST, TRUE, -1);
	n2 = borg_attack_aux(BF_RACIAL_VAMP, TRUE, -1);

	/* Choose the better */
	if (n2 > b_n) {
		b_n = n2;
		attack = BF_RACIAL_VAMP;
	}

	/* Nothing good */
	if (b_n <= 0) {
		borg_attacking = FALSE;
		return (FALSE);
	}

	/* Note */
	borg_note(format("# Performing munchkin melee attack with value %d.", b_n));

	/* Instantiate */
	borg_simulate = FALSE;

	/* Instantiate */
	(void)borg_attack_aux(attack, TRUE, -1);

	borg_attacking = FALSE;

	/* Success */
	return (TRUE);
}

/* Log the pathway and feature of the spell pathway
 * Useful for debugging beams and Tport Other spell
 */
static void borg_log_spellpath(bool beam) {
	int n_x, n_y, x, y;

	int dist = 0;

	borg_grid *ag;
	borg_kill *kill;

	y = borg_target_y;
	x = borg_target_x;
	n_x = c_x;
	n_y = c_y;

	while (1) {
		ag = &borg_grids[n_y][n_x];

		/* Note the Pathway */
		if (n_y == c_y && n_x == c_x) {
			borg_note(
				 format("# Logging Spell pathway (%d,%d): My grid.", n_y, n_x));
		} else if (ag->kill) {
			kill = &borg_kills[ag->kill];
			borg_note(format("# Logging Spell pathway (%d,%d): %s, danger %d", n_y,
								  n_x, (r_name + r_info[kill->r_idx].name),
								  borg_danger_aux(n_y, n_x, 1, ag->kill, TRUE, FALSE)));
		} else if (!borg_cave_floor_grid(ag)) {
			borg_note(
				 format("# Logging Spell pathway (%d,%d): Wall grid.", n_y, n_x));
			break;
		} else {
			borg_note(format("# Logging Spell pathway (%d,%d).", n_y, n_x));
		}

		/* Stop loop if we reach our target if using bolt */
		if (n_x == x && n_y == y && !beam)
			break;

		/* Safegaurd not to loop */
		dist++;
		if (dist >= MAX_RANGE)
			break;

		/* Calculate the new location */
		mmove2(&n_y, &n_x, c_y, c_x, y, x);

		/* Bounds Check */
		if (!in_bounds(n_y, n_x))
			break;
	}
}

/*
 *
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Bless\Prayer
 *   Berserk\Heroism
 *   Temp Resist (either all or just cold/fire?)
 *   Shield
 *   Teleport away
 *   Glyph of Warding
 *   See inviso
 *
 * * and many others
 *
 */
enum {
	BD_SPEED,
	BD_PROT_FROM_EVIL,
	BD_BLESS,
	BD_BERSERK,
	BD_HERO,
	BD_RESIST_FCE,
	BD_RESIST_FECAP,
	BD_RESIST_FECAP_POT,
	BD_RESIST_F,
	BD_RESIST_C,
	BD_RESIST_A,
	BD_RESIST_E,
	BD_RESIST_P,
	BD_SHIELD,
	BD_GOI,
	BD_GOI_POT,
	BD_GLYPH,
	BD_WARDING,
	BD_TPORTOTHER,
	BD_CREATE_WALLS,
	BD_MASS_GENOCIDE,
	BD_GENOCIDE,
	BD_GENOCIDE_NASTIES,
	BD_EARTHQUAKE,
	BD_DESTRUCTION,
	BD_BANISH_EVIL,
	BD_BANISH_GOOD,
	BD_BANISHMENT_LUCIFER,
	BD_TPORTOTHER_LUCIFER,
	BD_DETECT_INVISO,
	BD_LIGHT_BEAM,
	BD_SHIFT_PANEL,
	BD_TRUMP_SERVANT,
	BD_OFFENSE,
	BD_TELE_WAVE,
	BD_MIND_WAVE,
	BD_DISMISS_PETS,
	BD_DIVINE_INTERV,
	BD_DAY_DOVE,
	BD_REST,
	BD_STERILITY,
	BD_HALL_SLEEP,
	BD_WRAITH,
	BD_BRANDING,

	BD_MAX
};

/*
 * Bless/Prayer to prepare for battle
 */
static int borg_defend_aux_bless(int p1) {
	int fail_allowed = 10;

	/* already blessed */
	if (borg_bless)
		return (0);

	/* Cant when Blind */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* Dont bother if not a melee class */
	if (borg_skill[BI_NO_MELEE])
		return (0);

	/* Use our turn for some fast killing of summoners */
	if (borg_fighting_summoner)
		return (0);

	if (borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)
		return (0);

	/* no spell */
	if (!borg_spell_okay_fail(REALM_LIFE, 0, 2, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_LIFE, 3, 1, fail_allowed) &&
		 -1 == borg_slot(TV_SCROLL, SV_SCROLL_BLESSING) &&
		 -1 == borg_slot(TV_SCROLL, SV_SCROLL_HOLY_CHANT) &&
		 -1 == borg_slot(TV_SCROLL, SV_SCROLL_HOLY_PRAYER))
		return (0);

	/* if we are in some danger but not much, go for a quick bless */
	if ((p1 > avoidance / 12 && p1 < avoidance / 2) || borg_surrounded() ||
		 (p1 >= avoidance / 20 && borg_class == CLASS_RANGER)) {
		/* Simulation */
		/* bless is a low priority */
		if (borg_simulate)
			return (1);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_spell(REALM_LIFE, 0, 2) || borg_spell(REALM_LIFE, 3, 1) ||
			 borg_read_scroll(SV_SCROLL_BLESSING) ||
			 borg_read_scroll(SV_SCROLL_HOLY_CHANT) ||
			 borg_read_scroll(SV_SCROLL_HOLY_PRAYER))
			return 1;
	}

	return (0);
}

/*
 * Speed to prepare for battle
 */
static int borg_defend_aux_speed(int p1) {
	int p2 = 0;
	bool good_speed = FALSE;
	bool speed_spell = FALSE;
	bool speed_staff = FALSE;
	bool speed_rod = FALSE;
	int fail_allowed = 39;
	bool need_boost = FALSE;

	/* already fast */
	if (borg_speed)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	/* Do it */
	if (!borg_simulate) {
		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_spell_fail(REALM_SORCERY, 1, 5, fail_allowed) ||
			 borg_spell_fail(REALM_DEATH, 2, 3, fail_allowed) ||
			 borg_mindcr_fail(MIND_ADRENALINE, 35, fail_allowed))
			return (1);

		if (borg_zap_rod(SV_ROD_SPEED) ||
			 borg_activate_artifact(ART_BOOTS_FURCIFER, FALSE) ||
			 borg_activate_artifact(ART_BOOTS_GABRIEL, FALSE) ||
			 borg_activate_artifact(ART_RING_GABRIEL, FALSE) ||
			 borg_activate_artifact(ART_BELETH, FALSE) ||
			 borg_activate_activation(ACT_XTRA_SPEED, FALSE) ||
			 borg_activate_activation(ACT_SPEED, FALSE) ||
			 borg_use_staff(SV_STAFF_SPEED) || borg_quaff_potion(SV_POTION_SPEED))

			/* Value */
			return (1);
	}

	/* only cast defence spells if fail rate is not too high */
	if (borg_spell_okay_fail(REALM_SORCERY, 1, 5, fail_allowed) ||
		 borg_spell_okay_fail(REALM_DEATH, 2, 3, fail_allowed) ||
		 borg_mindcr_okay_fail(MIND_ADRENALINE, 35, fail_allowed))
		speed_spell = TRUE;

	/* staff must have charges */
	if (borg_equips_staff_fail(SV_STAFF_SPEED))
		speed_staff = TRUE;

	/* rod can't be charging */
	if (borg_equips_rod(SV_ROD_SPEED) ||
		 borg_equips_activation(ACT_SPEED, TRUE) ||
		 borg_equips_activation(ACT_XTRA_SPEED, TRUE))
		speed_rod = TRUE;

	if (0 > borg_slot(TV_POTION, SV_POTION_SPEED) && !speed_staff &&
		 !speed_rod && !speed_spell &&
		 !borg_equips_artifact(ART_BOOTS_FURCIFER) &&
		 !borg_equips_artifact(ART_BOOTS_GABRIEL) &&
		 !borg_equips_artifact(ART_BELETH) &&
		 !borg_equips_artifact(ART_RING_GABRIEL))
		return (0);

	/* if we have an infinite/large suppy of speed we can */
	/* be generious with our use */
	if (speed_rod || speed_spell || speed_staff ||
		 borg_equips_artifact(ART_BOOTS_FURCIFER) ||
		 borg_equips_artifact(ART_BOOTS_GABRIEL) ||
		 borg_equips_artifact(ART_BELETH) ||
		 borg_equips_artifact(ART_RING_GABRIEL))
		good_speed = TRUE;

	/* pretend we are protected and look again */
	borg_speed_spell = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_speed_spell = FALSE;

	/* if we are fighting a unique cast it. */
	if (good_speed && borg_fighting_unique) {
		/* HACK pretend that it was scary and will be safer */
		p2 = p2 * 7 / 10;
	}
	/* if we are fighting a quest monster cast it. */
	if (borg_fighting_questor) {
		/* HACK pretend that it was scary and will be safer */
		p2 = p2 * 7 / 10;
	}
	/* if we are fighting a unique and a summoner cast it. */
	if (borg_fighting_summoner && borg_fighting_unique) {
		/* HACK pretend that it was scary and will be safer */
		p2 = p2 * 7 / 10;
	}
	/* if the unique is Lilith cast it */
	if (borg_skill[BI_CDEPTH] == 99 && borg_fighting_unique == RACE_LILITH) {
		p2 = p2 * 6 / 10;
	}

	/* if the unique is Lucifer cast it */
	if (borg_skill[BI_CDEPTH] == LUCIFER_DEPTH &&
		 borg_fighting_unique == RACE_LUCIFER) {
		p2 = p2 * 5 / 10;
	}

	/* Attempt to conserve Speed at end of game */
	if (borg_skill[BI_CDEPTH] >= 97 && !borg_fighting_unique && !good_speed)
		p2 = 9999;

	/* might need to boost our bravery a bit */
	if (borg_fighting_unique || borg_fighting_questor)
		need_boost = TRUE;
	if (unique_on_level || (borg_depth & DEPTH_QUEST))
		need_boost = TRUE;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (((p1 > p2) &&
		  p2 <= (need_boost ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		  (p1 > (avoidance / 5)) && good_speed) ||
		 ((p1 > p2) &&
		  p2 <= (need_boost ? ((avoidance * 2) / 3) : (avoidance / 3)) &&
		  (p1 > (avoidance / 7)))) {
		/* if we just did GOI do a speed right after. */
		if (good_speed && (borg_goi || borg_wraith)) {
			/* HACK pretend that it was scary and will be very safe */
			/* This is done because GOI messes up our calculations */
			p1 = 10000;
			p2 = 1;
		}

		/* if we just did GOI do a speed right after. */
		else if (good_speed && need_boost) {
			/* HACK pretend that it was scary and will be very safe */
			p1 = 1000;
			p2 = 1;
		}

		/* Simulation */
		return (p1 - p2 + ((borg_goi + borg_wraith) / 100) * 50);
	}

	/* default to can't do it. */
	return (0);
}

/*
 * Globe of Invulnurability
 */
static int borg_defend_aux_goi(int p1) {
	int p2 = 0;
	int fail_allowed = 50;

	if (borg_goi || borg_wraith)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* a bit scary */
		 if (p1 > (avoidance / 2))
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 4)
		fail_allowed += 10;

	/* If fighting regular unique boost the fail rate */
	if (borg_fighting_unique >= 1)
		fail_allowed = 25;

	/* If fighting Questor boost the fail rate */
	if (borg_fighting_unique == RACE_LUCIFER ||
		 borg_skill[BI_CDEPTH] == LUCIFER_DEPTH)
		fail_allowed = 33;

	if (!borg_spell_okay_fail(REALM_SORCERY, 3, 7, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_LIFE, 3, 7, fail_allowed) &&
		 !borg_equips_activation(ACT_INVULN, TRUE) &&
		 !borg_spell_okay_fail(REALM_DEATH, 3, 7, fail_allowed) &&
		 !borg_equips_activation(ACT_WRAITH, TRUE))
		return (0);

	/* pretend we are protected and look again */
	borg_goi = 100;
	borg_wraith = 100;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_goi = 0;
	borg_wraith = 0;

	/*  if we are fighting a unique enhance the value by reducing p2*/
	if (borg_fighting_unique) {
		p2 = p2 / 2;
	}

	/* if the unique is Sauron cast it */
	if (borg_skill[BI_CDEPTH] == LILITH_DEPTH &&
		 borg_fighting_unique == RACE_LILITH) {
		p2 = p2 * 4 / 10;
	}

	/* if the unique is Lucifer cast it */
	if (borg_skill[BI_CDEPTH] == LUCIFER_DEPTH &&
		 borg_fighting_unique == RACE_LUCIFER) {
		p2 = 0;
	}

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 3)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_activation(ACT_INVULN, FALSE) ||
			 borg_activate_activation(ACT_WRAITH, FALSE) ||
			 borg_spell_fail(REALM_DEATH, 3, 7, fail_allowed) ||
			 borg_spell_fail(REALM_SORCERY, 3, 7, fail_allowed) ||
			 borg_spell_fail(REALM_LIFE, 3, 7, fail_allowed))
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}
/*
 * Globe of Invulnurability Potion
 */
static int borg_defend_aux_goi_pot(int p1) {
	int p2 = 0;

	if (borg_goi || borg_wraith)
		return (0);

	/* Save for fighting uniques */
	if (!borg_fighting_unique)
		return (0);

	/* have some in inven? */
	if (borg_has[POTION_INVULN] == 0)
		return (0);

	/* pretend we are protected and look again */
	borg_goi = 100;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_goi = 0;

	/*  Fighting a unique, enhance the value by reducing p2*/
	p2 = p2 / 2;

	/* if the unique is Sauron cast it */
	if (borg_skill[BI_CDEPTH] == LILITH_DEPTH &&
		 borg_fighting_unique == RACE_LILITH) {
		p2 = p2 * 4 / 10;
	}

	/* if the unique is Lucifer cast it */
	if (borg_skill[BI_CDEPTH] == LUCIFER_DEPTH &&
		 borg_fighting_unique == RACE_LUCIFER) {
		p2 = 0;
	}

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_quaff_potion(SV_POTION_INVULNERABILITY))
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/*
 * Wraith form spell.
 * Allows for Passwall, and Reflection
 */
static int borg_defend_aux_wraith(int p1) {
	int p2 = 0;
	int fail_allowed = 39;

	/* no need */
	if (borg_wraith || borg_goi)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* a bit scary */
		 if (p1 > (avoidance / 2))
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 4)
		fail_allowed += 10;

	/* If fighting regular unique boost the fail rate */
	if (borg_fighting_unique >= 1)
		fail_allowed = 25;

	/* If fighting Questor boost the fail rate */
	if (borg_fighting_unique == RACE_LUCIFER &&
		 borg_skill[BI_CDEPTH] == LUCIFER_DEPTH)
		fail_allowed = 33;

	/* If fighting Questor boost the fail rate */
	if (borg_fighting_unique == RACE_LILITH &&
		 borg_skill[BI_CDEPTH] == LILITH_DEPTH)
		fail_allowed = 33;

	if (!borg_spell_okay_fail(REALM_DEATH, 3, 7, fail_allowed) &&
		 !borg_equips_activation(ACT_INVULN, TRUE))
		return (0);

	/* pretend we are protected and look again */
	borg_skill[BI_REFLECT] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_skill[BI_REFLECT] = FALSE;

	/*  if we are fighting a unique enhance the value by reducing p2*/
	if (borg_fighting_unique) {
		p2 = p2 / 2;
	}

	/* if the unique is Sauron cast it */
	if (borg_skill[BI_CDEPTH] == LILITH_DEPTH &&
		 borg_fighting_unique == RACE_LILITH) {
		p2 = p2 * 4 / 10;
	}

	/* if the unique is Lucifer cast it */
	if (borg_skill[BI_CDEPTH] == LUCIFER_DEPTH &&
		 borg_fighting_unique == RACE_LUCIFER) {
		p2 = 0;
	}

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 3)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_activation(ACT_WRAITH, FALSE) ||
			 borg_spell_fail(REALM_DEATH, 3, 7, fail_allowed))
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/* cold/fire */
static int borg_defend_aux_resist_fce(int p1) {
	int p2 = 0;
	int fail_allowed = 39;
	bool save_fire, save_cold, save_elec;

	if (borg_skill[BI_TRFIRE] && borg_skill[BI_TRCOLD] && borg_skill[BI_TRELEC])
		return (0);

#if 0
        if (borg_skill[BI_RFIRE] &&
        borg_skill[BI_RCOLD])
        return (0);
#endif

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_spell_okay_fail(REALM_NATURE, 0, 6, fail_allowed) &&
		 !borg_equips_artifact(ART_JOSEPH))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	/* pretend we are protected and look again */
	save_fire = borg_skill[BI_TRFIRE];
	save_cold = borg_skill[BI_TRCOLD];
	save_elec = borg_skill[BI_TRELEC];

	borg_skill[BI_TRFIRE] = TRUE;
	borg_skill[BI_TRCOLD] = TRUE;
	borg_skill[BI_TRELEC] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRFIRE] = save_fire;
	borg_skill[BI_TRCOLD] = save_cold;
	borg_skill[BI_TRELEC] = save_elec;

	/* Hack -
	 * If the borg is fighting a particular unique enhance the
	 * benefit of the spell.
	 */
	if (borg_fighting_unique && (unique_on_level == 838) /* Tarresque */
																		  /* ||
																			* (unique_on_level == XX) ||
																			*/
		 )
		p2 = p2 * 8 / 10;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_spell_fail(REALM_NATURE, 0, 6, fail_allowed))

			/* Value */
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/* all resists */
static int borg_defend_aux_resist_fecap(int p1) {
	int p2 = 0;
	int fail_allowed = 39;
	bool save_fire, save_acid, save_poison, save_elec, save_cold;

	/* execute it */
	if (!borg_simulate) {
		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_activate_activation(ACT_RESIST_ALL, FALSE) ||
			 borg_mutation(COR1_RESIST, FALSE, 100, FALSE) ||
			 borg_spell(REALM_NATURE, 2, 3) || borg_mindcr(MIND_CHAR_ARMOUR, 35))

			/* Value */
			return (p1);
	}

	/* No Need */
	if (borg_skill[BI_TRFIRE] && borg_skill[BI_TRACID] &&
		 borg_skill[BI_TRPOIS] && borg_skill[BI_TRELEC] && borg_skill[BI_TRCOLD])
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_equips_artifact(ART_JOSEPH) &&
		 !borg_equips_activation(ACT_RESIST_ALL, TRUE) &&
		 !borg_mutation(COR1_RESIST, TRUE, fail_allowed, FALSE) &&
		 !borg_spell_okay_fail(REALM_NATURE, 2, 3, fail_allowed) &&
		 !borg_mindcr_okay_fail(MIND_CHAR_ARMOUR, 35, fail_allowed))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	/* pretend we are protected and look again */
	save_fire = borg_skill[BI_TRFIRE];
	save_acid = borg_skill[BI_TRACID];
	save_poison = borg_skill[BI_TRPOIS];
	save_elec = borg_skill[BI_TRELEC];
	save_cold = borg_skill[BI_TRCOLD];
	borg_skill[BI_TRFIRE] = TRUE;
	borg_skill[BI_TRCOLD] = TRUE;
	borg_skill[BI_TRACID] = TRUE;
	borg_skill[BI_TRPOIS] = TRUE;
	borg_skill[BI_TRELEC] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRFIRE] = save_fire;
	borg_skill[BI_TRCOLD] = save_cold;
	borg_skill[BI_TRACID] = save_acid;
	borg_skill[BI_TRPOIS] = save_poison;
	borg_skill[BI_TRELEC] = save_elec;

	/* Hack -
	 * If the borg is fighting a particular unique enhance the
	 * benefit of the spell.
	 TODO SHOULD did I rename Tarrasque or just delete him??
	if (borg_fighting_unique && (unique_on_level == RACE_TARRASQUE))
		p2 = p2 * 8 / 10;
	*/

	/* Hack -
	 * If borg is high enough level, he does not need to worry
	 * about mana consumption.  Cast the good spell.
	 */
	if (borg_skill[BI_CLEVEL] >= 45)
		p2 = p2 * 8 / 10;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return ((p1 - p2) - 1);
	}

	/* default to can't do it. */
	return (0);
}
/* all resists from potion */
static int borg_defend_aux_resist_fecap_pot(int p1) {
	int p2 = 0;
	bool save_fire, save_acid, save_poison, save_elec, save_cold;

	/* Only check if 4 of the 5 are down */
	if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRACID] + borg_skill[BI_TRPOIS] +
			  borg_skill[BI_TRELEC] + borg_skill[BI_TRCOLD] <
		 5)
		return (0);

	/* Check to see if we have any Potions */
	if (borg_has[POTION_RESISTANCE] == 0)
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	/* pretend we are protected and look again */
	save_fire = borg_skill[BI_TRFIRE];
	save_acid = borg_skill[BI_TRACID];
	save_poison = borg_skill[BI_TRPOIS];
	save_elec = borg_skill[BI_TRELEC];
	save_cold = borg_skill[BI_TRCOLD];
	borg_skill[BI_TRFIRE] = TRUE;
	borg_skill[BI_TRCOLD] = TRUE;
	borg_skill[BI_TRACID] = TRUE;
	borg_skill[BI_TRPOIS] = TRUE;
	borg_skill[BI_TRELEC] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRFIRE] = save_fire;
	borg_skill[BI_TRCOLD] = save_cold;
	borg_skill[BI_TRACID] = save_acid;
	borg_skill[BI_TRPOIS] = save_poison;
	borg_skill[BI_TRELEC] = save_elec;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return ((p1 - p2) - 1);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_quaff_potion(SV_POTION_RESISTANCE))
			return ((p1 - p2) - 1);
	}

	/* default to can't do it. */
	return (0);
}

/* fire */
static int borg_defend_aux_resist_f(int p1) {

	int p2 = 0;
	int fail_allowed = 39;
	bool save_fire;

	if (borg_skill[BI_TRFIRE])
		return (0);

	if (!borg_simulate) {
		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* Rings require a target */
		if (borg_equips_item(TV_RING, SV_RING_FLAMES))
			borg_target(c_y, c_x);

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_activate_item(TV_RING, SV_RING_FLAMES, TRUE) ||
			 borg_spell_fail(REALM_ARCANE, 1, 7, fail_allowed) ||
			 borg_mindcr_fail(MIND_CHAR_ARMOUR, 20, fail_allowed) ||
			 borg_quaff_potion(SV_POTION_RESIST_HEAT))

			/* Value */
			return (p1 - p2);
	}

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_spell_okay_fail(REALM_ARCANE, 1, 7, fail_allowed) &&
		 !borg_mindcr_okay_fail(MIND_CHAR_ARMOUR, 20, fail_allowed) &&
		 !borg_equips_artifact(ART_JOSEPH) &&
		 -1 == borg_slot(TV_POTION, SV_POTION_RESIST_HEAT) &&
		 !borg_equips_item(TV_RING, SV_RING_FLAMES))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	save_fire = borg_skill[BI_TRFIRE];

	/* pretend we are protected and look again */
	borg_skill[BI_TRFIRE] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRFIRE] = save_fire;

	/* Hack -
	 * If the borg is fighting a particular unique enhance the
	 * benefit of the spell.
	 */
	if (borg_fighting_unique && (unique_on_level == 838) /* Tarresque */
																		  /* ||
																			* (unique_on_level == XX)
																			*/
		 )
		p2 = p2 * 8 / 10;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/* cold */
static int borg_defend_aux_resist_c(int p1) {

	int p2 = 0;
	int fail_allowed = 39;
	bool save_cold;

	if (borg_skill[BI_TRCOLD])
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_spell_okay_fail(REALM_NATURE, 1, 6, fail_allowed) &&
		 !borg_equips_artifact(ART_JOSEPH) &&
		 !borg_mindcr_okay_fail(MIND_CHAR_ARMOUR, 25, fail_allowed) &&
		 !borg_equips_item(TV_RING, SV_RING_ICE) &&
		 -1 == borg_slot(TV_POTION, SV_POTION_RESIST_COLD))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	save_cold = borg_skill[BI_TRCOLD];
	/* pretend we are protected and look again */
	borg_skill[BI_TRCOLD] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRCOLD] = save_cold;

	/* Hack -
	 * If the borg is fighting a particular unique enhance the
	 * benefit of the spell.
	 */
	if (borg_fighting_unique && (unique_on_level == 838) /* Tarresque */
																		  /* ||
																			* (unique_on_level == XX) ||
																			*/
		 )
		p2 = p2 * 8 / 10;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* Rings require a target */
		if (borg_equips_item(TV_RING, SV_RING_ICE))
			borg_target(c_y, c_x);

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_activate_item(TV_RING, SV_RING_ICE, TRUE) ||
			 borg_spell_fail(REALM_NATURE, 1, 6, fail_allowed) ||
			 borg_mindcr_fail(MIND_CHAR_ARMOUR, 25, fail_allowed) ||
			 borg_quaff_potion(SV_POTION_RESIST_COLD))

			/* Value */
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/* acid */
static int borg_defend_aux_resist_a(int p1) {

	int p2 = 0;
	int fail_allowed = 39;
	bool save_acid;

	if (borg_skill[BI_TRACID])
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_spell_okay_fail(REALM_NATURE, 2, 7, fail_allowed) &&
		 !borg_mindcr_okay_fail(MIND_CHAR_ARMOUR, 15, fail_allowed) &&
		 !borg_equips_artifact(ART_JOSEPH) &&
		 !borg_equips_item(TV_RING, SV_RING_ACID))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	save_acid = borg_skill[BI_TRACID];
	/* pretend we are protected and look again */
	borg_skill[BI_TRACID] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRACID] = save_acid;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* Rings require a target */
		if (borg_equips_item(TV_RING, SV_RING_ACID))
			borg_target(c_y, c_x);

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_activate_item(TV_RING, SV_RING_ACID, TRUE) ||
			 borg_mindcr_fail(MIND_CHAR_ARMOUR, 15, fail_allowed) ||
			 borg_spell_fail(REALM_NATURE, 2, 7, fail_allowed))

			/* Value */
			return (p1 - p2);
	}
	/* default to can't do it. */
	return (0);
}

/* poison */
static int borg_defend_aux_resist_p(int p1) {
	int p2 = 0;
	int fail_allowed = 39;
	bool save_poison;

	if (borg_skill[BI_TRPOIS])
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (!borg_spell_okay_fail(REALM_DEATH, 0, 5, fail_allowed) &&
		 !borg_equips_artifact(ART_JOSEPH))
		return (0);

	/* For Elemental and PFE the p1 needs to look at danger with FALSE */
	p1 = borg_danger(c_y, c_x, 1, FALSE);

	save_poison = borg_skill[BI_TRPOIS];
	/* pretend we are protected and look again */
	borg_skill[BI_TRPOIS] = TRUE;
	p2 = borg_danger(c_y, c_x, 1, FALSE);
	borg_skill[BI_TRPOIS] = save_poison;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_artifact(ART_JOSEPH, FALSE) ||
			 borg_spell_fail(REALM_DEATH, 0, 5, fail_allowed))

			/* Value */
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/* This and the next routine is used when
 * attacking Lucifer or other summoners. The borg has found a safe place to wait
 * for the monster to show.
 *
 * If the borg is not being threatened immediately by a monster,
 * then rest right here.
 *
 */
static int borg_defend_aux_rest(void) {
	int i;
	int b_i = -1;
	int j;
	bool borg_lure_position = borg_lure_monster();
	/*borg_grid *ag;*/
	borg_kill *kill;
	monster_race *r_ptr;

	/* Only if in a good place */
	if (borg_depth & (DEPTH_SUMMONER | DEPTH_BORER)) {
		/* This is acceptable if these guys are on the level */
	} else if (!borg_lure_position && !(borg_position & POSITION_SUMM)) {
		if (borg_t - borg_t_position >= 50 || time_this_panel > 75)
			return (0);
	} else if (!borg_lure_position && !borg_fighting_tunneler &&
				  ((!(borg_position & POSITION_SUMM) ||
					 borg_t - borg_t_position >= 50) &&
					borg_t_position != 0))
		return (0);

	/* Monster might not want to flow to the borg */
	if (time_this_panel >= 700)
		return (FALSE);

	/* Not if starving */
	if (borg_skill[BI_ISWEAK] || borg_skill[BI_ISHUNGRY])
		return (0);

	/* Not if sitting in a wall */
	if (borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)
		return (0);

	/* Not if a monster can see me */
	/* Examine all the monsters */
	for (i = 0; i < borg_kills_nxt; i++) {
		/*int x9; int y9;*/
		bool monster_in_vault = FALSE;

		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];
		/*x9 = kill->x; y9 = kill->y;*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip sleeping ones */
		if (!kill->awake)
			continue;

		/* Skip ones that one chase me */
		if (!kill->killer || (r_ptr->flags1 & RF1_NEVER_MOVE))
			continue;

		/* need to have seen it recently */
		if (borg_t - kill->when > 75)
			continue;

		/* Minimal and maximum distance */
		if (kill->dist > MAX_RANGE)
			continue;
		if (kill->dist == 1)
			return (0);

		/* Get the grid */
		/*ag = &borg_grids[kill->y][kill->x];*/

		/* If I can see Lucifer/Summoner or a guy with Ranged Attacks, don't rest.
		 */
		if (kill->los) {
			borg_note("# Not resting. I can see a monster.");
			return (0);
		}

		/* Do not rest if I can get an offset attack on him. */
		if (kill->ranged_attack || kill->unique || kill->questor) {
			/* Do I have a good offset attack? XXX XXX */
			if (borg_mindcr_okay_fail(MIND_PULVERISE, 11, 25) ||
				 borg_spell_okay_fail(REALM_LIFE, 1, 4, 25) || /* Orb of Draining */
				 borg_spell_okay_fail(
					  REALM_ARCANE, 2, 6,
					  25) || /* Elemental Ball --Lets make sure it is targetable. */
				 borg_spell_okay_fail(REALM_NATURE, 3, 4, 25) || /* Whirlpool. */
				 borg_spell_okay_fail(REALM_DEATH, 1, 0,
											 25) || /* Orb of Entropy. (drain life) */
				 /* Sorcery does not have a good one */
				 /* Chaos Offset Balls */
				 (borg_skill[BI_CDEPTH] <= 20 &&
				  borg_spell_okay_fail(REALM_CHAOS, 0, 4, 25)) || /* Manaburst */
				 (borg_skill[BI_CDEPTH] <= 50 &&
				  borg_spell_okay_fail(REALM_CHAOS, 1, 2, 25)) || /* Sonic Boom */
				 (borg_skill[BI_CDEPTH] >= 51 &&
				  borg_spell_okay_fail(REALM_CHAOS, 3, 5, 25)) || /* Mana Storm */
				 (borg_skill[BI_CDEPTH] >= 51 &&
				  borg_spell_okay_fail(REALM_CHAOS, 1, 7, 25))) /* Raw Logrus */
			/* (borg_skill[BI_CDEPTH] >= 51 && borg_spell_okay_fail(8, 7, 25))) */
			/* Mana Swarm */
			{
				/* Do I have LOS to this monsters adjacent grid? */
				for (j = 0; j < 8; j++) {
					/* check bounds */
					if (!in_bounds(kill->y + ddy_ddd[j], kill->x + ddx_ddd[j]))
						continue;

					/* If I have LOS then do not rest */
					if (borg_projectable(c_y, c_x, kill->y + ddy_ddd[j],
												kill->x + ddx_ddd[j], TRUE, TRUE)) {
						borg_note("# Not resting. I have LOS to offset grid.");
						return (0);
					}
				}
			}
		}

		/* Not if monster is on a icky grid */
		if ((borg_depth & DEPTH_VAULT)) {
			for (j = 0; j < 8; j++) {
				/* check bounds */
				if (!in_bounds(kill->y + ddy_ddd[j], kill->x + ddx_ddd[j]))
					continue;

				if ((borg_grids[kill->y + ddy_ddd[j]][kill->x + ddx_ddd[j]].feat >=
							FEAT_PERM_EXTRA &&
					  borg_grids[kill->y + ddy_ddd[j]][kill->x + ddx_ddd[j]].feat <=
							FEAT_PERM_SOLID) ||
					 borg_grids[c_y + ddy_ddd[j]][c_x + ddx_ddd[j]].feat ==
						  FEAT_PERM_INNER)
					monster_in_vault = TRUE;
			}
		}
		if (monster_in_vault)
			continue;

		/* If a little twitchy, its ok to stay put */
		if (avoidance > borg_skill[BI_CURHP])
			continue;

		/* See if the monster is able to flow to me. */
		borg_flow_clear_m();
		borg_flow_enqueue_grid_m(c_y, c_x);
		borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
		if (!borg_flow_commit_m(kill->y, kill->x))
			continue;

		/* Track a monster */
		b_i = i;
	}

	/* Return some value for this rest */
	if (borg_simulate) {
		if (b_i != -1 || (((borg_position & (POSITION_SEA | POSITION_BORE))) &&
								(borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] &&
								 borg_skill[BI_MAXSP] > 25)))
			return (200);
		else
			return (0);
	}

	/* Rest */
	if (b_i != -1) {
		kill = &borg_kills[b_i];
		r_ptr = &r_info[kill->r_idx];
		borg_keypress(',');
		borg_note(format("# Resting on grid (%d, %d), waiting for %s.", c_y, c_x,
							  r_name + r_ptr->name));
	} else {
		borg_keypress(',');
		borg_note(format("# Resting on grid (%d, %d), waiting in sweet spot.",
							  c_y, c_x));
	}

	/* All done */
	return (200);
}

static int borg_defend_aux_prot_evil(int p1) {
	int p2 = 0;
	int fail_allowed = 39;
	bool pfe_spell = FALSE;
	borg_grid *ag = &borg_grids[c_y][c_x];

	/* if already protected */
	if (borg_prot_from_evil)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	if (borg_spell_okay_fail(REALM_LIFE, 1, 5, fail_allowed))
		pfe_spell = TRUE;

	if (0 <= borg_slot(TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL))
		pfe_spell = TRUE;

	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE])
		pfe_spell = FALSE;
	if (!(ag->info & BORG_GLOW) && borg_skill[BI_CUR_LITE] == 0)
		pfe_spell = FALSE;

	if (borg_equips_artifact(ART_AMULET_RAPHAEL) ||
		 borg_equips_artifact(ART_AMULET_MICHAEL) ||
		 borg_equips_activation(ACT_PROT_EVIL, TRUE))
		pfe_spell = TRUE;

	if (!pfe_spell)
		return (0);

	/* Testing */
	if (borg_simulate) {

		/* For Elemental and PFE the p1 needs to look at danger with FALSE */
		p1 = borg_danger(c_y, c_x, 1, FALSE);

		/* pretend we are protected and look again */
		borg_prot_from_evil = TRUE;
		p2 = borg_danger(c_y, c_x, 1, FALSE);
		borg_prot_from_evil = FALSE;

		/* if this is an improvement and we may not avoid monster now and */
		/* we may have before */

		if (p1 > p2 &&
			 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
												  : (avoidance / 2)) &&
			 p1 > (avoidance / 7)) {
			/* Simulation */
			if (borg_simulate)
				return (p1 - p2);
		}

		/* Value */
		return (p1 - p2);
	}

	if (!borg_simulate) {
		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_spell_fail(REALM_LIFE, 1, 5, fail_allowed) ||
			 borg_activate_artifact(ART_AMULET_RAPHAEL, FALSE) ||
			 borg_activate_artifact(ART_AMULET_MICHAEL, FALSE) ||
			 borg_activate_activation(ACT_PROT_EVIL, FALSE) ||
			 borg_read_scroll(SV_SCROLL_PROTECTION_FROM_EVIL))

			/* Value */
			return (5);
	}

	/* default to can't do it. */
	return (0);
}

static int borg_defend_aux_shield(int p1) {
	int p2 = 0;
	int fail_allowed = 39;

	/* if already protected */
	if (borg_shield || borg_goi || borg_wraith)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 5;

	if (!borg_spell_okay_fail(REALM_NATURE, 2, 2, fail_allowed) &&
		 !borg_mindcr_okay_fail(MIND_CHAR_ARMOUR, 13, fail_allowed) &&
		 !borg_racial_check(RACE_GOLEM, fail_allowed))
		return (0);

	/* pretend we are protected and look again */
	borg_shield = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_shield = FALSE;

	/* slightly enhance the value if fighting a unique */
	if (borg_fighting_unique)
		p2 = (p2 * 7 / 10);

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_spell_fail(REALM_NATURE, 2, 2, fail_allowed) ||
			 borg_mindcr_fail(MIND_CHAR_ARMOUR, 13, fail_allowed) ||
			 borg_racial(RACE_GOLEM, 1))
			return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/*
 * Try to get rid of all of the non-uniques around so you can go at it
 * 'mano-e-mano' with the unique.
 */
static int borg_defend_aux_tportother(int p1) {
	int p2 = 0, b_n = 0;
	int fail_allowed = 30;
	bool spell_ok;
	int i, x, y;

	borg_grid *ag;
	/*monster_race *r_ptr;*/

	/* Only tell away if scared */
	if (p1 < avoidance * 6 / 10 && !goal_recalling)
		return (0);

	spell_ok = FALSE;

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 18;
	else
		 /* scary */
		 if (p1 > avoidance * 3)
		fail_allowed -= 12;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 5) / 2)
		fail_allowed += 5;

	if (borg_spell_okay_fail(REALM_ARCANE, 3, 3, fail_allowed) ||
		 borg_spell_okay_fail(REALM_SORCERY, 1, 4, fail_allowed) ||
		 borg_spell_okay_fail(REALM_CHAOS, 1, 5, fail_allowed) ||
		 borg_equips_activation(ACT_TELE_AWAY, TRUE) ||
		 borg_equips_artifact(ART_TRITONS) ||
		 borg_equips_rod(SV_ROD_TELEPORT_AWAY) ||
		 borg_equips_wand(SV_WAND_TELEPORT_AWAY))
		spell_ok = TRUE;

	if (!spell_ok)
		return (0);

	/* No Teleport Other if surrounded */
	if (borg_surrounded() == TRUE)
		return (0);

	/* Borg_temp_n temporarily stores several things.
	 * Some of the borg_attack() sub-routines use these numbers,
	 * which would have been filled in borg_attack().
	 * Since this is a defence manuever which will move into
	 * and borrow some of the borg_attack() subroutines, we need
	 * to make sure that the borg_temp_n arrays are properly
	 * filled.  Otherwise, the borg will attempt to consider
	 * these grids which were left filled by some other routine.
	 * Which was probably a flow routine which stored about 200
	 * grids into the array.
	 * Any change in inclusion/exclusion criteria for filling this
	 * array in borg_attack() should be included here also.
	 */
	/* Nobody around so dont worry */
	if (!borg_kills_cnt && borg_simulate)
		return (0);

	/* Reset list */
	borg_temp_n = 0;
	borg_tp_other_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;
		/* if ((ag->feat >= FEAT_RUBBLE) &&
			(ag->feat <= FEAT_PERM_SOLID)) continue; */

		/* Check the distance XXX XXX XXX */
		if (kill->dist > MAX_RANGE)
			continue;

		/* Skip Questors.  The borg is all in when it comes to them */
		if (kill->questor)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No targets for me. */
	if (!borg_temp_n && borg_simulate)
		return (0);

	/* choose, then target a bad guy.
	* Damage will be the danger to my grid which the monster creates.
	* We are targetting the single most dangerous monster.
	* p2 will be the original danger (p1) minus the danger from the most
	* dangerous
	* monster eliminated.
	* ie:  if we are fighting only a single monster who is generating 500 danger
	* and we
	* target him, then p2 _should_ end up 0, since p1 - his danger is 500-500.
	* If we are fighting two guys each creating 500 danger, then p2 will be 500,
	* since
	* 1000-500 = 500.
	 */
	p2 = p1 - borg_launch_bolt(-1, p1, GF_AWAY_ALL, MAX_RANGE, TRUE, 0, -1, TRUE,
										FALSE);

	/* check to see if I am left better off */
	if (borg_simulate) {
		/* Reset list */
		borg_temp_n = 0;
		borg_tp_other_n = 0;

		if (p1 > p2 && (p2 < avoidance / 2 ||
							 (goal_recalling && !borg_skill[BI_ADIMDOOR]))) {
			/* Simulation */
			return (p1 - p2);
		} else
			return (0);
	}

	/* Log the Path for Debug */
	borg_log_spellpath(TRUE);

	/* Log additional info for debug */
	for (i = 0; i < borg_tp_other_n; i++) {
		borg_note(format("# T.O. %d, index %d (%d,%d)", borg_tp_other_n,
							  borg_tp_other_index[i], borg_tp_other_y[i],
							  borg_tp_other_x[i]));
	}

	/* Reset list */
	borg_temp_n = 0;
	borg_tp_other_n = 0;

	/* Cast the spell */
	if (borg_spell(REALM_SORCERY, 1, 4) || borg_spell(REALM_ARCANE, 3, 3) ||
		 borg_spell(REALM_CHAOS, 1, 5) || borg_zap_rod(SV_ROD_TELEPORT_AWAY) ||
		 borg_activate_activation(ACT_TELE_AWAY, FALSE) ||
		 borg_activate_artifact(ART_TRITONS, FALSE) ||
		 borg_aim_wand(SV_WAND_TELEPORT_AWAY)) {
		/* Use target */
		borg_keypress('5');

		/* Set our shooting flag */
		successful_target = -1;

		/* Value */
		return (b_n);
	}

	return (0);
}
/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_tportother_lucifer(void) {
	int p2 = 0;
	int fail_allowed = 40;
	int i, x, y;

	borg_grid *ag;

	/* Need to be actively building my Sea of Runes */
	/* if (borg_needs_new_sea) return (0); */

	/* Only if borer/questor is not on this level */
	if (!(borg_depth & (DEPTH_BORER)))
		return (0);

	/* Cant when screwed */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* I have to actually be able to make a sea of runes */
	if (borg_skill[BI_AGENOCIDE] < 10)
		return (0);

	/* Do I have the T.O. spell? */
	if (!borg_spell_okay_fail(REALM_SORCERY, 1, 4, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_ARCANE, 3, 3, fail_allowed) &&
		 !borg_equips_rod(SV_ROD_TELEPORT_AWAY) &&
		 !borg_equips_wand(SV_WAND_TELEPORT_AWAY) &&
		 !borg_equips_activation(ACT_TELE_AWAY, TRUE) &&
		 !borg_equips_artifact(ART_TRITONS) &&
		 !borg_spell_okay_fail(REALM_CHAOS, 1, 5, fail_allowed))
		return (0);

	/* Do I have the Glyph spell? No good to use TO if I cant build the sea of
	 * runes */
	/* if (borg_skill[BI_AGLYPH] < 10) return (0); */

	/* No Teleport Other if surrounded */
	/* if (borg_surrounded() == TRUE) return (0); */

	/* Borg_temp_n temporarily stores several things.
	 * Some of the borg_attack() sub-routines use these numbers,
	 * which would have been filled in borg_attack().
	 * Since this is a defence manuever which will move into
	 * and borrow some of the borg_attack() subroutines, we need
	 * to make sure that the borg_temp_n arrays are properly
	 * filled.  Otherwise, the borg will attempt to consider
	 * these grids which were left filled by some other routine.
	 * Which was probably a flow routine which stored about 200
	 * grids into the array.
	 * Any change in inclusion/exclusion criteria for filling this
	 * array in borg_attack() should be included here also.
	 */

	/* Nobody around so dont worry */
	if (!borg_kills_cnt && borg_simulate)
		return (0);

	/* Reset list */
	borg_temp_n = 0;
	borg_tp_other_n = 0;

	/* Find "nearby" monsters */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Use the terrain to advantage */
		/* See if the monster is able to flow to me. */
		borg_flow_clear_m();
		borg_flow_enqueue_grid_m(c_y, c_x);
		borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
		if (!borg_flow_commit_m(kill->y, kill->x))
			continue;

		/* Check the distance XXX XXX XXX */
		else if (kill->dist > MAX_RANGE)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No destinations */
	if (!borg_temp_n && borg_simulate)
		return (0);

	/* choose then target a bad guy or several
	 * If left as bolt, he targets the single most nasty guy.
	 * If left as beam, he targets the collection of monsters.
	 */
	p2 = borg_launch_bolt(-1, 50, GF_AWAY_ALL_LUCIFER, MAX_RANGE, TRUE, 0, -1,
								 TRUE, FALSE);

	/* Normalize the value a bit */
	if (p2 > 1000)
		p2 = 1000;

	/* Return a good score to make him do it */
	if (borg_simulate) {
		/* Reset list */
		borg_temp_n = 0;
		borg_tp_other_n = 0;
		return (p2);
	}

	/* Log the Path for Debug */
	borg_log_spellpath(TRUE);

#if 0
	/* Log additional info for debug */
	for (i = 0; i < borg_tp_other_n; i++)
	{
		borg_note(format("# %d, index %d (%d,%d)",borg_tp_other_n,
			borg_tp_other_index[i],	borg_tp_other_y[i],
			borg_tp_other_x[i]));
	}
#endif

	/* Cast the spell */
	if (borg_activate_artifact(ART_TRITONS, FALSE) ||
		 borg_activate_activation(ACT_TELE_AWAY, FALSE) ||
		 borg_zap_rod(SV_ROD_TELEPORT_AWAY) ||
		 borg_aim_wand(SV_WAND_TELEPORT_AWAY) ||
		 borg_spell(REALM_SORCERY, 1, 4) || borg_spell(REALM_ARCANE, 3, 3) ||
		 borg_spell(REALM_CHAOS, 1, 5)) {
		/* Use target */
		borg_keypress('5');

		/* Set our shooting flag */
		successful_target = -1;

		/* Remove the monster from this grid */
		for (i = 0; i < borg_tp_other_n; i++) {
			if (borg_grids[borg_tp_other_y[i]][borg_tp_other_x[i]].kill) {
				borg_relocate_kill(borg_tp_other_index[i]);
			}
		}

		/* Reset list */
		borg_temp_n = 0;
		borg_tp_other_n = 0;

		/* Value */
		return (p2);
	}

	return (0);
}

/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_banishment_lucifer(void) {
	int fail_allowed = 50;
	int i, x, y;
	int count = 0;
	int glyphs = 0;

	borg_grid *ag;
	borg_kill *kill;
	monster_race *r_ptr;

	/* Not if Lucifer is not on this level */
	if (!(borg_depth & (DEPTH_BORER)))
		return (0);

	/* Cant when screwed */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* I have to actually be able to make a sea of runes */
	if (borg_skill[BI_AGENOCIDE] < 10)
		return (0);

	/* Scan grids looking for glyphs */
	for (i = 0; i < 8; i++) {
		/* Access offset */
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Check for Glyphs */
		if (ag->feat == FEAT_GLYPH)
			glyphs++;
	}

	/* Only if in a sea of runes or
	 * in the process of building one
	 */
	if (!(borg_position & POSITION_SEA) &&
		 !(borg_depth & (DEPTH_BORER)) /* && glyphs < 3 */) {
		return (0);
	}

	/* Do I have the spell? (Banish Evil) */
	if (!borg_spell_okay_fail(REALM_LIFE, 2, 5, fail_allowed) &&
		 !borg_mindcr_okay_fail(MIND_TELE_WAVE, 28, fail_allowed) &&
		 !borg_equips_activation(ACT_BANISH_EVIL, TRUE))
		return (0);

	/* Nobody around so dont worry */
	if (!borg_kills_cnt && borg_simulate)
		return (0);

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never try on non-evil guys except MC who can Wave everyone. */
		if (borg_class != CLASS_ORPHIC && !(r_ptr->flags3 & RF3_EVIL))
			continue;

		/* Check the distance  */
		if (kill->dist > MAX_RANGE)
			continue;

		/* Monster must be LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Minimal distance. */
		if (kill->dist > 1)
			continue;

		/* Count the number of monsters too close double (it was d7)*/
		if (kill->unique || kill->questor)
			count = 8;

		/* Count the number of monster on screen */
		count++;
	}

	/* No destinations */
	if (count <= 7 && borg_simulate)
		return (0);

	/* Return a good score to make him do it */
	if (borg_simulate)
		return (1500);

	borg_note(
		 format("# Attempting to cast Banishment/Wave.  %d monsters ", count));

	/* Cast the spell */
	if (borg_spell(REALM_LIFE, 2, 5) || borg_mindcr(MIND_TELE_WAVE, 28) ||
		 borg_activate_activation(ACT_BANISH_EVIL, FALSE)) {
		/* Remove this race from the borg_kill */
		for (i = 0; i < borg_kills_nxt; i++) {

			/* Monster */
			borg_kill *kill = &borg_kills[i];
			/*monster_race *r_ptr = &r_info[kill->r_idx];*/

			/* Cant kill uniques like this */
			if (kill->unique)
				continue;

			/* remove this monster */
			borg_delete_kill(i);
		}

		/* Value */
		return (1000);
	}

	return (0);
}

/*
 * Sometimes the borg will not fire on Lucifer/borer as he approaches
 * while tunneling through rock.  The borg still remembers and
 * assumes that the rock is unknown grid.
 */
#if 0
static int borg_defend_aux_light_lucifer(void)
{
    int fail_allowed = 50;
    int i, x, y;
    int b_y = -1;
    int b_x = -1;
	int count = 0;

    borg_grid *ag;
    borg_kill *kill;
    monster_race *r_ptr;

	/* Only if on level 100 and in a sea of runes */
	if (!(borg_position & (POSITION_SEA | POSITION_BORE))) return (0);

	/* Not if Lucifer is not on this level */
	if (!(borg_depth & (DEPTH_BORER))) return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

	/* Do I have the spell? Light Beam*/
    if (!borg_spell_okay_fail(REALM_NATURE, 1, 4, fail_allowed) &&
        !borg_spell_okay_fail(REALM_ARCANE, 2, 5, fail_allowed) &&
		!borg_equips_activation(ACT_SUNLIGHT, TRUE)) return (0);

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate) return (0);

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        /* Monster */
        kill = &borg_kills[i];
	    r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip non- Lucifer monsters */
	    if (kill->r_idx != RACE_LUCIFER) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Check the distance  */
        if (kill->dist > MAX_RANGE) continue;
        if (kill->dist <= 5) continue;

        /* We want at least one dark spot on the path */
        if (!borg_projectable_dark(c_y, c_x, y, x)) continue;

		/* Count Lucifer so I try the spell */
		count ++;
		b_y = y;
		b_x = x;
    }

    /* No destinations */
    if (count <= 0 && borg_simulate) return (0);

	/* Return a good score to make him do it */
    if (borg_simulate) return (500);

	borg_note(format("# Attempting to Illuminate a Pathway to (%d, %d)",b_y,b_x));

	/* Target Lucifer Grid */
	(void)borg_target(b_y,b_x);

    /* Cast the spell */
    if (borg_spell(REALM_NATURE, 1, 4) ||
        borg_spell(REALM_ARCANE, 2, 5) ||
		borg_activate_activation(ACT_SUNLIGHT, FALSE)) return (0);
    {
		/* Select the target */
		borg_keypress('5');
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;

        /* Value */
        return (200);
    }

    return (0);
}
#endif

/*
 * If in a hallway, try to Sleep creatures near by so I can sneak away
 */
static int borg_defend_aux_hall_sleep(int p1) {
	int p2 = 0, b_n = 0;
	int fail_allowed = 30;
	bool spell_ok;
	int i, x, y;
	bool hallway = FALSE;

	borg_grid *ag;
	/*monster_race *r_ptr;*/

	/* Only if scared */
	if (p1 < avoidance * 6 / 10)
		return (0);

	spell_ok = FALSE;

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 18;
	else
		 /* scary */
		 if (p1 > avoidance * 3)
		fail_allowed -= 12;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 5) / 2)
		fail_allowed += 5;

	/* If I can put a hall-way critter to sleep, thats a better option */
	if (borg_spell_okay_fail(REALM_SORCERY, 0, 6, fail_allowed) ||
		 borg_spell_okay_fail(REALM_DEATH, 0, 4, fail_allowed) ||
		 borg_equips_rod(SV_ROD_SLEEP_MONSTER) ||
		 borg_equips_staff_fail(SV_STAFF_SLEEP_MONSTERS) ||
		 borg_equips_wand(SV_WAND_SLEEP_MONSTER))
		spell_ok = TRUE;
	if (!spell_ok)
		return (0);

	/* Need to be in a hallway for this maneuver. */
	/* Case 1a: north-south corridor */
	if (borg_cave_floor_bold(c_y - 1, c_x) &&
		 borg_cave_floor_bold(c_y + 1, c_x) &&
		 !borg_cave_floor_bold(c_y, c_x - 1) &&
		 !borg_cave_floor_bold(c_y, c_x + 1) &&
		 !borg_cave_floor_bold(c_y + 1, c_x - 1) &&
		 !borg_cave_floor_bold(c_y + 1, c_x + 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x - 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1b: east-west corridor */
	if (borg_cave_floor_bold(c_y, c_x - 1) &&
		 borg_cave_floor_bold(c_y, c_x + 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x) &&
		 !borg_cave_floor_bold(c_y + 1, c_x) &&
		 !borg_cave_floor_bold(c_y + 1, c_x - 1) &&
		 !borg_cave_floor_bold(c_y + 1, c_x + 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x - 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1aa: north-south doorway */
	if (borg_cave_floor_bold(c_y - 1, c_x) &&
		 borg_cave_floor_bold(c_y + 1, c_x) &&
		 !borg_cave_floor_bold(c_y, c_x - 1) &&
		 !borg_cave_floor_bold(c_y, c_x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1ba: east-west doorway */
	if (borg_cave_floor_bold(c_y, c_x - 1) &&
		 borg_cave_floor_bold(c_y, c_x + 1) &&
		 !borg_cave_floor_bold(c_y - 1, c_x) &&
		 !borg_cave_floor_bold(c_y + 1, c_x)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* not in a hallway */
	if (!hallway)
		return (0);

	/* Borg_temp_n temporarily stores several things.
	 * Some of the borg_attack() sub-routines use these numbers,
	 * which would have been filled in borg_attack().
	 * Since this is a defence manuever which will move into
	 * and borrow some of the borg_attack() subroutines, we need
	 * to make sure that the borg_temp_n arrays are properly
	 * filled.  Otherwise, the borg will attempt to consider
	 * these grids which were left filled by some other routine.
	 * Which was probably a flow routine which stored about 200
	 * grids into the array.
	 * Any change in inclusion/exclusion criteria for filling this
	 * array in borg_attack() should be included here also.
	 */
	/* Nobody around so dont worry */
	if (!borg_kills_cnt && borg_simulate)
		return (0);

	/* Reset list */
	borg_temp_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;
		if ((ag->feat >= FEAT_RUBBLE) && (ag->feat <= FEAT_PERM_SOLID))
			continue;

		/* Check the distance XXX XXX XXX */
		if (kill->dist > MAX_RANGE)
			continue;

		/* Skip Questors.  The borg is all in when it comes to them */
		if (kill->questor)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No targets for me. */
	if (!borg_temp_n && borg_simulate)
		return (0);

	/* Will a sleep spell do damage? */
	if (borg_simulate) {
		p2 = (borg_attack_aux(BF_DEATH_SLEEP_I, TRUE, -1) +
				borg_attack_aux(BF_ROD_SLEEP_MONSTER, TRUE, -1) +
				borg_attack_aux(BF_SORC_SLEEP_I, TRUE, -1) +
				borg_attack_aux(BF_STAFF_SLEEP_MONSTERS, TRUE, -1) +
				borg_attack_aux(BF_WAND_SLEEP_MONSTER, TRUE, -1));
	}

	/* check to see if I am left better off */
	if (borg_simulate) {
		/* Reset list */
		borg_temp_n = 0;

		if (p2 >= 1) {
			/* Simulation */
			return (p2);
		} else
			return (0);
	}

	/* Cast the spell */
	if (borg_attack_aux(BF_DEATH_SLEEP_I, TRUE, -1) ||
		 borg_attack_aux(BF_ROD_SLEEP_MONSTER, TRUE, -1) ||
		 borg_attack_aux(BF_SORC_SLEEP_I, TRUE, -1) ||
		 borg_attack_aux(BF_STAFF_SLEEP_MONSTERS, TRUE, -1) ||
		 borg_attack_aux(BF_WAND_SLEEP_MONSTER, TRUE, -1)) {
		/* Use target */
		borg_keypress('5');

		/* Set our shooting flag */
		successful_target = -1;

		/* Value */
		return (b_n);
	}

	return (0);
}

/*
 * Hero to prepare for battle
 */
static int borg_defend_aux_hero(int p1) {
	int fail_allowed = 10;

	/* already hero */
	if (borg_hero || borg_berserk)
		return (0);

	if (!borg_spell_okay_fail(REALM_LIFE, 3, 0, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 0, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 3, fail_allowed) &&
		 !borg_racial_check(RACE_HALF_TROLL, fail_allowed) &&
		 !borg_mutation(COR1_BERSERK, TRUE, fail_allowed, FALSE) &&
		 !borg_racial_check(RACE_BARBARIAN, fail_allowed) &&
		 !borg_equips_activation(ACT_BERSERK, TRUE) &&
		 !borg_mindcr_okay_fail(MIND_ADRENALINE, 23, fail_allowed) &&
		 -1 == borg_slot(TV_POTION, SV_POTION_HEROISM))
		return (0);

	/* if we are in some danger but not much, go for a quick bless */
	if (borg_goi || borg_wraith || (p1 > avoidance / 12 && p1 < avoidance / 2) ||
		 (borg_fighting_unique && p1 < avoidance * 13 / 10) ||
		 borg_surrounded() ||
		 (p1 >= avoidance / 20 && borg_class == CLASS_RANGER)) {
		/* Simulation */
		/* hero is a low priority */
		if (borg_simulate)
			return (1);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_activate_activation(ACT_BERSERK, FALSE) ||
			 borg_spell(REALM_LIFE, 3, 0) || borg_spell(REALM_DEATH, 2, 3) ||
			 borg_spell(REALM_DEATH, 2, 0) || borg_mindcr(MIND_ADRENALINE, 23) ||
			 borg_racial(RACE_HALF_TROLL, 1) || borg_racial(RACE_BARBARIAN, 1) ||
			 borg_mutation(COR1_BERSERK, FALSE, 50, FALSE) ||
			 borg_quaff_potion(SV_POTION_HEROISM))
			return 1;
	}

	return (0);
}

/*
 * Hero to prepare for battle
 */
static int borg_defend_aux_berserk(int p1) {
	/* already hero */
	if (borg_hero || borg_berserk)
		return (0);

	if (-1 == borg_slot(TV_POTION, SV_POTION_BESERK_STRENGTH) &&
		 !borg_mutation(COR1_BERSERK, TRUE, 30, FALSE) &&
		 !borg_racial_check(RACE_BARBARIAN, 30) &&
		 !borg_racial_check(RACE_HALF_TROLL, 30))
		return (0);

	/* if we are in some danger but not much, go for a quick bless */
	if (borg_goi || borg_wraith || (p1 > avoidance / 12 && p1 < avoidance / 2) ||
		 (borg_fighting_unique && p1 < avoidance * 13 / 10) ||
		 borg_surrounded() ||
		 (p1 >= avoidance / 20 && borg_class == CLASS_RANGER)) {
		/* Simulation */
		/* berserk is a low priority */
		if (borg_simulate)
			return (5);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_mutation(COR1_BERSERK, FALSE, 31, FALSE) ||
			 borg_racial(RACE_BARBARIAN, 1) || borg_racial(RACE_HALF_TROLL, 1) ||
			 borg_quaff_potion(SV_POTION_BESERK_STRENGTH))
			return 2;
	}

	return (0);
}

/* Glyph of Warding and Rune of Protection */
static int borg_defend_aux_glyph(int p1) {
	int p2 = 0, i;
	int fail_allowed = 30;
	bool glyph_spell = FALSE;
	int adjacent = 0;

	borg_grid *ag = &borg_grids[c_y][c_x];

	/* He should not cast it while on an object.
	 * I have addressed this inadequately in borg9.c when dealing with
	 * messages.  The message "the object resists" will delete the glyph
	 * from the array.  Then I set a broken door on that spot, the borg ignores
	 * broken doors, so he won't loop.
	 */

	if ((ag->take) || (ag->feat == FEAT_MINOR_GLYPH) ||
		 (ag->feat == FEAT_GLYPH) ||
		 ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP)) ||
		 ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_TAIL)) ||
		 (ag->feat == FEAT_LESS) || (ag->feat == FEAT_MORE) ||
		 (ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN) ||
		 (ag->feat >= FEAT_SECRET && ag->feat <= FEAT_BLDG_TAIL)) {
		return (0);
	}

	/* Lucifer breaks these in one try so its a waste of mana against him */
	if (borg_fighting_unique == RACE_LUCIFER &&
		 !borg_racial_check(RACE_HALF_OGRE, 25))
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 20;

	if (borg_spell_okay_fail(REALM_LIFE, 1, 7, fail_allowed) ||
		 borg_spell_okay_fail(REALM_SORCERY, 3, 2, fail_allowed) ||
		 borg_equips_activation(ACT_RUNE_EXPLO, TRUE) ||
		 borg_equips_activation(ACT_RUNE_PROT, TRUE) ||
		 borg_racial_check(RACE_HALF_OGRE, 25))
		glyph_spell = TRUE;

	if (0 <= borg_slot(TV_SCROLL, SV_SCROLL_RUNE_OF_PROTECTION))
		glyph_spell = TRUE;

	if ((borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		  borg_skill[BI_ISIMAGE]) &&
		 glyph_spell)
		glyph_spell = FALSE;
	if (!(ag->info & BORG_GLOW) && borg_skill[BI_CUR_LITE] == 0 &&
		 !borg_racial_check(RACE_HALF_OGRE, 25))
		glyph_spell = FALSE;

	if (!glyph_spell)
		return (0);

	/* pretend we are protected and look again */
	borg_on_glyph = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_on_glyph = FALSE;

	/* Count the adjacent monsters for the explosive rune */
	if (p2 <= avoidance && borg_racial_check(RACE_HALF_OGRE, 25)) {
		/* Find "nearby" monsters */
		for (i = 1; i < borg_kills_nxt; i++) {
			borg_kill *kill;

			/* Monster */
			kill = &borg_kills[i];

			/* Skip dead monsters */
			if (!kill->r_idx)
				continue;

			/* Require current knowledge */
			if (kill->when < borg_t - 2)
				continue;

			/* Skip pets and friendly */
			if (kill->ally)
				continue;

			/* be close */
			if (kill->dist <= 1)
				adjacent++;
		}

		/* Bonus for using explosive runes when surrouned */
		p2 = p2 / (adjacent + 1);
	}

	/* Dont use this defence if the monster can use teleport-to and move the borg
	 */
	if (p2 <= avoidance) {
		/* Find "nearby" monsters */
		for (i = 1; i < borg_kills_nxt; i++) {
			borg_kill *kill;
			monster_race *r_ptr;

			/* Monster */
			kill = &borg_kills[i];
			r_ptr = &r_info[kill->r_idx];

			/* Skip dead monsters */
			if (!kill->r_idx)
				continue;

			/* Skip ones who don't have the spell */
			if (!(r_ptr->flags6 & RF6_TELE_TO))
				continue;

			/* Must be able to use LOS spell against the borg */
			if (!kill->los)
				continue;

			/* No advantage to using this since the monster might t-port the borg
			 * off the site. */
			p2 = p1;
		}
	}

	/* if this is an improvement and we may not avoid monster now and
	  * we may have before
	 * In an A-S corridor, good idea.
	 */
	if ((p1 > p2 &&
		  p2 <=
				(borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		  p1 > (avoidance / 7)) ||
		 ((borg_position & POSITION_SUMM) && borg_t - borg_t_position < 50)) {
		/* Simulation */
		if (borg_simulate) {
			/* AS corridor, need a higher reward than borg_defend_aux_rest() */
			if ((borg_position & POSITION_SUMM) && borg_t - borg_t_position < 50)
				return (205);
			else
				return (p1 - p2);
		}

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_racial(RACE_HALF_OGRE, 1) ||
			 borg_spell_fail(REALM_LIFE, 1, 7, fail_allowed) ||
			 borg_spell_fail(REALM_SORCERY, 3, 2, fail_allowed) ||
			 borg_read_scroll(SV_SCROLL_RUNE_OF_PROTECTION) ||
			 borg_activate_activation(ACT_RUNE_EXPLO, FALSE) ||
			 borg_activate_activation(ACT_RUNE_PROT, FALSE)) {
			/* Check for an existing glyph */
			for (i = 0; i < track_glyph_num; i++) {
				/* Stop if we already new about this glyph */
				if ((track_glyph_x[i] == c_x) && (track_glyph_y[i] == c_y))
					return (p1 - p2);
			}

			/* Track the newly discovered glyph */
			borg_note("# Noting the creation of a glyph.");
			track_glyph_x[track_glyph_num] = c_x;
			track_glyph_y[track_glyph_num] = c_y;
			track_glyph_num++;

			return (p1 - p2);
		}
	}

	/* default to can't do it. */
	return (0);
}

/* True Warding */
static int borg_defend_aux_true_warding(int p1) {
	int p2 = 0;
	int fail_allowed = 30;
	int glyph_bad = 0;
	int tglyph_x, tglyph_y, x, y;

	borg_grid *ag;

	/* any summoners near?*/
	if (!borg_fighting_summoner)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 20;

	if (!borg_spell_okay_fail(REALM_LIFE, 2, 7, fail_allowed))
		return (0);

	/* Do not cast if surounded by doors or something */
	/* Get grid */
	for (tglyph_x = -1; tglyph_x <= 1; tglyph_x++) {
		for (tglyph_y = -1; tglyph_y <= 1; tglyph_y++) {
			/* Acquire location */
			x = tglyph_x + c_x;
			y = tglyph_y + c_y;

			ag = &borg_grids[y][x];

			/* track spaces already protected */
			if ((ag->feat == FEAT_GLYPH) || (ag->feat == FEAT_MINOR_GLYPH) ||
				 ag->kill ||
				 ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_PERM_SOLID))) {
				glyph_bad++;
			}

			/* track spaces that cannot be protected */
			if ((ag->take) || ((ag->feat >= FEAT_TRAP_TRAPDOOR) &&
									 (ag->feat <= FEAT_TRAP_SLEEP)) ||
				 (ag->feat == FEAT_LESS) || (ag->feat == FEAT_MORE) ||
				 (ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN) ||
				 (ag->kill)) {
				glyph_bad++;
			}
		}
	}

	/* Track it */
	/* lets make sure that we going to be benifited */
	if (glyph_bad >= 6) {
		/* not really worth it.  Only 2 spaces protected */
		return (0);
	}

	/* pretend we are protected and look again (use the door code) */
	borg_create_door = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_create_door = FALSE;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* do it! */
		if (borg_spell_fail(REALM_LIFE, 2, 7, fail_allowed)) {
			/* Set the breeder flag to keep doors closed. Avoid summons */
			borg_depth |= DEPTH_BREEDER;

			/* Value */
			return (p1 - p2);
		}
	}

	/* default to can't do it. */
	return (0);
}

/* Create Granite Walls-- Nature spell */
static int borg_defend_aux_create_walls(int p1) {
	int p2 = 0;
	int fail_allowed = 30;
	int wall_bad = 0;
	int wall_x, wall_y, x, y;

	borg_grid *ag;

	/* any summoners near? or am a really low on HP */
	if (!borg_fighting_summoner &&
		 borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 3)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 5;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 20;

	if (!borg_spell_okay_fail(REALM_NATURE, 2, 6, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_NATURE, 2, 0, fail_allowed))
		return (0);

	/* Do not cast if surounded by doors or something */
	/* Get grid */
	for (wall_x = -1; wall_x <= 1; wall_x++) {
		for (wall_y = -1; wall_y <= 1; wall_y++) {
			/* Acquire location */
			x = wall_x + c_x;
			y = wall_y + c_y;

			ag = &borg_grids[y][x];

			/* track spaces already protected */
			if (ag->feat == FEAT_GLYPH || ag->feat == FEAT_MINOR_GLYPH ||
				 ag->kill ||
				 (ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_PERM_SOLID)) {
				wall_bad++;
			}

			/* track spaces that cannot be protected */
			if ((ag->take) || ((ag->feat >= FEAT_TRAP_TRAPDOOR) &&
									 (ag->feat <= FEAT_TRAP_SLEEP)) ||
				 (ag->feat == FEAT_LESS) || (ag->feat == FEAT_MORE) ||
				 (ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN) ||
				 (ag->kill)) {
				wall_bad++;
			}
		}
	}

	/* lets make sure that we going to be benifited */
	if (wall_bad >= 6) {
		/* not really worth it.  Only 2 spaces protected */
		return (0);
	}

	/* pretend we are protected and look again */
	borg_create_door = TRUE;
	p2 = borg_danger(c_y, c_x, 1, TRUE);
	borg_create_door = FALSE;

	/* If recalling, then it is even better.  I can hide here until the recall
	 * engages */
	if (goal_recalling)
		p2 /= 4;

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 7)) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* do it! */
		if ((!goal_recalling &&
			  borg_spell_fail(REALM_NATURE, 2, 0, fail_allowed)) ||
			 (goal_recalling &&
			  borg_spell_fail(REALM_NATURE, 2, 6, fail_allowed)) ||
			 (borg_spell_fail(REALM_NATURE, 2, 0, fail_allowed)) ||
			 (borg_spell_fail(REALM_NATURE, 2, 6, fail_allowed))) {
			/* Set the breeder flag to keep doors closed. Avoid summons */
			borg_depth |= DEPTH_BREEDER;

			/* Must make a new Sea too */
			borg_needs_new_sea = TRUE;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			/* Value */
			return (p1 - p2);
		}
	}

	/* default to can't do it. */
	return (0);
}

/* This will simulate and cast the Day of the Dove as a defense maneuver.
 * It is also an attack, but if the borg is in danger, he will perfrom a
 * defence or teleport away before doing an attack.
 */
static int borg_defend_aux_day_dove(int p1) {
	int i = 0, p2;
	int b_p = 0, p;

	/*borg_grid *ag;*/
	borg_kill *kill;
	monster_race *r_ptr;

	/* Can't charm if aggrevating */
	if (borg_skill[BI_CRSAGRV])
		return (0);

	/* see if prayer is legal */
	if (!borg_spell_okay_fail(REALM_LIFE, 2, 3, 40))
		return (0);

	/* See if he is in real danger, or surrounded */
	if (p1 < avoidance * 7 / 10 && !borg_surrounded())
		return (0);

	/* Find a monster and calculate its danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Check the projectable */
		if (!kill->los)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Unlikely to charm this monster. */
		if (kill->unique || kill->questor || (r_ptr->flags3 & RF3_NO_CONF) ||
			 (r_ptr->level > borg_skill[BI_CLEVEL] + 5))
			continue;

		/* Calculate danger */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* store the danger for this individual of monster */
		b_p = b_p + p;
	}

	/* normalize the value */
	if (b_p >= p1 * 9 / 10 && b_p > borg_skill[BI_CURHP])
		b_p = p1 * 8 / 10;
	p2 = (p1 - b_p);
	if (p2 < 0)
		p2 = 0;

	/* Be more likely to use this if fighting Lucifer */
	if (borg_fighting_unique == RACE_LUCIFER ||
		 borg_fighting_unique == RACE_LILITH) {
		p2 = p2 * 6 / 10;
	}

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? (avoidance * 2 / 3) : (avoidance / 2))) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* Cast the spell */
		if (borg_spell(REALM_LIFE, 2, 3)) {
			/* Value */
			return (p1 - p2);
		}
	}
	/* Not worth it */
	return (0);
}
/* This will simulate and cast the mass genocide spell.
 */
static int borg_defend_aux_mass_genocide(void) {
	int p1 = 0, hit = 0, i = 0, p2;
	int b_p = 0, p;
	int x, y;
	int q_x = w_x / 33;
	int q_y = w_y / 11;
	bool omnicide = FALSE;

	/*borg_grid *ag;*/
	borg_kill *kill;
	/*monster_race *r_ptr;*/

	/* see if prayer is legal */
	if (!borg_spell_okay_fail(REALM_DEATH, 2, 7, 40) &&
		 !borg_spell_okay_fail(REALM_DEATH, 3, 6, 40) &&
		 !borg_equips_artifact(ART_TROLLS) &&
		 !borg_equips_activation(ACT_MASS_GENO, TRUE))
		return (0);

	/* Omnicide works a little differently */
	if (borg_spell_okay_fail(REALM_DEATH, 3, 6, 40))
		omnicide = TRUE;

	/* Obtain initial danger, measured over time*/
	p1 = borg_danger(c_y, c_x, 1, TRUE);

	/* See if he is in real danger */
	if (p1 < avoidance * 12 / 10)
		return (0);

	/* Cant inside a quest level
	if (borg_skill[BI_INSIDEQUEST])
		return (0);
	*/

	/* Borg needs a recent idea of how many monsters will be affected */
	if (!borg_skill[BI_ESP] && (!borg_detect_evil[q_y + 0][q_x + 0] &&
										 !borg_detect_evil[q_y + 0][q_x + 1] &&
										 !borg_detect_evil[q_y + 1][q_x + 0] &&
										 !borg_detect_evil[q_y + 1][q_x + 1]) &&
		 (borg_t - when_detect_evil >= 25))
		return (0);

	/* Find a monster and calculate its danger */
	for (i = 0; i < borg_kills_nxt; i++) {

		/* Monster */
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Check the distance */
		if (kill->dist > 20)
			continue;

		/* Omnicide behaves differently.  The monster must also be LOS */
		if (omnicide && !kill->los)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* we try not to genocide uniques and questors */
		if (kill->unique || kill->questor)
			continue;

		/* Calculate danger */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* store the danger for this type of monster */
		b_p = b_p + p;
		hit = hit + 4;
	}

	/* normalize the value */
	p2 = (p1 - b_p);
	if (p2 < 0)
		p2 = 0;

	/* if strain (plus a pad incase we did not know about some monsters)
	 * is greater than hp, don't cast it
	 */
	if ((hit * 11 / 10) >= borg_skill[BI_CURHP])
		return (0);

	/* Penalize the strain from casting the spell */
	p2 = p2 + hit;

	/* Be more likely to use this if fighting the final two */
	if ((borg_fighting_unique == RACE_LUCIFER ||
		  borg_fighting_unique == RACE_LILITH) &&
		 (hit / 3 > 8)) {
		p2 = p2 * 6 / 10;
	}

	/* if this is an improvement and we may not avoid monster now and */
	/* we may have before */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? (avoidance * 2 / 3) : (avoidance / 2))) {
		/* Simulation */
		if (borg_simulate)
			return (p1 - p2);

		/* Cast the spell */
		if (borg_spell(REALM_DEATH, 3, 6) || borg_spell(REALM_DEATH, 2, 7) ||
			 borg_activate_artifact(ART_TROLLS, FALSE) ||
			 borg_activate_activation(ACT_MASS_GENO, FALSE)) {
			/* Remove monsters */
			for (i = 1; i < borg_kills_nxt; i++) {
				borg_kill *kill;

				/* Monster */
				kill = &borg_kills[i];
				/*monster_race *r_ptr = &r_info[kill->r_idx];*/

				/* Our char of the monster */
				if (kill->dist > MAX_RANGE)
					continue;

				/* we do not genocide uniques */
				if (kill->unique || kill->questor)
					continue;

				/* remove this monster */
				borg_delete_kill(i);
			}

			/* Remove regional fear induced from unseen monsters */
			/* Hack -- Clear "fear" */
			for (y = 0; y < 6; y++) {
				for (x = 0; x < 18; x++) {
					borg_fear_region[y][x] = 0;
				}
			}

			/* Remove regional fear from monsters, it gets added back in later. */
			for (y = 0; y < AUTO_MAX_Y; y++) {
				for (x = 0; x < AUTO_MAX_X; x++) {
					borg_fear_monsters[y][x] = 0;
				}
			}

			/* Value */
			return (p1 - p2);
		}
	}
	/* Not worth it */
	return (0);
}

/* This will simulate and cast the genocide spell.
 * There are two seperate functions happening here.
 * 1. will genocide the race which is immediately threatening me.
 * 2. will genocide the race which is most dangerous on the level.  Though it
 * may not be
 *    threatening the borg right now.  It was considered to nuke the escorts of
 * a unique.
 *    But it could also be used to nuke a race if it becomes too dangerous, for
 * example
 *    a summoner called up 15-20 hounds, and they must be dealt with.
 * The first option may be called at any time.  While the 2nd option is only
 * called when the
 * borg is in relatively good health.
 */
static int borg_defend_aux_genocide(int p1) {
	int i, p, u, b_i = 0;
	int p2 = 0;
	int threat = 0;
	int max = 1;

	int b_p[256];
	int b_num[256];
	int b_threat[256];
	int b_threat_num[256];

	int total_danger_to_me = 0;

	char genocide_target = (char)0;
	char b_threat_id = (char)0;

	/*borg_grid *ag;*/

	bool genocide_spell = FALSE;
	int fail_allowed = 25;

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance)
		fail_allowed -= 19;
	else
		 /* a little scary */
		 if (p1 > (avoidance * 2) / 3)
		fail_allowed -= 10;
	else
		 /* not very scary, allow lots of fail */
		 if (p1 < avoidance / 3)
		fail_allowed += 10;

	/* Cant when screwed */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* Cant inside a quest level
	if (borg_skill[BI_INSIDEQUEST])
		return (0);
	*/

	/* Normalize the p1 value.  It contains danger added from
	 * regional fear and monster fear.  Which wont be counted
	 * in the post-genocide checks
	 */
	if (borg_fear_region[c_y / 11][c_x / 11])
		p1 -= borg_fear_region[c_y / 11][c_x / 11];
	if (borg_fear_monsters[c_y][c_x])
		p1 -= borg_fear_monsters[c_y][c_x];

	/* Make sure I have the spell */
	if (borg_spell_okay_fail(REALM_DEATH, 1, 6, fail_allowed) ||
		 /*borg_equips_artifact(ART_CELEBORN) || */
		 borg_equips_activation(ACT_GENOCIDE, TRUE) ||
		 borg_equips_staff_fail(SV_STAFF_GENOCIDE) ||
		 (-1 != borg_slot(TV_SCROLL, SV_SCROLL_GENOCIDE))) {
		genocide_spell = TRUE;
	}

	if (genocide_spell == FALSE)
		return (0);

	/* Don't try it if really weak */
	if (borg_skill[BI_CURHP] <= 75)
		return (0);

	/* two methods to calculate the threat:
	 *1. cycle each character of monsters on screen
	 *   collect collective threat of each char
	 *2  select race of most dangerous guy, and choose him.
	 * Method 2 is cheaper and faster.
	 *
	 * The borg uses method #1
	 */

	/* Clear previous dangers */
	for (i = 0; i < 256; i++) {
		b_p[i] = 0;
		b_num[i] = 0;
		b_threat[i] = 0;
		b_threat_num[i] = 0;
	}

	/* Find a monster and calculate its danger */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;
		monster_race *r_ptr;

		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Our char of the monster */
		u = r_ptr->d_char;

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* we try not to genocide uniques */
		if (kill->unique || kill->questor)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Calculate danger */
		/* Danger to me by this monster */
		p = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);

		/* Danger of this monster to his own grid */
		threat = borg_danger_aux(kill->y, kill->x, 1, i, TRUE, FALSE);

		/* store the danger for this type of monster */
		b_p[u] = b_p[u] + p; /* Danger to me */
		total_danger_to_me += p;
		b_threat[u] = b_threat[u] + threat; /* Danger to monsters grid */

		/* Store the number of this type of monster */
		b_num[u]++;
		b_threat_num[u]++;
	}

	/* Now, see which race contributes the most danger
	 * both to me and danger on the level
	 */

	for (i = 0; i < 256; i++) {

		/* skip the empty ones */
		if (b_num[i] == 0 && b_threat_num[i] == 0)
			continue;

		/* for the race threatening me right now */
		if (b_p[i] > max) {
			/* track the race */
			max = b_p[i];
			b_i = i;

			/* note the danger with this race gone.  Note that the borg does max
			 * his danger
			 * at 2000 points.  It could be much, much higher at depth 99 or so.
			 * What the borg should do is recalculate the danger without
			 * considering this monster
			 * instead of this hack which does not yeild the true danger.
			 */
			p2 = total_danger_to_me - b_p[b_i];
		}

		/* for this race on the whole level */
		if (b_threat[i] > max) {
			/* track the race */
			max = b_threat[i];
			b_threat_id = i;
		}

		/* Leave an interesting note for debugging */
		if (!borg_simulate)
			borg_note(format("# Race '%c' is a threat with total danger %d from "
								  "%d individuals.",
								  i, b_threat[i], b_threat_num[i]));
	}

	/* This will track and decide if it is worth genociding this dangerous race
	 * for the level */
	if (b_threat_id) {
		/* Not if I am weak (should have 400 HP really in case of a Pit) */
		if (borg_skill[BI_CURHP] < 375)
			b_threat_id = 0;

		/* The threat must be real */
		if (b_threat[(int)b_threat_id] < borg_skill[BI_MAXHP] * 3)
			b_threat_id = 0;

		/* Too painful to cast it (padded to be safe incase of unknown monsters)
		 */
		if ((b_num[(int)b_threat_id] * 4) * 12 / 10 >= borg_skill[BI_CURHP])
			b_threat_id = 0;

		/* Loads of monsters might be a pit, in which case, try not to nuke them
		 */
		if (b_num[(int)b_threat_id] >= 75)
			b_threat_id = 0;

		/* Do not perform in Danger */
		if (borg_danger(c_y, c_x, 1, TRUE) > avoidance / 5)
			b_threat_id = 0;

		/* report the danger and most dangerous race */
		if (b_threat_id) {
			borg_note(format("# Race '%c' is a real threat with total danger %d "
								  "from %d individuals.",
								  b_threat_id, b_threat[(int)b_threat_id],
								  b_threat_num[(int)b_threat_id]));
		}

		/* Genociding this race would reduce the danger of the level */
		genocide_target = b_threat_id;
	}

	/* Consider the immediate threat genocide */
	if (b_i) {
		/* Too painful to cast it (padded to be safe incase of unknown monsters)
		 */
		if ((b_num[b_i] * 4) * 12 / 10 >= borg_skill[BI_CURHP])
			b_i = 0;

		/* See if he is in real danger, generally,
		 * or deeper in the dungeon, conservatively,
		 */
		if (p1 < avoidance * 7 / 10 ||
			 (borg_skill[BI_CDEPTH] > 75 && p1 < avoidance * 6 / 10))
			b_i = 0;

		/* Did this help improve my situation? */
		if (p2 <= (avoidance / 2))
			b_i = 0;

		/* Genociding this race would help me immediately */
		genocide_target = b_i;
	}

	/* Complete the genocide routine */
	if (genocide_target) {
		if (borg_simulate) {
			/* Simulation for immediate threat */
			if (b_i)
				return (p1 - p2);

			/* Simulation for immediate threat */
			if (b_threat_id)
				return (b_threat[(int)b_threat_id]);
		}

		if (b_i)
			borg_note(
				 format("# Banishing race '%c' (qty:%d).  Danger after spell:%d",
						  genocide_target, b_num[b_i], p2));
		if (b_threat_id)
			borg_note(
				 format("# Banishing race '%c' (qty:%d).  Danger from them:%d",
						  genocide_target, b_threat_num[(int)b_threat_id],
						  b_threat[(int)b_threat_id]));

		/* do it! ---use scrolls first since they clutter inventory */
		if (borg_read_scroll(SV_SCROLL_GENOCIDE) ||
			 borg_activate_activation(ACT_GENOCIDE, FALSE) ||
			 borg_spell(REALM_DEATH, 1, 6) ||
			 /*borg_activate_artifact(ART_CELEBORN, INVEN_BODY) ||*/
			 borg_use_staff(SV_STAFF_GENOCIDE)) {
			/* and the winner is.....*/
			borg_keypress((genocide_target));
		}

		/* Remove this race from the borg_kill */
		for (i = 1; i < borg_kills_nxt; i++) {
			borg_kill *kill;
			monster_race *r_ptr;

			/* Monster */
			kill = &borg_kills[i];
			r_ptr = &r_info[kill->r_idx];

			/* Our char of the monster */
			if (r_ptr->d_char != genocide_target)
				continue;

			/* we do not genocide uniques */
			if (kill->unique || kill->questor)
				continue;

			/* remove this monster */
			borg_delete_kill(i);
		}

		return (p1 - p2);
	}
	/* default to can't do it. */
	return (0);
}

/* This will cast the genocide spell on Hounds and other
 * really nasty guys like Angels, Demons, Dragons and Liches
 * at the beginning of each level or when they get too numerous.
 * The acceptable numbers are defined in borg_nasties_limit[]
 * The definition for the list is in borg1.c
 * borg_nasties[]
 *
 */
static int borg_defend_aux_genocide_nasties(int p1) {
	int i = 0;
	int b_i = -1;

	bool genocide_spell = FALSE;

	/* Not if I am weak */
	if (borg_skill[BI_CURHP] < (borg_skill[BI_MAXHP] * 7 / 10) ||
		 borg_skill[BI_CURHP] < 250)
		return (0);

	/* only do it when Hounds start to show up, */
	if (borg_skill[BI_CDEPTH] < 25)
		return (0);

	/* Cant inside a quest level
	if (borg_skill[BI_INSIDEQUEST])
		return (0);
	*/

	/* Cant when screwed */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* Do not perform in Danger */
	if (p1 > avoidance / 4)
		return (0);

	if (borg_spell_okay_fail(REALM_DEATH, 1, 6, 35) ||
		 /*borg_equips_artifact(ART_CELEBORN) ||*/
		 borg_equips_staff_fail(SV_STAFF_GENOCIDE) ||
		 borg_equips_activation(ACT_GENOCIDE, TRUE)) {
		genocide_spell = TRUE;
	}

	if (genocide_spell == FALSE)
		return (0);

	/* Find the numerous nasty in order of nastiness */
	for (i = 0; i < borg_nasties_num; i++) {
		if (borg_nasties_count[i] >= borg_nasties_limit[i])
			b_i = i;
	}

	/* Nothing good to Genocide */
	if (b_i == -1)
		return (0);

	if (borg_simulate)
		return (10);

	/* Note it */
	borg_note(format("# Banishing nasties '%c' (qty:%d).", borg_nasties[b_i],
						  borg_nasties_count[b_i]));

	/* Execute -- Nice pun*/
	if (/*borg_activate_artifact(ART_CELEBORN, INVEN_BODY) ||*/
		 borg_activate_activation(ACT_GENOCIDE, FALSE) ||
		 borg_use_staff(SV_STAFF_GENOCIDE) || borg_spell(REALM_DEATH, 1, 6)) {
		/* and the winner is.....*/
		borg_keypress(borg_nasties[b_i]);

		/* set the count to not do it again */
		borg_nasties_count[b_i] = 0;

		/* Remove this race from the borg_kill */
		for (i = 1; i < borg_kills_nxt; i++) {
			borg_kill *kill;
			monster_race *r_ptr;

			/* Monster */
			kill = &borg_kills[i];
			r_ptr = &r_info[kill->r_idx];

			/* Our char of the monster */
			if (r_ptr->d_char != borg_nasties[b_i])
				continue;

			/* remove this monster */
			borg_delete_kill(i);
		}

		return (10);
	}

	/* default to can't do it. */
	return (0);
}

/* Earthquake, priest and mage spells.
 */
static int borg_defend_aux_earthquake(void) {
	int p1 = 0;
	int p2 = 9999;
	/*int door_bad = 0;*/
	/*int door_x, door_y, x, y;*/
	int threat_count = 0;
	int i;

	/*borg_grid *ag;*/
	borg_kill *kill;

	/* Obtain initial danger */
	p1 = borg_danger(c_y, c_x, 1, TRUE);

	if (!borg_spell_okay_fail(REALM_NATURE, 3, 0, 35) &&
		 !borg_mutation(COR1_EARTHQUAKE, TRUE, 35, FALSE) &&
		 !borg_equips_activation(ACT_QUAKE, TRUE))
		return (0);

	/* See if he is in real danger or fighting summoner*/
	if (p1 < avoidance && !borg_fighting_summoner)
		return (0);

	/* Several monsters can see the borg and they have ranged attacks */
	for (i = 0; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];

		/* Look for threats */
		if (kill->los && kill->ranged_attack && kill->dist >= 2) {
			/* They can hit me */
			threat_count++;
		}
	}

	/* Real danger? */
	if (threat_count >= 4 && p1 > avoidance * 7 / 10)
		p2 = p1 / 3;
	if (threat_count == 3 && p1 > avoidance * 7 / 10)
		p2 = p1 * 6 / 10;

	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance / 5)) {
		/* Simulation */
		if (borg_simulate)
			return (p2);

		/* Cast the spell */
		if (borg_spell(REALM_NATURE, 3, 0) ||
			 borg_mutation(COR1_EARTHQUAKE, FALSE, 35, FALSE) ||
			 borg_activate_activation(ACT_QUAKE, FALSE)) {
			/* Must make a new Sea too */
			borg_needs_new_sea = TRUE;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (p2);
		}
	}
	return (0);
}

/* Word of Destruction, priest and mage spells.  Death is right around the
 *  corner, so kill everything.
 */
static int borg_defend_aux_destruction(void) {
	int p1 = 0;
	int p2 = 0;
	int d = 0;
	bool spell = FALSE;

	/* Borg_defend() is called before borg_escape().  He may have some
	 * easy ways to escape (teleport scroll/staff) but he may attempt this spell
	 * instead of using the scrolls.  So we will not allow the use
	 * of this spell if he can reasonably teleport away.
	 */
	/* Use teleport scrolls instead of WoD */
	if (borg_skill[BI_ATELEPORT] && !borg_skill[BI_ISBLIND] &&
		 !borg_skill[BI_ISCONFUSED])
		return (0);

	/* Use teleport staff instead of WoD */
	if (borg_skill[BI_AESCAPE])
		return (0);

	/* Not if in a sea of runes */
	if ((borg_position & POSITION_SEA))
		return (0);

	/* Obtain initial danger */
	p1 = borg_danger(c_y, c_x, 1, TRUE);

	if (borg_spell_okay_fail(REALM_CHAOS, 1, 6, 55) ||
		 borg_equips_staff_fail(SV_STAFF_DESTRUCTION))
		spell = TRUE;

	/* Special check for super danger--no fail check */
	if (p1 > (avoidance * 4) && borg_equips_staff_fail(SV_STAFF_DESTRUCTION))
		spell = TRUE;

	if (spell == FALSE)
		return (0);

	/* See if he is in real danger */
	if (p1 < avoidance * 2)
		return (0);

	/* What effect is there? */
	p2 = 0;

	/* value is d */
	d = (p1 - p2);

	/* Try not to cast this against uniques */
	if (borg_fighting_unique && p1 < avoidance * 5)
		d = 0;
	if (borg_fighting_unique == RACE_LUCIFER)
		d = 0;

	/* Simulation */
	if (borg_simulate)
		return (d);

	/* Cast the spell */
	if (borg_spell(REALM_CHAOS, 1, 6) || borg_use_staff(SV_STAFF_DESTRUCTION)) {
		/* Must make a new Sea too */
		borg_needs_new_sea = TRUE;
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;

		return (d);
	}

	/* oops it did not work */
	return (0);
}

static int borg_defend_aux_banish_evil(int p1) {
	int p2 = 0;
	int fail_allowed = 15;
	int i;

	/*borg_grid *ag;*/

	/* Only tell away if scared */
	if (p1 < avoidance * 12 / 10)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 10;

	if (!borg_spell_okay_fail(REALM_LIFE, 2, 5, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 1, 7, fail_allowed) &&
		 !borg_equips_activation(ACT_BANISH_EVIL, TRUE))
		return (0);

	/* reset initial danger */
	p1 = 1;

	/* Two passes to determine exact danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;
		/*monster_race *r_ptr;*/

		/* Monster */
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p1 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
	}

	/* Pass two -- Find a monster and calculate its danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;
		monster_race *r_ptr;

		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* ag = &borg_grids[kill->y][kill->x]; */

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* get rid of evil monsters*/
		if (r_ptr->flags3 & RF3_EVIL)
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p2 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
	}

	/* no negatives */
	if (p2 <= 0)
		p2 = 0;

	/* Try not to cast this against Morgy/Sauron */
	if (borg_fighting_unique == RACE_LILITH && borg_skill[BI_CURHP] > 250 &&
		 borg_skill[BI_CDEPTH] == LILITH_DEPTH)
		p2 = 9999;
	if (borg_fighting_unique == RACE_LUCIFER && borg_skill[BI_CURHP] > 350 &&
		 borg_skill[BI_CDEPTH] == LUCIFER_DEPTH)
		p2 = 9999;

	/* check to see if I am left better off */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance * 2)) {
		/* Simulation */
		if (borg_simulate)
			return (p2);

		/* Cast the spell */
		if (borg_spell_fail(REALM_LIFE, 2, 5, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 1, 7, fail_allowed) ||
			 borg_activate_activation(ACT_BANISH_EVIL, FALSE)) {
			/* Value */
			return (p2);
		}
	}
	return (0);
}

static int borg_defend_aux_banish_good(int p1) {
	int p2 = 0;
	int fail_allowed = 15;
	int i;

	/*borg_grid *ag;*/

	/* Only tell away if scared */
	if (p1 < avoidance * 12 / 10)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 10;

	if (!borg_spell_okay_fail(REALM_DEATH, 1, 5, fail_allowed))
		return (0);

	/* reset initial danger */
	p1 = 1;

	/* Two passes to determine exact danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*monster_race r_ptr = &r_info[kill->r_idx];*/

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p1 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
	}

	/* Pass two -- Find a monster and calculate its danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		monster_race *r_ptr = &r_info[kill->r_idx];

		/*ag = &borg_grids[kill->y][kill->x];*/

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* get rid of good monsters*/
		if (r_ptr->flags3 & RF3_GOOD)
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p2 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
	}

	/* no negatives */
	if (p2 <= 0)
		p2 = 0;

	/* Try not to cast this against certain uniques */
	/*
	* if (borg_fighting_unique == RACE_LILITH && borg_skill[BI_CURHP] > 250 &&
	* borg_skill[BI_CDEPTH] == LILITH_DEPTH) p2 = 9999;
	 * if (borg_fighting_unique == RACE_LUCIFER && borg_skill[BI_CURHP] > 350 &&
	* borg_skill[BI_CDEPTH] == LUCIFER_DEPTH) p2 = 9999;
	*/

	/* check to see if I am left better off */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance * 2)) {
		/* Simulation */
		if (borg_simulate)
			return (p2);

		/* Cast the spell */
		if (borg_spell_fail(REALM_DEATH, 1, 5, fail_allowed)) {
			/* Value */
			return (p2);
		}
	}
	return (0);
}

/*
 * Detect Inviso/Monsters
 * Used only if I am hit by an unseen guy.
 * Casts detect invis.
 */
static int borg_defend_aux_inviso(int p1) {
	int fail_allowed = 25;
	/*borg_grid *ag = &borg_grids[c_y][c_x];*/

	/* no need */
	if (borg_see_inv)
		return (0);

	/* not recent */
	if (borg_t > need_see_inviso + 5)
		return (0);

	/* too dangerous to cast */
	if (p1 > avoidance * 7)
		return (0);

	/* Do I have anything that will work? */
	if (-1 == borg_slot(TV_POTION, SV_POTION_DETECT_INVIS) &&
		 -1 == borg_slot(TV_SCROLL, SV_SCROLL_DETECT_INVIS) &&
		 !borg_equips_staff_fail(SV_STAFF_DETECT_INVIS) &&
		 !borg_equips_staff_fail(SV_STAFF_DETECT_EVIL) &&
		 !borg_spell_okay_fail(REALM_LIFE, 1, 3, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_SORCERY, 2, 4, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_ARCANE, 2, 7, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 0, 6, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_ARCANE, 0, 2, fail_allowed) &&
		 !borg_equips_activation(ACT_DETECT_ALL, TRUE) &&
		 !borg_equips_activation(ACT_DETECT_XTRA, TRUE) &&
		 !borg_equips_activation(ACT_ESP, TRUE))
		return (0);

	/* Darkness */
	if (no_lite())
		return (0);

	/* No real value known, but lets cast it to find the bad guys. */
	if (borg_simulate)
		return (10);

	/* smoke em if you got em */
	/* short time */
	if (borg_quaff_potion(SV_POTION_DETECT_INVIS)) {
		borg_see_inv = 18000;
		return (10);
	}
	/* long time */
	if (borg_spell_fail(REALM_LIFE, 1, 3, fail_allowed) ||
		 borg_spell_fail(REALM_ARCANE, 2, 7, fail_allowed) ||
		 borg_spell_fail(REALM_TRUMP, 0, 6, fail_allowed) ||
		 borg_spell_fail(REALM_SORCERY, 2, 4, fail_allowed) ||
		 borg_activate_activation(ACT_DETECT_XTRA, FALSE) ||
		 borg_activate_activation(ACT_DETECT_ALL, FALSE) ||
		 borg_activate_activation(ACT_ESP, FALSE)) {
		borg_see_inv = 20000;
		return (10);
	}
	/* snap shot */
	if (borg_read_scroll(SV_SCROLL_DETECT_INVIS) ||
		 borg_use_staff(SV_STAFF_DETECT_INVIS) ||
		 borg_spell_fail(REALM_ARCANE, 0, 2, fail_allowed) ||
		 borg_use_staff(SV_STAFF_DETECT_EVIL)) {
		borg_see_inv = 3000; /* hack, actually a snap shot, no ignition message */
		return (10);
	}

	/* ah crap, I guess I wont be able to see them */
	return (0);
}

/*
 * Light Beam to spot lurkers
 * Used only if I am hit by an unseen guy.
 * Lights up a hallway.
 */
static int borg_defend_aux_lbeam(void) {
	bool hallway = FALSE;
	int x = c_x;
	int y = c_y;

	/* no need */
	if (borg_skill[BI_ISBLIND])
		return (0);

	/* not recent, dont bother */
	if (borg_t > (need_see_inviso + 2))
		return (0);

	/* Check to see if I am in a hallway */
	/* Case 1a: north-south corridor
  *   #.#
  *   #@#
  *   #.#
  */
	if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1b: east-west corridor */
	if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1aa: north-south doorway
  *    .
  *   #@#
  *    .
  */
	if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 1ba: east-west doorway
  *   #
  *  .@.
  *   #
  */

	if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 2a: north-south corridor, 3-way & 4-way intersection
	 *   #.#
	 *   x@x
	 *   #.#
	 */
	if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1) &&
		 (borg_cave_floor_bold(y, x - 1) + borg_cave_floor_bold(y, x + 1) >= 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* Case 2b: east-west corridor, 3-way & 4-way intersection
  *  #x#
  *  .@.
  *  #x#
  */
	if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
		 !borg_cave_floor_bold(y + 1, x - 1) &&
		 !borg_cave_floor_bold(y + 1, x + 1) &&
		 !borg_cave_floor_bold(y - 1, x - 1) &&
		 !borg_cave_floor_bold(y - 1, x + 1) &&
		 (borg_cave_floor_bold(y - 1, x) + borg_cave_floor_bold(y + 1, x) >= 1)) {
		/* ok to light up */
		hallway = TRUE;
	}

	/* not in a hallway */
	if (!hallway)
		return (0);

	/* Make sure I am not in too much danger */
	if (borg_simulate && p1 > avoidance * 3 / 4)
		return (0);

	/* test the beam function */
	if (!borg_lite_beam(TRUE))
		return (0);

	/* return some value */
	if (borg_simulate)
		return (10);

	/* if in a hallway call the Light Beam routine */
	if (borg_lite_beam(FALSE)) {
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
		return (10);
	}
	return (0);
}

/* Shift the panel to locate offscreen monsters */
static int borg_defend_aux_panel_shift(void) {
	int dir = 0;
	int wx = panel_col_min / (SCREEN_WID / 2);
	int wy = panel_row_min / (SCREEN_HGT / 2);
	int screen_hgt, screen_wid;
	int new_wy = wy;
	int new_wx = wx;
	/*int panel_wid, panel_hgt;*/
	int grids = 6;

	screen_hgt = SCREEN_HGT;
	screen_wid = SCREEN_WID;
	/*
		panel_wid = screen_wid / 2;
		panel_hgt = screen_hgt / 2;
	*/
	/* no need */
	if (!need_shift_panel && borg_skill[BI_CDEPTH] < 70)
		return (0);

	/* if Questor is on my panel, dont do it */
	if ((borg_depth & (DEPTH_BORER)) && w_y == questor_panel_y &&
		 w_x == questor_panel_x)
		return (0);

	/* If looking for a guy specifically, increase the buffer */
	if (need_shift_panel)
		grids = 8;

	/* Scroll screen vertically when x grids from edge */
	if (c_y <= panel_row_min + grids)
		new_wy = wy - 1;
	if (c_y >= panel_row_min + screen_hgt - grids)
		new_wy = wy + 1;

	/* Scroll screen horizontally when x grids from edge */
	if (c_x <= panel_col_min + grids)
		new_wx = wx - 1;
	if (c_x >= panel_col_min + screen_wid - grids)
		new_wx = wx + 1;

	/* Verify wy, adjust if needed */
	if (new_wy > MAX_HGT - SCREEN_HGT)
		new_wy = MAX_HGT - SCREEN_HGT;
	if (new_wy < 0)
		new_wy = 0;

	/* Verify wx, adjust if needed */
	if (new_wx > MAX_WID - SCREEN_WID)
		new_wx = MAX_WID - SCREEN_WID;
	if (new_wx < 0)
		new_wx = 0;

	/* Determine the direction of shift */
	if (new_wx > wx && new_wy == wy)
		dir = 6;
	if (new_wx < wx && new_wy == wy)
		dir = 4;
	if (new_wy > wy && new_wx == wx)
		dir = 2;
	if (new_wy < wy && new_wx == wx)
		dir = 8;

	if (new_wx > wx && new_wy > wy)
		dir = 3;
	if (new_wx > wx && new_wy < wy)
		dir = 9;
	if (new_wx < wx && new_wy > wy)
		dir = 1;
	if (new_wx < wx && new_wy < wy)
		dir = 7;

	/* Do the Shift if needed, then note it,  reset the flag */
	if (need_shift_panel == TRUE && dir != 0) {
		/* Send action (view panel info) */
		borg_keypress('L');

		if (dir)
			borg_keypress(I2D(dir));
		borg_keypress(ESCAPE);

		borg_note("# Shifted panel to locate offscreen monster.");
		need_shift_panel = FALSE;
		when_shift_panel = borg_t;

		/* Leave the panel shift mode */
		borg_keypress(ESCAPE);
	} else if (dir != 0)
	/* check to make sure its appropriate */
	{

		/* Hack Not if I just did one */
		if (when_shift_panel &&
			 (borg_t - when_shift_panel <= 10 || borg_t - borg_t_questor <= 10)) {
			/* do nothing */
		} else
			 /* shift up? only if a north corridor */
			 if (dir >= 7 && track_step_num > 0 &&
				  track_step_y[track_step_num - 1] != c_y - 1) {
			if ((borg_projectable_pure(c_y, c_x, c_y - 2, c_x)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y - 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x - 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y - 2, c_x - 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y - 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x + 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y - 2, c_x + 2))) {
				/* Send action (view panel info) */
				borg_keypress('L');
				if (dir)
					borg_keypress(I2D(dir));
				borg_note("# Shifted panel as a precaution.");
				/* Mark the time to avoid loops */
				when_shift_panel = borg_t;
				/* Leave the panel shift mode */
				borg_keypress(ESCAPE);
			}
		} else /* shift down? only if a south corridor */
			 if (dir <= 3 && track_step_num > 0 &&
				  track_step_y[track_step_num - 1] != c_y + 1) {
			if ((borg_projectable_pure(c_y, c_x, c_y + 2, c_x)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y + 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x - 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y + 2, c_x - 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y + 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x + 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y + 2, c_x + 2))) {
				/* Send action (view panel info) */
				borg_keypress('L');
				borg_keypress(I2D(dir));
				borg_note("# Shifted panel as a precaution.");
				/* Mark the time to avoid loops */
				when_shift_panel = borg_t;
				/* Leave the panel shift mode */
				borg_keypress(ESCAPE);
			}
		} else /* shift Left? only if a west corridor */
			 if ((dir == 4 || dir == 1 || dir == 7) && track_step_num > 0 &&
				  track_step_x[track_step_num - 1] != c_x - 1) {
			if ((borg_projectable_pure(c_y, c_x, c_y, c_x - 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y + 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x - 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y + 2, c_x - 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y - 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x - 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y - 2, c_x - 2))) {
				/* Send action (view panel info) */
				borg_keypress('L');
				if (dir)
					borg_keypress(I2D(dir));
				borg_note("# Shifted panel as a precaution.");
				/* Mark the time to avoid loops */
				when_shift_panel = borg_t;
				/* Leave the panel shift mode */
				borg_keypress(ESCAPE);
			}
		} else /* shift Right? only if a east corridor */
			 if ((dir == 6 || dir == 9 || dir == 3) && track_step_num > 0 &&
				  track_step_x[track_step_num - 1] != c_x + 1) {
			if ((borg_projectable_pure(c_y, c_x, c_y, c_x + 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y + 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x + 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y + 2, c_x + 2)) ||
				 ((borg_projectable_pure(c_y, c_x, c_y - 2, c_x) &&
					borg_projectable_pure(c_y, c_x, c_y, c_x + 2)) ||
				  borg_projectable_pure(c_y, c_x, c_y - 2, c_x + 2))) {
				/* Send action (view panel info) */
				borg_keypress('L');
				if (dir)
					borg_keypress(I2D(dir));
				borg_note("# Shifted panel as a precaution.");
				/* Mark the time to avoid loops */
				when_shift_panel = borg_t;
				/* Leave the panel shift mode */
				borg_keypress(ESCAPE);
			}
		}
	}
	/* This uses no energy */
	return (0);
}

/*
 * Phantasmal Servant, and other summoned friendlies.
 * If the borg is in a room and several monsters have LOS on the borg,
 * then it might be a good idea to summon a few meat-shields.
 * Generally using Charm is usually a better idea than summoning because
 * Charming adds a pet AND removes one monster at the same time.
 * Charming monsters is done in borg_attack() which happens after the
 * defence checks.
 *
 * Borg will try to assume value of summoning a low level servant in a deep
 * dungeon
 */
static int borg_defend_aux_servant(int p1) {
	int fail_allowed = 35;
	int i, p2;
	int friendlies = 0;
	int have_los = 0;
	int adjacent = 0;
	bool need_calvary = FALSE;
	bool landing_grid = FALSE;
	int y, x;
	int strength = -1;

	borg_grid *ag;

	/* must have the ability and check quality */
	if (borg_spell_okay_fail(REALM_TRUMP, 1, 1, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 1, 2, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 1, 3, fail_allowed) ||
		 borg_spell_okay_fail(REALM_NATURE, 1, 6, fail_allowed) ||
		 borg_equips_activation(ACT_SUMMON_ANIMAL, TRUE))
		strength = 1;

	if (borg_spell_okay_fail(REALM_TRUMP, 1, 4, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 2, 1, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 2, 2, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 2, 3, fail_allowed))
		strength = 2;

	if (borg_equips_activation(ACT_SUMMON_PHANTOM, TRUE) ||
		 borg_equips_activation(ACT_SUMMON_ELEMENTAL, TRUE) ||
		 borg_spell_okay_fail(REALM_TRUMP, 3, 2, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 3, 3, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 3, 4, fail_allowed))
		strength = 3;

	if (borg_spell_okay_fail(REALM_TRUMP, 3, 5, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 3, 6, fail_allowed) ||
		 borg_spell_okay_fail(REALM_CHAOS, 2, 7, fail_allowed) ||
		 borg_equips_activation(ACT_SUMMON_DEMON, TRUE) ||
		 borg_equips_activation(ACT_SUMMON_UNDEAD, TRUE))
		strength = 4;

	if (borg_spell_okay_fail(REALM_TRUMP, 3, 7, fail_allowed) ||
		 borg_spell_okay_fail(REALM_TRUMP, 2, 7, fail_allowed))
		strength = 5;

	/* require ability */
	if (strength == -1)
		return (0);

	/* reset initial danger */
	p2 = 1;

	/* Two passes to determine exact count */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Need to be able to project to the borg */
		if (!borg_projectable(kill->y, kill->x, c_y, c_x, TRUE, TRUE))
			continue;

		/* Skip and count Friendly */
		if (kill->ally) {
			friendlies++;
			continue;
		}

		/* Count monsters who can see the borg. */
		have_los++;

		/* Count ones next to the borg */
		if (kill->dist == 1)
			adjacent++;

		/* add some danger */
		p2 += borg_danger_aux(c_y, c_x, i, 1, TRUE, FALSE);
	}

	/* Ratio of monsters:danger?  We do not always want to cast the summon for 1
	 * enemy,
	 * but if that one is fierce, then it might be a good idea.  If surrounded by
	 * lots
	 * of whimpy guys, still might be a good idea
	 */
	if (have_los <= 2 && p2 >= avoidance * 6 / 10)
		need_calvary = TRUE;
	if (have_los >= 3 && have_los <= 5 && p2 >= avoidance * 5 / 10)
		need_calvary = TRUE;
	if (have_los >= 6 && have_los <= 7 && p2 >= avoidance * 3 / 10)
		need_calvary = TRUE;
	if (have_los >= 8)
		need_calvary = TRUE;
	if (p1 > avoidance)
		need_calvary = FALSE;

	/* if it is not too dangerous, have some meat shields. */
	if (friendlies < BORG_MAX_FRIENDLIES && need_calvary) {
		/* do we want him to do this in a hallway?  should it matter? */
		for (i = 0; i < 8; i++) {
			/* Grid in that direction */
			x = c_x + ddx_ddd[i];
			y = c_y + ddy_ddd[i];

			/* check for legal */
			if (!in_bounds(y, x))
				continue;

			/* Access the grid */
			ag = &borg_grids[y][x];

			/* Can a summoned guy land on this grid? */
			if (!borg_cave_clean_bold(y, x))
				continue;
			if (ag->kill)
				continue;

			/* At least 1 grid is available */
			landing_grid = TRUE;
		}

		/* No landing grid for the summoned guy */
		if (!landing_grid)
			return (0);

		/* Is the summoned guy likely to survive to do any good? */
		if (borg_simulate) {
			/* Strength of summoned vs depth */
			if ((strength >= 5) || (strength >= 4 && borg_skill[BI_CDEPTH] < 65) ||
				 (strength >= 3 && borg_skill[BI_CDEPTH] < 50) ||
				 (strength >= 2 && borg_skill[BI_CDEPTH] < 35) ||
				 (strength >= 1 && borg_skill[BI_CDEPTH] < 20)) {
				/* Probably of some value */
				return (strength * 2);
			}

			/* Probably of no value*/
			return (0);
		}

		/* Cast the spell */
		if (borg_spell_fail(REALM_TRUMP, 3, 7, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 3, 6, fail_allowed) ||
			 borg_spell_fail(REALM_CHAOS, 2, 7, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 3, 5, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 3, 4, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 3, 3, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 3, 2, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 2, 7, fail_allowed) ||
			 borg_activate_activation(ACT_SUMMON_DEMON, FALSE) ||
			 borg_activate_activation(ACT_SUMMON_UNDEAD, FALSE) ||
			 borg_activate_activation(ACT_SUMMON_PHANTOM, FALSE) ||
			 borg_spell_fail(REALM_TRUMP, 2, 3, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 2, 2, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 2, 1, fail_allowed) ||
			 borg_activate_activation(ACT_SUMMON_ELEMENTAL, FALSE) ||
			 borg_spell_fail(REALM_TRUMP, 1, 4, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 1, 3, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 1, 2, fail_allowed) ||
			 borg_spell_fail(REALM_TRUMP, 1, 1, fail_allowed) ||
			 borg_spell_fail(REALM_NATURE, 1, 6, fail_allowed) ||
			 borg_activate_activation(ACT_SUMMON_ANIMAL, FALSE)) {
			/* Value */
			return (strength);
		}
	}
	return (0);
}

/*
 * Sometimes the best defense is a strong offense.  The borg can be threatened
 * by an
 * enemy with powerful attacks which induce a very high danger rating.  This
 * enemy might
 * be fairly weak and easy to kill, but the borg will not engage him because of
 * the
 * high danger.  This routine will look at the various attack options which the
 * borg
 * possesses and determine if the borg can likely dispatch the enemy before it
 * is able
 * to use it's potent spells.  A good example would be a borg at level 10
 * fighting an
 * Illusionist which possess Confusion, Paralyze and Blindness spells.  All of
 * which
 * create very high danger for the borg.  But an Illusionist is fairly weak and
 * will
 * die quickly to most borg attacks.  Rather than escape with spells, the borg
 * should
 * consider dispatching him quickly.
 *
  */
static int borg_defend_aux_offense(int p1) {
	int p2;
	int i, x, y;
	int b_i = -1, b_y = 0, b_x = 0;
	int n, b_n = 0;
	int g, b_g = -1;
	/*int a_y, a_x;
	int b_p = 0;*/
	bool real_borg_simulate = borg_simulate;

	borg_grid *ag;
	borg_kill *kill;
	monster_race *r_ptr;

	/* Nobody around */
	if (!borg_kills_cnt)
		return (0);

	/* Dangerous, but not in real danger */
	if (p1 < avoidance * 65 / 100)
		return (0);
	if (p1 > avoidance * 3)
		return (0);

	/* Not if sitting in a wall */
	if (borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)
		return (0);

	/* Set the attacking flag so that danger is boosted for monsters
	 * we want to attack first.
	 */
	borg_attacking = TRUE;

	/* Reset list */
	borg_temp_n = 0;

	/* Find "nearby" monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Require current knowledge */
		if (kill->when < borg_t - 2)
			continue;

		/* Ignore multiplying monsters and when fleeing from scaries*/
		if (goal_ignoring && !borg_skill[BI_ISAFRAID] &&
			 (r_info[kill->r_idx].flags2 & RF2_MULTIPLY))
			continue;

		/* Skip the Greater Hell Beast */
		if (strstr(r_name + r_ptr->name, "Greater hell"))
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Acquire location */
		x = kill->x;
		y = kill->y;

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Never shoot off-screen */
		if (!(ag->info & BORG_OKAY))
			continue;

		/* Never shoot through walls */
		if (!(ag->info & BORG_VIEW))
			continue;

		/* Check the distance XXX XXX XXX */
		if (kill->dist > 16)
			continue;

		/* Save the location (careful) */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* No destinations */
	if (!borg_temp_n && real_borg_simulate) {
		borg_attacking = FALSE;
		return (0);
	}

	/* Fake the simulation during the tests */
	borg_simulate = TRUE;

	/* Analyze the possible attacks */
	for (g = 0; g < BF_MAX; g++) {
		/* Simulate */
		n = borg_attack_aux(g, FALSE, -1);

		/* Track "best" attack  < */
		if (n <= 0)
			continue;
		if (n < b_n)
			continue;

		/* Track best */
		b_g = g;
		b_n = n;

		/* Track the target location */
		b_x = borg_target_x;
		b_y = borg_target_y;
	}

	/* Nothing good */
	if ((b_n <= 0 || b_g < 0) && real_borg_simulate) {
		borg_attacking = FALSE;
		return (0);
	}

	/* What is the targeted monster's index? */
	for (i = 1; i < borg_kills_nxt; i++) {

		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Location must match */
		if (kill->y != b_y && kill->x != b_x)
			continue;

		/* Save the index location */
		b_i = i;
	}

	/* Just in case it fails */
	if (b_i < 0 && real_borg_simulate)
		return (0);

	/* What is the danger imposed by this monster? */
	p2 = p1 - borg_danger_aux(c_y, c_x, 1, b_i, TRUE, FALSE);

	/* Do not attempt if the creature has more HP than my damage capacity? */
	if (b_n < (borg_kills[b_i].power * 75 / 100) && real_borg_simulate)
		return (0);

	/* Does removing this monster make it somewhat safe for me? */
	if (p2 > avoidance * 15 / 10 && real_borg_simulate)
		return (0);

	/* Return the value of the defence maneuvers */
	if (real_borg_simulate)
		return (p1 - p2);

	/* Instantiate */
	borg_attacking = TRUE;
	borg_simulate = FALSE;

	/* Note */
	borg_note(format("# Best defense is offense. Type %d with damage %d. p1:%d, "
						  "p2:%d, hp:%d, name:%s",
						  b_g, b_n, p1, p2, borg_kills[b_i].power,
						  r_name + r_info[b_i].name));

	(void)borg_attack_aux(b_g, FALSE, -1);
	borg_attacking = FALSE;

	/* Success */
	return (p1 - p2);
}

/* Mindcrafter power to use Telek Wave to dispurse the monsters adjacent to the
 * borg
 * p1 is all the danger
 * p2 is the assumed danger of monsters who were adjacent, but were relocated
 * after the simulated spell.
 * p3 is the danger of monsters who are adjacent to the borg before the spell.
 * p4 is the improvement of the borg's condition because he relocated monsters
 * d/t the spell.
 * To be useful:
 *  A.  p3 needs to be a substantial part of p1
 *  B.  p2 needs to be substantially lower than p3.
 *  C.  p4 needs to be substantially lower than p1.
 */
static int borg_defend_aux_tele_wave(int p1) {
	int p2 = 0;
	int fail_allowed = 25;
	int i, attempts = 0;
	byte mon_y;
	byte mon_x;
	int y, x;
	/*int d;*/
	int p3 = 0;
	int p = 0;
	int p4 = 0;
	borg_grid *ag;

	/* Cast the spell */
	if (!borg_simulate) {
		/* Perform the function */
		(void)borg_mindcr_fail(MIND_TELE_WAVE, 28, fail_allowed);
		/* Value */
		return (5);
	}

	/* Only tell away if scared */
	if (p1 < avoidance * 8 / 10)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 10;

	/* Have the ability */
	if (!borg_mindcr_okay_fail(MIND_TELE_WAVE, 28, fail_allowed))
		return (0);

	/* Not if in an AS-corridor unless wounded */
	if (((borg_position & (POSITION_SEA | POSITION_BORE | POSITION_SUMM))) &&
		 borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] / 2)
		return (0);

	/* reset initial danger */
	p1 = 1;

	/* Two passes to determine exact danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;
		/*monster_race *r_ptr;*/

		/* Monster */
		kill = &borg_kills[i];
		/*r_ptr = &r_info[kill->r_idx];*/

		ag = &borg_grids[kill->y][kill->x];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip ones too far away */
		if (kill->dist >= MAX_RANGE)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p = borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		p1 += p;
		if (kill->dist == 1)
			p3 += p;
		borg_full_damage = FALSE;
	}

	/* Pass two -- Find adjacent monster and calculate its danger if it were not
	 * adjacent. */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*monster_race r_ptr = &r_info[kill->r_idx];*/

		ag = &borg_grids[kill->y][kill->x];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Skip non adjacent monsters */
		if (kill->dist != 1)
			continue;

		/* Skip ones too far away */
		if (kill->dist >= MAX_RANGE)
			continue;

		/* Store the real location of this monster */
		mon_y = kill->y;
		mon_x = kill->x;
		y = kill->y;
		x = kill->x;

		/** Temporarily spoof the location of the monster **/
		for (attempts = 0; attempts < 300; attempts++) {

			/* Pick a location */
			y = rand_spread(kill->y, 7);
			x = rand_spread(kill->x, 7);

			/* Ignore illegal locations */
			if (!in_bounds(y, x))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip walls */
			if (!borg_cave_floor_bold(y, x))
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Stop looking */
			kill->y = y;
			kill->x = x;
			break;
		}

		/* Calculate the ranged-only-attacks danger of who is left over.  Note
	  * that we have
	  * just spoofed the monster location so borg_danger_aux() will think the
	  * monster is
	  * elsewhere
	  */
		borg_full_damage = TRUE;
		p2 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Return the monster's true position */
		kill->y = mon_y;
		kill->x = mon_x;
	}

	/* no negatives */
	if (p2 <= 0)
		p2 = 0;

	/* improvement */
	p4 = p3 - p2;

	/* check to see if I am left better off
	*  A.  p3 needs to be a substantial part of p1
	 *  B.  p2 needs to be substantially lower than p3.
	*/
	if (p3 >= p1 * 5 / 10 && p2 <= p3 * 5 / 10 &&
		 borg_fear_region[c_y / 11][c_x / 11] <= avoidance &&
		 p1 > (avoidance * 8 / 10)) {
		/* Simulation */
		if (borg_simulate)
			return (p4);
	}
	return (0);
}

/* Mindcrafter power to use Mind Wave in a pit.
 * The borg will jump into a pit in order to use Mind Wave in it.
 * But after landing inthe pit, he will call his defence routines
 * and see that it is a dangerous place and want to tport out.
 * He will just jump back in the next round.
 * The borg needs to be able to cast Mind Wave as a defence.
 */
static int borg_defend_aux_mind_wave(void) {
	int fail_allowed = 25;
	int i;
	int count = 0;
	int empty_danger = 0;

	/* Cast the spell */
	if (!borg_simulate) {
		/* Perform the function */
		(void)borg_mindcr_fail(MIND_MIND_WAVE, 25, fail_allowed);
		/* Value */
		return (5);
	}

	/* Have the ability */
	if (!borg_mindcr_okay_fail(MIND_MIND_WAVE, 25, fail_allowed))
		return (0);

	/* better be healthy for all the attacks coming our way */
	if (borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)
		return (0);

	/* Need to have lots of mana.  It might be better to use Psi_Drain */
	if (borg_skill[BI_CURSP] < 20)
		return (0);

	/* passes to determine exact count */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;
		monster_race *r_ptr;

		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip the missing ones */
		if (kill->when <= borg_t - 10)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* non-Suseptable */
		if ((r_ptr->flags2 & RF2_EMPTY_MIND) || (r_ptr->flags3 & RF3_DEMON) ||
			 (r_ptr->flags3 & RF3_UNDEAD) ||
			 (kill->level >= borg_skill[BI_CLEVEL])) {
			/* keep a tally of monsters not effected by the spell.
			 * They can be dangerous.
			 */
			empty_danger += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
			continue;
		}

		/* count */
		count++;
	}

	/* not the right situation */
	if (count >= BORG_PIT_JUMP && empty_danger < avoidance * 5 / 10) {
		/* Simulation */
		if (borg_simulate)
			return (2000);
	}
	return (0);
}

/* Life realm Divine Intervention:
 * 		project(0, 1, py, px, 777, GF_HOLY_FIRE, PROJECT_KILL);
 *		dispel_monsters(plev * 4);
 *		slow_monsters();
 *		stun_monsters(plev * 4);
 *		confuse_monsters(plev * 4);
 * 		turn_monsters(plev * 4);
 * 		stasis_monsters(plev * 4);
 *		summon_specific(-1, py, px, plev, SUMMON_ANGEL, TRUE, TRUE, TRUE);
 *		(void)set_shero(p_ptr->shero + randint(25) + 25);
 *		(void)hp_player(300);
 *		(void)set_fast(randint(20 + plev) + plev);
 *		(void)set_afraid(0);
 */
static int borg_defend_aux_divine(int p1) {
	int p2 = 0;
	int fail_allowed = 25;
	int i, attempts = 0;
	byte mon_y;
	byte mon_x;
	int y, x;
	/*int d;*/
	bool true_speed = borg_speed;

	borg_grid *ag;

	/* Cast the spell */
	if (!borg_simulate) {
		/* Perform the function */
		(void)borg_spell(REALM_LIFE, 3, 6);
		/* Value */
		return (5);
	}

	/* if scared */
	if (p1 < avoidance * 8 / 10)
		return (0);

	/* if very scary, do not allow for much chance of fail */
	if (p1 > avoidance * 4)
		fail_allowed -= 10;

	/* Have the ability */
	if (!borg_spell_okay_fail(REALM_LIFE, 3, 6, fail_allowed))
		return (0);

	/* reset initial danger */
	p1 = 1;

	/* Two passes to determine exact danger */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*monster_race r_ptr = &r_info[kill->r_idx];*/

		ag = &borg_grids[kill->y][kill->x];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Check the LOS */
		if (!borg_projectable(c_y, c_x, kill->y, kill->x, TRUE, TRUE))
			continue;

		/* Calculate danger of who is left over */
		borg_full_damage = TRUE;
		p1 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
	}

	/* Pass two -- Find adjacent monster and calculate its danger if it were not
	 * adjacent. */
	for (i = 0; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		/*monster_race r_ptr = &r_info[kill->r_idx];*/

		ag = &borg_grids[kill->y][kill->x];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Skip pets and friendly */
		if (kill->ally)
			continue;

		/* Skip non adjacent monsters */
		if (kill->dist != 1)
			continue;

		/* Store the real location of this monster */
		mon_y = kill->y;
		mon_x = kill->x;
		y = kill->y;
		x = kill->x;

		/** Temporarily spoof the location of the monster **/
		for (attempts = 0; attempts < 300; attempts++) {
			/* Pick a (possibly illegal) location */
			y = rand_spread(kill->y, 7);
			x = rand_spread(kill->x, 7);

			/* Ignore illegal locations */
			if (!in_bounds(y, x))
				continue;

			/* Access */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip weird grids */
			if (ag->feat == FEAT_INVIS)
				continue;

			/* Skip walls */
			if (!borg_cave_floor_bold(y, x))
				continue;

			/* Skip monsters */
			if (ag->kill)
				continue;

			/* Stop looking */
			kill->y = y;
			kill->x = x;
			break;
		}

		/* Calculate the ranged-only-attacks danger of who is left over.  Note
	  * that we have
	  * just spoofed the monster location so borg_danger_aux() will think the
	  * monster is
	  * elsewhere.
	  *
	  * And add speed bonus
	  */
		borg_speed_spell = TRUE;
		borg_full_damage = TRUE;
		p2 += borg_danger_aux(c_y, c_x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;
		borg_speed_spell = FALSE;

		/* Return the monster's true position */
		kill->y = mon_y;
		kill->x = mon_x;
	}

	/* Restore true speed. */
	borg_speed = true_speed;

	/* Boost the benefit since it has other positive effects */
	p2 /= 2;

	/* no negatives */
	if (p2 <= 0)
		p2 = 0;

	/* check to see if I am left better off */
	if (p1 > p2 &&
		 p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
		 p1 > (avoidance)) {
		/* Simulation */
		if (borg_simulate)
			return (p2);
	}
	return (0);
}
/*
 * Dismiss pets as a defence maneuver.
 *
 * This should really be made much smarter.  Right now, it will dismiss all pets
 * unless the borg
 * has Trump as a realm.  The purpose of this routine is to intended to dismiss
 * monsters after
 * they have been charmed; especially Warrior of the Dawn, and summoners.
 *
 * Note that Dismissing Pets does not consume energy.
 * Why would dismissing pets be a defencive move?
 */
static int borg_defend_aux_dismiss(void) {
	int i;
	int num_friendly = 0;

	borg_kill *kill;

	/* Not in town */
	if (borg_skill[BI_CDEPTH] == 0)
		return (0);

	/* Am I supposed to have pets (Trumper) */
	if (borg_skill[BI_REALM1] == REALM_TRUMP ||
		 borg_skill[BI_REALM2] == REALM_TRUMP ||
		 borg_equips_activation(ACT_SUMMON_PHANTOM, FALSE) ||
		 borg_skill[BI_REALM1] == REALM_NATURE ||
		 borg_skill[BI_REALM2] == REALM_NATURE)
		return (0);

	/* Scan the monster list and count the Pets */
	if (borg_simulate) {
		/* Look at each known monster */
		for (i = 1; i < borg_kills_nxt; i++) {
			kill = &borg_kills[i];

			/* Skip dead monsters */
			if (!kill->r_idx)
				continue;

			if (kill->when <= borg_t - 100)
				continue;

			/* Is he a pet? */
			if ((kill->ally) && !(kill->unique))
				num_friendly++;
		}

		/* Do I have any Pets? */
		if (num_friendly <= 20)
			return (0);

		/* Return some value */
		return (num_friendly);
	}

	/* Dismiss the pets */
	borg_keypress('p');
	/* Should I confirm the Pet Screen before continueing? */
	borg_keypress('a');
	borg_keypress('y');

	/* Remove the friendly animal from my array */
	for (i = 1; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* delete all pets */
		/*TODO MUST not sure this is how you dismiss a pet/ally ;) */
		/*TODO COOL dismissed pets should become true neutral ;) */
		if (kill->ally)
			borg_delete_kill(i);
	}

	return (1);
}

#if 0
/*
 * Force no more breeding
 */
static int borg_defend_aux_sterility()
{
    int fail_allowed = 10;

    /* already in effect */
    if (num_repro >= MAX_REPRO)
        return (0);

	/* No need */
	if (!(borg_depth & DEPTH_BREEDER)) return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISCONFUSED]) return (0);

    /* no ability */
    if (!borg_mutation(COR1_STERILITY, TRUE, fail_allowed, FALSE))
        return (0);

	/* Too painful */
	if (borg_skill[BI_CURHP] < 50) return (0);

    /* Simulation */
    if (borg_simulate) return (5);

    /* do it! */
    if (borg_mutation(COR1_STERILITY, FALSE, 100, FALSE))
         return 5;

	return (0);
}
#endif

/*
 * Brand my weapon with Vamp if I am fighting certain types of monsters.
 */
static bool borg_defend_aux_brand(/*int p1*/) {
	int i, b_i = -1;
	int b_realm = -1;
	int b_book = -1;
	int b_spell = -1;
	int value = 0;
	int b_m = -1;

	/*bool proceed = FALSE;*/

	borg_kill *kill;
	monster_race *r_ptr;
	/*borg_item *item = &borg_items[INVEN_WIELD];*/

	/* No Need */
	if (borg_skill[BI_NO_MELEE])
		return (FALSE);

	/* Not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Need "brand" ability */
	if (!borg_spell_okay_fail(REALM_SORCERY, 2, 0, 30) &&
		 !borg_spell_okay_fail(REALM_NATURE, 3, 6, 30) &&
		 !borg_spell_okay_fail(REALM_CHAOS, 2, 6, 30) &&
		 !borg_spell_okay_fail(REALM_DEATH, 1, 4, 30) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 5, 30) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 2, 4, 30))
		return (FALSE);

	/* Look at each known monster.  Only do it if there are certain types near
	 * by. */
	for (i = 1; i < borg_kills_nxt; i++) {
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Is he a pet? */
		if (!kill->killer)
			continue;

		/* Right type of guy? */
		if (!kill->cautious && !kill->summoner && !kill->unique && !kill->questor)
			continue;

		/* too far away.  We save it until he is close */
		if (kill->dist >= 6)
			continue;

		/* Track the monster */
		b_m = i;
	}

	/* look through inventory for ammo */
	for (i = INVEN_WIELD; i < INVEN_BOW; i++) {
		borg_item *item = &borg_items[i];

		/* no qualifying monster */
		if (b_m == -1)
			continue;
		kill = &borg_kills[b_m];
		r_ptr = &r_info[kill->r_idx];

		/* For now skip the Bow */
		if (i == INVEN_BOW)
			continue;

		/* Skip non-identified items or empty  */
		if (!item->ident)
			continue;
		if (!item->iqty)
			continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING)
			continue;

		/* Item already branded */
		if (item->name3 != 0)
			continue;

		/* Check Sorcery */
		if (borg_spell_okay_fail(REALM_SORCERY, 2, 0, 30)
			 /* && Creature is not immune? */) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_SORCERY;
			b_book = 2;
			b_spell = 0;
			value = 30;
		}

		/* Check Nature */
		if (borg_spell_okay_fail(REALM_NATURE, 3, 6, 30)
			 /* && Creature is not immune? */) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_NATURE;
			b_book = 3;
			b_spell = 6;
			value = 30;
		}

		/* Check Chaos */
		if (borg_spell_okay_fail(REALM_CHAOS, 2, 6, 30) &&
			 !(item->flags1 & TR1_CHAOTIC) && !(r_ptr->flags4 & RF4_BR_CHAO) &&
			 !(r_ptr->flags3 & RF3_DEMON)) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_CHAOS;
			b_book = 2;
			b_spell = 6;
			value = 30;
		}

		/* Check Death, Poison */
		if (borg_spell_okay_fail(REALM_DEATH, 1, 4, 30) &&
			 !(item->flags1 & TR1_BRAND_POIS) && !(r_ptr->flags3 & RF3_IM_POIS)) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_DEATH;
			b_book = 1;
			b_spell = 4;
			value = 50;
		}

		/* Check Death, Vampirism */
		if (borg_spell_okay_fail(REALM_DEATH, 2, 5, 30) &&
			 !(item->flags1 & TR1_VAMPIRIC) &&
			 !(r_ptr->flags3 & (RF3_DEMON | RF3_UNDEAD | RF3_NONLIVING))) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_DEATH;
			b_book = 2;
			b_spell = 5;
			value = 100;
		}

		/* Check Trump */
		if (borg_spell_okay_fail(REALM_TRUMP, 2, 4, 30) && !item->name1 &&
			 !item->name2 && !item->cursed && (r_ptr->flags3 & (RF3_DEMON))) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_TRUMP;
			b_book = 2;
			b_spell = 4;
			value = 75;
		}
	}

	/* No useful target of the spell */
	if (b_m == -1 || b_i == -1)
		return (0);

	if (borg_simulate)
		return (value);

	/* Enchant it */
	if (borg_spell(b_realm, b_book, b_spell)) {
		/* choose the weapon */
		/*borg_keypress('/');
		  borg_keypress(I2A(b_i - INVEN_WIELD)); */

		/* examine Inven next turn */
		borg_do_inven = TRUE;

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* Success */
		return (value);
	}

	/* Nothing to do */
	return (0);
}

/*
 * Simulate/Apply the optimal result of using the given "type" of defence
 * p1 is the current danger level (passed in for effiency)
 */
static int borg_defend_aux(int what, int p1) {
	/* Analyze */
	switch (what) {
	case BD_SPEED: {
		return (borg_defend_aux_speed(p1));
	}
	case BD_PROT_FROM_EVIL: {
		return (borg_defend_aux_prot_evil(p1));
	}
	case BD_RESIST_FCE: {
		return (borg_defend_aux_resist_fce(p1));
	}
	case BD_RESIST_FECAP: {
		return (borg_defend_aux_resist_fecap(p1));
	}
	case BD_RESIST_FECAP_POT: {
		return (borg_defend_aux_resist_fecap_pot(p1));
	}
	case BD_RESIST_F: {
		return (borg_defend_aux_resist_f(p1));
	}
	case BD_RESIST_C: {
		return (borg_defend_aux_resist_c(p1));
	}
	case BD_RESIST_A: {
		return (borg_defend_aux_resist_a(p1));
	}
	case BD_RESIST_P: {
		return (borg_defend_aux_resist_p(p1));
	}
	case BD_BLESS: {
		return (borg_defend_aux_bless(p1));
	}

	case BD_HERO: {
		return (borg_defend_aux_hero(p1));
		break;
	}
	case BD_BERSERK: {
		return (borg_defend_aux_berserk(p1));
		break;
	}
	case BD_SHIELD: {
		return (borg_defend_aux_shield(p1));
	}
	case BD_GOI: {
		return (borg_defend_aux_goi(p1));
	}
	case BD_GOI_POT: {
		return (borg_defend_aux_goi_pot(p1));
	}

	case BD_TPORTOTHER: {
		return (borg_defend_aux_tportother(p1));
	}
	case BD_GLYPH: {
		return (borg_defend_aux_glyph(p1));
	}
	case BD_WARDING: {
		return (borg_defend_aux_true_warding(p1));
	}
	case BD_CREATE_WALLS: {
		return (borg_defend_aux_create_walls(p1));
	}
	case BD_MASS_GENOCIDE: {
		return (borg_defend_aux_mass_genocide());
	}
	case BD_GENOCIDE: {
		return (borg_defend_aux_genocide(p1));
	}
	case BD_GENOCIDE_NASTIES: {
		return (borg_defend_aux_genocide_nasties(p1));
	}
	case BD_EARTHQUAKE: {
		return (borg_defend_aux_earthquake());
	}
	case BD_DESTRUCTION: {
		return (borg_defend_aux_destruction());
	}
	case BD_BANISH_EVIL: {
		return (borg_defend_aux_banish_evil(p1));
	}
	case BD_BANISH_GOOD: {
		return (borg_defend_aux_banish_good(p1));
	}
	case BD_BANISHMENT_LUCIFER: {
		return (borg_defend_aux_banishment_lucifer());
	}
	case BD_TPORTOTHER_LUCIFER: {
		return (borg_defend_aux_tportother_lucifer());
	}
	case BD_DETECT_INVISO: {
		return (borg_defend_aux_inviso(p1));
	}
	case BD_LIGHT_BEAM: {
		return (borg_defend_aux_lbeam());
	}
	case BD_SHIFT_PANEL: {
		return (borg_defend_aux_panel_shift());
	}
	case BD_TRUMP_SERVANT: {
		return (borg_defend_aux_servant(p1));
	}
	case BD_OFFENSE: {
		return (borg_defend_aux_offense(p1));
	}

	case BD_TELE_WAVE: {
		return (borg_defend_aux_tele_wave(p1));
	}

	case BD_MIND_WAVE: {
		return (borg_defend_aux_mind_wave());
	}

	case BD_DISMISS_PETS: {
		return (borg_defend_aux_dismiss());
	}

	case BD_DIVINE_INTERV: {
		return (borg_defend_aux_divine(p1));
	}

	case BD_DAY_DOVE: {
		return (borg_defend_aux_day_dove(p1));
	}

	case BD_REST: {
		return borg_defend_aux_rest();
	}

	case BD_HALL_SLEEP: {
		return borg_defend_aux_hall_sleep(p1);
	}

	case BD_WRAITH: {
		return borg_defend_aux_wraith(p1);
	}

	case BD_BRANDING: {
		return borg_defend_aux_brand(p1);
	}
	}
	return (0);
}

/*
 * prepare to attack... this is setup for a battle.
 */
bool borg_defend(int p1) {
	int n, b_n = 0;
	int g, b_g = -1;

	/* Simulate */
	borg_simulate = TRUE;

	/* if you have a globe up and it is about to drop, */
	/* refresh it (if you can) */
	if ((borg_goi || borg_wraith) &&
		 (borg_goi + borg_wraith) < (borg_game_ratio * 2)) {
		int p;

		/* check 'true' danger. This will make sure we do not */
		/* refresh our GOI if no-one is around */
		borg_attacking = TRUE;
		p = borg_danger(c_y, c_x, 1, TRUE);
		borg_attacking = FALSE;
		if (p > borg_fear_region[c_y / 11][c_x / 11] || borg_fighting_unique) {
			if (borg_spell(REALM_ARCANE, 3, 7) || borg_spell(REALM_LIFE, 3, 7)) {
				borg_note(format(
					 "# refreshing GOI.  borg_goi=%d, p_ptr->invuln=%d, (ratio=%d)",
					 borg_goi, p_ptr->invuln, borg_game_ratio));
				borg_attempting_refresh = TRUE;
				borg_goi = 12000;
				return (TRUE);
			}
			if (borg_spell(REALM_DEATH, 3, 7)) {
				borg_note(format("# refreshing Wraithform.  borg_wraith=%d, "
									  "p_ptr->wraith_form=%d, (ratio=%d)",
									  borg_wraith, p_ptr->wraith_form, borg_game_ratio));
				borg_attempting_refresh = TRUE;
				borg_wraith = 30000; /*25000 + 12000; 37000 will not fit in s16b */
				return (TRUE);
			}
		}
	}

	/* Analyze the possible setup moves */
	for (g = 0; g < BD_MAX; g++) {
		/* We must skip certain ones if starving */
		if (borg_skill[BI_ISWEAK] /* &&
			(g != SomethingWhichMakesNurishment) */)
			continue;

		/* Simulate */
		n = borg_defend_aux(g, p1);

		/* Track "best" attack */
		if (n <= b_n)
			continue;

		/* Track best */
		b_g = g;
		b_n = n;
	}

	/* Nothing good */
	if (b_n <= 0) {
		return (FALSE);
	}

	/* Not worth it if sitting in a wall */
	if (b_g < BD_TPORTOTHER && borg_skill[BI_PASSWALL] &&
		 borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)
		return (FALSE);

	/* Note */
	borg_note(format("# Performing defence type %d with value %d", b_g, b_n));

	/* Instantiate */
	borg_simulate = FALSE;

	/* Instantiate */
	(void)borg_defend_aux(b_g, p1);
	/* Success */
	return (TRUE);
}
/*
 * Perma spells.  Some are cool to have on all the time, so long as their
 * mana cost is not too much.
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Prayer
 *   Temp Resist (either all or just cold/fire?)
 *   Shield
 *
 */
enum {
	BP_SPEED,
	BP_PROT_FROM_EVIL,
	BP_BLESS,
	BP_TELEPATHY,
	BP_RESIST_ALL,
	BP_RESIST_ALL_COLLUIN,

	BP_RESIST_F,
	BP_RESIST_C,
	BP_RESIST_A,
	BP_RESIST_P,
	BP_RESIST_FCE,

	BP_GOI,
	BP_SHIELD,
	BP_SHIELD_RACIAL,
	BP_HERO,
	BP_BERSERK,
	BP_BERSERK_POTION,

	BP_GLYPH,

	BP_MAX
};

/*
 * Bless/Prayer to prepare for battle
 */
static int borg_perma_aux_bless(void) {
	int fail_allowed = 5, cost;

	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* already blessed */
	if (borg_bless)
		return (0);

	/* Cant when Blind */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* XXX Dark */

	if (!borg_spell_okay_fail(REALM_LIFE, 3, 1, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_LIFE][3][1];
	cost = as->power;
	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	/* bless is a low priority */
	if (borg_simulate)
		return (1);

	/* No resting for a few minutes */
	borg_no_rest_prep = 10000;

	/* do it! */
	if (borg_spell(REALM_LIFE, 3, 1))
		return 1;

	return (0);
}
/* all resists */
static int borg_perma_aux_resist(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRACID] + borg_skill[BI_TRPOIS] +
			  borg_skill[BI_TRELEC] + borg_skill[BI_TRCOLD] >=
		 3)
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_NATURE, 2, 3, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_NATURE][2][3];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_spell_fail(REALM_NATURE, 2, 3, fail_allowed))

		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

	/* Value */
	return (2);

	/* default to can't do it. */
	return (0);
}

/* all resists from the cloak*/
static int borg_perma_aux_resist_colluin(void) {
	if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRACID] + borg_skill[BI_TRPOIS] +
			  borg_skill[BI_TRELEC] + borg_skill[BI_TRCOLD] >=
		 3)
		return (0);

	/* Only use it when Unique is close */
	if (!borg_fighting_unique)
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_equips_artifact(ART_JOSEPH))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_activate_artifact(ART_JOSEPH, FALSE))

		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

	/* Value */
	return (2);

	/* default to can't do it. */
	return (0);
}

/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_f(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (borg_skill[BI_TRFIRE] || !unique_on_level)
		return (0);

	if (borg_skill[BI_IFIRE])
		return (0);

	if (!borg_spell_okay_fail(REALM_ARCANE, 1, 7, fail_allowed))
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_ARCANE][1][7];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= borg_skill[BI_CURSP] / 20)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell_fail(REALM_ARCANE, 1, 7, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		/* Value */
		return (1);
	}

	/* default to can't do it. */
	return (0);
}
/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_c(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (borg_skill[BI_TRCOLD] || !unique_on_level)
		return (0);

	if (borg_skill[BI_ICOLD])
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_ARCANE, 1, 6, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_ARCANE][1][6];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= borg_skill[BI_CURSP] / 20)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell_fail(REALM_ARCANE, 1, 6, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		/* Value */
		return (1);
	}

	/* default to can't do it. */
	return (0);
}

/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_a(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (borg_skill[BI_TRACID] || !unique_on_level)
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_ARCANE, 2, 1, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_ARCANE][2][1];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= borg_skill[BI_CURSP] / 20)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell_fail(REALM_ARCANE, 2, 1, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		/* Value */
		return (1);
	}

	/* default to can't do it. */
	return (0);
}

/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_p(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (borg_skill[BI_TRPOIS] || !unique_on_level)
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_DEATH, 0, 5, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_DEATH][0][5];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= borg_skill[BI_CURSP] / 20)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell_fail(REALM_DEATH, 0, 5, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		/* Value */
		return (1);
	}
	/* default to can't do it. */
	return (0);
}
/* resist fire and cold for priests */
static int borg_perma_aux_resist_fce(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* cast if one drops and unique is near */
	if (borg_fighting_unique &&
		 ((borg_skill[BI_TRFIRE] || borg_skill[BI_IFIRE]) &&
		  (borg_skill[BI_TRELEC] || borg_skill[BI_IELEC]) &
				(borg_skill[BI_TRCOLD] || borg_skill[BI_ICOLD])))
		return (0);

	/* cast if both drop and no unique is near */
	if (!borg_fighting_unique &&
		 (borg_skill[BI_TRFIRE] || borg_skill[BI_TRCOLD]))
		return (0);

	/* no need if immune */
	if (borg_skill[BI_IFIRE] && borg_skill[BI_ICOLD] && borg_skill[BI_IELEC])
		return (0);

	/* Not needed if GOI is on */
	if (borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_NATURE, 0, 6, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_NATURE][0][6];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_spell_fail(REALM_NATURE, 0, 6, fail_allowed))

		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
	/* Value */
	return (2);

	/* default to can't do it. */
	return (0);
}

/*
 * Speed to prepare for battle
 */
static int borg_perma_aux_speed(void) {
	int fail_allowed = 7;
	int cost;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* already fast */
	if (borg_speed)
		return (0);

	/* only cast defence spells if fail rate is not too high */
	if (!borg_spell_okay_fail(REALM_SORCERY, 1, 5, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 3, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	if (borg_spell_okay_fail(REALM_SORCERY, 1, 4, fail_allowed)) {
		as = &borg_magics[REALM_SORCERY][1][4];
		cost = as->power;
	} else {
		as = &borg_magics[REALM_DEATH][2][3];
		cost = as->power;
	}

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (5);

	/* do it! */
	if (borg_spell_fail(REALM_SORCERY, 1, 5, fail_allowed) ||
		 borg_spell_fail(REALM_DEATH, 2, 3, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		return (5);
	}

	/* default to can't do it. */
	return (0);
}
static int borg_perma_aux_goi(void) {
	int fail_allowed = 5;
	int cost;
	borg_magic *as = &borg_magics[0][0][0];

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* if already protected */
	if (borg_shield || borg_goi || borg_wraith)
		return (0);

	/* only on 100 and when Morgy is near */
	if (borg_skill[BI_CDEPTH] != 100 || borg_fighting_unique <= 9)
		return (0);

	if (!borg_spell_okay_fail(REALM_LIFE, 3, 7, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_SORCERY, 3, 7, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	if (borg_spell_okay_fail(REALM_LIFE, 3, 7, fail_allowed)) {
		as = &borg_magics[REALM_LIFE][3][7];
	} else if (borg_spell_okay_fail(REALM_SORCERY, 3, 7, fail_allowed)) {
		as = &borg_magics[REALM_LIFE][3][7];
	}
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (3);

	/* do it! */
	if (borg_spell_fail(REALM_SORCERY, 3, 7, fail_allowed) ||
		 borg_spell_fail(REALM_LIFE, 3, 7, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		return (2);
	}

	/* default to can't do it. */
	return (0);
}
/*
 * Telepathy
 */
static int borg_perma_aux_telepathy(void) {
	int fail_allowed = 5, cost;

	borg_magic *as = &borg_magics[0][0][0];

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* already blessed */
	if (borg_esp || borg_skill[BI_ESP])
		return (0);

	/* must be able to */
	if (!borg_spell_okay_fail(REALM_ARCANE, 3, 7, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_SORCERY, 3, 3, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_SORCERY, 2, 4, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 0, 6, fail_allowed) &&
		 !borg_equips_activation(ACT_ESP, TRUE))
		return (0);

	/* Obtain the cost of the spell */
	if (borg_spell_legal_fail(REALM_ARCANE, 3, 7, fail_allowed)) {
		as = &borg_magics[REALM_ARCANE][3][7];
		cost = as->power;
	} else if (borg_spell_legal_fail(REALM_SORCERY, 3, 3, fail_allowed)) {
		as = &borg_magics[REALM_SORCERY][3][3];
		cost = as->power;
	} else if (borg_spell_legal_fail(REALM_SORCERY, 2, 4, fail_allowed)) {
		as = &borg_magics[REALM_SORCERY][2][4];
		cost = as->power;
	} else if (borg_spell_legal_fail(REALM_TRUMP, 0, 6, fail_allowed)) {
		as = &borg_magics[REALM_TRUMP][0][6];
		cost = as->power;
	} else {
		/* Randart Activation */
		cost = 0;
	}

	/* If its cheap, go ahead */
	if (cost >= 1 &&
		 cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell(REALM_ARCANE, 3, 7) || borg_spell(REALM_SORCERY, 3, 3) ||
		 borg_spell(REALM_SORCERY, 2, 4) || borg_spell(REALM_TRUMP, 0, 6) ||
		 borg_activate_activation(ACT_ESP, FALSE)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		return 1;
	}

	return (0);
}

static int borg_perma_aux_shield(void) {
	int fail_allowed = 5;
	int cost;
	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* if already protected */
	if (borg_shield || borg_goi || borg_wraith)
		return (0);

	if (!borg_spell_okay_fail(REALM_NATURE, 2, 2, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_NATURE][2][2];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_spell_fail(REALM_NATURE, 2, 2, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
		return (2);
	}
	/* default to can't do it. */
	return (0);
}

static int borg_perma_aux_shield_racial(void) {
	int cost;

	/* if already protected */
	if (borg_shield || borg_goi || borg_wraith)
		return (0);

	if (!borg_racial_check(RACE_GOLEM, 20))
		return (0);

	/* Obtain the cost of the spell */
	cost = 15;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURHP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_racial(RACE_GOLEM, 1)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
		return (2);
	}

	/* default to can't do it. */
	return (0);
}

static int borg_perma_aux_prot_evil(void) {
	int cost = 0;
	int fail_allowed = 5;
	borg_magic *as = &borg_magics[0][0][0];

	/* if already protected */
	if (borg_prot_from_evil)
		return (0);

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	if (!borg_spell_okay_fail(REALM_LIFE, 1, 5, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_LIFE][1][5];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (3);

	/* do it! */
	if (borg_spell_fail(REALM_LIFE, 1, 5, fail_allowed)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
		/* Value */
		return (3);
	}

	/* default to can't do it. */
	return (0);
}
/*
 * Hero to prepare for battle
 */
static int borg_perma_aux_hero(void) {
	int fail_allowed = 5, cost;

	borg_magic *as = &borg_magics[0][0][0];

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* already blessed */
	if (borg_hero || borg_berserk)
		return (0);

	/* Cant when Blind */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* XXX Dark */

	if (!borg_spell_okay_fail(REALM_LIFE, 3, 0, fail_allowed) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 0, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	if (borg_spell_legal_fail(REALM_LIFE, 3, 0, fail_allowed)) {
		as = &borg_magics[REALM_LIFE][3][0];
	} else if (borg_spell_legal_fail(REALM_DEATH, 2, 0, fail_allowed)) {
		as = &borg_magics[REALM_DEATH][2][0];
	}

	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	/* hero is a low priority */
	if (borg_simulate)
		return (1);

	/* do it! */
	if (borg_spell(REALM_LIFE, 3, 0) || borg_spell(REALM_DEATH, 2, 0)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;

		return 1;
	}

	return (0);
}

/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk(void) {
	int fail_allowed = 5, cost;

	borg_magic *as;

	/* increase the threshold */
	if (unique_on_level)
		fail_allowed = 10;
	if (borg_fighting_unique)
		fail_allowed = 15;

	/* already blessed */
	if (borg_hero || borg_berserk)
		return (0);

	/* Cant when Blind */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (0);

	/* XXX Dark */

	if (!borg_spell_okay_fail(REALM_DEATH, 2, 0, fail_allowed))
		return (0);

	/* Obtain the cost of the spell */
	as = &borg_magics[REALM_DEATH][2][2];
	cost = as->power;

	/* If its cheap, go ahead */
	if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7
											 : borg_skill[BI_CURSP] / 10))
		return (0);

	/* Simulation */
	/* Berserk is a low priority */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_spell(REALM_DEATH, 2, 0)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
		return 2;
	}

	return (0);
}
/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk_potion(void) {

	/* Saver the potions */
	if (!borg_fighting_unique)
		return (0);

	/* already blessed */
	if (borg_hero || borg_berserk)
		return (0);

	/* do I have any? */
	if (-1 == borg_slot(TV_POTION, SV_POTION_BESERK_STRENGTH))
		return (0);

	/* Simulation */
	/* Berserk is a low priority */
	if (borg_simulate)
		return (2);

	/* do it! */
	if (borg_quaff_potion(SV_POTION_BESERK_STRENGTH)) {
		/* No resting for a few minutes */
		borg_no_rest_prep = 10000;
		return (2);
	}

	return (0);
}

/* Glyph of Warding in a a-s corridor */
static int borg_perma_aux_glyph(void) {
	int i, wall_y, wall_x, wall_count = 0, y, x;
	int fail_allowed = 20;

	borg_grid *ag = &borg_grids[c_y][c_x];

	/* check to make sure a summoner is near */
	if (borg_kills_summoner == -1)
		return (0);

	/* Only while tunneling */
	if (goal != GOAL_DIGGING)
		return (0);

	/* make sure I have the spell */
	if (!borg_spell_okay_fail(REALM_LIFE, 1, 7, fail_allowed))
		return (0);

	/* He should not cast it while on an object.
	 * I have addressed this inadequately in borg9.c when dealing with
	 * messages.  The message "the object resists" will delete the glyph
	 * from the array.  Then I set a broken door on that spot, the borg ignores
	 * broken doors, so he won't loop.
	 */
	if ((ag->take) || (ag->feat == FEAT_GLYPH) ||
		 ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP)) ||
		 ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_TAIL)) ||
		 (ag->feat == FEAT_LESS) || (ag->feat == FEAT_MORE) ||
		 (ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN)) {
		return (0);
	}

	/* This spell is cast while he is digging and AS Corridor */
	/* Get grid */
	for (wall_x = -1; wall_x <= 1; wall_x++) {
		for (wall_y = -1; wall_y <= 1; wall_y++) {
			/* Acquire location */
			x = wall_x + c_x;
			y = wall_y + c_y;

			ag = &borg_grids[y][x];

			/* track adjacent walls */
			if ((ag->feat == FEAT_GLYPH) ||
				 ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_WALL_SOLID))) {
				wall_count++;
			}
		}
	}

	/* must be in a corridor */
	if (wall_count < 6)
		return (0);

	/* Simulation */
	if (borg_simulate)
		return (10);

	/* do it! */
	if (borg_spell_fail(REALM_LIFE, 1, 7, fail_allowed) ||
		 borg_read_scroll(SV_SCROLL_RUNE_OF_PROTECTION)) {
		/* Check for an existing glyph */
		for (i = 0; i < track_glyph_num; i++) {
			/* Stop if we already new about this glyph */
			if ((track_glyph_x[i] == c_x) && (track_glyph_y[i] == c_y))
				return (p1 - p2);
		}

		/* Track the newly discovered glyph */
		if ((i == track_glyph_num) && (i < track_glyph_size)) {
			borg_note("# Noting the creation of a corridor glyph.");
			track_glyph_num++;
			track_glyph_x[i] = c_x;
			track_glyph_y[i] = c_y;
		}
		return (p1 - p2);
	}

	/* default to can't do it. */
	return (0);
}

/*
 * Simulate/Apply the optimal result of using the given "type" of set-up
 */
static int borg_perma_aux(int what) {

	/* Analyze */
	switch (what) {
	case BP_SPEED: {
		return (borg_perma_aux_speed());
	}
	case BP_TELEPATHY: {
		return (borg_perma_aux_telepathy());
	}

	case BP_PROT_FROM_EVIL: {
		return (borg_perma_aux_prot_evil());
	}
	case BP_RESIST_ALL: {
		return (borg_perma_aux_resist());
	}
	case BP_RESIST_ALL_COLLUIN: {
		return (borg_perma_aux_resist_colluin());
	}
	case BP_RESIST_F: {
		return (borg_perma_aux_resist_f());
	}
	case BP_RESIST_C: {
		return (borg_perma_aux_resist_c());
	}
	case BP_RESIST_A: {
		return (borg_perma_aux_resist_a());
	}
	case BP_RESIST_P: {
		return (borg_perma_aux_resist_p());
	}
	case BP_RESIST_FCE: {
		return (borg_perma_aux_resist_fce());
	}
	case BP_BLESS: {
		return (borg_perma_aux_bless());
	}
	case BP_HERO: {
		return (borg_perma_aux_hero());
	}
	case BP_BERSERK: {
		return (borg_perma_aux_berserk());
	}
	case BP_BERSERK_POTION: {
		return (borg_perma_aux_berserk_potion());
	}
	case BP_GOI: {
		return (borg_perma_aux_goi());
	}

	case BP_SHIELD: {
		return (borg_perma_aux_shield());
	}
	case BP_SHIELD_RACIAL: {
		return (borg_perma_aux_shield_racial());
	}
	case BP_GLYPH: {
		return (borg_perma_aux_glyph());
	}
	}
	return (0);
}

/*
 * prepare to attack... this is setup for a battle.
 */
bool borg_perma_spell() {
	int n, b_n = 0;
	int g, b_g = -1;

	/* Simulate */
	borg_simulate = TRUE;

	/* Not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* No perma-spells until clevel 30 to avoid nasty loops with resting for
	 * mana regain
	 */
	if (borg_skill[BI_CLEVEL] < 30)
		return (FALSE);
	if (borg_skill[BI_CURSP] < borg_skill[BI_CURSP] * 7 / 10)
		return (FALSE);

	/* Analyze the possible setup moves */
	for (g = 0; g < BP_MAX; g++) {
		/* Simulate */
		n = borg_perma_aux(g);

		/* Track "best" move */
		if (n <= b_n)
			continue;

		/* Track best */
		b_g = g;
		b_n = n;
	}

	/* Nothing good */
	if (b_n <= 0) {
		return (FALSE);
	}

	/* Note */
	borg_note(
		 format("# Performing perma-spell type %d with value %d", b_g, b_n));

	/* Instantiate */
	borg_simulate = FALSE;

	/* Instantiate */
	(void)borg_perma_aux(b_g);

	/* No resting to recoop mana */
	borg_no_rest_prep = 10000;

	/* Success */
	return (TRUE);
}

/*
 * check to make sure there are no monsters around
 * that should prevent resting also make sure the ground
 * is safe for us.
 */
bool borg_check_rest(int y, int x) {
	int i, ii;
	int mon_scan, my_scan;
	bool borg_in_vault = FALSE;

	/* Do not rest in Sunlight */
	if (borg_skill[BI_FEAR_LITE] && borg_skill[BI_CDEPTH] == 0) {
		/* day time */
		if ((borg_skill[BI_HRTIME] >= 5) && (borg_skill[BI_HRTIME] <= 18)) {
			return (FALSE);
		}
	}

	/* Do not rest with Phial or Star if it hurts */
	if (borg_skill[BI_FEAR_LITE] &&
		 (borg_items[INVEN_LITE].name1 == ART_GALADRIEL ||
		  borg_items[INVEN_LITE].name1 == ART_EOS)) {
		return (FALSE);
	}

	/* now check the ground to see if safe. */
	if (borg_on_safe_grid(y, x) == FALSE)
		return (FALSE);

	/* Dont rest on the doorstep of a town shop */
	if (borg_skill[BI_CDEPTH] == 0 &&
		 ((borg_grids[y][x].feat >= FEAT_SHOP_HEAD &&
			borg_grids[y][x].feat <= FEAT_SHOP_TAIL) ||
		  (borg_grids[y][x].feat >= FEAT_BLDG_HEAD &&
			borg_grids[y][x].feat <= FEAT_BLDG_TAIL)))
		return (FALSE);

	/* Be concerned about resting where invisible/offscreen monsters lurk */
	/* if (need_see_inviso > borg_t-30) return (FALSE); */

	/* Do not rest recently after killing a multiplier */
	/* This will avoid the problem of resting next to */
	/* an unkown area full of breeders */
	if (when_last_kill_mult > (borg_t - 10) && when_last_kill_mult <= borg_t)
		return (FALSE);

	when_last_kill_mult = 0;

	/* Generally disturb_move is off */
	disturb_move = FALSE;

	/* No resting to recover if I just cast a prepatory spell
	 * which is what I like to do right before I take a stair,
	 * Unless I am down by half my SP or in a vault.
	 */
	if (borg_no_rest_prep >= 1 &&
		 (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 2 &&
		  !borg_munchkin_mode && borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 2))
		return (FALSE);

	/* Be concerned about the Monster Fear. */
	if (borg_fear_monsters[y][x] > borg_skill[BI_CURHP] / 10 &&
		 borg_skill[BI_CDEPTH] != 100 && !borg_munchkin_mode)
		return (FALSE);

	/* Be concerned if low on food */
	if ((borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISWEAK]) &&
		 !borg_munchkin_mode)
		return (FALSE);

	/* Be concerned during munchkin mode */
	if (borg_munchkin_mode && (borg_depth & DEPTH_SCARY))
		return (FALSE);

	/* Most of the time, its ok to rest in a vault */
	if ((borg_depth & DEPTH_VAULT)) {
		for (i = -1; i < 1; i++) {
			for (ii = -1; ii < 1; ii++) {
				/* check bounds */
				if (!in_bounds(c_y + i, c_x + ii))
					continue;

				if (borg_grids[c_y + i][c_x + ii].feat == FEAT_PERM_INNER)
					borg_in_vault = TRUE;
			}
		}
	}

	/* Dont worry about fears if in a vault */
	if (!borg_in_vault) {
		/* Be concerned about the Regional Fear. */
		if (borg_fear_region[y / 11][x / 11] > borg_skill[BI_CURHP] / 20 &&
			 borg_skill[BI_CDEPTH] != 100)
			return (FALSE);

		/* Be concerned about the Monster Fear. */
		if (borg_fear_monsters[y][x] > borg_skill[BI_CURHP] / 10 &&
			 borg_skill[BI_CDEPTH] != 100)
			return (FALSE);

		/* Be concerned about the Monster Danger. */
		if (borg_danger(y, x, 1, TRUE) > borg_skill[BI_CURHP] / 40 &&
			 borg_skill[BI_CDEPTH] >= 85)
			return (FALSE);

		/* Be concerned if low on food */
		if ((borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISWEAK] ||
			  borg_skill[BI_FOOD] < 2) &&
			 !borg_munchkin_mode)
			return (FALSE);
	}

	/* Examine all the monsters */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];
		monster_race *r_ptr = &r_info[kill->r_idx];

		int x9 = kill->x;
		int y9 = kill->y;
		int d;
		int p = 0;

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Dont worry about our ally */
		if (!kill->killer)
			continue;

		/* Distance */
		d = distance(y9, x9, y, x);

		/* Minimal distance */
		if (d > MAX_RANGE)
			continue;

		/* if too close to a Mold or other Never-Mover, don't rest */
		if (d == 2 && !(r_ptr->flags1 & RF1_NEVER_MOVE)) {
			/** Monster must be allowed to stand next to me
			 * Scan the grids near me to see if the monster is allowed
			 * to step there.
			 **/
			/* Scan grids adjacent to me */
			for (my_scan = 0; my_scan < 8; my_scan++) {
				int my_y = c_y + ddy[my_scan];
				int my_x = c_x + ddx[my_scan];

				/* Scan grids adjacent to monster to see if they are also adjacent
				 * to me */
				for (mon_scan = 0; mon_scan < 8; mon_scan++) {
					/* These grids are next to me and next to the monster. */
					if (kill->y + ddy[mon_scan] == my_y &&
						 kill->x + ddx[mon_scan] == my_x) {
						/* Terrain Types and Restrictions of Movement */
						if (can_go_monster(kill->y + ddy[mon_scan],
												 kill->x + ddx[mon_scan], kill->r_idx)) {
							/* Monster can cross the distance. Be concerned. */
							return (FALSE);
						}
					}
				}
			}
		}
		if (d == 1)
			return (FALSE);

		/* if too close to a Multiplier, don't rest */
		if (d < 10 && (r_ptr->flags2 & RF2_MULTIPLY))
			return (FALSE);

		/* If monster is asleep and far away, dont worry */
		if (!kill->awake && d > 8 && !borg_munchkin_mode)
			continue;

		/* If monster is near and in munchkin mode, be concerned */
		/* Special handling for the munchkin mode */
		if (borg_munchkin_mode && borg_los(y9, x9, y, x) &&
			 !(r_ptr->flags1 & RF1_NEVER_MOVE))
			return FALSE;

		/* one call for dangers-- */
		borg_full_damage = TRUE;
		p = borg_danger_aux(y, x, 1, i, TRUE, FALSE);
		borg_full_damage = FALSE;

		/* Ignore proximity checks while inside a vault */
		if (!borg_in_vault) {
			/* Real scary guys pretty close */
			if (d < 5 && (p > avoidance / 3) && !borg_munchkin_mode)
				return (FALSE);

			/* Scary guys kinda close, tinker with disturb near.
			* We do not want a borg with ESP stopping his rest
			* each round, having only rested one turn. So we set
			* disturb_move to true only if some scary guys are
			* somewhat close to us.
			*/
			if (d < 13 && (p > avoidance * 6 / 10))
				disturb_move = TRUE;

			if (d < 8 && (p > avoidance / 2))
				disturb_move = TRUE;
		}

		/* should check LOS... monster to me */
		if (borg_los(y9, x9, y, x) && kill->ranged_attack && p >= 1)
			return FALSE;

		/* Special handling for the munchkin mode */
		if (borg_munchkin_mode && borg_los(y9, x9, y, x) && kill->awake &&
			 !(r_ptr->flags1 & RF1_NEVER_MOVE))
			return FALSE;

		/* Can it flow to me? */

		/* The monster-to-grid flow check. */
		if (kill->awake) {
			borg_flow_clear_m();
			borg_digging = TRUE;
			borg_flow_enqueue_grid_m(y, x);
			borg_flow_spread_m(BORG_MON_FLOW, i, kill->r_idx);
			if (borg_flow_commit_m(kill->y, kill->x))
				return (FALSE);
		}
	}

	/* Otherwise ok */
	return TRUE;
}

/*
 * Attempt to recover from damage and such after a battle
 *
 * Note that resting while in danger is counter-productive, unless
 * the danger is low, in which case it may induce "farming".
 *
 * Note that resting while recall is active will often cause you
 * to lose hard-won treasure from nasty monsters, so we disable
 * resting when waiting for recall in the dungeon near objects.
 *
 * First we try spells/prayers, which are "free", and then we
 * try food, potions, scrolls, staffs, rods, artifacts, etc.
 *
 * XXX XXX XXX
 * Currently, we use healing spells as if they were "free", but really,
 * this is only true if the "danger" is less than the "reward" of doing
 * the healing spell, and if there are no monsters which might soon step
 * around the corner and attack.
 */
bool borg_recover(void) {
	int p = 0;
	int q;

	/*** Handle annoying situations ***/

	/* Refuel current torch */
	if ((borg_items[INVEN_LITE].tval == TV_LITE) &&
		 (borg_items[INVEN_LITE].sval == SV_LITE_TORCH)) {
		/* Refuel the torch if needed */
		if (borg_items[INVEN_LITE].pval < 2500) {
			if (borg_refuel_torch())
				return (TRUE);

			/* Take note */
			borg_note(format("# Need to refuel but cant!", p));

			/* Allow Pets to Roam so we dont hit them in the dark. */
			/* TODO SHOULD implement pet follow distances */
			/*p_ptr->pet_follow_distance = PET_STAY_AWAY;*/
		}
	}

	/* Refuel current lantern */
	if ((borg_items[INVEN_LITE].tval == TV_LITE) &&
		 (borg_items[INVEN_LITE].sval == SV_LITE_LANTERN)) {
		/* Refuel the lantern if needed */
		if (borg_items[INVEN_LITE].pval < 1000) {
			if (borg_refuel_lantern())
				return (TRUE);

			/* Take note */
			borg_note(format("# Need to refuel but cant!", p));
		}
	}

	/*** Do not recover when in danger ***/

	/* Look around for danger */
	p = borg_danger(c_y, c_x, 1, TRUE);

	/* Never recover in dangerous situations */
	if (p > avoidance / 4)
		return (FALSE);

	/*** Roll for "paranoia" ***/

	/* Base roll */
	q = rand_int(100);

	/* Half dead */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
		q = q - 10;

	/* Almost dead */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 4)
		q = q - 10;

	/*** Use "cheap" cures ***/

	/* Hack -- cure stun */
	if (borg_skill[BI_ISSTUN] && (q < 75)) {
		if (borg_activate_artifact(ART_RONOVE, FALSE) ||
			 borg_spell(REALM_LIFE, 0, 1) || borg_spell(REALM_LIFE, 0, 6) ||
			 borg_spell(REALM_ARCANE, 0, 7))

		{
			/* Take note */
			borg_note(format("# Cure Stun", p));

			return (TRUE);
		}
	}

	/* Hack -- cure stun */
	if (borg_skill[BI_ISHEAVYSTUN]) {
		if (borg_activate_artifact(ART_RONOVE, FALSE) ||
			 borg_spell(REALM_LIFE, 1, 2)) {
			/* Take note */
			borg_note(format("# Cure Heavy Stun", p));

			return (TRUE);
		}
	}

	/* Hack -- cure cuts */
	if (borg_skill[BI_ISCUT] && (q < 75)) {
		if (borg_activate_artifact(ART_RONOVE, FALSE) ||
			 borg_spell(REALM_LIFE, 1, 2) || borg_spell(REALM_NATURE, 0, 7) ||
			 borg_spell(REALM_LIFE, 0, 6)) {
			/* Take note */
			borg_note(format("# Cure Cuts", p));

			return (TRUE);
		}
	}

	/* Hack -- cure poison */
	if (borg_skill[BI_ISPOISONED] && (q < 75)) {
		if (borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 borg_spell(REALM_ARCANE, 1, 7) || borg_spell(REALM_NATURE, 0, 7) ||
			 borg_spell(REALM_LIFE, 1, 2) || borg_racial(NEPHILIM, 2)) {
			/* Take note */
			borg_note(format("# Cure poison", p));

			return (TRUE);
		}
	}

	/* Hack -- cure fear */
	if (borg_skill[BI_ISAFRAID] && (q < 75)) {
		if (borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 borg_spell(REALM_LIFE, 0, 3) || borg_spell(REALM_DEATH, 2, 0)) {
			/* Take note */
			borg_note(format("# Cure fear", p));

			return (TRUE);
		}
	}

	/* Hack -- satisfy hunger */
	if (((borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		  borg_skill[BI_ISWEAK]) &&
		 (q < 75)) {
		if (borg_spell_fail(REALM_LIFE, 0, 7, 65) ||
			 borg_spell_fail(REALM_ARCANE, 2, 6, 65) ||
			 borg_spell_fail(REALM_NATURE, 0, 3, 65) ||
			 borg_activate_activation(ACT_SATIATE, FALSE) ||
			 borg_read_scroll(SV_SCROLL_SATISFY_HUNGER)) {
			return (TRUE);
		}
	}

	/* Hack -- heal damage */
	if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) && (q < 75) &&
		 p == 0 && (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 4)) {
		if (borg_spell(REALM_LIFE, 1, 6) || borg_spell(REALM_NATURE, 1, 7) ||
			 borg_activate_activation(ACT_CURE_700, FALSE)) {
			/* Take note */
			borg_note(format("# heal damage (recovering)"));

			return (TRUE);
		}
	}

	/* cure experience loss with prayer */
	if (borg_skill[BI_ISFIXEXP] &&
		 (borg_activate_artifact(ART_LIFE, FALSE) ||
		  borg_spell(REALM_LIFE, 3, 3) || borg_spell(REALM_DEATH, 1, 7) ||
		  borg_racial(RACE_SKELETON, 1) || /*borg_racial(RACE_ZOMBIE, 1) ||*/
		  borg_activate_activation(ACT_REST_LIFE, FALSE) ||
		  borg_activate_activation(ACT_REST_ALL, FALSE))) {
		return (TRUE);
	}

	/* cure stat drain with prayer */
	if ((borg_skill[BI_ISFIXSTR] || borg_skill[BI_ISFIXINT] ||
		  borg_skill[BI_ISFIXWIS] || borg_skill[BI_ISFIXDEX] ||
		  borg_skill[BI_ISFIXCON] || borg_skill[BI_ISFIXCHR] ||
		  borg_skill[BI_ISFIXALL]) &&
		 (borg_spell(REALM_LIFE, 3, 3) || borg_racial(NEPHILIM, 2) ||
		  borg_activate_activation(ACT_REST_ALL, FALSE))) {
		return (TRUE);
	}

	/*** Use "expensive" cures ***/

	/* Hack -- cure stun */
	if (borg_skill[BI_ISSTUN] && (q < 25)) {
		if (borg_use_staff_fail(SV_STAFF_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_zap_rod(SV_ROD_HEALING) || borg_quaff_crit(FALSE) ||
			 borg_quaff_potion(SV_POTION_CURING)) {
			return (TRUE);
		}
	}

	/* Hack -- cure heavy stun */
	if (borg_skill[BI_ISHEAVYSTUN] && (q < 95)) {
		if (borg_quaff_crit(TRUE) || borg_quaff_potion(SV_POTION_CURING) ||
			 borg_use_staff_fail(SV_STAFF_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_zap_rod(SV_ROD_HEALING) ||
			 borg_activate_artifact(ART_RING_RAPHAEL, FALSE) ||
			 borg_activate_artifact(ART_EMMANUEL, FALSE) ||
			 borg_activate_artifact(ART_ROBE_MICHAEL, FALSE) ||
			 borg_activate_artifact(ART_CORSON, FALSE) ||
			 borg_racial(NEPHILIM, 2)) {
			return (TRUE);
		}
	}

	/* Hack -- cure cuts */
	if (borg_skill[BI_ISCUT] && (q < 25)) {
		if (borg_use_staff_fail(SV_STAFF_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_zap_rod(SV_ROD_HEALING) || borg_quaff_potion(SV_POTION_CURING) ||
			 borg_quaff_crit(borg_skill[BI_CURHP] < 10)) {
			return (TRUE);
		}
	}

	/* Hack -- cure poison */
	if (borg_skill[BI_ISPOISONED] && (q < 25)) {
		if (borg_quaff_potion(SV_POTION_CURE_POISON) ||
			 borg_quaff_potion(SV_POTION_SLOW_POISON) ||
			 borg_eat_food(SV_FOOD_WAYBREAD) ||
			 borg_eat_food(SV_FOOD_CURE_POISON) ||
			 borg_quaff_crit(borg_skill[BI_CURHP] < 10) ||
			 borg_use_staff_fail(SV_STAFF_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_quaff_potion(SV_POTION_CURING) ||
			 borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 borg_racial(NEPHILIM, 2)) {
			return (TRUE);
		}
	}

	/* Hack -- cure blindness */
	if (borg_skill[BI_ISBLIND] && (q < 25)) {
		if (borg_eat_food(SV_FOOD_CURE_BLINDNESS) ||
			 borg_quaff_potion(SV_POTION_CURE_LIGHT) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS) || borg_quaff_crit(FALSE) ||
			 borg_use_staff_fail(SV_STAFF_CURING) ||
			 borg_quaff_potion(SV_POTION_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_racial(NEPHILIM, 2)) {
			return (TRUE);
		}
	}

	/* Hack -- cure confusion */
	if (borg_skill[BI_ISCONFUSED] && (q < 25)) {
		if (borg_eat_food(SV_FOOD_CURE_CONFUSION) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS) || borg_quaff_crit(FALSE) ||
			 borg_use_staff_fail(SV_STAFF_CURING) || borg_zap_rod(SV_ROD_CURING) ||
			 borg_quaff_potion(SV_POTION_CURING)) {
			return (TRUE);
		}
	}

	/* Hack -- cure fear */
	if (borg_skill[BI_ISAFRAID] && (q < 25)) {
		if (borg_eat_food(SV_FOOD_CURE_PARANOIA) ||
			 borg_quaff_potion(SV_POTION_BOLDNESS) ||
			 borg_quaff_potion(SV_POTION_HEROISM) ||
			 borg_quaff_potion(SV_POTION_BESERK_STRENGTH) ||
			 borg_activate_artifact(ART_DANCING, FALSE) ||
			 borg_activate_activation(ACT_CURE_POISON, FALSE) ||
			 borg_racial(RACE_HALF_TROLL, 1) || borg_racial(NEPHILIM, 2)) {
			return (TRUE);
		}
	}

	/* Fix Hallucination */
	if (borg_skill[BI_ISIMAGE] && q < 25) {
		if (borg_racial(NEPHILIM, 2)) {
			return (TRUE);
		}
	}

	/* Hack -- satisfy hunger */
	if (((borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		  borg_skill[BI_ISWEAK]) &&
		 (q < 25)) {
		if (borg_read_scroll(SV_SCROLL_SATISFY_HUNGER) ||
			 borg_activate_activation(ACT_SATIATE, FALSE)) {
			return (TRUE);
		}
	}

	/* Hack -- heal damage */
	if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) && (q < 25)) {
		if (borg_zap_rod(SV_ROD_HEALING) ||
			 borg_quaff_potion(SV_POTION_CURE_SERIOUS) || borg_quaff_crit(FALSE) ||
			 borg_activate_artifact(ART_RONOVE, FALSE) ||
			 borg_activate_activation(ACT_CURE_MW, FALSE) ||
			 borg_activate_activation(ACT_CURE_LW, FALSE)) {
			return (TRUE);
		}
	}

	/* Get some Mana.  Consume a junky staff or Rod */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 3 / 10)) {
		if (borg_eat_magic(TRUE, 25)) {
			borg_note("# Use Eat Magic (from junk)");
			return (TRUE);
		}
	}

	/* Get some Mana Consume a staff or Rod */
	if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 2 / 10)) {
		if (borg_eat_magic(FALSE, 25)) {
			borg_note("# Use Eat Magic");
			return (TRUE);
		}
	}

	/* Hack -- Rest to recharge Rods of Healing or Recall, or munchkin mode
	 * attack rods */
	if ((borg_has[ROD_HEAL] || borg_has[ROD_RECALL]) ||
		 (borg_munchkin_mode && borg_skill[BI_AROD1])) {
		/* Step 1.  Recharge just 1 rod. */
		if ((borg_has[ROD_HEAL] &&
			  borg_items[borg_slot(TV_ROD, SV_ROD_HEALING)].pval !=
					borg_has[ROD_HEAL] &&
			  borg_skill[BI_CDEPTH] >= 1) ||
			 (borg_has[ROD_RECALL] &&
			  !borg_items[borg_slot(TV_ROD, SV_ROD_RECALL)].pval) ||
			 (borg_munchkin_mode &&
			  ((borg_slot(TV_ROD, SV_ROD_ACID_BOLT) >= 0 &&
				 !borg_items[borg_slot(TV_ROD, SV_ROD_ACID_BOLT)].pval) ||
				(borg_slot(TV_ROD, SV_ROD_COLD_BOLT) >= 0 &&
				 !borg_items[borg_slot(TV_ROD, SV_ROD_COLD_BOLT)].pval) ||
				(borg_slot(TV_ROD, SV_ROD_FIRE_BOLT) >= 0 &&
				 !borg_items[borg_slot(TV_ROD, SV_ROD_FIRE_BOLT)].pval) ||
				(borg_slot(TV_ROD, SV_ROD_ELEC_BOLT) >= 0 &&
				 !borg_items[borg_slot(TV_ROD, SV_ROD_ELEC_BOLT)].pval)))) {
			/* Mages can cast the recharge spell */
			if (borg_has[ROD_HEAL] && borg_recharging())
				return (TRUE);

			/* If I must recharge a rod, foget about the no_rest_prep rules */
			borg_no_rest_prep = 0;

			/* Rest until at least one recharges */
			if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] &&
				 !borg_skill[BI_ISHUNGRY] && !borg_skill[BI_ISPOISONED] &&
				 borg_check_rest(c_y, c_x)) {
				/* Take note */
				borg_note("# Resting to recharge a rod...");

				/* Reset the Bouncing-borg Timer */
				time_this_panel = 0;

				/* Rest until done */
				borg_keypress(',');

				/* Done */
				return (TRUE);
			}
		}
	}

	/*** Just Rest (nested for easier debugging) ***/
	if (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISPOISONED] &&
		 !borg_skill[BI_ISCUT] && !borg_goi && !borg_wraith &&
		 borg_see_inv <= 0 && borg_skill[BI_CDEPTH] != 100) {
		if (!borg_skill[BI_ISWEAK] &&
			 (!borg_skill[BI_ISHUNGRY] || borg_skill[BI_FOOD] > 4) &&
			 p < avoidance / 5) {
			if (borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE] ||
				 borg_skill[BI_ISAFRAID] || borg_skill[BI_ISSTUN] ||
				 borg_skill[BI_ISHEAVYSTUN] ||
				 borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] ||
				 borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 7 / 10) {
				if (borg_check_rest(c_y, c_x) &&
					 p <= borg_fear_region[c_y / 11][c_x / 11] &&
					 goal != GOAL_RECOVER && goal != GOAL_TOWN) {
					if (!borg_skill[BI_PASSWALL] ||
						 ((borg_skill
								 [BI_PASSWALL] /* || borg_race == RACE_SPECTRE */) &&
						  (borg_grids[c_y][c_x].feat < FEAT_SECRET ||
							borg_grids[c_y][c_x].feat > FEAT_PERM_SOLID))) {
						/* Do not rest certain hours if in town and fearing light */
						if ((borg_skill[BI_FEAR_LITE] &&
							  (borg_skill[BI_CDEPTH] == 0 &&
								borg_skill[BI_HRTIME] <= 5 &&
								borg_skill[BI_HRTIME] >= 18)) ||
							 !borg_skill[BI_FEAR_LITE] || borg_skill[BI_CDEPTH] >= 1) {
							/* no resting in munchking mode if not on a stair */
							if (!borg_munchkin_mode ||
								 borg_grids[c_y][c_x].feat == FEAT_MORE ||
								 borg_grids[c_y][c_x].feat == FEAT_LESS) {

								/* check for then call lite in dark room before resting
								 */
								if (!borg_check_lite_only()) {
									/* Take note */
									borg_note(format("# Resting to recover HP/SP..."));

									/* Rest until done */
									borg_keypress('R');
									borg_keypress('&');
									borg_keypress('\n');

									/* Reset our panel clock, we need to be here */
									time_this_panel = 0;

									/* reset the inviso clock to avoid loops */
									need_see_inviso = borg_t - 50;

									/* Done */
									return (TRUE);
								} else {
									/* Must have been a dark room */
									borg_note(format("# Lighted the darkened room "
														  "instead of resting."));
									return (TRUE);
								}
							}
						}
					}
				}
			}
		}
	}

	/* Hack to recharge mana if a low level mage or priest */
	if (borg_skill[BI_MAXSP] &&
		 (borg_skill[BI_CLEVEL] <= 40 || borg_skill[BI_CDEPTH] >= 85) &&
		 borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 8 / 10) &&
		 borg_check_rest(c_y, c_x)) {
		if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] &&
			 (!borg_skill[BI_ISHUNGRY] || borg_skill[BI_FOOD] > 4) &&
			 !borg_skill[BI_ISPOISONED] &&
			 (!borg_munchkin_mode || borg_grids[c_y][c_x].feat == FEAT_MORE ||
			  borg_grids[c_y][c_x].feat == FEAT_LESS)) {
			/* Take note */
			borg_note(format("# Resting to gain Mana. (danger %d)...", p));

			/* Rest until done */
			borg_keypress('R');
			borg_keypress('*');
			borg_keypress('\n');

			/* Done */
			return (TRUE);
		}
	}
	/* Hack to recharge mana if a low level mage in munchkin mode */
	if (borg_skill[BI_MAXSP] && borg_munchkin_mode == TRUE &&
		 (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] ||
		  borg_skill[BI_CURHP] < borg_skill[BI_MAXHP]) &&
		 borg_check_rest(c_y, c_x)) {
		if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] &&
			 !borg_skill[BI_ISHUNGRY] && !borg_skill[BI_ISPOISONED] &&
			 borg_skill[BI_FOOD] >= 2 &&
			 (borg_grids[c_y][c_x].feat == FEAT_LESS ||
			  borg_grids[c_y][c_x].feat == FEAT_MORE)) {
			/* Take note */
			borg_note(
				 format("# Resting to gain munchkin HP/mana. (danger %d)...", p));

			/* Rest until done */
			borg_keypress('R');
			borg_keypress('*');
			borg_keypress('\n');

			/* Done */
			return (TRUE);
		}
	}

	/* Hack to heal blindness if in munchkin mode */
	if (borg_skill[BI_ISBLIND] && borg_munchkin_mode == TRUE) {
		/* Take note */
		borg_note("# Resting to cure problem. (danger %d)...");

		/* Rest until done */
		borg_keypress('R');
		borg_keypress('*');
		borg_keypress('\n');

		/* Done */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Take one "step" towards the given location, return TRUE if possible
 */
static bool borg_play_step(int y2, int x2) {
	borg_grid *ag;
	borg_grid *ag2;
	/*borg_kill *kill;*/

	int dir, x, y, ox, oy, i;
	int d;
	int o_y = 0, o_x = 0, door_found = 0;

	/* Danger used later */
	d = borg_danger(c_y, c_x, 1, TRUE);

	/* Breeder levels, close all doors */
	if ((borg_depth & DEPTH_BREEDER)) {
		/* scan the adjacent grids */
		for (ox = -1; ox <= 1; ox++) {
			for (oy = -1; oy <= 1; oy++) {
				/* skip our own spot */
				if ((oy + c_y == c_y) && (ox + c_x == c_x))
					continue;

				/* skip our orignal goal */
				if ((oy + c_y == y2) && (ox + c_x == x2))
					continue;

				/* Acquire location */
				ag = &borg_grids[oy + c_y][ox + c_x];

				/* skip non open doors */
				if (ag->feat != FEAT_OPEN)
					continue;

				/* skip monster on door */
				if (ag->kill)
					continue;

				/* Skip repeatedly closed doors */
				if (track_door_num >= 255)
					continue;

				/* save this spot */
				o_y = oy;
				o_x = ox;
				door_found = 1;
			}
		}

		/* Is there a door to close? */
		if (door_found) {
			/* Get a direction, if possible */
			dir = borg_goto_dir(c_y, c_x, c_y + o_y, c_x + o_x);

			/* Obtain the destination */
			x = c_x + ddx[dir];
			y = c_y + ddy[dir];

			/* Hack -- set goal */
			g_x = x;
			g_y = y;

			/* Close */
			borg_note("# Closing a door");
			borg_keypress('c');
			borg_keypress(I2D(dir));

			/* Check for an existing flag */
			for (i = 0; i < track_door_num; i++) {
				/* Stop if we already new about this door */
				if ((track_door_x[i] == x) && (track_door_y[i] == y))
					return (TRUE);
			}

			/* Track the newly closed door */
			if (i == track_door_num && i < track_door_size) {

				borg_note("# Noting the closing of a door.");
				track_door_num++;
				track_door_x[i] = x;
				track_door_y[i] = y;
			}
			return (TRUE);
		}
	}

	/* Get a direction, if possible */
	dir = borg_goto_dir(c_y, c_x, y2, x2);

	/* We have arrived */
	if (dir == 5)
		return (FALSE);

	/* Obtain the destination */
	x = c_x + ddx[dir];
	y = c_y + ddy[dir];

	/* Access the grid we are stepping on */
	ag = &borg_grids[y][x];

	/* Hack -- set goal */
	g_x = x;
	g_y = y;

	/* Monsters -- Attack */
	if (ag->kill) {
		int chance;

		borg_kill *kill = &borg_kills[ag->kill];
		monster_race *r_ptr = &r_info[kill->r_idx];

		/* can't attack someone if afraid! */
		if (borg_skill[BI_ISAFRAID])
			return (FALSE);

		/* not if digging a tunnel */
		if (goal == GOAL_DIGGING || goal == GOAL_UNREACH) {
			return (FALSE);
		}

		/* Not if in munchkin mode.  He can't plow through a monster on his way to
		 * a stair. */
		if (borg_munchkin_mode == TRUE)
			return (FALSE);

		/* Check the chance to hit */
		chance = (borg_skill[BI_THN] +
					 ((borg_skill[BI_TOHIT] + borg_items[INVEN_WIELD].to_h) *
					  BTH_PLUS_ADJ));

		/* Do I stand at least 80% chance of hitting hit */
		if (chance < r_ptr->ac * 8 / 10)
			return (FALSE);

		/* Hack -- ignore Maggot until later.  */
		if (kill->unique && borg_skill[BI_CDEPTH] == 0 &&
			 borg_skill[BI_CLEVEL] < 5)
			return (FALSE);

		/* Avoid walking into pets sometimes */
		if ((!kill->killer) && (borg_skill[BI_ISSTUN] || borg_skill[BI_ISIMAGE] ||
										borg_skill[BI_ISCONFUSED] || borg_berserk))
			return (FALSE);

		/* Message */
		borg_note(format("# Walking into a '%s' at (%d,%d)",
							  r_name + r_info[kill->r_idx].name, kill->y, kill->x));

		/* Walk into it */
		if (my_no_alter || (kill->y == 0 && kill->x == 0) ||
			 ((kill->ally) && time_this_panel < 200 &&
			  (borg_grids[c_y][c_x].feat == FEAT_FLOOR ||
				borg_grids[c_y][c_x].feat == FEAT_OPEN))) {
			borg_keypress(';');
			my_no_alter = FALSE;
		} else {
			borg_keypress('+');
		}
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Objects -- Take */
	if (ag->take) {
		borg_take *take = &borg_takes[ag->take];

		/*** Handle Chests ***/
		/* The borg will cheat when it comes to chests.
		 * He does not have to but it makes him faster and
		 * it does not give him any information that a
		 * person would not know by looking at the trap.
		 * So there is no advantage to the borg.
		 */

		if (strstr(k_name + k_info[take->k_idx].name, "chest") &&
			 !strstr(k_name + k_info[take->k_idx].name, "Ruined")) {
			cave_type *c_ptr = &cave[y2][x2];
			object_type *o_ptr = &o_list[c_ptr->o_idx];

			borg_take *take = &borg_takes[ag->take];

			/* Unknown, Search it */
			if (!object_known_p(o_ptr) && chest_traps[o_ptr->pval]) {
				borg_note(format("# Searching a '%s' at (%d,%d)",
									  k_name + k_info[take->k_idx].name, take->y,
									  take->x));

				/* Walk onto it */
				borg_keypress('0');
				borg_keypress('5');
				borg_keypress('s');
				return (TRUE);
			}

			/* Traps. Disarm it w/ fail check */
			if (o_ptr->pval >= 1 && object_known_p(o_ptr) &&
				 borg_skill[BI_DEV] - o_ptr->pval >= borg_chest_fail_tolerance) {
				borg_note(format("# Disarming a '%s' at (%d,%d)",
									  k_name + k_info[take->k_idx].name, take->y,
									  take->x));

				/* Open it */
				borg_keypress('D');
				borg_keypress(I2D(dir));
				return (TRUE);
			}

			/* No trap, or unknown trap that passed above checks - Open it */
			if (o_ptr->pval < 0 || !object_known_p(o_ptr)) {
				borg_note(format("# Opening a '%s' at (%d,%d)",
									  k_name + k_info[take->k_idx].name, take->y,
									  take->x));

				/* Open it */
				borg_keypress('o');
				borg_keypress(I2D(dir));
				return (TRUE);
			}

			/* Empty chest */
			/* continue in routine and pick it up */
		}

		/*** Handle Orb of Draining ***/

		/* Priest/Paladin borgs who have very limited ID ability can save some
		 * money and
		 * inventory space my casting Orb of Draining on objects.  Cursed objects
		 * will melt
		 * under the Orb of Draining spell.  This will save the borg from carrying
		 * the item
		 * around until he can ID it.
		 *
		 * By default, the flag ORBED is set to false when an item is created.  If
		 * the borg
		 * gets close to an item, and the conditions are favorable, he will cast
		 * OoD on the
		 * item and change the flag.
		 */
		if (take->orbed == FALSE && take->tval >= 16 && take->tval <= 45) {
			if (distance(take->y, take->x, c_y, c_x) == 1) {
				int yy;
				int xx;
				bool do_cast = TRUE;

				/* Only cast the spell if a Pet is not in the AoE.  It could cause
				 * them to turn. */
				for (i = 0; i < 24; i++) {
					/* Grid in that direction */
					xx = take->x + borg_ddx_ddd[i];
					yy = take->y + borg_ddy_ddd[i];

					/* Access the grid */
					if (!in_bounds(yy, xx))
						continue;
					ag2 = &borg_grids[yy][xx];
					if (ag2->kill) {
						borg_kill *kill = &borg_kills[ag2->kill];
						monster_race *r_ptr = &r_info[kill->r_idx];

						if (kill->ally && !(r_ptr->flags3 & RF3_GOOD))
							do_cast = FALSE;
					}
				}

				if (do_cast == TRUE && borg_spell_okay_fail(REALM_LIFE, 1, 4, 25)) {
					/* Target the Take location */
					borg_target(take->y, take->x);

					/* Cast the prayer */
					borg_spell(REALM_LIFE, 1, 4);

					/* Message */
					borg_note("# Orbing an object to check for cursed item.");

					/* use the old target */
					borg_keypress('5');

					/* Change the take flag */
					take->orbed = TRUE;

					/* check the blast radius of the prayer for other items */
					for (i = 0; i < 24; i++) {
						/* Extract the location */
						int xx = take->x + borg_ddx_ddd[i];
						int yy = take->y + borg_ddy_ddd[i];

						/* Check the grid for a take */
						if (!in_bounds(yy, xx))
							continue;
						ag2 = &borg_grids[yy][xx];
						if (ag2->take) {
							/* This item was orbed (mostly true)*/
							borg_takes[borg_grids[yy][xx].take].orbed = TRUE;
						}
					}

					/* Return */
					return (TRUE);
				}
			}
		}

		/*** Handle other takes ***/
		/* Message */
		borg_note(format("# Walking onto a '%s' at (%d,%d)",
							  k_name + k_info[take->k_idx].name, take->y, take->x));

		/* Walk onto it */
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Glyph of Warding */
	if (ag->feat == FEAT_MINOR_GLYPH || ag->feat == FEAT_GLYPH) {
		/* Message */
		borg_note(format("# Walking onto a glyph of warding."));

		/* Walk onto it */
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Traps -- disarm -- */
	if (borg_skill[BI_CUR_LITE] && !borg_skill[BI_ISBLIND] &&
		 !borg_skill[BI_ISCONFUSED] && !(borg_depth & DEPTH_SCARY) &&
		 (ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP)) {

		/* NOTE: If a scary guy is on the level, we allow the borg to run over the
		 * trap in order to escape this level.
		 */

		/* allow "destroy doors" */
		if (borg_spell(REALM_ARCANE, 0, 6)) {
			borg_note("# Unbarring ways");
			borg_keypress(I2D(dir));
			return (TRUE);
		}
		/* allow "destroy doors" */
		if (borg_spell(REALM_CHAOS, 0, 1) ||
			 borg_activate_activation(ACT_DEST_DOOR, FALSE)) {
			borg_note("# Unbarring ways");
			return (TRUE);
		}

		/* Disarm */
		borg_note("# Disarming a trap");
		borg_keypress('D');
		borg_keypress(I2D(dir));

		/* We are not sure if the trap will get 'untrapped'. pretend it will*/
		ag->feat = FEAT_NONE;
		return (TRUE);
	}

	/* Closed Doors -- Open */
	if ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_HEAD + 0x07) &&
		 borg_skill[BI_CDEPTH]) {
		/* monsters are adjacent to the door in question (not good) */
		bool monsters = FALSE;

		/* Paranoia XXX XXX XXX */
		if (!rand_int(100))
			return (FALSE);

		/* do not walk through the door if a monster is touching the door */
		for (i = 0; i < 8; i++) {
			/* Grid in that direction */
			x = x2 + ddx_ddd[i];
			y = y2 + ddy_ddd[i];

			/* Access the grid */
			ag2 = &borg_grids[y][x];

			/* If monster adjacent to the door, dont even try to open the door */
			if (ag2->kill) {
				/* Monster needs to be on the other side of the door. */
				if (distance(y, x, c_y, c_x) == 2)
					monsters = TRUE;
			}
		}

		/* Avoid some loops */
		if (!rand_int(10))
			monsters = TRUE;

		/* PassWalls can walk through which is good while low level because
		 * monsters cant follow */
		if (borg_skill[BI_PASSWALL] && !borg_allies &&
			 !monsters /* || borg_race == RACE_SPECTRE */) {
			/* Check for an existing flag */
			for (i = 0; i < track_door_num; i++) {
				/* we already know about this door */
				if ((track_door_x[i] == x2) && (track_door_y[i] == y2)) {
					borg_keypress(I2D(dir));
					return (TRUE);
				}
			}

			/* Track the newly examined door */
			if (i == track_door_num && i < track_door_size) {

				borg_note("# Noting the examined closed door.");
				track_door_num++;
				track_door_x[i] = x2;
				track_door_y[i] = y2;
			}
			borg_keypress(I2D(dir));
			return (TRUE);
		}

		/* Not a good idea to open locked doors if a monster
		 * is next to the borg beating on him
		 */
		/* scan the adjacent grids */
		for (i = 0; i < 8; i++) {
			/* Grid in that direction */
			x = c_x + ddx_ddd[i];
			y = c_y + ddy_ddd[i];

			/* Access the grid */
			ag2 = &borg_grids[y][x];

			/* If monster adjacent to me and I'm weak, dont
			 * even try to open the door
			 */
			if (ag2->kill && borg_skill[BI_CLEVEL] < 15 &&
				 !borg_skill[BI_ISAFRAID])
				return (FALSE);
		}

		/* Open */
		if (my_need_alter) {
			borg_keypress('+');
			my_need_alter = FALSE;
		} else {
			borg_note("# Opening a door");
			borg_keypress('0');
			borg_keypress('9');
			borg_keypress('o');
		}
		borg_keypress(I2D(dir));

		/* Remove this closed door from the list.
			* Its faster to clear all doors from the list
			* then rebuild the list.
			*/
		if (track_closed_num) {
			track_closed_num = 0;
		}

		return (TRUE);
	}

	/* Jammed Doors -- Bash or destroy */
	if ((ag->feat >= FEAT_DOOR_HEAD + 0x08) && (ag->feat <= FEAT_DOOR_TAIL) &&
		 borg_skill[BI_CDEPTH]) {
		/* Paranoia XXX XXX XXX */
		if (!rand_int(100))
			return (FALSE);

		/* PassWalls can walk through which is good while low level because
		 * monsters cant follow */
		if (borg_skill[BI_PASSWALL] /* || borg_race == RACE_SPECTRE */) {
			borg_keypress(I2D(dir));
			return (TRUE);
		}

		/* Not if hungry */
		if (borg_skill[BI_ISWEAK])
			return (FALSE);

		if (borg_spell(REALM_ARCANE, 0, 6)) {
			borg_keypress(I2D(dir));
			borg_note("# Destroying doors");
			return (TRUE);
		}
		/* allow "destroy doors" */
		if (borg_spell(REALM_CHAOS, 0, 1) ||
			 borg_activate_activation(ACT_DEST_DOOR, FALSE)) {
			borg_note("# Unbarring ways");
			return (TRUE);
		}

		/* Mega-Hack -- allow "stone to mud" */
		if (borg_aim_wand(SV_WAND_STONE_TO_MUD) ||
			 borg_spell(REALM_ARCANE, 2, 4) || borg_spell(REALM_NATURE, 1, 0) ||
			 borg_spell(REALM_CHAOS, 0, 6) ||
			 borg_mutation(
				  COR1_EAT_ROCK, FALSE, 40,
				  FALSE) || /* He should be standing right next to it here */
			 borg_racial(RACE_HALF_GIANT, 1)) {
			borg_note("# Melting a door");
			borg_keypress(I2D(dir));

			/* Remove this closed door from the list.
			* Its faster to clear all doors from the list
			* then rebuild the list.
			*/
			if (track_closed_num) {
				track_closed_num = 0;
			}

			return (TRUE);
		}

		/* Bash */
		borg_note("# Bashing a door");
		borg_keypress('B');
		/* Remove this closed door from the list.
		 * Its faster to clear all doors from the list
		 * then rebuild the list.
		 */
		if (track_closed_num) {
			track_closed_num = 0;
		}
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Rubble, Treasure, Seams, Walls -- Tunnel or Melt */
	if (ag->feat >= FEAT_SECRET && ag->feat <= FEAT_WALL_SOLID) {

		/* Mega-Hack -- prevent infinite loops */
		if (rand_int(500) < 3 && !(borg_depth & DEPTH_VAULT))
			return (FALSE);

		/* PassWalls can walk through which is good while low level because
		 * monsters cant follow */
		if (borg_skill[BI_PASSWALL] && goal != GOAL_DIGGING &&
			 ((ag->feat != FEAT_MAGMA_H && ag->feat != FEAT_MAGMA_K &&
				ag->feat != FEAT_QUARTZ_K && ag->feat != FEAT_QUARTZ_H &&
				goal != GOAL_BORE && goal != GOAL_FLEE) ||
			  (goal == GOAL_BORE || goal == GOAL_FLEE))) {
			borg_keypress(I2D(dir));
			return (TRUE);
		}

		/* Not if hungry */
		if (borg_skill[BI_ISWEAK])
			return (FALSE);

		/* Not when generally exploring */
		if (goal == GOAL_DARK &&
			 (ag->feat > FEAT_RUBBLE || borg_skill[BI_CLEVEL] < 10))
			return (FALSE);

		/* Mega-Hack -- allow "stone to mud" */
		if (borg_aim_wand(SV_WAND_STONE_TO_MUD) ||
			 borg_spell(REALM_ARCANE, 2, 4) || borg_spell(REALM_NATURE, 1, 0) ||
			 borg_spell(REALM_CHAOS, 0, 6) ||
			 borg_mutation(COR1_EAT_ROCK, FALSE, 40, FALSE) ||
			 borg_racial(RACE_HALF_GIANT, 1) ||
			 borg_activate_artifact(ART_DESTINY, INVEN_WIELD) ||
			 borg_activate_activation(ACT_STONE_MUD, FALSE)) {
			borg_note("# Melting a wall");
			borg_keypress(I2D(dir));

			/* Remove mineral veins from the list.
			 * Its faster to clear all veins from the list
			 * then rebuild the list.
			 */
			if (track_vein_num) {
				track_vein_num = 0;
			}
			return (TRUE);
		}

		/* No tunneling if in danger */
		if (d >= borg_skill[BI_CURHP] / 4)
			return (FALSE);

#if 0
		/* Some borgs just cant dig it */
		if ((borg_skill[BI_DIG] < BORG_DIG &&
			 (borg_items[weapon_swap].tval != TV_DIGGING && borg_items[weapon_swap].tval != TV_DIGGING)) &&
			(borg_skill[BI_DIG] < BORG_DIG + 20) */ && goal != GOAL_VAULT */)
	    {
			/* Clear the flow grids and do not dig */
			goal = 0;
			return (FALSE);
		}
#endif
		/* No digging if a monster is right next to me */
		for (i = 0; i < 8; i++) {
			if (borg_grids[c_y + ddy[i]][c_x + ddx[i]].kill) {
				goal = 0;
				return (FALSE);
			}
		}

		/* Tunnel */
		/* If I have a shovel then use it */
		if (borg_items[weapon_swap].tval == TV_DIGGING &&
			 !(borg_items[INVEN_WIELD].cursed)) {
			borg_note("# Swapping Digger");
			borg_keypress(ESCAPE);
			borg_keypress('w');
			borg_keypress(I2A(weapon_swap));
			borg_keypress(' ');
			borg_keypress(' ');
		}
		borg_note("# Digging through wall/etc");
		borg_keypress('0');
		borg_keypress('9');
		borg_keypress('9');
		/* Some borgs will dig more */
		if (borg_worships_gold) {
			borg_keypress('9');
		}

		borg_keypress('T');
		borg_keypress(I2D(dir));

		/* Remove mineral veins from the list.
		 * Its faster to clear all veins from the list
		 * then rebuild the list.
		 */
		if (track_vein_num) {
			track_vein_num = 0;
		}
		return (TRUE);
	}

	/* Shops -- Enter */
	if ((ag->feat >= FEAT_SHOP_HEAD) && (ag->feat <= FEAT_SHOP_TAIL)) {
		/* Message */
		borg_note(
			 format("# Entering a '%d' shop", (ag->feat - FEAT_SHOP_HEAD) + 1));

		/* Enter the shop */
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Special Bldgs */
	if ((ag->feat >= FEAT_BLDG_HEAD) && (ag->feat <= FEAT_BLDG_TAIL)) {
		cptr name = (f_name + f_info[ag->feat - FEAT_SHOP_HEAD].name);

		/* Message */
		borg_note(
			 format("# Entering  a '%c' special bldg (feat: %d)", name, ag->feat));

		/* Enter the shop */
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* Perhaps the borg could search for traps as he walks around level one. */
	if (borg_skill[BI_MAXCLEVEL] <= 5 && borg_skill[BI_CDEPTH] &&
		 !(borg_depth & DEPTH_SCARY) && !borg_skill[BI_ISSEARCHING] &&
		 borg_needs_searching && d < avoidance / 5) {
		borg_keypress('S');
		borg_note("# Searching engaged.");
	}

	/* Turn off the searching if needed */
	if ((!borg_needs_searching || d >= avoidance / 5 ||
		  (borg_depth & DEPTH_SCARY)) &&
		 (borg_skill[BI_ISSEARCHING] || p_ptr->searching)) {
		borg_keypress('S');
		borg_note("# Searching disengaged.");
	}

	/* Walk in that direction */
	if (my_need_alter) {
		borg_keypress('+');
		my_need_alter = FALSE;
	} else {
		/* nothing */
	}

	/* Note if Borg is searching */
	if (borg_skill[BI_ISSEARCHING] && borg_verbose)
		borg_note("# Borg is searching while walking.");

	borg_keypress(I2D(dir));

	/* Stand stairs up */
	if (goal_less) {
		/* Up stairs */
		if (ag->feat == FEAT_LESS) {
			/* Stand on stairs */
			borg_on_upstairs = TRUE;

			/* reset the flee to stair flag */
			goal_less = FALSE;

			/* Success */
			return (TRUE);
		}
	}

	/* Did something */
	return (TRUE);
}

/*
 * Act twitchy
 */
bool borg_twitchy(void) {
	int dir;

	/* This is a bad thing */
	borg_note("# Twitchy!");

	/* try to phase out of it */
	if (borg_caution_phase(15, 2) &&
		 (borg_spell_fail(REALM_ARCANE, 0, 4, 40) ||
		  borg_spell_fail(REALM_SORCERY, 0, 1, 40) ||
		  borg_spell_fail(REALM_TRUMP, 0, 0, 40) ||
		  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
		  borg_activate_artifact(ART_NYNAULD, FALSE) ||
		  borg_read_scroll(SV_SCROLL_PHASE_DOOR))) {
		/* We did something */
		return (TRUE);
	}
	/* Pick a random direction */
	dir = randint(9);

	/* Hack -- set goal */
	g_x = c_x + ddx[dir];
	g_y = c_y + ddy[dir];

	/* Maybe alter */
	if (rand_int(100) < 10 && dir != 5) {
		/* Send action (alter) */
		borg_keypress('+');

		/* Send direction */
		borg_keypress(I2D(dir));
	}

	/* Normally move */
	else {
		/* Send direction */
		borg_keypress(I2D(dir));
	}

	/* We did something */
	return (TRUE);
}

cptr goal_descriptions[GOAL_LAST + 1] = {
	 "Danger Robinson, 0", "Monsters, 1",			"Objects, 2",
	 "Stores, 3",			  "Exploring, 4",			"Searching, 5",
	 "Leaving, 6",			  "Fleeing, 7",			"Town, 8",
	 "Vault, 9",			  "Recover, 10",			"Digging, 11",
	 "Get to safety, 12",  "Danger Robinson, 13"};

/*
 * Attempt to take an optimal step towards the current goal location
 *
 * Note that the "borg_update()" routine notices new monsters and objects,
 * and movement of monsters and objects, and cancels any flow in progress.
 *
 * Note that the "borg_update()" routine notices when a grid which was
 * not thought to block motion is discovered to in fact be a grid which
 * blocks motion, and removes that grid from any flow in progress.
 *
 * When given multiple alternative steps, this function attempts to choose
 * the "safest" path, by penalizing grids containing embedded gold, monsters,
 * rubble, doors, traps, store doors, and even floors.  This allows the Borg
 * to "step around" dangerous grids, even if this means extending the path by
 * a step or two, and encourages him to prefer grids such as objects and stairs
 * which are not only interesting but which are known not to be invisible traps.
 *
 * XXX XXX XXX XXX This function needs some work.  It should attempt to
 * analyze the "local region" around the player and determine the optimal
 * choice of locations based on some useful computations.
 *
 * If it works, return TRUE, otherwise, cancel the goal and return FALSE.
 */
bool borg_flow_old(int why) {
	int x, y;
	/*int x1, x2, y1, y2;*/
	/*int dx = 0;*/
	bool use_dim = FALSE;
	int false_y = c_y;
	int false_x = c_x;
	int b_y = c_y;
	int b_x = c_x;
	int dim_loop;
	/*int b_dim = 0;*/
	int walls = 0;

	bool bad_spot;

	borg_grid *ag;

	/* Continue */
	if (goal == why) {
		int b_n = 0;

		int i, b_i = -1;
		int p;

		int c, b_c = 250;

		/* Could the borg use Dim Door to leap to the grid he wants */
		if ((goal != GOAL_KILL || (goal == GOAL_KILL && borg_skill[BI_VAMPIRE] &&
											borg_skill[BI_ISWEAK])) &&
			 borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 7 / 10 &&
			 borg_skill[BI_ADIMDOOR]) {
			use_dim = FALSE;
			false_y = c_y;
			false_x = c_x;
			b_c = borg_data_flow->data[c_y][c_x];

			/* Don't bother if the original goal grid is close unless it's an
			 * Anti-summon Corridor or im a starving vamp */
			if (b_c <= 10) {
				/* Permissable to DimDoor */
				if (goal == GOAL_DIGGING && b_c >= 2)
					b_c = b_c + 10;
				else if (goal == GOAL_KILL && borg_skill[BI_VAMPIRE] &&
							borg_skill[BI_ISWEAK] && b_c >= 3)
					b_c = b_c + 10;
				else
					b_c = 0;
			} else {
				if (goal == GOAL_KILL && borg_skill[BI_VAMPIRE] &&
					 borg_skill[BI_ISWEAK] && b_c >= 3)
					b_c = b_c + 10;
				else
					b_c = 0;
			}

			/* Step 1.  Can I jump right to the goal grid? */
			if (b_c > 10) {
				/* Access the grid */
				y = borg_goal_y;
				x = borg_goal_x;
				ag = &borg_grids[y][x];

				/* Assume ok */
				use_dim = TRUE;

				/* Must be on my panel */
				if (!panel_fully_contains(y, x))
					use_dim = FALSE;
				if (!in_bounds(y, x))
					use_dim = FALSE;
				if (distance(c_y, c_x, y, c_x) >= 17)
					use_dim = FALSE;

				/* Avoid the vault grids */
				if (cave[y][x].info & CAVE_ICKY)
					use_dim = FALSE;

				/* Avoid wall grids */
				if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM_SOLID)
					use_dim = FALSE;

				/* Avoid minerals */
				if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
					use_dim = FALSE;

				/* Avoid Water if dangerous */
				if (ag->feat == FEAT_WATER)
					use_dim = FALSE;

				/* Unused feats
				// Avoid Mountains, dark pits, lava, and some water
				if (ag->feat == FEAT_MOUNTAIN) use_dim = FALSE;
				if (ag->feat == FEAT_DARK_PIT) use_dim = FALSE;
				if (ag->feat == FEAT_DEEP_LAVA) use_dim = FALSE;
				if (ag->feat == FEAT_SHAL_LAVA) use_dim = FALSE;
				*/

				/* Avoid Monsters */
				if (ag->kill)
					use_dim = FALSE;

				/* Avoid Closed Doors */
				if (ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL)
					use_dim = FALSE;

				/* Avoid Rubble */
				if (ag->feat == FEAT_RUBBLE)
					use_dim = FALSE;

				/* Track the bad landing zones.
				 * I may have tried to jump onto this grid before and failed.
				 * Maybe there is an object, feature, or unknown monster on it.
				 */
				for (i = 0; i < track_land_num; i++) {
					if (y == track_land_y[i] && x == track_land_x[i] &&
						 borg_t - track_land_when[i] < 50)
						use_dim = FALSE;
				}

				/*
				 * Some times the borg will want to jump onto a take, but the take
				 * will have a monster sitting on it
				 * There are some rooms with that configuration.
				 * Borg will need to check to make sure the setup is right for that
				 * type of take
				 */
				if (ag->take) {
					for (i = 0; i < 8; i++) {
						/* Access the grid */
						ag = &borg_grids[y + ddy_ddd[i]][x + ddx_ddd[i]];

						/* Count walls and closed doors */
						if ((ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM_SOLID) ||
							 (ag->feat >= FEAT_DOOR_HEAD &&
							  ag->feat <= FEAT_DOOR_TAIL))
							walls++;
					}

					/* 8 walls means the item is surrounded */
					if (walls == 8)
						use_dim = FALSE;
				}

				/* define my landing grids for the activation later */
				if (use_dim == TRUE) {
					b_y = borg_goal_y;
					b_x = borg_goal_x;
				}
			}

			/* Step 2. Loop the grid points along the path */
			if (use_dim == FALSE) {
				for (dim_loop = 0; dim_loop < borg_skill[BI_CLEVEL] - 2 && b_c >= 1;
					  dim_loop++) {
					/* Look around */
					for (i = 0; i < 8; i++) {
						/* Grid in that direction */
						x = false_x + ddx_ddd[i];
						y = false_y + ddy_ddd[i];

						/* Access the grid */
						ag = &borg_grids[y][x];

						/* Flow cost at that grid */
						c = borg_data_flow->data[y][x];

						/* must be able to pass */
						if (c == 255)
							continue;

						/* Must be on my panel */
						if (!panel_fully_contains(y, x))
							continue;
						if (!in_bounds(y, x))
							continue;
						if (distance(c_y, c_x, y, c_x) >= 17)
							continue;

						/* Avoid the vault grids */
						if (cave[y][x].info & CAVE_ICKY)
							continue;

						/* Avoid wall grids */
						if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM_SOLID)
							continue;

						/* Avoid minerals */
						if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
							continue;

						/* Avoid Lava
						if (ag->feat == FEAT_DEEP_LAVA)
							continue;
						if (ag->feat == FEAT_SHAL_LAVA)
							continue;
						*/

						/* Avoid Water if dangerous */
						if (ag->feat == FEAT_WATER)
							continue;

						/* Avoid Some features which we dont implement just yet
						if (ag->feat == FEAT_MOUNTAIN)
							continue;
						if (ag->feat == FEAT_DARK_PIT)
							continue;
						*/

						/* Avoid Monsters */
						if (ag->kill)
							continue;

						/* Avoid Closed Doors */
						if (ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL)
							continue;

						/* Avoid Rubble */
						if (ag->feat == FEAT_RUBBLE)
							continue;

						/* Track the bad landing zones.
						 * I may have tried to jump onto this grid before and failed.
						 * Maybe there is an object, feature, or unknown monster on
						 * it.
						 */
						bad_spot = FALSE;
						for (p = 0; p < track_land_num; p++) {
							bad_spot = FALSE;

							if (y == track_land_y[p] && x == track_land_x[p] &&
								 borg_t - track_land_when[p] < 50)
								bad_spot = TRUE;
						}
						if (bad_spot == TRUE)
							continue;

						/* Move closer to the target grid */
						if (c >= b_c)
							continue;

						/* Track new best values */
						/*b_dim = dim_loop;*/
						b_i = i;
						b_c = c;

					} /* looking around */

					/* Looking around found a better grid.  Use it as the basis for
					 * the next check */
					if (b_i >= 0) {
						false_y = false_y + ddy_ddd[b_i];
						false_x = false_x + ddx_ddd[b_i];
						b_y = false_y;
						b_x = false_x;
						b_i = -1;

						/* Minimal distance to jump */
						if (dim_loop >= 10)
							use_dim = TRUE;
					}
				} /* Dim_loop */
			}	 /* step 2 */

			/* Is it still a good idea to use Dim Door */
			if (use_dim == TRUE) {
				/* Use Dim Door, if possible. */
				if (borg_dim_door_to(b_y, b_x)) {
					borg_note(format("# Dim Door: Flow short cut.  Starting: (%d, "
										  "%d) Landing: (%d, %d)",
										  c_y, c_x, b_y, b_x));

					/* Success */
					return (TRUE);
				}
			}
		}

		/* Flow cost of current grid */
		b_c = borg_data_flow->data[c_y][c_x] * 10;

		/* Prevent loops */
		b_c = b_c - 5;

		/* Look around */
		for (i = 0; i < 8; i++) {
			/* Grid in that direction */
			x = c_x + ddx_ddd[i];
			y = c_y + ddy_ddd[i];

			/* Access the grid */
			ag = &borg_grids[y][x];

			/* Flow cost at that grid */
			c = borg_data_flow->data[y][x] * 10;

			/* Avoid "perma-wall" grids */
			if (ag->feat >= FEAT_PERM_EXTRA && ag->feat <= FEAT_PERM_SOLID)
				continue;

			/* Avoid Lava
			if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
				continue;
			if (ag->feat == FEAT_SHAL_LAVA && !borg_skill[BI_IFIRE] &&
				 !borg_skill[BI_FEATH])
				continue;
			*/

			/* Avoid Water if dangerous */
			if (ag->feat == FEAT_WATER) {
				if (borg_skill[BI_ENCUMBERD] && !borg_skill[BI_FEATH])
					continue;
			}

			/* Note that if the borg is surrounded by water or lava and taking
			 * damage,
			 * he can either tport/phase out or attempt to run out, taking a few
			 * hits
			 * in the process.
			 */

			/* Avoid Mountains
			if (ag->feat == FEAT_MOUNTAIN)
				continue;
			if (ag->feat == FEAT_DARK_PIT && !borg_skill[BI_FEATH])
				continue;
			*/

			/* Penalty for choosing to flow through rocks */
			if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_WALL_SOLID &&
				 (goal != GOAL_UNREACH && goal != GOAL_DIGGING))
				c += 3;

			/* Penalty for choosing to flow through rocks */
			if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_WALL_SOLID &&
				 (goal != GOAL_UNREACH && goal == GOAL_DIGGING))
				c += 1;

			/* Never back track */
			if (c > b_c)
				continue;

			/* Notice new best value */
			if (c < b_c)
				b_n = 0;

			/* Apply the randomizer to equivalent values */
			if (borg_skill[BI_CDEPTH] == 0 && (++b_n >= 2) && (rand_int(b_n) != 0))
				continue;
			else if (borg_skill[BI_CDEPTH] >= 1 && ++b_n >= 2)
				continue;

			/* Special case when digging anti-summon corridor */
			if (goal == GOAL_DIGGING && (ddx_ddd[i] == 0 || ddy_ddd[i] == 0)) {
				/* No straight lines */
				if (distance(c_y, c_x, borg_flow_y[0], borg_flow_x[0]) <= 2)
					continue;
			}

			/* Avoid flowing through a monster-- unless on a breeder level or a
			 * pet/friendly */
			if (ag->kill && !goal_ignoring && !borg_kills[ag->kill].ally)
				continue;

			/* Carefull when trying to flow through a pet */
			if (ag->kill && borg_kills[ag->kill].ally &&
				 (borg_skill[BI_ISSTUN] || borg_skill[BI_ISIMAGE] ||
				  borg_skill[BI_ISCONFUSED] || borg_berserk))
				continue;

			/* if attempting to flow to a grid, make sure first step is not too
			 * dangerous */
			if (goal == GOAL_UNREACH && borg_danger(y, x, 1, TRUE) > avoidance / 3)
				continue;

			/* Track it */
			b_i = i;
			b_c = c;
		} /* looking around */

		/* Try it */
		if (b_i >= 0) {
			/* Access the location */
			x = c_x + ddx_ddd[b_i];
			y = c_y + ddy_ddd[b_i];

			/* Attempt motion */
			if (borg_play_step(y, x))
				return (TRUE);
		}

		/* Mark a timestamp to wait on a anti-summon spot for a few turns */
		if (goal == GOAL_DIGGING && c_y == borg_flow_y[0] &&
			 c_x == borg_flow_x[0]) {
			borg_t_position = borg_t;
			/* time_this_panel = 0; */
		}

		/* Cancel goal */
		goal = 0;
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Prepare to use Dim Door to get closer to a stair.  This is usually an
 * emergency
 */
bool borg_flow_stair_less_dim(/*int why*/) {
	int i;
	int p, b_p = -1;
	int b_s = -1;
	/*int x1, y1, x2, y2;*/
	int b_y, b_x;
	int dis;

	/* None to flow to */
	if (!track_less_num)
		return (FALSE);

	/* Do we have Dim Door spell ? */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* clear the possible searching flag */
	borg_needs_searching = FALSE;

	/* Enqueue useful grids */
	for (i = 0; i < track_less_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_less_y[i]][track_less_x[i]].kill &&
			 borg_skill[BI_DEPTH] != 0) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Close enough? */
		dis = distance(c_y, c_x, track_less_y[i], track_less_x[i]);

		/* Adjacent to a stair */
		if (dis == 1 || dis == 0)
			return (FALSE);
		if (dis > borg_skill[BI_CLEVEL] + 2)
			continue;
		if (dis < 3)
			continue;

		/* Dangerous? */
		p = borg_danger(track_less_y[i], track_less_x[i], 1, TRUE);

		/* Compare and track the least dangerous */
		if (p < b_p || b_p == -1) {
			b_p = p;
			b_s = i;
		}
	}

	/* No good landing stair */
	if (b_s == -1)
		return (FALSE);

	/* do it */
	if (borg_dim_door_to(track_less_y[b_s], track_less_x[b_s])) {
		/* Assign the landing zone */
		b_y = track_less_y[b_s];
		b_x = track_less_x[b_s];

		borg_note(format(
			 "# Dim Door: Flee to stair. Starting: (%d, %d) Landing: (%d, %d)",
			 c_y, c_x, b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* Something went wrong */
	borg_note("# Failed to Dim Door when I wanted to.");
	return (FALSE);
}

/*
 * Prepare to use Dim Door to get closer to a stair.  This is usually an
 * emergency
 */
bool borg_flow_stair_more_dim(/*int why*/) {
	int i;
	int p, b_p = -1;
	int b_s = -1;
	/*int x1, y1, x2, y2;*/
	int b_y, b_x;
	int dis;

	/* None to flow to */
	if (!track_more_num)
		return (FALSE);

	/* Do we have Dim Door spell ? */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* clear the possible searching flag */
	borg_needs_searching = FALSE;

	/* Enqueue useful grids */
	for (i = 0; i < track_more_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_more_y[i]][track_more_x[i]].kill &&
			 borg_skill[BI_DEPTH] != 0) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Close enough? */
		dis = distance(c_y, c_x, track_more_y[i], track_more_x[i]);
		/* Adjacent to a stair */
		if (dis == 1 || dis == 0)
			return (FALSE);
		if (dis > borg_skill[BI_CLEVEL] + 2)
			continue;
		if (dis < 3)
			continue;

		/* Dangerous? */
		p = borg_danger(track_more_y[i], track_more_x[i], 1, TRUE);

		/* Compare and track the least dangerous */
		if (p < b_p || b_p == -1) {
			b_p = p;
			b_s = i;
		}
	}

	/* No good landing stair */
	if (b_s == -1)
		return (FALSE);

	/* do it */
	if (borg_dim_door_to(track_more_y[b_s], track_more_x[b_s])) {
		/* Assign the landing zone */
		b_y = track_more_y[b_s];
		b_x = track_more_x[b_s];

		borg_note(format(
			 "# Dim Door: Flee to stair. Starting: (%d, %d) Landing: (%d, %d)",
			 c_y, c_x, b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* Something went wrong */
	borg_note("# Failed to Dim Door when I wanted to.");
	return (FALSE);
}

/*
 * Prepare to use Dim Door to get closer to a stair.  This is usually an
 * emergency
 */
bool borg_flow_stair_both_dim(/*int why*/) {
	int i;
	int p, b_p = -1;
	int b_s = -1;
	/*int x1, y1, x2, y2;*/
	int b_y, b_x;
	int dis;
	/*bool twitchy = FALSE;*/

	/* None to flow to */
	if (!track_less_num && !track_more_num)
		return (FALSE);

	/* Do we have Dim Door spell ? */
	if (!borg_skill[BI_ADIMDOOR])
		return (FALSE);

	/* dont go down if hungry or low on food, unless fleeing a scary town */
	if ((!goal_fleeing && !borg_skill[BI_CDEPTH]) &&
		 (borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISWEAK] ||
		  (borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		  borg_skill[BI_FOOD] < 2))
		return (FALSE);

	/* clear the possible searching flag */
	borg_needs_searching = FALSE;

	/* Is the borg moving under boosted bravery?
	if (avoidance > borg_skill[BI_CURHP]) twitchy = TRUE;
	*/

	/* Enqueue useful grids */
	for (i = 0; i < track_less_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_less_y[i]][track_less_x[i]].kill &&
			 borg_skill[BI_DEPTH] != 0) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Close enough? */
		dis = distance(c_y, c_x, track_less_y[i], track_less_x[i]);
		/* Adjacent to a stair */
		if (dis == 1 || dis == 0)
			return (FALSE);
		if (dis > borg_skill[BI_CLEVEL] + 2)
			continue;
		if (dis < 3)
			continue;

		/* Dangerous? */
		p = borg_danger(track_less_y[i], track_less_x[i], 1, TRUE);

		/* Compare and track the least dangerous */
		if (p < b_p || b_p == -1) {
			b_p = p;
			b_s = i;
		}
	}

	/* Enqueue useful grids */
	for (i = 0; i < track_more_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_more_y[i]][track_more_x[i]].kill) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Close enough? */
		dis = distance(c_y, c_x, track_more_y[i], track_more_x[i]);

		/* Adjacent to a stair */
		if (dis == 1 || dis == 0)
			return (FALSE);

		if (dis > borg_skill[BI_CLEVEL] + 2)
			continue;
		if (dis < 3)
			continue;

		/* Dangerous? */
		p = borg_danger(track_more_y[i], track_more_x[i], 1, TRUE);

		/* Compare and track the least dangerous */
		if (p < b_p || b_p == -1) {
			b_p = p;
			b_s = i + 10;
		}
	}

	/* No good landing stair */
	if (b_s == -1)
		return (FALSE);

	/* do it */
	/* Assign the landing zone */
	if (b_s < 10) {
		b_y = track_less_y[b_s];
		b_x = track_less_x[b_s];
	} else {
		b_y = track_more_y[b_s - 10];
		b_x = track_more_x[b_s - 10];
	}

	if (borg_dim_door_to(b_y, b_x)) {
		borg_note(format(
			 "# Dim Door: Flee to stair. Starting: (%d, %d) Landing: (%d, %d)",
			 c_y, c_x, b_y, b_x));

		/* Success */
		return (TRUE);
	}

	/* Something went wrong */
	borg_note("# Failed to Dim Door when I wanted to.");
	return (FALSE);
}

/*
 * Prepare to flee the level via stairs
 */
bool borg_flow_stair_both(int why, bool sneak, bool prep_check) {
	int i;

	/* None to flow to */
	if (track_less_num < 1 && track_more_num < 1)
		return (FALSE);

	/* dont go down if hungry or low on food, unless fleeing a scary town */
	if ((!goal_fleeing && !borg_skill[BI_CDEPTH] && !prep_check) &&
		 (borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISWEAK] ||
		  (borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		  borg_skill[BI_FOOD] < 2))
		return (FALSE);

	/* clear the possible searching flag */
	borg_needs_searching = FALSE;

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue useful grids */
	for (i = 0; i < track_less_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_less_y[i]][track_less_x[i]].kill &&
			 borg_skill[BI_CLEVEL] != 0) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Enqueue the grid */
		borg_flow_enqueue_grid(track_less_y[i], track_less_x[i]);
	}

	/* Enqueue useful grids */
	for (i = 0; i < track_more_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_less_y[i]][track_less_x[i]].kill) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Enqueue the grid */
		borg_flow_enqueue_grid(track_more_y[i], track_more_x[i]);
	}

	/* Spread the flow.
	* 3rd element is the AVOID boolean which will avoid unknown grid.
	* If the borg is fleeing, he should stick to a known pathway.
	*/

	borg_flow_spread(250, TRUE, (borg_depth & DEPTH_SCARY), FALSE, -1, sneak);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("stairs", why, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(why))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to flow towards "up" stairs
 */
bool borg_flow_stair_less(int why, bool sneak) {
	int i;

	/* None to flow to */
	if (track_less_num < 1)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* clear the possible searching flag */
	borg_needs_searching = FALSE;

	/* Enqueue useful grids */
	for (i = 0; i < track_less_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_less_y[i]][track_less_x[i]].kill &&
			 borg_skill[BI_CLEVEL] != 0) {
			borg_note("# Monster camped on my stairs!");
			continue;
		}

		/* Enqueue the grid */
		borg_flow_enqueue_grid(track_less_y[i], track_less_x[i]);
	}

	if (borg_skill[BI_CLEVEL] > 35 || borg_skill[BI_CUR_LITE] == 0) {
		/* Spread the flow */
		borg_flow_spread(250, TRUE, FALSE, FALSE, -1, sneak);
	} else {
		/* Spread the flow, No Optimize, Avoid */
		borg_flow_spread(250, FALSE, (borg_depth & (DEPTH_SCARY | DEPTH_BREEDER)),
							  FALSE, -1, sneak);
	}

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("up-stairs", why, FALSE))
		return (FALSE);

	/* if Standing on a step, go ahead and take it now */
	if (borg_grids[c_y][c_x].feat == FEAT_LESS) {
		borg_note("# Already on a stair.  Taking it.");
		if (dungeon_stair)
			borg_on_dnstairs = TRUE;
		borg_keypress('<');

		return (TRUE);
	}

	/* Take one step */
	if (!borg_flow_old(why))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to flow towards "down" stairs
 */
bool borg_flow_stair_more(int why, bool sneak, bool prep_check) {
	int i;
	int leash = 250;

	/* None to flow to */
	if (track_more_num < 0)
		return (FALSE);

	/* if not fleeing do not go down unless safe */
	if (prep_check && !borg_lunal_mode && !borg_munchkin_mode &&
		 (cptr)NULL != borg_prepared[borg_skill[BI_CDEPTH] + 1])
		return (FALSE);

	/* dont go down if hungry or low on food, unless fleeing a scary town */
	if (prep_check && borg_skill[BI_CDEPTH] && !borg_munchkin_mode &&
		 (borg_skill[BI_ISWEAK] ||
		  (borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		  borg_skill[BI_FOOD] < 2))
		return (FALSE);

	/* If I need to sell crap, then don't go down */
	if (prep_check && borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 &&
		 borg_gold < 25000 && borg_count_sell() >= 13 && !borg_munchkin_mode)
		return (FALSE);

	/* No diving if no light */
	if (prep_check && borg_skill[BI_CUR_LITE] == 0 && borg_gold >= 3)
		return (FALSE);

	/* don't head for the stairs if you are recalling,  */
	/* even if you are fleeing. */
	if (goal_recalling)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Make sure we can move in town */
	if (borg_skill[BI_CDEPTH] >= 1 && avoidance <= borg_skill[BI_MAXHP])
		leash = borg_skill[BI_CLEVEL] * 3 + 9;

	/* Enqueue useful grids */
	for (i = 0; i < track_more_num; i++) {
		/* Must skip stairs on which a monster sits */
		if (borg_grids[track_more_y[i]][track_more_x[i]].kill &&
			 borg_skill[BI_CLEVEL] != 0) {
			borg_note(format("# # Monster camped on my stairs at (%d,%d)!",
								  track_more_y[i], track_more_x[i]));
			continue;
		}

		/* Enqueue the grid */
		borg_flow_enqueue_grid(track_more_y[i], track_more_x[i]);
	}

	if (borg_skill[BI_CLEVEL] > 35 || borg_skill[BI_CUR_LITE] == 0) {
		/* Spread the flow */
		borg_flow_spread(250, TRUE, FALSE, FALSE, -1, sneak);
	} else {
		/* Spread the flow, leash, No Optimize, Avoid */
		borg_flow_spread(leash, FALSE, (borg_depth & DEPTH_SCARY), FALSE, -1,
							  sneak);
	}

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("down-stairs", why, FALSE))
		return (FALSE);

	/* if Standing on a step, go ahead and take it now */
	if (borg_grids[c_y][c_x].feat == FEAT_MORE) {
		borg_note("# Already on a stair.  Taking it.");
		if (dungeon_stair)
			borg_on_upstairs = TRUE;
		borg_keypress('>');

		return (TRUE);
	}

	/* Take one step */
	if (!borg_flow_old(why))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to flow towards a location and create a
 * special glyph of warding pattern.
 *
 * The borg will look for a room that is at least 7x7.
 * ##########
 * #3.......#
 * #2.xxxxx.#
 * #1.xxxxx.#
 * #0.xx@xx.#
 * #1.xxxxx.#
 * #2.xxxxx.#
 * #3.......#
 * # 3210123#
 * ##########
 * and when he locates one, he will attempt to:
 * 1. flow to a central location and
 * 2. begin planting Runes in a pattern. When complete,
 * 3. move to the center of it.
 */
/*
 * ghijk  The borg will use the following ddx and ddy to search
 * d827a  for a suitable grid in an open room.
 * e4@3b
 * f615c
 * lmnop  24 grids
 *
 */
bool borg_flow_glyph(/*int why*/) {
	int i;
	int cost;

	int x, y;
	int v = 0;

	int b_x = c_x;
	int b_y = c_y;
	int b_v = -1;
	int goal_glyph = 0;
	int glyph = 0;
	int glyphs_needed = 8;
	bool center_void = FALSE;

	borg_grid *ag;

	/* DimDoor should leave the center blank in order to DimDoor into the center
	 */
	if (borg_skill[BI_ADIMDOOR])
		center_void = TRUE;

	/* Use a 5x5 or a 3x3 sea of runes */
	if (borg_skill[BI_CHEAPGLYPH])
		glyphs_needed = 24;

	/* Not made yet or too far away.  must make new one. */
	if ((glyph_y_center == 0 && glyph_x_center == 0) ||
		 distance(c_y, c_x, glyph_y_center, glyph_x_center) >= 50) {
		borg_needs_new_sea = TRUE;
	}

	/* We leave the center empty if we have DimDoor */
	if (center_void && borg_needs_new_sea && glyph_x != 0 && glyph_y != 0) {
		/* Cancel */
		glyph_x = 0;
		glyph_y = 0;

		/* Store the center of the glyphs */
		glyph_y_center = c_y;
		glyph_x_center = c_x;
		borg_needs_new_sea = FALSE;

		/* We do not want to put a glyph in the center of our sea of we have
		 * DimDoor */
		if (c_y == glyph_y_center && c_x == glyph_x_center) {
			/* Take note */
			borg_note(format("# Leaving sea center void at (%d,%d)", c_x, c_y));
		}
	}
	/* We have arrived. Makes runes */
	else if ((glyph_x == c_x) && (glyph_y == c_y)) {
		/* Cancel */
		glyph_x = 0;
		glyph_y = 0;

		/* Store the center of the glyphs */
		if (borg_needs_new_sea) {
			glyph_y_center = c_y;
			glyph_x_center = c_x;
		}

		borg_needs_new_sea = FALSE;

		/* We do not want to put a glyph in the center of our sea of we have
		 * DimDoor */
		if (center_void && c_y == glyph_y_center && c_x == glyph_x_center) {
			/* Take note */
			borg_note(format("# Leaving sea center void at (%d,%d)", c_x, c_y));

			/* No action */
			borg_keypress(',');
			return (TRUE);
		}

		/* Take note */
		borg_note(format("# Glyph Creating at (%d,%d)", c_x, c_y));

		/* Create the Glyph */
		if (borg_spell(REALM_LIFE, 1, 7) || borg_spell(REALM_LIFE, 2, 7) ||
			 borg_spell(REALM_SORCERY, 3, 2) ||
			 borg_read_scroll(SV_SCROLL_RUNE_OF_PROTECTION) ||
			 borg_racial(RACE_HALF_OGRE, 1)) {
			/* Check for an existing glyph */
			for (i = 0; i < track_glyph_num; i++) {
				/* Stop if we already new about this glyph */
				if ((track_glyph_x[i] == c_x) && (track_glyph_y[i] == c_y))
					continue;
			}

			/* Track the newly discovered glyph */
			if (track_glyph_num < track_glyph_size) {
				borg_note("# Noting the creation of a glyph.");
				track_glyph_x[track_glyph_num] = c_x;
				track_glyph_y[track_glyph_num] = c_y;
				track_glyph_num++;
			}

			/* Success */
			return (TRUE);
		}

		/* If not creating glphs, but excavating an open area for use against
		 * borers, this is ok. */
		return (FALSE);
	}

	/* Reverse flow */
	borg_flow_reverse();

	/* Scan the entire map */
	for (y = borg_wall_buffer; y <= AUTO_MAX_Y - borg_wall_buffer; y++) {
		for (x = borg_wall_buffer * 3; x <= AUTO_MAX_X - borg_wall_buffer * 3;
			  x++) {
			borg_grid *ag_ptr[24];

			int floor = 0;
			int glyph = 0;
			bool grid_out_of_bounds = FALSE;

			/* Acquire the grid */
			ag = &borg_grids[y][x];

			/* Skip every non floor/glyph */
			if (ag->feat != FEAT_FLOOR && ag->feat != FEAT_GLYPH)
				continue;

			/* Acquire the cost */
			cost = borg_data_cost->data[y][x];

			/* Skip grids that are really far away.  He probably
			 * won't be able to safely get there
			 */
			if (cost >= 75)
				continue;

			/* Extract adjacent locations to each considered grid */
			for (i = 0; i < 24; i++) {
				/* Extract the location */
				int xx = x + borg_ddx_ddd[i];
				int yy = y + borg_ddy_ddd[i];

				/* Get the grid contents */
				ag_ptr[i] = &borg_grids[yy][xx];
			}

			/* Center Grid */
			if (borg_needs_new_sea) {
				goal_glyph = glyphs_needed;

				/* Count Adjacent Flooors  and ruleout out-of-bounds grids. */
				for (i = 0; i < glyphs_needed; i++) {
					int xx = x + borg_ddx_ddd[i];
					int yy = y + borg_ddy_ddd[i];

					ag = ag_ptr[i];
					if (ag->feat == FEAT_FLOOR || ag->feat == FEAT_GLYPH)
						floor++;

					/* Glyphs */
					if (ag->feat == FEAT_GLYPH) {
						glyph++;
					}

					/* Although this grid might be within our buffer, let's make sure
					 * each of the out-lying ones will be */
					if (yy < borg_wall_buffer - 3 ||
						 yy > AUTO_MAX_Y - borg_wall_buffer + 3 ||
						 xx < borg_wall_buffer - 3 ||
						 xx > AUTO_MAX_X - borg_wall_buffer + 3)
						grid_out_of_bounds = TRUE;
				}

				/* Not a good location if not the center of the sea */
				if (floor != glyphs_needed || grid_out_of_bounds) {
					continue;
				}

				/* Tweak -- Reward certain floors, punish distance */
				v = 100 + (glyph * 500) - (cost * 1);
				if (borg_grids[y][x].feat == FEAT_FLOOR)
					v += 3000;

				/* If this grid is surrounded by glyphs, select it */
				if (glyph == goal_glyph) {
					v += 5000;
					glyph_x_center = x;
					glyph_y_center = y;
				}

				/* If this grid is already glyphed but not
				 * surrounded by glyphs, then choose another.
				 */
				if (glyph != goal_glyph && borg_grids[y][x].feat == FEAT_GLYPH)
					v = -1;

				/* The grid is not searchable */
				if (v <= 0)
					continue;

				/* Track "best" grid */
				if ((b_v >= 0) && (v < b_v))
					continue;

				/* Save the data */
				b_v = v;
				b_x = x;
				b_y = y;
			}
			/* old center, making outlying glyphs, */
			else {
				/* Count Adjacent Flooors */
				for (i = 0; i < glyphs_needed; i++) {
					/* Leave if this grid is not in good array */
					if (glyph_x_center + borg_ddx_ddd[i] != x)
						continue;
					if (glyph_y_center + borg_ddy_ddd[i] != y)
						continue;

					/* Already got a glyph on it */
					if (borg_grids[y][x].feat == FEAT_GLYPH)
						continue;

					/* Tweak -- Reward certain floors, punish distance */
					v = 500 + (glyph * 500) - (cost * 1);

					/* The grid is not searchable */
					if (v <= 0)
						continue;

					/* Track "best" grid */
					if ((b_v >= 0) && (v < b_v))
						continue;

					/* Save the data */
					b_v = v;
					b_x = x;
					b_y = y;
				}
			}
		}
	}

	/* Attempt to move into the center of the sea */
	if (glyph_y_center != 0 && glyph_x_center != 0) {

		for (i = 0; i < glyphs_needed; i++) {
			/* Extract the location */
			int xx = glyph_x_center + borg_ddx_ddd[i];
			int yy = glyph_y_center + borg_ddy_ddd[i];

			borg_grid *ag_ptr[24];

			/* Get the grid contents */
			ag_ptr[i] = &borg_grids[yy][xx];
			ag = ag_ptr[i];

			/* If it is not a glyph, count it. */
			if (ag->feat == FEAT_GLYPH)
				glyph++;

			/* Save the data */
			if (glyph == glyphs_needed) {
				b_v = 5000;
				b_x = glyph_x_center;
				b_y = glyph_y_center;
			}
		}
	}

	/* Clear the flow codes */
	borg_flow_clear();

	/* Hack -- Nothing found */
	if (b_v < 0)
		return (FALSE);

	/* Access grid */
	ag = &borg_grids[b_y][b_x];

	/* Memorize */
	glyph_x = b_x;
	glyph_y = b_y;

	/* Enqueue the grid */
	borg_flow_enqueue_grid(b_y, b_x);

	/* attempt to DimDoor over there */
	if (distance(c_y, c_x, b_y, b_x) > 5 && borg_dim_door_to(b_y, b_x)) {
		goal = GOAL_MISC;
		return (TRUE);
	}

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Glyph", GOAL_MISC, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_MISC))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to flow towards light
 */
bool borg_flow_light(int why) {
	int y, x, i;

	/* reset counters */
	borg_glow_n = 0;
	i = 0;

	/* build the glow array */
	/* Scan map */
	for (y = w_y; y < w_y + SCREEN_HGT; y++) {
		for (x = w_x; x < w_x + SCREEN_WID; x++) {
			borg_grid *ag = &borg_grids[y][x];

			/* Not a perma-lit, and not our spot. */
			if (!(ag->info & BORG_GLOW))
				continue;

			/* keep count */
			borg_glow_y[borg_glow_n] = y;
			borg_glow_x[borg_glow_n] = x;
			borg_glow_n++;
		}
	}
	/* None to flow to */
	if (!borg_glow_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue useful grids */
	for (i = 0; i < borg_glow_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_glow_y[i], borg_glow_x[i]);
	}

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("a lighted area", why, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(why))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Step 2A -- Consider seeing the restoration guy to fix our drained stats
 */
static bool borg_flow_shop_restoration(void) {

	int cost = 0;

	/* Guild Members get a discount, but the borg doesnt bother calculating it.
	 */
	cost = 500;

	/* No need if not drained. */
	if (borg_skill[BI_ISFIXSTR] + borg_skill[BI_ISFIXINT] +
			  borg_skill[BI_ISFIXWIS] + borg_skill[BI_ISFIXDEX] +
			  borg_skill[BI_ISFIXCON] + borg_skill[BI_ISFIXCHR] ==
		 0)
		return (FALSE);

	/* No need if I can do it with Spells */
	/* TODO MUST implement */

	/* No need if I can do it with Racial abilities */
	/* TODO MUST implement */

	/* I need enough money */
	if (borg_gold < cost)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/*TODO MUST The healer is not in a harcoded place!! */
	/* Restoration-Temple is */
	borg_flow_enqueue_grid(26, 83);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Inner Temple", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider seeing the Inn/restaurant guy to buy some food
 */
bool borg_flow_shop_inn(void) {

	int cost = 2;

	/* No need if not hungry. */
	if (borg_skill[BI_ISHUNGRY] + borg_skill[BI_ISWEAK] == 0)
		return (FALSE);

	/* No need if I have a good supply */
	if (borg_skill[BI_FOOD] >= 3 &&
		 !borg_spell_legal_fail(REALM_DEATH, 1, 3, 30))
		return (FALSE);

	/* No need if I can't eat */
	if (borg_skill[BI_NOEAT])
		return (FALSE);

	/* Need to be in the right town */
	if (borg_skill[BI_CDEPTH] != 0)
		return (FALSE);

	/* I need enough money */
	if (borg_gold < cost)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Selection for correct town */
	/*TODO MUST The inn is not in a harcoded place!! */
	borg_flow_enqueue_grid(43, 92);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Inn", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/* There is no Trump Tower in Hellband
	bool borg_flow_shop_planar(void)
*/

/*
 * Consider seeing the Library for *ID*'ing my gear
 */
static bool borg_flow_shop_starid(void) {
	int i;
	int b_i = -1;

	/* No need if I am carrying some *ID* */
	if (borg_has[SCROLL_STAR_ID])
		return (FALSE);

	/* TODO MUST No need if I can do it with Racial abilities */

	/* TODO MUST No need if  I have the right star sign  */

	/* TODO MUST No need if  can cast it  */

	/* TODO MUST No need if I have the proper artefact  */

	/* I need enough money */
	if (borg_gold < 2200)
		return (FALSE);

	/* Count the number of Non-ID'd items. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Check the list of good things to *ID* */
		if (!borg_starid_item(item))
			continue;

		/* Keep it */
		b_i = i;
	}

	/* Nothing worth *ID* */
	if (b_i == -1)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* TODO MUST,  Library is not hardcoded */
	borg_flow_enqueue_grid(41, 106);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Library", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider seeing the Theives Guidld for ID'ing my gear
 */
static bool borg_flow_shop_thieves(void) {
	int i;
	int count = 0;
	int cost = 600;
	bool rewards = FALSE;

	/* A rogue should stop by to get free money (Cheat the availability
	if (borg_class == CLASS_ROGUE && !p_ptr->rewards[BACT_GOLD])
		rewards = TRUE;
	*/

	/* Guild Members get a discount
		TODO Should, is this true ? Leverage the cost function */
	if (borg_class == CLASS_ROGUE)
		cost = 100;

	/* No need if I can do it with Spells */
	if (borg_skill[BI_AID] > 10 && !rewards)
		return (FALSE);

	/* No need if I can do it with Racial abilities */
	/*TODO SHOULD*/

	/* I need enough money */
	if (borg_gold < cost && !rewards)
		return (FALSE);

	/* Count the number of Non-ID'd items. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip ID'd items */
		if (item->ident)
			continue;

		/* tally the non-id'd stuff */
		count++;
	}

	/* No need if very few need to be ID'd */
	if (count < (borg_class == CLASS_ROGUE ? 1 : 7) && !rewards)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* TODO MUST dont harcode thieves guild location */
	borg_flow_enqueue_grid(43, 122);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Thieves Guild", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider seeing the Sorcery Tower for ID'ing my gear if I am a sorcerer.
 * static bool borg_flow_shop_sorc_id(void) {
 * There is no Sorcery tower in Hellband (yet?)
 */

/*
 * Consider seeing the Recharge guy for my wands/staves
 * Realm Sorcery can ID their gear here affordably.
 */
static bool borg_flow_shop_recharge(void) {
	int i;
	int b_i = -1;
	int cost = 200;

	/* TODO MUST No need if I can do it with Racial abilities */
	/* TODO MUST No need if I have an artifact */

	/* The price should probably be calculated from the potential recharges */
	if (borg_gold < cost)
		return (FALSE);

	/* Look at our items for things that need a recharge. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Only staff and wand */
		if (item->tval != TV_WAND && item->tval != TV_STAFF &&
			 item->tval != TV_ROD)
			continue;

		/* Safety check */
		if (item->iqty == 0)
			continue;

		/* Is it already full? */
		if ((item->tval != TV_ROD &&
			  (item->pval / item->iqty >= k_info[item->kind].pval)) ||
			 (item->tval == TV_ROD && item->pval == item->iqty))
			continue;

		/* Does it have a minimal amout of charges to be acceptable? */
		if (item->tval == TV_WAND && item->sval == SV_WAND_MAGIC_MISSILE &&
			 item->pval / item->iqty > 7)
			continue;
		if (item->tval == TV_WAND && item->pval / item->iqty > 5)
			continue;
		if (item->tval == TV_STAFF && item->sval <= SV_STAFF_HEALING &&
			 item->pval / item->iqty >= 3)
			continue;
		if (item->tval == TV_STAFF && item->sval > SV_STAFF_HEALING &&
			 item->pval > item->iqty)
			continue;

		/* Keep it */
		b_i = i;
	}

	/* Nothing worth Recharging */
	if (b_i == -1)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Should we have a store that can recharge??*/
	/* TODO MUST it would not be in harcoded location */
	borg_flow_enqueue_grid(20, 55);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Tower of Sorcery (Recharge)", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider seeing the Ranger Guild for Enchanting my gear.
 * Hellband does all enchanting in the weapon store
 * static bool borg_flow_shop_bow(void) {
 */

/*
 * Consider seeing the Fighter Hall for Enchanting my gear
 */
static bool borg_flow_shop_fighter(void) {
	int i;
	/*int count = 0;*/
	int max_pimp = borg_skill[BI_CLEVEL] / 5;
	bool enchant = FALSE;
	int cost_weapon = 400;

	/* Guild members get a discount
	TODO MUST
	* Prices are wrong
	* Armour is enchanted in the armory, not here
	*/
	if (borg_class == CLASS_WARRIOR) {
		cost_weapon = 200;
	}

	/* No need if I can do it with Spells */
	/*TODO MUST*/

	/* No need if I can do it with Racial abilities */
	/*TODO MUST*/

	/* I need enough money (assume the more expensive one) */
	if (borg_gold < cost_weapon)
		return (FALSE);

	/* scan the items we wield */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip certain inventory slots */
		if (!item->iqty)
			continue;
		if (!item->ident)
			continue;
		if (i != INVEN_WIELD && i < INVEN_BODY)
			continue;
		/*TODO MUST some rings can take enchanting..*/
		if (item->tval == TV_RING || item->tval == TV_AMULET)
			continue;

		/* Can my melee weapon be pimped by him? */
		/* TODO MUST Make sure that ranged weapons and missiles are good */
		if (i == INVEN_WIELD &&
			 (item->to_h >= max_pimp && item->to_d >= max_pimp))
			continue;

		/* Can my armor stuff be pimped by him?
		if (i >= INVEN_BODY && item->to_a >= max_pimp)
			continue;
		*/

		/* I should have an item to enchant */
		enchant = TRUE;
	}

	/* Check the swap weapon. */
	if (!enchant && weapon_swap > 1) {

		borg_item *item = &borg_items[weapon_swap];

		/* Can my melee weapon be pimped by him? */
		if (item->to_h < max_pimp || item->to_d < max_pimp) {
			/* I should have my swap weapon enchanted */
			enchant = TRUE;
		}
	}

	/* Check the swap armor. */
	if (!enchant && armour_swap > 1) {

		borg_item *item = &borg_items[armour_swap];

		/* Paladin's only pimp their weapons here.  They can do their armor at the
		 * Pally guild cheaper */
		if (item->to_a < max_pimp && borg_items[armour_swap].tval != TV_RING &&
			 borg_items[armour_swap].tval != TV_AMULET) {
			/* I should have my swap weapon enchanted */
			enchant = TRUE;
		}
	}

	/* We have something to enchant */
	if (!enchant)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/*TODO MUST dont harcode the location, man!*/
	borg_flow_enqueue_grid(19, 111);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Fighters Hall", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider seeing the Paladin Guild for Enchanting my armor.
 * There is no paladin guild, yet
 * static bool borg_flow_shop_paladin(void) {
 */

/*
 * Consider seeing the Ranger Guild for Enchanting my Arrows/Bolts
 * There is no range guild, yet
 * static bool borg_flow_shop_missile(void) {
 */

/* Do a quick scan of the mutations and assign a value to each one.
 * Some are negative, some are positive.
 * If grossly negative, try to cure them.
 * TODO SHOULD this should be a table!!
 */
int borg_net_mutations(void) {
	/*int i;*/
	int v = 0;
	if (p_ptr->muta1) {
		if (p_ptr->muta1 & COR1_SPIT_ACID)
			v = v + 2;
		if (p_ptr->muta1 & COR1_BR_FIRE)
			v = v + 2;
		if (p_ptr->muta1 & COR1_HYPN_GAZE)
			v = v + 1;
		if (p_ptr->muta1 & COR1_TELEKINES)
			v = v + 1;
		if (p_ptr->muta1 & COR1_VTELEPORT)
			v = v + 5;
		if (p_ptr->muta1 & COR1_MIND_BLST)
			v = v + 2;
		if (p_ptr->muta1 & COR1_SLIME)
			v = v + 2;
		if (p_ptr->muta1 & COR1_VAMPIRISM)
			v = v + 5;
		if (p_ptr->muta1 & COR1_SMELL_MET)
			v = v + 0;
		if (p_ptr->muta1 & COR1_SMELL_MON)
			v = v + 1;
		if (p_ptr->muta1 & COR1_BLINK)
			v = v + 5;
		if (p_ptr->muta1 & COR1_EAT_ROCK)
			v = v + 3;
		if (p_ptr->muta1 & COR1_SWAP_POS)
			v = v + 4;
		if (p_ptr->muta1 & COR1_SHRIEK)
			v = v + 0;
		if (p_ptr->muta1 & COR1_ILLUMINE)
			v = v + 1;
		if (p_ptr->muta1 & COR1_DET_CURSE)
			v = v + 0;
		if (p_ptr->muta1 & COR1_BERSERK)
			v = v + 1;
		if (p_ptr->muta1 & COR1_POLYMORPH)
			v = v + 0;
		if (p_ptr->muta1 & COR1_MIDAS_TCH)
			v = v + 0;
		if (p_ptr->muta1 & COR1_GROW_MOLD)
			v = v + 0;
		if (p_ptr->muta1 & COR1_RESIST)
			v = v + 3;
		if (p_ptr->muta1 & COR1_EARTHQUAKE)
			v = v + 2;
		if (p_ptr->muta1 & COR1_EAT_MAGIC)
			v = v + 2;
		if (p_ptr->muta1 & COR1_WEIGH_MAG)
			v = v + 0;
		if (p_ptr->muta1 & COR1_STERILITY)
			v = v + 1;
		if (p_ptr->muta1 & COR1_PANIC_HIT)
			v = v + 1;
		if (p_ptr->muta1 & COR1_DAZZLE)
			v = v + 2;
		if (p_ptr->muta1 & COR1_EYE_BEAM)
			v = v + 2;
		if (p_ptr->muta1 & COR1_RECALL)
			v = v + 4;
		if (p_ptr->muta1 & COR1_BANISH)
			v = v + 4;
		if (p_ptr->muta1 & COR1_COLD_TOUCH)
			v = v + 2;
		if (p_ptr->muta1 & COR1_LAUNCHER)
			v = v + 2;
	}

	if (p_ptr->muta2) {
		if (p_ptr->muta2 & COR2_BERS_RAGE)
			v = v + 2;
		if (p_ptr->muta2 & COR2_COWARDICE)
			v = v - 7;
		if (p_ptr->muta2 & COR2_RTELEPORT)
			v = v - 2;
		if (p_ptr->muta2 & COR2_HALLU)
			v = v - 12;
		if (p_ptr->muta2 & COR2_PROD_MANA)
			v = v + 1;
		if (p_ptr->muta2 & COR2_ATT_DEMON)
			v = v + 2;

		if (p_ptr->muta2 & COR2_SPEED_FLUX)
			v = v + 0;
		if (p_ptr->muta2 & COR2_BANISH_ALL)
			v = v + 2;
		if (p_ptr->muta2 & COR2_EAT_LIGHT)
			v = v + 1;

		if (p_ptr->muta2 & COR2_ATT_ANIMAL)
			v = v + 1;
		if (p_ptr->muta2 & COR2_RAW_CHAOS)
			v = v - 1;
		if (p_ptr->muta2 & COR2_NORMALITY)
			v = v + 0;
		if (p_ptr->muta2 & COR2_WRAITH)
			v = v + 2;
		if (p_ptr->muta2 & COR2_POLY_WOUND)
			v = v - 2;
		if (p_ptr->muta2 & COR2_WASTING)
			v = v - 3;
		if (p_ptr->muta2 & COR2_ATT_DRAGON)
			v = v + 1;
		if (p_ptr->muta2 & COR2_WEIRD_MIND)
			v = v + 1;
		if (p_ptr->muta2 & COR2_NAUSEA)
			v = v - 2;
		/*
		 TODO SHOULD this should be part of the table in tables.c
		if (p_ptr->muta2 & COR2_ALCOHOL)				v = v - 2;
		if (p_ptr->muta2 & COR2_FLATULENT)			v = v - 1;
		if (p_ptr->muta2 & COR2_SCOR_TAIL)			v = v + 2;
		if (p_ptr->muta2 & COR2_HORNS)					v = v + 2;
		if (p_ptr->muta2 & COR2_BEAK)						v = v + 2;
		if (p_ptr->muta2 & COR2_TRUNK)					v = v + 1;
		if (p_ptr->muta2 & COR2_TENTACLES)			v = v + 1;
		if (p_ptr->muta2 & COR2_CHAOS_GIFT)			v = v + 1;
		if (p_ptr->muta2 & COR2_WALK_SHAD)			v = v + 1;
		*/
		if (p_ptr->muta2 & COR2_WARNING)
			v = v + 1;
		if (p_ptr->muta2 & COR2_INVULN)
			v = v + 3;
		if (p_ptr->muta2 & COR2_SP_TO_HP)
			v = v + 1;
		if (p_ptr->muta2 & COR2_HP_TO_SP)
			v = v + 1;
		if (p_ptr->muta2 & COR2_DISARM)
			v = v - 3;
	}

	if (p_ptr->muta3) {
		if (p_ptr->muta3 & COR3_HYPER_STR)
			v = v + 4;
		if (p_ptr->muta3 & COR3_PUNY) {
			v = v - 4;
			/*TODO SHOULD this should be a function or a table*/
			if (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE ||
				 borg_class == CLASS_PALADIN || borg_class == CLASS_MONK)
				v = v - 4;
		}
		if (p_ptr->muta3 & COR3_HYPER_INT)
			v = v + 4;
		if (p_ptr->muta3 & COR3_IDIOTIC) {
			v = v - 4;
			if (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST ||
				 borg_class == CLASS_ORPHIC || borg_class == CLASS_HIGH_MAGE)
				v = v - 4;
		}
		if (p_ptr->muta3 & COR3_RESILIENT)
			v = v + 3;
		if (p_ptr->muta3 & COR3_XTRA_FAT)
			v = v - 3;
		if (p_ptr->muta3 & COR3_ALBINO)
			v = v + 3;
		if (p_ptr->muta3 & COR3_FLESH_ROT)
			v = v - 3;
		if (p_ptr->muta3 & COR3_SILLY_VOI)
			v = v - 1;
		if (p_ptr->muta3 & COR3_FORKED_TONGUE)
			v = v - 0;
		if (p_ptr->muta3 & COR3_ILL_NORM)
			v = v + 0;
		if (p_ptr->muta3 & COR3_GLOW_EYES)
			v = v + 1;
		if (p_ptr->muta3 & COR3_MAGIC_RES)
			v = v + 2;
		if (p_ptr->muta3 & COR3_STENCH)
			v = v - 1;
		if (p_ptr->muta3 & COR3_INFRAVIS)
			v = v + 1;
		if (p_ptr->muta3 & COR3_GOAT_LEGS)
			v = v + 3;
		if (p_ptr->muta3 & COR3_SHORT_LEG)
			v = v - 3;
		if (p_ptr->muta3 & COR3_ELEC_TOUC)
			v = v + 2;
		if (p_ptr->muta3 & COR3_FIRE_BODY)
			v = v + 2;
		if (p_ptr->muta3 & COR3_WART_SKIN)
			v = v + 1;
		if (p_ptr->muta3 & COR3_SCALES)
			v = v + 1;
		if (p_ptr->muta3 & COR3_IRON_SKIN)
			v = v + 1;
		if (p_ptr->muta3 & COR3_WINGS)
			v = v + 3;
		if (p_ptr->muta3 & COR3_FEARLESS)
			v = v + 2;
		if (p_ptr->muta3 & COR3_REGEN)
			v = v + 3;
		if (p_ptr->muta3 & COR3_ESP)
			v = v + 5;
		if (p_ptr->muta3 & COR3_LIMBER)
			v = v + 3;
		if (p_ptr->muta3 & COR3_ARTHRITIS)
			v = v - 2;
		if (p_ptr->muta3 & COR3_RES_TIME)
			v = v + 2;
		if (p_ptr->muta3 & COR3_VULN_ELEM)
			v = v - 3;
		if (p_ptr->muta3 & COR3_MOTION)
			v = v + 1;
		if (p_ptr->muta3 & COR3_SUS_STATS)
			v = v + 2;
		/*
		if (p_ptr->muta3 & COR3_GOOD_LUCK)	v = v + 2;
		if (p_ptr->muta3 & COR3_BAD_LUCK)		v = v - 2;
		*/
	}

	return (v);
}
/*
 * Consider seeing the Cure Mutation guy to fix some bad mutations.
 */
static bool borg_flow_shop_mutation(void) {
	/*int i;
	int count = 0;*/
	int cost = 5000;
	int mutations = 0;

	/* Discount */
	/* if (borg_race == RACE_BEASTMAN) cost = 1000; */

	/* Do a quick scan of my mutations score the negativity */
	mutations = borg_net_mutations();

	/* No need if I am no too bad off */
	if (mutations >= 0)
		return (FALSE);

	/* I need enough money */
	if (borg_gold < cost)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/*TODO MUST do not harcode temple location*/
	borg_flow_enqueue_grid(7, 153);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Temple", GOAL_TOWN, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TOWN))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Consider flowing to specialty shops.
 */
bool borg_flow_shop_special(bool first) {

	/* Must be in town */
	if (borg_skill[BI_CDEPTH])
		return (FALSE);

	/* First double check the inventory */
	borg_cheat_inven();
	borg_cheat_equip();
	borg_notice(TRUE);

	/* Buy some food */
	if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) &&
		 borg_flow_shop_inn())
		return (TRUE);

	/* Restoration of drained Stats */
	if (first && borg_flow_shop_restoration())
		return (TRUE);

	/* ID'ing the gear for most classes */
	if (first && borg_flow_shop_thieves())
		return (TRUE);

	/* ID'ing the gear for Sorcery Realm
	if (first && (borg_skill[BI_REALM1] == REALM_SORCERY ||
					  borg_skill[BI_REALM2] == REALM_SORCERY) &&
		 borg_flow_shop_sorc_id())
		return (TRUE);
*/

	/* *ID* of the gear */
	if (first && borg_flow_shop_starid())
		return (TRUE);

	/* Mutation Cure guy */
	if (!first && borg_flow_shop_mutation())
		return (TRUE);

	/* Enchant the Bow
	if (!first && borg_flow_shop_bow())
		return (TRUE);
	*/

	/* Enchant the Armor for Paladins.  They get a guild discount.
	if (!first && borg_class == CLASS_PALADIN && borg_flow_shop_paladin())
		return (TRUE);
	*/

	/* Enchant the Armor and weapons */
	if (!first && borg_flow_shop_fighter())
		return (TRUE);

	/* Recharge the wands and staves. */
	if (!first && borg_flow_shop_recharge())
		return (TRUE);

	/* No need */
	return (FALSE);
}

/*
 * Prepare to "flow" towards any non-visited shop
 */
bool borg_flow_shop_visit(void) {
	int i, x, y;

	/* Must be in town */
	if (borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Visit the shops */
	for (i = 0; i < MAX_STORES; i++) {

		/* No need to visit bookstore if not a spell caster */
		if (realms_info[borg_class].book == NULL)
			if (i == BORG_BOOKSTORE || i == BORG_MAGIC2)
				borg_shops[i].when = borg_t;

		/* No need to visit Home if we did not buy it yet*/
		if (i == BORG_HOME && !store[i].bought)
			borg_shops[i].when = borg_t;

		/* No need to visit Mage Guild if we did not buy it yet */
		/*TODO MUST .. unless we have a 4th book ;) */
		if (i == BORG_MAGIC2 && !store[i].bought && borg_gold < 50000)
			borg_shops[i].when = borg_t;

		/* No need to visit Home if we did not buy it yet,and we are too poor*/
		if (i == BORG_ALCHEMY2 && !store[i].bought && borg_gold < 50000)
			borg_shops[i].when = borg_t;

		/* If when is set, then stop */
		if (borg_shops[i].when)
			continue;

		/* if poisoned or bleeding skip non temples */
		if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) &&
			 (i != BORG_TEMPLE && i != BORG_HOME))
			continue;

		/* if starving--skip non food places */
		if (borg_skill[BI_ISWEAK]) {
			/* Some races have special food requirements */
			if (borg_skill[BI_NOEAT]) {
				/* Alchemist for Sat Hung scrolls */
				if (i != BORG_ALCHEMIST)
					continue;
			} else if (i != BORG_GSTORE && i != BORG_HOME)
				continue;
		}

		/* The borg only visits the inn if he really needs to */
		if (i == BORG_INN && (!borg_skill[BI_ISWEAK] || borg_skill[BI_NOEAT])
			continue;

		/* if dark--skip non food places */
		if (borg_skill[BI_CUR_LITE] == 0 && (i != 0) &&
			 borg_skill[BI_CLEVEL] >= 2)
			continue;

		/* Obtain the location */
		x = track_shop_x[i];
		y = track_shop_y[i];

		/* Hack -- Must be known and not under the player */
		if (!x || !y || ((c_x == x) && (c_y == y)))
			continue;

		/* Enqueue the grid */
		borg_flow_enqueue_grid(y, x);
	}

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("all shops", GOAL_MISC, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_MISC))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards a specific shop entry
 */
bool borg_flow_shop_entry(int i) {
	int x, y;

	cptr name = (f_name + f_info[FEAT_SHOP_HEAD + i].name);

	/* Must be in town */
	if (borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Obtain the location */
	x = track_shop_x[i];
	y = track_shop_y[i];

	/* Hack -- Must be known */
	if (!x || !y)
		return (FALSE);

	/* Hack -- re-enter a shop if needed */
	if ((x == c_x) && (y == c_y)) {
		/* Note */
		borg_note("# Re-entering a shop");

		/* Enter the store */
		borg_keypress('5');

		/* Success */
		return (TRUE);
	}

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue the grid */
	borg_flow_enqueue_grid(y, x);

	/* Spread the flow */
	borg_flow_spread(250, TRUE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit(name, GOAL_MISC, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_MISC))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * The borg can take a shot from a distance
 *
 */
static bool borg_has_distance_attack(void) {
	int rad;
	int dam;

	borg_simulate = TRUE;

	/* XXX For now only line up Magic Missle shots */
	rad = 0;
	dam = (3 + ((borg_skill[BI_CLEVEL]) / 4)) * (4 + 1) / 2;
	if (borg_attack_aux_spell_bolt(REALM_CHAOS, 0, 0, rad, dam, GF_MISSILE, TRUE,
											 -1) > 0)
		return TRUE;

	return FALSE;
}

/*
 * Take a couple of steps to line up a shot
 *
 */
bool borg_flow_kill_aim(bool viewable) {
	int o_y, o_x;
	int y = c_y;
	int x = c_x;
	int i;

	/* Efficiency -- Nothing to kill */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Sometimes we loop on this if we back  up to a point where */
	/* the monster is out of site */
	if (time_this_panel > 500)
		return (FALSE);

	/* If you can shoot from where you are, don't bother reaiming */
	if (borg_has_distance_attack())
		return (FALSE);

	/* Consider each adjacent spot */
	for (o_x = -2; o_x <= 2; o_x++) {
		for (o_y = -2; o_y <= 2; o_y++) {
			/* borg_attack would have already checked
				for a shot from where I currently am */
			if (o_x == 0 && o_y == 0)
				continue;

			/* XXX  Mess with where the program thinks the
				player is */
			c_x = x + o_x;
			c_y = y + o_y;

			/* avoid screen edgeds */
			if (c_x > AUTO_MAX_X - 2 || c_x < 2 || c_y > AUTO_MAX_Y - 2 || c_y < 2)
				continue;

			/* Make sure we do not end up next to a monster */
			for (i = 0; i < borg_temp_n; i++) {
				if (distance(c_y, c_x, borg_temp_y[i], borg_temp_x[i]) == 1)
					break;
			}
			if (i != borg_temp_n)
				continue;

			/* Check for a distance attack from here */
			if (borg_has_distance_attack()) {
				/* Clear the flow codes */
				borg_flow_clear();

				/* Enqueue the grid */
				borg_flow_enqueue_grid(c_y, c_x);

				/* restore the saved player position */
				c_x = x;
				c_y = y;

				/* Spread the flow */
				borg_flow_spread(5, TRUE, !viewable, FALSE, -1, FALSE);

				/* Attempt to Commit the flow */
				if (!borg_flow_commit("targetable position", GOAL_KILL, FALSE))
					return (FALSE);

				/* Take one step */
				if (!borg_flow_old(GOAL_KILL))
					return (FALSE);

				return (TRUE);
			}
		}
	}

	/* restore the saved player position */
	c_x = x;
	c_y = y;

	return FALSE;
}

/*
 * Dig an anti-summon corridor. Type I
 *
 *            ############## We want the borg to dig a tunnel which
 *            #............# limits the LOS of summoned monsters.
 *          ###............# It works better in hallways.
 *         ##@#............#
 *         #p##............# The borg will build an array of grids
 * ########## #######+###### near him.  Then look at specific patterns
 * #                  #      to find the good grids to excavate.
 * # ################ #
 *   #              # #
 * ###              # #
 *
 * Look at wall array to see if it is acceptable
 * We want to find this in the array:
 *
 * #####  ..@..  ####.  .####
 * ##.##  ##.##	 ##.#.  .#.##
 * #.#.#  #.#.#  #.#.@  @.#.#
 * ##.##  ##.##  ##.#.  .#.##
 * ..@..  #####  ####.  .####
 *
 * NORTH  SOUTH  WEST   East
 *
 */
bool borg_flow_kill_corridor_1(/*bool viewable*/) {
	int o_y = 0;
	int o_x = 0;
	int m_x = 0;
	int m_y = 0;
	int b_y = 0, b_x = 0;
	int b_distance = 99;
	int x, y;

	int i;
	bool b_n = FALSE;
	bool b_s = FALSE;
	bool b_e = FALSE;
	bool b_w = FALSE;

	int n_array[25] = {1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
							 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1};
	int ny[25] = {-4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -2, -2, -2,
					  -2, -2, -1, -1, -1, -1, -1, 0,  0,  0,  0,  0};
	int nx[25] = {-2, -1, 0,  1,  2, -2, -1, 0,  1,  2, -2, -1, 0,
					  1,  2,  -2, -1, 0, 1,  2,  -2, -1, 0, 1,  2};

	int s_array[25] = {1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0,
							 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1};
	int sy[25] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2,
					  2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4};
	int sx[25] = {-2, -1, 0,  1,  2, -2, -1, 0,  1,  2, -2, -1, 0,
					  1,  2,  -2, -1, 0, 1,  2,  -2, -1, 0, 1,  2};

	int e_array[25] = {1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0,
							 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1};
	int ey[25] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0,
					  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2, 2};
	int ex[25] = {0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2,
					  3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4};

	int w_array[25] = {1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0,
							 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1};
	int wy[25] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0,
					  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2, 2};
	int wx[25] = {-4, -3, -2, -1, 0,  -4, -3, -2, -1, 0,  -4, -3, -2,
					  -1, 0,  -4, -3, -2, -1, 0,  -4, -3, -2, -1, 0};

	int wall_north = 0;
	int wall_south = 0;
	int wall_east = 0;
	int wall_west = 0;
	int q_x;
	int q_y;

	borg_kill *kill;

	borg_digging = FALSE;

	/* Efficiency -- Nothing to kill */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Only do this to summoners when they are close*/
	if (borg_kills_summoner == -1)
		return (FALSE);

	/* Hungry,starving */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Sometimes we loop on this */
	if (time_this_panel > 350)
		return (FALSE);

	/* Do not dig when confused */
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Not when darkened */
	if (borg_skill[BI_CUR_LITE] == 0)
		return (FALSE);

	/* Not if sitting in a sea of runes */
	if (borg_position & (POSITION_SEA | POSITION_SUMM | POSITION_BORE))
		return (FALSE);

	/* Maybe not even a good idea since these guys foil the plan */
	if (borg_depth & DEPTH_BORER)
		return (FALSE);

	/* Not if a monster is adjacent to me */
	for (i = 0; i < 8; i++) {
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		if (!in_bounds(y, x))
			continue;

		if (borg_grids[y][x].kill)
			return (FALSE);
	}

	/* get the summoning monster */
	kill = &borg_kills[borg_kills_summoner];

	/* Summoner must be mobile */
	if (r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE)
		return (FALSE);
	/* Summoner not must be able to pass through walls */
	if (r_info[kill->r_idx].flags2 & RF2_PASS_WALL)
		return (FALSE);
	if (r_info[kill->r_idx].flags2 & RF2_KILL_WALL)
		return (FALSE);

	/* Summoner has to be awake (so he will chase me */
	if (!kill->awake)
		return (FALSE);

	/* Must have Stone to Mud spell or be a good digger */
	if (borg_skill[BI_ASTONE2MUD] <= 0 &&
		 (borg_items[weapon_swap].tval != TV_DIGGING &&
		  borg_items[weapon_swap].tval != TV_DIGGING &&
		  borg_skill[BI_DIG] < BORG_DIG + 20))
		return (FALSE);

	/* Summoner needs to be able to follow me.
	 * So I either need to be able to
	 * 1) have LOS on him or
	 * 2) this panel needs to have had Magic Map or Wizard light cast on it.
	 * If Mapped, then the flow codes needs to be used.
	 */
	if (kill->los) {
		/* Must be able to excavate since digging takes time */
		if (borg_skill[BI_ASTONE2MUD] <= 0)
			return (FALSE);
	} else {
		/* Extract panel */
		q_x = w_x / PANEL_WID;
		q_y = w_y / PANEL_HGT;

		if (borg_detect_wall[q_y + 0][q_x + 0] == TRUE &&
			 borg_detect_wall[q_y + 0][q_x + 1] == TRUE &&
			 borg_detect_wall[q_y + 1][q_x + 0] == TRUE &&
			 borg_detect_wall[q_y + 1][q_x + 1] == TRUE) {
			borg_flow_clear_m();
			borg_digging = TRUE;
			borg_flow_enqueue_grid_m(c_y, c_x);
			borg_flow_spread_m(BORG_MON_FLOW, borg_kills_summoner, kill->r_idx);
			if (!borg_flow_commit_m(kill->y, kill->x))
				return (FALSE);
		} else {
			borg_flow_clear_m();
			borg_digging = TRUE;
			borg_flow_enqueue_grid_m(c_y, c_x);
			borg_flow_spread_m(BORG_MON_FLOW, borg_kills_summoner, kill->r_idx);
			if (!borg_flow_commit_m(kill->y, kill->x))
				return (FALSE);
		}
	}

	/* NORTH -- Consider each area near the borg, looking for a good spot to hide
	 */
	for (o_y = -2; o_y < 1; o_y++) {
		/* Resest Wall count */
		wall_north = 0;

		/* No E-W offset when looking North-South */
		o_x = 0;

		for (i = 0; i < 25; i++) {
			borg_grid *ag;

			/* Check grids near borg */
			m_y = c_y + o_y + ny[i];
			m_x = c_x + o_x + nx[i];

			/* avoid screen edgeds */
			if (!in_bounds(m_y, m_x)) {
				continue;
			}

			/* grid the grid */
			ag = &borg_grids[m_y][m_x];

			/* Certain grids must not be floor types */
			if (n_array[i] == 0 &&
				 ((ag->feat == FEAT_NONE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* This is a good grid */
				wall_north++;
			}
			if (n_array[i] == 1 &&
				 ((ag->feat <= FEAT_MORE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* A good wall would score 25. */
				wall_north++;
			}
			/* But position 11 and 13 cant both be floor grids */
			if (i == 13 && ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE) {
				/* get grid #7 */
				ag = &borg_grids[c_y + o_y + ny[11]][c_x + o_x + nx[11]];

				if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
					/* Alter the count because #11 is a floor grid */
					wall_north = 100;
				}
			}
		}

		/* If I found 25 grids, then that spot will work well */
		if (wall_north == 25) {
			if (distance(c_y, c_x, c_y + o_y + ny[7], c_x + o_x + nx[7]) <
				 b_distance) {
				b_y = o_y;
				b_x = o_x;
				b_n = TRUE;
				b_distance =
					 distance(c_y, c_x, c_y + o_y + ny[7], c_x + o_x + nx[7]);
			}
		}
	}

	/* SOUTH -- Consider each area near the borg, looking for a good spot to hide
	 */
	for (o_y = -1; o_y < 2; o_y++) {
		/* Resest Wall count */
		wall_south = 0;

		for (i = 0; i < 25; i++) {
			borg_grid *ag;

			/* No lateral offset on South check */
			o_x = 0;

			/* Check grids near borg */
			m_y = c_y + o_y + sy[i];
			m_x = c_x + o_x + sx[i];

			/* avoid screen edgeds */
			if (!in_bounds(m_y, m_x))
				continue;

			/* grid the grid */
			ag = &borg_grids[m_y][m_x];

			/* Certain grids must not be floor types */
			if (s_array[i] == 0 &&
				 ((ag->feat == FEAT_NONE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* This is a good grid */
				wall_south++;
			}
			if (s_array[i] == 1 &&
				 ((ag->feat <= FEAT_MORE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* A good wall would score 25. */
				wall_south++;
			}
			/* But position 11 and 13 cant both be floor grids */
			if (i == 13 && ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE) {
				/* get grid # 11 */
				ag = &borg_grids[c_y + o_y + sy[11]][c_x + o_x + sx[11]];

				if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
					/* Alter the count because #11 is a floor grid */
					wall_south = 100;
				}
			}
		}

		/* If I found 25 grids, then that spot will work well */
		if (wall_south == 25) {
			if (distance(c_y, c_x, c_y + o_y + sy[17], c_x + o_x + sx[17]) <
				 b_distance) {
				b_y = o_y;
				b_x = o_x;
				b_s = TRUE;
				b_n = FALSE;
				b_distance =
					 distance(c_y, c_x, c_y + b_y + sy[17], c_x + b_x + sx[17]);
			}
		}
	}

	/* EAST -- Consider each area near the borg, looking for a good spot to hide
	 */
	for (o_x = -1; o_x < 2; o_x++) {
		/* Resest Wall count */
		wall_east = 0;

		/* No N-S offset check when looking E-W */
		o_y = 0;

		for (i = 0; i < 25; i++) {
			borg_grid *ag;

			/* Check grids near borg */
			m_y = c_y + o_y + ey[i];
			m_x = c_x + o_x + ex[i];

			/* avoid screen edgeds */
			if (!in_bounds(m_y, m_x))
				continue;

			/* grid the grid */
			ag = &borg_grids[m_y][m_x];

			/* Certain grids must not be floor types */
			if (e_array[i] == 0 &&
				 ((ag->feat == FEAT_NONE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* This is a good grid */
				wall_east++;
			}
			if (e_array[i] == 1 &&
				 ((ag->feat <= FEAT_MORE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* A good wall would score 25. */
				wall_east++;
			}
			/* But position 17 and 7 cant both be floor grids */
			if (i == 17 && ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE) {
				/* get grid # 7 */
				ag = &borg_grids[c_y + o_y + ey[7]][c_x + o_x + ex[7]];

				if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
					/* Alter the count because #7 is a floor grid */
					wall_east = 100;
				}
			}
		}

		/* If I found 25 grids, then that spot will work well */
		if (wall_east == 25) {
			if (distance(c_y, c_x, c_y + o_y + ey[13], c_x + o_x + ex[13]) <
				 b_distance) {
				b_y = o_y;
				b_x = o_x;
				b_e = TRUE;
				b_s = FALSE;
				b_n = FALSE;
				b_distance =
					 distance(c_y, c_x, c_y + b_y + ey[13], c_x + b_x + ex[13]);
			}
		}
	}

	/* WEST -- Consider each area near the borg, looking for a good spot to hide
	 */
	for (o_x = -2; o_x < 1; o_x++) {
		/* Resest Wall count */
		wall_west = 0;

		/* No N-S offset check when looking E-W */
		o_y = 0;

		for (i = 0; i < 25; i++) {
			borg_grid *ag;

			/* Check grids near borg */
			m_y = c_y + o_y + wy[i];
			m_x = c_x + o_x + wx[i];

			/* avoid screen edgeds */
			if (!in_bounds(m_y, m_x))
				continue;

			/* grid the grid */
			ag = &borg_grids[m_y][m_x];

			/* Certain grids must not be floor types */
			if (w_array[i] == 0 &&
				 ((ag->feat == FEAT_NONE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* This is a good grid */
				wall_west++;
			}
			if (w_array[i] == 1 &&
				 ((ag->feat <= FEAT_MORE) ||
				  (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
				  (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID))) {
				/* A good wall would score 25. */
				wall_west++;
			}
			/* But position 7 and 17 cant both be floor grids */
			if (i == 17 && ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE) {
				/* get grid #7 */
				ag = &borg_grids[c_y + o_y + wy[7]][c_x + o_x + wx[7]];

				if ((ag->feat >= FEAT_FLOOR && ag->feat <= FEAT_MORE)) {
					/* Alter the count because #7 is a floor grid */
					wall_west = 100;
				}
			}
		}

		/* If I found 25 grids, then that spot will work well */
		if (wall_west == 25) {
			if (distance(c_y, c_x, c_y + o_y + wy[11], c_x + o_x + wx[11]) <
				 b_distance) {
				b_y = o_y;
				b_x = o_x;
				b_w = TRUE;
				b_e = FALSE;
				b_s = FALSE;
				b_n = FALSE;
				b_distance =
					 distance(c_y, c_x, c_y + o_y + wy[11], c_x + o_x + wx[11]);
			}
		}
	}

	/* Attempt to enqueu the grids that should be floor grids and have the borg
	 * move onto those grids
	 */
	if (b_n == TRUE) {
		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid where I will hide */
		borg_digging = TRUE;
		borg_flow_enqueue_grid(c_y + b_y + ny[7], c_x + b_x + nx[7]);

		/* Spread the flow */
		borg_flow_spread(5, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("anti-summon corridor north type 1", GOAL_DIGGING,
									 FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_DIGGING))
			return (FALSE);

		return (TRUE);
	}
	if (b_s == TRUE) {
		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid where I will hide */
		borg_digging = TRUE;
		borg_flow_enqueue_grid(c_y + b_y + sy[17], c_x + b_x + sx[17]);

		/* Spread the flow */
		borg_flow_spread(6, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("anti-summon corridor south type 1", GOAL_DIGGING,
									 FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_DIGGING))
			return (FALSE);

		return (TRUE);
	}
	if (b_e == TRUE) {
		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid where I will hide */
		borg_digging = TRUE;
		borg_flow_enqueue_grid(c_y + b_y + ey[13], c_x + b_x + ex[13]);

		/* Spread the flow */
		borg_digging = TRUE;
		borg_flow_spread(5, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("anti-summon corridor east type 1", GOAL_DIGGING,
									 FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_DIGGING))
			return (FALSE);

		return (TRUE);
	}
	if (b_w == TRUE) {
		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid where I will hide */
		borg_digging = TRUE;
		borg_flow_enqueue_grid(c_y + b_y + wy[11], c_x + b_x + wx[11]);

		/* Spread the flow */
		borg_flow_spread(5, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("anti-summon corridor west type 1", GOAL_DIGGING,
									 FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_DIGGING))
			return (FALSE);

		return (TRUE);
	}

	return FALSE;
}

/*
 * Dig an anti-summon corridor
 *
 *            ############## We want the borg to not dig #1
 *            #............# but to dig #2, and hopefully shoot from the
 *      #######............# last #2 and try to avoid standing on #3.
 *      #222223............# This is great for offset ball attacks but
 *      #2#####..s.........# not for melee.  Warriors need to dig a wall
 * ######2###########+###### adjacent to the monsters so he can swing on them.
 * #            1     #
 * # ################ #
 *   #              # #
 * ###              # #
 *
 */
bool borg_flow_kill_corridor_2(/*bool viewable*/) {
	int o_y, o_x;
	int m_x, m_y;
	int f_y, f_x;
	int floors = 0;
	int monsters = 0;
	int b_y = 0, b_x = 0;
	int perma_grids = 0;

	borg_kill *kill;

	/* Efficiency -- Nothing to kill */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Only do this to summoners when they are close*/
	if (borg_kills_summoner == -1)
		return (FALSE);

	/* get the summoning monster */
	kill = &borg_kills[borg_kills_summoner];

	/* Only if the summoner is a never mover guy, otherwise use the Type 1
	 * corridor */
	if (!(r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE))
		return (FALSE);

	/* Do not dig when weak. It takes too long */
	if (borg_skill[BI_STR] < 17)
		return (FALSE);

	/* Sometimes we loop on this */
	if (time_this_panel > 500)
		return (FALSE);

	/* Do not dig when confused */
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Not when darkened */
	if (borg_skill[BI_CUR_LITE] == 0)
		return (FALSE);

	/* Not if sitting in a sea of runes */
	if ((borg_position & (POSITION_SEA | POSITION_SUMM | POSITION_BORE)))
		return (FALSE);

	/* Reset the adjacent monster count */
	monsters = 0;

	/* Consider each adjacent spot to monster*/
	for (o_x = -1; o_x <= 1; o_x++) {
		for (o_y = -1; o_y <= 1; o_y++) {
			borg_grid *ag;

			/* Check grids near monster */
			m_x = kill->x + o_x;
			m_y = kill->y + o_y;

			/* grid the grid */
			ag = &borg_grids[m_y][m_x];

			/* Monsters adjacent to summoner make it a risky maneuver */
			if (ag->kill)
				monsters++;

			/* avoid screen edgeds */
			if (m_x > AUTO_MAX_X - 2 || m_x < 2 || m_y > AUTO_MAX_Y - 2 || m_y < 2)
				continue;

			/* Can't tunnel a non wall or permawall*/
			if (ag->feat != FEAT_NONE && ag->feat < FEAT_MAGMA)
				continue;
			if (ag->feat >= FEAT_PERM_EXTRA && ag->feat <= FEAT_PERM_SOLID) {
				perma_grids++;
				continue;
			}

			/* Do not dig unless we appear strong enough to succeed or we have a
			 * digger */
			if (/* borg_spell_legal(REALM_SORCERY,1, 8) || */
				 borg_spell_legal(REALM_ARCANE, 2, 4) ||
				 borg_spell_legal(REALM_NATURE, 1, 0) ||
				 borg_spell_legal(REALM_CHAOS, 0, 6) ||
				 borg_racial_check(RACE_HALF_GIANT, 10) ||
				 borg_equips_activation(ACT_STONE_MUD, FALSE) ||
				 (borg_skill[BI_DIG] > BORG_DIG &&
				  borg_items[weapon_swap].tval == TV_DIGGING) ||
				 (borg_skill[BI_DIG] > BORG_DIG + 20))

			{
				/* digging ought to work */
			} else {
				/* do not try digging */
				continue;
			}

			/* reset floors counter */
			floors = 0;

			/* That grid must not have too many floors adjacent, or monsters */
			for (f_x = -1; f_x <= 1; f_x++) {
				for (f_y = -1; f_y <= 1; f_y++) {
					/* grid the grid */
					ag = &borg_grids[m_y + f_y][m_x + f_x];

					/* check if this neighbor is a floor */
					if (ag->feat == FEAT_FLOOR || ag->feat == FEAT_BROKEN)
						floors++;
				}
			}

			/* Do not dig if too many floors near. */
			if (floors >= 5)
				continue;

			/* or monsters */
			if (monsters >= 4) {
				b_y = 0;
				b_x = 0;
				continue;
			}

			/* Track the good location */
			b_y = m_y;
			b_x = m_x;
		}
	}
	/* NOTE: Perma_grids count the number of grids which contain permawalls.
	 * The borg may try to flow to an unknown grid but may get stuck on a perma
	 * wall.  This will keep him from flowing to a summoner if the summoner is
	 * near a perma grid.  The real fix out to be in the flow_spread so that
	 * he will not flow through perma_grids.  I will work on that next.
	 */
	if (b_y != 0 && b_x != 0 && perma_grids == 0 && floors < 5 && monsters < 4) {
		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid */
		borg_flow_enqueue_grid(m_y, m_x);

		/* Spread the flow */
		borg_flow_spread(15, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("anti-summon corridor 2", GOAL_DIGGING, FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_DIGGING))
			return (FALSE);

		return (TRUE);
	}

	return FALSE;
}

/*
 * Move to a grid unreachable by monsters.
 *
 * We want to move to a grid which is not reachable by a monster and it allows
 * us to range attack the monster.
 * 1  Grid must not be reachable by the monster
 * 2  Grid must a free and reachable by us (close range).
 * 3*  Monster must not be able to range attack me
 * 4  I must be able to range attack the monster
 * 5*  Try not to bounce
 * 6* Maybe our current grid is a good one and I don't need to move
 *
 */
bool borg_flow_kill_unreachable(int nearness) {
	int o_y, o_x;
	int y = c_y;
	int x = c_x;
	int limit = 16;
	int b_x[16];
	int b_y[16];
	/*int b_d[16];*/
	int count = -1;
	int d;
	bool proceed = FALSE;
	int i, ii;
	int monster_idx = -1;

	borg_kill *kill;

	monster_race *r_ptr;

	/* Efficiency -- Nothing to kill */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Limit the nearness check a bit */
	if (nearness > limit / 2)
		nearness = (limit / 2);

	/* DimDoor allows a bit more range */
	if (borg_skill[BI_ADIMDOOR] > 1)
		nearness *= 2;

	/* Avoid loops */
	if (time_this_panel > 500)
		return (FALSE);

	/* Reset all the 'good' grid place holders */
	for (i = 0; i < limit; i++) {
		b_y[i] = -1;
		b_x[i] = -1;
		/*b_d[i] = 5000;*/
	}

	/* Anti-loop check.
	 * Need to make sure I am not already protected.
	 */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill;

		/* Monster */
		kill = &borg_kills[i];
		r_ptr = &r_info[kill->r_idx];

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* skip friendlies */
		if (!kill->killer)
			continue;

		/* Provide some exclusion criteria for the monsters */
		if (!kill->cautious)
			continue;

		/* Some will get the borg regardless */
		if (((r_ptr->flags2 & RF2_PASS_WALL) ||
			  (r_ptr->flags2 & RF2_KILL_WALL)) &&
			 r_ptr->flags7 & RF7_CAN_FLY)
			continue;

		/* Need to make sure there is some distance */
		d = (kill->dist);
		if (d > 8)
			continue;

		/* Examine the monster's ability to get to this grid */
		borg_flow_clear_m();
		borg_flow_enqueue_grid_m(c_y, c_x);
		borg_flow_spread_m(nearness + limit, i, kill->r_idx);

		/* if the monster can flow to this grid, then we should proceed to look
		 * for another spot */
		if (borg_flow_commit_m(kill->y, kill->x)) {
			proceed = TRUE;
		}
	}

	/* No need to do these checks */
	if (proceed == FALSE)
		return (FALSE);

	/* Consider each spot within range */
	for (o_x = -nearness; o_x <= nearness; o_x++) {
		for (o_y = -nearness; o_y <= nearness; o_y++) {
			/* where the borg is looking */
			x = c_x + o_x;
			y = c_y + o_y;

			/* avoid screen edgeds */
			if (!in_bounds(y, x))
				continue;

			/* avoid my own grid */
			if (x == c_x && c_y == y)
				continue;

			/* Avoid sitting in a wall and unknown grids */
			if (borg_grids[y][x].feat >= FEAT_SECRET &&
				 borg_grids[y][x].feat <= FEAT_PERM_SOLID)
				continue;
			if (borg_grids[y][x].feat == FEAT_NONE)
				continue;

			/* Clear the flow codes */
			borg_flow_clear();

			/* Enqueue the grid */
			borg_flow_enqueue_grid(y, x);

			/* Spread the flow */
			borg_flow_spread(nearness, TRUE, FALSE, FALSE, -1, TRUE);

			/* could the borg get there? */
			if (borg_data_cost->data[c_y][c_x] >= nearness + 3)
				continue;

			/* Make sure we do not end up next to or on top of a monster on that
			 * grid*/
			for (i = 0; i < borg_kills_nxt; i++) {
				if (distance(y, x, borg_kills[i].y, borg_kills[i].x) <= 1)
					break;
			}
			if (i != borg_kills_nxt)
				continue;

			/* See if the monster is forbidden from getting near it. */
			for (i = 1; i < borg_kills_nxt; i++) {
				borg_kill *kill;

				/* Monster */
				kill = &borg_kills[i];
				r_ptr = &r_info[kill->r_idx];

				/* Skip dead monsters */
				if (!kill->r_idx)
					continue;

				/* Provide some exclusion criteria for the monsters */
				if (!kill->cautious)
					continue;

				/* skip friendlies */
				if (!kill->killer)
					continue;

				/* Monster needs to be LOS from that grid so I can beat on him with
				 * ranged spells */
				if (!borg_projectable_pure(y, x, kill->y, kill->x))
					continue;

				/* Examine the monster's ability to get to this grid */
				borg_flow_clear_m();
				borg_flow_enqueue_grid_m(y, x);
				d = (distance(y, x, kill->y, kill->x));
				borg_flow_spread_m(nearness + d + 1, i, kill->r_idx);

				/* if the monster cannot flow to this grid &
				 * and he cannot threaten with ranged attacks,
				 * can he get to the adjacent ones?
				 */
				if (!borg_flow_commit_m(kill->y, kill->x))
					if (borg_danger_aux(y, x, 1, i, TRUE, FALSE) < avoidance / 10) {
						/* These grids will work, remember them. */
						for (ii = 0; ii < limit; ii++) {
							/* Do I already have this one marked? */
							if ((b_x[ii] == x && b_y[ii] == y) || count == limit - 1)
								break;

							count++;
							b_x[count] = x;
							b_y[count] = y;
							monster_idx = i;

							break;
						}
					}
			}
		}
	}

	/* Attempt to start the movement*/
	if (count != -1) {
		borg_flow_clear();

		/****  Some how pick the best one.  Right now it is only selecting the
		* closest
		* on the list.  Perhaps there is some criteria to assess the various
		* spots.
		* how much danger added along the path?
		* XXX
		*/
		for (i = count; i >= 1; i--) {
			/* Sort the entries */
			if (distance(c_y, c_x, b_y[i], b_x[i]) <
				 distance(c_y, c_x, b_y[i - 1], b_x[i - 1])) {
				b_y[i - 1] = b_y[i];
				b_x[i - 1] = b_x[i];
			}
		}

		/* enqueue the list */
		borg_flow_enqueue_grid(b_y[0], b_x[0]);

		/* attempt to DimDoor over there */
		if (distance(c_y, c_x, b_y[0], b_x[0]) >= 3 &&
			 borg_dim_door_to(b_y[0], b_x[0]))
			return (TRUE);

		/* Spread the flow */
		borg_flow_spread(nearness, TRUE, FALSE, FALSE, -1, TRUE);

		kill = &borg_kills[monster_idx];
		r_ptr = &r_info[kill->r_idx];

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("unreachable grid", GOAL_UNREACH, FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_UNREACH))
			return (FALSE);

		/* Success */
		return (TRUE);
	}

	/* restore the saved player position */
	return (FALSE);
}

/*
 * Attempt to flow to a safe grid in order to rest up properly.  Following a
 * battle, a borg needs to heal up.
 * He will attempt to heal up right where the fight was, but if he cannot, then
 * he needs to retreat a bit.
 * This will help him find a good safe place to hide.
 *
 */
bool borg_flow_recover(/*bool viewable,*/ int dist) {
	/* int b_y = 0, b_x = 0; */
	int i, x, y;
	int d;

	/* Not in town */
	if (borg_skill[BI_CDEPTH] == 0)
		return (FALSE);

	/* Sometimes we loop on this */
	if (time_this_panel > 500)
		return (FALSE);

	/* Anti-summon corridor is a good place to recover */
	if ((borg_position & POSITION_SUMM))
		return (FALSE);

	/* No retreating and recovering when low level */
	if (borg_skill[BI_CLEVEL] <= 5)
		return (FALSE);

	/* Mana for spell casters */
	if (borg_skill[BI_MAXSP] >= 2) {
		if (borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 3 &&
			 borg_skill[BI_CURSP] >
				  borg_skill[BI_MAXSP] / 4 && /* Non spell casters? */
			 !borg_skill[BI_ISCUT] &&
			 !borg_skill[BI_ISSTUN] && !borg_skill[BI_ISHEAVYSTUN] &&
			 !borg_skill[BI_ISAFRAID])
			return (FALSE);

		/* No resting if casting and healthy */
		if (borg_skill[BI_CURHP] == borg_skill[BI_MAXHP] &&
			 (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 4 ||
			  borg_no_rest_prep > 2))
			return (FALSE);

	} else /* Non Spell Casters */
	{
		/* do I need to recover some? */
		if (borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 3 &&
			 !borg_skill[BI_ISCUT] && !borg_skill[BI_ISSTUN] &&
			 !borg_skill[BI_ISHEAVYSTUN] && !borg_skill[BI_ISAFRAID])
			return (FALSE);
	}

	/* If Scumming, then do not rest */
	if (borg_lunal_mode || borg_munchkin_mode)
		return (FALSE);

	/* No need if hungry */
	if (borg_skill[BI_ISHUNGRY])
		return (FALSE);

	/* Not if fighting a unique, stay to finish the fight */
	/* if (borg_fighting_unique) return (FALSE); */

	/* Not if fighting a passwalling monster */
	if (borg_fighting_tunneler)
		return (FALSE);

	/* No need if passwalling */
	if ((borg_skill[BI_PASSWALL] /*|| borg_race == RACE_SPECTRE*/) &&
		 borg_grids[c_y][c_x].feat >= FEAT_DOOR_HEAD &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID)
		return (FALSE);

	/* Nothing found */
	borg_temp_n = 0;

	/* Scan some known Grids
	 * Favor the following types of grids:
	 * 1. Happy grids
	 */

	/* look at grids within 20 grids of me */
	for (y = c_y - 25; y < c_y + 25; y++) {

		for (x = c_x - 25; x < c_x + 25; x++) {
			/* Stay in bounds */
			if (!in_bounds(y, x))
				continue;

			/* Skip my own grid */
			if (y == c_y && x == c_x)
				continue;

			/* Skip grids that are too close to me */
			d = (distance(c_y, c_x, y, x));
			if (d < 7 && d > 2)
				continue;

			/* While I am checking for grids, check the adjacent ones to make sure
			 * a monster
			 * is not sitting next to me.  I don't want to flee a battle in order
			 * to recover
			 */
			if (d == 1 && borg_grids[y][x].kill)
				return (FALSE);

			/* Is this grid a happy grid? */
			if (!borg_happy_grid_bold(y, x) && (d > 2 || track_step_num >= 50))
				continue;

			/* Cant rest on a wall grid. */
			if (!borg_skill[BI_PASSWALL] &&
				 (borg_grids[y][x].feat == FEAT_DOOR_HEAD + 0x00 ||
				  borg_grids[y][x].feat == FEAT_RUBBLE ||
				  borg_grids[y][x].feat == FEAT_MAGMA ||
				  borg_grids[y][x].feat == FEAT_MAGMA_K ||
				  borg_grids[y][x].feat == FEAT_WALL_EXTRA))
				continue;
			if (borg_grids[y][x].feat == FEAT_PERM_EXTRA ||
				 borg_grids[y][x].feat == FEAT_PERM_EXTRA ||
				 borg_grids[y][x].feat == FEAT_PERM_INNER ||
				 borg_grids[y][x].feat == FEAT_PERM_SOLID)
				continue;

			/* Can I rest on that one? */
			if (!borg_check_rest(y, x))
				continue;

			/* Careful -- Remember it */
			borg_temp_x[borg_temp_n] = x;
			borg_temp_y[borg_temp_n] = y;
			borg_temp_n++;
		}
	}

	/* Nothing */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look through the good grids */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);

		/* Attempt to DimDoor there */
		if (distance(borg_temp_y[i], borg_temp_x[i], c_y, c_x) >= 4 &&
			 borg_dim_door_to(borg_temp_y[i], borg_temp_x[i]))
			return (TRUE);
	}

	/* Spread the flow */
	borg_flow_spread(dist, FALSE, TRUE, FALSE, -1, TRUE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Recover Grid", GOAL_RECOVER, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_RECOVER))
		return (FALSE);

	return (TRUE);
}

/*
 * Prepare to "flow" towards monsters to "kill"
 * But in a few phases, viewable, near and far.
 * Note that monsters under the player are always deleted
 */
bool borg_flow_kill(bool viewable, int nearness) {
	int i, x, y, p, j, b_j = -1;
	int b_stair = -1;
	int d;
	bool borg_in_hall = FALSE;
	int hall_y, hall_x, hall_walls = 0;
	bool skip_monster = FALSE;

	borg_grid *ag;

	/* Efficiency -- Nothing to kill */
	if (!borg_kills_cnt)
		return (FALSE);

	/* Already chasing one down */
	if (goal == GOAL_KILL)
		return (FALSE);

	/* Don't chase down town monsters when you are just starting out */
	if (borg_skill[BI_CDEPTH] == 0 && borg_skill[BI_CLEVEL] < 15 &&
		 (!borg_skill[BI_ISWEAK] && !borg_skill[BI_VAMPIRE]))
		return (FALSE);

	/* Not when afraid */
	if (borg_skill[BI_ISAFRAID])
		return (FALSE);

	/* YOU ARE NOT A WARRIOR!! DON'T ACT LIKE ONE!! */
	if (borg_skill[BI_NO_MELEE] &&
		 borg_skill[BI_CLEVEL] < (borg_skill[BI_CDEPTH] ? 35 : 5))
		return (FALSE);

	/* Not if sitting in a sea of runes */
	if ((borg_position & (POSITION_SEA | POSITION_BORE)))
		return (FALSE);

	/* Nothing found */
	borg_temp_n = 0;

	/* check to see if in a hall, used later */
	for (hall_x = -1; hall_x <= 1; hall_x++) {
		for (hall_y = -1; hall_y <= 1; hall_y++) {
			/* Acquire location */
			x = hall_x + c_x;
			y = hall_y + c_y;

			ag = &borg_grids[y][x];

			/* track walls */
			if ((ag->feat == FEAT_GLYPH) || (ag->feat == FEAT_MINOR_GLYPH) ||
				 ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_PERM_SOLID))) {
				hall_walls++;
			}

			/* addem up */
			if (hall_walls >= 5)
				borg_in_hall = TRUE;
		}
	}

	/* Check distance away from stairs, used later */

	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Scan the monster list */
	for (i = 1; i < borg_kills_nxt; i++) {
		borg_kill *kill = &borg_kills[i];

		/* Reset the skip monster flag */
		skip_monster = FALSE;

		/* Skip dead monsters */
		if (!kill->r_idx)
			continue;

		/* Ignore multiplying monsters */
		if (goal_ignoring && !borg_skill[BI_ISAFRAID] &&
			 (r_info[kill->r_idx].flags2 & RF2_MULTIPLY))
			continue;

		/* Ignore molds when low level */
		if (borg_skill[BI_MAXCLEVEL] < 5 &&
			 (r_info[kill->r_idx].flags1 & RF1_NEVER_MOVE))
			continue;

		/* Avoid fighting if a scary guy is on the level */
		if ((borg_depth & DEPTH_SCARY))
			continue;

		/* Don't chase our friends or pets */
		if (kill->ally)
			continue;

		/* Not if monster is hard to kill */
		if (kill->avoid)
			continue;

		/* Avoid multiplying monsters when low level */
		if (borg_skill[BI_CLEVEL] < 10 &&
			 (r_info[kill->r_idx].flags2 & RF2_MULTIPLY))
			continue;

		/* Hack -- ignore Maggot until later.  Player will chase Maggot
		 * down all accross the screen waking up all the monsters.  Then
		 * he is stuck in a comprimised situation.
		 */
		if (kill->unique && borg_skill[BI_CDEPTH] == 0 &&
			 borg_skill[BI_CLEVEL] < 5)
			continue;

		/* Make sure he does not continually chase a hidden ghost in a wall */
		d = kill->dist;
		if (d < 3 && (kill->when < borg_t - 10))
			continue;
		if (kill->when < borg_t - 50)
			continue;

		/* Access the location */
		x = kill->x;
		y = kill->y;

		/* Get the grid */
		ag = &borg_grids[y][x];

		/* Require line of sight if requested */
		if (viewable && !(ag->info & BORG_VIEW))
			continue;

		/* Calculate danger */
		borg_full_damage = FALSE;
		p = borg_danger(y, x, 1, TRUE);
		borg_full_damage = FALSE;

		/* Hack -- Skip "deadly" monsters unless uniques*/
		if (borg_skill[BI_CLEVEL] > 15 && (!kill->unique) && p > avoidance / 2)
			continue;
		if (borg_skill[BI_CLEVEL] <= 15 && p > avoidance / 3)
			continue;

		/* Skip town monsters that are outside of town */
		if (borg_skill[BI_CDEPTH] == 0 &&
			 ((borg_skill[BI_VAMPIRE] && !borg_skill[BI_ISWEAK]) ||
			  (!borg_skill[BI_VAMPIRE]))) {
			/* Don't chase down town monsters when you are high level */
			if (borg_skill[BI_CLEVEL] > 25 && kill->level < 5)
				skip_monster = TRUE;
		}

		/* Hack -- Avoid getting surrounded */
		if (borg_in_hall && (r_info[kill->r_idx].flags1 & RF1_FRIENDS)) {
			/* check to see if monster is in a hall, */
			for (hall_x = -1; hall_x <= 1; hall_x++) {
				for (hall_y = -1; hall_y <= 1; hall_y++) {
					ag = &borg_grids[hall_y + y][hall_x + x];

					/* track walls */
					if ((ag->feat == FEAT_GLYPH) || (ag->feat == FEAT_MINOR_GLYPH) ||
						 ((ag->feat >= FEAT_MAGMA) &&
						  (ag->feat <= FEAT_PERM_SOLID))) {
						hall_walls++;
					}

					/* we want the monster to be in a hall also
					 *
					 *  ########################
					 *  ############      S  ###
					 *  #         @'     SSS ###
					 *  # ##########       SS###
					 *  # #        #       Ss###
					 *  # #        ###### ######
					 *  # #             # #
					 * Currently, we would like the borg to avoid
					 * flowing to a situation like the one above.
					 * We would like him to stay in the hall and
					 * attack from a distance.  One problem is the
					 * lower case 's' in the corner, He will show
					 * up as being in a corner, and the borg may
					 * flow to it.  Let's hope that is a rare case.
					 *
					 * The borg might flow to the 'dark' south exit
					 * of the room.  This would be dangerous for
					 * him as well.
					 */
					/* add 'em up */
					if (hall_walls < 4) {
						/* This monster is not in a hallway.
						 * It may not be safe to fight.
						 */
						skip_monster = TRUE;
					}
				}
			}
		}

		/* Skip this one if it is just 2 grids from me and it can attack me as
		 * soon as I
		 * move 1 grid closer to it.  Note that some monsters are faster than me
		 * and it
		 * could still cover the 1 grid and hit me. I'll fix it (based on my
		 * speed) later XXX
		 */
		if (d == 2 &&						  /* Spacing is important */
			 (!(kill->ranged_attack)) && /* Ranged Attacks, don't rest. */
			 (!(r_info[kill->r_idx].flags1 &
				 RF1_NEVER_MOVE))) /* Skip monsters that dont chase */
		{
			skip_monster = TRUE;
		}

		/* skip certain ones */
		if (skip_monster)
			continue;

		/* Clear the flow codes */
		borg_flow_clear();

		/* Check the distance to stair for this proposed grid (leash)*/
		if (borg_flow_cost_stair(y, x, b_stair) > borg_skill[BI_CLEVEL] * 3 + 9 &&
			 borg_skill[BI_CLEVEL] < 20)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing to kill */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look for something to kill */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
	}

	/* Spread the flow */
	/* if we are not flowing toward monsters that we can see, make sure they */
	/* are at least easily reachable.  The second flag is whether or not */
	/* to avoid unknown squares.  This was for performance when we have ESP. */
	borg_flow_spread(nearness, TRUE, !viewable, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("kill", GOAL_KILL, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_KILL))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards mineral veins with treasure
 *
 */
bool borg_flow_vein(bool viewable, int nearness) {
	int i, x, y;
	int b_stair = -1, j, b_j = -1;
	int cost = 0;
	int leash = borg_skill[BI_CLEVEL] * 3 + 9;

	borg_grid *ag;

	/* Efficiency -- Nothing to take */
	if (!track_vein_num)
		return (FALSE);

	/* If rich, don't bother */
	if (borg_gold >= 100000)
		return (FALSE);

	/* Increase leash */
	if (borg_skill[BI_CLEVEL] >= 20)
		leash = 250;

	/* Require digger, capacity, or skill */
	if (borg_skill[BI_ASTONE2MUD] <= 0 &&
		 borg_items[weapon_swap].tval != TV_DIGGING)
		return (FALSE);

	/* Nothing yet */
	borg_temp_n = 0;

	/* Set the searching flag for low level borgs */
	borg_needs_searching = TRUE;

	/* Check distance away from stairs, used later */
	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Scan the vein list */
	for (i = 0; i < track_vein_num; i++) {
		/* Access the location */
		x = track_vein_x[i];
		y = track_vein_y[i];

		/* If the vein is under me, then delete them all and rebuild the list */
		if (x == c_x && y == c_y) {
			/* Remove all the veins */
			track_vein_num = 0;

			/* no flowing to veins */
			return (FALSE);
		}

		/* Get the grid */
		ag = &borg_grids[y][x];

		/* Require line of sight if requested */
		if (viewable && !(ag->info & BORG_VIEW))
			continue;

		/* Clear the flow codes */
		borg_flow_clear();

		/* obtain the number of steps from this take to the stairs */
		cost = borg_flow_cost_stair(y, x, b_stair);

		/* Check the distance to stair for this proposed grid, unless i am looking
		 * for very close items (leash) */
		if (nearness > 5 && cost > leash)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing to mine */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look for something to take */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
	}

	/* Spread the flow */
	/* if we are not flowing toward items that we can see, make sure they */
	/* are at least easily reachable.  The second flag is weather or not  */
	/* to avoid unkown squares.  This was for performance. */
	borg_flow_spread(nearness, TRUE, !viewable, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("vein", GOAL_TAKE, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TAKE))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_scum(bool viewable, int nearness) {
	int i, x, y;
	int j;
	int b_j = -1;
	int b_stair = -1;

	borg_grid *ag;

	/* Efficiency -- Nothing to take */
	if (!borg_takes_cnt)
		return (FALSE);

	/* Require one empty slot */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* Not if scaryguys on the level */
	if ((borg_depth & DEPTH_SCARY))
		return (FALSE);

	/* Nothing yet */
	borg_temp_n = 0;

	/* Set the searching flag for low level borgs */
	borg_needs_searching = TRUE;

	/* Check distance away from stairs, used later */

	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Scan the object list -- set filter*/
	for (i = 1; i < borg_takes_nxt; i++) {
		borg_take *take = &borg_takes[i];

		/*int a;*/
		/*bool item_bad;*/

		/* Skip dead objects */
		if (!take->k_idx)
			continue;

		/* skip worthless items */
		if (take->value <= 0)
			continue;

		/* Access the location */
		x = take->x;
		y = take->y;

		/* Get the grid */
		ag = &borg_grids[y][x];

		/* Require line of sight if requested */
		if (viewable && !(ag->info & BORG_VIEW))
			continue;

		/* Clear the flow codes */
		borg_flow_clear();

		/* Check the distance to stair for this proposed grid */
		if (borg_flow_cost_stair(y, x, b_stair) > borg_skill[BI_CLEVEL] * 3 + 9 &&
			 borg_skill[BI_CLEVEL] < 20)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing to take */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look for something to take */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
	}

	/* Spread the flow */
	/* if we are not flowing toward items that we can see, make sure they */
	/* are at least easily reachable.  The second flag is weather or not  */
	/* to avoid unknown squares.  This was for performance. */
	borg_flow_spread(nearness, TRUE, !viewable, FALSE, -1, TRUE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Scum item", GOAL_TAKE, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TAKE))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take(bool viewable, int nearness) {
	int i, x, y;
	int b_stair = -1, j, b_j = -1;
	int cost = 0;
	int leash = borg_skill[BI_CLEVEL] * 3 + 9;

	borg_grid *ag;

	/* Efficiency -- Nothing to take */
	if (!borg_takes_cnt)
		return (FALSE);

	/* Needing to get off level */
	if (goal_rising)
		return (FALSE);

	/* Increase leash */
	if (borg_skill[BI_CLEVEL] >= 20)
		leash = 250;

	/* Require one empty slot */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* Nothing yet */
	borg_temp_n = 0;

	/* Set the searching flag for low level borgs */
	borg_needs_searching = TRUE;

	/* Check distance away from stairs, used later */
	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Scan the object list */
	for (i = 1; i < borg_takes_nxt; i++) {
		borg_take *take = &borg_takes[i];

		int a;
		bool item_bad;

		/* Skip dead objects */
		if (!take->k_idx)
			continue;

		/* skip worthless items */
		if (take->value <= 0)
			continue;

		/* Recently seen */
		if (borg_t - take->when > 10)
			continue;

		/* Access the location */
		x = take->x;
		y = take->y;

		/* Get the grid */
		ag = &borg_grids[y][x];

		/* look to see if this is on the bad items list */
		item_bad = FALSE;
		for (a = 0; a < bad_obj_num; a++) {
			if (x == bad_obj_x[a] && y == bad_obj_y[a])
				item_bad = TRUE;
		}

		/* Is it on the good item list? */
		for (a = 0; a < good_obj_num; a++) {
			if (x == good_obj_x[a] && y == good_obj_y[a])
				item_bad = FALSE;
		}

		/** make sure take sits on safe grid, avoid lava
		 * This seems stupid, items surving lava should be good..
		if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
			item_bad = TRUE;
		if (ag->feat == FEAT_SHAL_LAVA && !borg_skill[BI_IFIRE] &&
			 !borg_skill[BI_FEATH])
			item_bad = TRUE;
		*/

		/* Avoid Water if dangerous */
		if (ag->feat == FEAT_WATER &&
			 (borg_skill[BI_ENCUMBERD] && !borg_skill[BI_FEATH]))
			item_bad = TRUE;

		/* it is a bad item, do not track it */
		if (item_bad)
			continue;

		/* Require line of sight if requested */
		if (viewable && !(ag->info & BORG_VIEW))
			continue;

		/* No need to chase certain things down after a certain amount.  Dont
		 * chase:
		 * Money
		 * Other spell books
		 * Wrong ammo
		 */
		if (borg_gold >= 200000) {
			if (take->k_idx >= K_MONEY_START && take->k_idx <= K_MONEY_STOP)
				continue;
			if (take->tval != REALM1_BOOK && take->tval == REALM2_BOOK)
				continue;
			if ((take->tval == TV_SHOT || take->tval == TV_ARROW ||
				  take->tval == TV_BOLT) &&
				 take->tval != my_ammo_tval)
				continue;
			/*
			Restore Mana for warriors?
			low level potions
			low level scrolls
			*/
		}

		/* Clear the flow codes */
		borg_flow_clear();

		/* obtain the number of steps from this take to the stairs */
		cost = borg_flow_cost_stair(y, x, b_stair);

		/* Check the distance to stair for this proposed grid, unless i am looking
		 * for very close items (leash) */
		if (nearness > 5 && cost > leash && borg_skill[BI_CLEVEL] < 20)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing to take */
	if (!borg_temp_n)
		return (FALSE);

	/* Not if monsters are too close to the borg */
	if (borg_skill[BI_CLEVEL] < 5) {
		for (i = 1; i < borg_kills_nxt; i++) {
			/* Skip "dead" monsters */
			if (!borg_kills[i].r_idx)
				continue;

			/* Distance away */
			if (borg_kills[i].dist < 5)
				return (FALSE);
		}
	}

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look for something to take */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
	}

	/* Spread the flow */
	/* if we are not flowing toward items that we can see, make sure they */
	/* are at least easily reachable.  The second flag is weather or not  */
	/* to avoid unkown squares.  This was for performance. */
	borg_flow_spread(nearness, TRUE, !viewable, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("item", GOAL_TAKE, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TAKE))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_lunal(bool viewable, int nearness) {
	int i, ii, x, y;
	int j;
	int b_j = -1;
	int b_stair = -1;
	/*int leash;*/
	bool dim_door = FALSE;

	int hole = INVEN_PACK - 1;
	s32b p, b_p = 0L;
	char buf[256];

	borg_grid *ag;
	/*object_kind *k_ptr;*/
	/*cave_type *c_ptr;*/
	/*object_type *o_ptr;*/

	/* Trump are the only ones with Dim Door during the clevel time when scumming
	 * happens.
	 * Otherwise, Sorc_realm (2,3) and Mindcrafter (MIND_MINOR_DISP, 40) has Dim
	 * Door.
	 */
	borg_magic *as;

	/* Efficiency -- Nothing to take */
	if (!borg_takes_cnt)
		return (FALSE);

	as = &borg_magics[REALM_TRUMP][0][5];

	/* If Dim Door is available, safe to move out a bit if I can Dim-Door back
	 * again. */
	if (borg_spell_okay_fail(REALM_TRUMP, 0, 5, 17) &&
		 as->power * 2 < borg_skill[BI_CURSP]) {
		/* Size of Dim Door range */
		/*leash = borg_skill[BI_CLEVEL] + 2;*/
		dim_door = TRUE;
	}

	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Nothing yet */
	borg_temp_n = 0;

	/* Set the searching flag for low level borgs */
	borg_needs_searching = TRUE;

	/* Scan the object list -- set filter*/
	for (i = 1; i < borg_takes_nxt; i++) {
		borg_take *take = &borg_takes[i];
		object_kind *k_ptr = &k_info[take->k_idx];
		cave_type *c_ptr = &cave[take->y][take->x];
		/*object_type *o_ptr = &o_list[c_ptr->o_idx];*/

		int a;
		bool item_bad;

		/* Skip dead objects */
		if (!take->k_idx)
			continue;

		/* Access the location */
		x = take->x;
		y = take->y;

		/* all items start bad */
		item_bad = TRUE;

		/* Gold is good to have */
		if (take->k_idx >= K_MONEY_START && take->k_idx <= K_MONEY_STOP) {
			borg_note(format("# Lunal Item %s, at %d,%d",
								  k_name + k_info[take->k_idx].name, y, x));
			item_bad = FALSE;
		}

		/* If full can I absorb the item? */
		if (borg_items[INVEN_PACK - 1].iqty && item_bad == TRUE) {
			/* Scan the inventory */
			for (ii = 0; ii < INVEN_PACK - 1; ii++) {
				/* skip empty slots */
				if (!borg_items[ii].iqty)
					continue;

				/* Certain types of items can stack */
				if (k_ptr->sval == borg_items[ii].sval &&
					 k_ptr->tval == borg_items[ii].tval &&
					 (borg_items[ii].tval == TV_POTION ||
					  borg_items[ii].tval == TV_SCROLL ||
					  borg_items[ii].tval == TV_ROD)) {
					item_bad = FALSE;
				}
			}
		}

		/* Require one empty slot */
		if (!borg_items[INVEN_PACK - 1].iqty && item_bad == TRUE) {
			/* Certain Potions are good to have */
			if (take->k_idx == POTION_STR || take->k_idx == POTION_INT ||
				 take->k_idx == POTION_WIS || take->k_idx == POTION_DEX ||
				 take->k_idx == POTION_CON || take->k_idx == POTION_CHR ||
				 take->k_idx == POTION_INC_ALL || take->k_idx == POTION_HEAL ||
				 take->k_idx == POTION_STAR_HEAL ||
				 (take->k_idx >= POTION_CURE_CRIT && take->k_idx <= POTION_EXP)) {
				borg_note(format("# Lunal Item %s, at %d,%d",
									  k_name + k_info[take->k_idx].name, y, x));
				item_bad = FALSE;
			}

			/* Certain insta_arts are good */
			if (take->k_idx >= K_PHIAL && take->k_idx <= K_GEM) {
				borg_note(format("# Lunal Item %s, at %d,%d",
									  k_name + k_info[take->k_idx].name, y, x));
				item_bad = FALSE;
			}

			/* if scumming the start of the game, take all items to sell them */
			if (borg_munchkin_start) {
				/* Certain known items are junky and should be ignored.  Grab only
				 * things of value
				 */
				if (take->value >= 1 &&
					 ((borg_skill[BI_CDEPTH] >= borg_skill[BI_MAXDEPTH] - 10) ||
					  take->tval == my_ammo_tval))
					item_bad = FALSE;
			}

			/*** Would the acquisition of the item be good for my power? ***/

			/* Is the item an inventory item? */
			if (borg_wield_slot_take(take) < INVEN_WIELD) {
				/* Base power without the item */
				p = my_power;

				/* Default to "nothing" */
				buf[0] = '\0';

				/* Describe it */
				object_desc(buf, &o_list[c_ptr->o_idx], TRUE, 3);

				/* Save hole (could be either empty slot or stack */
				COPY(&safe_items[hole], &borg_items[hole], borg_item);

				/* Analyze the item in the last inven slot (no price) */
				borg_item_analyze(&borg_items[hole], &o_list[c_ptr->o_idx], buf,
										hole);

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the equipment */
				b_p = borg_power();

				/* Is it worth grabbing */
				if (p < b_p)
					item_bad = FALSE;

				/* Restore hole */
				COPY(&borg_items[hole], &safe_items[hole], borg_item);

				/* Examine the inventory */
				borg_notice(TRUE);
			}

			/* look to see if this is on the bad items list */
			for (a = 0; a < bad_obj_num; a++) {
				if (x == bad_obj_x[a] && y == bad_obj_y[a])
					item_bad = TRUE;
			}
			/* Is it on the good item list? */
			for (a = 0; a < good_obj_num; a++) {
				if (x == good_obj_x[a] && y == good_obj_y[a])
					item_bad = FALSE;
			}

			/* If starving, do not waste time fetching non-food.  Get to town */
			if (borg_skill[BI_FOOD] <= 1 && take->tval != TV_FOOD)
				item_bad = TRUE;

			if (borg_items[INVEN_LITE].tval == TV_LITE &&
				 borg_items[INVEN_LITE].sval == SV_LITE_TORCH &&
				 borg_items[INVEN_LITE].timeout < 500 && take->tval == TV_LITE &&
				 (take->sval == SV_LITE_TORCH || take->sval == SV_LITE_LANTERN))
				item_bad = FALSE;
			if (borg_items[INVEN_LITE].tval == TV_LITE &&
				 borg_items[INVEN_LITE].sval == SV_LITE_LANTERN &&
				 borg_items[INVEN_LITE].timeout < 500 &&
				 ((take->tval == TV_LITE && take->sval == SV_LITE_LANTERN) ||
				  take->tval == TV_FLASK))
				item_bad = FALSE;
			if (borg_skill[BI_ISWEAK] && take->tval == TV_FOOD &&
				 borg_skill[BI_VAMPIRE])
				item_bad = FALSE;

			/* Not if it is adjacent to a monster */
			for (p = 0; p < 8; p++) {
				ag = &borg_grids[take->y + ddy_ddd[p]][take->x + ddx_ddd[p]];
				if (ag->kill)
					item_bad = TRUE;
			}
		}

		/* it is a bad item, do not track it */
		if (item_bad)
			continue;

		/* Get the grid */
		ag = &borg_grids[y][x];

		/* Require line of sight if requested */
		if (viewable && !(ag->info & BORG_VIEW))
			continue;

		/* Clear the flow codes */
		borg_flow_clear();

		/* Can I Dim_Door over to the item */
		if (dim_door) {
			/* Too close. out of Dim_door range */
			if (distance(c_y, c_x, y, x) < 5)
				dim_door = FALSE;
		}

		/* Check the distance to stair for this proposed grid */
		if (dim_door == FALSE) {
			if (borg_flow_cost_stair(y, x, b_stair) >
					  borg_skill[BI_CLEVEL] * 3 + 9 &&
				 borg_skill[BI_CLEVEL] < 20)
				continue;
		}

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing to take */
	if (!borg_temp_n)
		return (FALSE);

	/* Before leaving the stair, make sure HP is full */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] &&
		 !borg_skill[BI_ISHUNGRY]) {
		/* We have already determined that it is a safe grid so rest */
		borg_keypress(',');
		return (TRUE);
	}

	/* Clear the flow codes */
	borg_flow_clear();

	/* Look for something to take */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);

		/* Attempt the Dim Door to the item */
		if (dim_door && borg_dim_door_to(borg_temp_y[i], borg_temp_x[i]))
			return (TRUE);
	}

	/* Spread the flow */
	/* if we are not flowing toward items that we can see, make sure they */
	/* are at least easily reachable.  The second flag is weather or not  */
	/* to avoid unknown squares.  This was for performance. */
	borg_flow_spread(nearness, TRUE, !viewable, FALSE, -1, TRUE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("lunal item", GOAL_TAKE, FALSE))
		return (FALSE);

	/* Before walking over, check for monsters hiding in the halls */
	if ((borg_grids[c_y][c_x].feat == FEAT_MORE ||
		  borg_grids[c_y][c_x].feat == FEAT_LESS) &&
		 borg_check_lite())
		return (TRUE);

	/* Take one step */
	if (!borg_flow_old(GOAL_TAKE))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Determine if a grid is "interesting" (and should be explored)
 *
 * A grid is "interesting" if it is a closed door, rubble, hidden treasure,
 * or a visible trap, or an "unknown" grid.
 * or a non-perma-wall adjacent to a perma-wall. (GCV)
 *
 * b_stair is the index to the closest upstairs.
 */
extern bool borg_flow_dark_interesting(int y, int x) {
	int oy;
	int ox, i;

	borg_grid *ag;

	/* Have the borg so some Searching */
	borg_needs_searching = TRUE;

	/* Get the borg_grid */
	ag = &borg_grids[y][x];

	/* Explore unknown grids */
	if (ag->feat == FEAT_NONE)
		return (TRUE);

	/* Efficiency -- Ignore "boring" grids */
	if (ag->feat < FEAT_TRAP_TRAPDOOR)
		return (FALSE);

	/* Explore "known treasure" */
	if ((ag->feat == FEAT_MAGMA_K) || (ag->feat == FEAT_QUARTZ_K)) {
		/* Do not disarm when confused */
		if (borg_skill[BI_ISCONFUSED])
			return (FALSE);

		/* Do not bother if super rich */
		if (borg_gold >= 350000)
			return (FALSE);

		/* Not when darkened */
		if (borg_skill[BI_CUR_LITE] == 0)
			return (FALSE);

		/* Allow "stone to mud" ability */
		if (borg_skill[BI_ASTONE2MUD] >= 1000)
			return (TRUE);

		/* Do not dig unless we appear strong enough to succeed or we have a
		 * digger */
		if ((borg_skill[BI_DIG] > BORG_DIG &&
			  borg_items[weapon_swap].tval == TV_DIGGING) ||
			 (borg_skill[BI_DIG] > BORG_DIG + 20)) {
			/* digging ought to work */
		} else {
			return (FALSE);
		}

		/* Okay */
		return (TRUE);
	}

	/* "Vaults" Explore non perma-walls adjacent to a perma wall */
	if (ag->feat == FEAT_WALL_EXTRA || ag->feat == FEAT_MAGMA ||
		 ag->feat == FEAT_QUARTZ) {
		/* Do not attempt when confused */
		if (borg_skill[BI_ISCONFUSED])
			return (FALSE);

		/* hack and cheat.  No vaults  on this level */
		if (!(borg_depth & DEPTH_VAULT))
			return (FALSE);

		/* AJG Do not attempt on the edge */
		if (x < AUTO_MAX_X - 1 && y < AUTO_MAX_Y - 1 && x > 1 && y > 1) {
			/* scan the adjacent grids */
			for (ox = -1; ox <= 1; ox++) {
				for (oy = -1; oy <= 1; oy++) {

					/* Acquire location */
					ag = &borg_grids[oy + y][ox + x];

					/* skip non perma grids wall */
					if (ag->feat != FEAT_PERM_INNER)
						continue;

					/* Allow "stone to mud" ability */
					if (borg_skill[BI_ASTONE2MUD] >= 1000 ||
						 borg_equips_artifact(ART_DESTINY))
						return (TRUE);

					/* Do not dig unless we appear strong enough to succeed or we
					 * have a digger */
					if ((borg_skill[BI_DIG] > BORG_DIG &&
						  borg_items[weapon_swap].tval == TV_DIGGING) ||
						 (borg_skill[BI_DIG] > BORG_DIG + 20)) {
						/* digging ought to work, proceed */
					} else {
						continue;
					}

					/* Final check on ability */
					if ((borg_skill[BI_DIG] < BORG_DIG &&
						  borg_items[weapon_swap].tval == TV_DIGGING) ||
						 (borg_skill[BI_DIG] < BORG_DIG + 20))
						return (FALSE);

					/* Glove up and dig in */
					return (TRUE);
				}
			}
		}
		/* not adjacent to a GCV,  Restore Grid */
		ag = &borg_grids[y][x];
	}

	/* Explore "rubble" */
	if (ag->feat == FEAT_RUBBLE &&
		 !borg_skill[BI_PASSWALL] /*&& borg_race != RACE_SPECTRE*/) {
		return (TRUE);
	}

	/* Explore "Trees" somewhat
	if (ag->feat == FEAT_TREES) {
		// Scan near trees for unknown grids

		// AJG Do not attempt on the edge
		if (x < AUTO_MAX_X - 1 && y < AUTO_MAX_Y - 1 && x > 1 && y > 1) {
			// scan the adjacent grids
			for (ox = -1; ox <= 1; ox++) {
				for (oy = -1; oy <= 1; oy++) {
					// Acquire location
					ag = &borg_grids[oy + y][ox + x];

					// look for Unknown grid
					if (ag->feat == FEAT_NONE)
						return (TRUE);
				}
			}
		}
		// this forest is already explored
		return (FALSE);
	}
	*/

	/* Explore "closed doors" */
	if ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_TAIL)) {
		/* some closed doors leave alone */
		if ((borg_depth & DEPTH_BREEDER)) {
			/* Did I close this one */
			for (i = 0; i < track_door_num; i++) {
				/* mark as icky if I closed this one */
				if ((track_door_x[i] == x) && (track_door_y[i] == y)) {
					/* not interesting */
					return (FALSE);
				}
			}
		}

		/* Wraith/Passwall guys just walk though them. */
		if (borg_skill[BI_PASSWALL] /*|| borg_race == RACE_SPECTRE*/) {
			/* Did I walk through this one */
			for (i = 0; i < track_door_num; i++) {
				/* skip it if I already walked through it */
				if ((track_door_x[i] == x) && (track_door_y[i] == y)) {
					/* not interesting */
					return (FALSE);
				}
			}
		}

		/* this door should be ok walk through */
		return (TRUE);
	}

	/* Explore "visible traps" */
	if ((ag->feat >= FEAT_TRAP_TRAPDOOR) && (ag->feat <= FEAT_TRAP_SLEEP)) {
		/* Do not disarm when blind */
		if (borg_skill[BI_ISBLIND])
			return (FALSE);

		/* Do not disarm when confused */
		if (borg_skill[BI_ISCONFUSED])
			return (FALSE);

		/* Do not disarm when hallucinating */
		if (borg_skill[BI_ISIMAGE])
			return (FALSE);

		/* Do not flow without lite */
		if (borg_skill[BI_CUR_LITE] == 0)
			return (FALSE);

		/* Do not disarm trap doors on level 99 */
		if (borg_skill[BI_CDEPTH] == 99 && ag->feat == FEAT_TRAP_TRAPDOOR)
			return (FALSE);

		/* Do not disarm when you could end up dead */
		if (borg_skill[BI_CURHP] < 60)
			return (FALSE);

		/* Do not disarm when clumsy */
		if (borg_skill[BI_DIS] < 30 && borg_skill[BI_CLEVEL] < 20)
			return (FALSE);
		if (borg_skill[BI_DIS] < 45 && borg_skill[BI_CLEVEL] < 10)
			return (FALSE);

		/* NOTE: the flow code allows a borg to flow through a trap and so he may
		 * still try to disarm one on his way to the other interesting grid.  If
		 * mods
		 * are made to the above criteria for disarming traps, then mods must also
		 * be
		 * made to borg_flow_spread()
		 */

		/* Okay */
		return (TRUE);
	}

	/* Ignore other grids */
	return (FALSE);
}

/*
 * Determine if a grid is "reachable" (and can be explored)
 */
extern bool borg_flow_dark_reachable(int y, int x) {
	int j;

	borg_grid *ag;

	/* Scan neighbors */
	for (j = 0; j < 8; j++) {
		int y2 = y + ddy_ddd[j];
		int x2 = x + ddx_ddd[j];

		/* Get the grid */
		ag = &borg_grids[y2][x2];

		/* Skip unknown grids (important) */
		if (ag->feat == FEAT_NONE)
			continue;

		/* Accept known floor grids */
		if (borg_cave_floor_grid(ag))
			return (TRUE);

		/* Accept Trees too
		if (ag->feat == FEAT_TREES)
			return (TRUE);
		*/
		/* Accept Lava if immune
		if (ag->feat == FEAT_DEEP_LAVA && borg_skill[BI_IFIRE])
			return (TRUE);

		if (ag->feat == FEAT_SHAL_LAVA &&
			 (borg_skill[BI_IFIRE] || borg_skill[BI_FEATH]))
			return (TRUE);
		*/

		/* Accept Water if not drowning */
		if (ag->feat == FEAT_WATER &&
			 (!borg_skill[BI_ENCUMBERD] || borg_skill[BI_FEATH]))
			return (TRUE);

		/* I can push pass friendly monsters */
		if (ag->kill && borg_kills[ag->kill].ally)
			return (TRUE);
	}

	/* Failure */
	return (FALSE);
}

/* Dig a straight Tunnel to a close monster */
bool borg_flow_kill_direct(/*bool viewable*/) {
	/*int o_y, o_x;
	int m_x, m_y;
	int f_y, f_x;*/
	/*int b_y = 0, b_x = 0;
	int perma_grids = 0;*/
	int i;
	int b_i = -1;
	int d;
	int b_d = MAX_SIGHT;

	borg_kill *kill;

	/* Do not dig when weak. It takes too long */
	if ((borg_skill[BI_DIG] < BORG_DIG &&
		  borg_items[weapon_swap].tval == TV_DIGGING) ||
		 (borg_skill[BI_DIG] < BORG_DIG + 20))
		return (FALSE);

	/* Not if Weak from hunger or no food */
	if ((borg_skill[BI_ISHUNGRY] && !borg_skill[BI_VAMPIRE]) ||
		 borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] == 0)
		return (FALSE);

	/* Only when sitting for too long or twitchy */
	if (borg_t - borg_began < 3000 && borg_times_twitch < 5)
		return (FALSE);

	/* Do not dig when confused */
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Not when darkened */
	if (borg_skill[BI_CUR_LITE] == 0)
		return (FALSE);

	/* Efficiency -- Nothing to kill */
	if (borg_kills_cnt) {
		/* Scan the monsters */
		for (i = 1; i < borg_kills_nxt; i++) {
			kill = &borg_kills[i];

			/* Skip "dead" monsters */
			if (!kill->r_idx)
				continue;

			/* Not if monster is hard to kill */
			if (kill->avoid)
				continue;

			/* Distance away */
			d = kill->dist;

			/* Track closest one */
			if (d > b_d)
				continue;

			/* Track it */
			b_i = i;
			b_d = d;
		}
	}

	/* If no Kill, then pick the center of the map */
	if (b_i == -1) {

		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid */
		borg_flow_enqueue_grid(AUTO_MAX_Y / 2, AUTO_MAX_X / 2);

		/* Spread the flow */
		borg_flow_spread(150, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("center direct", GOAL_KILL, FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_KILL))
			return (FALSE);

		return (TRUE);
	}

	if (b_i) /* don't want it near permawall */
	{
		/* get the closest monster */
		kill = &borg_kills[b_i];

		/* Clear the flow codes */
		borg_flow_clear();

		/* Enqueue the grid */
		borg_flow_enqueue_grid(kill->y, kill->x);

		/* Spread the flow */
		borg_flow_spread(15, TRUE, FALSE, TRUE, -1, FALSE);

		/* Attempt to Commit the flow */
		if (!borg_flow_commit("kill direct", GOAL_KILL, FALSE))
			return (FALSE);

		/* Take one step */
		if (!borg_flow_old(GOAL_KILL))
			return (FALSE);

		return (TRUE);
	}

	return FALSE;
}

/*
 * Place a "direct path" into the flow array, checking danger
 *
 * Modify the "cost" array in such a way that from any point on
 * one "direct" path from the player to the given grid, as long
 * as the rest of the path is "safe" and "clear", the Borg will
 * walk along the path to the given grid.
 *
 * This function is used by "borg_flow_dark_1()" to provide an
 * optimized "flow" during the initial exploration of a level.
 *
 * It is also used to move around town without looking like a drunk.
 */
extern void borg_flow_direct(int y, int x) {
	int n = 0;

	int x1, y1, x2, y2;

	int ay, ax;

	int shift;

	borg_grid *ag;

	/* Avoid icky grids */
	if (borg_data_icky->data[y][x])
		return;

	/* Unknown */
	if (!borg_data_know->data[y][x]) {
		/* Mark as known */
		borg_data_know->data[y][x] = TRUE;

		/* Mark dangerous grids as icky */
		if (borg_danger(y, x, 1, TRUE) > avoidance / 3) {
			/* Icky */
			borg_data_icky->data[y][x] = TRUE;

			/* Avoid */
			return;
		}
	}

	/* Save the flow cost (zero) */
	borg_data_cost->data[y][x] = 0;

	/* Save "origin" */
	y1 = y;
	x1 = x;

	/* Save "destination" */
	y2 = c_y;
	x2 = c_x;

	/* Calculate distance components */
	ay = (y2 < y1) ? (y1 - y2) : (y2 - y1);
	ax = (x2 < x1) ? (x1 - x2) : (x2 - x1);

	/* Path */
	while (1) {
		/* Check for arrival at player */
		if ((x == x2) && (y == y2))
			return;

		/* Next */
		n++;

		/* Move mostly vertically */
		if (ay > ax) {
			/* Extract a shift factor XXX */
			shift = (n * ax + (ay - 1) / 2) / ay;

			/* Sometimes move along the minor axis */
			x = (x2 < x1) ? (x1 - shift) : (x1 + shift);

			/* Always move along major axis */
			y = (y2 < y1) ? (y1 - n) : (y1 + n);
		}

		/* Move mostly horizontally */
		else {
			/* Extract a shift factor XXX */
			shift = (n * ay + (ax - 1) / 2) / ax;

			/* Sometimes move along the minor axis */
			y = (y2 < y1) ? (y1 - shift) : (y1 + shift);

			/* Always move along major axis */
			x = (x2 < x1) ? (x1 - n) : (x1 + n);
		}

		/* Access the grid */
		ag = &borg_grids[y][x];

		/* Floors are ok */

		/* Trees are ok */

		/* Some things are not ok, Mountains not ok
		if (ag->feat == FEAT_MOUNTAIN)
			return;
		if (ag->feat == FEAT_DARK_PIT && !borg_skill[BI_FEATH])
			return;
		*/

		/* Ignore certain "non-wall" grids */
		if ((ag->feat == FEAT_WATER &&
			  (borg_skill[BI_ENCUMBERD] && !borg_skill[BI_FEATH])))
			return;
		/*
		if (ag->feat == FEAT_DEEP_LAVA && !borg_skill[BI_IFIRE])
			return;
		if (ag->feat == FEAT_SHAL_LAVA &&
			 (!borg_skill[BI_FEATH] && !borg_skill[BI_IFIRE]))
			return;
		*/

		/* Certain types of Walls */
		if (ag->feat >= FEAT_WALL_EXTRA && ag->feat <= FEAT_WALL_SOLID)
			return;

		/* Certain types of diggable Walls */
		if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) {
			if (borg_skill[BI_ASTONE2MUD] <= 0)
				if (borg_items[weapon_swap].tval != TV_DIGGING &&
					 borg_skill[BI_DIG] < BORG_DIG)
					if (borg_skill[BI_DIG] < BORG_DIG + 20)
						return;
		}

		/* Certain types of Perm walls */
		if (ag->feat >= FEAT_PERM_EXTRA && ag->feat <= FEAT_PERM_SOLID)
			return;

		/* Abort at "icky" grids */
		if (borg_data_icky->data[y][x])
			return;

		/* Analyze every grid once */
		if (!borg_data_know->data[y][x]) {
			/* Mark as known */
			borg_data_know->data[y][x] = TRUE;

			/* Avoid dangerous grids (forever) */
			if (borg_danger(y, x, 1, TRUE) > avoidance / 3) {
				/* Mark as icky */
				borg_data_icky->data[y][x] = TRUE;

				/* Abort */
				return;
			}
		}

		/* Abort "pointless" paths if possible */
		if (borg_data_cost->data[y][x] <= n)
			break;

		/* Save the new flow cost */
		borg_data_cost->data[y][x] = n;
	}
}

/*
 * Prepare to flow towards a vault grid which can be excavated
 */
bool borg_flow_vault(int nearness) {
	int y, x, i;
	int b_y, b_x;

	borg_grid *ag;

	/* reset counters */
	borg_temp_n = 0;
	i = 0;

	/* no need if no vault on level */
	if (!(borg_depth & DEPTH_VAULT))
		return (FALSE);

	/* build the array -- Scan screen */
	for (y = w_y; y < w_y + SCREEN_HGT; y++) {
		for (x = w_x; x < w_x + SCREEN_WID; x++) {

			/* only bother with near ones */
			if (distance(c_y, c_x, y, x) > nearness)
				continue;

			/* only deal with excavatable walls */
			if (borg_grids[y][x].feat < FEAT_MAGMA ||
				 borg_grids[y][x].feat > FEAT_PERM_EXTRA)
				continue;

			/* Examine grids adjacent to this grid to see if there is a perma wall
			 * adjacent */
			for (i = 0; i < 8; i++) {
				b_x = x + ddx_ddd[i];
				b_y = y + ddy_ddd[i];

				/* Bounds check */
				if (!in_bounds2(b_y, b_x))
					continue;

				/* Access the grid */
				ag = &borg_grids[b_y][b_x];

				/* Not a perma, and not our spot. */
				if (ag->feat != FEAT_PERM_INNER)
					continue;

				/* keep count */
				borg_temp_y[borg_temp_n] = y;
				borg_temp_x[borg_temp_n] = x;
				borg_temp_n++;
			}
		}
	}

	/* None to flow to */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Examine each one */
	for (i = 0; i < borg_temp_n; i++) {
		/* Enqueue the grid */
		borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
	}

	/* Spread the flow.  Note the use of tunneling  TRUE */
	borg_flow_spread(250, TRUE, FALSE, TRUE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("vault excavation", GOAL_VAULT, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_VAULT))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/* Excavate an existing vault using ranged spells.
 * Stand where you are, use stone to mud to excavate the vault.  This will allow
 * the mage
 * borgs to get a few more attack spells on the monster.  Without this routine,
 * he would
 * approach the vault and use Stone to Mud when he was adjacent to the wall,
 * giving him
 * only 1 or 2 shots before the monster is next to the borg.
 *
 * A problem arises when the borg will melt an interior wall to a vault and free
 * the imprisoned
 * monster.  The monster then has LOS on the borg but he will not know that he
 * has LOS on
 * the monster since the now-melted wall is dark and considered a FEAT_INVIS
 * grid.  The borg either
 * needs to illuminate the area or be allowed to attack over a FEAT_INVIS grid
 * in a vault.
 */
bool borg_excavate_vault(int range) {
	int y, x, i, ii;
	int b_y, b_x /*b_n*/;

	borg_grid *ag;

	/* reset counters */
	borg_temp_n = 0;
	i = 0;
	ii = 0;
	/*b_n = 0;*/

	/* no need if no vault on level */
	if (!(borg_depth & DEPTH_VAULT))
		return (FALSE);

	/* only if you can cast the spell */
	if (!borg_spell_okay_fail(REALM_ARCANE, 2, 4, 40) &&
		 !borg_spell_okay_fail(REALM_CHAOS, 0, 6, 40) &&
		 !borg_spell_okay_fail(REALM_NATURE, 1, 0, 40) &&
		 !borg_racial_check(RACE_HALF_GIANT, 10) &&
		 !borg_equips_activation(ACT_STONE_MUD, TRUE))
		return (FALSE);

	/* Danger/bad idea checks */

	/* build the array -- Scan screen */
	for (y = w_y; y < w_y + SCREEN_HGT; y++) {
		for (x = w_x; x < w_x + SCREEN_WID; x++) {

			/* only bother with near ones */
			if (distance(c_y, c_x, y, x) > range)
				continue;

			/* only deal with excavatable walls */
			if (borg_grids[y][x].feat < FEAT_MAGMA ||
				 borg_grids[y][x].feat > FEAT_PERM_EXTRA)
				continue;

			/* Examine grids adjacent to this grid to see if there is a perma wall
			 * adjacent */
			for (i = 0; i < 8; i++) {
				b_x = x + ddx_ddd[i];
				b_y = y + ddy_ddd[i];

				/* Bounds check */
				if (!in_bounds2(b_y, b_x))
					continue;

				ag = &borg_grids[b_y][b_x];

				/* Not a perma, and not our spot. */
				if (ag->feat != FEAT_PERM_INNER)
					continue;

				/* Make sure they are in my plane */
				if (b_y != c_y || b_x != c_x)
					continue;

				/* Track the new grid */
				for (ii = 0; ii < borg_temp_n; ii++) {
					if (borg_temp_y[ii] == y && borg_temp_x[ii] == x)
						break;
				}

				/* Track the newly discovered excavatable wall */
				if ((ii == borg_temp_n) && (ii < AUTO_TEMP_MAX)) {
					borg_temp_x[borg_temp_n] = x;
					borg_temp_y[borg_temp_n] = y;
					borg_temp_n++;

					/* do not overflow */
					if (borg_temp_n > AUTO_TEMP_MAX)
						borg_temp_n = AUTO_TEMP_MAX;
				}
			}
		}
	}

	/* None to excavate */
	if (!borg_temp_n)
		return (FALSE);

	/* Review the useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		/* skip non-projectable grids grid (I cant shoot them) */
		if (!borg_projectable(borg_temp_y[i], borg_temp_x[i], c_y, c_x, TRUE,
									 TRUE))
			continue;

		/* Attempt to target the grid */
		borg_target(borg_temp_y[i], borg_temp_x[i]);

		/* Attempt to excavate it with "stone to mud" */
		if (borg_spell(REALM_ARCANE, 2, 4) || borg_spell(REALM_CHAOS, 0, 6) ||
			 borg_spell(REALM_NATURE, 1, 0) || borg_racial(RACE_HALF_GIANT, 1) ||
			 borg_activate_activation(ACT_STONE_MUD, FALSE)) {
			borg_note("# Excavation of vault");
			borg_keypress('5');

			/* turn that wall into a floor grid.  If the spell failed, it will
			 * still look
			 * like a wall and the borg_update routine will redefine it as a wall
			 */
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			/* Not Lit */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].info &= ~BORG_GLOW;
			/* Dark */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].info |= BORG_GLOW;
			/* Feat Floor */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].feat = FEAT_FLOOR;

			return (TRUE);
		}

		/* Success */
		return (TRUE);
	}

	/* No grid to excavate */
	return (FALSE);
}

/* Excavate a 3x3 region around the borg using ranged spells.
 * Stand where you are, use stone to mud to excavate the area.
 *
 * This is used to clear an area so he can built a sea of runes.
 * Borg will have to illuminate the excavated grid.
 */
bool borg_excavate_region(int nearness) {
	/*int y, x, i, ii;*/
	int i;
	/*int b_y, b_x, b_n;*/
	int y1, x1, floors = 0;
	int glyphs = 0;
	int min_floors = 0;
	/*int j;*/
	int range = nearness;

	borg_grid *ag;

	/* reset counters */
	borg_temp_n = 0;
	i = 0;
	/*ii = 0;*/
	/*b_n = 0;*/

	/* Number of floor grids needed based on radius */
	switch (nearness) {
	case 1:
		min_floors = 9;
		break;
	case 2:
		min_floors = 25;
		break;
	case 3:
		min_floors = 49;
		break;
	}

	/* Not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* only if you can cast the spell */
	if (!borg_spell_okay_fail(REALM_ARCANE, 2, 4, 40) &&
		 !borg_spell_okay_fail(REALM_CHAOS, 0, 6, 40) &&
		 !borg_spell_okay_fail(REALM_NATURE, 1, 0, 40) &&
		 !borg_racial_check(RACE_HALF_GIANT, 10) &&
		 !borg_equips_activation(ACT_STONE_MUD, TRUE))
		return (FALSE);

	/* Don't allow for illegal grids */
	if (nearness > borg_wall_buffer)
		nearness = borg_wall_buffer - 2;

	/* Must be in a fairly central region */
	if (c_y <= borg_wall_buffer || c_y >= AUTO_MAX_Y - borg_wall_buffer ||
		 c_x <= borg_wall_buffer * 3 || c_x >= AUTO_MAX_X - borg_wall_buffer * 3)
		return (FALSE);

	/* Scan neighbors for something that can be excavated.  But scan from the
	 * inside-out */
	for (range = 1; range <= nearness; range++) {
		for (y1 = c_y - range; y1 < c_y + range; y1++) {
			for (x1 = c_x - range; x1 < c_x + range; x1++) {
				/* Get the grid */
				ag = &borg_grids[y1][x1];

				/* Count the good grids that we can work with */
				if (ag->feat == FEAT_GLYPH || ag->feat == FEAT_NONE ||
					 ag->feat == FEAT_FLOOR || ag->feat == FEAT_INVIS ||
					 (ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_WALL_SOLID))
					floors++;

				/* Count the number of glyphs currently on the ground, used later */
				if (ag->feat == FEAT_GLYPH)
					glyphs++;
			}
		}
	}

	/* Must have a minimal amount to create effect */
	if (floors < min_floors)
		return (FALSE);

	/* build the array -- Scan screen */
	/* Scan neighbors for something that can be excavated.  But scan from the
	 * inside-out */
	for (range = 1; range <= nearness; range++) {
		for (y1 = c_y - range; y1 <= c_y + range; y1++) {
			for (x1 = c_x - range; x1 <= c_x + range; x1++) {
				/* Get the grid */
				ag = &borg_grids[y1][x1];

				/* only deal with excavatable walls */
				if (ag->feat == FEAT_NONE ||
					 (ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_WALL_SOLID) ||
					 (/*ag->feat == FEAT_FLOOR && */ ag->info & BORG_DARK)) {
					/* Should be ok */
				} else {
					/* Shouldnt excavate this */
					continue;
				}

				/* skip non-projectable grids grid (I cant shoot them) */
				if (!(ag->info & BORG_VIEW))
					continue;

				/* Skips ones with monsters on them */
				if (ag->kill)
					continue;

				/* Track the newly discovered excavatable wall */
				borg_temp_x[borg_temp_n] = x1;
				borg_temp_y[borg_temp_n] = y1;
				borg_temp_n++;

				/* do not overflow */
				if (borg_temp_n > AUTO_TEMP_MAX)
					borg_temp_n = AUTO_TEMP_MAX;
			}
		}
	}

	/* None to excavate */
	if (!borg_temp_n)
		return (FALSE);

	/* Review the useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		/* Attempt to target the grid */
		borg_target(borg_temp_y[i], borg_temp_x[i]);

		/* Only grids that are tagged at dark (no lite, no glow). */
		if ((borg_grids[borg_temp_y[i]][borg_temp_x[i]].info & BORG_DARK)) {
			/* Ray of Light */
			if (borg_spell_fail(REALM_NATURE, 1, 4, 20) ||
				 borg_spell_fail(REALM_ARCANE, 2, 5, 20) ||
				 borg_zap_rod(SV_ROD_LITE) ||
				 borg_activate_activation(ACT_SUNLIGHT, FALSE) ||
				 borg_aim_wand(SV_WAND_LITE)) {
				/* apply the direction */
				borg_keypress('5');
				borg_note("# Illuminating the empty grid");
				borg_do_update_view = TRUE;
				borg_do_update_lite = TRUE;
				return (TRUE);
			}

			/* Cast illumination */
			if (borg_activate_artifact(ART_GALADRIEL, FALSE) ||
				 borg_zap_rod(SV_ROD_ILLUMINATION) ||
				 borg_use_staff(SV_STAFF_LITE) ||
				 borg_read_scroll(SV_SCROLL_LIGHT) ||
				 borg_spell(REALM_ARCANE, 0, 5) || borg_spell(REALM_CHAOS, 0, 2) ||
				 borg_spell(REALM_NATURE, 0, 4) ||
				 borg_spell(REALM_SORCERY, 0, 3) || borg_spell(REALM_LIFE, 0, 4) ||
				 borg_mutation(COR1_ILLUMINE, FALSE, 50, FALSE) ||
				 borg_activate_activation(ACT_MAP_LIGHT, FALSE)) {
				when_call_lite = borg_t;
				borg_do_update_view = TRUE;
				borg_do_update_lite = TRUE;
				return (TRUE);
			}

		}

		/* Attempt to excavate the whole region with "Call Void" */
		else if (glyphs <= 0 && borg_temp_n >= 10 && range < 4 &&
					(borg_spell(REALM_CHAOS, 3, 7) ||
					 borg_spell(REALM_NATURE, 3, 7))) {
			borg_note("# Excavation of region");

			/* turn that wall into a floor grid.  If the spell failed, it will
			 * still look
			 * like a wall and the borg_update routine will redefine it as a wall
			 */
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;
			borg_needs_new_sea = TRUE;
			glyph_x_center = c_x;
			glyph_y_center = c_y;

			return (TRUE);
		}
		/* Attempt to excavate it with "stone to mud" */
		else if (borg_spell(REALM_ARCANE, 2, 4) ||
					borg_spell(REALM_CHAOS, 0, 6) ||
					borg_spell(REALM_NATURE, 1, 0) ||
					borg_racial(RACE_HALF_GIANT, 1) ||
					borg_activate_activation(ACT_STONE_MUD, FALSE)) {
			borg_note("# Excavation of area");
			borg_keypress('5');

			/* turn that wall into a floor grid.  If the spell failed, it will
			 * still look
			 * like a wall and the borg_update routine will redefine it as a wall
			 */
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			/* define at Not Lit */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].info &= ~BORG_GLOW;
			/* define as Dark */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].info |= BORG_GLOW;
			/* define as Feat Floor */
			borg_grids[borg_temp_y[i]][borg_temp_x[i]].feat = FEAT_FLOOR;

			/* If digging, then i may need to make a new sea */
			if (distance(glyph_y_center, glyph_x_center, c_y, c_x) >= 10) {
				glyph_x_center = c_x;
				glyph_y_center = c_y;
			}

			return (TRUE);
		}

		/* Success */
		return (TRUE);
	}

	/* No grid to excavate */
	return (FALSE);
}

/* Currently not used, I thought I might need it for anti-summoning */
extern void borg_flow_direct_dig(int y, int x) {
	int n = 0;

	int x1, y1, x2, y2;

	int ay, ax;

	int shift;

/*borg_grid *ag;*/

#if 0
    /* Avoid icky grids */
    if (borg_data_icky->data[y][x]) return;

    /* Unknown */
    if (!borg_data_know->data[y][x])
    {
        /* Mark as known */
        borg_data_know->data[y][x] = TRUE;

        /* Mark dangerous grids as icky */
        if (borg_danger(y, x, 1, TRUE) > avoidance / 3)
        {
            /* Icky */
            borg_data_icky->data[y][x] = TRUE;

            /* Avoid */
            return;
        }
    }

#endif

	/* Save the flow cost (zero) */
	borg_data_cost->data[y][x] = 0;

	/* Save "origin" */
	y1 = y;
	x1 = x;

	/* Save "destination" */
	y2 = c_y;
	x2 = c_x;

	/* Calculate distance components */
	ay = (y2 < y1) ? (y1 - y2) : (y2 - y1);
	ax = (x2 < x1) ? (x1 - x2) : (x2 - x1);

	/* Path */
	while (1) {
		/* Check for arrival at player */
		if ((x == x2) && (y == y2))
			return;

		/* Next */
		n++;

		/* Move mostly vertically */
		if (ay > ax) {
			/* Extract a shift factor XXX */
			shift = (n * ax + (ay - 1) / 2) / ay;

			/* Sometimes move along the minor axis */
			x = (x2 < x1) ? (x1 - shift) : (x1 + shift);

			/* Always move along major axis */
			y = (y2 < y1) ? (y1 - n) : (y1 + n);
		}

		/* Move mostly horizontally */
		else {
			/* Extract a shift factor XXX */
			shift = (n * ay + (ax - 1) / 2) / ax;

			/* Sometimes move along the minor axis */
			y = (y2 < y1) ? (y1 - shift) : (y1 + shift);

			/* Always move along major axis */
			x = (x2 < x1) ? (x1 - n) : (x1 + n);
		}

		/* Access the grid */
		/*ag = &borg_grids[y][x];*/

		/* Abort at "icky" grids */
		if (borg_data_icky->data[y][x])
			return;

		/* Analyze every grid once */
		if (!borg_data_know->data[y][x]) {
			/* Mark as known */
			borg_data_know->data[y][x] = TRUE;

			/* Avoid dangerous grids (forever) */
			if (borg_danger(y, x, 1, TRUE) > avoidance / 3) {
				/* Mark as icky */
				borg_data_icky->data[y][x] = TRUE;

				/* Abort */
				return;
			}
		}

		/* Abort "pointless" paths if possible */
		if (borg_data_cost->data[y][x] <= n)
			break;

		/* Save the new flow cost */
		borg_data_cost->data[y][x] = n;
	}
}

/*
 * Be concerned about leaving Passwall or Wraith Form while being in a wall.
 * The walls will crush the borg.  He has a couple of options.
 * 1.  Cast a spell and reacquire the wraithform
 * 2.  Exit the wall to an adjacent non-wall grid
 * 3.  Cast Earthquake to make empty floor grids
 * 4.  Use Stone to Mud to make an empty floor grid
 * 5.  Attempt to dig a tunnel and excavate a floor grid.
 * 6.  Cast Phase/Teleport/TPLevel/Alter Reality
 * 7.  Cast Recall and hope that I make it out on time.
 */
bool borg_flow_passwall(void) {
	int i;
	int y, x;
	int b_i = -1;
	borg_grid *ag;
	int p;
	int b_p = 2000;
	int dir;
	/*int g_y, g_x;*/

	/* Verify Wraithform is off */
	if (borg_skill[BI_PASSWALL])
		return (FALSE);

	/* Verify that I am stuck in a wall */
	if (borg_grids[c_y][c_x].feat < FEAT_DOOR_HEAD ||
		 borg_grids[c_y][c_x].feat > FEAT_PERM_SOLID)
		return (FALSE);

	/* 1.  Cast a spell and reacquire the wraithform  */
	if (borg_spell(REALM_DEATH, 3, 7) ||
		 borg_activate_activation(ACT_WRAITH, FALSE))
		return (TRUE);

	/* 2.  Exit the wall to an adjacent non-wall grid  */
	for (i = 0; i < 8; i++) {
		/* Look around */
		y = c_y + borg_ddy_ddd[i];
		x = c_x + borg_ddx_ddd[i];

		if (!in_bounds(y, x))
			continue;

		ag = &borg_grids[y][x];

		/* Cant walk into these grids */
		if (ag->feat > FEAT_MORE && ag->feat <= FEAT_PERM_SOLID)
			continue;

		/* check the danger of that grid */
		p = borg_danger(y, x, 1, TRUE);

		/* Track it */
		if (p < b_p) {
			b_p = p;
			b_i = i;
			dir = borg_extract_dir(c_y, c_x, y, x);
		}
	}

	/* Non-Wall found.  Exit that way */
	if (b_i != -1) {
		/* exit the wall */
		borg_keypress(I2D(dir));

		borg_note("# Exiting wall.");
		return (TRUE);
	}

	/* 3.  Cast Earthquake to make empty floor grids  */
	if (borg_spell(REALM_NATURE, 3, 0) ||
		 borg_activate_activation(ACT_QUAKE, FALSE) ||
		 borg_mutation(COR1_EARTHQUAKE, FALSE, 100, TRUE) ||
		 borg_spell(REALM_CHAOS, 3, 7))
		return (TRUE);

	/* 4.  Use Stone to Mud to make an empty floor grid  */
	for (i = 0; i < 8; i++) {
		/* Look around */
		y = c_y + borg_ddy_ddd[i];
		x = c_x + borg_ddx_ddd[i];

		if (!in_bounds(y, x))
			continue;

		ag = &borg_grids[y][x];

		/* Cant tunnel these grids */
		if (ag->feat >= FEAT_PERM_EXTRA && ag->feat <= FEAT_PERM_SOLID)
			continue;

		/* Limit to only excavatable walls */
		if (ag->feat < FEAT_SECRET || ag->feat > FEAT_WALL_SOLID)
			continue;

		/* Track it */
		b_i = i;
		dir = borg_extract_dir(c_y, c_x, y, x);
	}

	/* Good-Wall found.  Excavate that way */
	if (b_i != -1) {
		/* excavate the wall */
		if (borg_aim_wand(SV_WAND_STONE_TO_MUD) ||
			 borg_spell(REALM_ARCANE, 2, 4) || borg_spell(REALM_NATURE, 1, 0) ||
			 borg_spell(REALM_CHAOS, 0, 6) ||
			 borg_mutation(COR1_EAT_ROCK, FALSE, 40, FALSE) ||
			 borg_racial(RACE_HALF_GIANT, 1) ||
			 borg_activate_artifact(ART_DESTINY, INVEN_WIELD) ||
			 borg_activate_activation(ACT_STONE_MUD, FALSE)) {
			borg_keypress(I2D(dir));
			borg_note("# Excavating wall.");
			return (TRUE);
		}

		/* 5.  Attempt to dig a tunnel and excavate a floor grid.  */
		borg_note("# Digging through wall/etc");
		borg_keypress('0');
		borg_keypress('9');
		borg_keypress('9');
		borg_keypress('T');
		borg_keypress(I2D(dir));
		return (TRUE);
	}

	/* 6.  Cast Phase/Teleport/TPLevel/Alter Reality  */
	if (borg_read_scroll(SV_SCROLL_PHASE_DOOR) ||
		 borg_spell_fail(REALM_ARCANE, 2, 3, 100) ||
		 borg_spell_fail(REALM_TRUMP, 0, 4, 100) ||
		 borg_spell_fail(REALM_CHAOS, 0, 7, 100) ||
		 borg_spell_fail(REALM_SORCERY, 0, 5, 100) ||
		 borg_mindcr_fail(MIND_MAJOR_DISP, 7, 100) ||
		 borg_mutation(COR1_VTELEPORT, FALSE, 100, FALSE) ||
		 borg_swap_position(avoidance) ||
		 borg_use_staff_fail(SV_STAFF_TELEPORTATION) ||
		 borg_activate_artifact(ART_NYNAULD, FALSE) ||
		 borg_activate_activation(ACT_TELEPORT, FALSE) ||
		 borg_read_scroll(SV_SCROLL_TELEPORT) || borg_activate_planar(100) ||
		 borg_racial(RACE_GNOME, 1) || borg_spell(REALM_ARCANE, 0, 4) ||
		 borg_spell(REALM_SORCERY, 0, 1) || borg_spell(REALM_TRUMP, 0, 0) ||
		 borg_mindcr(MIND_MINOR_DISP, 3) ||
		 borg_mutation(COR1_BLINK, FALSE, 100, FALSE) ||
		 borg_activate_artifact(ART_ANDROMALIUS, FALSE) ||
		 borg_activate_artifact(ART_DRAEBOR, FALSE))
		return (TRUE);

	/* 7.  Cast Recall and hope that I make it out on time.  */
	if (borg_recall())
		return (TRUE);

	/* Nothing to do */
	return (FALSE);
}

/*
 * Hack -- mark off the edges of a rectangle as "avoid" or "clear"
 */
static void borg_flow_border(int y1, int x1, int y2, int x2, bool stop) {
	int x, y;

	/* Scan west/east edges */
	for (y = y1; y <= y2; y++) {
		/* Avoid/Clear west edge */
		borg_data_know->data[y][x1] = stop;
		borg_data_icky->data[y][x1] = stop;

		/* Avoid/Clear east edge */
		borg_data_know->data[y][x2] = stop;
		borg_data_icky->data[y][x2] = stop;
	}

	/* Scan north/south edges */
	for (x = x1; x <= x2; x++) {
		/* Avoid/Clear north edge */
		borg_data_know->data[y1][x] = stop;
		borg_data_icky->data[y1][x] = stop;

		/* Avoid/Clear south edge */
		borg_data_know->data[y2][x] = stop;
		borg_data_icky->data[y2][x] = stop;
	}
}

/*
 * Prepare to "flow" towards "interesting" grids (method 1)
 *
 * This function examines the torch-lit grids for "interesting" grids.
 */
static bool borg_flow_dark_1(int b_stair) {
	int i;

	int x, y;
	int cost;
	int brave = 3;

	/* Some classes and races are very weak and need to stay close to the stairs
	 */
	if (borg_skill[BI_CLEVEL] < 20 &&
		 (borg_skill[BI_NO_MELEE] || borg_race == RACE_SPECTRE ||
		  borg_class == CLASS_HIGH_MAGE || borg_skill[BI_REALM1] == REALM_LIFE))
		brave = 1;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Reset */
	borg_temp_n = 0;

	/* Scan torch-lit grids */
	for (i = 0; i < borg_lite_n; i++) {
		y = borg_lite_y[i];
		x = borg_lite_x[i];

		/* Skip "boring" grids (assume reachable) */
		if (!borg_flow_dark_interesting(y, x))
			continue;

		/* Clear the flow codes */
		borg_flow_clear();

		/* obtain the number of steps from this take to the stairs */
		cost = borg_flow_cost_stair(y, x, b_stair);

		/* Check the distance to stair for this proposed grid if dangerous */
		if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 &&
			 cost > borg_skill[BI_CLEVEL] * brave + 9 &&
			 borg_skill[BI_CLEVEL] < 20)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Create paths to useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		y = borg_temp_y[i];
		x = borg_temp_x[i];

		/* Create a path */
		borg_flow_direct(y, x);
	}

	/* Attempt to Commit the flow */
	if (!borg_flow_commit(NULL, GOAL_DARK, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_DARK))
		return (FALSE);

	/* Forget goal */
	goal = 0;

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards "interesting" grids (method 2)
 *
 * This function is only used when the player is at least 4 grids away
 * from the outer dungeon wall, to prevent any nasty memory errors.
 *
 * This function examines the grids just outside the torch-lit grids
 * for "unknown" grids, and flows directly towards them (one step).
 */
static bool borg_flow_dark_2(int b_stair) {
	int i, r;
	int cost;

	int x, y;

	borg_grid *ag;

	int brave = 3;

	/* Some classes and races are very weak and need to stay close to the stairs
	 */
	if (borg_skill[BI_CLEVEL] < 20 &&
		 (borg_skill[BI_NO_MELEE] || borg_race == RACE_SPECTRE ||
		  borg_class == CLASS_HIGH_MAGE || borg_skill[BI_REALM1] == REALM_LIFE))
		brave = 1;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Set the searching flag for low level borgs */
	borg_needs_searching = TRUE;

	/* Maximal radius */
	r = borg_skill[BI_CUR_LITE] + 1;

	/* Reset */
	borg_temp_n = 0;

	/* Four directions */
	for (i = 0; i < 4; i++) {
		y = c_y + ddy_ddd[i] * r;
		x = c_x + ddx_ddd[i] * r;

		/* Check legality */
		if (y < 1)
			continue;
		if (x < 1)
			continue;
		if (y > AUTO_MAX_Y - 2)
			continue;
		if (x > AUTO_MAX_X - 2)
			continue;

		/* Acquire grid */
		ag = &borg_grids[y][x];

		/* Require unknown */
		if (ag->feat != FEAT_NONE)
			continue;

		/* Require viewable */
		if (!(ag->info & BORG_VIEW))
			continue;

		/* if it makes me wander, skip it */
		/* Clear the flow codes */
		borg_flow_clear();

		/* obtain the number of steps from this take to the stairs */
		cost = borg_flow_cost_stair(y, x, b_stair);

		/* Check the distance to stair for this proposed grid */
		if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 &&
			 cost > borg_skill[BI_CLEVEL] * brave + 9 &&
			 borg_skill[BI_CLEVEL] < 20)
			continue;

		/* Careful -- Remember it */
		borg_temp_x[borg_temp_n] = x;
		borg_temp_y[borg_temp_n] = y;
		borg_temp_n++;
	}

	/* Nothing */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Create paths to useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		y = borg_temp_y[i];
		x = borg_temp_x[i];

		/* Create a path */
		borg_flow_direct(y, x);
	}

	/* Attempt to Commit the flow */
	if (!borg_flow_commit(NULL, GOAL_DARK, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_DARK))
		return (FALSE);

	/* Forget goal */
	goal = 0;

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards "interesting" grids (method 3)
 *
 * Note the use of a limit on the "depth" of the flow, and of the flag
 * which avoids "unknown" grids when calculating the flow, both of which
 * help optimize this function to only handle "easily reachable" grids.
 *
 * The "borg_temp" array is much larger than any "local region".
 */
static bool borg_flow_dark_3(int b_stair) {
	int i;

	int x, y;

	int x1, y1, x2, y2;

	int brave = 3;

	/* Some classes and races are very weak and need to stay close to the stairs
	 */
	if (borg_skill[BI_CLEVEL] < 20 &&
		 (borg_skill[BI_NO_MELEE] || borg_race == RACE_SPECTRE ||
		  borg_class == CLASS_HIGH_MAGE || borg_skill[BI_REALM1] == REALM_LIFE))
		brave = 1;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Local region */
	y1 = c_y - 4;
	x1 = c_x - 4;
	y2 = c_y + 4;
	x2 = c_x + 4;

	/* Restrict to "legal" grids */
	if (y1 < 1)
		y1 = 1;
	if (x1 < 1)
		x1 = 1;
	if (y2 > AUTO_MAX_Y - 2)
		y2 = AUTO_MAX_Y - 2;
	if (x2 > AUTO_MAX_X - 2)
		x2 = AUTO_MAX_X - 2;

	/* Reset */
	borg_temp_n = 0;

	/* Examine the region */
	for (y = y1; y <= y2; y++) {
		/* Examine the region */
		for (x = x1; x <= x2; x++) {
			/* Skip "boring" grids */
			if (!borg_flow_dark_interesting(y, x))
				continue;

			/* Skip "unreachable" grids */
			if (!borg_flow_dark_reachable(y, x))
				continue;

			/* Clear the flow codes */
			borg_flow_clear();

			/* Check the distance to stair for this proposed grid */
			if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 &&
				 borg_skill[BI_CLEVEL] <= 20 &&
				 borg_flow_cost_stair(y, x, b_stair) >
					  borg_skill[BI_CLEVEL] * brave + 9)
				continue;

			/* Careful -- Remember it */
			borg_temp_x[borg_temp_n] = x;
			borg_temp_y[borg_temp_n] = y;
			borg_temp_n++;
		}
	}

	/* Nothing interesting */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		y = borg_temp_y[i];
		x = borg_temp_x[i];

		/* Enqueue the grid */
		borg_flow_enqueue_grid(y, x);
	}

	/* Spread the flow (limit depth- leash) */
	borg_flow_spread(5, TRUE, TRUE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit("Dark-3", GOAL_DARK, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_DARK))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards "interesting" grids (method 4)
 *
 * Note that we avoid grids close to the edge of the panel, since they
 * induce panel scrolling, which is "expensive" in terms of CPU usage,
 * and because this allows us to "expand" the border by several grids
 * to lay down the "avoidance" border in known legal grids.
 *
 * We avoid paths that would take us into different panels by setting
 * the "icky" flag for the "border" grids to prevent path construction,
 * and then clearing them when done, to prevent confusion elsewhere.
 *
 * The "borg_temp" array is large enough to hold one panel full of grids.
 */
static bool borg_flow_dark_4(int b_stair) {
	int i, x, y;
	int leash = 250;
	int x1, y1, x2, y2;
	int brave = 3;

	/* Some classes and races are very weak and need to stay close to the stairs
	 */
	if (borg_skill[BI_CLEVEL] < 20 &&
		 (borg_skill[BI_NO_MELEE] || borg_race == RACE_SPECTRE ||
		  borg_class == CLASS_HIGH_MAGE || borg_skill[BI_REALM1] == REALM_LIFE))
		brave = 1;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Local region */
	y1 = c_y - 11;
	x1 = c_x - 11;
	y2 = c_y + 11;
	x2 = c_x + 11;

	/* Restrict to "legal" grids */
	if (y1 < 1)
		y1 = 1;
	if (x1 < 1)
		x1 = 1;
	if (y2 > AUTO_MAX_Y - 2)
		y2 = AUTO_MAX_Y - 2;
	if (x2 > AUTO_MAX_X - 2)
		x2 = AUTO_MAX_X - 2;

	/* Nothing yet */
	borg_temp_n = 0;

	/* check the leash length */
	if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5)
		leash = borg_skill[BI_CLEVEL] * brave + 9;

	/* Examine the panel */
	for (y = y1; y <= y2; y++) {
		/* Examine the panel */
		for (x = x1; x <= x2; x++) {
			/* Skip "boring" grids */
			if (!borg_flow_dark_interesting(y, x))
				continue;

			/* Skip "unreachable" grids */
			if (!borg_flow_dark_reachable(y, x))
				continue;

			/* Clear the flow codes */
			borg_flow_clear();

			/* Check the distance to stair for this proposed grid */
			if (borg_flow_cost_stair(y, x, b_stair) > leash &&
				 borg_skill[BI_CLEVEL] < 20)
				continue;

			/* Careful -- Remember it */
			borg_temp_x[borg_temp_n] = x;
			borg_temp_y[borg_temp_n] = y;
			borg_temp_n++;
		}
	}

	/* Nothing useful */
	if (!borg_temp_n)
		return (FALSE);

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		y = borg_temp_y[i];
		x = borg_temp_x[i];

		/* Enqueue the grid */
		borg_flow_enqueue_grid(y, x);
	}

	/* Expand borders */
	y1--;
	x1--;
	y2++;
	x2++;

	/* Avoid the edges */
	borg_flow_border(y1, x1, y2, x2, TRUE);

	/* Spread the flow (limit depth Leash) */
	if (borg_skill[BI_CLEVEL] < 15) {
		/* Short Leash */
		borg_flow_spread(leash, TRUE, TRUE, FALSE, -1, FALSE);
	} else {
		/* Long Leash */
		borg_flow_spread(250, TRUE, TRUE, FALSE, -1, FALSE);
	}

	/* Clear the edges */
	borg_flow_border(y1, x1, y2, x2, FALSE);

	/* Attempt to Commit the flow */
	if (!borg_flow_commit(NULL, GOAL_DARK, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_DARK))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards "interesting" grids (method 5)
 */
static bool borg_flow_dark_5(int b_stair) {
	int i, x, y;
	int leash = 250;
	int brave = 3;

	/* Some classes and races are very weak and need to stay close to the stairs
	 */
	if (borg_skill[BI_CLEVEL] < 20 &&
		 (borg_skill[BI_NO_MELEE] || borg_race == RACE_SPECTRE ||
		  borg_class == CLASS_HIGH_MAGE || borg_skill[BI_REALM1] == REALM_LIFE))
		brave = 1;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Nothing yet */
	borg_temp_n = 0;

	/* check the leash length */
	if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 &&
		 borg_skill[BI_CLEVEL] < 20)
		leash = borg_skill[BI_CLEVEL] * brave + 9;

	/* Examine every "legal" grid */
	for (y = 1; y < AUTO_MAX_Y - 1; y++) {
		for (x = 1; x < AUTO_MAX_X - 1; x++) {
			/* Skip "boring" grids */
			if (!borg_flow_dark_interesting(y, x))
				continue;

			/* Skip "unreachable" grids */
			if (!borg_flow_dark_reachable(y, x))
				continue;

			/* Clear the flow codes */
			borg_flow_clear();

			/* Check the distance to stair for this proposed grid */
			if (borg_flow_cost_stair(y, x, b_stair) > leash &&
				 borg_skill[BI_CLEVEL] < 20)
				continue;

			/* Careful -- Remember it */
			borg_temp_x[borg_temp_n] = x;
			borg_temp_y[borg_temp_n] = y;
			borg_temp_n++;

			/* Paranoia -- Check for overflow */
			if (borg_temp_n == AUTO_TEMP_MAX) {
				/* Hack -- Double break */
				y = AUTO_MAX_Y;
				x = AUTO_MAX_X;
				break;
			}
		}
	}

	/* Nothing useful */
	if (!borg_temp_n)
		return (FALSE);

	/* Wipe icky codes from grids if needed */
	if (goal_ignoring || (borg_depth & DEPTH_SCARY))
		borg_danger_wipe = TRUE;

	/* Clear the flow codes */
	borg_flow_clear();

	/* Enqueue useful grids */
	for (i = 0; i < borg_temp_n; i++) {
		y = borg_temp_y[i];
		x = borg_temp_x[i];

		/* Enqueue the grid */
		borg_flow_enqueue_grid(y, x);
	}

	/* Spread the flow */
	if (borg_skill[BI_CLEVEL] < 5) {
		/* Short Leash */
		borg_flow_spread(15, FALSE, TRUE, FALSE, -1, FALSE);
	} else if (borg_skill[BI_CLEVEL] >= 5 && borg_skill[BI_CLEVEL] <= 15) {
		/* Short Leash */
		borg_flow_spread(leash, FALSE, TRUE, FALSE, -1, FALSE);
	} else {
		/* Long Leash */
		borg_flow_spread(250, FALSE, TRUE, FALSE, -1, FALSE);
	}

	/* Attempt to Commit the flow */
	if (!borg_flow_commit(NULL, GOAL_DARK, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_DARK))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Prepare to "flow" towards "interesting" grids
 *
 * The "exploration" routines are broken into "near" and "far"
 * exploration, and each set is chosen via the flag below.
 */
bool borg_flow_dark(bool neer) {
	int i;
	int x, y, j, b_j = -1;
	int b_stair = -1;

	/* Paranoia */
	if (borg_flow_dark_interesting(c_y, c_x)) {
		return (FALSE);
	}

	/* Check distance away from stairs, used later */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* Near */
	if (neer) {
		/* Method 1 */
		if (borg_flow_dark_1(b_stair))
			return (TRUE);

		/* Method 2 */
		if (borg_flow_dark_2(b_stair))
			return (TRUE);

		/* Method 3 */
		if (borg_flow_dark_3(b_stair))
			return (TRUE);
	}

	/* Far */
	else {
		/* Method 4 */
		if (borg_flow_dark_4(b_stair))
			return (TRUE);

		/* Method 5 */
		if (borg_flow_dark_5(b_stair))
			return (TRUE);
	}

	/* Fail */
	return (FALSE);
}

/*
 * Hack -- spastic searching
 */

static byte spastic_x;
static byte spastic_y;

/*
 * Search carefully for secret doors and such
 */
bool borg_flow_spastic(bool bored) {
	int cost;

	int i, x, y, v;

	int b_x = c_x;
	int b_y = c_y;
	int b_v = -1;
	int j, b_j = -1;
	int b_stair = -1;
	/*char reason;*/

	borg_grid *ag;

	/* Hack -- not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Not bored */
	if (!bored) {
		/* Look around for danger */
		int p = borg_danger(c_y, c_x, 1, TRUE);

		/* Avoid searching when in danger */
		if (p > avoidance / 4)
			return (FALSE);
	}

	/* Check distance away from stairs, used later */
	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* skip the closer ones */
		if (b_j >= j)
			continue;

		/* track it */
		b_j = j;
		b_stair = i;
	}

	/* We have arrived */
	if ((spastic_x == c_x) && (spastic_y == c_y)) {
		/* Cancel */
		spastic_x = 0;
		spastic_y = 0;

		/* Take note */
		borg_note(format("# Spastic Searching at (%d,%d)...value %d", c_x, c_y,
							  borg_grids[c_y][c_x].xtra));

		/* Count searching */
		for (i = 0; i < 9; i++) {
			/* Extract the location */
			int xx = c_x + ddx_ddd[i];
			int yy = c_y + ddy_ddd[i];

			/* Current grid */
			ag = &borg_grids[yy][xx];

			/* Tweak -- Remember the search */
			if (ag->xtra < 100)
				ag->xtra += 5;
		}

		/* Tweak -- Search a little */
		borg_keypress('0');
		borg_keypress('5');
		borg_keypress('s');

		/* Success */
		return (TRUE);
	}

	/* Reverse flow */
	borg_flow_reverse();

	/* Scan the entire map */
	for (y = 1; y < AUTO_MAX_Y - 1; y++) {
		for (x = 1; x < AUTO_MAX_X - 1; x++) {
			borg_grid *ag_ptr[8];

			int wall = 0;
			int supp = 0;
			int diag = 0;

			/* Acquire the grid */
			ag = &borg_grids[y][x];

			/* Skip unknown grids */
			if (ag->feat == FEAT_NONE)
				continue;

			/* Skip walls/doors */
			if (!borg_cave_floor_grid(ag))
				continue;

			/* Acquire the cost */
			cost = borg_data_cost->data[y][x];

			/* Skip "unreachable" grids */
			if (cost >= 250)
				continue;

			/* Tweak -- Limit total searches */
			if (ag->xtra >= 50)
				continue;
			if (ag->xtra >= borg_skill[BI_CLEVEL])
				continue;
			if (borg_t > 1000 && (track_less_num || track_more_num))
				continue;

			/* Limit initial searches until bored */
			if (!bored && (ag->xtra > 5))
				continue;

			/* Avoid searching detected sectors */
			if (borg_detect_door[y / 11][x / 33])
				continue;

			/* Skip ones that make me wander too far (Leash) */
			if (b_stair != -1 && borg_skill[BI_CLEVEL] < 15) {
				/* Check the distance of this grid to the stair */
				j = distance(track_less_y[b_stair], track_less_x[b_stair], y, x);
				/* Distance of me to the stairs */
				b_j = distance(c_y, c_x, track_less_y[b_stair],
									track_less_x[b_stair]);

				/* skip far away grids while I am close to stair*/
				if (b_j <= borg_skill[BI_CLEVEL] * 3 + 9 &&
					 j >= borg_skill[BI_CLEVEL] * 3 + 9)
					continue;

				/* If really low level don't do this much */
				if (borg_skill[BI_CLEVEL] <= 3 &&
					 b_j <= borg_skill[BI_CLEVEL] + 9 &&
					 j >= borg_skill[BI_CLEVEL] + 9)
					continue;

				/* Do not Venture too far from stair */
				if (borg_skill[BI_CLEVEL] <= 3 && j >= borg_skill[BI_CLEVEL] + 5)
					continue;

				/* Do not Venture too far from stair */
				if (borg_skill[BI_CLEVEL] <= 10 && j >= borg_skill[BI_CLEVEL] + 9)
					continue;
			}

			/* Extract adjacent locations */
			for (i = 0; i < 8; i++) {
				/* Extract the location */
				int xx = x + ddx_ddd[i];
				int yy = y + ddy_ddd[i];

				/* Get the grid contents */
				ag_ptr[i] = &borg_grids[yy][xx];
			}

			/* Count possible door locations */
			for (i = 0; i < 4; i++) {
				ag = ag_ptr[i];
				if (ag->feat >= FEAT_WALL_EXTRA)
					wall++;
			}

			/* No possible secret doors */
			if (wall < 1)
				continue;

			/* Count supporting evidence for secret doors */
			for (i = 0; i < 4; i++) {
				ag = ag_ptr[i];

				/* Rubble */
				if (ag->feat == FEAT_RUBBLE)
					continue;

				/* Walls, Doors */
				if (((ag->feat >= FEAT_SECRET) && (ag->feat <= FEAT_PERM_SOLID)) ||
					 ((ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN)) ||
					 ((ag->feat >= FEAT_DOOR_HEAD) &&
					  (ag->feat <= FEAT_DOOR_TAIL))) {
					supp++;
				}
			}

			/* Count supporting evidence for secret doors */
			for (i = 4; i < 8; i++) {
				ag = ag_ptr[i];

				/* Rubble */
				if (ag->feat == FEAT_RUBBLE)
					continue;

				/* Walls */
				if (ag->feat >= FEAT_SECRET) {
					diag++;
				}
			}

			/* No possible secret doors */
			if (diag < 2)
				continue;

			/* Tweak -- Reward walls, punish visitation, distance, time on level */
			v = (supp * 500) + (diag * 100) - (ag->xtra * 40) - (cost * 2) -
				 (borg_t - borg_began);

			/* Punish low level and searching too much */
			v -= (50 - borg_skill[BI_CLEVEL]) * 5;

			/* The grid is not searchable */
			if (v <= 0)
				continue;

			/* Tweak -- Minimal interest until bored */
			if (!bored && (v < 1500))
				continue;

			/* Track "best" grid */
			if ((b_v >= 0) && (v < b_v))
				continue;

			/* Save the data */
			b_v = v;
			b_x = x;
			b_y = y;
		}
	}

	/* Clear the flow codes */
	borg_flow_clear();

	/* Hack -- Nothing found */
	if (b_v < 0)
		return (FALSE);

	/* Access grid */
	ag = &borg_grids[b_y][b_x];

	/* Memorize */
	spastic_x = b_x;
	spastic_y = b_y;

	/* Enqueue the grid */
	borg_flow_enqueue_grid(b_y, b_x);

	/* Spread the flow */
	borg_flow_spread(250, FALSE, FALSE, FALSE, -1, FALSE);

	/* Attempt to Commit the flow */
	if (bored && !borg_flow_commit("bored spastic", GOAL_XTRA, FALSE))
		return (FALSE);
	if (!bored && !borg_flow_commit("spastic", GOAL_XTRA, FALSE))
		return (FALSE);

	/* Take one step */
	if (!borg_flow_old(GOAL_XTRA))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
* Initialize this file
*/
void borg_init_6(void) { /* Nothing */ }

#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
