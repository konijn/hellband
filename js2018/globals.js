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
  };
});