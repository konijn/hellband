/* File: borg8.c */
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
#include "zborg8.h"

#ifdef BABLOS
extern bool borg_clock_over;
#endif /* bablos */

byte *test;
byte *best;
s32b *b_home_power;

char *borg_itoa(long i, char *s /*, int dummy_radix*/) {
	sprintf(s, "%ld", i);
	return s;
}

/* money Scumming is a type of town scumming for money */
void borg_money_scum(void) {

	int dir;

	/* Take note */
	borg_note(format("# Waiting for towns people to breed.  I need %d...",
						  borg_money_scum_amount - borg_gold));

	/* Rest for 9 months */
	if (borg_skill[BI_CLEVEL] >= 35) {
		borg_keypress(ESCAPE);
		borg_keypress('R');
		borg_keypress('9');
		borg_keypress('9');
		borg_keypress('9');
		borg_keypress('9');
		borg_keypress('\n');
	} else if (borg_skill[BI_CLEVEL] >= 15) {
		borg_keypress(ESCAPE);
		borg_keypress('R');
		borg_keypress('2');
		borg_keypress('5');
		borg_keypress('\n');
	} else /* Low level, dont want to get mobbed */
	{
		borg_keypress(ESCAPE);
		borg_keypress('R');
		borg_keypress('5');
		borg_keypress('\n');
	}

	/* sometimes twitch in order to move around some */
	if (borg_t % 10) {
		borg_keypress(ESCAPE);

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
	}

	/* reset the clocks */
	borg_t = 10;
	time_this_panel = 1;
	borg_began = 1;

	/* Done */
	return;
}

/*
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow wands/staffs (if they are known to have equal
 * charges) and rods (if fully charged) to combine.
 *
 * Note that rods/staffs/wands are then unstacked when they are used.
 *
 * If permitted, we allow weapons/armor to stack, if they both known.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests never stack (for various reasons).
 *
 * We do NOT allow activatable items (artifacts or dragon scale mail)
 * to stack, to keep the "activation" code clean.  Artifacts may stack,
 * but only with another identical artifact (which does not exist).
 *
 * Ego items may stack as long as they have the same ego-item type.
 * This is primarily to allow ego-missiles to stack.
 */
static bool borg_object_similar(borg_item *o_ptr, borg_item *j_ptr) {
	/* NOTE: This assumes the giving of one item at a time */
	int total = o_ptr->iqty + 1;

	/* Require identical object types */
	if (o_ptr->kind != j_ptr->kind)
		return (0);

	/* Analyze the items */
	switch (o_ptr->tval) {
	/* Chests */
	case TV_CHEST: {
		/* Never okay */
		return (0);
	}

	/* Food and Potions and Scrolls */
	case TV_FOOD:
	case TV_POTION:
	case TV_SCROLL: {
		/* Assume okay */
		break;
	}

	/* Staffs and Wands */
	case TV_STAFF: {
		/* Require identical charges */
		if (o_ptr->pval != j_ptr->pval)
			return (0);
	}
	case TV_WAND: {
		/* Require knowledge of charges */
		if ((!o_ptr->ident) || (!j_ptr->ident))
			return (0);
	}

	/* Staffs and Wands and Rods */
	case TV_ROD: {
		/* Require identical charges */
		if (o_ptr->pval != j_ptr->pval)
			return (0);

		/* Probably okay */
		break;
	}

	/* Weapons and Armor */
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
	case TV_DRAG_ARMOR: {
		return (0);

		/* Fall through */
	}

	/* Rings, Amulets, Lites */
	case TV_RING:
	case TV_AMULET:
	case TV_LITE: {
		/* Require full knowledge of both items */
		if ((!o_ptr->aware) || (!j_ptr->aware))
			return (0);

		/* Fall through */
	}

	/* Missiles */
	case TV_BOLT:
	case TV_ARROW:
	case TV_SHOT: {
		/* Require identical "bonuses" */
		if (o_ptr->to_h != j_ptr->to_h)
			return (FALSE);
		if (o_ptr->to_d != j_ptr->to_d)
			return (FALSE);
		if (o_ptr->to_a != j_ptr->to_a)
			return (FALSE);

		/* Require identical "pval" code */
		if (o_ptr->pval != j_ptr->pval)
			return (FALSE);

		/* Require identical "artifact" names */
		if (o_ptr->name1 != j_ptr->name1 || o_ptr->name1 == ART_RANDART)
			return (FALSE);

		/* Require identical "ego-item" names */
		if (o_ptr->name2 != j_ptr->name2)
			return (FALSE);

		/* Hack -- Never stack "powerful" items */
		if (o_ptr->flags1 || j_ptr->flags1)
			return (FALSE);
		if (o_ptr->flags2 || j_ptr->flags2)
			return (FALSE);
		if (o_ptr->flags3 || j_ptr->flags3)
			return (FALSE);

		/* Hack -- Never stack recharging items */
		if (o_ptr->timeout || j_ptr->timeout)
			return (FALSE);

		/* Require identical "values" */
		if (o_ptr->ac != j_ptr->ac)
			return (FALSE);
		if (o_ptr->dd != j_ptr->dd)
			return (FALSE);
		if (o_ptr->ds != j_ptr->ds)
			return (FALSE);

		/* Probably okay */
		break;
	}

	/* Various */
	default: {
		/* Require knowledge */
		if ((!o_ptr->aware) || (!j_ptr->aware))
			return (0);

		/* Probably okay */
		break;
	}
	}

	/* Hack -- Require identical "broken" status */
	if ((o_ptr->fully_identified) != (j_ptr->fully_identified))
		return (0);

	/* The stuff with 'note' is not right but it is close.  I think it */
	/* has him assuming that he can't stack sometimes when he can.  This */
	/* is alright, it just causes him to take a bit more time to do */
	/* some exchanges. */
	/* Hack -- require semi-matching "inscriptions" */
	if (((o_ptr->note != NULL) && (j_ptr->note != NULL)) &&
		 (o_ptr->note[0] && j_ptr->note[0]) && (!streq(o_ptr->note, j_ptr->note)))
		return (0);

	/* Hack -- normally require matching "inscriptions" */
	if (!stack_force_notes && (!streq(o_ptr->note, j_ptr->note)))
		return (0);

	/* Hack -- normally require matching "discounts" */
	if (!stack_force_costs && (o_ptr->discount != j_ptr->discount))
		return (0);

	/* Maximal "stacking" limit */
	if (total >= MAX_STACK_SIZE)
		return (0);

	/* They match, so they must be similar */
	return (TRUE);
}

/*
 * Find the mininum amount of some item to buy/sell. For most
 * items this is 1, but for certain items (such as ammunition)
 * it may be higher.  -- RML
 */
static int borg_min_item_quantity(borg_item *item) {
	/* Only trade in bunches if sufficient cash */
	if (borg_gold < 250)
		return (1);

	/* Don't trade expensive items in bunches */
	if (item->cost > 5)
		return (1);

	/* Don't trade non-known items in bunches */
	if (!item->aware)
		return (1);

	/* Only allow some types */
	switch (item->tval) {
	case TV_SPIKE:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		/* Maximum number of items */
		if (item->iqty < 5)
			return (item->iqty);
		return (5);

	case TV_FOOD:
		if (item->iqty < 3)
			return (item->iqty);
		return (3);
#if 0
    case TV_POTION:
    case TV_SCROLL:
        if (item->iqty < 2)
            return (item->iqty);
        return (2);
#endif

	default:
		return (1);
	}
}

/*
 * This file handles the highest level goals, and store interaction.
 *
 * Store interaction strategy
 *
 *   (1) Sell items to the home (for later use)
 ** optimize the stuff in the home... this involves buying and selling stuff
 ** not in the 'best' list.
 *       We sell anything we may need later (see step 4)
 *
 *   (2) Sell items to the shops (for money)
 *       We sell anything we do not actually need
 *
 *   (3) Buy items from the shops (for the player)
 *       We buy things that we actually need
 *
 *   (4) Buy items from the home (for the player)
 *       We buy things that we actually need (see step 1)
 *
 *   (5) Buy items from the shops (for the home)
 *       We buy things we may need later (see step 1)
 *
 *   (6) Buy items from the home (for the stores)
 *       We buy things we no longer need (see step 2)
 *
 *   The basic principle is that we should always act to improve our
 *   "status", and we should sometimes act to "maintain" our status,
 *   especially if there is a monetary reward.  But first we should
 *   attempt to use the home as a "stockpile", even though that is
 *   not worth any money, since it may save us money eventually.
 */

/* this optimized the home storage by trying every combination... it was too
 * slow.*/
/* put this code back when running this on a Cray. */
static void borg_think_home_sell_aux2_slow(int n, int start_i) {
	int i;

	/* All done */
	if (n == STORE_INVEN_MAX) {
		s32b home_power;

		/* Examine the home  */
		borg_notice_home(NULL, FALSE);

		/* Evaluate the home */
		home_power = borg_power_home();

		/* Track best */
		if (home_power > *b_home_power) {
			/* Save the results */
			for (i = 0; i < STORE_INVEN_MAX; i++)
				best[i] = test[i];

#if 0
            /* dump, for debugging */
            borg_note(format("Trying Combo (best home power %ld)",
                              *b_home_power));
            borg_note(format("             (test home power %ld)",home_power));
            for (i = 0; i < STORE_INVEN_MAX; i++)
            {
                if (borg_shops[BORG_HOME].ware[i].iqty)
                    borg_note(format("store %d %s (qty-%d).",  i,
                                       borg_shops[BORG_HOME].ware[i].desc,
                                       borg_shops[BORG_HOME].ware[i].iqty ));
                else
                    borg_note(format("store %d (empty).",  i));
            }
            borg_note(" "); /* add a blank line */
#endif

			/* Use it */
			*b_home_power = home_power;
		}

		/* Success */
		return;
	}

	/* Note the attempt */
	test[n] = n;

	/* Evaluate the default item */
	borg_think_home_sell_aux2_slow(n + 1, start_i);

	/* if this slot and the previous slot is empty, move on to previous slot*/
	/* this will prevent trying a thing in all the empty slots to see if */
	/* empty slot b is better than empty slot a.*/
	if ((n != 0) && !borg_shops[BORG_HOME].ware[n].iqty &&
		 !borg_shops[BORG_HOME].ware[n - 1].iqty)
		return;

	/* try other combinations */
	for (i = start_i; i < INVEN_PACK; i++) {
		borg_item *item;
		borg_item *item2;
		/*bool stacked = FALSE;*/

		item = &borg_items[i];
		item2 = &borg_shops[BORG_HOME].ware[n];

		/* Skip empty items */
		/* Require "aware" */
		/* Require "known" */
		if (!item->iqty || !item->kind || !item->aware)
			continue;

		/* Hack -- ignore "worthless" items */
		if (!item->value)
			continue;

		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;

		/* stacking? */
		if (borg_object_similar(item2, item)) {
			item2->iqty++;
			item->iqty--;
			/*stacked = TRUE;*/
		} else {
			int k;
			bool found_match = FALSE;

			/* eliminate items that would stack else where in the list. */
			for (k = 0; k < STORE_INVEN_MAX; k++) {
				if (borg_object_similar(&safe_home[k], item)) {
					found_match = TRUE;
					break;
				}
			}
			if (found_match)
				continue;

			/* replace current item with this item */
			COPY(item2, item, borg_item);

			/* only move one into a non-stack slot */
			item2->iqty = 1;

			/* remove item from pack */
			item->iqty--;
		}

		/* Note the attempt */
		test[n] = i + STORE_INVEN_MAX;

		/* Evaluate the possible item */
		borg_think_home_sell_aux2_slow(n + 1, i + 1);

		/* restore stuff */
		COPY(item2, &safe_home[n], borg_item);

		/* put item back into pack */
		item->iqty++;
	}
}
/* Interact with special shop */
bool borg_think_shop_temple(void) {

	/* I am here for some restoration */
	borg_keypress('r');

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	borg_respawning = 5;

	return (TRUE);
}
/* Interact with special shop */
bool borg_think_shop_thieves(void) {
	int count = 0;
	int i;

	/* thieves can get some free loot.  Cheat the availability.
	if (borg_class == CLASS_ROGUE && !p_ptr->rewards[BACT_GOLD]) {
		borg_keypress('s');
		borg_keypress(' ');
	}
	*/

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

	/* Attempt to ID my stuff */
	if (count >= 1) {
		/* I am here to ID my gear */
		borg_keypress('i');
	}

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	borg_respawning = 5;

	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_library(void) {
	int i;
	int b_i = -1;

	/* select Non-ID'd items. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Check the list of good things to *ID* */
		if (!borg_starid_item(item))
			continue;

		/* Keep it */
		b_i = i;
	}

	/* Nothing worth *ID* */
	if (b_i == -1) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* Reasearch this item */
	borg_note(format("# Researching item '%s'", borg_items[b_i].desc));
	borg_keypress('a');
	if (b_i >= 24) {
		/* Wielded Item */
		borg_keypress('/');
		borg_keypress(I2A(b_i - 24));
	} else {
		/* Inventory Item */
		borg_keypress(I2A(b_i));
	}

	borg_respawning = 5;

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_inn(void) {
	/*int i;*/
	/*int b_i = -1;*/
	int cost = 2;

	/* I dont eat food */
	if (borg_skill[BI_NOEAT] ||
		 /* Im poor */
		 borg_gold < cost ||
		 /* Im stuffed */
		 borg_skill[BI_ISGORGED]) {
		/* Exit the store */
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* Barkeep!  Another round! */
	borg_note("# Buying food and drink.");
	borg_keypress('f');

	borg_respawning = 5;

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_mutation(void) {
	/*int i;*/
	/*int count = 0;*/
	int cost = 5000;
	int mutations = 0;

	/* Discount */
	/* if (borg_race == RACE_BEASTMAN) cost = 1000; */

	/* Do a quick scan of my mutations score the negativity */
	mutations = borg_net_mutations();

	/* No need if I am no too bad off */
	if (mutations < 0) {
		/* I need enough money */
		if (borg_gold > cost) {
			/* Reasearch this item */
			borg_note(format("# Fixing Mutations '%d'", mutations));
			borg_keypress('m');
		}
	}

	borg_respawning = 5;

	/* Exit the store */
	if (mutations >= 0 || borg_gold < cost) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
	}

	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_sorcery(void) {
	int i;
	int b_i = -1;
	/*int charges = 0;*/
	bool id = FALSE;
	bool recharge = FALSE;
	int count = 0;

	/* Let us first test if we are here for ID'ing our stuff */
	if (borg_skill[BI_REALM1] == REALM_SORCERY ||
		 borg_skill[BI_REALM2] == REALM_SORCERY) {
		/* Count the number of Non-ID'd items. */
		for (i = 0; i < INVEN_TOTAL; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				break;

			/* Skip ID'd items */
			if (item->ident)
				continue;

			/* tally the non-id'd stuff */
			count++;
		}

		/* No need if very few need to be ID'd */
		if (count >= 1)
			id = TRUE;
	}

	/* Look at our items for things that need a recharge. */
	for (i = 0; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Only staff and wand */
		if (item->tval != TV_WAND && item->tval != TV_STAFF &&
			 item->tval != TV_ROD)
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
		if (item->tval == TV_STAFF && item->pval / item->iqty > 3)
			continue;
		if (item->tval == TV_WAND && item->pval / item->iqty > 5)
			continue;

		/* Keep it */
		b_i = i;
	}

	/* Nothing worth recharging */
	if (b_i != -1)
		recharge = TRUE;

	/* Nothing to do at all */
	if (id == FALSE && recharge == FALSE) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/** Action: ID **/
	if (id) {
		/* ID my stuff */
		borg_note("# Identifying items");
		borg_keypress('i');
	} else if (recharge) {
		/* How many charges will the stack hold? */
		/* TODO SHOULD this does not seem to work.. */
		/*charges = (k_info[borg_items[b_i].kind].pval * borg_items[b_i].iqty) -
					 borg_items[b_i].pval;*/

		/* Recharge this item */
		borg_note(format("# Recharging item '%s'", borg_items[b_i].desc));
		borg_keypress('r');
		borg_keypress(I2A(b_i));

		/* Rods need confirmation */
		if (borg_items[b_i].tval == TV_ROD) {
			borg_keypress('y');
		}
		/* Non-Rods continue */
		if (borg_items[b_i].tval != TV_ROD) {
			/* Does it need to be ID'd first */
			if (!borg_items[b_i].ident) {
				borg_keypress('y');
			}
			/* How many charges? prompt */
			borg_keypresses("99");
			borg_keypress('\r');
		}
	}

	borg_respawning = 5;

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	return (TRUE);
}
/* Interact with the special shop */
bool borg_think_shop_trump(void) {
	bool recall = FALSE;
	bool jump = FALSE;
	int jump_depth = 1;
	int i;
	int cost = 10000;
	char buf[3];

	/* Waiting for recall to engage */
	if (goal_recalling) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/*** Determine if the borg needs to Recall or Jump to dungeon depth ***/

	/* To what depth am I comfortable? */
	for (i = 1; i < 100; i++) {
		/* Am I prepared? */
		if ((cptr)NULL == borg_prepared[i]) {
			jump_depth = i;
			jump = TRUE;
			cost = 10000;
		}
	}

	/* Special case of the borg needing to stay on depth 98 */
	if (borg_skill[BI_MAXDEPTH] == 100 && borg_skill[BI_ATELEPORTLVL] == 0 &&
		 (cptr)NULL == borg_prepared[98]) {
		jump_depth = 98;
		jump = TRUE;
		cost = 10000;
	}

	/* Determine if I am recalling or jumping to a depth */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_MAXDEPTH] * 6 / 10]) {
		recall = TRUE;
		jump = FALSE;
		cost = 200;
	}

	/* if I am prepped for super deep but playing shallow, jump to the prepped
	 * depth */
	if (jump_depth > borg_skill[BI_MAXDEPTH] + 10) {
		jump = TRUE;
		recall = FALSE;
		cost = 10000;
	}

	/* Make sure I have enough money */
	if (cost > borg_gold) {
		borg_note("# Failed to use the Trump Tower when I wanted to.  Too poor.");
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* Activate */
	if (jump) {
		borg_note(format("# Using Trump Tower to jump to depth %d", jump_depth));
		borg_keypress('t');
		borg_itoa(jump_depth, buf /*, 10*/);
		borg_keypresses(buf);
		borg_keypress('\n');
		goal_recalling = TRUE;
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}
	if (recall) {
		borg_note("# Using Trump Tower to recall.");
		borg_keypress('r');
		borg_keypress('\n');
		goal_recalling = TRUE;
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* Unable */
	borg_note("# Failed to use the Trump Tower when I wanted to");

	/* Exit the shop */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_fighter(void) {
	int i;
	int b_i = -1;
	int b_pimp = 1;
	int max_pimp = borg_skill[BI_CLEVEL] / 5;
	int cost_weapon = 400;
	/*int cost_armor = 300;
	bool enchant_swap_armour = FALSE;
	bool enchant_swap_weapon = FALSE;
	*/

	/* Guild members get a discount
	if (borg_class == CLASS_WARRIOR) {
		cost_weapon = 200;
		cost_armor = 150;
	}
	*/

	/* Can I pimp my swap weapon */
	if (weapon_swap > 1 &&
		 (borg_items[weapon_swap].iqty && borg_items[weapon_swap].ident &&
		  (borg_items[weapon_swap].to_h < max_pimp ||
			borg_items[weapon_swap].to_d < max_pimp))) {
		b_i = weapon_swap;
		b_pimp = MAX(max_pimp - borg_items[weapon_swap].to_h,
						 max_pimp - borg_items[weapon_swap].to_d);
	}

	/* Can I pimp my swap armor */
	if (armour_swap > 1 &&
		 (borg_items[armour_swap].iqty && borg_items[armour_swap].ident &&
		  borg_items[armour_swap].to_a < max_pimp &&
		  borg_items[armour_swap].tval != TV_RING &&
		  borg_items[armour_swap].tval != TV_AMULET)) {
		b_i = armour_swap;
		b_pimp = max_pimp - borg_items[armour_swap].to_a;
	}

	/* Should I pimp my main melee weapon? */
	if (borg_items[INVEN_WIELD].iqty && borg_items[INVEN_WIELD].ident &&
		 (borg_items[INVEN_WIELD].to_h < max_pimp ||
		  borg_items[INVEN_WIELD].to_d < max_pimp)) {
		b_i = INVEN_WIELD;
		b_pimp = MAX(max_pimp - borg_items[INVEN_WIELD].to_h,
						 max_pimp - borg_items[INVEN_WIELD].to_d);
	}

	/* select items. */
	for (i = INVEN_BODY; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty slots */
		if (!item->iqty)
			continue;

		/* Skip non-id'd */
		if (!item->ident)
			continue;

		/* Skip those that can't be improved */
		if (item->tval == TV_RING || item->tval == TV_AMULET)
			continue;

		/* Skip those already pumped */
		if (item->to_a >= max_pimp)
			continue;

		/* Keep it */
		b_i = i;
		b_pimp = max_pimp - item->to_a;
	}

	/* I need enough money (assume the more expensive one) */
	if (borg_gold < cost_weapon * b_pimp) {
		int temp = b_pimp;

		/* If I can't afford the full pimp, how much can I afford */
		for (i = 1; i < temp; i++) {
			if (borg_gold >= cost_weapon * i) {
				b_pimp = i;
			}
		}
	}

	/* Nothing worth *ID* */
	if (b_i == -1) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* Boost this item */
	for (i = 1; i <= b_pimp; i++) {
		borg_note(format("# Enchanting item '%s'", borg_items[b_i].desc));
		if (b_i == INVEN_WIELD || b_i == weapon_swap)
			borg_keypress('w');
		else
			borg_keypress('a');
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');
			borg_keypress(I2A(b_i - 24));
		} else
			borg_keypress(I2A(b_i));
		borg_keypress(' ');
	}

	borg_respawning = 5;

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	return (TRUE);
}

/* Interact with special shop */
bool borg_think_shop_paladin(void) {
	int i;
	int b_i = -1;
	int b_pimp = 1;
	int max_pimp = borg_skill[BI_CLEVEL] / 5;
	int cost = 200;

	/* Can I pimp my swap armor */
	if (armour_swap > 1 &&
		 (borg_items[armour_swap].iqty && borg_items[armour_swap].ident &&
		  borg_items[armour_swap].to_a < max_pimp)) {
		b_i = armour_swap;
		b_pimp = max_pimp - borg_items[armour_swap].to_a;
	}

	/* select Non-ID'd items. */
	for (i = INVEN_BODY; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty slots */
		if (!item->iqty)
			continue;

		/* Skip non-id'd */
		if (!item->ident)
			continue;

		/* Skip those already pumped */
		if (item->to_a >= max_pimp)
			continue;

		/* Keep it */
		b_i = i;
		b_pimp = max_pimp - item->to_a;
	}

	/* Nothing worth *ID* */
	if (b_i == -1) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		return (TRUE);
	}

	/* I need enough money (assume the more expensive one) */
	if (borg_gold < cost * b_pimp) {
		int temp = b_pimp;

		/* If I can't afford the full pimp, how much can I afford */
		for (i = 1; i < temp; i++) {
			if (borg_gold >= cost * i) {
				b_pimp = i;
			}
		}
	}

	/* Boost this item */
	for (i = 1; i <= b_pimp; i++) {
		borg_note(format("# Enchanting armor item '%s'", borg_items[b_i].desc));
		borg_keypress('a');
		if (b_i >= INVEN_WIELD) {
			borg_keypress('/');
			borg_keypress(I2A(b_i - 24));
		} else
			borg_keypress(I2A(b_i));
		borg_keypress(' ');
	}

	borg_respawning = 5;

	/* Exit the store */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);

	return (TRUE);
}

/* Interact with special shop Ranger Guild */
bool borg_think_shop_bow(void) {
	int i;
	int b_i = -1;
	int cost = 20;
	int max_pimp = borg_skill[BI_CLEVEL] / 5;
	int b_pimp = 1;
	int bow_cost = 400;

	/* Check for a bow to enchant */
	borg_item *item = &borg_items[INVEN_BOW];

	/* Make sure I have an appropriate bow to enchant */
	if (item->iqty &&													  /* I got one */
		 item->ident &&												  /* It's ID'd */
		 (item->to_h < max_pimp || item->to_d < max_pimp) && /* it's not maxed */
		 borg_gold > bow_cost) {
		b_i = INVEN_BOW;
		b_pimp = MAX(max_pimp - item->to_h, max_pimp - item->to_d);
		cost = bow_cost;
	}

	/* scan the items we carry for a good missile to enchant */
	if (b_i == -1) {
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Skip certain inventory slots */
			if (!item->iqty)
				continue;
			if (!item->ident)
				continue;
			if (item->tval != my_ammo_tval)
				continue;
			/* Dont waste money on cursed ones */
			if (item->to_h < 0 || item->to_d < 0)
				continue;

			/* Can my missile be pimped by this shop owner? */
			if (item->to_h >= max_pimp && item->to_d >= max_pimp)
				continue;

			/* Can I afford it ? */
			if (item->iqty * cost > borg_gold)
				continue;

			/* I should have an item to enchant */
			b_i = i;
			b_pimp = MAX(max_pimp - item->to_h, max_pimp - item->to_d);
		}
	}

	/* We have nothing to enchant */
	if (b_i == -1) {
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);

		/* Need a chance to update the character inventory */
		borg_respawning = 5;

		return (TRUE);
	}

	/* I need enough money (assume the more expensive one) */
	if (borg_gold < cost * b_pimp) {
		int temp = b_pimp;

		/* If I can't afford the full pimp, how much can I afford */
		for (i = 1; i < temp; i++) {
			if (borg_gold >= cost * i) {
				b_pimp = i;
			}
		}
	}

	/* Boost this item */
	for (i = 1; i <= b_pimp; i++) {
		if (b_i == INVEN_BOW) {
			/* I am here to enchant... bow */
			borg_keypress('b');

			/* my bow. */
			borg_keypress('/');
			borg_keypress('b');
			borg_keypress(' ');
		} else {
			/* I am here to enchant... missiles */
			borg_keypress('a');

			/* missile. */
			borg_keypress(I2A(b_i));
			borg_keypress(' ');
		}
	}

	/* Need a chance to update the character inventory */
	borg_respawning = 5;

	return (TRUE);
}

/*
 * this will see what single addition/substitution is best for the home.
 * The formula is not as nice as the one above because it will
 * not check all possible combinations of items. but it is MUCH faster.
 */

static void borg_think_home_sell_aux2_fast(int n /*, int start_i*/) {
	borg_item *item;
	borg_item *item2;
	s32b home_power;
	int i, k;
	/*bool stacked = FALSE;*/

	/* get the starting best (current) */
	/* Examine the home  */
	borg_notice_home(NULL, FALSE);

	/* Evaluate the home  */
	*b_home_power = borg_power_home();

	/* try individual substitutions/additions.   */
	for (n = 0; n < STORE_INVEN_MAX; n++) {
		item2 = &borg_shops[BORG_HOME].ware[n];
		for (i = 0; i < INVEN_PACK; i++) {
			item = &borg_items[i];

			/* Skip empty items */
			/* Require "aware" */
			/* Require "known" */

			if (!item->iqty || !item->kind || !item->aware)
				continue;
			if (i == weapon_swap && weapon_swap != 0)
				continue;
			if (i == armour_swap && armour_swap != 0)
				continue;

			/* Do not dump stuff at home that is not fully id'd and should be  */
			/* this is good with random artifacts. */
			if (!item->fully_identified && item->name1)
				continue;

			/* skip stuff if we sold bought it */
			if (bought_item_tval == item->tval && bought_item_sval == item->sval &&
				 bought_item_pval == item->pval && bought_item_store == BORG_HOME)
				continue;

			/* Hack -- ignore "worthless" items */
			if (!item->value)
				continue;

			/* stacking? */
			if (borg_object_similar(item2, item)) {
				/* if this stacks with what was previously here */
				item2->iqty++;
				/*stacked = TRUE;*/
			} else {
				bool found_match = FALSE;

				/* eliminate items that would stack else where in the list. */
				for (k = 0; k < STORE_INVEN_MAX; k++) {
					if (borg_object_similar(&safe_home[k], item)) {
						found_match = TRUE;
						break;
					}
				}
				if (found_match)
					continue;

				/* replace current item with this item */
				COPY(item2, item, borg_item);

				/* only move one into a non-stack slot */
				item2->iqty = 1;
			}

			/* remove item from pack */
			item->iqty--;

			/* Note the attempt */
			test[n] = i + STORE_INVEN_MAX;

			/* Test to see if this is a good substitution. */
			/* Examine the home  */
			borg_notice_home(NULL, FALSE);

			/* Evaluate the home  */
			home_power = borg_power_home();

			/* Track best */
			if (home_power > *b_home_power) {
				/* Save the results */
				for (k = 0; k < STORE_INVEN_MAX; k++)
					best[k] = test[k];

#if 0
                /* dump, for debugging */
                borg_note(format("Trying Combo (best home power %ld)",
                                    *b_home_power));
                borg_note(format("             (test home power %ld)",
                                    home_power));
                for (i = 0; i < STORE_INVEN_MAX; i++)
                    if (borg_shops[BORG_HOME].ware[i].iqty)
                        borg_note(format("store %d %s (qty-%d).",  i,
                                           borg_shops[BORG_HOME].ware[i].desc,
                                           borg_shops[BORG_HOME].ware[i].iqty ));
                    else
                    borg_note(format("store %d (empty).",  i));

                borg_note(" "); /* add a blank line */
#endif

				/* Use it */
				*b_home_power = home_power;
			}

			/* restore stuff */
			COPY(item2, &safe_home[n], borg_item);

			/* put item back into pack */
			item->iqty++;

			/* put the item back in the test array */
			test[n] = n;
		}
	}
}

/* locate useless item */
static void borg_think_home_sell_aux3() {
	int i;
	s32b borg_empty_home_power;
	s32b power;

	/* get the starting power */
	borg_notice(TRUE);
	power = borg_power();

	/* get what an empty home would have for power */
	borg_notice_home(NULL, TRUE);
	borg_empty_home_power = borg_power_home();

	/* go through the inventory and eliminate items that either  */
	/* 1) will not increase the power of an empty house. */
	/* 2) will reduce borg_power if given to home */
	for (i = 0; i < INVEN_PACK; i++) {
		int num_items_given;
		num_items_given = 0;

		/* if there is no item here, go to next slot */
		if (!borg_items[i].iqty)
			continue;

		/* 1) eliminate garbage items (items that add nothing to an */
		/*     empty house) */
		borg_notice_home(&borg_items[i], FALSE);
		if (borg_power_home() <= borg_empty_home_power) {
			safe_items[i].iqty = 0;
			continue;
		}

		/* 2) will reduce borg_power if given to home */
		while (borg_items[i].iqty) {
			/* reduce inventory by this item */
			num_items_given++;
			borg_items[i].iqty--;

			/* Examine borg */
			borg_notice(FALSE);

			/* done if this reduces the borgs power */
			if (borg_power() < power) {
				/* we gave up one to many items */
				num_items_given--;
				break;
			}
		}

		/* restore the qty */
		borg_items[i].iqty = safe_items[i].iqty;

		/* Examine borg */
		borg_notice(FALSE);

		/* set the qty to number given without reducing borg power */
		safe_items[i].iqty = num_items_given;
	}
}

/*
 * Step 1 -- sell "useful" things to the home (for later)
 */
static bool borg_think_home_sell_aux(bool save_best) {
	int icky = STORE_INVEN_MAX - 1;

	s32b home_power = -1L;

	int i = -1;

	byte test_a[STORE_INVEN_MAX];
	byte best_a[STORE_INVEN_MAX];

	/* if the best is being saved (see borg_think_shop_grab_aux) */
	/* !FIX THIS NEEDS TO BE COMMENTED BETTER */
	if (!save_best)
		b_home_power = &home_power;
	test = test_a;
	best = best_a;

	/* if I have not been to home, do not try this yet. */
	if (!borg_shops[BORG_HOME].when)
		return FALSE;

	/* Dont sell stuff to our home in the Outpost, we won't stay in this town
	if (borg_skill[BI_TOWN_NUM] == 1 && !vanilla_town)
		return FALSE;
	*/

	/* Hack -- the home is full */
	/* and pack is full */
	if (borg_shops[BORG_HOME].ware[icky].iqty && borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* Copy all the store slots */
	for (i = 0; i < STORE_INVEN_MAX; i++) {
		/* Save the item */
		COPY(&safe_home[i], &borg_shops[BORG_HOME].ware[i], borg_item);

		/* clear test arrays (test[i] == i is no change) */
		best[i] = test[i] = i;
	}

	/* Hack -- Copy all the slots */
	for (i = 0; i < INVEN_PACK; i++) {
		/* Save the item */
		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;
		COPY(&safe_items[i], &borg_items[i], borg_item);
	}

	/* get rid of useless items */
	borg_think_home_sell_aux3();

	/* Examine the borg once more with full inventory then swap in the */
	/* safe_items for the home optimization */
	borg_notice(FALSE);

	/* swap quantities (this should be all that is different) */
	for (i = 0; i < INVEN_PACK; i++) {
		byte save_qty;
		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;

		save_qty = safe_items[i].iqty;
		safe_items[i].iqty = borg_items[i].iqty;
		borg_items[i].iqty = save_qty;
	}

	*b_home_power = -1;

	/* find best combo for home. */
	if (borg_slow_optimizehome) {
		borg_think_home_sell_aux2_slow(0, 0);
	} else {
		borg_think_home_sell_aux2_fast(0 /*, 0*/);
	}

	/* restore bonuses and such */
	for (i = 0; i < STORE_INVEN_MAX; i++) {
		COPY(&borg_shops[BORG_HOME].ware[i], &safe_home[i], borg_item);
	}

	/* Scan items in the inventory, and equipment.  */
	for (i = 0; i < INVEN_TOTAL; i++) {
		if (i == weapon_swap && weapon_swap != 0)
			continue;
		if (i == armour_swap && armour_swap != 0)
			continue;
		COPY(&borg_items[i], &safe_items[i], borg_item);
	}

	borg_notice(FALSE);
	borg_notice_home(NULL, FALSE);

	/* Drop stuff that will stack in the home */
	for (i = 0; i < STORE_INVEN_MAX; i++) {
		/* if this is not the item that was there, */
		/* drop off the item that replaces it. */
		if (best[i] != i && best[i] != 255) {
			borg_item *item = &borg_items[best[i] - STORE_INVEN_MAX];
			borg_item *item2 = &borg_shops[BORG_HOME].ware[i];

			/* if this item is not the same as what was */
			/* there before then take it. */
			if (!borg_object_similar(item2, item))
				continue;

			goal_shop = BORG_HOME;
			goal_item = best[i] - STORE_INVEN_MAX;

			return (TRUE);
		}
	}

	/* Get rid of stuff in house but not in 'best' house if  */
	/* pack is not full */
	if (!borg_items[INVEN_PACK - 1].iqty) {
		for (i = 0; i < STORE_INVEN_MAX; i++) {
			/* if this is not the item that was there, */
			/* get rid of the item that was there */
			if ((best[i] != i) && (borg_shops[BORG_HOME].ware[i].iqty)) {
				borg_item *item = &borg_items[best[i] - STORE_INVEN_MAX];
				borg_item *item2 = &borg_shops[BORG_HOME].ware[i];

				/* if this item is not the same as what was */
				/* there before take it. */
				if (borg_object_similar(item, item2))
					continue;

				/* skip stuff if we sold bought it */
				if ((item2->tval == TV_WAND || item2->tval == TV_STAFF) &&
					 (sold_item_tval == item2->tval &&
					  sold_item_sval == item2->sval && sold_item_store == BORG_HOME))
					return (FALSE);
				else if (sold_item_tval == item2->tval &&
							sold_item_sval == item2->sval &&
							sold_item_pval == item2->pval &&
							sold_item_store == BORG_HOME)
					return (FALSE);

				goal_shop = BORG_HOME;
				goal_ware = i;

				return TRUE;
			}
		}
	}

	/* Drop stuff that is in best house but currently in inventory */
	for (i = 0; i < STORE_INVEN_MAX; i++) {
		/* if this is not the item that was there,  */
		/* drop off the item that replaces it. */
		if (best[i] != i && best[i] != 255) {
			/* hack dont sell */
			if (!borg_items[best[i] - STORE_INVEN_MAX].iqty)
				return (FALSE);

			/* Full house */
			if (borg_shops[BORG_HOME].ware[icky].iqty)
				return (FALSE);

			goal_shop = BORG_HOME;
			goal_item = best[i] - STORE_INVEN_MAX;

			return (TRUE);
		}
	}

	/* Return our num_ counts to normal */
	borg_notice_home(NULL, FALSE);

	/* Assume not */
	return (FALSE);
}

/*
 * Step 2 -- sell "useless" items to a shop (for cash)
 */
static bool borg_think_shop_sell_aux(bool immediate) {
	int icky = STORE_INVEN_MAX - 1;

	int k, b_k = -1;
	int i, b_i = -1;
	int qty = 1;
	s32b p, b_p = 0L;
	/*s32b c = 0L;*/
	/*s32b b_c = 30001L;*/

	int q, q_max, b_q = 1;
	int full_shop;

	bool fix = FALSE;
	bool absorb = FALSE;

	/* Evaluate */
	b_p = borg_power();

	/* Check each shop */
	for (k = 0; k < (MAX_STORES); k++) {
		/* If immediate selling, skip the wrong stores */
		if (immediate && k != shop_num)
			continue;

		/* Quick cheat to verify store contents */
		/* borg_cheat_store(borg_skill[BI_TOWN_NUM], k); */

		full_shop = borg_shops[k].ware[icky].iqty;

		/* Save the store hole */
		COPY(&safe_shops[k].ware[icky], &borg_shops[k].ware[icky], borg_item);

		/* Sell stuff */
		for (i = 0; i < INVEN_PACK; i++) {
			borg_item *item = &borg_items[i];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			/* Skip some important type items */
			if ((item->tval == my_ammo_tval) && borg_skill[BI_AMISSILES] < 45)
				continue;
			if (item->tval == TV_ROD && item->sval == SV_ROD_HEALING &&
				 borg_has[ROD_HEAL] <= ROD_HEAL_GOAL)
				continue;

			/* ok to sell this if i am broke */
			if (borg_gold >= 2) {
				if ((item->tval == my_ammo_tval) && (borg_skill[BI_AMISSILES] < 45))
					continue;

				/* don't sell attack wands early on */
				if (item->tval == TV_WAND && item->sval == SV_WAND_MAGIC_MISSILE &&
					 item->iqty == 1 && borg_skill[BI_CLEVEL] <= 20)
					continue;
			}

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

			/* Not these wands / rods either */
			if (item->tval == TV_WAND && item->sval == SV_WAND_STONE_TO_MUD &&
				 item->iqty <= 4 && borg_skill[BI_ASTONE2MUD] < item->iqty * 5)
				continue;
			if (item->tval == TV_ROD && item->sval == SV_ROD_TELEPORT_AWAY &&
				 item->iqty <= 5)
				continue;

			/* We always run short on these */
			if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_TELEPORT)
				continue;
			if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_PHASE_DOOR)
				continue;

			/* Skip "bad" sales */
			if (!borg_good_sell(item, k))
				continue;

			/* Hack -- Skip "full" shops unless the shop can merge the item */
			if (full_shop >= 1) {
				/* compare my item to the shop's inventory to see if it blends.
				 * But for now, just merge books.
				 */
				absorb = FALSE;
				for (q = 0; q < icky; q++) {
					if (borg_shops[k].ware[q].tval == item->tval &&
						 borg_shops[k].ware[q].sval == item->sval &&
						 borg_shops[k].ware[q].iqty + item->iqty <= 99 &&
						 borg_shops[k].ware[q].to_a == item->to_a &&
						 borg_shops[k].ware[q].to_h == item->to_h &&
						 borg_shops[k].ware[q].to_d == item->to_d &&
						 borg_shops[k].ware[q].dd == item->dd &&
						 borg_shops[k].ware[q].ds == item->ds &&
						 borg_shops[k].ware[q].pval == item->pval &&
						 borg_shops[k].ware[q].discount == item->discount) {
						absorb = TRUE;
						break;
					}
				}

				/* Cant buy this, too full */
				if (!absorb)
					continue;
			}

			/* Save the item */
			COPY(&safe_items[i], &borg_items[i], borg_item);

			/* Give the item to the shop */
			COPY(&borg_shops[k].ware[icky], &safe_items[i], borg_item);

			/* get the quantity */
			q_max = item->iqty;

			/* Give a single item */
			borg_shops[k].ware[icky].iqty = qty;

			/* See how many I want to sell */
			for (q = 1; q <= q_max; q++) {
				/* Give a single item */
				borg_shops[k].ware[icky].iqty = q;

				/* Lose a single item per loop-pass */
				borg_items[i].iqty -= 1;

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the inventory */
				p = borg_power();

				/* Ignore "bad" sales */
				if (p < b_p)
					continue;

				/* Maintain the "best" */
				b_k = k;
				b_i = i;
				b_p = p;
				/*b_c = c;*/
				b_q = q;
			}

			/* Correct the quantity for certain items sold */
			if (borg_shops[k].ware[icky].tval == TV_WAND ||
				 borg_shops[k].ware[icky].tval == TV_STAFF ||
				 borg_shops[k].ware[icky].tval == TV_ROD ||
				 borg_shops[k].ware[icky].tval == TV_POTION ||
				 borg_shops[k].ware[icky].tval == TV_SCROLL)
				b_q = 1;

			/* Restore the item */
			COPY(&borg_items[i], &safe_items[i], borg_item);
		}

		/* Restore the store hole */
		COPY(&borg_shops[k].ware[icky], &safe_shops[k].ware[icky], borg_item);
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Sell something (if useless) */
	if ((b_k >= 0) && (b_i >= 0)) {
		/* Visit that shop */
		goal_shop = b_k;

		/* Sell that item */
		goal_item = b_i;
		goal_qty = b_q;

		/* Success */
		return (TRUE);
	}

	/* Assume not */
	return (FALSE);
}

/*
 * Help decide if an item should be bought from a real store
 *
 * We prevent the purchase of enchanted (or expensive) ammo,
 * so we do not spend all our money on temporary power.
 *
 * if level 35, who needs cash?  buy the expecive ammo!
 *
 * We prevent the purchase of low level discounted books,
 * so we will not waste slots on cheap books.
 *
 * We prevent the purchase of items from the black market
 * which are often available at normal stores, currently,
 * this includes low level books, and all wands and staffs.
 */
static bool borg_good_buy(borg_item *item, int who) {

	/* Check the object */
	switch (item->tval) {
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		if (borg_skill[BI_CLEVEL] < 35) {
			if (item->to_h)
				return (FALSE);
			if (item->to_d)
				return (FALSE);
		}
		break;

	case TV_BOW:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
		if (borg_skill[BI_NO_MELEE]) {
			if (item->to_h)
				return (FALSE);
			if (item->to_d)
				return (FALSE);
		}
		break;

	case TV_LIFE_BOOK:
	case TV_SORCERY_BOOK:
	case TV_NATURE_BOOK:
	case TV_CHAOS_BOOK:
	case TV_DEATH_BOOK:
	case TV_TRUMP_BOOK:
	case TV_ARCANE_BOOK:
		if (item->discount)
			return (TRUE);
		break;

	case TV_WAND:
	case TV_STAFF:
		break;
	}

	/* Don't buy from the BM until we are rich */
	if (who == 6) {
		/* buying Remove Curse scroll is acceptable */
		if (item->tval == TV_SCROLL && item->sval == SV_SCROLL_REMOVE_CURSE &&
			 borg_wearing_cursed)
			return (TRUE);

		/* Buying certain potions is acceptable */
		if ((item->tval == TV_POTION && ((item->sval == SV_POTION_STAR_HEALING) ||
													(item->sval == SV_POTION_LIFE) ||
													(item->sval == SV_POTION_HEALING) ||
													(item->sval == SV_POTION_INC_STR &&
													 my_stat_cur[A_STR] < (18 + 100)) ||
													(item->sval == SV_POTION_INC_INT &&
													 my_stat_cur[A_INT] < (18 + 100)) ||
													(item->sval == SV_POTION_INC_WIS &&
													 my_stat_cur[A_WIS] < (18 + 100)) ||
													(item->sval == SV_POTION_INC_DEX &&
													 my_stat_cur[A_DEX] < (18 + 100)) ||
													(item->sval == SV_POTION_INC_CON &&
													 my_stat_cur[A_CON] < (18 + 100)))) ||
			 (item->tval == TV_ROD &&
			  ((item->sval == SV_ROD_HEALING) ||
				(item->sval == SV_ROD_RECALL &&
				 (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
				(item->sval == SV_ROD_SPEED &&
				 (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
				(item->sval == SV_ROD_TELEPORT_AWAY &&
				 (borg_class == CLASS_WARRIOR || borg_class == CLASS_ROGUE)) ||
				(item->sval == SV_ROD_ILLUMINATION && (!borg_skill[BI_ALITE])))) ||
			 (item->tval == TV_SCROLL && (item->sval == SV_SCROLL_TELEPORT_LEVEL ||
													item->sval == SV_SCROLL_TELEPORT)))
			return (TRUE);

		if ((borg_skill[BI_CLEVEL] < 15) && (borg_gold < 20000))
			return (FALSE);
		if ((borg_skill[BI_CLEVEL] < 35) && (borg_gold < 15000))
			return (FALSE);
		if (borg_gold < 10000)
			return (FALSE);
	}

	/* do not buy the item if I just sold it. */
	if ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
		 (sold_item_tval == item->tval && sold_item_sval == item->sval &&
		  sold_item_store == who))
		return (FALSE);
	else if (sold_item_tval == item->tval && sold_item_sval == item->sval &&
				sold_item_pval == item->pval && sold_item_store == who) {
#if 0
              borg_note(format("# Choosing not to buy back %s",item->desc));
#endif
		return (FALSE);
	}

	/* Do not buy a second digger */
	if (item->tval == TV_DIGGING) {
		int ii;

		/* scan for an existing digger */
		for (ii = 0; ii < INVEN_PACK; ii++) {
			borg_item *item2 = &borg_items[ii];

			/* skip non diggers */
			if (item2->tval == TV_DIGGING)
				return (FALSE);
#if 0
            /* perhaps let him buy a digger with a better
             * pval than his current digger
             */
            {if (item->pval <= item2->pval) return (FALSE);}
#endif
		}
	}

	/* Low level borgs should not waste the money on certain things */
	if (borg_skill[BI_MAXCLEVEL] < 5) {
/* next book, cant read it */
#if 0
		if ((item->tval == TV_MAGIC_BOOK || item->tval == TV_PRAYER_BOOK) &&
            item->sval >=1) return (FALSE);
#endif
		/* Too much food is heavy and spendy */
		if (item->tval == TV_FOOD && borg_skill[BI_FOOD] >= 5 &&
			 borg_skill[BI_SDIG])
			return (FALSE);

		/* Too many torches are heavy and spendy */
		if (item->tval == TV_LITE && item->sval == SV_LITE_TORCH &&
			 borg_skill[BI_AFUEL] >= 3)
			return (FALSE);
	}

	/* Rangers and Paladins and the extra books */
	if ((borg_class == CLASS_PALADIN || borg_class == CLASS_RANGER) &&
		 borg_skill[BI_MAXCLEVEL] <= 8) {
#if 0
		if ((item->tval == TV_MAGIC_BOOK || item->tval == TV_PRAYER_BOOK) &&
            item->sval >=1) return (FALSE);
#endif
	}

	/* Okay */
	return (TRUE);
}

/*
 * Step 3 -- buy "useful" things from a shop (to be used)
 */
static bool borg_think_shop_buy_aux(void) {
	int hole = INVEN_PACK - 1;

	int slot;
	int qty = 1;

	int k, b_k = -1;
	int n, b_n = -1;
	s32b p, b_p = 0L;
	s32b c, b_c = 0L;
	int q, q_max, b_q = -1;

	bool fix = FALSE;

	/* Require one empty slot */
	if (borg_items[hole - 1].iqty)
		return (FALSE);

	/* Extract the "power" */
	b_p = borg_power() /* my_power */;

	/* Check the shops */
	for (k = 0; k < (MAX_STORES); k++) {
		/* Skip home */
		if (k == BORG_HOME)
			continue;

		/* Scan the wares */
		for (n = 0; n < STORE_INVEN_MAX; n++) {
			borg_item *item = &borg_shops[k].ware[n];

			/* Skip empty items and leave this store. */
			if (!item->iqty)
				break;

			/* second check on empty */
			if (!item->kind)
				continue;

			/* Skip "bad" buys */
			if (!borg_good_buy(item, k))
				continue;

			/* Hack -- Require "sufficient" cash unless it is a boosting item */
			if (borg_gold < item->cost)
				continue;

			/* Special check for 'immediate shopping' */
			if (borg_skill[BI_FOOD] == 0) {
				if (item->tval == TV_FOOD && borg_skill[BI_NOEAT])
					continue;
				if (item->tval != TV_FOOD &&
					 (item->tval != TV_SCROLL &&
					  item->sval != SV_SCROLL_SATISFY_HUNGER))
					continue;
			}
			if (borg_items[INVEN_LITE].pval == 0 &&
				 (item->tval != TV_LITE &&
				  (item->tval != TV_FLASK &&
					borg_items[INVEN_WIELD].sval == SV_LITE_LANTERN)))
				continue;

			/* Skip it if I just sold it */
			if ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
				 (sold_item_tval == item->tval && sold_item_sval == item->sval &&
				  sold_item_store == k))
				continue;
			else if (sold_item_tval == item->tval &&
						sold_item_sval == item->sval &&
						sold_item_pval == item->pval && sold_item_store == k) {
				if (borg_verbose)
					borg_note(format("# Choosing not to buy back '%s' from home.",
										  item->desc));
				continue;
			}

			/* Save shop item */
			COPY(&safe_shops[k].ware[n], &borg_shops[k].ware[n], borg_item);

			/* Save hole */
			COPY(&safe_items[hole], &borg_items[hole], borg_item);

			/* Save the number to trade */
			q_max = borg_shops[k].ware[n].iqty;

			/* Obtain "slot" */
			slot = borg_wield_slot(item);

			/* Hack, we keep diggers as a back-up, not to
			 * replace our current weapon
			 */
			if (item->tval == TV_DIGGING)
				slot = -1;

			/* if our current equip is cursed, then I can't
			 * buy a new replacement.
			 * XXX  Perhaps he should not buy anything but save
			 * money for the Remove Curse Scroll.
			 */
			if (slot >= INVEN_WIELD) {
				if (borg_items[slot].cursed)
					continue;
				if (borg_items[slot].flags3 & TR3_HEAVY_CURSE)
					continue;
				if (borg_items[slot].flags3 & TR3_PERMA_CURSE)
					continue;
			}

			/* Consider new equipment */
			if (slot >= 0) {
				/* Save old item */
				COPY(&safe_items[slot], &borg_items[slot], borg_item);

				/* Move equipment into inventory */
				COPY(&borg_items[hole], &safe_items[slot], borg_item);

				/* Move new item into equipment */
				COPY(&borg_items[slot], &safe_shops[k].ware[n], borg_item);

				/* Only a single item */
				borg_items[slot].iqty = qty;

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the inventory */
				p = borg_power();

				/* Restore old item */
				COPY(&borg_items[slot], &safe_items[slot], borg_item);
				/* Obtain the "cost" of the item */
				c = item->cost * qty;

				/* Ignore "bad" purchases */
				if (p <= b_p)
					continue;

				/* Ignore "expensive" purchases */
				if ((p == b_p) && (c >= b_c))
					continue;

				/* Save the item and cost */
				b_k = k;
				b_n = n;
				b_p = p;
				b_c = c;
				b_q = 1;
			}

			/* Consider new inventory */
			else {
				/* See how many items I want to buy */
				for (q = 1; q <= q_max; q++) {
					/* Obtain the "cost" of the item */
					c = item->cost * q;

					/* Too expensive to purchase this many */
					if (c > borg_gold)
						continue;

					/* Remove one item from shop for each loop-pass */
					borg_shops[k].ware[n].iqty -= 1;

					/* Move new item into inventory */
					COPY(&borg_items[hole], &safe_shops[k].ware[n], borg_item);

					/* multiple items, if available */
					borg_items[hole].iqty = q;

					/* Fix later */
					fix = TRUE;

					/* Examine the inventory */
					borg_notice(FALSE);

					/* Evaluate the equipment */
					p = borg_power();

					/* Ignore "bad" purchases */
					if (p <= b_p)
						continue;

					/* Ignore "expensive" purchases */
					if ((p == b_p) && (c >= b_c))
						continue;

					/* Save the item and cost */
					b_k = k;
					b_n = n;
					b_p = p;
					b_c = c;
					b_q = q;
				}
			}

			/* Restore hole */
			COPY(&borg_items[hole], &safe_items[hole], borg_item);

			/* Restore shop item */
			COPY(&borg_shops[k].ware[n], &safe_shops[k].ware[n], borg_item);
		}
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Buy something */
	if ((b_k >= 0) && (b_n >= 0)) {
		/* Visit that shop */
		goal_shop = b_k;

		/* Buy that item */
		goal_ware = b_n;
		goal_qty = b_q;

		/* Success */
		return (TRUE);
	}

	/* Nope */
	goal_shop = -1;
	goal_ware = -1;
	return (FALSE);
}

/*
 * Step 4 -- buy "useful" things from the home (to be used)
 */
static bool borg_think_home_buy_aux(void) {
	int hole = INVEN_PACK - 1;

	int slot /*, i*/;
	int stack;
	/*int qty = 1;*/
	int n, b_n = -1;
	s32b p, b_p = 0L;
	s32b p_left = 0;
	s32b p_right = 0;
	int q, q_max, b_q = 1;

	bool fix = FALSE;

	/* Extract the "power" */
	b_p = borg_power();

	/* Scan the home */
	for (n = 0; n < STORE_INVEN_MAX; n++) {
		borg_item *item = &borg_shops[BORG_HOME].ware[n];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* second check on empty */
		if (!item->kind)
			continue;

		/* Skip it if I'm starving and it is not a food item */
		if (borg_skill[BI_ISWEAK] && borg_skill[BI_FOOD] == 0) {
			if ((item->tval == TV_FOOD && !borg_skill[BI_NOEAT]) ||
				 (item->tval != TV_FOOD &&
				  (item->tval != TV_SCROLL &&
					item->sval != SV_SCROLL_SATISFY_HUNGER))) {
				goal_shop = -1;
				goal_ware = -1;
				goal_qty = -1;
				continue;
			}
		}

		/* Skip it if I just sold it */
		if ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
			 (sold_item_tval == item->tval && sold_item_sval == item->sval &&
			  sold_item_store == BORG_HOME))
			continue;
		else if (sold_item_tval == item->tval && sold_item_sval == item->sval &&
					sold_item_pval == item->pval && sold_item_store == BORG_HOME) {
#if 0
            borg_note(format("# Choosing not to buy back '%s' from home.", item->desc));
#endif
			continue;
		}

		/* Reset the 'hole' in case it was changed by the last stacked item.*/
		hole = INVEN_PACK - 1;

		/* Save shop item */
		COPY(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Save hole */
		COPY(&safe_items[hole], &borg_items[hole], borg_item);

		/* Save the number to trade */
		q_max = borg_shops[7].ware[n].iqty;

		/* Obtain "slot" */
		slot = borg_wield_slot(item);
		stack = borg_slot(item->tval, item->sval);

		/* Consider new equipment-- Must check both ring slots */
		if (slot >= 0) {
			/* Require one empty slot */
			if (borg_items[INVEN_PACK - 1].iqty)
				continue;
			if (borg_items[INVEN_PACK - 2].iqty)
				continue;

			/* Check Rings */
			if (slot == INVEN_LEFT) {
				/** First Check Left Hand **/

				/* special curse check for left ring */
				if (!borg_items[INVEN_LEFT].cursed) {
					/* Save old item */
					COPY(&safe_items[slot], &borg_items[slot], borg_item);

					/* Move equipment into inventory */
					COPY(&borg_items[hole], &safe_items[slot], borg_item);

					/* Move new item into equipment */
					COPY(&borg_items[slot], &safe_shops[BORG_HOME].ware[n],
						  borg_item);

					/* Only a single item */
					borg_items[slot].iqty = b_q;

					/* Fix later */
					fix = TRUE;

					/* Examine the inventory */
					borg_notice(FALSE);

					/* Evaluate the inventory */
					p_left = borg_power();
#if 0
			/* dump list and power...  for debugging */
            borg_note(format("Trying Item %s (best power %ld)",borg_items[slot].desc, p_left));
            borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[slot].desc, my_power));
#endif
					/* Restore old item */
					COPY(&borg_items[slot], &safe_items[slot], borg_item);
				}

				/** Second Check Right Hand **/
				/* special curse check for right ring */
				if (!borg_items[INVEN_RIGHT].cursed) {
					/* Save old item */
					COPY(&safe_items[INVEN_RIGHT], &borg_items[INVEN_RIGHT],
						  borg_item);

					/* Move equipment into inventory */
					COPY(&borg_items[hole], &safe_items[INVEN_RIGHT], borg_item);

					/* Move new item into equipment */
					COPY(&borg_items[INVEN_RIGHT], &safe_shops[BORG_HOME].ware[n],
						  borg_item);

					/* Only a single item */
					borg_items[INVEN_RIGHT].iqty = b_q;

					/* Fix later */
					fix = TRUE;

					/* Examine the inventory */
					borg_notice(FALSE);

					/* Evaluate the inventory */
					p_right = borg_power();

#if 0
					/* dump list and power...  for debugging */
                    borg_note(format("Trying Item %s (best power %ld)",borg_items[INVEN_RIGHT].desc, p_right));
                    borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[INVEN_RIGHT].desc, my_power));
#endif
					/* Restore old item */
					COPY(&borg_items[INVEN_RIGHT], &safe_items[INVEN_RIGHT],
						  borg_item);
				}

				/* Is this ring better than one of mine? */
				p = MAX(p_right, p_left);

			}

			else /* non rings */
			{

				/* do not consider if my current item is cursed */
				if (slot != -1 && borg_items[slot].cursed)
					continue;

				/* Save old item */
				COPY(&safe_items[slot], &borg_items[slot], borg_item);

				/* Move equipment into inventory */
				COPY(&borg_items[hole], &safe_items[slot], borg_item);

				/* Move new item into equipment */
				COPY(&borg_items[slot], &safe_shops[BORG_HOME].ware[n], borg_item);

				/* Only a single item */
				borg_items[slot].iqty = b_q;

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the inventory */
				p = borg_power();
#if 0
                /* dump list and power...  for debugging */
                borg_note(format("Trying Item %s (best power %ld)",borg_items[slot].desc, p));
                borg_note(format("   Against Item %s   (borg_power %ld)",safe_items[slot].desc, my_power));
#endif
				/* Restore old item */
				COPY(&borg_items[slot], &safe_items[slot], borg_item);
			} /* non rings */
		}	 /* equip */

		/* Consider new inventory */
		else {
			for (q = 1; q <= q_max; q++) {
				if (stack != -1)
					hole = stack;

				/* Require one empty slot */
				if (stack == -1 && borg_items[INVEN_PACK - 1].iqty)
					continue;
				if (stack == -1 && borg_items[INVEN_PACK - 2].iqty)
					continue;

				/* Save hole (could be either empty slot or stack */
				COPY(&safe_items[hole], &borg_items[hole], borg_item);

				/* Move new item into inventory */
				COPY(&borg_items[hole], &safe_shops[7].ware[n], borg_item);

				/* Is this new item merging into an exisiting stack? */
				if (stack != -1) {
					/* Add a quantity to the stack */
					borg_items[hole].iqty = safe_items[hole].iqty + q;
				} else {
					/* Adding a qty to my inventory */
					borg_items[hole].iqty = q;
				}

				/* Fix later */
				fix = TRUE;

				/* Examine the inventory */
				borg_notice(FALSE);

				/* Evaluate the equipment */
				p = borg_power();

				/* Ignore "silly" purchases */
				if (p <= b_p)
					continue;

				/* Save the item and cost */
				b_n = n;
				b_p = p;
				b_q = q;

			} /* Qty loop */
		}

		/* Restore hole */
		COPY(&borg_items[hole], &safe_items[hole], borg_item);

		/* Restore shop item */
		COPY(&borg_shops[7].ware[n], &safe_shops[7].ware[n], borg_item);
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Buy something */
	if ((b_n >= 0) && (b_p > my_power)) {
		/* Go to the home */
		goal_shop = BORG_HOME;

		/* Buy that item */
		goal_ware = b_n;
		goal_qty = b_q;

		/* Success */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Step 5 -- buy "interesting" things from a shop (to be used later)
 */
static bool borg_think_shop_grab_aux(void) {

	int k, b_k = -1;
	int n, b_n = -1;
	int qty = 1;

	s32b s, b_s = 0L;
	s32b c, b_c = 0L;
	s32b borg_empty_home_power;

	/* Dont do this if Lilith is dead */
	if (borg_race_death[RACE_LILITH] != 0)
		return (FALSE);

	/* Wait until I can afford it */
	if (borg_skill[BI_CLEVEL] < 20)
		return (FALSE);

	/* Dont do this in the Outpost, we will move to a new town
	if (borg_skill[BI_TOWN_NUM] == 1 && !vanilla_town)
		return (FALSE);
	*/

	/* get what an empty home would have for power */
	borg_notice_home(NULL, TRUE);
	borg_empty_home_power = borg_power_home();

	b_home_power = &s;

	/* Require two empty slots */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);
	if (borg_items[INVEN_PACK - 2].iqty)
		return (FALSE);

	/* Examine the home */
	borg_notice_home(NULL, FALSE);

	/* Evaluate the home */
	b_s = borg_power_home();

	/* Check the shops */
	for (k = 0; k < (MAX_STORES); k++) {
		/* Scan the wares */
		for (n = 0; n < STORE_INVEN_MAX; n++) {
			borg_item *item = &borg_shops[k].ware[n];

			/* Skip empty items */
			if (!item->iqty)
				continue;

			/* skip home */
			if (k == BORG_HOME)
				continue;

			/* Skip "bad" buys */
			if (!borg_good_buy(item, k))
				continue;

			/* Skip mundane items */
			if (borg_wield_slot(item) >= INVEN_WIELD) {
				if (!item->to_h && !item->to_d && !item->to_a)
					continue;
			}

			/* Dont buy easy spell books for the house */
			if (item->tval >= TV_LIFE_BOOK && item->tval <= TV_ARCANE_BOOK)
				continue;

			/* A borg won't need to by weapons like this */
			if (item->tval >= TV_BOW && item->tval <= TV_SWORD)
				continue;

			/* Most purchases not required after a certain level */
			if (borg_skill[BI_MAXCLEVEL] >= 50 && item->tval != TV_POTION &&
				 item->tval != TV_SCROLL)
				continue;

			/* Dont bother with scrolls.  They are easy to come by */
			if (item->tval == TV_SCROLL || item->tval == TV_WAND ||
				 item->tval == TV_STAFF)
				continue;

			/* Hack -- Require some "extra" cash */
			if (borg_gold < 1000L + item->cost * 5)
				continue;

			/* make this the next to last item that the player has */
			/* (can't make it the last or it thinks that both player and */
			/*  home are full) */
			COPY(&borg_items[INVEN_PACK - 2], &borg_shops[k].ware[n], borg_item);

			/* Save the number */
			qty = borg_min_item_quantity(item);

			/* Give a single item */
			borg_items[INVEN_PACK - 2].iqty = qty;

			/* make sure this item would help an empty home */
			borg_notice_home(&borg_shops[k].ware[n], FALSE);
			if (borg_empty_home_power >= borg_power_home())
				continue;

			/* optimize the home inventory */
			if (!borg_think_home_sell_aux(TRUE))
				continue;

			/* Obtain the "cost" of the item */
			c = item->cost * qty;

			/* Penalize expensive items */
			if (c > borg_gold / 10)
				s -= c;

			/* Ignore "bad" sales */
			if (s < b_s)
				continue;

			/* Ignore "expensive" purchases */
			if ((s == b_s) && (c >= b_c))
				continue;

			/* Save the item and cost */
			b_k = k;
			b_n = n;
			b_s = s;
			b_c = c;
		}
	}

	/* restore inventory hole (just make sure the last slot goes back to */
	/* empty) */
	borg_items[INVEN_PACK - 2].iqty = 0;

	/* Examine the real home */
	borg_notice_home(NULL, FALSE);

	/* Evaluate the home */
	s = borg_power_home();

	/* remove the target that optimizing the home gave */
	goal_shop = goal_ware = goal_item = -1;

	/* Buy something */
	if ((b_k >= 0) && (b_n >= 0)) {
		/* Visit that shop */
		goal_shop = b_k;

		/* Buy that item */
		goal_ware = b_n;

		/* Success */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Step 6 -- take "useless" things from the home (to be sold)
 */
static bool borg_think_home_grab_aux(void) {
	int n, b_n = -1;
	s32b s, b_s = 0L;
	int qty = 1;

	/* Require two empty slots */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);
	if (borg_items[INVEN_PACK - 2].iqty)
		return (FALSE);

	/* Examine the home */
	borg_notice_home(NULL, FALSE);

	/* Evaluate the home */
	b_s = borg_power_home();

	/* Scan the home */
	for (n = 0; n < STORE_INVEN_MAX; n++) {
		borg_item *item = &borg_shops[BORG_HOME].ware[n];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* skip stuff if we sold bought it */
		if ((item->tval == TV_WAND || item->tval == TV_STAFF) &&
			 (sold_item_tval == item->tval && sold_item_sval == item->sval &&
			  sold_item_store == BORG_HOME))
			continue;
		else if (sold_item_tval == item->tval && sold_item_sval == item->sval &&
					sold_item_pval == item->pval && sold_item_store == BORG_HOME)
			continue;

		/* Save shop item */
		COPY(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Save the number */
		qty = borg_min_item_quantity(item);

		/* Remove one item from shop */
		borg_shops[BORG_HOME].ware[n].iqty -= qty;

		/* Examine the home */
		borg_notice_home(NULL, FALSE);

		/* Evaluate the home */
		s = borg_power_home();

		/* Restore shop item */
		COPY(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Ignore "bad" sales */
		if (s < b_s)
			continue;

		/* Maintain the "best" */
		b_n = n;
		b_s = s;
	}

	/* Examine the home */
	borg_notice_home(NULL, FALSE);

	/* Evaluate the home */
	s = borg_power_home();

	/* Stockpile */
	if (b_n >= 0) {
		/* Visit the home */
		goal_shop = BORG_HOME;

		/* Grab that item */
		goal_ware = b_n;

		/* Success */
		return (TRUE);
	}

	/* Assume not */
	return (FALSE);
}

/*
 * Step 7A -- buy "useful" weapons from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_weapon(void) {
	int hole;

	int slot;
	int old_weapon_swap;
	s32b old_weapon_swap_value;
	int old_armour_swap;
	s32b old_armour_swap_value;
	int n, b_n = -1;
	s32b p, b_p = 0L;

	bool fix = FALSE;

	/* save the current values */
	old_weapon_swap = weapon_swap;
	old_weapon_swap_value = weapon_swap_value;
	old_armour_swap = armour_swap;
	old_armour_swap_value = armour_swap_value;

	if (weapon_swap <= 0 || weapon_swap_value <= 0) {
		hole = INVEN_PACK - 1;
		weapon_swap_value = -1L;
	} else {
		hole = weapon_swap;
	}

	/* Extract the "power" */
	b_p = weapon_swap_value;

	/* Scan the home */
	for (n = 0; n < STORE_INVEN_MAX; n++) {
		borg_item *item = &borg_shops[BORG_HOME].ware[n];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Obtain "slot" make sure its a weapon */
		slot = borg_wield_slot(item);
		if (slot != INVEN_WIELD)
			continue;

		/* Save shop item */
		COPY(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Save hole */
		COPY(&safe_items[hole], &borg_items[hole], borg_item);

		/* Remove one item from shop */
		borg_shops[BORG_HOME].ware[n].iqty--;

		/* Consider new equipment */
		if (slot == INVEN_WIELD) {
			/* Move new item into inventory */
			COPY(&borg_items[hole], &safe_shops[BORG_HOME].ware[n], borg_item);

			/* Only a single item */
			borg_items[hole].iqty = 1;

			/* Fix later */
			fix = TRUE;

			/* Examine the iventory and swap value*/
			borg_notice(TRUE);

			/* Evaluate the new equipment */
			p = weapon_swap_value;
		}

		/* Restore hole */
		COPY(&borg_items[hole], &safe_items[hole], borg_item);

		/* Restore shop item */
		COPY(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Ignore "silly" purchases */
		if (p <= b_p)
			continue;

		/* Save the item and value */
		b_n = n;
		b_p = p;
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Buy something */
	if ((b_n >= 0) && (b_p > weapon_swap_value)) {
		/* Go to the home */
		goal_shop = BORG_HOME;

		/* Buy that item */
		goal_ware = b_n;

		/* Restore the values */
		weapon_swap = old_weapon_swap;
		weapon_swap_value = old_weapon_swap_value;
		armour_swap = old_armour_swap;
		armour_swap_value = old_armour_swap_value;

		/* Success */
		return (TRUE);
	}

	/* Restore the values */
	weapon_swap = old_weapon_swap;
	weapon_swap_value = old_weapon_swap_value;
	armour_swap = old_armour_swap;
	armour_swap_value = old_armour_swap_value;

	/* Nope */
	return (FALSE);
}
/*
 * Step 7B -- buy "useful" armour from the home (to be used as a swap)
 */
static bool borg_think_home_buy_swap_armour(void) {
	int hole;

	/*int slot;*/

	int n, b_n = -1;
	s32b p, b_p = 0L;
	bool fix = FALSE;
	int old_weapon_swap;
	s32b old_weapon_swap_value;
	int old_armour_swap;
	s32b old_armour_swap_value;

	/* save the current values */
	old_weapon_swap = weapon_swap;
	old_weapon_swap_value = weapon_swap_value;
	old_armour_swap = armour_swap;
	old_armour_swap_value = armour_swap_value;

	if (armour_swap <= 1 || armour_swap_value <= 0) {
		hole = INVEN_PACK - 1;
		armour_swap_value = -1L;
	} else {
		hole = armour_swap;
	}

	/* Extract the "power" */
	b_p = armour_swap_value;

	/* Scan the home */
	for (n = 0; n < STORE_INVEN_MAX; n++) {
		borg_item *item = &borg_shops[BORG_HOME].ware[n];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Obtain "slot".  Elimination of non armours in borg4.c*/
		/*slot = borg_wield_slot(item);*/

		/* Save shop item */
		COPY(&safe_shops[BORG_HOME].ware[n], &borg_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Save hole */
		COPY(&safe_items[hole], &borg_items[hole], borg_item);

		/* Remove one item from shop */
		borg_shops[BORG_HOME].ware[n].iqty--;

		/* Move new item into inventory */
		COPY(&borg_items[hole], &safe_shops[BORG_HOME].ware[n], borg_item);

		/* Only a single item */
		borg_items[hole].iqty = 1;

		/* Fix later */
		fix = TRUE;

		/* Examine the inventory (false)*/
		borg_notice(TRUE);

		/* Evaluate the new equipment */
		p = armour_swap_value;

		/* Restore hole */
		COPY(&borg_items[hole], &safe_items[hole], borg_item);

		/* Restore shop item */
		COPY(&borg_shops[BORG_HOME].ware[n], &safe_shops[BORG_HOME].ware[n],
			  borg_item);

		/* Ignore "silly" purchases */
		if (p <= b_p)
			continue;

		/* Save the item and value */
		b_n = n;
		b_p = p;
	}

	/* Examine the inventory */
	if (fix)
		borg_notice(TRUE);

	/* Buy something */
	if ((b_n >= 0) && (b_p > armour_swap_value)) {
		/* Go to the home */
		goal_shop = BORG_HOME;

		/* Buy that item */
		goal_ware = b_n;

		/* Restore the values */
		weapon_swap = old_weapon_swap;
		weapon_swap_value = old_weapon_swap_value;
		armour_swap = old_armour_swap;
		armour_swap_value = old_armour_swap_value;

		/* Success */
		return (TRUE);
	}
	/* Restore the values */
	weapon_swap = old_weapon_swap;
	weapon_swap_value = old_weapon_swap_value;
	armour_swap = old_armour_swap;
	armour_swap_value = old_armour_swap_value;

	/* Nope */
	return (FALSE);
}

/*
 * Choose a shop to visit (see above)
 */
static bool borg_choose_shop(void) {
	int i;

	/* Must be in town */
	if (borg_skill[BI_CDEPTH])
		return (FALSE);

	/* Forbid if been sitting on level forever */
	/*    Just come back and work through the loop later */
	if (borg_t - borg_began > 2000)
		return (FALSE);
	if (time_this_panel > 850)
		return (FALSE);

	/* If poisoned or bleeding -- flow to temple */
	if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && borg_gold >= 50)
		goal_shop = 3;

	/* If Starving or Out of Light -- flow to food */
	if (borg_skill[BI_ISWEAK] ||
		 (borg_skill[BI_CUR_LITE] == 0 && borg_skill[BI_CLEVEL] >= 2 &&
		  borg_gold >= 5)) {
		/* Hungry and on a special gruten-free diet?  Try the scroll shop */
		if (borg_skill[BI_NOEAT] && borg_skill[BI_ISWEAK]) {
			/* Alchemist for Sat Hung scrolls */
			goal_shop = BORG_ALCHEMIST;
		}
		/* Either not on a special diet or not hungry.  GStore has what you need
			*/
		else
			goal_shop = BORG_GSTORE;
	}

	/* If poisoned or bleeding or hungry-- Buy items from temple */
	if ((borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISWEAK] ||
		  borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) &&
		 borg_gold >= 5) {
		if (borg_think_shop_buy_aux()) {
			/* Message */
			borg_note(format("# Buying '%s' at '%s' (for player immediately)",
								  borg_shops[goal_shop].ware[goal_ware].desc,
								  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name)));

			/* Success */
			return (TRUE);
		}

		/* if temple is out of healing stuff, try the house */
		if (borg_think_home_buy_aux()) {
			/* Message */
			borg_note(format("# Buying '%s' from the home, immediately.",
								  borg_shops[goal_shop].ware[goal_ware].desc));

			/* Success */
			return (TRUE);
		}

		/* If no help in these shops, don't bother selling stuff, consider
		 * returning to dungeon */
		return (FALSE);
	}

	/* Sell immediately if in munchkin mode */
	if (borg_munchkin_start &&
		 (borg_skill[BI_MAXCLEVEL] <= borg_munchkin_level)) {
		/* Sell items to the shops */
		if (borg_think_shop_sell_aux(TRUE)) {
			/* Message */
			borg_note(format("# Immediate selling '%s' at '%s'",
								  borg_items[goal_item].desc,
								  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name)));

			/* Success */
			return (TRUE);
		}
	}

	/* Must have visited all shops first---complete information */
	for (i = 0; i < (MAX_STORES); i++) {
		borg_shop *shop = &borg_shops[i];

		/* Skip "visited" shops */
		if (!shop->when)
			return (FALSE);
	}

	/* if we are already flowing toward a shop do not check again... */
	if (goal_shop != -1)
		return TRUE;

	/* Assume no important shop */
	goal_shop = goal_ware = goal_item = -1;

	/* if the borg is scumming for cash, we dont want him messing with the
		home inventory */
	if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
		 !borg_skill[BI_CDEPTH] && borg_skill[BI_LITE]) {
		/* Step 0 -- Buy items from the shops (for the player while scumming) */
		if (borg_think_shop_buy_aux()) {
			/* Message */
			borg_note(format("# Buying '%s' at '%s' (money scumming)",
								  borg_shops[goal_shop].ware[goal_ware].desc,
								  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name)));

			/* Success */
			return (TRUE);
		} else
			return (FALSE);
	}

	/* Step 1 -- Sell items to the home */
	if (borg_think_home_sell_aux(FALSE)) {
		/* Message */
		if (goal_item != -1)
			borg_note(
				 format("# Selling '%s' to the home", borg_items[goal_item].desc));
		else
			borg_note(format("# Buying '%s' from the home (step 1)",
								  borg_shops[goal_shop].ware[goal_ware].desc));

		/* Success */
		return (TRUE);
	}

	/* Step 2 -- Sell items to the shops */
	if (borg_think_shop_sell_aux(FALSE)) {
		/* Message */
		borg_note(format("# Selling '%s' at '%s'", borg_items[goal_item].desc,
							  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name)));

		/* Success */
		return (TRUE);
	}

	/* Step 3 -- Buy items from the shops (for the player) */
	if (borg_think_shop_buy_aux()) {
		/* Message */
		borg_note(format("# Buying '%s' at '%s' (%d for player)",
							  borg_shops[goal_shop].ware[goal_ware].desc,
							  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name),
							  goal_qty));

		/* Success */
		return (TRUE);
	}

	/* Step 4 -- Buy items from the home (for the player) */
	if (borg_think_home_buy_aux()) {
		/* Message */
		borg_note(format("# Buying '%s' from the home (step 4)",
							  borg_shops[goal_shop].ware[goal_ware].desc));

		/* Success */
		return (TRUE);
	}

	/* get rid of junk from home first.  That way the home is 'uncluttered' */
	/* before you buy stuff for it.  This will prevent the problem where an */
	/* item has become a negative value and swapping in a '0' gain item */
	/* (like pottery) is better. */

	/* Step 5 -- Grab items from the home (for the shops) */
	if (borg_think_home_grab_aux()) {
		/* Message */
		borg_note(format("# Grabbing (to sell) '%s' from the home",
							  borg_shops[goal_shop].ware[goal_ware].desc));

		/* Success */
		return (TRUE);
	}

	/* Step 6 -- Buy items from the shops (for the home) */
	if (borg_think_shop_grab_aux()) {
		/* Message */
		borg_note(format("# Grabbing (for home) '%s' at '%s'",
							  borg_shops[goal_shop].ware[goal_ware].desc,
							  (f_name + f_info[FEAT_SHOP_HEAD + goal_shop].name)));

		/* Success */
		return (TRUE);
	}
	/* Step 7A -- Buy weapons from the home (as a backup item) */
	if (borg_uses_swaps && borg_think_home_buy_swap_weapon()) {
		/* Message */
		borg_note(format("# Buying '%s' from the home as a backup",
							  borg_shops[goal_shop].ware[goal_ware].desc));

		/* Success */
		return (TRUE);
	}
	/* Step 7B -- Buy armour from the home (as a backup item) */
	if (borg_uses_swaps && borg_think_home_buy_swap_armour()) {
		/* Message */
		borg_note(format("# Buying '%s' from the home as a backup",
							  borg_shops[goal_shop].ware[goal_ware].desc));

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
}

/*
 * Sell items to the current shop, if desired
 */
static bool borg_think_shop_sell(void) {
	/*int qty = 1;*/
	char buf[3];

	/* Sell something if requested */
	if ((goal_shop == shop_num) && (goal_item >= 0)) {
		borg_item *item = &borg_items[goal_item];

		/* Remove the inscription */
		if (goal_shop != BORG_HOME)
			borg_send_deinscribe(goal_item);

		/* Log */
		borg_note(format("# Selling %s", item->desc));

		/* Buy an item */
		borg_keypress('s');

		/* Buy the desired item */
		borg_keypress(I2A(goal_item));

		/* Hack -- Sell a qty of the item */
		borg_itoa(goal_qty, buf /*, 10*/);
		borg_keypresses(buf);
		borg_keypress('\n');

		/* Mega-Hack -- Accept the price */
		borg_keypress('\n');
		borg_keypress('\n');
		borg_keypress('\n');
		borg_keypress('\n');

		/* Mark our last item sold but not ezheals*/
		if ((item->tval == TV_POTION && item->sval != SV_POTION_STAR_HEALING &&
			  item->sval != SV_POTION_LIFE) ||
			 item->tval != TV_POTION) {
			sold_item_pval = item->pval;
			sold_item_tval = item->tval;
			sold_item_sval = item->sval;
			sold_item_store = goal_shop;
		}

		/* The purchase is complete */
		goal_shop = goal_item = -1;
		goal_qty = 1;

		/* Go back to the first page and rebrowse this store AJG*/
		if (borg_shops[shop_num].page)
			borg_keypress(' ');
		borg_do_browse_what = -1;

		/* tick the anti-loop clock */
		time_this_panel++;

		/* rebrowse this store */
		borg_do_browse_what = -1;

		/* Success */
		return (TRUE);
	}

	/* Nope */
	return (FALSE);
}

/*
 * Buy items from the current shop, if desired
 */
static bool borg_think_shop_buy(void) {
	char buf[3];

	/* Buy something if requested */
	if ((goal_shop == shop_num) && (goal_ware >= 0)) {
		borg_shop *shop = &borg_shops[goal_shop];

		borg_item *item = &shop->ware[goal_ware];

		/* Paranoid */
		if (item->tval == 0) {
			/* The purchase is complete */
			goal_shop = goal_ware = -1;
			goal_qty = 1;

			/* rebrowse this store*/
			borg_do_browse_what = -1;

			/* Increment our clock to avoid loops */
			time_this_panel++;

			return (FALSE);
		}

		/* go to correct Page */
		if ((goal_ware / 12) != shop->page)
			borg_keypress(' ');

		/* Log */
		borg_note(format("# Buying %s.", item->desc));

		/* Buy an item */
		borg_keypress('p');

		/* Buy the desired item */
		borg_keypress(I2A(goal_ware % 12));

		/* Quantity of the item */
		borg_itoa(goal_qty, buf /*, 10*/);
		borg_keypresses(buf);
		borg_keypress('\n');

		/* Mega-Hack -- Accept the price */
		borg_keypress('\n');
		borg_keypress('\n');
		borg_keypress('\n');
		borg_keypress('\n');

		/* Remember what we bought to avoid buy/sell loops */
		bought_item_pval = item->pval;
		bought_item_tval = item->tval;
		bought_item_sval = item->sval;
		bought_item_store = goal_shop;

		/* The purchase is complete */
		goal_shop = goal_ware = -1;
		goal_qty = 1;

		/* rebrowse this store*/
		borg_do_browse_what = -1;

		/* Increment our clock to avoid loops */
		time_this_panel++;

		/*
		 * It is easier for the borg to wear the Equip if he exits
		 * the shop after buying it, even though there may be a few
		 * more items he'd like to buy.
		 */
		if (borg_wield_slot(item) >= INVEN_WIELD || time_this_panel > 100 ||
			 item->tval == TV_FOOD || item->tval == TV_FLASK) {
			/* leave the store */
			borg_keypress(ESCAPE);

			/* rebrowse this store */
			borg_do_browse_what = -1;
		}

		/* Success */
		return (TRUE);
	}

	/* Nothing to buy */
	return (FALSE);
}

/*
 * Deal with being in a store
 */
bool borg_think_store(void) {
	/* Hack -- prevent clock wrapping */
	if (borg_t >= 20000 && borg_t <= 20010) {
		/* Clear Possible errors */
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);

		/* Re-examine inven and equip */
		borg_do_inven = TRUE;
		borg_do_equip = TRUE;
	}

	/* update all my equipment and swap items */
	borg_notice(TRUE);

	/* Stamp the shop with a time stamp */
	borg_shops[shop_num].when = borg_t;

	/* Remove "backwards" rings */
	if (borg_swap_rings())
		return (TRUE);

	/* Repair "backwards" rings */
	if (borg_wear_rings())
		return (TRUE);

	/* Wear "optimal" equipment */
	if (/* shop_num != BORG_HOME && */ borg_best_stuff())
		return (TRUE);
	if (shop_num == BORG_HOME && borg_best_combo(TRUE))
		return (TRUE);

	/* If using a digger, Wear "useful" equipment */
	if (borg_items[INVEN_WIELD].tval == TV_DIGGING && borg_wear_stuff())
		return (TRUE);

	/* Remove "useless" equipment */
	if (borg_remove_stuff())
		return (TRUE);

	/* Choose a shop to visit */
	if (borg_choose_shop()) {
		/* Try to sell stuff */
		if (borg_think_shop_sell())
			return (TRUE);

		/* Try to buy stuff */
		if (borg_think_shop_buy())
			return (TRUE);
	}

	/* No shop */
	shop_num = -1;

	/* Leave the store */
	borg_keypress(ESCAPE);

	/* Done */
	return (TRUE);
}

/* Attempt a series of maneuvers to stay alive when you run out of light */
bool borg_think_dungeon_light(void) {
	int i, ii, x, y;
	bool not_safe = FALSE;
	borg_grid *ag;
	borg_kill *kill;

	if (!borg_skill[BI_LITE] &&
		 (borg_skill[BI_CUR_LITE] <= 0 || borg_items[INVEN_LITE].pval <= 3) &&
		 borg_skill[BI_CDEPTH] >= 1) {
		/* I am recalling, sit here till it engages. */
		if (goal_recalling) {
			/* just wait */
			borg_keypress('R');
			borg_keypress('9');
			borg_keypress('\n');
			return (TRUE);
		}

		/* wear stuff and see if it glows */
		if (borg_wear_stuff())
			return (TRUE);

		/* attempt to refuel */
		if (borg_refuel_torch() || borg_refuel_lantern())
			return (TRUE);

		/* Can I recall out with a rod */
		if (!goal_recalling && borg_zap_rod(SV_ROD_RECALL))
			return (TRUE);

		/* Can I recall out with a spell */
		if (!goal_recalling && borg_recall())
			return (TRUE);

		/* Log */
		borg_note("# Testing for stairs .");

		/* Test for stairs */
		borg_keypress('<');

		/* If on a glowing grid, got some food, and low mana, then rest here */
		if ((borg_grids[c_y][c_x].info & BORG_GLOW) &&
			 (borg_spell_legal(REALM_LIFE, 0, 7) ||
			  borg_spell_legal(REALM_ARCANE, 2, 6) ||
			  borg_spell_legal(REALM_NATURE, 0, 3)) &&
			 (borg_spell_legal(REALM_ARCANE, 0, 5) ||
			  borg_spell_legal(REALM_CHAOS, 0, 2) ||
			  borg_spell_legal(REALM_NATURE, 0, 4) ||
			  borg_spell_legal(REALM_SORCERY, 0, 3) ||
			  borg_spell_legal(REALM_LIFE, 0, 4)) &&
			 (!borg_spell_okay(REALM_ARCANE, 0, 5) &&
			  !borg_spell_okay(REALM_CHAOS, 0, 2) &&
			  !borg_spell_okay(REALM_NATURE, 0, 4) &&
			  !borg_spell_okay(REALM_SORCERY, 0, 3) &&
			  !borg_spell_okay(REALM_LIFE, 0, 4))) {
			/* Scan grids adjacent to me */
			for (ii = 0; ii < 8; ii++) {
				x = c_x + ddx_ddd[ii];
				y = c_y + ddy_ddd[ii];

				/* Bounds check */
				if (!in_bounds2(y, x))
					continue;

				/* Access the grid */
				ag = &borg_grids[y][x];

				/* Check for adjacent Monster */
				if (ag->kill) {
					not_safe = TRUE;
				}
			}

			/* Be concerned about the Regional Fear. */
			if (borg_fear_region[c_y / 11][c_x / 11] > borg_skill[BI_CURHP] / 10)
				not_safe = TRUE;

			/* Be concerned about the Monster Fear. */
			if (borg_fear_monsters[c_y][c_x] > borg_skill[BI_CURHP] / 10)
				not_safe = TRUE;

			/* rest here to gain some mana */
			if (not_safe == FALSE) {
				borg_note("# Resting on this Glowing Grid to gain mana.");
				borg_keypress('R');
				borg_keypress('*');
				borg_keypress('\n');
				return (TRUE);
			}
		}

		/* Dismiss Pets so we don't mistakingly attack them in the darkness */
		/* Dismiss the pets */
		borg_keypress('p');
		borg_keypress('a');
		borg_keypress('y');

		/* Remove the friendly animal from my array */
		for (i = 1; i < borg_kills_nxt; i++) {
			kill = &borg_kills[i];

			/* Skip dead monsters */
			if (!kill->r_idx)
				continue;

			/* delet all pets */
			if (kill->ally)
				borg_delete_kill(i);
		}

		/* If I have the capacity to Call Light, then do so if adjacent to a dark
		 * grid.
		 * We can illuminate the entire dungeon, looking for stairs.
		 */
		/* Scan grids adjacent to me */
		for (ii = 0; ii < 8; ii++) {
			x = c_x + ddx_ddd[ii];
			y = c_y + ddy_ddd[ii];

			/* Bounds check */
			if (!in_bounds2(y, x))
				continue;

			/* Access the grid */
			ag = &borg_grids[y][x];

			/* skip the Wall grids */
			if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM_SOLID)
				continue;

			/* Problem with casting Call Light on Open Doors */
			if ((ag->feat == FEAT_OPEN || ag->feat == FEAT_BROKEN) &&
				 (y == c_y && x == c_x)) {
				/* Cheat the grid info to see if the door is lit */
				if (cave[c_y][c_x].info == CAVE_GLOW)
					ag->info |= BORG_GLOW;
				continue;
			}

			/* Look for a dark one */
			if ((ag->info & BORG_DARK) ||  /* Known to be dark */
				 ag->feat == FEAT_NONE ||	/* Nothing known about feature */
				 !(ag->info & BORG_MARK) || /* Nothing known about info */
				 !(ag->info & BORG_GLOW))	/* not glowing */
			{
				/* Attempt to Call Light */
				if (borg_activate_artifact(ART_GALADRIEL, FALSE) ||
					 borg_zap_rod(SV_ROD_ILLUMINATION) ||
					 borg_use_staff(SV_STAFF_LITE) ||
					 borg_read_scroll(SV_SCROLL_LIGHT) ||
					 borg_mutation(COR1_ILLUMINE, FALSE, 50, FALSE) ||
					 borg_spell(REALM_ARCANE, 0, 5) ||
					 borg_spell(REALM_CHAOS, 0, 2) ||
					 borg_spell(REALM_NATURE, 0, 4) ||
					 borg_spell(REALM_SORCERY, 0, 3) ||
					 borg_spell(REALM_LIFE, 0, 4)) {
					borg_note("# Illuminating the region while dark.");
					borg_react("SELF:lite", "SELF:lite");

					return (TRUE);
				}

				/* Attempt to use Light Beam requiring a direction. */
				if (borg_lite_beam(FALSE))
					return (TRUE);
			}
		}

		/* Try to flow to upstairs if on level one */
		if (borg_flow_stair_less(GOAL_FLEE, FALSE)) {
			/* Take the stairs */
			/* Log */
			borg_note("# Taking up Stairs stairs (low Light).");
			borg_keypress('<');
			return (TRUE);
		}

		/* Try to flow to a lite */
		if (borg_skill[BI_RECALL] && borg_flow_light(GOAL_FLEE)) {
			return (TRUE);
		}
	}
	/* Nothing to do */
	return (FALSE);
}
/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to grab items close to the stair.
 */
bool borg_think_stair_scum(/*bool from_town*/) {
	int j, b_j = -1;
	int i;

	borg_grid *ag = &borg_grids[c_y][c_x];

	byte feat = cave[c_y][c_x].feat;

	borg_item *item = &borg_items[INVEN_LITE];

	/* examine equipment and swaps */
	borg_notice(TRUE);

	/* No scumming mode if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0 || borg_skill[BI_ISWEAK]) {
		borg_note("# Leaving Scumming Mode. (Town or Weak)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* No scumming if inventory is full.  Require one empty slot */
	if (borg_items[INVEN_PACK - 1].iqty)
		return (FALSE);

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE) {

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			/* We already knew about that one */
			if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y))
				break;
		}

		/* Track the newly discovered "down stairs" */
		if ((i == track_more_num) && (i < track_more_size)) {
			track_more_x[i] = c_x;
			track_more_y[i] = c_y;
			track_more_num++;
		}
		/* tell the array */
		ag->feat = FEAT_MORE;
	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS) {

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			/* We already knew about this one */
			if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y))
				continue;
		}

		/* Track the newly discovered "up stairs" */
		if ((i == track_less_num) && (i < track_less_size)) {
			track_less_x[i] = c_x;
			track_less_y[i] = c_y;
			track_less_num++;
		}

		/* Tell the array */
		ag->feat = FEAT_LESS;
	}

	/** First deal with staying alive **/

	/* Hack -- require light */
	if ((item->tval == TV_LITE && item->sval == SV_LITE_TORCH) ||
		 (item->tval == TV_LITE && item->sval == SV_LITE_LANTERN)) {

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

		if (item->pval < 250) {
			borg_note("# Scum. (need fuel)");
		}
	}

	/* Heal up from confusion if needed */
	if (borg_recover())
		return (TRUE);

	/** Track down some interesting gear **/
	/* Continue flowing towards objects */
	if (borg_flow_old(GOAL_TAKE))
		return (TRUE);

	/* Find a (viewable) object */
	if (borg_flow_take_scum(TRUE, 6))
		return (TRUE);

	/*leave level right away. */
	borg_note("# Fleeing level. Town Scumming Mode");
	goal_fleeing = TRUE;

#if 0
    /* Scumming Mode - Going down */
    if (track_more_num &&
        (ag->feat == FEAT_MORE ||
         ag->feat == FEAT_LESS ||
         borg_skill[BI_CDEPTH] < 30))
    {
        int y, x;

		if (track_more_num >= 2) borg_note("# Scumming Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more_num; i++)
        {
            x = track_more_x[i];
            y = track_more_y[i];

            /* How far is the nearest down stairs */
            j = distance(c_y, c_x, y, x);

            /* skip the far ones */
            if (b_j <= j && b_j != -1) continue;

            /* track it */
            b_j =j;
        }


        /* if the downstair is close and path is safe, continue on */
        if (b_j < 8  ||
             ag->feat == FEAT_MORE ||
             borg_skill[BI_CDEPTH] < 30)
		{
        	/* Note */
        	borg_note("# Scumming Mode.  Power Diving. ");

	       	/* Continue leaving the level */
        	if (borg_flow_old(GOAL_FLEE)) return (TRUE);

			/* Flow to DownStair */
        	if (borg_flow_stair_more(GOAL_FLEE, FALSE, TRUE)) return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE)
			{
				/* Take the DownStair */
	        	borg_on_upstairs = TRUE;
	        	borg_keypress('>');

	        	return (TRUE);
			}
		}
    }
#endif

	/* Scumming Mode - Going up */
	if (track_less_num && borg_skill[BI_CDEPTH] != 1 &&
		 (ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
		int y, x;

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
		}

		/* if the upstair is close and safe path, continue */
		if (b_j < 8 || ag->feat == FEAT_LESS) {

			/* Note */
			borg_note("# Scumming Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Scumming Mode.");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}
	}

	/* Special case where the borg is off a stair and there
	 * is a monster in LOS.  He could freeze and unhook, or
	 * move to the closest stair and risk the run.
	 */
	if (borg_skill[BI_CDEPTH] >= 2) {
		/* Continue fleeing to stair */
		if (borg_flow_old(GOAL_FLEE))
			return (TRUE);

		/* Note */
		borg_note("# Scumming Mode.  Any Stair. ");

		/* Try to find some stairs */
		if (borg_flow_stair_both_dim(GOAL_FLEE))
			return (TRUE);
		if (borg_flow_stair_both(GOAL_FLEE, FALSE, FALSE))
			return (TRUE);
	}

	/* return to normal borg_think_dungeon */
	return (FALSE);
}

/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to the bottom of the dungeon asap.
 * Once down there, he can be told to do something.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 */
bool borg_think_dungeon_lunal(void) {
	bool safe_place = FALSE;

	int j, b_j = -1;
	int i;
	/*int target_depth = 7;*/

	borg_grid *ag = &borg_grids[c_y][c_x];

	byte feat = cave[c_y][c_x].feat;

	/*borg_item *item = &borg_items[INVEN_LITE];*/

	/* examine equipment and swaps */
	borg_notice(TRUE);

	/* No Lunal mode if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0 || borg_skill[BI_ISWEAK]) {
		borg_note("# Leaving Lunal Mode. (Town or Weak)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE) {

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			/* We already knew about that one */
			if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y))
				break;
		}

		/* Track the newly discovered "down stairs" */
		if ((i == track_more_num) && (i < track_more_size)) {
			track_more_x[i] = c_x;
			track_more_y[i] = c_y;
			track_more_num++;
		}
		/* tell the array */
		ag->feat = FEAT_MORE;
	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS) {

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			/* We already knew about this one */
			if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y))
				continue;
		}

		/* Track the newly discovered "up stairs" */
		if ((i == track_less_num) && (i < track_less_size)) {
			track_less_x[i] = c_x;
			track_less_y[i] = c_y;
			track_less_num++;
		}

		/* Tell the array */
		ag->feat = FEAT_LESS;
	}

	/* Act normal on 1 unless stairs are seen */
	if (borg_skill[BI_CDEPTH] == 1 && track_more_num == 0) {
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* If no stair is known, act normal */
	if (track_more_num == 0 && track_less_num == 0) {
		borg_note("# Leaving Lunal Mode. (No Stairs seen)");
		borg_lunal_mode = FALSE;
		return (FALSE);
	}

	/* If self scumming and getting closer to zone, act normal */
	if (borg_self_lunal) {
		if (borg_skill[BI_MAXDEPTH] <= borg_skill[BI_CDEPTH] + 15 ||
			 (cptr)NULL != borg_prepared[borg_skill[BI_CDEPTH] - 5] ||
			 borg_skill[BI_CDEPTH] >= 50 || borg_skill[BI_CDEPTH] == 0 ||
			 borg_skill[BI_ISWEAK]) {
			borg_lunal_mode = FALSE;
			goal_fleeing = FALSE;
			goal_fleeing_lunal = FALSE;
			borg_note("# Self Lunal mode disengaged normally.");
			return (FALSE);
		}
	}

	/** First deal with staying alive **/

	/* Hack -- require light */
	if ((borg_items[INVEN_LITE].tval == TV_LITE &&
		  borg_items[INVEN_LITE].sval == SV_LITE_TORCH) ||
		 (borg_items[INVEN_LITE].tval == TV_LITE &&
		  borg_items[INVEN_LITE].sval == SV_LITE_LANTERN)) {

		/* Must have light -- Refuel current torch */
		if (borg_items[INVEN_LITE].tval == TV_LITE &&
			 borg_items[INVEN_LITE].sval == SV_LITE_TORCH) {
			/* Try to refuel the torch */
			if ((borg_items[INVEN_LITE].pval < 500) && borg_refuel_torch())
				return (TRUE);
		}

		/* Must have light -- Refuel current lantern */
		if (borg_items[INVEN_LITE].tval == TV_LITE &&
			 borg_items[INVEN_LITE].sval == SV_LITE_LANTERN) {
			/* Try to refill the lantern */
			if ((borg_items[INVEN_LITE].pval < 1000) && borg_refuel_lantern())
				return (TRUE);
		}

		if (borg_items[INVEN_LITE].pval < 250) {
			borg_note("# Lunal. (need fuel)");
		}
	}

	/* No Light at all */
	if (borg_skill[BI_CUR_LITE] == 0 && borg_items[INVEN_LITE].tval == 0) {
		borg_note("# No Light at all.");
		return (FALSE);
	}

	/* Define if safe_place is true or not */
	safe_place = borg_check_rest(c_y, c_x);

	/* Light Room, looking for monsters */
	/* if (safe_place && borg_check_lite_only()) return (TRUE); */

	/* Check for stairs and doors and such */
	/* if (safe_place && borg_check_lite()) return (TRUE); */

	/* Recover from any nasty condition */
	if (safe_place && borg_recover())
		return (TRUE);

	/* Consume needed things */
	if (safe_place && borg_use_things())
		return (TRUE);

	/* Consume needed things */
	if (borg_skill[BI_ISHUNGRY] && borg_use_things())
		return (TRUE);

	/* Junk the junk */
	if (safe_place && borg_crush_junk())
		return (TRUE);

	/** Track down some interesting gear **/
	/* XXX Should we allow him great flexibility in retreiving loot? (not always
	 * safe?)*/
	/* Continue flowing towards objects */
	if (safe_place && borg_flow_old(GOAL_TAKE))
		return (TRUE);

	/* Find a (viewable) object */
	if (safe_place && borg_flow_take_lunal(TRUE, 4))
		return (TRUE);

	/*leave level right away. */
	borg_note("# Fleeing level. Lunal Mode");
	goal_fleeing_lunal = TRUE;
	goal_fleeing = TRUE;

	/* Full of Items - Going up */
	if (track_less_num && borg_items[INVEN_PACK - 2].iqty &&
		 (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
		int y, x;
		int closeness = 8;

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
		}

		/* if on depth 1, try to venture more to get back to town */
		if (borg_skill[BI_CDEPTH] == 1) {
			if (track_less_num) {
				closeness = 20;
			}
		}

		/* if the upstair is close and safe path, continue */
		if ((b_j < closeness && safe_place) || ag->feat == FEAT_LESS) {

			/* Note */
			borg_note("# Lunal Mode.  Power Climb (needing to sell). ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Lunal Mode (needing to sell).");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}
	}

	/* Lunal Mode - Going down */
	if (track_more_num &&
		 (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
		int y, x;

		if (track_more_num >= 2)
			borg_note("# Lunal Mode: I know of a down stair.");

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			x = track_more_x[i];
			y = track_more_y[i];

			/* How far is the nearest down stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
		}

		/* if the downstair is close and path is safe, continue on */
		if ((b_j < 8 && safe_place) || ag->feat == FEAT_MORE) {
			/* Note */
			borg_note("# Lunal Mode.  Power Diving. ");

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to DownStair */
			if (borg_flow_stair_more(GOAL_FLEE, TRUE, TRUE))
				return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE) {
				/* Take the DownStair */
				borg_on_upstairs = TRUE;
				borg_keypress('>');

				return (TRUE);
			}
		}
	}

	/* Lunal Mode - Going up */
	if (track_less_num && borg_skill[BI_CDEPTH] != 1 &&
		 (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
		int y, x;

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
		}

		/* if the upstair is close and safe path, continue */
		if ((b_j < 8 && safe_place) || ag->feat == FEAT_LESS) {

			/* Note */
			borg_note("# Lunal Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Lunal Mode.");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}
	}

	/* Special case where the borg is off a stair and there
	 * is a monster in LOS.  He could freeze and unhook, or
	 * move to the closest stair and risk the run.
	 */
	if (borg_skill[BI_CDEPTH] >= 2) {
		/* Continue fleeing to stair */
		if (borg_flow_old(GOAL_FLEE))
			return (TRUE);

		/* Note */
		borg_note("# Lunal Mode.  Any Stair. ");

		/* Try to find some stairs */
		if (borg_flow_stair_both_dim(GOAL_FLEE))
			return (TRUE);
		if (borg_flow_stair_both(GOAL_FLEE, FALSE, FALSE))
			return (TRUE);
	}

	/* Lunal Mode - Reached 99 */
	if (borg_skill[BI_CDEPTH] == 99) {
		borg_note("# Lunal Mode ended at depth.");
	}

	/* Unable to do it */
	if (borg_skill[BI_CDEPTH] > 1) {
		borg_note("# Lunal Mode ended incorrectly.");
	}

	/* return to normal borg_think_dungeon */
	borg_note("Leaving Lunal Mode. (End of Lunal Mode)");
	borg_lunal_mode = FALSE;
	goal_fleeing = goal_fleeing_lunal = FALSE;
	return (FALSE);
}

/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to a sweet spot to gather items.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 *
 * This function can also help a borg rise dungeon levels to a safe area.
 */
bool borg_think_dungeon_munchkin(bool prep_check) {
	bool safe_place = FALSE;
	bool fearing_sunlight = FALSE;
	bool safe_monsters = TRUE;
	bool on_stair = FALSE;
	bool create_stair = FALSE;
	bool need_stair = FALSE;

	int j, b_j = -1, bb_j = MAX_RANGE;
	int i;
	int y, x;

	borg_grid *ag = &borg_grids[c_y][c_x];

	byte feat = cave[c_y][c_x].feat;

	borg_item *item = &borg_items[INVEN_LITE];

	/* examine equipment and swaps */
	borg_notice(TRUE);

	/* Not if starving or in town */
	if (borg_skill[BI_CDEPTH] == 0) {
		borg_note("# Leaving munchkin Mode. (Town or Weak)");
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/* Not if stuck in a loop */
	if (time_this_panel >= 350) {
		borg_note("# Leaving munchkin Mode. (Time Panel)");
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/* Do not climb up if during daylight hours.  Stay deeper in the dungeon
	 * until night falls.  Otherwise,
	 * borg will sit on depth 1 all afternoon unitl night falls.
	 */
	if (borg_skill[BI_FEAR_LITE] && (borg_skill[BI_HRTIME] >= 5) &&
		 (borg_skill[BI_HRTIME] <= 18))
		fearing_sunlight = TRUE;

	/* if borg is just starting on this level, he may not
	 * know that a stair is under him.  Cheat to see if one is
	 * there
	 */
	if (feat == FEAT_MORE && ag->feat != FEAT_MORE) {

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			/* We already knew about that one */
			if ((track_more_x[i] == c_x) && (track_more_y[i] == c_y))
				break;
		}

		/* Track the newly discovered "down stairs" */
		if ((i == track_more_num) && (i < track_more_size)) {
			track_more_x[i] = c_x;
			track_more_y[i] = c_y;
			track_more_num++;
		}
		/* tell the array */
		ag->feat = FEAT_MORE;
	}

	if (feat == FEAT_LESS && ag->feat != FEAT_LESS) {

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			/* We already knew about this one */
			if ((track_less_x[i] == c_x) && (track_less_y[i] == c_y))
				continue;
		}

		/* Track the newly discovered "up stairs" */
		if ((i == track_less_num) && (i < track_less_size)) {
			track_less_x[i] = c_x;
			track_less_y[i] = c_y;
			track_less_num++;
		}

		/* Tell the array */
		ag->feat = FEAT_LESS;
	}

	/* Can the borg create his own stair? */
	if (borg_spell_okay_fail(REALM_NATURE, 2, 1, 50))
		create_stair = TRUE;

	/* Set a flag if the borg is on a stair */
	if (ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)
		on_stair = TRUE;

	/* Act normal on 1 unless stairs are seen*/
	if (borg_skill[BI_CDEPTH] == 1 && track_more_num == 0 &&
		 create_stair == FALSE) {
		borg_munchkin_mode = FALSE;
		return (FALSE);
	}

	/* If no down stair is known, act normal */
	if (track_more_num == 0 && track_less_num == 0) {
		if (create_stair == FALSE) {
			borg_note("# Leaving Munchkin Mode. (No Stairs seen)");
			borg_munchkin_mode = FALSE;
			return (FALSE);
		}

		/* Can I create a stair if needed? */
		if (create_stair == TRUE) {
			need_stair = TRUE;
		}
	}

	/* Stairs are known, but are they too far away? */
	if (track_more_num >= 1 || track_less_num >= 1) {
		/* Reset */
		b_j = -1;

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			x = track_more_x[i];
			y = track_more_y[i];

			/* How far is the nearest down stairs */
			j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE) &&
				 !borg_skill[BI_PASSWALL] && !borg_skill[BI_ADIMDOOR])
				continue;

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE) &&
				 !borg_skill[BI_PASSWALL] && !borg_skill[BI_ADIMDOOR])
				continue;

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* Closest stair to me */
		if (bb_j >= 10 /* || bb_j <= 1 */) {
			/* I am not able to cast Create Stairs */
			if (create_stair == FALSE) {
				borg_note("# Leaving Munchkin Mode. (Stairs too far away)");
				borg_munchkin_mode = FALSE;
				return (FALSE);
			}

			/* I can create a stair if needed with a spell */
			if (create_stair == TRUE) {
				need_stair = TRUE;
			}
		}
	}

	/** First deal with staying alive **/

	/* Hack -- require light */
	if ((item->tval == TV_LITE && item->sval == SV_LITE_TORCH) ||
		 (item->tval == TV_LITE && item->sval == SV_LITE_LANTERN)) {

		/* Must have light -- Refuel current torch */
		if (item->tval == TV_LITE && item->sval == SV_LITE_TORCH) {
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

		if (item->pval < 250) {
			borg_note("# Munchkin. (need fuel)");
		}
	}

	/* No Light at all */
	if (borg_skill[BI_CUR_LITE] == 0 && borg_items[INVEN_LITE].pval == 0 &&
		 borg_skill[BI_CDEPTH] == 1 &&
		 (borg_mutation(COR1_ILLUMINE, TRUE, 50, FALSE) ||
		  borg_spell_okay(REALM_ARCANE, 0, 5) ||
		  borg_spell_okay(REALM_CHAOS, 0, 2) ||
		  borg_spell_okay(REALM_NATURE, 0, 4) ||
		  borg_spell_okay(REALM_SORCERY, 0, 3) ||
		  borg_spell_okay(REALM_LIFE, 0, 4) ||
		  borg_activate_activation(ACT_MAP_LIGHT, FALSE))) {
		borg_note("# Munchkin.  No Light at all.  Exploring. ");
		return (FALSE);
	}

	/* Define if safe_place is true or not */
	safe_place = borg_check_rest(c_y, c_x);
	if (borg_near_monster_type(10) >= avoidance)
		safe_monsters = FALSE;

	/* Do smart things only if there are no dangerous monster types near */
	if (!borg_fighting_tele_to) {
		/* Heal if needed */
		if (borg_skill[BI_FOOD] >= 2 && borg_heal(borg_danger(c_y, c_x, 1, TRUE)))
			return (TRUE);

		/* Can do a little attacking. */
		if (borg_skill[BI_FOOD] >= 2 && borg_munchkin_magic())
			return (TRUE);
		if (borg_skill[BI_FOOD] >= 2 && borg_munchkin_melee())
			return (TRUE);

		/* Consume needed things */
		if (safe_place && borg_use_things())
			return (TRUE);

		/* Consume needed things */
		if (borg_skill[BI_ISHUNGRY] && borg_use_things())
			return (TRUE);
		if (borg_skill[BI_ISWEAK] && (borg_eat_food_any() || borg_eat_vamp() ||
												borg_eat_unknown() || borg_quaff_unknown()))
			return (TRUE);

		/* Learn useful spells immediately */
		if (safe_place && on_stair && borg_play_magic(TRUE))
			return (TRUE);

		/* Recharge things */
		if (safe_place && on_stair && borg_recharging())
			return (TRUE);

		/* Wear stuff and see if it's good */
		if (safe_place && on_stair && borg_wear_stuff())
			return (TRUE);
		if (safe_place && on_stair && borg_remove_stuff())
			return (TRUE);

		/* Crush junk if convienent */
		if (safe_place && on_stair && borg_crush_junk())
			return (TRUE);
		if (safe_place && borg_crush_hole())
			return (TRUE);

		/* Do I need to add some light? */
		if (safe_place && borg_check_lite_only())
			return (TRUE);

		/** Track down some interesting gear **/
		/* Continue flowing towards objects */
		if (borg_flow_old(GOAL_TAKE))
			return (TRUE);

		/* Find a (viewable) object */
		if (safe_place && safe_monsters &&
			 borg_flow_take_lunal(TRUE, 5 + borg_skill[BI_CLEVEL] / 5))
			return (TRUE);

		/* Recover from any nasty condition */
		if (safe_place && on_stair && borg_recover())
			return (TRUE);

		/* Recharge wands when needed */
		if (safe_place && on_stair && borg_recharging())
			return (TRUE);
	}

	/*leave level right away. */
	borg_note("# Fleeing level. Munchkin Mode");
	goal_fleeing_munchkin = TRUE;
	goal_fleeing = TRUE;

	/* No Convenient Stair.  Create stair, if needed and able */
	if (need_stair && ag->feat == FEAT_FLOOR && borg_spell(REALM_NATURE, 2, 1)) {
		borg_note("# Munchkin Mode. Create Stair.");
		return (TRUE);
	}

	/* Full of Items - Going up */
	if ((track_less_num || create_stair) &&
		 (borg_items[INVEN_PACK - 2].iqty || !borg_items[INVEN_LITE].pval ||
		  borg_skill[BI_FOOD] <= 2 || goal_rising) &&
		 (safe_place || ag->feat == FEAT_LESS) && !fearing_sunlight) {
		int closeness = 8;

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Higher level, deeper dungeon, use Recall instead of climbing all the
		 * way */
		if (borg_skill[BI_MAXCLEVEL] >= 10 && borg_skill[BI_CDEPTH] >= 8 &&
			 borg_recall()) {
			borg_note("# Munchkin Mode.  Recalling to town to sell stuff");
		}

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE) &&
				 !borg_skill[BI_PASSWALL] && !borg_skill[BI_ADIMDOOR])
				continue;

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* if on depth 1, try to venture more to get back to town */
		if (borg_skill[BI_CDEPTH] == 1) {
			if (track_less_num) {
				closeness = 20;
			}
		}

		/* If Dim Door is available, safe to move out a bit */
		if (borg_skill[BI_ADIMDOOR]) {
			/* Size of Dim Door range */
			closeness = borg_skill[BI_CLEVEL] + 2;
		}

		/* if the upstair is close and safe path, try to take it */
		if (b_j != -1 &&
			 ((b_j < closeness && (safe_place)) || ag->feat == FEAT_LESS)) {

			/* Note */
			borg_note("# Munchkin Mode.  Power Climb (needing to sell). ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less_dim(GOAL_FLEE)) {
				borg_note("# Dim Door to stairs. Munchkin Mode (needing to sell).");
				return (TRUE);
			}
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Munchkin Mode (needing to sell).");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}

		/* if a stair is desired but not close enough. */
		if (safe_place && create_stair && (track_less_num == 0 || need_stair)) {
			/* No Convenient Stair.  Create stair, if needed and able */
			if (ag->feat == FEAT_FLOOR && borg_spell(REALM_NATURE, 2, 1)) {
				borg_note("# Munchkin Mode. Create Stair.");
				return (TRUE);
			}

			/* Note */
			borg_note("# Munchkin Mode. Move off the stair (1).");

			/* attempt to create a stair */
			for (i = 0; i < 8; i++) {
				y = c_y + ddy[i];
				x = c_x + ddx[i];

				/* in bounds */
				if (!in_bounds(c_y, c_x))
					continue;

				/* cast to the grid */
				ag = &borg_grids[y][x];

				/* Free grid, no monster or items */
				if (ag->feat == FEAT_FLOOR && !ag->kill && !ag->take) {
					/* move to that grid */
					borg_keypress(I2D(i));
					return (TRUE);
				}
			}
		}
	}

	/* Too deep; trying to gradually move shallow.  Going up */
	if (prep_check && ((track_less_num || create_stair) &&
							 ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH])) &&
							 (safe_place || ag->feat == FEAT_LESS))) {

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Reset */
		b_j = -1;

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE) &&
				 !borg_skill[BI_PASSWALL] && !borg_skill[BI_ADIMDOOR])
				continue;

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* if the upstair is close and safe path, continue */
		if ((b_j < 8 + borg_skill[BI_CLEVEL] / 5 && (safe_place)) ||
			 ag->feat == FEAT_LESS) {

			/* Note */
			borg_note("# Munchkin Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less_dim(GOAL_FLEE)) {
				borg_note("# Looking for stairs. Munchkin Mode.");
				return (TRUE);
			}
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Munchkin Mode.");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}

		/* if a stair is desired but not close enough. */
		if (safe_place && create_stair && (track_less_num == 0 || need_stair)) {
			/* No Convenient Stair.  Create stair, if needed and able */
			if (ag->feat == FEAT_FLOOR && borg_spell(REALM_NATURE, 2, 1)) {
				borg_note("# Munchkin Mode. Create Stair.");
				return (TRUE);
			}

			/* Note */
			borg_note("# Munchkin Mode. Move off the stair (2).");

			/* attempt to create a stair */
			for (i = 0; i < 8; i++) {
				y = c_y + ddy[i];
				x = c_x + ddx[i];

				/* in bounds */
				if (!in_bounds(c_y, c_x))
					continue;

				/* cast to the grid */
				ag = &borg_grids[y][x];

				/* Free grid, no monster or items */
				if (ag->feat == FEAT_FLOOR && !ag->kill && !ag->take) {
					/* move to that grid */
					borg_keypress(I2D(i));
					return (TRUE);
				}
			}
		}
	}

	/* No value in bouncing if on a quest level.  Borg will need to stay and
	 * fight the quest monsters */
	if (borg_depth & DEPTH_QUEST) {
		/* return to normal borg_think_dungeon */
		borg_note("# Leaving Munchkin Mode. (Quest Level)");
		borg_munchkin_mode = FALSE;
		goal_fleeing = goal_fleeing_munchkin = FALSE;
		return (FALSE);
	}

	/* Going down */
	if (((track_more_num || create_stair) &&
		  ((borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level &&
			 borg_skill[BI_MAXDEPTH] > borg_skill[BI_CDEPTH] &&
			 (cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH]]) ||
			(borg_skill[BI_CDEPTH] < borg_munchkin_depth ||
			 (cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH + 1]])) &&
		  safe_place) ||
		 ag->feat == FEAT_MORE) {
		int closeness = 8 + borg_skill[BI_CLEVEL] / 5;

		/* Reset */
		b_j = -1;

		if (track_more_num >= 1)
			borg_note("# Munchkin Mode: I know of a down stair.");

		/* Check for an existing "down stairs" */
		for (i = 0; i < track_more_num; i++) {
			x = track_more_x[i];
			y = track_more_y[i];

			/* How far is the nearest down stairs */
			j = distance(c_y, c_x, y, x);

			/* Is it reachable or behind a wall? */
			if (!borg_projectable(y, x, c_y, c_x, TRUE, TRUE) &&
				 !borg_skill[BI_PASSWALL] && !borg_skill[BI_ADIMDOOR])
				continue;

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* If Dim Door is available, safe to move out a bit */
		if (borg_skill[BI_ADIMDOOR]) {
			/* Size of Dim Door range */
			closeness = borg_skill[BI_CLEVEL] + 2;
		}

		/* if the downstair is close and path is safe, continue on */
		if (b_j != -1 && ((b_j < closeness && safe_place) ||
								ag->feat == FEAT_MORE || borg_skill[BI_CDEPTH] == 1)) {
			/* Note */
			borg_note("# Munchkin Mode.  Power Diving. ");

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to DownStair */
			if (borg_flow_stair_more_dim(GOAL_FLEE))
				return (TRUE);
			if (borg_flow_stair_more(GOAL_FLEE, TRUE, TRUE))
				return (TRUE);

			/* if standing on a stair */
			if (ag->feat == FEAT_MORE) {
				/* Take the DownStair */
				borg_on_upstairs = TRUE;
				borg_keypress('>');

				return (TRUE);
			}
		}

		/* if a stair is desired but not close enough. */
		if (safe_place && create_stair && (track_more_num == 0 || need_stair)) {
			/* No Convenient Stair.  Create stair, if needed and able */
			if (ag->feat == FEAT_FLOOR && borg_spell(REALM_NATURE, 2, 1)) {
				borg_note("# Munchkin Mode. Create Stair.");
				return (TRUE);
			}

			/* Note */
			borg_note("# Munchkin Mode. Move off the stair. (3)");

			/* attempt to create a stair */
			for (i = 0; i < 8; i++) {
				y = c_y + ddy[i];
				x = c_x + ddx[i];

				/* in bounds */
				if (!in_bounds(c_y, c_x))
					continue;

				/* cast to the grid */
				ag = &borg_grids[y][x];

				/* Free grid, no monster or items */
				if (ag->feat == FEAT_FLOOR && !ag->kill && !ag->take) {
					/* move to that grid */
					borg_keypress(I2D(i));
					return (TRUE);
				}
			}
		}
	}

	/* Going up */
	if ((((track_less_num || create_stair) && safe_place) ||
		  ag->feat == FEAT_LESS)) {

		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Reset */
		b_j = -1;

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
			if (b_j < bb_j)
				bb_j = b_j;
		}

		/* if the upstair is close and safe path, continue */
		if ((b_j < 8 + borg_skill[BI_CLEVEL] / 5 && (safe_place)) ||
			 ag->feat == FEAT_LESS) {

			/* Note */
			borg_note("# Munchkin Mode.  Power Climb. ");

			/* Set to help borg move better */
			goal_less = TRUE;

			/* Continue leaving the level */
			if (borg_flow_old(GOAL_FLEE))
				return (TRUE);

			/* Flow to UpStair */
			if (borg_flow_stair_less_dim(GOAL_FLEE)) {
				borg_note("# Looking for stairs. Munchkin Mode.");
				return (TRUE);
			}
			if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
				borg_note("# Looking for stairs. Munchkin Mode.");

				/* Success */
				return (TRUE);
			}

			if (ag->feat == FEAT_LESS) {
				/* Take the Up Stair */
				borg_on_dnstairs = TRUE;
				borg_keypress('<');
				return (TRUE);
			}
		}

		/* if a stair is desired but not close enough. */
		if (safe_place && create_stair && (track_less_num == 0 || need_stair)) {
			/* No Convenient Stair.  Create stair, if needed and able */
			if (ag->feat == FEAT_FLOOR && borg_spell(REALM_NATURE, 2, 1)) {
				borg_note("# Munchkin Mode. Create Stair.");
				return (TRUE);
			}

			/* Note */
			borg_note("# Munchkin Mode. Move off the stair. (4)");

			/* attempt to create a stair */
			for (i = 0; i < 8; i++) {
				y = c_y + ddy[i];
				x = c_x + ddx[i];

				/* in bounds */
				if (!in_bounds(c_y, c_x))
					continue;

				/* cast to the grid */
				ag = &borg_grids[y][x];

				/* Free grid, no monster or items */
				if (ag->feat == FEAT_FLOOR && !ag->kill && !ag->take) {
					/* move to that grid */
					borg_keypress(I2D(i));
					return (TRUE);
				}
			}
		}
	}

	/* Check for an existing "up stairs" */
	for (i = 0; i < track_less_num; i++) {
		x = track_less_x[i];
		y = track_less_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* track it */
		if (j < bb_j)
			bb_j = j;
	}

	/* Check for an existing "downstairs" */
	for (i = 0; i < track_more_num; i++) {
		x = track_more_x[i];
		y = track_more_y[i];

		/* How far is the nearest up stairs */
		j = distance(c_y, c_x, y, x);

		/* track it */
		if (j < bb_j)
			bb_j = j;
	}

	/* Special case where the borg is off a stair and there
	 * is a monster in LOS.  He could freeze and unhook, or
	 * move to the closest stair and risk the run.
	 */
	if (borg_skill[BI_CDEPTH] >= 2 || !safe_place || goal == GOAL_FLEE) {

		/* Note */
		borg_note("# Munchkin Mode.  Any Stair. ");

		/* Try to find some stairs */
		if (borg_flow_stair_both_dim(GOAL_FLEE))
			return (TRUE);

		/* If the borg is several steps away from the stair, and he will take lots
		 * of damage along the way,
		 * he might want to consider doing some attacks.
		 * For example, if he is encumbered, and 5 steps from the stair, and meets
		 * up with Fang, the borg will
		 * take several (maybe 8 attacks per round) for each of the 5 steps.  The
		 * borg might be in a position to
		 * attack Fang and scare him away or even kill him.
		 *
		 * We cant really return FALSE or he will attempt to phase door which
		 * would move him further from the stair.
		 * He needs to stand and fight.
		 */
		if (bb_j >= 4 && borg_skill[BI_FOOD] >= 2) {
			if (!borg_surrounded() && borg_skill[BI_ENCUMBERD]) {
				borg_note("# Munchkin Mode, Attack.");
				if (borg_attack(TRUE, TRUE, -1, FALSE) >= 1)
					return (TRUE);
			}
		}
		/* Continue leaving the level */
		if (borg_flow_old(GOAL_FLEE))
			return (TRUE);
		if (borg_flow_stair_both(GOAL_FLEE, FALSE, FALSE))
			return (TRUE);
	}

	/* return to normal borg_think_dungeon */
	borg_note("# Leaving Munchkin Mode. (End of Mode)");
	borg_munchkin_mode = FALSE;
	goal_fleeing = goal_fleeing_munchkin = FALSE;
	return (FALSE);
}

/*
 * Hack -- perform an action in the dungeon under boosted bravery
 *
 * This function is a sub-set of the standard dungeon goals, and is
 * only executed when all of the standard dungeon goals fail, because
 * of excessive danger, or because the level is "bizarre".
 */
static bool borg_think_dungeon_brave(void) {
	/*** Local stuff ***/

	/* Attack monsters */
	if (borg_attack(TRUE, TRUE, -1, FALSE) >= 1)
		return (TRUE);

	/* Cast a light beam to remove fear of an area */
	if (borg_lite_beam(FALSE))
		return (TRUE);

	/*** Flee (or leave) the level ***/

	/* Take stairs down from town */
	if (borg_skill[BI_CDEPTH] == 0) {
		/* Current grid */
		borg_grid *ag = &borg_grids[c_y][c_x];

		/* Usable stairs */
		if (ag->feat == FEAT_MORE) {

			/* Take the stairs */
			borg_on_upstairs = TRUE;
			borg_note("# Fleeing town via Stairs.");
			borg_keypress('>');

			/* Success */
			return (TRUE);
		}
	}

	/* Return to Stairs, but not use them */
	if (goal_less) {
		/* Continue fleeing to stair */
		if (borg_flow_old(GOAL_FLEE))
			return (TRUE);

		/* Try to find some stairs */
		if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
			 borg_flow_stair_both_dim(GOAL_FLEE))
			return (TRUE);
		if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
			 borg_flow_stair_both(GOAL_FLEE, FALSE, FALSE))
			return (TRUE);

		/* Try to find some stairs up */
		if (borg_flow_stair_less(GOAL_FLEE, TRUE))
			return (TRUE);
	}

	/* Flee the level */
	if (goal_fleeing || goal_leaving || (borg_depth & DEPTH_SCARY)) {
		/* Hack -- Take the next stairs */
		stair_less = goal_fleeing;
		if (borg_ready_lucifer == 0)
			stair_less = TRUE;

		/* Only go down if fleeing or prepared. */
		stair_more = goal_fleeing;
		/* if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH]+1]) */
		stair_more = TRUE;

		/* Continue fleeing the level */
		if (borg_flow_old(GOAL_FLEE))
			return (TRUE);

		/* Try to find some stairs up */
		if (stair_less)
			if (borg_flow_stair_less(GOAL_FLEE, TRUE))
				return (TRUE);

		/* Try to find some stairs down */
		if (stair_more)
			if (borg_flow_stair_more(GOAL_FLEE, TRUE, FALSE))
				return (TRUE);
	}

	/* Do short looks on special levels */
	if ((borg_depth & DEPTH_VAULT)) {
		/* Continue flowing towards monsters */
		if (borg_flow_old(GOAL_KILL))
			return (TRUE);

		/* Find a (viewable) monster */
		if (borg_flow_kill(TRUE, 35))
			return (TRUE);

		/* Continue flowing towards objects */
		if (borg_flow_old(GOAL_TAKE))
			return (TRUE);

		/* Find a (viewable) object */
		if (borg_flow_take(TRUE, 35))
			return (TRUE);
		if (borg_flow_vein(TRUE, 35))
			return (TRUE);
	}

	/* Continue flowing towards monsters */
	if (borg_flow_old(GOAL_KILL))
		return (TRUE);

	/* Find a (viewable) monster */
	if (borg_flow_kill(TRUE, 250))
		return (TRUE);

	/* Continue flowing towards objects */
	if (borg_flow_old(GOAL_TAKE))
		return (TRUE);

	/* Find a (viewable) object */
	if (borg_flow_take(TRUE, 250))
		return (TRUE);
	if (borg_flow_vein(TRUE, 250))
		return (TRUE);

	/*** Exploration ***/

	/* Continue flowing (see below) */
	if (borg_flow_old(GOAL_MISC))
		return (TRUE);

	/* Continue flowing (see below) */
	if (borg_flow_old(GOAL_DARK))
		return (TRUE);

	/* Continue flowing (see below) */
	if (borg_flow_old(GOAL_XTRA))
		return (TRUE);

	/* Continue flowing (see below) */
	if (borg_flow_old(GOAL_BORE))
		return (TRUE);

	/*** Explore the dungeon ***/

	/* Explore interesting grids */
	if (borg_flow_dark(TRUE))
		return (TRUE);

	/* Explore interesting grids */
	if (borg_flow_dark(FALSE))
		return (TRUE);

	/*** Track down old stuff ***/

	/* Chase old objects */
	if (borg_flow_take(FALSE, 250))
		return (TRUE);
	if (borg_flow_vein(FALSE, 250))
		return (TRUE);

	/* Chase old monsters */
	if (borg_flow_kill(FALSE, 250))
		return (TRUE);

	/* Attempt to leave the level */
	if (borg_leave_level(TRUE))
		return (TRUE);

	/* Search for secret doors */
	if (borg_flow_spastic(TRUE))
		return (TRUE);

	/* Nothing */
	return (FALSE);
}

/*
 * Perform an action in the dungeon
 *
 * Return TRUE if a "meaningful" action was performed
 * Otherwise, return FALSE so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 *
 * Fleeing:
 *   Use word of recall when level is "scary"
 *   Flee to stairs when there is a chance of death
 *   Avoid "stair bouncing" if at all possible
 *
 * Note that the various "flow" actions allow the Borg to flow
 * "through" closed doors, which will be opened when he attempts
 * to pass through them, so we do not have to pursue terrain until
 * all monsters and objects have been dealt with.
 *
 * XXX XXX XXX The poor Borg often kills a nasty monster, and
 * then takes a nap to recover from damage, but gets yanked
 * back to town before he can collect his reward.
 */
bool borg_think_dungeon(void) {
	int i, j /*, ii*/;
	int b_j = -1;
	int y, x;

/*borg_grid *ag;*/
#ifdef BORG_TK
	extern byte borgtk_delay_factor;
	int msec = borgtk_delay_factor * borgtk_delay_factor;
#else  /* not BORG_TK */
	int msec = (borg_delay_factor * borg_delay_factor * borg_delay_factor);
#endif /* not BORG_TK */

	bool fearing_sunlight = FALSE;

	/* Do not climb up if during daylight hours.  Stay deeper in the dungeon
	 * until night falls.  Otherwise,
	 * borg will sit on depth 1 all afternoon unitl night falls.
	 */
	if (borg_skill[BI_FEAR_LITE] && (borg_skill[BI_HRTIME] >= 5) &&
		 (borg_skill[BI_HRTIME] <= 18))
		fearing_sunlight = TRUE;

	/* allows user to stop the borg on certain levels */
	if (borg_skill[BI_CDEPTH] == borg_stop_dlevel)
		borg_oops("Auto-stop for user DLevel.");
	if (borg_skill[BI_CLEVEL] == borg_stop_clevel)
		borg_oops("Auto-stop for user CLevel.");

	/* allow the borg to stop if money scumming */
	if (borg_gold > borg_money_scum_amount && borg_money_scum_amount != 0 &&
		 !borg_skill[BI_CDEPTH]) {
		borg_oops("Money Scum complete.");
	}

	/* Hack -- Stop the borg if money scumming and the shops are out of food. */
	if (!borg_skill[BI_CDEPTH] && borg_money_scum_amount != 0 &&
		 borg_food_onsale == 0) {
		borg_oops("Money Scum stopped.  No more food in shop.");
	}
	/* Prevent some keypress loops */
	if (borg_t % 300 == 0) {
		borg_keypress(ESCAPE);
	}

	/* Hack -- prevent clock wrapping Step 1*/
	if (borg_t >= 7000 && borg_t <= 7010) {
		/* Clear Possible errors */
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);

		/* Re-examine inven and equip */
		borg_do_inven = TRUE;
		borg_do_equip = TRUE;

		/* enter a special routine to handle this behavior.  Messing with
		 * the old_level forces him to re-explore this level, and reshop,
		 * if in town.
		 */
		old_depth = 126;

		/* Continue on */
		return (TRUE);
	}

	/* Hack -- prevent clock wrapping Step 2*/
	if (borg_t >= 30000) {
		/* Panic */
		borg_oops("clock overflow");

#ifdef BABLOS
		/* Clock overflow escape code */
		printf("Clock overflow code!\n");
		p_ptr->playing = FALSE;
		p_ptr->leaving = TRUE;
		borg_clock_over = TRUE;
#endif /* BABLOS */

		/* Oops */
		return (TRUE);
	}

	/* Allow respawning borgs to update their variables */
	if (borg_respawning > 1) {
		borg_note(format("# Pressing 'space' to catch up and get in sync (%d).",
							  borg_respawning));
		borg_keypress(' ');
		borg_keypress(ESCAPE);
		borg_keypress(ESCAPE);
		borg_respawning--;
		return (TRUE);
	}
#ifdef BORG_TK
	if (msec)
#endif /* BORG_TK */
		/* add a short pause to slow the borg down for viewing */
		Term_xtra(TERM_XTRA_DELAY, msec);

	/* redraw the screen if we need to */
	if (my_need_redraw) {
		borg_note(format("#  Redrawing screen."));
		do_cmd_redraw();
		my_need_redraw = FALSE;
	}
	/* Prevent clock overflow */
	if (borg_t - borg_began >= 10000) {
		/* Start leaving */
		if (!goal_leaving) {
			/* Note */
			borg_note("# Leaving (boredom)");

			/* Start leaving */
			goal_leaving = TRUE;
		}

		/* Start fleeing */
		if (!goal_fleeing) {
			/* Note */
			borg_note("# Fleeing (boredom)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}
	}

	/* am I fighting a unique or a summoner, or scaryguy? */
	borg_near_monster_type(borg_skill[BI_MAXCLEVEL] < 15 ? 12 : MAX_SIGHT);

	/* Allow borg to jump back up to town if needed.  He probably fled town
	* because
	* he saw a scaryguy (BSV, SER, Maggot).  Since he is here on depth 1, do a
	* quick
	* check for items near the stairs that I can pick up before I return to town.
	*/
	if (borg_skill[BI_CDEPTH] == 1 && borg_fleeing_town) {

		/* Try to grab a close item while I'm down here */
		if (borg_think_stair_scum(/*TRUE*/))
			return (TRUE);

		/* Start leaving */
		if (!goal_leaving) {
			/* Note */
			borg_note("# Leaving (finish shopping)");

			/* Start leaving */
			goal_leaving = TRUE;
		}

		/* Start fleeing */
		if (!goal_fleeing) {
			/* Note */
			borg_note("# Fleeing (finish shopping)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}
	}

	/* Prevent a "bouncing Borg" bug. Where borg with telepathy
	 * will sit in a narrow area bouncing between 2 or 3 places
	 * tracking and flowing to a bouncing monster behind a wall.
	 *
	 * 1. Clear goals
	 * 2. Clear all objects
	 * 3. Flee the level
	 */
	if (borg_skill[BI_CDEPTH] &&
		 (time_this_panel >= 300 && time_this_panel <= 303)) {
		/* Clear goals, start flow over */
		goal = 0;
	}
	if (borg_skill[BI_CDEPTH] &&
		 (time_this_panel >= 400 && time_this_panel <= 405)) {
		/* Clear all objects */
		borg_takes_cnt = 0;
		borg_takes_nxt = 1;
		C_WIPE(borg_takes, 256, borg_take);
	}

	if (borg_skill[BI_CDEPTH] && time_this_panel >= 350 &&
		 time_this_panel <= 399 &&
		 !(borg_depth & (DEPTH_SUMMONER | DEPTH_BORER | POSITION_SEA))) {

		/* Start leaving */
		if (!goal_leaving) {
			/* Note */
			borg_note("# Leaving (bouncing-borg)");

			/* Start leaving */
			goal_leaving = TRUE;
		}

		/* Start fleeing */
		if (!goal_fleeing) {
			/* Note */
			borg_note("# Fleeing (bouncing-borg)");

			/* Start fleeing */
			goal_fleeing = TRUE;
		}
	}

	/* Avoid the burning sun */
	if (borg_skill[BI_FEAR_LITE] && borg_skill[BI_CDEPTH] == 0 &&
		 (borg_skill[BI_HRTIME] >= 5) && (borg_skill[BI_HRTIME] <= 18)) {
		/* Get out of the Sun */
		if (!goal_fleeing) {
			/* Flee */
			borg_note("# Avoiding Sunlight.");

			/* Ignore multipliers */
			goal_fleeing = TRUE;
		}
	}

	/* Avoid annoying farming */
	if (borg_t - borg_began >= 2000) {
		/* Ignore monsters from boredom */
		if (!goal_ignoring) {
			/* Flee */
			borg_note("# Ignoring breeders (boredom)");

			/* Ignore multipliers */
			goal_ignoring = TRUE;
		}
	}

	/* Reset avoidance */
	if (avoidance != borg_skill[BI_CURHP]) {
		/* Reset "avoidance" */
		avoidance = borg_skill[BI_CURHP];

		/* Re-calculate danger */
		borg_danger_wipe = TRUE;
	}

	/* Keep borg on a short leash */
	if (track_less_num && borg_skill[BI_CLEVEL] < 15 &&
		 borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 && !goal_less) {
		int y, x;

		/* Check for an existing "up stairs" */
		for (i = 0; i < track_less_num; i++) {
			x = track_less_x[i];
			y = track_less_y[i];

			/* How far is the nearest up stairs */
			j = distance(c_y, c_x, y, x);

			/* skip the far ones */
			if (b_j <= j && b_j != -1)
				continue;

			/* track it */
			b_j = j;
		}

		/* is the upstair too far away? */
		if (b_j > borg_skill[BI_CLEVEL] * 3 + 14) {
			/* Return to Stairs */
			if (!goal_less) {
				/* Note */
				borg_note(format("# Return to Stair (wandered too far.  Leash: %d)",
									  borg_skill[BI_CLEVEL] * 5 + 14));

				/* Start returning */
				goal_less = TRUE;
			}

		}
		/* Clear the flag to Return to the upstair-- we are close enough now */
		else if (goal_less && b_j < 3) {
			/* Note */
			borg_note("# Close enough to Stair.");

			/* Clear the flag */
			goal_less = FALSE;
			goal = 0;
		}
	}

	/* Quick check to see if borg needs to engage his lunal mode */
	if (borg_self_lunal && !borg_plays_risky) /* Risky borg in a hurry */
	{
		if ((cptr)NULL ==
				  borg_prepared[borg_skill[BI_CDEPTH] + 15] && /* Prepared */
			 borg_skill[BI_MAXDEPTH] >=
				  borg_skill[BI_CDEPTH] + 15 && /* Right zone */
			 borg_skill[BI_CDEPTH] >= 1 &&	  /* In dungeon fully */
			 borg_skill[BI_CDEPTH] > borg_skill[BI_CLEVEL] / 3) /* Not shallow */
		{
			borg_lunal_mode = TRUE;

			/* Enter the Lunal scumming mode */
			if (borg_lunal_mode && borg_think_dungeon_lunal())
				return (TRUE);
		}
	}

	/* Quick check to see if borg needs to engage his lunal mode for
	 * munchkin_start */
	if (borg_munchkin_start &&
		 (borg_skill[BI_MAXCLEVEL] <= borg_munchkin_level ||
		  (borg_skill[BI_FOOD] <= 2 && !borg_skill[BI_RECALL]) ||
		  (borg_skill[BI_MAXCLEVEL] >= borg_munchkin_level &&
			borg_skill[BI_MAXDEPTH] > borg_skill[BI_CDEPTH] + 5)))

	{
		if (borg_skill[BI_CDEPTH] >= 1) {
			borg_munchkin_mode = TRUE;

			/* Enter the munchkin scumming mode */
			if (borg_think_dungeon_munchkin(FALSE))
				return (TRUE);
		}

	} else
		borg_munchkin_mode = FALSE;

	/* If critially low on food, scum to the top */
	if (borg_skill[BI_FOOD] <= 2 && !borg_skill[BI_RECALL])

	{
		if (borg_skill[BI_CDEPTH] >= 1) {
			borg_munchkin_mode = TRUE;

			/* Enter the munchkin scumming mode */
			if (borg_think_dungeon_munchkin(TRUE))
				return (TRUE);
		}

	} else
		borg_munchkin_mode = FALSE;

	/* Keep borg on a suitable level */
	if (track_less_num && borg_skill[BI_CLEVEL] < 10 && !goal_less &&
		 (cptr)NULL != borg_prepared[borg_skill[BI_CDEPTH]]) {
		/* Note */
		borg_note("# Needing to get back on correct depth");

		/* Start returning */
		goal_less = TRUE;

		/* Take stairs */
		if (borg_grids[c_y][c_x].feat == FEAT_LESS) {
			borg_keypress('<');
			return (TRUE);
		}
	}

	/*** crucial goals ***/

	/* examine equipment and swaps */
	borg_notice(TRUE);

	/* Quick check on the inventory in case we need to ininscribe something and
	 * avoid loops. */
	if (borg_needed_deinscribe())
		return (TRUE);

	/* require light-- Special handle for being out of a light source.*/
	if (borg_think_dungeon_light())
		return (TRUE);

	/* Decrease the amount of time not allowed to retreat */
	if (borg_no_retreat > 0)
		borg_no_retreat--;

	/* Use things */
	if (borg_skill[BI_ISWEAK] && borg_use_things())
		return (TRUE);

	/*** Important goals ***/

	/* Continue flowing towards good anti-summon grid */
	if (borg_flow_old(GOAL_DIGGING)) {
	}
	borg_note(format("# Current goal digging is at (%d,%d)", borg_goal_y,
						  borg_goal_y));
	return (TRUE);
}

/* Continue flowing to a safe grid so monsters cant reach me */
if (borg_flow_old(GOAL_UNREACH)) {
	borg_note(
		 format("# Current goal fleeing to (%d,%d)", borg_goal_y, borg_goal_y));
	return (TRUE);
}

/* Try not to die */
if (borg_caution())
	return (TRUE);

/*** if returning from dungeon in bad shape...***/
if (borg_skill[BI_CUR_LITE] == 0 || borg_skill[BI_ISCUT] ||
	 borg_skill[BI_ISPOISONED] || borg_skill[BI_ISWEAK]) {
	/* First try to wear something */
	if (borg_skill[BI_CUR_LITE] == 0) {
		/* attempt to refuel */
		if (borg_refuel_torch() || borg_refuel_lantern())
			return (TRUE);

		/* wear stuff and see if it glows */
		if (borg_wear_stuff())
			return (TRUE);
	}

	/* Recover from damage */
	if (borg_recover())
		return (TRUE);

	/* Continue flowing (see below) */
	if (borg_flow_old(GOAL_TOWN))
		return (TRUE);

	/* Try to get to town location when not moving towns (town gate for now)
	if (when_waypoint[0] == 0 && borg_flow_town_exit(GOAL_TOWN))
		return (TRUE);
	*/

	/* Vamps need to eat right away */
	if ((borg_skill[BI_VAMPIRE] ||
		  borg_spell_legal_fail(REALM_DEATH, 1, 3, 30)) &&
		 borg_skill[BI_ISWEAK]) {
		if (borg_attack(FALSE, TRUE, -1, FALSE) >= 1)
			return (TRUE);
		if (borg_flow_old(GOAL_KILL))
			return (TRUE);
		if (borg_flow_kill(FALSE, 50))
			return (TRUE);
	}

	/* Try to get food from the Inn */
	if (borg_flow_shop_inn())
		return (TRUE);

	/* shop for something that will help us */
	if (borg_flow_shop_visit())
		return (TRUE);
	if (borg_choose_shop()) {
		/* Try and visit a shop, if so desired */
		if (borg_flow_shop_entry(goal_shop))
			return (TRUE);
	}
}

/* if I must go to town without delay */
if ((cptr)NULL != borg_restock(borg_skill[BI_CDEPTH])) {
	/* Recover from damage */
	if (borg_recover())
		return (TRUE);

	/* Leave if able to */
	if (borg_leave_level(FALSE))
		return (TRUE);
}

/* Learn useful spells immediately */
if (borg_play_magic(FALSE))
	return (TRUE);

/* If using a digger, Wear "useful" equipment before fighting monsters */
if (borg_items[INVEN_WIELD].tval == TV_DIGGING && borg_wear_stuff())
	return (TRUE);

/* If fighting certain types of creatures, consider the swap gear for better
 * damage. */
if ((borg_fighting_summoner || borg_fighting_unique || borg_fighting_questor ||
	  borg_fighting_dragon || borg_fighting_demon) &&
	 borg_wear_swap())
	return (TRUE);

/* Dig an anti-summon corridor */
if (borg_flow_kill_corridor_1(TRUE))
	return (TRUE);

/* Move to a safe grid and beat monsters with ranged attacks (play
 * nanny-boo-boo) */
if (borg_flow_kill_unreachable(4))
	return (TRUE);

/* Attack monsters */
if (borg_attack(FALSE, TRUE, -1, FALSE) >= 1)
	return (TRUE);

/* Wear things that need to be worn, but try to avoid swap loops */
if (borg_wear_stuff())
	return (TRUE);

/* Repair "backwards" rings */
if (borg_wear_rings())
	return (TRUE);

/* Remove stuff that is useless or detrimental */
if (borg_remove_stuff())
	return (TRUE);

/* Continue flowing towards really close objects */
if (borg_flow_old(GOAL_TAKE))
	return (TRUE);

/* Recover from damage */
if ((borg_position & POSITION_SUMM) && borg_recover())
	return (TRUE);

/* Flee to a safe Sea of Runes / Morgoth grid if appropriate */
if (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCONFUSED] &&
	 (goal == GOAL_MISC || ((borg_depth & (DEPTH_BORER & DEPTH_SUMMONER)) &&
									!(borg_position & POSITION_SEA)) ||
	  ((borg_depth & DEPTH_BORER) &&
		!(borg_position & (POSITION_BORE | POSITION_SEA))))) {
	/* Continue flowing towards good morgoth grid */
	if (!(borg_position & (POSITION_SEA | POSITION_BORE)) &&
		 borg_skill[BI_AGLYPH] >= 10 && borg_flow_old(GOAL_MISC))
		return (TRUE);

	/* Attempt to locate a good Glyphed grid */
	if (!(borg_position & (POSITION_SEA | POSITION_BORE)) &&
		 borg_skill[BI_AGLYPH] >= 10 && borg_flow_glyph(GOAL_MISC))
		return (TRUE);

	/* Have the borg excavate the dungeon with Stone to Mud.  Leave range at
	 * 1, 2, or 3.  */
	if (borg_excavate_region(3))
		return (TRUE);
}

/* Attempt to continue some excavation while in the sea of runes */
if (((borg_depth & (DEPTH_SUMMONER & DEPTH_BORER)) &&
	  (borg_position & POSITION_SEA)) ||
	 ((borg_depth & DEPTH_BORER) &&
	  (borg_position & (POSITION_BORE | POSITION_SEA)))) {
	/* Have the borg excavate the dungeon with Stone to Mud
	 * This should be smaller than borg_wall_buffer
	 */
	if (borg_excavate_region(6))
		return (TRUE);

	/* Test for resting on the grid one more time. */
	if (borg_recover())
		return (TRUE);
	else {
		borg_keypress(',');
		borg_note(format("# Waiting for borer. Time since seen: %d",
							  borg_t - borg_t_questor));
		return (TRUE);
	}
}

/* Find a really close object */
if (borg_flow_take(TRUE, 5))
	return (TRUE);

/* Continue flowing to a safe grid on which I may recover */
if (borg_flow_old(GOAL_RECOVER))
	return (TRUE);

/* Check the light */
if (borg_check_lite())
	return (TRUE);

/* Recover from damage */
if (borg_recover())
	return (TRUE);

/* Attempt to find a grid which is safe and I can recover on it.  This should
 * work closely with borg_recover. */
if (borg_flow_recover(/*FALSE, */ 50))
	return (TRUE);

/* Stuck in a wall without Wraith Form */
if (!borg_skill[BI_PASSWALL] && borg_grids[c_y][c_x].feat >= FEAT_DOOR_HEAD &&
	 borg_grids[c_y][c_x].feat <= FEAT_PERM_SOLID) {
	if (borg_flow_passwall())
		return (TRUE);
}

/* Perform "cool" perma spells */
if (borg_perma_spell())
	return (TRUE);

/* Destroy junk */
if (borg_crush_junk())
	return (TRUE);

/* Destroy items to make space */
if (borg_crush_hole())
	return (TRUE);

/* Destroy items if we are slow */
if (borg_crush_slow())
	return (TRUE);

/* Try to stick close to stairs if weak */
if (borg_skill[BI_CLEVEL] < 10 && borg_skill[BI_MAXSP] >= 2 &&
	 borg_skill[BI_CURSP] <= 1) {
	if (borg_skill[BI_CDEPTH]) {
		/* Check for an existing "up stairs" */
		if (borg_grids[c_y][c_x].feat == FEAT_LESS) {
			/* I am standing on a stair */

			/* reset the goal_less flag */
			goal_less = FALSE;

			/* if not dangerous, wait here */
			if (borg_danger(c_y, c_x, 1, TRUE) == 0) {
				/* rest here a moment */
				borg_note("# Resting on stair to gain Mana.");
				borg_keypress('R');
				borg_keypress('*');
				borg_keypress('\n');
				return (TRUE);
			}
		}
	} else /* in town */
	{
		int i, y, x;

		/* Check for an existing "dn stairs" */
		for (i = 0; i < track_more_num; i++) {
			x = track_more_x[i];
			y = track_more_y[i];

			/* Not on a stair */
			if (c_y != y || c_x != x)
				continue;

			/* I am standing on a stair */

			/* if not dangerous, wait here */
			if (borg_danger(c_y, c_x, 1, TRUE) == 0) {
				/* rest here a moment */
				borg_note("# Resting on town stair to gain Mana.");
				borg_keypress('R');
				borg_keypress('*');
				borg_keypress('\n');
				return (TRUE);
			}
		}
	}

	/* Continue to find a stair on which to camp */
	if (borg_flow_old(GOAL_FLEE))
		return (TRUE);

	/* Try to find some stairs up */
	if (borg_flow_stair_less(GOAL_FLEE, TRUE)) {
		borg_note("# Looking for stairs. Stair hugging.");
		return (TRUE);
	}
}

/* If in town and have no money, and nothing to sell,
 * then do not stay in town, its too dangerous.
 */
if (borg_skill[BI_CDEPTH] == 0 && borg_gold < 25 && borg_count_sell() < 3 &&
	 goal != GOAL_BORE && goal != GOAL_FLEE && borg_time_town < 250) {
	goal_leaving = TRUE;

	/* Wait here if need be.  Do Vamps need to worry about waiting? */
	if (goal_recalling) {
		borg_keypresses("R10");
		borg_keypress('\r');
		return (TRUE);
	}

	/* Continue fleeing the level */
	if (borg_flow_old(GOAL_FLEE))
		return (TRUE);
	if (borg_flow_old(GOAL_TOWN))
		return (TRUE);

	/* Note */
	borg_note("# Nothing to sell in town (leaving).");

	/* Flow back to the stairs, but don't take them yet. */
	/* Use recall when appropriate */
	if (borg_leave_level(TRUE))
		return (TRUE);
}

/*** Flee the level XXX XXX XXX ***/

/* Return to Stairs, but not use them */
if (goal_less) {
	/* Continue fleeing to stair */
	if (borg_flow_old(GOAL_FLEE))
		return (TRUE);

	/* Try to find some stairs */
	if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
		 borg_flow_stair_both_dim(GOAL_FLEE))
		return (TRUE);
	if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
		 borg_flow_stair_both(GOAL_FLEE, FALSE, TRUE))
		return (TRUE);

	/* Try to find some stairs up */
	if (borg_flow_stair_less(GOAL_FLEE, TRUE))
		return (TRUE);
}

/* Flee the level */
if (goal_fleeing && !goal_recalling) {
	/* Hack -- Take the next stairs */
	stair_less = stair_more = TRUE;

	/* Continue fleeing the level */
	if (borg_flow_old(GOAL_FLEE))
		return (TRUE);

	/* Give a short reason */
	if (borg_verbose)
		borg_note("# Fleeing level but not Recalling.");

	/* Try to find some stairs */
	if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
		 borg_flow_stair_both_dim(GOAL_FLEE))
		return (TRUE);
	if ((borg_depth & DEPTH_SCARY) && !borg_skill[BI_CDEPTH] &&
		 borg_flow_stair_both(GOAL_FLEE, FALSE, FALSE))
		return (TRUE);

	/* Try to find some stairs up */
	if (borg_flow_stair_less(GOAL_FLEE, TRUE))
		return (TRUE);

	/* Try to find some stairs down */
	if (borg_flow_stair_more(GOAL_FLEE, TRUE, TRUE))
		return (TRUE);
}

/* Dig an anti-summon corridor */
if (borg_flow_kill_corridor_2(TRUE))
	return (TRUE);

/* Continue flowing towards really close objects */
if (borg_flow_old(GOAL_TAKE))
	return (TRUE);

/* Find a really close object */
if (borg_flow_take(TRUE, 5))
	return (TRUE);

/*** Wait for recall ***/

/* Wait for recall, unless in danger */
if (goal_recalling && (borg_danger(c_y, c_x, 1, TRUE) <= 0) &&
	 borg_check_rest(c_y, c_x)) {
	/* Take note */
	borg_note("# Waiting for Recall...");

	if (borg_skill[BI_CDEPTH]) {
		/* Rest until done */
		borg_keypress('R');
		borg_keypress('&');
		borg_keypress('\n');
	} else {
		/* Rest one round-- we keep count of turns while in town */
		borg_keypress('0');
		borg_keypress('1');
		borg_keypress('R');
	}

	/* Done */
	return (TRUE);
}

/* Find a really close mineral vein */
if (borg_flow_vein(TRUE, 5))
	return (TRUE);

/* Continue flowing towards monsters */
if (borg_flow_old(GOAL_KILL))
	return (TRUE);

/* Find a close monster */
if (borg_flow_kill(TRUE, 20))
	return (TRUE);

/* Continue flowing towards objects */
if (borg_flow_old(GOAL_TAKE))
	return (TRUE);

/* Find a close object */
if (borg_flow_take(FALSE, 20))
	return (TRUE);

/* Find a close mineral vein */
if (borg_flow_vein(TRUE, 20))
	return (TRUE);

/* Find a mid-ranged monster */
if (borg_flow_kill(TRUE, 35))
	return (TRUE);

/* Find an object */
if (borg_flow_take(FALSE, 150))
	return (TRUE);

/* Find a mineral vein */
if (borg_flow_vein(TRUE, 150))
	return (TRUE);

/* Find a viewable monster and line up a shot on him */
if (borg_flow_kill_aim(TRUE))
	return (TRUE);

/* Dig an anti-summon corridor */
if (borg_flow_kill_corridor_2(TRUE))
	return (TRUE);

/*** Deal with inventory objects ***/

/* check for anything that should be inscribed
	if (borg_inscribe_food()) return (TRUE);*/

/* Use things */
if (borg_use_things())
	return (TRUE);

/* MindCrafters psuedoID unknown things */
if (borg_pseudoid_stuff())
	return (TRUE);

/* Identify unknown things */
if (borg_test_stuff(FALSE))
	return (TRUE);

/* *Id* unknown things */
if (borg_test_stuff(TRUE))
	return (TRUE);

/* Enchant things */
if (borg_enchanting())
	return (TRUE);

/* Recharge things */
if (borg_recharging())
	return (TRUE);

/*** Flow towards objects ***/

/* Continue flowing towards objects */
if (borg_flow_old(GOAL_TAKE))
	return (TRUE);

/* Find a (viewable) object */
if (borg_flow_take(FALSE, 250))
	return (TRUE);
if (borg_flow_vein(FALSE, 250))
	return (TRUE);

/*** Leave the level XXX XXX XXX ***/

/* Leave the level */
if ((goal_leaving && !goal_recalling && !unique_on_level &&
	  !(borg_depth & DEPTH_QUEST)) ||
	 (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
	  borg_count_sell() >= 12)) {
	/* Hack -- Take the next stairs */
	if (borg_ready_lucifer == 0)
		stair_less = TRUE;

	/* Continue leaving the level */
	if (borg_flow_old(GOAL_FLEE))
		return (TRUE);

	/* Try to find some stairs up */
	if (stair_less && !fearing_sunlight) {
		if (borg_flow_stair_less(GOAL_FLEE, FALSE)) {
			borg_note("# Looking to leave level to sell stuff.");
			return (TRUE);
		}
	}

	/* Only go down if fleeing or prepared. */
	if ((cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1])
		stair_more = TRUE;

	/* No down if needing to sell */
	if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 &&
		 borg_gold < 25000 && borg_count_sell() >= 12) {
		stair_more = FALSE;
	}

	/* Try to find some stairs down */
	if (stair_more) {
		if (borg_flow_stair_more(GOAL_FLEE, FALSE, TRUE)) {
			/* Give a short reason */
			borg_note("# Leaving level but not Recalling or Fleeing.");
			return (TRUE);
		}
	}
}

/* Power dive if I am playing too shallow
 * This is also seen in leave_level().  If
 * this formula is modified here, change it
 * in leave_level too.
 */
if (borg_skill[BI_CDEPTH] != 0 &&
	 (cptr)NULL == borg_prepared[borg_skill[BI_CDEPTH] + 1] && !stair_less) {
	if (borg_leave_level(FALSE))
		return (TRUE);
}

/*** Exploration ***/

/* Continue flowing (see below) */
if (borg_flow_old(GOAL_MISC))
	return (TRUE);

/* Continue flowing (see below) */
if (borg_flow_old(GOAL_DARK))
	return (TRUE);

/* Continue flowing (see below) */
if (borg_flow_old(GOAL_XTRA))
	return (TRUE);

/* Continue flowing (see below) */
if (borg_flow_old(GOAL_BORE))
	return (TRUE);

/* Continue flowing (see below) */
if (borg_flow_old(GOAL_TOWN))
	return (TRUE);

/*** Explore the dungeon ***/

if ((borg_depth & DEPTH_VAULT)) {
	/* Chase close monsters */
	if (borg_flow_kill(FALSE, MAX_RANGE + 1))
		return (TRUE);

	/* Chase close objects */
	if (borg_flow_take(FALSE, 35))
		return (TRUE);
	if (borg_flow_vein(FALSE, 35))
		return (TRUE);

	/* Excavate a vault safely */
	if (borg_excavate_vault(MAX_RANGE - 2))
		return (TRUE);

	/* Find a vault to excavate */
	if (borg_flow_vault(35))
		return (TRUE);

	/* Explore close interesting grids */
	if (borg_flow_dark(TRUE))
		return (TRUE);
}

/* Chase old monsters */
if (borg_flow_kill(FALSE, 250))
	return (TRUE);

/* Chase old objects */
if (borg_flow_take(FALSE, 250))
	return (TRUE);
if (borg_flow_vein(FALSE, 250))
	return (TRUE);

/* Explore interesting grids */
if (borg_flow_dark(TRUE))
	return (TRUE);

/* Leave the level (if needed) */
if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
	 !borg_skill[BI_CDEPTH] && borg_skill[BI_LITE]) {
	/* Stay in town and scum for money after shopping */
} else {
	if (borg_leave_level(FALSE))
		return (TRUE);
}

/* Explore interesting grids */
if (borg_flow_dark(FALSE))
	return (TRUE);

/*** Deal with shops ***/

/* Flow to a different (favorite) town in order to take advantage of other
 * shops
if (borg_skill[BI_ENCUMBERD] < 150 &&
	 borg_flow_demenager(borg_town_pref, GOAL_TOWN))
	return (TRUE);
*/

/* Try to get to town location (town gate for now)
if (when_waypoint[0] == 0 && borg_flow_town_exit(GOAL_TOWN))
	return (TRUE);
*/

/* Specialty shops (Restoration, ID, *ID*) */
if (borg_flow_shop_special(TRUE))
	return (TRUE);

/* Hack -- visit all the shops */
if (borg_flow_shop_visit())
	return (TRUE);

/* Hack -- Visit the shops */
if (borg_choose_shop()) {
	/* Try and visit a shop, if so desired */
	if (borg_flow_shop_entry(goal_shop))
		return (TRUE);
}

/* Specialty shops (Recharge, Enchant weapon/armor/missiles) */
if (borg_flow_shop_special(FALSE))
	return (TRUE);

/*** Leave the Level ***/

/* Study/Test boring spells/prayers */
if (!goal_fleeing && borg_play_magic(TRUE))
	return (TRUE);

/* Search for secret doors */
if (borg_flow_spastic(FALSE))
	return (TRUE);

/* Recharge items before leaving the level */
if (borg_wear_recharge())
	return (TRUE);

/* Leave the level (if possible) */
if (borg_gold < borg_money_scum_amount && borg_money_scum_amount != 0 &&
	 !borg_skill[BI_CDEPTH] && borg_skill[BI_LITE]) {
	/* Stay in town, scum for money now that shopping is done. */
	borg_money_scum();

	/* Done */
	return (TRUE);
} else {
	if (borg_leave_level(TRUE))
		return (TRUE);
}

/* Search for secret doors */
if (borg_flow_spastic(TRUE))
	return (TRUE);

/*** Nothing to do ***/

/* Twitching in town can be fatal.  Really he should not become twitchy
 * but sometimes he cant recall to the dungeon and that may induce the
 * twitchy behavior.  So we reset the level if this happens.  That will
 * force him to go shopping all over again.
 */
if ((borg_skill[BI_CDEPTH] == 0 && borg_t - borg_began > 800 &&
	  when_waypoint[1] == 0) ||
	 borg_t > 28000)
	old_depth = 126;

/* Set a flag that the borg is  not allowed to retreat for 5 rounds */
borg_no_retreat = 5;

/* Boost slightly */
if (avoidance < borg_skill[BI_CURHP] * 2) {
	bool done = FALSE;

	/* Note */
	borg_note(format("# Boosting bravery (1) from %d to %d!", avoidance,
						  borg_skill[BI_CURHP] * 2));

	/* Hack -- ignore some danger */
	avoidance = (borg_skill[BI_CURHP] * 2);

	/* Forget the danger fields */
	borg_danger_wipe = TRUE;

	/* Try anything */
	if (borg_think_dungeon_brave())
		done = TRUE;

	/* Reset "avoidance" */
	avoidance = borg_skill[BI_CURHP];

	/* Re-calculate danger */
	borg_danger_wipe = TRUE;

	/* Forget goals */
	/*        goal = 0;*/

	/* Done */
	if (done)
		return (TRUE);
}

/* try phase before boosting bravery further and acting goofy */
borg_times_twitch++;

/* Phase to get out of being twitchy up to 3 times per level. */
if (borg_times_twitch < 3) {
	borg_note("# Considering Phase (twitchy)");

	/* Phase */
	if (amt_phase && borg_caution_phase(15, 2) &&
		 (borg_spell(REALM_SORCERY, 0, 1) || borg_spell(REALM_TRUMP, 0, 0) ||
		  borg_spell(REALM_ARCANE, 0, 4) ||
		  borg_mindcr_fail(MIND_MAJOR_DISP, 7, 35) ||
		  borg_mindcr_fail(MIND_MINOR_DISP, 3, 35) ||
		  borg_activate_artifact(ART_DRAEBOR, FALSE) ||
		  borg_activate_artifact(ART_NYNAULD, FALSE) ||
		  borg_read_scroll(SV_SCROLL_PHASE_DOOR))) {
		/* Success */
		return (TRUE);
	}
}

/* Set a flag that the borg is not allowed */
/*  to retreat for 10 rounds */
borg_no_retreat = 10;

/* Boost some more */
if (avoidance < borg_skill[BI_MAXHP] * 4) {
	bool done = FALSE;

	/* Note */
	borg_note(format("# Boosting bravery (2) from %d to %d!", avoidance,
						  borg_skill[BI_MAXHP] * 4));

	/* Hack -- ignore some danger */
	avoidance = (borg_skill[BI_MAXHP] * 4);

	/* Forget the danger fields */
	borg_danger_wipe = TRUE;

	/* Try anything */
	if (borg_think_dungeon_brave())
		done = TRUE;

	/* Reset "avoidance" */
	avoidance = borg_skill[BI_CURHP];

	/* Re-calculate danger */
	borg_danger_wipe = TRUE;

	/* Forget goals */
	/*        goal = 0;*/

	/* Done */
	if (done)
		return (TRUE);
}

/* Boost a lot */
if (avoidance < 30000) {
	bool done = FALSE;

	/* Note */
	borg_note(format("# Boosting bravery (3) from %d to %d!", avoidance, 30000));

	/* Hack -- ignore some danger */
	avoidance = 30000;

	/* Forget the danger fields */
	borg_danger_wipe = TRUE;

	/* Reset the spastic searching counts */
	for (y = 0; y < AUTO_MAX_Y; y++) {
		for (x = 0; x < AUTO_MAX_X; x++) {
			borg_grids[y][x].xtra = 0;
		}
	}

	/* Reset multiple factors to jumpstart the borg */
	unique_on_level = 0;
	borg_depth &= ~DEPTH_SCARY;

	/* reset our breeder flag */
	borg_depth &= ~DEPTH_BREEDER;

	/* Hack -- Clear "panel" flags */
	for (y = 0; y < 6; y++) {
		for (x = 0; x < 6; x++) {
			borg_detect_wall[y][x] = FALSE;
			borg_detect_trap[y][x] = FALSE;
			borg_detect_door[y][x] = FALSE;
			borg_detect_evil[y][x] = FALSE;
		}
	}

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

	/* Forget goals */
	goal = 0;

	/* use any stairs */
	borg_note("# Fleeing due to twitch.");
	stair_less = stair_more = goal_fleeing = goal_leaving = TRUE;

	/* Hack -- cannot rise past town */
	if (!borg_skill[BI_CDEPTH])
		goal_rising = FALSE;

	/* Assume not fleeing the level */
	goal_fleeing_lunal = FALSE;
	goal_fleeing_munchkin = FALSE;

	/* Assume not ignoring monsters */
	goal_ignoring = FALSE;

	/* No known glyph */
	track_glyph_num = 0;

	/* No known steps */
	track_step_num = 0;

	/* No known bad landing grids */
	track_land_num = 0;

	/* No known doors */
	track_door_num = 0;

	/* No known doors */
	track_closed_num = 0;

	/* No mineral veins */
	track_vein_num = 0;

	/* No objects here */
	borg_takes_cnt = 0;
	borg_takes_nxt = 1;

	/* Forget old objects */
	C_WIPE(borg_takes, 256, borg_take);

	/* Try anything */
	if (borg_think_dungeon_brave())
		done = TRUE;

	/* Reset "avoidance" */
	avoidance = borg_skill[BI_CURHP];

	/* Re-calculate danger */
	borg_danger_wipe = TRUE;

	/* Forget goals */
	/* goal = 0;*/

	/* Done */
	if (done)
		return (TRUE);
}

/* try teleporting before acting goofy */
borg_times_twitch++;

/* Teleport to get out of being twitchy up to 5 times per level. */
if (borg_times_twitch < 5) {
	borg_note("# Teleport (twitchy)");

	/* Teleport */
	if (borg_spell_fail(REALM_ARCANE, 2, 3, 45) ||
		 borg_spell_fail(REALM_TRUMP, 0, 4, 45) ||
		 borg_spell_fail(REALM_CHAOS, 0, 7, 45) ||
		 borg_spell_fail(REALM_SORCERY, 0, 5, 45) ||
		 borg_mindcr_fail(MIND_MAJOR_DISP, 7, 35) ||
		 borg_use_staff(SV_STAFF_TELEPORTATION) ||
		 borg_read_scroll(SV_SCROLL_TELEPORT) ||
		 borg_activate_activation(ACT_TELEPORT, FALSE)) {
		/* Success */
		return (TRUE);
	}
}

/* Recall to town */
if (borg_skill[BI_CDEPTH] && (borg_recall())) {
	/* Note */
	borg_note("# Recalling (twitchy)");

	/* Success */
	return (TRUE);
}
/* Recall to town */
if (borg_skill[BI_CDEPTH] && (borg_recall())) {
	/* Note */
	borg_note("# Recalling (twitchy)");

	/* Success */
	return (TRUE);
}

/* Reset the spastic searching counts */
for (y = 0; y < AUTO_MAX_Y; y++) {
	for (x = 0; x < AUTO_MAX_X; x++) {
		borg_grids[y][x].xtra = 0;
	}
}

/* Reset multiple factors to jumpstart the borg */
unique_on_level = 0;
borg_depth &= ~DEPTH_SCARY;

/* reset our breeder flag */
borg_depth &= ~DEPTH_BREEDER;

/* Hack -- Clear "panel" flags */
for (y = 0; y < 6; y++) {
	for (x = 0; x < 6; x++) {
		borg_detect_wall[y][x] = FALSE;
		borg_detect_trap[y][x] = FALSE;
		borg_detect_door[y][x] = FALSE;
		borg_detect_evil[y][x] = FALSE;
	}
}

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

/* No known doors */
track_closed_num = 0;

/* No known veins */
track_vein_num = 0;

/* No objects here */
borg_takes_cnt = 0;
borg_takes_nxt = 1;

/* Forget old objects */
C_WIPE(borg_takes, 256, borg_take);

/* No monsters here */
borg_kills_cnt = 0;
borg_kills_nxt = 1;

/* Forget old monsters */
C_WIPE(borg_kills, 256, borg_kill);

/* Hack -- Forget race counters */
C_WIPE(borg_race_count, max_r_idx, s16b);
borg_note("# Wiping monster (2)");

/* Attempt to dig to the center of the dungeon */
if (borg_flow_kill_direct(TRUE))
	return (TRUE);

/* Teleport to get out of being twitchy */
if (borg_times_twitch < 500) {
	borg_note("# Teleport (twitchy)");

	/* Teleport */
	if (borg_spell_fail(REALM_ARCANE, 2, 3, 45) ||
		 borg_spell_fail(REALM_TRUMP, 0, 4, 45) ||
		 borg_spell_fail(REALM_CHAOS, 0, 7, 45) ||
		 borg_spell_fail(REALM_SORCERY, 0, 5, 45) ||
		 borg_activate_activation(ACT_TELEPORT, FALSE) ||
		 borg_mindcr_fail(MIND_MAJOR_DISP, 7, 35) ||
		 borg_use_staff(SV_STAFF_TELEPORTATION) ||
		 borg_read_scroll(SV_SCROLL_TELEPORT)) {
		/* Success */
		return (TRUE);
	}
}

/* Twitch around */
if (borg_twitchy())
	return (TRUE);

/* Oops */
return (FALSE);
}

/*
 * Initialize this file
 */
void borg_init_8(void) { /* Nothing */
}

#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
