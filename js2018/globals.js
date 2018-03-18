"use strict";

define(['log'],function(log){
  return {
    //File locations
    ANGBAND_DIR: undefined,
    ANGBAND_DIR_EDIT: undefined,
    ANGBAND_DIR_FILE: undefined,
    ANGBAND_DIR_HELP: undefined,
    ANGBAND_DIR_INFO: undefined,
    ANGBAND_DIR_PREF: undefined,
    ANGBAND_DIR_USER: undefined,
    ANGBAND_DIR_APEX: undefined,
    ANGBAND_DIR_BONE: undefined,
    ANGBAND_DIR_DATA: undefined,
    ANGBAND_DIR_SAVE: undefined,
    ANGBAND_DIR_DUMP: undefined,
    ANGBAND_DIR_XTRA: undefined,
    ANGBAND_SYS: undefined,
    //Runtime Parameters
    arg_fiddle: false,
    arg_sound:false,
    arg_graphics: false,
    arg_force_roguelike: false,
    arg_force_original: false,
    //Cached file contents
    fc_news:'',
    use_colour: true,
    /*
    * Angband "attributes" (with symbols, and base (R,G,B) codes)
    *
    * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
    * and should "gamma corrected" on most (non-Macintosh) machines.
    */
    TERM_DARK:    0,       /* 'd' */       /* 0,0,0 */
    TERM_WHITE:   1,       /* 'w' */       /* 4,4,4 */
    TERM_SLATE:   2,       /* 's' */       /* 2,2,2 */
    TERM_ORANGE:  3,       /* 'o' */       /* 4,2,0 */
    TERM_RED:     4,       /* 'r' */       /* 3,0,0 */
    TERM_GREEN:   5,       /* 'g' */       /* 0,2,1 */
    TERM_BLUE:    6,       /* 'b' */       /* 0,0,4 */
    TERM_UMBER:   7,       /* 'u' */       /* 2,1,0 */
    TERM_L_DARK:  8,       /* 'D' */       /* 1,1,1 */
    TERM_L_WHITE: 9,       /* 'W' */       /* 3,3,3 */
    TERM_VIOLET:  10,      /* 'v' */       /* 4,0,4 */
    TERM_YELLOW:  11,      /* 'y' */       /* 4,4,0 */
    TERM_L_RED:   12,      /* 'R' */       /* 4,0,0 */
    TERM_L_GREEN: 13,      /* 'G' */       /* 0,4,0 */
    TERM_L_BLUE:  14,      /* 'B' */       /* 0,4,4 */
    TERM_L_UMBER: 15,      /* 'U' */       /* 3,2,1 */
    VERSION: '0.8.7',
    f_info: [],
    k_info: [],
    a_info: [],
    e_info: [],
    r_info: [],
    /*
    * As of 2.7.8, the "object flags" are valid for all objects, and as
    * of 2.7.9, these flags are not actually stored with the object.
    *
    * Note that "flags1" contains all flags dependant on "pval" (including
    * stat bonuses, but NOT stat sustainers), plus all "extra attack damage"
    * flags (SLAY_XXX and BRAND_XXX).
    *
    * Note that "flags2" contains all "resistances" (including "Stat Sustainers",
    * actual immunities, and resistances).  Note that "Hold Life" is really an
    * "immunity" to ExpLoss, and "Free Action" is "immunity to paralysis".
    *
    * Note that "flags3" contains everything else -- including the three "CURSED"
    * flags, and the "BLESSED" flag, several "item display" parameters, some new
    * flags for powerful Bows, and flags which affect the player in a "general"
    * way (LITE, TELEPATHY, SEE_INVIS, SLOW_DIGEST, REGEN, FEATHER), including
    * all the "general" curses (TELEPORT, AGGRAVATE, EXP_DRAIN).  It also has
    * four new flags called "ITEM_IGNORE_XXX" which lets an item specify that
    * it can not be affected by various forms of destruction.  This is NOT as
    * powerful as actually granting resistance/immunity to the wearer.
    */
    TR1_STR                     :0x00000001,     /* STR += "pval" */
    TR1_INT                     :0x00000002,     /* INT += "pval" */
    TR1_WIS                     :0x00000004,     /* WIS += "pval" */
    TR1_DEX                     :0x00000008,     /* DEX += "pval" */
    TR1_CON                     :0x00000010,     /* CON += "pval" */
    TR1_CHA                     :0x00000020,     /* CHA += "pval" */
    TR1_XXX1                    :0x00000040,     /* Later */
    TR1_XXX2                    :0x00000080,     /* Later */
    TR1_STEALTH                 :0x00000100,     /* Stealth += "pval" */
    TR1_SEARCH                  :0x00000200,     /* Search += "pval" */
    TR1_INFRA                   :0x00000400,     /* Infra += "pval" */
    TR1_TUNNEL                  :0x00000800,     /* Tunnel += "pval" */
    TR1_SPEED                   :0x00001000,     /* Speed += "pval" */
    TR1_BLOWS                   :0x00002000,     /* Blows += "pval" */
    TR1_CHAOTIC                 :0x00004000,     /* Weapon is Chaotic */
    TR1_VAMPIRIC                :0x00008000,     /* Weapon is Vampiric */
    TR1_SLAY_ANIMAL             :0x00010000,     /* Weapon does extra damage to animals */
    TR1_SLAY_EVIL               :0x00020000,     /* Weapon does extra damage to evil */
    TR1_SLAY_UNDEAD             :0x00040000,     /* Weapon does extra damage to undead */
    TR1_SLAY_DEMON              :0x00080000,     /* Weapon does extra damage to demons */
    TR1_SLAY_ANGEL              :0x00100000,     /* Weapon does extra damage to angels */
    TR1_KILL_ANGEL              :0x00200000,     /* Weapon executes angels */
    TR1_SLAY_GIANT              :0x00400000,     /* Weapon does extra damage to giants */
    TR1_SLAY_DRAGON             :0x00800000,     /* Weapon does extra damage to dragons  */
    TR1_KILL_DRAGON             :0x01000000,     /* Weapons executes dragons */
    TR1_VORPAL                  :0x02000000,     /* Later */
    TR1_IMPACT                  :0x04000000,     /* Cause Earthquakes */
    TR1_BRAND_POIS              :0x08000000,     /* Poison branded damage */
    TR1_BRAND_ACID              :0x10000000,     /* Acid branded damage */
    TR1_BRAND_ELEC              :0x20000000,     /* Electricity branded damage */
    TR1_BRAND_FIRE              :0x40000000,     /* Fire branded damage */
    TR1_BRAND_COLD              :0x80000000,     /* Cold branded damage */
    /* Second bitmap*/
    TR2_SUST_STR                :0x00000001,     /* Sustain Strength */
    TR2_SUST_INT                :0x00000002,     /* Sustain Intelligence */
    TR2_SUST_WIS                :0x00000004,     /* Sustain Wisdom */
    TR2_SUST_DEX                :0x00000008,     /* Sustain Dexterity */
    TR2_SUST_CON                :0x00000010,     /* Sustain Constitution */
    TR2_SUST_CHA                :0x00000020,     /* Sustain Charisma */
    TR2_XXX1                    :0x00000040,     /* Later */
    TR2_XXX2                    :0x00000080,     /* Later */
    TR2_IM_ACID                 :0x00000100,     /* Immunity to acid */
    TR2_IM_ELEC                 :0x00000200,     /* Immunity to electricity */
    TR2_IM_FIRE                 :0x00000400,     /* Immunity to fire */
    TR2_IM_COLD                 :0x00000800,     /* Immunity to cold */
    TR2_XXX3                    :0x00001000,     /* Later */
    TR2_REFLECT                 :0x00002000,     /* Reflect 'bolts' */
    TR2_FREE_ACT                :0x00004000,     /* Free Action */
    TR2_HOLD_LIFE               :0x00008000,     /* Hold Life */
    TR2_RES_ACID                :0x00010000,     /* Resist Acid */
    TR2_RES_ELEC                :0x00020000,     /* Resist Electricity */
    TR2_RES_FIRE                :0x00040000,     /* Resist Fire */
    TR2_RES_COLD                :0x00080000,     /* Resist Cold */
    TR2_RES_POIS                :0x00100000,     /* Resist Poison */
    TR2_RES_FEAR                :0x00200000,     /* Resist Fear */
    TR2_RES_LITE                :0x00400000,     /* Resist Light */
    TR2_RES_DARK                :0x00800000,     /* Resist Dark */
    TR2_RES_BLIND               :0x01000000,     /* Resist Blindness */
    TR2_RES_CONF                :0x02000000,     /* Resist Confusion */
    TR2_RES_SOUND               :0x04000000,     /* Resist Sound */
    TR2_RES_SHARDS              :0x08000000,     /* Resist Shards */
    TR2_RES_NETHER              :0x10000000,     /* Resist Nether */
    TR2_RES_NEXUS               :0x20000000,     /* Resist Nexus */
    TR2_RES_CHAOS               :0x40000000,     /* Resist Chaos */
    TR2_RES_DISEN               :0x80000000,     /* Resist Disenchantment */
    /* Third bitmap*/
    TR3_SH_FIRE                 :0x00000001,     /* Immolation (Fire) */
    TR3_SH_ELEC                 :0x00000002,     /* Electric Sheath */
    TR3_XP                      :0x00000004,     /* Sentient Weapon */
    TR3_XXX4                    :0x00000008,     /* Later */
    TR3_NO_TELE                 :0x00000010,     /* Anti-teleportation */
    TR3_NO_MAGIC                :0x00000020,     /* Anti-magic */
    TR3_WRAITH                  :0x00000040,     /* Wraithform */
    TR3_TY_CURSE                :0x00000080,     /* The Ancient Curse */
    TR3_EASY_KNOW               :0x00000100,     /* Aware -> Known */
    TR3_HIDE_TYPE               :0x00000200,     /* Hide "pval" description */
    TR3_SHOW_MODS               :0x00000400,     /* Always show Tohit/Todam */
    TR3_INSTA_ART               :0x00000800,     /* Item must be an artefact */
    TR3_FEATHER                 :0x00001000,     /* Feather Falling */
    TR3_LITE                    :0x00002000,     /* Permanent Light */
    TR3_SEE_INVIS               :0x00004000,     /* See Invisible */
    TR3_TELEPATHY               :0x00008000,     /* Telepathy */
    TR3_SLOW_DIGEST             :0x00010000,     /* Item slows down digestion */
    TR3_REGEN                   :0x00020000,     /* Item induces regeneration */
    TR3_XTRA_MIGHT              :0x00040000,     /* Bows get extra multiplier */
    TR3_XTRA_SHOTS              :0x00080000,     /* Bows get extra shots */
    TR3_IGNORE_ACID             :0x00100000,     /* Item ignores Acid Damage */
    TR3_IGNORE_ELEC             :0x00200000,     /* Item ignores Elec Damage */
    TR3_IGNORE_FIRE             :0x00400000,     /* Item ignores Fire Damage */
    TR3_IGNORE_COLD             :0x00800000,     /* Item ignores Cold Damage */
    TR3_ACTIVATE                :0x01000000,     /* Item can be activated */
    TR3_DRAIN_EXP               :0x02000000,     /* Item drains Experience */
    TR3_TELEPORT                :0x04000000,     /* Item teleports player */
    TR3_AGGRAVATE               :0x08000000,     /* Item aggravates monsters */
    TR3_BLESSED                 :0x10000000,     /* Item is Blessed */
    TR3_CURSED                  :0x20000000,     /* Item is Cursed */
    TR3_HEAVY_CURSE             :0x40000000,     /* Item is Heavily Cursed */
    TR3_PERMA_CURSE             :0x80000000,     /* Item is Perma Cursed */
    /*
    * Monster blow constants
    * New monster blow methods
    */
    RBM_HIT                     :1,
    RBM_TOUCH                   :2,
    RBM_PUNCH                   :3,
    RBM_KICK                    :4,
    RBM_CLAW                    :5,
    RBM_BITE                    :6,
    RBM_STING                   :7,
    RBM_XXX1                    :8,
    RBM_BUTT                    :9,
    RBM_CRUSH                   :10,
    RBM_ENGULF                  :11,
    RBM_CHARGE                  :12,
    RBM_CRAWL                   :13,
    RBM_DROOL                   :14,
    RBM_SPIT                    :15,
    RBM_XXX3                    :16,
    RBM_GAZE                    :17,
    RBM_WAIL                    :18,
    RBM_SPORE                   :19,
    RBM_WORSHIP                 :20,
    RBM_BEG                     :21,
    RBM_INSULT                  :22,
    RBM_MOAN                    :23,
    RBM_SHOW                    :24,
    /*
    * New monster blow effects
    */
    RBE_HURT                    :1,
    RBE_POISON                  :2,
    RBE_UN_BONUS                :3,
    RBE_UN_POWER                :4,
    RBE_EAT_GOLD                :5,
    RBE_EAT_ITEM                :6,
    RBE_EAT_FOOD                :7,
    RBE_EAT_LITE                :8,
    RBE_ACID                    :9,
    RBE_ELEC                    :10,
    RBE_FIRE                    :11,
    RBE_COLD                    :12,
    RBE_BLIND                   :13,
    RBE_CONFUSE                 :14,
    RBE_TERRIFY                 :15,
    RBE_PARALYZE                :16,
    RBE_LOSE_STR                :17,
    RBE_LOSE_INT                :18,
    RBE_LOSE_WIS                :19,
    RBE_LOSE_DEX                :20,
    RBE_LOSE_CON                :21,
    RBE_LOSE_CHA                :22,
    RBE_LOSE_ALL                :23,
    RBE_SHATTER                 :24,
    RBE_EXP_10                  :25,
    RBE_EXP_20                  :26,
    RBE_EXP_40                  :27,
    RBE_EXP_80                  :28,
    /*** Monster flag values (hard-coded)
    * New monster race bit flags
    ***/
    RF1_UNIQUE                  :0x00000001,      /* Unique Monster */
    RF1_GUARDIAN                :0x00000002,      /* Level Guardian */
    RF1_MALE                    :0x00000004,      /* Male gender */
    RF1_FEMALE                  :0x00000008,      /* Female gender */
    RF1_CHAR_CLEAR              :0x00000010,      /* Absorbs symbol */
    RF1_CHAR_MULTI              :0x00000020,      /* Changes symbol */
    RF1_ATTR_CLEAR              :0x00000040,      /* Absorbs colour */
    RF1_ATTR_MULTI              :0x00000080,      /* Changes colour */
    RF1_ALWAYS_GUARD            :0x00000100,      /* Start at "correct" depth */
    RF1_FORCE_MAXHP             :0x00000200,      /* Start with max hitpoints */
    RF1_FORCE_SLEEP             :0x00000400,      /* Start out sleeping */
    RF1_FORCE_EXTRA             :0x00000800,      /* Start out something */
    RF1_FRIEND                  :0x00001000,      /* Arrive with a friend */
    RF1_FRIENDS                 :0x00002000,      /* Arrive with some friends */
    RF1_ESCORT                  :0x00004000,      /* Arrive with an escort */
    RF1_ESCORTS                 :0x00008000,      /* Arrive with some escorts */
    RF1_NEVER_BLOW              :0x00010000,      /* Never make physical blow */
    RF1_NEVER_MOVE              :0x00020000,      /* Never make physical move */
    RF1_RAND_25                 :0x00040000,      /* Moves randomly (25%) */
    RF1_RAND_50                 :0x00080000,      /* Moves randomly (50%) */
    RF1_ONLY_GOLD               :0x00100000,      /* Drop only gold */
    RF1_ONLY_ITEM               :0x00200000,      /* Drop only items */
    RF1_DROP_60                 :0x00400000,      /* Drop an item/gold (60%) */
    RF1_DROP_90                 :0x00800000,      /* Drop an item/gold (90%) */
    RF1_DROP_1D2                :0x01000000,      /* Drop 1d2 items/gold */
    RF1_DROP_2D2                :0x02000000,      /* Drop 2d2 items/gold */
    RF1_DROP_3D2                :0x04000000,      /* Drop 3d2 items/gold */
    RF1_DROP_4D2                :0x08000000,      /* Drop 4d2 items/gold */
    RF1_DROP_GOOD               :0x10000000,      /* Drop good items */
    RF1_DROP_GREAT              :0x20000000,      /* Drop great items */
    RF1_DROP_USEFUL             :0x40000000,      /* Drop "useful" items */
    RF1_DROP_CHOSEN             :0x80000000,      /* Drop "chosen" items */
    /*
    * New monster race bit flags
    */
    RF2_STUPID                  :0x00000001,      /* Monster is stupid */
    RF2_SMART                   :0x00000002,      /* Monster is smart */
    RF2_CAN_SPEAK               :0x00000004,      /* TY: can speak */
    RF2_REFLECTING              :0x00000008,      /* Reflects bolts */
    RF2_INVISIBLE               :0x00000010,      /* Monster avoids vision */
    RF2_COLD_BLOOD              :0x00000020,      /* Monster avoids infra */
    RF2_EMPTY_MIND              :0x00000040,      /* Monster avoids telepathy */
    RF2_WEIRD_MIND              :0x00000080,      /* Monster avoids telepathy? */
    RF2_MULTIPLY                :0x00000100,      /* Monster reproduces */
    RF2_REGENERATE              :0x00000200,      /* Monster regenerates */
    RF2_SHAPECHANGER            :0x00000400,      /* shapechanger */
    RF2_ATTR_ANY                :0x00000800,      /* Attr_any */
    RF2_POWERFUL                :0x00001000,      /* Monster has strong breath */
    RF2_ELDRITCH_HORROR         :0x00002000,      /* Sanity-blasting horror    */
    RF2_AURA_FIRE               :0x00004000,      /* Burns in melee */
    RF2_AURA_ELEC               :0x00008000,      /* Shocks in melee */
    RF2_OPEN_DOOR               :0x00010000,      /* Monster can open doors */
    RF2_BASH_DOOR               :0x00020000,      /* Monster can bash doors */
    RF2_PASS_WALL               :0x00040000,      /* Monster can pass walls */
    RF2_KILL_WALL               :0x00080000,      /* Monster can destroy walls */
    RF2_MOVE_BODY               :0x00100000,      /* Monster can move monsters */
    RF2_KILL_BODY               :0x00200000,      /* Monster can kill monsters */
    RF2_TAKE_ITEM               :0x00400000,      /* Monster can pick up items */
    RF2_KILL_ITEM               :0x00800000,      /* Monster can crush items */
    RF2_BRAIN_1                 :0x01000000,
    RF2_BRAIN_2                 :0x02000000,
    RF2_BRAIN_3                 :0x04000000,
    RF2_BRAIN_4                 :0x08000000,
    RF2_BRAIN_5                 :0x10000000,
    RF2_BRAIN_6                 :0x20000000,
    RF2_BRAIN_7                 :0x40000000,
    RF2_BRAIN_8                 :0x80000000,
    /*
    * New monster race bit flags
    */
    RF3_ORC                     :0x00000001      /* Orc */
    RF3_TROLL                   :0x00000002      /* Troll */
    RF3_GIANT                   :0x00000004      /* Giant */
    RF3_DRAGON                  :0x00000008      /* Dragon */
    RF3_DEMON                   :0x00000010      /* Demon */
    RF3_UNDEAD                  :0x00000020      /* Undead */
    RF3_EVIL                    :0x00000040      /* Evil */
    RF3_ANIMAL                  :0x00000080      /* Animal */
    RF3_FALLEN_ANGEL            :0x00000100      /* TY: fallen angel */
    RF3_GOOD                    :0x00000200      /* Good */
    RF3_PLAYER_GHOST            :0x00000400      /* Monster is a player ghost */
    RF3_NONLIVING               :0x00000800      /* TY: Non-Living (?) */
    RF3_HURT_LITE               :0x00001000      /* Hurt by lite */
    RF3_HURT_ROCK               :0x00002000      /* Hurt by rock remover */
    RF3_HURT_FIRE               :0x00004000      /* Hurt badly by fire */
    RF3_HURT_COLD               :0x00008000      /* Hurt badly by cold */
    RF3_IM_ACID                 :0x00010000      /* Resist acid a lot */
    RF3_IM_ELEC                 :0x00020000      /* Resist elec a lot */
    RF3_IM_FIRE                 :0x00040000      /* Resist fire a lot */
    RF3_IM_COLD                 :0x00080000      /* Resist cold a lot */
    RF3_IM_POIS                 :0x00100000      /* Resist poison a lot */
    RF3_RES_TELE                :0x00200000      /* Resist teleportation */
    RF3_RES_NETH                :0x00400000      /* Resist nether a lot */
    RF3_RES_WATE                :0x00800000      /* Resist water */
    RF3_RES_PLAS                :0x01000000      /* Resist plasma */
    RF3_RES_NEXU                :0x02000000      /* Resist nexus */
    RF3_RES_DISE                :0x04000000      /* Resist disenchantment */
    RF3_DEVIL                   :0x08000000      /* devil */
    RF3_NO_FEAR                 :0x10000000      /* Cannot be scared */
    RF3_NO_STUN                 :0x20000000      /* Cannot be stunned */
    RF3_NO_CONF                 :0x40000000      /* Cannot be confused */
    RF3_NO_SLEEP                :0x80000000      /* Cannot be slept */

/*
* New monster race bit flags
*/
    RF4_SHRIEK                  :0x00000001,      /* Shriek for help */
    RF4_CUSTOM                  :0x00000002,      /* (?) */
    RF4_XXX3                    :0x00000004,      /* (?) */
    RF4_SHARD                   :0x00000008,      /* Fire shard balls */
    RF4_ARROW_1                 :0x00000010,      /* Fire an arrow (light) */
    RF4_ARROW_2                 :0x00000020,      /* Fire an arrow (heavy) */
    RF4_ARROW_3                 :0x00000040,      /* Fire missiles (light) */
    RF4_ARROW_4                 :0x00000080,      /* Fire missiles (heavy) */
    RF4_BR_ACID                 :0x00000100,      /* Breathe Acid */
    RF4_BR_ELEC                 :0x00000200,      /* Breathe Elec */
    RF4_BR_FIRE                 :0x00000400,      /* Breathe Fire */
    RF4_BR_COLD                 :0x00000800,      /* Breathe Cold */
    RF4_BR_POIS                 :0x00001000,      /* Breathe Poison */
    RF4_BR_NETH                 :0x00002000,      /* Breathe Nether */
    RF4_BR_LITE                 :0x00004000,      /* Breathe Lite */
    RF4_BR_DARK                 :0x00008000,      /* Breathe Dark */
    RF4_BR_CONF                 :0x00010000,      /* Breathe Confusion */
    RF4_BR_SOUN                 :0x00020000,      /* Breathe Sound */
    RF4_BR_CHAO                 :0x00040000,      /* Breathe Primal Chaos */
    RF4_BR_DISE                 :0x00080000,      /* Breathe Disenchant */
    RF4_BR_NEXU                 :0x00100000,      /* Breathe Nexus */
    RF4_BR_TIME                 :0x00200000,      /* Breathe Time */
    RF4_BR_INER                 :0x00400000,      /* Breathe Inertia */
    RF4_BR_GRAV                 :0x00800000,      /* Breathe Gravity */
    RF4_BR_SHAR                 :0x01000000,      /* Breathe Shards */
    RF4_BR_PLAS                 :0x02000000,      /* Breathe Plasma */
    RF4_BR_WALL                 :0x04000000,      /* Breathe Force */
    RF4_BR_MANA                 :0x08000000,      /* Breathe Mana */
    RF4_BA_SLIM                 :0x10000000,      /* TY: Hell slime */
    RF4_BR_SLIM                 :0x20000000,      /* TY: Toxic Breath */
    RF4_BA_CHAO                 :0x40000000,      /* TY: Primal Chaos Ball */
    RF4_BR_DISI                 :0x80000000,      /* Breathe Disintegration */

/*
* New monster race bit flags
*/
    RF5_BA_ACID                 :0x00000001,      /* Acid Ball */
    RF5_BA_ELEC                 :0x00000002,      /* Elec Ball */
    RF5_BA_FIRE                 :0x00000004,      /* Fire Ball */
    RF5_BA_COLD                 :0x00000008,      /* Cold Ball */
    RF5_BA_POIS                 :0x00000010,      /* Poison Ball */
    RF5_BA_NETH                 :0x00000020,      /* Nether Ball */
    RF5_BA_WATE                 :0x00000040,      /* Water Ball */
    RF5_BA_MANA                 :0x00000080,      /* Mana Storm */
    RF5_BA_DARK                 :0x00000100,      /* Darkness Storm */
    RF5_DRAIN_MANA              :0x00000200,      /* Drain Mana */
    RF5_MIND_BLAST              :0x00000400,      /* Blast Mind */
    RF5_BRAIN_SMASH             :0x00000800,      /* Smash Brain */
    RF5_CAUSE_1                 :0x00001000,      /* Cause Light Wound */
    RF5_CAUSE_2                 :0x00002000,      /* Cause Serious Wound */
    RF5_CAUSE_3                 :0x00004000,      /* Cause Critical Wound */
    RF5_CAUSE_4                 :0x00008000,      /* Cause Mortal Wound */
    RF5_BO_ACID                 :0x00010000,      /* Acid Bolt */
    RF5_BO_ELEC                 :0x00020000,      /* Elec Bolt (unused) */
    RF5_BO_FIRE                 :0x00040000,      /* Fire Bolt */
    RF5_BO_COLD                 :0x00080000,      /* Cold Bolt */
    RF5_BO_POIS                 :0x00100000,      /* Poison Bolt (unused) */
    RF5_BO_NETH                 :0x00200000,      /* Nether Bolt */
    RF5_BO_WATE                 :0x00400000,      /* Water Bolt */
    RF5_BO_MANA                 :0x00800000,      /* Mana Bolt */
    RF5_BO_PLAS                 :0x01000000,      /* Plasma Bolt */
    RF5_BO_ICEE                 :0x02000000,      /* Ice Bolt */
    RF5_MISSILE                 :0x04000000,      /* Magic Missile */
    RF5_SCARE                   :0x08000000,      /* Frighten Player */
    RF5_BLIND                   :0x10000000,      /* Blind Player */
    RF5_CONF                    :0x20000000,      /* Confuse Player */
    RF5_SLOW                    :0x40000000,      /* Slow Player */
    RF5_HOLD                    :0x80000000,      /* Paralyze Player */
    /*
    * New monster race bit flags
    */
    RF6_HASTE                   :0x00000001,      /* Speed self */
    RF6_MARK_CAIN               :0x00000002,      /* mark of Cain */
    RF6_HEAL                    :0x00000004,      /* Heal self */
    RF6_XXX2                    :0x00000008,      /* Heal a lot (?) */
    RF6_BLINK                   :0x00000010,      /* Teleport Short */
    RF6_TPORT                   :0x00000020,      /* Teleport Long */
    RF6_XXX3                    :0x00000040,      /* Move to Player (?) */
    RF6_XXX4                    :0x00000080,      /* Move to Monster (?) */
    RF6_TELE_TO                 :0x00000100,      /* Move player to monster */
    RF6_TELE_AWAY               :0x00000200,      /* Move player far away */
    RF6_TELE_LEVEL              :0x00000400,      /* Move player vertically */
    RF6_XXX5                    :0x00000800,      /* Move player (?) */
    RF6_DARKNESS                :0x00001000,      /* Create Darkness */
    RF6_TRAPS                   :0x00002000,      /* Create Traps */
    RF6_FORGET                  :0x00004000,      /* Cause amnesia */
    RF6_XXX6                    :0x00008000,      /* ??? */
    RF6_S_KIN                   :0x00010000,      /* Summon "kin" */
    RF6_S_REAVER                :0x00020000,      /* Summon Black Reavers! */
    RF6_S_MONSTER               :0x00040000,      /* Summon Monster */
    RF6_S_MONSTERS              :0x00080000,      /* Summon Monsters */
    RF6_S_ANT                   :0x00100000,      /* Summon Ants */
    RF6_S_SPIDER                :0x00200000,      /* Summon Spiders */
    RF6_S_HOUND                 :0x00400000,      /* Summon Hounds */
    RF6_S_HYDRA                 :0x00800000,      /* Summon Hydras */
    RF6_S_DEVIL                 :0x01000000,      /* Summon devil */
    RF6_S_DEMON                 :0x02000000,      /* Summon Demon */
    RF6_S_UNDEAD                :0x04000000,      /* Summon Undead */
    RF6_S_DRAGON                :0x08000000,      /* Summon Dragon */
    RF6_S_HI_UNDEAD             :0x10000000,      /* Summon Greater Undead */
    RF6_S_HI_DRAGON             :0x20000000,      /* Summon Ancient Dragon */
    RF6_S_FALLEN                :0x40000000,      /* Summon fallen angel */
    RF6_S_UNIQUE                :0x80000000,      /* Summon Unique Monster */
    /*
    * Hellband only flags
    */
    RF7_ANNOYED                 :0x00000001,      /* Annoyed ? */
    RF7_NO_SPAWN                :0x00000002,      /* Can only be summoned */
    RF7_STORM                   :0x00000004,      /* Afflicted by the Lovers' Storm */
    RF7_RES_DARK                :0x00000008,      /* Resist darkness */
    RF7_IM_DARK                 :0x00000010,      /* Immunity darkness */
    RF7_HEAL_DARK               :0x00000020,      /* Darkness healing */
    RF7_HURT_DARK               :0x00000040,      /* Extra Hurt by darkness */
    RF7_RES_LITE                :0x00000080,      /* Resist light */
    RF7_IM_LITE                 :0x00000100,      /* Immunity to light */
    RF7_HEAL_LITE               :0x00000200,      /* Heal by light */
    RF7_RES_COLD                :0x00000400,      /* Resist cold*/
    RF7_HEAL_COLD               :0x00000800,      /* Heal by cold */
    RF7_RES_FIRE                :0x00001000,      /* Resist Fire */
    RF7_HEAL_FIRE               :0x00002000,      /* Heal by Fire */
    RF7_RES_ACID                :0x00004000,      /* Resists Acid */
    RF7_RES_ELEC                :0x00008000,      /* Resist Electricity */
    RF7_HEAL_ELEC               :0x00010000,      /* Heal by Electricity */
    RF7_HEAL_NETH               :0x00020000,      /* Heal by Nether */
    RF7_RES_POIS                :0x00040000,      /* Resist poison */
    RF7_AQUATIC                 :0x00080000,      /* Thrives in water */
    RF7_FLIGHT                  :0x00100000,      /* Can fly or levitate */
    RF7_NEUTRAL                 :0x00200000,      /* Doesnt want to kill player when generated */
    RF7_REBORN                  :0x00400000,      /* Doesnt stay dead */
    RF7_XXX24                   :0x00800000,      /*  */
    RF7_XXX25                   :0x01000000,      /*  */
    RF7_XXX26                   :0x02000000,      /*  */
    RF7_XXX27                   :0x04000000,      /*  */
    RF7_XXX28                   :0x08000000,      /*  */
    RF7_XXX29                   :0x10000000,      /*  */
    RF7_XXX30                   :0x20000000,      /*  */
    RF7_XXX31                   :0x40000000,      /*  */
    RF7_XXX32                   :0x80000000,      /*  */

  };
});