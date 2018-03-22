/* File: wizard2.c */

/* Purpose: Wizard commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 *
 * James E. Wilson and Robert A. Koeneke and Ben Harrison have released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2),
 * or under the terms of the traditional Angband license.
 */

#include "angband.h"

void do_cmd_wiz_help(void);

/*
* Hack -- Rerate Hitpoints
*/
void do_cmd_rerate(void)
{
	int i;/* percent */;
	int lastroll,j;

	/* Pre-calculate level 1 hitdice */
	player_hp[0] = p_ptr->hitdie;

	/* Roll out the hitpoints */

	/* 'Roll' the hitpoint values */
	lastroll = p_ptr->hitdie;
	for (i = 1; i < PY_MAX_LEVEL; i++)
	{
		player_hp[i]=lastroll;
		lastroll--;
		if(lastroll<1) lastroll = p_ptr->hitdie;
	}
	/* Now shuffle them */
	for(i=1;i<PY_MAX_LEVEL;i++)
	{
		j=randint(PY_MAX_LEVEL-1);
		lastroll=player_hp[i];
		player_hp[i]=player_hp[j];
		player_hp[j]=lastroll;
	}
	/* Make each a cumulative score */
	for(i=1;i<PY_MAX_LEVEL;i++)
	{
		player_hp[i] = player_hp[i-1] +player_hp[i];
	}

	/*percent = (int)(((long)player_hp[PY_MAX_LEVEL - 1] * 200L) / (p_ptr->hitdie + ((PY_MAX_LEVEL - 1) * p_ptr->hitdie)));*/

	/* Update and redraw hitpoints */
	p_ptr->update |= (PU_HP);
	p_ptr->redraw |= (PR_HP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle stuff */
	handle_stuff();
}



#ifdef ALLOW_WIZARD


/*
* Create the artefact of the specified number -- DAN
*
*/
static void wiz_create_named_art(int a_idx)
{

	object_type forge;
	object_type *q_ptr;
	int i;

	artefact_type *a_ptr = &a_info[a_idx];

	/* Get local object */
	q_ptr = &forge;

	/* Wipe the object */
	object_wipe(q_ptr);

	/* Ignore "empty" artefacts */
	if (!a_ptr->name) return;

	/* Acquire the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return;

	/* Create the artefact */
	object_prep(q_ptr, i);

	/* Save the name */
	q_ptr->name1 = a_idx;

	/* Extract the fields */
	q_ptr->pval = a_ptr->pval;
	q_ptr->ac = a_ptr->ac;
	q_ptr->dd = a_ptr->dd;
	q_ptr->ds = a_ptr->ds;
	q_ptr->to_a = a_ptr->to_a;
	q_ptr->to_h = a_ptr->to_h;
	q_ptr->to_d = a_ptr->to_d;
	q_ptr->weight = a_ptr->weight;

	/* Hack -- acquire "cursed" flag */
	if (a_ptr->flags3 & (TR3_CURSED)) q_ptr->ident |= (IDENT_CURSED);

	random_artefact_resistance(q_ptr);

	/* Drop the artefact from heaven */
	drop_near(q_ptr, -1, py, px);

	/* All done */
	msg_print("Allocated.");
}





/*
* Hack -- quick debugging hook
*/
static void do_cmd_wiz_hack_ben(void)
{
	msg_print("No 'Wizard Hack' command coded.");
}



#ifdef MONSTER_HORDES
/* Summon a horde of monsters */
static void do_cmd_summon_horde()
{
	int wy = py, wx = px;
	int attempts = 1000;
	while (--attempts)
	{
		scatter(&wy, &wx, py, px, 3);
		if (cave_naked_bold(wy, wx)) break;
	}
	(void)alloc_horde(wy, wx);
}
#endif

/*
* Output a long int in binary format.
*/
static void prt_binary(u32b flags, int row, int col)
{
	int        	i;
	u32b        bitmask;

	/* Scan the flags */
	for (i = bitmask = 1; i <= 32; i++, bitmask *= 2)
	{
		/* Dump set bits */
		if (flags & bitmask)
		{
			Term_putch(col++, row, TERM_BLUE, '*');
		}

		/* Dump unset bits */
		else
		{
			Term_putch(col++, row, TERM_WHITE, '-');
		}
	}
}


/*
* Hack -- Teleport to the target
*/
static void do_cmd_wiz_bamf(void)
{
	/* Must have a target */
	if (!target_who) return;

	/* Teleport to the target */
	teleport_player_to(target_row, target_col);
}



/*
* Aux function for "do_cmd_wiz_change()".	-RAK-
*/
static void do_cmd_wiz_change_aux(void)
{
	int			i;

	int			tmp_int;

	long		tmp_long;

	char		tmp_val[160];

	char		ppp[80];


	/* Query the stats */
	for (i = 0; i < 6; i++)
	{
		/* Prompt */
		sprintf(ppp, "%s (3-118): ", stat_names[i]);

		/* Default */
		sprintf(tmp_val, "%d", p_ptr->stat_max[i]);

		/* Query */
		if (!get_string(ppp, tmp_val, 3)) return;

		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Verify */
		if (tmp_int > 18+100) tmp_int = 18+100;
		else if (tmp_int < 3) tmp_int = 3;

		/* Save it */
		p_ptr->stat_cur[i] = p_ptr->stat_max[i] = tmp_int;
	}


	/* Default */
	sprintf(tmp_val, "%ld", (long)(p_ptr->au));

	/* Query */
	if (!get_string("Gold: ", tmp_val, 9)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->au = tmp_long;


	/* Default */
	sprintf(tmp_val, "%ld", (long)(p_ptr->max_exp));

	/* Query */
	if (!get_string("Experience: ", tmp_val, 9)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->max_exp = tmp_long;

	/* Update */
	check_experience();
}


/*
* Change various "permanent" player variables.
*/
static void do_cmd_wiz_change(void)
{
	/* Interact */
	do_cmd_wiz_change_aux();

	/* Redraw everything */
	do_cmd_redraw();
}


/*
* Wizard routines for creating objects		-RAK-
* And for manipulating them!                   -Bernd-
*
* This has been rewritten to make the whole procedure
* of debugging objects much easier and more comfortable.
*
* The following functions are meant to play with objects:
* Create, modify, roll for them (for statistic purposes) and more.
* The original functions were by RAK.
* The function to show an item's debug information was written
* by David Reeve Sward <sward+@CMU.EDU>.
*                             Bernd (wiebelt@mathematik.hu-berlin.de)
*
* Here are the low-level functions
* - wiz_display_item()
*     display an item's debug-info
* - wiz_create_itemtype()
*     specify tval and sval (type and subtype of object)
* - wiz_tweak_item()
*     specify pval, +AC, +tohit, +todam
*     Note that the wizard can leave this function anytime,
*     thus accepting the default-values for the remaining values.
*     pval comes first now, since it is most important.
* - wiz_reroll_item()
*     apply some magic to the item or turn it into an artefact.
* - wiz_roll_item()
*     Get some statistics about the rarity of an item:
*     We create a lot of fake items and see if they are of the
*     same type (tval and sval), then we compare pval and +AC.
*     If the fake-item is better or equal it is counted.
*     Note that cursed items that are better or equal (absolute values)
*     are counted, too.
*     HINT: This is *very* useful for balancing the game!
* - wiz_quantity_item()
*     change the quantity of an item, but be sane about it.
*
* And now the high-level functions
* - do_cmd_wiz_play()
*     play with an existing object
* - wiz_create_item()
*     create a new object
*
* Note -- You do not have to specify "pval" and other item-properties
* directly. Just apply magic until you are satisfied with the item.
*
* Note -- For some items (such as wands, staffs, some rings, etc), you
* must apply magic, or you will get "broken" or "uncharged" objects.
*
* Note -- Redefining artefacts via "do_cmd_wiz_play()" may destroy
* the artefact.  Be careful.
*
* Hack -- this function will allow you to create multiple artefacts.
* This "feature" may induce crashes or other nasty effects.
*/

/*
* Just display an item's properties (debug-info)
* Originally by David Reeve Sward <sward+@CMU.EDU>
* Verbose item flags by -Bernd-
*/
static void wiz_display_item(object_type *o_ptr)
{
	int 	i, j = 13;

	u32b	f1, f2, f3;

	char        buf[256];


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Clear the screen */
	for (i = 1; i <= 23; i++) prt("", i, j - 2);

	/* Describe fully */
	object_desc_store(buf, o_ptr, TRUE, 3);

	prt(buf, 2, j);

	prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
		o_ptr->k_idx, k_info[o_ptr->k_idx].level,
		o_ptr->tval, o_ptr->sval), 4, j);

	prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
		o_ptr->number, o_ptr->weight,
		o_ptr->ac, o_ptr->dd, o_ptr->ds), 5, j);

	prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
		o_ptr->pval, o_ptr->to_a, o_ptr->to_h, o_ptr->to_d), 6, j);

	prt(format("name1 = %-4d  name2 = %-4d  cost = %ld,%ld ",
		o_ptr->name1, o_ptr->name2, (long)object_value(o_ptr) , object_value_real(o_ptr) ), 7, j);

	prt(format("ident = %04x  timeout = %-d",
		o_ptr->ident, o_ptr->timeout), 8, j);

	prt("+------------FLAGS1------------+", 10, j);
	prt("AFFECT........SLAY........BRAND.", 11, j);
	prt("              cvae      xsqpaefc", 12, j);
	prt("siwdcc  ssidsahanvudotgddhuoclio", 13, j);
	prt("tnieoh  trnipttmiinmrrnrrraiierl", 14, j);
	prt("rtsxna..lcfgdkcpmldncltggpksdced", 15, j);
	prt_binary(f1, 16, j);

	prt("+------------FLAGS2------------+", 17, j);
	prt("SUST....IMMUN.RESIST............", 18, j);
	prt("        aefcprpsaefcpfldbc sn   ", 19, j);
	prt("siwdcc  cliooeatcliooeialoshtncd", 20, j);
	prt("tnieoh  ierlifraierliatrnnnrhehi", 21, j);
	prt("rtsxna..dcedslatdcedsrekdfddrxss", 22, j);
	prt_binary(f2, 23, j);

	prt("+------------FLAGS3------------+", 10, j+32);
	prt("fe      ehsi  st    iiiiadta  hp", 11, j+32);
	prt("il   n taihnf ee    ggggcregb vr", 12, j+32);
	prt("re  nowysdose eld   nnnntalrl ym", 13, j+32);
	prt("ec  omrcyewta ieirmsrrrriieaeccc", 14, j+32);
	prt("aa  taauktmatlnpgeihaefcvnpvsuuu", 15, j+32);
	prt("uu  egirnyoahivaeggoclioaeoasrrr", 16, j+32);
	prt("rr  litsopdretitsehtierltxrtesss", 17, j+32);
	prt("aa  echewestreshtntsdcedeptedeee", 18, j+32);
	prt_binary(f3, 19, j+32);
}


/*
* A structure to hold a tval and its description
*/
typedef struct tval_desc
{
	int        tval;
	cptr       desc;
} tval_desc;

/*
* A list of tvals and their textual names
*/
static tval_desc tvals[] =
{
	{ TV_SWORD,             "Sword"                },
	{ TV_POLEARM,           "Polearm"              },
	{ TV_HAFTED,            "Hafted Weapon"        },
	{ TV_BOW,               "Bow"                  },
	{ TV_ARROW,             "Arrows"               },
	{ TV_BOLT,              "Bolts"                },
	{ TV_SHOT,              "Shots"                },
	{ TV_SHIELD,            "Shield"               },
	{ TV_CROWN,             "Crown"                },
	{ TV_HELM,              "Helm"                 },
	{ TV_GLOVES,            "Gloves"               },
	{ TV_BOOTS,             "Boots"                },
	{ TV_CLOAK,             "Cloak"                },
	{ TV_DRAG_ARMOR,        "Dragon Scale Mail"    },
	{ TV_HARD_ARMOR,        "Hard Armor"           },
	{ TV_SOFT_ARMOR,        "Soft Armor"           },
	{ TV_RING,              "Ring"                 },
	{ TV_AMULET,            "Amulet"               },
	{ TV_LITE,              "Lite"                 },
	{ TV_POTION,            "Potion"               },
	{ TV_SCROLL,            "Scroll"               },
	{ TV_WAND,              "Wand"                 },
	{ TV_STAFF,             "Staff"                },
	{ TV_ROD,               "Rod"                  },
	{ TV_MIRACLES_BOOK,     "Miracles Spellbook"   },
	{ TV_SORCERY_BOOK,      "Sorcery Spellbook"    },
	{ TV_NATURE_BOOK,       "Nature Spellbook"     },
	{ TV_CHAOS_BOOK,        "Chaos Spellbook"      },
	{ TV_DEATH_BOOK,        "Death Spellbook"      },
	{ TV_TAROT_BOOK,        "Tarot Spellbook"      },
	{ TV_CHARMS_BOOK,       "Charms Spellbook"     },
	{ TV_SOMATIC_BOOK,      "Somatic Spellbook"    },
	{ TV_DEMONIC_BOOK,      "Demonic Spellbook"    },
	{ TV_SPIKE,             "Spikes"               },
	{ TV_DIGGING,           "Digger"               },
	{ TV_CHEST,             "Chest"                },
	{ TV_FOOD,              "Food"                 },
	{ TV_FLASK,             "Flask"                },
	{ 0,                    NULL                   }
};


/*
* Strip an "object name" into a buffer
*/
static void strip_name(char *buf, int k_idx)
{
	char *t;

	object_kind *k_ptr = &k_info[k_idx];

	cptr str = (k_name + k_ptr->name);


	/* Skip past leading characters */
	while ((*str == ' ') || (*str == '&')) str++;

	/* Copy useful chars */
	for (t = buf; *str; str++)
	{
		if (*str != '~') *t++ = *str;
	}

	/* Terminate the new name */
	*t = '\0';
}


/*
 * Hack -- title for each column
 */
static char head[3] =
{ 'a', 'A', '0' };

/* Refactoring of the wiz_Create_item code so that I do not need to copy paste code */

static void drop_item( int k_idx )
{
	object_type	forge;
	object_type *q_ptr;
	
	/* Return if failed */
	if (!k_idx) return;
	
	/* Get local object */
	q_ptr = &forge;
	
	/* Create the item */
	object_prep(q_ptr, k_idx);
	
	/* Apply magic (no messages, no artefacts) */
	apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
	
	/* Drop the object from heaven */
	drop_near(q_ptr, -1, py, px);
	
	/* All done */
	msg_print("Allocated.");
	
	
}

/*
* Specify tval and sval (type and subtype of object) originally
* by RAK, heavily modified by -Bernd-
*
* This function returns the k_idx of an object type, or zero if failed
*
* List up to 50 choices in three columns
*/
static int wiz_create_itemtype(bool all_of_them)
{
	int  i, num, max_num;
	int  col, row;
	int  tval;

	cptr tval_desc;
	char ch;

	int  choice[60];

	char buf[160];


	/* Clear screen */
	Term_clear();

	/* Print all tval's and their descriptions */
	for (num = 0; (num < 60) && tvals[num].tval; num++)
	{
		row = 2 + (num % 20);
		col = 30 * (num / 20);
		ch = head[num/20] + (num%20);
		prt(format("[%c] %s", ch, tvals[num].desc), row, col);
	}

	/* Me need to know the maximal possible tval_index */
	max_num = num;

	/* Choose! */
	if (!get_com("Get what type of object? ", &ch)) return (0);

	/* Analyze choice */
	num = -1;
	if ((ch >= head[0]) && (ch < head[0] + 20)) num = ch - head[0];
	if ((ch >= head[1]) && (ch < head[1] + 20)) num = ch - head[1] + 20;
	if ((ch >= head[2]) && (ch < head[2] + 10)) num = ch - head[2] + 40;

	/* Bail out if choice is illegal */
	if ((num < 0) || (num >= max_num)) return (0);

	/* Base object type chosen, fill in tval */
	tval = tvals[num].tval;
	tval_desc = tvals[num].desc;


	/*** And now we go for k_idx ***/

	/* Clear screen */
	Term_clear();

	/* We have to search the whole itemlist. */
	for (num = 0, i = 1; (num < 60) && (i < MAX_K_IDX); i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Analyze matching items */
		if (k_ptr->tval == tval)
		{
			/* Hack -- Skip instant artefacts */
			if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

			/* Prepare it */
			row = 2 + (num % 20);
			col = 30 * (num / 20);
			ch = head[num/20] + (num%20);

			/* Acquire the "name" of object "i" */
			strip_name(buf, i);

			/* Print it */
			prt(format("[%c] %s", ch, buf), row, col);

			/* Remember the object index */
			choice[num++] = i;
            
            /* If we want them all, drop them all*/
            if(all_of_them)
                drop_item( i );
		}
	}

	/* Me need to know the maximal possible remembered object_index */
	max_num = num;

	/* Choose! */
	if (!get_com(format("What Kind of %s? ", tval_desc), &ch)) return (0);
	
	/* Analyze choice */
	num = -1;
	if ((ch >= head[0]) && (ch < head[0] + 20)){
		num = ch - head[0];
	} else if ((ch >= head[1]) && (ch < head[1] + 20)){
		num = ch - head[1] + 20;
	} else if ((ch >= head[2]) && (ch < head[2] + 20)){
		num = ch - head[2] + 40;
	} else if( ch == '*' ){
		/*If we want it all, press * */
		wiz_create_itemtype(TRUE);
	}
	/* Bail out if choice is "illegal" */
	if ((num < 0) || (num >= max_num))
		return (0);

	/* Otherwise return successful */
	return (choice[num]);
}


/*
* Tweak an item
*/
static void wiz_tweak_item(object_type *o_ptr)
{
	cptr	p;
	char        tmp_val[80];


	/* Hack -- leave artefacts alone */
	if (artefact_p(o_ptr) || o_ptr->art_name) return;

	p = "Enter new 'pval' setting: ";
	sprintf(tmp_val, "%d", o_ptr->pval);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->pval = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_a' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_a);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_a = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_h' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_h);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_h = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_d' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_d);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_d = atoi(tmp_val);
	wiz_display_item(o_ptr);
	
	p = "Enter new 'ego' setting: ";
	sprintf(tmp_val, "%d", o_ptr->name2);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->name2 = atoi(tmp_val);
	wiz_display_item(o_ptr);
}


/*
* Apply magic to an item or turn it into an artefact. -Bernd-
*/
static void wiz_reroll_item(object_type *o_ptr)
{
	object_type forge;
	object_type *q_ptr;

	char ch;

	bool changed = FALSE;


	/* Hack -- leave artefacts alone */
	if (artefact_p(o_ptr) || o_ptr->art_name) return;


	/* Get local object */
	q_ptr = &forge;

	/* Copy the object */
	object_copy(q_ptr, o_ptr);


	/* Main loop. Ask for magification and artefactification */
	while (TRUE)
	{
		/* Display full item debug information */
		wiz_display_item(q_ptr);

		/* Ask wizard what to do. */
		if (!get_com("[a]ccept, [n]ormal, [g]ood, [e]xcellent [r]andart? ", &ch))
		{
			changed = FALSE;
			break;
		}

		/* Create/change it! */
		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}

		/* Apply normal magic, but first clear object */
		else if (ch == 'n' || ch == 'N')
		{
			object_prep(q_ptr, o_ptr->k_idx);
			apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
		}

		/* Create a rand art, but first clear object, create it as normal, then rand art it */
		else if (ch == 'r' || ch == 'R')
		{
			object_prep(q_ptr, o_ptr->k_idx);
			apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
			create_artefact(q_ptr, FALSE);
		}

		/* Apply good magic, but first clear object */
		else if (ch == 'g' || ch == 'g')
		{
			object_prep(q_ptr, o_ptr->k_idx);
			apply_magic(q_ptr, dun_level, FALSE, TRUE, FALSE);
		}

		/* Apply great magic, but first clear object */
		else if (ch == 'e' || ch == 'e')
		{
			object_prep(q_ptr, o_ptr->k_idx);
			apply_magic(q_ptr, dun_level, FALSE, TRUE, TRUE);
		}
	}


	/* Notice change */
	if (changed)
	{
		/* Apply changes */
		object_copy(o_ptr, q_ptr);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);
	}
}



/*
* Maximum number of rolls
*/
#define TEST_ROLL 100000


/*
* Try to create an item again. Output some statistics.    -Bernd-
*
* The statistics are correct now.  We acquire a clean grid, and then
* repeatedly place an object in this grid, copying it into an item
* holder, and then deleting the object.  We fiddle with the artefact
* counter flags to prevent weirdness.  We use the items to collect
* statistics on item creation relative to the initial item.
*/
static void wiz_statistics(object_type *o_ptr)
{
	long i, matches, better, worse, other;

	char ch;
	char *quality;

	bool good, great;

	object_type forge;
	object_type	*q_ptr;

	cptr q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";


	/* XXX XXX XXX Mega-Hack -- allow multiple artefacts */
	if (artefact_p(o_ptr)) a_info[o_ptr->name1].cur_num = 0;


	/* Interact */
	while (TRUE)
	{
		cptr pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";

		/* Display item */
		wiz_display_item(o_ptr);

		/* Get choices */
		if (!get_com(pmt, &ch)) break;

		if (ch == 'n' || ch == 'N')
		{
			good = FALSE;
			great = FALSE;
			quality = "normal";
		}
		else if (ch == 'g' || ch == 'G')
		{
			good = TRUE;
			great = FALSE;
			quality = "good";
		}
		else if (ch == 'e' || ch == 'E')
		{
			good = TRUE;
			great = TRUE;
			quality = "excellent";
		}
		else
		{
			good = FALSE;
			great = FALSE;
			break;
		}

		/* Let us know what we are doing */
		msg_format("Creating a lot of %s items. Base level = %d.",
			quality, dun_level);
		msg_print(NULL);

		/* Set counters to zero */
		matches = better = worse = other = 0;

		/* Let's rock and roll */
		for (i = 0; i <= TEST_ROLL; i++)
		{
			/* Output every few rolls */
			if ((i < 100) || (i % 100 == 0))
			{
				/* Do not wait */
				inkey_scan = TRUE;

				/* Allow interupt */
				if (inkey())
				{
					/* Flush */
					flush();

					/* Stop rolling */
					break;
				}

				/* Dump the stats */
				prt(format(q, i, matches, better, worse, other), 0, 0);
				Term_fresh();
			}


			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Create an object */
			make_object(q_ptr, good, great);


			/* XXX XXX XXX Mega-Hack -- allow multiple artefacts */
			if (artefact_p(q_ptr)) a_info[q_ptr->name1].cur_num = 0;


			/* Test for the same tval and sval. */
			if ((o_ptr->tval) != (q_ptr->tval)) continue;
			if ((o_ptr->sval) != (q_ptr->sval)) continue;

			/* Check for match */
			if ((q_ptr->pval == o_ptr->pval) &&
				(q_ptr->to_a == o_ptr->to_a) &&
				(q_ptr->to_h == o_ptr->to_h) &&
				(q_ptr->to_d == o_ptr->to_d))
			{
				matches++;
			}

			/* Check for better */
			else if ((q_ptr->pval >= o_ptr->pval) &&
				(q_ptr->to_a >= o_ptr->to_a) &&
				(q_ptr->to_h >= o_ptr->to_h) &&
				(q_ptr->to_d >= o_ptr->to_d))
			{
				better++;
			}

			/* Check for worse */
			else if ((q_ptr->pval <= o_ptr->pval) &&
				(q_ptr->to_a <= o_ptr->to_a) &&
				(q_ptr->to_h <= o_ptr->to_h) &&
				(q_ptr->to_d <= o_ptr->to_d))
			{
				worse++;
			}

			/* Assume different */
			else
			{
				other++;
			}
		}

		/* Final dump */
		msg_format(q, i, matches, better, worse, other);
		msg_print(NULL);
	}


	/* Hack -- Normally only make a single artefact */
	if (artefact_p(o_ptr)) a_info[o_ptr->name1].cur_num = 1;
}


/*
* Change the quantity of a the item
*/
static void wiz_quantity_item(object_type *o_ptr)
{
	int         tmp_int;

	char        tmp_val[100];


	/* Never duplicate artefacts */
	if (artefact_p(o_ptr) || o_ptr->art_name) return;


	/* Default */
	sprintf(tmp_val, "%d", o_ptr->number);

	/* Query */
	if (get_string("Quantity: ", tmp_val, 2))
	{
		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Paranoia */
		if (tmp_int < 1) tmp_int = 1;
		if (tmp_int > 99) tmp_int = 99;

		/* Accept modifications */
		o_ptr->number = tmp_int;
	}
}



/*
* Play with an item. Options include:
*   - Output statistics (via wiz_roll_item)
*   - Reroll item (via wiz_reroll_item)
*   - Change properties (via wiz_tweak_item)
*   - Change the number of items (via wiz_quantity_item)
*/
static void do_cmd_wiz_play(void)
{
	int item;

	object_type	forge;
	object_type *q_ptr;

	object_type *o_ptr;

	char ch;

	bool changed;


	/* Get an item (from equip or inven) */
	if (!get_item(&item, "Play with which object? ", "You have nothing to play with." , USE_EQUIP | USE_INVEN | USE_FLOOR))
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


	/* The item was not changed */
	changed = FALSE;


	/* Icky */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Get local object */
	q_ptr = &forge;

	/* Copy object */
	object_copy(q_ptr, o_ptr);


	/* The main loop */
	while (TRUE)
	{
		/* Display the item */
		wiz_display_item(q_ptr);

		/* Get choice */
		if (!get_com("[a]ccept [s]tatistics [r]eroll [t]weak [q]uantity? ", &ch))
		{
			changed = FALSE;
			break;
		}

		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}

		if (ch == 's' || ch == 'S')
		{
			wiz_statistics(q_ptr);
		}

		if (ch == 'r' || ch == 'r')
		{
			wiz_reroll_item(q_ptr);
		}

		if (ch == 't' || ch == 'T')
		{
			wiz_tweak_item(q_ptr);
		}

		if (ch == 'q' || ch == 'Q')
		{
			wiz_quantity_item(q_ptr);
		}
	}


	/* Restore the screen */
	Term_load();

	/* Not Icky */
	character_icky = FALSE;


	/* Accept change */
	if (changed)
	{
		/* Message */
		msg_print("Changes accepted.");

		/* Change */
		object_copy(o_ptr, q_ptr);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);
	}

	/* Ignore change */
	else
	{
		msg_print("Changes ignored.");
	}
}

/*
* Wizard routine for mimicking patron rewards
*/
static void wiz_mimic_patron_reward( void )
{

	int choice = 0;
	int  result = 0;
	int i = 0;

	/* Icky */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Clear the screen */
	Term_clear();

	for( i = 0 ; patron_rewards[i].description != NULL ; i++ )
	{
		prt( format("%2d, %s" , i , patron_rewards[i].description ) , 2 + (i%20) , 1 + (i/20)*30 );
	}
	choice = get_quantity("Which reward ?", 35 , 0 );

	/* Restore the screen */
	Term_load();

	/* Execute the reward */
	result = patron_rewards[ choice ].func();

	/* Not Icky */
	character_icky = FALSE;

	if( !result )
		msg_format( "The reward was pointless" );

}


/*
* Wizard routine for creating objects		-RAK-
* Heavily modified to allow magification and artefactification  -Bernd-
*
* Note that wizards cannot create objects on top of other objects.
*
* Hack -- this routine always makes a "dungeon object", and applies
* magic to it, and attempts to decline cursed items.
*/
static void wiz_create_item(void)
{
	int k_idx;

	/* Icky */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Get object base type */
	k_idx = wiz_create_itemtype(FALSE);
	
	/* Restore the screen */
	Term_load();
	
	/* Not Icky */
	character_icky = FALSE;
	
	drop_item( k_idx );

}


/*
 *  Set up the ultimate supply stack for dungeon wandering
*/
static void do_cmd_wiz_supplies(void)
{
	int i;
	/* Lets get 15 copies of the essentials */
	for (i = 0; i < 15; i++)
	{
		drop_item( 200 ); /* Scroll of mass banishment */
		drop_item( 422 ); /* potion of *enlightenment */
		drop_item( 419 ); /* potion of *healing */
		drop_item( 266 ); /* potion of restore mana */
		drop_item( 249 ); /* potion of speed */
		drop_item( 249 ); /* potion of speed */
		drop_item( 221 ); /* *destruction* */
	}
	drop_item( 79 ); /* seeker arrow */
	drop_item( 81 ); /* seeker bolt */
}


static void do_cmd_perma_prison_aux(int lx , int ly)
{
	cave_type		*c_ptr;
	
	/* Access the grid */
	c_ptr = &cave[ly][lx];

	/*Delete monsters*/
	delete_monster(ly, lx);
		
	/* Delete objects */
	delete_object(ly, lx);
				
	/* Access the grid */
	c_ptr = &cave[ly][lx];
				
	/* Create floor */
	c_ptr->feat = FEAT_PERM_BUILDING;
				
	/* No longer part of a room or vault */
	c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
				
	/* No longer illuminated or known */
	c_ptr->info |= (CAVE_MARK | CAVE_GLOW);
}


/*
 * Sets up a prison for the player to test dropping
 */
static void do_cmd_perma_prison(void)
{


	do_cmd_perma_prison_aux( px+1,py );
	do_cmd_perma_prison_aux( px-1,py );
	
	do_cmd_perma_prison_aux( px+1,py+1 );
	do_cmd_perma_prison_aux( px-1,py-1 );
	do_cmd_perma_prison_aux( px+1,py-1 );
	do_cmd_perma_prison_aux( px-1,py+1 );
	
	do_cmd_perma_prison_aux( px,py+1 );
	do_cmd_perma_prison_aux( px,py-1 );
	
	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
	
	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);
	
	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);
	
	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
	
}

/*
 * Learn all spells ;)
 */
static void do_cmd_study_all(void)
{
	int i;
	/* And much was accomplished */
	spell_learned1 = 0xFFFFFFFF;
	spell_learned2 = 0xFFFFFFFF;

	/* Make sure that forgetting/remembering works*/
	for (i = 0; i < 64; i++)
		spell_order[i] = i;
}

static void do_cmd_parse(void)
{
	char		in[SCRIPT_MAX_LENGTH];
	double		answer;
	
	dice_mode = BEST_CASE;
	
	/* Get a new inscription (possibly empty) */
	if (get_string("Expression (max length 80) : ", in, SCRIPT_MAX_LENGTH))
	{
		/*Point at the start*/
		script = &in[0];
		/*Evaluate*/
		while(*script)
		{
			eval_script(&answer);
			/*Show & Tell*/
			msg_format("Answer for %s is: %.0f", variable_token, answer);
			/*There might be more*/
			if(*script==';')script++;
		}
	}
}

/**END**/

/*
 * Set up an ultra debug character
 */
static void do_cmd_wiz_michael(void)
{
	int			tmp_int;
	int			i;
	/* Set all stats to maximum */
	tmp_int = 18+100;
	for (i = 0; i < 6; i++)
	{
		p_ptr->stat_cur[i] = p_ptr->stat_max[i] = tmp_int;
   }
   wiz_create_named_art( 87 ); /*Longsword of Michael*/
   wiz_create_named_art( 34 ); /*/Bloodstained Armour of Saint Michael*/
   wiz_create_named_art( 46 ); /*/Crown of the Seventh Day*/
   wiz_create_named_art( 128 ); /*/Crossbow of Death*/
   wiz_create_named_art( 3 );   /*/Gem of Hippo*/
   wiz_create_named_art( 35 ); /*/Shield of Saint Michael*/
   wiz_create_named_art( 54 ); /*/Cloak Of the Frost Plains*/
   wiz_create_named_art( 65 ); /*/Boots of Furcifer*/
   wiz_create_named_art( 62 ); /*/cesti of the Grim Reaper*/
   wiz_create_named_art( 14 ); /*/The First Ring*/
   wiz_create_named_art( 12 ); /*/Ring of sheating*/
   wiz_create_named_art( 5 );  /*/Amulet of Raphael*/
   wiz_create_named_art( 24 );  /*/Robe of Gabriel for spellcasters*/
   
   p_ptr->au = 12345678;

}

/* This is a hack apparently, I wouldnt know ;)  */
extern void do_cmd_wiz_cure_all(void);


/*
* Cure everything instantly
*/
void do_cmd_wiz_cure_all(void)
{
	/* Remove curses */
	(void)remove_all_curse();

	/* Restore stats */
	(void)res_stat(A_STR);
	(void)res_stat(A_INT);
	(void)res_stat(A_WIS);
	(void)res_stat(A_CON);
	(void)res_stat(A_DEX);
	(void)res_stat(A_CHA);

	/* Restore the level */
	(void)restore_level();

	/* Heal the player */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Restore mana */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* Cure stuff */
	(void)set_timed_effect( TIMED_BLIND , 0);
	(void)set_timed_effect( TIMED_CONFUSED , 0);
	(void)set_timed_effect( TIMED_POISONED , 0);
	(void)set_timed_effect( TIMED_AFRAID , 0);
	(void)set_timed_effect( TIMED_PARALYZED , 0);
	(void)set_timed_effect( TIMED_IMAGE , 0);
	(void)set_timed_effect( TIMED_STUN, 0);
	(void)set_timed_effect( TIMED_CUT, 0);
	(void)set_timed_effect( TIMED_SLOW, 0);

	/* No longer hungry */
	(void)set_food(PY_FOOD_MAX - 1);

	/* Redraw everything */
	do_cmd_redraw();
}


/*
* Go to any level
*/
static void do_cmd_wiz_jump(void)
{
	/* Ask for level */
	if (command_arg <= 0)
	{
		char	ppp[80];

		char	tmp_val[160];

		/* Prompt */
		sprintf(ppp, "Jump to level (0-%d): ", MAX_DEPTH);

		/* Default */
		sprintf(tmp_val, "%d", dun_level);

		/* Ask for a level */
		if (!get_string(ppp, tmp_val, 10)) return;

		/* Extract request */
		command_arg = atoi(tmp_val);
	}

	/* Paranoia */
	if (command_arg < 1) command_arg =1;

	/* Paranoia */
	if (command_arg > MAX_DEPTH) command_arg = MAX_DEPTH;

	/* Accept request */
	msg_format("You jump to dungeon level %d.", command_arg);

	if (autosave_l)
	{
		is_autosave = TRUE;
		msg_print("Autosaving the game...");
		do_cmd_save_game();
		is_autosave = FALSE;
	}

	/* Change level */
	dun_level = command_arg;
	new_level_flag = TRUE;
}

/*
* Become aware of a lot of objects
*/
static void do_cmd_wiz_learn(void)
{
	int i;

	object_type forge;
	object_type *q_ptr;

	/* Scan every object */
	for (i = 1; i < MAX_K_IDX; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Induce awareness */
		if (k_ptr->level <= command_arg)
		{
			/* Get local object */
			q_ptr = &forge;

			/* Prepare object */
			object_prep(q_ptr, i);

			/* Awareness */
			object_aware(q_ptr);
		}
	}
}


/*
* Summon some creatures
*/
static void do_cmd_wiz_summon(int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		(void)summon_specific(py, px, dun_level, 0);
	}
}


/*
* Summon a creature of the specified type
*
* XXX XXX XXX This function is rather dangerous
*/
static void do_cmd_wiz_named(int r_idx, int slp)
{
	int i, x, y;

	/* Paranoia */
	/* if (!r_idx) return; */

	/* Prevent illegal monsters */
	if (r_idx >= MAX_R_IDX-1) return;

	/* Try 10 times */
	for (i = 0; i < 10; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, py, px, d);

		/* Require empty grids */
		/*if (!cave_empty_bold(y, x) || (cave[y][x].feat == FEAT_WATER && !water_ok(r_idx) )) continue;*/
		if(!can_place_monster(y,x,r_idx))continue;

		/* Place it (allow groups) */
		if (place_monster_aux(y, x, r_idx, (bool)slp, (bool)TRUE, (bool)FALSE)) break;
	}
}


/*
* Summon a creature of the specified type
*
* XXX XXX XXX This function is rather dangerous
*/
static void do_cmd_wiz_named_friendly(int r_idx, int slp)
{
	int i, x, y;

	/* Paranoia */
	/* if (!r_idx) return; */

	/* Prevent illegal monsters */
	if (r_idx >= MAX_R_IDX-1) return;

	/* Try 10 times */
	for (i = 0; i < 10; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, py, px, d);

		/* Require empty grids */
		/*if (!cave_empty_bold(y, x) || (cave[y][x].feat == FEAT_WATER && !water_ok(r_idx))) continue;*/
		if(!can_place_monster(y,x,r_idx))continue;

		/* Place it (allow groups) */
		if (place_monster_aux(y, x, r_idx, (bool)slp, (bool)TRUE, (bool)TRUE)) break;
	}
}

/*
* Hack -- Delete all nearby monsters
*/
static void do_cmd_wiz_zap(void)
{
	int        i;

	/* Genocide everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Delete nearby monsters */
		if (m_ptr->cdis <= MAX_SIGHT) delete_monster_idx(i,TRUE);
	}
}

/*
* Fire a magebolt at a creature that does 'enough' damage
*/
static void do_cmd_magebolt()
{
	int dir;
	int tx, ty;
	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Get a direction */
	if (!get_aim_dir(&dir)) return;

	/* Use the given direction */
	tx = px + 99 * ddx[dir];
	ty = py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	project(0, 0, ty, tx, 1000000, GF_MANA, flg);
}


#ifdef ALLOW_SPOILERS

/*
* External function
*/
extern void do_cmd_spoilers(void);

#endif





/*
* Hack -- declare external function
*/
extern void do_cmd_debug(void);



/*
* Ask for and parse a "debug command"
* The "command_arg" may have been set.
*/
void do_cmd_debug(void)
{
	char		cmd;


	/* Get a "debug command" */
	(void)(get_com("Debug Command: ", &cmd));

	/* Analyze the command */
	switch (cmd)
	{
		/* Nothing */
	case ESCAPE:
	case ' ':
	case '\n':
	case '\r':
		break;


#ifdef ALLOW_SPOILERS

		/* Hack -- Generate Spoilers */
	case '"':
		do_cmd_spoilers();
		break;

#endif


		/* Hack -- Help */
	case '?':
		do_cmd_wiz_help();
		break;
	case ']':
		lose_all_info();
		break;
	case '0':
		do_cmd_perma_prison();
		break;
	case '1':
		do_cmd_study_all();
		break;
	case '2':
		do_cmd_parse();
		break;
		/* Allow player to dress to kill, naming the character Michael */
	case 'A':
		do_cmd_wiz_michael();
		break;
		/* Cure all maladies */
	case 'a':
		do_cmd_wiz_cure_all();
		break;

		/* Teleport to target */
	case 'b':
		do_cmd_wiz_bamf();
		break;

		/* Create any object */
	case 'c':
		wiz_create_item();
		break;


		/* Create a named artefact */
	case 'C':
		wiz_create_named_art(command_arg);
		break;

		/* Detect everything */
	case 'd':
		detect_all();
		break;

		/* Edit character */
	case 'e':
		do_cmd_wiz_change();
		break;

		/* View item info */
	case 'f':
		(void)identify_fully();
		break;

		/* Good Objects */
	case 'g':
		if (command_arg <= 0) command_arg = 1;
		acquirement(py, px, command_arg, FALSE);
		break;

		/* Hitpoint rerating */
	case 'h':
		do_cmd_rerate(); break;

#ifdef MONSTER_HORDES
	case 'H':
		do_cmd_summon_horde(); break;
#endif

		/* Identify */
	case 'i':
		identify_pack();
		break;

		/* Go up or down in the dungeon */
	case 'j':
		do_cmd_wiz_jump();
		break;

		/* Self-Knowledge */
	case 'k':
		self_knowledge();
		break;

		/* Learn about objects */
	case 'l':
		do_cmd_wiz_learn();
		break;

		/* Magic Mapping */
	case 'm':
		map_area();
		break;

		/* Corruption */
	case 'M':
		(void) gain_corruption(command_arg);
		break;
		/* Corruption */
	case '-':
		(void) lose_corruption(command_arg);
		break;
		/* Specific reward */
	case 'r':
		/*(void) gain_level_reward(command_arg);*/
		(void) wiz_mimic_patron_reward();
		break;
		/* Summon _friendly_ named monster */
	case 'N':
		do_cmd_wiz_named_friendly(command_arg, TRUE);
		break;

		/* Summon Named Monster */
	case 'n':
		do_cmd_wiz_named(command_arg, TRUE);
		break;


		/* Object playing routines */
	case 'o':
		do_cmd_wiz_play();
		break;

		/* Phase Door */
	case 'p':
		teleport_player(10);
		break;

		/* Summon Random Monster(s) */
	case 's':
		if (command_arg <= 0) command_arg = 1;
		do_cmd_wiz_summon(command_arg);
		break;
		
	case 'S':
		do_cmd_wiz_supplies();
		break;

		/* Teleport */
	case 't':
		teleport_player(100);
		break;

		/* Very Good Objects */
	case 'v':
		if (command_arg <= 0) command_arg = 1;
		acquirement(py, px, command_arg, TRUE);
		break;

		/* Wizard Light the Level */
	case 'w':
		wiz_lite();
		break;

		/* Increase Experience */
	case 'x':
		if (command_arg)
		{
			gain_exp(command_arg);
		}
		else
		{
			gain_exp(p_ptr->exp + 1);
		}
		break;

		/* Zap Monsters (Genocide) */
	case 'Z':
		do_cmd_wiz_zap();
		break;

	case 'z':
		{
			do_cmd_magebolt();
			break;
		}

		/* Hack -- whatever I desire */
	case '_':
		do_cmd_wiz_hack_ben();
		break;

		/* Not a Wizard Command */
	default:
		msg_print("That is not a valid wizard command.");
		break;
	}
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

void do_cmd_wiz_help(void)
{
	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Flush */
	Term_fresh();

	/* Clear the screen */
	Term_clear();

	c_put_str(TERM_RED,"Debug Commands",1,32);
	c_put_str(TERM_RED,"==============",2,32);

	c_put_str(TERM_RED,"Character Editing",4,1);
	c_put_str(TERM_RED,"=================",5,1);
	put_str("a = Cure All",7,1);
	put_str("e = Edit Stats",8,1);
	put_str("h = Reroll Hitpoints",9,1);
	put_str("k = Self Knowledge",10,1);
	put_str("M = Gain Corruption",11,1);
	put_str("r = Gain Level Reward",12,1);
	put_str("x = Gain Experience",13,1);
	put_str("- = Loose Corruption",14,1);

	c_put_str(TERM_RED,"Movement",15,1);
	c_put_str(TERM_RED,"========",16,1);
	put_str("b = Teleport to Target",18,1);
	put_str("j = Jump Levels",19,1);
	put_str("p = Phase Door",20,1);
	put_str("t = Teleport",21,1);

	c_put_str(TERM_RED,"Monsters",4,26);
	c_put_str(TERM_RED,"========",5,26);
	put_str("s = Summon Monster",7,26);
	put_str("n = Summon Named Monster",8,26);
	put_str("N = Summon Named Ally",9,26);
	put_str("H = Summon Horde",10,26);
	put_str("Z = Genocide True",11,26);
	put_str("z = Zap (Magebolt)",12,26);

	c_put_str(TERM_RED,"General Commands",14,26);
	c_put_str(TERM_RED,"================",15,26);
	put_str("] = Induce Amnesia",16,26);
	put_str("\" = Generate spoilers",17,26);
	put_str("d = Detect All",18,26);
	put_str("m = Map Area",19,26);
	put_str("w = Wizard Light",20,26);
	put_str("0 = Create Perm Prison",21,26);
	put_str("1 = Learn All Spells",22,26);

	c_put_str(TERM_RED,"Object Commands",4,51);
	c_put_str(TERM_RED,"===============",5,51);
	put_str("c = Create Item",7,51);
	put_str("C = Create Named Artifact",8,51);
	put_str("f = Identify Fully",9,51);
	put_str("g = Generate Good Object",10,51);
	put_str("i = Identify Pack",11,51);
	put_str("l = Learn About Objects",12,51);
	put_str("o = Object Editor",13,51);
	put_str("v = Generate Very Good Object",14,51);
	put_str("S = Drop Endgame supplies",15,51);
	put_str("A = Drop Ultimate Kit",16,51);

	/* Wait for it */
	put_str("Hit any key to continue", 23, 23);

	/* Get any key */
	inkey();

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}
