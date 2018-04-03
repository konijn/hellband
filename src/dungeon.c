/* File: dungeon.c */

/* Purpose: Angband game engine */

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

#define TY_CURSE_CHANCE 100
#define CHAINSWORD_NOISE 100


/*
* Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
*/
cptr value_check_aux1(object_type *o_ptr)
{

	/* Artifacts */
	if (artefact_p(o_ptr) || o_ptr->art_name)
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "terrible";

		/* Normal */
		return "special";
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "worthless";

		/* Normal */
		return "excellent";
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items */
	if (broken_p(o_ptr)) return "broken";

	/* Good "armour" bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/*If the shopkeeper doesnt like it, it's worthless ;)*/
	if( object_value(o_ptr) <= 0 )
		return "worthless";
	else if ( object_value(o_ptr) > GREAT_PRICE )
		return "great";
	else
		return "average";
}


/*
* Return a "feeling" (or NULL) about an item.  Method 2 (Light).
*/
cptr value_check_aux2(object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return "broken";

	/* Artifacts -- except cursed/broken ones */
	if (artefact_p(o_ptr) || o_ptr->art_name) return "good";

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return "good";

	/* Good armour bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/* No feeling */
	return (NULL);
}

bool has_heavy_pseudo_id()
{
	if(p_ptr->psign == SIGN_PLUTUS)
		return TRUE;
	switch (p_ptr->pclass)
	{
		case CLASS_BLACK_KNIGHT:
		case CLASS_CHAOS_KNIGHT:
		case CLASS_HELL_KNIGHT:
		case CLASS_PALADIN:
		case CLASS_WARRIOR:
		case CLASS_RANGER:
		case CLASS_ROGUE:
			return( TRUE );
		default:
			return (FALSE);
	}
}

/*
* Sense the inventory
*
*   Class 0 = Warrior --> fast and heavy
*   Class 1 = Mage    --> slow and light
*   Class 2 = Priest  --> fast but light
*   Class 3 = Rogue   --> okay and heavy
*   Class 4 = Ranger  --> slow but heavy  (changed!)
*   Class 5 = Paladin --> slow but heavy
*/
static void sense_inventory(void)
{
	int		i;

	int		plev = p_ptr->lev;

	bool	heavy = FALSE;

	cptr	feel;

	object_type *o_ptr;

	char o_name[80];


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (p_ptr->confused) return;

	heavy = has_heavy_pseudo_id();

	/* Analyze the class */
	switch (p_ptr->pclass)
	{
	case CLASS_WARRIOR:
		{
			/* Good sensing */
			if (0 != rand_int(9000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_MAGE: case CLASS_HIGH_MAGE: case CLASS_WARLOCK: case CLASS_BLOOD_MAGE:
		{
			/* Very bad (light) sensing */
			if (0 != rand_int(240000L / (plev + 5))) return;

			/* Done */
			break;
		}

	case CLASS_PRIEST: case CLASS_DRUID:
		{
			/* Good (light) sensing */
			if (0 != rand_int(10000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_ROGUE:
		{
			/* Okay sensing */
			if (0 != rand_int(20000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_RANGER:
		{
			/* Bad sensing */
			if (0 != rand_int(95000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_PALADIN:
		{
			/* Bad sensing */
			if (0 != rand_int(77777L / (plev * plev + 40))) return;

			/* Done */
			break;
		}
	case CLASS_WARRIOR_MAGE:
		{
			/* Bad sensing */
			if (0 != rand_int(75000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_ORPHIC:
		{
			/* Bad sensing */
			if (0 != rand_int(55000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_HELL_KNIGHT:case CLASS_CHAOS_KNIGHT: case CLASS_BLACK_KNIGHT:
		{
			/* Bad sensing */
			if (0 != rand_int(80000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

	case CLASS_MYSTIC:
		{
			/* Okay sensing */
			if (0 != rand_int(20000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}


	}


	/*** Sense everything ***/

	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool okay = FALSE;

		o_ptr = &inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
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
			{
				okay = TRUE;
				break;
			}
			case TV_LITE: /* Only orbs */
			{
				if(o_ptr->sval == SV_LITE_ORB)
				{
					okay = TRUE;
				}
				break;
			}
			default:
			{
				if( !object_aware_p(o_ptr) && heavy )
					okay = TRUE;
			}
		}

		/* Skip non-sense machines */
		if (!okay) continue;

		/* We know about it already, do not tell us again */
		if (o_ptr->ident & (IDENT_SENSE)) continue;

		/* It is fully known, no information needed */
		if (object_known_p(o_ptr)) continue;

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

		/* Check for a feeling */
		feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

		/* We have "felt" it , we should set this before considering to squelch */
		o_ptr->ident |= (IDENT_SENSE);

		/*Should we be squelchin' ?*/
		consider_squelch( o_ptr );

		/* Skip non-feelings */
		if (!feel) continue;

		/* Stop everything */
		if (disturb_minor) disturb(0, 0);

		/* Get an object description */
		object_desc(o_name, o_ptr, FALSE, 0);

		/* Message (equipment) */
		if (i >= INVEN_WIELD)
		{
			msg_format("You feel the %s (%c) you are %s %s %s...",
				o_name, index_to_label(i), describe_use(i),
				((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* Message (inventory) */
		else
		{
			msg_format("You feel the %s (%c) in your pack %s %s...",
				o_name, index_to_label(i),
				((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* Inscribe it textually */
		if (!o_ptr->note) o_ptr->note = quark_add(feel);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}

/*
* Regenerate hit points				-RAK-
*/
static void regenhp(int percent)
{
	s32b        new_chp, new_chp_frac;
	int                   old_chp;

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
	p_ptr->chp += (short)(new_chp >> 16);   /* div 65536 */

	/* check for overflow */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		p_ptr->chp_frac = (short)(new_chp_frac - 0x10000L);
		p_ptr->chp++;
	}
	else
	{
		p_ptr->chp_frac = (short)new_chp_frac;
	}

	/* Fully healed */
	if (p_ptr->chp >= p_ptr->mhp)
	{
		p_ptr->chp = p_ptr->mhp;
		p_ptr->chp_frac = 0;
	}

	/* Notice changes */
	if (old_chp != p_ptr->chp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
* Regenerate mana points				-RAK-
*/
static void regenmana(int percent)
{
	s32b        new_mana, new_mana_frac;
	int                   old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += (short)(new_mana >> 16);	/* div 65536 */
	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = (short)(new_mana_frac - 0x10000L);
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = (short)new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->window |= (PW_PLAYER);
	}
}






/*
* Regenerate the monsters (once per 100 game turns)
*
* XXX XXX XXX Should probably be done during monster turns.
*/
static void regen_monsters(void)
{
	int i, frac;


	/* Regenerate everyone */
	for (i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];



		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags2 & (RF2_REGENERATE)) frac *= 2;


			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (health_who == i) p_ptr->redraw |= (PR_HEALTH);
		}
	}
}

/*
 * Centralized Recall effect
 */
void do_recall( cptr activation )
{

	if (dun_level && (p_ptr->max_dun_level > dun_level))
	{
		if (get_check("Reset recall depth? "))
		p_ptr->max_dun_level = dun_level;
	}

	if (p_ptr->word_recall == 0)
	{
		if( activation != NULL )
			msg_print(activation);
		p_ptr->word_recall = rand_s16b(21) + 15;
		msg_print("The air about you becomes charged...");
	}
	else
	{
		p_ptr->word_recall = 0;
		msg_print("A tension leaves the air around you...");
	}
}



/*
* Forcibly pseudo-identify an object in the inventory
* (or on the floor)
*
* note: currently this function allows pseudo-id of any object,
* including silly ones like potions & scrolls, which always
* get '{average}'. This should be changed, either to stop such
* items from being pseudo-id'd, or to allow psychometry to
* detect whether the unidentified potion/scroll/etc is
* good (Cure Light Wounds, Restore Strength, etc) or
* bad (Poison, Weakness etc) or 'useless' (Slime Mold Juice, etc).
*/
bool psychometry(void)
{
	int                     item;

	object_type             *o_ptr;

	char            o_name[80];
	cptr            feel;

	/* Get an item (from equip or inven or floor) */
	if (!get_item(&item, "Meditate on which item? ", "You have nothing appropriate." , USE_EQUIP | USE_INVEN | USE_FLOOR))
	{
		return (FALSE);
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

	/* It is fully known, no information needed */
	if ((object_known_p(o_ptr)) || (o_ptr->ident & IDENT_SENSE))  {
		msg_print("You cannot find out anything more about that.");
		return TRUE;
	}

	/* Check for a feeling */
	feel = value_check_aux1(o_ptr);

	/* Get an object description */
	object_desc(o_name, o_ptr, FALSE, 0);

	/* Skip non-feelings */
	if (!feel) {
		msg_format("You do not perceive anything unusual about the %s.",
			o_name);
		return TRUE;
	}

	msg_format("You feel that the %s %s %s...",
		o_name, ((o_ptr->number == 1) ? "is" : "are"), feel);

	/* We have "felt" it */
	o_ptr->ident |= (IDENT_SENSE);

	/* Inscribe it textually */
	if (!o_ptr->note) o_ptr->note = quark_add(feel);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Something happened */
	return (TRUE);
}




/*
* Handle certain things once every 10 game turns
*/
static void process_world(void)
{
	int		x, y, i, j;

	int		regen_amount;
	bool    cave_no_regen = FALSE;
	int     upkeep_factor = 0;
	s32b day;

	cave_type		*c_ptr;

	object_type		*o_ptr;
	u32b            f1 = 0 , f2 = 0 , f3 = 0;

	int timed_counter;

	/* Every 10 game turns */
	if (turn % 10) return;

	/*** Check the Time and Load ***/

	if (!(turn % 1000))
	{
		/* Check time and load */
		if ((0 != check_time()) || (0 != check_load()))
		{
			/* Warning */
			if (closing_flag <= 2)
			{
				/* Disturb */
				disturb(0, 0);

				/* Count warnings */
				closing_flag++;

				/* Message */
				msg_print("The Gates of Slumber are closing...");
				msg_print("Please finish up and/or save your game.");
			}

			/* Slam the gate */
			else
			{
				/* Message */
				msg_print("The Gates of Slumber are now closed.");

				/* Stop playing */
				alive = FALSE;
			}
		}
	}

	/*** Attempt timed autosave ***/
	if (autosave_t && autosave_freq)
	{
		if (!(turn % ((s32b) autosave_freq * 10 )))
		{
			is_autosave = TRUE;
			msg_print("Autosaving the game...");
			do_cmd_save_game();
			is_autosave = FALSE;
		}
	}

	/* Handle special dates */
	if ((turn % (10L* TOWN_DAWN)) == ((10L*TOWN_DAWN)/4))
	{
		/* It's Noon */
		/* Work out day */
		if (turn <= 3*(10L * TOWN_DAWN)/4)
		{
			day = 1;
		}
		else
		{
			day = (turn - 3*(10L * TOWN_DAWN / 4)) / (10L * TOWN_DAWN) + 2;
		}
		day += p_ptr->startdate;

		if (p_ptr->birthday == (day % 365))
		{
			/* It's your birthday... */
			msg_print("Happy Birthday!");
			acquirement(py, px, randint(2) + 1, TRUE);
			p_ptr->age++;
		}
	}
	if ((turn % (10L* TOWN_DAWN)) == ((30L*TOWN_DAWN)/4))
	{
		/* It's Midnight */
		/* Work out day */
		if (turn <= 3*(10L * TOWN_DAWN)/4)
		{
			day = 1;
		}
		else
		{
			day = (turn - 3*(10L * TOWN_DAWN / 4)) / (10L * TOWN_DAWN) + 2;
		}
		day += p_ptr->startdate;

		/* Specials on some nights of the year */
		switch (day % 365)
		{
		case 0: /* January 1st */
			{
				msg_print("Happy New Year!");
				acquirement(py,px,randint(2)+1,FALSE);
				break;
			}
		case 110: /* April 20 */
		{
			msg_print("Happy Konijn Day!");
			acquirement(py,px,randint(2)+1,FALSE);
			break;
		}
		case 152: /* Dante's birthday 1 july*/
		{
			msg_print("When Dis celebrates, the devils come out to play...");
			summon_specific(py,px,dun_level,FILTER_DEVIL);
			break;
		}
		case 303: /* November 1st (Night of October 31st) */
			{
				msg_print("All Hallows Eve and the ghouls come out to play...");
				summon_specific(py,px,dun_level,FILTER_UNDEAD);
				break;
			}
		default: /* Any other night */
			{
				break;
			}
		}
	}


	/*** Handle the "town" (stores and sunshine) ***/

	/* While in town */
	if (dun_level <= 0)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if (!(turn % ((10L * TOWN_DAWN) / 2)))
		{
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn % (10L * TOWN_DAWN)));

			/* Day breaks */
			if (dawn)
			{
				/* Message */
				msg_print("The sun has risen.");

				/* Hack -- Scan the town */
				for (y = 0; y < cur_hgt; y++)
				{
					for (x = 0; x < cur_wid; x++)
					{
						/* Get the cave grid */
						c_ptr = &cave[y][x];

						/* Assume lit */
						c_ptr->info |= (CAVE_GLOW);

						/* Hack -- Memorize lit grids if allowed */
						if (view_perma_grids) c_ptr->info |= (CAVE_MARK);

						/* Hack -- Notice spot */
						note_spot(y, x);
					}
				}
			}

			/* Night falls */
			else
			{
				/* Message */
				msg_print("The sun has fallen.");

				/* Hack -- Scan the town */
				for (y = 0; y < cur_hgt; y++)
				{
					for (x = 0; x < cur_wid; x++)
					{
						/* Get the cave grid */
						c_ptr = &cave[y][x];

						/* Darken "boring" features */
						if (c_ptr->feat <= FEAT_INVIS)
						{
							/* Forget the grid */
							c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

							/* Hack -- Notice spot */
							note_spot(y, x);
						}
					}
				}
			}

			/* Update the monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
		}
	}


	/* While in the dungeon */
	else
	{
		/*** Update the Stores ***/

		/* Update the stores once a day (while in dungeon) */
		if (!(turn % (10L * STORE_TURNS)))
		{
			/* Message */
			if (debug_xtra) msg_print("Updating Shops...");

			/* Maintain each shop once*/
			store_maint_all(1);

			/* Sometimes, shuffle the shop-keepers */
			if (rand_int(STORE_SHUFFLE) == 0)
			{
				/* Message */
				if (debug_xtra) msg_print("Shuffling a Shopkeeper...");

				/* Shuffle a random shop */
				store_shuffle(rand_int(MAX_STORES));
			}

			/* Message */
			if (debug_xtra) msg_print("Done.");
		}
	}


	/*** Process the monsters ***/

	/* Check for creature generation */
	if (rand_int(MAX_M_ALLOC_CHANCE) == 0)
	{
		/* Make a new monster */
		(void)alloc_monster(MAX_SIGHT + 5, FALSE);
	}

	/* Hack -- Check for creature regeneration */
	if (!(turn % 100)) regen_monsters();


	/*** Damage over Time ***/

	/* Take damage from poison */
	if ((p_ptr->poisoned) && !(p_ptr->invuln))
	{
		/* Take damage */
		take_hit(1, "poison");
	}

	/* (Vampires) Take damage from sunlight */
	if (rp_ptr->hates_light)

	{
		if ((dun_level <= 0)
			&& (!(p_ptr->resist_lite)) && !(p_ptr->invuln)
			&& (!((turn / ((10L * TOWN_DAWN)/2)) % 2)))
		{
			if (cave[py][px].info & (CAVE_GLOW))
			{
				/* Take damage */
				msg_print("The sun's rays scorch your undead flesh!");
				take_hit(1, "sunlight");
				cave_no_regen = TRUE;
			}
		}

		if ((inventory[INVEN_LITE].tval)
			&& (inventory[INVEN_LITE].sval >= SV_LITE_BEATRICE)
			&& (inventory[INVEN_LITE].sval < SV_LITE_INSIGHT)
			&& !(p_ptr->resist_lite))
		{
			object_type * o_ptr = &inventory[INVEN_LITE];
			char o_name [80];
			char ouch [80];

			/* Get an object description */
			object_desc(o_name, o_ptr, FALSE, 0);


			msg_format("The %s scorches your undead flesh!", o_name);

			cave_no_regen = TRUE;

			/* Get an object description */
			object_desc(o_name, o_ptr, TRUE, 0);

			sprintf(ouch, "wielding %s", o_name);
			if (!(p_ptr->invuln))
				take_hit(1, ouch);
		}
	}

	/* Spectres -- take damage when moving through walls */
	/* Added: ANYBODY takes damage if inside through walls
	without wraith form -- NOTE: Spectres will never be
	reduced below 0 hp by being inside a stone wall; others
	WILL BE!  */

	if ((!cave_floor_bold(py, px)) && (cave[py][px].feat != FEAT_BUSH))  {

		cave_no_regen = TRUE;
		if (!(p_ptr->invuln)
			&& !(p_ptr->wraith_form) && ((p_ptr->chp > ((p_ptr->lev)/5))
			|| (p_ptr->prace != SPECTRE)))
		{
			cptr dam_desc;
			if (p_ptr->prace == SPECTRE)
			{
				msg_print("Your body feels disrupted!");
				dam_desc = "density";

			}
			else
			{
				msg_print("You are being crushed!");
				dam_desc = "solid rock";
			}

			take_hit(1 + ((p_ptr->lev)/5), dam_desc);
		}

	}


	/* Take damage from cuts */
	if ((p_ptr->cut) && !(p_ptr->invuln))
	{
		/* Mortal wound or Deep Gash */
		if (p_ptr->cut > 200)
		{
			i = 3;
		}

		/* Severe cut */
		else if (p_ptr->cut > 100)
		{
			i = 2;
		}

		/* Other cuts */
		else
		{
			i = 1;
		}

		/* Take damage */
		take_hit(i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if (!(turn % 100))
		{
			/* Basic digestion rate no longer based on speed */
			i = 20;

			/* Regeneration takes more food */
			if (p_ptr->regenerate) i += 30;

			/* Slow digestion takes less food */
			if (p_ptr->slow_digest) i -= 10;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			(void)set_food(p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		(void)set_food(p_ptr->food - 100);
	}

	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		if (!(p_ptr->invuln))
			take_hit(i, "starvation");
	}

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Getting Weak */
	if (p_ptr->food < PY_FOOD_WEAK)
	{
		/* Lower regeneration */
		if (p_ptr->food < PY_FOOD_STARVE)
		{
			regen_amount = 0;
		}
		else if (p_ptr->food < PY_FOOD_FAINT)
		{
			regen_amount = PY_REGEN_FAINT;
		}
		else
		{
			regen_amount = PY_REGEN_WEAK;
		}

		/* Getting Faint */
		if (p_ptr->food < PY_FOOD_FAINT)
		{
			/* Faint occasionally */
			if (!p_ptr->paralyzed && (rand_int(100) < 10))
			{
				/* Message */
				msg_print("You faint from the lack of food.");
				disturb(1, 0);

				/* Hack -- faint (bypass free action) */
				(void)set_timed_effect( TIMED_PARALYZED , p_ptr->paralyzed + 1 + rand_int(5));
			}
		}
	}

	/* Regeneration ability */
	if (p_ptr->regenerate)
	{
		regen_amount = regen_amount * 2;
	}

	/* Searching or Resting */
	if (p_ptr->searching || resting)
	{
		regen_amount = regen_amount * 2;
	}

	if (total_friends)
	{
		int upkeep_divider = 20;

		if (p_ptr->pclass == CLASS_MAGE)
			upkeep_divider = 15;
		else if (p_ptr->pclass == CLASS_BLOOD_MAGE)
			upkeep_divider = 12;
		else if (p_ptr->pclass == CLASS_HIGH_MAGE)
			upkeep_divider = 12;

#ifdef TRACK_FRIENDS
		if (debug_mode)
			msg_format("Total friends: %d.", total_friends);
#endif
		if (total_friends > 1 + (p_ptr->lev / (upkeep_divider)))
		{
			upkeep_factor = (total_friend_levels);

			if (upkeep_factor > 100) upkeep_factor = 100;
			else if (upkeep_factor < 10) upkeep_factor = 10;

#ifdef TRACK_FRIENDS
			if (debug_mode)
				msg_format("Levels %d, upkeep %d", total_friend_levels, upkeep_factor);
#endif
		}
	}

	/* Regenerate the mana */
	if (p_ptr->csp < p_ptr->msp)
	{
		if (upkeep_factor)
		{
			s16b upkeep_regen = (((100 - upkeep_factor) * regen_amount) / 100);
			regenmana(upkeep_regen);

#ifdef TRACK_FRIENDS
			if (debug_mode)
				msg_format("Regen: %d/%d", upkeep_regen,
				regen_amount);
#endif
		}
		else
		{
			regenmana(regen_amount);
		}
	}

	/* Poisoned or cut yields no healing */
	if (p_ptr->poisoned) regen_amount = 0;
	if (p_ptr->cut) regen_amount = 0;

	/* Special floor -- Pattern, in a wall -- yields no healing */
	if (cave_no_regen) regen_amount = 0;

	/* Regenerate Hit Points if needed */
	if ((p_ptr->chp < p_ptr->mhp) && !(cave_no_regen))
	{
		regenhp(regen_amount);
	}

	/*** Timeout Various Things ***/

	if (multi_rew)
		multi_rew = FALSE;

	/* Count down all timed effects */
	for( timed_counter = 0 ; timed_counter < TIMED_COUNT ; timed_counter++ )
	{
		 if( *(timed[timed_counter].timer) )
			(void)set_timed_effect( timed_counter , *(timed[timed_counter].timer) - 1);
	}

	/*** Poison and Stun and Cut ***/

	/* Poison */
	if (p_ptr->poisoned)
	{
		int adjust = (adj_stat[p_ptr->stat_ind[A_CON]][ADJ_REGEN] );

		/* Apply some healing */
		if( adjust )
			(void)set_timed_effect( TIMED_POISONED , p_ptr->poisoned - adjust);
	}

	/* Stun */
	if (p_ptr->stun)
	{
		int adjust = (adj_stat[p_ptr->stat_ind[A_CON]][ADJ_REGEN] );

		/* Apply some healing */
		if( adjust )
			(void)set_timed_effect( TIMED_STUN, p_ptr->stun - adjust);
	}

	/* Cut */
	if (p_ptr->cut)
	{
		int adjust = (adj_stat[p_ptr->stat_ind[A_CON]][ADJ_REGEN] );

		/* Hack -- Truly "mortal" wound */
		if (p_ptr->cut > 1000) adjust = 0;

		/* Apply some healing */
		if( adjust )
			(void)set_timed_effect( TIMED_CUT, p_ptr->cut - adjust);
	}

	/*** Process Light ***/

	/* Check for light being wielded */
	o_ptr = &inventory[INVEN_LITE];

	/* Burn some fuel in the current lite */
	if (o_ptr->tval == TV_LITE)
	{
		/* Hack -- Use some fuel (except on artefacts) */
		if (((o_ptr->sval == SV_LITE_TORCH) || (o_ptr->sval == SV_LITE_LANTERN)) && (o_ptr->pval > 0))
		{
			/* Decrease life-span */
			o_ptr->pval--;

			/* Hack -- notice interesting fuel steps */
			if ((o_ptr->pval < 100) || (!(o_ptr->pval % 100)))
			{
				/* Window stuff */
				p_ptr->window |= (PW_EQUIP);
			}

			/* Hack -- Special treatment when blind */
			if (p_ptr->blind)
			{
				/* Hack -- save some light for later */
				if (o_ptr->pval == 0) o_ptr->pval++;
			}

			/* The light is now out */
			else if (o_ptr->pval == 0)
			{
				disturb(0, 0);
				msg_print("Your light has gone out!");
			}

			/* The light is getting dim */
			else if ((o_ptr->pval < 100) && (!(o_ptr->pval % 10)))
			{
				if (disturb_minor) disturb(0, 0);
				msg_print("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);


	/*** Process Inventory ***/

	/* Handle experience draining */
	if (p_ptr->exp_drain)
	{
		if ((rand_int(100) < 10) && (p_ptr->exp > 0))
		{
			p_ptr->exp--;
			p_ptr->max_exp--;
			check_experience();
		}
	}

	/* Process equipment */
	for (j = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Get the object */
		o_ptr = &inventory[i];

		object_flags(o_ptr, &f1, &f2, &f3);

		if ((f3 & TR3_TY_CURSE) && (randint(TY_CURSE_CHANCE)==1))
			activate_ty_curse();

		/* Hack: Uncursed teleporting items (e.g. Planar Weapons)
		can actually be useful! */

		if ((f3 & TR3_TELEPORT) && (rand_int(100)<1))
		{
			if ((o_ptr->ident & IDENT_CURSED) && !(p_ptr->anti_tele))
			{
				disturb(0,0);
				/* Teleport player */
				teleport_player(40);
			}
			else
			{
				if ((!disturb_other) ||
					(o_ptr->note && (strchr(quark_str(o_ptr->note),'.'))))
				{
					/* Do nothing */
					/* msg_print("Teleport aborted.") */ ;
				}
				else if (get_check("Teleport? "))
				{
					disturb(0,0);
					teleport_player(50);
				}
			}
		}
		/*** Process corruption effects ***/
		if (p_ptr->muta2)
		{
			if ((p_ptr->muta2 & COR2_BERS_RAGE) && (randint(3000)==1))
			{
				disturb(0,0);
				msg_print("RAAAAGHH!");
				msg_print("You feel a fit of rage coming over you!");
				(void) set_timed_effect( TIMED_SHERO, p_ptr->shero + 10 + randint(p_ptr->lev));
			}

			if ((p_ptr->muta2 & COR2_COWARDICE) && (randint(3000)==13))
			{
				if (!(p_ptr->resist_fear || p_ptr->hero || p_ptr->shero))
				{
					disturb(0,0);
					msg_print("It's so dark... so scary!");
					p_ptr->redraw |= PR_AFRAID;
					p_ptr->afraid = (p_ptr->afraid) + 13 + rand_s16b(26);
				}
			}

			if ((p_ptr->muta2 & COR2_RTELEPORT) && (randint(5000)==88))
			{
				if (!(p_ptr->resist_nexus) && !(p_ptr->muta1 & COR1_VTELEPORT)
					&& !(p_ptr->anti_tele))
				{
					disturb(0,0);

					/* Teleport player */
					msg_print("Your position suddenly seems very uncertain...");
					msg_print(NULL);
					teleport_player(40);
				}
			}

			if ((p_ptr->muta2 & COR2_FORGET) && (randint(6400)==321))
			{
				if (!(p_ptr->resist_chaos || p_ptr->resist_conf))
				{
					disturb(0,0);
					p_ptr->redraw |= PR_EXTRA;
					msg_print("You feel a little confused...!");

					if (randint(20)==1)
					{
						msg_print(NULL);
						if (randint(3)==1) lose_all_info();
						else wiz_dark();
						teleport_player(100);
						wiz_dark();
						msg_print("You can't remember a thing, or how you got here!");
					}
					else
					{
						if (!(p_ptr->resist_conf))
						{
							(void)set_timed_effect( TIMED_CONFUSED , p_ptr->confused + rand_int(20) + 15);
						}

						if ((randint(3)==1) && !(p_ptr->resist_chaos))
						{
							msg_print("Your mind is playing tricks on you!");
							(void)set_timed_effect( TIMED_IMAGE , p_ptr->image + rand_int(150) + 150);
						}
					}
				}
			}

			if ((p_ptr->muta2 & COR2_HALLU) && (randint(6400)==42))
			{
				if (!(p_ptr->resist_chaos))
				{
					disturb(0,0);
					p_ptr->redraw |= PR_EXTRA;
					(void)set_timed_effect( TIMED_IMAGE , p_ptr->image + rand_int(50) + 20);
				}
			}

			if ((p_ptr->muta2 & COR2_BRIMSTONE) && (randint(3000)==13))
			{
				disturb(0,0);

				msg_print("Clouds of brimstone well up around you.");
				msg_print(NULL);
				fire_ball(GF_POIS, 0, p_ptr->lev,3);
			}

			if ((p_ptr->muta2 & COR2_PROD_MANA) &&
				(!(p_ptr->anti_magic)) && (randint(9000)==1))
			{
				int dire = 0;
				disturb(0,0);
				msg_print("Magical energy flows through you! You must release it!");
				flush();
				msg_print(NULL);
				(void)get_hack_dir(&dire);
				fire_ball(GF_MANA, dire, p_ptr->lev * 2, 3);
			}

			if ((p_ptr->muta2 & COR2_ATT_DEMON) &&
				(!(p_ptr->anti_magic)) && (randint(6666)==666))
			{
				bool d_summon = FALSE;
				if (randint(6)==1)
				{
					d_summon = summon_specific_friendly(py, px,
						dun_level, FILTER_DEMON, TRUE);
				}
				else
				{
					d_summon = summon_specific(py, px,
						dun_level, FILTER_DEMON);
				}

				if (d_summon)
				{
					msg_print("You have attracted a demon!");
					disturb(0,0);
				}
			}

			if ((p_ptr->muta2 & COR2_SPEED_FLUX) && (randint(6000)==1))
			{

				disturb(0,0);
				if (randint(2)==1)
				{
					msg_print("You feel less energetic.");
					if (p_ptr->fast > 0)
					{
						set_timed_effect( TIMED_FAST, 0);
					}
					else
					{
						set_timed_effect( TIMED_SLOW, p_ptr->slow + randint(30) + 10);
					}
				}
				else
				{
					msg_print("You feel more energetic.");
					if (p_ptr->slow > 0)
					{
						set_timed_effect( TIMED_SLOW, 0);
					}
					else
					{
						set_timed_effect( TIMED_FAST, p_ptr->fast + randint(30) + 10);
					}
				}
				msg_print(NULL);
			}
			if ((p_ptr->muta2 & COR2_BANISH_ALL) &&
				(randint(9000)==1))
			{

				disturb(0,0);
				msg_print("You suddenly feel almost lonely.");
				banish_monsters(100);
				msg_print(NULL);
			}
			if ((p_ptr->muta2 & COR2_EAT_LIGHT) && (randint(3000) == 1))
			{
				object_type *o_ptr;

				msg_print("A shadow passes over you.");
				msg_print(NULL);

				/* Absorb light from the current possition */
				if (cave[py][px].info & CAVE_GLOW)
				{
					hp_player(10);
				}

				o_ptr = &inventory[INVEN_LITE];

				/* Absorb some fuel in the current lite */
				if (o_ptr->tval == TV_LITE)
				{
					/* Use some fuel (except on artefacts) */
					if (((o_ptr->sval == SV_LITE_TORCH) || (o_ptr->sval == SV_LITE_LANTERN)) && (o_ptr->pval > 0))
					{
						/* Heal the player a bit */
						hp_player(o_ptr->pval/20);

						/* Decrease life-span of lite */
						o_ptr->pval /= 2;

						msg_print("You absorb energy from your light!");

						/*
						* ToDo: Implement a function to handle
						* changes of the fuel
						*/

						/* Hack -- notice interesting fuel steps */
						if ((o_ptr->pval < 100) || (!(o_ptr->pval % 100)))
						{
							/* Window stuff */
							p_ptr->window |= (PW_EQUIP);
						}

						/* Hack -- Special treatment when blind */
						if (p_ptr->blind)
						{
							/* Hack -- save some light for later */
							if (o_ptr->pval == 0) o_ptr->pval++;
						}

						/* The light is now out */
						else if (o_ptr->pval == 0)
						{
							disturb(0, 0);
							msg_print("Your light has gone out!");
						}

						/* The light is getting dim */
						else if ((o_ptr->pval < 100) && (!(o_ptr->pval % 10)))
						{
							if (disturb_minor) disturb(0, 0);
							msg_print("Your light is growing faint.");
						}
					}
				}

				/*
				* Unlite the area (radius 10) around player and
				* do 50 points damage to every affected monster
				*/
				unlite_area(50, 10);
			}
			if ((p_ptr->muta2 & COR2_ATT_ANIMAL) && !(p_ptr->anti_magic) &&
				(randint(7000)==1))
			{

				bool a_summon = FALSE;
				if (randint(3)==1)
					a_summon = summon_specific_friendly(py,
					px, dun_level, FILTER_ANIMAL, TRUE);
				else
					a_summon = summon_specific(py,
					px, dun_level, FILTER_ANIMAL);
				if (a_summon)
				{
					msg_print("You have attracted an animal!");
					disturb(0,0);
				}
			}
			if ((p_ptr->muta2 & COR2_RAW_CHAOS) && !(p_ptr->anti_magic) &&
				(randint(8000)==1))
			{

				disturb(0,0);
				msg_print("You feel the world warping around you!");
				msg_print(NULL);
				fire_ball(GF_CHAOS, 0, p_ptr->lev,8);
			}
			if ((p_ptr->muta2 & COR2_NORMALITY) &&
				(randint(5000)==1))
			{
				lose_corruption(0);
			}
			if ((p_ptr->muta2 & COR2_WRAITH) && !(p_ptr->anti_magic) &&
				(randint(3000)==13))
			{

				disturb(0,0);
				msg_print("You feel insubstantial!");
				msg_print(NULL);
				set_timed_effect( TIMED_WRAITH_FORM, p_ptr->wraith_form + randint(p_ptr->lev/2) + (p_ptr->lev/2));
			}
			if ((p_ptr->muta2 & COR2_POLY_WOUND) &&
				(randint(3000)==1))
			{
				do_poly_wounds();
			}
			if ((p_ptr->muta2 & COR2_WASTING) &&
				(randint(3000)==13))
			{
				int which_stat = rand_int(6);
				int sustained = FALSE;

				switch (which_stat)
				{
				case A_STR:
					if (p_ptr->sustain_str) sustained = TRUE;
					break;
				case A_INT:
					if (p_ptr->sustain_int) sustained = TRUE;
					break;
				case A_WIS:
					if (p_ptr->sustain_wis) sustained = TRUE;
					break;
				case A_DEX:
					if (p_ptr->sustain_dex) sustained = TRUE;
					break;
				case A_CON:
					if (p_ptr->sustain_con) sustained = TRUE;
					break;
				case A_CHA:
					if (p_ptr->sustain_cha) sustained = TRUE;
					break;
				default:
					msg_print("Invalid stat chosen!");
					sustained = TRUE;
				}

				if (!sustained)
				{
					disturb(0,0);
					msg_print("You can feel yourself wasting away!");
					msg_print(NULL);
					(void)dec_stat(which_stat, randint(6)+6, randint(3)==1);
				}
			}
			if ((p_ptr->muta2 & COR2_ATT_DRAGON) && !(p_ptr->anti_magic) &&
				(randint(3000)==13))
			{

				bool d_summon = FALSE;
				if (randint(5)==1)
					d_summon = summon_specific_friendly(py,
					px, dun_level, FILTER_DRAGON, TRUE);
				else
					d_summon = summon_specific(py,
					px, dun_level, FILTER_DRAGON);
				if (d_summon)
				{
					msg_print("You have attracted a dragon!");
					disturb(0,0);
				}
			}
			if ((p_ptr->muta2 & COR2_WEIRD_MIND) && !(p_ptr->anti_magic) &&
				(randint(3000)==1))
			{
				if (p_ptr->tim_esp > 0)
				{
					msg_print("Your mind feels cloudy!");
					set_timed_effect( TIMED_ESP, 0);
				}
				else
				{
					msg_print("Your mind expands!");
					set_timed_effect( TIMED_ESP, p_ptr->lev);
				}
			}
			if ((p_ptr->muta2 & COR2_NAUSEA) && !(p_ptr->slow_digest) &&
				(randint(9000)==1))
			{

				disturb(0,0);
				msg_print("Your stomach roils, and you lose your lunch!");
				msg_print(NULL);
				set_food(PY_FOOD_WEAK);
			}

			if ((p_ptr->muta2 & COR2_INTUITION) &&
				!(p_ptr->anti_magic) &&
				(randint(12000) == 1))
			{
				alter_reality();
			}

			if ((p_ptr->muta2 & COR2_WARNING) &&
				(randint(1000)==1))
			{
				int danger_amount = 0;
				int monster;

				for (monster = 0; monster < m_max; monster++)
				{
					monster_type    *m_ptr = &m_list[monster];
					monster_race    *r_ptr = &r_info[m_ptr->r_idx];

					/* Paranoia -- Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					if (r_ptr->level >= p_ptr->lev)
					{
						danger_amount += r_ptr->level - p_ptr->lev + 1;
					}
				}

				if (danger_amount > 100)
					msg_print("You feel utterly terrified!");
				else if (danger_amount > 50)
					msg_print("You feel terrified!");
				else if (danger_amount > 20)
					msg_print("You feel very worried!");
				else if (danger_amount > 10)
					msg_print("You feel paranoid!");
				else if (danger_amount > 5)
					msg_print("You feel almost safe.");
				else
					msg_print("You feel lonely.");
			}
			if ((p_ptr->muta2 & COR2_INVULN) && !(p_ptr->anti_magic) &&
				(randint(5000)==1))
			{

				disturb(0,0);
				msg_print("You feel invincible!");
				msg_print(NULL);
				(void)set_timed_effect( TIMED_INVULN, p_ptr->invuln + randint(8) + 8);
			}
			if ((p_ptr->muta2 & COR2_SP_TO_HP) &&
				(randint(2000)==1))
			{
				int wounds = p_ptr->mhp - p_ptr->chp;

				if (wounds > 0)
				{
					int healing = p_ptr->csp;

					if (healing > wounds)
					{
						healing = wounds;
					}

					hp_player(healing);
					p_ptr->csp -= healing;
				}
			}
			if ((p_ptr->muta2 & COR2_HP_TO_SP) && !(p_ptr->anti_magic) &&
				(randint(4000)==1))
			{
				int wounds = p_ptr->msp - p_ptr->csp;

				if (wounds > 0)
				{
					int healing = p_ptr->chp;

					if (healing > wounds)
					{
						healing = wounds;
					}

					p_ptr->csp += healing;
					take_hit(healing, "blood rushing to the head");
				}
			}
			if ((p_ptr->muta2 & COR2_DISARM) &&
				(randint(10000)==1))
			{
				object_type *o_ptr;

				disturb(0,0);
				msg_print("You trip over your own feet!");
				take_hit(randint(p_ptr->wt/6), "tripping");

				msg_print(NULL);
				o_ptr = &inventory[INVEN_WIELD];
				if (o_ptr->k_idx)
				{
					msg_print("You drop your weapon!");
					inven_drop(INVEN_WIELD,1);
				}
			}
		}


		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Recharge activatable objects */
		if (o_ptr->timeout > 0)
		{
			/* Recharge */
			o_ptr->timeout--;

			/* Notice changes */
			if (!(o_ptr->timeout)) j++;
		}
	}

	/* Notice changes */
	if (j)
	{
		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}

	/* Recharge rods */
	for (j = 0, i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Examine all charging rods */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->pval))
		{
			/* Charge it */
			o_ptr->pval--;

			/* Notice changes */
			if (!(o_ptr->pval)) j++;
		}
	}

	/* Notice changes */
	if (j)
	{
		/* Combine pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/* Feel the inventory */
	sense_inventory();


	/*** Process Objects ***/

	/* Process objects */
	for (i = 1; i < o_max; i++)
	{
		/* Access object */
		o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Recharge rods on the ground */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->pval)) o_ptr->pval--;
	}

	/* Delayed Word-of-Recall */
	if (p_ptr->word_recall)
	{
		/* Count down towards recall */
		p_ptr->word_recall--;

		/* Activate the recall */
		if (!p_ptr->word_recall)
		{
			/* Disturbing! */
			disturb(0, 0);

			/* Determine the level */
			if (dun_level != 0)
			{
				msg_print("You feel yourself yanked upwards!");

				if (autosave_l)
				{
					is_autosave = TRUE;
					msg_print("Autosaving the game...");
					do_cmd_save_game();
					is_autosave = FALSE;
				}

				dun_level = 0;

				new_level_flag = TRUE;
				came_from=START_RANDOM;
			}
			else
			{
				msg_print("You feel yourself yanked downwards!");

				if (autosave_l)
				{
					is_autosave = TRUE;
					msg_print("Autosaving the game...");
					do_cmd_save_game();
					is_autosave = FALSE;
				}
				dun_level = p_ptr->max_dun_level;
				if (dun_level < 1) dun_level = 1;
				new_level_flag = TRUE;
			}

			/* Sound */
			sound(SOUND_TPLEVEL);
		}
	}
}





#ifdef ALLOW_WIZARD
/*
* Hack -- Declare the Debug Routines
*/
extern void do_cmd_debug(void);
#endif


#ifdef ALLOW_BORG

/*
* Verify use of "borg" commands
*/
static bool enter_borg_mode(void)
{
	/* Ask first time */
	if (!(noscore & 0x0010))
	{
		/* Mention effects */
		msg_print("The borg commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use borg commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use borg commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0010;
	}

	/* Success */
	return (TRUE);
}

/*
* Hack -- Declare the Ben Borg
*/
extern void do_cmd_borg(void);

#endif

/*
 * Deal with unified commands or specific command, based on unify_commands
 * This prevented much copy pasting..
 */
static void do_cmd_considering_unified(  void (*command)(int)  )
{
	if(!unify_commands)
	{
		command(-999);
	}else{
		do_cmd_handle();
	}
}

/*
* Parse and execute the current command
* Give "Warning" on illegal commands.
*
* XXX XXX XXX Make some "blocks"
*/
static void process_command(void)
{

	/* char error_m[80]; */

#ifdef ALLOW_REPEAT /* TNB */

	/* Handle repeating the last command */
	repeat_check();

#endif /* ALLOW_REPEAT -- TNB */


    /*** Help and Such ***/
    /* First we do the menu'd help, if the user presses ? again,
     * we go into the 'fullblown' help
     */
	if( command_cmd == '?' )
    {
        command_cmd = menu_key_help();
        /*plog_fmt_fiddle("Char received : %c" , command_cmd);*/
        if( command_cmd == '?' )
        {
            do_cmd_help(syshelpfile);
        }
    }

	/* Parse the command */
	switch (command_cmd)
	{

#ifdef ALLOW_WIZARD
	/* Special "debug" commands */
	case KTRL('A'):
	{
		/* Enter debug mode */
		if (debug_mode)
			do_cmd_debug();
		else
			prt("Your name is not wizardly enough..", 0, 0);
		break;
	}
#endif

#ifdef ALLOW_BORG
	/* Special "borg" commands */
	case KTRL('Z'):
	{
		/* Enter borg mode */
		if (enter_borg_mode())
		{
			do_cmd_borg();
		}
		break;
	}
#endif

#ifndef VERIFY_SAVEFILE
	/* Hack -- Save and don't quit */
	case KTRL('S'):
	{
		is_autosave = FALSE;
		do_cmd_save_game();
		break;
	}
#endif

	/* Ignore                      */ case ESCAPE : {  break; }
	/* Ignore                      */ case ' ' : { break; }
	/* Ignore return               */ case '\r': { break; }
	/* Wear/wield equipment        */ case 'w' : { do_cmd_wield();              break; }
	/* Take off equipment          */ case 't' : { do_cmd_takeoff();            break; }
	/* Drop an item                */ case 'd' : { do_cmd_drop();               break; }
	/* Destroy an item             */ case 'k' : { do_cmd_destroy();            break; }
	/* Destroy all worthless items */ case 'K' : { do_cmd_destroy_all();        break; }
	/* Equipment list              */ case 'e' : { do_cmd_equip();              break; }
	/* Inventory list              */ case 'i' : { do_cmd_inven();              break; }
	/* Identify an object          */ case 'I' : { do_cmd_observe();            break; }
	/* Alter a grid                */ case '+' : { do_cmd_alter();              break; }
	/* Dig a tunnel                */ case 'T' : { do_cmd_tunnel();             break; }
	/* Move (pick up things)       */ case ';' : { do_cmd_walk(FALSE);          break; }
	/* Move (do not pick up)       */ case '-' : { do_cmd_walk(TRUE);           break; }
	/* Begin Running               */ case '.' : { do_cmd_run();                break; }
	/* Stay still , pick up        */ case ',' : { do_cmd_stay(0);              break; }
	/* Stay still , no pick up     */ case 'g' : { do_cmd_stay(1);              break; }
	/* Rest -- Arg is time         */ case 'R' : { do_cmd_rest();               break; }
	/* Search for traps/doors      */ case 's' : { do_cmd_search();             break; }
	/* Toggle search mode          */ case 'S' : { do_cmd_toggle_search();      break; }
	/* Enter store                 */ case '_' : { do_cmd_store();              break; }
	/* Go up staircase             */ case '<' : { do_cmd_go_up();              break; }
	/* Go down staircase           */ case '>' : { do_cmd_go_down();            break; }
	/* Open a door or chest        */ case 'o' : { do_cmd_open();               break; }
	/* Close a door                */ case 'c' : { do_cmd_close();              break; }
	/* Jam a door with spikes      */ case 'j' : { do_cmd_spike();              break; }
	/* Bash a door                 */ case 'B' : { do_cmd_bash();               break; }
	/* Disarm a trap or chest      */ case 'D' : { do_cmd_disarm();             break; }
	/* Gain new spells/prayers     */ case 'G' : { do_cmd_gain_spell();         break; }
	/* Cast a spell                */ case 'm' : { do_cmd_magic_spell();        break; }
	/* Do alchemy                  */ case 'p' : { do_cmd_mix();                break; }
	/* Inscribe an object          */ case '{' : { do_cmd_inscribe();           break; }
	/* Uninscribe an object        */ case '}' : { do_cmd_uninscribe();	        break; }
	/* Fire an item                */ case 'f' : { do_cmd_fire();               break; }
	/* Throw an item               */ case 'v' : { do_cmd_throw();              break; }
	/* Racial Power                */ case 'U' : { do_cmd_racial_power();       break; }
	/* Full dungeon map            */ case 'M' : { do_cmd_view_map();           break; }
	/* Locate player on map        */ case 'L' : { do_cmd_locate();             break; }
	/* Look around                 */ case 'l' : { do_cmd_look();               break; }
	/* Target monster or location  */ case '*' : { do_cmd_target();             break; }
	/* Identify symbol             */ case '/' : { do_cmd_query_symbol();       break; }
	/* Character description       */ case 'C' : { do_cmd_change_name();        break; }
	/* Hack -- User interface      */ case '!' : { Term_user(0);                break; }
	/* 1 line from a pref file     */ case '"' : { do_cmd_pref();               break; }
	/* Interact with macros        */ case '@' : { do_cmd_macros();             break; }
	/* Interact with visuals       */ case '%' : { do_cmd_visuals();            break; }
	/* Interact with colours       */ case '&' : { do_cmd_colours();            break; }
	/* Interact with options       */ case '=' : { do_cmd_options();            break; }
	/* Take notes                  */ case ':' : { do_cmd_note();               break; }
	/* Version info                */ case 'V' : { do_cmd_version();            break; }
	/* Commit suicide              */ case 'Q' : { do_cmd_suicide();            break; }
	/* Check knowledge             */ case '~' : { do_cmd_knowledge();          break; }
    /* Check knowledge             */ case '|' : { do_cmd_knowledge();          break; }
	/* Load "screen dump"          */ case '(' : { do_cmd_screen_dump();        break; }
	/* Save "screen dump"          */ case ')' : { do_cmd_save_screen();        break; }
	/* Show visible monsters       */ case '[' : { do_cmd_monsters();           break; }
	/* Show visible objects        */ case ']' : { do_cmd_objects();            break; }
	/* Browse a book               */ case 'b' : {do_cmd_considering_unified( do_cmd_browse );       break; }
	/* Activate an artefact        */ case 'A' : {do_cmd_considering_unified( do_cmd_activate );     break; }
	/* Eat some food               */ case 'E' : {do_cmd_considering_unified( do_cmd_eat_food );     break; }
	/* Fuel your lantern/torch     */ case 'F' : {do_cmd_considering_unified( do_cmd_refill );       break; }
	/* Aim a wand                  */ case 'a' : {do_cmd_considering_unified( do_cmd_aim_wand );     break; }
	/* Zap a rod                   */ case 'z' : {do_cmd_considering_unified( do_cmd_zap_rod );      break; }
	/* Quaff a potion              */ case 'q' : {do_cmd_considering_unified( do_cmd_quaff_potion ); break; }
	/* Read a scroll               */ case 'r' : {do_cmd_considering_unified( do_cmd_read_scroll );  break; }
	/* Use a staff                 */ case 'u' : {do_cmd_considering_unified( do_cmd_use_staff );    break; }

	/* Repeat level feeling   */ case KTRL('F'): { do_cmd_feeling(FALSE); break; }
	/* Show previous message  */ case KTRL('O'): { do_cmd_message_one();  break; }
	/* Show previous messages */ case KTRL('P'): { do_cmd_messages();     break; }
	/* Redraw the screen      */ case KTRL('R'): { do_cmd_redraw();       break; }
	/* Time of day            */ case KTRL('T'): { do_cmd_time();         break; }
	/* Save and quit          */ case KTRL('X'): { alive = FALSE;         break; }
	/* Toggle windows         */ case KTRL('I'): { toggle_inven_equip();  break; }
	/* Unknown command        */ default       : {  prt("Type '?' for help.", 0, 0); }
	}
}


/*
* Process the player
*
* Notice the annoying code to handle "pack overflow", which
* must come first just in case somebody manages to corrupt
* the savefiles by clever use of menu commands or something.
*/
static void process_player(void)
{
	int i;

	/*** Apply energy ***/

	if (hack_corruption)
	{
		msg_print("You feel different!");
		(void)gain_corruption(0);
		hack_corruption = FALSE;
	}

	/* Give the player some energy */
	p_ptr->energy += 10;

	/* No turn yet */
	if (p_ptr->energy < 1000) return;


	/*** Check for interupts ***/

	/* Complete resting */
	if (resting < 0)
	{
		/* Basic resting */
		if (resting == -1)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
				(p_ptr->csp >= p_ptr->msp))
			{
				disturb(0, 0);
			}
		}

		/* Complete resting */
		else if (resting == -2)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
				(p_ptr->csp == p_ptr->msp) &&
				!p_ptr->blind && !p_ptr->confused &&
				!p_ptr->poisoned && !p_ptr->afraid &&
				!p_ptr->stun && !p_ptr->cut &&
				!p_ptr->slow && !p_ptr->paralyzed &&
				!p_ptr->image && !p_ptr->word_recall)
			{
				disturb(0, 0);
			}
		}
	}

	/* Handle "abort" */
	if (!avoid_abort)
	{
		/* Check for "player abort" (semi-efficiently for resting) */
		if (running || command_rep || (resting && !(resting & 0x0F)))
		{
			/* Do not wait */
			inkey_scan = TRUE;

			/* Check for a key */
			if (inkey())
			{
				/* Flush input */
				flush();

				/* Disturb */
				disturb(0, 0);

				/* Hack -- Show a Message */
				msg_print("Cancelled.");
			}
		}
	}


	/*** Handle actual user input ***/

	/* Repeat until out of energy */
	while (p_ptr->energy >= 1000)
	{
		/* Notice stuff (if needed) */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff (if needed) */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->window) window_stuff();


		/* Place the cursor on the player */
		move_cursor_relative(py, px);

		/* Refresh (optional) */
		if (fresh_before) Term_fresh();


		/* Hack -- Pack Overflow */
		if (inventory[INVEN_PACK].k_idx)
		{
			int item = INVEN_PACK;

			char o_name[80];

			object_type *o_ptr;

			/* Access the slot to be dropped */
			o_ptr = &inventory[item];

			/* Disturbing */
			disturb(0, 0);

			/* Warning */
			msg_print("Your pack overflows!");

			/* Describe */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Message */
			msg_format("You drop %s (%c).", o_name, index_to_label(item));

			/* Drop it (carefully) near the player */
			drop_near(o_ptr, 0, py, px);

			/* Modify, Describe, Optimize */
			inven_item_increase(item, -255);
			inven_item_describe(item);
			inven_item_optimize(item);

			/* Notice stuff (if needed) */
			if (p_ptr->notice) notice_stuff();

			/* Update stuff (if needed) */
			if (p_ptr->update) update_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->redraw) redraw_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->window) window_stuff();
		}

		/* Squelch stuff */
		do_squelch();

		/* Hack -- cancel "lurking browse mode" */
		if (!command_new) command_see = FALSE;


		/* Assume free turn */
		energy_use = 0;


		/* Paralyzed or Knocked Out */
		if ((p_ptr->paralyzed) || (p_ptr->stun >= 100))
		{
			/* Take a turn */
			energy_use = 100;
		}

		/* Resting */
		else if (resting)
		{
			/* Timed rest */
			if (resting > 0)
			{
				/* Reduce rest count */
				resting--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);
			}

			/* Take a turn */
			energy_use = 100;
		}

		/* Running */
		else if (running)
		{
			/* Take a step */
			run_step(0);
		}

		/* Repeated command */
		else if (command_rep)
		{
			/* Count this execution */
			command_rep--;

			/* Redraw the state */
			p_ptr->redraw |= (PR_STATE);

			/* Redraw stuff */
			redraw_stuff();

			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command();
		}

		/* Normal command */
		else
		{
			/* Place the cursor on the player */
			move_cursor_relative(py, px);

			/* Get a command (normal) */
			request_command(FALSE);

			/* Process the command */
			process_command();
		}


		/*** Clean up ***/

		/* Significant */
		if (energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= energy_use;


			/* Hack -- constant hallucination */
			if (p_ptr->image) p_ptr->redraw |= (PR_MAP);


			/* Shimmer monsters if needed */
			if (!avoid_other && shimmer_monsters)
			{
				/* Clear the flag */
				shimmer_monsters = FALSE;

				/* Shimmer multi-hued monsters */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;
					monster_race *r_ptr;

					/* Access monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Access the monster race */
					r_ptr = &r_info[m_ptr->r_idx];

					/* Skip non-multi-hued monsters */
					if (!(r_ptr->flags1 & (RF1_ATTR_MULTI))) continue;

					/* Reset the flag */
					shimmer_monsters = TRUE;

					/* Redraw regardless */
					lite_spot(m_ptr->fy, m_ptr->fx);
				}
			}


			/* Handle monster detection */
			if (repair_monsters)
			{
				/* Reset the flag */
				repair_monsters = FALSE;

				/* Rotate detection flags */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;

					/* Access monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Nice monsters get mean */
					if (m_ptr->mflag & (MFLAG_NICE))
					{
						/* Nice monsters get mean */
						m_ptr->mflag &= ~(MFLAG_NICE);
					}

					/* Handle memorized monsters */
					if (m_ptr->mflag & (MFLAG_MARK))
					{
						/* Maintain detection */
						if (m_ptr->mflag & (MFLAG_SHOW))
						{
							/* Forget flag */
							m_ptr->mflag &= ~(MFLAG_SHOW);

							/* Still need repairs */
							repair_monsters = TRUE;
						}

						/* Remove detection */
						else
						{
							/* Forget flag */
							m_ptr->mflag &= ~(MFLAG_MARK);

							/* Assume invisible */
							m_ptr->ml = FALSE;

							/* Update the monster */
							update_mon(i, FALSE);

							/* Redraw regardless */
							lite_spot(m_ptr->fy, m_ptr->fx);
						}
					}
				}
			}
		}


		/* Hack -- notice death or departure */
		if (!alive || death || new_level_flag) break;
	}
}


/*
* Interact with the current dungeon level.
*
* This function will not exit until the level is completed,
* the user dies, or the game is terminated.
*/
static void dungeon(void)
{
	/* Reset various flags */
	new_level_flag = FALSE;
	hack_mind = FALSE;



	/* Reset the "command" vars */
	command_cmd = 0;
	command_new = 0;
	command_rep = 0;
	command_arg = 0;
	command_dir = 0;


	/* Cancel the target */
	target_who = 0;

	/* Cancel the health bar */
	health_track(0);


	/* Check visual effects */
	shimmer_monsters = TRUE;
	shimmer_objects = TRUE;
	repair_monsters = TRUE;
	repair_objects = TRUE;

	/* Disturb */
	disturb(1, 0);

	/* Track maximum player level */
	if (p_ptr->max_plv < p_ptr->lev)
	{
		p_ptr->max_plv = p_ptr->lev;
	}


	/* Track maximum dungeon level */
	if (p_ptr->max_dun_level < dun_level)
	{
		p_ptr->max_dun_level = dun_level;
	}


	/* Paranoia -- No stairs down from Quest */
	if (is_quest(dun_level))
	{
		create_down_stair = FALSE;
	}

	/* Paranoia -- no stairs from town */
	if (dun_level <= 0) create_down_stair = create_up_stair = FALSE;

	/* Option -- no connected stairs */
	if (!dungeon_stair) create_down_stair = create_up_stair = FALSE;

	/* Make a stairway. */
	if (create_up_stair || create_down_stair)
	{
		/* Place a stairway */
		if (cave_valid_bold(py, px))
		{
			/* XXX XXX XXX */
			delete_object(py, px);

			/* Make stairs */
			if (create_down_stair)
			{
				cave_set_feat(py, px, FEAT_MORE);
			}
			else
			{
				cave_set_feat(py, px, FEAT_LESS);
			}
		}

		/* Cancel the stair request */
		create_down_stair = create_up_stair = FALSE;
	}

	/* Verify the panel */
	verify_panel();

	/* Validate the panel */
	if (centre_view)
	{
		panel_bounds_center();
	}
	else
	{
		panel_bounds();
	}

	/* Flush messages */
	msg_print(NULL);


	/* Enter "xtra" mode */
	character_xtra = TRUE;

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Window stuff */
    p_ptr->window |= (PW_MONSTER | PW_VISIBLE);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Redraw stuff */
	window_stuff();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_DISTANCE);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Leave "xtra" mode */
	character_xtra = FALSE;

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Combine / Reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Notice stuff */
	notice_stuff();

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Window stuff */
	window_stuff();

	/* Refresh */
	Term_fresh();


	/* Hack -- notice death or departure */
	if (!alive || death || new_level_flag) return;

	/* Notice a Quest Level */
	if (is_quest(dun_level)) quest_discovery();

	/*** Process this dungeon level ***/

	/* Reset the monster generation level */
	monster_level = dun_level;

	/* Reset the object generation level */
	object_level = dun_level;

	hack_mind = TRUE;

	if(arg_fiddle)msg_note( "About to go into the main loop" );

	/* Main loop */
	while (TRUE)
	{
		/* Hack -- Compact the monster list occasionally */
		if (m_cnt + 32 > MAX_M_IDX) compact_monsters(64);

		/* Hack -- Compress the monster list occasionally */
		if (m_cnt + 32 < m_max) compact_monsters(0);

		/* Hack -- Compact the object list occasionally */
		if (o_cnt + 32 > MAX_O_IDX) compact_objects(64);

		/* Hack -- Compress the object list occasionally */
		if (o_cnt + 32 < o_max) compact_objects(0);


		/* Process the player */
		process_player();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(py, px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death || new_level_flag) break;

		total_friends = 0;
		total_friend_levels = 0;

		/* Process all of the monsters */
		process_monsters();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(py, px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death || new_level_flag) break;


		/* Process the world */
		process_world();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(py, px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Hack -- Notice death or departure */
		if (!alive || death || new_level_flag) break;


		/* Count game turns */
		turn++;
		if ((turn-old_turn)==2500)
		{
			if(dun_level >0)
			{
				do_cmd_feeling(TRUE);
			}
		}
	}
}




/* Arcum Dagsson's code to support separate macro files for different realms */

/*
* Load some "user pref files"
*/
static void load_all_pref_files(void)
{
	char buf[1024];

	/* Access the "race" pref file */
	sprintf(buf, "%s.prf", rp_ptr->title);

	/* Process that file */
	process_pref_file(buf);

	/* Access the "class" pref file */
	sprintf(buf, "%s.prf", cp_ptr->title);

	/* Process that file */
	process_pref_file(buf);

	/* Access the "character" pref file */
	sprintf(buf, "%s.prf", player_base);

	/* Process that file */
	process_pref_file(buf);

	/* Access the "realm 1" pref file */
	/* But only if the realm name is NOT set to 'no magic' */
	if (strcmp(realm_names[p_ptr->realm1].name, "no magic") != 0)
		/* And the realm name is NOT set to 'unknown' */
		if (strcmp(realm_names[p_ptr->realm1].name, "unknown") != 0)
		{
			sprintf(buf, "%s.prf",realm_names[p_ptr->realm1].name);

			/* Process that file */
			process_pref_file(buf);
		}

	/* Access the "realm 2" pref file */
	/* But only if the realm name is NOT set to 'no magic' */
	if (strcmp(realm_names[p_ptr->realm2].name, "no magic") != 0)
		/* And the realm name is NOT set to 'unknown' */
		if (strcmp(realm_names[p_ptr->realm2].name, "unknown") != 0)
			{
				sprintf(buf, "%s.prf",realm_names[p_ptr->realm2].name);

				/* Process that file */
				process_pref_file(buf);
			}
}

/*
* Actually play a game
*
* If the "new_game" parameter is true, then, after loading the
* savefile, we will commit suicide, if necessary, to allow the
* player to start a new game. (But see below for win32 systems.)
*/
void play_game(bool new_game)
{
	/*,j,x,y; removed for some compiler warning */
	int i;

	hack_corruption = FALSE;

	/* Hack -- Character is "icky" */
	character_icky = TRUE;


	/* Hack -- turn off the cursor */
	(void)Term_set_cursor(0);

	/*
	* In win32 settings we are unlikely to have a command argument passed,
	* but on the other hand we have a dialog box so that we will have already
	* given a name for our new save file by this point. In this case, we must
	* suppress the attempted load & suicide as there is no file to load.
	*
	* (It's a mega-hack, but it works...)
	*
	*/

#ifdef WINDOWS
	if (!new_game)
	{
#endif /* WINDOWS */

		/* Attempt to load */
		if (!load_player())
		{
			/* Oops */
			quit("broken savefile");
		}

#ifdef WINDOWS
	}
#endif /* WINDOWS */

	/* Nothing loaded */
	if (!character_loaded)
	{
		/* Make new player */
		new_game = TRUE;

		/* The dungeon is not ready */
		character_dungeon = FALSE;
	}

	/* Process old character */
	if (!new_game)
	{
		/* Process the player name */
		process_player_name();
	}

	/* Init the RNG */
	if (Rand_quick)
	{
		u32b seed;

		/* Basic seed, casting time_t to u32b */
		seed = (u32b)(time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() << 1));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Extract the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_set;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (option_info[i].o_var)
		{
			/* Set */
			if (option_flag[os] & (1L << ob))
			{
				/* Set */
				(*option_info[i].o_var) = TRUE;
			}

			/* Clear */
			else
			{
				/* Clear */
				(*option_info[i].o_var) = FALSE;
			}
		}
	}
#ifdef live
	/* On live, we should save every 50 turns, it can crash after all */
	autosave_l = TRUE;
	autosave_t = TRUE;
	autosave_freq = 50;
#endif

	/* Roll new character */
	if (new_game)
	{
		/* The dungeon is not ready */
		character_dungeon = FALSE;

		/* Hack -- seeds for flavors, wilderness & alchemy */
		seed_flavor = rand_int(0x10000000);
		seed_town = rand_int(0x10000000);
		seed_alchemy = rand_int(0x10000000);

		/* Start in town */
		dun_level = 0;
		came_from = START_RANDOM;

		/* Roll up a new character */
		player_birth();

		/* Hack -- enter the world at day unless undead*/
		if ( rp_ptr->undead  )
		{
			turn=50001;
		}
		else
		{
			turn=1;
		}

		/* You have received a letter */
		msg_print("You have a received a letter.");
		msg_print(NULL);

		/* Show the letter */
		do_cmd_load_screen(ANGBAND_DIR_FILE, "level0.txt");

		/*void restore_screen( void )*/
		(void)msg_flush_wait();
		msg_print("You travel to Volterra in Italy.");
		msg_print(NULL);
		msg_print("You mutter 'Lasciate ogne esperanza, voi chi intrate'.");
		msg_print(NULL);
	}


	/* Flash a message */
	prt("Please wait...", 0, 0);

	/* Flush the message */
	Term_fresh();

	/* Flavor the objects */
	flavor_init();

	/* Generate alchemy tables */
	alchemy_init();

	/* Reset the visual mappings */
	reset_visuals();


	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER | PW_VISIBLE);

	/* Window stuff */
	if(arg_fiddle)msg_note( "Doing windows stuff" );
	window_stuff();

	if(arg_fiddle)msg_note( "Load pref files" );
	/* Load the "pref" files */
	load_all_pref_files();

	/* Set or clear "rogue_like_commands" if requested */
	if (arg_force_original) rogue_like_commands = FALSE;
	if (arg_force_roguelike) rogue_like_commands = TRUE;

	/* React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Generate a dungeon level if needed */
	if (!character_dungeon) generate_cave();


	/* Character is now "complete" */
	character_generated = TRUE;


	/* Hack -- Character is no longer "icky" */
	character_icky = FALSE;


	/* Start game */
	alive = TRUE;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) death = TRUE;

	/* Process */
	while (TRUE)
	{
		/* Process the level */
		if(arg_fiddle)msg_note( "Process the level" );
		dungeon();

		/* Notice stuff */
		if(arg_fiddle)msg_note( "Notice stuff" );
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if(arg_fiddle)msg_note( " Update stuff" );
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if(arg_fiddle)msg_note( "Redraw stuff" );
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if(arg_fiddle)msg_note( "Window stuff" );
		if (p_ptr->window) window_stuff();


		/* Cancel the target */
		target_who = 0;

		/* Cancel the health bar */
		health_track(0);


		/* Forget the lite */
		forget_lite();

		/* Forget the view */
		forget_view();


		/* Handle "quit and save" */
		if (!alive && !death) break;


		/* Erase the old cave */
		wipe_o_list();

		/* Kill non-pet monsters - pets are saved */
		remove_non_pets();


		/* XXX XXX XXX */
		msg_print(NULL);

		/* Accidental Death */
		if (alive && death)
		{
			/* Mega-Hack -- Allow player to Debug death */
			if ((debug_mode || debug_live) && !get_check("Die? "))
			{
				/* Mark social class, reset age, if needed */
				if (p_ptr->sc) p_ptr->sc = p_ptr->age = 0;

				/* Increase age */
				p_ptr->age++;

				/* Mark savefile */
				noscore |= 0x0001;

				/* Message */
				msg_print("You invoke debug mode and cheat death.");
				msg_print(NULL);

				/* Restore hit points */
				p_ptr->chp = p_ptr->mhp;
				p_ptr->chp_frac = 0;

				/* Restore spell points */
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;

				/* Hack -- Healing */
				(void)set_timed_effect( TIMED_BLIND , 0);
				(void)set_timed_effect( TIMED_CONFUSED , 0);
				(void)set_timed_effect( TIMED_POISONED , 0);
				(void)set_timed_effect( TIMED_AFRAID , 0);
				(void)set_timed_effect( TIMED_PARALYZED , 0);
				(void)set_timed_effect( TIMED_IMAGE , 0);
				(void)set_timed_effect( TIMED_STUN, 0);
				(void)set_timed_effect( TIMED_CUT, 0);

				/* Hack -- Prevent starvation */
				(void)set_food(PY_FOOD_MAX - 1);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg_print("A tension leaves the air around you...");
					msg_print(NULL);

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Note cause of death XXX XXX XXX */
				(void)strcpy(died_from, "Cheating death");

				/* Teleport to town */
				new_level_flag = TRUE;
				came_from=START_RANDOM;

				/* Go to town */
				dun_level = 0;

				/* Do not die */
				death = FALSE;
			}

			/* Almost identical code -- Allow player to recall at point of death */
			if (p_ptr->ritual == 1)
			{
				/* Message */
				msg_print("Your ritual saves you from death.");
				msg_print(NULL);

				/* Restore hit points */
				p_ptr->chp = p_ptr->mhp;
				p_ptr->chp_frac = 0;

				/* Restore spell points */
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;

				/* Hack -- Healing */
				(void)set_timed_effect( TIMED_BLIND , 0);
				(void)set_timed_effect( TIMED_CONFUSED , 0);
				(void)set_timed_effect( TIMED_POISONED , 0);
				(void)set_timed_effect( TIMED_AFRAID , 0);
				(void)set_timed_effect( TIMED_PARALYZED , 0);
				(void)set_timed_effect( TIMED_IMAGE , 0);
				(void)set_timed_effect( TIMED_STUN, 0);
				(void)set_timed_effect( TIMED_CUT, 0);

				/* Hack -- Prevent starvation */
				(void)set_food(PY_FOOD_MAX - 1);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg_print("A tension leaves the air around you...");
					msg_print(NULL);

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Teleport to town */
				new_level_flag = TRUE;
				came_from=START_RANDOM;

				/* Go to town of recall */
				dun_level = 0;

				/* Do not die */
				death = FALSE;

				/* Ritual is used */
				p_ptr->ritual = 0;

				/* Lose most money and all items */
				/* You only keep the change in your pocket */
				if(p_ptr->au > 100) p_ptr->au = 100;
				destroy_pack();
			}
		}

		/* Handle "death" */
		if (death) break;

		/* Make a new level */
		generate_cave();
	}

	/* Close stuff */
	close_game();

	/* Quit */
	quit(NULL);
}
