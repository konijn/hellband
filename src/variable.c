/* File: variable.c */

/* Purpose: Angband variables */

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
* Hack -- Link a copyright message into the executable
*/
cptr copyright[5] =
{
	"Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
		"",
		"This software may be copied and distributed for educational, research,",
		"and not for profit purposes provided that this copyright and statement",
		"are included in all such copies."
};

/* The name of the help index file */
char syshelpfile[20];
char syshelpfile_birth[20];

/*
* Executable version
*/
byte version_major = VERSION_MAJOR;
byte version_minor = VERSION_MINOR;
byte version_patch = VERSION_PATCH;
byte version_extra = VERSION_EXTRA;

/*
* Savefile version
*/
byte sf_major;			/* Savefile's "version_major" */
byte sf_minor;			/* Savefile's "version_minor" */
byte sf_patch;			/* Savefile's "version_patch" */
byte sf_extra;			/* Savefile's "version_extra" */

/*
* Savefile information
*/
u32b sf_xtra;			/* Operating system info */
u32b sf_when;			/* Time when savefile created */
u16b sf_lives;			/* Number of past "lives" with this file */
u16b sf_saves;			/* Number of "saves" during this life */

/*
* Run-time arguments
*/
bool arg_fiddle;			/* Command arg -- Request fiddle mode */
bool arg_sound;				/* Command arg -- Request special sounds */
bool arg_graphics;			/* Command arg -- Request graphics mode */
bool arg_force_original;	/* Command arg -- Request original keyset */
bool arg_force_roguelike;	/* Command arg -- Request roguelike keyset */

/*
* Various things
*/

bool character_generated;	/* The character exists */
bool character_dungeon;		/* The character has a dungeon */
bool character_loaded;		/* The character was loaded from a savefile */
bool character_saved;		/* The character was just saved to a savefile */

bool character_icky;		/* The game is in an icky full screen mode */
bool character_xtra;		/* The game is in an icky startup mode */

u32b seed_flavor;		/* Hack -- consistent object colours */
u32b seed_town;			/* Hack -- consistent wilderness layout */
u32b seed_alchemy;		/* Hack -- consistent alchemy tables */

s16b command_cmd;		/* Current "Angband Command" */

s16b command_arg;		/* Gives argument of current command */
s16b command_rep;		/* Gives repetition of current command */
s16b command_dir;		/* Gives direction of current command */

s16b command_see;		/* See "cmd1.c" */
s16b command_wrk;		/* See "cmd1.c" */

s16b command_gap = 50;	/* See "cmd1.c" */

s16b command_new;		/* Command chaining from inven/equip view */

s16b energy_use;		/* Energy use this turn */

bool create_up_stair;	/* Auto-create "up stairs" */
bool create_down_stair;	/* Auto-create "down stairs" */

bool msg_flag;			/* Used in msg_print() for "buffering" */

bool alive;				/* True if game is running */

bool death;				/* True if player has died */

s16b running;			/* Current counter for running, if any */
s16b resting;			/* Current counter for resting, if any */

s16b cur_hgt;			/* Current dungeon height */
s16b cur_wid;			/* Current dungeon width */
s16b dun_level;			/* Current dungeon level */
s16b dun_bias;			/* Summon flag used to give the dungeon level a bias */
byte came_from;       /* Location player has come from onto this level */
s16b num_repro;			/* Current reproducer count */
s16b object_level;		/* Current object creation level */
s16b monster_level;		/* Current monster creation level */

s32b turn;				/* Current game turn */
s32b old_turn;			/* Turn when level began (feelings) */

bool debug_mode;			/* Is the player currently in Wizard mode? */

bool use_sound;			/* The "sound" mode is enabled */
bool use_graphics;		/* The "graphics" mode is enabled */

u16b total_winner;		/* Semi-Hack -- Game has been won */

u16b panic_save;		/* Track some special "conditions" */
u16b noscore;			/* Track various "Debugging" conditions */

s16b signal_count;		/* Hack -- Count interupts */

bool inkey_base;		/* See the "inkey()" function */
bool inkey_xtra;		/* See the "inkey()" function */
bool inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */

s16b coin_type;			/* Hack -- force coin type */

bool opening_chest;		/* Hack -- prevent chest generation */

bool shimmer_monsters;	/* Hack -- optimize multi-hued monsters */
bool shimmer_objects;	/* Hack -- optimize multi-hued objects */

bool repair_monsters;	/* Hack -- optimize detect monsters */
bool repair_objects;	/* Hack -- optimize detect objects */

s16b total_weight;		/* Total weight being carried */

s16b inven_nxt;			/* Hack -- unused */
bool hack_mind;
bool hack_corruption;
int artefact_bias;
bool is_autosave = FALSE;

s16b inven_cnt;			/* Number of items in inventory */
s16b equip_cnt;			/* Number of items in equipment */

s16b o_max = 1;			/* Number of allocated objects */
s16b o_cnt = 0;			/* Number of live objects */

s16b m_max = 1;			/* Number of allocated monsters */
s16b m_cnt = 0;			/* Number of live monsters */

s16b hack_m_idx = 0;	/* Hack -- see "process_monsters()" */
s16b hack_m_idx_ii = 0;
bool multi_rew = FALSE;
char summon_kin_type;   /* Hack, by Julian Lighton: summon 'relatives' */

int total_friends = 0;
s32b total_friend_levels = 0;


/*
* Software options (set via the '=' command).  See "tables.c"
*/


/* Option Set 1 -- User Interface */

bool rogue_like_commands;	/* Rogue-like commands */
bool quick_messages;		/* Activate quick messages */
bool other_query_flag;		/* Prompt for various information */
bool carry_query_flag;		/* Prompt before picking things up */
bool use_old_target;		/* Use old target by default */
bool always_pickup;			/* Pick things up by default */
bool always_repeat;			/* Repeat obvious commands */
bool depth_in_feet;			/* Show dungeon level in feet */

bool stack_force_notes;		/* Merge inscriptions when stacking */
bool stack_force_costs;		/* Merge discounts when stacking */

bool show_labels;			/* Show labels in object listings */
bool show_weights;			/* Show weights in object listings */
bool show_choices;			/* Show choices in certain sub-windows */
bool show_details;			/* Show details in certain sub-windows */

bool ring_bell;				/* Ring the bell (on errors, etc) */
bool use_colour;				/* Use colour if possible */


/* Option Set 2 -- Disturbance */

bool find_ignore_stairs;	/* Run past stairs */
bool find_ignore_doors;		/* Run through open doors */
bool find_cut;				/* Run past known corners */
bool find_examine;			/* Run into potential corners */

bool disturb_move;			/* Disturb whenever any monster moves */
bool disturb_near;			/* Disturb whenever viewable monster moves */
bool disturb_panel;			/* Disturb whenever map panel changes */
bool disturb_state;			/* Disturn whenever player state changes */
bool disturb_minor;			/* Disturb whenever boring things happen */
bool disturb_other;			/* Disturb whenever various things happen */


bool alert_hitpoint;		/* Alert user to critical hitpoints */
bool alert_failure;		/* Alert user to various failures */
bool small_levels;		/* Allow unusually small dungeon levels */
bool empty_levels;		/* Allow empty 'arena' levels */
bool player_symbols;		/* Use varying symbols for the player char */
bool equippy_chars;		/* Back by popular demand... */
bool skip_corruptions;		/* Skip corruption screen even if we have it */
bool plain_descriptions;	/* Plain object descriptions */
bool stupid_monsters;		/* Monsters use old AI */
bool auto_destroy;		/* Known worthless items are destroyed without confirmation */
bool confirm_stairs;		/* Prompt before staircases... */
bool wear_confirm;		/* Confirm before putting on known cursed items */
bool disturb_allies;		/* Allies moving nearby disturb us */
bool unify_commands; /* Combine object commands into a single 'u'se command */
bool centre_view; /* Centre view on player */
bool no_centre_run; /* Stop centring when running */
bool preserve_mode; /* Don't lose missed artefacts */
bool maximise_mode; /* Unify stat bonuses */
bool use_autoroller; /* Autoroll characters */
bool spend_points; /* Spend points on stats */
bool ironman_shop; /* Not allowed in shops */
bool apply_k_storebought;	/* Apply {!k} to storebought items */
bool apply_k_discover; /* When discovering a new consumable with 'id', offer to apply {!k} */
bool sanity_store; /* Dont kill storebought items */
bool sanity_speed; /* Dont kill items giving speed bonuses*/
bool sanity_immune; /*  Dont kill items with immunities */
bool sanity_telepathy; /* Dont Kill items with telepathy*/
bool sanity_high_resist; /* Dont kill items with high resists ( off )*/
bool sanity_stat; /*Dont kill items with stat bonuses*/
bool sanity_verbose; /* Inform player when a sanity check is used */
bool sanity_realm; /* Dont kill books of the realm I use */
bool sanity_price;	/* Dont kill items more expensive then this */
bool reverse_xp; /* Diplay xp till next level */
u32b sane_price; /* Limit price over which items will not be squelched */
bool sanity_id; /* Dont kill unknown consumables */

bool use_bigtile = FALSE; /* We aint not using bigtile yet */

byte squelch_options[SQ_HL_COUNT];

/* Option Set 3 -- Game-Play */

bool auto_haggle;			/* Auto-haggle in stores */

bool auto_scum;				/* Auto-scum for good levels */

bool stack_allow_items;		/* Allow weapons and armour to stack */
bool stack_allow_wands;		/* Allow wands/staffs/rods to stack */

bool expand_look;			/* Expand the power of the look command */
bool expand_list;			/* Expand the power of the list commands */

bool view_perma_grids;		/* Map remembers all perma-lit grids */
bool view_torch_grids;		/* Map remembers all torch-lit grids */

bool dungeon_align;			/* Generate dungeons with aligned rooms */
bool dungeon_stair;			/* Generate dungeons with connected stairs */
bool dungeon_small;         /* Generate dungeons with small levels always */

bool flow_by_sound;			/* Monsters track new player location */
bool flow_by_smell;			/* Monsters track old player location */

bool track_follow;			/* Monsters follow the player */
bool track_target;			/* Monsters target the player */

bool smart_learn;			/* Monsters learn from their mistakes */
bool smart_cheat;			/* Monsters exploit player weaknesses */


/* Option Set 4 -- Efficiency */

bool view_reduce_lite;		/* Reduce lite-radius when running */
bool view_reduce_view;		/* Reduce view-radius in town */

bool avoid_abort;			/* Avoid checking for user abort */
bool avoid_other;			/* Avoid processing special colours */

bool flush_failure;			/* Flush input on any failure */
bool flush_disturb;			/* Flush input on disturbance */
bool flush_command;			/* Flush input before every command */

bool fresh_before;			/* Flush output before normal commands */
bool fresh_after;			/* Flush output after normal commands */
bool fresh_message;			/* Flush output after all messages */

bool compress_savefile;		/* Compress messages in savefiles */

bool hilite_player;			/* Hilite the player with the cursor */

bool view_yellow_lite;		/* Use special colours for torch-lit grids */
bool view_bright_lite;		/* Use special colours for 'viewable' grids */

bool view_granite_lite;		/* Use special colours for wall grids */
bool view_special_lite;		/* Use special colours for floor grids */

/* Option set 5 -- Testing */

bool testing_stack;			/* Test the stacking code */

bool monsters_carry;			/* Test the carrying code */


/* Debugging options */

bool debug_peek;		/* Peek into object creation */
bool debug_hear;		/* Peek into monster creation */
bool debug_room;		/* Peek into dungeon creation */
bool debug_xtra;		/* Peek into something else */
bool debug_know;		/* Know complete monster info */
bool debug_live;		/* Allow player to avoid death */
bool debug_wild;        /* Peek into skill rolls */

/* Special options */

s16b hitpoint_warn = 6;		/* Hitpoint warning (0 to 9) */

s16b delay_factor = 4;		/* Delay factor (0 to 9) */

bool autosave_l;        /* Autosave before entering new levels */
bool autosave_t;        /* Timed autosave */
s16b autosave_freq;     /* Autosave frequency */


/*
* Dungeon variables
*/

s16b feeling;			/* Most recent feeling */
s16b rating;			/* Level's current rating */

bool good_item_flag;		/* True if "Artifact" on this level */

bool new_level_flag;		/* Start a new level */

bool closing_flag;		/* Dungeon is closing */


/*
* Dungeon size info
*/

s16b max_panel_rows, max_panel_cols;
s16b panel_row, panel_col;
s16b panel_row_min, panel_row_max;
s16b panel_col_min, panel_col_max;
s16b panel_col_prt, panel_row_prt;

/*
* Player location in dungeon
*/
s16b py;
s16b px;

/*
* Player location in wilderness
*/
s16b wildx;
s16b wildy;

/*
* Targetting variables
*/
s16b target_who;
s16b target_col;
s16b target_row;

/*
* Health bar variable -DRS-
*/
s16b health_who;

/*
* Monster race to track
*/
s16b monster_race_idx;

/*
* Object kind to track
* if one is set, the other should be unset
*/
s16b         term_k_idx;
object_type *term_o_ptr;
cptr         term_activity;

/*
* User info
*/
int player_uid;
int player_euid;
int player_egid;

/*
* Current player's character name
*/
char player_name[40];

/*
* Stripped version of "player_name"
*/
char player_base[40];

/*
* What killed the player
*/
char died_from[80];

/*
* Hack -- Textual "history" for the Player
*/
char history[4][70];

/*
* Buffer to hold the current savefile name
*/
char savefile[1024];


/*
* Array of grids lit by player lite (see "cave.c")
*/
s16b lite_n;
byte lite_y[LITE_MAX];
byte lite_x[LITE_MAX];

/*
* Array of grids viewable to the player (see "cave.c")
*/
s16b view_n;
byte view_y[VIEW_MAX];
byte view_x[VIEW_MAX];

/*
* Array of grids for use by various functions (see "cave.c")
*/
s16b temp_n;
byte temp_y[TEMP_MAX];
byte temp_x[TEMP_MAX];


/*
* Number of active macros.
*/
s16b macro__num;

/*
* Array of macro patterns [MACRO_MAX]
*/
cptr *macro__pat;

/*
* Array of macro actions [MACRO_MAX]
*/
cptr *macro__act;

/*
* Array of macro types [MACRO_MAX]
*/
bool *macro__cmd;

/*
* Current macro action [1024]
*/
char *macro__buf;

/*
* The number of quarks
*/
s16b quark__num;

/*
* The pointers to the quarks [QUARK_MAX]
*/
cptr *quark__str;


/*
* The next "free" index to use
*/
u16b message__next;

/*
* The index of the oldest message (none yet)
*/
u16b message__last;

/*
* The next "free" offset
*/
u16b message__head;

/*
* The offset to the oldest used char (none yet)
*/
u16b message__tail;

/*
* The array of offsets, by index [MESSAGE_MAX]
*/
u16b *message__ptr;

/*
* The array of chars, by offset [MESSAGE_BUF]
*/
char *message__buf;


/*
* The array of normal options
*/
u32b option_flag[8];
u32b option_mask[8];


/*
* The array of window options
*/
u32b window_flag[8];
u32b window_mask[8];


/*
* The array of window pointers
*/
term *angband_term[8];


/*
* Standard window names
*/
char angband_term_name[8][16] =
{
		"Hellband",
		"Mirror",
		"Recall",
		"Choice",
		"Xtra-1",
		"Xtra-2",
		"Xtra-3",
		"Xtra-4"
};

/*
* flexible term width & heigh
* let your main-xxx.c file set these variables
* for now this is only supported by main-gcu.c
*/
int session_width = 0;
int session_height = 0;

/*
* Global table of colour definitions
*/
byte angband_colour_table[256][4] =
{
	{0x00, 0x00, 0x00, 0x00},	/* TERM_DARK */
	{0x00, 0xFF, 0xFF, 0xFF},	/* TERM_WHITE */
	{0x00, 0x80, 0x80, 0x80},	/* TERM_SLATE */
	{0x00, 0xFF, 0x80, 0x00},	/* TERM_ORANGE */
	{0x00, 0xC0, 0x00, 0x00},	/* TERM_RED */
	{0x00, 0x00, 0x80, 0x40},	/* TERM_GREEN */
	{0x00, 0x00, 0x00, 0xFF},	/* TERM_BLUE */
	{0x00, 0x80, 0x40, 0x00},	/* TERM_UMBER */
	{0x00, 0x40, 0x40, 0x40},	/* TERM_L_DARK */
	{0x00, 0xC0, 0xC0, 0xC0},	/* TERM_L_WHITE */
	{0x00, 0xFF, 0x00, 0xFF},	/* TERM_VIOLET */
	{0x00, 0xFF, 0xFF, 0x00},	/* TERM_YELLOW */
	{0x00, 0xFF, 0x00, 0x00},	/* TERM_L_RED */
	{0x00, 0x00, 0xFF, 0x00},	/* TERM_L_GREEN */
	{0x00, 0x00, 0xFF, 0xFF},	/* TERM_L_BLUE */
	{0x00, 0xC0, 0x80, 0x40}	/* TERM_L_UMBER */
};


/*
* Standard sound names
*/
char angband_sound_name[SOUND_MAX][16] =
{
	"",
		"hit",
		"miss",
		"flee",
		"drop",
		"kill",
		"level",
		"death",
		"study",
		"teleport",
		"shoot",
		"quaff",
		"zap",
		"walk",
		"tpother",
		"hitwall",
		"eat",
		"store1",
		"store2",
		"store3",
		"store4",
		"dig",
		"opendoor",
		"shutdoor",
		"tplevel",
		"scroll",
		"buy",
		"sell",
		"warn"
};

/*
 * Hack -- The special Angband "Graphics Suffix"
 * This variable is used to choose an appropriate "graf-xxx" file
 */
cptr ANGBAND_GRAF = "old";

/*
 * Run-time arguments
 */
bool arg_fiddle;			/* Command arg -- Request fiddle mode */
bool arg_wizard;			/* Command arg -- Request wizard mode */
bool arg_sound;				/* Command arg -- Request special sounds */
bool arg_graphics;			/* Command arg -- Request graphics mode */
bool arg_force_original;	/* Command arg -- Request original keyset */
bool arg_force_roguelike;	/* Command arg -- Request roguelike keyset */

int arg_tile_size;          /* Windows only , Command arg - Request tilesize */

/*
* The array of "cave grids" [MAX_WID][MAX_HGT].
* Not completely allocated, that would be inefficient
* Not completely hardcoded, that would overflow memory
*/
cave_type *cave[MAX_HGT];

/*
* The array of dungeon items [MAX_O_IDX]
*/
object_type *o_list;

/*
* The array of dungeon monsters [MAX_M_IDX]
*/
monster_type *m_list;

/*
* Hack -- Quest array (Heino Vander Sanden)
*/
quest q_list[MAX_QUESTS];
int MAX_Q_IDX;


/*
* The stores [MAX_STORES_TOTAL]
*/
store_type *store;

/*
* The player's inventory [INVEN_TOTAL]
*/
object_type *inventory;


/*
* The size of "alloc_kind_table" (at most MAX_K_IDX * 4)
*/
s16b alloc_kind_size;

/*
* The entries in the "kind allocator table"
*/
alloc_entry *alloc_kind_table;


/*
* The size of "alloc_race_table" (at most MAX_R_IDX)
*/
s16b alloc_race_size;

/*
* The entries in the "race allocator table"
*/
alloc_entry *alloc_race_table;


/*
* Specify attr/char pairs for visual special effects
* Be sure to use "index & 0x7F" to avoid illegal access
*/
byte misc_to_attr[128];
char misc_to_char[128];


/*
* Specify attr/char pairs for inventory items (by tval)
* Be sure to use "index & 0x7F" to avoid illegal access
*/
byte tval_to_attr[128];
char tval_to_char[128];


/*
* Keymaps for each "mode" associated with each keypress.
*/
cptr keymap_act[KEYMAP_MODES][256];

/*** Player information ***/

/*
 * Static player info record
 * Hackery!
 */
player_type p_body;
player_type *p_ptr = &p_body;

/*
 * Static player info record
 * Hackery!
 */
player_race p_race;
player_race *rp_ptr = &p_race;

/*
* Pointer to the player tables
* (sex, race, class, magic)
*/
player_sex *sp_ptr;
player_class *cp_ptr;
class_magic *mp_ptr;
player_race *bsp_ptr;
char short_info[25];

/*
* More spell info
*/
u32b spell_learned1;	/* bit mask of spells learned */
u32b spell_learned2;	/* bit mask of spells learned */
u32b spell_worked1;	/* bit mask of spells tried and worked */
u32b spell_worked2;	/* bit mask of spells tried and worked */
u32b spell_forgotten1;	/* bit mask of spells learned but forgotten */
u32b spell_forgotten2;	/* bit mask of spells learned but forgotten */
byte spell_order[64];	/* order spells learned/remembered/forgotten */


/*
* Calculated base hp values for player at each level,
* store them so that drain life + restore life does not
* affect hit points.  Also prevents shameless use of backup
* savefiles for hitpoint acquirement.
*/
s16b player_hp[PY_MAX_LEVEL];


/*
* The vault generation arrays
*/
header *v_head;
vault_type *v_info;
char *v_name;
char *v_text;

/*
* The terrain feature arrays
*/
header *f_head;
feature_type *f_info;
char *f_name;
char *f_text;

/*
* The object kind arrays
*/
header *k_head;
object_kind *k_info;
char *k_name;
char *k_text;

/*
* The artefact arrays
*/
header *a_head;
artefact_type *a_info;
char *a_name;
char *a_text;

/*
* The ego-item arrays
*/
header *e_head;
ego_item_type *e_info;
char *e_name;
char *e_text;


/*
* The monster race arrays
*/
header *r_head;
monster_race *r_info;
char *r_name;
char *r_text;


/*
* Hack -- The special Angband "System Suffix"
* This variable is used to choose an appropriate "pref-xxx" file
*/
cptr ANGBAND_SYS = "xxx";


/*
* Path name: The main "lib" directory
* This variable is not actually used anywhere in the code
*/
cptr ANGBAND_DIR;

/*
* High score files (binary)
* These files may be portable between platforms
*/
cptr ANGBAND_DIR_APEX;

/*
 * Player dumps, screen dumps and movie dumps
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_DUMP;

/*
* Bone files for player ghosts (ascii)
* These files are portable between platforms
*/
cptr ANGBAND_DIR_BONE;

/*
* Binary image files for the "*_info" arrays (binary)
* These files are not portable between platforms
*/
cptr ANGBAND_DIR_DATA;

/*
* Textual template files for the "*_info" arrays (ascii)
* These files are portable between platforms
*/
cptr ANGBAND_DIR_EDIT;

/*
* Various extra files (ascii)
* These files may be portable between platforms
*/
cptr ANGBAND_DIR_FILE;

/*
* Help files (normal) for the online help (ascii)
* These files are portable between platforms
*/
cptr ANGBAND_DIR_HELP;

/*
* Help files (spoilers) for the online help (ascii)
* These files are portable between platforms
*/
cptr ANGBAND_DIR_INFO;

/*
* Savefiles for current characters (binary)
* These files are portable between platforms
*/
cptr ANGBAND_DIR_SAVE;

/*
* User "preference" files (ascii)
* These files are rarely portable between platforms
* THis might point to the standard lib/pref folder or the user home pref folder
*/
cptr ANGBAND_DIR_PREF;

/*
* User "preference" files (ascii)
* These files are rarely portable between platforms
* This will always point to the standard lib/pref folder
*/
cptr ANGBAND_DIR_LIB_PREF;

/*
* Various extra files (binary)
* These files are rarely portable between platforms
*/
cptr ANGBAND_DIR_XTRA;

/*
 * User defined "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_USER;

/*
* Total Hack -- allow all items to be listed (even empty ones)
* This is only used by "do_cmd_inven_e()" and is cleared there.
*/
bool item_tester_full;


/*
* Here is a "pseudo-hook" used during calls to "get_item()" and
* "show_inven()" and "show_equip()", and the choice window routines.
*/
byte item_tester_tval;


/*
* Here is a "hook" used during calls to "get_item()" and
* "show_inven()" and "show_equip()", and the choice window routines.
*/
bool (*item_tester_hook)(object_type*);



/*
* Current "comp" function for ang_sort()
*/
bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);


/*
* Current "swap" function for ang_sort()
*/
void (*ang_sort_swap)(vptr u, vptr v, int a, int b);



/*
* Hack -- function hook to restrict "get_mon_num_prep()" function
*/
bool (*monster_filter_hook)(int r_idx);



/*
* Hack -- function hook to restrict "get_obj_num_prep()" function
*/
bool (*get_obj_num_hook)(int k_idx);

/* XXX Mega-Hack - See main-win.c */
bool angband_keymap_flag = TRUE;


/* Hack, mystic armour */
bool mystic_armour_aux;
bool mystic_notify_aux;


bool easy_open = TRUE;



bool easy_disarm = TRUE;


/*
 * The alchemy information
 */
alchemy_info potion_alch[SV_POTION_MAX];

/*
 * Book swap information
 */

byte bookswap_level;

bookswap_deal bookswap_deals[10];

/*
 * Super mega hack, a variable that is true for pointer magic, brrr
 */

bool reallyTRUE = TRUE;
