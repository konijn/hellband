/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

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

/*
* Note that some files have their own header files
* (z-virt.h, z-util.h, z-form.h, term.h, random.h)
*/


/*
* Automatically generated "variable" declarations
*/

/* tables.c */
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern char hexsym[16];
extern byte adj_stat[][21];
extern byte blows_table[12][12];
extern owner_type owners[MAX_STORES][MAX_OWNERS];
extern u16b extract_energy[200];
extern s32b player_exp[PY_MAX_LEVEL];
extern player_sex sex_info[MAX_SEXES];
extern player_race race_info[COUNT_SUBRACES];
extern player_race sign_info[COUNT_SIGNS];
extern U_power racial_powers[];
extern U_power sign_powers[];
extern U_power freak_powers[];
extern corruption_type corruptions[COUNT_CORRUPTIONS];
extern opposed_corruptions_type opposed_corruptions[];
extern player_class class_info[MAX_CLASS];
extern class_magic realms_info[MAX_CLASS];
extern int spell_skill_mana[26][5];
extern byte spell_skill_level[50][5];
extern u32b fake_spell_flags[4];
extern u16b realm_choices[MAX_CLASS][2];
extern realm_type realm_names[];
extern spell_type spells[MAX_REALM][32];
extern byte chest_traps[64];
extern cptr player_title[MAX_CLASS][PY_MAX_LEVEL/5];
extern cptr colour_names[16];
extern cptr stat_names[STAT_COUNT];
extern cptr stat_names_reduced[STAT_COUNT];
extern cptr stats_short[STAT_COUNT];
extern cptr desc_stat_neg[STAT_COUNT];
extern cptr window_flag_desc[32];
extern option_type option_info[];
extern martial_arts ma_blows[MAX_MA];
extern mindcraft_power mindcraft_powers[MAX_MINDCRAFT_POWERS];
extern timed_type timed[];
extern cptr squelch_strings[];
extern menu_type menu_info[10][10];
extern patron_type patrons[];
extern realm_flag realm_flags[MAX_EGO_REALMS];

/* variable.c */
extern cptr copyright[5];
extern char syshelpfile[20];
extern char syshelpfile_birth[20];
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte version_extra;
extern byte sf_major;
extern byte sf_minor;
extern byte sf_patch;
extern byte sf_extra;
extern u32b sf_xtra;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern bool arg_fiddle;
extern bool arg_sound;
extern bool arg_graphics;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern bool character_generated;
extern bool character_dungeon;
extern bool character_loaded;
extern bool character_saved;
extern bool character_icky;
extern bool character_xtra;
extern u32b seed_flavor;
extern u32b seed_town;
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;
extern s16b command_new;
extern s16b energy_use;
extern s16b choose_default;
extern bool create_up_stair;
extern bool create_down_stair;
extern bool msg_flag;
extern bool alive;
extern bool death;
extern s16b running;
extern s16b resting;
extern s16b cur_hgt;
extern s16b cur_wid;
extern s16b dun_level;
extern byte came_from;
extern s16b num_repro;
extern s16b object_level;
extern s16b monster_level;
extern s32b turn;
extern s32b old_turn;
extern bool debug_mode;
extern bool use_sound;
extern bool use_graphics;
extern u16b total_winner;
extern u16b panic_save;
extern u16b noscore;
extern s16b signal_count;
extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;
extern s16b coin_type;
extern bool opening_chest;
extern bool shimmer_monsters;
extern bool shimmer_objects;
extern bool repair_monsters;
extern bool repair_objects;
extern s16b total_weight;
extern s16b inven_nxt;
extern s16b inven_cnt;
extern s16b equip_cnt;
extern s16b o_max;
extern s16b o_cnt;
extern s16b m_max;
extern s16b m_cnt;
extern s16b hack_m_idx;
extern s16b hack_m_idx_ii;
extern int total_friends;
extern s32b total_friend_levels;
extern bool multi_rew;
extern char summon_kin_type;
extern bool hack_mind;
extern bool hack_corruption;
extern bool is_autosave;
extern int artefact_bias;
extern bool rogue_like_commands;
extern bool quick_messages;
extern bool other_query_flag;
extern bool carry_query_flag;
extern bool always_pickup;
extern bool always_repeat;
extern bool use_old_target;
extern bool depth_in_feet;
extern bool use_colour;
extern bool compress_savefile;
extern bool hilite_player;
extern bool ring_bell;
extern bool find_ignore_stairs;
extern bool find_ignore_doors;
extern bool find_cut;
extern bool find_examine;
extern bool disturb_near;
extern bool disturb_move;
extern bool disturb_panel;
extern bool disturb_state;
extern bool disturb_minor;
extern bool disturb_other;
extern bool avoid_abort;
extern bool avoid_other;
extern bool flush_disturb;
extern bool flush_failure;
extern bool flush_command;
extern bool fresh_before;
extern bool fresh_after;
extern bool fresh_message;
extern bool alert_hitpoint;
extern bool alert_failure;
extern bool view_yellow_lite;
extern bool view_bright_lite;
extern bool view_granite_lite;
extern bool view_special_lite;
extern bool skip_corruptions;     /* Skip corruptions screen in 'C'haracter display */
extern bool plain_descriptions;
extern bool stupid_monsters;
extern bool auto_destroy;
extern bool wear_confirm;
extern bool confirm_stairs;
extern bool disturb_allies;
extern bool maximise_mode;
extern bool preserve_mode;
extern bool use_autoroller;
extern bool spend_points;
extern bool ironman_shop;
extern bool apply_k_storebought;
extern bool apply_k_discover;
extern bool sanity_store; /* Dont kill storebought items */
extern bool sanity_speed; /* Dont kill items giving speed bonuses*/
extern bool sanity_immune; /*  Dont kill items with immunities */
extern bool sanity_telepathy; /* Dont Kill items with telepathy*/
extern bool sanity_high_resist; /* Dont kill items with high resists ( off )*/
extern bool sanity_stat; /*Dont kill items with stat bonuses*/
extern bool sanity_verbose; /* Inform player when a sanity check is used */
extern bool sanity_realm; /* Dont kill books of the realm I use */
extern bool sanity_price;	/* Dont kill items more expensive then this */
extern bool sanity_id; /* Dont kill unknown consumables */
extern bool reverse_xp;
extern u32b sane_price;	/* Dont kill items more expensive then this, cant go negative */
extern bool use_bigtile;
extern byte squelch_options[SQ_HL_COUNT];
extern bool unify_commands;
extern bool centre_view;
extern bool no_centre_run;
extern bool view_perma_grids;
extern bool view_torch_grids;
extern bool flow_by_sound;
extern bool flow_by_smell;
extern bool track_follow;
extern bool track_target;
extern bool stack_allow_items;
extern bool stack_allow_wands;
extern bool stack_force_notes;
extern bool stack_force_costs;
extern bool view_reduce_lite;
extern bool view_reduce_view;
extern bool auto_haggle;
extern bool auto_scum;
extern bool expand_look;
extern bool expand_list;
extern bool dungeon_align;
extern bool dungeon_stair;
extern bool dungeon_small;
extern bool smart_learn;
extern bool smart_cheat;
extern bool show_labels;
extern bool show_weights;
extern bool show_choices;
extern bool show_details;
extern bool testing_stack;
extern bool monsters_carry;
extern bool debug_peek;
extern bool debug_hear;
extern bool debug_room;
extern bool debug_xtra;
extern bool debug_know;
extern bool debug_live;
extern bool debug_wild;
extern bool small_levels;
extern bool empty_levels;
extern bool player_symbols;
extern bool equippy_chars;
extern s16b hitpoint_warn;
extern s16b delay_factor;
extern s16b autosave_freq;
extern bool autosave_t;
extern bool autosave_l;
extern s16b feeling;
extern s16b rating;
extern bool good_item_flag;
extern bool new_level_flag;
extern bool closing_flag;
extern s16b max_panel_rows, max_panel_cols;
extern s16b panel_row, panel_col;
extern s16b panel_row_min, panel_row_max;
extern s16b panel_col_min, panel_col_max;
extern s16b panel_col_prt, panel_row_prt;
extern s16b py;
extern s16b px;
extern s16b wildx;
extern s16b wildy;
extern s16b target_who;
extern s16b target_col;
extern s16b target_row;
extern s16b health_who;
extern s16b monster_race_idx;
extern s16b term_k_idx;
extern int player_uid;
extern int player_euid;
extern int player_egid;
extern char player_name[40];
extern char player_base[40];
extern char died_from[80];
extern char history[4][70];
extern char savefile[1024];
extern s16b lite_n;
extern byte lite_y[LITE_MAX];
extern byte lite_x[LITE_MAX];
extern s16b view_n;
extern byte view_y[VIEW_MAX];
extern byte view_x[VIEW_MAX];
extern s16b temp_n;
extern byte temp_y[TEMP_MAX];
extern byte temp_x[TEMP_MAX];
extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;
extern s16b quark__num;
extern cptr *quark__str;
extern u16b message__next;
extern u16b message__last;
extern u16b message__head;
extern u16b message__tail;
extern u16b *message__ptr;
extern char *message__buf;
extern u32b option_flag[8];
extern u32b option_mask[8];
extern u32b window_flag[8];
extern u32b window_mask[8];
extern term *angband_term[8];
extern char angband_term_name[8][16];
extern int session_width;
extern int session_height;
extern byte angband_colour_table[256][4];
extern char angband_sound_name[SOUND_MAX][16];
extern cave_type *cave[MAX_HGT];
extern object_type *o_list;
extern monster_type *m_list;
extern monster_type *p_list;
extern quest q_list[MAX_QUESTS];  /* Heino Vander Sanden */
extern int MAX_Q_IDX;                              /* Heino Vander Sanden */
extern store_type *store;
extern object_type *inventory;
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[128];
extern char misc_to_char[128];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern cptr keymap_act[KEYMAP_MODES][256];
extern player_type p_body;
extern player_race p_race;
extern player_type *p_ptr;
extern player_sex *sp_ptr;
extern player_race *rp_ptr;
extern player_class *cp_ptr;
extern class_magic *mp_ptr;
extern player_race *bsp_ptr;
extern char short_info[25];
extern u32b spell_learned1;
extern u32b spell_learned2;
extern u32b spell_worked1;
extern u32b spell_worked2;
extern u32b spell_forgotten1;
extern u32b spell_forgotten2;
extern byte spell_order[64];
extern s16b player_hp[PY_MAX_LEVEL];
extern header *v_head;
extern vault_type *v_info;
extern char *v_name;
extern char *v_text;
extern header *f_head;
extern feature_type *f_info;
extern char *f_name;
extern char *f_text;
extern header *k_head;
extern object_kind *k_info;
extern char *k_name;
extern char *k_text;
extern header *a_head;
extern artefact_type *a_info;
extern char *a_name;
extern char *a_text;
extern header *e_head;
extern ego_item_type *e_info;
extern char *e_name;
extern char *e_text;
extern header *r_head;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern cptr ANGBAND_SYS;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_PREF;
extern cptr ANGBAND_DIR_LIB_PREF;
extern cptr ANGBAND_DIR_XTRA;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_DUMP;
extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(object_type *o_ptr);
extern bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(vptr u, vptr v, int a, int b);
extern bool (*monster_filter_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);
extern bool angband_keymap_flag; /* Hack for main-win.c */
extern bool mystic_armour_aux;
extern bool mystic_notify_aux;
extern alchemy_info potion_alch[SV_POTION_MAX];
extern u32b seed_alchemy;

/*
* Automatically generated "function declarations"
*/

/* birth.c */
extern void create_random_name( int type , byte sex , char *name );
extern void player_birth(void);
extern void outfit(object_type *q_ptr);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int y1, int x1, int y2, int x2);
extern bool player_can_see_bold(int y, int x);
extern bool cave_valid_bold(int y, int x);
extern bool no_lite(void);
#ifdef USE_TRANSPARENCY
extern void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp);
#else /* USE_TRANSPARENCY */
extern void map_info(int y, int x, byte *ap, char *cp);
#endif /* USE_TRANSPARENCY */
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern void forget_lite(void);
extern void update_lite(void);
extern void forget_view(void);
extern void update_view(void);
extern void forget_flow(void);
extern void update_flow(void);
extern void map_area(void);
extern void wiz_lite(void);
extern void wiz_dark(void);
extern void cave_set_feat(int y, int x, int feat);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int y1, int x1, int y2, int x2);
extern void scatter(int *yp, int *xp, int y, int x, int d /*int m*/);
extern void health_track(int m_idx);
extern void monster_race_track(int r_idx);
extern void object_track( object_type *o_ptr  , cptr activity );
extern void object_kind_track(int k_idx);
extern void disturb(int stop_search, int flush_output);
extern bool is_quest(int level);

/* cmd1.c */
extern bool test_hit_fire(int chance, int ac, int vis);
extern bool test_hit_norm(int chance, int ac, int vis);
extern s16b critical_shot(int weight, int plus, int dam);
extern s16b critical_norm(int weight, int plus, int dam);
extern s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr);
extern void search(void);
extern void carry(int pickup);
extern void py_attack(int y, int x);
extern void move_player(int dir, int do_pickup);
extern void run_step(int dir);
extern void term_show_most_expensive_item( int x , int y , cptr activity );

/* cmd2.c */
extern void do_cmd_go_up(void);
extern void do_cmd_go_down(void);
extern void do_cmd_search(void);
extern void do_cmd_toggle_search(void);
extern void do_cmd_open(void);
extern void do_cmd_close(void);
extern void do_cmd_tunnel(void);
extern void do_cmd_disarm(void);
extern void do_cmd_bash(void);
extern void do_cmd_alter(void);
extern void do_cmd_handle(void);
extern void do_cmd_spike(void);
extern void do_cmd_walk(int pickup);
extern void do_cmd_stay(int pickup);
extern void do_cmd_run(void);
extern void do_cmd_rest(void);
extern void do_cmd_fire(void);
extern void do_cmd_throw(void);
extern int chest_check(int y, int x);

/* cmd3.c */
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_wield(void);
extern void do_cmd_takeoff(void);
extern void do_cmd_drop(void);
extern void do_cmd_destroy(void);
extern void do_cmd_observe(void);
extern void do_cmd_destroy_all(void);
extern void do_cmd_uninscribe(void);
extern void do_cmd_inscribe(void);
extern void do_cmd_refill(int);
extern void do_cmd_target(void);
extern void do_cmd_look(void);
extern void do_cmd_locate(void);
extern void do_cmd_query_symbol(void);
extern void do_cmd_racial_power(void);
extern void destroy_pack(void);
extern void do_cmd_time(void);
extern void do_cmd_screen_dump(void);
extern void do_cmd_gain_spell(void);
extern void do_cmd_magic_spell(void);

/* cmd4.c */
extern void do_cmd_redraw(void);
extern void do_cmd_change_name(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_objects(void);
extern void do_cmd_monsters(void);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colours(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(bool);
extern void restore_screen( void );
extern void do_cmd_load_screen( cptr path , cptr file );
extern void do_cmd_save_screen(void);
extern void do_cmd_knowledge(void);
extern void do_cmd_options_aux(int,cptr);
/* cmd5.c */
extern void do_cmd_browse(int);
extern void do_cmd_study(void);
extern void do_cmd_cast(void);
extern void do_cmd_pray(void);
extern void do_cmd_mindcraft(void);
extern void do_poly_self(void);
extern void do_poly_wounds(void);

/* cmd6.c */
extern void do_cmd_eat_food(int);
extern void do_cmd_quaff_potion(int);
extern void do_cmd_read_scroll(int);
extern void do_cmd_aim_wand(int);
extern void do_cmd_use_staff(int);
extern void do_cmd_zap_rod(int);
extern void do_cmd_activate(int);
extern void do_cmd_mix(void);

/* dungeon.c */
extern void play_game(bool new_game);
extern bool psychometry(void);
extern cptr value_check_aux1(object_type *o_ptr);
extern cptr value_check_aux2(object_type *o_ptr);
extern bool has_heavy_pseudo_id(void);
extern void do_recall( cptr activation );

/* files.c */
#ifdef SET_UID
extern int player_uid;
extern int player_egid;
#endif /* SET_UID */
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern s16b tokenize(char *buf, s16b num, char **tokens);
extern void display_player(int mode);
extern errr file_character(cptr name, bool full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern void print_equippy(void);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern void read_times(void);
extern errr show_file(cptr name, cptr what);
extern errr show_highlighted_file(cptr name, cptr what, cptr *blue_highlights, cptr *green_highlights);
extern void do_cmd_help(cptr name);
extern void process_player_name(void);
extern void get_name(void);
extern void do_cmd_suicide(void);
extern void do_cmd_save_game(void);
extern long total_points(void);
extern void display_scores(int from, int to);
extern void close_game(void);
extern void exit_game_panic(void);
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);
extern errr get_rnd_line(char * file_name, char * output);
extern void race_score(int race_num);
extern void show_highclass(int pclass, int psubclasscode);
extern void html_screenshot(cptr name, int mode);
extern int calc_freak_stat_bonus(int stat);
extern void player_flags(u32b *f1, u32b *f2, u32b *f3);

/* generate.c */
extern void generate_cave(void);
void replace_secret_door(int y, int x);
extern int level_across(int, int);

/* init-txt.c */
extern errr init_v_info_txt(FILE *fp, char *buf);
extern errr init_f_info_txt(FILE *fp, char *buf);
extern errr init_k_info_txt(FILE *fp, char *buf);
extern errr init_a_info_txt(FILE *fp, char *buf);
extern errr init_e_info_txt(FILE *fp, char *buf);
extern errr init_r_info_txt(FILE *fp, char *buf);

/* init.c */
#ifdef PRIVATE_USER_PATH
  extern void create_user_dirs(void);
#endif
extern void init_file_paths( char *path ,  int attempt );
extern void init_angband();
extern cptr r_info_flags1[];
extern cptr r_info_flags2[];
extern cptr r_info_flags3[];
extern cptr r_info_flags4[];
extern cptr r_info_flags5[];
extern cptr r_info_flags6[];
extern void cleanup_angband(void);

/* load1.c */
extern errr rd_savefile_old(void);

/* load2.c */
extern errr rd_savefile_new(void);

/* melee1.c */
/* melee2.c */
extern bool make_attack_normal(int m_idx);
extern bool make_attack_spell(int m_idx);
extern void process_monsters(void);
extern void curse_equipment(int chance, int heavy_chance);

/* monster1.c */
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);
extern void display_visible(void);
extern void display_visible_items(void);

/* monster2.c */
extern s16b place_ghost(void);
extern void init_custom_race( int idx );
extern int patron_undead_servant(void);
extern int patron_demon_servant(void);
extern int patron_angel_servant(void);
extern int patron_undead_foe(void);
extern int patron_demon_foe(void);
extern int patron_angel_foe(void);
extern void delete_monster_idx(int i,bool visibly);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void remove_no_pets(void);
extern void wipe_m_list(void);
extern s16b m_pop(void);
extern errr get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void monster_desc(char *desc, monster_type *m_ptr, int mode);
extern void lore_do_probe(int m_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool full);
extern void update_monsters(bool full);
extern byte place_monster_aux(int y, int x, int r_idx, bool slp, bool grp, bool charm);
extern byte place_monster(int y, int x, bool slp, bool grp);
extern bool alloc_horde(int y, int x);
extern byte alloc_monster(int dis, int slp);
extern byte summon_specific(int y1, int x1, int lev, int type);
extern bool multiply_monster(int m_idx, bool charm, bool clone);
extern void update_smart_learn(int m_idx, int what);
extern bool summon_specific_friendly(int y1, int x1, int lev, int type, bool Group_ok);
extern bool place_monster_one(int y, int x, int r_idx, bool slp, bool charm);
extern void remove_non_pets(void);
extern bool summon_skulls(int y1, int x1);
extern int water_ok(int r_idx);
extern int can_place_monster_type( int y , int x , int summon_type);
extern int can_place_monster( int y , int x , int r_idx);
extern int can_go_monster( int y , int x , int r_idx);
extern int monster_filter_type;
extern bool monster_filter_okay(int r_idx);
extern bool is_potential_hater( monster_type *m_ptr );
extern bool is_ally( monster_type *m_ptr );
extern bool is_player_killer( monster_type *m_ptr );
extern bool is_monster_killer( monster_type *beholder , monster_type *m_ptr );
extern void set_hate_player( monster_type *m_ptr );
extern void set_ally( monster_type *m_ptr , byte ally );

/* object1.c */
extern void alchemy_init(void);
extern int scan_floor(int *items, int size, int y, int x, int mode);
extern void check_experience_obj(object_type *o_ptr);
/* object2.c */
extern void flavor_init(void);
extern void reset_visuals(void);
extern void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void object_desc(char *buf, object_type *o_ptr, int pref, int mode);
extern void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
extern bool identify_fully_aux(object_type *o_ptr);
extern s16b index_to_label(int i);
extern s16b label_to_inven(int c);
extern s16b label_to_equip(int c);
extern s16b wield_slot(object_type *o_ptr);
extern cptr mention_use(int i);
extern cptr describe_use(int i);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern void inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
extern bool inven_carry_okay(object_type *o_ptr);
extern s16b inven_carry(object_type *o_ptr, bool final);
extern s16b inven_takeoff(int item, int amt);
extern void inven_drop(int item, int amt);
extern bool item_tester_okay(object_type *o_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven(void);
extern void show_equip(void);
extern void toggle_inven_equip(void);
extern bool get_item(int *cp, cptr pmt, cptr str, int mode);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern void object_known(object_type *o_ptr, bool squelch );
extern void object_aware(object_type *o_ptr);
extern void object_full_id(object_type *o_ptr);
extern void object_storebought(object_type *o_ptr);
extern void object_tried(object_type *o_ptr);
extern s32b object_value(object_type *o_ptr);
extern s32b object_value_real(object_type *o_ptr);
extern bool object_similar(object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void object_wipe(object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_copy(object_type *o_ptr, object_type *j_ptr);
extern void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great);
extern bool make_object(object_type *j_ptr, bool good, bool great);
extern void place_object(int y, int x, bool good, bool great);
extern bool make_gold(object_type *j_ptr);
extern void place_gold(int y, int x);
extern void process_objects(void);
extern s16b drop_near(object_type *o_ptr, int chance, int y, int x);
extern void acquirement(int y1, int x1, int num, bool great);
extern void pick_trap(int y, int x);
extern void place_trap(int y, int x);
extern cptr item_activation(object_type *o_ptr);
extern void combine_pack(void);
extern void reorder_pack(void);
extern s16b spell_chance(int spell,int realm);
extern bool spell_okay(int spell, bool known, int realm);
extern void print_spells(byte *spells, int num, int y, int x, int realm);
extern void display_koff(int k_idx);
extern void random_artefact_resistance (object_type * o_ptr);
extern void alchemy_describe(char *buf, size_t max, int sval);
extern void do_squelch(void);
extern void consider_squelch( object_type *o_ptr );
extern void spell_info_short(char *p, int spell, int realm);
extern void spoil_spells( object_type *o_ptr );

/* save.c */
extern bool save_player(void);
extern bool load_player(void);

/* spells1.c */
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int dis);
extern void teleport_player_to(int ny, int nx);
extern void teleport_player_level(void);
extern void take_hit(int damage, cptr kb_str);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool inc_stat(int stat);
extern bool dec_stat(int stat, int amount, int permanent);
extern bool res_stat(int stat);
extern bool apply_disenchant(int mode);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg);
extern bool potion_smash_effect(int who, int y, int x, object_type *o_ptr, monster_type *m_ptr);
extern void get_extended_spell_info( u16b realm  , int spell , magic_type *s_ptr );

/* spells2.c */
extern bool hp_player(int num);
extern bool sp_player(int num);
extern void mind_leech(void);
extern void body_leech(void);
extern void warding_glyph(void);
extern void explosive_rune(void);
extern bool do_dec_stat(int stat);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern void message_pain(int m_idx, int dam);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
extern void self_knowledge(void);
extern bool lose_all_info(void);
extern bool detect_traps(void);
extern bool detect_doors(void);
extern bool detect_stairs(void);
extern bool detect_treasure(void);
extern bool detect_objects_gold(void);
extern bool detect_objects_normal(void);
extern bool detect_objects_magic(bool detect_entire_floor, bool do_instant_pseudo_id);
extern bool detect_monsters_normal(void);
extern bool detect_monsters_invis(void);
extern bool detect_monsters_evil(void);
extern bool charm_all_goats(void);
extern bool detect_monsters_xxx(u32b match_flag);
extern bool detect_monsters_string(cptr);
extern bool detect_monsters_nonliving(void);
extern bool detect_all(void);
extern void stair_creation(void);
extern bool wall_stone(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern bool create_artefact(object_type *o_ptr, bool a_scroll);
extern bool artefact_scroll();
extern bool ident_spell(void);
extern bool identify_fully(void);
extern bool recharge(int num);
extern bool speed_monsters(void);
extern bool slow_monsters(void);
extern bool sleep_monsters(void);
extern void aggravate_monsters(int who);
extern bool genocide(bool player_cast);
extern bool mass_genocide(bool player_cast);
extern bool probing(void);
extern bool banish_evil(int dist);
extern bool dispel_evil(int dam);
extern bool dispel_good(int dam);
extern bool dispel_undead(int dam);
extern bool dispel_monsters(int dam);
extern bool dispel_living(int dam);
extern bool dispel_demons(int dam);
extern bool dispel_devils(int dam);
extern bool dispel_fallen_angels(int dam);
extern bool turn_undead(void);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
extern void lite_room(int y1, int x1);
extern void unlite_room(int y1, int x1);
extern bool lite_area(int dam, int rad);
extern bool lite_area_hecate(int dam, int rad);
extern bool unlite_area(int dam, int rad);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool fire_bolt(int typ, int dir, int dam);
extern void call_chaos(void);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool lite_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool death_ray(int dir, int plev);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool wizard_lock(int dir);
extern bool heal_monster(int dir);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir);
extern bool stasis_monster(int dir);    /* Like sleep, affects undead as well */
extern bool confuse_monster(int dir, int plev);
extern bool stun_monster(int dir, int plev);
extern bool fear_monster(int dir, int plev);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(void);
extern bool glyph_creation(void);
extern bool destroy_doors_touch(void);
extern bool sleep_monsters_touch(void);
extern bool alchemy(void);
extern void activate_ty_curse();
extern void activate_hi_summon();
extern void summon_reaver();
extern void wall_breaker();
extern void bless_weapon();
extern bool confuse_monsters(int dam);
extern bool charm_monsters(int dam);
extern bool charm_animals(int dam);
extern bool stun_monsters(int dam);
extern bool stasis_monsters(int dam);
extern bool banish_monsters(int dist);
extern bool turn_monsters(int dam);
extern bool turn_evil(int dam);
extern bool deathray_monsters(void);
extern bool charm_monster(int dir, int plev);
extern bool charm_monster_type( int dir , int plev , int type );
extern bool control_one_undead(int dir, int plev);
extern bool charm_animal(int dir, int plev);
extern bool mindblast_monsters(int dam);
extern bool item_tester_hook_recharge(object_type *o_ptr);
extern void report_magics(void);
extern void teleport_swap(int dir);
extern void alter_reality(void);
extern void malphas_gift(void);
extern void behemoth_call(void);
extern int spell_dimensional_gate(int plev);
extern bool spell_basic_resistance(int duration);

/* store.c */
extern void do_cmd_store(void);
extern void store_shuffle(int which);
/*extern void store_maint(int which);*/
extern void store_maint_all( int times );
extern void store_init(int which);
extern void move_to_black_market(object_type * o_ptr);
extern int get_which_store(void);
extern void do_store_browse( object_type *o_ptr);
/* util.c */
extern errr path_parse(char *buf, size_t max, cptr file);
extern errr path_temp(char *buf, int max);
extern errr path_build(char *buf, int max, cptr path, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fputs(FILE *fff, cptr buf, huge n);
extern errr my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern errr fd_copy(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_chop(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);
extern void flush(void);
extern void bell(void);
extern void sound(int num);
extern void move_cursor(int row, int col);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern void keymap_init(void);
extern errr macro_add(cptr pat, cptr act);
extern sint macro_find_exact(cptr pat);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern s16b quark_add(cptr str);
extern s16b message_num(void);
extern cptr message_str(s16b age);
extern void message_add(cptr msg);
extern void msg_print(cptr msg);
extern void msg_note(cptr msg);
extern void msg_bell(cptr msg);
extern void msg_format(cptr fmt, ...);
extern void msg_fiddle(cptr fmt, ...);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern void c_roff(byte a, cptr str, int px , int py, int startx);
extern void roff(cptr str);
extern void clear_screen(void);
extern void clear_from(int row);
extern bool askfor_aux(char *buf, int len);
extern bool get_string(cptr prompt, char *buf, int len);
extern bool get_check(cptr prompt);
extern bool get_com(cptr prompt, char *command);
extern bool get_com_rep(cptr prompt, char *command);
extern s16b get_quantity(cptr prompt, int max,bool allbydefault);
extern void pause_line(int row);
extern void request_command(bool shopping);
extern bool is_a_vowel(int ch);
extern int get_keymap_dir(char ch);
extern void msg_flush_wait(void);
extern char menu_key_help(void);

/* xtra1.c */
extern void cnv_stat(int val, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern void notice_stuff(void);
extern void update_stuff(void);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void handle_stuff(void);
extern bool mystic_empty_hands();
extern bool mystic_heavy_armour();
extern void day_to_date(s16b day,char *date);
extern byte health_colour( s16b current, s16b max );

/* xtra2.c */
extern bool set_timed_effect( byte effect , int v );
extern bool set_food(int v);
extern void check_experience(void);
extern void gain_exp(s32b amount);
extern void lose_exp(s32b amount);
extern void monster_death(int m_idx);
extern bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);
extern void panel_bounds(void);
extern void panel_bounds_center(void);
extern void verify_panel(void);
extern cptr look_mon_desc(int m_idx);
extern bool ang_sort_comp_visible_hook(vptr u, vptr v, int a, int b);
extern void ang_sort_swap_visible_hook(vptr u, vptr v, int a, int b);
extern void ang_sort_swap_hook(vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_hook(vptr u, vptr v, int a, int b);
extern void ang_sort_aux(vptr u, vptr v, int p, int q);
extern void ang_sort(vptr u, vptr v, int n);
extern bool target_able(int m_idx);
extern bool target_okay(void);
extern bool target_set(int mode);
extern bool get_aim_dir(int *dp);
extern bool get_hack_dir(int *dp);
extern bool get_rep_dir(int *dp);
extern int get_evil_patron();
extern void gain_level_reward(int chosen_reward);
extern bool tgp_pt(int *x, int * y);
extern bool tgt_pt (int *x, int *y);
extern bool gain_corruption(int choose_mut);
extern void dump_corruptions(FILE *OutFile);
extern bool lose_corruption(int choose_mut);
extern u32b *corruption_idx_to_u32b(byte b);
extern reward_type patron_rewards[];
extern void calculate_xp(monster_race *r_ptr, s32b *exp , s32b *exp_frac);

/* quest.c */
extern int get_quest_monster(void);
extern int get_max_monster(void);
extern int get_quest_number(void);
extern void print_quest_message(void);
extern void quest_discovery(void);
extern int next_quest_level(void);
extern void initialise_quests();
extern int get_number_monster(int i);
extern int get_rnd_q_monster(int q_idx);
extern void player_birth_quests(void);
extern void put_quest_monster(int r_idx);

/* wizard1.c */

extern void spoiler_print_analyze_art(FILE *fff, object_type *q_ptr);

/*
* Hack -- conditional (or "bizarre") externs
*/

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifndef HAS_MEMSET
/* util.c */
extern char *memset(char*, int, huge);
#endif

#ifndef HAS_STRICMP
/* util.c */
extern int stricmp(cptr a, cptr b);
#endif

/* util.c */
#ifndef HAS_USLEEP
#  ifndef MACH_O_CARBON
extern int usleep(huge usecs);
#  endif
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void main(void); */
#endif

#ifdef WINDOWS
/* main-win.c */
/* extern int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, ...); */
#endif

/* util.c */
extern void repeat_push(int what);
extern bool repeat_pull(int *what);
extern void repeat_clear(void);
extern void repeat_push_char(char what);
extern bool repeat_pull_char(char *what);
extern void repeat_check(void);

extern char *script;
extern char variable_token[SCRIPT_MAX_LENGTH];
extern char dice_mode;
extern void eval_script(double *out);
extern void plog_fmt_fiddle(cptr fmt, ...);

/* variable.c */
extern bool easy_open;
extern cptr ANGBAND_GRAF;
extern bool arg_fiddle;
extern bool arg_wizard;
extern bool arg_sound;
extern bool arg_graphics;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern int  arg_tile_size;

extern bool easy_disarm;
extern bool reallyTRUE;
extern object_type *term_o_ptr;
extern cptr term_activity;

/* cmd2.c */
extern bool easy_open_door(int y, int x);
bool do_cmd_disarm_aux(int y, int x, int dir);
