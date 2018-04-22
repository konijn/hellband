/* File: borg3.c */

/* Purpose: Object and Spell routines for the Borg -BEN- */

#include "angband.h"

#ifdef ALLOW_BORG

#include "zborg1.h"
#include "zborg3.h"

/*
 * This file helps the Borg analyze "objects" and "shops", and to
 * deal with objects and spells.
 */

/*
 * Some variables
 */

borg_item *borg_items; /* Current "inventory" */

borg_shop *borg_shops; /* Current "shops" */

/*
 * Safety arrays for simulating possible worlds
 */

borg_item *safe_items; /* Safety "inventory" */
borg_item *safe_home;  /* Safety "home stuff" */

borg_shop *safe_shops; /* Safety "shops" */

/*
 * Spell info
 */

borg_magic borg_magics[8][4][8]; /* Spell info, by realm/book/what*/
borg_mind borg_minds[MAX_MINDCRAFT_POWERS];

/* Food Names */
static char *food_syllable1[] = {
	 "BBQ ",		"Boiled ",	"Fresh ",	"Frozen ",	"Burned ",
	 "Rotten ", "Raw ",		 "Toasted ", "Broiled ",  "Baked ",
	 "Fried ",  "Buttered ", "Steamed ", "Gramma's ",
};

/* Food Names */
static char *food_syllable2[] = {
	 "Pizza",			  "Eggs",			"Spam",		  "Oatmeal", "Chicken",
	 "Bacon",			  "Peanutbutter", "Roast Beef", "Cheese",  "Toast",
	 "Hamburger",		  "Carrots",		"Corn",		  "Potato",  "Pork Chops",
	 "Chinese Takeout", "Cookies",
};

/* Slime Molds */
static char *mold_syllable1[] = {
	 "Ab", "Ac",  "Ad", "Af",  "Agr", "Ast", "As",  "Al", "Adw",  "Adr", "Ar",
	 "B",  "Br",  "C",  "Cr",  "Ch",  "Cad", "D",	"Dr", "Dw",	"Ed",  "Eth",
	 "Et", "Er",  "El", "Eow", "F",	"Fr",  "G",	"Gr", "Gw",	"Gal", "Gl",
	 "H",  "Ha",  "Ib", "Jer", "K",	"Ka",  "Ked", "L",  "Loth", "Lar", "Leg",
	 "M",  "Mir", "N",  "Nyd", "Ol",  "Oc",  "On",  "P",  "Pr",	"R",	"Rh",
	 "S",  "Sev", "T",  "Tr",  "Th",  "V",	"Y",	"Z",  "W",	 "Wic",
};

static char *mold_syllable2[] = {
	 "a",		 "adrie", "ara",	"e",	"ebri", "ele",  "ere", "i", "io",
	 "ithra", "ilma",  "il-Ga", "ili", "o",	 "orfi", "u",	"y",
};

static char *mold_syllable3[] = {
	 "bur", "fur", "gan", "gnus", "gnar", "li",  "lin", "lir",
	 "mli", "nar", "nus", "rin",  "ran",  "sin", "sil", "sur",
};

/*
 * Define some MindCraft Spells
 #define MIND_PRECOGNIT 	 0
 #define MIND_NEURAL_BL	 1
 #define MIND_MINOR_DISP  2
 #define MIND_MAJOR_DISP  3
 #define MIND_DOMINATION  4
 #define MIND_PULVERISE   5
 #define MIND_CHAR_ARMOUR 6
 #define MIND_PSYCHOMETRY 7
 #define MIND_MIND_WAVE   8
 #define MIND_ADRENALINE  9
 #define MIND_PSYCHIC_DR	10
 #define MIND_TELE_WAVE	11
 */

/*
 * Hack -- help analyze the magic
 *
 * The comments yield the "name" of the spell or prayer.
 *
 * Also, the leading letter in the comment indicates how we use the
 * spell or prayer, if at all, using "A" for "attack", "D" for "call
 * light" and "detection", "E" for "escape", "H" for healing, "O" for
 * "object manipulation", and "F" for "terrain feature manipulation",
 * plus "!" for entries that can soon be handled.
 */

static byte borg_magic_method[8][4][8] = {
	 { /* 0 Realm 0 -- Non spell caster*/
	  {/* Book0... (sval 0) */
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */},
	  {/* Book1... (sval 1) */
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /* ! "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */},
	  {/* Book0... (sval 2) */
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */},
	  {/* Book3... (sval 3) */
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /* ! "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */, BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */,
		BORG_MAGIC_ICK /*   "(blank)" */}}, /* end of realm 0 */

	 {/* 1 Life Realm */
	  {
			/* Common Prayers (sval 0) */
			BORG_MAGIC_NOP /*   "Detect Evil" */,
			BORG_MAGIC_NOP /*   "Cure Light Wounds" */,
			BORG_MAGIC_NOP /*   "Bless" */, BORG_MAGIC_NOP /* H "Remove Fear" */,
			BORG_MAGIC_NOP /* D "Call Light" */,
			BORG_MAGIC_NOP /* D "Find Traps" */,
			BORG_MAGIC_NOP /* D "Cure Medium Wounds" */,
			BORG_MAGIC_NOP /*   "Satisfy Hunger" */,
	  },

	  {
			/* High Mass (sval 1) */
			BORG_MAGIC_NOP /*   "Remove Curse" */,
			BORG_MAGIC_NOP /* E "Cure Poison" */,
			BORG_MAGIC_NOP /* H "Cure Crit Wounds" */,
			BORG_MAGIC_NOP /*   "See Inv" */, BORG_MAGIC_AIM /*   "Holy Orb" */,
			BORG_MAGIC_NOP /* H "PFE" */, BORG_MAGIC_NOP /*   "Healing" */,
			BORG_MAGIC_NOP /*   "Rune of Protection" */,
	  },

	  {/* Book of the Unicorn (sval 2) */
		BORG_MAGIC_NOP /* H "Exorcism" */, BORG_MAGIC_NOP /* A "Dispel Curse" */,
		BORG_MAGIC_NOP /* H "Disp Undead and Demon" */,
		BORG_MAGIC_NOP /*   "Day of Dove" */,
		BORG_MAGIC_NOP /*   "Dispel Evil" */, BORG_MAGIC_NOP /* D "Banishment" */,
		BORG_MAGIC_NOP /* H "Holy Word" */,
		BORG_MAGIC_NOP /*   "Warding True" */},

	  {/* Blessings of the Grail (sval 3) */
		BORG_MAGIC_NOP /*   "Heroism" */, BORG_MAGIC_NOP /* ! "Prayer" */,
		BORG_MAGIC_OBJ /* H "Bless Weapon" */,
		BORG_MAGIC_NOP /* ! "Restoration" */,
		BORG_MAGIC_NOP /*   "Healing True" */,
		BORG_MAGIC_OBJ /* ! "Holy Vision" */,
		BORG_MAGIC_NOP /*   "Divine Intervent" */,
		BORG_MAGIC_NOP /*   "Holy Invuln" */}

	 }, /* endof Life Realm */

	 { /*2. Sorcery Realm*/
	  {/* Beginners Handbook (sval 0) */
		BORG_MAGIC_NOP /*   "Detect Monster" */,
		BORG_MAGIC_NOP /*   "Phase Door" */,
		BORG_MAGIC_NOP /*   "Detect Doors & Traps" */,
		BORG_MAGIC_NOP /*   "Light Area" */,
		BORG_MAGIC_AIM /*   "Confuse Monster" */,
		BORG_MAGIC_NOP /*   "Teleport Self" */,
		BORG_MAGIC_AIM /*   "Sleep Monster" */,
		BORG_MAGIC_OBJ /*   "Recharging" */},

	  {/* Master Sorc (sval 1) */
		BORG_MAGIC_NOP /*   "Magic Map" */, BORG_MAGIC_OBJ /*   "Ident" */,
		BORG_MAGIC_AIM /*   "Slow Monster" */,
		BORG_MAGIC_NOP /*   "Mass Sleep " */,
		BORG_MAGIC_AIM /*   "Teleport Away" */,
		BORG_MAGIC_NOP /*   "Haste Self" */,
		BORG_MAGIC_NOP /*   "Detection True" */, BORG_MAGIC_OBJ /*   "*ID*" */},

	  {/* Pattern Sorc (sval 2) */
		BORG_MAGIC_OBJ /*   "Elemental Branding" */,
		BORG_MAGIC_NOP /*   "Detect Enchant" */,
		BORG_MAGIC_AIM /*   "Charm Mon" */,
		BORG_MAGIC_EXT /*   "Dimension Door" */,
		BORG_MAGIC_NOP /*   "Sense Minds" */,
		BORG_MAGIC_NOP /*   "Self Knowledge" */,
		BORG_MAGIC_NOP /*   "Teleport Level" */,
		BORG_MAGIC_NOP /*   "Word of Recall" */},

	  {/* Grimoir of Power (sval 3) */
		BORG_MAGIC_AIM /*   "Stasis" */, BORG_MAGIC_AIM /*   "Telekinesis" */,
		BORG_MAGIC_NOP /*   "Explosive Rune" */,
		BORG_MAGIC_NOP /*   "Clairvoyance" */,
		BORG_MAGIC_OBJ /*   "*Enchant Weap" */,
		BORG_MAGIC_OBJ /*   "*Enchant Armor" */, BORG_MAGIC_ICK /*   "Alchemy" */,
		BORG_MAGIC_NOP /*   "GOI" */}}, /* End of Sorcery Realm */

	 { /* 3 Nature Realm */
	  {/* Call of the Wild (sval 0) */
		BORG_MAGIC_NOP /*   "Detect Creature" */,
		BORG_MAGIC_NOP /*   "First Aid" */, BORG_MAGIC_NOP /* ! "Detect Door" */,
		BORG_MAGIC_NOP /*   "Foraging" */, BORG_MAGIC_NOP /*   "Daylight" */,
		BORG_MAGIC_AIM /*   "Animal Taming" */,
		BORG_MAGIC_NOP /*   "Resist Environment" */,
		BORG_MAGIC_NOP /*   "Cure Wound&Poison" */},
	  {/* Nature Mastery (sval 1) */
		BORG_MAGIC_AIM /* ! "Stone to Mud" */,
		BORG_MAGIC_AIM /* ! "Lightning Bolt" */,
		BORG_MAGIC_NOP /*   "Nature Awareness" */,
		BORG_MAGIC_AIM /*   "Frost Bolt" */,
		BORG_MAGIC_AIM /*   "Ray of Sunlight" */,
		BORG_MAGIC_NOP /*   "Entangle" */,
		BORG_MAGIC_NOP /*   "Summon Animals" */,
		BORG_MAGIC_NOP /*   "Herbal Healing" */},
	  {/* Nature Gifts (sval 2) */
		BORG_MAGIC_NOP /* ! "Door Building" */,
		BORG_MAGIC_NOP /*   "Stair Building" */,
		BORG_MAGIC_NOP /* ! "Stone Skin" */,
		BORG_MAGIC_NOP /*   "Resistance True" */,
		BORG_MAGIC_NOP /*   "Animal Friend" */,
		BORG_MAGIC_OBJ /*   "Stone Tell" */,
		BORG_MAGIC_NOP /*   "Wall of Stone" */,
		BORG_MAGIC_OBJ /*   "Protect From Corros." */},
	  {/* Natures Wrath (sval 3) */
		BORG_MAGIC_NOP /* ! "Earthquake" */, BORG_MAGIC_NOP /*   "Whirlwind" */,
		BORG_MAGIC_AIM /* ! "Blizzard" */, BORG_MAGIC_AIM /*   "Lightning" */,
		BORG_MAGIC_AIM /*   "Whirpool" */, BORG_MAGIC_NOP /*   "Call Sunlight" */,
		BORG_MAGIC_OBJ /*   "Elemental Brand" */,
		BORG_MAGIC_NOP /*   "Natures Wrath" */}}, /* end of Natural realm  */

	 { /* 4.Chaos Realm */
	  {/* Sign of Chaos... (sval 0) */
		BORG_MAGIC_AIM /* "Magic Missile" */,
		BORG_MAGIC_NOP /* "Trap/Door Dest" */,
		BORG_MAGIC_NOP /* "Flash of Light" */,
		BORG_MAGIC_NOP /* "Touch of Conf" */, BORG_MAGIC_AIM /* "ManaBurst" */,
		BORG_MAGIC_AIM /* "Fire Bolt" */, BORG_MAGIC_AIM /* "Fist of Force" */,
		BORG_MAGIC_NOP /* "Teleport" */},
	  {/* Chaos Mastery... (sval 1) */
		BORG_MAGIC_AIM /*   "Wonder" */, BORG_MAGIC_AIM /*   "Chaos Bolt" */,
		BORG_MAGIC_NOP /*   "Sonic Boom" */, BORG_MAGIC_AIM /*   "Doom Beam" */,
		BORG_MAGIC_AIM /*   "Fire Ball" */,
		BORG_MAGIC_AIM /*   "Teleport Other" */,
		BORG_MAGIC_NOP /*   "Word of Dest" */,
		BORG_MAGIC_AIM /*   "Invoke Logrus" */},
	  {/* Chaos Channels (sval 2) */
		BORG_MAGIC_AIM /*   "Polymorph Other" */,
		BORG_MAGIC_NOP /*   "Chain Lightn" */,
		BORG_MAGIC_OBJ /*   "Arcane Binding (recharge)" */,
		BORG_MAGIC_AIM /*   "Disintegration" */,
		BORG_MAGIC_NOP /*   "Alter Reality" */,
		BORG_MAGIC_ICK /*   "Polymorph Self" */,
		BORG_MAGIC_OBJ /*   "Chaos Branding" */,
		BORG_MAGIC_NOP /*   "Summon Demon" */},
	  {/* Armageddon Tome (sval 3) */
		BORG_MAGIC_AIM /*   "Gravity Beam" */,
		BORG_MAGIC_NOP /*   "Meteor Swarm" */,
		BORG_MAGIC_NOP /*   "Flame Strike" */,
		BORG_MAGIC_NOP /*   "Call Chaos" */,
		BORG_MAGIC_AIM /*   "Magic Rocket" */,
		BORG_MAGIC_AIM /*   "Mana Storm" */,
		BORG_MAGIC_AIM /*   "Breath Logrus" */,
		BORG_MAGIC_NOP /*   "Call Void" */}}, /* end of Chaos Realm*/

	 { /* 5. Death Realm */
	  {/* Black Prayers (sval 0) */
		BORG_MAGIC_NOP /* ! "Detect Unlife" */,
		BORG_MAGIC_AIM /*   "Maledition" */, BORG_MAGIC_NOP /* ! "Detect Evil" */,
		BORG_MAGIC_AIM /*   "Stinking Cloud" */,
		BORG_MAGIC_AIM /*   "Black Sleep" */,
		BORG_MAGIC_NOP /*   "Resist Poison" */, BORG_MAGIC_AIM /*   "Horrify" */,
		BORG_MAGIC_AIM /*   "Enslave Undead" */},
	  {/* Black Mass (sval 1) */
		BORG_MAGIC_AIM /* ! "Orb of Entropy" */,
		BORG_MAGIC_AIM /*   "Nether Bolt" */, BORG_MAGIC_NOP /*   "Terror" */,
		BORG_MAGIC_AIM /*   "Vamp Drain" */,
		BORG_MAGIC_OBJ /*   "Poison Brand" */, BORG_MAGIC_NOP /*   "Disp Good" */,
		BORG_MAGIC_WHO /*   "Genocide" */, BORG_MAGIC_NOP /*   "Restore Life" */},
	  {/* Black Channels (sval 2) */
		BORG_MAGIC_NOP /* ! "Berserk" */, BORG_MAGIC_ICK /*   "Invoke Spirits" */,
		BORG_MAGIC_AIM /* ! "Dark Bolt" */,
		BORG_MAGIC_NOP /*   "Battle Frenzy" */,
		BORG_MAGIC_AIM /*   "Vamp True" */, BORG_MAGIC_OBJ /*   "Vamp Brand" */,
		BORG_MAGIC_AIM /*   "Dark Storm" */,
		BORG_MAGIC_NOP /*   "Mass Genocide" */},
	  {/* Necronomicon (sval 3) */
		BORG_MAGIC_AIM /* ! "Death Ray" */,
		BORG_MAGIC_ICK /*   "Raise the Dead" */,
		BORG_MAGIC_OBJ /* ! "Esoteria" */, BORG_MAGIC_NOP /*   "Word of Death" */,
		BORG_MAGIC_NOP /*   "Evocation" */, BORG_MAGIC_AIM /*   "Hellfire" */,
		BORG_MAGIC_NOP /*   "Omnicide" */,
		BORG_MAGIC_NOP /*   "Wraithform" */}}, /* end of Death Realm */

	 { /* 6 Trump Realm */
	  {/* Conjuring and Tricks (sval 0) */
		BORG_MAGIC_NOP /* ! "Phase Door" */, BORG_MAGIC_AIM /*   "Mind Blast" */,
		BORG_MAGIC_ICK /*   "Shuffle" */, BORG_MAGIC_ICK /*   "Reset Recall" */,
		BORG_MAGIC_NOP /*   "Teleport Self" */,
		BORG_MAGIC_EXT /*   "Dimension Door" */,
		BORG_MAGIC_NOP /*   "Trump Spying" */,
		BORG_MAGIC_AIM /*   "Teleport Away" */},
	  {/* Deck of Many Things (sval 1) */
		BORG_MAGIC_AIM /* ! "Trump Object" */,
		BORG_MAGIC_NOP /* ! "Trump Animal" */,
		BORG_MAGIC_NOP /*   "Phantasmal Servant" */,
		BORG_MAGIC_NOP /*   "Trump Monster" */,
		BORG_MAGIC_NOP /*   "Conjure Elemental" */,
		BORG_MAGIC_NOP /*   "Teleport Level" */,
		BORG_MAGIC_NOP /*   "Word of Recall" */, BORG_MAGIC_NOP /*   "Banish" */},
	  {/* Trumps of Doom (sval 2) */
		BORG_MAGIC_ICK /* ! "Joker Card" */,
		BORG_MAGIC_NOP /*   "Trump Spiders" */,
		BORG_MAGIC_NOP /*   "T. Reptiles" */, BORG_MAGIC_NOP /*   "T. Hounds" */,
		BORG_MAGIC_OBJ /*   "T. Branding" (Slay Demon) */,
		BORG_MAGIC_NOP /*   "Living Trump" */,
		BORG_MAGIC_NOP /*   "Death Dealing" */,
		BORG_MAGIC_NOP /*   "T. Cyberdemon" */},
	  {/* Five Aces (sval 3) */
		BORG_MAGIC_NOP /* ! "T. Divination" */, BORG_MAGIC_OBJ /*   "T. Lore" */,
		BORG_MAGIC_NOP /*   "T. Undead" */, BORG_MAGIC_NOP /*   "T. Dragon" */,
		BORG_MAGIC_NOP /*   "Mass Trump" */, BORG_MAGIC_NOP /*   "T. Demon" */,
		BORG_MAGIC_NOP /*   "T. Ancient Dragon " */,
		BORG_MAGIC_NOP /*   "T. Greater Undead" */}}, /* end of Trump Realm */

	 { /* 7 Arcane Realm */
	  {/* Cantrips (sval 0) */
		BORG_MAGIC_AIM /* ! "Zap" */, BORG_MAGIC_AIM /*   "Wiz Lock" */,
		BORG_MAGIC_NOP /*   "Det Invis" */, BORG_MAGIC_NOP /*   "Det Mon" */,
		BORG_MAGIC_NOP /*   "Blink" */, BORG_MAGIC_NOP /*   "Light Area" */,
		BORG_MAGIC_AIM /*   "Trap/Door Dest" */,
		BORG_MAGIC_NOP /*   "Cure Light Wounds" */},
	  {/* Minor Arcana (sval 1) */
		BORG_MAGIC_NOP /* ! "Det Door/Trap" */,
		BORG_MAGIC_NOP /* ! "Phlogiston" */,
		BORG_MAGIC_NOP /*   "Det Treasure" */,
		BORG_MAGIC_NOP /*   "Det Enchant" */, BORG_MAGIC_NOP /*   "Det Object" */,
		BORG_MAGIC_NOP /*   "Cure Poison" */,
		BORG_MAGIC_NOP /*   "Resist Cold" */,
		BORG_MAGIC_NOP /*   "Resist Fre" */},
	  {/* Major Arcana (sval 2) */
		BORG_MAGIC_NOP /* ! "Resist Elec" */,
		BORG_MAGIC_NOP /*   "Resist Acid" */,
		BORG_MAGIC_NOP /* ! "Cure Med Wounds" */,
		BORG_MAGIC_NOP /*   "Teleport" */, BORG_MAGIC_AIM /*   "Stone to Mud" */,
		BORG_MAGIC_AIM /*   "Ray of Light" */,
		BORG_MAGIC_NOP /*   "Satisfy Hunger" */,
		BORG_MAGIC_NOP /*   "See Invis" */},
	  {/* Manual of Mastery (sval 3) */
		BORG_MAGIC_OBJ /* ! "Recharge" */,
		BORG_MAGIC_NOP /*   "Teleport Level" */, BORG_MAGIC_OBJ /* ! "Ident" */,
		BORG_MAGIC_AIM /*   "Teleport Away" */,
		BORG_MAGIC_AIM /*   "Elemental Ball" */,
		BORG_MAGIC_NOP /*   "Detection" */,
		BORG_MAGIC_NOP /*   "Word of Recall" */,
		BORG_MAGIC_NOP /*   "Clairvoyance" */}} /* end of Arcane Realm */

};

/*
 * Hack -- help analyze the magic
 *
 * The comments yield the "name" of the spell or prayer.
 *
 * Also, the leading letter in the comment indicates how we use the
 * spell or prayer, if at all, using "A" for "attack", "D" for "call
 * light" and "detection", "E" for "escape", "H" for healing, "O" for
 * "object manipulation", "F" for "terrain feature manipulation",
 * "X" for "never use this", and "!" for "soon to be handled".
 *
 * The value indicates how much we want to know the spell/prayer.  A
 * rating of zero indicates that the spell/prayer is useless, and should
 * never be learned or used.  A rating from 1 to 49 indicates that the
 * spell/prayer is worth some experience to use once, so we should study
 * (and use) it when we get bored in town.  A rating from 50 to 99 means
 * that the spell/prayer should be learned as soon as possible (and used
 * when bored).
 *
 * XXX XXX XXX Verify ratings.
 */

static byte borg_magic_rating[8][4][8] = {
	 { /* Null Realm */
	  {/* Book0... (sval 0) */
		0 /* ! "(blank)" */, 0 /*   "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */},
	  {/* Book1... (sval 1) */
		0 /*   "(blank)" */, 0 /* ! "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */},
	  {/* Book2... (sval 2) */
		0 /*   "(blank)" */, 0 /* ! "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */},
	  {/* Book3... (sval 3) */
		0 /* ! "(blank)" */, 0 /*   "(blank)" */, 0 /* ! "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */, 0 /*   "(blank)" */,
		0 /*   "(blank)" */, 0 /*   "(blank)" */}}, /* end of Null Realm */

	 { /* Life Realm */
	  {/* Beginners Handbook (sval 0) */
		85 /*   "Detect Evil" */, 55 /*   "Cure Light Wounds" */,
		85 /*   "Bless" */, 55 /*   "Remove Fear" */, 85 /*   "Call Light" */,
		75 /*   "Find Traps & Doors" */, 65 /*   "Cure Medium Wounds" */,
		85 /*   "Satisfy Hunger" */},
	  {/* Words of Wisdom (sval 1) */
		65 /*   "Remove Curse" */, 65 /*   "Cure Poison" */,
		85 /*   "Cure Crit Wounds" */, 55 /*   "See Invis" */,
		95 /*   "Holy Orb" */, 85 /*   "Prot/Evil" */, 65 /*   "Heal 300" */,
		55 /*   "Glyph" */},
	  {/* Chants and Blessings (sval 2) */
		65 /*   "Exorcism" */, 65 /*   "Dispel Curse" */,
		55 /*   "Dispel Demon" */, 55 /*   "Day of Dove" */,
		65 /*   "Dispel Evil" */, 55 /*   "Banishment" */, 65 /*   "Holy Word" */,
		55 /*   "Warding True" */},
	  {/* Exorcism and Dispelling (sval 3) */
		55 /*   "Heroism" */, 65 /*   "Prayer" */, 55 /*   "Bless Weapon" */,
		55 /*   "Restoration" */, 65 /*   "Healing 2000" */,
		55 /*   "Holy Vision" */, 55 /*   "Divine Intervent" */,
		55 /*   "Holy Invuln" */}}, /* end of Life Magic */

	 { /* Sorcery Realm */
	  {/* Magic for Beginners (sval 0) */
		95 /*   "Detect Monsters" */, 85 /*   "Phase Door" */,
		65 /*   "Detect Door" */, 85 /*   "Light Area" */,
		75 /*   "Confuse Monster" */, 75 /*   "Teleport Selft" */,
		65 /*   "Sleep Monster" */, 65 /*   "Recharging" */},
	  {/* Conjurings and Tricks (sval 1) */
		55 /*   "Magic Map" */, 85 /*   "Identify" */, 55 /*   "Slow Monster" */,
		65 /*   "Mass Sleep" */, 95 /*   "Teleport Away" */,
		55 /*   "Haste Self" */, 85 /*   "Detection True" */,
		75 /*   "*Identify*" */},
	  {/* Incantations and Illusions (sval 2) */
		55 /*   "Elemental Branding " */, 55 /*   "Detect Enchantment" */,
		75 /*   "Charm Monster" */, 65 /*   "Dimension Door" */,
		65 /*   "Sense Minds" */, 55 /*   "Self Knowledge" */,
		65 /*   "Teleport Level" */, 65 /*   "Word of Recall" */},
	  {/* Sorcery and Evocations (sval 3) */
		55 /*   "Stasis" */, 55 /*   "Telekinesis" */,
		55 /*   "Explosive Rune" */, 65 /*   "Clairvoyance" */,
		55 /*   "Enchant Weap" */, 55 /*   "Enchant Armour" */,
		0 /*   "Alchemy" */, 95 /*   "GOI" */}}, /* end of Sorcery Realm */

	 { /* 3 Nature Realm */
	  {/* Call of the Wild (sval 0) */
		65 /*   "Detect Creature" */, 65 /*   "First Aid" */,
		55 /* ! "Detect Door" */, 75 /*   "Foraging" */, 75 /*   "Daylight" */,
		55 /*   "Animal Taming" */, 75 /*   "Resist Environment" */,
		65 /*   "Cure Wound&Poison" */},
	  {/* Nature Mastery (sval 1) */
		55 /* ! "Stone to Mud" */, 65 /* ! "Lightning Bolt" */,
		65 /*   "Nature Awareness" */, 65 /*   "Frost Bolt" */,
		65 /*   "Ray of Sunlight" */, 65 /*   "Entangle" */,
		65 /*   "Summon Animals" */, 65 /*   "Herbal Healing" */},
	  {/* Nature Gifts (sval 2) */
		65 /* ! "Door Building" */, 45 /*   "Stair Building" */,
		65 /* ! "Stone Skin" */, 65 /*   "Resistance True" */,
		55 /*   "Animal Friend" */, 65 /*   "Stone Tell" */,
		45 /*   "Wall of Stone" */, 45 /*   "Protect From Corros." */},
	  {/* Natures Wrath (sval 3) */
		65 /* ! "Earthquake" */, 65 /*   "Whirlwind" */, 65 /* ! "Blizzard" */,
		65 /*   "Lightning" */, 65 /*   "Whirpool" */, 65 /*   "Call Sunlight" */,
		45 /*   "Elemental Brand" */,
		65 /*   "Natures Wrath" */}}, /* end of Natural realm  */

	 { /* 4.Chaos Realm */
	  {/* Sign of Chaos... (sval 0) */
		95 /* "Magic Missile" */, 65 /* "Trap/Door Dest" */,
		75 /* "Flash of Light" */, 55 /* "Touch of Conf" */, 65 /* "ManaBurst" */,
		65 /* "Fire Bolt" */, 65 /* "Fist of Force" */, 75 /* "Teleport" */},
	  {/* Chaos Mastery... (sval 1) */
		5 /*   "Wonder" */, 65 /*   "Chaos Bolt" */, 65 /*   "Sonic Boom" */,
		65 /*   "Doom Beam" */, 65 /*   "Fire Ball" */,
		65 /*   "Teleport Other" */, 65 /*   "Word of Dest" */,
		55 /*   "Invoke Logrus" */},
	  {/* Chaos Channels (sval 2) */
		45 /*   "Polymorph Other" */, 65 /*   "Chain Lightn" */,
		65 /*   "Arcane Binding" */, 65 /*   "Disintegration" */,
		55 /*   "Alter Reality" */, 5 /*   "Polymorph Self" */,
		55 /*   "Chaos Binding" */, 55 /*   "Summon Demon" */},
	  {/* Armageddon Tome (sval 3) */
		65 /*   "Gravity Beam" */, 65 /*   "Meteor Swarm" */,
		65 /*   "Flame Strike" */, 65 /*   "Call Chaos" */,
		75 /*   "Magic Rocket" */, 75 /*   "Mana Storm" */,
		65 /*   "Breath Logrus" */,
		65 /*   "Call Void" */}}, /* end of Chaos Realm*/

	 { /* 5. Death Realm */
	  {/* Black Prayers (sval 0) */
		65 /* ! "Detect Unlife" */, 75 /*   "Maledition" */,
		75 /* ! "Detect Evil" */, 75 /*   "Stinking Cloud" */,
		65 /*   "Black Sleep" */, 65 /*   "Resist Poison" */,
		65 /*   "Horrify" */, 65 /*   "Enslave Undead" */},
	  {/* Black Mass (sval 1) */
		70 /* ! "Orb of Entropy" */, 65 /*   "Nether Bolt" */,
		50 /*   "Terror" */, 65 /*   "Vamp Drain" */, 55 /*   "Poison Brand" */,
		65 /*   "Disp Good" */, 65 /*   "Genocide" */, 65 /*   "Restore Life" */},
	  {/* Black Channels (sval 2) */
		65 /* ! "Berserk" */, 65 /*   "Invoke Spirits" */, 65 /* ! "Dark Bolt" */,
		85 /*   "Battle Frenzy" */, 65 /*   "Vamp True" */,
		65 /*   "Vamp Brand" */, 65 /*   "Dark Storm" */,
		65 /*   "Mass Genocide" */},
	  {/* Necronomicon (sval 3) */
		65 /* ! "Death Ray" */, 65 /*   "Raise the Dead" */,
		75 /* ! "Esoteria" */, 65 /*   "Word of Death" */, 65 /*   "Evocation" */,
		65 /*   "Hellfire" */, 65 /*   "Omnicide" */,
		55 /*   "Wraithform" */}}, /* end of Death Realm */

	 { /* Trump Realm */
	  {/* Trump Magic (sval 0) */
		95 /* ! "Phase Door" */, 85 /* ! "Mind Blast " */, 0 /*   "Shuffle" */,
		0 /*   "Reset Recall" */, 75 /*   "Telport Self" */,
		65 /*   "Dimension Door " */, 65 /*   "Trump Spying " */,
		70 /*   "Teleport Away " */},
	  {/* Deck of Many Things (sval 1) */
		55 /* ! "Trump Object " */, 85 /* ! "Trump animal " */,
		85 /*   "Phantasmal Servant " */, 55 /*   "Trump Monster " */,
		85 /*   "Conjure Elemental " */, 50 /*   "Teleport Level " */,
		65 /*   "Word of recall " */, 65 /*   "Banishment" */},
	  {/* Trump of Doom (sval 2) */
		0 /* ! "Joker Card " */, 0 /*   "Trump Spiders " */,
		0 /*   "Trump Reptiles " */, 0 /*   "Trump Hounds " */,
		85 /*   "Trump Branding " */, 85 /*   "Living Trump " */,
		85 /*   "Death Dealing " */, 85 /*   "Trump Cyberdemon " */},
	  {/* Five Aces (sval 3) */
		85 /* ! "Trump Divination " */, 85 /*   "Trump Lore " */,
		85 /*   "Trump Undead " */, 85 /*   "Trump Dragon " */,
		85 /*   "Mass Trump " */, 85 /*   "Trump Demon " */,
		85 /*   "Trump Ancient Dragon " */,
		85 /*   "Trump Greater Undead " */}}, /* end of Trump Realm */

	 { /* 7 Arcane Realm */
	  {/* Cantrips (sval 0) */
		95 /* ! "Zap" */, 55 /*   "Wiz Lock" */, 75 /*   "Det Invis" */,
		75 /*   "Det Mon" */, 85 /*   "Blink" */, 75 /*   "Light Area" */,
		65 /*   "Trap/Door Dest" */, 75 /*   "Cure Light Wounds" */},
	  {/* Minor Arcana (sval 1) */
		75 /* ! "Det Door/Trap" */, 75 /* ! "Phlogiston" */,
		75 /*   "Det Treasure" */, 75 /*   "Det Enchant" */,
		75 /*   "Det Object" */, 75 /*   "Cure Poison" */,
		75 /*   "Resist Cold" */, 75 /*   "Resist Fre" */},
	  {/* Major Arcana (sval 2) */
		75 /* ! "Resist Elec" */, 75 /*   "Resist Acid" */,
		75 /* ! "Cure Med Wounds" */, 75 /*   "Teleport" */,
		85 /*   "Stone to Mud" */, 85 /*   "Ray of Light" */,
		75 /*   "Satisfy Hunger" */, 75 /*   "See Invis" */},
	  {/* Manual of Mastery (sval 3) */
		75 /* ! "Recharge" */, 75 /*   "Teleport Level" */, 85 /* ! "Ident" */,
		85 /*   "Teleport Away" */, 70 /*   "Elemental Ball" */,
		75 /*   "Detection" */, 75 /*   "Word of Recall" */,
		75 /*   "Clairvoyance" */}} /* end of Arcane Realm */

};

/*
 * Constant "item description parsers" (singles)
 */
static int borg_single_size;	/* Number of "singles" */
static s16b *borg_single_what; /* Kind indexes for "singles" */
static cptr *borg_single_text; /* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int borg_plural_size;		 /* Number of "plurals" */
static s16b *borg_plural_what;	 /* Kind index for "plurals" */
static cptr *borg_plural_text;	 /* Textual prefixes for "plurals" */
static cptr *borg_sv_plural_text; /* Save Textual prefixes for "plurals" (in
												 kind order) */

/*
 * Constant "item description parsers" (suffixes)
 */
static int borg_artego_size;	/* Number of "artegos" */
static s16b *borg_artego_what; /* Indexes for "artegos" */
static cptr *borg_artego_text; /* Textual prefixes for "artegos" */
static cptr
	 *borg_sv_art_text; /* Save textual prefixes for "artifacts" (in kind order)
								  */

/*
 * Return the slot that items of the given type are wielded into
 *
 * Note that "rings" are tough because there are two slots
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 */
int borg_wield_slot(borg_item *item) {
	if ((item->tval == TV_SWORD) || (item->tval == TV_POLEARM) ||
		 (item->tval == TV_HAFTED) || (item->tval == TV_DIGGING))
		return (INVEN_WIELD);

	if ((item->tval == TV_DRAG_ARMOR) || (item->tval == TV_HARD_ARMOR) ||
		 (item->tval == TV_SOFT_ARMOR))
		return (INVEN_BODY);

	if (item->tval == TV_SHIELD)
		return (INVEN_ARM);

	if ((item->tval == TV_CROWN) || (item->tval == TV_HELM))
		return (INVEN_HEAD);

	if (item->tval == TV_BOW)
		return (INVEN_BOW);

	if (item->tval == TV_RING)
		return (INVEN_LEFT);

	if (item->tval == TV_AMULET)
		return (INVEN_NECK);

	if (item->tval == TV_LITE)
		return (INVEN_LITE);

	if (item->tval == TV_CLOAK)
		return (INVEN_OUTER);

	if (item->tval == TV_GLOVES)
		return (INVEN_HANDS);

	if (item->tval == TV_BOOTS)
		return (INVEN_FEET);

	/* No slot available */
	return (-1);
}

/*
 * Return the slot that items of the given type are wielded into
 *
 * Note that "rings" are tough because there are two slots
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 */
int borg_wield_slot_take(borg_take *take) {
	if ((take->tval == TV_SWORD) || (take->tval == TV_POLEARM) ||
		 (take->tval == TV_HAFTED) || (take->tval == TV_DIGGING))
		return (INVEN_WIELD);

	if ((take->tval == TV_DRAG_ARMOR) || (take->tval == TV_HARD_ARMOR) ||
		 (take->tval == TV_SOFT_ARMOR))
		return (INVEN_BODY);

	if (take->tval == TV_SHIELD)
		return (INVEN_ARM);

	if ((take->tval == TV_CROWN) || (take->tval == TV_HELM))
		return (INVEN_HEAD);

	if (take->tval == TV_BOW)
		return (INVEN_BOW);

	if (take->tval == TV_RING)
		return (INVEN_LEFT);

	if (take->tval == TV_AMULET)
		return (INVEN_NECK);

	if (take->tval == TV_LITE)
		return (INVEN_LITE);

	if (take->tval == TV_CLOAK)
		return (INVEN_OUTER);

	if (take->tval == TV_GLOVES)
		return (INVEN_HANDS);

	if (take->tval == TV_BOOTS)
		return (INVEN_FEET);

	/* No slot available */
	return (-1);
}

/*
 * Get the *ID information
 *
 * cheats information from the screen if it is not passed
 * a *real* item.  It is only passed in *real* items if the borg is allowed
 * to 'cheat' for inventory.
 * This function returns TRUE if space needs to be pressed
 */
bool borg_object_star_id_aux(borg_item *borg_item, object_type *real_item) {
	u32b f1, f2, f3;

	/* If a real item pointer is passed in then we are cheating to get */
	/* the data directly from the real item    */
	if (real_item) {
		object_flags(real_item, &f1, &f2, &f3);
	}

	borg_item->flags1 = f1;
	borg_item->flags2 = f2;
	borg_item->flags3 = f3;

	borg_item->needs_I_exam = FALSE;

	return (FALSE);
}

/*
 * Look for an item that needs to be analysed because it has been *ID*d
 *
 * This will go through inventory and look for items that were just*ID*'d
 * and examine them for their bonuses.
 */
bool borg_object_star_id(void) {
	int i;

	bool boring = TRUE;

	/* look in inventory and equiptment for something to *id* */
	for (i = 0; i < INVEN_TOTAL; i++) {

		borg_item *item = &borg_items[i];

		if (borg_items[i].needs_I_exam || item->quest ||
			 (!borg_items[i].flags1 || !borg_items[i].flags2 ||
			  !borg_items[i].flags3)) {
			if (!item->quest) {
				/* cheat to get the information. */
				borg_object_star_id_aux(&borg_items[i], &inventory[i]);
			}

			/* inscribe certain objects */
			if (!item->timeout &&
				 (item->name1 || item->name2 || item->name3 || item->quest) &&
				 strlen(item->desc) <
					  65 && /* some items have intrinsic names which are too long */
				 (item->note == NULL || streq(item->note, ".") ||
				  streq(item->note, " ") || streq(item->note, "") ||
				  strstr(item->note, "cursed") || strstr(item->note, "uncursed") ||
				  (item->fully_identified && strstr(item->note, "special")))) {

				/* make the inscription */
				borg_keypress('{');

				if (i >= INVEN_WIELD) {
					borg_keypress('/');
					borg_keypress(I2A(i - INVEN_WIELD));
				} else {
					borg_keypress(I2A(i));
				}

				/* make the inscription */
				if (item->quest && item->tval >= TV_SHOT && item->tval <= TV_RING) {
					borg_keypresses("Quest");
					boring = FALSE;
				}
				if (item->flags1 & TR1_SPEED) {
					borg_keypresses("Spd");
					boring = FALSE;
				}
				/* slays and immunities */
				if (item->flags2 & TR2_RES_POIS) {
					borg_keypresses("Poisn");
					boring = FALSE;
				}
				if (item->flags2 & TR2_IM_FIRE) {
					borg_keypresses("IFir");
					boring = FALSE;
				}
				if (item->flags2 & TR2_IM_COLD) {
					borg_keypresses("ICld");
					boring = FALSE;
				}
				if (item->flags2 & TR2_IM_ACID) {
					borg_keypresses("IAcd");
					boring = FALSE;
				}
				if (item->flags2 & TR2_IM_ELEC) {
					borg_keypresses("IElc");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_LITE) {
					borg_keypresses("Lite");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_DARK) {
					borg_keypresses("Dark");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_BLIND) {
					borg_keypresses("Blnd");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_CONF) {
					borg_keypresses("Conf");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_SOUND) {
					borg_keypresses("Sound");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_SHARDS) {
					borg_keypresses("Shrd");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_NETHER) {
					borg_keypresses("Nthr");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_NEXUS) {
					borg_keypresses("Nxs");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_CHAOS) {
					borg_keypresses("Chaos");
					boring = FALSE;
				}
				if (item->flags2 & TR2_RES_DISEN) {
					borg_keypresses("Disn");
					boring = FALSE;
				}
				if (item->flags3 & TR3_ACTIVATE) {
					borg_keypresses("Actv");
					boring = FALSE;
				}
				if (item->flags3 & TR3_TELEPATHY) {
					borg_keypresses("ESP");
					boring = FALSE;
				}
				if (item->flags2 & TR2_HOLD_LIFE) {
					borg_keypresses("HL");
					boring = FALSE;
				}
				if (item->flags2 & TR2_FREE_ACT) {
					borg_keypresses("FA");
					boring = FALSE;
				}
				if (item->flags3 & TR3_SEE_INVIS) {
					borg_keypresses("SInv");
					boring = FALSE;
				}
				if (item->flags3 & TR3_TELEPORT) {
					borg_keypresses(".");
					boring = FALSE;
				}
				if (item->flags3 & TR3_WRAITH) {
					borg_keypresses("Wraith");
				}
				if (boring == TRUE) {
					borg_keypresses("Boring");
				}

				/* end the inscription */
				borg_keypress('\n');
			}
		}
	}
	return (FALSE);
}

/*
 * Determine the "base price" of a known item (see below)
 *
 * This function is adapted from "object_value_known()".
 *
 * This routine is called only by "borg_item_analyze()", which
 * uses this function to guess at the "value" of an item, if it
 * was to be sold to a store, with perfect "charisma" modifiers.
 */
static s32b borg_object_value_known(borg_item *item) {
	s32b value;

	object_kind *k_ptr = &k_info[item->kind];

	/* Worthless items */
	if (!k_ptr->cost)
		return (0L);

	/* Extract the base value */
	value = k_ptr->cost;

	/* Hack -- use artifact base costs */
	if (item->name1) {
		if (item->name1 != ART_RANDART) {
			artefact_type *a_ptr = &a_info[item->name1];
			/* Worthless artifacts */
			if (!a_ptr->cost)
				return (0L);

			/* Hack -- use the artifact cost */
			value = a_ptr->cost;
		}
	}

	/* Hack -- add in ego-item bonus cost */
	if (item->name2) {
		ego_item_type *e_ptr = &e_info[item->name2];

		/* Worthless ego-items */
		if (!e_ptr->cost)
			return (0L);

		/* Hack -- reward the ego-item cost */
		value += e_ptr->cost;
	}

	/* Analyze pval bonus */
	switch (item->tval) {
	/* Wands/Staffs */
	case TV_WAND:
	case TV_STAFF: {
		/* Pay extra for charges */
		value += ((value / 20) * item->pval);

		break;
	}

	/* Wearable items */
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
	case TV_LITE:
	case TV_AMULET:
	case TV_RING: {
		/* Hack -- Negative "pval" is always bad */
		if (item->pval < 0)
			return (0L);

		/* No pval */
		if (!item->pval)
			break;

		/* Give credit for stat bonuses */
		if (item->flags1 & TR1_STR)
			value += (item->pval * 200L);
		if (item->flags1 & TR1_INT)
			value += (item->pval * 200L);
		if (item->flags1 & TR1_WIS)
			value += (item->pval * 200L);
		if (item->flags1 & TR1_DEX)
			value += (item->pval * 200L);
		if (item->flags1 & TR1_CON)
			value += (item->pval * 200L);
		if (item->flags1 & TR1_CHA)
			value += (item->pval * 200L);

		/* Give credit for stealth and searching */
		if (item->flags1 & TR1_STEALTH)
			value += (item->pval * 100L);
		if (item->flags1 & TR1_SEARCH)
			value += (item->pval * 100L);

		/* Give credit for infra-vision and tunneling */
		if (item->flags1 & TR1_INFRA)
			value += (item->pval * 50L);
		if (item->flags1 & TR1_TUNNEL)
			value += (item->pval * 50L);

		/* Give credit for extra attacks */
		if (item->flags1 & TR1_BLOWS)
			value += (item->pval * 2000L);

		/* Give credit for speed bonus */
		if (item->flags1 & TR1_SPEED)
			value += (item->pval * 30000L);

		break;
	}
	}

	/* Analyze the item */
	switch (item->tval) {
	/* Rings/Amulets */
	case TV_RING:
	case TV_AMULET: {
		/* Hack -- negative bonuses are bad */
		if (item->to_a < 0)
			return (0L);
		if (item->to_h < 0)
			return (0L);
		if (item->to_d < 0)
			return (0L);

		/* Give credit for bonuses */
		value += ((item->to_h + item->to_d + item->to_a) * 100L);

		break;
	}

	/* Armor */
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CLOAK:
	case TV_CROWN:
	case TV_HELM:
	case TV_SHIELD:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR: {
		/* Hack -- negative armor bonus */
		if (item->to_a < 0)
			return (0L);

		/* Give credit for bonuses */
		value += ((item->to_h + item->to_d + item->to_a) * 100L);

		break;
	}

	/* Bows/Weapons */
	case TV_BOW:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_SWORD:
	case TV_POLEARM: {
		/* Hack -- negative hit/damage bonuses */
		if (item->to_h + item->to_d < 0)
			return (0L);

		/* Factor in the bonuses */
		value += ((item->to_h + item->to_d + item->to_a) * 100L);

		/* Hack -- Factor in extra damage dice */
		if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
			value += (item->dd - k_ptr->dd) * item->ds * 200L;
		}

		break;
	}

	/* Ammo */
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT: {
		/* Hack -- negative hit/damage bonuses */
		if (item->to_h + item->to_d < 0)
			return (0L);

		/* Factor in the bonuses */
		value += ((item->to_h + item->to_d) * 5L);

		/* Hack -- Factor in extra damage dice */
		if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
			value += (item->dd - k_ptr->dd) * item->ds * 5L;
		}

		break;
	}
	}

	/* Return the value */
	return (value);
}

/*
 * Analyze an item given a description and (optional) cost
 *
 * From the description, extract the item identity, and the various
 * bonuses, plus the "aware" and "known" flags (in an encoded state).
 *
 * Note the use of a "prefix binary search" on the arrays of object
 * base names, and on the arrays of artifact/ego-item special names.
 *
 * The Vanilla borg does not cheat here, this borg does cheat.
 */
void borg_item_analyze(borg_item *item, object_type *real_item, cptr desc,
							  int location) {

	u32b f1, f2, f3;
	int i;

	char *scan;

	/* Wipe the item */
	WIPE(item, borg_item);

	/* Extract the flags */
	object_flags(real_item, &f1, &f2, &f3);

	/* Save the item description */
	strcpy(item->desc, desc);

	/* Advance to the "inscription" or end of string */
	for (scan = item->desc; *scan && (*scan != c1); scan++) /* loop */
		;

	/* Save a pointer to the inscription */
	item->note = scan;

	/* Empty item */
	if (!desc[0])
		return;
	if (strstr(desc, "(nothing)"))
		return;

	/* Cheat a pointer to the inscription.
	 * The game drops the inscription on very long-named items
	 */
	if (quark_str(real_item->note)) {
		item->note = quark__str[real_item->note];
	}

	/* Need the pseudo-id inscript too */
	/* psuedo-id is checked by strstr(item->desc */

	/* Quantity of item */
	item->iqty = real_item->number;

	/* empty item, leave here */
	if (item->iqty == 0)
		return;

	/* TVal and SVal */
	item->tval = real_item->tval;

	/* This is only known on some items if not id'd/known */
	item->sval = real_item->sval;

	/* Some Sense but not necessarily real ID
	 * some (Easy Know + Aware) items might sneak in here.
	 */
	item->aware = object_known_p(real_item);
	if (!item->aware) {
		if (object_aware_p(real_item))
			item->kind = real_item->k_idx;

		/* item has some awareness */
		if (item->kind)
			item->aware = TRUE;
	}

	/* Item has been ID'd (store, scroll, spell) */
	if ((real_item->ident & IDENT_KNOWN) || (real_item->ident & IDENT_STOREB) ||
		 (item->aware && (f3 & TR3_EASY_KNOW)) ||
		 (item->tval == TV_SCROLL && item->aware) ||
		 /* TODO have a super type value, use it here */
		 ((item->tval >= TV_MIRACLES_BOOK && item->tval <= TV_DEMONIC_BOOK) &&
		  item->aware) ||
		 (item->tval == TV_ROD && item->aware) ||
		 (item->tval == TV_POTION && item->aware) ||
		 (item->tval == TV_FOOD && item->aware))
		item->ident = TRUE;

	/* Item has been *ID*'d (store, scroll, spell) */
	if ((real_item->ident & IDENT_STOREB) || (real_item->ident & IDENT_MENTAL))
		item->fully_identified = TRUE;

	/* Kind index -- Only if partially ID*/
	if (item->aware)
		item->kind = real_item->k_idx;

	/* power value -- Only if ID'd */
	if (item->ident)
		item->pval = real_item->pval;

	/* Rods are considered pval 1 if charged */
	if (item->tval == TV_ROD) {
		/*char *buf;*/

		/*k_ptr = &k_info[real_item->k_idx];*/

		if (item->iqty == 1 && real_item->timeout)
			item->pval = 0;
		else {
			if (strstr(item->desc, "charging")) {
				cptr s;
				int number;

				/* Assume all are charging */
				item->pval = 0;

				/* Find the first "(" */
				for (s = desc; *s && (*s != '('); s++) /* loop */
					;

				desc = s + 1;

				if (isdigit(desc[0])) {
					cptr ss;

					/* Find the first space */
					for (ss = desc; *ss && (*ss != ' '); ss++) /* loop */
						;

					/* Paranoia -- Catch sillyness */
					if (*ss != ' ') {
						number = item->iqty;
					} else {
						/* Extract a quantity */
						number = atoi(desc);
					}

					/* Quantity more than number charging */
					if (item->iqty > number) {
						item->pval = item->iqty - number;
					}
				}
			} else
				item->pval = item->iqty;
		}
	}

	/* Staves and Wands are considered charged unless
	 * they are known to be empty or are {empty}
	 */
	if (item->tval == TV_STAFF || item->tval == TV_WAND) {
		/* assume good */
		item->pval = 1;

		/* if Known, get correct pval */
		if (item->ident || strstr(item->desc, "charges"))
			item->pval = real_item->pval;
		if (item->ident || strstr(item->desc, "x"))
			item->pval = real_item->pval * item->iqty;

		/* Gotta know charges */
		/* if (!object_known_p(real_item)) item->pval = 0; */

		/* No inscription, */
		if (!strstr(item->desc, "charge")) {
			/* assume one charge */
			item->pval = 1;

			/* Aware but not ID;d */
			item->ident = FALSE;
			item->aware = TRUE;
		}

		/* if seen {empty} assume pval 0 */
		if (real_item->ident & IDENT_EMPTY)
			item->pval = 0;
		if (strstr(item->note, "empty"))
			item->pval = 0;
	}

	/* Phial is always lit, even unid'd one. */
	if (item->kind == 500 && !item->ident)
		item->pval = 1;

	/* Weight of item */
	item->weight = real_item->weight;

	/* Index known if ID'd */
	if (item->ident) {
		/* Artifact Index --Only known if ID'd*/
		item->name1 = real_item->name1;

		/* Ego Index --Only known if ID'd*/
		item->name2 = real_item->name2;

		/* Artifact Index --Only known if ID'd*/
		if (real_item->art_name)
			item->name1 = ART_RANDART;
	}

	/* Timeout, must wait for recharge */
	item->timeout = real_item->timeout;

	/* Modifiers -- Only known if ID'd */
	if (item->ident) {
		item->to_h = real_item->to_h; /* Bonus to hit */
		item->to_d = real_item->to_d; /* Bonus to dam */
		item->to_a = real_item->to_a; /* Bonus to ac */
	}
	item->ac = real_item->ac; /* Armor class */
	item->dd = real_item->dd; /* Damage dice */
	item->ds = real_item->ds; /* Damage sides */

	/* Level of item */
	item->level = k_info[item->kind].level;

	/* Extract the base flags -- Kind only given if 'able' */
	item->flags1 = k_info[item->kind].flags1;
	item->flags2 = k_info[item->kind].flags2;
	item->flags3 = k_info[item->kind].flags3;

	/* Extract the added flags (like from Bless) */
	if (f3 & TR3_BLESSED) {
		/* Add bless flag */
		item->flags3 |= TR3_BLESSED;
	}

	/* Base Cost -- Guess */

	/* Aware items */
	if (item->aware) {
		/* Aware items can assume template cost */
		item->value = k_info[item->kind].cost;
	}
	/* Known items */
	if (item->ident) {
		/* Process various fields */
		item->value = borg_object_value_known(item);
	}

	/* No known price on non-aware  item */
	if (!item->aware && !item->value) {
		/* Guess at weight and cost */
		switch (item->tval) {
		case TV_SKELETON:
			item->value = 0L;
			break;
		case TV_FOOD: {
			item->value = 5L;
			break;
		}
		case TV_POTION: {
			item->value = 20L;
			break;
		}
		case TV_SCROLL: {
			item->value = 20L;
			break;
		}
		case TV_STAFF: {
			item->value = 70L;
			break;
		}
		case TV_WAND: {
			item->value = 50L;
			break;
		}
		case TV_ROD: {
			item->value = 90L;
			break;
		}
		case TV_RING: {
			item->value = 45L;
			break;
		}
		case TV_AMULET: {
			item->value = 45L;
			break;
		}
		}
	}

	/* Try to set the Quest Item flag correctly the first time the item is
	 * anayzed. */
	if (strstr(item->note, "Quest"))
		item->quest = TRUE;
	else {
		for (i = 0; i <= good_obj_num; i++) {
			if (good_obj_tval[i] == item->tval && good_obj_sval[i] == item->sval &&
				 distance(good_obj_y[i], good_obj_x[i], c_y, c_x) <= 3 &&
				 item->iqty >= 1 && location < INVEN_WIELD) {
				item->quest = TRUE;
				good_obj_x[i] = 0;
				good_obj_y[i] = 0;
				good_obj_tval[i] = 0;
				good_obj_sval[i] = 0;
			}
		}
	}

	/* Item is cursed */
	if (item->ident || strstr(item->note, "cursed")) {
		item->cursed = cursed_p(real_item);
	}

	/* Reduce value of cursed item */
	if (item->cursed)
		item->value = 0;

	/* Hack -- examine artifacts */
	if (item->name1 && item->name1 != ART_RANDART) {
		/* XXX XXX Hack -- fix "weird" artifacts */
		if ((item->tval != a_info[item->name1].tval) ||
			 (item->sval != a_info[item->name1].sval)) {
			/* Save the kind */
			item->kind = lookup_kind(item->tval, item->sval);

			/* Save the tval/sval */
			item->tval = k_info[item->kind].tval;
			item->sval = k_info[item->kind].sval;
		}

		/* Extract the weight */
		item->weight = a_info[item->name1].weight;

		/* Extract the artifact flags */
		item->flags1 = a_info[item->name1].flags1;
		item->flags2 = a_info[item->name1].flags2;
		item->flags3 = a_info[item->name1].flags3;
	}

	/* Check the Randart item flags. */
	if (item->name1 && item->name1 == ART_RANDART) {
		/* Extract the artifact flags.
 * This is not cheating.  If I know the name of the randart, then I know it's
 * proporties.
 */
		item->flags1 = real_item->art_flags1;
		item->flags2 = real_item->art_flags2;
		item->flags3 = real_item->art_flags3;
		item->xtra2 = real_item->xtra2;
	}

	/* Hack -- examine ego-items */
	if (item->name2) {
		/* XXX Extract the weight */

		/* Extract the ego-item flags */
		item->flags1 |= e_info[item->name2].flags1;
		item->flags2 |= e_info[item->name2].flags2;
		item->flags3 |= e_info[item->name2].flags3;
	}

	/* Hack -- examine temp branded-items */
	/*TODO: truly work on temporary brands*/

	/* Special "discount" */
	item->discount = real_item->discount;

	/* Cursed indicators */
	if (strstr(item->note, "cursed"))
		item->value = 0L;
	else if (strstr(item->note, "broken"))
		item->value = 0L;
	else if (strstr(item->note, "terrible"))
		item->value = 0L;
	else if (strstr(item->note, "worthless"))
		item->value = 0L;

	/* Hack -- repair the One Ring, which in Hellband is the ring of the Lammasu
	 */
	if (item->name1 == ART_LAMMASU ||
		 (item->tval == TV_RING && strstr(item->note, "special") &&
		  strstr(item->note, "cursed")))
		item->value = 999999;

	/* Ignore certain feelings */
	/* "{average}" */
	/* "{blessed}" */
	/* "{good}" */
	/* "{excellent}" */
	/* "{special}" */

	/* Ignore special inscriptions */
	/* "{empty}", "{tried}" */
}

/*
 * Send a command to inscribe item number "i" with the inscription "str".
 */
void borg_send_inscribe(int i, cptr str) {
	cptr s;

	/* Label it */
	borg_keypress(c1);

	/* Choose from inventory */
	if (i < INVEN_WIELD) {
		/* Choose the item */
		borg_keypress(I2A(i));
	}

	/* Choose from equipment */
	else {
		/* Go to equipment (if necessary) */
		if (borg_items[0].iqty)
			borg_keypress('/');

		/* Choose the item */
		borg_keypress(I2A(i - INVEN_WIELD));
	}

	/* Send the label */
	for (s = str; *s; s++)
		borg_keypress(*s);

	/* End the inscription */
	borg_keypress('\n');
}

/*
 * Find the slot of an item with the given tval/sval, if available.
 * Given multiple choices, choose the item with the largest "pval".
 * Given multiple choices, choose the smallest available pile.
 */
int borg_slot(int tval, int sval) {
	int i, n = -1;

	/* Scan the pack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip un-aware items */
		if (!item->kind)
			continue;

		/* Require correct tval */
		if (item->tval != tval)
			continue;

		/* Require correct sval */
		if (item->sval != sval)
			continue;

		/* Prefer largest "pval" */
		if ((n >= 0) && (item->pval < borg_items[n].pval))
			continue;

		/* Prefer smallest pile */
		if ((n >= 0) && (item->iqty > borg_items[n].iqty))
			continue;

		/* Save this item */
		n = i;
	}

	/* Done */
	return (n);
}

/*
 * Hack -- refuel a torch
 */
bool borg_refuel_torch(void) {
	int i;

	/* Look for a torch */
	i = borg_slot(TV_LITE, SV_LITE_TORCH);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* must first wield before one can refuel */
	if (borg_items[INVEN_LITE].sval != SV_LITE_TORCH) {
		return (FALSE);
	}

	/* Dont bother with empty */
	if (borg_items[i].pval == 0) {
		return (FALSE);
	}

	/* Cant refuel nothing */
	if (borg_items[INVEN_LITE].iqty == 0) {
		return (FALSE);
	}

	/* Log the message */
	borg_note(format("# Refueling with %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('F');
	borg_keypress(I2A(i));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- refuel a lantern
 */
bool borg_refuel_lantern(void) {
	int i = -1;

	/* Look for a Lantern */
	i = borg_slot(TV_LITE, SV_LITE_LANTERN);

	/* Look for a torch */
	if (i == -1)
		i = borg_slot(TV_FLASK, 0);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* Cant refuel a torch with oil */
	if (borg_items[INVEN_LITE].sval != SV_LITE_LANTERN) {
		return (FALSE);
	}

	/* Log the message */
	borg_note(format("# Refueling with %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('F');
	borg_keypress(I2A(i));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to eat the given food (by sval)
 */
bool borg_eat_food(int sval) {
	int i;

	/* Look for that food */
	i = borg_slot(TV_FOOD, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Eating %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('E');
	borg_keypress(I2A(i));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Quaff a potion of cure critical wounds.  This is a special case
 *   for several reasons.
 *   1) it is usually the only healing potion we have on us
 *   2) we should try to conserve a couple for when we really need them
 *   3) if we are burning through them fast we should probably teleport out of
 *      the fight.
 *   4) When it is the only/best way out of danger, drink away
	*/
bool borg_quaff_crit(bool no_check) {
	static s16b when_last_quaff = 0;

	if (no_check) {
		if (borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
			 borg_quaff_potion(SV_POTION_CURING)) {
			when_last_quaff = borg_t;
			return (TRUE);
		}
		return (FALSE);
	}

	/* Save the last two for when we really need them */
	if (borg_skill[BI_ACCW] < 2)
		return FALSE;

	/* Avoid drinking CCW twice in a row */
	if (when_last_quaff > (borg_t - 4) && when_last_quaff <= borg_t &&
		 (rand_int(100) < 75))
		return FALSE;

	if (borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
		 borg_quaff_potion(SV_POTION_CURING)) {
		when_last_quaff = borg_t;
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Hack -- attempt to quaff the given potion (by sval)
 */
bool borg_quaff_potion(int sval) {
	int i;

	/* Look for that potion */
	i = borg_slot(TV_POTION, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* Safety check */
	if (sval <= SV_POTION_INFRAVISION)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Quaffing %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('q');
	borg_keypress(I2A(i));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}
/*
 * Hack -- attempt to quaff an unknown potion
 */
bool borg_quaff_unknown(void) {
	int i, n = -1;

	/* Scan the pack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require correct tval */
		if (item->tval != TV_POTION)
			continue;

		/* Skip aware items */
		if (item->kind)
			continue;

		/* Save this item */
		n = i;
	}

	/* None available */
	if (n < 0)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Quaffing unknown potion %s.", borg_items[n].desc));

	/* Perform the action */
	borg_keypress('q');
	borg_keypress(I2A(n));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to read an unknown scroll
 */
bool borg_read_unknown(void) {
	int i, n = -1;

	/* Scan the pack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require correct tval */
		if (item->tval != TV_SCROLL)
			continue;

		/* Skip aware items */
		if (item->kind)
			continue;

		/* Save this item */
		n = i;
	}

	/* None available */
	if (n < 0)
		return (FALSE);

	/* Not when dark */
	if (no_lite())
		return (FALSE);

	/* Blind or Confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Log the message */
	borg_note(format("# Reading unknown scroll %s.", borg_items[n].desc));

	/* Perform the action */
	borg_keypress('r');
	borg_keypress(I2A(n));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to eat an unknown potion.  This is done in emergencies.
 */
bool borg_eat_unknown(void) {
	int i, n = -1;

	/* Scan the pack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require correct tval */
		if (item->tval != TV_FOOD)
			continue;

		/* Skip aware items */
		if (item->kind)
			continue;

		/* Save this item */
		n = i;
	}

	/* None available */
	if (n < 0)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Eating unknown mushroom %s.", borg_items[n].desc));

	/* Perform the action */
	borg_keypress('E');
	borg_keypress(I2A(n));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to use an unknown staff.  This is done in emergencies.
 */
bool borg_use_unknown(void) {
	int i, n = -1;

	/* Scan the pack */
	for (i = 0; i < INVEN_PACK; i++) {
		borg_item *item = &borg_items[i];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require correct tval */
		if (item->tval != TV_STAFF)
			continue;

		/* Skip aware items */
		if (item->kind)
			continue;

		/* Save this item */
		n = i;
	}

	/* None available */
	if (n < 0)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Using unknown Staff %s.", borg_items[n].desc));

	/* Perform the action */
	borg_keypress('u');
	borg_keypress(I2A(n));

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to read the given scroll (by sval)
 */
bool borg_read_scroll(int sval) {
	int i;

	/* Dark */
	if (no_lite())
		return (FALSE);

	/* Blind or Confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* Look for that scroll */
	i = borg_slot(TV_SCROLL, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Reading %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('r');
	borg_keypress(I2A(i));

	/* Hack -- Clear "shop" goals */
	goal_shop = goal_ware = goal_item = -1;

	/* Success */
	return (TRUE);
}

/*
 * Hack -- checks rod (by sval) and
 * make a fail check on it.
 */
bool borg_equips_rod(int sval) {
	int i, chance, lev;

	/* Look for that staff */
	i = borg_slot(TV_ROD, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < USE_DEVICE * 2)
		return (FALSE);

	/* Yep we got one */
	return (TRUE);
}

/*
 * Hack -- attempt to zap the given (charged) rod (by sval)
 */
bool borg_zap_rod(int sval) {
	int i, lev, chance;

	/* Look for that rod */
	i = borg_slot(TV_ROD, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* Hack -- Still charging */
	if (!borg_items[i].pval)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < USE_DEVICE + 2)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Zapping %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('z');
	borg_keypress(I2A(i));

	/* Success */
	return (TRUE);
}

/*
 * Hack -- checks inven and make a fail check on it.
 */
bool borg_equips_planar(/*int fail_allowed*/) {
	int i, chance, lev;

	/* Look for that staff */
	i = INVEN_WIELD;

	/* No item */
	if (!borg_items[i].iqty)
		return (FALSE);

	/* Not a Trump which is now a Planar */
	if (borg_items[i].name2 != EGO_PLANAR)
		return (FALSE);

	/* Not charged */
	if (borg_items[i].timeout)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* Cursed items make it much more difficult */
	if (borg_items[i].cursed)
		chance = chance / 3;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < USE_DEVICE * 2)
		return (FALSE);

	/* Yep we got one */
	return (TRUE);
}

/*
 * Hack -- attempt to zap the given (charged) Trump weapon
 */
bool borg_activate_planar(/*int allow_fail*/) {
	int i, lev, chance;

	/* Look for that staff */
	i = INVEN_WIELD;

	/* No item */
	if (!borg_items[i].iqty)
		return (FALSE);

	/* Not a Trump, which is not a planar */
	if (borg_items[i].name2 != EGO_PLANAR)
		return (FALSE);

	/* Not charged */
	if (borg_items[i].timeout)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < USE_DEVICE + 2)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Activating Trump %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('A');
	borg_keypress(I2A(i - INVEN_WIELD));

	/* Success */
	return (TRUE);
}

/*
 * Hack -- checks wand (by sval) and
 * make a fail check on it.
 */
bool borg_equips_wand(int sval) {
	int i, chance, lev;

	/* Look for that staff */
	i = borg_slot(TV_WAND, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < USE_DEVICE * 2)
		return (FALSE);

	/* Yep we got one */
	return (TRUE);
}

/*
 * Hack -- attempt to aim the given (charged) wand (by sval)
 */
bool borg_aim_wand(int sval) {
	int i;

	/* Look for that wand */
	i = borg_slot(TV_WAND, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* record the address to avoid certain bugs with inscriptions&amnesia */
	zap_slot = i;

	/* Log the message */
	borg_note(format("# Aiming %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('a');
	borg_keypress(I2A(i));

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to use the given (charged) staff (by sval)
 */
bool borg_use_staff(int sval) {
	int i;

	/* Look for that staff */
	i = borg_slot(TV_STAFF, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* record the address to avoid certain bugs with inscriptions&amnesia */
	zap_slot = i;

	/* Log the message */
	borg_note(format("# Using %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('u');
	borg_keypress(I2A(i));

	/* Success */
	return (TRUE);
}

/*
 * Hack -- attempt to use the given (charged) staff (by sval) and
 * make a fail check on it.
 */
bool borg_use_staff_fail(int sval) {
	int i, chance, lev;

	/* Look for that staff */
	i = borg_slot(TV_STAFF, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage, but if its a Teleport be generous. */
	if (chance < USE_DEVICE * 2) {
		if (sval != SV_STAFF_TELEPORTATION) {
			return (FALSE);
		}

		/* We need to give some "desparation attempt to teleport staff" */
		if (!borg_skill[BI_ISCONFUSED] && !borg_skill[BI_ISBLIND]) /* Dark? */
		{
			/* We really have no chance, return false, attempt the scroll */
			if (chance < USE_DEVICE)
				return (FALSE);
		}
		/* We might have a slight chance, or we cannot not read */
	}

	/* record the address to avoid certain bugs with inscriptions&amnesia */
	zap_slot = i;

	/* Log the message */
	borg_note(format("# Using %s.", borg_items[i].desc));

	/* Perform the action */
	borg_keypress('u');
	borg_keypress(I2A(i));

	/* Success */
	return (TRUE);
}
/*
 * Hack -- checks staff (by sval) and
 * make a fail check on it.
 */
bool borg_equips_staff_fail(int sval) {
	int i, chance, lev;

	/* Look for that staff */
	i = borg_slot(TV_STAFF, sval);

	/* None available */
	if (i < 0)
		return (FALSE);

	/* No charges */
	if (!borg_items[i].pval)
		return (FALSE);

	/* Extract the item level */
	lev = (borg_items[i].level);

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage, but if its a Teleport be generous. */
	if (chance < USE_DEVICE * 2) {
		if (sval != SV_STAFF_TELEPORTATION && sval != SV_STAFF_DESTRUCTION) {
			return (FALSE);
		}

		/* We need to give some "desparation attempt to teleport staff" */
		if (!borg_skill[BI_ISCONFUSED]) {
			/* We really have no chance, return false, attempt the scroll */
			if (chance < USE_DEVICE)
				return (FALSE);
		}

		/* We might have a slight chance, continue on */
	}

	/* Yep we got one */
	return (TRUE);
}

/*
 * Hack -- attempt to use the given artifact (by index)
 */
bool borg_activate_artifact(int name1, bool secondary) {
	int i;

	/* Check the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect artifacts */
		if (item->name1 != name1)
			continue;

		/* Check charge */
		if (item->timeout)
			return (FALSE);

		/*
		 * Random Artifact must be *ID* to know the activation power.
		 * The borg will cheat with random artifacts to know if the
		 * artifact number is activatable, but artifact names and
		 * types will be scrambled.  So he must first *ID* the artifact
		 * he must play with the artifact to learn its power, just as
		 * he plays with magic to gain experience.  But I am not about
		 * to undertake that coding.  He needs to *ID* it anyway to learn
		 * of the resists that go with the artifact.
		 * Lights dont need *id* just regular id.
		 */
		if (/* adult_rand_artifacts */ (item->name1 != ART_BEATRICE &&
												  item->name1 != ART_EOS &&
												  item->name1 != ART_HIPPO) &&
			 (!item->fully_identified)) {
			/* borg_note(format("# %s must be *ID*'d before activation.",
			 * item->desc)); */
			return (FALSE);
		}

		/* Log the message */
		borg_note(format("# Activating artifact %s.", item->desc));

		/* Perform the action */
		borg_keypress('A');
		borg_keypress(I2A(i - INVEN_WIELD));

		/* Jewel aslo gives Recall */
		if (item->name1 == ART_HIPPO) {
			if (secondary == FALSE) {
				borg_keypress('n');
			} else {
				borg_keypress('y');
			}
		}

		/* Success */
		return (TRUE);
	}

	/* Oops */
	return (FALSE);
}

/*
 *  Hack -- check and see if borg is wielding an artifact
 */
bool borg_equips_artifact(int name1) {
	int i;
	int lev, chance;

	/* Check the equipment-- */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect artifacts */
		if (item->name1 != name1)
			continue;

		/* Check charge.  But not on certain ones  Wor, ID, phase, TELEPORT.*/
		/* this is to ensure that his borg_prep code is working ok */
		if ((name1 != ART_AZRAEL && name1 != ART_NYNAULD) && (item->timeout))
			continue;
		/*
		 * Random Artifact must be *ID* to know the activation power.
		 * The borg will cheat with random artifacts to know if the
		 * artifact number is activatable, but artifact names and
		 * types will be scrambled.  So he must first *ID* the artifact
		 * he must play with the artifact to learn its power, just as
		 * he plays with magic to gain experience.  But I am not about
		 * to undertake that coding.  He needs to *ID* it anyway to learn
		 * of the resists that go with the artifact.
		 * Lights dont need *id* just regular id.
		 */
		if (/* adult_rand_artifacts */ (item->name1 != ART_BEATRICE &&
												  item->name1 != ART_EOS &&
												  item->name1 != ART_HIPPO) &&
			 (!item->fully_identified)) {
			/* borg_note(format("# %s must be *ID*'d before activation.",
			 * item->desc)); */
			continue;
		}

		/* Extract the item level for fail rate check*/
		lev = item->level;

		/* Base chance of success */
		chance = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED])
			chance = chance / 2;

		/* Cursed items make it much more difficult */
		if (item->cursed)
			chance = chance / 3;

		/* High level objects are harder */
		chance = chance - ((lev > 50) ? 50 : lev);

		/* Roll for usage.  Return Fail if greater than 50% fail */
		if (chance < (USE_DEVICE * 2))
			continue;

		/* Success */
		return (TRUE);
	}

	/* I guess I dont have it, or it is not ready */
	return (FALSE);
}

/*
 * Check and see if borg is wielding an randart with a specific activation
 * activation - the o_ptr->xtra2
 * immediate - is the item available for immediate use (not-charging)
 */
bool borg_equips_activation(int activation, bool immediate) {
	int i;
	int lev, chance;

	/* Check the equipment-- */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect activations */
		if (item->xtra2 != activation)
			continue;

		/* Check charge, if requested. */
		if (immediate && item->timeout)
			continue;

		/*
				 * Random Artifact must be *ID* to know the activation power.
				 * The borg will cheat with random artifacts to know if the
				 * artifact number is activatable.  But he is not allowed to
		 * activate it unless it's *ID*'d
				 */
		if (!item->fully_identified)
			continue;

		/* Extract the item level for fail rate check*/
		lev = item->level;

		/* Base chance of success */
		chance = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED])
			chance = chance / 2;

		/* Cursed items make it much more difficult */
		if (item->cursed)
			chance = chance / 3;

		/* High level objects are harder */
		chance = chance - ((lev > 50) ? 50 : lev);

		/* Roll for usage.  Return Fail if greater than 50% fail */
		if (chance < (USE_DEVICE * 2))
			continue;

		/* Success */
		return (TRUE);
	}

	/* I guess I dont have it, or it is not ready */
	return (FALSE);
}

/*
 * Check and see if borg is wielding an artifact
 */
bool borg_equips_item(int tval, int sval) {
	int i;
	int lev, chance;

	/* Check the equipment-- */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect artifacts */
		if (item->tval != tval)
			continue;
		if (item->sval != sval)
			continue;

		/* Check charge. */
		if (item->timeout)
			continue;
		if (!item->ident)
			continue;

		/* Extract the item level for fail rate check*/
		lev = item->level;

		/* Base chance of success */
		chance = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED])
			chance = chance / 2;

		/* Cursed items make it much more difficult */
		if (item->cursed)
			chance = chance / 3;

		/* High level objects are harder */
		chance = chance - ((lev > 50) ? 50 : lev);

		/* Roll for usage.  Return Fail if greater than 50% fail */
		if (chance < (USE_DEVICE * 2))
			continue;

		/* Success */
		return (TRUE);
	}

	/* I guess I dont have it, or it is not ready, or too hard to activate. */
	return (FALSE);
}

/*
 * Attempt to use the given equipment item.
 */
bool borg_activate_item(int tval, int sval, bool target) {
	int i;

	/* Check the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect items */
		if (item->tval != tval)
			continue;
		if (item->sval != sval)
			continue;

		/* Check charge */
		if (item->timeout)
			continue;

		/* Log the message */
		borg_note(format("# Activating item %s.", item->desc));

		/* Perform the action */
		borg_keypress('A');
		borg_keypress(I2A(i - INVEN_WIELD));

		/* Some items require a target */
		if (target) {
			borg_keypress('5');
		}

		/* Success */
		return (TRUE);
	}

	/* Oops */
	return (FALSE);
}

/*
 * Attempt to use the given equipment randart activation.
 */
bool borg_activate_activation(int activation, bool target) {
	int i;

	/* Check the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		borg_item *item = &borg_items[i];

		/* Skip incorrect items */
		if (item->xtra2 != activation)
			continue;

		/* Check charge */
		if (item->timeout)
			continue;

		/* Log the message */
		borg_note(format("# Activating randart %s.", item->desc));

		/* Perform the action */
		borg_keypress('A');
		borg_keypress(I2A(i - INVEN_WIELD));

		/* Some items require a target */
		if (target) {
			borg_keypress('5');
		}

		/* Success */
		return (TRUE);
	}

	/* Oops */
	return (FALSE);
}

/*
 * Hack -- check and see if borg is wielding a dragon armor and if
 * he will pass a fail check.
 */
bool borg_equips_dragon(int drag_sval) {
	int lev, chance;

	/* Check the equipment */
	borg_item *item = &borg_items[INVEN_BODY];

	/* Skip incorrect armours */
	if (item->tval != TV_DRAG_ARMOR)
		return (FALSE);
	if (item->sval != drag_sval)
		return (FALSE);

	/* Check charge */
	if (item->timeout)
		return (FALSE);

	/* Make Sure Mail is IDed */
	if (!item->ident)
		return (FALSE);

	/* check on fail rate
	  * The fail check is automatic for dragon armor.  It is an attack
	  * item.  He should not sit around failing 5 or 6 times in a row.
	  * he should attempt to activate it, and if he is likely to fail, then
	  * eh should look at a different attack option.  We are assuming
	  * that the fail rate is about 50%.  So He may still try to activate it
	  * and fail.  But he will not even try if he has negative chance or
	  * less than twice the USE_DEVICE variable
	  */
	/* Extract the item level */
	lev = borg_items[INVEN_BODY].level;

	/* Base chance of success */
	chance = borg_skill[BI_DEV];

	/* Confusion hurts skill */
	if (borg_skill[BI_ISCONFUSED])
		chance = chance / 2;

	/* Cursed items make it much more difficult */
	if (item->cursed)
		chance = chance / 3;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Roll for usage */
	if (chance < (USE_DEVICE * 2))
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Attempt to use the given dragon armour
 */
bool borg_activate_dragon(int drag_sval) {

	/* Check the equipment */

	borg_item *item = &borg_items[INVEN_BODY];

	/* Skip incorrect mails */
	if (item->tval != TV_DRAG_ARMOR)
		return (FALSE);
	if (item->sval != drag_sval)
		return (FALSE);

	/* Check charge */
	if (item->timeout)
		return (FALSE);

	/*  Make Sure Mail is IDed */
	if (!item->ident)
		return (FALSE);

	/* Log the message */
	borg_note(format("# Activating dragon scale %s.", item->desc));

	/* Perform the action */
	borg_keypress('A');
	borg_keypress(I2A(INVEN_BODY - INVEN_WIELD));

	/* Success */
	return (TRUE);
}

/*
 * Determine if borg can cast a given spell (when fully rested)
 */
bool borg_spell_legal(int realm, int book, int what) {
	borg_magic *as = &borg_magics[realm][book][what];

	/* The borg must be able to "cast" spells this realm*/
	if (borg_skill[BI_REALM1] != realm && borg_skill[BI_REALM2] != realm)
		return (FALSE);

	/* Make sure we have this realm book */
	if (amt_book[realm][book] <= 0)
		return (FALSE);

	/* The spell must be "known" */
	if (as->status < BORG_MAGIC_TEST)
		return (FALSE);

	/* The spell must be affordable (when rested) */
	if (as->power > borg_skill[BI_MAXSP])
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Determine if borg can cast a given spell (right now)
 */
bool borg_spell_okay(int realm, int book, int what) {
	int reserve_mana = 0;

	borg_magic *as = &borg_magics[realm][book][what];

	/* Dark */
	if (no_lite())
		return (FALSE);

	/* Not if locked down */
	if (borg_skill[BI_CRSNOMAGIC])
		return (FALSE);

	/* Define reserve_mana for each class */
	if (borg_skill[BI_REALM1] == REALM_SORCERY)
		reserve_mana = 6;
	if (borg_skill[BI_REALM1] == REALM_TAROT)
		reserve_mana = 6;
	if (borg_skill[BI_REALM1] == REALM_CHARMS)
		reserve_mana = 15;
	if (borg_skill[BI_REALM1] == REALM_CHAOS)
		reserve_mana = 15;

	/* Low level spell casters should not worry about this */
	if (borg_skill[BI_CLEVEL] < 35)
		reserve_mana = 0;

	/* Require ability (when rested) */
	if (!borg_spell_legal(realm, book, what))
		return (FALSE);

	/* Hack -- blind/confused */
	if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* The spell must be affordable (now) */
	if (as->power > borg_skill[BI_CURSP])
		return (FALSE);

	/* Do not cut into reserve mana (for final teleport) */
	if (borg_skill[BI_CURSP] - as->power < reserve_mana &&
		 realm == REALM_SORCERY) {
		/* Phase spells ok */
		if (book == 0 && what == 2)
			return (TRUE);

		/* Teleport spells ok */
		if (book == 1 && what == 5)
			return (TRUE);

		/* Satisfy Hunger OK */
		if (book == 2 && what == 0)
			return (TRUE);

		/* others are rejected */
		return (FALSE);
	}

	/* Success */
	return (TRUE);
}

/*
 * fail rate on a spell
 */
static int borg_spell_fail_rate(int realm, int book, int what) {
	int chance, minfail;
	int spell_stat = A_INT;

	borg_magic *as = &borg_magics[realm][book][what];

	/* Check the right stat */
	spell_stat = mp_ptr->spell_stat;

	/* Access the spell  */
	chance = as->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (borg_skill[BI_CLEVEL] - as->level);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_stat[ADJ_INTWIS][my_stat_ind[spell_stat]] - 1);

	/* Extract the minimum failure rate */
	minfail = adj_stat[ADJ_FAILURE][my_stat_ind[spell_stat]];

	/* Non mage characters never get too good */
	if (borg_class != CLASS_MAGE) {
		if (minfail < 5)
			minfail = 5;
	}

	/* Minimum failure rate */
	if (chance < minfail)
		chance = minfail;

	/* Stunning makes spells harder */
	if (borg_skill[BI_ISHEAVYSTUN])
		chance += 25;
	if (borg_skill[BI_ISSTUN])
		chance += 15;

	/* Always a 5 percent chance of working */
	if (chance > 95)
		chance = 95;

	/* Return the chance */
	return (chance);
}

/*
 * same as borg_spell_okay with a fail % check
 */
bool borg_spell_okay_fail(int realm, int book, int what, int allow_fail) {
	if (borg_spell_fail_rate(realm, book, what) > allow_fail)
		return FALSE;
	return borg_spell_okay(realm, book, what);
}

/*
 * Same as borg_spell with a fail % check
 */
bool borg_spell_fail(int realm, int book, int what, int allow_fail) {
	if (borg_spell_fail_rate(realm, book, what) > allow_fail)
		return FALSE;
	return borg_spell(realm, book, what);
}

/*
 * Same as borg_spell_legal with a fail % check
 */
bool borg_spell_legal_fail(int realm, int book, int what, int allow_fail) {
	if (borg_spell_fail_rate(realm, book, what) > allow_fail)
		return FALSE;
	return borg_spell_legal(realm, book, what);
}

/*
 * Attempt to cast a spell
 */
bool borg_spell(int realm, int book, int what) {
	int i;

	borg_magic *as = &borg_magics[realm][book][what];

	/* Require ability (right now) */
	if (!borg_spell_okay(realm, book, what))
		return (FALSE);

	/* Not if locked down */
	if (borg_skill[BI_CRSNOMAGIC])
		return (FALSE);

	/* Look for the book */
	i = borg_book[realm][book];

	/* Paranoia */
	if (i < 0)
		return (FALSE);

	/* Debugging Info */
	borg_note(format("# Casting %s (%d,%d) fail rate: %d.", as->name, book, what,
						  borg_spell_fail_rate(realm, book, what)));

	/* Cast a spell */
	borg_keypress('m');
	borg_keypress(I2A(i));
	borg_keypress(I2A(what));

	/* Because we have no launch message to indicate failure */
	if (realm == 1 && book == 3 && what == 4) {
		borg_casted_glyph = TRUE;
	} else {
		borg_casted_glyph = FALSE;
	}

	/* increment the spell counter */
	as->times++;

	/* Success */
	return (TRUE);
}

/*** Mindcrafter spells are much like realm spells ***/

/*
 * Determine if borg can cast a given Mindcraft spell (when fully rested)
 */
bool borg_mindcr_legal(int spell, int level) {
	borg_mind *as = &borg_minds[spell];

	/* The borg must be able to "cast" spells this realm*/
	if (borg_class != CLASS_ORPHIC)
		return (FALSE);

	/* The spell must be "known" */
	if (borg_skill[BI_CLEVEL] < level)
		return (FALSE);

	/* The spell must be affordable (when rested) */
	if (as->power > borg_skill[BI_MAXSP])
		return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Determine if borg can cast a given spell (right now)
 */
bool borg_mindcr_okay(int spell, int level) {
	int reserve_mana = 0;

	borg_mind *as = &borg_minds[spell];

	/* Antimagic curse */
	if (borg_skill[BI_CRSNOMAGIC])
		return (FALSE);

	/* Define reserve_mana for Displacement */
	if (borg_skill[BI_CLEVEL] >= 7)
		reserve_mana = 2;

	/* Low level spell casters should not worry about this */
	if (borg_skill[BI_CLEVEL] < 35)
		reserve_mana = 0;

	/* Require ability (when rested) */
	if (!borg_mindcr_legal(spell, level))
		return (FALSE);

	/* Hack -- blind/confused */
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/* The spell must be affordable (now) */
	if (as->power > borg_skill[BI_CURSP])
		return (FALSE);

	/* Do not cut into reserve mana (for final teleport) */
	if (borg_skill[BI_CURSP] - as->power < reserve_mana) {
		/* Minor Displacement spells ok */
		if (spell == 2)
			return (TRUE);

		/* Major Displacement ok */
		if (spell == 3)
			return (TRUE);

		/* Psi Drain */
		if (spell == 10)
			return (TRUE);

		/* others are rejected */
		return (FALSE);
	}
	/* Success */
	return (TRUE);
}

/*
 * fail rate on a mindcrafter spell
 */
static int borg_mindcr_fail_rate(int spell /*, int level*/) {
	int chance, minfail;
	borg_mind *as = &borg_minds[spell];

	/* Access the spell  */
	chance = as->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (borg_skill[BI_CLEVEL] - as->level);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_stat[ADJ_INTWIS][my_stat_ind[A_WIS]] - 1);

	/* Extract the minimum failure rate */
	minfail = adj_stat[ADJ_FAILURE][my_stat_ind[A_WIS]];

	/* Minimum failure rate */
	if (chance < minfail)
		chance = minfail;

	/* Stunning makes spells harder */
	if (borg_skill[BI_ISHEAVYSTUN])
		chance += 25;
	if (borg_skill[BI_ISSTUN])
		chance += 15;

	/* Always a 5 percent chance of working */
	if (chance > 95)
		chance = 95;

	/* Return the chance */
	return (chance);
}

/*
 * same as borg_mind_okay with a fail % check
 */
bool borg_mindcr_okay_fail(int spell, int level, int allow_fail) {
	if (borg_mindcr_fail_rate(spell /*, level*/) > allow_fail)
		return FALSE;
	return borg_mindcr_okay(spell, level);
}

/*
 * Same as borg_mind with a fail % check
 */
bool borg_mindcr_fail(int spell, int level, int allow_fail) {
	if (borg_mindcr_fail_rate(spell /*, level*/) > allow_fail)
		return FALSE;
	return borg_mindcr(spell, level);
}

/*
 * Same as borg_mind_legal with a fail % check
 */
bool borg_mindcr_legal_fail(int spell, int level, int allow_fail) {
	if (borg_mindcr_fail_rate(spell /*, level*/) > allow_fail)
		return FALSE;
	return borg_mindcr_legal(spell, level);
}

/*
 * Attempt to cast a mindcrafter spell
 */
bool borg_mindcr(int spell, int level) {
	borg_mind *as = &borg_minds[spell];

	/* Require ability (right now) */
	if (!borg_mindcr_okay(spell, level))
		return (FALSE);

	/* Not if locked down */
	if (borg_skill[BI_CRSNOMAGIC])
		return (FALSE);

	/* If attempting Phase Door vs Dimension Door */
	if (spell == MIND_MINOR_DISP && level == 3 && borg_skill[BI_CLEVEL] >= 40)
		return (FALSE);

	/* Debugging Info */
	borg_note(format("# Casting %s (spell: %d, level: %d, fail rate: %d).",
						  as->name, spell, level,
						  borg_mindcr_fail_rate(spell /*, level*/)));

	/* Cast a spell */
	borg_keypress('m');
	borg_keypress(as->letter);

	/* increment the spell counter */
	as->times++;

	/* Success */
	return (TRUE);
}

/*** Racial abilities are much like magic spells ***/

/*
 * Determine if borg can cast a given Racial spell
 * (when fully rested).
 * -or-
 * with a reasonable degree of difficulty with Check_fail
 */
bool borg_racial_check(int race, bool check_fail /*, int num*/) {
	int i;
	int val;
	int sum = 0;

	int lev_req = 99;
	int cost = 0;
	int stat = 0;
	int diff = 0;
	int use_stat = 0;
	int difficulty = 0;

	bool use_hp = FALSE;

	/* The borg must be able to "cast" spells this race */
	if (borg_race != race)
		return (FALSE);

	/* not when confused */
	if (borg_skill[BI_ISCONFUSED])
		return (FALSE);

	/*Assume there is no power, (aka set lev_req as 99)*/
	lev_req = 99;

	/*TODO SHOULD we should create a variable that points to either racial or
	sign powers
	https://stackoverflow.com/questions/16201607/c-pointer-to-array-of-structs*/

	if (race >= SIGN_HEAD) {
		race = race - SIGN_HEAD;
		/* New and improved racial power descriptions*/
		for (i = 0; sign_powers[i].description != NULL; i++) {
			if (sign_powers[i].idx == race) {
				use_stat = sign_powers[i].stat;
				lev_req = sign_powers[i].level;
				cost = sign_powers[i].cost +
						 borg_skill[BI_CLEVEL] * sign_powers[i].cost_level;
				break;
			}
		}
		switch (race) {
		case SIGN_MORUI:
			diff = 14;
			break;
		case SIGN_DRACO:
		case SIGN_PLUTUS:
			diff = 12;
			break;
		}

	} else {
		/* New and improved racial power descriptions*/
		for (i = 0; racial_powers[i].description != NULL; i++) {
			if (racial_powers[i].idx == race) {
				use_stat = racial_powers[i].stat;
				lev_req = racial_powers[i].level;
				cost = racial_powers[i].cost +
						 borg_skill[BI_CLEVEL] * racial_powers[i].cost_level;
				break;
			}
		}
		switch (race) {
		case SPECTRE:
			diff = 3;
			break;
		case GUARDIAN:
			diff = 8;
			break;
		case ATLANTIAN:
		case VAMPIRE:
			diff = 9;
			break;
		case LEPRECHAUN:
		case DWARF:
		case GNOME:
		case GIANT:
		case TITAN:
			diff = 12;
			break;
		case NORDIC:
		case TROLL:
			diff = ((borg_class == CLASS_WARRIOR) ? 6 : 12);
			break;
		case HORROR:
		case KOBOLD:
			diff = 14;
			break;
		case OGRE:
		case IMP:
		case FAE:
			diff = 15;
			break;
		case SKELETON:
		case MUMMY:
			diff = 18;
		case NEPHILIM:
			diff = 50;
			break;
		}
	}

	/* Power is not available yet */
	if (borg_skill[BI_CLEVEL] < lev_req)
		return (FALSE);

	/* Not enough mana - use hp */
	if (borg_skill[BI_CURSP] <= cost)
		use_hp = TRUE;

	/* Too confused */
	if (borg_skill[BI_ISCONFUSED])
		return FALSE;

	/* Cost -- dont go into debt */
	if (use_hp && (cost > borg_skill[BI_CURHP] * 5 / 10) && !borg_munchkin_mode)
		return (FALSE);

	/* Cost -- dont go into debt */
	if (use_hp && cost >= borg_skill[BI_CURHP])
		return (FALSE);

	/* Legal check ends here */
	if (!check_fail)
		return (TRUE);

	/* Otherwise continue on to a fail check */

	/* Cost -- dont go into debt */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 5 / 10 &&
		 borg_race != GNOME && !borg_munchkin_mode)
		return (FALSE);

	/* Cost -- Gnomes can go into emergency zone (mostly) */
	if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 3 / 10 &&
		 borg_race == GNOME && !borg_munchkin_mode)
		return (FALSE);

	/* Reasonable chance of success */
	stat = my_stat_cur[use_stat];
	difficulty = diff;

	/* Stun makes it more difficult */
	if (borg_skill[BI_ISSTUN]) {
		difficulty += 10;
	} else {
		int lev_adj = ((borg_skill[BI_CLEVEL] - lev_req) / 3);
		if (lev_adj > 10)
			lev_adj = 10;
		difficulty -= lev_adj;
	}

	if (difficulty < 5)
		difficulty = 5;

	/* We only need halfs of the difficulty */
	difficulty = difficulty / 2;

	for (i = 1; i <= stat; i++) {
		val = i - difficulty;
		if (val > 0)
			sum += (val <= difficulty) ? val : difficulty;
	}

	/* My % chance to fail */
	difficulty = 100 - (((sum * 100) / difficulty) / stat);

	/* difficulty is my fail percent */
	if (difficulty > 60)
		return (FALSE);
	else
		/* Success */
		return (TRUE);
}

/*
 * Attempt to cast a racial spell
 * We can support up to 2 racial powers
 */
bool borg_racial(int race, int num) {

	/* Require ability (right now) */
	if (!borg_racial_check(race, TRUE /*, num*/))
		return (FALSE);

	/* Debugging Info */
	borg_note("# Racial Power.");

	/* Cast a spell */
	borg_keypress('U');
	if (num != 1)
		borg_keypress('b');
	else
		borg_keypress('a');

	/* Success */
	return (TRUE);
}

/*
 * Inscribe food and Slime Molds
 */
extern bool borg_inscribe_food(void) {
	int ii;
	char name[80];

	for (ii = 0; ii < INVEN_TOTAL; ii++) {
		borg_item *item = &borg_items[ii];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Require correct tval */
		if (item->tval != TV_FOOD)
			continue;

		/* skip things already inscribed */
		if (item->note != NULL &&
			 (!(streq(item->note, "")) && !(streq(item->note, " "))))
			continue;

		/* inscribe foods and molds */
		if (item->sval == SV_FOOD_SLIME_MOLD || item->sval == SV_FOOD_RATION) {

			if (item->sval == SV_FOOD_RATION) {
				/* get a name */
				strcpy(name, food_syllable1[rand_int(sizeof(food_syllable1) /
																 sizeof(char *))]);
				strcat(name, food_syllable2[rand_int(sizeof(food_syllable2) /
																 sizeof(char *))]);

				borg_send_inscribe(ii, name);
				return (TRUE);
			}

			if (item->sval == SV_FOOD_SLIME_MOLD) {
				/* get a name */
				strcpy(name, mold_syllable1[rand_int(sizeof(mold_syllable1) /
																 sizeof(char *))]);
				strcat(name, mold_syllable2[rand_int(sizeof(mold_syllable2) /
																 sizeof(char *))]);
				strcat(name, mold_syllable3[rand_int(sizeof(mold_syllable3) /
																 sizeof(char *))]);

				borg_send_inscribe(ii, name);
				return (TRUE);
			}
		}
	}

	/* all done */
	return (FALSE);
}

/*
 * Deinscribe some items
 */
extern bool borg_needed_deinscribe(void) {
	int ii;

	for (ii = 0; ii < INVEN_TOTAL; ii++) {
		borg_item *item = &borg_items[ii];

		/* Skip empty items */
		if (!item->iqty)
			continue;

		/* Skip Artifacts */
		/* if (item->name1 || item->name3) continue; */

		/* Certain things to have inscriptions removed */
		if (strstr(item->note, "!") ||
			 (strstr(item->note, "Quest") && item->ident)) {
			/* this item is ID'd.  It no longer needs the inscription.  */
			item->quest = FALSE;
			borg_send_deinscribe(ii);
			return (TRUE);
		}

		if ((strstr(item->note, "special") && item->ident)) {
			/* this item is ID'd.  It no longer needs the inscription.  */
			borg_send_deinscribe(ii);
			borg_respawning = 5;
			return (TRUE);
		}
	}

	/* all done */
	return (FALSE);
}

/*
 * Send a command to de-inscribe item number "i" .
 */
void borg_send_deinscribe(int i) {

	/* Ok to inscribe Slime Molds */
	if (borg_items[i].tval == TV_FOOD &&
		 borg_items[i].sval == SV_FOOD_SLIME_MOLD)
		return;

	/* Label it */
	borg_keypress('}');

	/* Choose from inventory */
	if (i < INVEN_WIELD) {
		/* Choose the item */
		borg_keypress(I2A(i));
	}

	/* Choose from equipment */
	else {
		/* Go to equipment (if necessary) */
		if (borg_items[0].iqty)
			borg_keypress('/');

		/* Choose the item */
		borg_keypress(I2A(i - INVEN_WIELD));
	}

	/* Clear the Note */
	borg_items[i].note = "";
}
/*
 * Cheat the "Store" screen
 * There are no towns, so we ignore town_num
 */
void borg_cheat_store(int shop_num) {
	int i;
	int slot;
	char buf[256];
	int x;

	object_type *j_ptr;

	store_type *st_ptr = &store[shop_num];

	/* Clear the Inventory from memory */
	for (i = 0; i < 24; i++) {
		/* Wipe the ware */
		WIPE(&borg_shops[shop_num].ware[i], borg_item);
	}

	/* Check each existing object in this store */
	for (slot = 0; slot < 24; slot++) {
		/* Get the existing object */
		j_ptr = &st_ptr->stock[slot];

		/* Default to "nothing" */
		buf[0] = '\0';

		/* Describe it */
		object_desc(buf, j_ptr, TRUE, 3);

		/* Skip Empty slots */
		if (streq(buf, "(nothing)"))
			continue;

		/* Copy the Description to the borg's memory */
		strcpy(borg_shops[shop_num].ware[slot].desc, buf);

		/* Analyze the item */
		borg_item_analyze(&borg_shops[shop_num].ware[slot],
								&store[shop_num].stock[slot], buf, slot);

		/*need to be able to analize the home inventory to see if it was */
		/* *fully ID*d. */
		/* This is a BIG CHEAT!  It will be less of a cheat if code is put*/
		/* in place to allow 'I' in stores. */
		if (store[shop_num].stock[slot].ident & IDENT_MENTAL) {
			/* XXX XXX XXX for now, always cheat to get info on items at */
			/*   home. */
			borg_object_star_id_aux(&borg_shops[shop_num].ware[slot],
											&store[shop_num].stock[slot]);
			borg_shops[shop_num].ware[slot].fully_identified = TRUE;
		}

		/* hack -- see of store is selling food.  Needed for Money Scumming */
		if (shop_num == 0 && borg_shops[shop_num].ware[slot].tval == TV_FOOD &&
			 borg_shops[shop_num].ware[slot].sval == SV_FOOD_RATION) {
			borg_food_onsale = borg_shops[shop_num].ware[slot].iqty;
		}

		/* Hack -- Save the declared cost */
		if (shop_num == 7)
			borg_shops[shop_num].ware[slot].cost = 0;
		else {
			/* Extract the "minimum" price */
			x = price_item(j_ptr, owners[shop_num][st_ptr->owner].min_inflate,
								FALSE);

			/* Hack -- Apply Sales Tax if needed */
			if (!no_need_to_bargain(x))
				x += x / 10;

			borg_shops[shop_num].ware[slot].cost = x;
		}
	}
}

/*
 * Cheat the "equip" screen
 */
void borg_cheat_equip(void) {
	int i;

	char buf[256];

	/* Extract the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		/* Default to "nothing" */
		buf[0] = '\0';

		/* Describe a real item */
		if (inventory[i].k_idx) {
			/* Describe it */
			object_desc(buf, &inventory[i], TRUE, 3);
		}

		/* Analyze the item (no price) */
		borg_item_analyze(&borg_items[i], &inventory[i], buf, i);

		/* get the fully id stuff */
		if ((inventory[i].ident & IDENT_MENTAL) ||
			 (inventory[i].ident & IDENT_STOREB)) {
			borg_items[i].fully_identified = TRUE;
			borg_items[i].needs_I_exam = TRUE;
			borg_do_star_id = TRUE;
			/* if (streq(borg_items[i].note, "special"))
			 * borg_items[i].fully_identified = FALSE; */
		}

		/* Make sure my trump weapon has inscribed the . to prevent teleportation
		 */
		if ((borg_items[i].flags3 & TR3_TELEPORT) &&
			 !strstr(borg_items[i].note, "."))
			borg_send_inscribe(i, ".");
	}
}

/*
 * Cheat the "inven" screen
 */
void borg_cheat_inven(void) {
	int i;

	char buf[256];

	/* Extract the current weight */
	borg_cur_wgt = total_weight;

	/* Extract the inventory */
	for (i = 0; i < INVEN_PACK; i++) {
		/* Default to "nothing" */
		buf[0] = '\0';

		/* Describe a real item */
		if (inventory[i].k_idx) {
			/* Describe it */
			object_desc(buf, &inventory[i], TRUE, 3);
		}

		/* Analyze the item (no price) */
		borg_item_analyze(&borg_items[i], &inventory[i], buf, i);

		/* get the fully id stuff */
		if (((inventory[i].ident & IDENT_MENTAL) ||
			  (inventory[i].ident & IDENT_STOREB)) /* &&
			!streq(borg_items[i].note, "special") */) {
			borg_items[i].fully_identified = TRUE;
			borg_items[i].needs_I_exam = TRUE;
			borg_do_star_id = TRUE;
			/* if (streq(borg_items[i].note, "special"))
			 * borg_items[i].fully_identified = FALSE; */
		}

		/* Make sure my trump weapon has inscribed the . to prevent teleportation
		 */
		if ((borg_items[i].flags3 & TR3_TELEPORT) &&
			 !strstr(borg_items[i].note, "."))
			borg_send_inscribe(i, ".");

		/* Note changed inventory */
		borg_do_crush_junk = TRUE;
		borg_do_crush_hole = TRUE;
		borg_do_crush_slow = TRUE;
	}
}

/*
 * Hack -- Cheat the "spell" info
 *
 * Hack -- note the use of the "cheat" field for efficiency
 */
void borg_cheat_spell(int realm) {
	int j, what;
	int book;

	/* Can we use spells/prayers? */
	if (realm == 0)
		return;

	/* process books */
	for (book = 0; book < 4; book++) {
		/* Process the spells */
		for (what = 0; what < 8; what++) {
			/* Access the spell */
			borg_magic *as = &borg_magics[realm][book][what];

			/* Skip illegible spells */
			if (as->status == BORG_MAGIC_ICKY)
				continue;

			/* Access the index */
			j = as->cheat;

			/* Note "forgotten" spells */
			if ((realm == borg_skill[BI_REALM1])
					  ? ((spell_forgotten1 & (1L << j)))
					  : ((spell_forgotten2 & (1L << j)))) {
				/* Forgotten */
				as->status = BORG_MAGIC_LOST;
			}

			/* Note "difficult" spells */
			else if (borg_skill[BI_CLEVEL] < as->level) {
				/* Unknown */
				as->status = BORG_MAGIC_HIGH;
			}

			/* Note "unknown" spells */
			else if (!((realm == borg_skill[BI_REALM1])
								? (spell_learned1 & (1L << j))
								: (spell_learned2 & (1L << j)))) {
				/* Unknown */
				as->status = BORG_MAGIC_OKAY;
			}

			/* Note "untried" spells */
			else if (!((realm == borg_skill[BI_REALM1])
								? (spell_worked1 & (1L << j))
								: (spell_worked2 & (1L << j)))) {
				/* Untried */
				as->status = BORG_MAGIC_TEST;
			}

			/* Note "known" spells */
			else {
				/* Known */
				as->status = BORG_MAGIC_KNOW;
			}
		} /* book */
	}	 /* Realm */
}

/*
 * Prepare a book
 */
static void prepare_book_info(int realm, int book) {
	int i, what;

	int spell[64], num = 0;

	/* Can we use spells/prayers in that realm? */
	if (realm == 0)
		return;

	/* Reset each spell entry */
	for (what = 0; what < 8; what++) {
		borg_magic *as = &borg_magics[realm][book][what];

		/* Assume no name */
		as->name = NULL;

		/* Know the Realm, if any */
		as->realm = realm;

		/* Assume illegible */
		as->status = BORG_MAGIC_ICKY;

		/* Assume illegible */
		as->method = BORG_MAGIC_ICK;

		/* Impossible values */
		as->level = 99;
		as->power = 99;

		/* Impossible value */
		as->cheat = 99;

		/* Delete the text name */
		as->realm_name = NULL;
	}

	/* Extract spells */
	for (i = 0; i < 32; i++) {
		/* Check for this spell */
		if ((fake_spell_flags[book] & (1L << i))) {
			/* Collect this spell */
			spell[num++] = i;
		}
	}

	/* Process each existing spell */
	for (what = 0; what < num; what++) {
		borg_magic *as = &borg_magics[realm][book][what];

		/*magic_type *s_ptr = &mp_ptr->info[realm-1][spell[what]];*/
		magic_type s_val;
		magic_type *s_ptr = &s_val;
		/* The spells table does not have a NULL entry at the start, so -1 */
		get_extended_spell_info(realm - 1, what, s_ptr);

		/* Skip "illegible" spells */
		if (s_ptr->slevel == 99)
			continue;

		/* Save the spell name */
		as->name = s_ptr->name;

		/* Realm Name */
		as->realm_name = realm_names[realm].name;

		/* Save the Realm, if any */
		as->realm = realm;

		/* Save the spell index */
		as->cheat = spell[what];

		/* Hack -- assume excessive level */
		as->status = BORG_MAGIC_HIGH;

		/* Access the correct "method" */
		as->method = borg_magic_method[realm][book][what];

		/* Access the correct "rating" */
		as->rating = borg_magic_rating[realm][book][what];

		/* Extract the level and power */
		as->level = s_ptr->slevel;
		as->power = s_ptr->smana;

		/* extract fail rate. */
		as->sfail = s_ptr->sfail;
	}
}
/*
 * Prepare a Mindcrafter Array
 */
static void prepare_mind_info(void) {
	int spell;

	/* Reset each spell entry */
	for (spell = 0; spell < MAX_MINDCRAFT_POWERS; spell++) {
		borg_mind *as = &borg_minds[spell];
		mindcraft_power *s_ptr = &mindcraft_powers[spell];

		/* name */
		as->name = s_ptr->name;

		/* values */
		as->level = s_ptr->min_lev;
		as->power = s_ptr->mana_cost;

		/* Fail Rate */
		as->sfail = s_ptr->fail;

		/* Delete the text letter address */
		as->letter = 'a' + spell;
	}
}

/*
 * Hack -- prepare some stuff based on the player race and class
 */
void prepare_race_class_info(void) {
	int book;

	/* Hack -- Realms */
	borg_skill[BI_REALM1] = p_ptr->realm1;
	borg_skill[BI_REALM2] = p_ptr->realm2;

	/* Initialize the various spell arrays by book */
	for (book = 0; book < 4; book++)
		prepare_book_info(borg_skill[BI_REALM1], book);
	for (book = 0; book < 4; book++)
		prepare_book_info(borg_skill[BI_REALM2], book);

	/* MindCrafters */
	if (borg_class == CLASS_ORPHIC) {
		prepare_mind_info();
	}
}

/* Attempt to extract the exact effect of some mutations.  Some can effect our
 * stats and some give us abilities
 * This is a copy from Mutation.c  if new mutations are added there (or effect
 * altered) then the same change must
 * me made here.
 */
extern void borg_cheat_mutations(void) {
	/* Muta1 is Activatible */
	if (p_ptr->muta1 & COR1_VAMPIRISM)
		borg_skill[BI_VAMPIRE] = TRUE;

	/* Muta2 are randome events */

	/* Muta3 effect stats */
	if (p_ptr->muta3) {
		if (p_ptr->muta3 & COR3_HYPER_STR) {
			my_stat_add[A_STR] += 4;
		}
		if (p_ptr->muta3 & COR3_PUNY) {
			my_stat_add[A_STR] -= 4;
		}
		if (p_ptr->muta3 & COR3_HYPER_INT) {
			my_stat_add[A_INT] += 4;
			my_stat_add[A_WIS] += 4;
		}
		if (p_ptr->muta3 & COR3_IDIOTIC) {
			my_stat_add[A_INT] -= 4;
			my_stat_add[A_WIS] -= 4;
		}
		if (p_ptr->muta3 & COR3_RESILIENT) {
			my_stat_add[A_CON] += 4;
		}
		if (p_ptr->muta3 & COR3_XTRA_FAT) {
			my_stat_add[A_CON] += 2;
			borg_skill[BI_SPEED] -= 2;
		}
		if (p_ptr->muta3 & COR3_ALBINO) {
			my_stat_add[A_CON] -= 4;
		}
		if (p_ptr->muta3 & COR3_FLESH_ROT) {
			my_stat_add[A_CON] -= 2;
			my_stat_add[A_CHA] -= 1;
			borg_skill[BI_REG] = FALSE;
		}
		if (p_ptr->muta3 & COR3_SILLY_VOI) {
			my_stat_add[A_CHA] -= 4;
		}
		if (p_ptr->muta3 & COR3_FORKED_TONGUE) {
			my_stat_add[A_CHA] -= 1;
		}
		if (p_ptr->muta3 & COR3_ILL_NORM) {
			my_stat_add[A_CHA] += 0;
		}
		if (p_ptr->muta3 & COR3_GLOW_EYES) {
			borg_skill[BI_SRCH] += 15;
			borg_skill[BI_SRCHFREQ] += 15;
		}
		if (p_ptr->muta3 & COR3_MAGIC_RES) {
			borg_skill[BI_SAV] += (15 + (borg_skill[BI_CLEVEL] / 5));
		}
		if (p_ptr->muta3 & COR3_STENCH) {
			borg_skill[BI_STL] -= 3;
		}
		if (p_ptr->muta3 & COR3_INFRAVIS) {
			borg_skill[BI_INFRA] += 3;
		}
		if (p_ptr->muta3 & COR3_GOAT_LEGS) {
			borg_skill[BI_SPEED] += 3;
		}
		if (p_ptr->muta3 & COR3_SHORT_LEG) {
			borg_skill[BI_SPEED] -= 3;
		}
		if (p_ptr->muta3 & COR3_ELEC_TOUC) {
			borg_skill[BI_ELECSH] = TRUE;
		}
		if (p_ptr->muta3 & COR3_FIRE_BODY) {
			borg_skill[BI_FIRESH] = TRUE;
		}
		if (p_ptr->muta3 & COR3_WART_SKIN) {
			my_stat_add[A_CHA] -= 2;
			borg_skill[BI_ARMOR] += 5;
		}
		if (p_ptr->muta3 & COR3_SCALES) {
			my_stat_add[A_CHA] -= 1;
			borg_skill[BI_ARMOR] += 10;
		}
		if (p_ptr->muta3 & COR3_IRON_SKIN) {
			my_stat_add[A_DEX] -= 1;
			borg_skill[BI_ARMOR] += 25;
		}
		if (p_ptr->muta3 & COR3_WINGS) {
			borg_skill[BI_FEATH] = TRUE;
		}
		if (p_ptr->muta3 & COR3_FEARLESS) {
			borg_skill[BI_RFEAR] = TRUE;
		}
		if (p_ptr->muta3 & COR3_REGEN) {
			borg_skill[BI_REG] = TRUE;
		}
		if (p_ptr->muta3 & COR3_ESP) {
			borg_skill[BI_ESP] = TRUE;
		}
		if (p_ptr->muta3 & COR3_LIMBER) {
			my_stat_add[A_DEX] += 3;
		}
		if (p_ptr->muta3 & COR3_ARTHRITIS) {
			my_stat_add[A_DEX] -= 3;
		}
#ifdef COR3_RES_TIME
		if (p_ptr->muta3 & COR3_RES_TIME) {
		}
#endif /* COR3_RES_TIME */
		if (p_ptr->muta3 & COR3_VULN_ELEM) {
			/* Dble damage from elemental attacks */
			borg_skill[BI_VFIRE] = TRUE;
			borg_skill[BI_VELEC] = TRUE;
			borg_skill[BI_VACID] = TRUE;
			borg_skill[BI_VCOLD] = TRUE;
		}
		if (p_ptr->muta3 & COR3_MOTION) {
			borg_skill[BI_STL] += 1;
			borg_skill[BI_FRACT] = TRUE;
		}
#ifdef COR3_SUS_STATS
		if (p_ptr->muta3 & COR3_SUS_STATS) {
		}
#endif /* COR3_SUS_STATS */
	}
}

/*
 * check to see if the borg has some degree of success with this mutation.
 */
static bool borg_mutation_aux(s16b min_level, int cost, int use_stat,
										int difficulty, int fail_rate) {
	bool use_hp = FALSE;
	int stat;
	int lev_adj = 0;
	int i, val, sum = 0;

	/* Not enough mana - use hp */
	if (borg_skill[BI_CURSP] < cost)
		use_hp = TRUE;

	/* Power is not available yet */
	if (borg_skill[BI_CLEVEL] < min_level || borg_skill[BI_ISCONFUSED] ||
		 (use_hp && (borg_skill[BI_CURHP] / 5 < cost))) {
		return FALSE;
	}
	/* Reasonable chance of success */
	stat = my_stat_cur[use_stat];

	/* Stun makes it more difficult */
	if (borg_skill[BI_ISSTUN]) {
		difficulty += 10;
	} else {
		lev_adj = ((borg_skill[BI_CLEVEL] - min_level) / 3);
		if (lev_adj > 10)
			lev_adj = 10;
		difficulty = difficulty - lev_adj;
	}

	if (difficulty < 5)
		difficulty = 5;

	/* We only need halfs of the difficulty */
	difficulty = difficulty / 2;

	for (i = 1; i <= stat; i++) {
		val = i - difficulty;
		if (val > 0)
			sum += (val <= difficulty) ? val : difficulty;
	}

	/* My % chance to fail */
	difficulty = 100 - (((sum * 100) / difficulty) / stat);

	/* difficulty is my fail percent */
	if (difficulty > fail_rate)
		return (FALSE);
	else
		/* Success */
		return (TRUE);
}

/* Attempt to activate (or check if its legal) a specific mutation
 *
 */
extern bool borg_mutation(u32b power, bool simulation, int fail_rate,
								  bool legal) {
	int i = 0;
	int num = 0;

	/* Humans and others do not have an activatable power (slot A) on the 'U'
	 * list */
	if (borg_race != HUMAN && borg_race != ELF)
		num++;

	/* Nobody has an extra racial command */
	/* if (borg_race == <>) num++; */

	/* Need to have some type of activatable mutation */
	if (!p_ptr->muta1)
		return (FALSE);

	/* Attempt to find the letter designation on the 'U'list.  Order is
	 * important. */
	if (p_ptr->muta1 & COR1_SPIT_ACID) {
		num++;
		if (power == COR1_SPIT_ACID)
			i = num;
	}
	if (p_ptr->muta1 & COR1_BR_FIRE) {
		num++;
		if (power == COR1_BR_FIRE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_HYPN_GAZE) {
		num++;
		if (power == COR1_HYPN_GAZE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_TELEKINES) {
		num++;
		if (power == COR1_TELEKINES)
			i = num;
	}
	if (p_ptr->muta1 & COR1_VTELEPORT) {
		num++;
		if (power == COR1_VTELEPORT)
			i = num;
	}
	if (p_ptr->muta1 & COR1_MIND_BLST) {
		num++;
		if (power == COR1_MIND_BLST)
			i = num;
	}
	if (p_ptr->muta1 & COR1_SLIME) {
		num++;
		if (power == COR1_SLIME)
			i = num;
	}
	if (p_ptr->muta1 & COR1_VAMPIRISM) {
		num++;
		if (power == COR1_VAMPIRISM)
			i = num;
	}
	if (p_ptr->muta1 & COR1_SMELL_MET) {
		num++;
		if (power == COR1_SMELL_MET)
			i = num;
	}
	if (p_ptr->muta1 & COR1_SMELL_MON) {
		num++;
		if (power == COR1_SMELL_MON)
			i = num;
	}
	if (p_ptr->muta1 & COR1_BLINK) {
		num++;
		if (power == COR1_BLINK)
			i = num;
	}
	if (p_ptr->muta1 & COR1_EAT_ROCK) {
		num++;
		if (power == COR1_EAT_ROCK)
			i = num;
	}
	if (p_ptr->muta1 & COR1_SWAP_POS) {
		num++;
		if (power == COR1_SWAP_POS)
			i = num;
	}
	if (p_ptr->muta1 & COR1_SHRIEK) {
		num++;
		if (power == COR1_SHRIEK)
			i = num;
	}
	if (p_ptr->muta1 & COR1_ILLUMINE) {
		num++;
		if (power == COR1_ILLUMINE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_DET_CURSE) {
		num++;
		if (power == COR1_DET_CURSE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_BERSERK) {
		num++;
		if (power == COR1_BERSERK)
			i = num;
	}
	if (p_ptr->muta1 & COR1_POLYMORPH) {
		num++;
		if (power == COR1_POLYMORPH)
			i = num;
	}
	if (p_ptr->muta1 & COR1_MIDAS_TCH) {
		num++;
		if (power == COR1_MIDAS_TCH)
			i = num;
	}
	if (p_ptr->muta1 & COR1_GROW_MOLD) {
		num++;
		if (power == COR1_GROW_MOLD)
			i = num;
	}
	if (p_ptr->muta1 & COR1_RESIST) {
		num++;
		if (power == COR1_RESIST)
			i = num;
	}
	if (p_ptr->muta1 & COR1_EARTHQUAKE) {
		num++;
		if (power == COR1_EARTHQUAKE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_EAT_MAGIC) {
		num++;
		if (power == COR1_EAT_MAGIC)
			i = num;
	}
	if (p_ptr->muta1 & COR1_WEIGH_MAG) {
		num++;
		if (power == COR1_WEIGH_MAG)
			i = num;
	}
	if (p_ptr->muta1 & COR1_STERILITY) {
		num++;
		if (power == COR1_STERILITY)
			i = num;
	}
	if (p_ptr->muta1 & COR1_PANIC_HIT) {
		num++;
		if (power == COR1_PANIC_HIT)
			i = num;
	}
	if (p_ptr->muta1 & COR1_DAZZLE) {
		num++;
		if (power == COR1_DAZZLE)
			i = num;
	}
	if (p_ptr->muta1 & COR1_EYE_BEAM) {
		num++;
		if (power == COR1_EYE_BEAM)
			i = num;
	}
	if (p_ptr->muta1 & COR1_RECALL) {
		num++;
		if (power == COR1_RECALL)
			i = num;
	}
	if (p_ptr->muta1 & COR1_BANISH) {
		num++;
		if (power == COR1_BANISH)
			i = num;
	}
	if (p_ptr->muta1 & COR1_COLD_TOUCH) {
		num++;
		if (power == COR1_COLD_TOUCH)
			i = num;
	}
	if (p_ptr->muta1 & COR1_LAUNCHER) {
		num++;
		if (power == COR1_LAUNCHER)
			i = num;
	}

	/* look for the mutation */

	/* Do I have the mutation and do I have a reasonable fail rate? */
	if (power == COR1_SPIT_ACID) {
		if (!(p_ptr->muta1 & COR1_SPIT_ACID))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(9, 9, A_DEX, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_BR_FIRE) {
		if (!(p_ptr->muta1 & COR1_BR_FIRE))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(20, borg_skill[BI_CLEVEL], A_CON, 18, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_HYPN_GAZE) {
		if (!(p_ptr->muta1 & COR1_HYPN_GAZE))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(12, 12, A_CHA, 18, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_VTELEPORT) {
		if (!(p_ptr->muta1 & COR1_VTELEPORT))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(7, 7, A_WIS, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_MIND_BLST) {
		if (!(p_ptr->muta1 & COR1_MIND_BLST))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(5, 3, A_WIS, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_SLIME) {
		if (!(p_ptr->muta1 & COR1_SLIME))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(15, 15, A_CON, 14, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_VAMPIRISM) {
		if (!(p_ptr->muta1 & COR1_VAMPIRISM))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(2, (1 + (borg_skill[BI_CLEVEL] / 3)), A_CON, 9,
									  fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_SMELL_MON) {
		if (!(p_ptr->muta1 & COR1_SMELL_MON))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(5, 4, A_INT, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_BLINK) {
		if (!(p_ptr->muta1 & COR1_BLINK))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(5, 4, A_INT, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_EAT_ROCK) {
		if (!(p_ptr->muta1 & COR1_EAT_ROCK))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(8, 12, A_CON, 18, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_SWAP_POS) {
		if (!(p_ptr->muta1 & COR1_SWAP_POS))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(15, 12, A_DEX, 16, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_SHRIEK) {
		if (!(p_ptr->muta1 & COR1_SHRIEK))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(20, 14, A_DEX, 16, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_ILLUMINE) {
		if (!(p_ptr->muta1 & COR1_ILLUMINE))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(3, 2, A_INT, 10, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_BERSERK) {
		if (!(p_ptr->muta1 & COR1_BERSERK))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(8, 8, A_STR, 14, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_MIDAS_TCH) {
		if (!(p_ptr->muta1 & COR1_MIDAS_TCH))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(10, 5, A_INT, 12, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_RESIST) {
		if (!(p_ptr->muta1 & COR1_RESIST))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(10, 12, A_CON, 12, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_EARTHQUAKE) {
		if (!(p_ptr->muta1 & COR1_EARTHQUAKE))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(12, 12, A_STR, 16, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_EAT_MAGIC) {
		if (!(p_ptr->muta1 & COR1_EAT_MAGIC))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(17, 1, A_WIS, 15, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_PANIC_HIT) {
		if (!(p_ptr->muta1 & COR1_PANIC_HIT))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(10, 12, A_DEX, 14, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_DAZZLE) {
		if (!(p_ptr->muta1 & COR1_DAZZLE))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(7, 15, A_CHA, 8, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_EYE_BEAM) {
		if (!(p_ptr->muta1 & COR1_EYE_BEAM))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(7, 15, A_WIS, 9, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_RECALL) {
		if (!(p_ptr->muta1 & COR1_RECALL))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(17, 50, A_INT, 16, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_BANISH) {
		if (!(p_ptr->muta1 & COR1_BANISH))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(25, 25, A_WIS, 18, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == COR1_COLD_TOUCH) {
		if (!(p_ptr->muta1 & COR1_COLD_TOUCH))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(2, 2, A_CON, 11, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}
	if (power == 3) /* COR1_LAUNCHER */
	{
		if (!(p_ptr->muta1 & COR1_LAUNCHER))
			return (FALSE);
		/* Return if just checking for legality */
		if (legal)
			return (TRUE);
		if (!borg_mutation_aux(1, borg_skill[BI_CLEVEL], A_STR, 6, fail_rate))
			return (FALSE);
		if (simulation)
			return (TRUE);
	}

	/* Attempt to initiate*/
	borg_note("# Mutation Power");
	/* Cast a spell */
	borg_keypress(ESCAPE);
	borg_keypress(ESCAPE);
	borg_keypress('U');
	borg_keypress(I2A(i - 1));

	/* Success */
	return (TRUE);
}

/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
*/
bool ang_sort_comp_string_hook(vptr u, vptr v, int a, int b) {
	cptr *text = (cptr *)(u);
	s16b *what = (s16b *)(v);

	int cmp;

	// Compare the two strings
	cmp = (strcmp(text[a], text[b]));

	// Strictly less
	if (cmp < 0)
		return (TRUE);

	// Strictly more
	if (cmp > 0)
		return (FALSE);

	// Enforce "stable" sort
	return (what[a] <= what[b]);
}

/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
*/
void ang_sort_swap_string_hook(vptr u, vptr v, int a, int b) {
	cptr *text = (cptr *)(u);
	s16b *what = (s16b *)(v);

	cptr texttmp;
	s16b whattmp;

	// Swap "text"
	texttmp = text[a];
	text[a] = text[b];
	text[b] = texttmp;

	// Swap "what"
	whattmp = what[a];
	what[a] = what[b];
	what[b] = whattmp;
}

void borg_clear_3(void) {
	C_WIPE(borg_items, INVEN_TOTAL, borg_item);
	C_WIPE(borg_shops, MAX_STORES, borg_shop);
	C_WIPE(safe_items, INVEN_TOTAL, borg_item);
	C_WIPE(safe_home, STORE_INVEN_MAX, borg_item);
	C_WIPE(safe_shops, MAX_STORES, borg_shop);
	C_WIPE(borg_plural_text, borg_plural_size, cptr);
	C_WIPE(borg_sv_plural_text, borg_plural_size, cptr);
	C_WIPE(borg_plural_what, borg_plural_size, s16b);
	C_WIPE(borg_single_text, borg_single_size, cptr);
	C_WIPE(borg_single_what, borg_single_size, s16b);
	C_WIPE(borg_artego_text, borg_artego_size, cptr);
	C_WIPE(borg_sv_art_text, borg_artego_size, cptr);
	C_WIPE(borg_artego_what, borg_artego_size, s16b);
}

/*
 * Initialize this file
 *
 * Note that all six artifact "Rings" will parse as "kind 506"
 * (the first artifact ring) and both artifact "Amulets" will
 * parse as "kind 503" (the first of the two artifact amulets),
 * but as long as we use the "name1" field (and not the "kind"
 * or "sval" fields) we should be okay.
 *
 * We sort the two arrays of items names in reverse order, so that
 * we will catch "mace of disruption" before "mace", "Scythe of
 * Slicing" before "Scythe", and for "Ring of XXX" before "Ring".
 *
 * Note that we do not have to parse "plural artifacts" (!)
 *
 * Hack -- This entire routine is a giant hack, but it works
 */
void borg_init_3(void) {
	int i, k, n;

	int size = 0;

	s16b what[523]; /* 512->523 */
	cptr text[523]; /* 512->523 */

	char buf[256];

	/*** Item/Ware arrays ***/

	/* Make the inventory array */
	C_MAKE(borg_items, INVEN_TOTAL, borg_item);

	/* Make the stores in the town */
	C_MAKE(borg_shops, MAX_STORES, borg_shop);

	/*** Item/Ware arrays (simulation) ***/

	/* Make the "safe" inventory array */
	C_MAKE(safe_items, INVEN_TOTAL, borg_item);
	C_MAKE(safe_home, STORE_INVEN_MAX, borg_item);

	/* Make the "safe" stores in the town */
	C_MAKE(safe_shops, MAX_STORES, borg_shop);

	/*** Plural Object Templates ***/

	/* Start with no objects */
	size = 0;

	/* Analyze some "item kinds" */
	for (k = 1; k < MAX_K_IDX; k++) {
		object_type hack;

		/* Get the kind */
		object_kind *k_ptr = &k_info[k];

		/* Skip "empty" items */
		if (!k_ptr->name)
			continue;

		/* Skip "gold" objects */
		if (k_ptr->tval == TV_GOLD)
			continue;

		/* Skip "artifacts" */
		if (k_ptr->flags3 & TR3_INSTA_ART)
			continue;

		/* Hack -- make an item */
		object_prep(&hack, k);

		/* Describe a "plural" object */
		hack.number = 2;
		object_desc_store(buf, &hack, FALSE, 0);

		/* Save an entry */
		text[size] = string_make(buf);
		what[size] = k;
		size++;
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_string_hook;
	ang_sort_swap = ang_sort_swap_string_hook;

	C_MAKE(borg_sv_plural_text, MAX_K_IDX, cptr);
	for (i = 0; i < size; i++) {
		borg_sv_plural_text[what[i]] = text[i];
	}
	/* Sort */
	ang_sort(text, what, size);

	/* Save the size */
	borg_plural_size = size;

	/* Allocate the "item parsing arrays" (plurals) */
	C_MAKE(borg_plural_text, borg_plural_size, cptr);
	C_MAKE(borg_plural_what, borg_plural_size, s16b);

	/* Save the entries */
	for (i = 0; i < size; i++)
		borg_plural_text[i] = text[i];
	for (i = 0; i < size; i++)
		borg_plural_what[i] = what[i];

	/*** Singular Object Templates ***/

	/* Start with no objects */
	size = 0;

	/* Analyze some "item kinds" */
	for (k = 1; k < MAX_K_IDX; k++) {
		object_type hack;

		/* Get the kind */
		object_kind *k_ptr = &k_info[k];

		/* Skip "empty" items */
		if (!k_ptr->name)
			continue;

		/* Skip "dungeon terrain" objects */
		if (k_ptr->tval == TV_GOLD)
			continue;

		/* Skip "artifacts" */
		if (k_ptr->flags3 & TR3_INSTA_ART)
			continue;

		/* Hack -- make an item */
		object_prep(&hack, k);

		/* Describe a "singular" object */
		hack.number = 1;
		object_desc_store(buf, &hack, FALSE, 0);

		/* Save an entry */
		text[size] = string_make(buf);
		what[size] = k;
		size++;
	}

	/* Analyze the "INSTA_ART" items */
	for (i = 1; i < MAX_A_IDX; i++) {
		object_type hack;

		artefact_type *a_ptr = &a_info[i];

		cptr name = (a_name + a_ptr->name);

		/* Skip "empty" items */
		if (!a_ptr->name)
			continue;

		/* Skip non INSTA_ART things */
		if (!(a_ptr->flags3 & TR3_INSTA_ART))
			continue;

		/* Extract the "kind" */
		k = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Hack -- make an item */
		object_prep(&hack, k);

		/* Save the index */
		hack.name1 = i;

		/* Describe a "singular" object */
		hack.number = 1;
		object_desc_store(buf, &hack, FALSE, 0);

		/* Extract the "suffix" length */
		n = strlen(name) + 1;

		/* Remove the "suffix" */
		buf[strlen(buf) - n] = '\0';

		/* Save an entry */
		text[size] = string_make(buf);
		what[size] = k;
		size++;
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_string_hook;
	ang_sort_swap = ang_sort_swap_string_hook;

	/* Sort */
	ang_sort(text, what, size);

	/* Save the size */
	borg_single_size = size;

	/* Allocate the "item parsing arrays" (plurals) */
	C_MAKE(borg_single_text, borg_single_size, cptr);
	C_MAKE(borg_single_what, borg_single_size, s16b);

	/* Save the entries */
	for (i = 0; i < size; i++)
		borg_single_text[i] = text[i];
	for (i = 0; i < size; i++)
		borg_single_what[i] = what[i];

	/*** Artifact and Ego-Item Parsers ***/

	/* No entries yet */
	size = 0;

	/* Collect the "artifact names" */
	for (k = 1; k < MAX_A_IDX; k++) {
		artefact_type *a_ptr = &a_info[k];

		/* Skip non-items */
		if (!a_ptr->name)
			continue;

		/* Extract a string */
		sprintf(buf, " %s", (a_name + a_ptr->name));

		/* Save an entry */
		text[size] = string_make(buf);
		what[size] = k;
		size++;
	}

	C_MAKE(borg_sv_art_text, MAX_A_IDX, cptr);
	for (i = 0; i < size; i++)
		borg_sv_art_text[what[i]] = text[i];

	/* Collect the "ego-item names" */
	for (k = 1; k < MAX_E_IDX; k++) {
		ego_item_type *e_ptr = &e_info[k];

		/* Skip non-items */
		if (!e_ptr->name)
			continue;

		/* Extract a string */
		sprintf(buf, " %s", (e_name + e_ptr->name));

		/* Save an entry */
		text[size] = string_make(buf);
		what[size] = k + 256;
		size++;
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_string_hook;
	ang_sort_swap = ang_sort_swap_string_hook;

	/* Sort */
	ang_sort(text, what, size);

	/* Save the size */
	borg_artego_size = size;

	/* Allocate the "item parsing arrays" (plurals) */
	C_MAKE(borg_artego_text, borg_artego_size, cptr);
	C_MAKE(borg_artego_what, borg_artego_size, s16b);

	/* Save the entries */
	for (i = 0; i < size; i++)
		borg_artego_text[i] = text[i];
	for (i = 0; i < size; i++)
		borg_artego_what[i] = what[i];
}

cptr borg_prt_item(int item) {
	if (item < MAX_K_IDX) {
		return borg_sv_plural_text[item];
	}
	if (item < MAX_K_IDX + MAX_K_IDX)
		return borg_sv_plural_text[item - MAX_K_IDX];
	if (item < MAX_K_IDX + MAX_K_IDX + MAX_A_IDX)
		return borg_sv_art_text[item - MAX_K_IDX - MAX_K_IDX];
	return (prefix_pref[item - MAX_K_IDX - MAX_K_IDX - MAX_A_IDX]);
}

#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
