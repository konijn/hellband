

/* File: zborg1.h */
/* Purpose: Header file for "borg1.c" -BEN- */

#ifndef INCLUDED_BORG1_H
#define INCLUDED_BORG1_H

#include "angband.h"

#ifdef ALLOW_BORG

#define PANEL_WID  33
#define PANEL_HGT  11

/* The borg_has[] needs to know certain numbers */
#define RING_FLAMES       146
#define RING_ACID         147
#define RING_ICE          148
#define SCROLL_STAR_ID    177
#define SCROLL_CURSE      180
#define SCROLL_TPORTLEVEL 187
#define SCROLL_STAR_CURSE 191
#define SCROLL_MGENOCIDE  200
#define SCROLL_GENOCIDE   207

#define POTION_STR 225
#define POTION_INT 228
#define POTION_WIS 231
#define POTION_CHR 234
#define POTION_CON 243
#define POTION_DEX 225

#define POTION_CURE_CRIT  241
#define POTION_HEAL       242
#define POTION_EXP        244
#define POTION_RES_MANA   266
#define POTION_INVULN     238
#define POTION_RESISTANCE 268

#define WAND_ANNILATION 298
#define WAND_MM         282
#define WAND_SCLOUD     290
#define WAND_TPORTOTHER 285
#define WAND_S2M        273
#define STAFF_TPORT     303
#define STAFF_POWER     324
#define STAFF_HOLY      327
#define STAFF_MAGI      325
#define ROD_RECALL      354
#define ROD_HEAL        374

#define POTION_INC_ALL   418
#define POTION_STAR_HEAL 419
#define POTION_LIFE      420

#define K_MONEY_START    480
#define K_MONEY_STOP     497

#define K_PHIAL    500
#define K_GEM      502 /* WAS K_JEWEL*/
#define K_BROKEN_A 47
#define K_BROKEN_B 30
#define K_BROKEN_C 390
/*#define K_FIGURINE			567 magical figurine */

/*#define RACE_MEPHISTOPHELES 831*/
/*
#define RACE_SAURON    858
#define RACE_MORGOTH   861
#define RACE_OBERON    860
*/
#define RACE_LILITH    593
#define RACE_LUCIFER   594

#define LILITH_DEPTH   99
#define LUCIFER_DEPTH  100

#define BORG_PIT_JUMP       10  /* minimum # of monsters LOS for DimDoor Jump */
#define ROD_HEAL_GOAL       20  /* number of rods to collect */
#define BORG_MAX_FRIENDLIES 3   /* Max number of summoned minions */


/* WARNING: If you mess with the objects.txt or the monster.txt and change
 * the locations of things, then you must make those changes in zborg.txt
 * as well as in this borg code.  The borg is very concerned about item
 * index locations.  ie: borgs_has[POTION_HEAL] is looking for a Potion of Healing.
 * The borg is concerned over several items, broken swords, and several
 * monster locations (Tarresque, Sauron, Morgoth).
 */

/*
 * This file provides support for "borg1.c".
 */



/*** Some constants ***/

/*
 * Maximum possible dungeon size
 */
#define AUTO_MAX_X  MAX_WID
#define AUTO_MAX_Y  MAX_HGT

/*
 * Threshold where the borg will start to carry / use Digger items
 */
#define BORG_DIG			45  /* a dig skill of 40 + rnd(1600) is required to dig granite */

/* The borg calculates monster flow to him.  This is the depth
 * or steps needed to reach the borg.  Some monsters do not
 * chase the borg very well, so a low number is prefered.  The
 * game default MONSTER_FLOW_DEPTH = 32
 */
#ifdef MONSTER_FLOW
	#define BORG_MON_FLOW		8
#else
	#define BORG_MON_FLOW		5
#endif


/*
 * Flags for the "info" field of grids
 *
 * Note that some of the flags below are not "perfect", in particular,
 * several of the flags should be treated as "best guesses", see below.
 *
 * The "BORG_MARK" flag means that the grid has been "observed", though
 * the terrain feature may or may not be memorized.  Note the use of the
 * "FEAT_NONE", "FEAT_FLOOR", and "FEAT_INVIS" feature codes below.
 *
 * The "BORG_GLOW" flag means that a grid is probably "perma-lit", but
 * sometimes it is really only "recently" perma-lit, but was then made
 * dark with a darkness attack, and it is now torch-lit or off-screen.
 *
 * The "BORG_DARK" flag means that a grid is probably not "perma-lit",
 * but sometimes it is really only "recently" dark, but was then made
 * "lite" with a "call lite" spell, and it is now torch-lit or off-screen.
 *
 * The "BORG_LITE" flag means that a grid is probably lit by the player
 * torch, but this may not be true if the nearby "BORG_VIEW" flags are
 * not correct, or if the "lite radius" has changed recently.
 *
 * The "BORG_VIEW" flag means that a grid is probably in line of sight
 * of the player, but this may not be true if some of the grids between
 * the player and the grid contain previously unseen walls/doors/etc.
 *
 * The "BORG_TEMP" flag means that a grid has been added to the array
 * "borg_temp_x"/"borg_temp_y", though normally we ignore this flag.
 *
 * The "BORG_XTRA" flag is used for various "extra" purposes, primarily
 * to assist with the "update_view()" code.
 */
#define BORG_MARK   0x01    /* observed grid */
#define BORG_GLOW   0x02    /* probably perma-lit */
#define BORG_DARK   0x04    /* probably not perma-lit */
#define BORG_OKAY   0x08    /* on the current panel */
#define BORG_LITE   0x10    /* lit by the torch */
#define BORG_VIEW   0x20    /* in line of sight */
#define BORG_TEMP   0x40    /* temporary flag */
#define BORG_XTRA   0x80    /* extra flag */

/*
 * Borg's Dungeon Level handling and Sea of Runes handling
 * Borg will keep track of certain types of monsters on the level.  If they meet certain
 * requirements, he will use a Sea of Runes pattern for dealing with the monster
 *
 * He also keeps track of special quest levels this way.
 */
enum {
	DEPTH_NORMAL   = 0x00,   /* Normal Level */
	DEPTH_QUEST    = 0x01,   /* Quest monster on this level */
	DEPTH_UNIQUE   = 0x02,   /* Unique on the Level */
	DEPTH_SUMMONER = 0x04,   /* The quest/unique is a summoner */
	DEPTH_BORER    = 0x08,   /* The quest/unique is a borer */
	DEPTH_VAULT    = 0x10,   /* Vault here */
	DEPTH_SCARY    = 0x20,   /* Scary guy on level */
	DEPTH_BREEDER  = 0x40    /* Level infested with breeders */
};
extern byte borg_depth;		/* Flags for special guys on the depth */

/*
 * Maximum size of the "lite" array
 */
#define AUTO_LITE_MAX 1536

/*
 * Some assistance with the borg_attack and magic arrows
 */
#define GF_ARROW_ANIMAL      92
#define GF_ARROW_EVIL        93
#define GF_ARROW_DRAGON      94
#define GF_ARROW_SLAYING     95
#define GF_ARROW_ELEC        96
#define GF_ARROW_FLAME       97
#define GF_ARROW_FROST       98
#define GF_ARROW_SEEKER      99  // Heavy ammo
#define GF_ARROW_WOUNDING    100
#define GF_HOLY_WORD         101
#define GF_DEATHRAY          104
#define GF_DISP_UNDEAD_DEMON 102 // Effect both
#define GF_ELEMENTS          103 // All elements could be cast
#define GF_AWAY_ALL_LUCIFER  105


/* the Z randarts are considered #127 by the borg */
#define ART_RANDART  127

/* Magic Numbers are Evil */
#define BORG_GSTORE    0  /* 1 General Store */
#define BORG_ARMOURY   1  /* 2 Armoury */
#define BORG_SMITH     2  /* 3 Weapon Smith */
#define BORG_TEMPLE    3  /* 4 Temple */
#define BORG_ALCHEMIST 4  /* 5 Alchemist */
#define MAGIC          5  /* 6 Magic Shop */
#define BORG_BM        6  /* 7 Black Market */
#define BORG_HOME      7  /* 8 Home */
#define BORG_BOOKSTORE 8  /* 9 Bookstore */
#define BORG_INN       9  /* + Inn */
#define BORG_HALL      10 /* + Hall */
#define BORG_BROKERS   11 /* + Pawnbrokers */
#define BORG_MAGIC2    12 /* < Mage guild second floor */
#define BORG_ALCHEMY2  13 /* > Alchemist basement */
/*
 * Maximum size of the "view" array
 */
/*#define AUTO_VIEW_MAX 1536*/
#define AUTO_VIEW_MAX 9000

/*
 * Number of grids in the "temp" array
 */
#define AUTO_TEMP_MAX 1536


/*
 * Number of grids in the "flow" array
 */
#define AUTO_FLOW_MAX 1536

/*
 * Enable the "borg_note()" usage of the Recall Window
 * Also specify the number of "rolling rows" to use
 */
#define BORG_NOTE_ROWS      12

/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 8192

/*
 * Object information
 */

typedef struct borg_take borg_take;

struct borg_take
{
	s16b k_idx; /* Kind index */
	bool known; /* Verified kind */
	bool seen;  /* Assigned motion */
	bool extra; /* Unused */
  byte x, y;  /* Location */
	s16b when;  /* When last seen */
	int  value; /* Estimated value of item */
	bool orbed; /* Orb of Draining cast on it */
	int  tval;  /* Known tval */
	int  sval;  /* Known sval */
	bool quest; /* Dropped by a quest monster */
	int  slot;  /* Assumed equip slot, if worn */

};

/*
 * Monster information
 */
typedef struct borg_kill borg_kill;

struct borg_kill
{
	s16b    r_idx;      /* Race index */

	bool    known;      /* Verified race */
	bool    awake;      /* Probably awake */

	bool    confused;   /* Probably confused */
	bool    afraid;     /* Probably afraid */
	bool    quiver;     /* Probably quivering */
	bool    stunned;
	bool    poisoned;   /* Probably poisoned */
	bool    invulner;	/* Probably Invulnerable */

	bool    seen;       /* Assigned motion */
	bool    used;       /* Assigned message */

	byte    x, y;       /* Location */

	byte    ox, oy;     /* Old location */

	s16b  dist;			/* how far from the player */
	bool  los;		/* Probably in Line of Sight */
	byte  speed;      /* Estimated speed */
	byte  moves;      /* Estimates moves */

	byte	ranged_attack; /* numbr of ranged attacks */
	byte	spell[96];	/* spell flags for monsters */
	s16b	injury;		/* Percent dead */
	s16b    power;      /* Estimated hit-points */
	s16b    other;      /* Estimated something */
	s16b    level;      /* Monsters Level */
	s16b	m_idx;		/* Game's index of monster */

	bool  ally;     /* Fights with you */
	bool  neutral;  /* Does not care either way */
	bool killer;    /* Wants to kill you */
	s16b    when;   /* When last seen */
	bool  summoner; /* Summoner */
	bool  unique;   /* Unique */
	bool  questor;  /* level questor */
	bool  avoid;    /* Do not try to hunt this guy */
	bool  cautious; /* long battle ahead with this guy */
};

/*
 * Maximum number of rooms.  This may be too small.
 * But if AUTO_ROOMS * sizeof(borg_room) > 64K then some
 * machines may not be able to allocate the room array.
 */
#define AUTO_ROOMS  (AUTO_MAX_X * AUTO_MAX_Y / 8)

/*
 * Forward declare
 */
typedef struct borg_grid borg_grid;

/*
 * A grid in the dungeon.  Several bytes.
 *
 * There is a set of eight bit flags (see below) called "info".
 *
 * There is a terrain feature type, which may be incorrect.  It is
 * more or less based on the standard "feature" values, but some of
 * the legal values are never used, such as "secret door", and others
 * are used in bizarre ways, such as "invisible trap".
 *
 * There is an object index into the "object tracking" array.
 *
 * There is a monster index into the "monster tracking" array.
 *
 * There is a byte "hmmm" which is currently unused.
 *
 * There is a byte "xtra" which tracks how much "searching" has been done
 * in the grid or in any grid next to the grid.
 *
 * To perform "navigation" from one place to another, the "flow" routines
 * are used, which place "cost" information into the "cost" fields.  Then,
 * if the path is clear, the "cost" information is copied into the "flow"
 * fields, which are used for the actual navigation.  This allows multiple
 * routines to check for "possible" flowing, without hurting the current
 * flow, which may have taken a long time to construct.  We also assume
 * that the Borg never needs to follow a path longer than 250 grids long.
 * Note that the "cost" fields have been moved into external arrays.
 *
 * Hack -- note that the "char" zero will often crash the system!
 */
struct borg_grid
{
	byte feat;      /* Grid type */
	byte info;      /* Grid flags */

	byte take;      /* Object index */
	byte kill;      /* Monster index */

	byte hmmm;      /* Extra field (unused) */

	byte xtra;      /* Extra field (search count) */

	byte t_a;			/* Attribute of the wank */
	char t_c;			/* Character of the wank */
};

/*
 * Forward declare
 */
typedef struct borg_data borg_data;

/*
 * Hack -- one byte of info per grid
 *
 * We use a structure to encapsulate the data into a "typed" form.
 */
struct borg_data
{
	byte data[AUTO_MAX_Y][AUTO_MAX_X];
};

/*** Some macros ***/

/*
 * Determine "twice" the distance between two points
 * This results in "diagonals" being "correctly" ranged,
 * that is, a diagonal appears "furthur" than an adjacent.
 */
#define double_distance(Y1,X1,Y2,X2) \
	(distance(((int)(Y1))<<1,((int)(X1))<<1,((int)(Y2))<<1,((int)(X2))<<1))

/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(Y,X)" always implies "in_bounds2(Y,X)".
 */
#define panel_fully_contains(Y,X) \
  (((Y) >= panel_row_min + 1) && ((Y) <= panel_row_max - 1) && \
   ((X) >= panel_col_min + 1) && ((X) <= panel_col_max - 1))

/*** Some variables ***/

/* This option is turned off in Hellband as a design decision, de Borg abides */
/*extern bool cheat_live = FALSE; */

/*
 * Some variables
 */
extern bool borg_active;        /* Actually active */
extern bool borg_resurrect;     /* Continous play mode */
extern bool borg_cancel;        /* Being cancelled */
extern bool borg_allies;        /* borg near allies */
extern char genocide_target;    /* Identity of the poor unsuspecting soul */
extern int zap_slot;            /* to avoid a nasty game bug with amnesia */
extern bool borg_casted_glyph;  /* we dont have a launch messages anymore */
extern int borg_stop_dlevel;
extern int borg_stop_clevel;
extern bool borg_stop_king;
extern bool borg_dont_react;
extern int successful_target;

/* options from the zborg.txt file */
extern int borg_respawn_race;
extern int borg_respawn_class;
extern int borg_respawn_str;
extern int borg_respawn_int;
extern int borg_respawn_wis;
extern int borg_respawn_dex;
extern int borg_respawn_con;
extern int borg_respawn_chr;
extern int borg_dump_level;
extern bool borg_respawn_winners;
extern int borg_respawn_realm_1;
extern int borg_respawn_realm_2;

extern int borg_delay_factor;

extern bool borg_worships_damage;
extern bool borg_worships_speed;
extern bool borg_worships_hp;
extern bool borg_worships_mana;
extern bool borg_worships_ac;
extern bool borg_worships_gold;
extern bool borg_plays_risky;
extern bool borg_uses_swaps;
extern bool borg_uses_calcs;
extern bool borg_slow_optimizehome;
extern bool borg_scums_uniques;
extern bool borg_engage_cloak;
extern int borg_chest_fail_tolerance;
extern s32b borg_money_scum_amount;
extern bool borg_scums_money;
extern bool borg_verbose; /* Chatty, gives lots of messages */
extern bool borg_lunal_mode;  /* see borg.txt */
extern bool borg_self_lunal;  /* borg allowed to do this himself */
extern bool borg_munchkin_start;
extern int borg_munchkin_level;
extern int borg_munchkin_depth;
extern int borg_enchant_limit;

/*
 * POSITION_NONE  0x00 = No special positioning
 * POSITION_SUMM  0x01 = regular Anti-Summon Corridor
 * POSITION_BORE  0x02 = Boring Position.  No Sea, just clearing out the dungeon.
 * POSITION_SEA_L 0x04 = Light Sea.  8 grid Sea of Runes
 * POSITION_DEA_D 0x08 = Deep Sea.  Summoning & Boring position.  24 grid Sea of Runes
 */
enum {
	POSITION_NONE =		0x00,
	POSITION_SUMM =		0x01,
	POSITION_BORE =		0x02,
	POSITION_SEA_L =	0x04,
	POSITION_SEA_D =	0x08,
	POSITION_SEA =		0x10
};

extern byte borg_position;

extern s16b borg_t_position;		/* timestamp when in AS position */
extern s16b borg_t_questor;

extern bool goal_fleeing_lunal; /* Fleeing level while in lunal Mode */
extern bool goal_fleeing_munchkin; /* Fleeing level while in munchkin Mode */
extern bool borg_munchkin_mode;
extern int borg_wall_buffer;  /* distance from edge when building sea of runes */
extern bool borg_digging;
extern bool borg_scumming_pots;		/* Borg will quickly store pots in home */

/*
 * Hack -- Glyph creating
 */

extern byte glyph_x;
extern byte glyph_y;
extern byte glyph_y_center;
extern byte glyph_x_center;

/* HACK... this should really be a parm into borg_prepared */
/*         I am just being lazy */
extern bool borg_slow_return;

/* dynamic required items */
/* dynamic required items */
typedef struct req_item
{
   int depth;
   int item;
   int number;

} req_item;

extern req_item *borg_required_item[MAX_CLASS];
extern int n_req[MAX_CLASS];

typedef struct power_item
{
   int depth;
   int cnd;
   int item;
   int from;
   int to;
   int power;
   bool each;
} power_item;

extern power_item *borg_power_item[MAX_CLASS];
extern int n_pwr[MAX_CLASS];
extern int *borg_has;
extern int *borg_has_on;
extern int *borg_artifact;
extern int *borg_skill;
extern int size_depth;
extern int size_obj;

/* NOTE: This must exactly match the prefix_pref enums in BORG1.c */
enum
{
	BI_STR,
	BI_INT,
	BI_WIS,
	BI_DEX,
	BI_CON,
	BI_CHR,
	BI_CSTR,
	BI_CINT,
	BI_CWIS,
	BI_CDEX,
	BI_CCON,
	BI_CCHR,
	BI_SSTR,
	BI_SINT,
	BI_SWIS,
	BI_SDEX,
	BI_SCON,
	BI_SCHR,
	BI_INTMANA,
	BI_WISMANA,
	BI_LITE,
	BI_CURHP,
	BI_MAXHP,
	BI_HP_ADJ,
	BI_OLDCHP,
	BI_CURSP,
	BI_MAXSP,
	BI_SP_ADJ,
	BI_OLDCSP,
	BI_FAIL1,
	BI_FAIL2,
	BI_REALM1,
	BI_REALM2,
	BI_CLEVEL,
	BI_MAXCLEVEL,
	BI_ESP,
	BI_CUR_LITE,
	BI_RECALL,
	BI_FOOD,
	BI_SPEED,
	BI_SDIG,
	BI_FEATH,
	BI_REG,
	BI_SINV,
	BI_INFRA,
	BI_DIS,
	BI_DEV,
	BI_SAV,
	BI_STL,
	BI_SRCH,
	BI_SRCHFREQ,
	BI_THN,
	BI_THB,
	BI_THT,
	BI_DIG,
	BI_VFIRE,
	BI_VACID,
	BI_VCOLD,
	BI_VELEC,
	BI_IFIRE,
	BI_IACID,
	BI_ICOLD,
	BI_IELEC,
	BI_TRFIRE,
	BI_TRCOLD,
	BI_TRELEC,
	BI_TRACID,
	BI_TRPOIS,
	BI_RFIRE,
	BI_RCOLD,
	BI_RELEC,
	BI_RACID,
	BI_RPOIS,
	BI_RFEAR,
	BI_RLITE,
	BI_RDARK,
	BI_RBLIND,
	BI_RCONF,
	BI_RSND,
	BI_RSHRD,
	BI_RNXUS,
	BI_RNTHR,
	BI_RKAOS,
	BI_RDIS,
	BI_REFLECT,
	BI_HLIFE,
	BI_FRACT,
	BI_SESP,
	BI_SSINV,
	BI_SRFIRE,
	BI_SRCOLD,
	BI_SRELEC,
	BI_SRACID,
	BI_SRPOIS,
	BI_SRFEAR,
	BI_SRLITE,
	BI_SRDARK,
	BI_SRBLIND,
	BI_SRCONF,
	BI_SRSND,
	BI_SRSHRD,
	BI_SRNXUS,
	BI_SRNTHR,
	BI_SRKAOS,
	BI_SRDIS,
	BI_SHLIFE,
	BI_SFRACT,
	BI_DEPTH,
	BI_CDEPTH,
	BI_MAXDEPTH,
	BI_KING,

	BI_ISWEAK,
	BI_ISHUNGRY,
	BI_ISFULL,
	BI_ISGORGED,
	BI_ISBLIND,
	BI_ISAFRAID,
	BI_ISCONFUSED,
	BI_ISPOISONED,
	BI_ISCUT,
	BI_ISSTUN,
	BI_ISHEAVYSTUN,
	BI_ISIMAGE,
	BI_ISSTUDY,
	BI_ISSEARCHING,
	BI_ISFIXLEV,
	BI_ISFIXEXP,
	BI_ISFIXSTR,
	BI_ISFIXINT,
	BI_ISFIXWIS,
	BI_ISFIXDEX,
	BI_ISFIXCON,
	BI_ISFIXCHR,
	BI_ISFIXALL,
	BI_HRTIME,
	BI_MNTIME,
	BI_FIRESH, /* Aura */
	BI_ELECSH,  /* Aura */
	BI_WRAITH,  /* Wraith form */
	BI_VAMPIRE, /* Race or mutation */
	BI_PASSWALL, /* Can move through walls */

	BI_ARMOR,
	BI_TOHIT,
	BI_TODAM,
	BI_WTOHIT,
	BI_WTODAM,
	BI_BTOHIT,
	BI_BTODAM,
	BI_BLOWS,
	BI_SHOTS,
	BI_WMAXDAM,
	BI_WBASEDAM,
	BI_BMAXDAM,
	BI_HEAVYWEPON,
	BI_HEAVYBOW,
	BI_CRSTELE,
	BI_CRSAGRV,
	BI_CRSTY,
	BI_CRSNOTELE,  /* no teleport */
	BI_CRSNOMAGIC, /* no magic */
	BI_ENCUMBERD,
	BI_FEAR_LITE,
	BI_NOEAT, /* Races do not eat food */
	BI_NO_MELEE, /* does not fight hand to hand */
	BI_WS_ANIMAL,
	BI_WS_EVIL,
	BI_WS_UNDEAD,
	BI_WS_DEMON,
	BI_WS_GIANT,
	BI_WS_DRAGON,
	BI_WK_DRAGON,
	BI_W_IMPACT,
	BI_WB_ACID,
	BI_WB_ELEC,
	BI_WB_FIRE,
	BI_WB_COLD,
	BI_WB_POIS,
	BI_WB_VORPAL,
	BI_WB_VAMPIRIC,
	BI_WB_CHAOTIC,

	BI_ATELEPORT,
	BI_AESCAPE,
	BI_ADIMDOOR,
	BI_AFUEL,
	BI_AHEAL,
	BI_AEZHEAL,
	BI_ALIFE,
	BI_AID,
	BI_ASPEED,
	BI_ASTFMAGI,
	BI_ASTFDEST,
	BI_AMISSILES,
	BI_ACUREPOIS,
	BI_ADETTRAP,
	BI_ADETDOOR,
	BI_ADETEVIL,
	BI_AMAGICMAP,
	BI_ALITE,
	BI_ARECHARGE,
	BI_APFE,
	BI_AGLYPH,
	BI_AGENOCIDE,
	BI_ACCW,
	BI_ACSW,
	BI_ACLW,
	BI_ARESHEAT,
	BI_ARESCOLD,
	BI_ARESACID,
	BI_ARESALL,
	BI_ATELEPORTLVL,  /* scroll of teleport level */
	BI_AXGOI,            /* reliable GOI spell */
	BI_AGOI,            /* GOI spell */
	BI_AHWORD,            /* Holy Word prayer */
	BI_ASTONE2MUD,
	BI_AROD1,			/* Attack Rods */
	BI_AROD2,			/* Attack Rods */
	BI_NSRANGED,		/* Attacks with missile, wand, rod */
	BI_CHEAPGLYPH,		/* Runes are cheap to cast */
	/*BI_TOWN_NUM,    Current town number */
	/*BI_ARENA_NUM,   Monster number in arena -KMW- */
	/*BI_INSIDEARENA, Is character inside arena? */
	/*BI_INSIDEQUEST, Inside quest level */
	/*BI_X_WILD,      Coordinates in the wilderness
	  BI_Y_WILD,*/
	BI_MAX
};

#define MAX_FORMULA_ELEMENTS 60
enum
{
	BFO_DONE, /* just to make sure we end fast if there is no formula */
	BFO_NUMBER,
	BFO_VARIABLE,
	BFO_EQ,
	BFO_NEQ,
	BFO_NOT,
	BFO_LT,
	BFO_LTE,
	BFO_GT,
	BFO_GTE,
	BFO_AND,
	BFO_OR,
	BFO_PLUS,
	BFO_MINUS,
	BFO_DIVIDE,
	BFO_MULT
};

extern int *formula[1000];
extern cptr prefix_pref[];

extern char **borgdat_date;
extern char **borgdat_race;
extern char **borgdat_class;
extern int  **borgdat_clevel;
extern int  **borgdat_dlevel;
extern char **borgdat_killedby;

/*
 * For borg depth preparation reports
 */
extern cptr borg_prepared[127];

/*
 * Hack -- optional cheating flags
 */

extern bool borg_do_star_id;

/*
 * Keep track of immediate threats in combat.  Used in considering swap items.
 */
extern bool borg_threat_conf;
extern bool borg_threat_blind;
extern bool borg_threat_para;
extern bool borg_threat_invis;

/*
 * Various silly flags
 */

extern bool borg_flag_save;     /* Save savefile at each level */

extern bool borg_flag_dump;     /* Save savefile at each death */

extern bool borg_save; /* do a save next time we get to press a key! */

extern bool borg_borg_message;      /* List borg messages or not */
extern bool borg_graphics;          /* List borg messages or not */
extern bool borg_confirm_target;

extern char borg_engine_date[];       /* last update */

/*
 * Use a simple internal random number generator
 */

extern bool borg_rand_quick; /* Save system setting */
extern u32b borg_rand_value; /* Save system setting */
extern u32b borg_rand_local; /* Save personal setting */

/*
 * Hack -- time variables
 */

extern s16b borg_t;             /* Current "time" */
extern s16b need_see_inviso;    /* To tell me to cast it */
extern s16b borg_see_inv;
extern bool need_shift_panel;   /* to spot offscreeners */
extern s16b when_shift_panel;
extern s16b time_this_panel;    /* Current "time" for current panel*/
extern s16b time_last_swap;
extern int unique_on_level;     /* race index of unique on the level */
extern bool borg_swapped_rings; /* rings swapped on this level */

extern s16b old_depth;
extern s16b borg_respawning;       /* to prevent certain crashes */
extern bool borg_cheat_death;		/* borg will avoid death and respawn */
extern s16b borg_no_retreat;

/*
 * Hack -- Other time variables
 */

extern s16b when_call_lite; /* When we last did call light */
extern s16b when_wizard_lite;   /* When we last did wizard light */

extern s16b when_detect_traps;  /* When we last detected traps */
extern s16b when_detect_doors;  /* When we last detected doors */
extern s16b when_detect_walls;  /* When we last detected walls */
extern s16b when_detect_evil;
extern s16b when_last_kill_mult;   /* When a multiplier was last killed */

extern bool my_need_alter;     /* incase of walls/doors */
extern bool my_no_alter;     /* incase of walls/doors */
extern bool my_need_redraw;     /* incase of walls/doors */
extern bool borg_attempting_refresh;  /* for the goi spell */

/*
 * Some information
 */

extern s16b goal;       /* Flowing (goal type) */

extern bool goal_rising;    /* Currently returning to town */
extern bool goal_leaving;   /* Currently leaving the level */

extern bool goal_fleeing;   /* Currently fleeing the level */

extern bool goal_ignoring;  /* Currently ignoring monsters */
extern bool borg_fleeing_town; /* Currently fleeing the level to return to town */

extern int goal_recalling;  /* Currently waiting for recall, guessing turns left */
extern bool goal_less;      /* return to, but dont use, the next up stairs */

extern s16b borg_times_twitch; /* how often twitchy on this level */
extern s16b borg_escapes; /* how often teleported on this level */

extern bool stair_less;     /* Use the next "up" staircase */
extern bool stair_more;     /* Use the next "down" staircase */

extern s32b borg_began;     /* When this level began */
extern s32b borg_time_town; /* how long it has been since I was in town */

extern s16b avoidance;      /* Current danger thresh-hold */

extern bool borg_failure;   /* Notice failure */

extern bool borg_simulate;  /* Simulation flag */
extern bool borg_attacking; /* Are we attacking a monster? */
extern bool borg_offsetting; /* Are we attacking a monster? with offsett balls*/

extern bool borg_completed; /* Completed the level */
extern bool borg_on_upstairs;      /* used when leaving a level */
extern bool borg_on_dnstairs;      /* used when leaving a level */
extern bool borg_needs_searching;  /* borg will search with each step */
extern bool borg_full_damage;  /* make danger = full possible damage. */
extern s16b borg_no_rest_prep; /* borg wont rest for a few turns */

/* defence flags */
extern bool borg_prot_from_evil;
extern bool borg_speed;
extern bool borg_bless;
extern bool borg_hero;
extern bool borg_berserk;
extern s16b borg_goi;
extern s16b borg_wraith;
extern s16b borg_inviso;
extern bool borg_esp;
extern s16b borg_game_ratio;
extern bool borg_shield;
extern bool borg_on_glyph; /* borg is standing on a glyph of warding */
extern bool borg_create_door; /* borg is going to create doors */
extern bool borg_sleep_spell;
extern bool borg_sleep_spell_ii;
extern bool borg_slow_spell;
extern bool borg_confuse_spell;
extern bool borg_fear_mon_spell;
extern bool borg_speed_spell;

/*
 * Shop goals
 */

extern s16b goal_shop;      /* Next shop to visit */
extern s16b goal_ware;      /* Next item to buy there */
extern s16b goal_item;      /* Next item to sell there */
extern s16b goal_qty;
extern int borg_food_onsale;      /* Are shops selling food? */
extern int sold_item_tval;
extern int sold_item_sval;
extern int sold_item_pval;
extern int sold_item_store;
extern int bought_item_tval;
extern int bought_item_sval;
extern int bought_item_pval;
extern int bought_item_store;

/*
 * Other variables
 */

extern int w_x;         /* Current panel offset (X) */
extern int w_y;         /* Current panel offset (Y) */
extern int questor_panel_y;
extern int questor_panel_x;

extern int c_x;         /* Current location (X) */
extern int c_y;         /* Current location (Y) */

extern int g_x;         /* Goal location (X) */
extern int g_y;         /* Goal location (Y) */

extern int borg_town_pref;    /* Where do I like to live? */
extern int when_waypoint[10]; /* Waypoints for moving in wilderness */
extern int store_y[FEAT_SHOP_TAIL-FEAT_SHOP_HEAD]; /* Where are the stores, y */
extern int store_x[FEAT_SHOP_TAIL-FEAT_SHOP_HEAD]; /* Where are the stores, x */

/*
 * Some estimated state variables
 */

extern s16b my_stat_max[6]; /* Current "maximal" stat values    */
extern s16b my_stat_cur[6]; /* Current "natural" stat values    */
extern s16b my_stat_use[6]; /* Current "resulting" stat values  */
extern s16b my_stat_ind[6]; /* Current "additions" to stat values   */
extern bool my_need_stat_check[6];  /* do I need to check my stats */

extern s16b my_stat_add[6];  /* aditions to stats */

extern s16b home_stat_add[6];

extern int  weapon_swap;   /* location of my swap weapon   */
extern s32b weapon_swap_value;   /* value of my swap weapon   */
extern int  armour_swap;   /* location of my swap weapon   */
extern s32b armour_swap_value;   /* value of my swap weapon   */

/* a 3 state boolean */
/*-1 = not cursed, no help needed for it */
/* 0 = light curse, needs light remove curse spell */
/* 1 = heavy curse, needs heavy remove curse spell */
extern int decurse_weapon_swap;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_h;  /* my swap is great, except its cursed */
extern int enchant_weapon_swap_to_d;  /* my swap is great, except its cursed */
extern int decurse_armour_swap;  /* my swap is great, except its cursed */
extern int enchant_armour_swap_to_a;  /* my swap is great, except its cursed */
extern bool borg_wearing_cursed;

extern s16b weapon_swap_digger;
extern int  weapon_swap_slay_animal;
extern int  weapon_swap_slay_evil;
extern int  weapon_swap_slay_undead;
extern int  weapon_swap_slay_demon;
extern int  weapon_swap_slay_giant;
extern int  weapon_swap_slay_dragon;
extern int  weapon_swap_kill_dragon;
extern int  weapon_swap_impact;
extern int  weapon_swap_brand_acid;
extern int  weapon_swap_brand_elec;
extern int  weapon_swap_brand_fire;
extern int  weapon_swap_brand_cold;
extern int  weapon_swap_see_infra;
extern int  weapon_swap_slow_digest;
extern int  weapon_swap_aggravate;
extern int  weapon_swap_teleport;
extern int  weapon_swap_regenerate;
extern int  weapon_swap_telepathy;
extern int  weapon_swap_lite;
extern int  weapon_swap_see_invis;
extern int  weapon_swap_ffall;
extern int  weapon_swap_free_act;
extern int  weapon_swap_hold_life;
extern int  weapon_swap_immune_fire;
extern int  weapon_swap_immune_acid;
extern int  weapon_swap_immune_cold;
extern int  weapon_swap_immune_elec;
extern int  weapon_swap_resist_acid;
extern int  weapon_swap_resist_elec;
extern int  weapon_swap_resist_fire;
extern int  weapon_swap_resist_cold;
extern int  weapon_swap_resist_pois;
extern int  weapon_swap_resist_conf;
extern int  weapon_swap_resist_sound;
extern int  weapon_swap_resist_lite;
extern int  weapon_swap_resist_dark;
extern int  weapon_swap_resist_chaos;
extern int  weapon_swap_resist_disen;
extern int  weapon_swap_resist_shard;
extern int  weapon_swap_resist_nexus;
extern int  weapon_swap_resist_blind;
extern int  weapon_swap_resist_neth;
extern int  weapon_swap_resist_fear;
extern int  armour_swap_slay_animal;
extern int  armour_swap_slay_evil;
extern int  armour_swap_slay_undead;
extern int  armour_swap_slay_demon;
extern int  armour_swap_slay_giant;
extern int  armour_swap_slay_dragon;
extern int  armour_swap_kill_dragon;
extern int  armour_swap_impact;
extern int  armour_swap_brand_acid;
extern int  armour_swap_brand_elec;
extern int  armour_swap_brand_fire;
extern int  armour_swap_brand_cold;
extern int  armour_swap_see_infra;
extern int  armour_swap_slow_digest;
extern int  armour_swap_aggravate;
extern int  armour_swap_teleport;
extern int  armour_swap_regenerate;
extern int  armour_swap_telepathy;
extern int  armour_swap_lite;
extern int  armour_swap_see_invis;
extern int  armour_swap_ffall;
extern int  armour_swap_free_act;
extern int  armour_swap_hold_life;
extern int  armour_swap_immune_fire;
extern int  armour_swap_immune_acid;
extern int  armour_swap_immune_cold;
extern int  armour_swap_immune_elec;
extern int  armour_swap_resist_acid;
extern int  armour_swap_resist_elec;
extern int  armour_swap_resist_fire;
extern int  armour_swap_resist_cold;
extern int  armour_swap_resist_pois;
extern int  armour_swap_resist_conf;
extern int  armour_swap_resist_sound;
extern int  armour_swap_resist_lite;
extern int  armour_swap_resist_dark;
extern int  armour_swap_resist_chaos;
extern int  armour_swap_resist_disen;
extern int  armour_swap_resist_shard;
extern int  armour_swap_resist_nexus;
extern int  armour_swap_resist_blind;
extern int  armour_swap_resist_neth;
extern int  armour_swap_resist_fear;

extern int my_ammo_tval;   /* Ammo -- "tval"   */
extern int my_ammo_sides;  /* Ammo -- "sides"  */
extern s16b my_ammo_power;  /* Shooting multipler   */
extern s16b my_ammo_range;  /* Shooting range   */

extern s16b my_need_enchant_to_a;   /* Need some enchantment */
extern s16b my_need_enchant_to_h;   /* Need some enchantment */
extern s16b my_need_enchant_to_d;   /* Need some enchantment */
extern s16b my_need_brand_weapon;   /* Brand melee weapon */
extern s16b my_need_brand_missile;   /* Brand bolts */
extern s16b my_need_id; 			/* borg needs to buy ID source */

/*
 * Hack -- basic "power"
 */

extern s32b my_power;

/*
 * Various "amounts" (for the player)
 */

extern s16b amt_phase;
extern s16b amt_food_lowcal;
extern s16b amt_food_hical;

extern s16b amt_slow_poison;
extern s16b amt_cure_confusion;
extern s16b amt_cure_blind;

extern s16b amt_cool_staff;  /* holiness-power staff */

extern s16b amt_book[MAX_REALM+1][4]; /* [realm][sval] */

extern s16b amt_add_stat[STAT_COUNT];
extern s16b amt_fix_stat[STAT_COUNT+1];

extern s16b amt_fix_exp;

extern s16b amt_enchant_to_a;
extern s16b amt_enchant_to_d;
extern s16b amt_enchant_to_h;
extern s16b amt_brand_weapon;  /* cubragol and bolts */
extern s16b amt_enchant_weapon;
extern s16b amt_enchant_armor;
extern s16b amt_digger;

/*
 * Various "amounts" (for the home)
 */

extern int num_food;
extern int num_mold;
extern int num_ident;
extern int num_star_ident;
extern int num_recall;
extern int num_phase;
extern int num_escape;
extern int num_teleport;
extern int num_berserk;
extern int num_teleport_level;

extern int num_cure_critical;
extern int num_cure_serious;

extern int num_pot_rheat;
extern int num_pot_rcold;
extern int num_resist_pot;

extern int num_missile;

extern int num_book[8][4];

extern int num_fix_stat[7];

extern int num_fix_exp;
extern int num_mana;
extern int num_heal;
extern int num_heal_true;
extern int num_ez_heal;
extern int num_ez_heal_true;
extern int num_new_life;
extern int num_pfe;
extern int num_glyph;
extern int num_speed;
extern int num_goi_pot;

extern int num_enchant_to_a;
extern int num_enchant_to_d;
extern int num_enchant_to_h;
extern int num_brand_weapon;  /*  crubragol and bolts */
extern int num_genocide;
extern int num_mass_genocide;

extern int num_artifact;
extern int borg_target_y;
extern int borg_target_x;

extern s16b home_slot_free;
extern s16b home_damage;
extern s16b num_duplicate_items;
extern int num_slow_digest;
extern int num_regenerate;
extern int num_telepathy;
extern int num_lite;
extern int num_see_inv;

extern int num_invisible; /**/

extern int num_ffall;
extern int num_free_act;
extern int num_hold_life;
extern int num_immune_acid;
extern int num_immune_elec;
extern int num_immune_fire;
extern int num_immune_cold;
extern int num_resist_acid;
extern int num_resist_elec;
extern int num_resist_fire;
extern int num_resist_cold;
extern int num_resist_pois;
extern int num_resist_conf;
extern int num_resist_sound;
extern int num_resist_lite;
extern int num_resist_dark;
extern int num_resist_chaos;
extern int num_resist_disen;
extern int num_resist_shard;
extern int num_resist_nexus;
extern int num_resist_blind;
extern int num_resist_neth;
extern int num_sustain_str;
extern int num_sustain_int;
extern int num_sustain_wis;
extern int num_sustain_dex;
extern int num_sustain_con;
extern int num_sustain_all;

extern int num_speed;
extern int num_edged_weapon;
extern int num_bad_gloves;
extern int num_weapons;
extern int num_bow;
extern int num_rings;
extern int num_neck;
extern int num_armor;
extern int num_cloaks;
extern int num_shields;
extern int num_hats;
extern int num_gloves;
extern int num_boots;


/*
 * Hack -- extra state variables
 */

extern int borg_feeling;    /* Current level "feeling" */

/*
 * Hack -- current shop index
 */

extern s16b shop_num;       /* Current shop index */

/*
 * State variables extracted from the screen
 */

extern s32b borg_exp;       /* Current experience */

extern s32b borg_gold;      /* Current gold */

extern int borg_stat[6];    /* Current stats */

extern int borg_book[8][4];   /* Current book slots, Realm,sval */

/*
 * State variables extracted from the inventory/equipment
 */

extern int borg_cur_wgt;    /* Current weight */

/*
 * Constant state variables
 */

extern int borg_race;       /* Current race */
extern int borg_class;      /* Current class */

/*
 * Constant state structures
 */

extern player_race *rb_ptr; /* Player race info */
extern player_class *cb_ptr;    /* Player class info */
extern class_magic *mp_ptr;    /* Player magic info */

/*
 * Number of turns to step for (zero means forever)
 */
extern u16b borg_step;      /* Step count (if any) */

extern void borgmove2(int *y, int *x, int y1, int x1, int y2, int x2);

/*
 * Status message search string
 */
extern char borg_match[128];    /* Search string */

/*
 * Log file
 */
extern FILE *borg_fff;      /* Log file */

/*
 * Hack -- single character constants
 */

extern const char p1, p2, c1, c2, b1, b2;

/*
 * Hack -- the detection arrays
 */

extern bool borg_detect_wall[6][6];
extern bool borg_detect_trap[6][6];
extern bool borg_detect_door[6][6];
extern bool borg_detect_evil[6][6];

extern const s16b borg_ddx_ddd[48];
extern const s16b borg_ddy_ddd[48];

/*
 * Track the cursed and quest drops
 */
extern byte *bad_obj_x;   /* Dropped cursed artifact at location (X) */
extern byte *bad_obj_y;   /* Dropped cursed artifact at location (Y) */
extern byte *good_obj_x;	/* possible quest monster drop */
extern byte *good_obj_y;
extern byte *good_obj_tval;
extern byte *good_obj_sval;
extern s16b bad_obj_num;
extern s16b bad_obj_size;
extern s16b good_obj_num;
extern s16b good_obj_size;

extern s16b borg_questor_died; /* time stamp */

/*
 * Locate the store doors
 */

extern byte track_shop_x[MAX_STORES];
extern byte track_shop_y[MAX_STORES];
/*TODO: understand the difference btween track_shop and track_building*/
extern byte track_bldg_x[MAX_STORES];
extern byte track_bldg_y[MAX_STORES];

extern byte *track_quest_x;
extern byte *track_quest_y;

/*
 * Track "stairs up"
 */

extern s16b track_less_num;
extern s16b track_less_size;
extern byte *track_less_x;
extern byte *track_less_y;

/*
 * Track "stairs down"
 */

extern s16b track_more_num;
extern s16b track_more_size;
extern byte *track_more_x;
extern byte *track_more_y;

/*
 * Track glyphs
 */
extern s16b track_glyph_num;
extern s16b track_glyph_size;
extern byte *track_glyph_x;
extern byte *track_glyph_y;
extern bool borg_needs_new_sea;

/*
 * Track the items worn to avoid loops
 */
extern s16b track_worn_num;
extern s16b track_worn_size;
extern s16b track_worn_time;
extern byte *track_worn_name1;
extern byte *track_worn_name2;
extern byte *track_worn_tval;
extern byte *track_worn_sval;
extern byte *track_worn_pval;

/*
 * Track steps
 */
extern s16b track_step_num;
extern s16b track_step_size;
extern byte *track_step_x;
extern byte *track_step_y;

/*
 * Track bad landing grids
 */
extern s16b track_land_num;
extern s16b track_land_size;
extern byte *track_land_x;
extern byte *track_land_y;
extern s16b *track_land_when;

/*
 * Track closed doors
 */
extern s16b track_door_num;
extern s16b track_door_size;
extern byte *track_door_x;
extern byte *track_door_y;

/*
 * Track closed doors which started closed
 */
extern s16b track_closed_num;
extern s16b track_closed_size;
extern byte *track_closed_x;
extern byte *track_closed_y;

/*
 * Track the mineral veins with treasure
 */
extern s16b track_vein_num;
extern s16b track_vein_size;
extern byte *track_vein_x;
extern byte *track_vein_y;

/*
 * The object list.  This list is used to "track" objects.
 */

extern s16b borg_takes_cnt;
extern s16b borg_takes_nxt;
extern borg_take *borg_takes;

/*
 * The monster list.  This list is used to "track" monsters.
 */

extern s16b borg_kills_cnt;
extern s16b borg_kills_summoner;   /* index of a summoning guy */
extern s16b borg_kills_nxt;

extern borg_kill *borg_kills;

/*
 * Hack -- depth readiness
 */
extern int borg_ready_lucifer;

/*
 * Hack -- extra fear per "region"
 */

extern u16b borg_fear_region[6][18];
extern u16b borg_fear_monsters[AUTO_MAX_Y][AUTO_MAX_X];

/*
 * Hack -- count racial appearances per level
 */

extern s16b *borg_race_count;

/*
 * Hack -- count racial kills (for uniques)
 */

extern s16b *borg_race_death;

/*
 * Classification of map symbols
 */

extern bool borg_is_take[256];      /* Symbol may be an object */
extern bool borg_is_kill[256];      /* Symbol may be a monster */

/*
 * Current "grid" list
 */

extern borg_grid *borg_grids[AUTO_MAX_Y];   /* Current "grid list" */

/*
 * Maintain a set of grids (liteable grids)
 */

extern s16b borg_lite_n;
extern byte borg_lite_y[AUTO_LITE_MAX];
extern byte borg_lite_x[AUTO_LITE_MAX];

/*
 * Maintain a set of glow grids (liteable grids)
 */

extern s16b borg_glow_n;
extern byte borg_glow_y[AUTO_LITE_MAX];
extern byte borg_glow_x[AUTO_LITE_MAX];

/*
 * Maintain a set of grids (viewable grids)
 */

extern s16b borg_view_n;
extern byte borg_view_y[AUTO_VIEW_MAX];
extern byte borg_view_x[AUTO_VIEW_MAX];

/*
 * Maintain a set of grids (scanning arrays)
 */

extern s16b borg_temp_n;
extern byte borg_temp_y[AUTO_TEMP_MAX];
extern byte borg_temp_x[AUTO_TEMP_MAX];

/*
 * Maintain a set of special grids used for Teleport Other
 */
extern s16b borg_tp_other_n;
extern byte borg_tp_other_x[255];
extern byte borg_tp_other_y[255];
extern int borg_tp_other_index[255];

/*
 * Maintain a set of special grids used for Swap Position
 */
extern bool borg_swap_pos;
extern byte borg_swap_pos_x;
extern byte borg_swap_pos_y;
extern int borg_swap_pos_index;

extern byte offset_y;
extern byte offset_x;

/*
 * Maintain a set of grids (flow calculations)
 */

extern s16b borg_flow_n;
extern byte borg_flow_y[AUTO_FLOW_MAX];
extern byte borg_flow_x[AUTO_FLOW_MAX];
extern byte borg_mflow_y[AUTO_FLOW_MAX];
extern byte borg_mflow_x[AUTO_FLOW_MAX];
extern int borg_goal_y;
extern int borg_goal_x;

/*
 * Hack -- use "flow" array as a queue
 */

extern int borg_flow_head;
extern int borg_flow_tail;

/*
 * Some variables
 */

extern borg_data *borg_data_flow;   /* Current "flow" data */
extern borg_data *borg_data_cost;   /* Current "cost" data */
extern borg_data *borg_data_cost_m;   /* Current "cost" data */
extern borg_data *borg_data_hard;   /* Constant "hard" data */
extern borg_data *borg_data_hard_m;   /* Constant "hard" data for monster flow */
extern borg_data *borg_data_know;   /* Current "know" flags */
extern borg_data *borg_data_icky;   /* Current "icky" flags */

/*
 * Strategy flags -- recalculate things
 */

extern bool borg_danger_wipe;           /* Recalculate danger */
extern bool borg_do_update_view;        /* Recalculate view */
extern bool borg_do_update_lite;        /* Recalculate lite */

/*
 * Strategy flags -- examine the world
 */

extern bool borg_do_inven;       /* Acquire "inven" info */
extern bool borg_do_equip;       /* Acquire "equip" info */
extern bool borg_do_panel;       /* Acquire "panel" info */
extern bool borg_do_frame;       /* Acquire "frame" info */
extern bool borg_do_spell;       /* Acquire "spell" info */
extern bool borg_do_browse;      /* Acquire "store" info */
extern byte borg_do_browse_what; /* Hack -- store for "borg_do_browse" */
extern byte borg_do_browse_more; /* Hack -- pages for "borg_do_browse" */

/*
 * Strategy flags -- run certain functions
 */

extern bool borg_do_crush_junk;
extern bool borg_do_crush_hole;
extern bool borg_do_crush_slow;

/* am I fighting a unique */
extern int borg_fighting_unique;
extern bool borg_fighting_evil_unique;

/* am I fighting a summoner */
extern bool borg_fighting_summoner;

/* A quest monster */
extern bool borg_fighting_questor;

/* Used with the Kill weapon brand */
extern bool borg_fighting_dragon;
extern bool borg_fighting_demon;
extern bool borg_fighting_tunneler;

extern bool borg_fighting_tele_to; /* Monster can teleport player */
extern bool borg_fighting_chaser;  /* Monster can teleport and chase the player */

/*** Some functions ***/

/*
 * Queue a keypress
 */
extern errr borg_keypress(char k);

/*
 * Queue several keypresses
 */
extern errr borg_keypresses(cptr str);

/*
 * Dequeue a keypress
 */
extern char borg_inkey(bool take);

/*
 * Flush the keypresses
 */
extern void borg_flush(void);


/*
 * Obtain some text from the screen (single character)
 */
extern errr borg_what_char(int x, int y, byte *a, char *c);

/*
 * Obtain some text from the screen (multiple characters)
 */
extern errr borg_what_text(int x, int y, int n, byte *a, char *s);

/*
 * Log a message to a file
 */
extern void borg_info(cptr what);

/*
 * Log a message, Search it, and Show/Memorize it in pieces
 */
extern void borg_note(cptr what);

/*
 * Abort the Borg, noting the reason
 */
extern void borg_oops(cptr what);

/*
 * Take a "memory note"
 */
extern bool borg_tell(cptr what);

/*
 * Change the player name
 */
extern bool borg_change_name(cptr str);

/*
 * Dump a character description
 */
extern bool borg_dump_character(cptr str);

/*
 * Save the game (but do not quit)
 */
extern bool borg_save_game(void);

extern char shop_orig[24];
extern char shop_rogue[24];
extern byte borg_nasties_num;
extern byte borg_nasties_count[10];
extern char borg_nasties[10];
extern byte borg_nasties_limit[10];
extern int borg_count_summoners;
/*
 * Update the "frame" info from the screen
 */
extern void borg_update_frame(void);

/*
 * Calc a formula out in RPN
 */
extern int borg_calc_formula(int *);
/*
 * check out a formula in RPN
 */
extern int borg_check_formula(int *);
/*
 * return a string for the formula
 */
extern cptr borg_prt_formula(int *formula);

/*
 * Print the string for an item
 */
extern cptr borg_prt_item(int item);

/*
 * Initialize this file
 */
extern void borg_init_1(void);

#ifdef ALLOW_BORG_GRAPHICS

typedef struct glyph
{
   byte d_attr;        /* Attribute */
   char d_char;        /* Character */
} glyph;

extern glyph translate_visuals[255][255];

#endif /* ALLOW_BORG_GRAPHICS */

#ifdef BORG_TK
extern cptr BORG_DIR_ROOT;
extern cptr BORG_DIR_DATA;
#endif /* BORG_TK */

#endif

/* A hackery approach to Konijn stat tables */
#define adj_str_wgt  adj_stat[ADJ_WEIGHT]
#define adj_mag_stat adj_stat[ADJ_INTWIS]
#define adj_mag_mana adj_stat[ADJ_MANA]
#define adj_str_hold adj_stat[ADJ_WEIGHT]
#define adj_dex_ta   adj_stat[ADJ_AC]
#define adj_str_td   adj_stat[ADJ_DAM]
#define adj_dex_th   adj_stat[ADJ_DEX_HIT]
#define adj_str_th   adj_stat[ADJ_STR_HIT]
#define adj_str_blow adj_stat[ADJ_STR_BLOW]
#define adj_dex_blow adj_stat[ADJ_DEX_BLOW]
#define adj_dex_dis  adj_stat[ADJ_DEX_TRAP]
#define adj_int_dis  adj_stat[ADJ_INT_TRAP]
#define adj_int_dev  adj_stat[ADJ_DEVICE]
#define adj_wis_sav  adj_stat[ADJ_RESIST]
#define adj_str_dig  adj_stat[ADJ_DIG]


/* Some more hackery*/
#define SV_FOOD_WAYBREAD SV_FOOD_AMBROSIA
#define SV_POTION_RES_CHR SV_POTION_RES_CHA
#define SV_POTION_INC_CHR SV_POTION_INC_CHA
#define RACE_HALF_GIANT GIANT
#define SV_WAND_ROCKETS SV_WAND_SHARD
#define max_f_idx MAX_F_IDX
#define max_e_idx MAX_E_IDX
#define max_k_idx MAX_K_IDX
#define max_a_idx MAX_A_IDX
#define max_r_idx MAX_R_IDX
#define FEAT_PERM_EXTRA FEAT_PERM_BUILDING
#define REALM_LIFE REALM_MIRACLES
#define RACE_SPECTRE SPECTRE
#define RACE_GNOME GNOME
#define RACE_HALF_OGRE OGRE
#define RACE_HALF_TROLL TROLL
#define RACE_SKELETON SKELETON
#define RACE_GOLEM GUARDIAN
#define RACE_BARBARIAN NORDIC
#define RACE_VAMPIRE VAMPIRE
#define RACE_KOBOLD KOBOLD
#define RACE_SPRITE FAE
#define RACE_MIND_FLAYER HORROR
#define RACE_IMP IMP
#define RACE_DWARF DWARF
#define RACE_DARK_ELF ATLANTIAN
#define CLASS_CHAOS_WARRIOR CLASS_CHAOS_KNIGHT
#define REALM_TRUMP REALM_TAROT
#define REALM_ARCANE REALM_CHARMS
#define TV_TRUMP_BOOK TV_TAROT_BOOK
#define TV_LIFE_BOOK TV_MIRACLES_BOOK
#define TV_ARCANE_BOOK TV_CHARMS_BOOK
#define EGO_TRUMP EGO_PLANAR
#define EGO_RESISTANCE EGO_HEAVEN
#define ART_GALADRIEL ART_BEATRICE
#define REALM1_BOOK realm_names[p_ptr->realm1].tval
#define REALM2_BOOK realm_names[p_ptr->realm2].tval
#define RF7_CAN_FLY RF7_FLIGHT
#define CLASS_MONK CLASS_MYSTIC
#define FEAT_BLDG_HEAD FEAT_SHOP_HEAD
#define FEAT_BLDG_TAIL FEAT_SHOP_TAIL

#endif
