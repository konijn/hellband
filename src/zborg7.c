/* File: borg7.c */
/* Purpose: High level functions for the Borg -BEN- */

#include "angband.h"

#ifdef ALLOW_BORG

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg4.h"
#include "zborg5.h"
#include "zborg6.h"
#include "zborg7.h"

/*
 * This file handles various high level inventory related goals.
 *
 * Problems:
 *   Use "time stamps" (and not "random" checks) for several routines,
 *   including "kill junk" and "wear stuff", and maybe even "free space".
 *   But be careful with the "free space" routine, wear stuff first.
 *   Make sure nothing is "destroyed" if we do not do them every turn.
 *   Consider some special routines in stores (and in the home).
 *
 * Hack -- We should perhaps consider wearing "harmless" items into empty
 * slots when in the dungeon, to allow rings/amulets to be brought back up
 * to town to be sold.
 *
 * We should take account of possible combinations of equipment.  This may
 * be a potentially expensive computation, but could be done occasionally.
 * It is important to use a "state-less" formula to allow the exchange to
 * be spread over multiple turns.
 *
 * Hack -- We should attempt to only collect non-discounted items, at least
 * for the "expensive" slots, such as books, since we do not want to lose
 * value due to stacking.  We seem to sell the non-discounted items first,
 * and to buy the discounted items first, since they are cheap.  Oh well,
 * we may just be stuck with using discounted books.  Unless we actually
 * do correct "combining" in the simulations, and reward free slots.  Ick!
 *
 * XXX XXX XXX We really need a better "twitchy" function.
 *
 * XXX XXX XXX We need a better "flee this level" function
 *
 * XXX XXX XXX We need to stockpile possible useful items at home.
 *
 * XXX XXX XXX Perhaps we could simply maintain a list of abilities
 * that we might need at some point, such as the ability to identify, and
 * simply allow the Borg to "sell" items to the home which satisfy this
 * desire for "abilities".
 *
 * XXX XXX XXX Also, we should probably attempt to track the "best" item
 * in the home for each equipment slot, using some form of heuristic, and
 * reward that item based on its power, so that the Borg would always
 * have a "backup" item in case of disenchantment.
 *
 * XXX XXX XXX Also, we could reward equipment based on possible enchantment,
 * up to the maximal amount available in the home, which would induce item
 * switching when the item could be enchanted sufficiently.
 *
 * Fleeing from fast spell-casters is probably not a very smart idea, nor is
 * fleeing from fast monsters, nor is attempting to clear a room full of fast
 * moving breeding monsters, such as lice.
 */

/*
 * Hack -- importance of the various "level feelings"
 * Try to explore the level for at least this many turns
TODO SHOULD this is a great idea, we should look in to this!!
We might have to cheat a little, and reverse look up from the core
static s16b value_feeling[] = {500,  8000, 8000, 6000, 4000, 2000, 1000, 800,
600,  400,  200,  0};
*/

/*
 * Determine if an item is "probably" worthless
 *
 * This (very heuristic) function is a total hack, designed only to prevent
 * a very specific annoying situation described below.
 *
 * Note that a "cautious" priest (or low level mage/ranger) will leave town
 * with a few identify scrolls, wander around dungeon level 1 for a few turns,
 * and use all of the scrolls on leather gloves and broken daggers, and must
 * then return to town for more scrolls.  This may repeat indefinitely.
 *
 * The problem is that some characters (priests, mages, rangers) never get an
 * "average" feeling about items, and have no way to keep track of how long
 * they have been holding a given item for, so they cannot even attempt to
 * gain knowledge from the lack of "good" or "cursed" feelings.  But they
 * cannot afford to just identify everything they find by using scrolls of
 * identify, because, in general, some items are, on average, "icky", and
 * not even worth the price of a new scroll of identify.
 *
 * Even worse, the current algorithm refuses to sell un-identified items, so
 * the poor character will throw out all his good stuff to make room for crap.
 *
 * This function simply examines the item and assumes that certain items are
 * "icky", which is probably a total hack.  Perhaps we could do something like
 * compare the item to the item we are currently wearing, or perhaps we could
 * analyze the expected value of the item, or guess at the likelihood that the
 * item might be a blessed, or something.
 *
 */
bool borg_item_icky(borg_item *item) {
	int slot;

	/* if its average, dump it if you want to.*/
	if (strstr(item->note, "average"))
		return (TRUE);

	/* if Mindcrafter and not psych yet */
	if (!borg_mindcr_legal(MIND_PSYCHOMETRY, 15))
		return (TRUE);

	/* Mega-Hack -- allow "icky" items */
	if (borg_class == CLASS_PRIEST || borg_class == CLASS_RANGER ||
		 borg_class == CLASS_MAGE || borg_skill[BI_CLEVEL] < 20) {
		/* things that are good/excelent/special */
		if (strstr(item->note, "special") || strstr(item->note, "terrible") ||
			 strstr(item->note, "indestructible") || strstr(item->note, "Quest") ||
			 strstr(item->note, "excellent"))
			/* not icky */
			return (FALSE);

		/* Broken dagger/sword, Filthy rag */
		if (((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_DAGGER)) ||
			 ((item->tval == TV_SWORD) && (item->sval == SV_BROKEN_SWORD)) ||
			 ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_FILTHY_RAG))) {
			return (TRUE);
		}

		/* Dagger */
		if ((item->tval == TV_SWORD) && (item->sval == SV_DAGGER)) {
			return (TRUE);
		}

		/* Sling (and I already got one) */
		if ((item->tval == TV_BOW) && (item->sval == SV_SLING) &&
			 borg_items[INVEN_BOW].tval == TV_BOW) {
			return (TRUE);
		}

		/* Cloak, (and I already got one)*/
		if ((item->tval == TV_CLOAK) && (item->sval == SV_CLOAK) &&
			 borg_items[INVEN_OUTER].tval == TV_CLOAK) {
			return (TRUE);
		}

		/* Robe (and I already got one)*/
		if ((item->tval == TV_SOFT_ARMOR) && (item->sval == SV_ROBE) &&
			 borg_items[INVEN_BODY].tval >= TV_SOFT_ARMOR) {
			return (TRUE);
		}

		/* Leather Gloves (and I already got one)*/
		if ((item->tval == TV_GLOVES) &&
			 (item->sval == SV_SET_OF_LEATHER_GLOVES) &&
			 borg_items[INVEN_HANDS].tval == TV_GLOVES) {
			return (TRUE);
		}

		/* Assume the item is not icky */
		return (FALSE);
	}

	/* Process other classes which do get pseudo ID */
	/* things that are good/excelent/special/no P-ID */
	if (strstr(item->note, "special") || strstr(item->note, "terrible") ||
		 strstr(item->note, "excellent") || strstr(item->note, "Quest") ||
		 strstr(item->note, "indestructible") ||
		 item->note == NULL) /* no pseudo-id yet */
		/* not icky */
		return (FALSE);

	/*** {Good} items in inven, But I have {excellent} in equip ***/

	if (strstr(item->note, "good")) {
		/* Obtain the slot of the suspect item */
		slot = borg_wield_slot(item);

		/* safety check incase slot = -1 */
		if (slot < 0)
			return (FALSE);

		/* Obtain my equipped item in the slot */
		item = &borg_items[slot];

		/* Is my item an ego or artifact? */
		if (item->name2 || item->name1)
			return (TRUE);
	}
	/* Assume not icky, I should have extra ID for the item */
	return (FALSE);
}

/*
 * Use things in a useful, but non-essential, manner
 */
bool borg_use_things(void) {
	int i;

	/* Quaff experience restoration potion */
	if (borg_skill[BI_ISFIXEXP] &&
		 (borg_spell(REALM_LIFE, 3, 3) || borg_spell(REALM_DEATH, 1, 7) ||
		  borg_activate_artifact(ART_LIFE, FALSE) ||
		  borg_quaff_potion(SV_POTION_RESTORE_EXP))) {
		return (TRUE);
	}

	/* just drink the stat gains, at this dlevel we wont need cash */
	if (borg_quaff_potion(SV_POTION_INC_STR) ||
		 borg_quaff_potion(SV_POTION_INC_INT) ||
		 borg_quaff_potion(SV_POTION_INC_WIS) ||
		 borg_quaff_potion(SV_POTION_INC_DEX) ||
		 borg_quaff_potion(SV_POTION_INC_CON) ||
		 borg_quaff_potion(SV_POTION_INC_CHR)) {
		return (TRUE);
	}

	/* Quaff potions of "restore" stat if needed */
	if ((borg_skill[BI_ISFIXSTR] && (borg_quaff_potion(SV_POTION_RES_STR) ||
												borg_quaff_potion(SV_POTION_INC_STR) ||
												borg_eat_food(SV_FOOD_RESTORE_STR) ||
												borg_eat_food(SV_FOOD_RESTORING))) ||
		 (borg_skill[BI_ISFIXINT] && (borg_quaff_potion(SV_POTION_RES_INT) ||
												borg_quaff_potion(SV_POTION_INC_INT) ||
												borg_eat_food(SV_FOOD_RESTORING))) ||
		 (borg_skill[BI_ISFIXWIS] && (borg_quaff_potion(SV_POTION_RES_WIS) ||
												borg_quaff_potion(SV_POTION_INC_WIS) ||
												borg_eat_food(SV_FOOD_RESTORING))) ||
		 (borg_skill[BI_ISFIXDEX] && (borg_quaff_potion(SV_POTION_RES_DEX) ||
												borg_quaff_potion(SV_POTION_INC_DEX) ||
												borg_eat_food(SV_FOOD_RESTORING))) ||
		 (borg_skill[BI_ISFIXCON] && (borg_quaff_potion(SV_POTION_RES_CON) ||
												borg_quaff_potion(SV_POTION_INC_CON) ||
												borg_eat_food(SV_FOOD_RESTORE_CON) ||
												borg_eat_food(SV_FOOD_RESTORING))) ||
		 ((borg_skill[BI_ISFIXCHR]) && (borg_quaff_potion(SV_POTION_RES_CHR) ||
												  borg_quaff_potion(SV_POTION_INC_CHR) ||
												  borg_eat_food(SV_FOOD_RESTORING)))) {
		return (TRUE);
	}

	/* Use some items right away */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;
		if (!item->kind)
			continue;

		/* Process "force" items */
		switch (item->tval) {
		case TV_POTION: {
			/* Check the scroll */
			switch (item->sval) {
			case SV_POTION_ENLIGHTENMENT:

				/* Never quaff these in town */
				if (!borg_skill[BI_CDEPTH])
					break;

			case SV_POTION_AUGMENTATION:
			case SV_POTION_EXPERIENCE:

				/* Try quaffing the potion */
				if (borg_quaff_potion(item->sval))
					return (TRUE);
			}

			break;
		}
		case TV_SCROLL: {
			/* Hack -- check Blind/Confused */
			if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
				break;

			/* XXX XXX XXX Dark */

			/* Check the scroll */
			switch (item->sval) {
			case SV_SCROLL_MAPPING:
			case SV_SCROLL_DETECT_TRAP:
			case SV_SCROLL_DETECT_DOOR:
			case SV_SCROLL_ACQUIREMENT:
			case SV_SCROLL_STAR_ACQUIREMENT:
			case SV_SCROLL_ARTIFACT: {
				/* Never read these in town */
				if (!borg_skill[BI_CDEPTH])
					break;

				/* Try reading the scroll */
				if (borg_read_scroll(item->sval))
					return (TRUE);
				break;
			}
			}

			break;
		}
		}
	}

	/* Eat food */
	if (borg_skill[BI_ISHUNGRY]) {
		/* Attempt to satisfy hunger */
		if (borg_spell(REALM_LIFE, 0, 7) || borg_spell(REALM_ARCANE, 2, 6) ||
			 borg_spell(REALM_NATURE, 0, 3) || borg_eat_food(SV_FOOD_SLIME_MOLD) ||
			 borg_eat_food(SV_FOOD_BISCUIT) || borg_eat_food(SV_FOOD_JERKY) ||
			 borg_eat_food(SV_FOOD_WAYBREAD) ||
			 borg_activate_activation(ACT_SATIATE, FALSE) ||
			 borg_eat_food(SV_FOOD_RATION)) {
			return (TRUE);
		}
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Refuel, call lite, detect traps/doors/walls/evil, etc
 *
 * Note that we refuel whenever our lite starts to get low.
 *
 * Note that we detect traps/doors/walls/evil at least once in each
 * panel, as soon as possible after entering a new panel.
 *
 * Note that we call lite whenever the current grid is dark, and
 * all the grids touching the current grid diagonally are known
 * floors, which catches all rooms, including "checkerboard" rooms,
 * and only occasionally calls lite in corridors, and then only once.
 *
 * Note that we also sometimes call lite whenever we are using a
 * lantern or artifact lite, and when all of the grids in the box
 * of grids containing the maximal torch-lit region (the 5x5 or 7x7
 * region centered at the player) are non-glowing floor grids, and
 * when at least one of them is known to be "dark".  This catches
 * most of the "probable rooms" before the standard check succeeds.
 *
 * We use the special "SELF" messages to "borg_react()" to delay the
 * processing of "detection" and "call lite" until we know if it has
 * worked or not.
 *
 * The matching function borg_check_lite_only is used only with resting
 * to heal.  I don't want him teleporting into a room, resting to heal while
 * there is a dragon sitting in a dark corner waiting to breathe on him.
 * So now he will check for lite.
 *
 */
bool borg_check_lite(void) {
	int i, x, y;
	int corners, floors;

	int q_x, q_y;

	borg_grid *ag;

	bool do_lite;

	bool do_trap;
	bool do_door;
	bool do_wall;
	bool do_evil;

	bool do_lite_aux = FALSE;

	/* Never in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Never when comprimised, save your mana */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
		 borg_skill[BI_ISIMAGE] || borg_skill[BI_ISPOISONED] ||
		 borg_skill[BI_ISCUT] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* XXX XXX XXX Dark */

	/* Extract the panel */
	q_x = w_x / 33;
	q_y = w_y / 11;

	/* Start */
	do_trap = FALSE;

	/* Determine if we need to detect traps */
	if (!borg_detect_trap[q_y + 0][q_x + 0])
		do_trap = TRUE;
	if (!borg_detect_trap[q_y + 0][q_x + 1])
		do_trap = TRUE;
	if (!borg_detect_trap[q_y + 1][q_x + 0])
		do_trap = TRUE;
	if (!borg_detect_trap[q_y + 1][q_x + 1])
		do_trap = TRUE;

	/* Hack -- check traps every few turns anyway */
	if (!when_detect_traps ||
		 (borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 9 / 10 &&
		  borg_t - when_detect_traps >= 183))
		do_trap = TRUE;

	/* Start */
	do_door = FALSE;

	/* Determine if we need to detect doors */
	if (!borg_detect_door[q_y + 0][q_x + 0])
		do_door = TRUE;
	if (!borg_detect_door[q_y + 0][q_x + 1])
		do_door = TRUE;
	if (!borg_detect_door[q_y + 1][q_x + 0])
		do_door = TRUE;
	if (!borg_detect_door[q_y + 1][q_x + 1])
		do_door = TRUE;

	/* Hack -- check doors every few turns anyway */
	if (!when_detect_doors ||
		 (borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 9 / 10 &&
		  borg_t - when_detect_doors >= 731))
		do_door = TRUE;

	/* Start */
	do_wall = FALSE;

	/* Determine if we need to detect walls */
	if (!borg_detect_wall[q_y + 0][q_x + 0])
		do_wall = TRUE;
	if (!borg_detect_wall[q_y + 0][q_x + 1])
		do_wall = TRUE;
	if (!borg_detect_wall[q_y + 1][q_x + 0])
		do_wall = TRUE;
	if (!borg_detect_wall[q_y + 1][q_x + 1])
		do_wall = TRUE;

	/* Hack -- check walls every few turns anyway */
	if (!when_detect_walls ||
		 (borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 9 / 10 &&
		  borg_t - when_detect_walls >= 937))
		do_wall = TRUE;

	/* Start */
	do_evil = FALSE;

	/* Determine if we need to detect evil */
	if (!borg_detect_evil[q_y + 0][q_x + 0])
		do_evil = TRUE;
	if (!borg_detect_evil[q_y + 0][q_x + 1])
		do_evil = TRUE;
	if (!borg_detect_evil[q_y + 1][q_x + 0])
		do_evil = TRUE;
	if (!borg_detect_evil[q_y + 1][q_x + 1])
		do_evil = TRUE;

	/* Hack -- check evil every few turns anyway- more fq if low level or
	 * passwalling */
	if (!when_detect_evil ||
		 (borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 9 / 10 &&
		  borg_t - when_detect_evil >= 75 - (20 - borg_skill[BI_MAXCLEVEL])))
		do_evil = TRUE;

	if (borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_MAGMA &&
		 borg_grids[c_y][c_x].feat <= FEAT_WALL_SOLID &&
		 (borg_t - when_detect_evil >= 8))
		do_evil = TRUE;

	/* Dont bother if I have ESP */
	if (borg_skill[BI_ESP] || borg_esp)
		do_evil = FALSE;

	/* Do not do these if monsters near.  Save mana */
	if (!borg_check_rest(c_y, c_x) &&
		 (!borg_skill[BI_PASSWALL] ||
		  (borg_skill[BI_PASSWALL] && (borg_grids[c_y][c_x].feat < FEAT_MAGMA ||
			borg_grids[c_y][c_x].feat > FEAT_WALL_SOLID)))) {
		do_trap = FALSE;
		do_door = FALSE;
		do_wall = FALSE;
		do_evil = FALSE;
	}

	/* Dont detect evil in a sea of runes, it causes the borg to wait too long in
	 * the sea.
	 * problem with kill->when and the monster's aaf.
	 */
	if ((borg_position & (POSITION_SEA | POSITION_BORE)))
		do_evil = FALSE;

	/*** Do Things ***/

	/* Hack -- find traps and doors evil and walls */
	if ((do_trap || do_door || do_wall) && do_evil) {
		/* Check for walls, traps and doors and evil*/
		if (borg_mindcr(MIND_PRECOGNIT, 20) ||
			 borg_spell_fail(REALM_NATURE, 1, 2, 20)) {
			borg_note("# Checking for traps, doors, evil and walls.");

			borg_react("SELF:WTDE", "SELF:WTDE");

			when_detect_traps = borg_t;
			when_detect_doors = borg_t;
			when_detect_evil = borg_t;
			when_detect_walls = borg_t;
			if (borg_class == CLASS_ORPHIC && borg_skill[BI_CLEVEL] >= 45)
				when_wizard_lite = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;
			return (TRUE);
		}
	}

	/* Hack -- find traps and doors and evil*/
	if ((do_trap || do_door) && do_evil) {
		/* Check for traps and doors and evil*/
		if (borg_activate_artifact(ART_ATAL, FALSE) ||
			 borg_zap_rod(SV_ROD_DETECTION) || borg_mindcr(MIND_PRECOGNIT, 5) ||
			 borg_spell(REALM_SORCERY, 1, 6) ||
			 borg_spell_fail(REALM_ARCANE, 3, 5, 20) ||
			 borg_activate_activation(ACT_DETECT_ALL, FALSE)) {
			borg_note("# Checking for traps, doors, and evil.");

			borg_react("SELF:TDE", "SELF:TDE");

			when_detect_traps = borg_t;
			when_detect_doors = borg_t;
			when_detect_evil = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- find evil */
	if (do_evil) {
		/* Check for evil */
		if (borg_spell_fail(REALM_LIFE, 0, 0, 20) ||
			 borg_spell_fail(REALM_ARCANE, 0, 3, 20) ||
			 borg_spell_fail(REALM_NATURE, 0, 0, 20) ||
			 borg_spell_fail(REALM_SORCERY, 0, 0, 20) ||
			 borg_spell_fail(REALM_DEATH, 0, 2, 20) ||
			 borg_mindcr(MIND_PRECOGNIT, 1) ||
			 borg_spell_fail(REALM_TRUMP, 0, 6, 25) ||
			 borg_mutation(COR1_SMELL_MON, FALSE, 20, FALSE)) {
			borg_note("# Checking for monsters.");

			borg_react("SELF:evil", "SELF:evil");

			when_detect_evil = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- find traps and doors */
	if ((do_trap || do_door) &&
		 ((!when_detect_traps || (borg_t - when_detect_traps >= 15)) ||
		  (!when_detect_doors || (borg_t - when_detect_doors >= 15)))) {
		/* Check for traps and doors */
		if (borg_activate_artifact(ART_SKULLKEEPER, FALSE) ||
			 borg_activate_activation(ACT_DETECT_ALL, FALSE) ||
			 borg_spell_fail(REALM_LIFE, 0, 5, 20) ||
			 borg_spell_fail(REALM_SORCERY, 0, 2, 20) ||
			 borg_spell_fail(REALM_ARCANE, 1, 0, 20) ||
			 borg_racial(RACE_DWARF, 1)) {
			borg_note("# Checking for traps and doors.");

			borg_react("SELF:both", "SELF:both");

			when_detect_traps = borg_t;
			when_detect_doors = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- find traps */
	if (do_trap && (!when_detect_traps || (borg_t - when_detect_traps >= 7))) {
		/* Check for traps */
		if (/*borg_activate_artifact(ART_HOLHENNETH, INVEN_HEAD) ||*/
			 borg_read_scroll(SV_SCROLL_DETECT_TRAP) ||
			 borg_use_staff(SV_STAFF_DETECT_TRAP) ||
			 borg_zap_rod(SV_ROD_DETECT_TRAP)) {
			borg_note("# Checking for traps.");

			borg_react("SELF:trap", "SELF:trap");

			when_detect_traps = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- find doors */
	if (do_door && (!when_detect_doors || (borg_t - when_detect_doors >= 9))) {
		/* Check for traps */
		if (/*borg_activate_artifact(ART_HOLHENNETH, INVEN_HEAD) ||*/
			 borg_read_scroll(SV_SCROLL_DETECT_DOOR) ||
			 borg_use_staff(SV_STAFF_DETECT_DOOR) ||
			 borg_zap_rod(SV_ROD_DETECT_DOOR)) {
			borg_note("# Checking for doors.");

			borg_react("SELF:door", "SELF:door");

			when_detect_doors = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- find walls */
	if (do_wall && (!when_detect_walls || (borg_t - when_detect_walls >= 15))) {
		/* Check for walls */
		if (borg_activate_artifact(ART_EOS, FALSE) ||
			 borg_read_scroll(SV_SCROLL_MAPPING) ||
			 borg_use_staff(SV_STAFF_MAPPING) || borg_zap_rod(SV_ROD_MAPPING) ||
			 borg_spell(REALM_SORCERY, 1, 0) || borg_mindcr(MIND_PRECOGNIT, 20) ||
			 borg_activate_activation(ACT_MAP_LIGHT, FALSE)) {
			borg_note("# Checking for walls.");

			borg_react("SELF:wall", "SELF:wall");

			when_detect_walls = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Start */
	do_lite = FALSE;

	/* Get central grid */
	ag = &borg_grids[c_y][c_x];

	corners = 0;
	floors = 0;

	/* Scan diagonal neighbors */
	for (i = 4; i < 8; i++) {
		/* Get location */
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Location must be known */
		if (ag->feat == FEAT_NONE)
			corners++;

		/* Location must not be a wall/door */
		if (!borg_cave_floor_grid(ag))
			corners++;
	}
	/* Add them up */
	if (corners <= 2)
		do_lite = TRUE;

	/* Hack */
	if (do_lite && (borg_skill[BI_CUR_LITE] >= 2) &&
		 (c_x >= borg_skill[BI_CUR_LITE]) &&
		 (c_x < AUTO_MAX_X - borg_skill[BI_CUR_LITE]) &&
		 (c_y >= borg_skill[BI_CUR_LITE]) &&
		 (c_y < AUTO_MAX_Y - borg_skill[BI_CUR_LITE]) && (rand_int(100) < 90)) {

		floors = 0;
		/* Scan the "local" grids (5x5) 2 same as torch grid*/
		for (y = c_y - 2; y <= c_y + 2; y++) {
			/* Scan the "local" grids (5x5) */
			for (x = c_x - 2; x <= c_x + 2; x++) {
				/* Get grid */
				ag = &borg_grids[y][x];

				/* Location must be a lit floor */
				if (ag->info & BORG_LITE)
					floors++;

				/* Location must not be glowing */
				if (ag->info & BORG_GLOW && cave[y][x].info)
					floors--;

				/* Location must not be a wall/door */
				if (!borg_cave_floor_grid(ag))
					floors--;
			}
		}
	}
	/* add them up */
	if (floors <= 11)
		do_lite = do_lite_aux = FALSE;

	/* Hack -- call lite */
	if (do_lite && (!when_call_lite || (borg_t - when_call_lite >= 7))) {
		/* Call light */
		if (borg_activate_artifact(ART_BEATRICE, FALSE) ||
			 borg_zap_rod(SV_ROD_ILLUMINATION) || borg_use_staff(SV_STAFF_LITE) ||
			 borg_read_scroll(SV_SCROLL_LIGHT) ||
			 borg_mutation(COR1_ILLUMINE, FALSE, 50, FALSE) ||
			 borg_spell(REALM_ARCANE, 0, 5) || borg_spell(REALM_CHAOS, 0, 2) ||
			 borg_spell(REALM_NATURE, 0, 4) || borg_spell(REALM_SORCERY, 0, 3) ||
			 borg_spell(REALM_LIFE, 0, 4)) {
			borg_note("# Illuminating the room");

			borg_react("SELF:lite", "SELF:lite");

			when_call_lite = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Hack -- Wizard Lite */
	if (TRUE && (!when_wizard_lite || (borg_t - when_wizard_lite >= 1000))) {
		/* Wizard lite */
		if (borg_activate_artifact(ART_HIPPO, FALSE) ||
			 borg_spell(REALM_ARCANE, 3, 7) || borg_spell(REALM_NATURE, 3, 5) ||
			 borg_mindcr(MIND_PRECOGNIT, 45)) {
			borg_note("# Illuminating the dungeon");

			/* borg_react("SELF:wizard lite", "SELF:wizard lite"); */

			when_wizard_lite = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* Oops */
	return (FALSE);
}
bool borg_check_lite_only(void) {
	int i, x, y;
	int corners, floors;

	/*int q_x, q_y;*/

	borg_grid *ag;

	bool do_lite;

	bool do_lite_aux = FALSE;

	/* Never in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Never when blind or hallucinating */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISIMAGE])
		return (FALSE);

	/* XXX XXX XXX Dark */

	/* Extract the panel
	q_x = w_x / 33;
	q_y = w_y / 11;
	*/

	/* Start */
	do_lite = FALSE;

	/* Get central grid */
	ag = &borg_grids[c_y][c_x];

	corners = 0;
	floors = 0;

	/* Scan diagonal neighbors */
	for (i = 4; i < 8; i++) {
		/* Get location */
		x = c_x + ddx_ddd[i];
		y = c_y + ddy_ddd[i];

		/* Get grid */
		ag = &borg_grids[y][x];

		/* Location must be known */
		if (ag->feat == FEAT_NONE)
			corners++;

		/* Location must not be a wall/door */
		if (!borg_cave_floor_grid(ag))
			corners++;
	}
	/* Add them up ..2*/
	if (corners <= 2)
		do_lite = TRUE;

	/* Hack */
	if (do_lite && (borg_skill[BI_CUR_LITE] >= 2) &&
		 (c_x >= borg_skill[BI_CUR_LITE]) &&
		 (c_x < AUTO_MAX_X - borg_skill[BI_CUR_LITE]) &&
		 (c_y >= borg_skill[BI_CUR_LITE]) &&
		 (c_y < AUTO_MAX_Y - borg_skill[BI_CUR_LITE]) && (rand_int(100) < 90)) {

		floors = 0;
		/* Scan the "local" grids (5x5) 2 same as torch grid*/
		for (y = c_y - 2; y <= c_y + 2; y++) {
			/* Scan the "local" grids (5x5) */
			for (x = c_x - 2; x <= c_x + 2; x++) {
				/* Get grid */
				ag = &borg_grids[y][x];

				/* Location must be a lit floor */
				if (ag->info & BORG_LITE)
					floors++;

				/* Location must not be glowing.  Minimal game cheat. */
				if ((ag->info & BORG_GLOW) || (cave[y][x].info & CAVE_GLOW))
					floors--;

				/* Location must not be a wall/door */
				if (!borg_cave_floor_grid(ag))
					floors--;
			}
		}
	}
	/* add them up */
	if (floors <= 11)
		do_lite = do_lite_aux = FALSE;

	/* Hack -- call lite */
	if (do_lite && /* !borg_munchkin_mode && */
		 (!when_call_lite || (borg_t - when_call_lite >= 7))) {
		/* Call light */
		if (borg_activate_artifact(ART_GALADRIEL, FALSE) ||
			 borg_zap_rod(SV_ROD_ILLUMINATION) || borg_use_staff(SV_STAFF_LITE) ||
			 borg_read_scroll(SV_SCROLL_LIGHT) || borg_spell(REALM_ARCANE, 0, 5) ||
			 borg_spell(REALM_CHAOS, 0, 2) || borg_spell(REALM_NATURE, 0, 4) ||
			 borg_spell(REALM_SORCERY, 0, 3) || borg_spell(REALM_LIFE, 0, 4) ||
			 borg_mutation(COR1_ILLUMINE, FALSE, 50, FALSE) ||
			 borg_activate_activation(ACT_MAP_LIGHT, FALSE)) {
			borg_note("# Illuminating the room prior to resting");

			borg_react("SELF:lite", "SELF:lite");

			when_call_lite = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			/* dont rest. call light instead */
			return (TRUE);
		}
	}

	/* Hack -- Wizard Lite */
	if (TRUE && (!when_wizard_lite || (borg_t - when_wizard_lite >= 1000))) {
		/* Wizard lite */
		if (borg_activate_artifact(ART_HIPPO, FALSE) ||
			 borg_spell(REALM_ARCANE, 3, 7) || borg_spell(REALM_SORCERY, 3, 3) ||
			 borg_spell(REALM_NATURE, 3, 5) || borg_mindcr(MIND_PRECOGNIT, 45)) {
			borg_note("# Illuminating the dungeon prior to resting");

			/* borg_react("SELF:wizard lite", "SELF:wizard lite"); */

			when_wizard_lite = borg_t;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

			return (TRUE);
		}
	}

	/* nothing to light up.  OK to rest. */
	return (FALSE);
}

/* Check to see if the borg is standing on a nasty grid.
 * Lava can hurt the borg unless he is IFire.
 * Water can hurt if it is deep and encumbered.
 * Levetation item can reduce the effect of nasty grids.
 */
bool borg_on_safe_grid(int y, int x) {
	/* Get the grid under the borg */
	borg_grid *ag = &borg_grids[y][x];

	/* Lava
	if (ag->feat == FEAT_SHAL_LAVA || ag->feat == FEAT_DEEP_LAVA) {
		// Immunity helps
		if (borg_skill[BI_IFIRE])
			return (TRUE);

		// Invulnerability helps
		if (borg_goi || borg_wraith)
			return (TRUE);

		// Everything else hurts
		return (FALSE);
	}
	*/

	/* Water */
	if (ag->feat == FEAT_WATER) {
		/* Levatation helps */
		if (borg_skill[BI_FEATH])
			return (TRUE);

		/* Invulnerability helps */
		if (borg_goi || borg_wraith)
			return (TRUE);

		/* Being encumbered is not good */
		if (borg_skill[BI_ENCUMBERD] <= 0)
			return (TRUE);

		/* Everything else hurts */
		return (FALSE);
	}

	/* Passwall might end with us sitting in a wall. */
	if (/* (borg_skill[BI_PASSWALL] || borg_race == RACE_SPECTRE) && */
		 ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_PERM_SOLID)
		return (FALSE);

	/* Generally ok */
	return (TRUE);
}

/*
 * Enchant armor, not including my swap armour
 */
static bool borg_enchant_to_a(void) {
	int i, b_i = -1;
	int a, b_a = 99;

	/* Nothing to enchant */
	if (!my_need_enchant_to_a)
		return (FALSE);

	/* Need "enchantment" ability */
	if ((!amt_enchant_to_a) && (!amt_enchant_armor))
		return (FALSE);

	/* Look for armor that needs enchanting */
	for (i = INVEN_BODY; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip non-identified items */
		if (!item->ident)
			continue;

		/* Obtain the bonus */
		a = item->to_a;

		/* Skip "boring" items */
		if (borg_spell_okay_fail(REALM_SORCERY, 3, 5, 40) ||
			 amt_enchant_armor >= 1) {
			if (a >= borg_enchant_limit)
				continue;
		} else {
			if (a > 8)
				continue;
		}

		/* Find the least enchanted item */
		if ((b_i >= 0) && (b_a < a))
			continue;

		/* Save the info */
		b_i = i;
		b_a = a;
	}

	/* Nothing */
	if (b_i < 0)
		return (FALSE);

	/* Enchant it */
	if (borg_spell_fail(REALM_SORCERY, 3, 5, 40) ||
		 borg_read_scroll(SV_SCROLL_STAR_ENCHANT_ARMOR) ||
		 borg_read_scroll(SV_SCROLL_ENCHANT_ARMOR)) {
		/* Choose from equipment */
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');

			/* Choose that item */
			borg_keypress(I2A(b_i - INVEN_WIELD));
		}

		/* Success */
		return (TRUE);
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Enchant weapons to hit
 */
static bool borg_enchant_to_h(void) {
	int i, b_i = -1;
	int a, s_a, b_a = 99;

	/* Nothing to enchant */
	if (!my_need_enchant_to_h && !enchant_weapon_swap_to_h)
		return (FALSE);

	/* Need "enchantment" ability */
	if ((!amt_enchant_to_h) && (!amt_enchant_weapon))
		return (FALSE);

	/* Look for a weapon that needs enchanting */
	for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip non-identified items */
		if (!item->ident)
			continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING)
			continue;

		/* Obtain the bonus */
		a = item->to_h;

		/* Skip "boring" items */
		if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
			 amt_enchant_weapon >= 1) {
			if (a >= borg_enchant_limit)
				continue;
		} else {
			if (a >= 8)
				continue;
		}

		/* Find the least enchanted item */
		if ((b_i >= 0) && (b_a < a))
			continue;

		/* Save the info */
		b_i = i;
		b_a = a;
	}
	if (weapon_swap > 1) {
		for (i = weapon_swap; i <= weapon_swap; i++) {
			borg_item *item = &borg_items[weapon_swap];

			/* Obtain the bonus */
			s_a = item->to_h;

			/* Skip my swap digger */
			if (item->tval == TV_DIGGING)
				continue;

			/* Skip "boring" items */
			if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
				 amt_enchant_weapon >= 1) {
				if (s_a >= borg_enchant_limit)
					continue;
			} else {
				if (s_a >= 8)
					continue;
			}

			/* Find the least enchanted item */
			if ((b_i >= 0) && (b_a < s_a))
				continue;

			/* Save the info */
			b_i = weapon_swap;
			b_a = s_a;
		}
	}
	/* Nothing yet, check ammo */
	if (b_i < 0) {
		/* look through inventory for ammo */
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Only enchant if qty >= 5 */
			if (item->iqty < 5)
				continue;

			/* Skip non-identified items  */
			if (!item->ident)
				continue;

			/* Make sure it is the right type if missile */
			if (item->tval != my_ammo_tval)
				continue;

			/* Obtain the bonus  */
			a = item->to_h;

			/* Skip items that are already enchanted */
			if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
				 amt_enchant_weapon >= 1) {
				if (a >= 10)
					continue;
			} else {
				if (a >= 8)
					continue;
			}

			/* Find the least enchanted item */
			if ((b_i >= 0) && (b_a < a))
				continue;

			/* Save the info  */
			b_i = i;
			b_a = a;
		}
	}

	/* Nothing */
	if (b_i < 0)
		return (FALSE);

	/* Enchant it */
	if (borg_spell_fail(REALM_SORCERY, 3, 4, 40) ||
		 borg_read_scroll(SV_SCROLL_STAR_ENCHANT_WEAPON) ||
		 borg_read_scroll(SV_SCROLL_ENCHANT_WEAPON_TO_HIT)) {
		/* Choose from equipment */
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');

			/* Choose that item */
			borg_keypress(I2A(b_i - INVEN_WIELD));
		} else /* choose the swap or ammo */
		{
			borg_keypress(I2A(b_i));
		}

		/* Success */
		return (TRUE);
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Enchant weapons to dam
 */
static bool borg_enchant_to_d(void) {
	int i, b_i = -1;
	int a, s_a, b_a = 99;

	/* Nothing to enchant */
	if (!my_need_enchant_to_d && !enchant_weapon_swap_to_d)
		return (FALSE);

	/* Need "enchantment" ability */
	if ((!amt_enchant_to_d) && (!amt_enchant_weapon))
		return (FALSE);

	/* Look for a weapon that needs enchanting */
	for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip non-identified items */
		if (!item->ident)
			continue;

		/* Skip my swap digger */
		if (item->tval == TV_DIGGING)
			continue;

		/* Obtain the bonus */
		a = item->to_d;

		/* Skip "boring" items */
		if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
			 amt_enchant_weapon >= 1) {
			if (a >= borg_enchant_limit)
				continue;
		} else {
			if (a >= 8)
				continue;
		}

		/* Find the least enchanted item */
		if ((b_i >= 0) && (b_a < a))
			continue;

		/* Save the info */
		b_i = i;
		b_a = a;
	}
	if (weapon_swap > 1) {
		for (i = weapon_swap; i <= weapon_swap; i++) {
			borg_item *item = &borg_items[weapon_swap];

			/* Skip non-identified items */
			if (!item->ident)
				continue;

			/* Obtain the bonus */
			s_a = item->to_d;

			/* Skip my swap digger */
			if (item->tval == TV_DIGGING)
				continue;

			/* Skip "boring" items */
			if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
				 amt_enchant_weapon >= 1) {
				if (s_a >= borg_enchant_limit)
					continue;
			} else {
				if (s_a >= 8)
					continue;
			}

			/* Find the least enchanted item */
			if ((b_i >= 0) && (b_a < s_a))
				continue;

			/* Save the info */
			b_i = weapon_swap;
			b_a = s_a;
		}
	}
	/* Nothing, check ammo */
	if (b_i < 0) {
		/* look through inventory for ammo */
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Only enchant if qty >= 5 */
			if (item->iqty < 5)
				continue;

			/* Skip non-identified items  */
			if (!item->ident)
				continue;

			/* Make sure it is the right type if missile */
			if (item->tval != my_ammo_tval)
				continue;

			/* Obtain the bonus  */
			a = item->to_d;

			/* Skip items that are already enchanted */
			if (borg_spell_okay_fail(REALM_SORCERY, 3, 4, 40) ||
				 amt_enchant_weapon >= 1) {
				if (a >= 10)
					continue;
			} else {
				if (a >= 8)
					continue;
			}

			/* Find the least enchanted item */
			if ((b_i >= 0) && (b_a < a))
				continue;

			/* Save the info  */
			b_i = i;
			b_a = a;
		}
	}

	/* Nothing */
	if (b_i < 0)
		return (FALSE);

	/* Enchant it */
	if (borg_spell_fail(REALM_SORCERY, 3, 4, 40) ||
		 borg_read_scroll(SV_SCROLL_STAR_ENCHANT_WEAPON) ||
		 borg_read_scroll(SV_SCROLL_ENCHANT_WEAPON_TO_DAM)) {
		/* Choose from equipment */
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');

			/* Choose that item */
			borg_keypress(I2A(b_i - INVEN_WIELD));
		} else /* choose the swap or ammo */
		{
			borg_keypress(I2A(b_i));
		}

		/* Success */
		return (TRUE);
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Brand my weapon if I can.
 */
static bool borg_brand_weapon(void) {
	int i, b_i = -1;
	int b_realm = -1;
	int b_book = -1;
	int b_spell = -1;
	int min_mana = 50;

	/* No Need */
	if (!my_need_brand_weapon)
		return (FALSE);
	if (borg_skill[BI_NO_MELEE])
		return (FALSE);

	/* Only if good on mana.  Otherwise we will cast/rest loop */
	if (borg_skill[BI_CURSP] <= borg_skill[BI_MAXSP] * 9 / 10)
		return (FALSE);

	/* Not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Need "brand" ability:
	 * Life Realm: Bless Weapon -- done in borg_bless_weapon(). Perm effect
	 * Sorcery Realm: Elemental Brand
	 * Nature Realm:  Elemental Brand
	 * Chaos Realm: Chaos Brand
	 * Death Realm: Poison Brand
	  *				Vampiric Brand
	 * Trump Realm: Trump Branding (slay demon). Perm.
	 *
	*/
	if (!borg_spell_okay_fail(REALM_SORCERY, 2, 0, 30) &&
		 !borg_spell_okay_fail(REALM_NATURE, 3, 6, 30) &&
		 !borg_spell_okay_fail(REALM_CHAOS, 2, 6, 30) &&
		 !borg_spell_okay_fail(REALM_DEATH, 1, 4, 30) &&
		 !borg_spell_okay_fail(REALM_DEATH, 2, 5, 30) &&
		 !borg_spell_okay_fail(REALM_TRUMP, 2, 4, 30))
		return (FALSE);

	/* look through inventory for ammo */
	for (i = INVEN_WIELD; i < INVEN_BOW; i++) {
		borg_item *item = &borg_items[i];

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

		/* Check Sorcery
		if (borg_spell_okay_fail(REALM_SORCERY, 2, 0, 30) &&
			 (item->name3 != TEMP_ELEC && item->name3 != TEMP_COLD &&
			  item->name3 != TEMP_FIRE)) {
			// Save the info
			b_i = i;
			b_realm = REALM_SORCERY;
			b_book = 2;
			b_spell = 0;
		}*/

		/* Check Nature
		if (borg_spell_okay_fail(REALM_NATURE, 3, 6, 30) &&
			 (item->name3 != TEMP_ELEC && item->name3 != TEMP_COLD &&
			  item->name3 != TEMP_FIRE)) {
			// Save the info
			b_i = i;
			b_realm = REALM_NATURE;
			b_book = 3;
			b_spell = 6;
		}*/

		/* Check Chaos
		if (borg_spell_okay_fail(REALM_CHAOS, 2, 6, 30) &&
			 (item->name3 != TEMP_CHAOTIC && !(item->flags1 & TR1_CHAOTIC))) {
			// Save the info
			b_i = i;
			b_realm = REALM_CHAOS;
			b_book = 2;
			b_spell = 6;
		}*/

		/* Check Death, Poison
		if (borg_spell_okay_fail(REALM_DEATH, 1, 4, 30) &&
			 (item->name3 != TEMP_POIS && !(item->flags1 & TR1_BRAND_POIS))) {
			// Save the info
			b_i = i;
			b_realm = REALM_DEATH;
			b_book = 1;
			b_spell = 4;
			min_mana = 250;
		}*/

		/* Check Death, Vampirism
		if (borg_spell_okay_fail(REALM_DEATH, 2, 5, 30) &&
			 (item->name3 != TEMP_VAMPIRIC && !(item->flags1 & TR1_VAMPIRIC))) {
			// Save the info
			b_i = i;
			b_realm = REALM_DEATH;
			b_book = 2;
			b_spell = 5;
			min_mana = 200;
		}*/

		/* Check Trump */
		if (borg_spell_okay_fail(REALM_TRUMP, 2, 4, 30) && !item->name1 &&
			 !item->name2 && !item->cursed) {
			/* Save the info  */
			b_i = i;
			b_realm = REALM_TRUMP;
			b_book = 2;
			b_spell = 4;
		}
	}

	/* Nothing to Brand */
	if (b_i == -1)
		return (FALSE);

	/* Make sure the borg has enough mana to make the spell worth while. Some
	 * brands are very expensive. */
	if (borg_skill[BI_CURSP] < min_mana)
		return (FALSE);

	/* Enchant it */
	if (borg_spell(b_realm, b_book, b_spell))
	{
		/* choose the weapon */
		/*borg_keypress('/');
		  borg_keypress(I2A(b_i - INVEN_WIELD)); */

		/* examine Inven next turn */
		borg_do_inven = TRUE;

		/* No resting to recoop mana */
		borg_no_rest_prep = 10000;

		/* Success */
		return (TRUE);
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Bless Weapon.
 * Advantages:	Priests can wield it without penalty.
 *				Savings throw against cursing.
 */
static bool borg_bless_weapon(void) {
	int i, b_i = -1;

	borg_item *item;

	/* Not in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Unable to brand */
	if (!borg_spell_okay_fail(REALM_LIFE, 3, 2, 30))
		return (FALSE);

	/* Scan items */
	for (i = 0; i < INVEN_BOW; i++) {
		/* Redefine the Item */
		item = &borg_items[INVEN_WIELD];

		/* Skip non-identified items, to avoid a loop  */
		if (!item->ident)
			continue;

		/* Skip empty slot */
		if (!item->iqty)
			continue;

		/* Skip already blessed */
		if (item->flags3 & TR3_BLESSED)
			continue;

		/* Skip non weapons (melee/bow) and non-missiles */
		if (borg_wield_slot(item) != INVEN_WIELD &&
			 borg_wield_slot(item) != INVEN_BOW && item->tval != TV_SHOT &&
			 item->tval != TV_BOLT && item->tval != TV_ARROW)
			continue;

		/* Assume it can be blessed */
		b_i = i;
	}

	/* Nothing to bless */
	if (b_i == -1)
		return (FALSE);

	/* Bless it */
	if (borg_spell(REALM_LIFE, 3, 2)) {

		/* choose the weapon */
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');
			borg_keypress(I2A(b_i - INVEN_WIELD));
		} else {
			borg_keypress(I2A(b_i));
		}

		/* examine Inven next turn */
		borg_do_inven = TRUE;

		/* Success */
		return (TRUE);
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Remove Curse
 */
static bool borg_decurse_armour(void) {
	borg_item *item;

	/* Nothing to decurse */
	if (decurse_armour_swap == -1 && !borg_wearing_cursed)
		return (FALSE);

	/* Ability for heavy/ light curse */
	if (decurse_armour_swap == 0 || borg_wearing_cursed) {
		if (-1 == borg_slot(TV_SCROLL, SV_SCROLL_STAR_REMOVE_CURSE) &&
			 !borg_spell_okay_fail(REALM_LIFE, 2, 1, 40) &&
			 -1 == borg_slot(TV_SCROLL, SV_SCROLL_REMOVE_CURSE) &&
			 (-1 == borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE) &&
			  -1 == borg_items[borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE)].pval) &&
			 !borg_spell_okay_fail(REALM_LIFE, 2, 1, 40)) {
			return (FALSE);
		}

		if (borg_wearing_cursed) {
			/* no need to wear it first */
		} else {
			/** First wear the item **/

			/* Can it be worn (paranoid) */
			item = &borg_items[armour_swap];
			if (borg_wield_slot(item) < INVEN_WIELD)
				return (FALSE);
			borg_note("# Wearing item before decursing it.");
			borg_keypress('w');
			borg_keypress(I2A(armour_swap));

			/* ooops it feels deathly cold */
			borg_keypress(' ');
		}
		/* remove the curse */
		if (borg_read_scroll(SV_SCROLL_STAR_REMOVE_CURSE) ||
			 borg_spell(REALM_LIFE, 2, 1) ||
			 borg_read_scroll(SV_SCROLL_REMOVE_CURSE) ||
			 borg_use_staff(SV_STAFF_REMOVE_CURSE) ||
			 borg_spell(REALM_LIFE, 2, 1)) {
			/* Shekockazol! */
			borg_wearing_cursed = FALSE;
			return (TRUE);
		}
	}

	/* Nothing to do */
	return (FALSE);
}
/*
 * Remove Curse
 *
 */
static bool borg_decurse_weapon(void) {
	borg_item *item;

	/* Nothing to decurse */
	if (decurse_weapon_swap == -1)
		return (FALSE);

	/* Ability for heavy curse */
	if (decurse_weapon_swap == 1) {
		if (-1 == borg_slot(TV_SCROLL, SV_SCROLL_STAR_REMOVE_CURSE) &&
			 !borg_spell_okay_fail(REALM_LIFE, 2, 1, 40)) {
			return (FALSE);
		}

		/* First wear the item */
		/* Can it be worn (paranoid) */
		item = &borg_items[weapon_swap];
		if (borg_wield_slot(item) < INVEN_WIELD)
			return (FALSE);
		borg_note("# Wearing item before decursing it.");
		borg_keypress('w');
		borg_keypress(I2A(weapon_swap));

		/* ooops it feels deathly cold */
		borg_keypress(' ');

		/* remove the curse */
		if (borg_read_scroll(SV_SCROLL_STAR_REMOVE_CURSE) ||
			 borg_spell(REALM_LIFE, 2, 1)) {
			/* Shekockazol! */
			return (TRUE);
		}
	}

	/* Ability for light curse */
	if (decurse_weapon_swap == 0) {
		if (-1 == borg_slot(TV_SCROLL, SV_SCROLL_REMOVE_CURSE) &&
			 (-1 == borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE) &&
			  -1 == borg_items[borg_slot(TV_STAFF, SV_STAFF_REMOVE_CURSE)].pval) &&
			 !borg_spell_okay_fail(REALM_LIFE, 2, 1, 40)) {
			return (FALSE);
		}

		/* First wear the item */
		/* Can it be worn (paranoid) */
		item = &borg_items[weapon_swap];
		if (borg_wield_slot(item) < INVEN_WIELD)
			return (FALSE);
		borg_note("# Wearing item before decursing it.");
		borg_keypress('w');
		borg_keypress(I2A(weapon_swap));

		/* ooops it feels deathly cold */
		borg_keypress(' ');

		/* remove the curse */
		if (borg_read_scroll(SV_SCROLL_REMOVE_CURSE) ||
			 borg_use_staff(SV_STAFF_REMOVE_CURSE) ||
			 borg_spell(REALM_LIFE, 2, 1)) {
			/* Shekockazol! */
			return (TRUE);
		}
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * Enchant things
 */
bool borg_enchanting(void) {
	/* Forbid blind/confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and finish the job later */
	if ((borg_t - borg_began > 750 && borg_skill[BI_CDEPTH]) ||
		 (borg_t - borg_began > 1350 && !borg_skill[BI_CDEPTH]))
		return (FALSE);

	/* Remove Curses */
	if (borg_decurse_armour())
		return (TRUE);
	if (borg_decurse_weapon())
		return (TRUE);

	/* Enchant things */
	if (borg_enchant_to_h())
		return (TRUE);
	if (borg_enchant_to_d())
		return (TRUE);
	if (borg_enchant_to_a())
		return (TRUE);
	if (borg_brand_weapon())
		return (TRUE);
	if (borg_bless_weapon())
		return (TRUE);
	/* if (borg_brand_bolts()) return (TRUE); */

	/* Nope */
	return (FALSE);
}

/*
 * Recharge things
 *
 * XXX XXX XXX Prioritize available items
 */
bool borg_recharging(void) {
	int i = -1;
	bool charge = FALSE;

	/* Forbid blind/confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* XXX XXX XXX Dark */

	/* Look for an item to recharge */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip junk */
		if (!item->value)
			continue;

		/* Skip non-identified items */
		if (!item->ident && !strstr(item->note, "empty"))
			continue;

		/* assume we can't charge it. */
		charge = FALSE;

		/* Wands with no charges can be charged */
		if ((item->tval == TV_WAND) && (item->pval < 1))
			charge = TRUE;

		/* recharge staves sometimes */
		if (item->tval == TV_STAFF) {
			/* Allow staves to be recharged at 2 charges if
			 * the borg has the big recharge spell. And its not a *Dest*
			 */
			if ((item->pval < 3) && (borg_spell_okay(REALM_SORCERY, 0, 7) ||
											 borg_spell_okay(REALM_CHAOS, 2, 2) ||
											 borg_spell_okay(REALM_ARCANE, 3, 0)) &&
				 item->sval < SV_STAFF_POWER)
				charge = TRUE;

			/* recharge any staff at 0 charges */
			if (item->pval < 1)
				charge = TRUE;

			/* Staves of teleport get recharged at 2 charges in town */
			if ((item->sval == SV_STAFF_TELEPORTATION) && (item->pval < 3) &&
				 !borg_skill[BI_CDEPTH])
				charge = TRUE;
		}

		/* recharge rods that are 'charging' if we have the big recharge spell */
		if (item->tval == TV_ROD && item->pval < item->iqty &&
			 !borg_munchkin_mode &&
			 (borg_spell_okay(REALM_SORCERY, 0, 7) ||
			  borg_spell_okay(REALM_CHAOS, 2, 2) ||
			  borg_spell_okay(REALM_ARCANE, 3, 0) ||
			  borg_equips_artifact(ART_BARD))) {
			charge = TRUE;
		}

		/* Certain Wands need to be recharged */
		if (item->tval == TV_WAND && item->pval == 0 &&
			 (item->sval == SV_WAND_TELEPORT_AWAY ||
			  item->sval == SV_WAND_DRAIN_LIFE ||
			  item->sval == SV_WAND_STINKING_CLOUD ||
			  item->sval == SV_WAND_MAGIC_MISSILE ||
			  item->sval == SV_WAND_ANNIHILATION ||
			  item->sval == SV_WAND_STONE_TO_MUD
			/*item->sval == SV_WAND_ROCKETS*/)) {
			charge = TRUE;
		}

		/* get the next item if we are not to charge this one */
		if (!charge)
			continue;

		/* Attempt to recharge */
		if (borg_spell(REALM_SORCERY, 0, 7) || borg_spell(REALM_ARCANE, 3, 0) ||
			 borg_spell(REALM_CHAOS, 2, 2) ||
			 borg_read_scroll(SV_SCROLL_RECHARGING) ||
			 borg_activate_artifact(ART_BARD, FALSE) ||
			 borg_activate_activation(ACT_RECHARGE, FALSE)) {
			/* Message */
			borg_note(format("Recharging %s", item->desc));

			/* Recharge the item */
			borg_keypress(I2A(i));

			/* Success */
			return (TRUE);
		} else
			/* if we fail once, no need to try again. */
			break;
	}

	/* Nope */
	return (FALSE);
}

/*
 * Attempt to consume an item
 */
static bool borg_consume(int i) {
	borg_item *item = &borg_items[i];

	/* Special destruction */
	switch (item->tval) {
	case TV_POTION:

		/* Check the potion */
		switch (item->sval) {
		case SV_POTION_WATER:
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SLIME_MOLD:
		case SV_POTION_INFRAVISION:
		case SV_POTION_DETECT_INVIS:
		case SV_POTION_SLOW_POISON:
		case SV_POTION_CURE_POISON:
		case SV_POTION_SPEED:
		case SV_POTION_CURE_LIGHT:
		case SV_POTION_CURE_SERIOUS:
		case SV_POTION_CURE_CRITICAL:
		case SV_POTION_CURING:
		case SV_POTION_HEALING:
		case SV_POTION_STAR_HEALING:
		case SV_POTION_LIFE:
		case SV_POTION_RESTORE_EXP:
		case SV_POTION_RESTORE_MANA:
		case SV_POTION_RES_STR:
		case SV_POTION_RES_INT:
		case SV_POTION_RES_WIS:
		case SV_POTION_RES_DEX:
		case SV_POTION_RES_CON:
		case SV_POTION_RES_CHR:
		case SV_POTION_INC_STR:
		case SV_POTION_INC_INT:
		case SV_POTION_INC_WIS:
		case SV_POTION_INC_DEX:
		case SV_POTION_INC_CON:
		case SV_POTION_INC_CHR:
		case SV_POTION_HEROISM:
		case SV_POTION_BESERK_STRENGTH:
		case SV_POTION_RESIST_HEAT:
		case SV_POTION_RESIST_COLD:
		case SV_POTION_AUGMENTATION:
		case SV_POTION_ENLIGHTENMENT:
		case SV_POTION_STAR_ENLIGHTENMENT:
		case SV_POTION_SELF_KNOWLEDGE:
		case SV_POTION_EXPERIENCE:
		case SV_POTION_RESISTANCE:
		case SV_POTION_INVULNERABILITY:
			/* Try quaffing the potion */
			if (borg_quaff_potion(item->sval))
				return (TRUE);
		}

		break;

	case TV_SCROLL:

		/* Check the scroll */
		switch (item->sval) {
		case SV_SCROLL_REMOVE_CURSE:
		case SV_SCROLL_LIGHT:
		case SV_SCROLL_MAPPING:
		case SV_SCROLL_DETECT_GOLD:
		case SV_SCROLL_MONSTER_CONFUSION:
		case SV_SCROLL_DETECT_TRAP:
		case SV_SCROLL_DETECT_DOOR:
		case SV_SCROLL_DETECT_ITEM:
		case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
		case SV_SCROLL_SATISFY_HUNGER:
		case SV_SCROLL_DISPEL_UNDEAD:
		case SV_SCROLL_BLESSING:
		case SV_SCROLL_HOLY_CHANT:
		case SV_SCROLL_HOLY_PRAYER:
		case SV_SCROLL_DETECT_INVIS:
		case SV_SCROLL_STAR_ACQUIREMENT:
		case SV_SCROLL_ACQUIREMENT:
		case SV_SCROLL_ARTIFACT:

			/* Try reading the scroll */
			if (borg_read_scroll(item->sval))
				return (TRUE);
		}

		break;

	case TV_FOOD:

		/* Check the grub */
		switch (item->sval) {
		case SV_FOOD_CURE_POISON:
		case SV_FOOD_CURE_BLINDNESS:
		case SV_FOOD_CURE_PARANOIA:
		case SV_FOOD_CURE_CONFUSION:
		case SV_FOOD_CURE_SERIOUS:
		case SV_FOOD_RESTORE_STR:
		case SV_FOOD_RESTORE_CON:
		case SV_FOOD_RESTORING:
		case SV_FOOD_BISCUIT:
		case SV_FOOD_JERKY:
		case SV_FOOD_RATION:
		case SV_FOOD_SLIME_MOLD:
		case SV_FOOD_WAYBREAD:
		case SV_FOOD_PINT_OF_ALE:
		case SV_FOOD_PINT_OF_WINE:

			/* Not if it will do no good */
			if (borg_race == RACE_SKELETON && item->sval >= SV_FOOD_MIN_FOOD)
				return (FALSE);

			/* Try eating the food (unless Bloated) */
			if (!borg_skill[BI_ISFULL] && borg_eat_food(item->sval))
				return (TRUE);
		}

		break;
	}

	/* Nope */
	return (FALSE);
}

/*
 * Destroy "junk" items
 */
bool borg_crush_junk(void) {
	int i;
	bool fix = FALSE;
	s32b p;
	s32b value;

	/* Hack -- no need */
	if (!borg_do_crush_junk)
		return (FALSE);
	if (borg_skill[BI_CDEPTH] == 0)
		return (FALSE);

	/* No crush if even slightly dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) > borg_skill[BI_CURHP] / 10)
		return (FALSE);

	/* Destroy actual "junk" items */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* some items need to be id'd before we can crush them */
		if (!item->kind && !strstr(item->note, "cursed"))
			continue;

		/* dont crush the swap weapon */
		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;

		/* Hack -- skip quest rewards */
		if (item->quest && !item->ident && item->value)
			continue;

		/* Do not crush the One Ring */
		if (item->name1 == ART_FIRST)
			continue;

/* Dont crush weapons if we are wielding a digger */
#if 0
            if (item->tval >=TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;
#endif

		/* Don't crush an equipment if I have a vacant slot */
		if (borg_wield_slot(item) >= INVEN_WIELD &&
			 !borg_items[borg_wield_slot(item)].iqty)
			continue;

		/* dont crush our spell books */
		if (item->tval == REALM1_BOOK || item->tval == REALM2_BOOK)
			continue;

		/* Do not potential speed items */
		if ((item->tval == TV_BOOTS || item->tval == TV_RING) && !item->ident &&
			 item->value)
			continue;

		/* Never crush Rod of Healing or Annihilation*/
		if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING)
			continue;
		if (item->tval == TV_WAND && item->sval == SV_WAND_ANNIHILATION)
			continue;

		/* Special fake value added to certain types of missile potions while low
		 * clevel.
		 * Override the value of some take items in order for the borg to flow to
		 * it and carry it around.
		 * These values will allow the borg to avoid crushing (junk) these items.
		 * He can use them as
		  * thrown weapons
		 */
		if (borg_skill[BI_CLEVEL] < 10 &&
			 (item->tval == TV_POTION &&
			  (item->sval == SV_POTION_SLEEP || item->sval == SV_POTION_SLOWNESS ||
				item->sval == SV_POTION_POISON ||
				item->sval == SV_POTION_BLINDNESS ||
				item->sval == SV_POTION_CONFUSION)))

		{
			/* These can be used as missile weapons */
			continue;
		}

		/* Magical figurines are not junk
		if (item->kind == K_FIGURINE)
			continue;
		*/

		/* Do not crush the item if it has a resistance which I am lacking */
		if (item->value) {
			if ((item->flags2 & TR2_RES_POIS) && !borg_skill[BI_RPOIS])
				continue;
			if ((item->flags2 & TR2_RES_SOUND) && !borg_skill[BI_RSND])
				continue;
			if ((item->flags2 & TR2_RES_LITE) && !borg_skill[BI_RLITE])
				continue;
			if ((item->flags2 & TR2_RES_DARK) && !borg_skill[BI_RDARK])
				continue;
			if ((item->flags2 & TR2_RES_CHAOS) && !borg_skill[BI_RKAOS])
				continue;
			if ((item->flags2 & TR2_RES_CONF) && !borg_skill[BI_RCONF])
				continue;
			if ((item->flags2 & TR2_RES_BLIND) && !borg_skill[BI_RBLIND])
				continue;
			if ((item->flags2 & TR2_RES_DISEN) && borg_skill[BI_MAXDEPTH] >= 50 &&
				 !borg_skill[BI_RDIS])
				continue;
			if ((item->flags2 & TR2_RES_SHARDS) && borg_skill[BI_MAXDEPTH] >= 50 &&
				 !borg_skill[BI_RSHRD])
				continue;
			if ((item->flags2 & TR2_RES_NEXUS) && borg_skill[BI_MAXDEPTH] >= 50 &&
				 !borg_skill[BI_RNXUS])
				continue;
			if ((item->flags2 & TR2_RES_NETHER) && borg_skill[BI_MAXDEPTH] >= 50 &&
				 !borg_skill[BI_RNTHR])
				continue;
			if ((item->flags2 & TR2_REFLECT) && borg_skill[BI_MAXDEPTH] >= 50 &&
				 !borg_skill[BI_REFLECT])
				continue;
		}

		/* save the items value */
		value = item->value * item->iqty;

		/* Crush missiles that aren't mine */
		if (item->tval == TV_SHOT || item->tval == TV_ARROW ||
			 item->tval == TV_BOLT) {
			if (item->tval != my_ammo_tval)
				value = 0L;
		}

		/* No need to keep some stuff if I am super rich */
		if (borg_gold > 200000)
			value -= 3000;
		if (borg_gold > 300000)
			value -= 5000;
		if (borg_gold > 500000 && item->tval >= TV_LIFE_BOOK)
			value -= 25000;

		/* Crush food if I can't eat it */
		if (borg_skill[BI_NOEAT] && item->tval == TV_FOOD &&
			 (item->sval == SV_FOOD_WAYBREAD || item->sval == SV_FOOD_RATION ||
			  item->sval == SV_FOOD_JERKY || item->sval == SV_FOOD_BISCUIT ||
			  item->sval == SV_FOOD_SLIME_MOLD))
			value = 0L;

		/* Skip non "worthless" items */
		if (item->tval >= TV_SPIKE) {
			/* unknown and not worthless */
			if (!item->aware && !strstr(item->note, "average") && value > 0)
				continue;

			/* skip items that are 'valuable'.  This is level dependent */
			/* try to make the borg junk +1,+1 dagger at level 40 */

			/* if the item gives a bonus to a stat, boost its value */
			if (((item->flags1 & TR1_STR) || (item->flags1 & TR1_INT) ||
				  (item->flags1 & TR1_WIS) || (item->flags1 & TR1_DEX) ||
				  (item->flags1 & TR1_CON)) &&
				 value > 0) {
				value += 2000L;
			}

			/* Keep some stuff */
			if ((item->tval == my_ammo_tval && value > 0) ||
				 ((item->tval == TV_POTION &&
					item->sval == SV_POTION_RESTORE_MANA) &&
				  (borg_skill[BI_MAXSP] >= 1)) ||
				 (item->tval == TV_POTION && item->sval == SV_POTION_HEALING) ||
				 (item->tval == TV_POTION &&
				  item->sval == SV_POTION_STAR_HEALING) ||
				 (item->tval == TV_POTION && item->sval == SV_POTION_LIFE) ||
				 (item->tval == TV_POTION && item->sval == SV_POTION_SPEED) ||
				 (item->tval == TV_ROD && item->sval == SV_ROD_DRAIN_LIFE) ||
				 (item->tval == TV_ROD && item->sval == SV_ROD_TELEPORT_AWAY) ||
				 (item->tval == TV_ROD && item->sval == SV_ROD_ILLUMINATION &&
				  borg_skill[BI_ALITE] <= 1000) ||
				 (item->tval == TV_STAFF && item->sval == SV_STAFF_DISPEL_EVIL) ||
				 (item->tval == TV_STAFF && item->sval == SV_STAFF_POWER) ||
				 (item->tval == TV_STAFF && item->sval == SV_STAFF_HOLINESS) ||
				 (item->tval == TV_WAND && item->sval == SV_WAND_STONE_TO_MUD &&
				  borg_skill[BI_ASTONE2MUD] < 50) ||
				 (item->tval == TV_WAND && item->sval == SV_WAND_DRAIN_LIFE) ||
				 (item->tval == TV_CLOAK && item->name2 == EGO_AMAN) ||
				 (item->tval == TV_SCROLL &&
				  item->sval == SV_SCROLL_TELEPORT_LEVEL &&
				  borg_skill[BI_ATELEPORTLVL] < 1000) ||
				 (item->tval == TV_SCROLL &&
				  item->sval == SV_SCROLL_PROTECTION_FROM_EVIL)) {
				continue;
			}

			/* Certain types of scrolls should be kept */
			if (!borg_skill[BI_NO_MELEE]) {
				if (item->tval == TV_SCROLL &&
					 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
					  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_HIT) &&
					 (enchant_weapon_swap_to_h >= 1 ||
					  enchant_weapon_swap_to_h >= 1))
					continue;
				if (item->tval == TV_SCROLL &&
					 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
					  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_DAM) &&
					 (enchant_weapon_swap_to_d >= 1 ||
					  enchant_weapon_swap_to_d >= 1))
					continue;
			}
			if (item->tval == TV_SCROLL &&
				 (item->sval == SV_SCROLL_STAR_ENCHANT_ARMOR ||
				  item->sval == SV_SCROLL_ENCHANT_ARMOR) &&
				 (enchant_armour_swap_to_a >= 1 || enchant_armour_swap_to_a >= 1))
				continue;
			if (item->tval == TV_SCROLL &&
				 (item->sval == SV_SCROLL_ACQUIREMENT ||
				  item->sval == SV_SCROLL_STAR_ACQUIREMENT ||
				  item->sval == SV_SCROLL_ARTIFACT))
				continue;

			/* borg_worships_gold will sell all kinds of stuff,
				 * except {cursed} is junk
				 */
			if (item->value > 0 && /*Worth something*/
				 ((borg_worships_gold || borg_skill[BI_MAXCLEVEL] < 10) || /*We are low on gold and low level */
				  (borg_money_scum_amount < borg_gold && borg_money_scum_amount != 0 &&	borg_skill[BI_MAXCLEVEL] <= 20)) &&
						!(strstr(item->note, "cursed")))
				continue;

			/* up to level 5, keep anything of any value */
			if ((borg_skill[BI_MAXDEPTH] < 5 /* || borg_munchkin_mode */) &&
				 value > 0)
				continue;
			/* up to level 10, keep anything of any value */
			if (borg_skill[BI_MAXDEPTH] < 10 && value > 15)
				continue;
			/* up to level 15, keep anything of value 100 or better */
			if (borg_skill[BI_MAXDEPTH] < 15 && value > 100)
				continue;
			/* up to level 30, keep anything of value 500 or better */
			if (borg_skill[BI_MAXDEPTH] < 30 && value > 500)
				continue;
			/* up to level 40, keep anything of value 1000 or better */
			if (borg_skill[BI_MAXDEPTH] < 40 && value > 1000)
				continue;

			/* Save the item */
			COPY(&safe_items[i], &borg_items[i], borg_item);

			/* Destroy the item */
			WIPE(&borg_items[i], borg_item);

			/* Fix later */
			fix = TRUE;

			/* Examine the inventory */
			borg_notice(FALSE);

			/* Evaluate the inventory */
			p = borg_power();

			/* Restore the item */
			COPY(&borg_items[i], &safe_items[i], borg_item);

			/* skip things we are using */
			if (p < my_power)
				continue;
		}

		/* re-examine the inventory */
		if (fix)
			borg_notice(TRUE);

		/* Hack -- skip good un-id'd "artifacts" */
		if (strstr(item->note, "special") && !item->fully_identified)
			continue;
		if (strstr(item->desc, "terrible") && !item->fully_identified)
			continue;
		if (strstr(item->desc, "indestructible") && !item->fully_identified)
			continue;

		/* hack -- with random artifacts some are good and bad */
		/*         so check them all */
		if (/* adult_rand_artifacts  && */ item->name1 && !item->fully_identified)
			continue;

		/* Attempt to absorb charges for mana first */
		if ((item->tval == TV_ROD || item->tval == TV_STAFF ||
			  item->tval == TV_WAND) &&
			 (item->pval) && borg_mutation(COR1_EAT_MAGIC, FALSE, 30, FALSE)) {
			/* Note and drain */
			borg_keypress(I2A(i));
			borg_note(
				 format("# Draining charges from %s prior to junking", item->desc));
			return (TRUE);
		}

		/* Debug */
		borg_note(format("# Junking %ld gold (junk)", value));

		/* Message */
		borg_note(format("# Destroying (junk) %s.", item->desc));

		/* Attempt to consume the item, if its harmless, */
		if (borg_consume(i))
			return (TRUE);

		/* Destroy all items */
		borg_keypresses("099");

		/* Destroy that item */
		if (!item->name1) {
			/* Use Midas Touch mutation if you got it */
			if (item->value >= 5 &&
				 (borg_mutation(COR1_MIDAS_TCH, FALSE, 30, FALSE) ||
				  borg_activate_activation(ACT_ALCHEMY, FALSE))) {
				borg_keypress(I2A(i));

				/* Destroy one item */
				if (item->iqty >= 2) {
					borg_keypresses("01");
					borg_keypress('\r');
				}

				/* Verify destruction */
				borg_keypress('y');
				return (TRUE);

			} else {
				borg_keypress('k');
			}
		} else {
			/* worthless artifacts are dropped. */
			borg_keypress('d');

			/* mark the spot that the object was dropped so that  */
			/* it will not be picked up again. */
			bad_obj_x[bad_obj_num] = c_x;
			bad_obj_y[bad_obj_num] = c_y;
			borg_note(format("# Crappy artifact at %d,%d", bad_obj_x[bad_obj_num],
								  bad_obj_y[bad_obj_num]));
			bad_obj_num++;
		}
		borg_keypress(I2A(i));

		/* Success */
		return (TRUE);
	}

	/* re-examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Hack -- no need */
	borg_do_crush_junk = FALSE;

	/* Nothing to destroy */
	return (FALSE);
}

/*
 * Destroy something to make a free inventory slot.
 *
 * This function evaluates the possible worlds that result from
 * the destruction of each inventory slot, and attempts to destroy
 * that slot which causes the best possible resulting world.
 *
 * We attempt to avoid destroying unknown items by "rewarding" the
 * presence of unknown items by a massively heuristic value.
 *
 * If the Borg cannot find something to destroy, which should only
 * happen if he fills up with artifacts, then he will probably act
 * rather twitchy for a while.
 *
 * This function does not have to be very efficient.
 */
bool borg_crush_hole(void) {
	int i, b_i = -1;
	s32b p, b_p = 0L;

	s32b value;

	bool fix = FALSE;
	int ammo_slots = 0;

	/* Do not destroy items unless we need the space */
	if (!borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* No crush if even slightly dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) > borg_skill[BI_CURHP] / 10 &&
		 (borg_skill[BI_CURHP] != borg_skill[BI_MAXHP] ||
		  borg_danger(c_y, c_x, 1, TRUE) > (borg_skill[BI_CURHP] * 2) / 3))
		return (FALSE);

	/* Scan the inventory */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;
		if (!item->kind)
			continue;

		/* Hack -- skip "artifacts" or quest rewards */
		if (item->name1)
			continue;
		if (item->quest && !item->ident)
			continue;

		/* dont crush the swap weapon */
		if (i == weapon_swap && item->tval != TV_FOOD)
			continue;
		if (i == armour_swap && item->tval != TV_FOOD)
			continue;

		/* Do not crush the One Ring */
		if (item->name1 == ART_FIRST)
			continue;

		/* dont crush our spell books */
		if (item->tval == REALM1_BOOK || item->tval == REALM2_BOOK)
			continue;

		/* Do not crush Boots, they could be SPEED */
		if ((item->tval == TV_BOOTS || item->tval == TV_RING) && !item->ident)
			continue;

		/* Dont crush weapons if we are weilding a digger */
		if (item->tval >= TV_DIGGING && item->tval <= TV_SWORD &&
			 borg_items[INVEN_WIELD].tval == TV_DIGGING)
			continue;

		/* Don't crush an equipment if I have a vacant slot */
		if (borg_wield_slot(item) >= INVEN_WIELD &&
			 !borg_items[borg_wield_slot(item)].iqty)
			continue;

		/* Hack -- skip "artifacts" */
		if (item->name1 && !item->fully_identified)
			continue;
		if (strstr(item->desc, "special"))
			continue;
		if (strstr(item->desc, "terrible"))
			continue;
		if (strstr(item->desc, "indestructible"))
			continue;

		/* or lites if we are out of light */
		if (item->tval == TV_LITE && item->pval >= 1 &&
			 borg_skill[BI_CUR_LITE] == 0)
			continue;

		/* never crush cool stuff that we might be needing later */
		if ((item->tval == TV_POTION && item->sval == SV_POTION_RESTORE_MANA) &&
			 (borg_skill[BI_MAXSP] >= 1))
			continue;
		if (item->tval == TV_POTION && item->sval == SV_POTION_HEALING)
			continue;
		if (item->tval == TV_POTION && item->sval == SV_POTION_STAR_HEALING)
			continue;
		if (item->tval == TV_POTION && item->sval == SV_POTION_LIFE)
			continue;
		if (item->tval == TV_POTION && item->sval == SV_POTION_SPEED)
			continue;
		if (item->tval == TV_SCROLL &&
			 item->sval == SV_SCROLL_PROTECTION_FROM_EVIL)
			continue;
		if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_RUNE_OF_PROTECTION)
			continue;
		if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT_LEVEL &&
			 borg_skill[BI_ATELEPORTLVL] < 1000)
			continue;
		/* Certain types of scrolls should be kept */
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
			  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_HIT) &&
			 (enchant_weapon_swap_to_h >= 1 || enchant_weapon_swap_to_h >= 1))
			continue;
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
			  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_DAM) &&
			 (enchant_weapon_swap_to_d >= 1 || enchant_weapon_swap_to_d >= 1))
			continue;
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_ARMOR ||
			  item->sval == SV_SCROLL_ENCHANT_ARMOR) &&
			 (enchant_armour_swap_to_a >= 1 || enchant_armour_swap_to_a >= 1))
			continue;
		if (item->tval == TV_CLOAK && item->name2 == EGO_AMAN)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_TELEPORT_AWAY)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_RECALL &&
			 borg_skill[BI_RECALL] <= 1000)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_ILLUMINATION &&
			 borg_skill[BI_ALITE] <= 1000)
			continue;
		if (item->tval == TV_WAND && item->sval == SV_WAND_STONE_TO_MUD &&
			 borg_skill[BI_ASTONE2MUD] < 50)
			continue;

		/* Do not crush the item if it has a resistance which I am lacking */
		if ((item->flags2 & TR2_RES_POIS) && !borg_skill[BI_RPOIS])
			continue;
		if ((item->flags2 & TR2_RES_SOUND) && !borg_skill[BI_RSND])
			continue;
		if ((item->flags2 & TR2_RES_LITE) && !borg_skill[BI_RLITE])
			continue;
		if ((item->flags2 & TR2_RES_DARK) && !borg_skill[BI_RDARK])
			continue;
		if ((item->flags2 & TR2_RES_CHAOS) && !borg_skill[BI_RKAOS])
			continue;
		if ((item->flags2 & TR2_RES_CONF) && !borg_skill[BI_RCONF])
			continue;
		if ((item->flags2 & TR2_RES_BLIND) && !borg_skill[BI_RBLIND])
			continue;
		if ((item->flags2 & TR2_RES_DISEN) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RDIS])
			continue;
		if ((item->flags2 & TR2_RES_SHARDS) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RSHRD])
			continue;
		if ((item->flags2 & TR2_RES_NEXUS) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RNXUS])
			continue;
		if ((item->flags2 & TR2_RES_NETHER) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RNTHR])
			continue;
		if ((item->flags2 & TR2_REFLECT) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_REFLECT])
			continue;

		/* Count up the slots taken my ammo */
		if (item->tval == my_ammo_tval)
			ammo_slots++;

		/* save the items value */
		value = item->value * item->iqty;

		/* Save the item */
		COPY(&safe_items[i], &borg_items[i], borg_item);

		/* Destroy the item */
		WIPE(&borg_items[i], borg_item);

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory */
		borg_notice(FALSE);

		/* Evaluate the inventory */
		p = borg_power();

		/* power is much more important than gold. */
		p *= 100;

		/* Restore the item */
		COPY(&borg_items[i], &safe_items[i], borg_item);

		/* Penalize loss of "gold" early on */
		if (borg_munchkin_start &&
			 (borg_skill[BI_MAXCLEVEL] <= borg_munchkin_level)) {
			value *= 3;

			/* Keep certain items */
			if (value >= 100)
				continue;
		}

		/* if the item gives a bonus to a stat, boost its value */
		if ((item->flags1 & TR1_STR) || (item->flags1 & TR1_INT) ||
			 (item->flags1 & TR1_WIS) || (item->flags1 & TR1_DEX) ||
			 (item->flags1 & TR1_CON)) {
			value += 20000L;
		}

		/*  Keep the correct types of missiles which have value */
		if ((item->tval == my_ammo_tval) && (value > 0)) {
			value += 5000L;
		}

		/* Hack  show prefrence for destroying things we will not use */
		/* if we are high enough level not to worry about gold. */
		if (borg_skill[BI_CLEVEL] > 35) {
			switch (item->tval) {
			/* rings are under valued. */
			case TV_RING:
				p -= (item->iqty * value * 10);
				break;

			case TV_AMULET:
			case TV_BOW:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				p -= (item->iqty * value * 5);
				break;
			case TV_CLOAK:
				if (item->name2 != EGO_AMAN)
					p -= (item->iqty * (300000L));
				else
					p -= (item->iqty * value);
				break;

			case TV_ROD:
				/* BIG HACK! don't crush cool stuff. */
				if ((item->sval != SV_ROD_DRAIN_LIFE) ||
					 (item->sval != SV_ROD_ACID_BALL) ||
					 (item->sval != SV_ROD_ELEC_BALL) ||
					 (item->sval != SV_ROD_FIRE_BALL) ||
					 (item->sval != SV_ROD_COLD_BALL))
					p -= (item->iqty * (300000L)); /* value at 30k */
				else
					p -= (item->iqty * value);
				break;

			case TV_STAFF:
				/* BIG HACK! don't crush cool stuff. */
				if (item->sval != SV_STAFF_DISPEL_EVIL ||
					 ((item->sval != SV_STAFF_POWER ||
						item->sval != SV_STAFF_HOLINESS) &&
					  amt_cool_staff < 2) ||
					 (item->sval != SV_STAFF_DESTRUCTION &&
					  borg_skill[BI_ASTFDEST] < 2))
					p -= (item->iqty * (300000L)); /* value at 30k */
				else
					p -= (item->iqty * (value));
			case TV_WAND:
				/* BIG HACK! don't crush cool stuff. */
				if ((item->sval != SV_WAND_DRAIN_LIFE) ||
					 (item->sval != SV_WAND_ACID_BALL) ||
					 (item->sval != SV_WAND_ELEC_BALL) ||
					 (item->sval != SV_WAND_FIRE_BALL) ||
					 (item->sval != SV_WAND_COLD_BALL) ||
					 (item->sval != SV_WAND_ANNIHILATION) ||
					 (item->sval != SV_WAND_DRAGON_FIRE) ||
					 (item->sval != SV_WAND_DRAGON_COLD))
					p -= (item->iqty * (300000L)); /* value at 30k */
				else
					p -= (item->iqty * (value));
				break;

			/* scrolls and potions crush easy */
			case TV_SCROLL:
				if ((item->sval != SV_SCROLL_PROTECTION_FROM_EVIL) ||
					 (item->sval != SV_SCROLL_RUNE_OF_PROTECTION))
					p -= (item->iqty * (30000L));
				else
					p -= (item->iqty * (value));
				break;

			case TV_POTION:
				/* BIG HACK! don't crush heal/mana potions.  It could be */
				/* that we are in town and are collecting them. */
				if ((item->sval != SV_POTION_HEALING) ||
					 (item->sval != SV_POTION_STAR_HEALING) ||
					 (item->sval != SV_POTION_LIFE) ||
					 (item->sval != SV_POTION_RESTORE_MANA))
					p -= (item->iqty * (300000L)); /* value at 30k */
				else
					p -= (item->iqty * (value));
				break;

			default:
				p -= (item->iqty * value);
				break;
			}
		} else {
			p -= (item->iqty * value);
		}

		/* Hack -- try not to destroy "unaware" items */
		if (!item->kind && (value > 0)) {
			/* Hack -- Reward "unaware" items */
			switch (item->tval) {
			case TV_RING:
			case TV_AMULET:
				p -= (borg_skill[BI_MAXDEPTH] * 5000L);
				break;

			case TV_ROD:
				p -= (borg_skill[BI_MAXDEPTH] * 3000L);
				break;

			case TV_STAFF:
			case TV_WAND:
				p -= (borg_skill[BI_MAXDEPTH] * 2000L);
				break;

			case TV_SCROLL:
			case TV_POTION:
				p -= (borg_skill[BI_MAXDEPTH] * 500L);
				break;

			case TV_FOOD:
				p -= (borg_skill[BI_MAXDEPTH] * 10L);
				break;
			}
		}

		/* Hack -- try not to destroy "unknown" items (unless "icky") */
		if (!item->aware && (value > 0) && !borg_item_icky(item)) {
			/* Reward "unknown" items */
			switch (item->tval) {
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
				p -= 100L;
				break;

			case TV_BOW:
				p -= 20000L;
				break;

			case TV_DIGGING:
				p -= 10L;
				break;

			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
				p -= 10000L;
				break;

			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
				p -= 15000L;
				break;

			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				p -= 15000L;
				break;

			case TV_AMULET:
			case TV_RING:
				p -= 5000L;
				break;

			case TV_STAFF:
			case TV_WAND:
				p -= 1000L;
				break;
			}
		}

		/* Ignore "bad" swaps */
		if ((b_i >= 0) && (p < b_p))
			continue;

		/* Maintain the "best" */
		b_i = i;
		b_p = p;
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Attempt to destroy it */
	if (b_i >= 0) {
		borg_item *item = &borg_items[b_i];

		/* Attempt to absorb charges for mana first */
		if ((item->tval == TV_ROD || item->tval == TV_STAFF ||
			  item->tval == TV_WAND) &&
			 (item->pval) && borg_mutation(COR1_EAT_MAGIC, FALSE, 30, FALSE)) {
			/* Note and drain */
			borg_keypress(I2A(b_i));
			borg_note(
				 format("# Draining charges from %s prior to junking", item->desc));
			return (TRUE);
		}

		/* Debug */
		borg_note(format("# Junking %ld gold (full)", value));

		/* Try to consume the junk */
		if (borg_consume(b_i))
			return (TRUE);

		/* Message */
		borg_note(format("# Destroying (hole) %s.", item->desc));

		/* Destroy all items */
		borg_keypresses("099");

		/* Destroy that item */
		if (!item->name1) {
			/* Use Midas Touch mutation if you got it */
			if (item->value >= 5 &&
				 (borg_mutation(COR1_MIDAS_TCH, FALSE, 30, FALSE) ||
				  borg_activate_activation(ACT_ALCHEMY, FALSE))) {
				borg_keypress(I2A(b_i));

				/* Destroy one item */
				if (item->iqty >= 2) {
					borg_keypresses("01");
					borg_keypress('\r');
				}

				/* Verify destruction */
				borg_keypress('y');
				return (TRUE);

			} else {
				borg_keypress('k');
			}
		} else {
			/* worthless artifacts are dropped. */
			borg_keypress('d');

			/* mark the spot that the object was dropped so that  */
			/* it will not be picked up again. */
			bad_obj_x[bad_obj_num] = c_x;
			bad_obj_y[bad_obj_num] = c_y;
			borg_note(format("# Crappy artifact at %d,%d", bad_obj_x[bad_obj_num],
								  bad_obj_y[bad_obj_num]));
			bad_obj_num++;
		}
		borg_keypress(I2A(b_i));

		/* Success */
		return (TRUE);
	}

	/* Hack -- no need */
	borg_do_crush_hole = FALSE;

	/* Paranoia */
	return (FALSE);
}

/*
 * Destroy "junk" when slow (in the dungeon).
 *
 * We penalize the loss of both power and monetary value, and reward
 * the loss of weight that may be slowing us down.  The weight loss
 * is worth one gold per tenth of a pound.  This causes things like
 * lanterns and chests and spikes to be considered "annoying".
 */
bool borg_crush_slow(void) {
	int i, b_i = -1;
	s32b p, b_p = 0L;

	s32b temp;

	s32b greed;

	bool fix = FALSE;

	/* No crush if even slightly dangerous */
	if (borg_danger(c_y, c_x, 1, TRUE) > borg_skill[BI_CURHP] / 20)
		return (FALSE);

	/* Hack -- never in town */
	if (!borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Do not crush items unless we are slow */
	if (borg_skill[BI_SPEED] >= 110 && !borg_skill[BI_ENCUMBERD])
		return (FALSE);

	/* Calculate "greed" factor */
	greed = (borg_gold / 100L) + 100L;

	/* Minimal greed */
	if (greed < 500L && borg_skill[BI_CLEVEL] > 35)
		greed = 500L;
	if (greed > 25000L)
		greed = 25000L;

	/* Decrease greed by our slowness */
	greed -= (110 - borg_skill[BI_SPEED]) * 100;
	if (greed <= 0)
		greed = 0L;

	/* Scan for junk */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;
		if (!item->kind)
			continue;

		/* dont crush the swap weapon */
		if (i == weapon_swap && item->iqty == 1)
			continue;
		if (i == armour_swap)
			continue;

		/* Hack -- skip quest rewards */
		if (item->quest && !item->ident)
			continue;

		/* Skip wands */
		if (item->tval == TV_WAND && borg_skill[BI_CLEVEL] <= 30)
			continue;

		/* Skip "good" unknown items (unless "icky") */
		if (!item->aware && !borg_item_icky(item))
			continue;

		/* Do not crush potential speed items5 */
		if (item->tval == TV_BOOTS && !item->ident)
			continue;
		if (item->tval == TV_RING && item->sval == SV_RING_SPEED && !item->ident)
			continue;

		/* Don't crush an equipment if I have a vacant slot */
		if (borg_wield_slot(item) >= INVEN_WIELD &&
			 !borg_items[borg_wield_slot(item)].iqty)
			continue;

		/* Never crush Rod of Healing or Annihilation*/
		if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING)
			continue;
		if (item->tval == TV_WAND && item->sval == SV_WAND_ANNIHILATION)
			continue;

		/* Stone to Mud wand is really important for summoners late in the game */
		if (item->tval == TV_WAND && item->sval == SV_WAND_STONE_TO_MUD &&
			 borg_skill[BI_ASTONE2MUD] < 50)
			continue;

		/* Scrolls of escaping */
		if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_PHASE_DOOR)
			continue;
		if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_TELEPORT_AWAY)
			continue;
		if (item->tval == TV_ROD && item->sval == SV_ROD_ILLUMINATION &&
			 borg_skill[BI_ALITE] <= 1000)
			continue;

		/* Hack -- Skip artifacts */
		if (/* adult_rand_artifacts  && */ item->name1 && !item->fully_identified)
			continue;
		if (strstr(item->desc, "special"))
			continue;
		if (strstr(item->desc, "{terrible"))
			continue;
		if (strstr(item->desc, "{indestructible"))
			continue;

		/* Dont crush weapons if we are weilding a digger */
		if (item->tval >= TV_DIGGING && item->tval <= TV_SWORD &&
			 borg_items[INVEN_WIELD].tval == TV_DIGGING)
			continue;

		/* Certain types of scrolls should be kept */
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
			  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_HIT) &&
			 (enchant_weapon_swap_to_h >= 1 || enchant_weapon_swap_to_h >= 1))
			continue;
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_WEAPON ||
			  item->sval == SV_SCROLL_ENCHANT_WEAPON_TO_DAM) &&
			 (enchant_weapon_swap_to_d >= 1 || enchant_weapon_swap_to_d >= 1))
			continue;
		if (item->tval == TV_SCROLL &&
			 (item->sval == SV_SCROLL_STAR_ENCHANT_ARMOR ||
			  item->sval == SV_SCROLL_ENCHANT_ARMOR) &&
			 (enchant_armour_swap_to_a >= 1 || enchant_armour_swap_to_a >= 1))
			continue;

		/* Do not crush the One Ring */
		if (item->name1 == ART_FIRST)
			continue;

		/* Magical figurines are not junk
		if (item->kind == K_FIGURINE)
			continue;
		*/

		/* Do not crush the item if it has a resistance which I am lacking */
		if ((item->flags2 & TR2_RES_POIS) && !borg_skill[BI_RPOIS])
			continue;
		if ((item->flags2 & TR2_RES_SOUND) && !borg_skill[BI_RSND])
			continue;
		if ((item->flags2 & TR2_RES_LITE) && !borg_skill[BI_RLITE])
			continue;
		if ((item->flags2 & TR2_RES_DARK) && !borg_skill[BI_RDARK])
			continue;
		if ((item->flags2 & TR2_RES_CHAOS) && !borg_skill[BI_RKAOS])
			continue;
		if ((item->flags2 & TR2_RES_CONF) && !borg_skill[BI_RCONF])
			continue;
		if ((item->flags2 & TR2_RES_BLIND) && !borg_skill[BI_RBLIND])
			continue;
		if ((item->flags2 & TR2_RES_DISEN) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RDIS])
			continue;
		if ((item->flags2 & TR2_RES_SHARDS) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RSHRD])
			continue;
		if ((item->flags2 & TR2_RES_NEXUS) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RNXUS])
			continue;
		if ((item->flags2 & TR2_RES_NETHER) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_RNTHR])
			continue;
		if ((item->flags2 & TR2_REFLECT) && borg_skill[BI_MAXDEPTH] >= 50 &&
			 !borg_skill[BI_REFLECT])
			continue;

		/* Save the item */
		COPY(&safe_items[i], &borg_items[i], borg_item);

		/* Destroy one of the items */
		borg_items[i].iqty--;

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory */
		borg_notice(FALSE);

		/* Evaluate the inventory */
		p = borg_power();

		/* Restore the item */
		COPY(&borg_items[i], &safe_items[i], borg_item);

		/* Obtain the base price */
		temp = ((item->value < 30000L) ? item->value : 30000L);

		/* Hack -- ignore very cheap items */
		if (temp < greed)
			temp = 0L;

		/* Penalize */
		p -= temp;

		/* Obtain the base weight */
		temp = item->weight;

		/* Reward */
		p += (temp * 50);

		/* Ignore "bad" swaps */
		if (p < b_p)
			continue;

		/* Maintain the "best" */
		b_i = i;
		b_p = p;
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Destroy "useless" things */
	if ((b_i >= 0) && (b_p >= (my_power))) {
		borg_item *item = &borg_items[b_i];

		/* Attempt to absorb charges for mana first */
		if ((item->tval == TV_ROD || item->tval == TV_STAFF ||
			  item->tval == TV_WAND) &&
			 (item->pval) && borg_mutation(COR1_EAT_MAGIC, FALSE, 30, FALSE)) {
			/* Note and drain */
			borg_keypress(I2A(b_i));
			borg_note(
				 format("# Draining charges from %s prior to junking", item->desc));
			return (TRUE);
		}

		/* Message */
		borg_note(format("# Junking %ld gold (slow)", (my_power)-b_p));

		/* Attempt to consume it */
		if (borg_consume(b_i))
			return (TRUE);

		/* Message */
		borg_note(format("# Destroying (slow) %s.", item->desc));

		/* Destroy one item */
		borg_keypress('0');
		borg_keypress('1');

		/* Destroy that item */
		if (!item->name1) {
			/* Use Midas Touch mutation if you got it */
			if (item->value >= 5 &&
				 (borg_mutation(COR1_MIDAS_TCH, FALSE, 30, FALSE) ||
				  borg_activate_activation(ACT_ALCHEMY, FALSE))) {
				borg_keypress(I2A(b_i));

				/* Destroy one item */
				if (item->iqty >= 2) {
					borg_keypresses("01");
					borg_keypress('\r');
				}

				/* Verify destruction */
				borg_keypress('y');
				return (TRUE);

			} else {
				borg_keypress('k');
			}
		} else {
			/* worthless artifacts are dropped. */
			borg_keypress('d');

			/* mark the spot that the object was dropped so that  */
			/* it will not be picked up again. */
			bad_obj_x[bad_obj_num] = c_x;
			bad_obj_y[bad_obj_num] = c_y;
			borg_note(format("# Crappy artifact at %d,%d", bad_obj_x[bad_obj_num],
								  bad_obj_y[bad_obj_num]));
			bad_obj_num++;
		}
		borg_keypress(I2A(b_i));

		return (TRUE);
	}

	/* Hack -- no need */
	borg_do_crush_slow = FALSE;

	/* Nothing to destroy */
	return (FALSE);
}
/*
 * Attempt to suck charges from an item
 */
extern bool borg_eat_magic(bool cursed_only, int fail_allowed) {
	int i;
	int value;
	int b_value = -1;
	int b_i = -1;

	/* Have the capacity? */
	if (!borg_mutation(COR1_EAT_MAGIC, TRUE, fail_allowed, FALSE))
		return (FALSE);

	/* Look through the backpack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;
		if (!item->kind)
			continue;

		/* Skip non eatible items */
		if (item->tval != TV_ROD && item->tval != TV_STAFF &&
			 item->tval != TV_WAND)
			continue;

		/* only checking crap items */
		if (cursed_only && !item->value)
			continue;

		/* It needs to be charged */
		if (item->pval == 0)
			continue;

		/* Skip certain important ones */
		if ((item->tval == TV_STAFF && item->sval == SV_STAFF_TELEPORTATION) ||
			 (item->tval == TV_ROD && item->sval == SV_ROD_RECALL))
			continue;

		/* Amount of Health */
		if (item->tval == TV_ROD) {
			value = 2 * item->level;
		} else {
			value = item->pval * item->level;
		}

		/* Bonus if the staff sucks */
		if (!item->value)
			value *= 3;

		if (value > b_value) {
			value = b_value;
			b_i = i;
		}
	}

	/* Nothing good */
	if (b_i < 0)
		return (FALSE);

	/* Drain the Charges */
	if (borg_mutation(COR1_EAT_MAGIC, FALSE, fail_allowed, FALSE)) {
		/* Select the item to drain */
		borg_keypress(I2A(b_i));
		return (TRUE);
	}

	/* Assume can't do it */
	return (FALSE);
}

/*
 * Pseuo Identify items if possible, using the Mindcraft power
 */

bool borg_pseudoid_stuff(void) {
	int i, b_i = -1;

	/* don't ID stuff when you can't recover spent spell point immediately */
	if (borg_skill[BI_CURSP] < 15 || !borg_mindcr_legal(MIND_PSYCHOMETRY, 15) ||
		 !borg_check_rest(c_y, c_x))
		return (FALSE);

	/* Must have pseudo not regular ID */
	if (borg_mindcr_legal(MIND_PSYCHOMETRY, 25))
		return (FALSE);

	/* Look for an item to identify (inventory) */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip known items */
		if (item->ident)
			continue;
		if (item->aware)
			continue;

		/* Wearable items and ammo */
		if (item->tval < TV_SHOT || item->tval > TV_RING || item->tval == TV_LITE)
			continue;

		/* skip weak pseudo id */
		if (strstr(item->desc, "{average"))
			continue;
		else if (strstr(item->desc, "{curse"))
			continue;
		else if (strstr(item->desc, "{good"))
			continue;
		else if (strstr(item->desc, "{excellent"))
			continue;
		else if (strstr(item->desc, "Quest"))
			continue;
		else if (strstr(item->desc, "special"))
			continue;
		else if (strstr(item->desc, "{terrible"))
			continue;
		else if (strstr(item->desc, "{indestructible"))
			continue;

		/* Track it */
		b_i = i;
	}
	/* Found something */
	if (b_i >= 0) {
		borg_item *item = &borg_items[b_i];

		if (borg_mindcr(MIND_PSYCHOMETRY, 15)) {
			/* Log -- may be cancelled */
			borg_note(format("# Pseudo IDing %s.", item->desc));

			/* Select the item */
			borg_keypress(I2A(b_i));

			/* press enter a few time (get rid of display) */
			borg_keypress('\r');
			borg_keypress('\r');
			borg_keypress('\r');
			borg_keypress('\r');
			borg_keypress(ESCAPE);

			/* Success */
			return (TRUE);
		}
	}

	/* Nothing to do */
	return (FALSE);
}

/* This is used to determine if certain items should be *ID*d.  It is called
 * from test_stuff as well as
 * a routine used to see if the borg needs to visit the Library to *ID* his gear
 */
bool borg_starid_item(borg_item *item) {
	/* Skip empty items */
	if (!item->iqty)
		return (FALSE);

	/* Accept certain items */
	if (strstr(item->note, "special"))
		return (TRUE);

	/* Skip *ID*'d items */
	if (item->fully_identified)
		return (FALSE);

	/* Only certain types need *ID* */
	if (item->name2) {
		switch (item->name2) {
		/* Weapon (Blessed) */
		case EGO_BLESS_BLADE:
		/* Armor of Permanence */
		case EGO_PERMANENCE:
		/* Armor of Elvenkind */
		case EGO_ELVENKIND:
		/* Crown of the Magi */
		case EGO_MAGI:
		/* Cloak of Aman */
		case EGO_AMAN:
		/* Weapon (Holy Avenger) */
		case EGO_HA:
		/* Weapon (Defender) */
		case EGO_DF:
		/* Trump weapons */
		case EGO_TRUMP:
		/* Chaotic weapon */
		case EGO_CHAOTIC:
		case EGO_SLAY_DRAGON:
		case EGO_KILL_DRAGON:
		case EGO_MIGHT:
		case EGO_LORDLINESS:
		case EGO_SEEING:
		case EGO_POWER:
		case EGO_RESISTANCE:
		case EGO_XTRA_POWER:
			/* anything else */
			return (TRUE);
		default:
			return (FALSE);
		}
	}
	if (item->name1) {
		switch (item->name1) {
		/* we will id all artifacts */
		default:
			return (TRUE);
			break;
		}
	}
	/* Artifacts */
	if (strstr(item->desc, "special") || strstr(item->desc, "terrible}") ||
		 strstr(item->desc, "indestructible}")) {
		/* Get the value */
		return (TRUE);
	}

	/* Assume not worth it */
	return (FALSE);
}

/*
 * Identify items if possible
 *
 * Note that "borg_parse()" will "cancel" the identification if it
 * detects a "You failed..." message.  This is VERY important!!!
 * Otherwise the "identify" might induce bizarre actions by sending
 * the "index" of an item as a command.
 *
 * Hack -- recover from mind blanking by re-identifying the equipment.
 *
 * We instantly identify items known to be "good" (or "terrible").
 *
 * We identify most un-aware items as soon as possible.
 *
 * We identify most un-known items as soon as possible.
 *
 * We play games with items that get "feelings" to try and wait for
 * "sensing" to take place if possible.
 *
 * XXX XXX XXX Make sure not to sell "non-aware" items, unless
 * we are really sure we want to lose them.  For example, we should
 * wait for feelings on (non-icky) wearable items or else make sure
 * that we identify them before we try and sell them.
 *
 * Mega-Hack -- the whole "sometimes identify things" code is a total
 * hack.  Slightly less bizarre would be some form of "occasionally,
 * pick a random item and identify it if necessary", which might lower
 * the preference for identifying items that appear early in the pack.
 * Also, preventing inventory motion would allow proper time-stamping.
 *
 * This function is also repeated to *ID* objects.  Right now only objects
 * with random high resist or random powers are *ID*'d
 */
bool borg_test_stuff(bool star_id) {
	int i, b_i = -1;
	s32b v, b_v = -1;
	/*bool OK_toID = FALSE;*/

	/* don't ID stuff when you can't recover spent spell point immediately */
	if ((!star_id) &&
		 ((borg_skill[BI_CURSP] < 50 && borg_spell_legal(REALM_ARCANE, 3, 2)) ||
		  (borg_skill[BI_CURSP] < 50 && borg_spell_legal(REALM_LIFE, 3, 5)) ||
		  (borg_skill[BI_CURSP] < 50 && borg_spell_legal(REALM_DEATH, 3, 2)) ||
		  (borg_skill[BI_CURSP] < 50 && borg_spell_legal(REALM_SORCERY, 1, 1))) &&
		 !borg_check_rest(c_y, c_x) &&
		 !borg_equips_activation(ACT_ID_PLAIN, TRUE))
		return (FALSE);

	/* No ID if in danger */
	if (borg_danger(c_y, c_x, 1, TRUE) > 1)
		return (FALSE);

	/* No need to check for *ID* if I don't have any source. */
	/* if (!borg_has[SCROLL_STAR_ID] && star_id) return (FALSE); */

	/* Look for an item to *identify* (equipment) */
	if (star_id) {
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			/* Assume nothing */
			v = -1;

			/* Wrong pass */
			if (!star_id)
				continue;

			/* Skip *ID*'d items */
			if (item->fully_identified)
				continue;

			/* go ahead and check egos and artifacts */
			if (borg_starid_item(item))
				v = item->value + 100000L;

			/* Track the best */
			if (v <= b_v)
				continue;

			/* Track it */
			b_i = i;
			b_v = v;
		}

		/* Look for an ego or artifact item to *identify* (inventory) */
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			/* Assume nothing */
			v = -1;

			if (item->fully_identified)
				continue;
			/* go ahead and check egos and artifacts */
			if (borg_starid_item(item))
				v = item->value + 100000L;

			/* Identify "good" (and "terrible") items */
			/* weak pseudo id */
			if (strstr(item->desc, "{good") && !star_id &&
				 (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST ||
				  borg_class == CLASS_RANGER))
				v = item->value + 10000L;
			/* heavy pseudo id */
			else if (strstr(item->desc, "{good") && !star_id && borg_gold < 10000)
				v = item->value + 1000L;
			else if (strstr(item->desc, "{excellent") && !star_id)
				v = item->value + 20000L;
			else if (strstr(item->desc, "Quest") && !star_id)
				v = item->value + 20000L;
			else if (strstr(item->desc, "special"))
				v = item->value + 50000L;
			else if (strstr(item->desc, "{terrible"))
				v = item->value + 50000L;
			else if (strstr(item->desc, "{indestructible"))
				v = item->value + 50000L;
			/* Nothing */
			if (!v)
				continue;

			/* Track the best */
			if (v <= b_v)
				continue;

			/* Track it */
			b_i = i;
			b_v = v;
		}
	}

	/* Normal ID */
	if (!star_id) {
		/* Look for a wielded item to identify (equipment) */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			/* Already done */
			if (item->ident)
				continue;

			/* Get the value */
			v = item->value + 1000L;

			/* Identify "good" items */
			/* weak pseudo id */
			if (strstr(item->desc, "good") && !star_id &&
				 (borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST ||
				  borg_class == CLASS_RANGER))
				v = item->value + 10000L;
			/* heavy pseudo id */
			else if (strstr(item->desc, "good") && !star_id && borg_gold < 10000)
				v = item->value + 1000L;
			else if (strstr(item->desc, "excellent") && !star_id)
				v = item->value + 20000L;
			else if (strstr(item->desc, "Quest") && !star_id)
				v = item->value + 20000L;
			else if (strstr(item->note, "special"))
				v = item->value + 50000L;

			/* Track the best */
			if (v <= b_v)
				continue;

			/* Track it */
			b_i = i;
			b_v = v;
		}

		/* Look for an item to identify (inventory) */
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			if (item->ident)
				continue;

			/* Hack -- never identify "average" things (except rings and ammies)*/
			if (strstr(item->note, "average") && !star_id &&
				 item->tval != TV_RING && item->tval != TV_AMULET)
				continue;

			/* Try to ID specials/ego right away.
			if (strstr(item->note, "special"))
				OK_toID = TRUE;
			if (strstr(item->note, "excellent") && !star_id)
				OK_toID = TRUE;
			if (strstr(item->note, "Quest") && !star_id)
				OK_toID = TRUE;
			if (strstr(item->note, "good"))
				OK_toID = TRUE;
			if (strstr(item->desc, "Phial") && !star_id && !item->ident)
				OK_toID = TRUE;
			*/

			/* Hack -- assume no value */
			v = -1;

			/* Ego and Artifacts have intrinsic value */
			if (strstr(item->note, "special") || strstr(item->desc, "Quest") ||
				 strstr(item->desc, "{excellent}"))
				v = (borg_skill[BI_MAXDEPTH] * 5000L);

			/* {good} have intrinsic value */
			if (strstr(item->note, "good"))
				v = (borg_skill[BI_MAXDEPTH] * 500L);

			/* Hack -- reward "unaware" items */
			if (!item->ident && !star_id) {
				/* Analyze the type */
				switch (item->tval) {
				case TV_RING:
				case TV_AMULET:

					/* Hack -- reward depth */
					v += (borg_skill[BI_MAXDEPTH] * 5000L);

					break;

				case TV_ROD:

					/* Hack -- reward depth */
					v += (borg_skill[BI_MAXDEPTH] * 3000L);

					break;

				case TV_WAND:
				case TV_STAFF:

					/* Hack -- reward depth */
					v += (borg_skill[BI_MAXDEPTH] * 2000L);

					break;

				case TV_POTION:
				case TV_SCROLL:

					/* Hack -- boring levels */
					if (borg_skill[BI_MAXDEPTH] < 5)
						break;

					/* Hack -- reward depth */
					v += (borg_skill[BI_MAXDEPTH] * 500L);

					break;

				case TV_FOOD:

					/* Hack -- reward depth */
					v += (borg_skill[BI_MAXDEPTH] * 10L);

					break;
				}
			}

			switch (item->tval) {
			case TV_CHEST:

				/* Hack -- Always identify chests */
				v += item->value;
				break;

			case TV_WAND:
			case TV_STAFF:

				/* Hack -- Always identify (get charges) */
				v += item->value;
				break;

			case TV_RING:
			case TV_AMULET:

				/* Hack -- Always identify (get information) */
				v += item->value;
				break;

			case TV_LITE:

				/* Hack -- Always identify (get artifact info) */
				v += item->value;
				break;

			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:

				/* Mega-Hack -- use identify spell/prayer */
				if (borg_spell_legal(REALM_SORCERY, 1, 1) ||
					 borg_spell_legal(REALM_ARCANE, 3, 2) ||
					 borg_equips_artifact(ART_ERIRIL) ||
					 borg_spell_legal(REALM_LIFE, 3, 5) ||
					 borg_spell_legal(REALM_DEATH, 3, 2) ||
					 borg_mindcr_legal(MIND_PSYCHOMETRY, 25) ||
					 borg_equips_activation(ACT_ID_PLAIN, TRUE)) {
					v += item->value;
				}

				/* Certain items needs ID'ing if low level */
				if (borg_skill[BI_CLEVEL] <= 5) {
					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}

				/* Mega-Hack -- mages get bored */
				if ((borg_class == CLASS_MAGE) &&
					 (rand_int(1000) < borg_skill[BI_CLEVEL])) {

					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}

				/* Mega-Hack -- rangers get bored */
				else if ((borg_class == CLASS_RANGER) &&
							(rand_int(3000) < borg_skill[BI_CLEVEL])) {

					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}

				/* Mega-Hack -- priests get bored */
				else if ((borg_class == CLASS_PRIEST) &&
							(rand_int(5000) < borg_skill[BI_CLEVEL])) {

					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}
				/* Mega-Hack -- Mindcrafters get bored */
				else if ((borg_class == CLASS_ORPHIC) &&
							(rand_int(5000) < borg_skill[BI_CLEVEL])) {

					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}
				/* Mega-Hack -- everyone else gets bored */
				else if (rand_int(5000) < borg_skill[BI_CLEVEL]) {

					/* Mega-Hack -- ignore "icky" items */
					if (!borg_item_icky(item))
						v += item->value;
				}

				/* try to ID shovels */
				if (item->tval == TV_DIGGING)
					v += item->value;

				break;
			}

			/* Ignore */
			if (!v)
				continue;

			/* Track the best */
			if (v <= b_v)
				continue;

			/* Track it */
			b_i = i;
			b_v = v;
		}
	}

	/* Found something */
	if (b_i >= 0) {
		borg_item *item = &borg_items[b_i];

		if (star_id) {
			if (borg_spell(REALM_SORCERY, 1, 7) ||
				 borg_spell(REALM_NATURE, 2, 5) || borg_spell(REALM_DEATH, 3, 2) ||
				 borg_spell(REALM_LIFE, 3, 5) ||
				 borg_read_scroll(SV_SCROLL_STAR_IDENTIFY) ||
				 borg_activate_activation(ACT_ID_FULL, FALSE)) {
				/* Log -- may be cancelled */
				borg_note(format("# *IDENTIFY*ing %s.", item->desc));

				/* we need to look at the screen next time through */
				borg_do_star_id = TRUE;
				item->needs_I_exam = TRUE;
				item->fully_identified = TRUE;

				/* Equipment */
				if (b_i >= INVEN_WIELD) {
					/* Select the equipment */
					borg_keypress('/');

					/* Select the item */
					borg_keypress(I2A(b_i - INVEN_WIELD));

					/* HACK need to recheck stats if we id something on us. */
					for (i = 0; i < 6; i++) {
						my_need_stat_check[i] = TRUE;
						my_stat_max[i] = 0;
					}
				}

				/* Inventory */
				else {
					/* Select the item */
					borg_keypress(I2A(b_i));
				}

				/* press enter a few time (get rid of display) */
				borg_keypress('\r');
				borg_keypress('\r');
				borg_keypress('\r');
				borg_keypress('\r');
				borg_keypress(ESCAPE);

				/* Success */
				return (TRUE);
			}

		} else {
			/* Use a Spell/Prayer/Rod/Staff/Scroll of Identify */
			if (borg_spell(REALM_SORCERY, 1, 1) ||
				 borg_spell(REALM_ARCANE, 3, 2) || borg_spell(REALM_DEATH, 3, 2) ||
				 borg_mindcr(MIND_PSYCHOMETRY, 25) ||
				 borg_zap_rod(SV_ROD_IDENTIFY) || borg_spell(REALM_LIFE, 3, 5) ||
				 borg_activate_artifact(ART_ERIRIL, FALSE) ||
				 borg_use_staff(SV_STAFF_IDENTIFY) ||
				 borg_read_scroll(SV_SCROLL_IDENTIFY) ||
				 borg_activate_activation(ACT_ID_PLAIN, FALSE)) {
				/* Log -- may be cancelled */
				borg_note(format("# Identifying %s.", item->desc));

				/* Equipment */
				if (b_i >= INVEN_WIELD) {
					/* Select the equipment */
					borg_keypress('/');

					/* Select the item */
					borg_keypress(I2A(b_i - INVEN_WIELD));

					/* HACK need to recheck stats if we id something on us. */
					for (i = 0; i < 6; i++) {
						my_need_stat_check[i] = TRUE;
						my_stat_max[i] = 0;
					}
				}

				/* Inventory */
				else {
					/* Select the item */
					borg_keypress(I2A(b_i));
				}

				borg_keypress(ESCAPE);
				/* Success */
				return (TRUE);
			}
		}
	}

	/* Nothing to do */
	return (FALSE);
}

/*
 * This function is responsible for making sure that, if possible,
 * the "best" ring we have is always on the "right" (tight) finger,
 * so that the other functions, such as "borg_best_stuff()", do not
 * have to think about the "tight" ring, but instead, can just assume
 * that the "right" ring is "good for us" and should never be removed.
 *
 * In general, this will mean that our "best" ring of speed will end
 * up on the "right" finger, if we have one, or a ring of free action,
 * or a ring of see invisible, or some other "useful" ring.
 *
 */
bool borg_swap_rings(void) {
	int hole = INVEN_PACK - 1;
	int icky = INVEN_PACK - 2;

	s32b v1, v2;

	bool fix = FALSE;

	/*** Check conditions ***/

	/* Require two empty slots */
	if (borg_items[icky].iqty)
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 350)
		return (FALSE);
	if (time_this_panel > 350)
		return (FALSE);

	if (borg_items[INVEN_LEFT].cursed || borg_items[INVEN_RIGHT].cursed)
		return (FALSE);

	/*** Remove naked "loose" rings ***/

	/* Remove any naked loose ring */
	if (borg_items[INVEN_LEFT].iqty && !borg_items[INVEN_RIGHT].iqty &&
		 borg_items[INVEN_LEFT].name1 != ART_FIRST) {
		/* Log */
		borg_note("# Taking off naked loose ring.");

		/* Remove it */
		borg_keypress('t');
		borg_keypress(I2A(INVEN_LEFT - INVEN_WIELD));

		/* Success */
		return (TRUE);
	}

	/*** Check conditions ***/

	/* Require "tight" ring */
	if (!borg_items[INVEN_RIGHT].iqty)
		return (FALSE);

	/* Cannot remove the One Ring */
	if (!(borg_items[INVEN_RIGHT].name1 == ART_FIRST))
		return (FALSE);

	/* not if we have already done this on this level */
	if (borg_swapped_rings)
		return (FALSE);

	/*** Remove nasty "tight" rings ***/

	/* Save the hole */
	COPY(&safe_items[hole], &borg_items[hole], borg_item);

	/* Save the ring */
	COPY(&safe_items[INVEN_LEFT], &borg_items[INVEN_LEFT], borg_item);

	/* Take off the ring */
	COPY(&borg_items[hole], &borg_items[INVEN_LEFT], borg_item);

	/* Erase left ring */
	WIPE(&borg_items[INVEN_LEFT], borg_item);

	/* Fix later */
	fix = TRUE;

	/* Examine the inventory */
	borg_notice(FALSE);

	/* Evaluate the inventory */
	v1 = borg_power();

	/* Restore the ring */
	COPY(&borg_items[INVEN_LEFT], &safe_items[INVEN_LEFT], borg_item);

	/* Restore the hole */
	COPY(&borg_items[hole], &safe_items[hole], borg_item);

	/*** Consider taking off the "right" ring ***/

	/* Save the hole */
	COPY(&safe_items[hole], &borg_items[hole], borg_item);

	/* Save the ring */
	COPY(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT], borg_item);

	/* Take off the ring */
	COPY(&borg_items[hole], &borg_items[INVEN_RIGHT], borg_item);

	/* Erase the ring */
	WIPE(&borg_items[INVEN_RIGHT], borg_item);

	/* Fix later */
	fix = TRUE;

	/* Examine the inventory */
	borg_notice(FALSE);

	/* Evaluate the inventory */
	v2 = borg_power();

	/* Restore the ring */
	COPY(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT], borg_item);

	/* Restore the hole */
	COPY(&borg_items[hole], &safe_items[hole], borg_item);

	/*** Swap rings if necessary ***/

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Remove "useless" ring */
	if (v2 > v1) {
		/* Log */
		borg_note("# Taking off nasty tight ring.");

		/* Take it off */
		borg_keypress('t');
		borg_keypress(I2A(INVEN_RIGHT - INVEN_WIELD));

		/* set a flag so we don't do it again this level */
		borg_swapped_rings = TRUE;

		/* Success */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Place our "best" ring on the "tight" finger if needed
 *
 * This function is adopted from "borg_wear_stuff()"
 *
 * Basically, we evaluate the world in which each ring is added
 * to the current set of equipment, and we wear the ring, if any,
 * that gives us the most "power".
 *
 * The "borg_swap_rings()" code above occasionally allows us to remove
 * both rings, at which point this function will place the "best" ring
 * on the "tight" finger, and then the "borg_best_stuff()" function will
 * allow us to put on our second "best" ring on the "loose" finger.
 *
 * This function should only be used when a ring finger is empty.
 */
bool borg_wear_rings(void) {
	int slot;
	int hole = INVEN_PACK - 1;

	s32b p, b_p = my_power;

	int i, b_i = -1;

	borg_item *item;

	bool fix = FALSE;

	/* Require no rings */
	if (borg_items[INVEN_LEFT].iqty)
		return (FALSE);
	if (borg_items[INVEN_RIGHT].iqty)
		return (FALSE);

	/* Require two empty slots */
	if (borg_items[hole - 1].iqty)
		return (FALSE);

	/* hack prevent the swap till you drop loop */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Don't do this during temp effects that significantly alter our power and
	 * fear */
	if (borg_goi || borg_wraith)
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 2000)
		return (FALSE);
	if (time_this_panel > 300)
		return (FALSE);

	/* Scan inventory */
	for (i = 0; i < INVEN_PACK; i++) {
		item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Require "known" (or average) */
		if (!item->aware && !strstr(item->note, "average"))
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* skip artifact rings not star id'd  */
		if (/* adult_rand_artifacts && */ !item->fully_identified && item->name1)
			continue;

		/* Where does it go */
		slot = borg_wield_slot(item);

		/* Only process "rings" */
		if (slot != INVEN_LEFT)
			continue;

		/* Occassionally evaluate swapping into the tight finger */
		if (rand_int(100) > 75 || item->name1 == ART_FIRST) {
			slot = INVEN_RIGHT;
		}

		/* Need to be careful not to put the One Ring onto
		 * the Left Hand
		 */
		if (item->name1 == ART_FIRST && (borg_items[INVEN_RIGHT].iqty)) {
			/* Log */
			borg_note("# Taking off nasty tight ring to make room for One Ring.");

			/* Take it off */
			borg_keypress('t');
			borg_keypress(I2A(INVEN_RIGHT - INVEN_WIELD));

			/* Success */
			return (TRUE);
		}

		/* Save the old item (empty) */
		COPY(&safe_items[slot], &borg_items[slot], borg_item);

		/* Save the new item */
		COPY(&safe_items[i], &borg_items[i], borg_item);

		/* Wear new item */
		COPY(&borg_items[slot], &safe_items[i], borg_item);

		/* Only a single item */
		borg_items[slot].iqty = 1;

		/* Reduce the inventory quantity by one */
		borg_items[i].iqty--;

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory */
		borg_notice(FALSE);

		/* Evaluate the inventory */
		p = borg_power();

		/* Restore the old item (empty) */
		COPY(&borg_items[slot], &safe_items[slot], borg_item);

		/* Restore the new item */
		COPY(&borg_items[i], &safe_items[i], borg_item);

		/* Ignore "bad" swaps */
		if ((b_i >= 0) && (p < b_p))
			continue;

		/* Maintain the "best" */
		b_i = i;
		b_p = p;
	}

	/* Restore bonuses */
	if (fix)
		borg_notice(TRUE);

	/* No item */
	if ((b_i >= 0) && (b_p > my_power)) {
		/* Get the item */
		item = &borg_items[b_i];

		/* Log */
		borg_note("# Putting on best tight ring.");

		/* Log */
		borg_note(format("# Wearing %s.", item->desc));

		/* Wear it */
		borg_keypress('w');
		borg_keypress(I2A(b_i));

		/* Did something */
		time_this_panel++;
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Place our "swap" if needed.   We check both the armour and the weapon
 * then wear the one that give the best result (lowest danger).
 * This function is adopted from "borg_wear_stuff()" and borg_wear_rings
 *
 * Basically, we evaluate the world in which the swap is added
 * to the current set of equipment, and we use weapon,
 * that gives the largest drop in danger---based mostly on resists.
 *
 */
bool borg_backup_swap(int p) {
	int slot;
	int swap;
	int damage1 = 0;
	int damage2 = 0;
	int location[2];
	/*int hole;*/
	s32b b_p = 0L;
	/*s32b b_p1 = 0L;
	  s32b b_p2 = 0L;*/
	int i;

	int save_rconf = 0;
	int save_rblind = 0;
	int save_fract = 0;
	int save_esp = 0;
	int save_seeinvis = 0;

	/*int n;*/
	/*int b_n = -1;*/
	/*int danger;*/
	/*int b_danger = -1;
	int damage = 0;
	int n_damage = 0;*/

	s32b my_danger = p;
	int b_i = -1;

	borg_item *item;
	/*borg_grid *ag;*/

	/*borg_kill *kill;*/

	bool fix = FALSE;

	b_p = my_danger;

	/* hack prevent the swap till you drop loop */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (time_this_panel > 300)
		return (FALSE);

	/* make sure we have an appropriate swap */
	if (armour_swap < 1 && weapon_swap < 1)
		return (FALSE);

	/* Save our normal condition */
	save_rconf = borg_skill[BI_RCONF];
	save_rblind = borg_skill[BI_RBLIND];
	save_fract = borg_skill[BI_FRACT];
	save_seeinvis = borg_skill[BI_SINV];
	save_esp = borg_skill[BI_ESP];

	/* Keep track of immediate threats in combat.  Used in considering swap
	 * items. */
	borg_threat_conf = FALSE;
	borg_threat_blind = FALSE;
	borg_threat_para = FALSE;
	borg_threat_invis = FALSE;

	/* Check the items, first armour then weapon */
	location[0] = weapon_swap;
	location[1] = armour_swap;

	/* Consider the two swap items */
	for (i = 0; i <= 1; i++) {
		/* Careful */
		if (location[i] == -1)
			continue;

		item = &borg_items[location[i]];

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Zang has some very bad curses, do not wear items
		 * that are not ID'd
		 */
		/* Require "known" (or average, good, etc) */
		if (!item->aware && !strstr(item->note, "average") &&
			 !strstr(item->note, "good") && !strstr(item->note, "excellent") &&
			 !strstr(item->note, "Quest") &&
			 !strstr(item->note, "indestructible") &&
			 !strstr(item->note, "special"))
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* Do not wear non *ID*'d artifacts */
		if (!item->fully_identified && item->name1)
			continue;

		/* skip it if it has not been decursed */
		if (item->cursed)
			continue;

		/* Mages with GOI should not be swapping armor for better protection */
		if ((borg_goi || borg_wraith) && i == 1)
			continue;

		/* Where does it go */
		slot = borg_wield_slot(item);

		/* safety check incase slot = -1 */
		if (slot < 0)
			return (FALSE);

		/* skip it if it this slot has been decursed */
		if (borg_items[slot].cursed)
			continue;

		/* Now we check the weapon damage before the swap */
		if ((borg_goi || borg_wraith) && i == 0) {
			/* we will borrow the attack routines */
			borg_simulate = TRUE;
			damage1 = borg_attack_aux_thrust(TRUE, -1);
			borg_simulate = FALSE;
		}

		/* The item being worn will take the place of the swap item in the inven.
		 */
		/*hole = location[i];*/

		/* Save the old item (empty) */
		COPY(&safe_items[slot], &borg_items[slot], borg_item);

		/* Save the new item */
		COPY(&safe_items[location[i]], &borg_items[location[i]], borg_item);

		/* Wear new item */
		COPY(&borg_items[slot], &safe_items[location[i]], borg_item);

		/* Only a single item */
		borg_items[slot].iqty = 1;

		/* Reduce the inventory quantity by one */
		borg_items[i].iqty--;

		/* Fix later */
		fix = TRUE;

		/* Examine the benefits of the swap item */
		borg_notice(FALSE);

		/* Evaluate the power with the new item worn */
		p = borg_danger(c_y, c_x, 1, TRUE);

		/* Note the considerations if fighting a unique */
		if (borg_fighting_unique && borg_verbose) {
			/* dump list and power...  for debugging */
			borg_note(format("Swap: Trying Item %s (new power %ld)",
								  borg_items[slot].desc, p));
			borg_note(format("Swap: Against Item  %s  (old power %ld)",
								  safe_items[slot].desc, b_p));
		}

		/* Now we check the weapon damage after the swap */
		if ((borg_goi || borg_wraith) && i == 0) {
			/* we will borrow the attack routines */
			borg_simulate = TRUE;
			damage2 = borg_attack_aux_thrust(TRUE, -1);
			borg_simulate = FALSE;
		}

		/* Examine the critical skills with new gear on.  If i lose a critical
		 * skill, then boost the danger */
		if ((save_rconf) && borg_threat_conf && borg_skill[BI_RCONF] == 0)
			p = 9999;
		if ((save_rblind) && borg_threat_blind &&
			 (!borg_skill[BI_RBLIND] && !borg_skill[BI_RLITE] &&
			  !borg_skill[BI_RDARK] && borg_skill[BI_SAV] < 100))
			p = 9999;
		if ((save_fract) && (!borg_skill[BI_FRACT] && borg_skill[BI_SAV] < 100 &&
									borg_threat_para))
			p = 9999;
		if ((save_seeinvis || save_esp) && borg_threat_invis &&
			 (!borg_skill[BI_ESP] && !borg_skill[BI_SINV]))
			p = 9999;

		/* Restore the old item (empty) */
		COPY(&borg_items[slot], &safe_items[slot], borg_item);

		/* Restore the new item */
		COPY(&borg_items[i], &safe_items[i], borg_item);

		/* Ignore "bad" swaps */
		if (p < b_p)
			continue;

		/* Maintain the "best" */
		b_i = location[i];
		b_p = p;
	}

	/* Restore bonuses */
	if (fix)
		borg_notice(TRUE);

	/* Pass on the swap which yields the best result */
	swap = location[b_i];

	/* Consider the weapon swap for GOI to maximize damage */
	if ((borg_goi || borg_wraith) && damage2 > damage1 &&
		 !borg_skill[BI_NO_MELEE]) {
		swap = weapon_swap;

		/* log it */
		borg_note(format("# Swapping backup for more damage.  (%d > %d).",
							  damage2, damage1));

		/* Wear it */
		borg_keypress('w');
		borg_keypress(I2A(swap));

		/* Did something */
		return (TRUE);
	}

	/* good swap.  Make sure it helps a significant amount */
	if (b_p < my_danger * 9 / 10 &&
		 b_p <=
			  (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2))) {
		/* Log */
		borg_note(format("# Swapping backup.  (%d < %d).", b_p, my_danger));

		/* Wear it */
		borg_keypress('w');
		borg_keypress(I2A(swap));

		/* Did something */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Remove useless equipment.
 *
 * Look through the inventory for equipment that is reducing power.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are removed, and we take
 * one step towards the world in which we have the most "power".
 */
bool borg_remove_stuff(void) {
	int hole = INVEN_PACK - 1;

	s32b p, b_p = 0L;

	int i, b_i = -1;

	borg_item *item;

	bool fix = FALSE;

	/* hack to prevent the swap till you drop loop */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Dont unload our equipment while encumbered; we'll crush it */
	if (borg_skill[BI_ENCUMBERD])
		return (FALSE);

	/* Dont do this if inven slot w is full */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 2000)
		return (FALSE);
	if (time_this_panel > 150)
		return (FALSE);

	/* Dont bother under certain temp effects. */
	if (borg_goi || borg_wraith)
		return (FALSE);

	/* Start with good power */
	borg_notice(FALSE);
	b_p = borg_power();

	/* Scan equip */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Require "known" (or average, good, etc) */
		if (!item->aware || !item->ident || strstr(item->note, "average") ||
			 strstr(item->note, "good") || strstr(item->note, "excellent") ||
			 strstr(item->note, "Quest") || strstr(item->note, "indestructible") ||
			 strstr(item->note, "special"))
			continue;

		/* skip it if it has not been decursed */
		if ((item->cursed) || (item->flags3 & TR3_HEAVY_CURSE) ||
			 (inventory[i].ident & IDENT_CURSED))
			continue;

		/* Skip the empty lantern.  We will buy some fuel */
		if (i == INVEN_LITE)
			continue;

		/* Save the hole */
		COPY(&safe_items[hole], &borg_items[hole], borg_item);

		/* Save the item */
		COPY(&safe_items[i], &borg_items[i], borg_item);

		/* Take off the item */
		COPY(&borg_items[hole], &safe_items[i], borg_item);

		/* Erase the item */
		WIPE(&borg_items[i], borg_item);

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory */
		borg_notice(FALSE);

		/* Evaluate the inventory */
		p = borg_power();

		if (borg_verbose) {
			/* dump list and power...  for debugging */
			borg_note(format("Equip Item %d %s.", i, safe_items[i].desc));
			borg_note(format("With Item     (borg_power %ld)", b_p));
			borg_note(format("Removed Item  (best power %ld)", p));
		}

		/* Restore the item */
		COPY(&borg_items[i], &safe_items[i], borg_item);

		/* Restore the hole */
		COPY(&borg_items[hole], &safe_items[hole], borg_item);

		/* Track the crappy items */
		/* crappy includes things that do not add to power */

		if (p >= b_p) {
			b_i = i;
		}
	}

	/* Restore bonuses */
	if (fix)
		borg_notice(TRUE);

	/* No item */
	if (b_i >= 0) {
		/* Get the item */
		item = &borg_items[b_i];

		/* Log */
		borg_note(format("# Removing %s.  Old Power (%ld) New Power (%ld)",
							  item->desc, my_power, b_p));

		/* Wear it */
		borg_keypress('t');
		borg_keypress(I2A(b_i - INVEN_WIELD));

		/* Did something */
		time_this_panel++;
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Wear useful equipment.
 *
 * Look through the inventory for equipment that is better than
 * the current equipment, and wear it, in an optimal order.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are used instead of the items they would replace, and we take
 * one step towards the world in which we have the most "power".
 *
 * The "borg_swap_rings()" code above occasionally allows us to remove
 * both rings, at which point this function will replace the "best" ring
 * on the "tight" finger, and the second "best" ring on the "loose" finger.
 *
 * Since Monks have some bonuses for going naked, we consider an option of going
 * naked.
 */
bool borg_wear_stuff(void) {
	int hole = INVEN_PACK - 1;

	int slot;
	int d;
	bool recently_worn = FALSE;

	/* Store our ammo tval so we don't swap out our bow */
	int good_ammo = my_ammo_tval;

	s32b p, first_power, b_p = 0L;

	int i, b_i = -1;
	int ii, b_ii = -1;
	int o, danger;

	borg_item *item;

	bool fix = FALSE;
	bool monk_disarm = FALSE;
	int track;

	/* Require an empty slot */
	if (borg_items[hole].iqty)
		return (FALSE);

	/*  hack to prevent the swap till you drop loop */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 2000)
		return (FALSE);
	if (borg_t - time_last_swap < 15)
		return (FALSE);

	/* Don't do this during temp effects that significantly alter our power and
	 * fear */
	if (borg_goi || borg_wraith)
		return (FALSE);

	/* Evaluate the base inventory */
	borg_notice(FALSE);
	b_p = borg_power();
	first_power = b_p;

	/*** Process a monk wielding a digger.  He might want to drop it ***/
	if (borg_class == CLASS_MONK && borg_items[INVEN_WIELD].tval == TV_DIGGING) {
		/* Pretend the weapon slot is empty */
		borg_items[INVEN_WIELD].iqty = 0;

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory */
		borg_notice(FALSE);

		/* Evaluate the inventory */
		p = borg_power();

		/* Evaluate local danger */
		d = borg_danger(c_y, c_x, 1, TRUE);

		/* Restore the old item */
		borg_items[INVEN_WIELD].iqty = 1;

		/* Going unarmed is better than wielding the digger */
		if (p > b_p) {
			b_i = hole;
			monk_disarm = TRUE;
		}
	} /* monk */

	/* Scan inventory */
	for (i = 0; i < INVEN_PACK; i++) {
		item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Reset the worn flag */
		recently_worn = FALSE;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Zang has some very bad curses, do not wear items
		 * that are not ID'd
		 */
		/* Require "known" (or average, good, etc) */
		if (!item->aware && !strstr(item->note, "average") &&
			 !strstr(item->note, "good") && !strstr(item->note, "excellent") &&
			 !strstr(item->note, "Quest") &&
			 !strstr(item->note, "indestructible") &&
			 !strstr(item->note, "special"))
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* Do not wear non *ID*'d artifacts */
		if (!item->fully_identified && item->name1 && item->name1 != ART_FIRST)
			continue;

		/* skip it if it has not been decursed, unless the One Ring */
		if (((item->cursed) || (item->flags3 & TR3_HEAVY_CURSE) ||
			  (inventory[i].ident & IDENT_CURSED)) &&
			 (item->name1 != ART_FIRST))
			continue;

		/* Do not consider wearing this item if I worn it already this level,
		 * I might be stuck in a loop.
		 */
		for (o = 0; o < track_worn_num; o++) {
			/* Examine the worn list */
			if (track_worn_num >= 1 && track_worn_name1[o] == item->name1 &&
				 track_worn_name2[o] == item->name2 &&
				 track_worn_tval[o] == item->tval &&
				 track_worn_sval[o] == item->sval &&
				 track_worn_pval[o] == item->pval &&
				 track_worn_time > borg_t - 10) {
				/* Recently worn item */
				recently_worn = TRUE;
			}
		}

		/* Note and fail out */
		if (recently_worn == TRUE) {
			if (borg_verbose)
				borg_note(format("# Not considering a %s; it was recently worn.",
									  item->desc));
			continue;
		}

		/* Where does it go */
		slot = borg_wield_slot(item);

		/* Cannot wear this item */
		if (slot < 0)
			continue;

		/* Do not wear certain items if I am over weight limit.  It induces loops
		 */
		if (borg_skill[BI_ENCUMBERD]) {
			/* Compare Str bonuses */
			if (((borg_items[slot].flags1 & TR1_STR) &&
				  !(item->flags1 & TR1_STR)) ||
				 ((borg_items[slot].flags1 & TR1_STR) && (item->flags1 & TR1_STR) &&
				  borg_items[slot].pval > item->pval)) {
				/* Not a ring slot */
				if (slot != INVEN_LEFT)
					continue;

				/* If it is a ring, check the right ring for STR as well */
				if ((slot == INVEN_LEFT &&
					  (borg_items[INVEN_RIGHT].flags1 & TR1_STR) &&
					  !(item->flags1 & TR1_STR)) ||
					 (slot == INVEN_LEFT &&
					  (borg_items[INVEN_RIGHT].flags1 & TR1_STR) &&
					  (item->flags1 & TR1_STR) &&
					  borg_items[INVEN_RIGHT].pval > item->pval))
					continue;
			}
		}

		/* skip it if it this slot has been decursed */
		if (borg_items[slot].cursed)
			continue;

		/* Need to be careful not to put the One Ring onto
			* the Left Hand
			*/
		if (item->name1 == ART_FIRST && (borg_items[INVEN_RIGHT].iqty))
			continue;

		/* Obtain danger */
		danger = borg_danger(c_y, c_x, 1, TRUE);

		/* If this is a ring and both hands are full, then check each hand
		 * and compare the two.  If needed, the tight ring can be removed then
		 * the better ring placed there on.
		 */

		/*** Process regular items and non full rings ***/

		/* Non ring, non full hands */
		if (slot != INVEN_LEFT ||
			 (!borg_items[INVEN_LEFT].tval || !borg_items[INVEN_RIGHT].tval)) {
			/* Left hand full. right hand empty. test ring in right hand */
			if (slot == INVEN_LEFT && borg_items[INVEN_LEFT].iqty &&
				 !borg_items[INVEN_RIGHT].iqty)
				slot = INVEN_RIGHT;

			/* Save the old item */
			COPY(&safe_items[slot], &borg_items[slot], borg_item);

			/* Save the new item */
			COPY(&safe_items[i], &borg_items[i], borg_item);

			/* Save the hole */
			COPY(&safe_items[hole], &borg_items[hole], borg_item);

			/* Take off old item */
			COPY(&borg_items[hole], &safe_items[slot], borg_item);

			/* Wear new item */
			COPY(&borg_items[slot], &safe_items[i], borg_item);

			/* Only a single item */
			borg_items[slot].iqty = 1;

			/* Reduce the inventory quantity by one */
			borg_items[i].iqty--;

			/* Fix later */
			fix = TRUE;

			/* Examine the inventory */
			borg_notice(FALSE);

			/* Evaluate the possible inventory */
			p = borg_power();

			/* Evaluate local danger */
			d = borg_danger(c_y, c_x, 1, TRUE);
#if 0
            /* dump list and power...  for debugging */
            borg_note(format("Trying Item %s (power %ld)",borg_items[slot].desc, p));
            borg_note(format("Against Item %s   (power %ld)",safe_items[slot].desc, b_p));
#endif

			/* Restore the old item */
			COPY(&borg_items[slot], &safe_items[slot], borg_item);

			/* Restore the new item */
			COPY(&borg_items[i], &safe_items[i], borg_item);

			/* Restore the hole */
			COPY(&borg_items[hole], &safe_items[hole], borg_item);

			/*
			 * If the borg happens to be holding some ammo which does not match his
			 * bow (bolts vs shot),
			 * and he finds a new bow to match the ammo, he will mistakenly swap
			 * out the good bow for
			 * the new one becase the ammo matches.  After doing this simulated
			 * check, he just defined a
			 * new my_ammo_type.  So we compare this new value to the original
			 * tval.
			 * We want him to avoid this equipment change because he ends up
			 * his nice bow for the junky
			 */
			if (slot == INVEN_BOW && my_ammo_tval != good_ammo && good_ammo)
				continue;

			/* Ignore if more dangerous */
			if (danger < d)
				continue;

			/* XXX XXX XXX Consider if slot is empty */

			/* Hack -- Ignore "equal" swaps */
			if ((p <= b_p))
				continue;

			/* Maintain the "best" */
			b_i = i;
			b_p = p;

		} /* non-rings, non full */

		/* ring, full hands */
		if (slot == INVEN_LEFT && borg_items[INVEN_LEFT].tval &&
			 borg_items[INVEN_RIGHT].tval) {
			for (ii = INVEN_LEFT; ii <= INVEN_RIGHT; ii++) {
				slot = ii;

				/* Does One Ring need to be handled here? */

				/* Skip this slot if ring is cursed */
				if (borg_items[slot].cursed)
					continue;

				/* Save the old item */
				COPY(&safe_items[slot], &borg_items[slot], borg_item);

				/* Save the new item */
				COPY(&safe_items[i], &borg_items[i], borg_item);

				/* Save the hole */
				COPY(&safe_items[hole], &borg_items[hole], borg_item);

				/* Take off old item */
				COPY(&borg_items[hole], &safe_items[slot], borg_item);

				/* Wear new item */
				COPY(&borg_items[slot], &safe_items[i], borg_item);

				/* Only a single item */
				borg_items[slot].iqty = 1;

				/* Reduce the inventory quantity by one */
				borg_items[i].iqty--;

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the inventory */
				p = borg_power();

				/* Evaluate local danger */
				d = borg_danger(c_y, c_x, 1, TRUE);

#if 0
                    /* dump list and power...  for debugging */
                    borg_note(format("Ring: Trying Item %s (power %ld)",borg_items[slot].desc, b_p));
                    borg_note(format("Ring: Against Item  %s  (power %ld)",safe_items[slot].desc, p));
#endif

				/* Restore the old item */
				COPY(&borg_items[slot], &safe_items[slot], borg_item);

				/* Restore the new item */
				COPY(&borg_items[i], &safe_items[i], borg_item);

				/* Restore the hole */
				COPY(&borg_items[hole], &safe_items[hole], borg_item);

				/* Ignore "bad" or neutral swaps */
				if ((p <= b_p))
					continue;

				/* no swapping into more danger */
				if (danger < d)
					continue;

				/* Maintain the "best" */
				b_i = i;
				b_p = p;
				b_ii = ii;
			}
		} /* ring, looking at replacing each ring */
	}	 /* end scanning inventory */

	/* Restore bonuses */
	if (fix)
		borg_notice(TRUE);

	/* Change gear */
	if (b_i >= 0) {
		/* Get the item */
		item = &borg_items[b_i];

		/* Allow Monk to disarm */
		if (monk_disarm) {
			/* Log */
			borg_note(format("# Removing %s to go as unarmed monk.",
								  &borg_items[INVEN_WIELD].desc));

			/* remove it */
			borg_keypress('t');
			borg_keypress('a');
			return (TRUE);
		}

		/* Remove old ring to make room for good one */
		if (b_ii >= INVEN_RIGHT && item->tval == TV_RING &&
			 borg_items[INVEN_RIGHT].iqty) {
			/* Log */
			borg_note(format("# Removing %s to make room for %s.",
								  &borg_items[b_ii].desc, item->desc));

			/* Wear it */
			borg_keypress('t');
			borg_keypress(I2A(b_ii - INVEN_WIELD));

			/*
			 * Once the ring is removed the inventory location of the desired ring
			 * may change.
			 */
			return (TRUE);
		}

		/* Log */
		borg_note(format("# Wearing %s.  Old Power (%ld) New Power (%ld)",
							  item->desc, first_power, b_p));

		/* Wear it */
		borg_keypress('w');
		borg_keypress(I2A(b_i));

		/* Track the newly worn artifact item to avoid loops */
		if (track_worn_num <= track_worn_size) {
			track_worn_tval[track_worn_num] = item->tval;
			track_worn_sval[track_worn_num] = item->sval;
			track_worn_pval[track_worn_num] = item->pval;
			track_worn_name1[track_worn_num] = item->name1;
			track_worn_name2[track_worn_num] = item->name2;
			track_worn_time = borg_t;
			track_worn_num++;

			/* Step down the list if needed */
			if (track_worn_num >= track_worn_size) {
				for (track = 0; track < track_worn_size; track++) {
					track_worn_tval[track] = track_worn_tval[track + 1];
					track_worn_sval[track] = track_worn_sval[track + 1];
					track_worn_pval[track] = track_worn_pval[track + 1];
					track_worn_name1[track] = track_worn_name1[track + 1];
					track_worn_name2[track] = track_worn_name2[track + 1];
					track_worn_time = borg_t;
				}

				/* Don't let it overrun */
				track_worn_num = track_worn_size;
			}
		}

		/* Did something */
		time_this_panel++;
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Wear useful swap equipment.
 * Mainly used to improve the damage for a certain type of monster
 *
 * Look at the two swap items and see if it is better than
 * the current equipment, and wear it, in an optimal order.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are used instead of the items they would replace, and we take
 * one step towards the world in which we have the most "power".
 *
 */
bool borg_wear_swap(void) {
	int hole = INVEN_PACK - 1;

	int slot;

	s32b p, b_p = 0L;

	int i, b_i = -1;
	int ii, b_ii = -1;
	int b_damage = 1;
	int location[2];
	int damage = 0;
	int n_damage[2];
	int n;
	int b_n = -1;
	int danger;
	int b_danger = -1;

	borg_item *item;
	borg_grid *ag;
	borg_kill *kill;

	bool fix = FALSE;
	int save_rconf = 0;
	int save_rblind = 0;
	int save_fract = 0;
	int save_esp = 0;
	int save_seeinvis = 0;

	/*  hack to prevent the swap till you drop loop */
	if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 2000)
		return (FALSE);
	if (time_this_panel > 300)
		return (FALSE);
	if (borg_t - time_last_swap < 15)
		return (FALSE);

	/* No need if borg is not using swaps. */
	if (!borg_uses_swaps)
		return (FALSE);

	/* its all about damage, some borgs dont melee well */
	if (borg_skill[BI_NO_MELEE])
		return (FALSE);

	/* Save our normal condition */
	save_rconf = borg_skill[BI_RCONF];
	save_rblind = borg_skill[BI_RBLIND];
	save_fract = borg_skill[BI_FRACT];
	save_seeinvis = borg_skill[BI_SINV];
	save_esp = borg_skill[BI_ESP];

	/* Base line power value */
	if (borg_uses_calcs) {
		b_p = borg_power_aux1();
	} else {
		b_p = borg_power_aux3();
	}

	/* Obtain danger */
	b_danger = borg_danger(c_y, c_x, 1, TRUE);

	/* Weapon and armour addresses in the inventory */
	location[0] = weapon_swap;
	location[1] = armour_swap;
	n_damage[0] = 0;
	n_damage[1] = 0;
	damage = 0;

	/* what is the kill-index of the meanest adjacent monster */
	for (n = 0; n < 8; n++) {
		/* boundary check */
		if (!in_bounds(c_y + ddy[n], c_x + ddx[n]))
			continue;

		/* Access the grid */
		ag = &borg_grids[c_y + ddy[n]][c_x + ddx[n]];

		/* Must be a monster on that grid. */
		if (!ag->kill)
			continue;
		kill = &borg_kills[ag->kill];

		/* Skip pet/friendly */
		if (!kill->killer)
			continue;

		/* Danger of an adjacent grid */
		danger = borg_danger(c_y + ddy[n], c_x + ddx[n], 1, TRUE);

		/* Track the most dangerous grid */
		if (danger > b_danger) {
			b_danger = danger;
			b_n = n;

			/* how much damage am i doing to that monster with my current weapon?
			 */
			b_damage = borg_thrust_damage_one(ag->kill, FALSE);
			if (b_damage <= 0)
				b_damage = 1;
		}
	}

	/* No special type next to me */
	if (b_n <= 0)
		return (FALSE);

	/* Scan the two inventory slots */
	for (i = 0; i <= 1; i++) {
		/* Careful */
		if (i == -1)
			continue;

		item = &borg_items[location[i]];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Zang has some very bad curses, do not wear items
		 * that are not ID'd
		 */
		/* Require "known" (or average, good, etc) */
		if (!item->aware && !strstr(item->note, "average") &&
			 !strstr(item->note, "good") && !strstr(item->note, "excellent") &&
			 !strstr(item->note, "Quest") &&
			 !strstr(item->note, "indestructible") &&
			 !strstr(item->note, "special"))
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* Do not wear non *ID*'d artifacts */
		if (!item->fully_identified && item->name1)
			continue;

		/* skip it if it has not been decursed */
		if (item->cursed)
			continue;

		/* Where does it go */
		slot = borg_wield_slot(item);

		/* Cannot wear this item */
		if (slot < 0)
			continue;

		/* skip it if it this slot has been decursed */
		if (borg_items[slot].cursed)
			continue;

		/* If this is a ring and both hands are full, then check each hand
			* and compare the two.  If needed the tight ring can be removed then
			* the better ring placed there on.
			*/

		/* The item being worn will take the place of the swap item in the inven.
		 */
		hole = location[i];

		/*** Process regular items and non full rings ***/

		/* Non ring, non full hands */
		if (slot != INVEN_LEFT ||
			 (!borg_items[INVEN_LEFT].tval || !borg_items[INVEN_RIGHT].tval)) {
			/* Save the old item */
			COPY(&safe_items[slot], &borg_items[slot], borg_item);

			/* Save the new item */
			COPY(&safe_items[location[i]], &borg_items[location[i]], borg_item);

			/* Save the hole */
			COPY(&safe_items[hole], &borg_items[hole], borg_item);

			/* Take off old item */
			COPY(&borg_items[hole], &safe_items[slot], borg_item);

			/* Wear new item */
			COPY(&borg_items[slot], &safe_items[location[i]], borg_item);

			/* Only a single item */
			borg_items[slot].iqty = 1;

			/* Reduce the inventory quantity by one */
			borg_items[location[i]].iqty--;

			/* Fix later */
			fix = TRUE;

			/* Examine the inventory */
			borg_notice(FALSE);

			/* Evaluate the inventory */
			if (borg_uses_calcs) {
				p = borg_power_aux1();
			} else {
				p = borg_power_aux3();
			}

			/* Evaluate local danger */
			danger = borg_danger(c_y, c_x, 1, TRUE);

			/* Examine the critical skills.  If we lose a critical skill, reduce
			 * the benefit */
			if ((save_rconf) && borg_skill[BI_RCONF] == 0)
				p = 0;
			if ((save_rblind) &&
				 (!borg_skill[BI_RBLIND] && !borg_skill[BI_RLITE] &&
				  !borg_skill[BI_RDARK] && borg_skill[BI_SAV] < 100))
				p = 0;
			if ((save_fract) &&
				 (!borg_skill[BI_FRACT] && borg_skill[BI_SAV] < 100))
				p = 0;
			if ((save_seeinvis || save_esp) &&
				 (!borg_skill[BI_ESP] && !borg_skill[BI_SINV]))
				p = 0;

			/* Do a weapon damage check against the meanest adjacent monster */
			ag = &borg_grids[c_y + ddy[b_n]][c_x + ddx[b_n]];
			n_damage[i] = borg_thrust_damage_one(ag->kill, FALSE);

			/* Restore the old item */
			COPY(&borg_items[slot], &safe_items[slot], borg_item);

			/* Restore the new item */
			COPY(&borg_items[location[i]], &safe_items[location[i]], borg_item);

			/* Restore the hole */
			COPY(&borg_items[hole], &safe_items[hole], borg_item);

			/** Compare my base damage with the swap damage **/

			/* My current weapon does more damage */
			if (n_damage[i] <= b_damage)
				continue;

			/* My swap weapon does more damage */
			if (n_damage[i] > b_damage)
				damage = (n_damage[i] * 100) / b_damage;

			/* What percent improvement?  hopefully better than 10% */
			if (damage < 110)
				continue;

			/* Ignore swaps which leave us with missing critical skills */
			if (p == 0)
				continue;

			/* Maintain the "best" */
			b_i = location[i];
			b_p = p;
			b_damage = (n_damage[i] >= 1 ? n_damage[i] : 1);
		} /* non-rings, non full hands */

		/* ring, full hands */
		if (slot == INVEN_LEFT && borg_items[INVEN_LEFT].tval &&
			 borg_items[INVEN_RIGHT].tval) {
			for (ii = INVEN_LEFT; ii <= INVEN_RIGHT; ii++) {
				slot = ii;

				/* Does One Ring need to be handled here? */

				/* Save the old item */
				COPY(&safe_items[slot], &borg_items[slot], borg_item);

				/* Save the new item */
				COPY(&safe_items[location[i]], &borg_items[location[i]], borg_item);

				/* Save the hole */
				COPY(&safe_items[hole], &borg_items[hole], borg_item);

				/* Take off old item */
				COPY(&borg_items[hole], &safe_items[slot], borg_item);

				/* Wear new item */
				COPY(&borg_items[slot], &safe_items[location[i]], borg_item);

				/* Only a single item */
				borg_items[slot].iqty = 1;

				/* Reduce the inventory quantity by one */
				borg_items[location[i]].iqty--;

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the inventory */
				if (borg_uses_calcs) {
					p = borg_power_aux1();
				} else {
					p = borg_power_aux3();
				}

				/* Evaluate local danger */
				danger = borg_danger(c_y, c_x, 1, TRUE);

#if 0
                    /* dump list and power...  for debugging */
                    borg_note(format("Ring: Trying Item %s (power %ld)",borg_items[slot].desc, b_p));
                    borg_note(format("Ring: Against Item  %s  (power %ld)",safe_items[slot].desc, p));
#endif

				/* Examine the critical skills.  If we lose a critical skill, reduce
				 * the reward. */
				if ((save_rconf) && borg_skill[BI_RCONF] == 0)
					p = 0;
				if ((save_rblind) &&
					 (!borg_skill[BI_RBLIND] && !borg_skill[BI_RLITE] &&
					  !borg_skill[BI_RDARK] && borg_skill[BI_SAV] < 100))
					p = 0;
				if ((save_fract) &&
					 (!borg_skill[BI_FRACT] && borg_skill[BI_SAV] < 100))
					p = 0;
				if ((save_seeinvis || save_esp) &&
					 (!borg_skill[BI_ESP] && !borg_skill[BI_SINV]))
					p = 0;

				/* If needed, do a weapon damage check */
				if (b_n >= 0) {
					ag = &borg_grids[c_y + ddy[b_n]][c_x + ddx[b_n]];
					n_damage[i] = borg_thrust_damage_one(ag->kill, FALSE);
				}

				/* Restore the old item */
				COPY(&borg_items[slot], &safe_items[slot], borg_item);

				/* Restore the new item */
				COPY(&borg_items[location[i]], &safe_items[location[i]], borg_item);

				/* Restore the hole */
				COPY(&borg_items[hole], &safe_items[hole], borg_item);

				/* Ignore "bad" swaps */
				if (p < b_p)
					continue;

				/** Compare my base damage with the swap damage **/

				/* My current weapon does more damage */
				if (n_damage[i] <= b_damage)
					continue;

				/* My swap weapon does more damage */
				if (n_damage[i] > b_damage)
					damage = (n_damage[i] * 100) / b_damage;

				/* What percent improvement? hopefully better than 10% */
				if (damage < 110)
					continue;

				/* Ignore swaps which leave us with missing critical skills */
				if (p == 0)
					continue;

				/* Maintain the "best" */
				b_i = location[i];
				b_p = p;
				b_damage = (n_damage[i] >= 1 ? n_damage[i] : 1);
				b_ii = ii;
			}
		} /* ring, looking at replacing each ring */
	}	 /* end scanning inventory */

	/* Restore bonuses */
	if (fix)
		borg_notice(TRUE);

	/* No item */
	if (b_i >= 0) {
		/* Set a timestamp to avoid loops */
		time_last_swap = borg_t;

		/* Get the item */
		item = &borg_items[b_i];

		/* Remove old ring to make room for good one */
		if (b_ii >= INVEN_RIGHT && item->tval == TV_RING &&
			 borg_items[INVEN_RIGHT].iqty) {
			/* Log */
			borg_note(format("# Removing %s to make room for %s.",
								  &borg_items[b_ii].desc, item->desc));

			/* Wear it */
			borg_keypress('t');
			borg_keypress(I2A(b_ii - INVEN_WIELD));

			/*
			 * Once the ring is removed the inventory location of the desired ring
			 * may change.
			 */
			return (TRUE);
		}

		/* Log */
		borg_note(format("# Wearing swap for more damage: %s.", item->desc));

		/* Wear it */
		borg_keypress('w');
		borg_keypress(I2A(b_i));

		/* Did something */
		time_this_panel++;
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Hack -- order of the slots
 *
 * XXX XXX XXX Note that we ignore the "tight" ring, and we
 * assume that we will always be wearing our "best" ring on
 * our "right" (tight) finger, and if we are not, then the
 * "borg_swap_rings()" function will remove both the rings,
 * which will induce the "borg_best_stuff()" function to put
 * the rings back on in the "optimal" order.
 */
static byte borg_best_stuff_order[] = {
	 INVEN_BOW,	INVEN_WIELD, INVEN_BODY, INVEN_OUTER, INVEN_ARM,  INVEN_HEAD,
	 INVEN_HANDS, INVEN_FEET,  INVEN_LEFT, INVEN_LITE,  INVEN_NECK,

	 255};

/*
 * Helper function (see below)
 */
static void borg_best_stuff_aux2(int n, byte *test, byte *best, s32b *vp) {
	int i;

	int slot;
	int o;
	bool recently_worn = FALSE;

	/* Extract the slot */
	slot = borg_best_stuff_order[n];

	/* All done */
	if (slot == 255) {
		s32b p;

		/* Examine */
		borg_notice(FALSE);

		/* Evaluate */
		p = borg_power();

		/* Track best */
		if (p > *vp) {

			if (borg_verbose) {
				/* dump list and power...  for debugging */
				borg_note(format("Trying Combo (best power %ld)", *vp));
				borg_note(format("             (borg_power %ld)", p));
				for (i = 0; borg_best_stuff_order[i] != 255; i++)
					borg_note(format("stuff %s.",
										  borg_items[borg_best_stuff_order[i]].desc));
			}

			/* Save the results */
			for (i = 0; i < n; i++) {
				best[i] = test[i];
			}

			/* Use it */
			*vp = p;
		}

		/* Success */
		return;
	}

	/* Note the attempt */
	test[n] = slot;

	/* Evaluate the default item */
	borg_best_stuff_aux2(n + 1, test, best, vp);

	/* Try other possible objects */
	for (i = 0; i < ((shop_num == BORG_HOME) ? (INVEN_PACK + STORE_INVEN_MAX)
														  : INVEN_PACK);
		  i++) {
		borg_item *item;
		if (i < INVEN_PACK)
			item = &borg_items[i];
		else
			item = &borg_shops[BORG_HOME].ware[i - INVEN_PACK];

		/* Reset our recently worn flag.  This item shouldnt inherit the flag from
		 * the previous item. */
		recently_worn = FALSE;

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Zang has some very bad curses, do not wear items
		 * that are not ID'd
		 */
		/* Require "known" (or average, good, etc) */
		if (!item->aware && !strstr(item->note, "average") /* &&
            !strstr(item->note, "good") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "indestructible") &&
            !strstr(item->note, "special") */)
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* Skip it if it has not been decursed */
		if ((item->cursed) || (item->flags3 & TR3_HEAVY_CURSE) ||
			 (inventory[i].ident & IDENT_CURSED))
			continue;

		/* Make sure it goes in this slot, special consideration
		 * for checking rings
		 */
		if (slot != borg_wield_slot(item))
			continue;

		/* Make sure that slot does not have a cursed item */
		if ((borg_items[slot].cursed) ||
			 (borg_items[slot].flags3 & TR3_HEAVY_CURSE) ||
			 (inventory[slot].ident & IDENT_CURSED))
			continue;

		/* Do not consider wearing this item if I worn it already this level,
		 * I might be stuck in a loop.
		 */
		for (o = 0; o < track_worn_num; o++) {
			/* Examine the worn list */
			if (track_worn_num >= 1 && track_worn_name1[o] == item->name1 &&
				 track_worn_name2[o] == item->name2 &&
				 track_worn_tval[o] == item->tval &&
				 track_worn_sval[o] == item->sval &&
				 track_worn_pval[o] == item->pval &&
				 track_worn_time > borg_t - 10) {
				/* Recently worn item */
				recently_worn = TRUE;
			}
		}

		/* Note and fail out */
		if (recently_worn == TRUE) {
			continue;
		}

		/* Wear the new item */
		COPY(&borg_items[slot], item, borg_item);

		/* Note the attempt */
		if (i < INVEN_PACK)
			test[n] = i;
		else
			/* if in home, note by adding 100 to item number. */
			test[n] = (i - INVEN_PACK) + 100;

		/* Evaluate the possible item */
		borg_best_stuff_aux2(n + 1, test, best, vp);

		/* Restore equipment */
		COPY(&borg_items[slot], &safe_items[slot], borg_item);
	}
}

/*
 * Attempt to instantiate the *best* possible combination of equipment.
 * Although it currently does not check the rings.
 */
bool borg_best_combo(/*bool shops_too*/) {
	int hole = INVEN_PACK - 1;
	int weapon, bow, /*ring_r, ring_l,*/ neck, outer, shield, helm, glove, boot,
		 lite, body, i;
	s32b value;
	s32b b_value;
	/*int track, k;*/

	/*define how many of each inventory type we have */
	int weapon_cnt = 0;
	int bow_cnt = 0;
	int neck_cnt = 0;
	int outer_cnt = 0;
	int shield_cnt = 0;
	int helm_cnt = 0;
	int glove_cnt = 0;
	int boot_cnt = 0;
	int lite_cnt = 0;
	int body_cnt = 0;
	/*int ring_r_cnt = 0;*/
	int ring_l_cnt = 0;

	/* Define address of the best of each type */
	int b_weapon = -1;
	int b_bow = -1;
	int b_neck = -1;
	int b_outer = -1;
	int b_shield = -1;
	int b_helm = -1;
	int b_glove = -1;
	int b_boot = -1;
	int b_lite = -1;
	int b_body = -1;
	/*int b_ring_r = -1;
	int b_ring_l = -1;*/

	/* Define address of each item of each type */
	int n_weapon[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_bow[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_neck[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_outer[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_shield[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_helm[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_glove[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_boot[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_lite[INVEN_TOTAL + STORE_INVEN_MAX];
	int n_body[INVEN_TOTAL + STORE_INVEN_MAX];
	/*int n_ring_r[INVEN_TOTAL + STORE_INVEN_MAX];*/
	/*int n_ring_l[INVEN_TOTAL + STORE_INVEN_MAX];*/

	char b_weapon_desc[80];
	char b_bow_desc[80];
	/*char b_ring_r_desc[80];char b_ring_l_desc[80];*/
	char b_neck_desc[80];
	char b_outer_desc[80];
	char b_shield_desc[80];
	char b_helm_desc[80];
	char b_boot_desc[80];
	char b_lite_desc[80];
	char b_body_desc[80];
	char b_glove_desc[80];

	/* Hack -- Anti-loop */
	if (time_this_panel >= 300)
		return (FALSE);
	if (borg_t - borg_began >= 600)
		return (FALSE);

	/* Not if stats are drained at all */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP])
		return (FALSE);

	/* Not if full.  We need space to move equip around */
	if (borg_items[hole].iqty)
		return (FALSE);

	/* Hack -- Copy all the slots */
	borg_cheat_inven();
	borg_cheat_equip();
	for (i = 0; i < INVEN_TOTAL; i++) {
		/* Save the item */
		COPY(&safe_items[i], &borg_items[i], borg_item);
	}

	/* Hack -- Copy all the store slots */
	for (i = 0; i < STORE_INVEN_MAX; i++) {
		/* Save the item */
		COPY(&safe_home[i], &borg_shops[BORG_HOME].ware[i], borg_item);
	}

	/* Evaluate the inventory */
	b_value = my_power;
	borg_notice(FALSE);
	b_value = borg_power();

	/* Determine the quantity of each wield slot type */
	for (i = 0; i < (INVEN_TOTAL + STORE_INVEN_MAX); i++) {
		/* examine the item to see if it is wieldable */
		borg_item *item;
		if (i < INVEN_TOTAL)
			item = &borg_items[i];
		else
			item = &borg_shops[BORG_HOME].ware[i - INVEN_TOTAL];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require "aware" */
		if (!item->kind)
			continue;

		/* Scan each wield item */
		/* Zang has some very bad curses, do not wear items
		 * that are not ID'd
		 */
		/* Require "known" (or average, good, etc) */
		if (!item->aware && !strstr(item->note, "average") /* &&
            !strstr(item->note, "good") &&
            !strstr(item->note, "excellent") &&
            !strstr(item->note, "indestructible") &&
            !strstr(item->note, "special") */)
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		/* Skip it if it has not been decursed */
		if ((item->cursed) || (item->flags3 & TR3_HEAVY_CURSE) ||
			 (inventory[i].ident & IDENT_CURSED))
			continue;

		/* What slot does it belong to?
		 */
		switch (borg_wield_slot(item)) {
		case INVEN_WIELD:
			n_weapon[weapon_cnt] = i;
			weapon_cnt++;
			break;
		case INVEN_BOW:
			n_bow[bow_cnt] = i;
			bow_cnt++;
			break;
		case INVEN_LEFT:
			/*n_ring_l[ring_l_cnt] = i;*/
			ring_l_cnt++;
			break;
		case INVEN_NECK:
			n_neck[neck_cnt] = i;
			neck_cnt++;
			break;
		case INVEN_LITE:
			n_lite[lite_cnt] = i;
			lite_cnt++;
			break;
		case INVEN_BODY:
			n_body[body_cnt] = i;
			body_cnt++;
			break;
		case INVEN_OUTER:
			n_outer[outer_cnt] = i;
			outer_cnt++;
			break;
		case INVEN_ARM:
			n_shield[shield_cnt] = i;
			shield_cnt++;
			break;
		case INVEN_HEAD:
			n_helm[helm_cnt] = i;
			helm_cnt++;
			break;
		case INVEN_HANDS:
			n_glove[glove_cnt] = i;
			glove_cnt++;
			break;
		case INVEN_FEET:
			n_boot[boot_cnt] = i;
			boot_cnt++;
			break;
		}
	}

	/* Save the baseline equipment and home inventory */

	/*
	 * Nest the equipment checks.  This routine will check every permutation of
	 * equipment combinations.
	 * It is a bit slow, but will positively identify the best set up for maximum
	 * power.
	 * Getting the borg to fetch and wear those items is a different story.
	 */

	/* Report */
	if (borg_verbose) {
		borg_note(
			 format("# Best Combo: Attempting every permutation of %d weapons and",
					  weapon_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d bows and",
					  bow_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d rings and",
					  ring_l_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d amulets and",
					  neck_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d lite and",
					  lite_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d armors and",
					  body_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d shields and",
					  shield_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d helms and",
					  helm_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d gloves and",
					  glove_cnt));
		borg_note(
			 format("#             Attempting every permutation of %d boots and",
					  boot_cnt));
	}

	/* Weapons. 1st level nest. */
	for (weapon = 0; weapon < weapon_cnt; weapon++) {
		/* Move my current item to a safe place */
		COPY(&safe_items[INVEN_WIELD], &borg_items[INVEN_WIELD], borg_item);

		/* Move the new item into the weild spot */
		if (n_weapon[weapon] < INVEN_TOTAL) {
			COPY(&borg_items[INVEN_WIELD], &borg_items[n_weapon[weapon]],
				  borg_item);
		} else {
			COPY(&borg_items[INVEN_WIELD],
				  &borg_shops[BORG_HOME].ware[n_weapon[weapon] - INVEN_TOTAL],
				  borg_item);
		}

		/* Move the original item into the new items orignal inventory spot */
		if (n_weapon[weapon] < INVEN_TOTAL) {
			COPY(&borg_items[n_weapon[weapon]], &safe_items[INVEN_WIELD],
				  borg_item);
		} else {
			COPY(&borg_shops[BORG_HOME].ware[n_weapon[weapon] - INVEN_TOTAL],
				  &safe_items[INVEN_WIELD], borg_item);
		}

		if (borg_verbose) {
			/* dump list and power...  for debugging */
			borg_note(format("WIELD: %s", borg_items[INVEN_WIELD].desc));
		}
		/* Bow. 2nd level nest. */
		for (bow = 0; bow < bow_cnt; bow++) {
			/* Move my current item to a safe place */
			COPY(&safe_items[INVEN_BOW], &borg_items[INVEN_BOW], borg_item);

			/* Move the new item into the weild spot */
			if (n_bow[bow] < INVEN_TOTAL) {
				COPY(&borg_items[INVEN_BOW], &borg_items[n_bow[bow]], borg_item);
			} else {
				COPY(&borg_items[INVEN_BOW],
					  &borg_shops[BORG_HOME].ware[n_bow[bow] - INVEN_TOTAL],
					  borg_item);
			}

			/* Move the original item into the new items orignal inventory spot */
			if (n_bow[bow] < INVEN_TOTAL) {
				COPY(&borg_items[n_bow[bow]], &safe_items[INVEN_BOW], borg_item);
			} else {
				COPY(&borg_shops[BORG_HOME].ware[n_bow[bow] - INVEN_TOTAL],
					  &safe_items[INVEN_WIELD], borg_item);
			}

			if (borg_verbose) {
				/* dump list and power...  for debugging */
				borg_note(format(" BOW: %s", borg_items[INVEN_BOW].desc));
			}

			/* Neck. 3nd level nest. */
			for (neck = 0; neck < neck_cnt; neck++) {
				/* Move my current item to a safe place */
				COPY(&safe_items[INVEN_NECK], &borg_items[INVEN_NECK], borg_item);

				/* Move the new item into the weild spot */
				if (n_neck[neck] < INVEN_TOTAL) {
					COPY(&borg_items[INVEN_NECK], &borg_items[n_neck[neck]],
						  borg_item);
				} else {
					COPY(&borg_items[INVEN_NECK],
						  &borg_shops[BORG_HOME].ware[n_neck[neck] - INVEN_TOTAL],
						  borg_item);
				}

				/* Move the original item into the new items orignal inventory spot
				 */
				if (n_neck[neck] < INVEN_TOTAL) {
					COPY(&borg_items[n_neck[neck]], &safe_items[INVEN_NECK],
						  borg_item);
				} else {
					COPY(&borg_shops[BORG_HOME].ware[n_neck[neck] - INVEN_TOTAL],
						  &safe_items[INVEN_NECK], borg_item);
				}

				if (borg_verbose) {
					/* dump list and power...  for debugging */
					borg_note(format("  Neck: %s", borg_items[INVEN_NECK].desc));
				}

				/* Cloaks. 4th level nest. */
				for (outer = 0; outer < outer_cnt; outer++) {
					/* Move my current item to a safe place */
					COPY(&safe_items[INVEN_OUTER], &borg_items[INVEN_OUTER],
						  borg_item);

					/* Move the new item into the weild spot */
					if (n_outer[outer] < INVEN_TOTAL) {
						COPY(&borg_items[INVEN_OUTER], &borg_items[n_outer[outer]],
							  borg_item);
					} else {
						COPY(
							 &borg_items[INVEN_OUTER],
							 &borg_shops[BORG_HOME].ware[n_outer[outer] - INVEN_TOTAL],
							 borg_item);
					}

					/* Move the original item into the new items orignal inventory
					 * spot */
					if (n_outer[outer] < INVEN_TOTAL) {
						COPY(&borg_items[n_outer[outer]], &safe_items[INVEN_OUTER],
							  borg_item);
					} else {
						COPY(
							 &borg_shops[BORG_HOME].ware[n_outer[outer] - INVEN_TOTAL],
							 &safe_items[INVEN_OUTER], borg_item);
					}

					if (borg_verbose) {
						/* dump list and power...  for debugging */
						borg_note(
							 format("   Outer: %s", borg_items[INVEN_OUTER].desc));
					}

					/* Shields. 5th level nest. */
					for (shield = 0; shield < shield_cnt; shield++) {
						/* Move my current item to a safe place */
						COPY(&safe_items[INVEN_ARM], &borg_items[INVEN_ARM],
							  borg_item);

						/* Move the new item into the weild spot */
						if (n_shield[shield] < INVEN_TOTAL) {
							COPY(&borg_items[INVEN_ARM], &borg_items[n_shield[shield]],
								  borg_item);
						} else {
							COPY(&borg_items[INVEN_ARM],
								  &borg_shops[BORG_HOME]
										 .ware[n_shield[shield] - INVEN_TOTAL],
								  borg_item);
						}

						/* Move the original item into the new items orignal inventory
						 * spot */
						if (n_shield[shield] < INVEN_TOTAL) {
							COPY(&borg_items[n_shield[shield]], &safe_items[INVEN_ARM],
								  borg_item);
						} else {
							COPY(&borg_shops[BORG_HOME]
										 .ware[n_shield[shield] - INVEN_TOTAL],
								  &safe_items[INVEN_ARM], borg_item);
						}

						if (borg_verbose) {
							/* dump list and power...  for debugging */
							borg_note(
								 format("    Shield: %s", borg_items[INVEN_ARM].desc));
						}

						/* Helm. 6th level nest. */
						for (helm = 0; helm < helm_cnt; helm++) {
							/* Move my current item to a safe place */
							COPY(&safe_items[INVEN_HEAD], &borg_items[INVEN_HEAD],
								  borg_item);

							/* Move the new item into the weild spot */
							if (n_helm[helm] < INVEN_TOTAL) {
								COPY(&borg_items[INVEN_HEAD], &borg_items[n_helm[helm]],
									  borg_item);
							} else {
								COPY(&borg_items[INVEN_HEAD],
									  &borg_shops[BORG_HOME]
											 .ware[n_helm[helm] - INVEN_TOTAL],
									  borg_item);
							}

							/* Move the original item into the new items orignal
							 * inventory spot */
							if (n_helm[helm] < INVEN_TOTAL) {
								COPY(&borg_items[n_helm[helm]], &safe_items[INVEN_HEAD],
									  borg_item);
							} else {
								COPY(&borg_shops[BORG_HOME]
											 .ware[n_helm[helm] - INVEN_TOTAL],
									  &safe_items[INVEN_HEAD], borg_item);
							}

							if (borg_verbose) {
								/* dump list and power...  for debugging */
								borg_note(format("     Helm: %s",
													  borg_items[INVEN_HEAD].desc));
							}

							/* Glove. 7th level nest. */
							for (glove = 0; glove < glove_cnt; glove++) {
								/* Move my current item to a safe place */
								COPY(&safe_items[INVEN_HANDS], &borg_items[INVEN_HANDS],
									  borg_item);

								/* Move the new item into the weild spot */
								if (n_glove[glove] < INVEN_TOTAL) {
									COPY(&borg_items[INVEN_HANDS],
										  &borg_items[n_glove[glove]], borg_item);
								} else {
									COPY(&borg_items[INVEN_HANDS],
										  &borg_shops[BORG_HOME]
												 .ware[n_glove[glove] - INVEN_TOTAL],
										  borg_item);
								}

								/* Move the original item into the new items orignal
								 * inventory spot */
								if (n_glove[glove] < INVEN_TOTAL) {
									COPY(&borg_items[n_glove[glove]],
										  &safe_items[INVEN_HANDS], borg_item);
								} else {
									COPY(&borg_shops[BORG_HOME]
												 .ware[n_glove[glove] - INVEN_TOTAL],
										  &safe_items[INVEN_HANDS], borg_item);
								}

								if (borg_verbose) {
									/* dump list and power...  for debugging */
									borg_note(format("      Glove: %s",
														  borg_items[INVEN_HANDS].desc));
								}

								/* Boot. 8th level nest. */
								for (boot = 0; boot < boot_cnt; boot++) {
									/* Move my current item to a safe place */
									COPY(&safe_items[INVEN_FEET],
										  &borg_items[INVEN_FEET], borg_item);

									/* Move the new item into the weild spot */
									if (n_boot[boot] < INVEN_TOTAL) {
										COPY(&borg_items[INVEN_FEET],
											  &borg_items[n_boot[boot]], borg_item);
									} else {
										COPY(&borg_items[INVEN_FEET],
											  &borg_shops[BORG_HOME]
													 .ware[n_boot[boot] - INVEN_TOTAL],
											  borg_item);
									}

									/* Move the original item into the new items orignal
									 * inventory spot */
									if (n_boot[boot] < INVEN_TOTAL) {
										COPY(&borg_items[n_boot[boot]],
											  &safe_items[INVEN_FEET], borg_item);
									} else {
										COPY(&borg_shops[BORG_HOME]
													 .ware[n_boot[boot] - INVEN_TOTAL],
											  &safe_items[INVEN_FEET], borg_item);
									}

									if (borg_verbose) {
										/* dump list and power...  for debugging */
										borg_note(format("       Boots: %s",
															  borg_items[INVEN_FEET].desc));
									}

									/* Lite. 9th level nest. */
									for (lite = 0; lite < lite_cnt; lite++) {
										/* Move my current item to a safe place */
										COPY(&safe_items[INVEN_LITE],
											  &borg_items[INVEN_LITE], borg_item);

										/* Move the new item into the weild spot */
										if (n_lite[lite] < INVEN_TOTAL) {
											COPY(&borg_items[INVEN_LITE],
												  &borg_items[n_lite[lite]], borg_item);
										} else {
											COPY(&borg_items[INVEN_LITE],
												  &borg_shops[BORG_HOME]
														 .ware[n_lite[lite] - INVEN_TOTAL],
												  borg_item);
										}

										/* Move the original item into the new items
										 * orignal inventory spot */
										if (n_lite[lite] < INVEN_TOTAL) {
											COPY(&borg_items[n_lite[lite]],
												  &safe_items[INVEN_LITE], borg_item);
										} else {
											COPY(&borg_shops[BORG_HOME]
														 .ware[n_lite[lite] - INVEN_TOTAL],
												  &safe_items[INVEN_LITE], borg_item);
										}

										if (borg_verbose) {
											/* dump list and power...  for debugging */
											borg_note(format("        Lite: %s",
																  borg_items[INVEN_LITE].desc));
										}

										/* Armor. 10th level nest. */
										for (body = 0; body < body_cnt; body++) {
											/* Move my current item to a safe place */
											COPY(&safe_items[INVEN_BODY],
												  &borg_items[INVEN_BODY], borg_item);

											/* Move the new item into the weild spot */
											if (n_body[body] < INVEN_TOTAL) {
												COPY(&borg_items[INVEN_BODY],
													  &borg_items[n_body[body]], borg_item);
											} else {
												COPY(&borg_items[INVEN_BODY],
													  &borg_shops[BORG_HOME]
															 .ware[n_body[body] - INVEN_TOTAL],
													  borg_item);
											}

											/* Move the original item into the new items
											 * orignal inventory spot */
											if (n_body[body] < INVEN_TOTAL) {
												COPY(&borg_items[n_body[body]],
													  &safe_items[INVEN_BODY], borg_item);
											} else {
												COPY(&borg_shops[BORG_HOME]
															 .ware[n_body[body] - INVEN_TOTAL],
													  &safe_items[INVEN_BODY], borg_item);
											}

											/* Examine the inventory */
											borg_notice(FALSE);

											/* Evaluate the inventory */
											value = borg_power();

											if (borg_verbose) {
												/* dump list and power...  for debugging */
												borg_note(format(
													 "         Armor: %s (power %ld)",
													 borg_items[INVEN_BODY].desc, value));
											}
											/* Restore original item to the place holder */
											if (n_body[body] < INVEN_TOTAL) {
												COPY(&safe_items[INVEN_BODY],
													  &borg_items[n_body[body]], borg_item);
											} else {
												COPY(&safe_items[INVEN_BODY],
													  &borg_shops[BORG_HOME]
															 .ware[n_body[body] - INVEN_TOTAL],
													  borg_item);
											}

											/* Restore tested item back into its original
											 * inventory place */
											if (n_body[body] < INVEN_TOTAL) {
												COPY(&borg_items[n_body[body]],
													  &borg_items[INVEN_BODY], borg_item);
											} else {
												COPY(&borg_shops[BORG_HOME]
															 .ware[n_body[body] - INVEN_TOTAL],
													  &borg_items[INVEN_BODY], borg_item);
											}

											/* Restore original item back into its original
											 * place. */
											COPY(&borg_items[INVEN_BODY],
												  &safe_items[INVEN_BODY], borg_item);

											/* Ignore "bad" swaps */
											if (value <= b_value)
												continue;

											/* Maintain the "best" */
											b_value = value;
											b_weapon = borg_best_stuff_order[INVEN_WIELD] =
												 weapon;
											b_bow = borg_best_stuff_order[INVEN_BOW] = bow;
											/* b_ring_r =
											 * borg_best_stuff_order[INVEN_LEFT] = ring_r;
											 */
											/* b_ring_l =
											 * borg_best_stuff_order[INVEN_RIGHT] = ring_l;
											 */
											b_neck = borg_best_stuff_order[INVEN_NECK] =
												 neck;
											b_outer = borg_best_stuff_order[INVEN_OUTER] =
												 outer;
											b_shield = borg_best_stuff_order[INVEN_ARM] =
												 shield;
											b_helm = borg_best_stuff_order[INVEN_HEAD] =
												 helm;
											b_glove = borg_best_stuff_order[INVEN_HANDS] =
												 glove;
											b_boot = borg_best_stuff_order[INVEN_FEET] =
												 boot;
											b_lite = borg_best_stuff_order[INVEN_LITE] =
												 lite;
											b_body = borg_best_stuff_order[INVEN_BODY] =
												 body;

											/* Store descriptions for later */
											if (n_weapon[b_weapon] < INVEN_TOTAL)
												strcpy(b_weapon_desc,
														 borg_items[n_weapon[b_weapon]].desc);
											else
												strcpy(b_weapon_desc,
														 borg_shops[BORG_HOME]
															  .ware[n_weapon[b_weapon] -
																	  INVEN_TOTAL]
															  .desc);

											if (n_bow[b_bow] < INVEN_TOTAL)
												strcpy(b_bow_desc,
														 borg_items[n_bow[b_bow]].desc);
											else
												strcpy(b_bow_desc,
														 borg_shops[BORG_HOME]
															  .ware[n_bow[b_bow] - INVEN_TOTAL]
															  .desc);

											if (n_neck[b_neck] < INVEN_TOTAL)
												strcpy(b_neck_desc,
														 borg_items[n_neck[b_neck]].desc);
											else
												strcpy(
													 b_neck_desc,
													 borg_shops[BORG_HOME]
														  .ware[n_neck[b_neck] - INVEN_TOTAL]
														  .desc);

											if (n_outer[b_outer] < INVEN_TOTAL)
												strcpy(b_outer_desc,
														 borg_items[n_outer[b_outer]].desc);
											else
												strcpy(b_outer_desc,
														 borg_shops[BORG_HOME]
															  .ware[n_outer[b_outer] -
																	  INVEN_TOTAL]
															  .desc);

											if (n_shield[b_shield] < INVEN_TOTAL)
												strcpy(b_shield_desc,
														 borg_items[n_shield[b_shield]].desc);
											else
												strcpy(b_shield_desc,
														 borg_shops[BORG_HOME]
															  .ware[n_shield[b_shield] -
																	  INVEN_TOTAL]
															  .desc);

											if (n_helm[b_helm] < INVEN_TOTAL)
												strcpy(b_helm_desc,
														 borg_items[n_helm[b_helm]].desc);
											else
												strcpy(
													 b_helm_desc,
													 borg_shops[BORG_HOME]
														  .ware[n_helm[b_helm] - INVEN_TOTAL]
														  .desc);

											if (n_glove[b_glove] < INVEN_TOTAL)
												strcpy(b_glove_desc,
														 borg_items[n_glove[b_glove]].desc);
											else
												strcpy(b_glove_desc,
														 borg_shops[BORG_HOME]
															  .ware[n_glove[b_glove] -
																	  INVEN_TOTAL]
															  .desc);

											if (n_boot[b_boot] < INVEN_TOTAL)
												strcpy(b_boot_desc,
														 borg_items[n_boot[b_boot]].desc);
											else
												strcpy(
													 b_boot_desc,
													 borg_shops[BORG_HOME]
														  .ware[n_boot[b_boot] - INVEN_TOTAL]
														  .desc);

											if (n_lite[b_lite] < INVEN_TOTAL)
												strcpy(b_lite_desc,
														 borg_items[n_lite[b_lite]].desc);
											else
												strcpy(
													 b_lite_desc,
													 borg_shops[BORG_HOME]
														  .ware[n_lite[b_lite] - INVEN_TOTAL]
														  .desc);

											if (n_body[b_body] < INVEN_TOTAL)
												strcpy(b_body_desc,
														 borg_items[n_body[b_body]].desc);
											else
												strcpy(
													 b_body_desc,
													 borg_shops[BORG_HOME]
														  .ware[n_body[b_body] - INVEN_TOTAL]
														  .desc);

											if (borg_verbose) {
												/* dump list and power...  for debugging */
												borg_note(format(
													 "          Recommend change:  %ld, %ld",
													 b_value, value));
											}
										}

										/* Restore original item to the place holder */
										if (n_lite[lite] < INVEN_TOTAL) {
											COPY(&safe_items[INVEN_LITE],
												  &borg_items[n_lite[lite]], borg_item);
										} else {
											COPY(&safe_items[INVEN_LITE],
												  &borg_shops[BORG_HOME]
														 .ware[n_lite[lite] - INVEN_TOTAL],
												  borg_item);
										}

										/* Restore tested item back into its original
										 * inventory place */
										if (n_lite[lite] < INVEN_TOTAL) {
											COPY(&borg_items[n_lite[lite]],
												  &borg_items[INVEN_LITE], borg_item);
										} else {
											COPY(&borg_shops[BORG_HOME]
														 .ware[n_lite[lite] - INVEN_TOTAL],
												  &borg_items[INVEN_LITE], borg_item);
										}

										/* Restore original item back into its original
										 * place. */
										COPY(&borg_items[INVEN_LITE],
											  &safe_items[INVEN_LITE], borg_item);
									}

									/* Restore original item to the place holder */
									if (n_boot[boot] < INVEN_TOTAL) {
										COPY(&safe_items[INVEN_FEET],
											  &borg_items[n_boot[boot]], borg_item);
									} else {
										COPY(&safe_items[INVEN_FEET],
											  &borg_shops[BORG_HOME]
													 .ware[n_boot[boot] - INVEN_TOTAL],
											  borg_item);
									}

									/* Restore tested item back into its original
									 * inventory place */
									if (n_boot[boot] < INVEN_TOTAL) {
										COPY(&borg_items[n_boot[boot]],
											  &borg_items[INVEN_FEET], borg_item);
									} else {
										COPY(&borg_shops[BORG_HOME]
													 .ware[n_boot[boot] - INVEN_TOTAL],
											  &borg_items[INVEN_FEET], borg_item);
									}

									/* Restore original item back into its original
									 * place. */
									COPY(&borg_items[INVEN_FEET],
										  &safe_items[INVEN_FEET], borg_item);
								}

								/* Restore original item to the place holder */
								if (n_glove[glove] < INVEN_TOTAL) {
									COPY(&safe_items[INVEN_HANDS],
										  &borg_items[n_glove[glove]], borg_item);
								} else {
									COPY(&safe_items[INVEN_HANDS],
										  &borg_shops[BORG_HOME]
												 .ware[n_glove[glove] - INVEN_TOTAL],
										  borg_item);
								}

								/* Restore tested item back into its original inventory
								 * place */
								if (n_glove[glove] < INVEN_TOTAL) {
									COPY(&borg_items[n_glove[glove]],
										  &borg_items[INVEN_HANDS], borg_item);
								} else {
									COPY(&borg_shops[BORG_HOME]
												 .ware[n_glove[glove] - INVEN_TOTAL],
										  &borg_items[INVEN_HANDS], borg_item);
								}

								/* Restore original item back into its original place.
								 */
								COPY(&borg_items[INVEN_HANDS], &safe_items[INVEN_HANDS],
									  borg_item);
							}

							/* Restore original item to the place holder */
							if (n_helm[helm] < INVEN_TOTAL) {
								COPY(&safe_items[INVEN_HEAD], &borg_items[n_helm[helm]],
									  borg_item);
							} else {
								COPY(&safe_items[INVEN_HEAD],
									  &borg_shops[BORG_HOME]
											 .ware[n_helm[helm] - INVEN_TOTAL],
									  borg_item);
							}

							/* Restore tested item back into its original inventory
							 * place */
							if (n_helm[helm] < INVEN_TOTAL) {
								COPY(&borg_items[n_helm[helm]], &borg_items[INVEN_HEAD],
									  borg_item);
							} else {
								COPY(&borg_shops[BORG_HOME]
											 .ware[n_helm[helm] - INVEN_TOTAL],
									  &borg_items[INVEN_HEAD], borg_item);
							}

							/* Restore original item back into its original place. */
							COPY(&borg_items[INVEN_HEAD], &safe_items[INVEN_HEAD],
								  borg_item);
						}

						/* Restore original item to the place holder */
						if (n_shield[shield] < INVEN_TOTAL) {
							COPY(&safe_items[INVEN_ARM], &borg_items[n_shield[shield]],
								  borg_item);
						} else {
							COPY(&safe_items[INVEN_ARM],
								  &borg_shops[BORG_HOME]
										 .ware[n_shield[shield] - INVEN_TOTAL],
								  borg_item);
						}

						/* Restore tested item back into its original inventory place
						 */
						if (n_shield[shield] < INVEN_TOTAL) {
							COPY(&borg_items[n_shield[shield]], &borg_items[INVEN_ARM],
								  borg_item);
						} else {
							COPY(&borg_shops[BORG_HOME]
										 .ware[n_shield[shield] - INVEN_TOTAL],
								  &borg_items[INVEN_ARM], borg_item);
						}

						/* Restore original item back into its original place. */
						COPY(&borg_items[INVEN_ARM], &safe_items[INVEN_ARM],
							  borg_item);
					}

					/* Restore original item to the place holder */
					if (n_outer[outer] < INVEN_TOTAL) {
						COPY(&safe_items[INVEN_OUTER], &borg_items[n_outer[outer]],
							  borg_item);
					} else {
						COPY(
							 &safe_items[INVEN_OUTER],
							 &borg_shops[BORG_HOME].ware[n_outer[outer] - INVEN_TOTAL],
							 borg_item);
					}

					/* Restore tested item back into its original inventory place */
					if (n_outer[outer] < INVEN_TOTAL) {
						COPY(&borg_items[n_outer[outer]], &borg_items[INVEN_OUTER],
							  borg_item);
					} else {
						COPY(
							 &borg_shops[BORG_HOME].ware[n_outer[outer] - INVEN_TOTAL],
							 &borg_items[INVEN_OUTER], borg_item);
					}

					/* Restore original item back into its original place. */
					COPY(&borg_items[INVEN_OUTER], &safe_items[INVEN_OUTER],
						  borg_item);
				}

				/* Restore original item to the place holder */
				if (n_neck[neck] < INVEN_TOTAL) {
					COPY(&safe_items[INVEN_NECK], &borg_items[n_neck[neck]],
						  borg_item);
				} else {
					COPY(&safe_items[INVEN_NECK],
						  &borg_shops[BORG_HOME].ware[n_neck[neck] - INVEN_TOTAL],
						  borg_item);
				}

				/* Restore tested item back into its original inventory place */
				if (n_neck[neck] < INVEN_TOTAL) {
					COPY(&borg_items[n_neck[neck]], &borg_items[INVEN_NECK],
						  borg_item);
				} else {
					COPY(&borg_shops[BORG_HOME].ware[n_neck[neck] - INVEN_TOTAL],
						  &borg_items[INVEN_NECK], borg_item);
				}

				/* Restore original item back into its original place. */
				COPY(&borg_items[INVEN_NECK], &safe_items[INVEN_NECK], borg_item);
			}

			/* Restore original item to the place holder */
			if (n_bow[bow] < INVEN_TOTAL) {
				COPY(&safe_items[INVEN_BOW], &borg_items[n_bow[bow]], borg_item);
			} else {
				COPY(&safe_items[INVEN_BOW],
					  &borg_shops[BORG_HOME].ware[n_bow[bow] - INVEN_TOTAL],
					  borg_item);
			}

			/* Restore tested item back into its original inventory place */
			if (n_bow[bow] < INVEN_TOTAL) {
				COPY(&borg_items[n_bow[bow]], &borg_items[INVEN_BOW], borg_item);
			} else {
				COPY(&borg_shops[BORG_HOME].ware[n_bow[bow] - INVEN_TOTAL],
					  &borg_items[INVEN_BOW], borg_item);
			}

			/* Restore original item back into its original place. */
			COPY(&borg_items[INVEN_BOW], &safe_items[INVEN_BOW], borg_item);
		} /* Bow */

		/* Restore original item to the place holder */
		if (n_weapon[weapon] < INVEN_TOTAL) {
			COPY(&safe_items[INVEN_WIELD], &borg_items[n_weapon[weapon]],
				  borg_item);
		} else {
			COPY(&safe_items[INVEN_WIELD],
				  &borg_shops[BORG_HOME].ware[n_weapon[weapon] - INVEN_TOTAL],
				  borg_item);
		}

		/* Restore tested item back into its original inventory place */
		if (n_weapon[weapon] < INVEN_TOTAL) {
			COPY(&borg_items[n_weapon[weapon]], &borg_items[INVEN_WIELD],
				  borg_item);
		} else {
			COPY(&borg_shops[BORG_HOME].ware[n_weapon[weapon] - INVEN_TOTAL],
				  &borg_items[INVEN_WIELD], borg_item);
		}

		/* Restore original item back into its original place. */
		COPY(&borg_items[INVEN_WIELD], &safe_items[INVEN_WIELD], borg_item);

	} /* Weapon */

	/* make sure the borg knows his real gear */
	borg_notice(TRUE);

	/* Report best combo */
	if (b_weapon >= 0)
		borg_note(
			 format("# Best Weapon address: %d, %s", b_weapon, b_weapon_desc));
	if (b_bow >= 0)
		borg_note(format("# Best Bow address: %d, %s", b_bow, b_bow_desc));
	if (b_neck >= 0)
		borg_note(format("# Best Neck address: %d, %s", b_neck, b_neck_desc));
	if (b_outer >= 0)
		borg_note(format("# Best Outer address: %d, %s", b_outer, b_outer_desc));
	if (b_shield >= 0)
		borg_note(
			 format("# Best Shield address: %d, %s", b_shield, b_shield_desc));
	if (b_helm >= 0)
		borg_note(format("# Best Helm address: %d, %s", b_helm, b_helm_desc));
	if (b_glove >= 0)
		borg_note(format("# Best Glove address: %d, %s", b_glove, b_glove_desc));
	if (b_boot >= 0)
		borg_note(format("# Best Boot address: %d, %s", b_boot, b_boot_desc));
	if (b_lite >= 0)
		borg_note(format("# Best Lite address: %d, %s", b_lite, b_lite_desc));
	if (b_body >= 0)
		borg_note(format("# Best Body address: %d, %s", b_body, b_body_desc));
#if 0
	/* Make first change. */
    for (k = 0; k < 12; k++)
    {
        /* Get choice */
        i = borg_best_stuff_order[INVEN_WIELD + k];

        /* Ignore non-changes */
        if (i == borg_best_stuff_order[INVEN_WIELD + k] || 255 == i) continue;

        if (i < 100)
        {
            /* weild the item */
            borg_item *item = &borg_items[i];
            borg_note(format("# Best Combo %s.", item->desc));

            borg_keypress('w');
            borg_keypress(I2A(i));
            time_this_panel ++;

			/* Track the newly worn artifact item to avoid loops */
			if (track_worn_num <= track_worn_size)
			{
				track_worn_tval[track_worn_num] = item->tval;
				track_worn_sval[track_worn_num] = item->sval;
				track_worn_pval[track_worn_num] = item->pval;
				track_worn_name1[track_worn_num] = item->name1;
				track_worn_name2[track_worn_num] = item->name2;
				track_worn_time = borg_t;
				track_worn_num++;

				/* Step down the list if needed */
				if (track_worn_num >= track_worn_size)
				{
					for (track = 1; track < track_worn_size; track++)
					{
						track_worn_tval[track] = track_worn_tval[track - 1];
						track_worn_sval[track] = track_worn_sval[track - 1];
						track_worn_pval[track] = track_worn_pval[track - 1];
						track_worn_name1[track] = track_worn_name1[track - 1];
						track_worn_name2[track] = track_worn_name2[track - 1];
					}
				}
			}

			/* Wearing the item may have altered our inventory.  We need to finish our best gear check */
			return (1);
        }
        else
        {
            borg_item *item;

            /* can't get an item if full. */
            if (borg_items[hole].iqty) return (FALSE);

            i-=100;

            item = &borg_shops[BORG_HOME].ware[i];

            /* Dont do it if you just sold this item */
            if  ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
				(sold_item_tval == item->tval && sold_item_sval == item->sval &&
                 sold_item_store == BORG_HOME)) return (FALSE);
			else if	(sold_item_tval == item->tval && sold_item_sval == item->sval &&
					sold_item_pval == item->pval && sold_item_store == BORG_HOME) return (FALSE);

            /* Get the item */
            borg_note(format("# Getting (Best Fit) %s.", item->desc));

            /* Minor Hack -- Go to the correct page */
            if ((i / 12) != borg_shops[BORG_HOME].page)
            {
                borg_keypress(' ');
            }

            borg_keypress('p');
            borg_keypress(I2A(i % 12));

            /* press enter a few time (mulitple objects) */
            borg_keypress('\r');
            borg_keypress('\r');
            borg_keypress('\r');
            borg_keypress('\r');


            /* tick the clock */
            time_this_panel ++;

            return (TRUE);
        }
    }
#endif
	return (FALSE);
}

bool borg_best_stuff(void) {
	int hole = INVEN_PACK - 1;

	int k;

	s32b value;

	int i;

	byte test[12];
	byte best[12];

	/*char purchase_target[1];*/
	/*byte t_a;*/
	/*char buf[1024];*/
	int /*p, */track;

	/* Hack -- Anti-loop */
	if (time_this_panel >= 300)
		return (FALSE);
	if (borg_t - borg_began >= 600)
		return (FALSE);

	/* Not if stats are drained at all */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP])
		return (FALSE);

	/* Update the inventory in case it changed after our last equipment change */
	borg_cheat_inven();
	borg_cheat_equip();

	/* Hack -- Initialize */
	for (k = 0; k < 12; k++) {
		/* Initialize */
		best[k] = test[k] = 255;
	}

	/* Hack -- Copy all the slots */
	for (i = 0; i < INVEN_TOTAL; i++) {
		/* Save the item */
		COPY(&safe_items[i], &borg_items[i], borg_item);
	}

	if (shop_num == BORG_HOME) {
		/* Hack -- Copy all the store slots */
		for (i = 0; i < STORE_INVEN_MAX; i++) {
			/* Save the item */
			COPY(&safe_home[i], &borg_shops[BORG_HOME].ware[i], borg_item);
		}
	}

	/* Evaluate the inventory */
	borg_notice(FALSE);
	value = borg_power();

	/* Determine the best possible equipment */
	(void)borg_best_stuff_aux2(0, test, best, &value);

	/* Restore things incase they got screwed up */
	borg_cheat_inven();
	borg_cheat_equip();
	borg_notice(TRUE);

	/* Detail the best stuff list */
	if (borg_verbose) {
		for (k = 0; k < 12; k++) {
			borg_item *item;

			/* Get choice */
			i = best[k];

			if (i == 255) {
				borg_note(format("# Best Combo %d: blank.", k));
			} else if (i < 100) {
				/* describe the item. */
				item = &borg_items[i];
				borg_note(format("# Best Combo %d: %s.", k, item->desc));
			} else {
				i -= 100;
				item = &borg_shops[BORG_HOME].ware[i];
				borg_note(
					 format("# Best Combo %d: %s. (home item)", k, item->desc));
			}

			/* Note if change is required */
			if (i != borg_best_stuff_order[k] && i != 255) {
				borg_note("# Best Combo: must swap out current");
			}
		}
	}

	/* Make first change. */
	for (k = 0; k < 12; k++) {
		/* Get choice */
		i = best[k];

		/* Ignore non-changes */
		if (i == borg_best_stuff_order[k] || 255 == i)
			continue;

		if (i < 100) {
			/* weild the item */
			borg_item *item = &borg_items[i];
			borg_note(format("# Best Combo %s.", item->desc));

			borg_keypress('w');
			borg_keypress(I2A(i));
			time_this_panel++;

			/* Track the newly worn artifact item to avoid loops */
			if (track_worn_num <= track_worn_size) {
				track_worn_tval[track_worn_num] = item->tval;
				track_worn_sval[track_worn_num] = item->sval;
				track_worn_pval[track_worn_num] = item->pval;
				track_worn_name1[track_worn_num] = item->name1;
				track_worn_name2[track_worn_num] = item->name2;
				track_worn_time = borg_t;
				track_worn_num++;

				/* Step down the list if needed */
				if (track_worn_num >= track_worn_size) {
					for (track = 1; track < track_worn_size; track++) {
						track_worn_tval[track] = track_worn_tval[track - 1];
						track_worn_sval[track] = track_worn_sval[track - 1];
						track_worn_pval[track] = track_worn_pval[track - 1];
						track_worn_name1[track] = track_worn_name1[track - 1];
						track_worn_name2[track] = track_worn_name2[track - 1];
					}
				}
			}

			borg_best_stuff_order[k] = 255;
			/* Wearing the item may have altered our inventory.  We need to finish
			 * our best gear check */
			return (1);
		} else {
			borg_item *item;

			/* can't get an item if full. */
			if (borg_items[hole].iqty)
				return (FALSE);

			i -= 100;

			item = &borg_shops[BORG_HOME].ware[i];

			/* Dont do it if you just sold this item */
			if ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
				 (sold_item_tval == item->tval && sold_item_sval == item->sval &&
				  sold_item_store == BORG_HOME))
				return (FALSE);
			else if (sold_item_tval == item->tval &&
						sold_item_sval == item->sval &&
						sold_item_pval == item->pval && sold_item_store == BORG_HOME)
				return (FALSE);

			/* Get the item */
			borg_note(format("# Getting (Best Fit) %s.", item->desc));

			/* Minor Hack -- Go to the correct page */
			if ((i / 12) != borg_shops[BORG_HOME].page) {
				borg_keypress(' ');
			}

			borg_keypress('p');
			borg_keypress(I2A(i % 12));

			/* press enter a few time (mulitple objects) */
			borg_keypress('\r');
			borg_keypress('\r');
			borg_keypress('\r');
			borg_keypress('\r');

			/* tick the clock */
			time_this_panel++;

			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}

/*
 * Study and/or Test spells/prayers
 */
bool borg_play_magic(bool bored) {

	int rate, b_rate = -1;
	int realm, b_realm = -1;
	int book, b_book = -1;
	int spell, b_spell = -1;
	int inven, b_inven = -1;

	/* Hack -- must use magic or prayers */
	if (!mp_ptr->spell_book)
		return (FALSE);

	/* Hack -- blind/confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Dark */
	if (!borg_skill[BI_CUR_LITE])
		return (FALSE);
	/*    if (borg_grids[c_y][c_x].info == BORG_DARK) return (FALSE); */

	/* Check each realm, backwards */
	for (realm = MAX_REALM; realm > 0; realm--) {
		/* skip non my realms */
		if (realm != borg_skill[BI_REALM1] && realm != borg_skill[BI_REALM2])
			continue;

		/* Check each book (backwards) */
		for (book = 3; book >= 0; book--) {
			/* Look for the book */
			inven = borg_book[realm][book];

			/* No such book */
			if (inven < 0)
				continue;

			/* Check each spells */
			for (spell = 7; spell >= 0; spell--) {
				borg_magic *as = &borg_magics[realm][book][spell];

				/* Require "learnable" status */
				if (as->status != BORG_MAGIC_OKAY)
					continue;

				/* Obtain "rating" */
				rate = as->rating;

				/* Skip "boring" spells/prayers */
				if (!bored && (rate <= 50))
					continue;

				/* Skip "icky" spells/prayers */
				if (rate <= 0)
					continue;

				/* Skip "worse" spells/prayers */
				if (rate <= b_rate)
					continue;

				/* Track it */
				b_inven = inven;
				b_rate = rate;
				b_realm = realm;
				b_book = book;
				b_spell = spell;
			}
		} /* book */
	}	 /* realm  */

	/* Study */
	if (borg_skill[BI_ISSTUDY] && (b_rate > 0)) {

		/* Realm */
		borg_magic *as = &borg_magics[b_realm][b_book][b_spell];

		/* Debugging Info */
		borg_note(format("# Studying %s spell %s.", as->realm_name, as->name));

		/* Learn the spell */
		borg_keypress('G');

		/* Specify the book */
		borg_keypress(I2A(b_inven));

		/* Some Classes can not choose */
		if (borg_class != CLASS_PRIEST && borg_class != CLASS_PALADIN &&
			 borg_class != CLASS_MONK) {
			/* Specify the spell */
			borg_keypress(I2A(b_spell));
		}

		/* Success */
		return (TRUE);
	}

	/* Hack -- only in town */
	if (borg_skill[BI_CDEPTH] && !borg_munchkin_mode)
		return (FALSE);

	/* Check each realm backwards */
	for (realm = MAX_REALM; realm > 0; realm--) {
		/* Check each book (backwards) */
		for (book = 3; book >= 0; book--) {

			/* Only my realms */
			if (realm != borg_skill[BI_REALM1] && realm != borg_skill[BI_REALM2])
				continue;

			/* Look for the book */
			inven = borg_book[realm][book];

			/* No such book */
			if (inven < 0)
				continue;

			/* Check every spell (backwards) */
			for (spell = 8 - 1; spell >= 0; spell--) {
				borg_magic *as = &borg_magics[realm][book][spell];

				/* Only try "untried" spells/prayers */
				if (as->status != BORG_MAGIC_TEST)
					continue;

				/* Ignore "bizarre" spells/prayers */
				if (as->method == BORG_MAGIC_OBJ)
					continue;
				if (as->method == BORG_MAGIC_ICK)
					continue;

				/* Make sure I have enough mana */
				if (borg_skill[BI_CURSP] < as->power)
					continue;

				/* If in munchkin mode, do not test spells which cause me to move
				 * off my stair */
				if (borg_munchkin_mode) {
					/* Check the different realms for Phase Door */
					if (realm == REALM_SORCERY && book == 0 && spell == 1)
						continue;
					if (realm == REALM_CHAOS && book == 0 && spell == 7)
						continue;
					if (realm == REALM_TRUMP && book == 0 && spell == 0)
						continue;
					if (realm == REALM_TRUMP && book == 0 && spell == 4)
						continue;
					if (realm == REALM_TRUMP && book == 0 && spell == 5)
						continue;
					if (realm == REALM_ARCANE && book == 0 && spell == 4)
						continue;
				}

				/* Note */
				borg_note("# Testing untried spell/prayer");

				/* Hack -- Use spell or prayer */
				if (borg_spell(realm, book, spell)) {
					/* Hack -- Allow attack spells */
					if (as->method == BORG_MAGIC_AIM) {
						/* Hack -- target self */
						borg_keypress('*');
						borg_keypress('p');
						borg_keypress('t');
					}

					/* Hack -- Allow dimension Door */
					if (as->method == BORG_MAGIC_EXT) {
						/* Hack -- target self */
						borg_keypress(' ');
					}

					/* Hack -- Allow genocide spells */
					if (as->method == BORG_MAGIC_WHO) {
						/* Hack -- target self */
						borg_keypress('t');
					}

					/* Success */
					return (TRUE);
				}
			} /* spells */
		}	 /* books */
	}		  /* realm */

	/* Nope */
	return (FALSE);
}

/*
 * Determine if an item can be sold in the given store
 *
 * XXX XXX XXX Consider use of "icky" test on items
 */
extern bool borg_good_sell(borg_item *item, int who) {
	/* Never sell worthless items */
	if (item->value <= 0)
		return (FALSE);

	/* Never sell cursed items */
	if (item->cursed)
		return (FALSE);

	/* Never sell valuable non-id'd items */
	if (strstr(item->note, "good") || strstr(item->note, "excellent") ||
		 strstr(item->note, "Quest") || strstr(item->note, "special"))
		return (FALSE);

	/* Worshipping gold or scumming will allow the sale */
	if (item->value > 0 &&
		 ((borg_worships_gold || borg_skill[BI_MAXCLEVEL] < 10) ||
		  (borg_money_scum_amount < borg_gold && borg_money_scum_amount != 0)) &&
			!streq(item->note, "good") && !streq(item->note, "excellent")) {
		/* Borg is allowed to continue in this routine to sell non-ID items */
	} else /* Some items must be ID, or at least 'known' */
	{
		/* Analyze the type */
		switch (item->tval) {
		case TV_POTION:
		case TV_SCROLL:

			/* Never sell if not "known" and interesting */
			if (!item->aware && (borg_skill[BI_MAXDEPTH] > 10))
				return (FALSE);

			break;

		case TV_FOOD:
		case TV_ROD:
		case TV_WAND:
		case TV_STAFF:
		case TV_RING:
		case TV_AMULET:
		case TV_LITE:

			/* Never sell if not "known" */
			if (!item->aware && (borg_skill[BI_MAXDEPTH] > 10))
				return (FALSE);

			break;

		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:

			/* Only sell "known" items (unless "icky") */
			if (!item->aware && !borg_item_icky(item))
				return (FALSE);

			break;
		}
	}

	/* Do not sell stuff that is not fully id'd and should be  */
	if (!item->fully_identified && item->name1) {
		/* CHECK THE ARTIFACTS */
		/* For now check all artifacts */
		return (FALSE);
	}
	/* Do not sell stuff that is not fully id'd and should be  */
	if (!item->fully_identified && item->name2) {
		switch (item->name2) {
		/* Weapon (Blessed) */
		case EGO_BLESS_BLADE:
		/* Armor of Permanence */
		case EGO_PERMANENCE:
		/* Armor of Elvenkind */
		case EGO_ELVENKIND:
		/* Crown of the Magi */
		case EGO_MAGI:
		/* Cloak of Aman */
		case EGO_AMAN:
		/* Weapon (Holy Avenger) */
		case EGO_HA:
		/* Weapon (Defender) */
		case EGO_DF:
		/* Trump weapons */
		case EGO_TRUMP:
		/* Chaotic weapon */
		case EGO_CHAOTIC:
		/* Resistance items */
		case EGO_RESISTANCE:
		/* of Might */
		case EGO_MIGHT:
		/* of Lordliness */
		case EGO_LORDLINESS:
			return (FALSE);
		/* anything else */
		default:
			break;
		}
	}

	/* do not sell the item (unless to Home) if I just bought it. */
	if (bought_item_tval == item->tval && bought_item_sval == item->sval &&
		 bought_item_store != BORG_HOME) {
		return (FALSE);
	}

	/* if it looks half good, ID it first */
	if (strstr(item->desc, "{good}") || strstr(item->desc, "{excellent}") ||
		 (strstr(item->desc, "Phial") && !item->ident))
		return (FALSE);

	/* Switch on the store */
	switch (who + 1) {
	/* General Store */
	case 1:

		/* Analyze the type */
		switch (item->tval) {
		case TV_DIGGING:
		case TV_CLOAK:
		case TV_FOOD:
		case TV_FLASK:
		case TV_LITE:
		case TV_SPIKE:
			return (TRUE);
		}
		break;

	/* Armoury */
	case 2:

		/* Analyze the type */
		switch (item->tval) {
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			return (TRUE);
		}
		break;

	/* Weapon Shop */
	case 3:

		/* Analyze the type */
		switch (item->tval) {
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:

			return (TRUE);
		}
		break;

	/* Temple */
	case 4:

		/* Analyze the type */
		switch (item->tval) {
		case TV_HAFTED:
		case TV_LIFE_BOOK:
		case TV_SCROLL:
		case TV_POTION:
			return (TRUE);
		}
		break;

	/* Alchemist */
	case 5:

		/* Analyze the type */
		switch (item->tval) {
		case TV_SCROLL:
		case TV_POTION:
			return (TRUE);
		}
		break;

	/* Magic Shop */
	case 6:

		/* Analyze the type */
		switch (item->tval) {
		case TV_AMULET:
		case TV_RING:
		case TV_SCROLL:
		case TV_POTION:
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
		case TV_SORCERY_BOOK:
		case TV_NATURE_BOOK:
		case TV_CHAOS_BOOK:
		case TV_DEATH_BOOK:
		case TV_TRUMP_BOOK:
		case TV_ARCANE_BOOK:
			return (TRUE);
		}
		break;

	/* Black Market */

	/* Home */

	/* Book Shop */
	case 9:

		/* Analyze the type */
		switch (item->tval) {
		case TV_LIFE_BOOK:
		case TV_SORCERY_BOOK:
		case TV_NATURE_BOOK:
		case TV_CHAOS_BOOK:
		case TV_DEATH_BOOK:
		case TV_TRUMP_BOOK:
		case TV_ARCANE_BOOK:
			return (TRUE);
		}
		break;
	}

	/* Assume not */
	return (FALSE);
}

/*
 * Count the number of items worth "selling"
 *
 * This determines the choice of stairs.
 */
int borg_count_sell(void) {
	/*int icky = STORE_INVEN_MAX - 1;*/

	int k /*, b_k = -1*/;
	int i /*, b_i = -1*/;
	int qty = 1;
	s32b p, b_p = 0L;
	/*s32b c = 0L;*/
	/*s32b b_c = 30001L;*/
	int sell_count = 0;

	bool fix = FALSE;

	/* Evaluate */
	b_p = borg_power();

	/* Sell stuff */
	for (i = 0; i < INVEN_PACK; i++) {

		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip some important type items */
		if ((item->tval == my_ammo_tval) && (borg_skill[BI_AMISSILES] < 45))
			continue;
		if ((borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE) &&
			 item->tval == TV_ROD && item->sval == SV_ROD_HEALING &&
			 borg_has[ROD_HEAL] <= ROD_HEAL_GOAL)
			continue;

		if (borg_class == CLASS_WARRIOR && item->tval == TV_ROD &&
			 item->sval == SV_ROD_MAPPING && item->iqty <= 2)
			continue;

		/* Avoid selling staff of dest*/
		if (item->tval == TV_STAFF && item->sval == SV_STAFF_DESTRUCTION &&
			 borg_skill[BI_ASTFDEST] < 2)
			continue;

		/* dont sell our swap items */
		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;

		/* Check each shop */
		for (k = 0; k < (MAX_STORES); k++) {

			/* skip the home */
			if (k == BORG_HOME)
				continue;

			/* Save the store hole */
			/* COPY(&safe_shops[k].ware[icky], &borg_shops[k].ware[icky],
			 * borg_item); */

			/* Skip "bad" sales */
			if (!borg_good_sell(item, k))
				continue;

			/* Save the item */
			COPY(&safe_items[i], &borg_items[i], borg_item);

			/* Give the item to the shop */
			/* COPY(&borg_shops[k].ware[icky], &safe_items[i], borg_item); */

			/* get the quantity */
			qty = 1;

			/* Give a single item */
			/* borg_shops[k].ware[icky].iqty = qty; */

			/* Lose a single item */
			borg_items[i].iqty -= qty;

			/* Fix later */
			fix = TRUE;

			/* Examine the inventory */
			borg_notice(FALSE);

			/* Evaluate the inventory with this item gone*/
			p = borg_power();

			/* Restore the item */
			COPY(&borg_items[i], &safe_items[i], borg_item);

			/* Restore the store hole */
			/* COPY(&borg_shops[k].ware[icky], &safe_shops[k].ware[icky],
			 * borg_item); */

			/* Compare power after the sale */
			if (p < b_p)
				continue;

			/* Count the number of things to sell */
			sell_count++;

			/* Do not sell this to the next store as well. */
			break;
		}

		/* Restore the store hole */
		/* COPY(&borg_shops[k].ware[icky], &safe_shops[k].ware[icky], borg_item);
		 */
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Assume not */
	return (sell_count);
}

/*
 * Scan the item list and recharge items before leaving the
 * level.  Right now rod are not recharged from this.
 */
bool borg_wear_recharge(void) {
	int i, b_i = -1;
	int slot = -1;

	/* No resting in danger */
	if (!borg_check_rest(c_y, c_x))
		return (FALSE);

	/* Not if hungry */
	if (borg_skill[BI_ISWEAK])
		return (FALSE);

	/* Not if I am not supposed to be resting */
	if (borg_no_rest_prep >= 1)
		return (FALSE);

	/* Look for an (wearable- non rod) item to recharge */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];
		object_type *o_ptr;	 /* cheat */
		o_ptr = &inventory[i]; /* cheat */

		/* Skip empty items */
		if (!item->iqty)
			continue;
		if (!item->kind)
			continue;

		/* skip items that are charged */
		if (!item->timeout)
			continue;

		/* Skip the Phial, it recharges fast enough */
		if (i == INVEN_LITE)
			continue;

		/* Cheat-- the borg is misreading his equip.
		 * So this is pulling the info right from the game.
		 */
		if (!o_ptr->timeout)
			continue;

		/* Where does this belong? */
		slot = borg_wield_slot(item);

		/* Skip stuff that can't be worn */
		if (slot < INVEN_WIELD)
			continue;

		/* note this one */
		b_i = i;
	}

	if (b_i >= INVEN_WIELD) {
		/* Item is worn, no swap is nec. */
		borg_note(
			 format("# Waiting for '%s' to Recharge.", borg_items[b_i].desc));

		/* Rest for a while */
		borg_keypress('R');
		borg_keypress('7');
		borg_keypress('5');
		borg_keypress('\n');

		/* done */
		return (TRUE);
	}
	/* Item must be worn to be recharged
	 * But, none if some equip is cursed
	 */
	else if (b_i >= INVEN_WIELD && b_i <= INVEN_PACK && !borg_wearing_cursed) {

		/* wear the item */
		borg_note("# Swapping Item for Recharge.");
		borg_keypress(ESCAPE);
		borg_keypress('w');
		borg_keypress(I2A(b_i));
		borg_keypress(' ');
		borg_keypress(' ');

		/* rest for a while */
		borg_keypress('R');
		borg_keypress('7');
		borg_keypress('5');
		borg_keypress('\n');

		/* done */
		return (TRUE);
	}

	/* nothing to recharge */
	return (FALSE);
}

/*
 * Leave the level if necessary (or bored)
 * Scumming defined in borg_prepared.
 */
bool borg_leave_level(bool bored) {
	int k, g = 0;
	bool fearing_sunlight = FALSE;

	/* Hack -- waiting for "recall" other than depth 1 */
	if (goal_recalling && borg_skill[BI_CDEPTH] > 1)
		return (FALSE);

	/* Not bored if I have seen Lucifer recently or a questor */
	if ((borg_skill[BI_CDEPTH] == LUCIFER_DEPTH && (borg_depth & DEPTH_QUEST) &&
		  (borg_t - borg_t_questor < 5000)) ||
		 borg_fighting_questor) {
		goal_leaving = FALSE;
		goal_rising = FALSE;
		bored = FALSE;
	}

	/* There is a great concern about recalling back to level 100.
	 * Often the borg will fall down a trap door to level 100 when he is not
	 * prepared to be there.  Some classes can use Teleport Level to get
	 * back up to 99,  But Warriors cannot.  Realistically the borg needs
	 * be be able to scum deep in the dungeon.  But he cannot risk being
	 * on 100 and using the few *Healing* pots that he managed to collect.
	 * It is better for warriors to walk all the way down to 98 and scum.
	 * It seems like a long and nasty crawl, but it is the best way to
	 * make sure the borg survives.  Along the way he will collect the
	 * Healing, Life and *Healing* that he needs.
	 *
	 * The other classes (or at least those who can use the Teleport Level
	 * spell) will not need to do this nasty crawl.  Risky Borgs will
	 * not crawl either.
	*
	* The Trump Tower will allow a borg to jump to a good depth.
	 */

	/* Town */
	if (!borg_skill[BI_CDEPTH]) {
		/* Cancel rising */
		goal_rising = FALSE;

		/* Wait until bored */
		if (!bored)
			return (FALSE);

		/* If rich, or not prepped for depth, use the trump tower. */
		if (borg_gold > 1000 &&
			 ((borg_skill[BI_RECALL] <= 2) ||
			  ((cptr)NULL == borg_prepared[borg_skill[BI_MAXDEPTH] + 10]) ||
			  (borg_skill[BI_MAXDEPTH] == 100 &&
				borg_skill[BI_ATELEPORTLVL] == 0) ||
			  ((cptr)NULL != borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10]))) {
			/* Flow to Trump Tower */
			if (borg_flow_shop_trump())
				return (TRUE);
		}

		/* Case for those who cannot Teleport Level */
		if (borg_skill[BI_MAXDEPTH] == 100 && !borg_plays_risky) {
			if (borg_skill[BI_ATELEPORTLVL] == 0) {
				/* Note */
				borg_note("# Fleeing.  Borg must crawl to deep dungeon- no recall "
							 "to 100.");

				/* These pple must crawl down to 100, Sorry */
				goal_fleeing = TRUE;
				goal_leaving = TRUE;
				stair_more = TRUE;

				/* Attempt to use those stairs */
				if (borg_flow_stair_more(GOAL_BORE, FALSE, FALSE))
					return (TRUE);

				/* Oops */
				return (FALSE);
			}
		}

		/* Hack -- Recall into dungeon */
		if ((borg_skill[BI_MAXDEPTH] >= (borg_worships_gold ? 10 : 8)) &&
			 (borg_skill[BI_RECALL] >= 3) && !borg_skill[BI_FEAR_LITE] &&
			 ((cptr)NULL == borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10])) {
			/* But first try to heal up prior to re-entering the dungeon */
			if (borg_recover()) {
				/* Note */
				borg_note("# Recovering prior to recalling into dungeon.");

				/* Give it a shot */
				return (TRUE);
			}

			if (borg_recall()) {
				/* Note */
				borg_note("# Recalling into dungeon.");

				/* Give it a shot */
				return (TRUE);
			}
		} else {
			/* note why we didn't recall. */
			if (borg_skill[BI_MAXDEPTH] < (borg_worships_gold ? 10 : 8))
				borg_note("# Not deep enough to recall");
			else {
				if (borg_skill[BI_RECALL] <= 2)
					borg_note("# Not enough recalls to recall");
				else if (borg_skill[BI_FEAR_LITE])
					borg_note("# Need RLight before I can recall down.");
				else {
					/* recall unless way out of our league */
					if ((cptr)NULL !=
						 borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10]) {
						cptr reason = borg_prepared[borg_skill[BI_MAXDEPTH]];
						borg_slow_return = TRUE;
						borg_note(format("# Way too scary to recall down there!   %s",
											  reason));
						borg_slow_return = FALSE;
					} else
						borg_note("# failed to recall when I wanted to");
				}
			}

			/* Note */
			borg_note("# Fleeing in order to enter dungeon.");
			goal_fleeing = TRUE;
			goal_leaving = TRUE;
		}

		stair_more = TRUE;

		/* Try to get to town location (town gate for now) */
		if (borg_flow_town_exit(GOAL_TOWN))
			return (TRUE);

		/* Attempt to use those stairs */
		if (borg_flow_stair_more(GOAL_BORE, FALSE, FALSE))
			return (TRUE);

		/* Oops */
		return (FALSE);
	}

	/** In the Dungeon **/

	/* Avoid sitting on level 1 during daylight hours.  Try to stay deeper in the
	 * dungeon until night approaches. */
	if (borg_skill[BI_FEAR_LITE] && (borg_skill[BI_HRTIME] >= 5) &&
		 (borg_skill[BI_HRTIME] <= 18))
		fearing_sunlight = TRUE;

	/* do not hangout on boring levels for *too* long */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1])
		g = 1;

	/* Count sellable items */
	k = borg_count_sell();

	/* Do not dive when "full" of items */
	if (g && (k >= 12))
		g = 0;

	/* Do not dive when drained */
	if (g && borg_skill[BI_ISFIXEXP])
		g = 0;

	/* Rise a level if bored and unable to dive. */
	if (bored && ((cptr)NULL != borg_prepared[borg_skill[BI_CDEPTH] + 1])) {
		cptr reason = borg_prepared[borg_skill[BI_CDEPTH] + 1];
		g = -1;
		borg_slow_return = TRUE;
		borg_note(format("# heading up (bored and unable to dive: %s)", reason));
		borg_slow_return = FALSE;
	}

	/* Rise a level if bored and spastic. */
	if (bored && avoidance > borg_skill[BI_CURHP]) {
		g = -1;
		borg_slow_return = TRUE;
		borg_note("# heading up (bored and spastic).");
		borg_slow_return = FALSE;
	}

	/* Power dive if I am playing too shallow and not needing to sell stuff */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1] && k < 13)
		g = 1;

	/* Power dive if I am playing deep */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1] &&
		 borg_skill[BI_CDEPTH] >= 75)
		g = 1;

	/* Hack -- Power-climb upwards when needed */
	if ((cptr)NULL != borg_prepared[borg_skill[BI_CDEPTH]] &&
		 !(borg_depth & DEPTH_QUEST)) {
		cptr reason = borg_prepared[borg_skill[BI_CDEPTH]];

		borg_slow_return = TRUE;
		borg_note(format("# heading up (too deep: %s)", reason));
		borg_slow_return = FALSE;
		g = -1;

		/* if I must restock go to town */
		if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH]) && bored) {
			cptr reason = borg_prepared[borg_skill[BI_CDEPTH]];

			borg_note(
				 format("# returning to town to restock(too deep: %s)", reason));
			goal_rising = TRUE;
		}

		/* if I am really out of depth go to town */
		if ((cptr)NULL != borg_prepared[borg_skill[BI_MAXDEPTH] * 5 / 10]) {
			cptr reason = borg_prepared[borg_skill[BI_CDEPTH]];
			borg_slow_return = TRUE;
			borg_note(format("# returning to town (too deep: %s)", reason));
			goal_rising = TRUE;
			borg_slow_return = FALSE;
		}
	}

	/* Hack -- if I am playing way too shallow return to town */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 20] &&
		 (cptr)NULL == borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10] &&
		 borg_skill[BI_MAXDEPTH] > borg_skill[BI_CDEPTH] + 10 &&
		 (borg_skill[BI_RECALL] >= 3 || borg_gold > 2000)) {
		borg_note("# returning to town to recall back down (too shallow)");
		goal_rising = TRUE;
	}

	/* Hack -- It is much safer to scum for items on 98
	 * Check to see if depth 99, if Sauron is dead and Im not read to fight
	 * the final battle
	 */
	if (borg_skill[BI_CDEPTH] == LILITH_DEPTH &&
		 borg_race_death[RACE_LILITH] == 1 && borg_ready_lucifer != 1) {
		borg_note("# Returning to level 98 to scum for items.");
		g = -1;
	}

	/* Power dive too 100 if ready */
	if ((cptr)NULL == borg_prepared[100])
		g = 1;

	/* Power dive if Morgoth is dead */
	if (borg_skill[BI_KING])
		g = 1;

	/* Return to town to sell stuff -- No recall allowed.*/
	if ((borg_worships_gold && borg_skill[BI_MAXCLEVEL] <= 20) && (k >= 12) &&
		 !fearing_sunlight) {
		borg_note("# Going to town (Sell Stuff, Worshipping Gold).");
		g = -1;
	}

	/* Return to town to sell stuff */
	if (bored && (k >= 12) && !fearing_sunlight) {
		borg_note("# Going to town (Sell Stuff).");
		goal_rising = TRUE;
	}

	/* Return to town to sell too much stuff */
	if (k >= 14 && !fearing_sunlight) {
		borg_note("# Going to town (Sell lots of stuff).");
		goal_rising = TRUE;
	}

	/* Return to town when level drained */
	if (borg_skill[BI_ISFIXLEV] && !fearing_sunlight) {
		borg_note("# Going to town (Fix Level).");
		goal_rising = TRUE;
	}

	/* Return to town to restore experience */
	if (bored && borg_skill[BI_ISFIXEXP] && borg_skill[BI_CLEVEL] != 50 &&
		 !fearing_sunlight && !(borg_depth & DEPTH_QUEST)) {
		borg_note("# Going to town (Fix Experience).");
		goal_rising = TRUE;
	}

	/* return to town if it has been a while */
	if ((!goal_rising && bored && !(borg_depth & DEPTH_QUEST) &&
		  !(borg_depth & DEPTH_VAULT) && !borg_fighting_unique &&
		  !fearing_sunlight && borg_time_town + borg_t - borg_began > 8000) ||
		 (borg_time_town + borg_t - borg_began > 12000)) {
		borg_note("# Going to town (I miss my home).");
		goal_rising = TRUE;
	}

	/* return to town if been scumming for a bit */
	if (borg_skill[BI_MAXDEPTH] >= borg_skill[BI_CDEPTH] + 10 &&
		 borg_skill[BI_CDEPTH] <= 12 && !fearing_sunlight &&
		 borg_time_town + borg_t - borg_began > 3500) {
		borg_note("# Going to town (scumming check).");
		goal_rising = TRUE;
	}

	/* Low level dudes need to visit town more frequently to keep up on food */
	if (borg_skill[BI_CLEVEL] < 15) {
		if (borg_time_town + (borg_t - borg_began) >
				  (borg_skill[BI_CLEVEL] * 250) ||
			 borg_time_town + (borg_t - borg_began) > 2500 ||
			 (borg_time_town + (borg_t - borg_began) > 2000 &&
			  borg_skill[BI_REG])) {
			borg_note("# Going to town (short trips).");
			g = -1;
		}
	}

	/* Return to town to drop off some scumming stuff */
	if (borg_skill[BI_AHEAL] >= 1 && !(borg_depth & DEPTH_VAULT) &&
		 (borg_skill[BI_AEZHEAL] > 1 || borg_skill[BI_ALIFE] > 1) &&
		 borg_scumming_pots) {
		borg_note("# Going to town (Dropping off Potions).");
		goal_rising = TRUE;
	}

	/* if returning to town, try to go upstairs */
	if (goal_rising)
		g = -1;

	/* Mega-Hack -- spend time on the first level to rotate shops */
	if (borg_skill[BI_CLEVEL] > 10 && (borg_skill[BI_CDEPTH] == 1) &&
		 (borg_t - borg_began < 100) && (g < 0)) {
		g = 0;
	}

	/* Use random stairs when really bored */
	if (bored && (borg_t - borg_began >= 5000)) {
		/* Note */
		borg_note("# Choosing random stairs.");

		/* Use random stairs */
		g = ((rand_int(100) < 50) ? -1 : 1);
	}

	/* Use random stairs when on Breeder level */
	if ((borg_depth & DEPTH_BREEDER) && (borg_t - borg_began >= 1000)) {
		/* Note */
		borg_note("# Choosing random stairs.  Leaving breeder level.");

		/* Use random stairs */
		g = ((rand_int(100) < 50) ? -1 : 1);
	}

	/* Go Up */
	if (g < 0) {
		/* Take next stairs */
		stair_less = TRUE;

		/* Hack -- recall if going to town */
		if (goal_rising && ((borg_time_town + (borg_t - borg_began)) > 200) &&
			 (borg_skill[BI_CDEPTH] >= 5) && borg_recall()) {
			borg_note("# Recalling to town (goal rising)");
			return (TRUE);
		}

		/* Attempt to use stairs */
		if (borg_flow_stair_less(GOAL_BORE, FALSE))
			return (TRUE);

		/* Cannot find any stairs */
		if (goal_rising && bored && (borg_t - borg_began) >= 1000) {
			if (borg_recall()) {
				borg_note("# Recalling to town (no stairs)");
				return (TRUE);
			}
		}
	}

	/* Go Down */
	if (g > 0) {
		/* Take next stairs */
		stair_more = TRUE;

		/* Attempt to use those stairs */
		if (borg_flow_stair_more(GOAL_BORE, FALSE, TRUE)) {
			/* Give a short reason */
			borg_note("# Looking for down stairs from boredom.");
			return (TRUE);
		}
	}

	/* Failure */
	return (FALSE);
}

/*
 * Initialize this file
 */
void borg_init_7(void) { /* Nothing */
}

#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
