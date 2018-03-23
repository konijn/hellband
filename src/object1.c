/* File: object1.c */

/* Purpose: Object code, part 1 */

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


/*
* XXX XXX Hack -- note that "TERM_MULTI" is now just "TERM_VIOLET"
* We will have to find a cleaner method for "MULTI_HUED" later.
* There were only two multi-hued "flavors" (one potion, one food).
* Plus five multi-hued "base-objects" (3 dragon scales, one blade
* of chaos, and one something else).  See the SHIMMER_OBJECTS code
* in "dungeon.c" and the object colour extractor in "cave.c".
*/
#define TERM_MULTI      TERM_VIOLET


/*
* Max sizes of the following arrays
*/
#define MAX_ROCKS      56       /* Used with rings (min 38) */
#define MAX_AMULETS    19       /* Used with amulets (min 13) */
#define MAX_WOODS      32       /* Used with staffs (min 30) */
#define MAX_METALS     39       /* Used with wands/rods (min 29/28) */
#define MAX_COLORS     66       /* Used with potions (min 60) */
#define MAX_SHROOM     20       /* Used with mushrooms (min 20) */
#define MAX_TITLES     54       /* Used with scrolls (min 48) */
#define MAX_SYLLABLES 164       /* Used with scrolls (see below) */

/*
 * Rings (adjectives and colours)
 */

static cptr ring_adj[MAX_ROCKS] =
{
		/* Scary material 3*/
	    "Skull", "Bone" , "Bloodstone" ,
	    /* Utility Ring 4 */
		"Chevron", "Reeve's", "Signet" , "Poisoning" ,
	    /* Black materials 7*/
		"Onyx", "Obsidian", "Black Marble", "Black Pearl" , "Swarovski", "Jasper" , "Bixbyite" ,
		/* Symbolic/wish ring 2 */
	    "Prosperity", "Sun",
	    /* Organizations 3*/
	    "Crusader" , "Inquisitor" , "Imperial" ,
	    /*Cultural references 6*/
	    "Byzantine" , "Ancient", "Celtic", "Etruscan", "Goth" , "Umbrian" ,
	    /* Band count references 3*/
		"Double banded", "Triple banded" , "Gimmal" ,
	    /* Metals 7*/
		"Iron", "Steel", "Copper","White Gold", "Bronze" , "Gold", "Silver",
	    /* Coupling 4*/
		"Regards", "Posie", "Chastity", "Claddaigh" ,
	    /* Magical Positions 4*/
		"Mage's", "Wizard's", "Witches'", "Grandmaster's",
	    /*Various 4*/
		"Spikard", "Serpentine",  "Double", "Dragon's" ,
	    /* Metal states 6*/
		"Plain",  "Rusted", "Tarnished", "Dull" , "Blotted" , "Stained" ,
		/* Church Ranks 3*/
	    "Bishop's",  "Cardinal's", "Papal"

};

static byte ring_col[MAX_ROCKS] =
{
   /* Scary material 3*/
   TERM_WHITE, TERM_L_WHITE, TERM_RED,
   /* Utility Ring 4*/
   TERM_L_BLUE, TERM_L_GREEN, TERM_RED, TERM_VIOLET,
   /* Black materials 7*/
   TERM_L_DARK,TERM_L_DARK,TERM_L_DARK,TERM_L_DARK,TERM_L_DARK,TERM_L_DARK,TERM_L_DARK,
   /* Symbolic/wish ring 2 */
   TERM_YELLOW , TERM_YELLOW ,
    /* Organizations 3*/
    TERM_RED , TERM_WHITE , TERM_YELLOW,
   /*Cultural references 6*/
	TERM_BLUE,TERM_BLUE,TERM_GREEN,TERM_GREEN,TERM_UMBER,TERM_UMBER,
   /* Band count references 3*/
   TERM_ORANGE,TERM_ORANGE,TERM_ORANGE,
    /* Metals 7*/
	TERM_WHITE,TERM_WHITE, TERM_ORANGE, TERM_L_WHITE,TERM_ORANGE , TERM_YELLOW , TERM_L_WHITE ,
    /* Coupling 4*/
    TERM_L_RED , TERM_L_RED , TERM_L_RED , TERM_ORANGE,
     /* Magical Positions 4*/
    TERM_VIOLET , TERM_VIOLET , TERM_L_BLUE , TERM_L_GREEN,
	/*Various 4*/
	TERM_L_GREEN , TERM_L_GREEN , TERM_YELLOW , TERM_L_GREEN ,
	/* Metal states 6*/
	TERM_SLATE , TERM_UMBER , TERM_SLATE , TERM_SLATE , TERM_SLATE , TERM_UMBER ,
	/* Church Ranks 3*/
	TERM_L_RED , TERM_L_RED , TERM_L_RED
};


/*
* Amulets (adjectives and colours)
*/

static cptr amulet_adj[MAX_AMULETS] =
{
	"Amber", "Driftwood", "Coral", "Agate", "Ivory",
	"Obsidian", "Bone", "Brass", "Bronze", "Pewter",
	"Tortoise Shell", "Golden", "Azure", "Crystal", "Silver",
	"Copper", "Swastika", "Dragon Tooth" , "Dragon Claw" ,
};

static byte amulet_col[MAX_AMULETS] =
{
	TERM_YELLOW, TERM_L_UMBER, TERM_WHITE, TERM_L_WHITE, TERM_WHITE,
		TERM_L_DARK, TERM_WHITE, TERM_L_UMBER, TERM_L_UMBER, TERM_SLATE,
		TERM_GREEN, TERM_YELLOW, TERM_L_BLUE, TERM_L_BLUE, TERM_L_WHITE,
		TERM_L_UMBER, TERM_VIOLET, TERM_RED, TERM_RED /* Hack */
};


/*
* Staffs (adjectives and colours)
 Fancy stuff, but cornel is a type of dogwood, it sounds better
 Fir is kind of pine in Denmark ( guess what country's tree collection is well described in wikipedia ;)
 Cercis is also a tree found in Denmark, sounds good
 Chestnut is very european, much more than bamboo...
 Ash is also more european, then tons of trees of http://www.biblepicturegallery.com/Pictures/Trees.htm
 Sandal Wood , (1K.10.11, 2Chr.2.8, 11.10).
 Myrtle , (Neh.8.15, Is.41.19, 55.13).
 Fig Tree , According to some traditions, the fig was the forbidden fruit..
 Tamarisk , (Ps.92.14, 1King.6.29,32,35,7.36, Lev.23.40, Jn.12.31, Neh.8.15, Jn.12.13)
 Juniper
 Wormwood, Deut.29.18, Prov.5.4, Rev.8.11.
 
*/

static cptr staff_adj[MAX_WOODS] =
{
	    "Aspen", "Fir", "Cercis", "Birch", "Cedar",
		"Willow", "Cypress", "Cornel", "Elm", "Beech",
		"Poplar", "Hickory", "Sandal Wood", "Locust", "Ash",
		"Maple", "Mulberry", "Oak", "Pine", "Myrtle",
		"Rosewood", "Spruce", "Fig Tree", "Teak", "Walnut",
		"Mistletoe", "Hawthorn", "Chestnut", "Tamarisk", "Wormwood",
		"Olive Tree", "Juniper"/*,"Gnarled","Ivory"*/
};

static byte staff_col[MAX_WOODS] =
{
		TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
		TERM_L_GREEN, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
		TERM_L_GREEN, TERM_L_UMBER, TERM_UMBER, TERM_L_UMBER, TERM_UMBER,
		TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_RED,
		TERM_RED, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER,
		TERM_GREEN, TERM_L_UMBER, TERM_L_UMBER, TERM_GREEN, TERM_UMBER,
		TERM_L_GREEN, TERM_SLATE, /*???,???,???*/
};


/*
* Wands (adjectives and colours)
 Scheelite is the new tungsten ;)
 Femur is the thigh bone, with middle age people being much smaller, it would be about the size of a wand
*/

static cptr wand_adj[MAX_METALS] =
{
    	"Triple Plated", "Cast Iron", "Double Plated", "Copper", "Gold",
		"Iron", "Pointy", "Master", "Nickel", "Rusty",
		"Silver", "Steel", "Tin", "Femur", "Scheelite",
		"Black", "Zinc", "Aluminum-Plated", "Copper-Plated", "Gold-Plated",
		"Nickel-Plated", "Silver-Plated", "Steel-Plated", "Tin-Plated", "Zinc-Plated",
		"Dragon Tooth", "Unicorn", "Runed", "Bronze", "Brass",
		"Crystal", "Lead","Lead-Plated", "Ivory" , "Bejeweled",
		"Wizard's", "Long", "Short", "Hexagonal"
};

static byte wand_col[MAX_METALS] =
{
		TERM_L_BLUE, TERM_L_DARK, TERM_WHITE, TERM_L_UMBER, TERM_YELLOW,
		TERM_SLATE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_UMBER, TERM_RED,
		TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_WHITE, TERM_WHITE,
		TERM_L_DARK, TERM_L_WHITE, TERM_L_BLUE, TERM_L_UMBER, TERM_YELLOW,
		TERM_L_UMBER, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE,
		TERM_WHITE, TERM_WHITE, TERM_UMBER, TERM_L_UMBER, TERM_L_UMBER,
		TERM_L_BLUE, TERM_SLATE, TERM_SLATE, TERM_WHITE, TERM_VIOLET,
		TERM_L_RED, TERM_L_BLUE, TERM_BLUE, TERM_RED
};


/*
* Rods (adjectives and colours).
* Efficiency -- copied from wand arrays
*/

static cptr rod_adj[MAX_METALS];

static byte rod_col[MAX_METALS];

/*
* Mushrooms (adjectives and colours)
*/

static cptr food_adj[MAX_SHROOM] =
{
		"Blue", "Black", "Black Spotted", "Brown", "Dark Blue",
		"Dark Green", "Dark Red", "Yellow", "Furry", "Green",
		"Grey", "Light Blue", "Light Green", "Violet", "Red",
		"Slimy", "Tan", "White", "White Spotted", "Wrinkled",
};

static byte food_col[MAX_SHROOM] =
{
		TERM_BLUE, TERM_L_DARK, TERM_L_DARK, TERM_UMBER, TERM_BLUE,
		TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_L_WHITE, TERM_GREEN,
		TERM_SLATE, TERM_L_BLUE, TERM_L_GREEN, TERM_VIOLET, TERM_RED,
		TERM_SLATE, TERM_L_UMBER, TERM_WHITE, TERM_WHITE, TERM_UMBER
};


/*
* Color adjectives and colours, for potions.
* Hack -- The first four entries are hard-coded.
* (water, apple juice, slime mold juice, something)
*/

static cptr potion_adj[MAX_COLORS] =
{
		"Clear", "Light Brown", "Icky Green", "Burgundy",
		"Azure", "Blue", "Blue Speckled", "Black", "Brown", "Brown Speckled",
		"Bubbling", "Chartreuse", "Cloudy", "Copper Speckled", "Crimson", "Cyan",
		"Dark Blue", "Dark Green", "Dark Red", "Gold Speckled", "Green",
		"Green Speckled", "Grey", "Grey Speckled", "Hazy", "Indigo",
		"Light Blue", "Light Green", "Magenta", "Metallic Blue", "Metallic Red",
		"Metallic Green", "Metallic Purple", "Misty", "Orange", "Orange Speckled",
		"Pink", "Pink Speckled", "Puce", "Purple", "Purple Speckled",
		"Red", "Red Speckled", "Silver Speckled", "Smoky", "Tangerine",
		"Violet", "Vermilion", "White", "Yellow", "Violet Speckled",
		"Pungent", "Clotted Red", "Viscous Pink", "Oily Yellow", "Gloopy Green",
		"Shimmering", "Coagulated Crimson", "Yellow Speckled", "Gold",
		"Manly", "Stinking", "Oily Black", "Ichor", "Ivory White", "Sky Blue",
};

static byte potion_col[MAX_COLORS] =
{
	TERM_WHITE, TERM_L_UMBER, TERM_GREEN, 0,
		TERM_L_BLUE, TERM_BLUE, TERM_BLUE, TERM_L_DARK, TERM_UMBER, TERM_UMBER,
		TERM_L_WHITE, TERM_L_GREEN, TERM_WHITE, TERM_L_UMBER, TERM_RED, TERM_L_BLUE,
		TERM_BLUE, TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_GREEN,
		TERM_GREEN, TERM_SLATE, TERM_SLATE, TERM_L_WHITE, TERM_VIOLET,
		TERM_L_BLUE, TERM_L_GREEN, TERM_RED, TERM_BLUE, TERM_RED,
		TERM_GREEN, TERM_VIOLET, TERM_L_WHITE, TERM_ORANGE, TERM_ORANGE,
		TERM_L_RED, TERM_L_RED, TERM_VIOLET, TERM_VIOLET, TERM_VIOLET,
		TERM_RED, TERM_RED, TERM_L_WHITE, TERM_L_DARK, TERM_ORANGE,
		TERM_VIOLET, TERM_RED, TERM_WHITE, TERM_YELLOW, TERM_VIOLET,
		TERM_L_RED, TERM_RED, TERM_L_RED, TERM_YELLOW, TERM_GREEN,
		TERM_MULTI, TERM_RED, TERM_YELLOW, TERM_YELLOW,
		TERM_L_UMBER, TERM_UMBER, TERM_L_DARK, TERM_RED, TERM_WHITE, TERM_L_BLUE
};


/*
* Syllables for scrolls (must be 1-4 letters each)
*/

static cptr syllables[MAX_SYLLABLES] =
{
		"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
		"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
		"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
		"byt", "comp", "con", "cos", "cre", "dalf", "dan",
		"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
		"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
		"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
		"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
		"kho", "kli", "klis", "la", "lech", "man", "mar",
		"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
		"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
		"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
		"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
		"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
		"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
		"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
		"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
		"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
		"vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
		"yerg", "yp", "zun", "tri", "blaa", "jah", "bul", "on",
		"foo", "ju", "xuxu"
};


/*
* Hold the titles of scrolls, 6 to 14 characters each
* Also keep an array of scroll colours (always WHITE for now)
*/

static char scroll_adj[MAX_TITLES][16];

static byte scroll_col[MAX_TITLES];

#define ACT_NOTHING     0
#define ACT_SHOW_WEAPON 1
#define ACT_SHOW_ARMOUR 2
#define ACT_FULL_MONTY  3
#define NO_FLAVORS NULL
#define EASY TRUE
#define HARD FALSE

static tv_describer_type tv_describers[] =
{  /* tval            describing actions   flavors         no flavor      with flavor*/
	{ TV_SKELETON     , EASY , TERM_WHITE   , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_BOTTLE       , EASY , TERM_WHITE   , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_JUNK         , EASY , TERM_WHITE   , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SPIKE        , EASY , TERM_SLATE   , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_FLASK        , EASY , TERM_YELLOW  , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_CHEST        , HARD , TERM_SLATE   , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SHOT         , HARD , TERM_UMBER   , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_BOLT         , HARD , TERM_UMBER   , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_ARROW        , HARD , TERM_UMBER   , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_BOW          , HARD , TERM_UMBER   , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_HAFTED       , HARD , TERM_L_WHITE , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_POLEARM      , HARD , TERM_L_WHITE , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SWORD        , HARD , TERM_L_WHITE , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_DIGGING      , HARD , TERM_SLATE   , ACT_SHOW_WEAPON	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_BOOTS        , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_GLOVES       , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_CLOAK        , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_CROWN        , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_HELM         , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SHIELD       , HARD , TERM_L_UMBER , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SOFT_ARMOR   , HARD , TERM_SLATE   , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_HARD_ARMOR   , HARD , TERM_SLATE   , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_DRAG_ARMOR   , HARD , TERM_SLATE   , ACT_SHOW_ARMOUR	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_LITE         , HARD , TERM_YELLOW  , ACT_NOTHING		, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_AMULET       , HARD , TERM_ORANGE  , ACT_FULL_MONTY	, amulet_adj	, "& Amulet~"	, "& # Amulet~"				},
	{ TV_RING         , HARD , TERM_ORANGE  , ACT_FULL_MONTY	, ring_adj		, "& Ring~"		, "& # Ring~"				},
	{ TV_STAFF        , EASY , TERM_UMBER   , ACT_FULL_MONTY	, staff_adj		, "& Staff~"	, "& # Staff~"				},
	{ TV_WAND         , EASY , TERM_GREEN   , ACT_FULL_MONTY	, wand_adj		, "& Wand~"		, "& # Wand~"				},
	{ TV_ROD          , EASY , TERM_VIOLET  , ACT_FULL_MONTY	, rod_adj		, "& Rod~"		, "& # Rod~"				},
	{ TV_SCROLL       , EASY , TERM_WHITE   , ACT_FULL_MONTY	, NULL			, "& Scroll~"	, "& Scroll~ titled \"#\""	},
	{ TV_POTION       , EASY , TERM_BLUE    , ACT_FULL_MONTY	, potion_adj	, "& Potion~"	, "& # Potion~"				},
	{ TV_FOOD         , EASY , TERM_L_UMBER , ACT_FULL_MONTY	, food_adj		, "& Mushroom~"	, "& # Mushroom~"			},
	{ TV_MIRACLES_BOOK, EASY , TERM_WHITE   , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SORCERY_BOOK , EASY , TERM_L_BLUE  , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_NATURE_BOOK  , EASY , TERM_L_GREEN , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_CHAOS_BOOK   , EASY , TERM_L_RED   , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_DEATH_BOOK   , EASY , TERM_L_DARK  , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_TAROT_BOOK   , EASY , TERM_ORANGE  , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_CHARMS_BOOK  , EASY , TERM_L_WHITE , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_SOMATIC_BOOK , EASY , TERM_YELLOW  , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
	{ TV_DEMONIC_BOOK , EASY , TERM_RED     , ACT_NOTHING 	, NO_FLAVORS	, NO_FLAVORS	, NO_FLAVORS						},
};

static tv_describer_type read_tval_description( byte tval)
{
	int i;
	for( i = 0 ; i < N_ELEMENTS( tv_describers ) ; i++ )
	{
		if( tv_describers[i].tval == tval )
		{
			return tv_describers[i];
		}
	}
	/* msg_format("Could not translate tval %d." , tval); */
	return tv_describers[0];
}

/* Not sure this is a great idea.., I just suck at pointer magic.. */
static cptr info[128];

/*
 * Certain items have a flavor
 * This function is used only by "flavor_init()"
 */
static bool object_has_flavor(int i)
{
	object_kind *k_ptr = &k_info[i];
	tv_describer_type describer;

	/* The hard-coded mushroom exception */
	if( k_ptr->tval == TV_FOOD )
	{
		if (k_ptr->sval < SV_FOOD_MIN_FOOD) return (TRUE);
		return (FALSE);
	}
	
	/* Get describer */
	describer = read_tval_description( k_ptr->tval );
	/* Only full monty tvals have flavor */
	if( describer.action == ACT_FULL_MONTY )
		return TRUE;

	/* Assume no flavor */
	return (FALSE);
}


/*
* Certain items, if aware, are known instantly
* This function is used only by "flavor_init()"
*
* XXX XXX XXX Add "EASY_KNOW" flag to "k_info.txt" file
*/
static bool object_easy_know(int i)
{
	object_kind *k_ptr = &k_info[i];
	tv_describer_type describer;
	
	/* First rule, if it is easy know, it is easy know */
	if (k_ptr->flags3 & (TR3_EASY_KNOW)) return (TRUE);
	/* Get describer */
	describer = read_tval_description( k_ptr->tval );
	/* Weapons and armor are not easy */
	return describer.easy_know;
}


/*
* Hack -- prepare the default object attr codes by tval
*
* XXX XXX XXX Off-load to "pref.prf" file
*/
static byte default_tval_to_attr(int tval)
{
	tv_describer_type describer;
	/* Get describer */
	describer = read_tval_description( tval );
	/* Return color */
	return describer.attribute;
}


/*
* Hack -- prepare the default object char codes by tval
*
* XXX XXX XXX Off-load to "pref.prf" file (?)
*/
static byte default_tval_to_char(int tval)
{
	int i;

	/* Hack -- Guess at "correct" values for tval_to_char[] */
	for (i = 1; i < MAX_K_IDX; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Use the first value we find */
		if (k_ptr->tval == tval) return (k_ptr->k_char);
	}

	/* Default to space */
	return (' ');
}


/*
* Prepare the "variable" part of the "k_info" array.
*
* The "colour"/"metal"/"type" of an item is its "flavor".
* For the most part, flavors are assigned randomly each game.
*
* Initialize descriptions for the "coloured" objects, including:
* Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
*
* The first 4 entries for potions are fixed (Water, Apple Juice,
* Slime Mold Juice, Unused Potion).
*
* Scroll titles are always between 6 and 14 letters long.  This is
* ensured because every title is composed of whole words, where every
* word is from 1 to 8 letters long (one or two syllables of 1 to 4
* letters each), and that no scroll is finished until it attempts to
* grow beyond 15 letters.  The first time this can happen is when the
* current title has 6 letters and the new word has 8 letters, which
* would result in a 6 letter scroll title.
*
* Duplicate titles are avoided by requiring that no two scrolls share
* the same first four letters (not the most efficient method, and not
* the least efficient method, but it will always work).
*
* Hack -- make sure everything stays the same for each saved game
* This is accomplished by the use of a saved "random seed", as in
* "town_gen()".  Since no other functions are called while the special
* seed is in effect, so this function is pretty "safe".
*
* Note that the "hacked seed" may provide an RNG with alternating parity!
*/
void flavor_init(void)
{
	int             i, j;

	byte    temp_col;

	cptr    temp_adj;

	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;


	/* Efficiency -- Rods/Wands share initial array */
	for (i = 0; i < MAX_METALS; i++)
	{
		rod_adj[i] = wand_adj[i];
		rod_col[i] = wand_col[i];
	}


	/* Rings have "ring colours" */
	for (i = 0; i < MAX_ROCKS; i++)
	{
		j = rand_int(MAX_ROCKS);
		temp_adj = ring_adj[i];
		ring_adj[i] = ring_adj[j];
		ring_adj[j] = temp_adj;
		temp_col = ring_col[i];
		ring_col[i] = ring_col[j];
		ring_col[j] = temp_col;
	}

	/* Amulets have "amulet colours" */
	for (i = 0; i < MAX_AMULETS; i++)
	{
		j = rand_int(MAX_AMULETS);
		temp_adj = amulet_adj[i];
		amulet_adj[i] = amulet_adj[j];
		amulet_adj[j] = temp_adj;
		temp_col = amulet_col[i];
		amulet_col[i] = amulet_col[j];
		amulet_col[j] = temp_col;
	}

	/* Staffs */
	for (i = 0; i < MAX_WOODS; i++)
	{
		j = rand_int(MAX_WOODS);
		temp_adj = staff_adj[i];
		staff_adj[i] = staff_adj[j];
		staff_adj[j] = temp_adj;
		temp_col = staff_col[i];
		staff_col[i] = staff_col[j];
		staff_col[j] = temp_col;
	}

	/* Wands */
	for (i = 0; i < MAX_METALS; i++)
	{
		j = rand_int(MAX_METALS);
		temp_adj = wand_adj[i];
		wand_adj[i] = wand_adj[j];
		wand_adj[j] = temp_adj;
		temp_col = wand_col[i];
		wand_col[i] = wand_col[j];
		wand_col[j] = temp_col;
	}

	/* Rods */
	for (i = 0; i < MAX_METALS; i++)
	{
		j = rand_int(MAX_METALS);
		temp_adj = rod_adj[i];
		rod_adj[i] = rod_adj[j];
		rod_adj[j] = temp_adj;
		temp_col = rod_col[i];
		rod_col[i] = rod_col[j];
		rod_col[j] = temp_col;
	}

	/* Foods (Mushrooms) */
	for (i = 0; i < MAX_SHROOM; i++)
	{
		j = rand_int(MAX_SHROOM);
		temp_adj = food_adj[i];
		food_adj[i] = food_adj[j];
		food_adj[j] = temp_adj;
		temp_col = food_col[i];
		food_col[i] = food_col[j];
		food_col[j] = temp_col;
	}

	/* Potions */
	for (i = 4; i < MAX_COLORS; i++)
	{
		j = rand_int(MAX_COLORS - 4) + 4;
		temp_adj = potion_adj[i];
		potion_adj[i] = potion_adj[j];
		potion_adj[j] = temp_adj;
		temp_col = potion_col[i];
		potion_col[i] = potion_col[j];
		potion_col[j] = temp_col;
	}

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		/* Get a new title */
		while (TRUE)
		{
			char buf[80];

			bool okay;

			/* Start a new title */
			buf[0] = '\0';

			/* Collect words until done */
			while (1)
			{
				int q, s;

				char tmp[80];

				/* Start a new word */
				tmp[0] = '\0';

				/* Choose one or two syllables */
				s = ((rand_int(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				for (q = 0; q < s; q++)
				{
					/* Add the syllable */
					strcat(tmp, syllables[rand_int(MAX_SYLLABLES)]);
				}

				/* Stop before getting too long */
				if (strlen(buf) + 1 + strlen(tmp) > 15) break;

				/* Add a space */
				strcat(buf, " ");

				/* Add the word */
				strcat(buf, tmp);
			}

			/* Save the title */
			strcpy(scroll_adj[i], buf+1);

			/* Assume okay */
			okay = TRUE;

			/* Check for "duplicate" scroll titles */
			for (j = 0; j < i; j++)
			{
				cptr hack1 = scroll_adj[j];
				cptr hack2 = scroll_adj[i];

				/* Compare first four characters */
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;

				/* Not okay */
				okay = FALSE;

				/* Stop looking */
				break;
			}

			/* Break when done */
			if (okay) break;
		}

		/* All scrolls are white */
		scroll_col[i] = TERM_WHITE;
	}


	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < MAX_K_IDX; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* Check for a "flavor" */
		k_ptr->has_flavor = object_has_flavor(i);

		/* No flavor yields aware */
		if (!k_ptr->has_flavor) k_ptr->aware = TRUE;

		/* Check for "easily known" */
		k_ptr->easy_know = object_easy_know(i);
	}
}


/*
* Extract the "default" attr for each object
* This function is used only by "flavor_init()"
*/
static byte object_d_attr(int i)
{
	object_kind *k_ptr = &k_info[i];

	/* Flavored items */
	if (k_ptr->has_flavor)
	{
		/* Extract the indexx */
		int indexx = k_ptr->sval;

		/* Analyze the item */
		switch (k_ptr->tval)
		{
		case TV_FOOD:   return (food_col[indexx]);
		case TV_POTION: return (potion_col[indexx]);
		case TV_SCROLL: return (scroll_col[indexx]);
		case TV_AMULET: return (amulet_col[indexx]);
		case TV_RING:   return (ring_col[indexx]);
		case TV_STAFF:  return (staff_col[indexx]);
		case TV_WAND:   return (wand_col[indexx]);
		case TV_ROD:    return (rod_col[indexx]);
		}
	}

	/* Default attr if legal */
	if (k_ptr->k_attr) return (k_ptr->k_attr);

	/* Default to white */
	return (TERM_WHITE);
}


/*
* Extract the "default" char for each object
* This function is used only by "flavor_init()"
*/
static byte object_d_char(int i)
{
	object_kind *k_ptr = &k_info[i];

	return (k_ptr->k_char);
}


/*
* Reset the "visual" lists
*
* This involves resetting various things to their "default"
* state, and then loading the appropriate "user pref file"
* based on the "use_graphics" flag.
*
* This is useful for switching "graphics" on/off
*/
void reset_visuals(void)
{
	int i;

	char buf[1024];

	/* Extract some info about terrain features */
	for (i = 0; i < MAX_F_IDX; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		f_ptr->z_attr = f_ptr->f_attr;
		f_ptr->z_char = f_ptr->f_char;
	}

	/* Extract some info about objects */
	for (i = 0; i < MAX_K_IDX; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Extract the "underlying" attr */
		k_ptr->d_attr = object_d_attr(i);

		/* Extract the "underlying" char */
		k_ptr->d_char = object_d_char(i);

		/* Assume we will use the underlying values */
		k_ptr->x_attr = k_ptr->d_attr;
		k_ptr->x_char = k_ptr->d_char;
	}

	/* Extract some info about monsters */
	for (i = 0; i < MAX_R_IDX; i++)
	{
		/* Extract the "underlying" attr */
		r_info[i].x_attr = r_info[i].d_attr;

		/* Extract the "underlying" char */
		r_info[i].x_char = r_info[i].d_char;
	}

	/* Extract attr/chars for equippy items (by tval) */
	for (i = 0; i < 128; i++)
	{
		/* Extract a default attr */
		tval_to_attr[i] = default_tval_to_attr(i);

		/* Extract a default char */
		tval_to_char[i] = default_tval_to_char(i);
	}

	if( arg_tile_size == 0 )
		arg_tile_size = 16;

	/* Access the "font" or "graf" pref file, based on "use_graphics" */
	sprintf(buf, "%s-%s-%d.prf", (use_graphics ? "graf" : "font"), ANGBAND_SYS , arg_tile_size );

	/* Process that file */
	process_pref_file(buf);
}

/*
 * Obtain the "flags" for an item
 */
void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Base object */
	(*f1) = k_ptr->flags1;
	(*f2) = k_ptr->flags2;
	(*f3) = k_ptr->flags3;

	/* Artifact, append to the base object */
	if (o_ptr->name1)
	{
		artefact_type *a_ptr = &a_info[o_ptr->name1];

		(*f1) |= a_ptr->flags1;
		(*f2) |= a_ptr->flags2;
		(*f3) |= a_ptr->flags3;
	}

	/* Ego-item */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		(*f1) |= e_ptr->flags1;
		(*f2) |= e_ptr->flags2;
		(*f3) |= e_ptr->flags3;
	}

	/* Random artefact or blasted weapon ! */
	if (o_ptr->art_flags1 || o_ptr->art_flags2 || o_ptr->art_flags3 )
	{
		(*f1) |= o_ptr->art_flags1;
		(*f2) |= o_ptr->art_flags2;
		(*f3) |= o_ptr->art_flags3;

	}

	/* Extra powers, unless we're talking sentient weapon.. */
	if (!(o_ptr->art_name) && !( (*f3) & TR3_XP ) )
	{
		switch (o_ptr->xtra1)
		{
		case EGO_XTRA_SUSTAIN:
			{
				/* Choose a sustain */
				switch (o_ptr->xtra2 % 6)
				{
				case 0: (*f2) |= (TR2_SUST_STR); break;
				case 1: (*f2) |= (TR2_SUST_INT); break;
				case 2: (*f2) |= (TR2_SUST_WIS); break;
				case 3: (*f2) |= (TR2_SUST_DEX); break;
				case 4: (*f2) |= (TR2_SUST_CON); break;
				case 5: (*f2) |= (TR2_SUST_CHA); break;
				}

				break;
			}

		case EGO_XTRA_POWER:
			{
				/* Choose a power */
				switch (o_ptr->xtra2 % 11)
				{
				case 0: (*f2) |= (TR2_RES_BLIND); break;
				case 1: (*f2) |= (TR2_RES_CONF); break;
				case 2: (*f2) |= (TR2_RES_SOUND); break;
				case 3: (*f2) |= (TR2_RES_SHARDS); break;
				case 4: (*f2) |= (TR2_RES_NETHER); break;
				case 5: (*f2) |= (TR2_RES_NEXUS); break;
				case 6: (*f2) |= (TR2_RES_CHAOS); break;
				case 7: (*f2) |= (TR2_RES_DISEN); break;
				case 8: (*f2) |= (TR2_RES_POIS); break;
				case 9: (*f2) |= (TR2_RES_DARK); break;
				case 10: (*f2) |= (TR2_RES_LITE); break;
				}

				break;
			}

		case EGO_XTRA_ABILITY:
			{
				/* Choose an ability */
				switch (o_ptr->xtra2 % 8)
				{
				case 0: (*f3) |= (TR3_FEATHER); break;
				case 1: (*f3) |= (TR3_LITE); break;
				case 2: (*f3) |= (TR3_SEE_INVIS); break;
				case 3: (*f3) |= (TR3_TELEPATHY); break;
				case 4: (*f3) |= (TR3_SLOW_DIGEST); break;
				case 5: (*f3) |= (TR3_REGEN); break;
				case 6: (*f2) |= (TR2_FREE_ACT); break;
				case 7: (*f2) |= (TR2_HOLD_LIFE); break;
				}
				break;
			}
		}
	}
}



/*
* Obtain the "flags" for an item which are known to the player
*/
void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	bool spoil = FALSE;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Clear */
	(*f1) = (*f2) = (*f3) = 0L;

	/* Must be identified */
	if (!object_known_p(o_ptr)) return;

	/* Base object */
	(*f1) = k_ptr->flags1;
	(*f2) = k_ptr->flags2;
	(*f3) = k_ptr->flags3;

	/* Ego-item (known basic flags) */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		(*f1) |= e_ptr->flags1;
		(*f2) |= e_ptr->flags2;
		(*f3) |= e_ptr->flags3;
	}


#ifdef SPOIL_ARTIFACTS
	/* Full knowledge for some artefacts */
	if (artefact_p(o_ptr) || o_ptr->art_name) spoil = TRUE;
#endif

#ifdef SPOIL_EGO_ITEMS
	/* Full knowledge for some ego-items */
	if (ego_item_p(o_ptr)) spoil = TRUE;
#endif

	/* Need full knowledge or spoilers */
	if (!spoil && !(o_ptr->ident & IDENT_MENTAL)) return;

	/* Artifact */
	if (o_ptr->name1)
	{
		artefact_type *a_ptr = &a_info[o_ptr->name1];

		(*f1) = a_ptr->flags1;
		(*f2) = a_ptr->flags2;
		(*f3) = a_ptr->flags3;
	}

	/* Ego-item */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		(*f1) |= e_ptr->flags1;
		(*f2) |= e_ptr->flags2;
		(*f3) |= e_ptr->flags3;
	}

	/* Random artefact or sentient weapon ! */
	if (o_ptr->art_flags1 || o_ptr->art_flags2 || o_ptr->art_flags3)
	{
		(*f1) |= o_ptr->art_flags1;
		(*f2) |= o_ptr->art_flags2;
		(*f3) |= o_ptr->art_flags3;
	}

	/* Full knowledge for *identified* objects */
	if (!(o_ptr->ident & IDENT_MENTAL)) return;

	if (!(o_ptr->art_name))
	{
		/* Extra powers */
		switch (o_ptr->xtra1)
		{
		case EGO_XTRA_SUSTAIN:
			{
				/* Choose a sustain */
				switch (o_ptr->xtra2 % 6)
				{
				case 0: (*f2) |= (TR2_SUST_STR); break;
				case 1: (*f2) |= (TR2_SUST_INT); break;
				case 2: (*f2) |= (TR2_SUST_WIS); break;
				case 3: (*f2) |= (TR2_SUST_DEX); break;
				case 4: (*f2) |= (TR2_SUST_CON); break;
				case 5: (*f2) |= (TR2_SUST_CHA); break;
				}

				break;
			}

		case EGO_XTRA_POWER:
			{
				/* Choose a power */
				switch (o_ptr->xtra2 % 11)
				{
				case 0: (*f2) |= (TR2_RES_BLIND); break;
				case 1: (*f2) |= (TR2_RES_CONF); break;
				case 2: (*f2) |= (TR2_RES_SOUND); break;
				case 3: (*f2) |= (TR2_RES_SHARDS); break;
				case 4: (*f2) |= (TR2_RES_NETHER); break;
				case 5: (*f2) |= (TR2_RES_NEXUS); break;
				case 6: (*f2) |= (TR2_RES_CHAOS); break;
				case 7: (*f2) |= (TR2_RES_DISEN); break;
				case 8: (*f2) |= (TR2_RES_POIS); break;
				case 9: (*f2) |= (TR2_RES_DARK); break;
				case 10: (*f2) |= (TR2_RES_LITE); break;
				}

				break;
			}

		case EGO_XTRA_ABILITY:
			{
				/* Choose an ability */
				switch (o_ptr->xtra2 % 8)
				{
				case 0: (*f3) |= (TR3_FEATHER); break;
				case 1: (*f3) |= (TR3_LITE); break;
				case 2: (*f3) |= (TR3_SEE_INVIS); break;
				case 3: (*f3) |= (TR3_TELEPATHY); break;
				case 4: (*f3) |= (TR3_SLOW_DIGEST); break;
				case 5: (*f3) |= (TR3_REGEN); break;
				case 6: (*f2) |= (TR2_FREE_ACT); break;
				case 7: (*f2) |= (TR2_HOLD_LIFE); break;
				}

				break;
			}
		}
	}
}


/*
* Print a char "c" into a string "t", as if by sprintf(t, "%c", c),
* and return a pointer to the terminator (t + 1).
*/
static char *object_desc_chr(char *t, char c)
{
	/* Copy the char */
	*t++ = c;

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}


/*
* Print a string "s" into a string "t", as if by strcpy(t, s),
* and return a pointer to the terminator.
*/
static char *object_desc_str(char *t, cptr s)
{
	/* Copy the string */
	while (*s) *t++ = *s++;

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}



/*
* Print an unsigned number "n" into a string "t", as if by
* sprintf(t, "%u", n), and return a pointer to the terminator.
*/
static char *object_desc_num(char *t, uint n)
{
	uint p;

	/* Find "size" of "n" */
	for (p = 1; n >= p * 10; p = p * 10) /* loop */;

	/* Dump each digit */
	while (p >= 1)
	{
		/* Dump the digit */
		*t++ = '0' + n / p;

		/* Remove the digit */
		n = n % p;

		/* Process next digit */
		p = p / 10;
	}

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}

/*
* Print an signed number "v" into a string "t", as if by
* sprintf(t, "%+d", n), and return a pointer to the terminator.
* Note that we always print a sign, either "+" or "-".
*/
static char *object_desc_int(char *t, sint v)
{
	uint p, n;

	/* Negative */
	if (v < 0)
	{
		/* Take the absolute value */
		n = 0 - v;

		/* Use a "minus" sign */
		*t++ = '-';
	}

	/* Positive (or zero) */
	else
	{
		/* Use the actual number */
		n = v;

		/* Use a "plus" sign */
		*t++ = '+';
	}

	/* Find "size" of "n" */
	for (p = 1; n >= p * 10; p = p * 10) /* loop */;

	/* Dump each digit */
	while (p >= 1)
	{
		/* Dump the digit */
		*t++ = '0' + n / p;

		/* Remove the digit */
		n = n % p;

		/* Process next digit */
		p = p / 10;
	}

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}



/*
* Creates a description of the item "o_ptr", and stores it in "out_val".
*
* One can choose the "verbosity" of the description, including whether
* or not the "number" of items should be described, and how much detail
* should be used when describing the item.
*
* The given "buf" must be 80 chars long to hold the longest possible
* description, which can get pretty long, including incriptions, such as:
* "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
* Note that the inscription will be clipped to keep the total description
* under 79 chars (plus a terminator).
*
* Note the use of "object_desc_num()" and "object_desc_int()" as hyper-efficient,
* portable, versions of some common "sprintf()" commands.
*
* Note that all ego-items (when known) append an "Ego-Item Name", unless
* the item is also an artefact, which should NEVER happen.
*
* Note that all artefacts (when known) append an "Artifact Name", so we
* have special processing for "Specials" (artefact Lites, Rings, Amulets).
* The "Specials" never use "modifiers" if they are "known", since they
* have special "descriptions", such as "The Necklace of the Dwarves".
*
* Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
* plus the artefact name, just like any other artefact, if known.
*
* Special Ring's and Amulet's, if not "aware", use the same code as normal
* rings and amulets, and if "aware", use the "k_info" base-name (Ring or
* Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
* they will append the artefact name, just like any artefact, if known.
*
* None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
* at least, those which have no "pluses", such as the three artefact lites.
*
* Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
*
* If "pref" then a "numeric" prefix will be pre-pended.
*
* Mode:
*   0 -- The Cloak of Death
*   1 -- The Cloak of Death [1,+3]
*   2 -- The Cloak of Death [1,+3] (+2 to Stealth)
*   3 -- The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
*/

/* Julian Lighton's improved code */
void object_desc(char *buf, object_type *o_ptr, int pref, int mode)
{
	cptr            basenm, modstr;
	int             i, power, indexx;

	bool            aware = FALSE;
	bool            known = FALSE;

	bool            append_name = FALSE;

	bool            show_weapon = FALSE;
	bool            show_armour = FALSE;

	/* All it takes to do prefix ego types */
	cptr			egostr;
	bool            ego_prefix = FALSE;
	bool            ego_prefix_done = FALSE;

	/* All takes for artefact name overrides , we start
	   with the conviction that the a_info name does not override the k_info name */
	bool            artefact_overrides = FALSE;

	bool            tval_found = FALSE;

	cptr            s, u;
	char            *t;

	char            p1 = '(', p2 = ')';
	char            b1 = '[', b2 = ']';
	char            c1 = '{', c2 = '}';

	char            tmp_val[160];     /* Buffer for the description of the thing */
	char            tmp_val2[90];     /* Buffer the inscription of the thing */

	u32b            f1, f2, f3;

	object_kind             *k_ptr = &k_info[o_ptr->k_idx];
	artefact_type			*a_ptr = &a_info[o_ptr->name1];  /* This could be null pointer ! */

	tv_describer_type describer;
	
	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* See if the object is "aware" */
	if (object_aware_p(o_ptr)) aware = TRUE;

	/* See if the object is "known" */
	if (object_known_p(o_ptr)) known = TRUE;

	/* Hack -- Extract the sub-type "indexx" */
	indexx = o_ptr->sval;

	/* Extract default "base" string */
 	basenm = (k_name + k_ptr->name);

	/* Extract potential artefact string , still knowing that this could point to nothing */
	if (artefact_p(o_ptr) && aware && known )
	{ /* This gives some assurance that we are not point to null */
	  basenm = (a_name + a_ptr->name);
		if( basenm[0] == '!' )
		{
		    artefact_overrides = TRUE; /* This will prevent a double display later in the code */
			basenm++; /* This will prevent the exclamation mark in the artifact name to show up */
		}else{
			basenm = (k_name + k_ptr->name); /* False alert, let the the old logic run */
		}
	}

	/* Assume no "modifier" string */
	modstr = "";
	
	/* Hack -- Gold/Gems */
	if(o_ptr->tval == TV_GOLD)
	{
		strcpy(buf, basenm);
		return;
	}

	/* Check for spellbooks, magic users get a different basename */
	for( i = 1 ; i < MAX_REALM+1 ; i++ )
	{
		if( realm_names[i].tval == o_ptr->tval )
		{
			modstr = basenm;
			basenm = (mp_ptr->spell_book == TV_MIRACLES_BOOK)?realm_names[i].basename1:realm_names[i].basename2;
			tval_found = TRUE;
		}
	}

	if( !tval_found )
	{
		/* Get describer */
		describer = read_tval_description( o_ptr->tval );

		/* Hack ^ 2 Potions with sval being -1 are vials, indicating to player they are quite different */
		
		if( o_ptr->tval == TV_POTION && o_ptr->pval == -1 )
		{
			describer.plain = "& Vial~";
			describer.full  = "& # Vial~";
		}

		/* Make sure you found it */
		if( describer.tval == o_ptr->tval )
		{
			tval_found = TRUE;
			if( describer.action == ACT_SHOW_WEAPON )
				show_weapon = TRUE;
			if( describer.action == ACT_SHOW_ARMOUR )
				show_armour = TRUE;
			if( describer.action == ACT_FULL_MONTY )
			{
				/* The hard-coded mushroom exception */
				if( o_ptr->tval == TV_FOOD && o_ptr->sval >= SV_FOOD_MIN_FOOD )
					goto legacy_break;
				
				/* Known artefacts */
				if (artefact_p(o_ptr) && aware)
					goto legacy_break;

				/* Color the object */
				if( describer.adj != NULL )
					modstr = describer.adj[indexx];
				else
				/* Only scrolls are known to be a hack^2 */
					modstr = scroll_adj[indexx];
				if (aware) append_name = TRUE;
				if (((plain_descriptions) && (aware))  || o_ptr->ident & IDENT_STOREB)
					basenm = describer.plain;
				else
					basenm = describer.full;
			}
		}
	}
/* Hack from when we were still looping */
legacy_break:

	if(!tval_found)
	{
		strcpy(buf, "(nothing)");
		return;
	}

	/* Calm down compiler, waste a few bytes */
	egostr = "";

	/* If we know the object, and the object is an ego item and the description
	 * starts with &, then we are dealing with an ego that has a prefixed name
	 * this allows for 'Vampiric Dagger' instead of 'Dagger (Vampiric) */
	if( known && o_ptr->name2 && suffix( e_name + e_info[o_ptr->name2].name , "&" ) )
	{
		ego_prefix = TRUE;
		egostr = e_name + e_info[o_ptr->name2].name;
	}

	/* Start dumping the result */
	t = tmp_val;

	/* The object "expects" a "number" */
	if (basenm[0] == '&')
	{
		/* Skip the ampersand (and space) */
		s = basenm + 2;

		/* No prefix */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			t = object_desc_str(t, "no more ");
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			t = object_desc_num(t, o_ptr->number);
			t = object_desc_chr(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && (artefact_p(o_ptr) || o_ptr->art_name))
		{
			t = object_desc_str(t, "The ");
		}

		/* A single one, with a vowel in the modifier, no ego prefix */
		else if ((*s == '#') && (is_a_vowel(modstr[0])) && !ego_prefix)
		{
			t = object_desc_str(t, "an ");
		}
		/* A single one, with a vowel in the modifier, no ego prefix */
		else if (ego_prefix && is_a_vowel(egostr[0]) )
		{
			t = object_desc_str(t, "an ");
		}
		/* A single one, with a vowel */
		else if (is_a_vowel(*s))
		{
			t = object_desc_str(t, "an ");
		}

		/* A single one, without a vowel */
		else
		{
			t = object_desc_str(t, "a ");
		}
	}

	/* Hack -- objects that "never" take an article */
	else
	{
		/* No ampersand */
		s = basenm;

		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			t = object_desc_str(t, "no more ");
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			t = object_desc_num(t, o_ptr->number);
			t = object_desc_chr(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && (artefact_p(o_ptr) || o_ptr->art_name))
		{
			t = object_desc_str(t, "The ");
		}

		/* Hack -- single items get no prefix */
		else
		{
			/* Nothing */
		}
	}

	/* Paranoia -- skip illegal tildes */
	/* while (*s == '~') s++; */

	/* Copy the string */
	for (; *s; s++)
	{
		/* Pluralizer */
		if (*s == '~')
		{
			/* Add a plural if needed */
			if (o_ptr->number != 1)
			{
				char k = t[-1];

				/* XXX XXX XXX Mega-Hack */

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h')) *t++ = 'e';

				/* Add an 's' */
				*t++ = 's';
			}
		}
		/* Ego prefix, as documented before,
		   prefixes cannot display & as it is the anchor,
		   plus it would look silly */
		else if( ego_prefix && !ego_prefix_done )
		{
			for (u = egostr; *u; u++)
			{
				if(*u != '&')
					*t++ = *u;
			}
			ego_prefix_done = TRUE;

			/* HACK^2, still do the copy */
			*t++ = *s;
		}
		/* Modifier */
		else if (*s == '#')
		{
			/* Insert the modifier */
			for (u = modstr; *u; u++) *t++ = *u;
		}

		/* Normal */
		else
		{
			/* Copy */
			*t++ = *s;
		}
	}

	/* Terminate */
	*t = '\0';

	/* words ending in ius end in ii */
	if( o_ptr->number != 1 )
	{
		if( t[-3] == 'i' && t[-2] == 'u' && t[-1] == 's' )
		{
			t[-2] = 'i';
			t[-1] = '\0';
			t--;
		}
	}

	/* Append the "kind name" to the "base name" */
	if (append_name)
	{
		t = object_desc_str(t, " of ");
		t = object_desc_str(t, (k_name + k_ptr->name));
	}


	/* Hack -- Append "Artifact" or "Special" names */
	if (known)
	{

		/* Is it a new random artefact ? */
		if (o_ptr->art_name)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_str(t, quark_str(o_ptr->art_name));
		}

		/* Grab any artefact name */
		else if (o_ptr->name1  && artefact_overrides == FALSE)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_str(t, (a_name + a_ptr->name));
		}

		/* Grab any ego-item name */
		else if (!ego_prefix && o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			t = object_desc_chr(t, ' ');
			t = object_desc_str(t, (e_name + e_ptr->name));
		}
	}

	/* No more details wanted */
	if (mode < 1) goto copyback;


	/* Hack -- Chests must be described in detail */
	if (o_ptr->tval == TV_CHEST)
	{
		/* Not searched yet */
		if (!known)
		{
			/* Nothing */
		}
		/* May be "empty" */
		else if (!o_ptr->pval)
		{
			t = object_desc_str(t, " (empty)");
		}
		/* May be "disarmed" */
		else if (o_ptr->pval < 0)
		{
			/* To prevent out of bounds array access, we binary AND with the size of chest_traps */
			t = object_desc_str(t, chest_traps[o_ptr->pval & 64]? " (disarmed)" : " (unlocked)" );
		}
		/* Describe the traps, if any,  */
		else /* o_ptr->tval > 0 */
		{
			/* Describe the traps */
			switch (chest_traps[o_ptr->pval])
			{
			case 0:              t = object_desc_str(t, " (Locked)");           break;
			case CHEST_LOSE_STR: t = object_desc_str(t, " (Poison Needle)");    break;
			case CHEST_LOSE_CON: t = object_desc_str(t, " (Poison Needle)");    break;
			case CHEST_POISON:   t = object_desc_str(t, " (Gas Trap)");         break;
			case CHEST_PARALYZE: t = object_desc_str(t, " (Gas Trap)");         break;
			case CHEST_EXPLODE:  t = object_desc_str(t, " (Explosion Device)"); break;
			case CHEST_SUMMON:   t = object_desc_str(t, " (Summoning Runes)");  break;
			default:             t = object_desc_str(t, " (Multiple Traps)");   break;
			}
		}
	}

	/* Show the level if the level is known */
	if( known && o_ptr->elevel )
	{
		t = object_desc_str(t, " (Lvl " );
		t = object_desc_num(t, o_ptr->elevel);
		t = object_desc_str(t, ")" );
	}

	/* Display the item like a weapon */
	if (f3 & (TR3_SHOW_MODS)) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if (o_ptr->ac) show_armour = TRUE;

	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Missiles and Weapons */
	case TV_SHOT:
	case TV_BOLT:
	case TV_ARROW:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
	case TV_DIGGING:

		/* Append a "damage" string */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_num(t, o_ptr->dd);
		t = object_desc_chr(t, 'd');
		t = object_desc_num(t, o_ptr->ds);
		t = object_desc_chr(t, p2);

		/* All done */
		break;


		/* Bows get a special "damage string" */
	case TV_BOW:

		/* Mega-Hack -- Extract the "base power" */
		power = (o_ptr->sval % 10);

		/* Apply the "Extra Might" flag */
		if (f3 & (TR3_XTRA_MIGHT)) power++;

		/* Append a special "damage" string */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_chr(t, 'x');
		t = object_desc_num(t, power);
		t = object_desc_chr(t, p2);

		/* All done */
		break;
	}


	/* Add the weapon bonuses */
	if (known)
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_h);
			t = object_desc_chr(t, ',');
			t = object_desc_int(t, o_ptr->to_d);
			t = object_desc_chr(t, p2);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_h);
			t = object_desc_chr(t, p2);
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_d);
			t = object_desc_chr(t, p2);
		}
	}


	/* Add the armour bonuses */
	if (known)
	{
		/* Show the armour class info */
		if (show_armour)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, b1);
			t = object_desc_num(t, o_ptr->ac);
			t = object_desc_chr(t, ',');
			t = object_desc_int(t, o_ptr->to_a);
			t = object_desc_chr(t, b2);
		}

		/* No base armour, but does increase armour */
		else if (o_ptr->to_a)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, b1);
			t = object_desc_int(t, o_ptr->to_a);
			t = object_desc_chr(t, b2);
		}
	}

	/* Hack -- always show base armour */
	else if (show_armour)
	{
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, b1);
		t = object_desc_num(t, o_ptr->ac);
		t = object_desc_chr(t, b2);
	}


	/* No more details wanted */
	if (mode < 2) goto copyback;


	/* Hack -- Wands and Staffs have charges */
	if (known &&
		((o_ptr->tval == TV_STAFF) ||
		(o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges)" */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_num(t, o_ptr->pval);
		t = object_desc_str(t, " charge");
		if (o_ptr->pval != 1) t = object_desc_chr(t, 's');
		t = object_desc_chr(t, p2);
	}

	/* Hack -- Rods have a "charging" indicator */
	else if (known && (o_ptr->tval == TV_ROD))
	{
		/* Hack -- Dump " (charging)" if relevant */
		if (o_ptr->pval) t = object_desc_str(t, " (charging)");
	}

	/* Hack -- Process Lanterns/Torches */
	else if ((o_ptr->tval == TV_LITE) && ((o_ptr->sval == SV_LITE_TORCH) || (o_ptr->sval == SV_LITE_LANTERN)))
	{
		/* Hack -- Turns of light for normal lites */
		t = object_desc_str(t, " (with ");
		t = object_desc_num(t, o_ptr->pval);
		t = object_desc_str(t, " turns of light)");
	}


	/* Dump "pval" flags for wearable items */
	if (known && (f1 & (TR1_PVAL_MASK)))
	{
		/* Start the display */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);

		/* Dump the "pval" itself */
		t = object_desc_int(t, o_ptr->pval);

		/* Do not display the "pval" flags */
		if (f3 & (TR3_HIDE_TYPE))
		{
			/* Nothing */
		}

		/* Speed */
		else if (f1 & (TR1_SPEED))
		{
			/* Dump " to speed" */
			t = object_desc_str(t, " to movement speed");
		}

		/* Attack speed */
		else if (f1 & (TR1_BLOWS))
		{
			/* Dump " to speed" */
			t = object_desc_str(t, " to attack speed");
		}

		/* Stealth */
		else if (f1 & (TR1_STEALTH))
		{
			/* Dump " to stealth" */
			t = object_desc_str(t, " to stealth");
		}

		/* Search */
		else if (f1 & (TR1_SEARCH))
		{
			/* Dump " to searching" */
			t = object_desc_str(t, " to searching");
		}

		/* Infravision */
		else if (f1 & (TR1_INFRA))
		{
			/* Dump " to infravision" */
			t = object_desc_str(t, " to infravision");
		}

		/* Tunneling */
		else if (f1 & (TR1_TUNNEL))
		{
			/* Nothing */
		}

		/* Finish the display */
		t = object_desc_chr(t, p2);
	}


	/* Indicate "charging" artefacts XXX XXX XXX */
	if (known && o_ptr->timeout)
	{
		/* Hack -- Dump " (charging)" if relevant */
		t = object_desc_str(t, " (charging)");
	}


	/* No more details wanted */
	if (mode < 3) goto copyback;

	/* No inscription yet */
	tmp_val2[0] = '\0';

	/* Use the standard inscription if available */
	if (o_ptr->note)
	{
		my_commacat(tmp_val2, quark_str(o_ptr->note), sizeof( tmp_val2 ) );
	}

	/* Note "cursed" if the item is known to be cursed */
	if (cursed_p(o_ptr) && (known || (o_ptr->ident & (IDENT_SENSE))) && !o_ptr->note)
	{
		my_commacat(tmp_val2, "cursed", sizeof( tmp_val2 ) );
	}

	/* Mega-Hack -- note empty wands/staffs */
	if (!known && (o_ptr->ident & (IDENT_EMPTY)))
	{
		my_commacat(tmp_val2, "empty", sizeof( tmp_val2 ) );
	}
	/* Note the discount, if any */
	if (o_ptr->discount)
	{
		object_desc_num(tmp_val2, o_ptr->discount);
		my_strcat(tmp_val2, "% off", sizeof( tmp_val2 ) );
	}

	/* Add "tried" if the object has been tested unsuccessfully */
	if (!aware && object_tried_p(o_ptr))
	{
		my_commacat(tmp_val2, "tried", sizeof( tmp_val2 ) );
	}

	/* Append the inscription, if any */
	if (tmp_val2[0])
	{
		int n;

		/* Hack -- How much so far */
		n = (t - tmp_val);

		/* Paranoia -- do not be stupid */
		if (n > 75) n = 75;

		/* Hack -- shrink the inscription */
		tmp_val2[75 - n] = '\0';

		/* Append the inscription */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, c1);
		t = object_desc_str(t, tmp_val2);
		t = object_desc_chr(t, c2);
	}
copyback:
	/* Here's where we dump the built string into buf. */
	tmp_val[79] = '\0';
	t = tmp_val;
	while((*(buf++) = *(t++))); /* copy the string over */
}


/*
* Hack -- describe an item currently in a store's inventory
* This allows an item to *look* like the player is "aware" of it
*/
void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode)
{
	/* Save the "aware" flag */
	bool hack_aware = k_info[o_ptr->k_idx].aware;

	/* Save the "known" flag */
	bool hack_known = (o_ptr->ident & (IDENT_KNOWN)) ? TRUE : FALSE;

	/* Set the "known" flag */
	o_ptr->ident |= (IDENT_KNOWN);

	/* Force "aware" for description */
	k_info[o_ptr->k_idx].aware = TRUE;

	/* Describe the object */
	object_desc(buf, o_ptr, pref, mode);

	/* Restore "aware" flag */
	k_info[o_ptr->k_idx].aware = hack_aware;

	/* Clear the known flag */
	if (!hack_known) o_ptr->ident &= ~(IDENT_KNOWN);
}




/*
* Determine the "Activation" (if any) for an artefact
* Return a string, or NULL for "no activation"
*/
cptr item_activation(object_type *o_ptr)
{
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Require activation ability */
	if (!(f3 & (TR3_ACTIVATE))) return (NULL);

	/* We need to deduce somehow that it is a random artefact -- one
	problem: It could be a random artefact which has NOT YET received
	a name. Thus we eliminate other possibilities instead of checking
	for art_name */

	if (!(o_ptr->name1) && !(o_ptr->name2)
		&& !(o_ptr->xtra1) && (o_ptr->xtra2))
	{
		switch (o_ptr->xtra2)
		{
		case ACT_SUNLIGHT:
			{
				return "beam of sunlight every 10 turns";
			}
		case ACT_BO_MISS_1:
			{
				return "magic missile (2d6) every 2 turns";
			}
		case ACT_BA_POIS_1:
			{
				return "stinking cloud (12), rad. 3, every 4+d4 turns";
			}
		case ACT_BO_ELEC_1:
			{
				return "lightning bolt (4d8) every 6+d6 turns";
			}
		case ACT_BO_ACID_1:
			{
				return "acid bolt (5d8) every 5+d5 turns";
			}
		case ACT_BO_COLD_1:
			{
				return "frost bolt (6d8) every 7+d7 turns";
			}
		case ACT_BO_FIRE_1:
			{
				return "fire bolt (9d8) every 8+d8 turns";
			}
		case ACT_BA_COLD_1:
			{
				return "ball of cold (48) every 400 turns";
			}
		case ACT_BA_FIRE_1:
			{
				return "ball of fire (72) every 400 turns";
			}
		case ACT_DRAIN_1:
			{
				return "drain life (100) every 100+d100 turns";
			}
		case ACT_BA_COLD_2:
			{
				return "ball of cold (100) every 300 turns";
			}
		case ACT_BA_ELEC_2:
			{
				return "ball of lightning (100) every 500 turns";
			}
		case ACT_DRAIN_2:
			{
				return "drain life (120) every 400 turns";
			}
		case ACT_VAMPIRE_1:
			{
				return "vampiric drain (3*50) every 400 turns";
			}
		case ACT_BO_MISS_2:
			{
				return "arrows (150) every 90+d90 turns";
			}
		case ACT_BA_FIRE_2:
			{
				return "fire ball (120) every 225+d225 turns";
			}
		case ACT_BA_COLD_3:
			{
				return "ball of cold (200) every 325+d325 turns";
			}
		case ACT_WHIRLWIND:
			{
				return "whirlwind attack every 250 turns";
			}
		case ACT_VAMPIRE_2:
			{
				return "vampiric drain (3*100) every 400 turns";
			}
		case ACT_CALL_CHAOS:
			{
				return "call chaos every 350 turns";
			}
		case ACT_SHARD:
			{
				return "shard ball (120+level) every 400 turns";
			}
		case ACT_DISP_EVIL:
			{
				return "dispel evil (level*5) every 300+d300 turns";
			}
		case ACT_DISP_GOOD:
			{
				return "dispel good (level*5) every 300+d300 turns";
			}
		case ACT_BA_MISS_3:
			{
				return "elemental breath (300) every 500 turns";
			}
		case ACT_CONFUSE:
			{
				return "confuse monster every 15 turns";
			}
		case ACT_SLEEP:
			{
				return "sleep nearby monsters every 55 turns";
			}
		case ACT_QUAKE:
			{
				return "earthquake (rad 10) every 50 turns";
			}
		case ACT_TERROR:
			{
				return "terror every 3 * (level+10) turns";
			}
		case ACT_TELE_AWAY:
			{
				return "teleport away every 200 turns";
			}
		case ACT_BANISH_EVIL:
			{
				return "banish evil every 250+d250 turns";
			}
		case ACT_GENOCIDE:
			{
				return "genocide every 500 turns";
			}
		case ACT_MASS_GENO:
			{
				return "mass genocide every 1000 turns";
			}
		case ACT_CHARM_ANIMAL:
			{
				return "charm animal every 300 turns";
			}
		case ACT_CHARM_UNDEAD:
			{
				return "enslave undead every 333 turns";
			}
		case ACT_CHARM_OTHER:
			{
				return "charm monster every 400 turns";
			}
		case ACT_CHARM_ANIMALS:
			{
				return "animal friendship every 500 turns";
			}
		case ACT_CHARM_OTHERS:
			{
				return "mass charm every 750 turns";
			}
		case ACT_SUMMON_ANIMAL:
			{
				return "summon animal every 200+d300 turns";
			}
		case ACT_SUMMON_PHANTOM:
			{
				return "summon phantasmal servant every 200+d200 turns";
			}
		case ACT_SUMMON_ELEMENTAL:
			{
				return "summon elemental every 750 turns";
			}
		case ACT_SUMMON_DEMON:
			{
				return "summon demon every 666+d333 turns";
			}
		case ACT_SUMMON_UNDEAD:
			{
				return "summon undead every 666+d333 turns";
			}
		case ACT_CURE_LW:
			{
				return "remove fear & heal 30 hp every 10 turns";
			}
		case ACT_CURE_MW:
			{
				return "heal 4d8 & wounds every 3+d3 turns";
			}
		case ACT_CURE_POISON:
			{
				return "remove fear and cure poison every 5 turns";
			}
		case ACT_REST_LIFE:
			{
				return "restore life levels every 450 turns";
			}
		case ACT_REST_ALL:
			{
				return "restore stats and life levels every 750 turns";
			}
		case ACT_CURE_700:
			{
				return "heal 700 hit points every 250 turns";
			}
		case ACT_CURE_1000:
			{
				return "heal 1000 hit points every 888 turns";
			}
		case ACT_ESP:
			{
				return "temporary ESP (dur 25+d30) every 200 turns";
			}
		case ACT_BERSERK:
			{
				return "heroism and berserk (dur 50+d50) every 100+d100 turns";
			}
		case ACT_PROT_EVIL:
			{
				return "protect evil (dur level*3 + d25) every 225+d225 turns";
			}
		case ACT_RESIST_ALL:
			{
				return "resist elements (dur 40+d40) every 200 turns";
			}
		case ACT_SPEED:
			{
				return "speed (dur 20+d20) every 250 turns";
			}
		case ACT_XTRA_SPEED:
			{
				return "speed (dur 75+d75) every 200+d200 turns";
			}
		case ACT_WRAITH:
			{
				return "wraith form (level/2 + d(level/2)) every 1000 turns";
			}
		case ACT_INVULN:
			{
				return "invulnerability (dur 8+d8) every 1000 turns";
			}
		case ACT_LIGHT:
			{
				return "light area (dam 2d15) every 10+d10 turns";
			}
		case ACT_MAP_LIGHT:
			{
				return "light (dam 2d15) & map area every 50+d50 turns";
			}
		case ACT_DETECT_ALL:
			{
				return "detection every 55+d55 turns";
			}
		case ACT_DETECT_XTRA:
			{
				return "detection, probing and identify true every 1000 turns";
			}
		case ACT_ID_FULL:
			{
				return "identify true every 750 turns";
			}
		case ACT_ID_PLAIN:
			{
				return "identify spell every 10 turns";
			}
		case ACT_RUNE_EXPLO:
			{
				return "explosive rune every 200 turns";
			}
		case ACT_RUNE_PROT:
			{
				return "rune of protection every 400 turns";
			}
		case ACT_SATIATE:
			{
				return "satisfy hunger every 200 turns";
			}
		case ACT_DEST_DOOR:
			{
				return "destroy doors every 10 turns";
			}
		case ACT_STONE_MUD:
			{
				return "stone to mud every 5 turns";
			}
		case ACT_RECHARGE:
			{
				return "recharging every 70 turns";
			}
		case ACT_ALCHEMY:
			{
				return "alchemy every 500 turns";
			}
		case ACT_DIM_DOOR:
			{
				return "dimension door every 100 turns";
			}
		case ACT_TELEPORT:
			{
				return "teleport (range 100) every 45 turns";
			}
		case ACT_RECALL:
			{
				return "word of recall every 200 turns";
			}
		default:
			{
				return "something undefined";
			}
		}
	}

	/* Some artefacts can be activated */
	switch (o_ptr->name1)
	{
	case ART_DAGGER_INFERNO:
		{
			return "fire bolt (9d8) every 8+d8 turns";
		}
	case ART_COCYTUS:
		{
			return "frost bolt (6d8) every 7+d7 turns";
		}
	case ART_DAGGER_FURCIFER:
		{
			return "lightning bolt (4d8) every 6+d6 turns";
		}
	case ART_KINSLAYER:
		{
			return "stinking cloud (12) every 4+d4 turns";
		}
	case ART_FROST:
		{
			return "frost ball (48) every 5+d5 turns";
		}
	case ART_DANCING:
		{
			return "remove fear and cure poison every 5 turns";
		}
	case ART_MICHAEL:
		{
			return "frost ball (100) every 300 turns";
		}
	case ART_DAWN:
		{
			return "summon a Black Reaver every 500+d500 turns";
		}
	case ART_RASHAVERAK:
		{
			return "fire ball (72) every 400 turns";
		}
	case ART_MORNINGSTAR:
		{
			return "large fire ball (72) every 100 turns";
		}
	case ART_BOOTS_FURCIFER:
		{
			return "haste self (50 turns) every 200 turns";
		}
	case ART_BOOTS_GABRIEL:
	{
		return "haste self (50 turns) every 200 turns";
	}
	case ART_ELIGOR:
		{
			return "drain life (120) every 400 turns";
		}
	case ART_JUSTICE:
		{
			return "drain life (90) every 70 turns";
		}
	case ART_PRAVUIL:
		{
			return "door and trap destruction every 10 turns";
		}
	case ART_AZRAEL:
		{
			return "word of recall every 200 turns";
		}
	case ART_BELETH:
		{
			return "haste self (20+d20 turns) every 100+d100 turns";
		}
	case ART_ERIRIL:
		{
			return "identify every 10 turns";
		}
	case ART_ATAL:
		{
			return "probing, detection and full id  every 1000 turns";
		}
	case ART_TROLLS:
		{
			return "mass genocide every 1000 turns";
		}
	case ART_RONOVE:
		{
			return "cure wounds (4d7) every 3+d3 turns";
		}
	case ART_DEATH:
		{
			return "fire branding of bolts every 999 turns";
		}
	case ART_ANDROMALIUS:
		{
			return "a getaway every 35 turns";
		}
	case ART_ODIN:
		{
			return "lightning ball (100) every 500 turns";
		}
	case ART_DESTINY:
		{
			return "stone to mud every 5 turns";
		}
	case ART_CORSON:
		{
			return "heal (1000) every 888 turns";
		}
	case ART_AMAYMON:
	{
		return "poisonous nether breath(700) every 888 turns";
	}
	case ART_ROBE_MICHAEL:
		{
			return ("heal (777), curing and heroism every 300 turns");
		}
	case ART_VEPAR:
	    {
		return ("storm of the Dark Waters every 500 turns");
    	}
/*
	case ART_ORCS:
		{
			return "genocide every 500 turns";
		}
*/
	case ART_LIFE:
		{
			return "restore life levels every 450 turns";
		}
	case ART_TRITONS:
		{
			return "teleport away every 150 turns";
		}
	case ART_JOSEPH:
		{
			return "resistance (20+d20 turns) every 111 turns";
		}
	case ART_DRAEBOR:
		{
			return "confusion ball (200) radius 5 and blink";
		}
	case ART_BARD:
		{
			return "recharge item I every 70 turns";
		}
	case ART_SHIFTER:
		{
			return "teleport every 45 turns";
		}
	case ART_PHENEX:
		{
			return "confuse monster every 15 turns";
		}
	case ART_LIGHT:
		{
			return "magic missile (2d6) every 2 turns";
		}
	case ART_IRONFIST:
		{
			return "fire bolt (9d8) every 8+d8 turns";
		}
	case ART_GHOULS:
		{
			return "frost bolt (6d8) every 7+d7 turns";
		}
	case ART_FURFICER:
		{
			return "lightning bolt (4d8) every 6+d6 turns";
		}
	case ART_DEAD:
		{
			return "acid bolt (5d8) every 5+d5 turns";
		}
	case ART_GRIMREAPER:
		{
			return "a magical seeker bolt (450) every 90+d90 turns";
		}
	case ART_SKULLKEEPER:
		{
			return "detection every 55+d55 turns";
		}
	case ART_SUN:
		{
			return "heal (700) every 250 turns";
		}
	case ART_ASMODAI:
		{
			return "star ball (150) every 1000 turns";
		}
	case ART_BAPHOMET:
	{
		return "star ball (150) every 1000 turns";
	}
	case ART_SAMAEL:
	{
		return "breathe elements (300), berserk rage, bless, and resistance";
	}
	case ART_ABADDON:
	{
		return "breathe elements (300), berserk rage, bless, and resistance";
	}
	case ART_BLOOD_MICHAEL:
	{
		return "breathe elements (300), berserk rage, bless, and resistance";
	}
	case ART_BEATRICE:
		{
			return "illumination every 10+d10 turns";
		}
	case ART_EOS:
		{
			return "magic mapping and light every 20+d20 turns, draining you";
		}
	case ART_HIPPO:
		{
			return "clairvoyance and cure";
		}
	case ART_AMULET_MICHAEL : case ART_AMULET_RAPHAEL:
		{
			return "dispel evil (x5) and protection from evil every 450+d450 turns";
		}
	case ART_DOOM:
		{
			return "fireball(250) and protection from fire every 450+d50 turns";
		}
	case ART_RING_GEORGE:
		{
			return "shield and berserk (100+d25 turns) every 100+d100 turns";
		}
	case ART_RING_GABRIEL:
		{
			return "haste self (125 turns) every 150+d150 turns";
		}
	case ART_RING_RAPHAEL:
		{
			return "fireball(250), healing and protective magic every 700+d250 turns";
		}
	case ART_RING_MICHAEL:
		{
			return "frostball(200) and combat magic  every 250+d250 turns";
		}
	case ART_EMMANUEL:
		{
			return "lightning ball(250) and heal(700) every 700+d300 turns";
		}
	case ART_FIRST:
		{
			return "bizarre things every 450+d450 turns";
		}
	case ART_LAMMASU: case ART_MASK:
		{
			return "rays of fear in every direction";
		}
	}


	if (o_ptr->name2 == EGO_PLANAR)
	{
		return "teleport every 50+d50 turns";
	}

	if (o_ptr->tval == TV_RING)
	{
		switch(o_ptr->sval)
		{
		case SV_RING_FLAMES:
			return "ball of fire and resist fire";
		case SV_RING_ICE:
			return "ball of cold and resist cold";
		case SV_RING_ACID:
			return "ball of acid and resist acid";
		default:
			return NULL;
		}
	}

	/* Require dragon scale mail */
	if (o_ptr->tval != TV_DRAG_ARMOR) return (NULL);

	/* Branch on the sub-type */
	switch (o_ptr->sval)
	{
	case SV_DRAGON_BLUE:
		{
			return "breathe lightning (100) every 450+d450 turns";
		}
	case SV_DRAGON_WHITE:
		{
			return "breathe frost (110) every 450+d450 turns";
		}
	case SV_DRAGON_BLACK:
		{
			return "breathe acid (130) every 450+d450 turns";
		}
	case SV_DRAGON_GREEN:
		{
			return "breathe poison gas (150) every 450+d450 turns";
		}
	case SV_DRAGON_RED:
		{
			return "breathe fire (200) every 450+d450 turns";
		}
	case SV_DRAGON_MULTIHUED:
		{
			return "breathe multi-hued (250) every 225+d225 turns";
		}
	case SV_DRAGON_BRONZE:
		{
			return "breathe confusion (120) every 450+d450 turns";
		}
	case SV_DRAGON_GOLD:
		{
			return "breathe sound (130) every 450+d450 turns";
		}
	case SV_DRAGON_CHAOS:
		{
			return "breathe chaos/disenchant (220) every 300+d300 turns";
		}
	case SV_DRAGON_LAW:
		{
			return "breathe sound/shards (230) every 300+d300 turns";
		}
	case SV_DRAGON_BALANCE:
		{
			return "You breathe balance (250) every 300+d300 turns";
		}
	case SV_DRAGON_SHINING:
		{
			return "breathe light/darkness (200) every 300+d300 turns";
		}
	case SV_DRAGON_POWER:
		{
			return "breathe the elements (300) every 300+d300 turns";
		}
	}


	/* Oops */
	return NULL;
}

/* It is silly for wants, staves, rods, and scrolls to have 'secrets'*/
bool has_no_secrets_ever(object_type *o_ptr)
{
  if( o_ptr->tval == TV_SKELETON ||
      o_ptr->tval == TV_BOTTLE ||
      o_ptr->tval == TV_JUNK ||
      o_ptr->tval == TV_SPIKE ||
      o_ptr->tval == TV_STAFF ||
      o_ptr->tval == TV_WAND ||
      o_ptr->tval == TV_ROD ||
      o_ptr->tval == TV_SCROLL ||
      o_ptr->tval == TV_POTION ||
      o_ptr->tval == TV_FOOD ||
      o_ptr->tval == TV_GOLD )
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

/*
 * Return the "fully identified" item description in an array of strings
 */
int identify_fully_strings( object_type *o_ptr )
{
	char * string_ptr;
	u32b f1, f2, f3;
	object_kind		*k_ptr;
	char buf[2048];
	char weight_buf[60];
	int i = 0;
	size_t string_size;
	int delta;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);
	/* Get the kind info */
	k_ptr = &k_info[ o_ptr->k_idx ];

	/* Extract the kind description, calculate string_size and
	   make a copy of the buffer pointer */
	strcpy(buf, k_text + k_ptr->text);
	string_size = strlen( buf );
	string_ptr = buf;

	if( k_info[o_ptr->k_idx].aware)
	{
	/* While the moving pointer still has more than 60 characters , keep splitting */
	while( string_size > 60 ){
		info[i++] = string_ptr;
		/* Move forward until we hit a non space */
		while( info[i-1][0] == ' ' ){
			info[i-1]++;
		}
		/*Go back until we find a space or a hyphen*/
		delta = 0;
		while( string_ptr[60-1-delta] != ' ' && string_ptr[60-1-delta] != '-' && string_ptr[60-1-delta] != '\0' )
			delta++;
		/* Move , terminate , update string pointer & string size */
		memmove( string_ptr+60-delta , string_ptr+60-1-delta , string_size);
		string_ptr[60-1-delta] = '\0';
		string_size = string_size - 60 + delta;
		string_ptr = string_ptr + 60 - delta;
	}

	/* Place the remainder, make sure the remainder does not start with a space */
	info[i++] = string_ptr;
	while( info[i-1][0] == ' ' ){
		info[i-1]++;
	}
	}
	info[i++] = "";

	sprintf(weight_buf, "%s %d.%1d lb", o_ptr->number==1?"It weights":"They weigh" ,
	                                         o_ptr->weight * o_ptr->number / 10 ,
	                                         o_ptr->weight * o_ptr->number % 10 );
	info[i++] = weight_buf;

	/* It has been decided that these properties should be obvious,
	   so show them when the object is not known, otherwise let the below
	   routine take care of these properties
	*/
	if(!(o_ptr->ident & (IDENT_MENTAL) || k_ptr->flags3 & (TR3_EASY_KNOW) ))
	{
		if (f3 & (TR3_IGNORE_ACID))	info[i++] = "It cannot be harmed by acid.";
		if (f3 & (TR3_IGNORE_ELEC))	info[i++] = "It cannot be harmed by electricity.";
		if (f3 & (TR3_IGNORE_FIRE))	info[i++] = "It cannot be harmed by fire.";
		if (f3 & (TR3_IGNORE_COLD))	info[i++] = "It cannot be harmed by cold.";
	}

	if(o_ptr->ident & (IDENT_MENTAL) || k_ptr->flags3 & (TR3_EASY_KNOW) )
	{
		/* Mega-Hack -- describe activation */
		if (f3 & (TR3_ACTIVATE))
		{
			info[i++] = "It can be activated for...";
			info[i++] = item_activation(o_ptr);
			info[i++] = "...if it is being worn.";
		}

		/* Hack -- describe lite's */
		if (o_ptr->tval == TV_LITE)
		{
			if (artefact_p(o_ptr))
				info[i++] = "It provides light (radius 3) forever.";
			else if (o_ptr->sval == SV_LITE_LANTERN)
				info[i++] = "It provides light (radius 2) when fueled.";
			else if (o_ptr->sval == SV_LITE_TORCH)
				info[i++] = "It provides light (radius 1) when fueled.";
			else
				info[i++] = "It provides light (radius 2) forever.";
		}

		/* And then describe it fully */
		if (f1 & (TR1_STR))		info[i++] = "It affects your strength.";
		if (f1 & (TR1_INT))		info[i++] = "It affects your intelligence.";
		if (f1 & (TR1_WIS))		info[i++] = "It affects your wisdom.";
		if (f1 & (TR1_DEX))		info[i++] = "It affects your dexterity.";
		if (f1 & (TR1_CON))		info[i++] = "It affects your constitution.";
		if (f1 & (TR1_CHA))		info[i++] = "It affects your charisma.";

		if (f1 & (TR1_STEALTH))	info[i++] = "It affects your stealth.";
		if (f1 & (TR1_SEARCH))	info[i++] = "It affects your searching.";
		if (f1 & (TR1_INFRA))	info[i++] = "It affects your infravision.";
		if (f1 & (TR1_TUNNEL))	info[i++] = "It affects your ability to tunnel.";
		if (f1 & (TR1_SPEED))	info[i++] = "It affects your movement speed.";
		if (f1 & (TR1_BLOWS))	info[i++] = "It affects your attack speed.";
		
		if (f1 & (TR1_BRAND_ACID))	info[i++] = "It does extra damage from acid.";
		if (f1 & (TR1_BRAND_ELEC))	info[i++] = "It does extra damage from electricity.";
		if (f1 & (TR1_BRAND_FIRE))	info[i++] = "It does extra damage from fire.";
		if (f1 & (TR1_BRAND_COLD))	info[i++] = "It does extra damage from frost.";
		if (f1 & (TR1_BRAND_POIS))	info[i++] = "It poisons your foes.";

		if (f1 & (TR1_CHAOTIC))		info[i++] = "It produces chaotic effects.";
		if (f1 & (TR1_VAMPIRIC))	info[i++] = "It drains life from your foes.";
		if (f1 & (TR1_IMPACT))		info[i++] = "It can cause earthquakes.";
		if (f1 & (TR1_VORPAL))		info[i++] = "It is very sharp and can cut your foes.";
		if (f1 & (TR1_KILL_DRAGON))	info[i++] = "It is a great bane of dragons.";
		else if (f1 & (TR1_SLAY_DRAGON))info[i++] = "It is especially deadly against dragons.";
		if (f1 & (TR1_SLAY_ANGEL))	info[i++] = "It is especially deadly against fallen angels.";
		else if (f1 & (TR1_KILL_ANGEL))
		{
			info[i++] = "It is a bane of fallen angels.";
			info[i++] = "It protects you from Lucifers' curse.";
		}
		if (f1 & (TR1_SLAY_GIANT))	info[i++] = "It is especially deadly against giants.";
		if (f1 & (TR1_SLAY_DEMON))	info[i++] = "It strikes at demons with holy wrath.";
		if (f1 & (TR1_SLAY_UNDEAD))	info[i++] = "It strikes at undead with holy wrath.";
		if (f1 & (TR1_SLAY_EVIL))	info[i++] = "It fights against evil with holy fury.";
		if (f1 & (TR1_SLAY_ANIMAL))	info[i++] = "It is especially deadly against natural creatures.";

		if (f2 & (TR2_SUST_STR))	info[i++] = "It sustains your strength.";
		if (f2 & (TR2_SUST_INT))	info[i++] = "It sustains your intelligence.";
		if (f2 & (TR2_SUST_WIS))	info[i++] = "It sustains your wisdom.";
		if (f2 & (TR2_SUST_DEX))	info[i++] = "It sustains your dexterity.";
		if (f2 & (TR2_SUST_CON))	info[i++] = "It sustains your constitution.";
		if (f2 & (TR2_SUST_CHA))	info[i++] = "It sustains your charisma.";

		if (f2 & (TR2_IM_ACID))		info[i++] = "It provides immunity to acid.";
		if (f2 & (TR2_IM_ELEC))		info[i++] = "It provides immunity to electricity.";
		if (f2 & (TR2_IM_FIRE))		info[i++] = "It provides immunity to fire.";
		if (f2 & (TR2_IM_COLD))		info[i++] = "It provides immunity to cold.";

		if (f2 & (TR2_FREE_ACT))	info[i++] = "It provides immunity to paralysis.";
		if (f2 & (TR2_HOLD_LIFE))	info[i++] = "It provides resistance to life draining.";
		if (f2 & (TR2_RES_FEAR))	info[i++] = "It makes you completely fearless.";
		if (f2 & (TR2_RES_ACID))	info[i++] = "It provides resistance to acid.";
		if (f2 & (TR2_RES_ELEC))	info[i++] = "It provides resistance to electricity.";
		if (f2 & (TR2_RES_FIRE))	info[i++] = "It provides resistance to fire.";
		if (f2 & (TR2_RES_COLD))	info[i++] = "It provides resistance to cold.";
		if (f2 & (TR2_RES_POIS))	info[i++] = "It provides resistance to poison.";
		if (f2 & (TR2_RES_LITE))	info[i++] = "It provides resistance to light.";
		if (f2 & (TR2_RES_DARK))	info[i++] = "It provides resistance to dark.";
		if (f2 & (TR2_RES_BLIND))	info[i++] = "It provides resistance to blindness.";
		if (f2 & (TR2_RES_CONF))	info[i++] = "It provides resistance to confusion.";
		if (f2 & (TR2_RES_SOUND))	info[i++] = "It provides resistance to sound.";
		if (f2 & (TR2_RES_SHARDS))	info[i++] = "It provides resistance to shards.";
		if (f2 & (TR2_RES_NETHER))	info[i++] = "It provides resistance to nether.";
		if (f2 & (TR2_RES_NEXUS))	info[i++] = "It provides resistance to nexus.";
		if (f2 & (TR2_RES_CHAOS))	info[i++] = "It provides resistance to chaos.";
		if (f2 & (TR2_RES_DISEN))	info[i++] = "It provides resistance to disenchantment.";
		if (f3 & (TR3_WRAITH))		info[i++] = "It renders you incorporeal.";
		if (f3 & (TR3_FEATHER))		info[i++] = "It allows you to levitate.";
		if (f3 & (TR3_LITE))		info[i++] = "It provides permanent light.";
		if (f3 & (TR3_SEE_INVIS))	info[i++] = "It allows you to see invisible monsters.";
		if (f3 & (TR3_TELEPATHY))	info[i++] = "It gives telepathic powers.";
		if (f3 & (TR3_SLOW_DIGEST))	info[i++] = "It slows your metabolism.";
		if (f3 & (TR3_REGEN))		info[i++] = "It speeds your regenerative powers.";
		if (f2 & (TR2_REFLECT))		info[i++] = "It reflects bolts and arrows.";
		if (f3 & (TR3_SH_FIRE))		info[i++] = "It produces a fiery sheath.";
		if (f3 & (TR3_SH_ELEC))		info[i++] = "It produces an electric sheath.";
		if (f3 & (TR3_NO_MAGIC))	info[i++] = "It produces an anti-magic shell.";
		if (f3 & (TR3_NO_TELE))		info[i++] = "It prevents teleportation.";
		if (f3 & (TR3_XTRA_MIGHT))	info[i++] = "It fires missiles with extra might.";
		if (f3 & (TR3_XTRA_SHOTS))	info[i++] = "It fires missiles excessively fast.";
		if (f3 & (TR3_DRAIN_EXP))	info[i++] = "It drains experience.";
		if (f3 & (TR3_TELEPORT))	info[i++] = "It induces random teleportation.";
		if (f3 & (TR3_AGGRAVATE))	info[i++] = "It aggravates nearby creatures.";
		if (f3 & (TR3_BLESSED))		info[i++] = "It has been blessed by the gods.";

		if (cursed_p(o_ptr))
		{
			if (f3 & (TR3_PERMA_CURSE))
				info[i++] = "It is permanently cursed.";
			else if (f3 & (TR3_HEAVY_CURSE))
				info[i++] = "It is heavily cursed.";
			else
				info[i++] = "It is cursed.";
		}

		if (f3 & (TR3_TY_CURSE))	info[i++] = "It carries an ancient foul curse.";

		if (f3 & (TR3_IGNORE_ACID))	info[i++] = "It cannot be harmed by acid.";
		if (f3 & (TR3_IGNORE_ELEC))	info[i++] = "It cannot be harmed by electricity.";
		if (f3 & (TR3_IGNORE_FIRE))	info[i++] = "It cannot be harmed by fire.";
		if (f3 & (TR3_IGNORE_COLD))	info[i++] = "It cannot be harmed by cold.";
		if( has_no_secrets_ever(o_ptr) == FALSE)
		{
		  info[i++] = "It holds no secrets for you.";
		}
	}
	return i;
}

/*
* Retrieve the "fully identified" item description in an array of strings and write it out
*/
bool identify_fully_aux(object_type *o_ptr)
{
	int i = 0, j, k;

	i = identify_fully_strings( o_ptr );

	/* Save the screen */
	Term_save();

	/* Erase the screen */
	for (k = 1; k < 24; k++) prt("", k, 13);

	if( i )
	{
		/* Label the information */
		prt("", 1, 15);

		/* We will print on top of the map (column 13) */
		for (k = 2, j = 0; j < i; j++)
		{
			/* Show the info */
			prt(info[j], k++, 15);

			/* Every 20 entries (lines 2 to 21), start over */
			if ((k == 22) && (j+1 < i))
			{
				prt("-- more --", k, 15);
				inkey();
				for (; k > 2; k--) prt("", k, 15);
			}
		}
	}

	/* Wait for it */
	prt("[Press any key to continue]", k+1, 15);

	inkey();

	/* Restore the screen */
	Term_load();

	/* Gave knowledge */
	return (TRUE);
}

/*
 * Hack -- display an object kind in the current window
 *
 * s16b         term_k_idx;
 * object_type *term_o_ptr;
 *
 * Include list of usable spells for readible books
 */
void display_koff(int k_idx)
{
	int y;

	object_type forge;
	object_type *q_ptr;

	char o_name[80];


	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* No info */
	if (!term_k_idx && term_o_ptr == NULL ) return;

	if( term_k_idx )
	{
		/* Get local object */
		q_ptr = &forge;

		/* Prepare the object */
		object_wipe(q_ptr);

		/* Prepare the object */
		object_prep(q_ptr, k_idx);

		/* Describe */
		object_desc_store(o_name, q_ptr, FALSE, 0);

		/* Mention the object name */
		Term_putstr(0, 0, -1, TERM_WHITE, o_name);

		/* Warriors are illiterate */
		if (!(p_ptr->realm1 || p_ptr->realm2)) return;

		/* Display spells in readible books */
		if (q_ptr->tval == p_ptr->realm1+89 || q_ptr->tval == p_ptr->realm2+89)
		{
			int                     sval;
			int                     spell = -1;
			int                     num = 0;
			byte            spells[64];

			/* Access the item's sval */
			sval = q_ptr->sval;

			/* Extract spells */
			for (spell = 0; spell < 32; spell++)
			{
				/* Check for this spell */
				if (fake_spell_flags[sval] & (1L << spell))
	
				{
					/* Collect this spell */
					spells[num++] = spell;
				}
			}
	
			/* Print spells */
			print_spells(spells, num, 2, 0,
				(q_ptr->tval == p_ptr->realm1+89 ?p_ptr->realm1-1:p_ptr->realm2-1));
		}
	}

	if( term_o_ptr != NULL )
	{
		int  j,k;
		char o_name[80];
		int i = identify_fully_strings( term_o_ptr );
		if( i )
		{
			prt( term_activity , 1 , 0 );
			object_desc(o_name, term_o_ptr, TRUE, 3);
			prt( o_name, 1, strlen( term_activity ) + 1 );
			/* We will print on top of the map (column 13) */
			for (k = 3, j = 0; j < i; j++)
			{
				/* Show the info */
				prt(info[j], k++, 0);
			}
		}
	}
}


/*
* Convert an inventory index into a one character label
* Note that the label does NOT distinguish inven/equip.
*/
s16b index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
* Convert a label into the index of an item in the "inven"
* Return "-1" if the label does not indicate a real item
*/
s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}


/*
* Convert a label into the index of a item in the "equip"
* Return "-1" if the label does not indicate a real item
*/
s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

	/* Empty slots can never be chosen */
	if (!inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}



/*
* Determine which equipment slot (if any) an item likes
*/
s16b wield_slot(object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
		{
			return (INVEN_WIELD);
		}

	case TV_BOW:
		{
			return (INVEN_BOW);
		}

	case TV_RING:
		{
			/* TODO: clean me up at some point we should just return 1 value
			   Use the right hand first */
			 if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);
			/* Use the left hand for swapping (by default) */
			 return (INVEN_LEFT);
		}

	case TV_AMULET:
		{
			return (INVEN_NECK);
		}

	case TV_LITE:
		{
			return (INVEN_LITE);
		}

	case TV_DRAG_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
		{
			return (INVEN_BODY);
		}

	case TV_CLOAK:
		{
			return (INVEN_OUTER);
		}

	case TV_SHIELD:
		{
			return (INVEN_ARM);
		}

	case TV_CROWN:
	case TV_HELM:
		{
			return (INVEN_HEAD);
		}

	case TV_GLOVES:
		{
			return (INVEN_HANDS);
		}

	case TV_BOOTS:
		{
			return (INVEN_FEET);
		}

	case TV_POTION:
	case TV_SCROLL:
	case TV_WAND:
		{
			/* Use the pouch 1 first */
			if (!inventory[INVEN_POUCH_1].k_idx) return (INVEN_POUCH_1);
			if (!inventory[INVEN_POUCH_2].k_idx) return (INVEN_POUCH_2);
			if (!inventory[INVEN_POUCH_3].k_idx) return (INVEN_POUCH_3);
			if (!inventory[INVEN_POUCH_4].k_idx) return (INVEN_POUCH_4);
			if (!inventory[INVEN_POUCH_5].k_idx) return (INVEN_POUCH_5);
			return (INVEN_POUCH_6);
		}
	}

	/* No slot available */
	return (-1);
}


/*
* Return a string mentioning how a given item is carried
*/
cptr mention_use(int i)
{
	cptr p;
	byte weight_capacity;

	/* Examine the location */
	switch (i)
	{
	case INVEN_WIELD: p = "Wielding"; break;
	case INVEN_BOW:   p = "Shooting"; break;
	case INVEN_LEFT:  p = "On left hand"; break;
	case INVEN_RIGHT: p = "On right hand"; break;
	case INVEN_NECK:  p = "Around neck"; break;
	case INVEN_LITE:  p = "Light source"; break;
	case INVEN_BODY:  p = "On body"; break;
	case INVEN_OUTER: p = "About body"; break;
	case INVEN_ARM:   p = "On arm"; break;
	case INVEN_HEAD:  p = "On head"; break;
	case INVEN_HANDS: p = "On hands"; break;
	case INVEN_FEET:  p = "On feet"; break;
	case INVEN_POUCH_1: p = "In pouch";break;
	case INVEN_POUCH_2: p = "In pouch";break;
	case INVEN_POUCH_3: p = "In pouch";break;
	case INVEN_POUCH_4: p = "In pouch";break;
	case INVEN_POUCH_5: p = "In pouch";break;
	case INVEN_POUCH_6: p = "In pouch";break;
	default:          p = "In pack"; break;
	}

	weight_capacity = adj_stat[p_ptr->stat_ind[A_STR]][ADJ_WEIGHT];
	
	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (weight_capacity < o_ptr->weight / 10)
		{
			p = "Just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (weight_capacity < o_ptr->weight / 10)
		{
			p = "Just holding";
		}
	}

	/* Return the result */
	return (p);
}


/*
* Return a string describing how a given item is being worn.
* Currently, only used for items in the equipment, not inventory.
*/
cptr describe_use(int i)
{
	cptr p;
	byte weight_capacity;

	switch (i)
	{
	case INVEN_WIELD: p = "attacking monsters with"; break;
	case INVEN_BOW:   p = "shooting missiles with"; break;
	case INVEN_LEFT:  p = "wearing on your left hand"; break;
	case INVEN_RIGHT: p = "wearing on your right hand"; break;
	case INVEN_NECK:  p = "wearing around your neck"; break;
	case INVEN_LITE:  p = "using to light the way"; break;
	case INVEN_BODY:  p = "wearing on your body"; break;
	case INVEN_OUTER: p = "wearing on your back"; break;
	case INVEN_ARM:   p = "wearing on your arm"; break;
	case INVEN_HEAD:  p = "wearing on your head"; break;
	case INVEN_HANDS: p = "wearing on your hands"; break;
	case INVEN_FEET:  p = "wearing on your feet"; break;
	case INVEN_POUCH_1: p = "carrying in a pouch";break;
	case INVEN_POUCH_2: p = "carrying in a pouch";break;
	case INVEN_POUCH_3: p = "carrying in a pouch";break;
	case INVEN_POUCH_4: p = "carrying in a pouch";break;
	case INVEN_POUCH_5: p = "carrying in a pouch";break;
	case INVEN_POUCH_6: p = "carrying in a pouch";break;
	default:          p = "carrying in your pack"; break;
	}

	weight_capacity = adj_stat[p_ptr->stat_ind[A_STR]][ADJ_WEIGHT];
	
	/*TODO: These 20 lines could be 5 lines of code, would need testing*/
	
	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (weight_capacity < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &inventory[i];
		if (weight_capacity < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}

/* Hack: Check if a spellbook is one of the realms we can use. -- TY */

bool check_book_realm(const byte book_tval)
{
	return (p_ptr->realm1+89==book_tval || p_ptr->realm2+89==book_tval);
}


/*
* Check an item against the item tester info
*/
bool item_tester_okay(object_type *o_ptr)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->k_idx) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		/* Is it a spellbook? If so, we need a hack -- TY */
		if (item_tester_tval<=TV_DEATH_BOOK && item_tester_tval>=TV_MIRACLES_BOOK)
			return check_book_realm(o_ptr->tval);
		else
			if (!(item_tester_tval == o_ptr->tval)) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}

/*
 * Get the indexes of objects at a given floor location.
 *
 * Return the number of object indexes acquired.
 *
 * Never acquire more than "size" object indexes, and never return a
 * number bigger than "size", even if more floor objects exist.
 *
 * Valid flags are any combination of the bits:
 *
 *   0x01 -- Verify item tester
 *   0x02 -- Marked items only
 */
int scan_floor(int *items, int size, int y, int x, int mode)
{
	int this_o_idx, next_o_idx;
    
	int num = 0;
    
	/* Sanity */
	if (!in_bounds(y, x)) return (0);
    
	/* Scan all objects in the grid */
	for (this_o_idx = cave[py][px].o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;
        
		/* Get the object */
		o_ptr = &o_list[this_o_idx];
        
		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;
        
		/* Verify item tester */
		if ((mode & 0x01) && !item_tester_okay(o_ptr)) continue;
        
		/* Marked items only */
		if ((mode & 0x02) && !o_ptr->marked) continue;
        
		/* Accept this item */
		items[num++] = this_o_idx;
        
		/* Enforce size limit */
		if (num >= size) break;
	}
    
	/* Result */
	return (num);
}

/*
* Choice window "shadow" of the "show_inven()" function
*/
void display_inven(void)
{
	register        int i, n, z = 0;

	object_type *o_ptr;

	byte    attr = TERM_WHITE;

	char    tmp_val[80];

	char    o_name[80];


	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = (byte)(index_to_label(i));

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get a colour */
		attr = tval_to_attr[o_ptr->tval % 128];

		/* Hack -- fake monochrome */
		if (!use_colour) attr = TERM_WHITE;

		/* Display the entry itself */
		Term_putstr(3, i, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i, 255);

		/* Display the weight if needed */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = z; i < Term->hgt; i++)
	{
		/* Erase the line */
		Term_erase(0, i, 255);
	}
}



/*
* Choice window "shadow" of the "show_equip()" function
*/
void display_equip(void)
{
	register        int i, n;
	object_type *o_ptr;
	byte    attr = TERM_WHITE;

	char    tmp_val[80];

	char    o_name[80];


	/* Display the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Examine the item */
		o_ptr = &inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Is this item "acceptable"? */
		if (item_tester_okay(o_ptr))
		{
			/* Prepare an "index" */
			tmp_val[0] = (byte)(index_to_label(i));

			/* Bracket the "index" --(-- */
			tmp_val[1] = ')';
		}

		/* Display the index (or blank space) */
		Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

		/* Obtain an item description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get the colour */
		attr = tval_to_attr[o_ptr->tval % 128];

		/* Hack -- fake monochrome */
		if (!use_colour) attr = TERM_WHITE;

		/* Display the entry itself */
		Term_putstr(3, i - INVEN_WIELD, n, attr, o_name);

		/* Erase the rest of the line */
		Term_erase(3+n, i - INVEN_WIELD, 255);

		/* Display the slot description (if needed) */
		if (show_labels)
		{
			Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
			Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
		}

		/* Display the weight (if needed) */
		if (show_weights && o_ptr->weight)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			int col = (show_labels ? 52 : 71);
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
		}
	}

	/* Erase the rest of the window */
	for (i = INVEN_TOTAL - INVEN_WIELD; i < Term->hgt; i++)
	{
		/* Clear that line */
		Term_erase(0, i, 255);
	}
}






/*
* Display the inventory.
*
* Hack -- do not display "trailing" empty slots
*/
void show_inven(void)
{
	int             i, j, k, l, z = 0;
	int             col, len, lim;

	object_type     *o_ptr;

	char    o_name[80];

	char    tmp_val[80];

	int             out_index[23];
	byte    out_colour[23];
	char    out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Default "max-length" */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;

	/* Require space for icon */
	if (equippy_chars) lim -= 2;

	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the object index, colour, and description */
		out_index[k] = i;
		out_colour[k] = tval_to_attr[o_ptr->tval % 128];
		(void)strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		if (show_weights) l += 9;

		/* Account for icon if displayed */
		if (equippy_chars) l += 2;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display graphics for object, if desired */
		if (equippy_chars)
		{
			byte  a = object_attr(o_ptr);
			char c = object_char(o_ptr);

#ifdef AMIGA
			if (a & 0x80) a |= 0x40;
#endif

			Term_draw(col + 3, j + 1, a, c);
		}

		/* Display the entry itself */
		c_put_str(out_colour[j], out_desc[j], j + 1, equippy_chars ? (col + 5) : (col + 3));

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			(void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;
}



/*
* Display the equipment.
*/
void show_equip(void)
{
	int                     i, j, k, l;
	int                     col, len, lim;

	object_type             *o_ptr;

	char            tmp_val[80];

	char            o_name[80];

	int                     out_index[23];
	byte            out_colour[23];
	char            out_desc[23][80];


	/* Starting column */
	col = command_gap;

	/* Maximal length */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for labels (if needed) */
	if (show_labels) lim -= (14 + 2);

	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;
	if (equippy_chars) lim -= 2;

	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;

		/* Description */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Truncate the description */
		o_name[lim] = 0;

		/* Save the colour */
		out_index[k] = i;
		out_colour[k] = tval_to_attr[o_ptr->tval % 128];
		(void)strcpy(out_desc[k], o_name);

		/* Extract the maximal length (see below) */
		l = strlen(out_desc[k]) + (2 + 3);

		/* Increase length for labels (if needed) */
		if (show_labels) l += (14 + 2);

		/* Increase length for weight (if needed) */
		if (show_weights) l += 9;
		if (equippy_chars) l += 2;

		/* Maintain the max-length */
		if (l > len) len = l;

		/* Advance the entry */
		k++;
	}

	/* Hack -- Find a column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j+1, col);

		if (equippy_chars)
		{
			byte a = object_attr(o_ptr);
			char c = object_char(o_ptr);

#ifdef AMIGA
			if (a & 0x80) a |= 0x40;
#endif

			Term_draw(col + 3, j + 1, a, c);
		}

		/* Use labels */
		if (show_labels)
		{
			/* Mention the use */
			(void)sprintf(tmp_val, "%-14s: ", mention_use(i));
			put_str(tmp_val, j+1, equippy_chars ? col + 5 : col + 3);

			/* Display the entry itself */
			c_put_str(out_colour[j], out_desc[j], j+1, equippy_chars ? col + 21 : col + 19);
		}

		/* No labels */
		else
		{
			/* Display the entry itself */
			c_put_str(out_colour[j], out_desc[j], j+1, equippy_chars ? col + 5 : col + 3);
		}

		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			(void)sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j+1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	/* Save the new column */
	command_gap = col;
}


/*
 * Display a list of the items on the floor at the given location.
 */
void show_floor(const int *floor_list, int floor_num)
{
	int i, j, k, l;
	int col, len, lim;
    
	object_type *o_ptr;
    
	char o_name[80];
    
	char tmp_val[80];
    
	int out_index[MAX_FLOOR_STACK];
	byte out_color[MAX_FLOOR_STACK];
	char out_desc[MAX_FLOOR_STACK][80];
    
    
	/* Default length */
	len = 79 - 50;
    
	/* Maximum space allowed for descriptions */
	lim = 79 - 3;
    
	/* Require space for weight (if needed) */
	if (show_weights) lim -= 9;
    
	/* Display the inventory */
	for (k = 0, i = 0; i < floor_num; i++)
	{
		o_ptr = &o_list[floor_list[i]];
        
		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr)) continue;
        
		/* Description */
		object_desc(o_name, o_ptr, TRUE, 3);
        
		/* Hack -- enforce max length */
		o_name[lim] = '\0';
        
		/* Save the index */
		out_index[k] = i;
        
		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
        
		/* Save the object description */
		my_strcpy(out_desc[k], o_name, sizeof(out_desc[0]));
        
		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;
        
		/* Be sure to account for the weight */
		if (show_weights) l += 9;
        
		/* Maintain the maximum length */
		if (l > len) len = l;
        
		/* Advance to next "line" */
		k++;
	}
    
	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);
    
	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = floor_list[out_index[j]];
        
		/* Get the item */
		o_ptr = &o_list[i];
        
		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);
        
		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(out_index[j]));
        
		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);
        
		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);
        
		/* Display the weight if needed */
		if (show_weights)
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}
    
	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}


/*
* Flip "inven" and "equip" in any sub-windows
*/
void toggle_inven_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Flip inven to equip */
		if (window_flag[j] & (PW_INVEN))
		{
			/* Flip flags */
			window_flag[j] &= ~(PW_INVEN);
			window_flag[j] |= (PW_EQUIP);

			/* Window stuff */
			p_ptr->window |= (PW_EQUIP);
		}

		/* Flip inven to equip */
		else if (window_flag[j] & (PW_EQUIP))
		{
			/* Flip flags */
			window_flag[j] &= ~(PW_EQUIP);
			window_flag[j] |= (PW_INVEN);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN);
		}
	}
}



/*
* Verify the choice of an item.
*
* The item can be negative to mean "item on floor".
*/
static bool verify(cptr prompt, int item)
{
	char    o_name[80];

	char    out_val[160];

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Prompt */
	(void)sprintf(out_val, "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


/*
* Hack -- allow user to "prevent" certain choices
*
* The item can be negative to mean "item on floor".
*/
static bool get_item_allow(int item)
{
	cptr s;

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* No inscription */
	if (!o_ptr->note) return (TRUE);

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->note), '!');

	/* Process preventions */
	while (s)
	{
		/* Check the "restriction" */
		if ((s[1] == command_cmd) || (s[1] == '*'))
		{
			/* Verify the choice */
			if (!verify("Really try", item)) return (FALSE);
		}

		/* Find another '!' */
		s = strchr(s + 1, '!');
	}

	/* Allow it */
	return (TRUE);
}



/*
 * Verify the "okayness" of a given item.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_okay(int item)
{
	object_type *o_ptr;
    
	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}
    
	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}
    
	/* Verify the item */
	return (item_tester_okay(o_ptr));
}

/*
* Find the "first" inventory object with the given "tag".
*
* A "tag" is a char "n" appearing as "@n" anywhere in the
* inscription of an object.
*
* Also, the tag "@xn" will work as well, where "n" is a tag-char,
* and "x" is the "current" command_cmd code.
*/
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;


	/* Check every object */
	for (i = 0; i < INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip empty inscriptions */
		if (!o_ptr->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(o_ptr->note), '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return (FALSE);
}

/*
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven/floor
 * listings.  It is equal to USE_INVEN or USE_EQUIP or USE_FLOOR, except
 * when this function is first called, when it is equal to zero, which will
 * cause it to be set to USE_INVEN.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 *
 * Note that the "easy_floor" option affects this function in several ways.
 *
 * Note that only "acceptable" floor objects get indexes, so between two
 * commands, the indexes of floor objects may change.  XXX XXX XXX
 */
bool get_item(int *cp, cptr pmt, cptr str, int mode)
{
	char which;
    
	int i, j, k;
    
	int i1, i2;
	int e1, e2;
	int f1, f2;
    
	bool done, item;
    
	bool oops = FALSE;
    
	bool use_inven = ((mode & (USE_INVEN)) ? TRUE : FALSE);
	bool use_equip = ((mode & (USE_EQUIP)) ? TRUE : FALSE);
	bool use_floor = ((mode & (USE_FLOOR)) ? TRUE : FALSE);
	/*bool mode_pick = ((mode & (MODE_PICK)) ? TRUE : FALSE); UNUSED TODO*/

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_floor = FALSE;
    
	bool toggle = FALSE;
    
	char tmp_val[160];
	char out_val[160];
    
	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

	/* per MITZE, always start with the list */
	p_ptr->command_see = TRUE;

#ifdef ALLOW_REPEAT
    
	/* Get the item index */
	if (repeat_pull(cp))
	{
		/* Verify the item */
		if (get_item_okay(*cp))
		{
			/* Forget the item_tester_tval restriction */
			item_tester_tval = 0;
            
			/* Forget the item_tester_hook restriction */
			item_tester_hook = NULL;
            
			/* Success */
			return (TRUE);
		}
		else
		{
			/* Invalid repeat - reset it */
			repeat_clear();
		}
	}
    
#endif /* ALLOW_REPEAT */
    
    
	/* Paranoia XXX XXX XXX */
	msg_print( NULL );
    
    
	/* Not done */
	done = FALSE;
    
	/* No item selected */
	item = FALSE;
    
    
	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;
    
	/* Forbid inventory */
	if (!use_inven) i2 = -1;
    
	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2))) i2--;
    
	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;
    
    
	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = INVEN_TOTAL - 1;
    
	/* Forbid equipment */
	if (!use_equip) e2 = -1;
    
	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2))) e2--;
    
	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;
    
    
	/* Scan all objects in the grid */
	floor_num = scan_floor(floor_list, MAX_FLOOR_STACK, py, px, 0x00);
    
	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;
    
	/* Forbid floor */
	if (!use_floor) f2 = -1;
    
	/* Restrict floor indexes */
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f2]))) f2--;
    
	/* Accept floor */
	if (f1 <= f2) allow_floor = TRUE;
    
    
	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_floor)
	{
		/* Cancel p_ptr->command_see */
		p_ptr->command_see = FALSE;
        
		/* Oops */
		oops = TRUE;
        
		/* Done */
		done = TRUE;
	}
    
	/* Analyze choices */
	else
	{
		/* Hack -- Start on equipment if requested */
		if (p_ptr->command_see &&
		    (p_ptr->command_wrk == (USE_EQUIP)) &&
		    use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}
        
		/* Use inventory if allowed */
		else if (use_inven)
		{
			p_ptr->command_wrk = (USE_INVEN);
		}
        
		/* Use equipment if allowed */
		else if (use_equip)
		{
			p_ptr->command_wrk = (USE_EQUIP);
		}
        
		/* Use floor if allowed , konijn decided that floor is always allowed */
		else /*if (easy_floor)*/
		{
			p_ptr->command_wrk = (USE_FLOOR);
		}
	}
    
    
	/* Start out in "display" mode */
	if (p_ptr->command_see)
	{
		/* Save screen */
		Term_save();
	}
    
    
	/* Repeat until done */
	while (!done)
	{
		/* Show choices */
		if (show_choices)
		{
			int ni = 0;
			int ne = 0;
            
			/* Scan windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++)
			{
				/* Unused */
				if (!angband_term[j]) continue;
                
				/* Count windows displaying inven */
				if (window_flag[j] & (PW_INVEN)) ni++;
                
				/* Count windows displaying equip */
				if (window_flag[j] & (PW_EQUIP)) ne++;
			}
            
			/* Toggle if needed */
			if (((p_ptr->command_wrk == (USE_EQUIP)) && ni && !ne) ||
			    ((p_ptr->command_wrk == (USE_INVEN)) && !ni && ne))
			{
				/* Toggle */
				toggle_inven_equip();
                
				/* Track toggles */
				toggle = !toggle;
			}
            
			/* Update */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);
            
			/* Redraw windows */
			window_stuff();
		}

		/* Viewing inventory */
		if (p_ptr->command_wrk == (USE_INVEN))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_inven();
            
			/* Begin the prompt */
			sprintf(out_val, "Inven:");
            
			/* List choices */
			if (i1 <= i2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
				        index_to_label(i1), index_to_label(i2));
                
				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}
            
			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");
            
			/* Indicate legality of "toggle" */
			if (use_equip) strcat(out_val, " / for Equip,");
            
			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");
		}
        
		/* Viewing equipment */
		else if (p_ptr->command_wrk == (USE_EQUIP))
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_equip();
            
			/* Begin the prompt */
			sprintf(out_val, "Equip:");
            
			/* List choices */
			if (e1 <= e2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,",
				        index_to_label(e1), index_to_label(e2));
                
				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}
            
			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");
            
			/* Indicate legality of "toggle" */
			if (use_inven) strcat(out_val, " / for Inven,");
            
			/* Indicate legality of the "floor" */
			if (allow_floor) strcat(out_val, " - for floor,");
		}
        
		/* Viewing floor */
		else
		{
			/* Redraw if needed */
			if (p_ptr->command_see) show_floor(floor_list, floor_num);
            
			/* Begin the prompt */
			sprintf(out_val, "Floor:");
            
			/* List choices */
			if (f1 <= f2)
			{
				/* Build the prompt */
				sprintf(tmp_val, " %c-%c,", I2A(f1), I2A(f2));
                
				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}
            
			/* Indicate ability to "view" */
			if (!p_ptr->command_see) strcat(out_val, " * to see,");
            
			/* Append */
			if (use_inven) strcat(out_val, " / for Inven,");
            
			/* Append */
			else if (use_equip) strcat(out_val, " / for Equip,");
		}
        
		/* Finish the prompt */
		strcat(out_val, " ESC");
        
		/* Build the prompt */
		strnfmt(tmp_val, sizeof(tmp_val), "(%s) %s", out_val, pmt);
        
		/* Show the prompt */
		prt(tmp_val, 0, 0);
        
        
		/* Get a key */
		which = inkey();
        
		/* Parse it */
		switch (which)
		{
			case ESCAPE:
			{
				done = TRUE;
				break;
			}
                
			case '*':
			case '?':
			case ' ':
			{
				/* Hide the list */
				if (p_ptr->command_see)
				{
					/* Flip flag */
					p_ptr->command_see = FALSE;
                    
					/* Load screen */
					Term_load();
				}
                
				/* Show the list */
				else
				{
					/* Save screen */
					Term_save();
                    
					/* Flip flag */
					p_ptr->command_see = TRUE;
				}
                
				break;
			}
                
			case '/':
			{
				/* Toggle to inventory */
				if (use_inven && (p_ptr->command_wrk != (USE_INVEN)))
				{
					p_ptr->command_wrk = (USE_INVEN);
				}
                
				/* Toggle to equipment */
				else if (use_equip && (p_ptr->command_wrk != (USE_EQUIP)))
				{
					p_ptr->command_wrk = (USE_EQUIP);
				}
                
				/* No toggle allowed */
				else
				{
					msg_bell("Cannot switch item selector!");
					break;
				}
                
				/* Hack -- Fix screen */
				if (p_ptr->command_see)
				{
					/* Load screen */
					Term_load();
                    
					/* Save screen */
					Term_save();
				}
                
				/* Need to redraw */
				break;
			}
                
			case '-':
			{
				/* Paranoia */
				if (!allow_floor)
				{
					msg_bell("Cannot select floor!");
					break;
				}
                
				if (TRUE)
				{
					/* There is only one item */
					if (floor_num == 1)
					{
						/* Hack -- Auto-Select */
						if (p_ptr->command_wrk == (USE_FLOOR))
						{
							/* Special index */
							k = 0 - floor_list[0];
                            
							/* Allow player to "refuse" certain actions */
							if (!get_item_allow(k))
							{
								done = TRUE;
								break;
							}
                            
							/* Accept that choice */
							(*cp) = k;
							item = TRUE;
							done = TRUE;
                            
							break;
						}
					}
                    
					/* Hack -- Fix screen */
					if (p_ptr->command_see)
					{
						/* Load screen */
						Term_load();
                        
						/* Save screen */
						Term_save();
					}
                    
					p_ptr->command_wrk = (USE_FLOOR);
                    
					break;
				}
                
				/* Check each legal object */
				for (i = 0; i < floor_num; ++i)
				{
					/* Special index */
					k = 0 - floor_list[i];
                    
					/* Skip non-okay objects */
					if (!get_item_okay(k)) continue;
                    
					/* Verify the item (if required) */
					if (!verify("Try", k)) continue;
                    
					/* Allow player to "refuse" certain actions */
					if (!get_item_allow(k)) continue;
                    
					/* Accept that choice */
					(*cp) = k;
					item = TRUE;
					done = TRUE;
					break;
				}
                
				break;
			}
                
			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* Look up the tag */
				if (!get_tag(&k, which))
				{
					msg_bell("Illegal object choice (tag)!");
					break;
				}
                
				/* Hack -- Validate the item */
				if ((k < INVEN_WIELD) ? !allow_inven : !allow_equip)
				{
					msg_bell("Illegal object choice (tag)!");
					break;
				}
                
				/* Validate the item */
				if (!get_item_okay(k))
				{
					msg_bell("Illegal object choice (tag)!");
					break;
				}
                
				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
                
				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
                
			case '\n':
			case '\r':
			{
				/* Choose "default" inventory item */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					if (i1 != i2)
					{
						msg_bell("Illegal object choice (default)!");
						break;
					}
                    
					k = i1;
				}
                
				/* Choose "default" equipment item */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					if (e1 != e2)
					{
						msg_bell("Illegal object choice (default)!");
						break;
					}
                    
					k = e1;
				}
                
				/* Choose "default" floor item */
				else
				{
					if (f1 != f2)
					{
						msg_bell("Illegal object choice (default)!");
						break;
					}
                    
					k = 0 - floor_list[f1];
				}
                
				/* Validate the item */
				if (!get_item_okay(k))
				{
					msg_bell("Illegal object choice (default)!");
					break;
				}
                
				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
                
				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
                
			default:
			{
				bool do_verify;
                
				/* Note verify */
				do_verify = (isupper((unsigned char)which) ? TRUE : FALSE);
                
				/* Lowercase */
				which = tolower((unsigned char)which);
                
				/* Convert letter to inventory index */
				if (p_ptr->command_wrk == (USE_INVEN))
				{
					k = label_to_inven(which);
                    
					if (k < 0)
					{
						msg_bell("Illegal object choice (inven)!");
						break;
					}
				}
                
				/* Convert letter to equipment index */
				else if (p_ptr->command_wrk == (USE_EQUIP))
				{
					k = label_to_equip(which);
                    
					if (k < 0)
					{
						msg_bell("Illegal object choice (equip)!");
						break;
					}
				}
                
				/* Convert letter to floor index */
				else
				{
					k = (islower((unsigned char)which) ? A2I(which) : -1);
                    
					if (k < 0 || k >= floor_num)
					{
						msg_bell("Illegal object choice (floor)!");
						break;
					}
                    
					/* Special index */
					k = 0 - floor_list[k];
				}
                
				/* Validate the item */
				if (!get_item_okay(k))
				{
					msg_bell("Illegal object choice (normal)!");
					break;
				}
                
				/* Verify the item */
				if (do_verify && !verify("Try", k))
				{
					done = TRUE;
					break;
				}
                
				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}
                
				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}
    
    
	/* Fix the screen if necessary */
	if (p_ptr->command_see)
	{
		/* Load screen */
		Term_load();
        
		/* Hack -- Cancel "display" */
		p_ptr->command_see = FALSE;
	}
    
    
	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;
    
	/* Forget the item_tester_hook restriction */
	item_tester_hook = NULL;
    
    
	/* Clean up */
	if (show_choices)
	{
		/* Toggle again if needed */
		if (toggle) toggle_inven_equip();
        
		/* Update */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
        
		/* Window stuff */
		window_stuff();
	}
    
    
	/* Clear the prompt line */
	prt("", 0, 0);
    
	/* Warning if needed */
	if (oops && str) msg_print(str);
    
#ifdef ALLOW_REPEAT
    
	/* Save item if available */
	if (item) repeat_push(*cp);
    
#endif /* ALLOW_REPEAT */
    
	/* Result */
	return (item);
}

#if 0
static byte dragon_colour (object_type * o_ptr)
{
	u32b fl = o_ptr->art_flags2;
	if (fl & TR2_RES_CONF) return TERM_ORANGE;
	if (fl & TR2_RES_SOUND) return TERM_VIOLET;
	if (fl & TR2_RES_SHARDS) return TERM_UMBER;
	if (fl & TR2_RES_NETHER) return TERM_L_DARK;
	if (fl & TR2_RES_NEXUS) return TERM_L_RED;
	if (fl & TR2_RES_CHAOS) return (randint(15));
	if (fl & TR2_RES_DISEN) return TERM_L_BLUE;
	if (fl & TR2_RES_LITE) return TERM_WHITE;
	if (fl & TR2_RES_DARK) return TERM_L_DARK;
	if (fl & TR2_RES_ACID) return TERM_GREEN;
	if (fl & TR2_RES_ELEC) return TERM_BLUE;
	if (fl & TR2_RES_FIRE) return TERM_RED;
	if (fl & TR2_RES_COLD) return TERM_L_WHITE;
	if (fl & TR2_RES_POIS) return TERM_L_GREEN;

	return TERM_SLATE;

}
#endif

#if 0
byte object_attr(object_type *  o_ptr)
{
	/* rr9: Changed to display unidentified objects */

	if (!use_graphics)
	{
		if (k_info[(o_ptr)->k_idx].tval == TV_SCROLL)
			return TERM_WHITE;
		else if (k_info[o_ptr->k_idx].tval == TV_AMULET)
		{
			if (object_d_attr(o_ptr->k_idx) == TERM_VIOLET)
				return TERM_L_DARK;
			else
				return object_d_attr((o_ptr)->k_idx);
		}
		else if (object_has_flavor((o_ptr)->k_idx))
			return object_d_attr((o_ptr)->k_idx);

#ifdef SV_DRAGON_HELM
# ifdef SV_DRAGON_SHIELD

		else if (((o_ptr -> tval == TV_HELM) && (o_ptr-> sval == SV_DRAGON_HELM)) ||
			((o_ptr -> tval == TV_SHIELD) && (o_ptr-> sval == SV_DRAGON_SHIELD)))
			return dragon_colour(o_ptr);

# endif /* SV_DRAGON_SHIELD */
#endif /* SV_DRAGON_HELM */

		else return ((k_info[(o_ptr)->k_idx].aware) ?
			(k_info[(o_ptr)->k_idx].x_attr) :
		(k_info[(o_ptr)->k_idx].d_attr));
	}

	else if (streq(ANGBAND_SYS, "ibm"))
	{
		if (k_info[(o_ptr)->k_idx].tval == TV_SCROLL)
			return TERM_WHITE;
		else if (k_info[o_ptr->k_idx].tval == TV_AMULET)
		{
			if (object_d_attr(o_ptr->k_idx) == TERM_VIOLET)
				return TERM_L_DARK;
			else
				return object_d_attr((o_ptr)->k_idx);
		}
		else if (object_has_flavor((o_ptr)->k_idx))
			return object_d_attr((o_ptr)->k_idx);
		else if (((o_ptr -> tval == TV_HELM) && (o_ptr-> sval == SV_DRAGON_HELM)) ||
			((o_ptr -> tval == TV_SHIELD) && (o_ptr-> sval == SV_DRAGON_SHIELD)))
			return dragon_colour(o_ptr);
		else return
			((k_info[(o_ptr)->k_idx].aware) ?
			(k_info[(o_ptr)->k_idx].x_attr) :
		(k_info[(o_ptr)->k_idx].d_attr));
	}
	else return ((k_info[(o_ptr)->k_idx].aware) ?
		(k_info[(o_ptr)->k_idx].x_attr) :
	(k_info[(o_ptr)->k_idx].d_attr));
}

#endif


/*
* Create Alchemy lists
 */
void alchemy_init(void)
{
	int i, j, sv1, sv2, k1, k2;
	
	bool exists;
	
	int potion_list[SV_POTION_MAX]; /* list of k_idxs of all potions, by SVAL */
	
	object_kind *k_ptr;
	
	s16b target_pval = 0;
	s16b pval1 = 0;
	s16b pval2 = 0;
		
	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;
	
	/* Hack -- Induce consistant flavors */
	Rand_value = seed_alchemy;
	
	/* Build list of legal potions */
	for (i = 0; i < SV_POTION_MAX; i++)
	{
		potion_list[i] = -1;
		
		for (j = 0; j < MAX_K_IDX; j++)
		{
			k_ptr = &k_info[j];
			
			/* Found a match */
			if ((k_ptr->tval == TV_POTION) && (k_ptr->sval == i)) potion_list[i] = j;
		}
	}
	
	for (i = 0; i < SV_POTION_MAX; i++)
	{
		k_ptr = &k_info[potion_list[i]];
		
		if (potion_list[i] > -1 && k_ptr->pval > -1)
		{
			while (TRUE)
			{
				sv1 = rand_int(SV_POTION_MAX);
				sv2 = rand_int(SV_POTION_MAX);
				
				k1 = potion_list[sv1];
				k2 = potion_list[sv2];
				
				/* Can't mix with yourself */
				if (sv1 == sv2) continue;
				
				/* Neither can be the resulting potion */
				if ((sv1 == i) || (sv2 == i)) continue;
				
				/* Must be legal potions */
				if ((k1 == -1) || (k2 == -1)) continue;
				
				/*Must be potions that fill the stomach */
				/*For now this excludes really crappy potions (death etc.) and invulnerability*/
				if ((k_info[k1].pval < 0) || (k_info[k2].pval < 0)) continue;
				
				/* Same combination mustn't exist */
				exists = FALSE;
				
				for (j = 0; j < i; j++)
				{
					if (((sv1 == potion_alch[j].sval1) && (sv2 == potion_alch[j].sval2)) ||
						((sv1 == potion_alch[j].sval2) && (sv2 == potion_alch[j].sval1)))
						exists = TRUE;
				}
				
				if (exists) continue;
				
				/* For crazy potions like invulnerability, this is a solution*/
				/* It cuts of pval at 199 */
				/* Note that this whole idea might be stupid, pval in Hell is hunger*/
				/* Whereas Eytans comments seem to indicate that pval is money...*/
				pval1 = k_info[k1].pval + k_info[k1].level +  (s16b)( k_info[k1].cost > 199 ? 199 : k_info[k1].cost);
				pval2 = k_info[k2].pval + k_info[k2].level +  (s16b)( k_info[k2].cost > 199 ? 199 : k_info[k2].cost);
				target_pval = ( k_ptr->pval > 199 ? 199 : k_ptr->pval) + k_ptr->level + (s16b)( k_ptr->cost > 199 ? 199 : k_ptr->cost);
				
				
				/* Check for legal combination */
				if ( pval1 + pval2 < target_pval )
					continue;
				

				
				/* No single component should exceed the total with 100 hunger points */
				if (( pval1 > target_pval+50) || (pval2 > target_pval+50))
					continue;
				
				/* Not too expensive, if possible with a margin of 101 ( originally 1 ) */
				if (pval1 + pval2 > target_pval + 51)
				{
					/* Only rarely allow these potions */
					if (rand_int(15)) continue;
				}
				
				break;
			}
			
			potion_alch[i].sval1 = sv1;
			potion_alch[i].sval2 = sv2;
		}
		else
		{
			potion_alch[i].sval1 = i;
			potion_alch[i].sval2 = i;
		}
	}
	
	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;
}

/*
 * Convert all the different object types into a more general category
 * This will ease some things for squelching but also for determining if
 * an item is for example a book
 */

byte tval_to_sql_hl( byte tval )
{
	switch (tval)
	{
		
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			return SQ_HL_ARMOURS;
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
			return SQ_HL_WEAPONS;
		case TV_SHOT :
		case TV_ARROW :
		case TV_BOLT :
			return SQ_HL_AMMO;
		case TV_BOTTLE:
		case TV_SPIKE:
		case TV_FLASK:
		case TV_FOOD:
		case TV_JUNK:
			return SQ_HL_MISC;
		case TV_CHEST:
			return SQ_HL_CHESTS;
		case TV_LITE:
			return SQ_HL_LIGHTS;
		case TV_AMULET:
		case TV_RING:
			return SQ_HL_JEWELRY;
		case TV_STAFF:
			return SQ_HL_STAVES;
		case TV_WAND:
			return SQ_HL_WANDS;
		case TV_ROD:
			return SQ_HL_RODS;
		case TV_SCROLL:
			return SQ_HL_SCROLLS;
		case TV_MIRACLES_BOOK:
		case TV_SORCERY_BOOK:
		case TV_NATURE_BOOK:
		case TV_CHAOS_BOOK:
		case TV_DEATH_BOOK:
		case TV_TAROT_BOOK:
		case TV_CHARMS_BOOK:
		case TV_SOMATIC_BOOK:
		case TV_DEMONIC_BOOK:
			return SQ_HL_BOOKS;
		default:
			return SQ_HL_OTHERS;
	}
}

/*
 cptr value_check_aux1(object_type *o_ptr)
 return "terrible";
 return "special";
 return "worthless";
 return "excellent";
 return "cursed";
 return "broken";
 return "good";
 return "good";
 return "worthless";
 return "great";
 return "average";
 cptr value_check_aux2(object_type *o_ptr)
 return "cursed";
 return "broken";
 return "good";
 return "good";
 return "good";
 return "good";
 */

/* Mark it as a goner*/
void mark_squelch( object_type *o_ptr  )
{
	/* Take note */
	o_ptr->squelch = TRUE;
}

void do_squelch( void )
{
	object_type		*o_ptr;
	object_type		*prev_o_ptr=NULL;
	char o_name[80];
	cave_type *c_ptr;
	s16b this_o_idx, next_o_idx = 0;
	int i;
	
	/* Check everything */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];
		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;
		
		/*If we need to squelch , we squelcht*/
		if( o_ptr->squelch )
		{
			/* Describe the object */
			object_desc(o_name, o_ptr, TRUE, 3);
			msg_format("You stomp %s." , o_name);msg_print(NULL);
			inven_item_increase(i, -o_ptr->number);
			inven_item_describe(i);
			inven_item_optimize(i);
			/*All items just dropped a spot, we do this not to skip the next potentially squelchable item*/
			i=-1;
		}
	}
	
	/* Check the grid */
	c_ptr = &cave[py][px];
	
	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Acquire object */
		o_ptr = &o_list[this_o_idx];
		
		/* Skip empty stuff */
		if (!o_ptr->k_idx) continue;
		
		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;
		
		/* Wipe the object if needed,*/
		if( o_ptr->squelch )
		{
			/* Describe the object */
			object_desc(o_name, o_ptr, TRUE, 3);
			/*Inform the player*/
			msg_format("You stomp %s on the floor." , o_name);
			object_wipe(o_ptr);
			/* Count objects */
			o_cnt--;
			if(prev_o_ptr==NULL)
				c_ptr->o_idx = next_o_idx;
			else
				prev_o_ptr->next_o_idx = next_o_idx;
		}
		else
		{
			prev_o_ptr = o_ptr;
		}
	}
	
}

/*
 * Yah, so the masses wanted squelch and konijn provided squelch
 * I am not sure yet that this is a good idea, but such is life ;)
 * I keep being reminded by Leon Marrick on this one ;)
 * So, anyway an item index is given, positive is inventory,negative is floor
 * First we check what is the policy and do some minimum investigation of the object
 * Then depending on the squelch rule and what we know about the object do we decide to squelch
 * Then after that we check for all kinds of sanity checks that might prevent the squelching
 * Then we do the actual killing
 */

void consider_squelch( object_type *o_ptr )
{
	byte sq_hl;
	byte sq_option;
	bool storebought;
	bool id;
	bool fullid;
	bool sense;
	bool go_squelch;
	bool heavy;
	cptr	feel , feel_heavy;
	u32b f1,f2,f3;
	char o_name[80];
	cptr s;
	
	/*If we know already we need to squelch, get out*/
	if( o_ptr->squelch )
		return;
	
	/*Paranoia in case strchr might bomb out on empty inscriptions*/
	if(quark_str(o_ptr->note))
	{
		/*If the item is protected with !k we wont squelch*/
		/* First find a '!' */
		s = strchr(quark_str(o_ptr->note), '!');
		
		/* Process preventions */
		while (s)
		{
			/* Check the "restriction" */
			if ((s[1] == 'k') || (s[1] == '*'))
				return;
			/* Find another '!' */
			s = strchr(s + 1, '!');
		}
	}

	/* Dragon Armor is rare enough, we should not squelch it */
	if( o_ptr->tval == TV_DRAG_ARMOR )
		return;

	/* Get the item-category */
	sq_hl = tval_to_sql_hl( o_ptr->tval );
	
	/* If we dont know about the TVAL ( eg ) potions, we get out*/
	if(sq_hl == SQ_HL_OTHERS)
		return;
	
	/* Get the squelching option */
	sq_option = squelch_options[sq_hl];
	
	/* If we aint squelchin, we aint squelchin*/
	if(!sq_option)
		return;
	
	/* Get all options for future reference */
	object_flags( o_ptr , &f1 , &f2 , &f3 );
	
	/* Get some id flags for future reference */
	storebought = (o_ptr->ident & IDENT_STOREB);
	id          = (o_ptr->ident & (IDENT_MENTAL | IDENT_KNOWN ));
	fullid      = (o_ptr->ident & IDENT_MENTAL) | (byte_hack)( f3 | TR3_EASY_KNOW  );
	sense       = (o_ptr->ident & IDENT_SENSE);
	heavy       = has_heavy_pseudo_id();
	/* Check for feelings */
	feel       = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));
	if(!feel)
		feel="";
	feel_heavy = value_check_aux1(o_ptr);
	/* Assume we wont squelch */
	go_squelch = FALSE;
	
	/* Describe the object */
	object_desc(o_name, o_ptr, TRUE, 3);
	
	if (arg_fiddle)
	{
		msg_format("Squelch %s ?" , o_name);
		msg_format("SVAL %d ?" , o_ptr->sval);
		msg_format("Option for that :(%s)" , squelch_strings[sq_option] );
		if(sense || id)
		{
			msg_format("Feel:%s" , feel );
			if(heavy || id)
				msg_format("Feel_heavy : %s." , feel_heavy );
		}else{
			msg_format("No feeling or id." , feel_heavy );
		}
	}
	
	if( sq_hl == SQ_HL_ARMOURS || sq_hl == SQ_HL_WEAPONS || sq_hl == SQ_HL_AMMO )
	{
		/*If we only want artefacts we squelch it all, artefacts are the only ones to survive that*/
		if( sq_option == SQUELCH_ARTIFACT )
			go_squelch = TRUE;
		/*If we want excellent and artefacts, we need to have id or powerfull sense that it is excellent or have it id'd*/
		if( sq_option == SQUELCH_EXCELLENT )
		{
			/*If we have heavy pseudo-id or we id'd it, we can deleted up to good*/
			if( ( (sense && heavy) || id ) &&
				(
				  streq(feel_heavy, "cursed") ||
				  streq(feel_heavy, "broken")  ||
				  streq(feel_heavy, "average") ||
				  streq(feel_heavy, "good")
				  )
				)
				go_squelch = TRUE;
			/*If we have weak pseudo-id and not id'd it,we can delete up to average*/
			if( sense && !heavy && ( streq(feel, "cursed") ||  streq(feel, "broken")  || streq(feel, "") ) )
				go_squelch = TRUE;
		}
		
		if( sq_option == SQUELCH_GOOD )
		{
			/*If we have pseudo id (weak or not) or id, we can delete cursed and average*/
			if( ( sense || id ) && ( streq(feel_heavy, "cursed") ||  streq(feel_heavy, "average") ||  streq(feel_heavy, "broken") || streq(feel_heavy, "") ) )
				go_squelch = TRUE;
		}
	}
	
	if( sq_hl == SQ_HL_JEWELRY || sq_hl == SQ_HL_LIGHTS )
	{
		/*If we only want artefacts we squelch it all, artefacts are the only ones to survive that*/
		if( sq_option == SQUELCH_ARTIFACT )
			go_squelch = TRUE;
		/*If we want excellent and artefacts, we need to have id or powerfull sense that it is excellent or have it id'd*/
		if( sq_option == SQUELCH_GREAT )
		{
			/*If we have heavy pseudo-id or we id'd it, we can deleted up to good*/
			if( ( (sense && heavy) || id ) && ( streq(feel_heavy, "cursed") ||  streq(feel_heavy, "broken")  || streq(feel_heavy, "average") || streq(feel_heavy, "good") ) )
				go_squelch = TRUE;
			/*If we have heavy pseudo-id or we id'd it, we can deleted up to average*/
			if( ( (sense && !heavy)  ) && ( streq(feel, "cursed") || streq(feel, "broken")  || streq(feel, "") ) )
				go_squelch = TRUE;
			/* These can never be excellent or great, the day they can, this will be a bug ;) */
			if( o_ptr->sval == SV_LITE_LANTERN || o_ptr->sval == SV_LITE_TORCH )
				go_squelch = TRUE;
		}
	}
	
	if( sq_hl == SQ_HL_SCROLLS || sq_hl == SQ_HL_WANDS || sq_hl == SQ_HL_RODS || sq_hl == SQ_HL_STAVES )
	{
		/* We want it all gone, assumed is that sanity checks are configured on tval-sval level */
		if( sq_option == SQUELCH_ALL )
			go_squelch = TRUE;
		
		/*If we want great stuff, we need to have id or powerfull sense that it is great or have it id'd*/
		if( sq_option == SQUELCH_GREAT )
		{
			/*If we have heavy pseudo-id or we id'd it, we can deleted up to good*/
			if( ( (sense && heavy) || id ) && ( streq(feel_heavy, "cursed") || streq(feel_heavy, "broken") || streq(feel_heavy, "average") || streq(feel_heavy, "good") ) )
				go_squelch = TRUE;
		}
	}
	
	if( sq_hl == SQ_HL_BOOKS )
	{
		/* We want it all gone, assumed is that we are playing a warrior or that all books have been found and marked {!k}  */
		if( sq_option == SQUELCH_ALL )
			go_squelch = TRUE;
		/* We want it all gone,but for unknown stuff we wait for it to be id'd, assumed is that sanity checks are configured on tval-sval level */
		if( sq_option == SQUELCH_TOWN_BOOKS && o_ptr->sval < 2 )
			go_squelch = TRUE;
	}
	
	if( sq_hl == SQ_HL_MISC )
	{
		/* We want it all gone, assumed is that we are playing a warrior or that all books have been found and marked {!k}  */
		if( sq_option == SQUELCH_ALL )
			go_squelch = TRUE;
	}
	
	if( sq_hl == SQ_HL_CHESTS )
	{
		/* We want it all gone, assumed is that we is so tuff that we no need no crummy chests ;)  */
		if( sq_option == SQUELCH_ALL )
			go_squelch = TRUE;
		if( sq_option == SQUELCH_OPENED && o_ptr->pval == 0 )
			go_squelch = TRUE;
	}
	
	/*If we dont squelch then getoutahere */
	if(!go_squelch)
		return;
	
	if (arg_fiddle)
		msg_print("go_squelch=TRUE");
	
	if (arg_fiddle)
		msg_print("Artefact ?");
	/* Now we check for aretefacts */
	/* Artifacts cannot be destroyed */
	if (artefact_p(o_ptr) || o_ptr->art_name)
	{
		feel = "special";
		/* Message */
		/*msg_format("You cannot stomp %s.", o_name);*/
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
	if (arg_fiddle)
		msg_print("No.");
	
	if (arg_fiddle)
		msg_print("Sane & Storebought ?");
	/* Check for storebought only now ;) */
	if( sanity_store && storebought )
		return;
	
	if (arg_fiddle)
		msg_print("No.");
	
	if (arg_fiddle)
		msg_print("Sane & Unknown?");
	/*Check for the known status of consumables*/
	if( sanity_id && !( id || k_info[o_ptr->k_idx].aware ) )
		return;
	if (arg_fiddle)
		msg_print("No.");
	
	if (arg_fiddle)
		msg_print("Sane & High Price?");
	/*Check for sanity check on price*/
	if( sanity_price && id && object_value(o_ptr) > 0 && object_value(o_ptr) > sane_price )
		return;
	if (arg_fiddle)
		msg_print("No.");
	
	/* There are other sanity checks for misc stuff and chests */
	if( sq_hl == SQ_HL_CHESTS || sq_hl == SQ_HL_MISC )
	{
		mark_squelch(o_ptr);
		return;
	}
	
	/* Check for own realm if needed, otherwise kill it*/
	if( sq_hl == SQ_HL_BOOKS && sanity_realm)
	{
		if( o_ptr->tval-TV_MIRACLES_BOOK == p_ptr->realm1 - 1 || o_ptr->tval-TV_MIRACLES_BOOK== p_ptr->realm2-1  )
			return;
	}
	

	/* We make an exception, if we are dealing with boots that are not cursed
	   they might be boots of speed , so lets keep those */

	if( o_ptr->tval == TV_BOOTS && sq_option == SQUELCH_ARTIFACT )
	{
		go_squelch = FALSE;
		/*If we have heavy pseudo-id or we id'd it, we can deleted up to good*/
		if( ( (sense && heavy) || id ) &&
			(
			  streq(feel_heavy, "cursed") ||
			  streq(feel_heavy, "broken")  ||
			  streq(feel_heavy, "average") ||
			  streq(feel_heavy, "good")
			  )
			)
		go_squelch = TRUE;
		/*If we have weak pseudo-id and not id'd it,we can delete up to average*/
		if( sense && !heavy && ( streq(feel, "cursed") ||  streq(feel, "broken")  || streq(feel, "") ) )
			go_squelch = TRUE;
		/*If we dont squelch then getoutahere */
		if(!go_squelch)
			return;
		/* Paranoia, if anybody afterwards still checks for this... */
		go_squelch = TRUE;
	}

	/*Only sanity checks that are left need id to trigger so give it up if have no ID*/
	if(!id || cursed_p(o_ptr))
	{
		mark_squelch(o_ptr);
		return;
	}
	
	/* By know we know it is not cursed , so we assumed the player put it on to try it */
	if( sanity_speed && ( TR1_SPEED & f1 ) )
		return;
	
	/* Immunities are not as obvious , so we need *id* or obvious stuff */
	if( fullid  && sanity_immune && ( f2 & ( TR2_IM_ACID | TR2_IM_ELEC | TR2_IM_FIRE | TR2_IM_COLD ) ) )
		return;
	
	/* By know we know it is not cursed , so we assumed the player put it on to try it */
	if( sanity_telepathy && ( TR3_TELEPATHY & f3 ) )
		return;
	
	/* High resists are not as obvious , so we need *id* or obvious stuff */
	if( fullid  && sanity_immune && ( f2 & ( TR2_RES_CHAOS | TR2_RES_DISEN | TR2_RES_NETHER | TR2_RES_NEXUS ) ) )
		return;
	
	/*Sanity didnt save us, and now the thingy must die*/
	mark_squelch(o_ptr);
	
}


/* Add a flags group */
void gain_ego_realm(object_type *o_ptr, bool silent)
{
	int tries = 1000;
	int i;
	int points = 0;

	int  realm;
	u32b realms;

	/* We are adding the 2 bitflags bytes together into a u32b */
	realms = ( o_ptr->xtra1 << 8 ) + o_ptr->xtra2;

	/* Count how many points have been spent so far */
	for( i = 0 ; i < MAX_EGO_REALMS ; i++ )
	{
		if( realms & BIT( i ) )
			points = points + realm_flags[i].price;
	}

	while (tries--)
	{
		realm = rand_int(MAX_EGO_REALMS);

		/* If we already got this group continue */
		if (realms & BIT(realm)) continue;

		/* Not enough points ? */
		if (realm_flags[realm].price > o_ptr->elevel - points) continue;

		/* Ok, enough points and not already got it */
		break;
	}

	/* Ack, nothing found */
	if (tries <= 1) return;

	realms |= BIT(realm);

	if (!silent)
	{
		char o_name[80];

		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("%s gains access to the %s realm.", o_name, realm_flags[realm].name);
	}

	//Store realm back in xtra 1 and 2
	//realms = o_ptr->xtra1 << 8 + o_ptr->xtra2;
	o_ptr->xtra2 = realms & 0xFF;
	o_ptr->xtra1 = (byte)(realms >> 8);
}

/* Add a flags from a flag group */
void gain_ego_realm_flag(object_type *o_ptr, bool silent)
{
	int i , choice, k = 0;
	u32b f = 0;
	u32b *of;
	int tries = 20000;
	byte odds[33];
	int odds_count = 0;

	int  realm;
	u32b realms;

	/* We are adding the 2 bitflags bytes together into a u32b */
	realms = ( o_ptr->xtra1 << 8 ) + o_ptr->xtra2;

	/* If no realms are defined, we should consider yelling, but return quietly instead */
	if (!realms) return;

	while (tries--)
	{
		/* get a flag group */
		realm = rand_int(MAX_EGO_REALMS);

		if (!(realms & BIT(realm))) continue;

		k = randint(3);
		odds_count = 0;

		if(k==1){ f = realm_flags[realm].flags1; of = &(o_ptr->art_flags1); }
		else if(k==2){ f = realm_flags[realm].flags2; of = &(o_ptr->art_flags2); }
		else if(k==3){ f = realm_flags[realm].flags3; of = &(o_ptr->art_flags3); }
		else { msg_format("randint(3) returned %i.", k); return; }

		/* Loop over all 32 bits */
		for( i = 0 ; i < 32 ; i++ )
		{
			/* Assume this flag is not valid */
			odds[i] = 0;
			/* Only consider the flag if its not already present */
			if( !(*of & BIT(i)) && ( f & BIT(i) ) )
			{
				odds[i] = 1 + odds_count++;
			}
			/* There is a small chance for res_xxx to turn into IM_xxx */
			else if( k == 2 && i >= TR2_RES_ACID && i <= TR2_RES_COLD && magik(15) )
			{
				i = i - 8; /* Distance from RES_xxx to IMM_xxx */
				odds[i] = 1 + odds_count++;
			}
		}
		if( !odds_count ) continue;
		choice = randint( odds_count );
		for( i = 0 ; i < 32 ; i++ ){
			if( odds[i] == choice ){
				*of = *of | BIT(i);
			}
		}
		break;
	}

	if (tries <= 1) return;

	if (!silent)
	{
		char o_name[80];

		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("%s gains a new power from the %s realm.", o_name, realm_flags[realm].name);
	}
}

/*
 * When an object gain a level, he can gain some attributes
 */
void object_gain_level(object_type *o_ptr)
{
	u32b f1, f2, f3;
	byte hlt_tval; /* High Level TVAL */

	/* Get the item-category */
	hlt_tval = tval_to_sql_hl( o_ptr->tval );

	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3 );

	/* First it can gain some tohit and todam */
	if ( hlt_tval == SQ_HL_WEAPONS )
	{
		int k = rand_int(100);

		/* gain +2,+1 */
		if (k < 33)
		{
			o_ptr->to_h += (s16b)randint(2);
			o_ptr->to_d += 1;
		}
		/* +1 and 1 point */
		else if (k < 66)
		{
			o_ptr->to_h += 1;

			if (magik(40)) /* 40 percent chance of access to new group, 1/3rd of the time */
				gain_ego_realm(o_ptr, FALSE);

			if (magik(40) && o_ptr->to_a < 12 ) /* 40 percent chance of higher armor defense value */
				o_ptr->to_a += 1;

			if (magik(40) && o_ptr->to_d < 16 ) /* 40 percent chance of higher armor defense value */
				o_ptr->to_d += 1;
		}
		else
		{
			if (!o_ptr->xtra1 && !o_ptr->xtra2) gain_ego_realm(o_ptr, FALSE);
			gain_ego_realm_flag(o_ptr, FALSE);

			if (!o_ptr->pval)
				o_ptr->pval = 1;
			else
			{
				if (o_ptr->pval > 4) return;
				while (magik(20 - (o_ptr->pval * 2))) o_ptr->pval++;
				if (o_ptr->pval > 5) o_ptr->pval = 5;
			}
		}
	}
}

/*
 * Advance experience levels and print experience
 */
void check_experience_obj(object_type *o_ptr)
{
	/* Hack -- lower limit */
	if (o_ptr->exp < 0) o_ptr->exp = 0;

	/* Hack -- upper limit */
	if (o_ptr->exp > PY_MAX_EXP) o_ptr->exp = PY_MAX_EXP;

	/* Gain levels while possible */
	while ( (o_ptr->elevel < PY_MAX_LEVEL) && (o_ptr->exp >= player_exp[o_ptr->elevel] ) )
	{
		char buf[120];

		/* Add a level */
		o_ptr->elevel++;

		/* Get object name */
		object_desc(buf, o_ptr, 0, 0);
		/*TODO: This needs to be displayed in light blue ;]*/
		msg_format(/*TERM_L_BLUE, */ "%s gains a level!", buf);

		/* What does it gains ? */
		object_gain_level(o_ptr);
	}
}
