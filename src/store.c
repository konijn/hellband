/* File: store.c */

/* Purpose: Store commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 *
 * James E. Wilson and Robert A. Koeneke have released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2),
 * or under the terms of the traditional Angband license.
 */

#include "angband.h"

#define RUMOR_CHANCE 8

#define MAX_COMMENT_1	6

/*
* We store the current "store number" here so everyone can access it
*/
static int cur_store_num = 0;

/*
* We store the current "store page" here so everyone can access it
*/
static int store_top = 0;

/*
* We store the current "store pointer" here so everyone can access it
*/
static store_type *st_ptr = NULL;

/*
* We store the current "owner type" here so everyone can access it
*/
static owner_type *ot_ptr = NULL;

static cptr comment_1[MAX_COMMENT_1] =
{
		"Okay.",
		"Fine.",
		"Accepted!",
		"Agreed!",
		"Done!",
		"Taken!"
};

#define MAX_COMMENT_2A	2

static cptr comment_2a[MAX_COMMENT_2A] =
{
	"You try my patience.  %s is final.",
		"My patience grows thin.  %s is final."
};

#define MAX_COMMENT_2B	12

static cptr comment_2b[MAX_COMMENT_2B] =
{
		"I can take no less than %s gold pieces.",
		"I will accept no less than %s gold pieces.",
		"Ha!  No less than %s gold pieces.",
		"You knave!  No less than %s gold pieces.",
		"That's a pittance!  I want %s gold pieces.",
		"That's an insult!  I want %s gold pieces.",
		"As if!  How about %s gold pieces?",
		"My arse!  How about %s gold pieces?",
		"May the fleas of 1000 orcs molest you!  Try %s gold pieces.",
		"May your most favourite parts go moldy!  Try %s gold pieces.",
		"May Cerberus find you tasty!  Perhaps %s gold pieces?",
		"Your mother was an Ogre!  Perhaps %s gold pieces?"
};

#define MAX_COMMENT_3A	2

static cptr comment_3a[MAX_COMMENT_3A] =
{
		"You try my patience.  %s is final.",
		"My patience grows thin.  %s is final."
};


#define MAX_COMMENT_3B	12

static cptr comment_3b[MAX_COMMENT_3B] =
{
		"Perhaps %s gold pieces?",
		"How about %s gold pieces?",
		"I will pay no more than %s gold pieces.",
		"I can afford no more than %s gold pieces.",
		"Be reasonable.  How about %s gold pieces?",
		"I'll buy it as scrap for %s gold pieces.",
		"That is too much!  How about %s gold pieces?",
		"That looks war surplus!  Say %s gold pieces?",
		"Never!  %s is more like it.",
		"That's an insult!  %s is more like it.",
		"%s gold pieces and be thankful for it!",
		"%s gold pieces and not a copper more!"
};

#define MAX_COMMENT_4A	4

static cptr comment_4a[MAX_COMMENT_4A] =
{
		"Enough!  You have abused me once too often!",
		"Arghhh!  I have had enough abuse for one day!",
		"That does it!  You shall waste my time no more!",
		"This is getting nowhere!"
};

#define MAX_COMMENT_4B	4

static cptr comment_4b[MAX_COMMENT_4B] =
{
		"Leave my store!",
		"Get out of my sight!",
		"Begone, you scoundrel!",
		"Out, out, out!"
};

#define MAX_COMMENT_5	8

static cptr comment_5[MAX_COMMENT_5] =
{
		"Try again.",
		"Ridiculous!",
		"You will have to do better than that!",
		"Do you wish to do business or not?",
		"You've got to be kidding!",
		"You'd better be kidding!",
		"You try my patience.",
		"Hmmm, nice weather we're having."
};

#define MAX_COMMENT_6	4

static cptr comment_6[MAX_COMMENT_6] =
{
		"I must have heard you wrong.",
		"I'm sorry, I missed that.",
		"I'm sorry, what was that?",
		"Sorry, what was that again?"
};

/*
* Rest for a night in the Inn or House
* If the night flag is set, then rest until nightfalll instead of daybreak
*/
void room_rest(bool night)
{
	int n;
	int temp_store_num;
	if(night)
	{
		if (turn < 50000)
		{
			turn = 50000;
		}
		else
		{
			turn = (((turn - 50000)/100000)+1)*100000 + 50000;
		}
	}
	else
	{
		turn = ((turn/100000)+1)*100000;
	}
	p_ptr->chp = p_ptr->mhp;
	set_timed_effect( TIMED_BLIND , 0);
	set_timed_effect( TIMED_CONFUSED , 0);
	p_ptr->stun = 0;
	if(night)
	{
		msg_print("You awake, ready for the night.");
	}
	else
	{
		msg_print("You awake refreshed for the new day.");
	}
	/* Store globals to stop them getting corrupted */
	temp_store_num=cur_store_num;

	/* Maintain each shop (except homes etc) once */
	store_maint_all(1);

	/* And restore the globals */
	cur_store_num=temp_store_num;
	/* Reset the Store and Owner pointers */
	st_ptr = &store[cur_store_num];
	ot_ptr = &owners[cur_store_num][st_ptr->owner];
	/* Reset all timed effects */
	for( n = 0 ; n < TIMED_COUNT ; n ++ )
	{
		/*msg_note( timed[n].lose );*/
		*(timed[n].timer) = 0;
	}

	new_level_flag = TRUE;
	came_from = START_WALK; /* We don't want the player to be moved */
}

/*
* Free Homes - check if the town has any homes for sale
*/

bool free_homes(void)
{
	if(store[STORE_HOME].bought == 0)
	{
		return (TRUE);
	}
	return (FALSE);
}

/*
* Successful haggle.
*/
static void say_comment_1(void)
{
	/*char rumour[80];  */
	msg_print(comment_1[rand_int(MAX_COMMENT_1)]);
}

/*
* Continue haggling (player is buying)
*/
static void say_comment_2(s32b value, int annoyed)
{
	char	tmp_val[80];

	/* Prepare a string to insert */
	sprintf(tmp_val, "%ld", (long)value);

	/* Final offer */
	if (annoyed > 0)
	{
		/* Formatted message */
		msg_format(comment_2a[rand_int(MAX_COMMENT_2A)], tmp_val);
	}

	/* Normal offer */
	else
	{
		/* Formatted message */
		msg_format(comment_2b[rand_int(MAX_COMMENT_2B)], tmp_val);
	}
}

/*
* Continue haggling (player is selling)
*/
static void say_comment_3(s32b value, int annoyed)
{
	char	tmp_val[80];

	/* Prepare a string to insert */
	sprintf(tmp_val, "%ld", (long)value);

	/* Final offer */
	if (annoyed > 0)
	{
		/* Formatted message */
		msg_format(comment_3a[rand_int(MAX_COMMENT_3A)], tmp_val);
	}

	/* Normal offer */
	else
	{
		/* Formatted message */
		msg_format(comment_3b[rand_int(MAX_COMMENT_3B)], tmp_val);
	}
}


/*
* Kick 'da bum out.					-RAK-
*/
static void say_comment_4(void)
{
	msg_print(comment_4a[rand_int(MAX_COMMENT_4A)]);
	msg_print(comment_4b[rand_int(MAX_COMMENT_4B)]);
}


/*
* You are insulting me
*/
static void say_comment_5(void)
{
	msg_print(comment_5[rand_int(MAX_COMMENT_5)]);
}


/*
* That makes no sense.
*/
static void say_comment_6(void)
{
	msg_print(comment_6[rand_int(5)]);
}

/*
* Messages for reacting to purchase prices.
*/

#define MAX_COMMENT_7A	4

static cptr comment_7a[MAX_COMMENT_7A] =
{
		"Arrgghh!",
		"You bastard!",
		"You hear someone sobbing...",
		"The shopkeeper howls in agony!"
};

#define MAX_COMMENT_7B	4

static cptr comment_7b[MAX_COMMENT_7B] =
{
		"Damn!",
		"You bastard!",
		"The shopkeeper curses at you.",
		"The shopkeeper glares at you."
};

#define MAX_COMMENT_7C	4

static cptr comment_7c[MAX_COMMENT_7C] =
{
		"Cool!",
		"You've made my day!",
		"The shopkeeper giggles.",
		"The shopkeeper laughs loudly."
};

#define MAX_COMMENT_7D	4

static cptr comment_7d[MAX_COMMENT_7D] =
{
		"Yipee!",
		"I think I'll retire!",
		"The shopkeeper jumps for joy.",
		"The shopkeeper smiles gleefully."
};

/*
* Let a shop-keeper React to a purchase
*
* We paid "price", it was worth "value", and we thought it was worth "guess"
*/
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
	/* Item was worthless, but we bought it */
	if ((value <= 0) && (price > value))
	{
		/* Comment */
		msg_print(comment_7a[rand_int(MAX_COMMENT_7A)]);

		/* Sound */
		sound(SOUND_STORE1);
	}

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
	{
		/* Comment */
		msg_print(comment_7b[rand_int(MAX_COMMENT_7B)]);

		/* Sound */
		sound(SOUND_STORE2);
	}

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
	{
		/* Comment */
		msg_print(comment_7c[rand_int(MAX_COMMENT_7C)]);

		/* Sound */
		sound(SOUND_STORE3);
	}

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
	{
		/* Comment */
		msg_print(comment_7d[rand_int(MAX_COMMENT_7D)]);

		/* Sound */
		sound(SOUND_STORE4);
	}
}

/*
* Buying and selling adjustments for race combinations.
* Entry[owner][player] gives the basic "cost inflation".
*/
static byte rgold_adj[COUNT_SUBRACES][COUNT_SUBRACES] =
{
			  	  /*  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27 */
				  /*Flo, Gip, Nor, Atl, Dwa, Elf, Ogr, Tro, Gia, Tit, Nep, Aff,	Fae, Gno, Lep, Kob, Dev, Imp, Suc, Lil, Eld, Gua, Hor, Vam, Wer, Ske, Mum, Spe */

/* Florentian  0*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Gipsy       1*/ { 90,  90, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Nordic      2*/ { 90, 105,  90, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Atlantian   3*/ { 90, 105, 105,  90, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Dwarf       4*/ { 90, 105, 105, 110,  90, 115, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Elf         5*/ { 90, 105, 105, 110, 130,  90, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Ogre        6*/ {115, 115, 115, 115, 115, 115,  90,  90,  90, 100, 100, 100, 100, 105, 100, 100, 125, 125, 125, 125, 100, 115, 130, 130, 100, 115, 115, 115 },
/* Troll       7*/ {115, 115, 115, 115, 115, 115,  90,  90,  90, 100, 100, 100, 100, 105, 100, 100, 125, 125, 125, 125, 100, 115, 130, 130, 100, 115, 115, 115 },
/* Giant       8*/ {115, 115, 115, 115, 115, 115,  90,  90,  90, 100, 100, 100, 100, 105, 100, 100, 125, 125, 125, 125, 100, 115, 130, 130, 100, 115, 115, 115 },
/* Titan       9*/ { 90, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90, 100,  90, 100, 100, 100, 100, 100, 115, 115, 100, 100, 105, 105, 100, 100, 100, 105 },
/* Nephilim   10*/ { 90, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
/* Afflicted  11*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125, 125, 115, 115, 115, 100, 115, 130, 130, 130, 125, 125, 130 },
/* Fae        12*/ { 90,  90,  90,  90,  90,  90,  90, 115, 115,  90,  90, 100,  90,  90,  90, 115, 130, 130, 130, 130,  90,  90,  90, 130, 130, 130, 130, 130 },
/* Gnome      13*/ { 90, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90,  90, 100, 100, 100, 100, 115, 115, 100, 100, 105, 105, 100, 100, 100, 105 },
/* Leprechaun 14*/ { 90, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90,  90, 100, 100, 100, 100, 115, 115, 100, 100, 105, 105, 100, 100, 100, 105 },
/* Kobold     15*/ {120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 125, 125, 125, 125, 120, 120, 130, 120, 120, 120, 120, 120 },
/* Devilspawn 16*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130, 100, 100, 100, 100, 100 },
/* Imp        17*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130, 100, 100, 100, 100, 100 },
/* Succubus   18*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130, 100, 100, 100, 100, 100 },
/* Lili       19*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130, 100, 100, 100, 100, 100 },
/* Elder      20*/ { 90, 105, 105, 105, 105, 105, 105, 105, 105, 105,  90, 105,  90, 105, 105, 105, 105, 105, 115, 115,  90, 105, 105, 105, 105, 105, 105, 105 },
/* Guardian   21*/ { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90 },
/* Horror     22*/ { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90 },
/* Vampire    23*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130,  90, 140, 100, 100, 100 },
/* Werewolf   24*/ { 90, 105, 105, 110, 115, 100, 120, 125, 120, 105, 100, 100,  95, 105, 110, 125,  90,  90,  90,  90, 100, 115, 130, 140,  90, 100, 100, 100 },
/* Skeleton   25*/ { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90 },
/* Mummy      26*/ { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90 },
/* Spectre    27*/ { 90, 105, 105, 105, 105, 105, 105, 105, 105, 105,  90, 105,  90, 105, 105, 105, 105, 105, 115, 115,  90, 105, 105, 105, 105, 105, 105, 105 },

};

/*
* Determine the price of an item (qty one) in a store.
*
* This function takes into account the player's charisma, and the
* shop-keepers friendliness, and the shop-keeper's base greed, but
* never lets a shop-keeper lose money in a transaction.
*
* The "greed" value should exceed 100 when the player is "buying" the
* item, and should be less than 100 when the player is "selling" it.
*
* Hack -- the black market always charges twice as much as it should.
* Hack -- the pawnbroker always charges 33% of what it should.
*
* Charisma adjustment runs from 80 to 130
* Racial adjustment runs from 95 to 130
*
* Since greed/charisma/racial adjustments are centered at 100, we need
* to adjust (by 200) to extract a usable multiplier.  Note that the
* "greed" value is always something (?).
*/
static s32b price_item(object_type *o_ptr, int greed, bool flip)
{
	int     factor;
	int     adjust;
	s32b    price;


	/* Get the value of one of the items */
	price = object_value(o_ptr);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Compute the racial factor */
	factor = rgold_adj[ot_ptr->owner_race][p_ptr->prace];

	/* Add in the charisma factor */
	msg_fiddle( "Adjustment : %d" , adj_stat[p_ptr->stat_ind[A_CHA]][ADJ_PRICE]);
	factor += adj_stat[p_ptr->stat_ind[A_CHA]][ADJ_PRICE];


	/* Shop is buying */
	if (flip)
	{
		/* Adjust for greed */
		adjust = 100 + (300 - (greed + factor));

		/* Never get "silly" */
		if (adjust > 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (cur_store_num == STORE_BLACK) price = price / 2;
		if (cur_store_num == STORE_PAWN) price = price / 3;
	}

	/* Shop is selling */
	else
	{
		/* Adjust for greed */
		adjust = 100 + ((greed + factor) - 300);

		/* Never get "silly" */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (cur_store_num == STORE_BLACK) price = price * 2;
		if (cur_store_num == STORE_PAWN) price = price / 3;
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Note -- Never become "free" */
	if (price <= 0L) return (1L);

	/* Return the price */
	return (price);
}


/*
* Special "mass production" computation
*/
static int mass_roll(int num, int max)
{
	int i, t = 0;
	for (i = 0; i < num; i++) t += rand_int(max);
	return (t);
}


/*
* Certain "cheap" objects should be created in "piles"
* Some objects can be sold at a "discount" (in small piles)
*/
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	int discount = 0;

	s32b cost = object_value(o_ptr);


	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lites */
	case TV_FOOD:
	case TV_FLASK:
	case TV_LITE:
		{
			if (cost <= 5L) size += mass_roll(3, 5);
			if (cost <= 20L) size += mass_roll(3, 5);
			break;
		}

	case TV_POTION:
	case TV_SCROLL:
		{
			if (cost <= 60L) size += mass_roll(3, 5);
			if (cost <= 240L) size += mass_roll(1, 5);
			break;
		}

	case TV_MIRACLES_BOOK:
	case TV_SORCERY_BOOK:
	case TV_NATURE_BOOK:
	case TV_CHAOS_BOOK:
	case TV_DEATH_BOOK:
	case TV_TAROT_BOOK:
	case TV_CHARMS_BOOK:
	case TV_SOMATIC_BOOK:
	case TV_DEMONIC_BOOK:
		{
			if (cost <= 50L) size += mass_roll(2, 3);
			if (cost <= 500L) size += mass_roll(1, 3);
			break;
		}

	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SHIELD:
	case TV_GLOVES:
	case TV_BOOTS:
	case TV_CLOAK:
	case TV_HELM:
	case TV_CROWN:
	case TV_SWORD:
	case TV_POLEARM:
	case TV_HAFTED:
	case TV_DIGGING:
	case TV_BOW:
		{
			if (o_ptr->name2) break;
			if (cost <= 10L) size += mass_roll(3, 5);
			if (cost <= 100L) size += mass_roll(3, 5);
			break;
		}

	case TV_SPIKE:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		{
			if (cost <= 5L) size += mass_roll(5, 5);
			if (cost <= 50L) size += mass_roll(5, 5);
			if (cost <= 500L) size += mass_roll(5, 5);
			break;
		}
	}


	/* Pick a discount */
	if (cost < 5)
	{
		discount = 0;
	}
	else if (rand_int(25) == 0)
	{
		discount = 25;
	}
	else if (rand_int(150) == 0)
	{
		discount = 50;
	}
	else if (rand_int(300) == 0)
	{
		discount = 75;
	}
	else if (rand_int(500) == 0)
	{
		discount = 90;
	}


	if (o_ptr->art_name)
	{
		if (debug_peek && discount)
		{
			msg_print("No discount on random artefacts.");
		}
		discount = 0;
	}

	/* Save the discount */
	o_ptr->discount = discount;

	/* Save the total pile size */
	o_ptr->number = size - (size * discount / 100);
}

/*
* Determine if a store item can "absorb" another item
*
* See "object_similar()" for the same function for the "player"
*/
static bool store_object_similar(object_type *o_ptr, object_type *j_ptr)
{
	/* Hack -- Identical items cannot be stacked */
	if (o_ptr == j_ptr) return (0);

	/* Different objects cannot be stacked */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);

	/* Different charges (etc) cannot be stacked */
	if (o_ptr->pval != j_ptr->pval) return (0);

	/* Require many identical values */
	if (o_ptr->to_h  !=  j_ptr->to_h) return (0);
	if (o_ptr->to_d  !=  j_ptr->to_d) return (0);
	if (o_ptr->to_a  !=  j_ptr->to_a) return (0);

	/* Require identical "artefact" names */
	if (o_ptr->name1 != j_ptr->name1) return (0);

	/* Require identical "ego-item" names */
	if (o_ptr->name2 != j_ptr->name2) return (0);

	/* Random artefacts don't stack !*/
	if (o_ptr->art_name || j_ptr->art_name) return (0);

	/* Hack -- Identical art_flags! */
	if ((o_ptr->art_flags1 != j_ptr->art_flags1) ||
		(o_ptr->art_flags2 != j_ptr->art_flags2) ||
		(o_ptr->art_flags3 != j_ptr->art_flags3))
		return (0);

	/* Hack -- Never stack "powerful" items */
	if (o_ptr->xtra1 || j_ptr->xtra1) return (0);

	/* Hack -- Never stack recharging items */
	if (o_ptr->timeout || j_ptr->timeout) return (0);

	/* Require many identical values */
	if (o_ptr->ac    !=  j_ptr->ac)   return (0);
	if (o_ptr->dd    !=  j_ptr->dd)   return (0);
	if (o_ptr->ds    !=  j_ptr->ds)   return (0);

	/* Hack -- Never stack chests */
	if (o_ptr->tval == TV_CHEST) return (0);

	/* Require matching discounts */
	if (o_ptr->discount != j_ptr->discount) return (0);

	/* They match, so they must be similar */
	return (TRUE);
}


/*
* Allow a store item to absorb another item
*/
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > 99) ? 99 : total;
}


/*
* Check to see if the shop will be carrying too many objects	-RAK-
* Note that the shop, just like a player, will not accept things
* it cannot hold.  Before, one could "nuke" potions this way.
*/
static bool store_check_num(object_type *o_ptr)
{
	int        i;
	object_type *j_ptr;

	/* Free space is always usable */
	if (st_ptr->stock_num < st_ptr->stock_size) return TRUE;

	/* The "home" acts like the player */
	if (cur_store_num == STORE_HOME)
	{
		/* Check all the items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing item */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing item */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (store_object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* But there was no room at the inn... */
	return (FALSE);
}


static bool is_blessed(object_type *o_ptr)
{
	u32b f1, f2, f3;
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & TR3_BLESSED) return (TRUE);
	else return (FALSE);
}



/*
* Determine if the current store will purchase the given item
*
* Note that a shop-keeper must refuse to buy "worthless" items
* Konijn Note : shopkeepers no longer refuse to buy worthless items for 0 gold
* It's a service rendered ;)
*/
static bool store_will_buy(object_type *o_ptr)
{
	/* Hack -- The Home is simple */
	if (cur_store_num == STORE_HOME) return (TRUE);
	if (cur_store_num == 99) return (FALSE);

	/* Switch on the store */
	switch (cur_store_num)
	{
		/* General Store */
	case 0:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_FOOD:
			case TV_LITE:
			case TV_FLASK:
			case TV_SPIKE:
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_DIGGING:
			case TV_CLOAK:
			case TV_BOTTLE: /* 'Green', recycling Angband */
				break;
			default:
				return (FALSE);
			}
			break;
		}

		/* Armoury */
	case 1:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_CROWN:
			case TV_HELM:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				break;
			default:
				return (FALSE);
			}
			break;
		}

		/* Weapon Shop */
	case 2:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_SHOT:
			case TV_BOLT:
			case TV_ARROW:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
				break;
			default:
				return (FALSE);
			}
			break;
		}

		/* Temple */
	case 3:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_MIRACLES_BOOK:
            case TV_DEMONIC_BOOK:
            case TV_DEATH_BOOK:
			case TV_SCROLL:
			case TV_POTION:
			case TV_HAFTED:
				break;
			case TV_POLEARM:
			case TV_SWORD:
				if (is_blessed(o_ptr))
					break;
			default:
				return (FALSE);
			}
			break;
		}

		/* Alchemist */
	case 4:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_SCROLL:
			case TV_POTION:
				break;
			default:
				return (FALSE);
			}
			break;
		}

		/* Magic Shop */
	case 5:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
            case TV_LITE:
            {
                if(o_ptr->sval!=SV_LITE_ORB)
                    return (FALSE);
                 break;
            }
			case TV_SORCERY_BOOK:
			case TV_NATURE_BOOK:
			case TV_CHAOS_BOOK:
			case TV_DEATH_BOOK:
			case TV_TAROT_BOOK:
			case TV_CHARMS_BOOK:
			case TV_SOMATIC_BOOK:
            case TV_DEMONIC_BOOK:
			case TV_AMULET:
			case TV_RING:
			case TV_STAFF:
			case TV_WAND:
			case TV_ROD:
			case TV_SCROLL:
			case TV_POTION:
				break;
			default:
				return (FALSE);
			}
			break;
		}
		/* Bookstore */
	case 8:
	case 12:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
			case TV_SORCERY_BOOK:
			case TV_NATURE_BOOK:
			case TV_CHAOS_BOOK:
			case TV_DEATH_BOOK:
			case TV_MIRACLES_BOOK:
			case TV_TAROT_BOOK:
			case TV_CHARMS_BOOK:
			case TV_SOMATIC_BOOK:
            case TV_DEMONIC_BOOK:
				/* Player needs to start donating a really rare book */
				if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought && o_ptr->sval != 3 )
					return (FALSE);
				break;
			default:
				return (FALSE);
			}
			break;
		}
		/* Inn */
	case 9:
		/* The Inn will not buy anything */
		{
			if(o_ptr->tval != TV_FOOD && o_ptr->tval != TV_BOTTLE )
				return (FALSE);
			break;
		}
		/* Hall of Records */
	case 10:
		{
			/* Hall does not buy */
			return (FALSE);
		}
		/* Pawnbrokers */
	case 11:
		{
			/* Will buy anything */
			return (TRUE);
		}
	}

	/* XXX XXX XXX Ignore "worthless" items */
	/* Store will junk for you worthless items now */
	/* if (object_value(o_ptr) <= 0) return (FALSE); */

	/* Assume okay */
	return (TRUE);
}

/*
* Add the item "o_ptr" to the inventory of the "Home"
*
* In all cases, return the slot (or -1) where the object was placed
*
* Note that this is a hacked up version of "inven_carry()".
*
* Also note that it may not correctly "adapt" to "knowledge" bacoming
* known, the player may have to pick stuff up and drop it again.
*/
static int home_carry(object_type *o_ptr)
{
	int                 slot;
	s32b               value, j_value;
	int		i;
	object_type *j_ptr;


	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing item */
		j_ptr = &st_ptr->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Determine the "value" of the item */
	value = object_value(o_ptr);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that item */
		j_ptr = &st_ptr->stock[slot];

		/* Hack -- readable books always come first */
		if ((o_ptr->tval == mp_ptr->spell_book) &&
			(j_ptr->tval != mp_ptr->spell_book)) break;
		if ((j_ptr->tval == mp_ptr->spell_book) &&
			(o_ptr->tval != mp_ptr->spell_book)) continue;

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Can happen in the home */
		if (!object_aware_p(o_ptr)) continue;
		if (!object_aware_p(j_ptr)) break;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Objects in the home can be unknown */
		if (!object_known_p(o_ptr)) continue;
		if (!object_known_p(j_ptr)) break;


		/* Hack:  otherwise identical rods sort by
		increasing recharge time --dsb */
		if (o_ptr->tval == TV_ROD) {
			if (o_ptr->pval < j_ptr->pval) break;
			if (o_ptr->pval > j_ptr->pval) continue;
		}

		/* Objects sort by decreasing value */
		j_value = object_value(j_ptr);
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		st_ptr->stock[i] = st_ptr->stock[i-1];
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Insert the new item */
	st_ptr->stock[slot] = *o_ptr;

	/* Return the location */
	return (slot);
}


/*
* Add the item "o_ptr" to a real stores inventory.
*
* If the item is "worthless", it is thrown away (except in the home).
*
* If the item cannot be combined with an object already in the inventory,
* make a new slot for it, and calculate its "per item" price.  Note that
* this price will be negative, since the price will not be "fixed" yet.
* Adding an item to a "fixed" price stack will not change the fixed price.
*
* In all cases, return the slot (or -1) where the object was placed
*/
static int store_carry(object_type *o_ptr)
{
	int		i, slot;
	s32b	value, j_value;
	object_type	*j_ptr;


	/* Evaluate the object */
	value = object_value(o_ptr);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* All store items are fully *identified* */
	if(cur_store_num != STORE_PAWN)
	{
		o_ptr->ident |= IDENT_MENTAL;

		/* Erase the inscription */
		o_ptr->note = 0;
	}

	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing item */
		j_ptr = &st_ptr->stock[slot];

		/* Can the existing items be incremented? */
		if (store_object_similar(j_ptr, o_ptr))
		{
			/* Hack -- extra items disappear */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that item */
		j_ptr = &st_ptr->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;


		/* Hack:  otherwise identical rods sort by
		increasing recharge time --dsb */
		if (o_ptr->tval == TV_ROD) {
			if (o_ptr->pval < j_ptr->pval) break;
			if (o_ptr->pval > j_ptr->pval) continue;
		}

		/* Evaluate that slot */
		j_value = object_value(j_ptr);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		st_ptr->stock[i] = st_ptr->stock[i-1];
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Insert the new item */
	st_ptr->stock[slot] = *o_ptr;

	/* Return the location */
	return (slot);
}


/*
* Increase, by a given amount, the number of a certain item
* in a certain store.  This can result in zero items.
*/
static void store_item_increase(int item, int num)
{
	int         cnt;
	object_type *o_ptr;

	/* Get the item */
	o_ptr = &st_ptr->stock[item];

	/* Verify the number */
	cnt = o_ptr->number + num;
	if (cnt > 255) cnt = 255;
	else if (cnt < 0) cnt = 0;
	num = cnt - o_ptr->number;

	/* make sure wand charges are taken into account */

	/* Save the new number */
	o_ptr->number += num;
}


/*
* Remove a slot if it is empty
*/
static void store_item_optimize(int item)
{
	int         j;
	object_type *o_ptr;

	/* Get the item */
	o_ptr = &st_ptr->stock[item];

	/* Must exist */
	if (!o_ptr->k_idx) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* One less item */
	st_ptr->stock_num--;

	/* Slide everyone */
	for (j = item; j < st_ptr->stock_num; j++)
	{
		st_ptr->stock[j] = st_ptr->stock[j + 1];
	}

	/* Nuke the final slot */
	object_wipe(&st_ptr->stock[j]);
}


/*
* This function will keep 'crap' out of the black market.
* Crap is defined as any item that is "available" elsewhere
* Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
*/
static bool black_market_crap(object_type *o_ptr)
{
	int		i, j;

	/* Ego items are never crap */
	if (o_ptr->name2) return (FALSE);

	/* Good items are never crap */
	if (o_ptr->to_a > 0) return (FALSE);
	if (o_ptr->to_h > 0) return (FALSE);
	if (o_ptr->to_d > 0) return (FALSE);

	/* Check the other "normal" stores */
	for (i = 0; i < 6; i++)
	{
		/* Check every item in the store */
		for (j = 0; j < store[i].stock_num; j++)
		{
			object_type *j_ptr = &store[i].stock[j];

			/* Duplicate item "type", assume crappy */
			if (o_ptr->k_idx == j_ptr->k_idx) return (TRUE);
		}
	}

	/* Assume okay */
	return (FALSE);
}


/*
* Attempt to delete (some of) a random item from the store
* Hack -- we attempt to "maintain" piles of items when possible.
*/
static void store_delete(void)
{
	int what, num;

	/* Pick a random slot */
	what = rand_int(st_ptr->stock_num);

	/* Determine how many items are here */
	num = st_ptr->stock[what].number;

	/* Hack -- sometimes, only destroy half the items */
	if (rand_int(100) < 50) num = (num + 1) / 2;

	/* Hack -- sometimes, only destroy a single item */
	if (rand_int(100) < 50) num = 1;

	/* Actually destroy (part of) the item */
	store_item_increase(what, -num);
	store_item_optimize(what);
}


/*
* Creates a random item and gives it to a store
* This algorithm needs to be rethought.  A lot.
* Currently, "normal" stores use a pre-built array.
*
* Note -- the "level" given to "obj_get_num()" is a "favored"
* level, that is, there is a much higher chance of getting
* items with a level approaching that of the given level...
*
* Should we check for "permission" to have the given item?
*/
static void store_create(void)
{
	int i, tries, level, tries_count, bm_dice;

	object_type forge;
	object_type *q_ptr;

	/* Paranoia -- no room left */
	if (st_ptr->stock_num >= st_ptr->stock_size) return;

	tries_count = 4;
	/* Some, well ok, one store, for now, gets more items */
	if ( cur_store_num == STORE_BOOK_SWAP )
	{
		tries_count = tries_count * 2;
	}

	/* Hack -- consider up to four items */
	for (tries = 0; tries < tries_count; tries++)
	{
		/* Black Market */
		if (cur_store_num == STORE_BLACK)
		{
			/*
			  Konijn decided to potentially mess this up
			  In that this might make stat potions less frequent in the BM
			  And we all know that that is really the only reason the BM exists ;)
			  To restore the old logic, just go for `level = 25 + rand_int(bm_dice);`
			*/
			bm_dice = (p_ptr->lev >> 1) + (p_ptr->max_dun_level >> 2);

			/* Pick a level for object/magic */
			level = 25 + rand_int(bm_dice);

			/* Random item (usually of given level) */
			i = get_obj_num(level);

			/* Handle failure */
			if (!i) continue;
		}

		/* Normal Store */
		else
		{
			/* Hack -- Pick an item to sell */
			i = st_ptr->table[rand_int(st_ptr->table_num)];

			/* Hack -- fake level for apply_magic() */
			level = rand_range(1, STORE_OBJ_LEVEL);
		}


		/* Get local object */
		q_ptr = &forge;

		/* Create a new object of the chosen kind */
		object_prep(q_ptr, i);

		/* Apply some "low-level" magic (no artefacts) */
		apply_magic(q_ptr, level, FALSE, FALSE, FALSE);

		/* Hack -- Charge lite's */
		if (q_ptr->tval == TV_LITE)
		{
			if (q_ptr->sval == SV_LITE_TORCH) q_ptr->pval = FUEL_TORCH / 2;
			if (q_ptr->sval == SV_LITE_LANTERN) q_ptr->pval = FUEL_LAMP / 2;
		}

		/* The item is "known" */
		object_known(q_ptr,FALSE);

		/* Mark it storebought */
		q_ptr->ident |= IDENT_STOREB;

		/* Mega-Hack -- no chests in stores */
		if (q_ptr->tval == TV_CHEST) continue;

		/* Prune the black market */
		if (cur_store_num == STORE_BLACK)
		{
			/* Hack -- No "crappy" items */
			if (black_market_crap(q_ptr)) continue;

			/* Hack -- No "cheap" items */
			if (object_value(q_ptr) < 10) continue;
		}

		/* Prune normal stores */
		else
		{
			/* No "worthless" items */
			if (object_value(q_ptr) <= 0) continue;
		}

		/* Mass produce and/or Apply discount */
		mass_produce(q_ptr);

		/* Attempt to carry the (known) item */
		(void)store_carry(q_ptr);

		/* Definitely done */
		break;
	}
}

/*
* Eliminate need to bargain if player has haggled well in the past
*/
static bool noneedtobargain(s32b minprice)
{
	s32b good = st_ptr->good_buy;
	s32b bad = st_ptr->bad_buy;

	/* Cheap items are "boring" */
	if (minprice < 10L) return (TRUE);

	/* Perfect haggling */
	if (good == MAX_SHORT) return (TRUE);

	/* Reward good haggles, punish bad haggles, notice price */
	if (good > ((3 * bad) + (5 + (minprice/50)))) return (TRUE);

	/* Return the flag */
	return (FALSE);
}


/*
* Update the bargain info
*/
static void updatebargain(s32b price, s32b minprice)
{
	/* Hack -- auto-haggle */
	if (auto_haggle) return;

	/* Cheap items are "boring" */
	if (minprice < 10L) return;

	/* Count the successful haggles */
	if (price == minprice)
	{
		/* Just count the good haggles */
		if (st_ptr->good_buy < MAX_SHORT)
		{
			st_ptr->good_buy++;
		}
	}

	/* Count the failed haggles */
	else
	{
		/* Just count the bad haggles */
		if (st_ptr->bad_buy < MAX_SHORT)
		{
			st_ptr->bad_buy++;
		}
	}
}

/*
* Re-displays a single store entry
*/
static void display_entry(int pos)
{
	int			i;
	object_type		*o_ptr;
	s32b		x;

	char		o_name[80];
	char		out_val[160];


	int maxwid = 75;

	/* Get the item */
	o_ptr = &st_ptr->stock[pos];

	/* Get the "offset" */
	i = (pos % 12);

	/* Label it, clear the line --(-- */
	(void)sprintf(out_val, "%c) ", I2A(i));
	prt(out_val, i+6, 0);

	if (equippy_chars)
	{
		byte a = object_attr(o_ptr);
		char c = object_char(o_ptr);

#ifdef AMIGA
		if (a & 0x80)
			a |= 0x40;
#endif

		Term_draw(3, i + 6, a, c);
	}

	/* Describe an item in the home */
	if (cur_store_num == STORE_HOME)
	{
		maxwid = 75;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 10;

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i+6, equippy_chars ? 5 : 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			(void)sprintf(out_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(out_val, i+6, 68);
		}
	}

	/* Describe an item (fully) in a store */
	else
	{
		/* Must leave room for the "price" */
		maxwid = 65;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 7;

		/* Describe the object (fully) */
		if (cur_store_num == STORE_PAWN)
		{
			object_desc(o_name, o_ptr, TRUE, 3);
		}
		else
		{
			object_desc_store(o_name, o_ptr, TRUE, 3);
		}
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i+6, equippy_chars ? 5 : 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			(void)sprintf(out_val, "%3d.%d", wgt / 10, wgt % 10);
			put_str(out_val, i+6, 61);
		}

		/* Display a "fixed" cost */
		if (o_ptr->ident & (IDENT_FIXED))
		{
			/* Extract the "minimum" price */
			x = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

			/* Actually draw the price (not fixed) */
			(void)sprintf(out_val, "%9ld F", (long)x);
			if(x>p_ptr->au)
				c_put_str(TERM_L_DARK, out_val, i+6, 68);
			else
				put_str(out_val, i+6, 68);
		}

		/* Display a "taxed" cost */
		else if (auto_haggle)
		{
			/* Extract the "minimum" price */
			x = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

			/* Hack -- Apply Sales Tax if needed */
			if (!noneedtobargain(x)) x += x / 10;

			/* Actually draw the price (with tax) */
			(void)sprintf(out_val, "%9ld  ", (long)x);
			if(x>p_ptr->au)
				c_put_str(TERM_L_DARK, out_val, i+6, 68);
			else
				put_str(out_val, i+6, 68);		}

		/* Display a "haggle" cost */
		else
		{
			/* Extrect the "maximum" price */
			x = price_item(o_ptr, ot_ptr->max_inflate, FALSE);

			/* Actually draw the price (not fixed) */
			(void)sprintf(out_val, "%9ld  ", (long)x);
			if(x>p_ptr->au)
				c_put_str(TERM_L_DARK, out_val, i+6, 68);
			else
				put_str(out_val, i+6, 68);
		}
	}
}


/*
* Displays a store's inventory			-RAK-
* All prices are listed as "per individual object".  -BEN-
*/
static void display_inventory(void)
{
	int i, k;

	/* Only show the inventory of the Mage Guild if a high level book has been gifted */
	if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
	{
		put_str( "Before you can use these services, you need to donate any rare spellbook.   " , 5 , 3 );
		return;
	}

	/* Display the next 12 items */
	for (k = 0; k < 12; k++)
	{
		/* Do not display "dead" items */
		if (store_top + k >= st_ptr->stock_num) break;

		/* Display that line */
		display_entry(store_top + k);
	}

	/* Erase the extra lines and the "more" prompt */
	for (i = k; i < 13; i++) prt("", i + 6, 0);

	/* Assume "no current page" */
	put_str("        ", 5, 20);

	/* Visual reminder of "more items" */
	if (st_ptr->stock_num > 12)
	{
		/* Show "more" reminder (after the last item) */
		prt("-more-", k + 6, 3);

		/* Indicate the "current page" */
		put_str(format("(Page %d)", store_top/12 + 1), 5, 20);
	}
}


/*
* Displays players gold					-RAK-
*/
static void store_prt_gold(void)
{
	char out_val[64];

	prt("Gold Remaining: ", 19, 53);

	sprintf(out_val, "%9ld", (long)p_ptr->au);
	prt(out_val, 19, 68);
}


/*
* Displays store (after clearing screen)		-RAK-
*/
static void display_store(void)
{
	char buf[80];

	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (cur_store_num == STORE_HOME)
	{
		/* Put the owner name */
		put_str("Your Home", 3, 30);

		/* Label the item descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		if (show_weights)
		{
			put_str("Weight", 5, 70);
		}
	}

	/* So is the Hall */
	if (cur_store_num == STORE_HALL)
	{
		put_str("Hall Of Records",3,30);
	}

	/* Normal stores */
	if ((cur_store_num != STORE_HOME) && (cur_store_num != STORE_HALL))
	{
		cptr store_name;
		cptr owner_name = (ot_ptr->owner_name);
		cptr race_name = race_info[ot_ptr->owner_race].title;
		if(cur_store_num != STORE_BOOK_SWAP)
		{
			store_name = (f_name + f_info[FEAT_SHOP_HEAD + cur_store_num].name);
		}
		else
		{
			/*Hack, this is one the second floor of the feature ;\ */
			store_name = "Mage Guild";
		}

		/* Put the owner name and race for all except Inn, where only owner_name is shown*/
		if( cur_store_num != STORE_INN )
		{
		  sprintf(buf, "%s (%s)", owner_name, race_name);
		}else{
		  sprintf(buf, "%s", owner_name );
		}
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		sprintf(buf, "%s (%ld)", store_name, (long)(ot_ptr->max_cost));
		prt(buf, 3, 50);

		/* Label the item descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		if (show_weights)
		{
			put_str("Weight", 5, 60);
		}

		/* Label the asking price (in stores) */
		put_str("Price", 5, 72);
	}

	/* Display the current gold */
	store_prt_gold();



	/* Draw in the inventory */
	display_inventory();
}



/*
* Get the ID of a store item and return its value	-RAK-
*/
static int get_stock(int *com_val, cptr pmt, int i, int j)
{
	char	command;

	char	out_val[160];

#ifdef ALLOW_REPEAT /* TNB */

	/* Get the item index */
	if (repeat_pull(com_val)) {

		/* Verify the item */
		if ((*com_val >= i) && (*com_val <= j)) {

			/* Success */
			return (TRUE);
		}
	}

#endif /* ALLOW_REPEAT -- TNB */

	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Assume failure */
	*com_val = (-1);

	/* Build the prompt */
	(void)sprintf(out_val, "(Items %c-%c, ESC to exit) %s",
		I2A(i), I2A(j), pmt);

	/* Ask until done */
	while (TRUE)
	{
		int k;

		/* Escape */
		if (!get_com(out_val, &command)) break;

		/* Convert */
		k = (islower(command) ? A2I(command) : -1);

		/* Legal responses */
		if ((k >= i) && (k <= j))
		{
			*com_val = k;
			break;
		}

		/* Oops */
		bell();
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Cancel */
	if (command == ESCAPE) return (FALSE);

#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(*com_val);

#endif /* ALLOW_REPEAT -- TNB */


	/* Success */
	return (TRUE);
}


/*
* Increase the insult counter and get angry if too many -RAK-
*/
static int increase_insults(void)
{
	/* Increase insults */
	st_ptr->insult_cur++;

	/* Become insulted */
	if (st_ptr->insult_cur > ot_ptr->insult_max)
	{
		/* Complain */
		say_comment_4();

		/* Reset insults */
		st_ptr->insult_cur = 0;
		st_ptr->good_buy = 0;
		st_ptr->bad_buy = 0;

		/* Open tomorrow */
		st_ptr->store_open = turn + 25000 + randint(25000);

		/* Closed */
		return (TRUE);
	}

	/* Not closed */
	return (FALSE);
}


/*
* Decrease insults					-RAK-
*/
static void decrease_insults(void)
{
	/* Decrease insults */
	if (st_ptr->insult_cur) st_ptr->insult_cur--;
}


/*
* Have insulted while haggling				-RAK-
*/
static int haggle_insults(void)
{
	/* Increase insults */
	if (increase_insults()) return (TRUE);

	/* Display and flush insult */
	say_comment_5();

	/* Still okay */
	return (FALSE);
}


/*
* Mega-Hack -- Enable "increments"
*/
static bool allow_inc = FALSE;

/*
* Mega-Hack -- Last "increment" during haggling
*/
static s32b last_inc = 0L;


/*
* Get a haggle
*/
static int get_haggle(cptr pmt, s32b *poffer, s32b price, int final)
{
	s32b		i;

	cptr		p;

	char                buf[128];
	char		out_val[160];


	/* Clear old increment if necessary */
	if (!allow_inc) last_inc = 0L;


	/* Final offer */
	if (final)
	{
		sprintf(buf, "%s [accept] ", pmt);
	}

	/* Old (negative) increment, and not final */
	else if (last_inc < 0)
	{
		sprintf(buf, "%s [-%ld] ", pmt, (long)(ABS(last_inc)));
	}

	/* Old (positive) increment, and not final */
	else if (last_inc > 0)
	{
		sprintf(buf, "%s [+%ld] ", pmt, (long)(ABS(last_inc)));
	}

	/* Normal haggle */
	else
	{
		sprintf(buf, "%s ", pmt);
	}


	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Ask until done */
	while (TRUE)
	{
		/* Default */
		strcpy(out_val, "");

		/* Ask the user for a response */
		if (!get_string(buf, out_val, 32)) return (FALSE);

		/* Skip leading spaces */
		for (p = out_val; *p == ' '; p++) /* loop */;

		/* Empty response */
		if (*p == '\0')
		{
			/* Accept current price */
			if (final)
			{
				*poffer = price;
				last_inc = 0L;
				break;
			}

			/* Use previous increment */
			if (allow_inc && last_inc)
			{
				*poffer += last_inc;
				break;
			}
		}

		/* Normal response */
		else
		{
			/* Extract a number */
			i = atol(p);

			/* Handle "incremental" number */
			if ((*p == '+' || *p == '-'))
			{
				/* Allow increments */
				if (allow_inc)
				{
					/* Use the given "increment" */
					*poffer += i;
					last_inc = i;
					break;
				}
			}

			/* Handle normal number */
			else
			{
				/* Use the given "number" */
				*poffer = i;
				last_inc = 0L;
				break;
			}
		}

		/* Warning */
		msg_print("Invalid response.");
		msg_print(NULL);
	}

	/* Success */
	return (TRUE);
}


/*
* Receive an offer (from the player)
*
* Return TRUE if offer is NOT okay
*/
static bool receive_offer(cptr pmt, s32b *poffer,
						  s32b last_offer, int factor,
						  s32b price, int final)
{
	/* Haggle till done */
	while (TRUE)
	{
		/* Get a haggle (or cancel) */
		if (!get_haggle(pmt, poffer, price, final)) return (TRUE);

		/* Acceptable offer */
		if (((*poffer) * factor) >= (last_offer * factor)) break;

		/* Insult, and check for kicked out */
		if (haggle_insults()) return (TRUE);

		/* Reject offer (correctly) */
		(*poffer) = last_offer;
	}

	/* Success */
	return (FALSE);
}


/*
* Haggling routine					-RAK-
*
* Return TRUE if purchase is NOT successful
*/
static bool purchase_haggle(object_type *o_ptr, s32b *price)
{
	s32b               cur_ask, final_ask;
	s32b               last_offer, offer;
	s32b               x1, x2, x3;
	s32b               min_per, max_per;
	int                flag, loop_flag, noneed;
	int                annoyed = 0, final = FALSE;

	bool		cancel = FALSE;

	cptr		pmt = "Asking";

	char		out_val[160];


	*price = 0;


	/* Extract the starting offer and the final offer */
	cur_ask = price_item(o_ptr, ot_ptr->max_inflate, FALSE);
	final_ask = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

	/* Determine if haggling is necessary */
	noneed = noneedtobargain(final_ask);

	/* No need to haggle */
	if (noneed || auto_haggle)
	{
		/* No need to haggle */
		if (noneed)
		{
			/* Message summary */
			msg_print("You eventually agree upon the price.");
			msg_print(NULL);
		}

		/* No haggle option */
		else
		{
			/* Message summary */
			msg_print("You quickly agree upon the price.");
			msg_print(NULL);

			/* Apply Sales Tax */
			final_ask += final_ask / 10;
		}

		/* Final price */
		cur_ask = final_ask;

		/* Go to final offer */
		pmt = "Final Offer";
		final = TRUE;
	}


	/* Haggle for the whole pile */
	cur_ask *= o_ptr->number;
	final_ask *= o_ptr->number;


	/* Haggle parameters */
	min_per = ot_ptr->haggle_per;
	max_per = min_per * 3;

	last_offer = 1;
	/* No offer yet */
	offer = 0;

	/* No incremental haggling yet */
	allow_inc = FALSE;

	/* Haggle until done */
	for (flag = FALSE; !flag; )
	{
		loop_flag = TRUE;

		while (!flag && loop_flag)
		{
			(void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
			put_str(out_val, 1, 0);
			cancel = receive_offer("What do you offer? ",
				&offer, last_offer, 1, cur_ask, final);

			if (cancel)
			{
				flag = TRUE;
			}
			else if (offer > cur_ask)
			{
				say_comment_6();
				offer = last_offer;
			}
			else if (offer == cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}
			else
			{
				loop_flag = FALSE;
			}
		}

		if (!flag)
		{
			x1 = 100 * (offer - last_offer) / (cur_ask - last_offer);
			if (x1 < min_per)
			{
				if (haggle_insults())
				{
					flag = TRUE;
					cancel = TRUE;
				}
			}
			else if (x1 > max_per)
			{
				x1 = x1 * 3 / 4;
				if (x1 < max_per) x1 = max_per;
			}
			x2 = rand_range(x1-2, x1+2);
			x3 = ((cur_ask - offer) * x2 / 100L) + 1;
			/* don't let the price go up */
			if (x3 < 0) x3 = 0;
			cur_ask -= x3;

			/* Too little */
			if (cur_ask < final_ask)
			{
				final = TRUE;
				cur_ask = final_ask;
				pmt = "Final Offer";
				annoyed++;
				if (annoyed > 3)
				{
					(void)(increase_insults());
					cancel = TRUE;
					flag = TRUE;
				}
			}
			else if (offer >= cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}

			if (!flag)
			{
				last_offer = offer;
				allow_inc = TRUE;
				prt("", 1, 0);
				(void)sprintf(out_val, "Your last offer: %ld",
					(long)last_offer);
				put_str(out_val, 1, 39);
				say_comment_2(cur_ask, annoyed);
			}
		}
	}

	/* Cancel */
	if (cancel) return (TRUE);

	/* Update bargaining info */
	updatebargain(*price, final_ask);

	/* Those born under plutus can only part with a quarter of their treasure at a time */
	if( p_ptr->psign==SIGN_PLUTUS && *price > p_ptr->au/4)
	{
		msg_print("You cannot part from that much treasure!");
		return(TRUE);
	}

	/* Do not cancel */
	return (FALSE);
}

/* Haggle for a fixed price service from a store owner */
/* Altered form of purchase_haggle by DA */
static bool service_haggle(s32b service_cost, s32b *price)
{
	s32b               cur_ask, final_ask;
	s32b               last_offer, offer;
	s32b               x1, x2, x3;
	s32b               min_per, max_per;
	int                flag, loop_flag, noneed;
	int                annoyed = 0, final = FALSE;

	bool		cancel = FALSE;

	cptr		pmt = "Asking";

	char		out_val[160];


	*price = 0;


	/* Extract the starting offer and the final offer */
	cur_ask = service_cost * 2;
	final_ask = service_cost;

	/* Determine if haggling is necessary */
	noneed = noneedtobargain(final_ask);

	/* No need to haggle */
	if (noneed || auto_haggle)
	{
		/* No need to haggle */
		if (noneed)
		{
			/* Message summary */
			msg_print("You eventually agree upon the price.");
			msg_print(NULL);
		}

		/* No haggle option */
		else
		{
			/* Message summary */
			msg_print("You quickly agree upon the price.");
			msg_print(NULL);

			/* Apply Sales Tax */
			final_ask += final_ask / 10;
		}

		/* Final price */
		cur_ask = final_ask;

		/* Go to final offer */
		pmt = "Final Offer";
		final = TRUE;
	}


	/* Haggle parameters */
	min_per = ot_ptr->haggle_per;
	max_per = min_per * 3;

	last_offer = 1;
	/* No offer yet */
	offer = 0;

	/* No incremental haggling yet */
	allow_inc = FALSE;

	/* Haggle until done */
	for (flag = FALSE; !flag; )
	{
		loop_flag = TRUE;

		while (!flag && loop_flag)
		{
			(void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
			put_str(out_val, 1, 0);
			cancel = receive_offer("What do you offer? ",
				&offer, last_offer, 1, cur_ask, final);

			if (cancel)
			{
				flag = TRUE;
			}
			else if (offer > cur_ask)
			{
				say_comment_6();
				offer = last_offer;
			}
			else if (offer == cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}
			else
			{
				loop_flag = FALSE;
			}
		}

		if (!flag)
		{
			x1 = 100 * (offer - last_offer) / (cur_ask - last_offer);
			if (x1 < min_per)
			{
				if (haggle_insults())
				{
					flag = TRUE;
					cancel = TRUE;
				}
			}
			else if (x1 > max_per)
			{
				x1 = x1 * 3 / 4;
				if (x1 < max_per) x1 = max_per;
			}
			x2 = rand_range(x1-2, x1+2);
			x3 = ((cur_ask - offer) * x2 / 100L) + 1;
			/* don't let the price go up */
			if (x3 < 0) x3 = 0;
			cur_ask -= x3;

			/* Too little */
			if (cur_ask < final_ask)
			{
				final = TRUE;
				cur_ask = final_ask;
				pmt = "Final Offer";
				annoyed++;
				if (annoyed > 3)
				{
					(void)(increase_insults());
					cancel = TRUE;
					flag = TRUE;
				}
			}
			else if (offer >= cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}

			if (!flag)
			{
				last_offer = offer;
				allow_inc = TRUE;
				prt("", 1, 0);
				(void)sprintf(out_val, "Your last offer: %ld",
					(long)last_offer);
				put_str(out_val, 1, 39);
				say_comment_2(cur_ask, annoyed);
			}
		}
	}

	/* Cancel */
	if (cancel) return (TRUE);

	/* Update bargaining info */
	updatebargain(*price, final_ask);

	/* Those born under plutus can only part with a quarter of their treasure at a time */
	if( p_ptr->psign==SIGN_PLUTUS && *price > p_ptr->au/4)
	{
		msg_print("You cannot part from that much treasure!");
		return(TRUE);
	}

	/* Do not cancel */
	return (FALSE);
}


/*
* Haggling routine					-RAK-
*
* Return TRUE if purchase is NOT successful
*/
static bool sell_haggle(object_type *o_ptr, s32b *price)
{
	s32b               purse, cur_ask, final_ask;
	s32b               last_offer = 0, offer = 0;
	s32b               x1, x2, x3;
	s32b               min_per, max_per;

	int			flag, loop_flag, noneed;
	int			annoyed = 0, final = FALSE;

	bool		cancel = FALSE;

	cptr		pmt = "Offer";

	char		out_val[160];


	*price = 0;


	/* Obtain the starting offer and the final offer */
	cur_ask = price_item(o_ptr, ot_ptr->max_inflate, TRUE);
	final_ask = price_item(o_ptr, ot_ptr->min_inflate, TRUE);

	/* Determine if haggling is necessary */
	noneed = noneedtobargain(final_ask);

	/* Get the owner's payout limit */
	purse = (s32b)(ot_ptr->max_cost);

	/* No need to haggle */
	if (noneed || auto_haggle || (final_ask >= purse))
	{
		/* No reason to haggle */
		if (final_ask >= purse)
		{
			/* Message */
			msg_print("You instantly agree upon the price.");
			msg_print(NULL);

			/* Offer full purse */
			final_ask = purse;
		}

		/* No need to haggle */
		else if (noneed)
		{
			/* Message */
			msg_print("You eventually agree upon the price.");
			msg_print(NULL);
		}

		/* No haggle option */
		else
		{
			/* Message summary */
			msg_print("You quickly agree upon the price.");
			msg_print(NULL);

			/* Apply Sales Tax */
			final_ask -= final_ask / 10;
		}

		/* Final price */
		cur_ask = final_ask;

		/* Final offer */
		final = TRUE;
		pmt = "Final Offer";
	}

	/* Haggle for the whole pile */
	cur_ask *= o_ptr->number;
	final_ask *= o_ptr->number;


	/* XXX XXX XXX Display commands */

	/* Haggling parameters */
	min_per = ot_ptr->haggle_per;
	max_per = min_per * 3;

	/* Mega-Hack -- artificial "last offer" value */
	last_offer = object_value(o_ptr) * o_ptr->number;
	last_offer = last_offer * ot_ptr->max_inflate / 100L;

	/* No offer yet */
	offer = 0;

	/* No incremental haggling yet */
	allow_inc = FALSE;

	/* Haggle */
	for (flag = FALSE; !flag; )
	{
		while (1)
		{
			loop_flag = TRUE;

			(void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
			put_str(out_val, 1, 0);
			cancel = receive_offer("What price do you ask? ",
				&offer, last_offer, -1, cur_ask, final);

			if (cancel)
			{
				flag = TRUE;
			}
			else if (offer < cur_ask)
			{
				say_comment_6();
				/* rejected, reset offer for incremental haggling */
				offer = last_offer;
			}
			else if (offer == cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}
			else
			{
				loop_flag = FALSE;
			}

			/* Stop */
			if (flag || !loop_flag) break;
		}

		if (!flag)
		{
			x1 = 100 * (last_offer - offer) / (last_offer - cur_ask);
			if (x1 < min_per)
			{
				if (haggle_insults())
				{
					flag = TRUE;
					cancel = TRUE;
				}
			}
			else if (x1 > max_per)
			{
				x1 = x1 * 3 / 4;
				if (x1 < max_per) x1 = max_per;
			}
			x2 = rand_range(x1-2, x1+2);
			x3 = ((offer - cur_ask) * x2 / 100L) + 1;
			/* don't let the price go down */
			if (x3 < 0) x3 = 0;
			cur_ask += x3;

			if (cur_ask > final_ask)
			{
				cur_ask = final_ask;
				final = TRUE;
				pmt = "Final Offer";
				annoyed++;
				if (annoyed > 3)
				{
					flag = TRUE;
					(void)(increase_insults());
				}
			}
			else if (offer <= cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}

			if (!flag)
			{
				last_offer = offer;
				allow_inc = TRUE;
				prt("", 1, 0);
				(void)sprintf(out_val,
					"Your last bid %ld", (long)last_offer);
				put_str(out_val, 1, 39);
				say_comment_3(cur_ask, annoyed);
			}
		}
	}

	/* Cancel */
	if (cancel) return (TRUE);

	/* Update bargaining info */
	updatebargain(*price, final_ask);

	/* Do not cancel */
	return (FALSE);
}

/*
* Maintain the inventory at 1 store
*/
void store_maint(int which)
{
	int         j;

	int		old_rating = rating;

	/* Ignore home */
	if ((which == STORE_HOME) || (which == STORE_HALL) || (which == STORE_PAWN) )return;


	/* Save the store indices */
	cur_store_num = which;
	cur_store_num = which;

	/* Activate that store */
	st_ptr = &store[cur_store_num];

	/* Activate the owner */
	ot_ptr = &owners[cur_store_num][st_ptr->owner];

	/* Store keeper forgives the player */
	st_ptr->insult_cur = 0;

	/* Mega-Hack -- prune the black market */
	if (cur_store_num == STORE_BLACK)
	{
		/* Destroy crappy black market items */
		for (j = st_ptr->stock_num - 1; j >= 0; j--)
		{
			object_type *o_ptr = &st_ptr->stock[j];

			/* Destroy crappy items */
			if (black_market_crap(o_ptr))
			{
				/* Destroy the item */
				store_item_increase(j, 0 - o_ptr->number);
				store_item_optimize(j);
			}
		}
	}


	/* Choose the number of slots to keep */
	j = st_ptr->stock_num;

	/* Sell a few items */
	j = j - randint(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "underflow" */
	if (j < 0) j = 0;

	/* Destroy objects until only "j" slots are left */
	while (st_ptr->stock_num > j) store_delete();

	/* Choose the number of slots to fill */
	j = st_ptr->stock_num;

	/* Buy some more items */
	j = j + randint(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "overflow" */
	if (j >= st_ptr->stock_size) j = st_ptr->stock_size - 1;

	/* Hack -- Inn only has four possible items so use all four */
	if ((j > 4) && ( cur_store_num == STORE_INN)) j=4;

	/* Acquire some new items */
	while (st_ptr->stock_num < j)
	{
		store_create();
	}

	/* Hack -- Restore the rating */
	rating = old_rating;
}




/*
* Buy an item from a store				-RAK-
*/
static void store_purchase(void)
{
	int i, amt, choice;
	int item, item_new;

	s32b price, best;

	object_type forge;
	object_type *j_ptr;

	object_type *o_ptr;

	char o_name[80];

	char out_val[160];

	/* Only allow purchasing in the Mage Guild if a high level book has been gifted */
	if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
		return;

	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (cur_store_num == STORE_HOME) msg_print("Your home is empty.");
		else msg_print("I am currently out of stock.");
		return;
	}


	/* Find the number of objects on this and following pages */
	i = (st_ptr->stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	if (cur_store_num == STORE_HOME)
	{
		sprintf(out_val, "Which item do you want to take? ");
	}
	else
	{
		sprintf(out_val, "Which item are you interested in? ");
	}

	/* Get the item number to be bought */
	if (!get_stock(&item, out_val, 0, i-1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	o_ptr = &st_ptr->stock[item];

	/* Assume the player wants just one of them */
	amt = 1;

	/* Get local object */
	j_ptr = &forge;

	/* Get a copy of the object */
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many different items.");
		return;
	}

	/* Determine the "best" price (per item) */
	best = price_item(j_ptr, ot_ptr->min_inflate, FALSE);

	/* Find out how many the player wants */
	if (o_ptr->number > 1)
	{
		/* Hack -- note cost of "fixed" items */
		if ((cur_store_num != 7) && (o_ptr->ident & (IDENT_FIXED)))
		{
			msg_format("That costs %ld gold per item.", (long)(best));
		}

		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number,FALSE);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	j_ptr = &forge;

	/* Get desired object */
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = amt;





	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Attempt to buy it */
	if (cur_store_num != 7)
	{
		/* Fixed price, quick buy */
		if (o_ptr->ident & (IDENT_FIXED))
		{
			/* Assume accept */
			choice = 0;

			/* Go directly to the "best" deal */
			price = (best * j_ptr->number);
		}

		/* Haggle for it */
		else
		{
			/* Describe the object (fully) */
			if(cur_store_num == STORE_PAWN)
			{
				object_desc(o_name, j_ptr, TRUE, 3);
			}
			else
			{
				object_desc_store(o_name, j_ptr, TRUE, 3);
			}
			/* Message */
			msg_format("Buying %s (%c).", o_name, I2A(item));
			msg_print(NULL);

			/* Haggle for a final price */
			choice = purchase_haggle(j_ptr, &price);

			/* Hack -- Got kicked out */
			if (st_ptr->store_open >= turn) return;
		}


		/* Player wants it */
		if (choice == 0)
		{
			/* Fix the item price (if "correctly" haggled) */
			if (price == (best * j_ptr->number)) o_ptr->ident |= (IDENT_FIXED);

			/* Player can afford it */
			if (p_ptr->au >= price)
			{
				/* Say "okay" */
				say_comment_1();

				/* Make a sound */
				sound(SOUND_BUY);

				/* Be happy */
				decrease_insults();

				/* Spend the money */
				p_ptr->au -= price;

				/* Update the display */
				store_prt_gold();

				/* Hack -- buying an item makes you aware of it */
				if(cur_store_num != STORE_PAWN) object_aware(j_ptr);

				/* Hack -- clear the "fixed" flag from the item */
				j_ptr->ident &= ~(IDENT_FIXED);

				/* Distribute charges of wands, staves, or rods */
				/* Right here, before description happens, after the sell is final */
				distribute_charges(o_ptr, j_ptr, amt);


				/* Describe the transaction */
				object_desc(o_name, j_ptr, TRUE, 3);

				/* Message */
				if (cur_store_num ==STORE_PAWN)
				{
					msg_format("You bought back %s for %ld gold.",o_name,(long)price);
				}
				else
				{
					msg_format("You bought %s for %ld gold.", o_name, (long)price);
				}

				/* Erase the inscription */
				j_ptr->note = 0;

				/* Give it to the player */
				item_new = inven_carry(j_ptr, FALSE);

				/* Describe the final result */
				object_desc(o_name, &inventory[item_new], TRUE, 3);

				/* Message */
				msg_format("You have %s (%c).",
					o_name, index_to_label(item_new));

				/* Handle stuff */
				handle_stuff();

				/* Note how many slots the store used to have */
				i = st_ptr->stock_num;

				/* Remove the bought items from the store */
				store_item_increase(item, -amt);
				store_item_optimize(item);

				/* Store is empty*/
				if (st_ptr->stock_num == 0)
				{

					/* Nothing left  in Pawnbrokers*/
					if(cur_store_num == STORE_PAWN)
					{
						store_top = 0;
						display_inventory();
					}
					else
					{
						/* Shuffle */
						if (rand_int(STORE_SHUFFLE) == 0)
						{
							/* Message */
							msg_print("The shopkeeper retires.");

							/* Shuffle the store */
							store_shuffle(cur_store_num);
						}

						/* Maintain */
						else
						{
							/* Message */
							msg_print("The shopkeeper brings out some new stock.");
						}

						/* New inventory */
						for (i = 0; i < 10; i++)
						{
							/* Maintain the store */
							store_maint(cur_store_num);
						}

						/* Start over */
						store_top = 0;

						/* Redraw everything */
						display_inventory();
					}
				}

				/* The item is gone */
				else if (st_ptr->stock_num != i)
				{
					/* Pick the correct screen */
					if (store_top >= st_ptr->stock_num) store_top -= 12;

					/* Redraw everything */
					display_inventory();
				}

				/* Item is still here */
				else
				{
					/* Redraw the item */
					display_entry(item);
				}
			}

			/* Player cannot afford it */
			else
			{
				/* Simple message (no insult) */
				msg_print("You do not have enough gold.");
			}
		}
	}

	/* Home is much easier */
	else
	{
		/* Distribute charges of wands, staves, or rods */
		distribute_charges(o_ptr, j_ptr, amt);

		/* Give it to the player */
		item_new = inven_carry(j_ptr, FALSE);

		/* Describe just the result */
		object_desc(o_name, &inventory[item_new], TRUE, 3);

		/* Message */
		msg_format("You have %s (%c).", o_name, index_to_label(item_new));

		/* Handle stuff */
		handle_stuff();

		/* Take note if we take the last one */
		i = st_ptr->stock_num;

		/* Remove the items from the home */
		store_item_increase(item, -amt);
		store_item_optimize(item);

		/* Hack -- Item is still here */
		if (i == st_ptr->stock_num)
		{
			/* Redraw the item */
			display_entry(item);
		}

		/* The item is gone */
		else
		{
			/* Nothing left */
			if (st_ptr->stock_num == 0) store_top = 0;

			/* Nothing left on that screen */
			else if (store_top >= st_ptr->stock_num) store_top -= 12;

			/* Redraw everything */
			display_inventory();
		}
	}

	/* Not kicked out */
	return;
}


/*
* Sell an item to the store (or home)
*/
static void store_sell(void)
{
	int choice;
	int item, item_pos;
	int amt;

	s32b price, value, dummy;

	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr;

	cptr pmt = "Sell which item? ";

	char o_name[80];

	/* Going overboard probably ;] */
	if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
		pmt = "Donate which item? ";

	/* Prepare a prompt */
	if (cur_store_num == STORE_HOME) pmt = "Drop which item? ";

	/* Only allow items the store will buy */
	item_tester_hook = store_will_buy;

	/* Get an item (from equip or inven) */
	if (!get_item(&item, pmt,"You have nothing that I want.", USE_EQUIP | USE_INVEN))
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

	/* Assume one item */
	amt = 1;

	/* Find out how many the player wants (letter means "all") */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number,TRUE);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	q_ptr = &forge;

	/* Get a copy of the object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = amt;

	/* Modify charges for wands */
	if(o_ptr->tval==TV_WAND)
	{
		q_ptr->pval = o_ptr->pval / o_ptr->number * amt;
	}

	/* Get a full description */
	object_desc(o_name, q_ptr, TRUE, 3);

	/* Remove any inscription for stores */
	if (cur_store_num != 7) q_ptr->note = 0;

	/* Is there room in the store (or the home?) */
	if (!store_check_num(q_ptr))
	{
		if (cur_store_num == STORE_HOME) msg_print("Your home is full.");
		else msg_print("I have no room in my store to keep it.");
		return;
	}

	/* Real store */
	if (cur_store_num != STORE_HOME)
	{
		if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
		{
			/* Describe the transaction */
			msg_format("Donating %s (%c).", o_name, index_to_label(item));
			msg_print(NULL);
			/* Haggle for it */
			choice = 0; /* Paradoxically, this means a succesful sale */
			price = 30000;
		}
		else if (object_value(o_ptr) > 0 )
		{
			/* Describe the transaction */
			msg_format("Selling %s (%c).", o_name, index_to_label(item));
			msg_print(NULL);
			/* Haggle for it */
			choice = sell_haggle(q_ptr, &price);
		}
		else
		{
			if (get_check("This is worthless, do you want me to junk it?"))
			{
				msg_format("Junking %s (%c).", o_name, index_to_label(item));
				msg_print(NULL);
				/* Identify it fully */
				object_full_id( q_ptr );
				/* Get a potentially new full description */
				object_desc(o_name, q_ptr, TRUE, 3);
				/* Eco friendly shop owners will not charge for making Hell a cleaner place */
				price = 0;
				choice = 0;
			}
			else{
				/* Identify it fully, just to show the customer why the store owner wanted to junk it */
				object_full_id( q_ptr );
				return;
			}
		}

		/* Kicked out */
		if (st_ptr->store_open >= turn) return;

		/* Sold... */
		if (choice == 0)
		{
			/* Say "okay" */
			say_comment_1();

			/* Make a sound */
			sound(SOUND_SELL);

			/* Be happy */
			decrease_insults();

			/* Get some money, except if we donate */
			if(!( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought ) )
				p_ptr->au += price;

			/* Update the display */
			store_prt_gold();

			/* Get the "apparent" value */
			dummy = object_value(q_ptr) * q_ptr->number;

			if (cur_store_num != STORE_PAWN)
			{
				/* Identify original item */
				object_aware(o_ptr);
				object_known(o_ptr,FALSE);
			}

			/* Combine / Reorder the pack (later) */
			p_ptr->notice |= (PN_COMBINE | PN_REORDER);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

			/* Get local object */
			q_ptr = &forge;

			/* Get a copy of the object */
			object_copy(q_ptr, o_ptr);

			/* Modify quantity */
			q_ptr->number = amt;

			/* Modify charges for wands */
			if(o_ptr->tval==TV_WAND)
			{
				q_ptr->pval = o_ptr->pval / o_ptr->number * amt;
			}

			/* Get the "actual" value */
			if (cur_store_num == STORE_PAWN)
			{
				value = dummy;
			}
			else
			{
				value = object_value(q_ptr) * q_ptr->number;
				/* Get the description all over again */
				object_desc(o_name, q_ptr, TRUE, 3);
			}

			if( price <= 0)
			{
				/* Describe the result (in message buffer) */
				msg_format("You junked the %s.", o_name);
			}
			/* If we are dealing with the Mage Guild, then Mage Guild will open it */
			else if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
			{
				price = 0;
				value = 0;
				msg_format("You donate the %s.", o_name, (long)price);
			}
			else if (cur_store_num != STORE_PAWN)
			{
				/* Describe the result (in message buffer) */
				msg_format("You sold %s for %ld gold.", o_name, (long)price);
			}
			else
			{
				msg_format("You pawn %s for %ld gold.",o_name,(long)price);
			}

			/* Analyze the prices (and comment verbally) */
			purchase_analyze(price, value, dummy);

			/* Take the item from the player, describe the result */
			inven_item_increase(item, -amt);
			inven_item_describe(item);
			inven_item_optimize(item);

			/* If we are dealing with the Mage Guild, then Mage Guild will open it */
			if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
			{
				store[cur_store_num].bought = 1;
				display_store();
				msg_print("Welcome to the Mages Guild!");
			}

			/* Handle stuff */
			handle_stuff();

			/*** XXX XXX XXX Hack, get out of void function because we did the job if we junked */
			if(price<=0)return;

			if(
				(cur_store_num == STORE_TEMPLE && q_ptr->tval == TV_DEMONIC_BOOK ) ||
				(cur_store_num == STORE_TEMPLE && q_ptr->tval == TV_DEATH_BOOK )
			  )
			{
				msg_format( "%s destroys %s with glee!" , ot_ptr->owner_name , o_name );
			}
			else
			{
			if(cur_store_num != STORE_PAWN)
			{
				/* The store gets that (known) item */
				item_pos = store_carry(q_ptr);
			}
			else
			{
				/* The pawnshop gets that (unknown) item */
				item_pos = home_carry(q_ptr);
			}

			/* Re-display if item is now in store */
			if (item_pos >= 0)
			{
				store_top = (item_pos / 12) * 12;
				display_inventory();
			}

			}
		}
	}

	/* Player is at home */
	else
	{
		/* Describe */
		msg_format("You drop %s (%c).", o_name, index_to_label(item));

		/* Take it from the players inventory */
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		item_pos = home_carry(q_ptr);

		/* Update store display */
		if (item_pos >= 0)
		{
			store_top = (item_pos / 12) * 12;
			display_inventory();
		}
	}
}



/*
* Examine an item in a store             -JDL-
*/
static void store_examine(void)
{
	int i;
	int item;

	object_type *o_ptr;

	char o_name[80];

	char out_val[160];

	/* Only examine the inventory of the Mage Guild if a high level book has been gifted */
	if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
		return;

	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (cur_store_num == STORE_HOME) msg_print("Your home is empty.");
		else msg_print("I am currently out of stock.");
		return;
	}


	/* Find the number of objects on this and following pages */
	i = (st_ptr->stock_num - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	sprintf(out_val, "Which item do you want to examine? ");

	/* Get the item number to be examined */
	if (!get_stock(&item, out_val, 0, i-1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	o_ptr = &st_ptr->stock[item];

	/* If it is a spell book then browse it */
	if ((o_ptr->tval == TV_MIRACLES_BOOK) || (o_ptr->tval == TV_SORCERY_BOOK) ||
		(o_ptr->tval == TV_NATURE_BOOK) || (o_ptr->tval == TV_CHAOS_BOOK) ||
		(o_ptr->tval == TV_DEATH_BOOK) || (o_ptr->tval == TV_SOMATIC_BOOK) ||
		(o_ptr->tval == TV_TAROT_BOOK) || (o_ptr->tval == TV_CHARMS_BOOK) || (o_ptr->tval == TV_DEMONIC_BOOK))
	{
		do_store_browse(o_ptr);
		return;
	}
	/* Require full knowledge */
	if (!(o_ptr->ident & (IDENT_MENTAL)))
	{
		/* This can only happen in the home */
		msg_print("You have no special knowledge about that item.");
		return;
	}

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	msg_format("Examining %s...", o_name);

	/* Describe it fully */
	if (!identify_fully_aux(o_ptr)) msg_print("You see nothing special.");

	return;
}





/*
* Hack -- set this to leave the store
*/
static bool leave_store = FALSE;


/*
* Process a command in a store
*
* Note that we must allow the use of a few "special" commands
* in the stores which are not allowed in the dungeon, and we
* must disable some commands which are allowed in the dungeon
* but not in the stores, to prevent chaos.
*/
static void store_process_command(void)
{
	s32b price,cost;

#ifdef ALLOW_REPEAT /* TNB */

	/* Handle repeating the last command */
	repeat_check();

#endif /* ALLOW_REPEAT -- TNB */

	if (rogue_like_commands && command_cmd == 'l')
	{
		command_cmd = 'x';  /* hack! */
	}

	/* Parse the command */
	switch (command_cmd)
	{
		/* Leave */
	case ESCAPE:
		{
			leave_store = TRUE;
			break;
		}

		/* Browse */
	case ' ':
		{
			if (st_ptr->stock_num <= 12)
			{
				msg_print("Entire inventory is shown.");
			}
			else
			{
				store_top += 12;
				if (store_top >= st_ptr->stock_num) store_top = 0;
				display_inventory();
			}
			break;
		}

		/* Redraw */
	case KTRL('R'):
		{
			do_cmd_redraw();
			display_store();
			break;
		}

		/* Get (purchase) */
	case 'g':
		{
			store_purchase();
			break;
		}

		/* Drop (Sell) */
	case 'd':
		{
			store_sell();
			break;
		}

		/* Examine */
	case 'x':
		{
			store_examine();
			break;
		}
	case 'a':
		if(cur_store_num == STORE_INN)
		{
			price = 200;
			if (price >= p_ptr->au)
			{
				msg_format("You do not have the gold, I am sorry.");
			}
			else
			{
				/*Mimic a scroll of Rumor*/
				p_ptr->au -= price;
				char rumor[80];
				get_rnd_line("rumors.txt", rumor);
				msg_format("Someone tells you '%s'.", rumor);
			}
			p_ptr->window |= (PW_PLAYER);
			handle_stuff();
		}
		break;
		/* View hiscores in hall */
	case 'c':
		{
		if( cur_store_num == STORE_TEMPLE ){
			if (!service_haggle(1500000,&price))
			{
				if (price >= p_ptr->au)
				{
					msg_format("You do not have the gold, I am sorry.");
				}
				else
				{
					p_ptr->au -= price;
					/* Say "okay" */
					say_comment_1();
					/* Make a sound */
					sound(SOUND_BUY);
					/* Be happy */
					decrease_insults();
					store_prt_gold();
					/*Cleanse, clear all mutations*/
					p_ptr->muta1 = 0;
					p_ptr->muta2 = 0;
					p_ptr->muta3 = 0;
					msg_format("You feel remarkably normal.");
				}
				p_ptr->window |= (PW_SPELL | PW_PLAYER);
				handle_stuff();
			}
		} else if(cur_store_num == STORE_HALL)
		{
			show_highclass(p_ptr->pclass,p_ptr->realm1);
		}
		else
		{
			msg_print("That command does not work in this store.");
		}
		break;
		}
	case 'h':
		{
			if(cur_store_num == STORE_HALL)
			{
				race_score(p_ptr->prace);
			}
			else
			{
				msg_print("That command does not work in this store.");
			}
			break;
		}
	case '<':
		{
			if( cur_store_num == STORE_LIBRARY )
			{
				/* Go upstairs to get to the geek club */
				cur_store_num = STORE_BOOK_SWAP;
				/* Set pointer to correct store */
				st_ptr = &store[cur_store_num];
				/* Start at the beginning */
				store_top = 0;
				/* Display the store */
				display_store();
			}else{
				msg_print("Only the library has a second floor.");
			}
			break;
		}
	case '>':
		{
			if( cur_store_num == STORE_BOOK_SWAP )
			{
				/* Go upstairs to get to the geek club */
				cur_store_num = STORE_LIBRARY;
				/* Set pointer to correct store */
				st_ptr = &store[cur_store_num];
				/* Start at the beginning */
				store_top = 0;
				/* Display the store */
				display_store();
			}else{
				msg_print("Only the mage guild is on the second floor.");
			}
			break;
		}
		/* perform 'special' for store */
	case 'r':
		{
			switch (cur_store_num)
			{
			case STORE_GENERAL:
				{
					msg_print("That command does not work in this store.");
					break;
				}
			case STORE_ARMOURY:
				{
					if (!service_haggle(400,&price))
					{
						if (price >= p_ptr->au)
						{
							msg_format("You do not have the gold!");
						}
						else
						{
							p_ptr->au -= price;
							/* Say "okay" */
							say_comment_1();
							/* Make a sound */
							sound(SOUND_BUY);
							/* Be happy */
							decrease_insults();
							store_prt_gold();
							enchant_spell(0,0,4);
						}
						p_ptr->window |= (PW_PLAYER);
						handle_stuff();
					}
					break;
				}
			case STORE_WEAPON:
				{
					if (!service_haggle(800,&price))
					{
						if (price >= p_ptr->au)
						{
							msg_format("You do not have the gold!");
						}
						else
						{
							p_ptr->au -= price;
							/* Say "okay" */
							say_comment_1();
							/* Make a sound */
							sound(SOUND_BUY);
							/* Be happy */
							decrease_insults();
							store_prt_gold();
							enchant_spell(4,4,0);
						}
						p_ptr->window |= (PW_PLAYER);
						handle_stuff();
					}
					break;
				}
			case STORE_TEMPLE:
				{
					if (!service_haggle(750,&price))
					{
						if (price >= p_ptr->au)
						{
							msg_format("You do not have the gold!");
						}
						else
						{
							p_ptr->au -= price;
							/* Say "okay" */
							say_comment_1();
							/* Make a sound */
							sound(SOUND_BUY);
							/* Be happy */
							decrease_insults();
							store_prt_gold();
							do_res_stat(A_STR);
							do_res_stat(A_INT);
							do_res_stat(A_WIS);
							do_res_stat(A_DEX);
							do_res_stat(A_CON);
							do_res_stat(A_CHA);
							restore_level();
						}
						p_ptr->window |= (PW_SPELL | PW_PLAYER);
						handle_stuff();
					}
					break;
				}
			case STORE_ALCHEMIST:
				{
					if (!service_haggle(2000,&price))
					{
						if (price >= p_ptr->au)
						{
							msg_format("You do not have the gold!");
						}
						else
						{
							p_ptr->au -= price;
							/* Say "okay" */
							say_comment_1();
							/* Make a sound */
							sound(SOUND_BUY);
							/* Be happy */
							decrease_insults();
							store_prt_gold();
							identify_fully();
						}
						p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
						handle_stuff();
					}
					break;
				}
			case STORE_MAGIC:
				{
					if (p_ptr->ritual == 1)
					{
						msg_format("You have already done the ritual!");
					}
					else
					{
						/* cost based on experience */
						cost=10*p_ptr->max_exp;
						/* minimum of 100 gold */
						if(cost < 100) cost = 100;
						if (!service_haggle(cost,&price))
						{
							if (price >= p_ptr->au)
							{
								msg_format("You do not have the gold!");
							}
							else
							{
								p_ptr->au -= price;
								/* Say "okay" */
								say_comment_1();
								/* Make a sound */
								sound(SOUND_BUY);
								/* Be happy */
								decrease_insults();
								store_prt_gold();
								p_ptr->ritual = 1;
								msg_format("You perform the Ritual of True Recall.");
								msg_format("You feel as if Life might give you a second chance.");
							}
							p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
							handle_stuff();
						}
					}
					break;
				}
			case STORE_BLACK:
			case STORE_BOOK_SWAP:
				{
					msg_print("That command does not work in this store.");
					break;
				}
			case STORE_HOME:
				{
					if ((p_ptr->poisoned > 0) || (p_ptr->cut > 0))
					{
						msg_print("Your wounds prevent you from sleeping.");
					}
					else
					{
						if (rp_ptr->undead)
						{
							room_rest(TRUE);
						}
						else
						{
							room_rest(FALSE);
						}
					}
					break;
				}
			case STORE_LIBRARY:
				{
					do_cmd_study();
					break;
				}
			case STORE_INN:
				{
					if ((p_ptr->poisoned > 0) || (p_ptr->cut > 0))
					{
						msg_print("You need a healer, not a room!");
						msg_print("I'm sorry, but  I don't want anyone dying in here.");
					}
					else
					{
						if (!service_haggle(10,&price))
						{
							if (price >= p_ptr->au)
							{
								msg_format("You do not have the gold!");
							}
							else
							{
								p_ptr->au -= price;
								/* Say "okay" */
								say_comment_1();
								/* Make a sound */
								sound(SOUND_BUY);
								/* Be happy */
								decrease_insults();
								store_prt_gold();
								if (rp_ptr->undead)
								{
									room_rest(TRUE);
								}
								else
								{
									room_rest(FALSE);
								}
							}
						}
					}
					break;
				}
			case STORE_HALL:
				{
					if (store[STORE_HOME].bought == 1)
					{
						msg_format("You already have the deeds!");
					}
					else if (!free_homes())
					{
						msg_format("Sorry, we have no houses on our books.");
					}
					else
					{
						if (!service_haggle(30000,&price))
						{
							if (price >= p_ptr->au)
							{
								msg_format("You do not have the gold!");
							}
							else
							{
								p_ptr->au -= price;
								/* Say "okay" */
								say_comment_1();
								/* Make a sound */
								sound(SOUND_BUY);
								/* Be happy */
								decrease_insults();
								store_prt_gold();
								store[STORE_HOME].bought=1;
								msg_format("You may move in at once.");
							}
							p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
							handle_stuff();
						}
					}
					break;
				}
			case STORE_PAWN:
					if (!service_haggle(500,&price))
					{
						if (price >= p_ptr->au)
						{
							msg_format("You do not have the gold!");
						}
						else
						{
							p_ptr->au -= price;
							/* Say "okay" */
							say_comment_1();
							/* Make a sound */
							sound(SOUND_BUY);
							/* Be happy */
							decrease_insults();
							store_prt_gold();
							identify_pack();
							msg_format("All your goods have been identified.");
						}
						p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
						handle_stuff();
					}
					break;
			}
			break;
		}

		/* Ignore return */
	case '\r':
		{
			break;
		}



		/*** Inventory Commands ***/

		/* Wear/wield equipment */
	case 'w':
		{
			do_cmd_wield();
			break;
		}

		/* Take off equipment */
	case 't':
		{
			do_cmd_takeoff();
			break;
		}

		/* Destroy an item */
	case 'k':
		{
			do_cmd_destroy();
			break;
		}

		/* Destroy all worthless items */
	case 'K':
		{
			do_cmd_destroy_all();
			break;
		}

		/* Equipment list */
	case 'e':
		{
			do_cmd_equip();
			break;
		}

		/* Inventory list */
	case 'i':
		{
			do_cmd_inven();
			break;
		}


		/*** Various commands ***/

		/* Identify an object */
	case 'I':
		{
			do_cmd_observe();
			break;
		}

		/* Hack -- toggle windows */
	case KTRL('I'):
		{
			toggle_inven_equip();
			break;
		}



		/*** Use various objects ***/

		/* Browse a book */
	case 'b':
		{
			do_cmd_browse(-1);
			break;
		}

		/* Inscribe an object */
	case '{':
		{
			do_cmd_inscribe();
			break;
		}

		/* Uninscribe an object */
	case '}':
		{
			do_cmd_uninscribe();
			break;
		}



		/*** Help and Such ***/

		/* Help */
	case '?':
		{
			do_cmd_help(syshelpfile);
			break;
		}

		/* Identify symbol */
	case '/':
		{
			do_cmd_query_symbol();
			break;
		}

		/* Character description */
	case 'C':
		{
			do_cmd_change_name();
			display_store();
			break;
		}


		/*** System Commands ***/

		/* Hack -- User interface */
	case '!':
		{
			(void)Term_user(0);
			break;
		}

		/* Single line from a pref file */
	case '"':
		{
			do_cmd_pref();
			break;
		}

		/* Interact with macros */
	case '@':
		{
			do_cmd_macros();
			break;
		}

		/* Interact with visuals */
	case '%':
		{
			do_cmd_visuals();
			break;
		}

		/* Interact with colours */
	case '&':
		{
			do_cmd_colours();
			break;
		}

		/* Interact with options */
	case '=':
		{
			do_cmd_options();
			break;
		}


		/*** Misc Commands ***/

		/* Take notes */
	case ':':
		{
			do_cmd_note();
			break;
		}

		/* Version info */
	case 'V':
		{
			do_cmd_version();
			break;
		}

		/* Repeat level feeling */
	case KTRL('F'):
		{
			do_cmd_feeling(FALSE);
			break;
		}

		/* Show previous message */
	case KTRL('O'):
		{
			do_cmd_message_one();
			break;
		}

		/* Show previous messages */
	case KTRL('P'):
		{
			do_cmd_messages();
			break;
		}

		/* Check artefacts, uniques etc. */
	case '~':
	case '|':
		{
			do_cmd_knowledge();
			break;
		}

		/* Load "screen dump" */
	case '(':
		{
			do_cmd_load_screen( ANGBAND_DIR_PREF ,  "dump.txt" );
			(void)msg_flush_wait();
			(void)restore_screen();
			break;
		}

		/* Save "screen dump" */
	case ')':
		{
			do_cmd_save_screen();
			break;
		}


		/* Hack -- Unknown command */
	default:
		{
			msg_print("That command does not work in stores.");
			break;
		}
	}
}

/*
 * -Konijn- This is really all one big hodgepodge of hacks and aggregated common things between shops
 * I extracted it from do_cmd_store because I needed evil returns breaking normal flow
 * Yes, I will burn
*/

void put_store_commands()
{
		/*One day : c_put_str*/

		/* Basic commands */
		prt(" escape) Exit", 22, 0);

		/* Unbought Mage Guild does not follow any rule*/
		if( cur_store_num == STORE_BOOK_SWAP && !store[cur_store_num].bought )
		{
			prt("d) Donate" , 22 , 17 );
			prt(">) Go downstairs" , 22 , 30 );
			return;
		}

		/* Shop commands XXX XXX XXX */
		if ((cur_store_num != STORE_HOME) && (cur_store_num != STORE_HALL))
		{
			prt(" s) Sell", 22, 15);
			prt(" p) Purchase", 22, 30);
		}

		/* Home commands */
		if (cur_store_num == STORE_HOME)
		{
			prt(" g) Get", 22, 31);
			prt(" d) Drop", 23, 31);
		}

		if (cur_store_num == STORE_HALL)
		{
			prt(" h) view racial Heroes.", 22, 31);
			prt(" c) view Class heroes.", 23,31);
		}
		else
			/* Add in the eXamine option */
		{
			prt(" x) eXamine", 22, 45);
		}

		/* Browse if necessary */
		if (st_ptr->stock_num > 12)
		{
			prt("  space) More", 23, 0);
		}

		/* Special for each store */

		switch (cur_store_num)
		{
		case STORE_ARMOURY:
			{
				prt(" r) Enchant Armour", 22,60);
				break;
			}
		case STORE_WEAPON:
			{
				prt(" r) Enchant Weapon", 22,60);
				break;
			}
		case STORE_TEMPLE:
			{
				prt(" r) Restoration", 22,60);
				prt(" c) Cleanse", 23, 15);
				break;
			}
		case STORE_ALCHEMIST:
			{
				prt(" r) Research", 22, 60);
				break;
			}
		case STORE_MAGIC:
			{
				prt(" r) True Recall.", 22,60);
				break;
			}
		case STORE_HOME:
			{
				prt(" r) Rest.",23,56);
				break;
			}
		case STORE_LIBRARY:
			{
				prt(" r) Gain a spell.",22, 60);
				prt(" <) Go upstairs", 23, 15);
				break;
			}
		case STORE_BOOK_SWAP:
			{
				prt(">) Go downstairs" , 22 , 60 );
				break;
			}
		case STORE_INN:
			{
				prt(" r) Rent a Room", 22,60);
				prt(" a) Research Rumours", 23,5);
				break;
			}
		case STORE_HALL:
			{
				prt(" r) Buy a house.", 23,56);
				break;
			}
		case STORE_PAWN:
			{
				prt(" r) Identify all", 22,60);
				break;
			}
		}
}

/*
* Enter a store, and interact with it.
*
* Note that we use the standard "request_command()" function
* to get a command, allowing us to use "command_arg" and all
* command macros and other nifty stuff, but we use the special
* "shopping" argument, to force certain commands to be converted
* into other commands, normally, we convert "p" (pray) and "m"
* (cast magic) into "g" (get), and "s" (search) into "d" (drop).
*/
void do_cmd_store(void)
{
	int			which;

	int			tmp_cha;

	cave_type		*c_ptr;


	/* Access the player grid */
	c_ptr = &cave[py][px];

	/* Verify a store */
	if (!((c_ptr->feat >= FEAT_SHOP_HEAD) &&
		(c_ptr->feat <= FEAT_SHOP_TAIL)))
	{
		msg_print("You see no store here.");
		return;
	}

	/* Extract the store number */

	which = get_which_store();

	/* Check for 'Ironman' option */

	if((ironman_shop) && (which != STORE_LIBRARY))
	{
		msg_print("Only wimps hide indoors!");
		return;
	}

	/* Hack -- Check the "locked doors" */
	if (store[which].store_open >= turn)
	{
		msg_print("The door is locked.");
		return;
	}

	/* Can't enter house unless you own it */
	if((which == STORE_HOME) && (store[which].bought == 0))
	{
		msg_print("The door is locked.");
		return;
	}

	/* Forget the lite */
	forget_lite();

	/* Forget the view */
	forget_view();


	/* Hack -- Character is in "icky" mode */
	character_icky = TRUE;


	/* No command argument */
	command_arg = 0;

	/* No repeated command */
	command_rep = 0;

	/* No automatic command */
	command_new = 0;


	/* Save the store number */
	cur_store_num = which;

	/* Save the store and owner pointers */
	st_ptr = &store[cur_store_num];
	ot_ptr = &owners[cur_store_num][st_ptr->owner];

	/* Start at the beginning */
	store_top = 0;

	/* Display the store */
	display_store();

	/* Do not leave */
	leave_store = FALSE;

	/* Interact with player */
	while (!leave_store)
	{
		/* Hack -- Clear line 1 */
		prt("", 1, 0);

		/* Hack -- Check the charisma */
		tmp_cha = p_ptr->stat_use[A_CHA];

		/* Clear */
		clear_from(21);

		/* Prompt */
		put_str("You may: ", 21, 0);

		/* Put all possible command */
		put_store_commands();

		/* Get a command */
		request_command(TRUE);

		/* Process the command */
		store_process_command();

		/* Hack -- Character is still in "icky" mode */
		character_icky = TRUE;

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();

		/* XXX XXX XXX Pack Overflow */
		if (inventory[INVEN_PACK].k_idx)
		{
			int item = INVEN_PACK;

			object_type *o_ptr = &inventory[item];

			/* Hack -- Flee from the store */
			if (cur_store_num != 7)
			{
				/* Message */
				msg_print("Your pack is so full that you flee the store...");

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Flee from the home */
			else if (!store_check_num(o_ptr))
			{
				/* Message */
				msg_print("Your pack is so full that you flee your home...");

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Drop items into the home */
			else
			{
				int item_pos;

				object_type forge;
				object_type *q_ptr;

				char o_name[80];


				/* Give a message */
				msg_print("Your pack overflows!");

				/* Get local object */
				q_ptr = &forge;

				/* Grab a copy of the item */
				object_copy(q_ptr, o_ptr);

				/* Describe it */
				object_desc(o_name, q_ptr, TRUE, 3);

				/* Message */
				msg_format("You drop %s (%c).", o_name, index_to_label(item));

				/* Remove it from the players inventory */
				inven_item_increase(item, -255);
				inven_item_describe(item);
				inven_item_optimize(item);

				/* Handle stuff */
				handle_stuff();

				/* Let the home carry it */
				item_pos = home_carry(q_ptr);

				/* Redraw the home */
				if (item_pos >= 0)
				{
					store_top = (item_pos / 12) * 12;
					display_inventory();
				}
			}
		}

		/* Hack -- Redisplay store prices if charisma changes */
		if (tmp_cha != p_ptr->stat_use[A_CHA]) display_inventory();

		/* Hack -- get kicked out of the store */
		if (st_ptr->store_open >= turn) leave_store = TRUE;
	}


	/* Free turn XXX XXX XXX */
	energy_use = 0;


	/* Hack -- Character is no longer in "icky" mode */
	character_icky = FALSE;


	/* Hack -- Cancel automatic command */
	command_new = 0;

	/* Hack -- Cancel "see" mode */
	command_see = FALSE;


	/* Flush messages XXX XXX XXX */
	msg_print(NULL);


	/* Clear the screen */
	Term_clear();


	/* Update everything */
	p_ptr->update |= (PU_VIEW | PU_LITE);
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
* Shuffle one of the stores.
*/
void store_shuffle(int which)
{
	int i, j;


	/* Ignore home, hall and pawnbroker */
	if (which == 7 || which > 9) return;


	/* Save the store index */
	cur_store_num = which;

	/* Activate that store */
	st_ptr = &store[cur_store_num];

	/* Pick a new owner */
	for (j = st_ptr->owner; j == st_ptr->owner; )
	{
		st_ptr->owner = (byte)(rand_int(MAX_OWNERS));
	}

	/* Activate the new owner */
	ot_ptr = &owners[cur_store_num][st_ptr->owner];


	/* Reset the owner data */
	st_ptr->insult_cur = 0;
	st_ptr->store_open = 0;
	st_ptr->good_buy = 0;
	st_ptr->bad_buy = 0;


	/* Hack -- discount all the items */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		object_type *o_ptr;

		/* Get the item */
		o_ptr = &st_ptr->stock[i];

		/* Hack -- Sell all old items for "half price" */
		if (!(o_ptr->art_name))
			o_ptr->discount = 50;

		/* Hack -- Items are no longer "fixed price" */
		o_ptr->ident &= ~(IDENT_FIXED);

		/* Mega-Hack -- Note that the item is "on sale" */
		o_ptr->note = quark_add("on sale");
	}
}


/*
 * Maintain the inventory at 1 store, times times
 * So we can force a good measure of shuffling
 */
void store_maint_all( int times )
{
	int n; /* Store Indicator */
	int i; /* Generic counter */
	for (n = 0; n < MAX_STORES; n++)
	{
		/* Ignore home, hall  and pawnbrokers */
		if ((n != STORE_HOME) &&
			(n != STORE_HALL) &&
			(n != STORE_PAWN))
		{
			/* Maintain the shop (ten times) */
			for (i = 0; i < times; i++) store_maint(n);
		}
	}
}


/*
* Initialize the stores
*/
void store_init(int which)
{
	int         k;

	/* Save the store index */
	cur_store_num = which;

	/* Activate that store */
	st_ptr = &store[cur_store_num];

	/* Pick an owner */
	st_ptr->owner = (byte)(rand_int(MAX_OWNERS));
	st_ptr->bought = 0;

	/* Activate the new owner */
	ot_ptr = &owners[cur_store_num][st_ptr->owner];

	/* Initialize the store */
	st_ptr->store_open = 0;
	st_ptr->insult_cur = 0;
	st_ptr->good_buy = 0;
	st_ptr->bad_buy = 0;

	/* Nothing in stock */
	st_ptr->stock_num = 0;

	/* Clear any old items */
	for (k = 0; k < st_ptr->stock_size; k++)
	{
		object_wipe(&st_ptr->stock[k]);
	}
}


void move_to_black_market(object_type * o_ptr)
{
	st_ptr = &store[6];
	o_ptr->ident |= IDENT_STOREB;
	(void)store_carry(o_ptr);
	object_wipe(o_ptr); /* Don't leave a bogus object behind... */
}

int get_which_store(void)
{
	int i;

	for(i=0; i < MAX_STORES; i++)
	{
		if((px == store[i].x) && (py == store[i].y))
		{
			return i;
		}
	}
	/* Should never get to here, but just in case... */
	return 1;
}

/*
* Peruse the spells/prayers in a Book in the shop
*
* Note that *all* spells in the book are listed
*
*/
void do_store_browse( object_type *o_ptr)
{
	int		sval;
	int		spell = -1;
	int		num = 0;

	byte		spells[64];


	/* Access the item's sval */
	sval = o_ptr->sval;

	/* Extract spells */
	for (spell = 0; spell < 32; spell++)
	{
		/* Check for this spell */
		if ((fake_spell_flags[sval] & (1L << spell)))
		{
			/* Collect this spell */
			spells[num++] = spell;
		}
	}


	/* Save the screen */
	Term_save();

	/* Display the spells */
	print_spells(spells, num, 1, 15, (o_ptr->tval-90));

	/* Clear the top line */
	prt("", 0, 0);

	/* Prompt user */
	put_str("[(Browsing) Choose a spell or press Escape ]", 0, 17);

	/* Spoil the spells*/
	spoil_spells( o_ptr );
}
