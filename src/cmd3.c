/* File: cmd3.c */

/* Purpose: Inventory commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 *
 *
 * James E. Wilson and Robert A. Koeneke released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version), 
 * or under the terms of the traditional Angband license. 
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2), 
 * or under the terms of the traditional Angband license. 
 */ 

#include "angband.h"

/*
* Display inventory
*/
void do_cmd_inven(void)
{
	char out_val[160];


	/* Note that we are in "inventory" mode */
	command_wrk = FALSE;


	/* Save the screen */
	Term_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the inventory */
	show_inven();

	/* Hack -- hide empty slots */
	item_tester_full = FALSE;

	/* Build a prompt */
	sprintf(out_val, "Inventory: carrying %d.%d pounds (%d%% of capacity). Command: ",
		total_weight / 10, total_weight % 10,
		(total_weight * 100) / ((adj_stat[p_ptr->stat_ind[A_STR]][ADJ_WEIGHT] * 100) / 2));

	/* Get a command */
	prt(out_val, 0, 0);

	/* Get a new command */
	command_new = inkey();

	/* Restore the screen */
	Term_load();

	/* Process "Escape" */
	if (command_new == ESCAPE)
	{
		/* Reset stuff */
		command_new = 0;
		command_gap = 50;
	}

	/* Process normal keys */
	else
	{
		/* Hack -- Use "display" mode */
		command_see = TRUE;
	}
}


/*
* Display equipment
*/
void do_cmd_equip(void)
{
	char out_val[160];


	/* Note that we are in "equipment" mode */
	command_wrk = TRUE;


	/* Save the screen */
	Term_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the equipment */
	show_equip();

	/* Hack -- undo the hack above */
	item_tester_full = FALSE;

	/* Build a prompt */
	sprintf(out_val, "Equipment: carrying %d.%d pounds (%d%% of capacity). Command: ",
		total_weight / 10, total_weight % 10,
		(total_weight * 100) / ((adj_stat[p_ptr->stat_ind[A_STR]][ADJ_WEIGHT] * 100) / 2));

	/* Get a command */
	prt(out_val, 0, 0);

	/* Get a new command */
	command_new = inkey();

	/* Restore the screen */
	Term_load();


	/* Process "Escape" */
	if (command_new == ESCAPE)
	{
		/* Reset stuff */
		command_new = 0;
		command_gap = 50;
	}

	/* Process normal keys */
	else
	{
		/* Enter "display" mode */
		command_see = TRUE;
	}
}


/*
* The "wearable" tester
*/
static bool item_tester_hook_wear(object_type *o_ptr)
{
	/* Check for a usable slot */
	if (wield_slot(o_ptr) >= INVEN_WIELD) return (TRUE);

	/* Assume not wearable */
	return (FALSE);
}


/*
* Wield or wear a single item from the pack or floor
*/
void do_cmd_wield(void)
{
	int item, slot;

	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr;

	cptr act;

	char o_name[80];


	/* Restrict the choices */
	item_tester_hook = item_tester_hook_wear;

	/* Get an item (from inven or floor) */
	if (!get_item(&item, "Wear/Wield which item? ","You have nothing you can wear or wield." , USE_INVEN | USE_FLOOR))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Check the slot */
	slot = wield_slot(o_ptr);

	/* If we are dealing with rings, we should ask the player which finger */
	if( slot == INVEN_LEFT || slot == INVEN_RIGHT )
	{
		char ch;
		char	o_name[80];
		object_type *ring_ptr; 
		bool not_escape;
		/* Separator */
		put_str( "                     " , 1 , 0 );
		/* Show left ring */
		ring_ptr = &inventory[INVEN_LEFT];
		object_desc(o_name, ring_ptr, TRUE, 3);
		o_name[79] = 0;
		
		put_str( "c) =               : " , 2 , 0 );
		put_str( mention_use(INVEN_LEFT) , 2 , 5 );
		c_put_str( object_attr( ring_ptr )            ,  "="   , 2 , 3 );
		c_put_str( tval_to_attr[ring_ptr->tval % 128] , o_name , 2 , 21 );

		/* Show right ring */
		ring_ptr = &inventory[INVEN_RIGHT];
		object_desc(o_name, ring_ptr, TRUE, 3);
		o_name[79] = 0;
		
		put_str( "d) =               : "  , 3 , 0 );
		put_str( mention_use(INVEN_RIGHT) , 3 , 5 );
		c_put_str( object_attr( ring_ptr )            ,  "="   , 3 , 3 );
		c_put_str( tval_to_attr[ring_ptr->tval % 128] , o_name , 3 , 21 );

		not_escape = get_com("Use left or right finger ? (l/r) ", &ch);
		do_cmd_redraw();
		if ( not_escape )
		{	
			slot = -1;
			if( ch == 'l' || ch == 'L' || ch == 'c' || ch == 'C' ) slot = INVEN_LEFT;
			if( ch == 'r' || ch == 'R' || ch == 'd' || ch == 'D'  ) slot = INVEN_RIGHT;
			if( slot == -1 ) return;
		}
		else
		{
			return;
		}
	}

	/* Prevent wielding into a cursed slot */
	if (cursed_p(&inventory[slot]))
	{
		/* Describe it */
		object_desc(o_name, &inventory[slot], FALSE, 0);

		/* Message */
		msg_format("The %s you are %s appears to be cursed.",
			o_name, describe_use(slot));

		/* Cancel the command */
		return;
	}



	if ((cursed_p(o_ptr)) && (wear_confirm)
		&& (object_known_p(o_ptr) || (o_ptr->ident & (IDENT_SENSE))))
	{
		char dummy[512];

		/* Describe it */
		object_desc(o_name, o_ptr, FALSE, 0);

		sprintf(dummy, "Really use the %s {cursed}? ", o_name);
		if (!(get_check(dummy)))
			return;
	}


	/* Take a turn */
	energy_use = 100;

	/* Get local object */
	q_ptr = &forge;

	/* Obtain local object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = 1;

	/* Modify charges for wands */
	if(o_ptr->tval==TV_WAND)
	{
		q_ptr->pval = o_ptr->pval / o_ptr->number; /* we only really throw 1 * q_ptr->number; */
	}	
	
	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_optimize(0 - item);
	}

	/* Access the wield slot */
	o_ptr = &inventory[slot];

	/* Take off existing item */
	if (o_ptr->k_idx)
	{
		/* Take off existing item */
		(void)inven_takeoff(slot, 255);
	}

	/* Wear the new stuff */
	object_copy(o_ptr, q_ptr);

	/* Increase the weight */
	total_weight += q_ptr->weight;

	/* Increment the equip counter by hand */
	equip_cnt++;

	/* Where is the item now */
	if (slot == INVEN_WIELD)
	{
		act = "You are wielding";
	}
	else if (slot == INVEN_BOW)
	{
		act = "You are shooting with";
	}
	else if (slot == INVEN_LITE)
	{
		act = "Your light source is";
	}
	else if (slot >= INVEN_POUCH_1)
	{
		act = "You have readied";
	}
	else
	{
		act = "You are wearing";
	}

	/* Describe the result */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(slot));

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		msg_print("Oops! It feels deathly cold!");

		/* Note the curse */
		o_ptr->ident |= (IDENT_SENSE);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
}



/*
* Take off an item
*/
void do_cmd_takeoff(void)
{
	int item;

	object_type *o_ptr;

	/* Get an item (from equip) */
	if (!get_item(&item, "Take off which item? ", "You are not wearing anything to take off.", USE_EQUIP))
	{
		return;
	}

	/* you cannot take off items from the floor */
	o_ptr = &inventory[item];

	/* Item is cursed */
	if (cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}

	/* Take a turn */
	energy_use = 100;

	/* Take off the item */
	(void)inven_takeoff(item, 255);
}


/*
* Drop an item
*/
void do_cmd_drop(void)
{
	int item, amt = 1;

	object_type *o_ptr;

	/* Get an item (from equip or inven) */
	if (!get_item(&item, "Drop which item? ", "You have nothing to drop.", USE_EQUIP | USE_INVEN))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Hack -- Cannot remove cursed items */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}


	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number,TRUE);

		/* Allow user abort */
		if (amt <= 0) return;
	}


	/* Take a partial turn */
	energy_use = 50;

	/* Drop (some of) the item */
	inven_drop(item, amt);

	p_ptr->window |= (PW_VISIBLE_ITEMS);
}


static bool high_level_book(object_type * o_ptr)
{
	if ((o_ptr->tval == TV_MIRACLES_BOOK) || 
		(o_ptr->tval == TV_SORCERY_BOOK) ||
		(o_ptr->tval == TV_NATURE_BOOK) || 
		(o_ptr->tval == TV_CHAOS_BOOK) ||
		(o_ptr->tval == TV_DEATH_BOOK) || 
		(o_ptr->tval == TV_TAROT_BOOK) ||
		(o_ptr->tval == TV_SOMATIC_BOOK) ||
        (o_ptr->tval == TV_DEMONIC_BOOK))
	{
		if (o_ptr->sval>1) return TRUE;
		else return FALSE;
	}
	return FALSE;
}


void do_cmd_destroy_all()
{
	int                 i,amt;
	char o_name[80];
	int count;

	count=0;
	/* Simply destroy every item */
	for (i = INVEN_TOTAL-1; i>=0; i--)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;
		/* Skip valuable objects */
		if(object_value(o_ptr)<1)
		{

			/* Describe the object */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Item is cursed */
			if (!cursed_p(o_ptr) || (i <= INVEN_PACK))
			{
				msg_format("You destroy %s.", o_name);
				count++;

				amt = o_ptr->number;
				/* Mega-Hack -- preserve artefacts */
				if (preserve_mode)
				{
					/* Hack -- Preserve unknown artefacts */
					if (artefact_p(o_ptr))
					{
						/* Mega-Hack -- Preserve the artefact */
						a_info[o_ptr->name1].cur_num = 0;
					}
				}
				/* Eliminate the item (from the pack) */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);
			}
		}
	}

	/* Take a turn */
	energy_use = 100;

	if(!count)
	{
		msg_print("You are carrying nothing worth destroying.");
		energy_use=0; /* Don't take a turn after all */
	}
}

/*
* Destroy an item
*/
void do_cmd_destroy(void)
{
	int  item, amt = 1;
	int  old_number;
	s16b old_charges;
	bool force = FALSE;

	object_type		*o_ptr;

	char o_name[80];
	char out_val[160];

	/* Hack -- force destruction */
	if (command_arg > 0) force = TRUE;

	/* Get an item (from inven or floor) */
	if (!get_item(&item, "Destroy which item? ", "You have nothing to destroy." , USE_INVEN | USE_FLOOR))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number,TRUE);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Describe the to be destroyed object */
	old_number = o_ptr->number;
	old_charges = o_ptr->pval;
	if( o_ptr->tval == TV_WAND ) o_ptr->pval = o_ptr->pval / o_ptr->number * amt;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, TRUE, 3);
	/* Reset the object */
	o_ptr->number = old_number;
	o_ptr->pval   = old_charges;

	/* Verify unless quantity given */
	if (!force)
	{
		if (!((auto_destroy) && (object_value(o_ptr)<1)))
		{
			/* Make a verification */
			sprintf(out_val, "Really destroy %s? ", o_name);
			if (!get_check(out_val)) return;
		}
	}

	/* Take a turn */
	energy_use = 100;

	/* Artifacts cannot be destroyed */
	if (artefact_p(o_ptr) || o_ptr->art_name)
	{
		cptr feel = "special";

		energy_use = 0;

		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		/* Hack -- Handle icky artefacts */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) feel = "terrible";

		/* Hack -- inscribe the artefact */
		o_ptr->note = quark_add(feel);

		/* We have "felt" it (again) */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
		return;
	}

	/* Message */
	msg_format("You destroy %s.", o_name);

	if (high_level_book(o_ptr))
	{
		bool gain_expr = FALSE;
		if (p_ptr->pclass == CLASS_WARRIOR) gain_expr = TRUE;
		else if (p_ptr->pclass == CLASS_PALADIN || 
			     p_ptr->pclass == CLASS_HELL_KNIGHT || 
				 p_ptr->pclass == CLASS_BLACK_KNIGHT ||
				 p_ptr->pclass == CLASS_CHAOS_KNIGHT )
		{
			if (p_ptr->realm1 == 1)
			{
				if (o_ptr->tval != TV_MIRACLES_BOOK) gain_expr = TRUE;
			}
			else
			{
				if (o_ptr->tval == TV_MIRACLES_BOOK) gain_expr = TRUE;
			}
		}

		if ((gain_expr) && (p_ptr->exp < PY_MAX_EXP))

		{
			s32b tester_exp = p_ptr->max_exp / 20;
			if (tester_exp > 10000) tester_exp = 10000;
			if (o_ptr->sval < 3) tester_exp /= 4;
			if (tester_exp<1) tester_exp = 1;

			msg_print("You feel more experienced.");
			gain_exp(tester_exp * amt);

		}
	}

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -amt);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}


/*
* Destroy whole pack (and equip)
* This routine will keep artefacts if 'preserve' is on.
* Dean Anderson
*/
void destroy_pack(void)
{
	int                 i,amt;

	/* Simply destroy every item */
	for (i = INVEN_TOTAL-1; i>=0; i--)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;
		amt = o_ptr->number;
		/* Mega-Hack -- preserve artefacts */
		if (preserve_mode)
		{
			/* Hack -- Preserve unknown artefacts */
			if (artefact_p(o_ptr))
			{
				/* Mega-Hack -- Preserve the artefact */
				a_info[o_ptr->name1].cur_num = 0;
			}
		}
		/* Eliminate the item (from the pack) */
		inven_item_increase(i, -amt);
		inven_item_optimize(i);
	}
}


/*
* Observe an item which has been *identify*-ed
*/
void do_cmd_observe(void)
{
	int			item;

	object_type		*o_ptr;

	char		o_name[80];
    
    /*object_kind *k_ptr; UNUSED*/

	/* Get an item (from equip or inven or floor) */
	if (!get_item(&item, "Examine which item? ", "You have nothing to examine.", USE_EQUIP | USE_INVEN | USE_FLOOR))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

    /*k_ptr = &k_info[ o_ptr->k_idx ]; UNUSED*/
    
	/* Require full knowledge, konijn
	if (  !(o_ptr->ident & (IDENT_MENTAL) || k_ptr->flags3 & (TR3_EASY_KNOW) )  )
	{
		msg_print("You have no special knowledge about that item.");
		return;
	}*/

	object_track( o_ptr , "You are examining" );

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	msg_format("Examining %s...", o_name);

	/* Describe it fully */
	if (!identify_fully_aux(o_ptr)) msg_print("You see nothing special.");
}



/*
* Remove the inscription from an object
* XXX Mention item (when done)?
*/
void do_cmd_uninscribe(void)
{
	int   item;

	object_type *o_ptr;


	/* Get an item (from equip or inven or floor) */
	if (!get_item(&item, "Un-inscribe which item? ", "You have nothing to un-inscribe." , USE_EQUIP | USE_FLOOR | USE_INVEN))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Nothing to remove */
	if (!o_ptr->note)
	{
		msg_print("That item had no inscription to remove.");
		return;
	}

	/* Message */
	msg_print("Inscription removed.");

	/* Remove the incription */
	o_ptr->note = 0;

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
* Inscribe an object with a comment
*/
void do_cmd_inscribe(void)
{
	int			item;

	object_type		*o_ptr;

	char		o_name[80];

	char		out_val[80];


	/* Get an item (from equip or inven or floor) */
	if (!get_item(&item, "Inscribe which item? ", "You have nothing to inscribe.", USE_FLOOR | USE_EQUIP | USE_INVEN))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe the activity */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("Inscribing %s.", o_name);
	msg_print(NULL);

	/* Start with nothing */
	strcpy(out_val, "");

	/* Use old inscription */
	if (o_ptr->note)
	{
		/* Start with the old inscription */
		strcpy(out_val, quark_str(o_ptr->note));
	}

	/* Get a new inscription (possibly empty) */
	if (get_string("Inscription: ", out_val, 80))
	{
		/* Save the inscription */
		o_ptr->note = quark_add(out_val);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
* An "item_tester_hook" for refilling lanterns
*/
static bool item_tester_refill_lantern(object_type *o_ptr)
{
	/* Flasks of oil are okay */
	if (o_ptr->tval == TV_FLASK) return (TRUE);

	/* Lanterns are okay */
	if ((o_ptr->tval == TV_LITE) &&
		(o_ptr->sval == SV_LITE_LANTERN)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
* Refill the players lamp (from the pack or floor)
*/
static void do_cmd_refill_lamp(int item)
{

	object_type *o_ptr;
	object_type *j_ptr;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_lantern;

	/* Get an item if we weren't passed one */
	if (item == -999)
	{
		/* Get an item (from inven or floor) */
		if (!get_item(&item, "Refill with which flask? ", "You have no flasks of oil." ,  USE_INVEN | USE_FLOOR))
		{
			return;
		}
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	item_tester_hook = item_tester_refill_lantern;
	if(!item_tester_okay(o_ptr))
	{
		msg_print("You can't refill a lantern from that!");
		item_tester_hook = 0;
		return;
	}
	item_tester_hook = 0;

	/* Take a partial turn */
	energy_use = 50;

	/* Access the lantern */
	j_ptr = &inventory[INVEN_LITE];

	/* Refuel */
	j_ptr->pval += o_ptr->pval;

	/* Message */
	msg_print("You fuel your lamp.");

	/* Comment */
	if (j_ptr->pval >= FUEL_LAMP)
	{
		j_ptr->pval = FUEL_LAMP;
		msg_print("Your lamp is full.");
	}

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
}



/*
* An "item_tester_hook" for refilling torches
*/
static bool item_tester_refill_torch(object_type *o_ptr)
{
	/* Torches are okay */
	if ((o_ptr->tval == TV_LITE) &&
		(o_ptr->sval == SV_LITE_TORCH)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
* Refuel the players torch (from the pack or floor)
*/
static void do_cmd_refill_torch(int item)
{

	object_type *o_ptr;
	object_type *j_ptr;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_torch;

	/* Get an item if we weren't passed one */
	if(item == -999)
	{
		/* Get an item (from inven or floor) */
		if (!get_item(&item, "Refuel with which torch? ", "You have no extra torches." , USE_INVEN | USE_FLOOR))
		{
			return;
		}
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	item_tester_hook = item_tester_refill_torch;
	if(!item_tester_okay(o_ptr))
	{
		msg_print("You can't refill a torch with that!");
		item_tester_hook = 0;
		return;
	}
	item_tester_hook = 0;

	/* Take a partial turn */
	energy_use = 50;

	/* Access the primary torch */
	j_ptr = &inventory[INVEN_LITE];

	/* Refuel */
	j_ptr->pval += o_ptr->pval + 5;

	/* Message */
	msg_print("You combine the torches.");

	/* Over-fuel message */
	if (j_ptr->pval >= FUEL_TORCH)
	{
		j_ptr->pval = FUEL_TORCH;
		msg_print("Your torch is fully fueled.");
	}

	/* Refuel message */
	else
	{
		msg_print("Your torch glows more brightly.");
	}

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
}




/*
* Refill the players lamp, or restock his torches
*/
void do_cmd_refill(int item)
{
	object_type *o_ptr;

	/* Get the light */
	o_ptr = &inventory[INVEN_LITE];

	/* It is nothing */
	if (o_ptr->tval != TV_LITE)
	{
		msg_print("You are not wielding a light.");
	}

	/* It's a lamp */
	else if (o_ptr->sval == SV_LITE_LANTERN)
	{
		do_cmd_refill_lamp(item);
	}

	/* It's a torch */
	else if (o_ptr->sval == SV_LITE_TORCH)
	{
		do_cmd_refill_torch(item);
	}

	/* No torch to refill */
	else
	{
		msg_print("Your light cannot be refilled.");
	}
}






/*
* Target command
*/
void do_cmd_target(void)
{
	/* Target set */
	if (target_set(TARGET_KILL))
	{
		msg_print("Target Selected.");
	}

	/* Target aborted */
	else
	{
		msg_print("Target Aborted.");
	}
}



/*
* Look command
*/
void do_cmd_look(void)
{
	/* Look around */
	if (target_set(TARGET_LOOK))
	{
		msg_print("Target Selected.");
	}
}



/*
* Allow the player to examine other sectors on the map
*/
void do_cmd_locate(void)
{
	int		dir, y1, x1, y2, x2;

	char	tmp_val[80];

	char	out_val[160];


	/* Start at current panel */
	y2 = y1 = panel_row;
	x2 = x1 = panel_col;

	/* Show panels until done */
	while (1)
	{
		/* Describe the location */
		if ((y2 == y1) && (x2 == x1))
		{
			tmp_val[0] = '\0';
		}
		else
		{
			sprintf(tmp_val, "%s%s of",
				((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
				((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
		}

		/* Prepare to ask which way to look */
		sprintf(out_val,
			"Map sector [%d,%d], which is%s your sector.  Direction?",
			y2, x2, tmp_val);

		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			char command;

			/* Get a command (or Cancel) */
			if (!get_com(out_val, &command)) break;

			/* Extract the action (if any) */
			dir = get_keymap_dir(command);

			/* Error */
			if (!dir) bell();
		}

		/* No direction */
		if (!dir) break;

		/* Apply the motion */
		y2 += ddy[dir];
		x2 += ddx[dir];

		/* Verify the row */
		if (y2 > max_panel_rows) y2 = max_panel_rows;
		else if (y2 < 0) y2 = 0;

		/* Verify the col */
		if (x2 > max_panel_cols) x2 = max_panel_cols;
		else if (x2 < 0) x2 = 0;

		/* Handle "changes" */
		if ((y2 != panel_row) || (x2 != panel_col))
		{
			/* Save the new panel info */
			panel_row = y2;
			panel_col = x2;

			/* Recalculate the boundaries */
			panel_bounds();

			/* Update stuff */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Handle stuff */
			handle_stuff();
		}
	}


	/* Recenter the map around the player */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff();
}






/*
* The table of "symbol info" -- each entry is a string of the form
* "X:desc" where "X" is the trigger, and "desc" is the "info".
*/
static cptr ident_info[] =
{
	" :A dark grid",
		"!:A potion (or oil)",
		"\":An amulet (or necklace)",
		"#:A wall (or secret door)",
		"$:Treasure (gold or gems)",
		"%:A vein (magma or quartz)",
		/* "&:unused", */
		"':An open door",
		"(:Soft armour",
		"):A shield",
		"*:A vein with treasure",
		"+:A closed door",
		",:Food (or mushroom patch)",
		"-:A wand (or rod)",
		".:Floor",
		"/:A polearm (Axe/Pike/etc)",
		/* "0:unused", */
		"1:Entrance to General Store",
		"2:Entrance to Armory",
		"3:Entrance to Weaponsmith",
		"4:Entrance to Temple",
		"5:Entrance to Alchemy shop",
		"6:Entrance to Magic store",
		"7:Entrance to Black Market",
		"8:Entrance to your home",
		/* "9:unused", */
		"::Rubble",
		";:A glyph of warding / explosive rune",
		"<:An up staircase",
		"=:A ring",
		">:A down staircase",
		"?:A scroll",
		"@:You or an impersonator",
		"A:Fallen Angel",
		"B:Bird",
		"C:Canine",
		"D:Ancient Dragon/Wyrm",
		"E:Elemental",
		"F:Frog",
		"G:Ghost",
		"H:Horror",
		"I:Insect",
		"J:Snake",
		"K:Killer Beetle",
		"L:Lich",
		"M:Multi-Headed Reptile",
		/* "N:unused", */
		"O:Ogre",
		"P:Giant Humanoid",
		"Q:Quylthulg (Pulsing Flesh Mound)",
		"R:Reptile/Amphibian",
		"S:Spider/Scorpion/Tick",
		"T:Troll",
		"U:Major Demon",
		"V:Vampire",
		"W:Wight/Wraith/etc",
		"X:Xorn/Xaren/etc",
		"Y:Yeti",
		"Z:Zephyr Hound",
		"[:Hard armour",
		"\\:A hafted weapon (mace/whip/etc)",
		"]:Misc. armour",
		"^:A trap",
		"_:A staff",
		/* "`:unused", */
		"a:Ant",
		"b:Bat",
		"c:Centipede",
		"d:Dragon",
		"e:Ectoplasm",
		"f:Feline",
		"g:Golem",
		"h:Hybrid",
		"i:Icky Thing",
		"j:Jelly",
		"k:Kobold",
		"l:Louse",
		"m:Mold",
		"n:Naga",
		"o:Orc",
		"p:Sinner",
		"q:Quadruped",
		"r:Rodent",
		"s:Skeleton",
		"t:Townsperson",
		"u:Minor Demon",
		"v:Vortex",
		"w:Worm/Worm-Mass",
		/* "x:unused", */
		"y:Yeek",
		"z:Zombie/Mummy",
		"{:A missile (arrow/bolt/shot)",
		"|:An edged weapon (sword/dagger/etc)",
		"}:A launcher (bow/crossbow/sling)",
		"~:A tool (or miscellaneous item)",
		NULL
};

bool ang_sort_comp_r_idx( u16b why , int w1 , int w2 )
{
   	int z1, z2;

	/* Sort by player kills */
	if (why >= 4)
	{
		/* Extract player kills */
		z1 = r_info[w1].r_pkills;
		z2 = r_info[w2].r_pkills;
        
		/* Compare player kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}
    
    
	/* Sort by total kills */
	if (why >= 3)
	{
		/* Extract total kills */
		z1 = r_info[w1].r_tkills;
		z2 = r_info[w2].r_tkills;
        
		/* Compare total kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}
    
    
	/* Sort by monster level */
	if (why >= 2)
	{
		/* Extract levels */
		z1 = r_info[w1].level;
		z2 = r_info[w2].level;
        
		/* Compare levels */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}
    
    
	/* Sort by monster experience */
	if (why >= 1)
	{
		/* Extract experience */
		z1 = r_info[w1].mexp;
		z2 = r_info[w2].mexp;
        
		/* Compare experience */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}
    
    
	/* Compare indexes */
	return (w1 <= w2);
}

/*
* Sorting hook -- Comp function -- see below
*
* We use "u" to point to array of monster indexes,
* and "v" to select the type of sorting to perform on "u".
*/
bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b *why = (u16b*)(v);

	int w1 = who[a];
	int w2 = who[b];

    return ang_sort_comp_r_idx( *why , w1 , w2 );
}

bool ang_sort_comp_visible_hook(vptr u, vptr v, int a, int b)
{
	monster_list_entry *who = (monster_list_entry*)(u);
    
	u16b *why = (u16b*)(v);
    
	int w1 = who[a].r_idx;
	int w2 = who[b].r_idx;
    
    return ang_sort_comp_r_idx( *why , w1 , w2 );
}

/*
* Sorting hook -- Swap function -- see below
*
* We use "u" to point to array of monster indexes,
* and "v" to select the type of sorting to perform.
*/
void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b holder;

	/* XXX XXX */
	v = v ? v : 0;

	/* Swap */
	holder = who[a];
	who[a] = who[b];
	who[b] = holder;
}

void ang_sort_swap_visible_hook(vptr u, vptr v, int a, int b)
{
	monster_list_entry *who = (monster_list_entry*)(u);
    
	monster_list_entry holder;
    
	/* XXX XXX */
	v = v ? v : 0;
    
	/* Swap */
	holder = who[a];
	who[a] = who[b];
	who[b] = holder;
}



/*
* Hack -- Display the "name" and "attr/chars" of a monster race
*/
static void roff_top(int r_idx)
{
	monster_race	*r_ptr = &r_info[r_idx];

	byte		a1, a2;
	char		c1, c2;


	/* Access the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Access the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;

	/* Hack -- fake monochrome */
	if (!use_colour) a1 = TERM_WHITE;
	if (!use_colour) a2 = TERM_WHITE;


	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		Term_addstr(-1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

	/* Append the "standard" attr/char info */
	Term_addstr(-1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr(-1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr(-1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	Term_addstr(-1, TERM_WHITE, "'):");
}


/*
* Identify a character, allow recall of monsters
*
* Several "special" responses recall "multiple" monsters:
*   ^A (all monsters)
*   ^U (all unique monsters)
*   ^N (all non-unique monsters)
*
* The responses may be sorted in several ways, see below.
*
* Note that the player ghosts are ignored. XXX XXX XXX
*/
void do_cmd_query_symbol(void)
{
	int		i, n, r_idx;
	char	sym, query;
	char	buf[128];

	bool	all = FALSE;
	bool	uniq = FALSE;
	bool	norm = FALSE;

	bool	recall = FALSE;

	u16b	why = 0;
	u16b	who[MAX_R_IDX];


	/* Get a character, or abort */
	if (!get_com("Enter character to be identified: ", &sym)) return;

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (sym == KTRL('A'))
	{
		all = TRUE;
		strcpy(buf, "Full monster list.");
	}
	else if (sym == KTRL('U'))
	{
		all = uniq = TRUE;
		strcpy(buf, "Unique monster list.");
	}
	else if (sym == KTRL('N'))
	{
		all = norm = TRUE;
		strcpy(buf, "Non-unique monster list.");
	}
	else if (ident_info[i])
	{
		sprintf(buf, "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		sprintf(buf, "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	prt(buf, 0, 0);


	/* Collect matching monsters */
	for (n = 0, i = 1; i < MAX_R_IDX-1; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Nothing to recall */
		if (!debug_know && !r_ptr->r_sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Require unique monsters if needed */
		if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n) return;


	/* Prompt XXX XXX XXX */
	put_str("Recall details? (k/p/y/n): ", 0, 40);

	/* Query */
	query = inkey();

	/* Restore */
	prt(buf, 0, 0);


	/* Sort by kills (and level) */
	if (query == 'k')
	{
		why = 4;
		query = 'y';
	}

	/* Sort by level */
	if (query == 'p')
	{
		why = 2;
		query = 'y';
	}

	/* Catch "escape" */
	if (query != 'y') return;


	/* Sort if needed */
	if (why)
	{
		/* Select the sort method */
		ang_sort_comp = ang_sort_comp_hook;
		ang_sort_swap = ang_sort_swap_hook;

		/* Sort the array */
		ang_sort(who, &why, n);
	}


	/* Start at the end */
	i = n - 1;

	/* Scan the monster memory */
	while (1)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save the screen */
				Term_save();

				/* Recall on screen */
				screen_roff(who[i]);

				/* Hack -- Complete the prompt (again) */
				Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Restore */
				Term_load();
			}

			/* Normal commands */
			if (query != 'r') break;

			/* Toggle recall */
			recall = !recall;
		}

		/* Stop scanning */
		if (query == ESCAPE) break;

		/* Move to "prev" monster */
		if (query == '-')
		{
			if (++i == n)
			{
				i = 0;
				if (!expand_list) break;
			}
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
			{
				i = n - 1;
				if (!expand_list) break;
			}
		}
	}


	/* Re-display the identity */
	prt(buf, 0, 0);
}

/* 'Handle' an object, doing whatever seems the sensible thing to it... */
void do_cmd_handle(void)
{
	int item;

	object_type *o_ptr;

	/* Get an item (from equip or inven) */
	if (!get_item(&item, "Use which item? ", "You have nothing to use.", USE_EQUIP | USE_INVEN | USE_FLOOR ))
	{
		return;
	}

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}
	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* First test Wielded items */
	if ((item >= INVEN_WIELD) && (item < INVEN_POUCH_1))
	{
		/* Try to activate the wielded item, whatever it is */
		do_cmd_activate(item);
		return;
	}
	/* The item is in our inventory or in a pouch*/
	switch(o_ptr->tval)
	{
	case TV_STAFF:
		{
			do_cmd_use_staff(item);
			break;
		}
	case TV_WAND:
		{
			do_cmd_aim_wand(item);
			break;
		}
	case TV_ROD:
		{
			do_cmd_zap_rod(item);
			break;
		}
	case TV_SCROLL:
		{
			do_cmd_read_scroll(item);
			break;
		}
	case TV_POTION:
		{
			do_cmd_quaff_potion(item);
			break;
		}
	case TV_FLASK:case TV_LITE:
		{
			do_cmd_refill(item);
			break;
		}
	case TV_FOOD:
		{
			do_cmd_eat_food(item);
			break;
		}
	case TV_MIRACLES_BOOK:case TV_SORCERY_BOOK:
	case TV_NATURE_BOOK:case TV_CHAOS_BOOK:
	case TV_DEATH_BOOK:case TV_TAROT_BOOK:
	case TV_CHARMS_BOOK:case TV_SOMATIC_BOOK:case TV_DEMONIC_BOOK:
		{
			do_cmd_browse(item);
			break;
		}
	default:
		{
			item_tester_hook=item_tester_hook_wear;
			if(item_tester_okay(o_ptr))
			{
				msg_print("That item must be wielded to be used.");
			}
			else
			{
				msg_print("That item can't be used directly.");
			}
			item_tester_hook=0;
			return;
		}
	}
}

void do_cmd_magic_spell(void)
{
	if (p_ptr->anti_magic)
	{
		cptr which_power = "magic";
		if (p_ptr->pclass == CLASS_ORPHIC)
			which_power = "psionic powers";
		else if (mp_ptr->spell_book == TV_MIRACLES_BOOK)
			which_power = "prayer";

		msg_format("An anti-magic shell disrupts your %s!", which_power);

		energy_use = 5; /* Still use a bit */
	}
	else
	{
		if (p_ptr->pclass == CLASS_ORPHIC)
			do_cmd_mindcraft();
		else
			do_cmd_cast();
	}
}

void do_cmd_gain_spell(void)
{
	/* Require spell ability */
	if (p_ptr->realm1 == 0)
	{
		msg_print("You cannot cast spells!");
	}
	else
	{
		msg_format("You need some peace and quiet to research.");
		msg_format("Why not try a bookstore?");
	}
}

void do_cmd_screen_dump(void)
{
	do_cmd_load_screen( ANGBAND_DIR_PREF ,  "dump.txt" );
	(void)msg_flush_wait();
	(void)restore_screen();
}

void do_cmd_time(void)
{
	/* dummy is % thru day/night */
	bool morning = FALSE;
	int  dummy   = ((turn % ((10L * TOWN_DAWN)/2) * 100) / ((10L * TOWN_DAWN)/2));
	int  minute  = ((turn % ((10L * TOWN_DAWN)/2) * 720) / ((10L * TOWN_DAWN)/2)) % 60;
	int  hour    = ((12 * dummy) / 100) - 6;    /* -6 to +6 */
	int  hour12  = 0;
	/*int  day     = 0
	if (turn <= (10L * TOWN_DAWN)/4)
		day = 1;
	else
		day = (turn - (10L * TOWN_DAWN / 4)) / (10L * TOWN_DAWN) + 1;
	UNUSED
	*/

	/* night: 6pm -- 6am */
	if ((turn / ((10L * TOWN_DAWN)/2)) % 2)
	{   
		hour12 = (hour <= 0)?(12 - (hour * -1)):(hour);
		morning = (hour >= 0);

		msg_format("%d:%02d %s, day %d.", hour12, minute, (morning? "AM" : "PM"), turn / (10L * TOWN_DAWN) + 1);

		if      (dummy < 5)                     msg_print("The sun has set.");
		else if (dummy == 50)			        msg_print("It is midnight.");
		else if ((dummy > 94) && (dummy < 101)) msg_print("The sun is near to rising.");
		else if ((dummy > 75) && (dummy < 95))  msg_print("It is early morning, but still dark.");
		else if (dummy > 100                 )  msg_format("What a funny night-time! (%d)", dummy);
		else                                    msg_format("It is night.");
	} 
	else /* day */
	{  
		hour12 = (hour <= 0)?(12 - (hour * -1)):(hour);
		morning =  (hour >= 0);

		msg_format("%d:%02d %s, day %d.", hour12, minute, (morning? "AM" : "PM"), turn / (10L * TOWN_DAWN) + 1);
		if      (dummy <  5  ) msg_print("Morning has broken...");
		else if (dummy <  25 ) msg_print("It is early morning.");
		else if (dummy <  50 ) msg_print("It is late morning.");
		else if (dummy == 50 ) msg_print("It is noon.");
		else if (dummy <  65 ) msg_print("It is early afternoon.");
		else if (dummy <  85 ) msg_print("It is late afternoon.");
		else if (dummy <  95 ) msg_print("It is early evening.");
		else if (dummy <  101) msg_print("The sun is setting.");
		else                  msg_format("What a strange daytime! (%d)", dummy);
	}
}

