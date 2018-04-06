/* File: borg9.c */

/* Purpose: Highest level functions for the Borg -BEN- */
#include "angband.h"

#ifdef ALLOW_BORG

#include "zborg1.h"
#include "zborg2.h"
#include "zborg3.h"
#include "zborg4.h"
#include "zborg5.h"
#include "zborg6.h"
#include "zborg7.h"
#include "zborg8.h"
#include "zborg9.h"

static s16b stat_use[6];

/*
 * This file implements the "APWBorg", an "Automatic Angband Player".
 *
 * This version of the "APWBorg" is designed for use with ZAngband 2.4.0.
 *
 * Use of the "APWBorg" requires re-compilation with ALLOW_BORG defined,
 * and with the various "zborg*.c" files linked into the executable.
 *
 * Note that you can only use the Borg if your character has been marked
 * as a "Borg User".  You can do this, if necessary, by responding "y"
 * when asked if you really want to use the Borg.  This will (normally)
 * result in your character being inelligible for the high score list.
 *
 * The "do_cmd_borg()" function, called when the user hits "^Z", allows
 * the user to interact with the Borg.  You do so by typing "Borg Commands",
 * including 'z' to activate (or re-activate), 'K' to show monsters, 'T' to
 * show objects, 'd' to toggle "demo mode", 'f' to open/shut the "log file",
 * 'i' to display internal flags, etc.  See "do_cmd_borg()" for more info.
 *
 * The first time you enter a Borg command, the Borg is initialized.  This
 * consists of three major steps, and requires at least 400K of free memory,
 * if the memory is not available, the game may abort.
 *
 * (1) The various "borg" modules are initialized.
 *
 * (2) Some important "state" information is extracted, including the level
 *     and race/class of the player, and some more initialization is done.
 *
 * (3) Some "historical" information (killed uniques, maximum dungeon depth)
 *     is "stolen" from the game.
 *
 * When the Borg is "activated", it uses the "Term_inkey_hook" to steal
 * control from the user.  Later, if it detects any input from the real user,
 * it gracefully relinquishes control by clearing the "Term_inkey_hook" after
 * any pending key-sequences are complete.
 *
 * The Borg will abort if it detects any "errors", or if it detects any
 * "situations" such as "death", or if it detects any "panic" situations,
 * such as "imminent death", if the appropriate flags are set.
 *
 * The Borg is only supposed to "know" what is visible on the screen,
 * which it learns by using the "term.c" screen access function "Term_what()",
 * the cursor location function "Term_locate()", and the cursor visibility
 * extraction function "Term_get_cursor()".
 *
 * The Borg is only supposed to "send" keypresses when the "Term_inkey()"
 * function asks for a keypress, which is accomplished by using a special
 * function hook in the "z-term.c" file, which allows the Borg to "steal"
 * control from the "Term_inkey()" and "Term_flush()" functions.  This
 * allows the Borg to pretend to be a normal user.
 *
 * Note that if properly designed, the Borg could be run as an external
 * process, which would actually examine the screen (or pseudo-terminal),
 * and send keypresses directly to the keyboard (or pseudo-terminal).  Thus
 * it should never access any "game variables", unless it is able to extract
 * those variables for itself by code duplication or complex interactions,
 * or, in certain situations, if those variables are not actually "required".
 *
 * Currently, the Ben Borg is a few steps away from being able to be run as
 * an external process, primarily in the "low level" details, such as knowing
 * when the game is ready for a keypress.  Also, the Ben Borg assumes that a
 * character has already been rolled, and maintains no state between saves,
 * which is partially offset by "cheating" to "acquire" the maximum dungeon
 * depth, without which equipment analysis will be faulty.
 *
 * The "theory" behind the Borg is that is should be able to run as a
 * separate process, playing Angband in a window just like a human, that
 * is, examining the screen for information, and sending keypresses to
 * the game.  The current Borg does not actually do this, because it would
 * be very slow and would not run except on Unix machines, but as far as
 * possible, I have attempted to make sure that the Borg *could* run that
 * way.  This involves "cheating" as little as possible, where "cheating"
 * means accessing information not available to a normal Angband player.
 * And whenever possible, this "cheating" should be optional, that is,
 * there should be software options to disable the cheating, and, if
 * necessary, to replace it with "complex" parsing of the screen.
 *
 * Thus, the Borg COULD be written as a separate process which runs Angband
 * in a pseudo-terminal and "examines" the "screen" and sends keypresses
 * directly (as with a terminal emulator), although it would then have
 * to explicitly "wait" to make sure that the game was completely done
 * sending information.
 *
 * The Borg is thus allowed to examine the screen directly (by efficient
 * direct access of the "Term->scr->a" and "Term->scr->c" arrays, which
 * could be replaced by calls to "Term_grab()"), and to access the cursor
 * location (via "Term_locate()") and visibility (via "Term_get_cursor()"),
 * and, as mentioned above, the Borg is allowed to send keypresses directly
 * to the game, and only when needed, using the "Term_inkey_hook" hook, and
 * uses the same hook to know when it should discard all pending keypresses.
 *
 * The Borg should not know when the game is ready for a keypress, and
 * should really do something nasty such as "pause" between turns for
 * some amount of time to ensure that the game is really waiting for
 * a keypress.
 *
 * Various other "cheats" (mostly optional) are described where they are
 * used, primarily in this file.
 *
 * Note that any "user input" will be ignored, and will cancel the Borg,
 * after the Borg has completed any key-sequences currently in progress.
 *
 * Note that the "borg_t" parameter bears a close resemblance to the number of
 * "player turns" that have gone by.  Except that occasionally, the Borg will
 * do something that he *thinks* will take time but which actually does not
 * (for example, attempting to open a hallucinatory door), and that sometimes,
 * the Borg performs a "repeated" command (rest, open, tunnel, or search),
 * which may actually take longer than a single turn.  This has the effect
 * that the "borg_t" variable is slightly lacking in "precision".  Note that
 * we can store every time-stamp in a 's16b', since we reset the clock to
 * 1000 on each new level, and we refuse to stay on any level longer than
 * 30000 turns, unless we are totally stuck, in which case we abort.
 *
 * Note that as of 2.7.9, the Borg can play any class, that is, he can make
 * "effective" use of at least a few spells/prayers, and is not "broken"
 * by low strength, blind-ness, hallucination, etc.  This does not, however,
 * mean that he plays all classes equally well, especially since he is so
 * dependant on a high strength for so many things.  The "demo" mode is useful
 * for many classes (especially Mage) since it allows the Borg to "die" a few
 * times, without being penalized.
 *
 * The Borg assumes that the "maximize" flag is off, and that the
 * "preserve" flag is on, since he cannot actually set those flags.
 * If the "maximize" flag is on, the Borg may not work correctly.
 * If the "preserve" flag is off, the Borg may miss artifacts.
 */


/*
 * Currently, the Borg "cheats" in a few situations...
 *
 * Cheats that are significant, and possibly unavoidable:
 *   Knowledge of when we are being asked for a keypress.
 *   Note that this could be avoided by LONG timeouts/delays
 *
 * Cheats "required" by implementation, but not signifant:
 *   Direct access to the "screen image" (parsing screen)
 *   Direct access to the "keypress queue" (sending keys)
 *   Direct access to the "cursor visibility" (game state)
 *
 * Cheats that could be avoided by simple (ugly) code:
 *   Direct modification of the "current options"
 *
 * Cheats that could be avoided by duplicating code:
 *   Use of the tables in "tables.c"
 *   Use of the arrays initialized in "init.c"
 *
 * Cheats that the Borg would love:
 *   Immunity to hallucination, blindness, confusion
 *   Unique attr/char codes for every monster and object
 *   Removal of the "mimic" and "trapper" monsters
 *   Removal of the "mushroom" and "gold" monsters
 */


/*
 * Some variables
 */

static bool initialized;    /* Hack -- Initialized */



/*
 * Mega-Hack -- extract some "hidden" variables
 *
 * XXX XXX XXX This step would not be necessary if more info
 * was available on the screen.
 *
 */
static void borg_hidden(void)
{
    int i;

    /* Clear the stat modifiers */
    for (i = 0; i < 6; i++) my_stat_add[i] = 0;

    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* if we have on unidentified stuff we may have misguessed our */
        /* stats. */
        if (!item->aware)
        {
            my_need_stat_check[0] = TRUE;
            my_need_stat_check[1] = TRUE;
            my_need_stat_check[2] = TRUE;
            my_need_stat_check[3] = TRUE;
            my_need_stat_check[4] = TRUE;
            my_need_stat_check[5] = TRUE;
            break;
        }

        /* Affect stats */
        if (item->flags1 & TR1_STR) my_stat_add[A_STR] += item->pval;
        if (item->flags1 & TR1_INT) my_stat_add[A_INT] += item->pval;
        if (item->flags1 & TR1_WIS) my_stat_add[A_WIS] += item->pval;
        if (item->flags1 & TR1_DEX) my_stat_add[A_DEX] += item->pval;
        if (item->flags1 & TR1_CON) my_stat_add[A_CON] += item->pval;
        if (item->flags1 & TR1_CHR) my_stat_add[A_CHR] += item->pval;
    }

    /* Mega-Hack -- Guess at "my_stat_cur[]" */
    for (i = 0; i < 6; i++)
    {
        int value;

        if (!my_need_stat_check[i]) continue;

        /* Reverse known bonuses to get the base stat value */
        value = modify_stat_value(borg_stat[i], -my_stat_add[i]);

        /* If the displayed stat is 18/220 this was just a guess.  */
        /* The player still needs to take off some stuff to get the */
        /* real value. */
        if (borg_stat[i] < 238)
        {
            my_need_stat_check[i] = FALSE;
        }

        /* Hack -- save the maximum/current stats */
        my_stat_cur[i] = p_ptr->stat_cur[i];

        /* Max stat is the max that the cur stat ever is. */
        if (my_stat_cur[i] > my_stat_max[i])
            my_stat_max[i] = my_stat_cur[i];

    }
}

void borg_log_death(void)
{
   char buf[1024];
   FILE *borg_log_file;
   time_t death_time;

   path_build(buf, 1024, ANGBAND_DIR_USER, "borg-log.txt");

   /* Hack -- drop permissions */
   safe_setuid_drop();

   /* Append to the file */
   borg_log_file = my_fopen(buf, "a");

    /* Hack -- grab permissions */
   safe_setuid_grab();

   /* Failure */
   if (!borg_log_file) return;

   /* Get time of death */
   (void)time(&death_time);

   /* Save the date */
   strftime(buf, 80, "%Y/%m/%d %H:%M\n", localtime(&death_time));

   fprintf(borg_log_file, buf);

   fprintf(borg_log_file, "%s the %s %s, Level %d/%d\n", player_name,
           race_info[p_ptr->prace].title,
           class_info[p_ptr->pclass].title,
           p_ptr->lev, p_ptr->max_plv);

   fprintf(borg_log_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)total_points(), (long)p_ptr->au, (long)turn);
   fprintf(borg_log_file, "Killed on level: %d (max. %d) by %s\n", dun_level, p_ptr->max_dlv, died_from);

   fprintf(borg_log_file, "ZBorg Compile Date: %s\n", borg_engine_date);

   fprintf(borg_log_file, "----------\n\n");

   my_fclose(borg_log_file);
}

void borg_log_death_data(void)
{
   char buf[1024];
    FILE *borg_log_file;
   time_t death_time;

   path_build(buf, 1024, ANGBAND_DIR_USER, "zborg.dat");

   /* Hack -- drop permissions */
   safe_setuid_drop();

   /* Append to the file */
  borg_log_file = my_fopen(buf, "a");

    /* Hack -- grab permissions */
   safe_setuid_grab();

   /* Failure */
   if (!borg_log_file) return;

   /* Get time of death */
   (void)time(&death_time);

    /* dump stuff for easy import to database */
   fprintf(borg_log_file, "%s, %s, %s, %d, %d, %s\n",borg_engine_date, race_info[p_ptr->prace].title,
   class_info[p_ptr->pclass].title, p_ptr->lev, dun_level, died_from);

   my_fclose(borg_log_file);
}


/*
 * Think about the world and perform an action
 *
 * Check inventory/equipment/spells/panel once per "turn"
 *
 * Process "store" and other modes when necessary
 *
 * Note that the non-cheating "inventory" and "equipment" parsers
 * will get confused by a "weird" situation involving an ant ("a")
 * on line one of the screen, near the left, next to a shield, of
 * the same color, and using --(-- the ")" symbol, directly to the
 * right of the ant.  This is very rare, but perhaps not completely
 * impossible.  I ignore this situation.  :-)
 *
 * The handling of stores is a complete and total hack, but seems
 * to work remarkably well, considering... :-)  Note that while in
 * a store, time does not pass, and most actions are not available,
 * and a few new commands are available ("sell" and "purchase").
 *
 * Note the use of "cheat" functions to extract the current inventory,
 * the current equipment, the current panel, and the current spellbook
 * information.  These can be replaced by (very expensive) "parse"
 * functions, which cause an insane amount of "screen flashing".
 *
 * Technically, we should attempt to parse all the messages that
 * indicate that it is necessary to re-parse the equipment, the
 * inventory, or the books, and only set the appropriate flags
 * at that point.  This would not only reduce the potential
 * screen flashing, but would also optimize the code a lot,
 * since the "cheat_inven()" and "cheat_equip()" functions
 * are expensive.  For paranoia, we could always select items
 * and spells using capital letters, and keep a global verification
 * buffer, and induce failure and recheck the inventory/equipment
 * any time we get a mis-match.  We could even do some of the state
 * processing by hand, for example, charge reduction and such.  This
 * might also allow us to keep track of how long we have held objects,
 * especially if we attempt to do "item tracking" in the inventory
 * extraction code.
 */
static bool borg_think(void)
{
    int i, ii;

    byte t_a;

    char buf[128];
    static char svSavefile[1024];
    static char svSavefile2[1024];
    static bool justSaved = FALSE;


	window_stuff();


    /*** Process inventory/equipment ***/
    /* Examine the equipment/inventory */
    borg_notice(TRUE);

	/* Extract some "hidden" variables */
    borg_hidden();

	/* Cheat */
    if (borg_do_equip)
    {
        /* Only do it once */
        borg_do_equip = FALSE;

        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Done */
        return (FALSE);
    }

    /* Cheat */
    if (borg_do_inven)
    {
        /* Only do it once */
        borg_do_inven = FALSE;

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Done */
        return (FALSE);
    }

    /* save now */
    if (borg_save && borg_save_game())
    {
        /* Log */
        borg_note("# Auto Save!");

        borg_save = FALSE;

        /* Create a scum file */
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation"))
        {
            memcpy(svSavefile, savefile, sizeof(savefile));
            /* Process the player name */
            for (i = 0; player_name[i]; i++)
            {
                char c = player_name[i];

                /* No control characters */
                if (iscntrl(c))
                {
                    /* Illegal characters */
                    quit_fmt("Illegal control char (0x%02X) in player name", c);
                }

                /* Convert all non-alphanumeric symbols */
                if (!isalpha(c) && !isdigit(c)) c = '_';

                /* Build "file_name" */
                svSavefile2[i] = c;
            }
            svSavefile2[i]  = 0;
            path_build(savefile, 1024, ANGBAND_DIR_USER, svSavefile2);

            justSaved = TRUE;
        }
        return (TRUE);
    }
    if (justSaved)
    {
        memcpy(savefile, svSavefile, sizeof(savefile));
        borg_save_game();
        justSaved = FALSE;
        return (TRUE);
    }

	/* Parsing of inven and equip removed */

    /*** Find books ***/

    /* Only if needed */
    if (borg_do_spell)
    {
        /* Assume no books */
		for (ii = 0; ii < MAX_REALM+1; ii ++)
		{
        	for (i = 0; i < 4; i++) borg_book[ii][i] = -1;
		}

        /* Scan the pack */
		for (ii = 1; ii < MAX_REALM+1; ii ++)
		{
			/* skip non my realms */
			if (ii != borg_skill[BI_REALM1] &&
				ii != borg_skill[BI_REALM2]) continue;

	        for (i = 0; i < INVEN_PACK; i++)
	        {
	            borg_item *item = &borg_items[i];

	            /* Skip wrong-realm books */
	            if (item->tval != REALM1_BOOK &&
	                item->tval != REALM2_BOOK) continue;

	            /* Note book locations */
	            borg_book[item->tval - TV_LIFE_BOOK + 1][item->sval] = i;
			}
        }
    }

    /*** Process books ***/

    /* Hack -- Warriors never browse */
    if (borg_class == CLASS_WARRIOR) borg_do_spell = FALSE;

    /* Hack -- Blind or Confused prevents browsing */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) borg_do_spell = FALSE;

    /* XXX XXX XXX Dark */

    /* Cheat */
    if (borg_do_spell)
    {
        /* Only do it once for each realm */
        borg_do_spell = FALSE;

        /* Cheat that realm */
        borg_cheat_spell(borg_skill[BI_REALM1]);
        borg_cheat_spell(borg_skill[BI_REALM2]);


        /* Done */
        return (FALSE);
    }

    /* check for anything that needs *ID* */
    if (borg_do_star_id)
    {
        borg_do_star_id = FALSE;
		borg_object_star_id();
    }
    /* If king, maybe retire. */
    if (borg_skill[BI_KING])
    {
        /* Prepare to retire */
        if (borg_stop_king)
        {
            borg_write_map(FALSE);
            borg_oops("retire");
        }
        /* Borg will be respawning */
        if (borg_respawn_winners)
        {
            borg_write_map(FALSE);

			/* Write to log and borg.dat */
			borg_log_death();
			borg_log_death_data();

            /* respawn */
            resurrect_borg();
        }

    }

	/* Clear then Load up the borg_prepared[] reports */
	for (i = 0; i < 127; i++)
	{
		borg_prepared[i] = borg_prep(i);
	}

    /*** Handle stores ***/

    /* Hack -- Check for being in a store */
    if ((0 == borg_what_text(3, 5, 16, &t_a, buf)) &&
        (streq(buf, "Item Description")))
    {
        /* Assume the Home */
        shop_num = BORG_HOME;

        /* Clear the goal (the goal was probably going to a shop) */
        goal = 0;

        /* Extract the "store" name */
        if (0 == borg_what_text(50, 3, -20, &t_a, buf))
        {
            int i;

            /* Check the store names */
            for (i = 0; i < (MAX_STORES ); i++)
            {
                cptr name = (f_name + f_info[FEAT_SHOP_HEAD+i].name);
                if (prefix(buf, name)) shop_num = i;
            }
        }

        /* Hack -- reset page/more */
        borg_shops[shop_num].page = 0;
        borg_shops[shop_num].more = 0;

		/* Extract the page */
        if (0 == borg_what_text(21, 5, -6, &t_a, buf))
        {
            if (prefix(buf, "Page 1")) borg_shops[shop_num].page = 0;
            if (prefix(buf, "Page 2")) borg_shops[shop_num].page = 1;
        }


        /* Hack -- Reset food counter for money scumming */
        if (shop_num == 0) borg_food_onsale = 0;

        /* Cheat the current gold (unless in home) */
        borg_gold = p_ptr->au;

        /* Cheat the store (or home) inventory (all pages) */
		borg_cheat_store(borg_skill[BI_TOWN_NUM], shop_num);

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

        /* Evaluate the current world */
        my_power = borg_power();

        /* Hack -- allow user abort */
        if (borg_cancel) return (TRUE);

        /* Think until done */
        return (borg_think_store());
    }

	/* Check for being in a specialty shop -- Inner Temple */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Eldor")) || (streq(buf, "Crysa")) || (streq(buf, "Kabir"))))
    {
        /* Note */
        borg_note("# Entered the Temple.");

		/* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_temple());
	}

	/* Check for being in a specialty shop -- Thieves Guild */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Lardb")) || (streq(buf, "Palir")) || (streq(buf, "Molvr"))))
    {
        /* Note */
        borg_note("# Entered the Thieves Guild.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_thieves());
	}

	/* Check for being in a specialty shop -- Library */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Astin")) || (streq(buf, "Mirim")) || (streq(buf, "Vaner"))))
    {
        /* Note */
        borg_note("# Entered the Library.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_library());
	}

	/* Check for being in a specialty shop -- Tower or Sorc (recharge) */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Tanis")) || (streq(buf, "Valce"))))
    {
        /* Note */
        borg_note("# Entered the Tower of Sorcery.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_sorcery());
	}

	/* Check for being in a specialty shop -- Rangers Guild */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Trall")) || (streq(buf, "Angro"))))
    {
        /* Note */
        borg_note("# Entered the Rangers Guild.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_bow());
	}

	/* Check for being in a specialty shop -- Fighter Hall */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Hjolg")) || (streq(buf, "Hjolg"))))
    {
        /* Note */
        borg_note("# Entered the Fighter Hall.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_fighter());
	}

	/* Check for being in a specialty shop -- Paladin Guild */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        ((streq(buf, "Vilio")) || (streq(buf, "Vilio"))))
    {
        /* Note */
        borg_note("# Entered the Paladin Guilld.");

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Hack -- browse again later */
        borg_do_browse = TRUE;

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_paladin());
	}

	/* Check for being in a specialty shop -- Chaos Tower */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        (streq(buf, "Xeno ")))
    {
        /* Note */
        borg_note("# Entered the Chaos Tower.");

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_mutation());
	}

	/* Check for being in a specialty shop -- Inn */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        (streq(buf, "Celeb")))
    {
        /* Note */
        borg_note("# Entered the Inn.");

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_inn());
	}

	/* Check for being in a specialty shop -- Trump Tower */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        (streq(buf, "Pello")))
    {
        /* Note */
        borg_note("# Entered the Trump Tower.");

        /* Examine the inventory */
        borg_notice(TRUE);

		/* Done */
        return (borg_think_shop_trump());
	}

	/* Hack -- Check for being in a bad store and escape */
    if ((0 == borg_what_text(1, 2, 5, &t_a, buf)) &&
        (streq(buf, "Sonda") || streq(buf, "Otick") || streq(buf, "Uldri") || streq(buf, "Draxl") ||
		 streq(buf, "Bulin") || streq(buf, "Garad")))
    {
        /* Note our mistake */
        borg_note("# Mistakenly entered a store.");

        /* Leave */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
	}

	/* Hack -- Check for being in a Pet Command and escape */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
        (streq(buf, "(Command a-g,")))
    {
        /* Note our mistake */
        borg_note("# Mistakenly entered Pet Commands Menu.");

        /* Leave Pet Command mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
	}

	/* Hack -- Check for being in a menu */
    if ((0 == borg_what_text(0, 2, 8, &t_a, buf)) &&
        (streq(buf, "Interact")))
    {
        /* Note our mistake */
        borg_note("# Mistakenly entered Menu.");

        /* Leave Command mode */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
	}

	/* Hack -- Check for being in a Self Knowledge screen */
    if ((0 == borg_what_text(20, 1, 10, &t_a, buf)) &&
        (streq(buf, "Your Attri")))
    {
        /* Note our mistake */
        borg_note("# Leaving Self Knowledge screen.");

        /* Leave Command mode */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
	}

	/*** Determine panel ***/

    /* Hack -- cheat */
    w_y = panel_row_min;
    w_x = panel_col_min;

    /* Done */
    borg_do_panel = FALSE;

    /* Hack -- Check for "sector" mode */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
        (prefix(buf, "Map sector ")))
    {
        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (TRUE);
    }
    /* Check equipment */
    if (borg_do_panel)
    {
        /* Only do it once */
        borg_do_panel = FALSE;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return (TRUE);
    }

    /*** Analyze the Frame ***/

    /* Analyze the frame */
    if (borg_do_frame)
    {
        /* Only once */
        borg_do_frame = FALSE;

        /* Analyze the "frame" */
        borg_update_frame();
    }

    /*** Re-activate Tests ***/

    /* Check equip again later */
    borg_do_equip = TRUE;

    /* Check inven again later */
    borg_do_inven = TRUE;

    /* Check panel again later */
    borg_do_panel = TRUE;

    /* Check frame again later */
    borg_do_frame = TRUE;

    /* Check spells again later */
    borg_do_spell = TRUE;


    /*** Analyze status ***/

    /* Track best level */
    if (borg_skill[BI_CLEVEL] > borg_skill[BI_MAXCLEVEL]) borg_skill[BI_MAXCLEVEL] = borg_skill[BI_CLEVEL];
    if (borg_skill[BI_CDEPTH] > borg_skill[BI_MAXDEPTH])
    {
        borg_skill[BI_MAXDEPTH] = borg_skill[BI_CDEPTH];
    }

    /*** Think about it ***/

    /* Increment the clock */
    borg_t++;

    /* Increment the panel clock */
    time_this_panel++;

    /* Examine the equipment/inventory */
    borg_notice(TRUE);

	/* Extract some "hidden" variables */
    borg_hidden();

	/* Examine the screen */
    borg_update();

    /* Evaluate the current world */
    my_power = borg_power();

	/* Load up the borg_prepared[] reports */
	for (i = 0; i < 127; i++)
	{
		borg_prepared[i] = borg_prep(i);
	}

    /* Hack -- allow user abort */
    if (borg_cancel) return (TRUE);

    /* Do something */
    return (borg_think_dungeon());
}



/*
 * Hack -- methods of hurting a monster (order not important).
 *
 * See "message_pain()" for details.
 */
static cptr suffix_pain[] =
{
    " is unharmed."
    " barely notices.",
    " flinches.",
    " squelches.",
    " quivers in pain.",
    " writhes about.",
    " writhes in agony.",
    " jerks limply.",

	" roars thunderously.",
	" rumbles.",
	" grunts.",
	" hesitates.",
	" crumples.",

	" hisses.",
	" rears up in anger.",
	" hisses in pain.",
	" hisses with pain.",
	" growls angrily.",
	" hisses furiously.",
	" roars.",
	" mewls in pain.",
	" mewls pitifully.",

	" chitters.",
	" scuttles about.",
	" twitters.",
	" twitches.",
	" chirps.",
	" squawks.",
	" chatters.",
	" jeers.",
	" flutters about.",
	" squeaks.",

	" snarls with pain.",
    " roars with pain.",
    " gasps.",
    " snarls feebly.",
    " rattles.",
    " stumbles.",
    " staggers.",
    " clatters.",
    " groans.",
    " moans.",
    " hesitates.",
    " grunts.",
    " wails.",
    " howls.",
    " moans softly.",
    " sighs.",
    " shrieks in pain.",
    " shrieks in agony.",

    " spawns!",
    " looks healthier.",
    " starts moving faster.",
    " starts moving slower.",

    " is unaffected!",
    " resists!",
    " is immune.",
    " is immune!",
    " resists a lot.",
    " resists.",
    " resists somewhat.",

    " shrugs off the attack.",
    " snarls with pain.",
    " yelps in pain.",
    " howls in pain.",
    " howls in agony.",
    /* xxx */
    " yelps feebly.",

    " ignores the attack.",
    " grunts with pain.",
    " squeals in pain.",
    " shrieks in pain.",
    " shrieks in agony.",
    /* xxx */
    " cries out feebly.",

    /* xxx */
    /* xxx */
    " cries out in pain.",
    " screams in pain.",
    " screams in agony.",
    /* xxx */
    " cringes from the light!",
    " loses some skin!",

	" is suspended!",

    " is hit hard.",
	" is dazed.",
	" is more dazed.",

	" is suddenly very hot!",
	" gets zapped!",

	" is tamed!",
	" hates you too much!",
	" is in your thrall!",


    NULL
};


/*
 * Hack -- methods of killing a monster (order not important).
 *
 * See "mon_take_hit()" for details.
 */
static cptr prefix_kill[] =
{
    "You have killed ",
    "You have slain ",
    "You have destroyed ",
    NULL
};


/*
 * Hack -- methods of monster death (order not important).
 *
 * See "project_m()", "do_cmd_fire()", "mon_take_hit()" for details.
 */
static cptr suffix_died[] =
{
    " dies.",
    " dies from the Quivering Palm.",
    " is destroyed.",
    " is killed.",
    " dissolves!",
    " shrivels away in the light!",
    " collapses, a mindless husk.",
	" turns into a pile of ash.",
	" turns into a pile of cinder.",
	" evaporates!",
    NULL
};
static cptr suffix_blink[] =
{
    " disappears!",      /* from teleport other */
    " changes!",         /* from polymorph spell */
    " teleports away.",  /* RF6_TPORT */
    " blinks away.",                /* RF6_BLINK */
	"The thief flees laughing!",
	NULL
};

/*
 * Hack -- methods of hitting the player (order not important).
 *
 * The "insult", "moan", and "begs you for money" messages are ignored.
 *
 * See "make_attack_normal()" for details.
 */
static cptr suffix_hit_by[] =
{
    " hits you.",
    " touches you.",
    " punches you.",
    " kicks you.",
    " claws you.",
    " bites you.",
    " stings you.",
    " butts you.",
    " crushes you.",
    " engulfs you.",
    " charges you.",
    " crawls on you.",
    " drools on you.",
    " spits on you.",
    " gazes at you.",
    " wails at you.",
    " releases spores at you.",
    NULL
};

/*
 * Hack -- methods of hitting some other monster (order not important).
 *
 * Basically, two monsters are duking it out and we dont want to know.
 * See "make_attack_normal()" for details.
 */
static cptr suffix_mon_mon[] =
{
    " misses the",
/*    " hits the", Missiles thrown will trigger this as well */
    " touches the",
    " punches the",
    " kicks the",
    " claws the",
    " bites the",
    " stings the",
    " butts the",
    " crushes the",
    " engulfs the",
    " charges the",
    " crawls on the",
    " drools on the",
    " spits on the",
    " gazes at the",
    " wails at the",
    " releases spores at the",
    NULL
};

/*
 * Hack -- methods of casting spells at the player (order important).
 *
 * See "make_attack_spell()" for details.
 */
/* AJG These had gotten out of synch with where they are used. */
static cptr suffix_spell[] =
{
    " makes a high pitched shriek.",        /* 0 RF4_SHRIEK */
    " tries to cast a spell, but fails.",   /* 1 RF4_FAILS */
    " does something.",                     /* 2 RF4_XXX3X4 */
    " fires a rocket.",                     /* 3 RF4_XXX4X4 */
    " fires an arrow.",                     /* 4 RF4_ARROW_1 */
    " fires an arrow!",                     /* 5 RF4_ARROW_2 */
    " fires a bolt.",                       /* 6 RF4_ARROW_3 */
    " fires a bolt!",                       /* 7 RF4_ARROW_4 */
    " breathes acid.",                      /* 8 RF4_BR_ACID */
    " breathes lightning.",                 /* 9 RF4_BR_ELEC */
    " breathes fire.",                      /*10 RF4_BR_FIRE */
    " breathes frost.",                     /*11 RF4_BR_COLD */
    " breathes gas.",                       /*12 RF4_BR_POIS */
    " breathes nether.",                    /*13 RF4_BR_NETH */
    " breathes light.",                     /*14 RF4_BR_LITE */
    " breathes darkness.",                  /*15 RF4_BR_DARK */
    " breathes confusion.",                 /*16 RF4_BR_CONF */
    " breathes sound.",                     /*17 RF4_BR_SOUN */
    " breathes chaos.",                     /*18 RF4_BR_CHAO */
    " breathes disenchantment.",            /*19 RF4_BR_DISE */
    " breathes nexus.",                     /*20 RF4_BR_NEXU */
    " breathes time.",                      /*21 RF4_BR_TIME */
    " breathes inertia.",                   /*22 RF4_BR_INER */
    " breathes gravity.",                   /*23 RF4_BR_GRAV */
    " breathes shards.",                    /*24 RF4_BR_SHAR */
    " breathes plasma.",                    /*25 RF4_BR_PLAS */
    " breathes force.",                     /*26 RF4_BR_WALL */
    " breathes mana.",                      /*27 RF4_BR_MANA */
    " casts a ball of radiation.",          /*28 RF4_BR_NUKE */
    " breathes toxic waste.",               /*29 RF4_BR_NUKE */
    " invokes raw Logrus.",                 /*30 RF4_BR_CHAOS */
    " breathes disintegration.",            /*31 RF4_BR_DISI */
    " casts an acid ball.",                 /*32 RF5_BA_ACID 128+0*/
    " casts a lightning ball.",             /*33 RF5_BA_ELEC */
    " casts a fire ball.",                  /*34 RF5_BA_FIRE */
    " casts a frost ball.",                 /*35 RF5_BA_COLD */
    " casts a stinking cloud.",             /*36 RF5_BA_POIS */
    " casts a nether ball.",                /*37 RF5_BA_NETH */
    " gestures fluidly.",                   /*38 RF5_BA_WATE */
    " invokes a mana storm.",               /*39 RF5_BA_MANA */
    " invokes a darkness storm.",           /*40 RF5_BA_DARK */
    " draws psychic energy from you!",      /*41 RF5_DRAIN_MANA */
    " gazes deep into your eyes.",          /*42 RF5_MIND_BLAST */
    " looks deep into your eyes.",          /*43 RF5_BRAIN_SMASH */
    " points at you and curses.",           /*44 RF5_CAUSE_1 */
    " points at you and curses horribly.",  /*45 RF5_CAUSE_2 */
    " points at you, incanting terribly!",  /*46 RF5_CAUSE_3 */
    " points at you, screaming the word DIE!",  /*47 RF5_CAUSE_4 */
    " casts a acid bolt.",                  /*48 RF5_BO_ACID */
    " casts a lightning bolt.",             /*49 RF5_BO_ELEC */
    " casts a fire bolt.",                  /*50 RF5_BO_FIRE */
    " casts a frost bolt.",                 /*51 RF5_BO_COLD */
    " does something.",                     /*52 RF5_BO_POIS */
    " casts a nether bolt.",                /*53 RF5_BO_NETH */
    " casts a water bolt.",                 /*54 RF5_BO_WATE */
    " casts a mana bolt.",                  /*55 RF5_BO_MANA */
    " casts a plasma bolt.",                /*56 RF5_BO_PLAS */
    " casts an ice bolt.",                  /*57 RF5_BO_ICEE */
    " casts a magic missile.",              /*58 RF5_MISSILE */
    " casts a fearful illusion.",           /*59 RF5_SCARE */
    " casts a spell, burning your eyes!",   /*60 RF5_BLIND */
    " creates a mesmerising illusion.",     /*61 RF5_CONF */
    " drains power from your muscles!",     /*62 RF5_SLOW */
    " stares deep into your eyes!",         /*63 RF5_HOLD */
    " concentrates on XXX body.",           /*64 RF6_HASTE */
    " invokes the Hand of Doom!",           /*65 RF6_HAND_DOOM */
    " concentrates on XXX wounds.",         /*66 RF6_HEAL */
    " casts a Globe of Invulnerability.",   /*67 RF6_INVULNER */
    " does something.",                     /*68 RF6_XXX3X6 */
    " does something.",                     /*69 RF6_XXX4X6 */
    " commands you to return.",             /*70 RF6_TELE_TO */
    " teleports you away.",                 /*71 RF6_TELE_AWAY */
    " gestures at your feet.",              /*72 RF6_TELE_LEVEL */
    " does something.",                     /*73 RF6_XXX5 */
    " gestures in shadow.",                 /*74 RF6_DARKNESS */
    " casts a spell and cackles evilly.",   /*75 RF6_TRAPS */
    " tries to blank your mind.",           /*76 RF6_FORGET */
    " mutters quietly.",                    /*77 RF6_RAISE_DEAD */
    " magically summons XXX kin",           /*78 RF6_S_KIN */
    " magically summons Cyberdemons.",      /*79 RF6_S_CYBER */
    " magically summons help!",             /*80 RF6_S_MONSTER */
    " magically summons monsters!",         /*81 RF6_S_MONSTERS */
    " magically summons ants.",             /*82 RF6_S_ANT */
    " magically summons spiders.",          /*83 RF6_S_SPIDER */
    " magically summons hounds.",           /*84 RF6_S_HOUND */
    " magically summons hydras.",           /*85 RF6_S_HYDRA */
    " magically summons an angel!",         /*86 RF6_S_ANGEL */
    " magically summons a demon from the Courts of Chaos!", /*87 RF6_S_DEMON */
    " magically summons an undead adversary!",  /*88 RF6_S_UNDEAD */
    " magically summons a dragon!",         /*89 RF6_S_DRAGON */
    " magically summons greater undead!",   /*90 RF6_S_HI_UNDEAD */
    " magically summons ancient dragons!",  /*91 RF6_S_HI_DRAGON */
    " magically summons Lords of Amber!",   /*92 RF6_S_AMERITES */
    " magically summons special opponents!",        /*93 RF6_S_UNIQUE */

    NULL
};



/*
 * Hack -- Spontaneous level feelings (order important).
 *
 * See "do_cmd_feeling()" for details.
 */
static cptr prefix_feeling[] =
{
    "Looks like any other level",
    "You feel there is something special",
    "You have a superb feeling",
    "You have an excellent feeling",
    "You have a very good feeling",
    "You have a good feeling",
    "You feel strangely lucky",
    "You feel your luck is turning",
    "You like the look of this place",
    "This level can't be all bad",
    "What a boring place",
    NULL
};



/*
 * Hack -- Parse a message from the world
 *
 * Note that detecting "death" is EXTREMELY important, to prevent
 * all sorts of errors arising from attempting to parse the "tomb"
 * screen, and to allow the user to "observe" the "cause" of death.
 *
 * Note that detecting "failure" is EXTREMELY important, to prevent
 * bizarre situations after failing to use a staff of perceptions,
 * which would otherwise go ahead and send the "item index" which
 * might be a legal command (such as "a" for "aim").  This method
 * is necessary because the Borg cannot parse "prompts", and must
 * assume the success of the prompt-inducing command, unless told
 * otherwise by a failure message.  Also, we need to detect failure
 * because some commands, such as detection spells, need to induce
 * furthur processing if they succeed, but messages are only given
 * if the command fails.
 *
 * Note that certain other messages may contain useful information,
 * and so they are "analyzed" and sent to "borg_react()", which just
 * queues the messages for later analysis in the proper context.
 *
 * Along with the actual message, we send a special formatted buffer,
 * containing a leading "opcode", which may contain extra information,
 * such as the index of a spell, and an "argument" (for example, the
 * capitalized name of a monster), with a "colon" to separate them.
 *
 * XXX XXX XXX Several message strings take a "possessive" of the form
 * "his" or "her" or "its".  These strings are all represented by the
 * encoded form "XXX" in the various match strings.  Unfortunately,
 * the encode form is never decoded, so the Borg currently ignores
 * messages about several spells (heal self and haste self).
 *
 * XXX XXX XXX We notice a few "terrain feature" messages here so
 * we can acquire knowledge about wall types and door types.
 */
static void borg_parse_aux(cptr msg, int len)
{
    int i, tmp;

    char who[256];
    char buf[256];

    borg_grid *ag = &borg_grids[g_y][g_x];
	cave_type *c_ptr = &cave[c_y][c_x];

    /* Log (if needed) */
    if (borg_fff) borg_info(format("& Msg <%s>", msg));


    /* Hack -- Notice death */
    if (prefix(msg, "You die."))
    {
        /* Abort (unless cheating) */
        if (!(wizard || cheat_live || borg_cheat_death))
        {
            /* Abort */
            borg_oops("death");

            /* Abort right now! */
            borg_active = FALSE;

            /* Noise XXX XXX XXX */
            Term_xtra(TERM_XTRA_NOISE, 1);
        }

        /* Done */
        return;
    }

    /* Hack -- Notice equipment changes */
    if (prefix(msg, "You are wielding") ||
		prefix(msg, "You are shooting with ") ||
		prefix(msg, "Your light source is") ||
		prefix(msg, "You are wearing"))
    {
        /* Done */
		borg_note("# Parsed my equipment change.");
		return;
    }

	/* Hack -- Notice Quest complete, and turn off recall so we can grab our reward */
    if (prefix(msg, "You just completed your quest!") && goal_recalling)
    {
		if (goal_recalling)
		{
			borg_note("# Dismiss the recall to claim my reward.");
			goal_recalling = 0;
			if (borg_recall());
		}
	}

    /* Hack -- Notice "failure" */
    if (prefix(msg, "You failed ") ||
		prefix(msg, "You've failed "))
    {
        /* Hack -- store the keypress */
        borg_note("# Normal failure.");

        /* Set the failure flag */
        borg_failure = TRUE;

        /* Flush our key-buffer */
        borg_flush();

        /* If we were casting a targetted spell and failed */
        /* it does not mean we can't target that location */
        successful_target = 0;

        /* Incase we failed our emergency use of MM */
        borg_confirm_target = FALSE;

        /* Check to see if it was a door then convert it */
        if (ag->feat == FEAT_DOOR_HEAD)
        {
            /* What is my chance of opening the door? */
            if (borg_skill[BI_DIS] < 20)
            {
                /* Set door as jammed, then bash it */
                ag->feat = FEAT_DOOR_HEAD + 0x08;
            }
        }

        /* check for glyphs since we no longer have a launch message */
        if (borg_casted_glyph)
        {
            /* Forget the newly created-though-failed  glyph */
            track_glyph_num --;
            track_glyph_x[track_glyph_num] = 0;
            track_glyph_y[track_glyph_num] = 0;
            borg_note("# Removing glyph from array,");
            borg_casted_glyph = FALSE;
        }

        /* Incase it was a failed goi refresh */
        if (borg_attempting_refresh)
        {
            if (borg_goi > 1) borg_goi -=12000;
			if (borg_wraith >1) borg_wraith -=37000;
			borg_attempting_refresh = FALSE;
        }

        return;

    }


    /* Ignore teleport trap */
    if (prefix(msg, "You hit a teleport")) return;

    /* Ignore arrow traps */
    if (prefix(msg, "An arrow ")) return;

    /* Ignore dart traps */
    if (prefix(msg, "A small dart ")) return;

    if (prefix(msg, "The cave "))
    {
        borg_react(msg, "QUAKE");
		/* Must make a new Sea too */
		borg_needs_new_sea = TRUE;
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;

        return;
    }

	/* Finished a quest.  The drops are better. */
    if (prefix(msg, "A magical staircase appears..."))
    {
		/* I consider the drops as augmented in value */
        borg_questor_died = borg_t;
		return;
    }

	/* need to check stat */
    if (prefix(msg, "You feel very") ||
        prefix(msg, "You feel less") ||
        prefix(msg, "Wow!  You feel very"))
    {
        /* need to check str */
        if (prefix(msg, "You feel very weak"))
        {
            my_need_stat_check[0] = TRUE;
        }
        if (prefix(msg, "You feel less weak"))
        {
            my_need_stat_check[0] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very strong"))
        {
            my_need_stat_check[0] = TRUE;
        }

        /* need to check int */
        if (prefix(msg, "You feel very stupid"))
        {
            my_need_stat_check[1] = TRUE;
        }
        if (prefix(msg, "You feel less stupid"))
        {
            my_need_stat_check[1] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very smart"))
        {
            my_need_stat_check[1] = TRUE;
        }

        /* need to check wis */
        if (prefix(msg, "You feel very naive"))
        {
            my_need_stat_check[2] = TRUE;
        }
        if (prefix(msg, "You feel less naive"))
        {
            my_need_stat_check[2] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very wise"))
        {
            my_need_stat_check[2] = TRUE;
        }

        /* need to check dex */
        if (prefix(msg, "You feel very clumsy"))
        {
            my_need_stat_check[3] = TRUE;
        }
        if (prefix(msg, "You feel less clumsy"))
        {
            my_need_stat_check[3] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very dextrous"))
        {
            my_need_stat_check[3] = TRUE;
        }

        /* need to check con */
        if (prefix(msg, "You feel very sickly"))
        {
            my_need_stat_check[4] = TRUE;
        }
        if (prefix(msg, "You feel less sickly"))
        {
            my_need_stat_check[4] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very healthy"))
        {
            my_need_stat_check[4] = TRUE;
        }

        /* need to check cha */
        if (prefix(msg, "You feel very ugly"))
        {
            my_need_stat_check[5] = TRUE;
        }
        if (prefix(msg, "You feel less ugly"))
        {
            my_need_stat_check[5] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very cute"))
        {
            my_need_stat_check[5] = TRUE;
        }
    }

    /* time attacks, just do all stats. */
    if (prefix(msg, "You're not as"))
    {
        my_need_stat_check[0] = TRUE;
        my_need_stat_check[1] = TRUE;
        my_need_stat_check[2] = TRUE;
        my_need_stat_check[3] = TRUE;
        my_need_stat_check[4] = TRUE;
        my_need_stat_check[5] = TRUE;
    }

    /* Nexus attacks, need to check everything! */
    if (prefix(msg, "Your body starts to scramble..."))
    {
        my_need_stat_check[0] = TRUE;
        my_need_stat_check[1] = TRUE;
        my_need_stat_check[2] = TRUE;
        my_need_stat_check[3] = TRUE;
        my_need_stat_check[4] = TRUE;
        my_need_stat_check[5] = TRUE;

        /* max stats may have lowered */
        my_stat_max[0] = 0;
        my_stat_max[1] = 0;
        my_stat_max[2] = 0;
        my_stat_max[3] = 0;
        my_stat_max[4] = 0;
        my_stat_max[5] = 0;

    }

    /* amnesia attacks, re-id wands, staves, equipment. */
    if (prefix(msg, "Your memories fade") ||
		prefix(msg, "You forget everything in your utmost terror!") ||
		prefix(msg, "You wake up somewhere with a sore head...") ||
		prefix(msg, "Oh, no! Your mind has gone blank!") ||
		prefix(msg, "Huh? Who am I? What am I doing here?"))
    {
        int i;
		int y, x;

        /* I was hit by amnesia, forget things */
        /* forget equipment */
        /* Look for an item to forget (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip known items */
            if (item->fully_identified) continue;

            /* skip certain easy know items */
            if ((item->tval == TV_RING) &&
                ((item->sval == SV_RING_FREE_ACTION) ||
                 (item->sval == SV_RING_SEE_INVIS) ||
                 (item->sval <= SV_RING_SUSTAIN_CHR))) continue;

            /* skip already forgotten or non id'd items */
            if (!item->aware) continue;

            /* forget it */
            item->aware = FALSE;

            /* note the forgeting */
            borg_note(format("Borg 'forgetting' qualities of %s",item->desc));

        }

        /* Look for an item to forget (inventory) */
        for (i = 0; i <= INVEN_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* skip certain easy know items */
            if ((item->tval == TV_RING) &&
                (item->flags3 & TR3_EASY_KNOW)) continue;

            if (item->fully_identified) continue;

            switch (item->tval)
            {
                /* forget wands, staffs, weapons, armour */
                case TV_WAND:
                case TV_STAFF:
                case TV_ROD:
                case TV_RING:
                case TV_AMULET:
                case TV_LITE:
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
                break;

                default:
                    continue;
            }
                /* forget it */
                item->aware = FALSE;

                /* note the forgetting */
                borg_note(format("Borg 'forgetting' qualities of %s",item->desc));
         }

		/* Forget maping qualities */
        track_less_num = 0;
        track_more_num = 0;
		when_wizard_lite = 0;

		/* Hack -- Clear "panel" flags */
        for (y = 0; y < 6; y++)
        {
            for (x = 0; x < 6; x++)
            {
                borg_detect_wall[y][x] = FALSE;
                borg_detect_trap[y][x] = FALSE;
                borg_detect_door[y][x] = FALSE;
                borg_detect_evil[y][x] = FALSE;
            }
        }

    }

    if (streq(msg, "You have been knocked out."))
    {
        borg_note("Ignoring Messages While KO'd");
        borg_dont_react = TRUE;
    }

    if (streq(msg, "You are paralyzed"))
    {
        borg_note("# Ignoring Messages While Paralyzed");
        borg_dont_react = TRUE;
    }

	/* Hallucination --launch */
    if (streq(msg, "Oh, wow! Everything looks so cosmic now!"))
    {
        borg_note("# Hallucinating.  Special control of wanks.");
        borg_skill[BI_ISIMAGE] = TRUE;
    }

	/* Hallucination -- Close */
    if (streq(msg, "You can see clearly again."))
    {
        borg_note("# Hallucination ended.  Normal control of wanks.");
        borg_skill[BI_ISIMAGE] = FALSE;
    }

	/* Anti-Magic Zangband Curse effect */
    if (prefix(msg, "An anti-magic shell"))
    {
        borg_note("# Anti-magic curse.");
        borg_skill[BI_CRSNOMAGIC] = TRUE;
    }

	/* Anti-Magic Zangband Curse effect */
    if (prefix(msg, "Magical energy flows "))
    {
        borg_note("# Releasing my pent-up energy.  Look out, Frankie");
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
		borg_target(c_y,c_x);
    }

	/* Hit somebody */
    if (prefix(msg, "You hit "))
    {
        tmp = strlen("You hit ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You strike "))
    {
        tmp = strlen("You strike ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You hack at "))
    {
        tmp = strlen("You hack at ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You bash "))
    {
        tmp = strlen("You bash ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You slash "))
    {
        tmp = strlen("You slash ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You pound "))
    {
        tmp = strlen("You pound ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You score "))
    {
        tmp = strlen("You score ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You batter "))
    {
        tmp = strlen("You batter ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You gouge "))
    {
        tmp = strlen("You gouge ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Hit somebody */
    if (prefix(msg, "You bludgeon "))
    {
        tmp = strlen("You bludgeon ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }
    /* Hit somebody */
    if (prefix(msg, "You *smite* "))
    {
        tmp = strlen("You *smite* ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

	/* Vampire miss */
    if (prefix(msg, "You bite into thin air"))
    {
		/* There was no monster there.  we need to delete that monster. */
		if (ag->kill) borg_delete_kill(ag->kill);
        return;
    }

    /* Hit somebody */
    if (prefix(msg, "You bite "))
    {
        tmp = strlen("You bite ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Hit somebody */
    if (prefix(msg, "You claw "))
    {
        tmp = strlen("You claw ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Hit somebody */
    if (prefix(msg, "You grin "))
    {
        tmp = strlen("You grin and bare your fangs...");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss "))
    {
        tmp = strlen("You miss ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody (because of fear) */
    if (prefix(msg, "You are too afraid to attack "))
    {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%^s", who);
        borg_react(msg, buf);
        return;
    }


    /* "It screams in pain." (etc) */
    for (i = 0; suffix_pain[i]; i++)
    {
		/* Ignore some messages while resting */
		if (resting)
		{
			borg_note("# Ignoring certain messages while Resting.");
			return;
		}

        /* "It screams in pain." (etc) */
        if (suffix(msg, suffix_pain[i]))
        {
            tmp = strlen(suffix_pain[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "PAIN:%^s", who);
            borg_react(msg, buf);
            return;
        }
    }


    /* "You have killed it." (etc) */
    for (i = 0; prefix_kill[i]; i++)
    {
		/* Ignore some messages while resting */
		if (resting)
		{
			borg_note("# Ignoring certain messages while Resting.");
			return;
		}

        /* "You have killed it." (etc) */
        if (prefix(msg, prefix_kill[i]))
        {
            tmp = strlen(prefix_kill[i]);
            strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
            strnfmt(buf, 256, "KILL:%^s", who);
            borg_react(msg, buf);
            return;
        }
    }


    /* "It dies." (etc) */
    for (i = 0; suffix_died[i]; i++)
    {
		/* Ignore some messages while resting */
		if (resting)
		{
			borg_note("# Ignoring certain messages while Resting.");
			return;
		}

        /* "It dies." (etc) */
        if (suffix(msg, suffix_died[i]))
        {
            tmp = strlen(suffix_died[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "DIED:%^s", who);
            borg_react(msg, buf);
            return;
        }
    }

    /* "It blinks or telports." (etc) */
    for (i = 0; suffix_blink[i]; i++)
    {
        /* "It teleports." (etc) */
        if (suffix(msg, suffix_blink[i]))
        {
            tmp = strlen(suffix_blink[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "BLINK:%^s", who);
            borg_react(msg, buf);
            return;
        }
    }

    /* "One monsters attacks another." (etc) */
    for (i = 0; suffix_mon_mon[i]; i++)
    {
		/* Ignore some messages while resting */
		if (resting)
		{
			borg_note("# Ignoring certain messages while Resting.");
			return;
		}

        /* "AAA attacks BBB." (etc) */
        if (strstr(msg, suffix_mon_mon[i]))
        {
			/* Ignore the monsters fighting */
			borg_note("# Ignoring monster quarrel");
            return;
        }
    }

    /* "It misses you." */
    if (suffix(msg, " misses you."))
    {
        tmp = strlen(" misses you.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "MISS_BY:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* "It is repelled.." */
    /* treat as a miss */
    if (suffix(msg, " is repelled."))
    {
        tmp = strlen(" is repelled.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "MISS_BY:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* "It hits you." (etc) */
    for (i = 0; suffix_hit_by[i]; i++)
    {
        /* "It hits you." (etc) */
        if (suffix(msg, suffix_hit_by[i]))
        {
            tmp = strlen(suffix_hit_by[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "HIT_BY:%^s", who);
            borg_react(msg, buf);

            /* If I was hit, then I am not on a glyph */
            track_glyph_num = 0;
            return;
        }
    }


    /* "It casts a spell." (etc) */
    for (i = 0; suffix_spell[i]; i++)
    {
        /* "It casts a spell." (etc) */
        if (suffix(msg, suffix_spell[i]))
        {
            tmp = strlen(suffix_spell[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "SPELL_%03d:%^s", i, who);
            borg_react(msg, buf);
            return;
        }
    }

    /* State -- Asleep */
    if (suffix(msg, " falls asleep!"))
    {
        tmp = strlen(" falls asleep!");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_SLEEP:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- confused */
    if (suffix(msg, " looks confused."))
    {
        tmp = strlen(" looks confused.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_CONFUSED:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- confused */
    if (suffix(msg, " looks more confused."))
    {
        tmp = strlen(" looks more confused.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_CONFUSED:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Asleep */
    if (suffix(msg, " wakes up."))
    {
        tmp = strlen(" wakes up.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_AWAKE:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Afraid */
    if (suffix(msg, " flees in terror!"))
    {
        tmp = strlen(" flees in terror!");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__FEAR:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers his courage."))
    {
        tmp = strlen(" recovers his courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers her courage."))
    {
        tmp = strlen(" recovers her courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers its courage."))
    {
        tmp = strlen(" recovers its courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
    }

	/* Quest Level */
	if (prefix(msg, "Be warned, this level is guarded by "))
	{
		tmp = strlen("Be warned, this level is guarded by   ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
		strnfmt(buf, 256, "STATE__QUEST:%^s", who);
        borg_react(msg, buf);
		return;
	}
    /* Feature XXX XXX XXX */
    if (streq(msg, "The door appears to be broken."))
    {
        /* Only process open doors */
        if (ag->feat == FEAT_OPEN)
        {
            /* Mark as broken */
            ag->feat = FEAT_BROKEN;

            /* Clear goals */
            goal = 0;
        }

        /* We might need to delete a monster which we
         * thought was there.
         */
        if (ag->kill)
        {
			int i;
			int b_i = -1;

		    borg_kill *kill;

			/* Need to locate monster index */
		    /* Scan the monsters */
		    for (i = 1; i < borg_kills_nxt; i++)
		    {
		        kill = &borg_kills[i];

		        /* Skip "dead" monsters */
		        if (!kill->r_idx) continue;

				/* Verify locations */
				if (kill->y != g_y || kill->x != g_x) continue;

				/* Store the index of this monster */
				b_i = i;
			}

			/* Need to delete this monster */
			if (b_i) borg_delete_kill(b_i);
		}
        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "The door appears to be stuck."))
    {
        /* Only process non-jammed doors */
        if ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_HEAD + 0x07))
        {
            /* Mark the door as jammed */
            ag->feat = FEAT_DOOR_HEAD + 0x08;

            /* Clear goals */
            goal = 0;
        }

        return;
    }



    /* Feature XXX XXX XXX */
    if (streq(msg, "This seems to be permanent rock."))
    {
        /* Only process walls */
        if ((ag->feat >= FEAT_WALL_EXTRA) && (ag->feat <= FEAT_PERM_SOLID))
        {
            /* Mark the wall as permanent */
            ag->feat = FEAT_PERM_EXTRA;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the granite wall."))
    {
        /* reseting my panel clock */
        time_this_panel = 1;

        /* Only process walls */
        if ((ag->feat >= FEAT_WALL_EXTRA) && (ag->feat <= FEAT_PERM_SOLID))
        {
            /* Mark the wall as granite */
            ag->feat = FEAT_WALL_EXTRA;

            /* Clear goals */
            if (goal != GOAL_DIGGING) goal = 0;
        }

        return;
    }

    /* Feature Invisible Walls */
    if (streq(msg, "You bump into something."))
    {
        /* Only process floor grids */
        if (ag->feat == FEAT_FLOOR)
        {
            /* Mark the wall as granite */
            ag->feat = FEAT_WALL_EXTRA;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the quartz vein."))
    {
        /* Process magma veins with treasure */
        if (ag->feat == FEAT_MAGMA_K)
        {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ_K;

            /* Clear goals */
            goal = 0;
        }

        /* Process magma veins */
        else if (ag->feat == FEAT_MAGMA)
        {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the magma vein."))
    {
        /* Process quartz veins with treasure */
        if (ag->feat == FEAT_QUARTZ_K)
        {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA_K;

            /* Clear goals */
            goal = 0;
        }

        /* Process quartz veins */
        else if (ag->feat == FEAT_QUARTZ)
        {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Word of Recall -- Ignition */
    if (prefix(msg, "The air about you becomes "))
    {
        /* Initiate recall */
        /* Guess how long it will take to lift off */
        goal_recalling = 15000 + 5000; /* Guess. game turns x 1000 ( 15+rand(20))*/
        return;
    }

    /* Word of Recall -- Lift off */
    if (prefix(msg, "You feel yourself yanked "))
    {
        /* Recall complete */
        goal_recalling = 0;
        return;
    }

    /* Word of Recall -- Cancelled */
    if (prefix(msg, "A tension leaves "))
    {
        /* Hack -- Oops */
        goal_recalling = 0;
        return;
    }

    /* Shadow Shifting - Alternate Reality - makes a new level. */
    if (prefix(msg, "You start walking around."))
    {
        /* new level forces a complete re-explore. */
        old_depth = 126;
        return;
    }

	/* Wearing Cursed Item */
    if ((prefix(msg, "Oops! It feels deathly cold!")) ||
        (suffix(msg, " appears to be cursed.")) ||
        (suffix(msg, " seems to be cursed.")) )
    {
        /* Hack -- Oops */
        return;
    }

    /* protect from evil */
    if (prefix(msg, "You feel safe from evil!"))
    {
        borg_prot_from_evil = TRUE;
        return;
    }
    if (prefix(msg, "You no longer feel safe from evil."))
    {
        borg_prot_from_evil = FALSE;
        return;
    }
    /* haste self */
    if (prefix(msg, "You feel yourself moving faster!"))
    {
        borg_speed = TRUE;
        return;
    }
    if (prefix(msg, "You feel yourself slow down."))
    {
        borg_speed = FALSE;
        return;
    }
    /* Bless */
    if (prefix(msg, "You feel righteous!"))
    {
        borg_bless = TRUE;
        return;
    }
    if (prefix(msg, "The prayer has expired."))
    {
        borg_bless = FALSE;
        return;
    }

    /* hero */
    if (prefix(msg, "You feel like a hero!"))
    {
        borg_hero = TRUE;
        return;
    }
    if (prefix(msg, "The heroism wears off."))
    {
        borg_hero = FALSE;
        return;
    }
    /* berserk */
    if (prefix(msg, "You feel like a killing machine!"))
    {
        borg_berserk = TRUE;
        return;
    }
    if (prefix(msg, "You feel less Berserk."))
    {
        borg_berserk = FALSE;
        return;
    }

    /* Wraithform */
    if (prefix(msg, "You leave the physical world and turn into a wraith-being!"))
    {
        borg_wraith = 32000;
        return;
    }
    if (prefix(msg, "You feel opaque.") ||
		prefix(msg, "The light forces you"))
    {
        borg_wraith = 0;
        return;
    }

    /* check for wall blocking but not when confused*/
    if ((prefix(msg, "There is a wall ") &&
        (!borg_skill[BI_ISCONFUSED])))
    {
        my_need_redraw = TRUE;
        my_need_alter = TRUE;
        goal = 0;
        return;
    }


    /* check for closed door but not when confused*/
    if ((prefix(msg, "There is a closed door blocking your way.") &&
        (!borg_skill[BI_ISCONFUSED])))
    {
        my_need_redraw = TRUE;
        my_need_alter = TRUE;
        goal = 0;
        return;
    }

    /* check for mis-alter command.  Sometime induced by never_move guys*/
    if (streq(msg, "You spin around.") &&
    !borg_skill[BI_ISCONFUSED])
    {

        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill *kill = &borg_kills[i];

            /* Skip dead monsters */
            if (!kill->r_idx) continue;

            /* Distance components */
            if ((c_y == kill->y && c_x == kill->x) ||
               (g_y == kill->y && g_x == kill->x))
            {
                /* Hack -- kill em */
                borg_delete_kill(i);
            }
        }

        my_no_alter = TRUE;
        goal = 0;
        return;
    }

    /* check for mis-alter command.  Sometime induced by never_move guys*/
    if (streq(msg, "You attack the empty air.") &&
    !borg_skill[BI_ISCONFUSED])
    {
		int ax, ay, d;

        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill *kill = &borg_kills[i];

            /* Skip dead monsters */
            if (!kill->r_idx) continue;

	        /* Distance components */
	        ax = (kill->x > c_x) ? (kill->x - c_x) : (c_x - kill->x);
	        ay = (kill->y > c_y) ? (kill->y - c_y) : (c_y - kill->y);

	        /* Distance */
	        d = MAX(ax, ay);

	        /* Minimal distance */
	        if (d > 3) continue;

            /* Hack -- kill monsters close to me */
            borg_delete_kill(i);
        }

        my_no_alter = TRUE;
        goal = 0;
        return;
    }

    /* Feature XXX XXX XXX */
    if (prefix(msg, "You see nothing there "))
    {
        borg_grids[c_y][c_x].feat = FEAT_BROKEN;

        my_no_alter = TRUE;
        /* Clear goals */
        goal = 0;
        return;

    }
    /* Tunneling not understood correctly */
    if (prefix(msg, "You cannot tunnel through air."))
    {
        ag->feat = FEAT_BROKEN;

        my_no_alter = TRUE;

        /* Clear goals */
        goal = 0;
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
        return;

    }

    /* Pet or friendly in the flow path */
    if (prefix(msg, "You stop to avoid hitting your") ||
		suffix(msg, "is in your way!"))
    {
        my_need_alter = TRUE;
        return;

    }

	/* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Illegal "))
    {
        /* Hack -- Oops */
        borg_respawning = 7;
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel +=100;
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "You have nothing to identify"))
    {
        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel +=100;

        /* ID all items (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->aware = TRUE;
        }

        /* ID all items  (inventory) */
        for (i = 0; i <= INVEN_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->aware = TRUE;
        }
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Identifying The Phial"))
    {

        /* ID item (equipment) */
        borg_item *item = &borg_items[INVEN_LITE];
        item->aware = TRUE;

        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel +=100;
    }

    /* resist acid */
    if (prefix(msg, "You feel resistant to acid!"))
    {
        borg_skill[BI_TRACID] = TRUE;
        return;
    }
    if (prefix(msg, "You feel less resistant to acid."))
    {
        borg_skill[BI_TRACID] = FALSE;
        return;
    }
    /* resist electricity */
    if (prefix(msg, "You feel resistant to electricity!"))
    {
        borg_skill[BI_TRELEC] = TRUE;
        return;
    }
    if (prefix(msg, "You feel less resistant to electricity."))
    {
        borg_skill[BI_TRELEC] = FALSE;
        return;
    }
    /* resist fire */
    if (prefix(msg, "You feel resistant to fire!"))
    {
        borg_skill[BI_TRFIRE] = TRUE;
        return;
    }
    if (prefix(msg, "You feel less resistant to fire."))
    {
        borg_skill[BI_TRFIRE] = FALSE;
        return;
    }
    /* resist cold */
    if (prefix(msg, "You feel resistant to cold!"))
    {
        borg_skill[BI_TRCOLD] = TRUE;
        return;
    }
    if (prefix(msg, "You feel less resistant to cold."))
    {
        borg_skill[BI_TRCOLD] = FALSE;
        return;
    }
    /* resist poison */
    if (prefix(msg, "You feel resistant to poison!"))
    {
        borg_skill[BI_TRPOIS] = TRUE;
        return;
    }
    if (prefix(msg, "You feel less resistant to poison."))
    {
        borg_skill[BI_TRPOIS] = FALSE;
        return;
    }
    /* GOI! */
    if (prefix(msg, "Invulnerability!"))
    {
        borg_goi = 12000; /* keep track of how long it has left (a guess) */
        return;
    }
    if (prefix(msg, "The invulnerability wears off."))
    {
        borg_goi = 0;
        return;
    }
    /* Telepathy */
    if (prefix(msg, "You feel your consciousness expand!"))
    {
        borg_esp = TRUE;
        return;
    }
    if (prefix(msg, "Your consciousness contracts again."))
    {
        borg_esp = FALSE;
        return;
    }

	/* Invisible */

    /* Shield */
    if (prefix(msg, "Your skin turns to stone") ||
        prefix(msg, "A mystic shield forms around your body!"))
    {
        borg_shield = TRUE;
        return;
    }
    if (prefix(msg, "Your skin returns to normal.") ||
        prefix(msg, "Your mystic shield crumbles away."))
    {
        borg_shield = FALSE;
        return;
    }

    /* Glyph of Warding (the spell no longer gives a report)*/
    /* Sadly  Rune of Protection has no message */
    if (prefix(msg, "You inscribe a mystic symbol on the ground!"))
    {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph_num; i++)
            {
                /* Stop if we already new about this glyph */
                if ((track_glyph_x[i] == c_x) && (track_glyph_y[i] == c_y)) break;
            }

            /* Track the newly discovered glyph */
            if ((i == track_glyph_num) && (i < track_glyph_size))
            {
                borg_note("# Noting the creation of a glyph.");
                track_glyph_x[i] = c_x;
                track_glyph_y[i] = c_y;
                track_glyph_num++;
            }
		    return;
    }
    if (prefix(msg, "The rune of protection is broken!"))
    {
        /* we won't know which is broken so erase them all and
         * allow the borg to scan the screen and rebuild the array.
         * He won't see the one under him though.  So a special check
         * must be made.
         */
        byte feat = c_ptr->feat;

         /* Remove the entire array */
         for (i = 0; i < track_glyph_num; i++)
         {
             /* Stop if we already new about this glyph */
             track_glyph_x[i] = 0;
             track_glyph_y[i] = 0;
             track_glyph_num = 0;
         }

         /* no known glyphs */
         track_glyph_num = 0;

        /* Check for glyphs under player -- Cheat*/
        if (feat == FEAT_GLYPH ||
        	feat == FEAT_MINOR_GLYPH)
        {
            track_glyph_x[track_glyph_num] = c_x;
            track_glyph_y[track_glyph_num] = c_y;
            track_glyph_num++;
        }
        return;
    }
    /* failed glyph spell message */
    if (prefix(msg, "The object resists the spell"))
    {

        /* Forget the newly created-though-failed  glyph */
        track_glyph_x[track_glyph_num] = 0;
        track_glyph_y[track_glyph_num] = 0;
        track_glyph_num --;

        /* note it */
        borg_note("# Removing the Glyph under me, replacing with broken door.");

        /* mark that we are not on a clear spot.  The borg ignores
         * broken doors and this will keep him from casting it again.
         */
        borg_grids[c_y][c_x].feat = FEAT_BROKEN;
        return;
    }

    /* Removed rubble.  Important when out of lite */
    if (prefix(msg, "You have removed the "))
    {
        int x, y;
        /* remove rubbles from array */
        for (y = c_y -1; y < c_y +1; y++)
        {
            for (x = c_x -1; x < c_x +1; x++)
            {
                /* replace all rubble with broken doors, the borg ignores
                 * broken doors.  This routine is only needed if the borg
                 * is out of lite and searching in the dark.
                 */
                 if (borg_skill[BI_CUR_LITE]) continue;

                 if (ag->feat == FEAT_RUBBLE) ag->feat = FEAT_BROKEN;
             }
         }
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
        return;
    }

    if (prefix(msg, "The enchantment failed"))
    {
        /* reset our panel clock for this */
        time_this_panel = 1;
        return;
    }

    /* need to kill monsters when WoD is used */
    if (prefix(msg, "There is a searing blast of light!"))
    {
        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill *kill = &borg_kills[i];

            int x9 = kill->x;
            int y9 = kill->y;
            int ax, ay, d;

            /* Skip dead monsters */
            if (!kill->r_idx) continue;

            /* Distance components */
            ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
            ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

            /* Distance */
            d = MAX(ax, ay);

            /* Minimal distance */
            if (d > 12) continue;

            /* Hack -- kill em */
            borg_delete_kill(i);
        }

        /* Remove the region fear as well */
        borg_fear_region[c_y/11][c_x/11] = 0;

        return;
    }

    /* Be aware and concerned of busted doors */
    if (prefix(msg, "You hear a door burst open!"))
    {
        /* on level 1 and 2 be concerned.  Could be Grip or Fang */
        if (borg_skill[BI_CDEPTH] <= 3 && borg_skill[BI_CLEVEL] <= 5) borg_depth |= DEPTH_SCARY;
    }

	/* Some spells move the borg from his grid */
	if (prefix(msg, "commands you to return.") ||
	    prefix(msg, "teleports you away.") ||
	    prefix(msg, "gestures at your feet."))
	{
		/* If in Lunal mode better shut that off, he is not on the stairs anymore */
		if (borg_lunal_mode) borg_lunal_mode = FALSE;
		borg_note("# Disconnecting Lunal Mode due to monster spell.");
	}

	/* Recognize Drownin */
    if (prefix(msg, "You are drowning"))
    {
        borg_note(format("# Help! I can't swim.  (borg carry cap:%d, real carry cap:%d, real weight:%d)",
        	((adj_str_wgt[my_stat_ind[A_STR]] * 100) / 2),
        	((adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100) / 2),
        	p_ptr->total_weight));

		/* just in case, make the grid under us water */
		borg_grids[c_y][c_x].feat = FEAT_DEEP_WATER;

        return;
    }

	/* Recognize hurting */
    if (prefix(msg, "The lava burns you"))
    {
        borg_note("# Help! I'm burning");

		/* just in case, make the grid under us lava */
		borg_grids[c_y][c_x].feat = FEAT_SHAL_LAVA;

        return;
    }

	/* Recognize hurting */
    if (prefix(msg, "You are being crushed!"))
    {
        borg_note("# Help! I'm stuck in a stupid wall.");

		/* just in case, make the grid under us a wall.*/
		borg_grids[c_y][c_x].feat = FEAT_WALL_INNER;

        return;
    }

	/* Feelings about the level */
    for (i = 0; prefix_feeling[i]; i++)
    {
        /* "You feel..." (etc) */
        if (prefix(msg, prefix_feeling[i]))
        {
            strnfmt(buf, 256, "FEELING:%d", i);
            borg_react(msg, buf);
            return;
        }
    }

    /* Trump weapons can induce random teleports.  They need to be inscribed with a . */
    if (prefix(msg, "Teleport (Y/N)"))
    {
		/* Trump weapons inscribed with a . do not induce teleportation */

	}
    /* Make sure I am not fooling myself when stair scumming. */
    if (prefix(msg, "I see no up staircase here") ||
		prefix(msg, "I see no down staircase here"))
    {
		/* Remove all stairs from the array. */
        track_less_num = 0;
        track_more_num = 0;
		borg_on_dnstairs = FALSE;
		borg_on_upstairs = FALSE;
		borg_grids[c_y][c_x].feat = FEAT_BROKEN;

	}

	/* Track the bad grids for DimDoor landing */
	if (prefix(msg, "You fail to exit the astral plane correctly!"))
	{
		/* Add this to the list of bad grids */
        borg_note("# Noting the bad landing grid.");
        track_land_num++;
		return;
	}

	/* Stone to mud effect */
	if (suffix(msg, "turns to mud!"))
	{
		/* Redo the borg_view array */
		borg_do_update_view = TRUE;
		borg_do_update_lite = TRUE;
	}

    /* Help with crushing artifacts */
    if (prefix(msg, "You cannot destroy "))
    {
        /* Locate the name of the item which cannot be crushed */
		tmp = strlen("You cannot destroy ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);

		/* Locate the inventory position */
		for (i = 0; i < INVEN_PACK; i++)
		{
			/* Skip empty items */
			if (!borg_items[i].iqty) continue;

			/* Change the inscription */
			if (strstr(borg_items[i].desc, who))
			{
				borg_send_inscribe(i, "special");
				break;
			}
		}
	}
}



/*
 * Parse a message, piece of a message, or set of messages.
 *
 * We must handle long messages which are "split" into multiple
 * pieces, and also multiple messages which may be "combined"
 * into a single set of messages.
 */
static void borg_parse(cptr msg)
{
    static char len = 0;
    static char buf[1024];


    /* Flush messages */
    if (len && (!msg || (msg[0] != ' ')))
    {
        int i, j;

        /* Split out punctuation */
        for (j = i = 0; i < len-1; i++)
        {
            /* Check for punctuation */
            if ((buf[i] == '.') ||
                (buf[i] == '!') ||
                (buf[i] == '?') ||
                (buf[i] == '"'))
            {
                /* Require space */
                if (buf[i+1] == ' ')
                {
                    /* Terminate */
                    buf[i+1] = '\0';

                    /* Parse fragment */
                    borg_parse_aux(buf + j, (i + 1) - j);

                    /* Restore */
                    buf[i+1] = ' ';

                    /* Advance past spaces */
                    for (j = i + 2; buf[j] == ' '; j++) /* loop */;
                }
            }
        }

        /* Parse tail */
        borg_parse_aux(buf + j, len - j);

        /* Forget */
        len = 0;
    }


    /* No message */
    if (!msg)
    {
        /* Start over */
        len = 0;
    }

    /* Continued message */
    else if (msg[0] == ' ')
    {
        /* Collect, verify, and grow */
        len += strnfmt(buf+len, 1024-len, "%s", msg+1);
    }

    /* New message */
    else
    {
        /* Collect, verify, and grow */
        len = strnfmt(buf, 1024, "%s", msg);
    }
}

static int
borg_getval(char ** string, char * val)
{
    char    string2[4];
    int     retval;

    if (!prefix(*string, val))
    {
        return -1000;
    }
    (*string) += strlen(val);
    memmove(string2, *string, 3);
    string2[3] = 0;
    sscanf(string2, "%d", &retval);
    *string+=3;
    return retval;
}

static bool borg_load_formula(char * string)
{
    int formula_num;
    int iformula = 0;
    int x = 0;
    int value= 0;
    char string2[4];

    memmove(string2, string, 3);
    string2[3] = 0;
    sscanf(string2, "%d", &formula_num);
    string+=4;
    if (formula[formula_num])
    {
        borg_note(format("formula defined twice %03d", formula_num));
        return FALSE;
    }
    C_MAKE(formula[formula_num], MAX_FORMULA_ELEMENTS, int);

    while (string && *string)
    {
        switch (*string)
        {
            case ' ':
                string++;
                continue;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                sscanf(string, "%d", &value);
                if (iformula+2 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
                }
                formula[formula_num][iformula++] = BFO_NUMBER;
                formula[formula_num][iformula++] = value;
                break;
            case '_':
                if (iformula+2 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
                }
                formula[formula_num][iformula++] = BFO_VARIABLE;
                if (-1000 != (value = borg_getval(&string, "_ITEM")))
                {
                    formula[formula_num][iformula++] = value;
                    break;
                }
                if (-1000 != (value = borg_getval(&string, "_WITEM")))
                {
                    formula[formula_num][iformula++] = value + max_k_idx;
                    break;
                }
                if (-1000 != (value = borg_getval(&string, "_ARTIFACT")))
                {
                    formula[formula_num][iformula++] = value +
                                                       max_k_idx +
                                                       max_k_idx;
                    break;
                }

                for (x = 0; x < BI_MAX; x++)
                {
                    if (prefix(string, prefix_pref[x]))
                    {
                        value = max_k_idx + max_k_idx + max_a_idx + x;
                        formula[formula_num][iformula++] = value;
                        break;
                    }
                }
                if (x == BI_MAX)
                {
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    borg_note(format("bad item in formula%03d %s", formula_num, string));
                    return FALSE;
                }

                break;
            default:
                if (iformula+1 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
                }
                if (*string == '>')
                {
                    if (*(string+1) == '=')
                    {
                        formula[formula_num][iformula++] = BFO_GTE;
                        break;
                    }
                    formula[formula_num][iformula++] = BFO_GT;
                    break;
                }
                if (*string == '<')
                {
                    if (*(string+1) == '=')
                    {
                        formula[formula_num][iformula++] = BFO_LTE;
                        break;
                    }
                    formula[formula_num][iformula++] = BFO_LT;
                    break;
                }
                if (*string == '!')
                {
                    if (*(string+1) == '=')
                        formula[formula_num][iformula++] = BFO_NEQ;
                    else
                        formula[formula_num][iformula++] = BFO_NOT;
                    break;
                }
                if (*string == '=')
                {
                    formula[formula_num][iformula++] = BFO_EQ;
                    break;
                }
                if (*string == '&')
                {
                    formula[formula_num][iformula++] = BFO_AND;
                    break;
                }
                if (*string == '-')
                {
                    /* - followed by space is a minus.  */
                    if (*(string+1) == ' ')
                    {
                        formula[formula_num][iformula++] = BFO_MINUS;
                        break;
                    }
                    if (iformula+1 > MAX_FORMULA_ELEMENTS)
                    {
                        borg_note(format("too many elements in formula %03d", formula_num));
                        formula[formula_num][0] = BFO_NUMBER;
                        formula[formula_num][1] = 0;
                        return FALSE;
                    }

                    /* - followed by anything else is a negative number */
                    sscanf(string, "%d", &value);
                    formula[formula_num][iformula++] = BFO_NUMBER;
                    formula[formula_num][iformula++] = value;
                    break;
                }
                if (*string == '+')
                {
                    formula[formula_num][iformula++] = BFO_PLUS;
                    break;
                }
                if (*string == '/')
                {
                    formula[formula_num][iformula++] = BFO_DIVIDE;
                    break;
                }
                if (*string == '*')
                {
                    formula[formula_num][iformula++] = BFO_MULT;
                    break;
                }
                if (*string == '|')
                {
                    formula[formula_num][iformula++] = BFO_OR;
                    break;
                }
                borg_note(format("bad item in formula %03d %s", formula_num, string));
                formula[formula_num][0] = BFO_NUMBER;
                formula[formula_num][1] = 0;
                return FALSE;
        }
        string = strchr(string, ' ');
    }
    if (!borg_check_formula(formula[formula_num]))
    {
        borg_note(format("bad formula %03d", formula_num));
        formula[formula_num][0] = BFO_NUMBER;
        formula[formula_num][1] = 0;
        return FALSE;
    }
    return TRUE;
}


static bool add_power_item(int class_num,
                           int depth_num,
                           int cnd_num,
                           int range_to,
                           int range_from,
                           bool each,
                           int item_num,
                           int power)
{
    if ((class_num >= MAX_CLASS &&
        class_num != 999 ) ||
        depth_num >= MAX_DEPTH ||
        item_num >= (max_k_idx + max_k_idx + max_a_idx + BI_MAX) ||
        range_to < range_from)
    {
        borg_note("Malformed item power in zborg.txt: values out of range");
        return FALSE;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASS; class_num ++)
        {
			/* Skip those that are not ours */
			if (class_num != borg_class) continue;

            borg_power_item[class_num][n_pwr[class_num]].depth = depth_num;
            borg_power_item[class_num][n_pwr[class_num]].cnd = cnd_num;
            borg_power_item[class_num][n_pwr[class_num]].item = item_num;
            borg_power_item[class_num][n_pwr[class_num]].power = power;
            borg_power_item[class_num][n_pwr[class_num]].from = range_from;
            borg_power_item[class_num][n_pwr[class_num]].to = range_to;
            borg_power_item[class_num][n_pwr[class_num]].each = each;
            n_pwr[class_num]++;
        }
    }
    else if (class_num == borg_class)
    {
        borg_power_item[class_num][n_pwr[class_num]].depth = depth_num;
        borg_power_item[class_num][n_pwr[class_num]].cnd = cnd_num;
        borg_power_item[class_num][n_pwr[class_num]].item = item_num;
        borg_power_item[class_num][n_pwr[class_num]].power = power;
        borg_power_item[class_num][n_pwr[class_num]].from = range_from;
        borg_power_item[class_num][n_pwr[class_num]].to = range_to;
        borg_power_item[class_num][n_pwr[class_num]].each = each;
        n_pwr[class_num]++;
    }
    return TRUE;
}

static bool borg_load_power(char * string)
{
    int class_num= -1;
    int depth_num= -1;
    int cnd_num = -1;
    int range_to= -1;
    int range_from= -1;
    bool each = FALSE;
    int item_num= -1;
    int power= -1;
    int x;


    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item power in zborg.txt: missing _CLASS");
        return FALSE;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item power in zborg.txt: missing _DEPTH");
        return FALSE;
    }
    if (-1000 == (cnd_num = borg_getval(&string, "_CND")))
    {
        /* condition is optional */
        cnd_num = -1;
    }
    if (-1000 == (range_from = borg_getval(&string, "_RANGE")))
    {
        borg_note("Malformed item power in zborg.txt: missing _RANGE");
        return FALSE;
    }
    if (-1000 == (range_to = borg_getval(&string, "TO")))
    {
        borg_note("Malformed item power in zborg.txt: messed up _RANGE");
        return FALSE;
    }

    if (-1000 != (item_num = borg_getval(&string, "_FORMULA")))
    {
        if (range_to != 999 || range_from != 0)
        {
            borg_note("Malformed item power in zborg.txt: range must be 0-999 formulas");
            return FALSE;
        }
        return add_power_item(class_num,
                       depth_num,
                       cnd_num,
                       range_to,
                       range_from,
                       each,
                       -1,
                       item_num);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ITEM")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = TRUE;

        return add_power_item(class_num,
                       depth_num,
                       cnd_num,
                       range_to,
                       range_from,
                       each,
                       item_num,
                       power);
    }
    if (-1000 != (item_num = borg_getval(&string, "_WITEM")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = TRUE;

        return add_power_item(class_num,
                       depth_num,
                       cnd_num,
                       range_to,
                       range_from,
                       each,
                       max_k_idx + item_num,
                       power);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ARTIFACT")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = TRUE;
        item_num += max_k_idx + max_k_idx;
        return add_power_item(class_num,
                       depth_num,
                       cnd_num,
                       range_to,
                       range_from,
                       each,
                       item_num,
                       power);
    }

    for (x = 0; x < BI_MAX; x++)
    {

        if (prefix(string, prefix_pref[x]))
        {
            string += strlen(prefix_pref[x]);
            item_num = max_k_idx + max_k_idx + max_a_idx + x;
            string++;
            sscanf(string, "%d", &power);
            if (strstr(string, "EACH"))
                each = TRUE;
            return add_power_item(class_num,
                           depth_num,
                           cnd_num,
                           range_to,
                           range_from,
                           each,
                           item_num,
                           power);
        }
    }
    borg_note("Malformed item power in zborg.txt");
    return FALSE;
}
static bool add_required_item(int class_num, int depth_num, int item_num, int number_items)
{
    if ((class_num >= MAX_CLASS &&
        class_num != 999 ) ||
        depth_num >= MAX_DEPTH ||
        item_num >= (max_k_idx + max_k_idx + max_a_idx + BI_MAX))
    {
        borg_note("Malformed item requirment in zborg.txt: value out of range");
        return FALSE;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASS; class_num ++)
        {
			if (class_num != borg_class) continue;

            borg_required_item[class_num][n_req[class_num]].depth = depth_num;
            borg_required_item[class_num][n_req[class_num]].item = item_num;
            borg_required_item[class_num][n_req[class_num]].number = number_items;
            n_req[class_num]++;
        }
    }
    else if (class_num == borg_class)
    {
        borg_required_item[class_num][n_req[class_num]].depth = depth_num;
        borg_required_item[class_num][n_req[class_num]].item = item_num;
        borg_required_item[class_num][n_req[class_num]].number = number_items;
        n_req[class_num]++;
    }
    return TRUE;
}

static bool borg_load_requirement(char * string)
{
    int class_num=-1;
    int depth_num=-1;
    int item_num=-1;
    int number_items=-1;
    int x=-1;

    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item requirment in zborg.txt");
        return FALSE;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item requirment in zborg.txt");
        return FALSE;
    }
    if (-1000 != (item_num = borg_getval(&string, "_FORMULA")))
    {
        return add_required_item(class_num, depth_num, -1, item_num);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ITEM")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, item_num, number_items);
    }
    if (-1000 != (item_num = borg_getval(&string, "_WITEM")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num,  max_k_idx + item_num, number_items);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ARTIFACT")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, max_k_idx + max_k_idx +item_num, number_items);
    }

    for (x = 0; x < BI_MAX; x++)
    {
        if (prefix(string, prefix_pref[x]))
        {
            string += strlen(prefix_pref[x]);
            item_num = max_k_idx + max_k_idx + max_a_idx + x;
            string++;
            sscanf(string, "%d", &number_items);
            return add_required_item(class_num, depth_num, item_num, number_items);
        }
    }
    borg_note("Malformed item requirment in zborg.txt");
    return FALSE;
}

/* just used to do a quick sort on the required items array */
int borg_item_cmp(const void * item1, const void * item2)
{
    if (((req_item*)item1)->depth != ((req_item*)item2)->depth)
        return ((req_item*)item1)->depth - ((req_item*)item2)->depth;
    if (((req_item*)item1)->item != ((req_item*)item2)->item)
        return ((req_item*)item1)->item - ((req_item*)item2)->item;
    return ((req_item*)item1)->number - ((req_item*)item2)->number;
}


/*
 * Initialize zborg.txt
 */

void init_borg_txt_file(void)
{

    FILE *fp;

    char buf[1024];
    int i;

    /* Array of borg variables is stored as */
    /* 0 to k_max = items in inventory */
    /* k_max to 2*k_max  = items being worn */
    /* 2*k_max to a_max  = artifacts worn */
    /* 2*k_max + a_max to end of array = Other skills/possessions */
    size_obj = max_k_idx + max_k_idx + max_a_idx + BI_MAX;

    /* note: C_MAKE automaticly 0 inits things */

	/* Make sure we know who and what we are */
	borg_class = p_ptr->pclass;

    for (i = 0; i < MAX_CLASS; i++)
    {
		if (i != borg_class) continue;

        C_MAKE(borg_required_item[i], 400, req_item); /* externalize the 400 later */
        n_req[i] = 0;
        C_MAKE(borg_power_item[i], 400, power_item); /* externalize the 400 later */
        n_pwr[i] = 0;
    }
    C_MAKE(borg_has, size_obj, int);
    /* make some shortcut pointers into the array */
    borg_has_on = borg_has + max_k_idx;
    borg_artifact = borg_has_on + max_k_idx;
    borg_skill = borg_artifact + max_a_idx;
#ifdef BORG_TK
    path_build(buf, 1024, BORG_DIR_ROOT, "borg.prf");
#else /* BORG_TK */
    path_build(buf, 1024, ANGBAND_DIR_USER, "zborg.txt");
#endif /* BORG_TK */

    /* Open the file */
    fp = my_fopen(buf, "r");

    /* No file, use defaults*/
    if (!fp)
    {
        /* Complain */
        msg_print("*****WARNING***** You do not have a proper zborg.txt file!");
        msg_print("Make sure zborg.txt is located in the 'user' subdirectory!");
        msg_print(NULL);

        /* use default values */
        borg_worships_damage = FALSE;
        borg_worships_speed = FALSE;
        borg_worships_hp= FALSE;
        borg_worships_mana = FALSE;
        borg_worships_ac = FALSE;
        borg_worships_gold = FALSE;
        borg_plays_risky = FALSE;
        borg_scums_uniques = TRUE;
        borg_uses_swaps = TRUE;
        borg_slow_optimizehome = FALSE;
        borg_stop_dlevel = 128;
        borg_stop_clevel = 55;
        borg_stop_king = TRUE;
        borg_uses_calcs = FALSE;
        borg_respawn_winners = FALSE;
        borg_respawn_class = -1;
        borg_respawn_race = -2;
        borg_respawn_realm_1 = 0;
        borg_respawn_realm_2 = 0;
        borg_chest_fail_tolerance = 13;
        borg_delay_factor = 1;
        if (delay_factor == 0) delay_factor = 1;
        borg_money_scum_amount = 0;
        borg_scums_money = FALSE;
		borg_verbose = FALSE;
		borg_munchkin_start = FALSE;
		borg_munchkin_level = 12;
		borg_munchkin_depth = 16;
		borg_lunal_mode = FALSE;
		borg_self_lunal = TRUE;
		borg_enchant_limit = 8;


        return;
    }


    /* Parse the file */
/* AJG needed to make this wider so I could read long formulas */
    while (0 == my_fgets(fp, buf, sizeof(buf)-1))
    {
        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Chop the buffer */
        buf[sizeof(buf)-1] = '\0';

        /* Extract the true/false */
        if (prefix(buf, "borg_worships_damage ="))
        {
            if (buf[strlen("borg_worships_damage =")+1] == 'T' ||
                buf[strlen("borg_worships_damage =")+1] == '1' ||
                buf[strlen("borg_worships_damage =")+1] == 't')
                borg_worships_damage=TRUE;
            else
                borg_worships_damage = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_speed ="))
        {
            if (buf[strlen("borg_worships_speed =")+1] == 'T' ||
                buf[strlen("borg_worships_speed =")+1] == '1' ||
                buf[strlen("borg_worships_speed =")+1] == 't') borg_worships_speed=TRUE;
            else borg_worships_speed = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_hp ="))
        {
            if (buf[strlen("borg_worships_hp =")+1] == 'T' ||
                buf[strlen("borg_worships_hp =")+1] == '1' ||
                buf[strlen("borg_worships_hp =")+1] == 't') borg_worships_hp=TRUE;
            else borg_worships_hp= FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_mana ="))
        {
            if (buf[strlen("borg_worships_mana =")+1] == 'T' ||
                buf[strlen("borg_worships_mana =")+1] == '1' ||
                buf[strlen("borg_worships_mana =")+1] == 't') borg_worships_mana=TRUE;
            else borg_worships_mana = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_ac ="))
        {
            if (buf[strlen("borg_worships_ac =")+1] == 'T' ||
                buf[strlen("borg_worships_ac =")+1] == '1' ||
                buf[strlen("borg_worships_ac =")+1] == 't') borg_worships_ac=TRUE;
            else borg_worships_ac= FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_gold ="))
        {
            if (buf[strlen("borg_worships_gold =")+1] == 'T' ||
                buf[strlen("borg_worships_gold =")+1] == '1' ||
                buf[strlen("borg_worships_gold =")+1] == 't') borg_worships_gold=TRUE;
            else borg_worships_gold= FALSE;
            continue;
        }


        if (prefix(buf, "borg_plays_risky ="))
        {
            if (buf[strlen("borg_plays_risky =")+1] == 'T' ||
                buf[strlen("borg_plays_risky =")+1] == '1' ||
                buf[strlen("borg_plays_risky =")+1] == 't') borg_plays_risky=TRUE;
            else borg_plays_risky = FALSE;
            continue;
        }

        if (prefix(buf, "borg_scums_uniques ="))
        {
            if (buf[strlen("borg_scums_uniques =")+1] == 'T' ||
                buf[strlen("borg_scums_uniques =")+1] == '1' ||
                buf[strlen("borg_scums_uniques =")+1] == 't') borg_scums_uniques=TRUE;
            else borg_scums_uniques = FALSE;
            continue;
        }
        if (prefix(buf, "borg_uses_swaps ="))
        {
            if (buf[strlen("borg_uses_swaps =")+1] == 'T' ||
                buf[strlen("borg_uses_swaps =")+1] == '1' ||
                buf[strlen("borg_uses_swaps =")+1] == 't') borg_uses_swaps=TRUE;
            else borg_uses_swaps = FALSE;
            continue;
        }

        if (prefix(buf, "borg_slow_optimizehome ="))
        {
            if (buf[strlen("borg_slow_optimizehome =")+1] == 'T' ||
                buf[strlen("borg_slow_optimizehome =")+1] == '1' ||
                buf[strlen("borg_slow_optimizehome =")+1] == 't') borg_slow_optimizehome=TRUE;
            else borg_slow_optimizehome = FALSE;
            continue;
        }

        if (prefix(buf, "borg_stop_king ="))
        {
            if (buf[strlen("borg_stop_king =")+1] == 'T' ||
                buf[strlen("borg_stop_king =")+1] == '1' ||
                buf[strlen("borg_stop_king =")+1] == 't') borg_stop_king=TRUE;
            else borg_stop_king = FALSE;
            continue;
        }


        if (prefix(buf, "borg_uses_dynamic_calcs ="))
        {
            if (buf[strlen("borg_uses_dynamic_calcs =")+1] == 'T' ||
                buf[strlen("borg_uses_dynamic_calcs =")+1] == '1' ||
                buf[strlen("borg_uses_dynamic_calcs =")+1] == 't') borg_uses_calcs=TRUE;
            else borg_uses_calcs = FALSE;
            continue;
        }

        if (prefix(buf, "borg_respawn_winners ="))
        {
            if (buf[strlen("borg_respawn_winners =")+1] == 'T' ||
                buf[strlen("borg_respawn_winners =")+1] == '1' ||
                buf[strlen("borg_respawn_winners =")+1] == 't') borg_respawn_winners = TRUE;
            else borg_respawn_winners = FALSE;
            continue;
        }


        /* Extract the integers */
        if (prefix(buf, "borg_respawn_race ="))
        {
            sscanf(buf+strlen("borg_respawn_race =")+1, "%d", &borg_respawn_race);
            continue;
        }
        if (prefix(buf, "borg_respawn_class ="))
        {
            sscanf(buf+strlen("borg_respawn_class =")+1, "%d", &borg_respawn_class);
            continue;
        }
        if (prefix(buf, "borg_respawn_realm_1 ="))
        {
            sscanf(buf+strlen("borg_respawn_realm_1 =")+1, "%d", &borg_respawn_realm_1);
            continue;
        }
        if (prefix(buf, "borg_respawn_realm_2 ="))
        {
            sscanf(buf+strlen("borg_respawn_realm_2 =")+1, "%d", &borg_respawn_realm_2);
            continue;
        }
        if (prefix(buf, "borg_dump_level ="))
        {
            sscanf(buf+strlen("borg_dump_level =")+1, "%d", &borg_dump_level);
            continue;
        }

        if (prefix(buf, "borg_stop_clevel ="))
        {
            sscanf(buf+strlen("borg_stop_clevel =")+1, "%d", &borg_stop_clevel);
            continue;
        }
        if (prefix(buf, "borg_stop_dlevel ="))
        {
            sscanf(buf+strlen("borg_stop_dlevel =")+1, "%d", &borg_stop_dlevel);
            continue;
        }
        if (prefix(buf, "borg_chest_fail_tolerance ="))
        {
            sscanf(buf+strlen("borg_chest_fail_tolerance =")+1, "%d", &borg_chest_fail_tolerance);
            continue;
        }

        if (prefix(buf, "borg_money_scum_amount ="))
        {
            sscanf(buf+strlen("borg_money_scum_amount =")+1, "%d", &borg_money_scum_amount);
            continue;
        }
        if (prefix(buf, "borg_scums_money ="))
        {
            if (buf[strlen("borg_scums_money =")+1] == 'T' ||
                buf[strlen("borg_scums_money =")+1] == '1' ||
                buf[strlen("borg_scums_money =")+1] == 't') borg_scums_money = TRUE;
            else borg_scums_money = FALSE;
            continue;
        }


        if (prefix(buf, "borg_delay_factor ="))
        {
            sscanf(buf+strlen("borg_delay_factor =")+1, "%d", &borg_delay_factor);
            if (borg_delay_factor <= 1) borg_delay_factor = 1;
            if (borg_delay_factor >= 9) borg_delay_factor = 9;
            continue;
        }

        if (prefix(buf, "borg_verbose ="))
        {
            if (buf[strlen("borg_verbose =")+1] == 'T' ||
                buf[strlen("borg_verbose =")+1] == '1' ||
                buf[strlen("borg_verbose =")+1] == 't') borg_verbose = TRUE;
            else borg_verbose = FALSE;
            continue;
        }

        if (prefix(buf, "borg_munchkin_start ="))
        {
            if (buf[strlen("borg_munchkin_start =")+1] == 'T' ||
                buf[strlen("borg_munchkin_start =")+1] == '1' ||
                buf[strlen("borg_munchkin_start =")+1] == 't') borg_munchkin_start = TRUE;
            else borg_munchkin_start = FALSE;
            continue;
        }

        if (prefix(buf, "borg_munchkin_level ="))
        {
            sscanf(buf+strlen("borg_munchkin_level =")+1, "%d", &borg_munchkin_level);
            if (borg_munchkin_level <= 1) borg_munchkin_level = 1;
            if (borg_munchkin_level >= 50) borg_munchkin_level = 50;
            continue;
        }

        if (prefix(buf, "borg_munchkin_depth ="))
        {
            sscanf(buf+strlen("borg_munchkin_depth =")+1, "%d", &borg_munchkin_depth);
            if (borg_munchkin_depth <= 1) borg_munchkin_depth = 8;
            if (borg_munchkin_depth >= 100) borg_munchkin_depth = 100;
            continue;
        }

        if (prefix(buf, "borg_enchant_limit ="))
        {
            sscanf(buf+strlen("borg_enchant_limit =")+1, "%d", &borg_enchant_limit);
            if (borg_enchant_limit <= 8) borg_enchant_limit = 8;
            if (borg_enchant_limit >= 15) borg_enchant_limit = 15;
            continue;
        }

		if (prefix(buf, "borg_lunal_mode ="))
        {
            if (buf[strlen("borg_lunal_mode =")+1] == 'T' ||
                buf[strlen("borg_lunal_mode =")+1] == '1' ||
                buf[strlen("borg_lunal_mode =")+1] == 't') borg_lunal_mode = TRUE;
            else borg_lunal_mode = FALSE;
            continue;
        }

        if (prefix(buf, "borg_self_lunal ="))
        {
            if (buf[strlen("borg_self_lunal =")+1] == 'T' ||
                buf[strlen("borg_self_lunal =")+1] == '1' ||
                buf[strlen("borg_self_lunal =")+1] == 't') borg_self_lunal = TRUE;
            else borg_self_lunal = FALSE;
            continue;
        }

		if (prefix(buf, "REQ"))
        {
            if (!borg_load_requirement(buf+strlen("REQ")))
                borg_note(buf);
            continue;
        }
        if (prefix(buf, "FORMULA"))
        {
            if (!borg_load_formula(buf+strlen("FORMULA")))
                borg_note(buf);
            continue;
        }
        if (prefix(buf, "CND"))
        {
            if (!borg_load_formula(buf+strlen("CND")))
            borg_note(buf);
            continue;
        }
        if (prefix(buf, "POWER"))
        {
            if (!borg_load_power(buf+strlen("POWER")))
                borg_note(buf);

            continue;
        }
    }

    /* Close it */
    my_fclose(fp);

    for (i = 0; i < MAX_CLASS; i++)
        qsort(borg_required_item[i], n_req[i], sizeof(req_item), borg_item_cmp);

    /* Success */
    return;
}


#ifndef BABLOS

static int adjust_stat_borg(int value, int amount, int borg_roll)
{
    /* Negative amounts or maximize mode */
    if ((amount < 0) || maximize_mode)
    {
        return (modify_stat_value(value, amount));
    }

    /* Special hack */
    else
    {
        int i;

        /* Apply reward */
        for (i = 0; i < amount; i++)
        {
            if (value < 18)
            {
                value++;
            }
            else if (value < 18+70)
            {
                value += ((borg_roll ? 15 : randint(15)) + 5);
            }
            else if (value < 18+90)
            {
                value += ((borg_roll ? 6 : randint(6)) + 2);
            }
            else if (value < 18+100)
            {
                value++;
            }
        }
    }

    /* Return the result */
    return (value);
}

static void get_stats_borg_aux(void)
{
    int i, j;

    int bonus;

    int dice[18];

    /* Roll and verify some stats */
    while (TRUE)
    {
        /* Roll some dice */
        for (j = i = 0; i < 18; i++)
        {
            /* Roll the dice */
            dice[i] = randint(3 + i % 3);

            /* Collect the maximum */
            j += dice[i];
        }

        /* Verify totals */
        if ((j > 42) && (j < 54)) break;
    }

    /* Roll the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Extract 5 + 1d3 + 1d4 + 1d5 */
        j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

        /* Save that value */
        p_ptr->stat_max[i] = j;

        /* Obtain a "bonus" for "race" and "class" */
        bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

        /* Variable stat maxes */
        if (maximize_mode)
        {
            /* Start fully healed */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i];

            /* Efficiency -- Apply the racial/class bonuses */
            stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
        }

        /* Fixed stat maxes */
        else
        {
            /* Apply the bonus to the stat (somewhat randomly) */
            stat_use[i] = adjust_stat_borg(p_ptr->stat_max[i], bonus, FALSE);

            /* Save the resulting stat maximum */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
        }
    }
}
/*
 * Roll for a new characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats_borg(void)
{
    int i;

    int stat_limit[6];

    s32b borg_round = 0L;

    /* load up min. stats */
    stat_limit[A_STR] = 15; /* Str */
    stat_limit[A_INT] = 0; /* Int */
    stat_limit[A_WIS] = 0; /* Wis */
    stat_limit[A_DEX] = 11; /* Dex */
    stat_limit[A_CON] = 15;/* Con */
    stat_limit[A_CHR] = 0; /* Chr */

	/* Minimal stat based on Race (for activation) */
	switch (borg_race)
	{
		case RACE_HALF_GIANT:
		case RACE_CYCLOPS:
            stat_limit[A_STR] = 14;
			break;
		case RACE_HOBBIT:
		case RACE_HALF_OGRE:
		case RACE_HALF_TITAN:
		case RACE_DARK_ELF:
		case RACE_MIND_FLAYER:
		case RACE_SPECTRE:
		case RACE_SPRITE:
            stat_limit[A_INT] = 14;
			break;
		case RACE_DWARF:
		case RACE_GNOME:
		case RACE_HALF_ORC:
		case RACE_HALF_TROLL:
		case RACE_AMBERITE:
		case RACE_BARBARIAN:
		case RACE_YEEK:
		case RACE_NIBELUNG:
		case RACE_IMP:
		case RACE_SKELETON:
		case RACE_ZOMBIE:
            stat_limit[A_WIS] = 14;
			break;
		case RACE_KLACKON:
		case RACE_KOBOLD:
            stat_limit[A_DEX] = 14;
			break;
		case RACE_DRACONIAN:
		case RACE_GOLEM:
		case RACE_VAMPIRE:
            stat_limit[A_CON] = 14;
			break;
	}

	/* Minimal Stats based on class */
	switch (p_ptr->pclass)
    {
        case CLASS_WARRIOR:
        case CLASS_RANGER:
        case CLASS_ROGUE:
        case CLASS_PALADIN:
		case CLASS_WARRIOR_MAGE:
		case CLASS_CHAOS_WARRIOR:
            stat_limit[A_STR] = 16;
            stat_limit[A_DEX] = 15;
            break;
        case CLASS_MAGE:
		case CLASS_HIGH_MAGE:
            stat_limit[A_INT] = 16;
            stat_limit[A_DEX] = 15;
            break;
        case CLASS_PRIEST:
		case CLASS_ORPHIC:
            stat_limit[A_WIS] = 16;
            stat_limit[A_STR] = 15;
            break;
		case CLASS_MONK:
			stat_limit[A_DEX] = 16;
            stat_limit[A_CON] = 15;
            break;
		default:
            stat_limit[A_STR] = 16;
            stat_limit[A_DEX] = 15;
            break;
    }


    /* Minimal stats selected */
    if (stat_limit[0] + stat_limit[1] + stat_limit[2] + stat_limit[3] +
        stat_limit[4] + stat_limit[5] >= 1)
    {
        /* Auto-roll */
        while (1)
        {
            bool accept = TRUE;

            /* Get a new character */
            get_stats_borg_aux();

            /* Advance the round */
            borg_round++;

            /* Hack -- Prevent overflow */
            if (borg_round >= 750000L)
            {
                borg_note("# Minimal Stats too high.");
                break;
            }

            /* Check and count acceptable stats */
            for (i = 0; i < A_MAX; i++)
            {
                /* This stat is okay (JesperN)*/
                if (p_ptr->stat_max[i] >= stat_limit[i])

                {
                    accept = TRUE;
                }

                /* This stat is not okay */
                else
                {
                    accept = FALSE;
                    break;
                }
             }

             /* Break if "happy" */
             if (accept) break;
        } /* while */

    /* Note the number of rolls to achieve stats */
    borg_note(format("# Minimal stats rolled in %d turns.",borg_round));

    } /* minimal stats */
    else /* Otherwise just get a character */
    {
        borg_note("# Rolling random stats.");
        get_stats_borg_aux();
    }


}

/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra_borg(void)
{
	int		i, j, min_value, max_value;

#ifdef SHOW_LIFE_RATE
	int percent;
#endif

	/* Level one */
	p_ptr->max_plv = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

	/* Initialize arena and rewards information -KMW- */
	p_ptr->arena_number = 0;
	p_ptr->inside_arena = 0;
	p_ptr->inside_quest = 0;
	p_ptr->leftbldg = FALSE;
	p_ptr->exit_bldg = TRUE; /* only used for arena now -KMW- */

	/* Reset rewards */
	for (i = 0; i < MAX_BACT; i++)
	{
		p_ptr->rewards[i] = 0;
	}

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

	/* Initial hitpoints */
	p_ptr->mhp = p_ptr->hitdie;

	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Pre-calculate level 1 hitdice */
	player_hp[0] = p_ptr->hitdie;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			/* Add in racial hit dice */
			j = randint(rp_ptr->r_mhp);
			player_hp[i] = player_hp[i - 1] + j;

			/* If class hit dice is non zero - add it on */
			if (cp_ptr->c_mhp)
			{
				player_hp[i] += randint(cp_ptr->c_mhp);
			}
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (player_hp[PY_MAX_LEVEL - 1] < min_value) continue;
		if (player_hp[PY_MAX_LEVEL - 1] > max_value) continue;

		/* Acceptable */
		break;
	}

#ifdef SHOW_LIFE_RATE

	percent = (int)(((long)player_hp[PY_MAX_LEVEL - 1] * 200L) /
		(2 * p_ptr->hitdie +
		((PY_MAX_LEVEL - 1) * (p_ptr->hitdie + 1))));

	msg_format("Current Life Rating is %d/100.", percent);
	msg_print(NULL);

#endif /* SHOW_LIFE_RATE */

}


/*
 * Get the racial history, and social class, using the "history charts".
 */
/*
 * Forward declare
 */
typedef struct hist_type hist_type;

/*
 * Player background information
 */
struct hist_type
{
    cptr info;              /* Textual History */

    byte roll;              /* Frequency of this entry */
    byte chart;             /* Chart index */
    byte next;              /* Next chart index */
    byte bonus;             /* Social Class Bonus + 50 */
};


/*
 * Background information (see below)
 *
 * Chart progression by race:
 *   Human/Dunadan -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Half-Elf      -->  4 -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Elf/High-Elf  -->  7 -->  8 -->  9 --> 54 --> 55 --> 56
 *   Hobbit        --> 10 --> 11 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Gnome         --> 13 --> 14 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Dwarf         --> 16 --> 17 --> 18 --> 57 --> 58 --> 59 --> 60 --> 61
 *   Half-Orc      --> 19 --> 20 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Half-Troll    --> 22 --> 23 --> 62 --> 63 --> 64 --> 65 --> 66
 *
 * XXX XXX XXX This table *must* be correct or drastic errors may occur!
 */
static hist_type bg[] =
{
	{"You are the illegitimate and unacknowledged child ",   10, 1, 2, 25},
	{"You are the illegitimate but acknowledged child ",     20, 1, 2, 35},
	{"You are one of several children ",                     95, 1, 2, 45},
	{"You are the first child ",                            100, 1, 2, 50},

	{"of a Serf.  ",                                         40, 2, 3, 65},
	{"of a Yeoman.  ",                                       65, 2, 3, 80},
	{"of a Townsman.  ",                                     80, 2, 3, 90},
	{"of a Guildsman.  ",                                    90, 2, 3, 105},
	{"of a Landed Knight.  ",                                96, 2, 3, 120},
	{"of a Noble Family in the Courts of Chaos.  ",          99, 2, 3, 130},
	{"of the Royal Blood Line of Amber.  ",                 100, 2, 3, 140},

	{"You are the black sheep of the family.  ",             20, 3, 50, 20},
	{"You are a credit to the family.  ",                    80, 3, 50, 55},
	{"You are a well liked child.  ",                       100, 3, 50, 60},

	{"Your mother was of the Teleri.  ",			 40, 4, 1, 50},
	{"Your father was of the Teleri.  ",			 75, 4, 1, 55},
	{"Your mother was of the Noldor.  ",		 	 90, 4, 1, 55},
	{"Your father was of the Noldor.  ",		 	 95, 4, 1, 60},
	{"Your mother was of the Vanyar.  ",			 98, 4, 1, 65},
	{"Your father was of the Vanyar.  ",			100, 4, 1, 70},

	{"You are one of several children ",			 60, 7, 8, 50},
	{"You are the only child ",					100, 7, 8, 55},

	{"of a Teleri ",						 75, 8, 9, 50},
	{"of a Noldor ",						 95, 8, 9, 55},
	{"of a Vanyar ",						100, 8, 9, 60},

	{"Ranger.  ",						 40, 9, 54, 80},
	{"Archer.  ",						 70, 9, 54, 90},
	{"Warrior.  ",						 87, 9, 54, 110},
	{"Mage.  ",							 95, 9, 54, 125},
	{"Prince.  ",						 99, 9, 54, 140},
	{"King.  ",							100, 9, 54, 145},

	{"You are one of several children of a Hobbit ",		 85, 10, 11, 45},
	{"You are the only child of a Hobbit ",		        100, 10, 11, 55},

	{"Bum.  ",							 20, 11, 3, 55},
	{"Tavern Owner.  ",						 30, 11, 3, 80},
	{"Miller.  ",						 40, 11, 3, 90},
	{"Home Owner.  ",						 50, 11, 3, 100},
	{"Burglar.  ",						 80, 11, 3, 110},
	{"Warrior.  ",						 95, 11, 3, 115},
	{"Mage.  ",							 99, 11, 3, 125},
	{"Clan Elder.  ",						100, 11, 3, 140},

	{"You are one of several children of a Gnome ",		 85, 13, 14, 45},
	{"You are the only child of a Gnome ",			100, 13, 14, 55},

	{"Beggar.  ",						 20, 14, 3, 55},
	{"Braggart.  ",						 50, 14, 3, 70},
	{"Prankster.  ",						 75, 14, 3, 85},
	{"Warrior.  ",						 95, 14, 3, 100},
	{"Mage.  ",							100, 14, 3, 125},

	{"You are one of two children of a Dwarven ",		 25, 16, 17, 40},
	{"You are the only child of a Dwarven ",			100, 16, 17, 50},

	{"Thief.  ",						 10, 17, 18, 60},
	{"Prison Guard.  ",						 25, 17, 18, 75},
	{"Miner.  ",						 75, 17, 18, 90},
	{"Warrior.  ",						 90, 17, 18, 110},
	{"Priest.  ",						 99, 17, 18, 130},
	{"King.  ",							100, 17, 18, 150},

	{"You are the black sheep of the family.  ",		 15, 18, 57, 10},
	{"You are a credit to the family.  ",			 85, 18, 57, 50},
	{"You are a well liked child.  ",				100, 18, 57, 55},

	{"Your mother was an Orc, but it is unacknowledged.  ",	 25, 19, 20, 25},
	{"Your father was an Orc, but it is unacknowledged.  ",	100, 19, 20, 25},

	{"You are the adopted child ",				100, 20, 2, 50},

	{"Your mother was a Cave-Troll ",				 30, 22, 23, 20},
	{"Your father was a Cave-Troll ",				 60, 22, 23, 25},
	{"Your mother was a Hill-Troll ",				 75, 22, 23, 30},
	{"Your father was a Hill-Troll ",				 90, 22, 23, 35},
	{"Your mother was a Water-Troll ",				 95, 22, 23, 40},
	{"Your father was a Water-Troll ",				100, 22, 23, 45},

	{"Cook.  ",							  5, 23, 62, 60},
	{"Warrior.  ",						 95, 23, 62, 55},
	{"Shaman.  ",						 99, 23, 62, 65},
	{"Clan Chief.  ",						100, 23, 62, 80},

	{"You have dark brown eyes, ",				 20, 50, 51, 50},
	{"You have brown eyes, ",					 60, 50, 51, 50},
	{"You have hazel eyes, ",					 70, 50, 51, 50},
	{"You have green eyes, ",					 80, 50, 51, 50},
	{"You have blue eyes, ",					 90, 50, 51, 50},
	{"You have blue-gray eyes, ",				100, 50, 51, 50},

	{"straight ",						 70, 51, 52, 50},
	{"wavy ",							 90, 51, 52, 50},
	{"curly ",							100, 51, 52, 50},

	{"black hair, ",						 30, 52, 53, 50},
	{"brown hair, ",						 70, 52, 53, 50},
	{"auburn hair, ",						 80, 52, 53, 50},
	{"red hair, ",						 90, 52, 53, 50},
	{"blond hair, ",						100, 52, 53, 50},

	{"and a very dark complexion.",				 10, 53, 0, 50},
	{"and a dark complexion.",					 30, 53, 0, 50},
	{"and an average complexion.",				 80, 53, 0, 50},
	{"and a fair complexion.",					 90, 53, 0, 50},
	{"and a very fair complexion.",				100, 53, 0, 50},

	{"You have light grey eyes, ",				 85, 54, 55, 50},
	{"You have light blue eyes, ",				 95, 54, 55, 50},
	{"You have light green eyes, ",				100, 54, 55, 50},

	{"straight ",						 75, 55, 56, 50},
	{"wavy ",							100, 55, 56, 50},

	{"black hair, and a fair complexion.",			 75, 56, 0, 50},
	{"brown hair, and a fair complexion.",			 85, 56, 0, 50},
	{"blond hair, and a fair complexion.",			 95, 56, 0, 50},
	{"silver hair, and a fair complexion.",			100, 56, 0, 50},

	{"You have dark brown eyes, ",				 99, 57, 58, 50},
	{"You have glowing red eyes, ",				100, 57, 58, 60},

	{"straight ",						 90, 58, 59, 50},
	{"wavy ",							100, 58, 59, 50},

	{"black hair, ",						 75, 59, 60, 50},
	{"brown hair, ",						100, 59, 60, 50},

	{"a one foot beard, ",					 25, 60, 61, 50},
	{"a two foot beard, ",					 60, 60, 61, 51},
	{"a three foot beard, ",					 90, 60, 61, 53},
	{"a four foot beard, ",					100, 60, 61, 55},

	{"and a dark complexion.",					100, 61, 0, 50},

	{"You have slime green eyes, ",				 60, 62, 63, 50},
	{"You have puke yellow eyes, ",				 85, 62, 63, 50},
	{"You have blue-bloodshot eyes, ",				 99, 62, 63, 50},
	{"You have glowing red eyes, ",				100, 62, 63, 55},

	{"dirty ",							 33, 63, 64, 50},
	{"mangy ",							 66, 63, 64, 50},
	{"oily ",							100, 63, 64, 50},

	{"sea-weed green hair, ",					 33, 64, 65, 50},
	{"bright red hair, ",					 66, 64, 65, 50},
	{"dark purple hair, ",					100, 64, 65, 50},

	{"and green ",						 25, 65, 66, 50},
	{"and blue ",						 50, 65, 66, 50},
	{"and white ",						 75, 65, 66, 50},
	{"and black ",						100, 65, 66, 50},

	{"ulcerous skin.",						 33, 66, 0, 50},
	{"scabby skin.",						 66, 66, 0, 50},
	{"leprous skin.",                       100, 66, 0, 50},

	{"You are an unacknowledged child of ", 50, 67, 68, 45},
	{"You are a rebel child of ",         80, 67, 68, 65},
	{"You are a long lost child of ",     100, 67, 68, 55},

	{"an unknown Amberite.  ",               50, 68, 50, 80 },
	{"an unknown third generation Amberite.  ", 65, 68, 50, 90 },
	{"an unknown second generation Amberite.  ", 79, 68, 50, 100 },
	{"Oberon.  ",       80, 68, 50, 130 },
	{"Osric.  ",        83, 68, 50, 105 },
	{"Finndo.  ",       84, 68, 50, 105 },
	{"Brand.  ",        85, 68, 50, 90 },
	{"Flora.  ",        87, 68, 50, 100 },
	{"Gerard.  ",       88, 68, 50, 125 },
	{"Deirdre.  ",      89, 68, 50, 120 },
	{"Random.  ",       90, 68, 50, 140 },
	{"Benedict.  ",     91, 68, 50, 115 },
	{"Corwin.  ",       92, 68, 50, 110 },
	{"Julian.  ",       93, 68, 50, 105 },
	{"Caine.  ",        94, 68, 50, 95 },
	{"Bleys.  ",        95, 68, 50, 115 },
	{"Fiona.  ",        96, 68, 50, 110 },
	{"Eric.  ",         97, 68, 50, 135 },
	{"Rinaldo.  ",      98, 68, 50, 90 },
	{"Merlin.  ",       99, 68, 50, 105 },
	{"Martin.  ",       100, 68, 50, 80 },


	{"You are one of several children of a Dark Elven ",      85, 69, 70, 45},
	{"You are the only child of a Dark Elven ",          100, 69, 70, 55},

	{"Warrior.  ", 50, 70, 71, 60 },
	{"Warlock.  ", 80, 70, 71, 75 },
	{"Noble.  ", 100, 70, 71, 95 },

	{"You have black eyes, ", 100, 71, 72, 50},

	{"straight ",                        70, 72, 73, 50},
	{"wavy ",                            90, 72, 73, 50},
	{"curly ",                          100, 72, 73, 50},

	{"black hair and a very dark complexion.", 100, 73, 0, 50 },

	{"Your mother was an Ogre, but it is unacknowledged.  ", 25, 74, 20, 25},
	{"Your father was an Ogre, but it is unacknowledged.  ", 100, 74, 20, 25},

	{"Your mother was a Hill Giant.  ", 10, 75, 20, 50},
	{"Your mother was a Fire Giant.  ", 12, 75, 20, 55},
	{"Your mother was a Frost Giant.  ", 20, 75, 20, 60},
	{"Your mother was a Cloud Giant.  ", 23, 75, 20, 65},
	{"Your mother was a Storm Giant.  ", 25, 75, 20, 70},
	{"Your father was a Hill Giant.  ",  60, 75, 20, 50},
	{"Your father was a Fire Giant.  ",  70, 75, 20, 55},
	{"Your father was a Frost Giant.  ",  80, 75, 20, 60},
	{"Your father was a Cloud Giant.  ",  90, 75, 20, 65},
	{"Your father was a Storm Giant.  ", 100, 75, 20, 70},

	{"Your father was an unknown Titan.  ", 75, 76, 20, 50 },
	{"Your mother was Themis.  ",        80, 76, 20, 100 },
	{"Your mother was Mnemosyne.  ",     85, 76, 20, 100 },
	{"Your father was Okeanoas.  ",      90, 76, 20, 100 },
	{"Your father was Crius.  ",         95, 76, 20, 100 },
	{"Your father was Hyperion.  ",      98, 76, 20, 125 },
	{"Your father was Kronos.  ",       100, 76, 20, 150 },

	{"You are the offspring of an unknown Cyclops.  ", 90, 77, 109, 50 },
	{"You are Polyphemos's child.  ", 98, 77, 109, 80 },
	{"You are Uranos's child.  ", 100, 77, 109, 135 },

	{"You are one of several children of ", 100, 78, 79, 50 },

	{"a Brown Yeek. ", 50, 79, 80, 50 },
	{"a Blue Yeek.  ", 75, 79, 80, 50 },
	{"a Master Yeek.  ", 95, 79, 80, 85 },
	{"Boldor, the King of the Yeeks.  ", 100, 79, 80, 120 },

	{"You have pale eyes, ",    25, 80, 81, 50 },
	{"You have glowing eyes, ",    50, 80, 81, 50 },
	{"You have tiny black eyes, ",    75, 80, 81, 50 },
	{"You have shining black eyes, ",    100, 80, 81, 50 },

	{"no hair at all, ",        20, 81, 65, 50 },
	{"short black hair, ",        40, 81, 65, 50 },
	{"long black hair, ",        60, 81, 65, 50 },
	{"bright red hair, ",        80, 81, 65, 50 },
	{"colourless albino hair, ",        100, 81, 65, 50 },

	{"You are one of several children of ", 100, 82, 83, 50 },

	{"a Small Kobold.  ",   40, 83, 80, 50 },
	{"a Kobold.  ",         75, 83, 80, 55 },
	{"a Large Kobold.  ",   95, 83, 80, 65 },
	{"Mughash, the Kobold Lord.  ",     100, 83, 80, 100 },

	{"You are one of several children of a Klackon hive queen.  "
	, 100, 84, 85, 50 },

	{"You have red skin, ", 40, 85, 86, 50 },
	{"You have black skin, ", 90, 85, 86, 50 },
	{"You have yellow skin, ", 100, 85, 86, 50 },

	{"and black eyes.", 100, 86, 0, 50 },

	{"You are one of several children of ", 100, 87, 88, 89 },

	{"a Nibelung Slave.  ", 30, 88, 18, 20 },
	{"a Nibelung Thief.  ", 50, 88, 18, 40 },
	{"a Nibelung Smith.  ", 70, 88, 18, 60 },
	{"a Nibelung Miner.  ", 90, 88, 18, 75 },
	{"a Nibelung Shaman.  ", 95, 88, 18, 100 },
	{"Mime, the Nibelung.  ", 100, 88, 18, 100 },

	{"You are ", 100, 89, 135, 50 },

	{"the oldest child of a Draconian ", 30, 135, 90, 55 },
	{"the youngest child of a Draconian ", 50, 135, 90, 50 },
	{"the adopted child of a Draconian ", 55, 135, 90, 50 },
	{"an orphaned child of a Draconian ", 60, 135, 90, 45 },
	{"one of several children of a Draconian ", 85, 135, 90, 50 },
	{"the only child of a Draconian ", 100, 135, 90, 55 },

	{"Beggar.  ", 10, 90, 91, 20 },
	{"Thief.  ", 21, 90, 91, 30 },
	{"Sailor.  ", 26, 90, 91, 45 },
	{"Mercenary.  ", 42, 90, 91, 45 },
	{"Warrior.  ", 73, 90, 91, 50 },
	{"Merchant.  ", 78, 90, 91, 50 },
	{"Artisan.  ", 85, 90, 91, 55 },
	{"Healer.  ", 89, 90, 91, 60 },
	{"Priest.  ", 94, 90, 91, 65 },
	{"Mage.  ", 97, 90, 91, 70 },
	{"Scholar.  ", 99, 90, 91, 80 },
	{"Noble.  ", 100, 90, 91, 100 },

	{"You have ", 100, 91, 136, 50 },

	{"charcoal wings, charcoal skin and a smoke-gray belly.", 11, 136, 0, 50 },
	{"bronze wings, bronze skin, and a copper belly.", 16, 136, 0, 50 },
	{"golden wings, and golden skin.", 24, 136, 0, 50 },
	{"white wings, and white skin.", 26, 136, 0, 60 },
	{"blue wings, blue skin, and a cyan belly.", 32, 136, 0, 50 },
	{"multi-hued wings, and multi-hued skin.", 33, 136, 0, 70 },
	{"brown wings, and brown skin.", 37, 136, 0, 45 },
	{"black wings, black skin, and a white belly.", 41, 136, 0, 50 },
	{"lavender wings, lavender skin, and a white belly.", 48, 136, 0, 50 },
	{"green wings, green skin and yellow belly.", 65, 136, 0, 50 },
	{"green wings, and green skin.", 75, 136, 0, 50 },
	{"red wings, and red skin.", 88, 136, 0, 50 },
	{"black wings, and black skin.", 94, 136, 0, 50 },
	{"metallic skin, and shining wings.", 100, 136, 0, 55},

	{"You have slimy skin, empty glowing eyes, and ", 100, 92, 93, 80 },
	{"three tentacles around your mouth.", 20, 93, 0, 45 },
	{"four tentacles around your mouth.", 80, 93, 0, 50 },
	{"five tentacles around your mouth.", 100, 93, 0, 55 },

	{"You ancestor was ", 100, 94, 95, 50 },

	{"a mindless demonic spawn.  ", 30, 95, 96, 20 },
	{"a minor demon.  ", 60, 95, 96, 50 },
	{"a major demon.  ", 90, 95, 96, 75 },
	{"a demon lord.  ", 100, 95, 96, 99 },

	{"You have red skin, ", 50, 96, 97, 50 },
	{"You have brown skin, ", 100, 96, 97, 50},

	{"claws, fangs, spikes, and glowing red eyes.", 40, 97, 0, 50 },
	{"claws, fangs, and glowing red eyes.", 70, 97, 0, 50 },
	{"claws, and glowing red eyes.", 100, 97, 0, 50 },

	{"You were shaped from ", 100, 98, 99, 50 },

	{"clay ", 40, 99, 100, 50 },
	{"stone ", 80, 99, 100, 50 },
	{"wood ", 85, 99, 100, 40 },
	{"iron ", 99, 99, 100, 50 },
	{"pure gold ", 100, 99, 100, 100},

	{"by a Kabbalist", 40, 100, 101, 50 },
	{"by a Wizard", 65, 100, 101, 50 },
	{"by an Alchemist", 90, 100, 101, 50},
	{"by a Priest", 100, 100, 101, 60},

	{" to fight evil.", 10, 101, 0, 65 },
	{".", 100, 101, 0, 50 },

	{"You were created by ", 100, 102, 103, 50 },

	{"a Necromancer.  ", 30, 103, 104, 50 },
	{"a magical experiment.  ", 50, 103, 104, 50 },
	{"an Evil Priest.  ", 70, 103, 104, 50 },
	{"a pact with the demons.  ", 75, 103, 104, 50 },
	{"a restless spirit.  ", 85, 103, 104, 50 },
	{"a curse.  ", 95, 103, 104, 30 },
	{"an oath.  ", 100, 103, 104, 50 },

	{"You have ", 100, 104, 105, 50 },
	{"dirty, dry bones, ", 40, 105, 106, 50 },
	{"rotten black bones, ", 60, 105, 106, 50 },
	{"filthy, brown bones, ", 80, 105, 106, 50 },
	{"shining white bones, ", 100, 105, 106, 50 },

	{"and glowing eyes.", 30, 106, 0, 50 },
	{"and eyes which burn with hellfire.", 50, 106, 0, 50 },
	{"and empty eyesockets.", 100, 106, 0, 50 },

	{"You were created by ", 100, 107, 108, 50 },

	{"a Necromancer.  ", 30, 108, 62, 50 },
	{"a Wizard.  ", 50, 108, 62, 50 },
	{"a restless spirit.  ", 60, 108, 62, 50 },
	{"an Evil Priest.  ", 70, 108, 62, 50 },
	{"a pact with the demons.  ", 80, 108, 62, 50 },
	{"a curse.  ", 95, 108, 62, 30 },
	{"an oath.  ", 100, 108, 62, 50 },

	{"You have a dark brown eye, ",               20, 109, 110, 50},
	{"You have a brown eye, ",                    60, 109, 110, 50},
	{"You have a hazel eye, ",                    70, 109, 110, 50},
	{"You have a green eye, ",                    80, 109, 110, 50},
	{"You have a blue eye, ",                     90, 109, 110, 50},
	{"You have a blue-gray eye, ",               100, 109, 110, 50},

	{"straight ",                        70, 110, 111, 50},
	{"wavy ",                            90, 110, 111, 50},
	{"curly ",                          100, 110, 111, 50},

	{"black hair, ",                         30, 111, 112, 50},
	{"brown hair, ",                         70, 111, 112, 50},
	{"auburn hair, ",                        80, 111, 112, 50},
	{"red hair, ",                       90, 111, 112, 50},
	{"blond hair, ",                        100, 111, 112, 50},

	{"and a very dark complexion.",              10, 112, 0, 50},
	{"and a dark complexion.",                   30, 112, 0, 50},
	{"and an average complexion.",               80, 112, 0, 50},
	{"and a fair complexion.",                   90, 112, 0, 50},
	{"and a very fair complexion.",             100, 112, 0, 50},

	{"You arose from an unmarked grave.  ", 20, 113, 114, 50 },
	{"In life you were a simple peasant, the victim of a powerful Vampire Lord.  ", 40, 113, 114, 50 },
	{"In life you were a Vampire Hunter, but they got you.  ", 60, 113, 114, 50 },
	{"In life you were a Necromancer.  ", 80, 113, 114, 50 },
	{"In life you were a powerful noble.  ", 95, 113, 114, 50 },
	{"In life you were a powerful and cruel tyrant.  ", 100, 113, 114, 50 },

	{"You have ", 100, 114, 115, 50 },

	{"jet-black hair, ", 25, 115, 116, 50 },
	{"matted brown hair, ", 50, 115, 116, 50 },
	{"white hair, ", 75, 115, 116, 50 },
	{"a hairless head, ", 100, 115, 116, 50 },

	{"eyes like red coals, ", 25, 116, 117, 50 },
	{"blank white eyes, ", 50, 116, 117, 50 },
	{"feral yellow eyes, ", 75, 116, 117, 50 },
	{"bloodshot red eyes, ", 100, 116, 117, 50 },

	{"and a deathly pale complexion.", 100, 117, 0, 50 },

	{"You were created by ", 100, 118, 119, 50 },

	{"a Necromancer.  ", 30, 119, 134, 50 },
	{"a magical experiment.  ", 50, 119, 134, 50 },
	{"an Evil Priest.  ", 70, 119, 134, 50 },
	{"a pact with the demons.  ", 75, 119, 134, 50 },
	{"a restless spirit.  ", 85, 119, 134, 50 },
	{"a curse.  ", 95, 119, 134, 30 },
	{"an oath.  ", 100, 119, 134, 50 },

	{"jet-black hair, ", 25, 120, 121, 50 },
	{"matted brown hair, ", 50, 120, 121, 50 },
	{"white hair, ", 75, 120, 121, 50 },
	{"a hairless head, ", 100, 120, 121, 50 },

	{"eyes like red coals, ", 25, 121, 122, 50 },
	{"blank white eyes, ", 50, 121, 122, 50 },
	{"feral yellow eyes, ", 75, 121, 122, 50 },
	{"bloodshot red eyes, ", 100, 121, 122, 50 },

	{" and a deathly gray complexion. ", 100, 122, 123, 50 },
	{"An eerie green aura surrounds you.", 100, 123, 0, 50 },

	{"Your parents were ", 100, 124, 125, 50 },

	{"pixies.  ", 20, 125, 126, 35 },
	{"nixies.  ", 30, 125, 126, 25 },
	{"wood sprites.  ", 75, 125, 126, 50 },
	{"wood spirits.  ", 90, 125, 126, 75 },
	{"noble faerie folk.  ", 100, 125, 126, 85 },

	{"You have light blue wings attached to your back, ", 100, 126, 127, 50 },

	{"straight blond hair, ",                        80, 127, 128, 50},
	{"wavy blond hair, ",                            100, 127, 128, 50},

	{"blue eyes, and a very fair complexion.", 100, 128, 0, 50},

	{"You were produced by a magical experiment.  ", 30, 129, 130, 40},
	{"In your childhood, you were stupid enough to stick your head in raw Logrus.  ",
	50, 129, 130, 50 },
	{"A Demon Lord of Chaos decided to have some fun, and so he created you.  ",
	60, 129, 130, 60 },
	{"You are the magical crossbreed of an animal and a man.  ", 75, 129, 130, 50},
	{"You are the blasphemous crossbreed of unspeakable creatures of chaos.  ", 100, 129, 130, 30},


	{"You have green reptilian eyes, ",              60, 130, 131, 50},
	{"You have the black eyes of a bird, ",              85, 130, 131, 50},
	{"You have the orange eyes of a cat, ",               99, 130, 131, 50},
	{"You have the fiery eyes of a demon, ",             100, 130, 131, 55},

	{"no hair at all, ",                 10, 131, 133, 50 },
	{"dirty ",                           33, 131, 132, 50},
	{"mangy ",                           66, 131, 132, 50},
	{"oily ",                           100, 131, 132, 50},

	{"brown fur, ",                    33, 132, 133, 50},
	{"gray fur, ",                    66, 132, 133, 50},
	{"albino fur, ",                  100, 132, 133, 50},

	{"and the hooves of a goat.",      50, 133, 0, 50 },
	{"and human feet.",        75, 133, 0, 50 },
	{"and bird's feet.",       85, 133, 0, 50 },
	{"and reptilian feet.",    90, 133, 0, 50 },
	{"and bovine feet.",       95, 133, 0, 50 },
	{"and feline feet.",       97, 133, 0, 50 },
	{"and canine feet.",       100, 133, 0, 50 },

	{"You have ", 100, 134, 120, 50 },
};


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history_borg(void)
{
    int i, n, chart, roll, social_class;

    char *s, *t;

    char buf[240];



    /* Clear the previous history strings */
    for (i = 0; i < 4; i++) history[i][0] = '\0';


    /* Clear the history text */
    buf[0] = '\0';

    /* Initial social class */
    social_class = randint(4);

	/* Starting place */
	switch (p_ptr->prace)
	{
		case RACE_AMBERITE:
		{
			chart = 67;
			break;
		}
		case RACE_HUMAN:
		case RACE_BARBARIAN:
		{
			chart = 1;
			break;
		}
		case RACE_HALF_ELF:
		{
			chart = 4;
			break;
		}
		case RACE_ELF:
		case RACE_HIGH_ELF:
		{
			chart = 7;
			break;
		}
		case RACE_HOBBIT:
		{
			chart = 10;
			break;
		}
		case RACE_GNOME:
		{
			chart = 13;
			break;
		}
		case RACE_DWARF:
		{
			chart = 16;
			break;
		}
		case RACE_HALF_ORC:
		{
			chart = 19;
			break;
		}
		case RACE_HALF_TROLL:
		{
			chart = 22;
			break;
		}
		case RACE_DARK_ELF:
		{
			chart = 69;
			break;
		}
		case RACE_HALF_OGRE:
		{
			chart = 74;
			break;
		}
		case RACE_HALF_GIANT:
		{
			chart = 75;
			break;
		}
		case RACE_HALF_TITAN:
		{
			chart = 76;
			break;
		}
		case RACE_CYCLOPS:
		{
			chart = 77;
			break;
		}
		case RACE_YEEK:
		{
			chart = 78;
			break;
		}
		case RACE_KOBOLD:
		{
			chart = 82;
			break;
		}
		case RACE_KLACKON:
		{
			chart = 84;
			break;
		}
		case RACE_NIBELUNG:
		{
			chart = 87;
			break;
		}
		case RACE_DRACONIAN:
		{
			chart = 89;
			break;
		}
		case RACE_MIND_FLAYER:
		{
			chart = 92;
			break;
		}
		case RACE_IMP:
		{
			chart = 94;
			break;
		}
		case RACE_GOLEM:
		{
			chart = 98;
			break;
		}
		case RACE_SKELETON:
		{
			chart = 102;
			break;
		}
		case RACE_ZOMBIE:
		{
			chart = 107;
			break;
		}
		case RACE_VAMPIRE:
		{
			chart = 113;
			break;
		}
		case RACE_SPECTRE:
		{
			chart = 118;
			break;
		}
		case RACE_SPRITE:
		{
			chart = 124;
			break;
		}
		case RACE_BEASTMAN:
		{
			chart = 129;
			break;
		}
		default:
		{
			chart = 0;
			break;
		}
	}

    /* Process the history */
    while (chart)
    {
        /* Start over */
        i = 0;

        /* Roll for nobility */
        roll = randint(100);

        /* Get the proper entry in the table */
        while ((chart != bg[i].chart) || (roll > bg[i].roll)) i++;

        /* Get the textual history */
        strcat(buf, (bg[i].info));

        /* Add in the social class */
        social_class += (int)(bg[i].bonus) - 50;

        /* Enter the next chart */
        chart = bg[i].next;
    }



    /* Verify social class */
    if (social_class > 100) social_class = 100;
    else if (social_class < 1) social_class = 1;

    /* Save the social class */
    p_ptr->sc = social_class;


    /* Skip leading spaces */
    for (s = buf; *s == ' '; s++) /* loop */;

    /* Get apparent length */
    n = strlen(s);

    /* Kill trailing spaces */
    while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';


    /* Start at first line */
    i = 0;

    /* Collect the history */
    while (TRUE)
    {
        /* Extract remaining length */
        n = strlen(s);

        /* All done */
        if (n < 60)
        {
            /* Save one line of history */
            strcpy(history[i++], s);

            /* All done */
            break;
        }

        /* Find a reasonable break-point */
        for (n = 60; ((n > 0) && (s[n-1] != ' ')); n--) /* loop */;

        /* Save next location */
        t = s + n;

        /* Wipe trailing spaces */
        while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';

        /* Save one line of history */
        strcpy(history[i++], s);

        /* Start next line */
        for (s = t; *s == ' '; s++) /* loop */;
    }
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw_borg(void)
{
	int h_percent;

	/* Calculate the age */
	p_ptr->age = rp_ptr->b_age + randint(rp_ptr->m_age);

	/* Calculate the height/weight for males */
	if (p_ptr->psex == SEX_MALE)
	{
		p_ptr->ht = randnor(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
		h_percent = (int)(p_ptr->ht) * 100 / (int)(rp_ptr->m_b_ht);
		p_ptr->wt = randnor((int)(rp_ptr->m_b_wt) * h_percent / 100,
			(int)(rp_ptr->m_m_wt) * h_percent / 300 );
	}
	/* Calculate the height/weight for females */
	else if (p_ptr->psex == SEX_FEMALE)
	{
		p_ptr->ht = randnor(rp_ptr->f_b_ht, rp_ptr->f_m_ht);

		h_percent = (int)(p_ptr->ht) * 100 / (int)(rp_ptr->f_b_ht);
		p_ptr->wt = randnor((int)(rp_ptr->f_b_wt) * h_percent / 100,
			(int)(rp_ptr->f_m_wt) * h_percent / 300 );
	}
}




/*
 * Get the player's starting money
 */
static void get_money_borg(void)
{
    int i;

    int gold = 0;

    /* Social Class determines starting gold */
    gold = (p_ptr->sc * 6) + randint(100) + 300;

    /* Process the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Mega-Hack -- reduce gold for high stats */
        if (stat_use[i] >= 18+50) gold -= 300;
        else if (stat_use[i] >= 18+20) gold -= 200;
        else if (stat_use[i] > 18) gold -= 150;
        else gold -= (stat_use[i] - 8) * 10;
    }

    /* Minimum 100 gold */
    if (gold < 100) gold = 100;

    /* Save the gold */
    p_ptr->au = gold;
}
/*
 * Name segments for random player names
 * Copied Cth by DvE
 * Copied from borgband by
 */

/* Dwarves */
static char *dwarf_syllable1[] =
{
    "B", "D", "F", "G", "Gl", "H", "K", "L", "M", "N", "R", "S", "T", "Th", "V",
};

static char *dwarf_syllable2[] =
{
    "a", "e", "i", "o", "oi", "u",
};

static char *dwarf_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar", "nus", "rin", "ran", "sin", "sil", "sur",
};

/* Elves */
static char *elf_syllable1[] =
{
    "Al", "An", "Bal", "Bel", "Cal", "Cel", "El", "Elr", "Elv", "Eow", "Ear", "F", "Fal", "Fel", "Fin", "G", "Gal", "Gel", "Gl", "Is", "Lan", "Leg", "Lom", "N", "Nal", "Nel",  "S", "Sal", "Sel", "T", "Tal", "Tel", "Thr", "Tin",
};

static char *elf_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
};

static char *elf_syllable3[] =
{
    "l", "las", "lad", "ldor", "ldur", "linde", "lith", "mir", "n", "nd", "ndel", "ndil", "ndir", "nduil", "ng", "mbor", "r", "rith", "ril", "riand", "rion", "s", "thien", "viel", "wen", "wyn",
};

/* Gnomes */
static char *gnome_syllable1[] =
{
    "Aar", "An", "Ar", "As", "C", "H", "Han", "Har", "Hel", "Iir", "J", "Jan", "Jar", "K", "L", "M", "Mar", "N", "Nik", "Os", "Ol", "P", "R", "S", "Sam", "San", "T", "Ter", "Tom", "Ul", "V", "W", "Y",
};

static char *gnome_syllable2[] =
{
    "a", "aa",  "ai", "e", "ei", "i", "o", "uo", "u", "uu",
};

static char *gnome_syllable3[] =
{
    "ron", "re", "la", "ki", "kseli", "ksi", "ku", "ja", "ta", "na", "namari", "neli", "nika", "nikki", "nu", "nukka", "ka", "ko", "li", "kki", "rik", "po", "to", "pekka", "rjaana", "rjatta", "rjukka", "la", "lla", "lli", "mo", "nni",
};

/* Hobbit */
static char *hobbit_syllable1[] =
{
    "B", "Ber", "Br", "D", "Der", "Dr", "F", "Fr", "G", "H", "L", "Ler", "M", "Mer", "N", "P", "Pr", "Per", "R", "S", "T", "W",
};

static char *hobbit_syllable2[] =
{
    "a", "e", "i", "ia", "o", "oi", "u",
};

static char *hobbit_syllable3[] =
{
    "bo", "ck", "decan", "degar", "do", "doc", "go", "grin", "lba", "lbo", "lda", "ldo", "lla", "ll", "lo", "m", "mwise", "nac", "noc", "nwise", "p", "ppin", "pper", "tho", "to",
};

/* Human */
static char *human_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir", "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V", "Y", "Z", "W", "Wic",
};

static char *human_syllable2[] =
{
    "a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei", "ea", "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie", "ire", "ira", "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe", "ore", "u", "y",
};

static char *human_syllable3[] =
{
    "a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch", "can", "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g", "gord", "gan", "l", "li", "lgrin", "lin", "lith", "lath", "loth", "ld", "ldric", "ldan", "m", "mas", "mos", "mar", "mond", "n", "nydd", "nidd", "nnon", "nwan", "nyth", "nad", "nn", "nnor", "nd", "p", "r", "ron", "rd", "s", "sh", "seth", "sean", "t", "th", "tha", "tlan", "trem", "tram", "v", "vudd", "w", "wan", "win", "wyn", "wyr", "wyr", "wyth",
};

/* Orc */
static char *orc_syllable1[] =
{
    "B", "Er", "G", "Gr", "H", "P", "Pr", "R", "V", "Vr", "T", "Tr", "M", "Dr",
};

static char *orc_syllable2[] =
{
    "a", "i", "o", "oo", "u", "ui",
};

static char *orc_syllable3[] =
{
    "dash", "dish", "dush", "gar", "gor", "gdush", "lo", "gdish", "k", "lg", "nak", "rag", "rbag", "rg", "rk", "ng", "nk", "rt", "ol", "urk", "shnak", "mog", "mak", "rak",
};


/*
 * Random Name Generator
 * based on a Javascript by Michael Hensley
 * "http://geocities.com/timessquare/castle/6274/"
 * Copied from Cth by DvE
 * Copied from borgband by
 */
static void create_random_name(int race, char *name)
{
    /* Paranoia */
    if (!name) return;

    /* Select the monster type */
    switch (race)
    {
        /* Create the monster name */
	case RACE_AMBERITE:
    case RACE_DWARF:
        strcpy(name, dwarf_syllable1[rand_int(sizeof(dwarf_syllable1) / sizeof(char*))]);
        strcat(name, dwarf_syllable2[rand_int(sizeof(dwarf_syllable2) / sizeof(char*))]);
        strcat(name, dwarf_syllable3[rand_int(sizeof(dwarf_syllable3) / sizeof(char*))]);
        break;
    case RACE_HUMAN:
    case RACE_BARBARIAN:
        strcpy(name, human_syllable1[rand_int(sizeof(human_syllable1) / sizeof(char*))]);
        strcat(name, human_syllable2[rand_int(sizeof(human_syllable2) / sizeof(char*))]);
        strcat(name, human_syllable3[rand_int(sizeof(human_syllable3) / sizeof(char*))]);
        break;
    case RACE_ELF:
    case RACE_HALF_ELF:
    case RACE_HIGH_ELF:
    case RACE_DARK_ELF:
        strcpy(name, elf_syllable1[rand_int(sizeof(elf_syllable1) / sizeof(char*))]);
        strcat(name, elf_syllable2[rand_int(sizeof(elf_syllable2) / sizeof(char*))]);
        strcat(name, elf_syllable3[rand_int(sizeof(elf_syllable3) / sizeof(char*))]);
        break;
    case RACE_GNOME:
        strcpy(name, gnome_syllable1[rand_int(sizeof(gnome_syllable1) / sizeof(char*))]);
        strcat(name, gnome_syllable2[rand_int(sizeof(gnome_syllable2) / sizeof(char*))]);
        strcat(name, gnome_syllable3[rand_int(sizeof(gnome_syllable3) / sizeof(char*))]);
        break;
    case RACE_HOBBIT:
        strcpy(name, hobbit_syllable1[rand_int(sizeof(hobbit_syllable1) / sizeof(char*))]);
        strcat(name, hobbit_syllable2[rand_int(sizeof(hobbit_syllable2) / sizeof(char*))]);
        strcat(name, hobbit_syllable3[rand_int(sizeof(hobbit_syllable3) / sizeof(char*))]);
        break;
    case RACE_HALF_ORC:
    case RACE_HALF_TROLL:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_HALF_TITAN:
    	strcpy(name, orc_syllable1[rand_int(sizeof(orc_syllable1) / sizeof(char*))]);
        strcat(name, orc_syllable2[rand_int(sizeof(orc_syllable2) / sizeof(char*))]);
        strcat(name, orc_syllable3[rand_int(sizeof(orc_syllable3) / sizeof(char*))]);
        break;
    /* Create an empty name */
    default:
        strcpy(name, orc_syllable1[rand_int(sizeof(orc_syllable1) / sizeof(char*))]);
        strcat(name, dwarf_syllable2[rand_int(sizeof(dwarf_syllable2) / sizeof(char*))]);
        strcat(name, elf_syllable3[rand_int(sizeof(elf_syllable3) / sizeof(char*))]);

        break;
    }
}



static byte player_init_borg[MAX_CLASS][3][2] =
{
	{
		/* Warrior */
		{ TV_RING, SV_RING_RES_FEAR }, /* Warriors need it! */
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL }
	},

	{
		/* Mage */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: for realm1 book */
		{ TV_SWORD, SV_DAGGER },
		{ TV_DEATH_BOOK, 0 } /* Hack: for realm2 book */
	},

	{
		/* Priest */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: for Life / Death book */
		{ TV_HAFTED, SV_MACE },
		{ TV_DEATH_BOOK, 0 } /* Hack: for realm2 book */
	},

	{
		/* Rogue */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: for realm1 book */
		{ TV_SWORD, SV_DAGGER },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR }
	},

	{
		/* Ranger */
		{ TV_NATURE_BOOK, 0 },
		{ TV_SWORD, SV_DAGGER },
		{ TV_DEATH_BOOK, 0 }		/* Hack: for realm2 book */
	},

	{
		/* Paladin */
		{ TV_SORCERY_BOOK, 0 },
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL }
	},

	{
		/* Warrior-Mage */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: for realm1 book */
		{ TV_SWORD, SV_SHORT_SWORD },
		{ TV_DEATH_BOOK, 0 } /* Hack: for realm2 book */
	},

	{
		/* Chaos Warrior */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: For realm1 book */
		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_HARD_ARMOR, SV_METAL_SCALE_MAIL }
	},

	{
		/* Monk */
		{ TV_SORCERY_BOOK, 0 },
		{ TV_POTION, SV_POTION_HEALING },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
	},

	{
		/* Mindcrafter */
		{ TV_SWORD, SV_SMALL_SWORD },
		{ TV_POTION, SV_POTION_RESTORE_MANA },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
	},

	{
		/* High Mage */
		{ TV_SORCERY_BOOK, 0 }, /* Hack: for realm1 book */
		{ TV_SWORD, SV_DAGGER },
		{ TV_RING, SV_RING_SUSTAIN_INT}
	},
};

/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit_borg(void)
{
	int i, tv, sv;

	object_type	forge;
	object_type	*q_ptr;


#ifdef USE_SCRIPT
	if (player_outfit_callback()) return;
#endif /* USE_SCRIPT */

	/* Get local object */
	q_ptr = &forge;

 	/* Give the player some food */
	switch (p_ptr->prace)
	{
		case RACE_GOLEM:
		case RACE_SKELETON:
		case RACE_ZOMBIE:
		case RACE_VAMPIRE:
		case RACE_SPECTRE:
		{
			/* Scrolls of satisfy hunger */
			object_prep(q_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_SATISFY_HUNGER));
			q_ptr->number = (byte)rand_range(2, 5);
			object_aware(q_ptr);
			object_known(q_ptr);

			/* These objects are "storebought" */
			q_ptr->ident |= IDENT_STOREB;

#ifdef USE_SCRIPT
			q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

			(void)inven_carry(q_ptr);

			break;
		}
		default:
		{
			/* Food rations */
			object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
			q_ptr->number = (byte)rand_range(3, 7);
			object_aware(q_ptr);
			object_known(q_ptr);

#ifdef USE_SCRIPT
			q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

			(void)inven_carry(q_ptr);
		}
	}


	/* Get local object */
	q_ptr = &forge;

	if (p_ptr->prace == RACE_VAMPIRE)
	{
		/* Hack -- Give the player scrolls of DARKNESS! */
		object_prep(q_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_DARKNESS));

		q_ptr->number = (byte)rand_range(2, 5);

		object_aware(q_ptr);
		object_known(q_ptr);

		/* These objects are "storebought" */
		q_ptr->ident |= IDENT_STOREB;

#ifdef USE_SCRIPT
		q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);
	}
	else
	{
		/* Hack -- Give the player some torches */
		object_prep(q_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
		q_ptr->number = (byte)rand_range(3, 7);
		q_ptr->pval = rand_range(3, 7) * 500;
		object_aware(q_ptr);
		object_known(q_ptr);

#ifdef USE_SCRIPT
	q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);
	}


	/* Get local object */
	q_ptr = &forge;

	if (p_ptr->pclass == CLASS_RANGER)
	{
		/* Hack -- Give the player some arrows */
		object_prep(q_ptr, lookup_kind(TV_ARROW, SV_AMMO_NORMAL));
		q_ptr->number = (byte)rand_range(15, 20);

		/* These objects are "storebought" */
		q_ptr->ident |= IDENT_STOREB;

		object_aware(q_ptr);
		object_known(q_ptr);

#ifdef USE_SCRIPT
		q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);

		/* Hack -- Give the player some arrows */
		object_prep(q_ptr, lookup_kind(TV_BOW, SV_SHORT_BOW));

		/* These objects are "storebought" */
		q_ptr->ident |= IDENT_STOREB;

		object_aware(q_ptr);
		object_known(q_ptr);

#ifdef USE_SCRIPT
		q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);
	}
	else if (p_ptr->pclass == CLASS_HIGH_MAGE)
	{
		/* Hack -- Give the player some arrows */
		object_prep(q_ptr, lookup_kind(TV_WAND, SV_WAND_MAGIC_MISSILE));
		q_ptr->number = 1;
		q_ptr->pval = (byte)rand_range(25, 30);

		/* These objects are "storebought" */
		q_ptr->ident |= IDENT_STOREB;

		object_aware(q_ptr);
		object_known(q_ptr);

#ifdef USE_SCRIPT
		q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);
	}


	/* Hack -- Give the player three useful objects */
	for (i = 0; i < 3; i++)
	{
		/* Look up standard equipment */
		tv = player_init_borg[p_ptr->pclass][i][0];
		sv = player_init_borg[p_ptr->pclass][i][1];

		/* Hack to initialize spellbooks */
		if (tv == TV_SORCERY_BOOK) tv = TV_LIFE_BOOK + p_ptr->realm1 - 1;
		else if (tv == TV_DEATH_BOOK) tv = TV_LIFE_BOOK + p_ptr->realm2 - 1;

		else if (tv == TV_RING && sv == SV_RING_RES_FEAR &&
		    p_ptr->prace == RACE_BARBARIAN)
			/* Barbarians do not need a ring of resist fear */
			sv = SV_RING_SUSTAIN_STR;

		/* Get local object */
		q_ptr = &forge;

		/* Hack -- Give the player an object */
		object_prep(q_ptr, lookup_kind(tv, sv));

		/* Assassins begin the game with a poisoned dagger */
		if (tv == TV_SWORD && p_ptr->pclass == CLASS_ROGUE &&
			p_ptr->realm1 == REALM_DEATH) /* Only assassins get a poisoned weapon */
		{
			q_ptr->name2 = EGO_BRAND_POIS;
		}

		/* These objects are "storebought" */
		q_ptr->ident |= IDENT_STOREB;

		object_aware(q_ptr);
		object_known(q_ptr);

#ifdef USE_SCRIPT
		q_ptr->python = object_create_callback(q_ptr);
#endif /* USE_SCRIPT */

		(void)inven_carry(q_ptr);
	}


}

#ifdef BORG_TK
extern void borg_forget_messages(void);
#endif /* borg_tk */

/* Allow the borg to play continously.  Reset all values, */
void resurrect_borg(void)
{
    int i,ii,j;
    int n, t;
	bool borg_cheat_live;

    /* Cheat death */
    borg_cheat_live = cheat_live;
    death = FALSE;
    borg_skill[BI_MAXDEPTH] = 0;
    borg_skill[BI_MAXCLEVEL] = 1;

    /* Flush message buffer */
    borg_parse(NULL);

    /* flush the commands */
    borg_flush();

#ifdef BORG_TK
    void borg_forget_messages(void);
#endif /* borg_tk */

	borg_note("# Preparing to Resurrect.");

    /* remove the spell counters */
    for (ii = 1; ii < MAX_REALM+1; ii++)
    {
        for (i = 0; i < 4; i++ )
        {
           for (j = 0; j < 8; j++)
            {
                /* get the magics */
                borg_magic *as = &borg_magics[ii][i][j];
                /* reset the counter */
                as->times = 0;
            }
        }
	}

	/* Mindcrafter spell counts? */

    /*** Wipe the player ***/
    (void)WIPE(p_ptr, player_type);

	borg_note("# Wiped Player.");


	/* Game turn Starting over */
    turn = 0;

    borg_skill[BI_ISCUT] = borg_skill[BI_ISSTUN] = borg_skill[BI_ISHEAVYSTUN] = borg_skill[BI_ISIMAGE] = borg_skill[BI_ISSTUDY] = FALSE;

	/* Assign the 'Outpost' as town number */
	p_ptr->town_num = 1;
	p_ptr->wilderness_y = 4;
	p_ptr->wilderness_x = 4;
	dun_level = 0;
	leaving_quest = 0;
	py = p_ptr->oldpy = 33;
	px = p_ptr->oldpx = 133;

	/* Vanilla town */
	if (vanilla_town)
	{
		py = p_ptr->oldpy = 10;
		px = p_ptr->oldpx = 34;
		p_ptr->wilderness_y = 1;
		p_ptr->wilderness_x = 1;
	}


	/* Clean the mutation count */
	mutant_regenerate_mod = 100;

	/* Default pet command settings */
	p_ptr->pet_open_doors = FALSE;
	p_ptr->pet_pickup_items = FALSE;

#ifdef GJW_RANDART
    /* Hack -- seed for random artifacts */
    seed_randart = rand_int(0x10000000);

    /* Randomize the artifacts */
    if (adult_rand_artifacts)
    {
        do_randart(seed_randart);
    }
#endif

    /* Set some player flags to keep it running */
    alive = TRUE;
	death = FALSE;
    p_ptr->leaving = TRUE;

	/* No items */
	inven_cnt = 0;
	equip_cnt = 0;

    /* Clear the inventory */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_wipe(&inventory[i]);
    }
	borg_note("# Wiped Inventory.");


    flavor_init();
	borg_note("# Flavor Init'd.");


    /* borg_clear_3(); */
    /* borg_init_3(); */

	borg_note("# Init 3 Completed.");


    /* Start with no artifacts made yet */
    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        a_ptr->cur_num = 0;
    }

	/* Wipe the quests */
	for (i = 0; i < max_quests; i++)
	{
		quest[i].status = QUEST_STATUS_UNTAKEN;

		quest[i].cur_num = 0;
		quest[i].max_num = 0;
		quest[i].type = 0;
		quest[i].level = 0;
		quest[i].r_idx = 0;
	}
	borg_note("# Reset Artifacts and Quests.");


#if 0
    /* Free the "quarks" */
    for (i = 1; i < quark__num; i++)
    {
        string_free(quark__str[i]);
    }
#endif


	/* Reset the "objects" */
    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];


        /* skip some stuff */
        if (k_ptr->tval <= TV_AMULET ||
            k_ptr->tval >= TV_FLASK) continue;

        /* Reset "tried" */
        k_ptr->tried = FALSE;

        /* Reset "aware" */
        k_ptr->aware = FALSE;
    }
	borg_note("# Reset Objects.");

    /* Reset the "monsters" */
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Hack -- Reset the counter */
        r_ptr->cur_num = 0;

        /* Hack -- Reset the max counter */
        r_ptr->max_num = 100;

        /* Hack -- Reset the max counter */
        if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 1;
		if (r_ptr->flags3 & RF3_UNIQUE_7) r_ptr->max_num = 7;

        /* Clear player kills */
        r_ptr->r_pkills = 0;
    }
	borg_note("# Reset Monsters.");


    /* Hack -- no ghosts */
    r_info[max_r_idx-1].max_num = 0;


    /* Hack -- Well fed player */
    p_ptr->food = PY_FOOD_FULL - 1;

	/* Wipe the spells */
	spell_learned1 = spell_learned2 = 0L;
	spell_worked1 = spell_worked2 = 0L;
	spell_forgotten1 = spell_forgotten2 = 0L;
	for (i = 0; i < 64; i++) spell_order[i] = 99;

	/* Clean the mutation count */
	mutant_regenerate_mod = 100;


    /** Roll up a new character **/

    /* Sex */
    p_ptr->psex = rand_int(MAX_SEXES);
    sp_ptr = &sex_info[p_ptr->psex];


    /* Class */
    if (borg_respawn_class == -1)
    {
        p_ptr->pclass = rand_int(MAX_CLASS);
    }
    else
    {
        p_ptr->pclass = borg_respawn_class;
    }

    cp_ptr = &class_info[p_ptr->pclass];
    mp_ptr = &magic_info[p_ptr->pclass];

	borg_note("# Class Defined.");


   if (borg_respawn_race == -1)
   {
    while (1)
    {
       /* Race */
       p_ptr->prace = rand_int(MAX_RACES);

       rp_ptr = &race_info[p_ptr->prace];

       /* Try again if not a legal choice */
       if (!(rp_ptr->choice & (1L << p_ptr->pclass))) continue;
       break;
    }
   }
   else
   {
        p_ptr->prace = borg_respawn_race;
        rp_ptr = &race_info[p_ptr->prace];
   }

	borg_note("# Race Defined.");


    /* Some Extra things */
    get_stats_borg();
    get_extra_borg();
    get_ahw_borg();
    get_history_borg();
    get_money_borg();

	borg_note("# Set stats, extra, ahw, history, money.");


    /* Get a random name */
    create_random_name(p_ptr->prace,player_name);

	/* Hack -- get a chaos patron even if you are not a chaos warrior */
	p_ptr->chaos_patron = (s16b)rand_int(MAX_PATRON);

	/* Hack -- enter the world */
	if ((p_ptr->prace == RACE_VAMPIRE) ||
	    (p_ptr->prace == RACE_SKELETON) ||
	    (p_ptr->prace == RACE_ZOMBIE) ||
	    (p_ptr->prace == RACE_SPECTRE))
	{
		/* Undead start just after midnight */
		turn = (30L * TOWN_DAWN) / 4 + 1;
	}
	else
	{
		turn = 1;
	}


    /* Reset the Shops */
    for (t = 1; t < max_towns; t++)
    {
    	for (n = 0; n < MAX_STORES; n++)
    	{
    	    /* Initialize */
    	    store_init(t, n);

    	    /* Maintain the shop (ten times), */
    	    for (i = 0; i < 10; i++) store_maint(t, n);
    	}
	}
	borg_note("# Wiped and Reset Shops.");


    /* Hack -- flush it */
    Term_fresh();

    /*** Hack -- Extract race ***/

    /* Insert the player Race--cheat */
    borg_race = p_ptr->prace;

    /* Extract the race pointer */
    rb_ptr = &race_info[borg_race];


    /*** Hack -- Extract class ***/

    /* Cheat the class */
    borg_class = p_ptr->pclass;

	/* Reset Realms */
	p_ptr->realm1 = 0;
	p_ptr->realm2 = 0;

	/* Get some Realms of Magic */
	switch (borg_class)
	{
		case CLASS_WARRIOR:
		case CLASS_ORPHIC:
			break;
		case CLASS_CHAOS_WARRIOR:
			/* No Realm Choices */
			p_ptr->realm1 = REALM_CHAOS;
			break;
		case CLASS_MAGE:
			/* Realm One */
			if (borg_respawn_realm_1 == 0) p_ptr->realm1 = rand_int(MAX_REALM) +1;
			else p_ptr->realm1 = borg_respawn_realm_1;

			/* Realm Two */
			while (1)
			{
				/* Assign random realm */
				if (borg_respawn_realm_2 == 0 ||
				    borg_respawn_realm_2 == p_ptr->realm1)
				{
				    p_ptr->realm2 = rand_int(MAX_REALM) +1;
				}
				else p_ptr->realm2 = borg_respawn_realm_2;

				/* Make sure its not same */
				if (p_ptr->realm2 != p_ptr->realm1) break;

				/* Gotta get a new Realm Two */
				continue;
			}
			break;
		case CLASS_PRIEST:
			/* Realm One -- Life or Death */
			if (borg_respawn_realm_1 == REALM_LIFE ||
			    borg_respawn_realm_1 == REALM_DEATH)
			{
				p_ptr->realm1 = borg_respawn_realm_1;
			}
			else /* Random */
			{
				p_ptr->realm1 = ((rand_int(50) > 25) ?
					REALM_LIFE : REALM_DEATH);
			}

			/* Realm Two */
			while (1)
			{
				/* Assign random realm */
				if (borg_respawn_realm_2 == 0 ||
				    borg_respawn_realm_2 == p_ptr->realm1 ||
				    (p_ptr->realm1 == REALM_LIFE && borg_respawn_realm_2 == REALM_DEATH) ||
				    (p_ptr->realm1 == REALM_DEATH && borg_respawn_realm_2 == REALM_LIFE))
				{
				    p_ptr->realm2 = rand_int(MAX_REALM) +1;
				}
				else
				{
					p_ptr->realm2 = borg_respawn_realm_2;
				}

				/* Certain Class Restrictions: */
				if (p_ptr->realm1 == REALM_LIFE &&
				     p_ptr->realm2 == REALM_DEATH) continue;
				if (p_ptr->realm2 == REALM_LIFE &&
				     p_ptr->realm1 == REALM_DEATH) continue;
				if (p_ptr->realm2 == p_ptr->realm1) continue;


				/* OK, realm2 is legal */
				break;
			}
			break;
		case CLASS_ROGUE:
			/* Realm One */
			while (1)
			{
				if (borg_respawn_realm_1 == REALM_LIFE ||
				    borg_respawn_realm_1 == REALM_NATURE ||
				    borg_respawn_realm_1 == REALM_CHAOS ||
				    borg_respawn_realm_1 == REALM_NONE)
				{
					/* Assign random realm */
					p_ptr->realm1 = rand_int(MAX_REALM) +1;
				}
				else
				{
					p_ptr->realm1 = borg_respawn_realm_1;
				}

				/* Certain Class Restrictions: */
				if (p_ptr->realm1 == REALM_LIFE) continue;
				if (p_ptr->realm1 == REALM_NATURE) continue;
				if (p_ptr->realm1 == REALM_CHAOS) continue;
				if (p_ptr->realm1 == REALM_NONE) continue;

				/* OK, realm is legal */
				break;
			}
			break;
		case CLASS_RANGER:
			/* Realm One */
			p_ptr->realm1 = REALM_NATURE;

			/* Realm Two */
			while (1)
			{
				if (borg_respawn_realm_2 == REALM_LIFE ||
				    borg_respawn_realm_2 == REALM_NATURE ||
				    borg_respawn_realm_2 == REALM_NONE)
				{
					/* Assign random realm */
					p_ptr->realm2 = rand_int(MAX_REALM) + 1;
				}
				else
				{
					p_ptr->realm2 = borg_respawn_realm_1;
				}

				/* Certain Class Restrictions: */
				if (p_ptr->realm2 == REALM_LIFE) continue;
				if (p_ptr->realm2 == REALM_NATURE) continue;
				if (p_ptr->realm2 == REALM_NONE) continue;

 				/* OK, realm is legal */
				break;
			}
			break;

		case CLASS_PALADIN:
			/* Realm One -- Life or Death */
			if (borg_respawn_realm_1 == REALM_LIFE ||
			    borg_respawn_realm_1 == REALM_DEATH)
			{
				p_ptr->realm1 = borg_respawn_realm_1;
			}
			else /* Random */
			{
				p_ptr->realm1 = ((rand_int(50) > 25) ?
					REALM_LIFE : REALM_DEATH);
			}
			break;
		case CLASS_WARRIOR_MAGE:
			/* Realm One */
			p_ptr->realm1 = REALM_ARCANE;

			/* Realm Two */
			while (1)
			{
				if (borg_respawn_realm_2 == REALM_ARCANE ||
				    borg_respawn_realm_2 == REALM_NONE)
				{
					/* Assign random realm */
					p_ptr->realm2 = rand_int(MAX_REALM) + 1;
				}
				else
				{
					p_ptr->realm2 = borg_respawn_realm_1;
				}

				/* Certain Class Restrictions: */
				if (p_ptr->realm2 == REALM_ARCANE) continue;
				if (p_ptr->realm2 == REALM_NONE) continue;

 				/* OK, realm is legal */
				break;
			}
			break;
		case CLASS_MONK:
			/* Realm One */
			while (1)
			{
				if (borg_respawn_realm_1 == REALM_ARCANE ||
				 	borg_respawn_realm_1 == REALM_SORCERY ||
				 	borg_respawn_realm_1 == REALM_TRUMP ||
				 	borg_respawn_realm_1 == REALM_CHAOS ||
				    borg_respawn_realm_1 == REALM_NONE)
				{
					/* Assign random realm */
					p_ptr->realm1 = rand_int(MAX_REALM) + 1;
				}
				else
				{
					p_ptr->realm1 = borg_respawn_realm_1;
				}

				/* Certain Class Restrictions: */
				if (p_ptr->realm1 == REALM_ARCANE) continue;
				if (p_ptr->realm1 == REALM_SORCERY) continue;
				if (p_ptr->realm1 == REALM_TRUMP) continue;
				if (p_ptr->realm1 == REALM_CHAOS) continue;
				if (p_ptr->realm1 == REALM_NONE) continue;

 				/* OK, realm is legal */
				break;
			}
			break;
		case CLASS_HIGH_MAGE:
			/* Assign random realm */
			if (borg_respawn_realm_1 == 0) p_ptr->realm1 = rand_int(MAX_REALM) +1;
			else p_ptr->realm1 = borg_respawn_realm_1;
			break;
	}

    /* Extract the class pointer */
    cb_ptr = &class_info[borg_class];

    /* Extract the magic pointer */
    mb_ptr = &magic_info[borg_class];

    /* outfit the player */
    (void)player_outfit_borg();
	borg_note("# Outfitted Player.");

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    prepare_race_class_info();
	borg_note("# Prepared Race/Class.");

	/* Fully healed */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Fully rested */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* need to check all stats */
    my_need_stat_check[0] = TRUE;
    my_need_stat_check[1] = TRUE;
    my_need_stat_check[2] = TRUE;
    my_need_stat_check[3] = TRUE;
    my_need_stat_check[4] = TRUE;
    my_need_stat_check[5] = TRUE;

    /* Allowable Cheat -- Obtain "recall" flag */
    goal_recalling = p_ptr->word_recall * 1000;

    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
    borg_prot_from_evil = (p_ptr->protevil ? TRUE : FALSE);
    /* Allowable Cheat -- Obtain "speed" flag */
    borg_speed = (p_ptr->fast ? TRUE : FALSE);
    /* Allowable Cheat -- Obtain "goi" flag */
    borg_goi = (p_ptr->invuln ? 9000 : 0);
    /* Allowable Cheat -- Obtain "wraithform" flag */
    borg_wraith = (p_ptr->wraith_form ? 25000 : 0);
    /* Allowable Cheat -- Obtain "resist" flags */
    borg_skill[BI_TRACID] = (p_ptr->oppose_acid ? TRUE : FALSE);
    borg_skill[BI_TRELEC] = (p_ptr->oppose_elec ? TRUE : FALSE);
    borg_skill[BI_TRFIRE] = (p_ptr->oppose_fire ? TRUE : FALSE);
    borg_skill[BI_TRCOLD] = (p_ptr->oppose_cold ? TRUE : FALSE);
    borg_skill[BI_TRPOIS] = (p_ptr->oppose_pois ? TRUE : FALSE);
    borg_bless = (p_ptr->blessed ? TRUE : FALSE);
    borg_shield = (p_ptr->shield ? TRUE : FALSE);
    borg_hero = (p_ptr->hero ? TRUE : FALSE);
    borg_berserk = (p_ptr->shero ? TRUE : FALSE);

	/* Reload the Borg.txt */
    borg_note("Reloading the Borg rules... (zborg.txt)");
    /*** Hack -- initialize borg.ini options ***/

    for (i = 0; i < MAX_CLASS; i++)
    {
		if (i != borg_class) continue;

        C_KILL(borg_required_item[i], 400, req_item); /* externalize the 400 later */
        C_KILL(borg_power_item[i], 400, power_item); /* externalize the 400 later */
    }
    C_KILL(borg_has, size_obj, int);
    for (i = 0; i < 1000; i++)
    {
        if (formula[i])
            C_KILL(formula[i], MAX_FORMULA_ELEMENTS, int);
    }

	init_borg_txt_file();


    /* Message */
    borg_note("# Respawning");
    borg_respawning = 5;

    /* fully healed and rested */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->csp = p_ptr->msp;

	/* Place borg in town */


#ifdef BORG_TK
    angtk_eval("angband_borg", "rebirth", NULL);
#endif /* borg_tk */

    /* Make sure Cheat_live was not reset to FALSE */
    cheat_live = borg_cheat_live;

   /* Done.  Play on */
}

#endif /*bablos */
/*
 * Mega-Hack -- special "inkey_hack" hook.  XXX XXX XXX
 *
 * A special function hook (see "util.c") which allows the Borg to take
 * control of the "inkey()" function, and substitute in fake keypresses.
 */
#ifdef BORG_TK
ANG_EXTERN char (*inkey_hack)(int flush_first);
#else /* BORG_TK */
extern char (*inkey_hack)(int flush_first);
#endif /* BORG_TK */

/*
 * This function lets the Borg "steal" control from the user.
 *
 * The "util.c" file provides a special "inkey_hack" hook which we use
 * to steal control of the keyboard, using the special function below.
 *
 * Since this function bypasses the code in "inkey()" which "refreshes"
 * the screen whenever the game has to wait for a keypress, the screen
 * will only get refreshed when (1) an option such as "fresh_before"
 * induces regular screen refreshing or (2) various explicit calls to
 * "Term_fresh" are made, such as in the "project()" function.  This
 * has the interesting side effect that the screen is never refreshed
 * while the Borg is browsing stores, checking his inventory/equipment,
 * browsing spell books, checking the current panel, or examining an
 * object, which reduces the "screen flicker" considerably.  :-)
 *
 * The only way that the Borg can be stopped once it is started, unless
 * it dies or encounters an error, is to press any key.  This function
 * checks for real user input on a regular basic, and if any is found,
 * it is flushed, and after completing any actions in progress, this
 * function hook is removed, and control is returned to the user.
 *
 * We handle "broken" messages, in which long messages are "broken" into
 * pieces, and all but the first message are "indented" by one space, by
 * collecting all the pieces into a complete message and then parsing the
 * message once it is known to be complete.
 *
 * This function hook automatically removes itself when it realizes that
 * it should no longer be active.  Note that this may take place after
 * the game has asked for the next keypress, but the various "keypress"
 * routines should be able to handle this.
 */
static char borg_inkey_hack(int flush_first)
{
    char ch;

    int y = 0;
    int x = 80;

    byte t_a;

    char buf[128];

bool borg_prompt;  /* ajg  For now we can just use this locally.
                           in the 283 borg he uses this to optimize knowing if
                           we are waiting at a prompt for info */

#ifndef BORG_TK
    /* Refresh the screen */
    Term_fresh();
#endif /* not BORG_TK */

    /* Deactivate */
    if (!borg_active)
    {
        /* Message */
        borg_note("# Removing keypress hook");

        /* Remove hook */
        inkey_hack = NULL;

        /* Flush keys */
        borg_flush();

        /* Flush */
        flush();
#ifdef BORG_TK
        angtk_eval("angband_borg", "deactivate", NULL);
#endif /* BORG_TK */

        /* Done */
        return (0);
    }


    /* Mega-Hack -- flush keys */
    if (flush_first)
    {
        /* Only flush if needed */
        if (borg_inkey(FALSE) != 0)
        {
            /* Message */
            borg_note("# Flushing keypress buffer");

            /* Flush keys */
            borg_flush();
        }
    }


#ifdef BORG_TK
    /* Do this after flushing (or else?) */
    if (inkey_flags)
        Bind_Generic(EVENT_INKEY, inkey_flags);
#endif /* BORG_TK */


    /* Locate the cursor */
    (void)Term_locate(&x, &y);


    /* Assume no prompt/message is available */
    borg_prompt = FALSE;

#ifdef BORG_TK

    /* Mega-Hack -- check for possible prompts/messages */
    if (strlen(angtk_prompt_text))
    {
        /* Get the text */
        strcpy(buf, angtk_prompt_text);

        /* Assume a prompt/message is available */
        borg_prompt = TRUE;
    }

    /* with 292, there is a flush() introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * Attempt to catch "Attempt it anyway? [y/n]"
     */
    if (borg_prompt && !inkey_flag &&
        streq(buf, "Atte"))
    {
        /* Return the confirmation */
        borg_note("# Confirming use of Spell/Prayer.");
        return ('y');
    }

    /* with 292, there is a flush() introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * This is used only with emergency use of spells like Magic Missile
     * Attempt to catch "Direction (5 old target"
     */
    if (borg_prompt && !inkey_flag && borg_confirm_target &&
        streq(buf, "Dire"))
    {
        /* reset the flag */
        borg_confirm_target = FALSE;
        /* Return 5 for old target */
            return ('5');
    }

    /* Mega-Hack -- Catch "Die? [y/n]" messages */
    if (borg_prompt && !inkey_flag &&
        (streq(buf, "Die? [y/n]") ||
		 streq(buf, "Dump the screen? [y/n]"))
    {
        /* Flush messages */
        borg_parse(NULL);

        /* flush the buffer */
        borg_flush();

        /* Take note */
        borg_note("# Cheating death...");

#ifndef BABLOS
        /* Dump the Character Map*/
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation")) borg_write_map(FALSE);

        /* Log the death */
        borg_log_death();
        borg_log_death_data();

        /* Reset the player game data then resurrect a new player */
        resurrect_borg();
#endif /* BABLOS */

        /* Cheat death */
        return ('n');
    }


    /* Mega-Hack -- Handle death */
    if (death)
    {

#ifndef BABLOS
        /* Print the map */
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation"))  borg_write_map(FALSE);

        /* Log death */
        borg_log_death();
        borg_log_death_data();

#endif /* BABLOS */

        /* flush the buffer */
        borg_flush();

        if (borg_cheat_death)
        {
            /* Reset death flag */
            death = FALSE;

#ifndef BABLOS
            /* Reset the player game data then resurrect a new player */
            resurrect_borg();
#endif /* BABLOS */

        }
        else
        {
            /* Oops  */
            borg_oops("player died");

            /* Useless keypress */
            return (KTRL('C'));
        }
    }


    /* Mega-Hack -- Catch "-more-" messages */
    if (borg_prompt && (inkey_flags == INKEY_MORE))
    {
        char msg[160];

        strcpy(msg, buf);
        msg[strlen(msg) - 7] = '\0';

        /* Parse it */
        borg_parse(msg);

        /* Clear the message */
        return (KTRL('M'));
    }

    /* Mega-Hack -- catch normal messages */
    if (borg_prompt && inkey_flag)
    {
        int k = strlen(buf);

        /* Strip trailing spaces */
        while ((k > 0) && (buf[k-1] == ' ')) k--;

        /* Terminate */
        buf[k] = '\0';

        /* Parse it */
        borg_parse(buf);

        /* Clear the message */
        return (KTRL('M'));
    }

#else /* not BORG_TK */

    /* Mega-Hack -- check for possible prompts/messages */
    /* If the first four characters on the message line all */
    /* have the same attribute (or are all spaces), and they */
    /* are not all spaces (ascii value 0x20)... */
    if ((0 == borg_what_text(0, 0, 4, &t_a, buf)) &&
        (t_a != TERM_DARK) &&
        (*((u32b*)(buf)) != 0x20202020))
    {
        /* Assume a prompt/message is available */
        borg_prompt = TRUE;
    }


    /* Mega-Hack -- Catch "Die? [y/n]" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And the text acquired above is "Die?" */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 4) &&
        (streq(buf, "Die?") ||
		streq(buf, "Dump"))	&&
        borg_cheat_death)
    {
        /* Flush messages */
        borg_parse(NULL);

        /* flush the buffer */
        borg_flush();

        /* Take note */
        borg_note("# Cheating death...");

        /* Dump the Character Map*/
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation")) borg_write_map(FALSE);

        /* Log the death */
		save_player();
        borg_log_death();
        borg_log_death_data();

        /* Reset the player game data then resurrect a new player */
        resurrect_borg();

        /* Dont dump the screen */
        return ('n');
    }
    /* Catch Munchkin_death line "Save Death" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And the text acquired above is "Die?" */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Save") ||
		streq(buf, "Dump"))
    {
        /* Flush messages */
        borg_parse(NULL);

        /* flush the buffer */
        borg_flush();

        /* Take note */
        borg_note("# Borg death...");

        /* Dump the Character Map*/
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation")) borg_write_map(FALSE);

        /* Log the death */
		save_player();
        borg_log_death();
        borg_log_death_data();
		borg_cancel = TRUE;

        /* dump the screen? */
        return ('n');
    }

	/* with V292, there is a flush() introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * Attempt to catch "Attempt it anyway? [y/n]"
     */
    /* There is no flush() in Z240 */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Atte"))
    {
        /* Return the confirmation */
        borg_note("# Confirming use of Spell/Prayer.");
        return ('y');
    }


    /* with 292, there is a flush() introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * This is used only with emergency use of spells like Magic Missile
     * Attempt to catch "Direction (5 old target"
     */
    if (borg_prompt && !inkey_flag && borg_confirm_target &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Dire"))
    {
        /* reset the flag */
        borg_confirm_target = FALSE;
        /* Return 5 for old target */
        return ('5');
    }

    /* Mega-Hack -- Handle death */
    if (death)
    {
#ifndef BABLOS
        /* Print the map */
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(died_from, "starvation"))  borg_write_map(FALSE);

        /* Log death */
		/* save_player(); */
        borg_log_death();
        borg_log_death_data();


#endif /* bablos */
        /* flush the buffer */
        borg_flush();

        if (borg_cheat_death)
        {
            /* Reset death flag */
            death = FALSE;
#ifndef BABLOS
            /* Reset the player game data then resurrect a new player */
            resurrect_borg();
#endif /* bablos */
        }
        else
        {
            /* Oops  */
            borg_oops("player died");

            /* Useless keypress */
            return (KTRL('C'));
        }
    }


    /* Mega-Hack -- Catch "-more-" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And there is text before the cursor... */
    /* And that text is "-more-" */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 7) &&
        (0 == borg_what_text(x-7, y, 7, &t_a, buf)) &&
        (streq(buf, " -more-")))
    {
        /* Get the message */
        if (0 == borg_what_text(0, 0, x-7, &t_a, buf))
        {
            /* Parse it */
            borg_parse(buf);
        }
        /* Clear the message */
        return (KTRL('M'));
    }

    /* Mega-Hack -- catch normal messages */
    /* If there is text on the first line... */
    /* And the game wants a command */
    if (borg_prompt && inkey_flag)
    {
        /* Get the message(s) */
        if (0 == borg_what_text(0, 0, -80, &t_a, buf))
        {
            int k = strlen(buf);

            /* Strip trailing spaces */
            while ((k > 0) && (buf[k-1] == ' ')) k--;

            /* Terminate */
            buf[k] = '\0';

            /* Parse it */
            borg_parse(buf);
        }

        /* Clear the message */
        return (KTRL('M'));
#endif /* not BORG_TK */
    }
    /* Flush messages */
    borg_parse(NULL);
    borg_dont_react = FALSE;

    /* Check for key */
    ch = borg_inkey(TRUE);

    /* Use the key */
    if (ch) return (ch);


    /* Check for user abort */
    (void)Term_inkey(&ch, FALSE, FALSE);

    /* User Abort */
    if (ch != 0)
    {
        /* Oops */
        borg_oops("user abort");

        /* Hack -- Escape */
        return (ESCAPE);
    }

    /* Save the system random info */
    borg_rand_quick = Rand_quick;
    borg_rand_value = Rand_value;

    /* Use the local random info */
    Rand_quick = TRUE;
    Rand_value = borg_rand_local;


    /* Think */
    while (!borg_think()) /* loop */;

    /* DVE- Update the status screen */
    borg_status();

    /* Save the local random info */
    borg_rand_local = Rand_value;

    /* Restore the system random info */
    Rand_quick = borg_rand_quick;
    Rand_value = borg_rand_value;

    /* Hack -- allow stepping to induce a clean cancel */
    if (borg_step && (!--borg_step)) borg_cancel = TRUE;


    /* Check for key */
    ch = borg_inkey(TRUE);

    /* Use the key */
    if (ch) return (ch);


    /* Oops */
    borg_oops("normal abort");

    /* Hack -- Escape */
    return (ESCAPE);
}

/*
 * Output a long int in binary format.
 */
static void borg_prt_binary(u32b flags, int row, int col)
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

/* this will display the values which the borg believes an
 * item has.  Select the item by inven # prior to hitting
 * the ^zo.
 */
static void borg_display_item(object_type *item2)
{
	int i, j = 13;
	u32b f1, f2, f3;

	borg_item *item;

	item = &borg_items[command_arg];


	/* Extract the flags */
	object_flags(item2, &f1, &f2, &f3);

	/* Clear the screen */
	for (i = 1; i <= 23; i++) prt("", i, j - 2);

	/* Describe fully */
	prt(item->desc, 2, j);

	prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
	           item->kind, item->level,
	           item->tval, item->sval), 4, j);

	prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
	           item->iqty, item->weight,
	           item->ac, item->dd, item->ds), 5, j);

	prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
	           item->pval, item->to_a, item->to_h, item->to_d), 6, j);

	prt(format("name1 = %-4d  name2 = %-4d  name3 = %-5d value = %-2d   cursed = %-2ld",
	           item->name1, item->name2, item->name3, (long)item->value, item->cursed), 7, j);

	prt(format("aware = %-1d     id'd = %-1d      fully_id = %-1d  timeout = %-1d",
	           item->aware, item->ident,item->fully_identified, item->timeout), 8, j);

	/* maybe print the inscription */
	prt(format("Inscription: %s",item->note),9,j);


	prt("+------------FLAGS1------------+", 10, j);
	prt("AFFECT........SLAY........BRAND.", 11, j);
	prt("              cvae      xsqpaefc", 12, j);
	prt("siwdcc  ssidsahanvudotgddhuoclio", 13, j);
	prt("tnieoh  trnipttmiinmrrnrrraiierl", 14, j);
	prt("rtsxna..lcfgdkcpmldncltggpksdced", 15, j);
	borg_prt_binary(f1, 16, j);

	prt("+------------FLAGS2------------+", 17, j);
	prt("SUST...IMMUN..RESIST............", 18, j);
	prt("        aefctrpsaefcpfldbc sn   ", 19, j);
	prt("siwdcc  clioheatcliooeialoshtncd", 20, j);
	prt("tnieoh  ierlrfraierliatrnnnrhehi", 21, j);
	prt("rtsxna..dcedwlatdcedsrekdfddrxss", 22, j);
	borg_prt_binary(f2, 23, j);

	prt("+------------FLAGS3------------+", 10, j+32);
	prt("fe      ehsi  st    iiiiadta  hp", 11, j+32);
	prt("il   n taihnf ee    ggggcregb vr", 12, j+32);
	prt("re  nowysdose eld   nnnntalrl ym", 13, j+32);
	prt("ec  omrcyewta ieirmsrrrriieaeccc", 14, j+32);
	prt("aa  taauktmatlnpgeihaefcvnpvsuuu", 15, j+32);
	prt("uu  egirnyoahivaeggoclioaeoasrrr", 16, j+32);
	prt("rr  litsopdretitsehtierltxrtesss", 17, j+32);
	prt("aa  echewestreshtntsdcedeptedeee", 18, j+32);
	borg_prt_binary(f3, 19, j+32);

	return;
}

#ifdef ALLOW_BORG_GRAPHICS

glyph translate_visuals[255][255];

/*
 * Return the "attr" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_kind_attr(T) \
   (((T)->flavor) ? \
    (misc_to_attr[(T)->flavor]) : \
    ((T)->x_attr))

/*
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_kind_char(T) \
   (((T)->flavor) ? \
    (misc_to_char[(T)->flavor]) : \
    ((T)->x_char))


void init_translate_visuals(void)
{
   int i,j;

   f_info[1].x_char = 46;
   f_info[2].x_char = 46;

   /* Extract default attr/char code for features */
   for (i = 0; i < max_f_idx; i++)
   {
       feature_type *f_ptr = &f_info[i];

       if (!f_ptr->name) continue;

       /* Store the underlying values */
       translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
       translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

       /* Boring grids (floors, etc) */
           if (view_special_lite && (f_ptr->x_attr == TERM_WHITE) && (i <= FEAT_INVIS))
           {
               translate_visuals[TERM_YELLOW][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
               translate_visuals[TERM_YELLOW][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

               translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
               translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

               translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
               translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_char = f_ptr->d_char;
           }
           else if (view_granite_lite && (f_ptr->x_attr == TERM_WHITE) && (i >= FEAT_SECRET))
           {
               translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
               translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

               translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
               translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_char = f_ptr->d_char;
           }

   }

   /* Extract default attr/char code for objects */
   for (i = 0; i < max_k_idx; i++)
   {
       object_kind *k_ptr = &k_info[i];

       if (!k_ptr->name) continue;

       /* Store the underlying values */
       translate_visuals[(byte)object_kind_attr(k_ptr)][(byte)object_kind_char(k_ptr)].d_attr = k_ptr->d_attr;
       translate_visuals[(byte)object_kind_attr(k_ptr)][(byte)object_kind_char(k_ptr)].d_char = k_ptr->d_char;
   }

   /* Extract default attr/char code for monsters */
   for (i = 0; i < max_r_idx; i++)
   {
       monster_race *r_ptr = &r_info[i];

       if (!r_ptr->name) continue;

       /* Store the underlying values */
       translate_visuals[(byte)r_ptr->x_attr][(byte)r_ptr->x_char].d_attr = r_ptr->d_attr;
       translate_visuals[(byte)r_ptr->x_attr][(byte)r_ptr->x_char].d_char = r_ptr->d_char;

       /* Multi-hued monster in ASCII mode */
       if ((r_ptr->flags1 & (RF1_ATTR_MULTI)) &&
           !((r_ptr->x_attr & 0x80) && (r_ptr->x_char & 0x80)))
       {
           for (j = 0; j < 16; j++)
           {
               translate_visuals[j][(byte)r_ptr->x_char].d_attr = j;
               translate_visuals[j][(byte)r_ptr->x_char].d_char = r_ptr->d_char;
           }
       }
   }
}

#endif /* ALLOW_BORG_GRAPHICS */


/*
 * Initialize the Borg
 */
void borg_init_9(void)
{
    byte *test;

#ifndef ALLOW_BORG_GRAPHICS
	int i;
#endif /* allow_borg_graphics */

    /*** Hack -- verify system ***/


    /* Message */
    prt("Initializing the Borg... (memory)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    C_MAKE(test, 400 * 1024L, byte);
    C_KILL(test, 400 * 1024L, byte);


    /*** Hack -- initialize borg.ini options ***/

    /* Message */
    prt("Initializing the Borg... (zborg.txt)", 0, 0);
    init_borg_txt_file();


    /*** Hack -- initialize game options ***/

    /* Message */
    prt("Initializing the Borg... (options)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* We use the original keypress codes */
    rogue_like_commands = FALSE;

    /* We pick up items when we step on them */
    always_pickup = TRUE;

    /* We specify targets by hand */
    use_old_target = FALSE;


    /* We must pick items up without verification */
    carry_query_flag = FALSE;

    /* We repeat by hand */
    always_repeat = FALSE;

    /* We do not haggle */
    auto_haggle = TRUE;

    /* We do not start Scumming */
    auto_scum = FALSE;

    /* We need space */
    show_labels = FALSE;
    show_weights = FALSE;

    /* We need the dungeon level */
    depth_in_feet = FALSE;

    /* Allow items to stack */
    stack_force_notes = TRUE;
    stack_force_costs = TRUE;


	/* Zangband Commands options */
	plain_descriptions = TRUE;
	easy_open = FALSE;
	easy_disarm = FALSE;
	small_levels = FALSE;
	auto_destroy= FALSE;
	empty_levels = FALSE;
	last_words = FALSE;

    /* Ignore discounts */
    stack_force_costs = TRUE;

    /* Ignore inscriptions */
    stack_force_notes = TRUE;

    /* Efficiency */
    avoid_abort = TRUE;

    /* Efficiency */
    alert_hitpoint = 0;

    /* Hack -- notice "command" mode */
    hilite_player = FALSE;

#ifdef BORG_TK
#ifdef ALLOW_EASY_SENSE
    easy_sense = FALSE;
#endif /* ALLOW_EASY_SENSE */

#ifdef ALLOW_EASY_RING
    easy_ring = FALSE; /* This could actually help the Borg! */
#endif /* ALLOW_EASY_RING */

#ifdef ALLOW_REMEMBER_RECALL
    remember_recall = FALSE;
#endif /* ALLOW_REMEMBER_RECALL */

#endif /* BORG_TK */

#ifndef ALLOW_BORG_GRAPHICS
    if (!borg_graphics)
    {
        /* Reset the # and % -- Scan the features */
        for (i = 1; i < max_f_idx; i++)
        {
            feature_type *f_ptr = &f_info[i];

            /* Skip non-features */
            if (!f_ptr->name) continue;

            /* Switch off "graphics" */
            f_ptr->x_attr = f_ptr->d_attr;
            f_ptr->x_char = f_ptr->d_char;
        }
    }
#endif

#ifdef ALLOW_BORG_GRAPHICS

   init_translate_visuals();

#else /* ALLOW_BORG_GRAPHICS */

#ifdef USE_GRAPHICS
#ifndef BORG_TK
   /* The Borg can't work with graphics on, so switch it off */
   if (use_graphics)
   {
       /* Reset to ASCII mode */
       use_graphics = FALSE;
       arg_graphics = FALSE;

       /* Reset visuals */
       reset_visuals();
   }
#endif /* not BORG_TK */
#endif /* USE_GRAPHICS */

#endif /* ALLOW_BORG_GRAPHICS */

    /*** Redraw ***/
    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Window stuff */
    p_ptr->window |= (PW_OVERHEAD);

    /* Redraw everything */
    do_cmd_redraw();
    /*** Various ***/

    /* Message */
    prt("Initializing the Borg... (various)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();


    /*** Cheat / Panic ***/

    /* more cheating */
    borg_cheat_death = FALSE;

    /* set the continous play mode if the game cheat death is on */
    if (cheat_live) borg_cheat_death = TRUE;

	/* Do not overwrite the character file */
	munchkin_death = TRUE;

    /*** Initialize ***/

    /* Initialize */
	borg_init_1();
    borg_init_2();
    borg_init_3();
	borg_init_4();
    borg_init_5();
    borg_init_6();
    borg_init_7();
    borg_init_8();


    /*** Hack -- Extract race ***/

    /* Insert the player Race--cheat */
    borg_race = p_ptr->prace;

    /* Extract the race pointer */
    rb_ptr = &race_info[borg_race];


    /*** Hack -- Extract class ***/
    borg_class = p_ptr->pclass;

    /* Extract the class pointer */
    cb_ptr = &class_info[borg_class];

    /* Extract the magic pointer */
    mb_ptr = &magic_info[borg_class];


    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    prepare_race_class_info();


    /*** All done ***/

    /* Done initialization */
    prt("Initializing the Borg... done.", 0, 0);

    /* Clear line */
    prt("", 0, 0);

    /* Reset the clock */
    borg_t = 10;

    /* Official message */
    borg_note("# Ready...");

    /* Now it is ready */
    initialized = TRUE;

	/* Set some town stuff */
	town_x_wild[1] = 4;	/* Wilderness location of Towns */
	town_y_wild[1] = 4;	/* Wilderness location of Towns */
	town_x_wild[2] = 6;	/* Wilderness location of Towns */
	town_y_wild[2] = 4;	/* Wilderness location of Towns */
	town_x_wild[3] = 4;	/* Wilderness location of Towns */
	town_y_wild[3] = 2;	/* Wilderness location of Towns */
	town_x_wild[4] = 5;	/* Wilderness location of Towns */
	town_y_wild[4] = 6;	/* Wilderness location of Towns */
	town_x_wild[5] = 5;	/* Wilderness location of Towns */
	town_y_wild[5] = 8;	/* Wilderness location of Towns */
	/*  # Wilderness layout
	 *  W:D:##########
	 *  W:D:#GGGGMVMM#
	 *  W:D:#GGG3GGMM#
	 *  W:D:#GGG|GGGG#
	 *  W:D:#GGG1-2GG#
	 *  W:D:#GGDD|DGG#
	 *  W:D:#WSFF4FFS#
	 *  W:D:#WWSSSSSS#
	 *  W:D:#WWWW5WWW#
	 *  W:D:##########
	 * Towns at 1,2,3,4,5.
	 */

}

#ifndef BABLOS
/*
 * Write a file with the current dungeon info (Borg)
 * and his equipment, inventory and home (Player)
 * and his swap armor, weapon (Borg)
 * From Dennis Van Es,  With an addition of last messages from me ()
 */
void borg_write_map(bool ask)
{
    char buf2[1024];
    char buf[80];
    FILE *borg_map_file;
    char line[MAX_WID + 1];

    borg_item *item;

    int i,j,k;
    int to, itemm;
    s16b m_idx;

    store_type *st_ptr = &town[p_ptr->town_num].store[BORG_HOME];

    bool *okay;
#ifdef BORG_TK
    char o_name[O_NAME_MAX];
#else
    char o_name[80];
#endif
    /* Allocate the "okay" array */
    C_MAKE(okay, max_a_idx, bool);

    /* Hack -- drop permissions */
    safe_setuid_drop();

    /* Process the player name */
    for (i = 0; player_name[i]; i++)
    {
        char c = player_name[i];

        /* No control characters */
        if (iscntrl(c))
        {
            /* Illegal characters */
            quit_fmt("Illegal control char (0x%02X) in player name", c);
        }

        /* Convert all non-alphanumeric symbols */
        if (!isalpha(c) && !isdigit(c)) c = '_';

        /* Build "file_name" */
        buf[i] = c;
    }


    /* Terminate */
    buf[i++] = '.';
    buf[i++] = 'm';
    buf[i++] = 'a';
    buf[i++] = 'p';
    buf[i++] = '\0';
#ifdef BORG_TK
    path_build(buf2, 1024, BORG_DIR_DATA, buf);
#else /* BORG_TK */
    path_build(buf2, 1024, ANGBAND_DIR_USER, buf);
#endif /* BORG_TK */

    /* XXX XXX XXX Get the name and open the map file */
    if (ask && get_string("Borg map File: ", buf2, 70))
    {
        /* Open a new file */
        borg_map_file = my_fopen(buf2, "w");

        /* Failure */
        if (!borg_map_file) msg_print("Cannot open that file.");
    }
    else if (!ask) borg_map_file = my_fopen(buf2, "w");


    /* Hack -- grab permissions */
    safe_setuid_grab();

   fprintf(borg_map_file, "%s the %s %s, Level %d/%d\n", player_name,
           race_info[p_ptr->prace].title,
           class_info[p_ptr->pclass].title,
           p_ptr->lev, p_ptr->max_plv);

   fprintf(borg_map_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)total_points(), (long)p_ptr->au, (long)turn);
   fprintf(borg_map_file, "Killed on level: %d (max. %d) by %s\n", dun_level, p_ptr->max_dlv, died_from);
   fprintf(borg_map_file, "Zborg Compile Date: %s\n", borg_engine_date);


    for (i = 0; i < MAX_HGT; i++)
    {
        for (j = 0; j < MAX_WID; j++)
        {
            char ch;

		    cave_type   *c_ptr = &cave[i][j];

            borg_grid *ag= &borg_grids[i][j];

            m_idx = c_ptr->m_idx;

            /* reset the ch each time through */
            ch = ' ';

            /* Known grids */
            if (ag->feat)
            {
                ch = f_info[ag->feat].d_char;
            }

            /* Known Items */
            if (ag->take)
            {
                borg_take *take = &borg_takes[ag->take];
                object_kind *k_ptr = &k_info[take->k_idx];
                ch = k_ptr->d_char;
            }

            /* UnKnown Monsters */
            if (m_idx)
            {
                ch = '&';
            }

            /* Known Monsters */
            if (ag->kill)
            {
                borg_kill *kill = &borg_kills[ag->kill];
                monster_race *r_ptr = &r_info[kill->r_idx];
                ch = r_ptr->d_char;
            }


            /* The Player */
            if ((i == c_y) && (j == c_x)) ch = '@';

            line[j] = ch;
        }
        /* terminate the line */
        line[j++] = '\0';

        fprintf(borg_map_file, "%s\n", line);
    }



    /* Known/Seen monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Note */
        fprintf(borg_map_file,"monster '%s' (%d) at (%d,%d) speed %d \n",
                         (r_name + r_info[kill->r_idx].name), kill->r_idx,
                         kill->y, kill->x, kill->speed);
    }


    /*** Dump the last few messages ***/
    i = message_num();
    if (i > 250) i = 250;
    fprintf(borg_map_file, "\n\n  [Last Messages]\n\n");
    while (i-- >0)
    {
        cptr msg  = message_str((s16b)i);

        /* Eliminate some lines */
        if (prefix(msg, "# Matched")
        ||  prefix(msg, "# There is")
        ||  prefix(msg, "# Tracking")
        ||  prefix(msg, "# MISS_BY:")
        ||  prefix(msg, "# HIT_BY:")
        ||  prefix(msg, "> "))
            continue;

        fprintf(borg_map_file, "%s\n", msg);
    }


    /*** Player Equipment ***/
    fprintf(borg_map_file, "\n\n  [Character Equipment]\n\n");
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        object_desc(o_name, &inventory[i], TRUE, 3);
        fprintf(borg_map_file, "%c) %s\n",
                index_to_label(i), o_name);
    }

    fprintf(borg_map_file, "\n\n");



    /* Dump the inventory */
    fprintf(borg_map_file, "  [Character Inventory]\n\n");
    for (i = 0; i < INVEN_PACK; i++)
    {
        object_desc(o_name, &inventory[i], TRUE, 3);
        fprintf(borg_map_file, "%c) %s\n",
                index_to_label(i), o_name);
    }
    fprintf(borg_map_file, "\n\n");



    /* Dump the Home (page 1) */
    fprintf(borg_map_file, "  [Home Inventory (page 1)]\n\n");
    if (borg_skill[BI_CLEVEL] > 35)
	{
		for (i = 0; i < 12; i++)
		{
			object_desc(o_name, &st_ptr->stock[i], TRUE, 3);
			fprintf(borg_map_file, "%c) %s\n", I2A(i%12), o_name);
		}
    }
    fprintf(borg_map_file, "\n\n");


    /* Dump the Home (page 2) */
    fprintf(borg_map_file, "  [Home Inventory (page 2)]\n\n");
	if (borg_skill[BI_CLEVEL] > 35)
	{
		for (i = 12; i < 24; i++)
		{
			object_desc(o_name, &st_ptr->stock[i], TRUE, 3);
			fprintf(borg_map_file, "%c) %s\n", I2A(i%12), o_name);
		}
    }
    fprintf(borg_map_file, "\n\n");


    /* Write swap info */
    if (borg_uses_swaps)
    {
        fprintf(borg_map_file, "  [Swap info]\n\n");
        item = &borg_items[weapon_swap];
        fprintf(borg_map_file,"Swap Weapon:  %s\n", item->desc);
        item = &borg_items[armour_swap];
        fprintf(borg_map_file,"Swap Armour:  %s", item->desc);
        fprintf(borg_map_file, "\n\n");
    }
    fprintf(borg_map_file, "   [Player State at Death] \n\n");


    /* Dump the player state */
    fprintf(borg_map_file,  format("Current speed: %d. \n", borg_skill[BI_SPEED]));

    if (p_ptr->blind)
    {
        fprintf(borg_map_file,  "You cannot see.\n");
    }
    if (p_ptr->confused)
    {
        fprintf(borg_map_file,  "You are confused.\n");
    }
    if (p_ptr->afraid)
    {
        fprintf(borg_map_file,  "You are terrified.\n");
    }
    if (p_ptr->cut)
    {
        fprintf(borg_map_file,  "You are bleeding.\n");
    }
    if (p_ptr->stun)
    {
        fprintf(borg_map_file,  "You are stunned.\n");
    }
    if (p_ptr->poisoned)
    {
        fprintf(borg_map_file,  "You are poisoned.\n");
    }
    if (p_ptr->image)
    {
        fprintf(borg_map_file,  "You are hallucinating.\n");
    }
    if (p_ptr->aggravate)
    {
        fprintf(borg_map_file,  "You aggravate monsters.\n");
    }
    if (p_ptr->blessed)
    {
        fprintf(borg_map_file,  "You feel rightous.\n");
    }
    if (p_ptr->hero)
    {
        fprintf(borg_map_file,  "You feel heroic.\n");
    }
    if (p_ptr->shero)
    {
        fprintf(borg_map_file,  "You are in a battle rage.\n");
    }
    if (p_ptr->protevil)
    {
        fprintf(borg_map_file,  "You are protected from evil.\n");
    }
    if (p_ptr->shield)
    {
        fprintf(borg_map_file,  "You are protected by a mystic shield.\n");
    }
    if (p_ptr->invuln)
    {
        fprintf(borg_map_file,  "You are temporarily invulnerable.\n");
    }
    if (p_ptr->confusing)
    {
        fprintf(borg_map_file,  "Your hands are glowing dull red.\n");
    }
    if (p_ptr->word_recall)
    {
        fprintf(borg_map_file,  format("You will soon be recalled.  (%d turns)\n", p_ptr->word_recall));
    }
    if (p_ptr->oppose_fire)
    {
        fprintf(borg_map_file,  format("You resist fire exceptionally well.\n"));
    }
    if (p_ptr->oppose_acid)
    {
        fprintf(borg_map_file,  format("You resist acid exceptionally well.\n"));
    }
    if (p_ptr->oppose_elec)
    {
        fprintf(borg_map_file,  format("You resist elec exceptionally well.\n"));
    }
    if (p_ptr->oppose_cold)
    {
        fprintf(borg_map_file,  format("You resist cold exceptionally well.\n"));
    }
    if (p_ptr->oppose_pois)
    {
        fprintf(borg_map_file,  format("You resist poison exceptionally well.\n"));
    }

	/* Dump the Time Variables */
    fprintf(borg_map_file, "Time on this panel; %d\n", time_this_panel);
    fprintf(borg_map_file, "Time on this level; %d\n", borg_t - borg_began);
    fprintf(borg_map_file, "Time since left town; %d\n", borg_time_town + (borg_t - borg_began));
    fprintf(borg_map_file, "Food in town; %d\n", borg_food_onsale);
    fprintf(borg_map_file, "Borg_no_retreat; %d\n", borg_no_retreat);
    fprintf(borg_map_file, "Breeder_level; %d\n", (borg_depth & DEPTH_BREEDER));
    fprintf(borg_map_file, "Unique_on_level; %d\n", unique_on_level);
    if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))
		fprintf(borg_map_file, "It is daytime in town.\n");
    else fprintf(borg_map_file, "It is night-time in town.\n");
    fprintf(borg_map_file, "\n\n");
    fprintf(borg_map_file, "\n\n");

    /* Dump the spells */
    if (borg_skill[BI_REALM1])
    {
        fprintf(borg_map_file,"\n\n   [ Realm 1 ] \n\n");
        fprintf(borg_map_file,"Name                           Legal Times cast\n");
        for (i = 0; i < 4; i++ )
        {
            for (j = 0; j < 8; j++)
            {
                borg_magic *as = &borg_magics[borg_skill[BI_REALM1]][i][j];
                cptr legal;

                if (as->level <99)
                {
                    legal = (borg_spell_legal(borg_skill[BI_REALM1], i, j) ? "Yes" : "No ");
                    fprintf(borg_map_file,"%-30s   %s   %d\n",as->name, legal, (long)as->times);
                }
            }
            fprintf(borg_map_file,"\n");
        }
	}

    /* Dump the spells */
    if (borg_skill[BI_REALM2])
    {
        fprintf(borg_map_file,"\n\n   [ Realm 2 ] \n\n");
        fprintf(borg_map_file,"Name                           Legal Times cast\n");
        for (i = 0; i < 4; i++ )
        {
            for (j = 0; j < 8; j++)
            {
                borg_magic *as = &borg_magics[borg_skill[BI_REALM2]][i][j];
                cptr legal;

                if (as->level <99)
                {
                    legal = (borg_spell_legal(borg_skill[BI_REALM2], i, j) ? "Yes" : "No ");
                    fprintf(borg_map_file,"%-30s   %s   %d\n",as->name, legal, (long)as->times);
                }
            }
            fprintf(borg_map_file,"\n");
        }

    }

	/* Dump the borg_skill[] information */
    itemm = max_k_idx + max_k_idx + max_a_idx;
    to = max_k_idx + max_k_idx + max_a_idx + BI_MAX;
    for (;itemm < to; itemm++)
    {
         fprintf(borg_map_file,"skill %d (%s) value= %d.\n",itemm,
                      prefix_pref[itemm -
                      max_k_idx -
                      max_k_idx -
                      max_a_idx], borg_has[itemm]);
	}

#if 0
    /*** Dump the Uniques and Artifact Lists ***/

    /* Scan the artifacts */
    for (k = 0; k < max_a_idx; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* Default */
        okay[k] = FALSE;

        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Skip "uncreated" artifacts */
        if (!a_ptr->cur_num) continue;

        /* Assume okay */
        okay[k] = TRUE;
    }

    /* Check the dungeon */
    for (y = 0; y < MAX_HGT; y++)
    {
        for (x = 0; x < MAX_WID; x++)
        {
            s16b this_o_idx, next_o_idx = 0;

            /* Scan all objects in the grid */
            for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
            {
                object_type *o_ptr;

                /* Get the object */
                o_ptr = &o_list[this_o_idx];

                /* Get the next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Ignore non-artifacts */
                if (!artifact_p(o_ptr)) continue;

                /* Ignore known items */
                if (object_known_p(o_ptr)) continue;

                /* Note the artifact */
                okay[o_ptr->name1] = FALSE;
            }
        }
    }

    /* Check the inventory and equipment */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_type *o_ptr = &inventory[i];

        /* Ignore non-objects */
        if (!o_ptr->k_idx) continue;

        /* Ignore non-artifacts */
        if (!artifact_p(o_ptr)) continue;

        /* Ignore known items */
        if (object_known_p(o_ptr)) continue;

        /* Note the artifact */
        okay[o_ptr->name1] = FALSE;
    }

    fprintf(borg_map_file, "\n\n");


    /* Hack -- Build the artifact name */
    fprintf(borg_map_file, "   [Artifact Info] \n\n");

    /* Scan the artifacts */
    for (k = 0; k < max_a_idx; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* List "dead" ones */
        if (!okay[k]) continue;

        /* Paranoia */
        strcpy(o_name, "Unknown Artifact");

        /* Obtain the base object type */
        z = lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Real object */
        if (z)
        {
            object_type *i_ptr;
            object_type object_type_body;

            /* Get local object */
            i_ptr = &object_type_body;

            /* Create fake object */
            object_prep(i_ptr, z);

            /* Make it an artifact */
            i_ptr->name1 = k;

            /* Describe the artifact */
            object_desc_store(o_name, i_ptr, FALSE, 0);
        }

        /* Hack -- Build the artifact name */
        fprintf(borg_map_file, "The %s\n", o_name);
    }

    /* Free the "okay" array */
    C_KILL(okay, max_a_idx, bool);
    fprintf(borg_map_file, "\n\n");

 /* Display known uniques
  *
  * Note that the player ghosts are ignored.  XXX XXX XXX
  */
    /* Allocate the "who" array */
    C_MAKE(who, max_r_idx, u16b);

    /* Collect matching monsters */
    for (i = 1, n = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        monster_lore *l_ptr = &l_list[i];

        /* Require known monsters */
        if (!cheat_know && !l_ptr->r_sights) continue;

        /* Require unique monsters */
        if (!(r_ptr->flags1 & (RF1_UNIQUE))) continue;

        /* Collect "appropriate" monsters */
        who[n++] = i;
    }

    /* Select the sort method */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort the array by dungeon depth of monsters */
    ang_sort(who, &why, n);


    /* Hack -- Build the artifact name */
    fprintf(borg_map_file, "   [Unique Info] \n\n");

    /* Print the monsters */
    for (i = 0; i < n; i++)
    {
        monster_race *r_ptr = &r_info[who[i]];
        bool dead = (r_ptr->max_num == 0);

        /* Print a message */
        fprintf(borg_map_file, "%s is %s\n",
                (r_name + r_ptr->name),
                (dead ? "dead" : "alive"));
    }

    /* Free the "who" array */
    C_KILL(who, max_r_idx, u16b);

#endif


    my_fclose(borg_map_file);

}

#endif /* BABLOS */

/* DVE's function for displaying the status of various info */
/* Display what the borg is thinking DvE*/
void borg_status(void)
{
#ifndef BORG_TK
 int j;

 quest_type      *q_ptr;
 monster_race    *r_ptr;
 int             q_num;
 char            name[80];


	/* For displaying information on random quests */
	if (borg_depth & DEPTH_QUEST)
	{
		q_ptr = &quest[quest_number(borg_skill[BI_CDEPTH])];
	 	r_ptr = &r_info[q_ptr->r_idx];
	    q_num = q_ptr->max_num;
		strcpy(name, (r_name + r_ptr->name));
	}

 /* Scan windows */
 for (j = 0; j < 8; j++)
 {
  term *old = Term;

  /* Unused */
  if (!angband_term[j]) continue;

  /* Check for borg status term */
  if (window_flag[j] & (PW_BORG_2))
  {
   byte attr;

   /* Activate */
   Term_activate(angband_term[j]);

   /* Display what resists the borg (thinks he) has */
   Term_putstr(5, 0, -1,TERM_WHITE, "RESISTS");

   /* Basic four */
   attr = TERM_SLATE;
   if (borg_skill[BI_RACID]) attr = TERM_BLUE;
   if (borg_skill[BI_TRACID]) attr = TERM_GREEN;
   if (borg_skill[BI_IACID]) attr = TERM_WHITE;
   Term_putstr(1, 1, -1, attr, "Acid");

   attr = TERM_SLATE;
   if (borg_skill[BI_RELEC]) attr = TERM_BLUE;
   if (borg_skill[BI_TRELEC]) attr = TERM_GREEN;
   if (borg_skill[BI_IELEC]) attr = TERM_WHITE;
   Term_putstr(1, 2, -1, attr, "Elec");

   attr = TERM_SLATE;
   if (borg_skill[BI_RFIRE]) attr = TERM_BLUE;
   if (borg_skill[BI_TRFIRE]) attr = TERM_GREEN;
   if (borg_skill[BI_IFIRE]) attr = TERM_WHITE;
   Term_putstr(1, 3, -1, attr, "Fire");

   attr = TERM_SLATE;
   if (borg_skill[BI_RCOLD]) attr = TERM_BLUE;
   if (borg_skill[BI_TRCOLD]) attr = TERM_GREEN;
   if (borg_skill[BI_ICOLD]) attr = TERM_WHITE;
   Term_putstr(1, 4, -1, attr, "Cold");

   /* High resists */
   attr = TERM_SLATE;
   if (borg_skill[BI_RPOIS]) attr = TERM_BLUE;
   if (borg_skill[BI_TRPOIS]) attr = TERM_GREEN;
   Term_putstr(1, 5, -1, attr, "Pois");

   if (borg_skill[BI_RFEAR]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 6, -1, attr, "Fear");

   if (borg_skill[BI_RLITE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 7, -1, attr, "Lite");

   if (borg_skill[BI_RDARK]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 8, -1, attr, "Dark");

   if (borg_skill[BI_RBLIND]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 1, -1, attr, "Blind");

   if (borg_skill[BI_RCONF]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 2, -1, attr, "Confu");

   if (borg_skill[BI_RSND]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 3, -1, attr, "Sound");

   if (borg_skill[BI_RSHRD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 4, -1, attr, "Shard");

   if (borg_skill[BI_RNXUS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 5, -1, attr, "Nexus");

   if (borg_skill[BI_RNTHR]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 6, -1, attr, "Nethr");

   if (borg_skill[BI_RKAOS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 7, -1, attr, "Chaos");

   if (borg_skill[BI_RDIS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 8, -1, attr, "Disen");

   /* Other abilities */
   if (borg_skill[BI_SDIG]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 1, -1, attr, "S.Dig");

   if (borg_skill[BI_FEATH]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 2, -1, attr, "Feath");

   if (borg_skill[BI_LITE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 3, -1, attr, "PLite");

   if (borg_skill[BI_REG]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 4, -1, attr, "Regen");

   if (borg_skill[BI_ESP]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 5, -1, attr, "Telep");

   if (borg_skill[BI_SINV]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 6, -1, attr, "Invis");

   if (borg_skill[BI_FRACT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 7, -1, attr, "FrAct");

   if (borg_skill[BI_HLIFE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 8, -1, attr, "HLife");

   /* Display the slays */
   Term_putstr(5, 10, -1,TERM_WHITE, "Weapon Slays:");

   if (borg_skill[BI_WS_ANIMAL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 11, -1, attr, "Animal");

   if (borg_skill[BI_WS_EVIL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 11, -1, attr, "Evil");

   if (borg_skill[BI_WS_UNDEAD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 11, -1, attr, "Undead");

   if (borg_skill[BI_WS_DEMON]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(22, 11, -1, attr, "Demon");

   if (borg_skill[BI_WS_ORC]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 12, -1, attr, "Orc");

   if (borg_skill[BI_WS_TROLL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 12, -1, attr, "Troll");

   if (borg_skill[BI_WS_GIANT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 12, -1, attr, "Giant");

   if (borg_skill[BI_WS_DRAGON]) attr = TERM_BLUE;
   if (borg_skill[BI_WK_DRAGON]) attr = TERM_GREEN;
   else attr = TERM_SLATE;
   Term_putstr(22, 12, -1, attr, "Dragon");

   if (borg_skill[BI_WB_ACID]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 13, -1, attr, "Acid");

   if (borg_skill[BI_WB_COLD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 13, -1, attr, "Cold");

   if (borg_skill[BI_WB_ELEC]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 13, -1, attr, "Elec");

   if (borg_skill[BI_WB_FIRE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(22, 13, -1, attr, "Fire");


   /* Display the Concerns */
   Term_putstr(36, 10, -1,TERM_WHITE, "Concerns:");

   if (borg_wearing_cursed) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 11, -1, attr, "Cursed");

   if (borg_skill[BI_ISWEAK]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(36, 11, -1, attr, "Weak");

   if (borg_skill[BI_ISPOISONED]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 11, -1, attr, "Poison");

   if (borg_skill[BI_ISCUT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 12, -1, attr, "Cut");

   if (borg_skill[BI_ISSTUN]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(36, 12, -1, attr, "Stun");

   if (borg_skill[BI_ISCONFUSED]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 12, -1, attr, "Confused");

   if (borg_skill[BI_ENCUMBERD] > 0) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 13, -1, attr, "Encumbered");

   if (borg_skill[BI_ISFIXEXP]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 13, -1, attr, "Exp Drain");

   /* Display the Time */
   Term_putstr(60, 10, -1,TERM_WHITE, "Time:");

   Term_putstr(54, 11, -1, TERM_SLATE, "This Level         ");
   Term_putstr(65, 11, -1, TERM_WHITE, format("%d",borg_t - borg_began));

   Term_putstr(54, 12, -1, TERM_SLATE, "Since Town         ");
   Term_putstr(65, 12, -1, TERM_WHITE, format("%d",borg_time_town + (borg_t - borg_began)));

   Term_putstr(54, 13, -1, TERM_SLATE, "This Panel         ");
   Term_putstr(65, 13, -1, TERM_WHITE, format("%d",time_this_panel));


   /* Sustains */
   Term_putstr(19, 0, -1, TERM_WHITE, "Sustains");

   if (borg_skill[BI_SSTR]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 1, -1, attr, "STR");

   if (borg_skill[BI_SINT]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 2, -1, attr, "INT");

   if (borg_skill[BI_SWIS]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 3, -1, attr, "WIS");

   if (borg_skill[BI_SDEX]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 4, -1, attr, "DEX");

   if (borg_skill[BI_SCON]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 5, -1, attr, "CON");

   if (borg_skill[BI_SCHR]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 6, -1, attr, "CHR");


   /* Temporary effects */
   Term_putstr(28, 0, -1, TERM_WHITE, "Temp Effects");

   if (borg_prot_from_evil) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 1, -1, attr, "Prot. Evil");

   if (borg_goi || borg_wraith) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 2, -1, attr, "Invulnerable");

   if (borg_hero) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 3, -1, attr, "Heroism");

   if (borg_berserk) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 4, -1, attr, "Berserk");

   if (borg_shield) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 5, -1, attr, "Shielded");

   if (borg_bless) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 6, -1, attr, "Blessed");

    if (borg_speed) attr = TERM_WHITE;
    else attr = TERM_SLATE;
    Term_putstr(28, 7, -1, attr, "Fast");

    if (borg_inviso) attr = TERM_WHITE;
    else attr = TERM_SLATE;
    Term_putstr(28, 8, -1, attr, "Invisible");

   /* Temporary effects */
   Term_putstr(42, 0, -1, TERM_WHITE, "Level Information");

   if ((borg_depth & DEPTH_VAULT)) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 1, -1, attr, "Vault on level");

   if (unique_on_level) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 2, -1, attr, "Unique on level");
   if (unique_on_level) Term_putstr(58, 2, -1, attr, format("(%s)",
                       r_name + r_info[unique_on_level].name));
   else Term_putstr(58, 2, -1, attr, "                                   ");

   if (borg_depth & DEPTH_QUEST) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 3, -1, attr, "Quest Level");
   if (borg_depth & DEPTH_QUEST) Term_putstr(55, 3, -1, attr, format("%s (%d)",name, q_num));
   else Term_putstr(55, 3, -1, attr, "                                   ");

   if ((borg_depth & DEPTH_BREEDER)) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 4, -1, attr,"Breeder level (close the door, will ye)");

   if (borg_kills_summoner != -1) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 5, -1, attr,"Summoner very close (AS-Corridor appropriate)");

    /* level preparedness */
   attr = TERM_SLATE;
   Term_putstr(42, 6, -1, attr,"Reason for not diving:");
   attr = TERM_WHITE;
   Term_putstr(64, 6, -1, attr,format("%s                              ",
   			 borg_prepared[borg_skill[BI_MAXDEPTH]+1]));

   if (goal_fleeing) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 7, -1, attr,"Fleeing Level");

   attr = TERM_SLATE;
   Term_putstr(42, 8, -1, attr,"Maximal Depth:");
   attr = TERM_WHITE;
   Term_putstr(56, 8, -1, attr,format("%d    ", borg_skill[BI_MAXDEPTH]));


	/* Important endgame information */
    Term_putstr(5, 15, -1,TERM_WHITE, "Important Home Inven:");

    attr = TERM_SLATE;
    Term_putstr(1, 16, -1, attr, "EZ_Heal:        ");
    attr = TERM_WHITE;
    Term_putstr(10, 16, -1, attr, format("%d   ",num_ez_heal));

    attr = TERM_SLATE;
    Term_putstr(1, 17, -1, attr, "Num_Heal:        ");
    attr = TERM_WHITE;
    Term_putstr(11, 17, -1, attr, format("%d   ",num_heal));

    attr = TERM_SLATE;
    Term_putstr(1, 18, -1, attr, "Res_Mana:        ");
    attr = TERM_WHITE;
    Term_putstr(11, 18, -1, attr, format("%d   ",num_mana));
	attr = TERM_SLATE;

	Term_putstr(1, 19, -1, attr, "Res_Mana:        ");
    attr = TERM_WHITE;
    Term_putstr(11, 19, -1, attr, format("%d   ",num_mana));

	if (borg_depth & DEPTH_BORER) attr = TERM_BLUE;
	else attr = TERM_SLATE;
	Term_putstr(1, 20, -1, attr, format("Borer on Level.  Last seen:%d       ", borg_t - borg_t_questor));
	if (borg_depth & (DEPTH_BORER | DEPTH_SUMMONER))  attr = TERM_BLUE;
	else attr = TERM_SLATE;
	if (borg_skill[BI_CDEPTH] == 100) Term_putstr(1, 20, -1, attr, format("Bor/Sum on Level.  Last seen:%d       ", borg_t - borg_t_questor));

    if ((borg_position & POSITION_SEA))  attr = TERM_BLUE;
    else attr = TERM_SLATE;
    if (borg_needs_new_sea) attr = TERM_WHITE;
    Term_putstr(1, 21, -1, attr, "Sea of Runes.");

    if (borg_ready_morgoth)  attr = TERM_BLUE;
    else attr = TERM_SLATE;
    Term_putstr(1, 22, -1, attr, "Ready for Morgoth.");



   /* Fresh */
   Term_fresh();

   /* Restore */
   Term_activate(old);
  }
 }
#endif /* borg_tk */
}


/* keep a log of certain battles, used on Questor uniques */
void borg_log_battle(bool keep_log)
{
     char buf[80];

/* for now the log is deactivated */
return;

    /* Close the log file */
    if (!keep_log)
    {
        borg_info("Closing Battle Log");
        borg_info("**************");
        my_fclose(borg_fff);
        borg_fff=NULL; /* needed on unix! */
        return;
    }

    /* Hack -- drop permissions */
    safe_setuid_drop();

    /* Default  */
    strcpy(buf, "battle.log");

    /* Open a new file */
    borg_fff = my_fopen(buf, "a");
    /* Failure */
    if (!borg_fff) msg_print("Cannot open that file.");

    /* Note start of log */
    borg_info("**************");
    borg_info("Opening Battle Log");

    /* Hack -- grab permissions */
    safe_setuid_grab();
    return;
}


/*
 * Hack -- forward declare
 */
void do_cmd_borg(void);


/*
 * Hack -- interact with the "Ben Borg".
 */
void do_cmd_borg(void)
{
    char cmd;


#ifdef BABLOS

    if (auto_play)
    {
        auto_play = FALSE;
        keep_playing = TRUE;
        cmd = 'z';
    }
    else
    {

#endif /* BABLOS */

    /* Get a "Borg command", or abort */
    if (!get_com("Borg command: ", &cmd)) return;

#ifdef BABLOS

    }

#endif /* BABLOS */
#ifndef BORG_TK
    /* Simple help */
    if (cmd == '?')
    {
        int i = 2;

        /* Save the screen */
        Term_save();

        /* Clear the screen */
        Term_clear();

        i++;
        Term_putstr(2, i, -1, TERM_WHITE, "Command 'z' activates the Borg.");
        Term_putstr(42, i++, -1, TERM_WHITE, "Command 'u' updates the Borg.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'x' steps the Borg.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'f' modifies the normal flags.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'c' modifies the cheat flags.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'l' activates a log file.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 's' activates search mode.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'i' displays grid info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'g' displays grid feature.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'a' displays avoidances.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'k' displays monster info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 't' displays object info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '%' displays current flow.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '#' displays danger grid.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '_' Regional Fear info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'p' Borg Power.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '1' change max depth.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '2' level prep info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '3' Feature of grid.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '!' Time.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '@' Borg LOS.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'w' My Swap Weapon.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'q' Auto stop on level.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'v' Version stamp.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'd' Dump spell info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'h' Borg_Has function.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '$' Reload Borg.txt.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'y' Last 75 steps.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '^' Flow Pathway.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'o' Examine Inven Item.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'R' Respawn Borg.");


        /* Prompt for key */
        msg_print("Commands: ");
        msg_print(NULL);

        /* Restore the screen */
        Term_load();

        /* Done */
        return;
    }

#endif /* not BORG_TK */

    /* Hack -- force initialization */
    if (!initialized) borg_init_9();

    switch (cmd)
    {
        /* Command: Nothing */
        case '$':
        {
            int i;
            /*** Hack -- initialize borg.ini options ***/

            /* Message */
            borg_note("Reloading the Borg rules... (zborg.txt)");
            for (i = 0; i < MAX_CLASS; i++)
            {
                C_KILL(borg_required_item[i], 400, req_item); /* externalize the 400 later */
                C_KILL(borg_power_item[i], 400, power_item); /* externalize the 400 later */
            }
            C_KILL(borg_has, size_obj, int);
            for (i = 0; i < 1000; i++)
            {
                if (formula[i])
                    C_KILL(formula[i], MAX_FORMULA_ELEMENTS, int);
            }
            init_borg_txt_file();
            borg_note("# Ready...");
            break;
        }
        /* Command: Activate */
        case 'z':
        case 'Z':
        {
            /* Activate */
            borg_active = TRUE;

            /* Reset cancel */
            borg_cancel = FALSE;

            /* Step forever */
            borg_step = 0;
            /* need to check all stats */
            my_need_stat_check[0] = TRUE;
            my_need_stat_check[1] = TRUE;
            my_need_stat_check[2] = TRUE;
            my_need_stat_check[3] = TRUE;
            my_need_stat_check[4] = TRUE;
            my_need_stat_check[5] = TRUE;

            /* Allowable Cheat -- Obtain "recall" flag */
            goal_recalling = p_ptr->word_recall * 1000;

            /* Allowable Cheat -- Obtain "prot_from_evil" flag */
            borg_prot_from_evil = (p_ptr->protevil ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "speed" flag */
            borg_speed = (p_ptr->fast ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "goi" flag */
            borg_goi = (p_ptr->invuln ? 9000 : 0);
            borg_inviso = (p_ptr->tim_invis ? 9000 : 0);
		    borg_wraith = (p_ptr->wraith_form ? 25000 : 0);


            /* Allowable Cheat -- Obtain "resist" flags */
            borg_skill[BI_TRACID] = (p_ptr->oppose_acid ? TRUE : FALSE);
            borg_skill[BI_TRELEC] = (p_ptr->oppose_elec ? TRUE : FALSE);
            borg_skill[BI_TRFIRE] = (p_ptr->oppose_fire ? TRUE : FALSE);
            borg_skill[BI_TRCOLD] = (p_ptr->oppose_cold ? TRUE : FALSE);
            borg_skill[BI_TRPOIS] = (p_ptr->oppose_pois ? TRUE : FALSE);
            borg_bless = (p_ptr->blessed ? TRUE : FALSE);
            borg_shield = (p_ptr->shield ? TRUE : FALSE);
            borg_hero = (p_ptr->hero ? TRUE : FALSE);
            borg_berserk = (p_ptr->shero ? TRUE : FALSE);

            /* Message */
            borg_note("# Installing keypress hook");

            /* If the clock overflowed, fix that  */
            if (borg_t > 9000)
                borg_t = 9000;

            /* Activate the key stealer */
            inkey_hack = borg_inkey_hack;
#ifdef BORG_TK
            angtk_eval("angband_borg", "activate", NULL);
#endif /* BORG_TK */

            break;
        }

        /* Command: Update */
        case 'u':
        case 'U':
        {
            /* Activate */
            borg_active = TRUE;
			borg_do_update_view = TRUE;
			borg_do_update_lite = TRUE;

            /* Immediate cancel */
            borg_cancel = TRUE;

            /* Step forever */
            borg_step = 0;

            /* Allowable Cheat -- Obtain "recall" flag */
            goal_recalling = p_ptr->word_recall * 1000;

            /* Allowable Cheat -- Obtain "prot_from_evil" flag */
            borg_prot_from_evil = (p_ptr->protevil ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "speed" flag */
            borg_speed = (p_ptr->fast ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "goi" flag */
            borg_goi = (p_ptr->invuln ? 9000 : 0);
		    borg_wraith = (p_ptr->wraith_form ? 25000 : 0);
            borg_inviso = (p_ptr->tim_invis ? 9000 : 0);
            /* Allowable Cheat -- Obtain "resist" flags */
            borg_skill[BI_TRACID] = (p_ptr->oppose_acid ? TRUE : FALSE);
            borg_skill[BI_TRELEC] = (p_ptr->oppose_elec ? TRUE : FALSE);
            borg_skill[BI_TRFIRE] = (p_ptr->oppose_fire ? TRUE : FALSE);
            borg_skill[BI_TRCOLD] = (p_ptr->oppose_cold ? TRUE : FALSE);
            borg_skill[BI_TRPOIS] = (p_ptr->oppose_pois ? TRUE : FALSE);
            borg_bless = (p_ptr->blessed ? TRUE : FALSE);
            borg_shield = (p_ptr->shield ? TRUE : FALSE);
            borg_hero = (p_ptr->hero ? TRUE : FALSE);
            borg_berserk = (p_ptr->shero ? TRUE : FALSE);

            /* Message */
            borg_note("# Installing keypress hook");
#ifdef BORG_TK
            angtk_eval("angband_borg", "activate", NULL);
#endif /* BORG_TK */

            /* Activate the key stealer */
            inkey_hack = borg_inkey_hack;

            break;
        }


        /* Command: Step */
        case 'x':
        case 'X':
        {
            /* Activate */
            borg_active = TRUE;

            /* Reset cancel */
            borg_cancel = FALSE;

            /* Step N times */
            borg_step = 1;

            /* need to check all stats */
            my_need_stat_check[0] = TRUE;
            my_need_stat_check[1] = TRUE;
            my_need_stat_check[2] = TRUE;
            my_need_stat_check[3] = TRUE;
            my_need_stat_check[4] = TRUE;
            my_need_stat_check[5] = TRUE;

            /* Allowable Cheat -- Obtain "recall" flag */
            goal_recalling = p_ptr->word_recall * 1000;
            /* Allowable Cheat -- Obtain "prot_from_evil" flag */
            borg_prot_from_evil = (p_ptr->protevil ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "speed" flag */
            borg_speed = (p_ptr->fast ? TRUE : FALSE);
            /* Allowable Cheat -- Obtain "goi" flag */
            borg_goi = (p_ptr->invuln ? 9000 : 0);
		    borg_wraith = (p_ptr->wraith_form ? 25000 : 0);
            borg_inviso = (p_ptr->tim_invis ? 9000 : 0);
            /* Allowable Cheat -- Obtain "resist" flags */
            borg_skill[BI_TRACID] = (p_ptr->oppose_acid ? TRUE : FALSE);
            borg_skill[BI_TRELEC] = (p_ptr->oppose_elec ? TRUE : FALSE);
            borg_skill[BI_TRFIRE] = (p_ptr->oppose_fire ? TRUE : FALSE);
            borg_skill[BI_TRCOLD] = (p_ptr->oppose_cold ? TRUE : FALSE);
            borg_skill[BI_TRPOIS] = (p_ptr->oppose_pois ? TRUE : FALSE);
            borg_bless = (p_ptr->blessed ? TRUE : FALSE);
            borg_shield = (p_ptr->shield ? TRUE : FALSE);
            borg_hero = (p_ptr->hero ? TRUE : FALSE);
            borg_berserk = (p_ptr->shero ? TRUE : FALSE);

            /* Message */
            borg_note("# Installing keypress hook");

            /* If the clock overflowed, fix that  */
            if (borg_t > 9000)
                borg_t = 9000;

            /* Activate the key stealer */
            inkey_hack = borg_inkey_hack;
#ifdef BORG_TK
            angtk_eval("angband_borg", "activate", NULL);
#endif /* BORG_TK */

            break;
        }

        /* Command: toggle "flags" */
        case 'f':
        case 'F':
        {
            /* Get a "Borg command", or abort */
            if (!get_com("Borg command: Toggle Flag: (m/d/s/f/g) ", &cmd)) return;

            switch (cmd)
            {
                /* Give borg thought messages in window */
                case 'm':
                case 'M':
                {
                    break;
                }

                /* Give borg the ability to use graphics ----broken */
                case 'g':
                case 'G':
                {
#if 0
					borg_graphics = !borg_graphics;
                    msg_format("Borg -- borg_graphics is now %d.",
                                borg_graphics);
#endif
                    break;
                }

                /* Dump savefile at each death */
                case 'd':
				case 'D':
                {
                    borg_flag_dump = !borg_flag_dump;
                    msg_format("Borg -- borg_flag_dump is now %d.",
                                borg_flag_dump);
                    break;
                }

                /* Dump savefile at each level */
                case 's':
                case 'S':
                {
                    borg_flag_save = !borg_flag_save;
                    msg_format("Borg -- borg_flag_save is now %d.",
                                borg_flag_save);
                    break;
                }

                /* clear 'fear' levels */
                case 'f':
                case 'F':
                {
                    msg_format("Command No Longer Usefull");
                    break;
                }
            }
            break;
        }



        /* Command: toggle "cheat" flags */
        case 'c':
        {
            /* Get a "Borg command", or abort */
            if (!get_com("Borg command: Toggle Cheat: (d/i/e/s/p)", &cmd))
                return;

            switch (cmd)
            {
                case 'd':
                case 'D':
                {
                    borg_cheat_death = !borg_cheat_death;
                    msg_format("Borg -- borg_cheat_death is now %d.",
                                borg_cheat_death);
                    break;
                }
            }
            break;

        }


        /* Start a new log file */
        case 'l':
        case 'L':
        {
            char buf[80];

            /* Close the log file */
            if (borg_fff) my_fclose(borg_fff);

            /* Hack -- drop permissions */
            safe_setuid_drop();

            /* Default  */
            strcpy(buf, "borg.log");

            /* XXX XXX XXX Get the name and open the log file */
            if (get_string("Borg Log File: ", buf, 70))
            {
                /* Open a new file */
                borg_fff = my_fopen(buf, "w");

                /* Failure */
                if (!borg_fff) msg_print("Cannot open that file.");
            }

            /* Hack -- grab permissions */
            safe_setuid_grab();
            break;
        }


        /* Activate a search string */
        case 's':
        case 'S':
        {
            /* Get the new search string (or cancel the matching) */
            if (!get_string("Borg Match String: ", borg_match, 70))
            {
                /* Cancel it */
                strcpy(borg_match, "");

                /* Message */
                msg_print("Borg Match String de-activated.");
            }
            break;
        }

#ifndef BORG_TK
        /* Command: check Grid "feature" flags */
        case 'g':
        case 'G':
        {
            int x, y;

            u16b low = 0, high = 0;

            /* Get a "Borg command", or abort */
            if (!get_com("Borg command: Show grids: ", &cmd)) return;

            /* Extract a flag */
            switch (cmd)
            {
                case '0': low = high = 1 << 0; break;
                case '1': low = high = 1 << 1; break;
                case '2': low = high = 1 << 2; break;
                case '3': low = high = 1 << 3; break;
                case '4': low = high = 1 << 4; break;
                case '5': low = high = 1 << 5; break;
                case '6': low = high = 1 << 6; break;
                case '7': low = high = 1 << 7; break;

                case '.': low = high = FEAT_FLOOR; break;
                case ' ': low = high = FEAT_NONE; break;
                case 'i': low = high = FEAT_INVIS; break;
                case ';': low = high = FEAT_GLYPH; break;
                case ',': low = high = FEAT_OPEN; break;
                case 'x': low = high = FEAT_BROKEN; break;
                case '<': low = high = FEAT_LESS; break;
                case '>': low = high = FEAT_MORE; break;
                case '@': low = FEAT_SHOP_HEAD;
                          high = FEAT_SHOP_TAIL;
                           break;
                case '^': low = FEAT_TRAP_TRAPDOOR;
                          high = FEAT_TRAP_SLEEP;
                          break;
                case '+': low  = FEAT_DOOR_HEAD;
                          high = FEAT_DOOR_TAIL; break;
                case 's': low = high = FEAT_SECRET; break;
                case ':': low = high = FEAT_RUBBLE; break;
                case 'q': low = FEAT_MAGMA;
                		  high = FEAT_QUARTZ_K;
                case 'm': low = high = FEAT_MOUNTAIN; break;
                case 't': low = high = FEAT_TREES; break;
                case 'l': low = high = FEAT_SHAL_LAVA; break;
                case 'L': low = high = FEAT_DEEP_LAVA; break;
                case 'a': low = high = FEAT_SHAL_WATER; break;
                case 'A': low = high = FEAT_DEEP_WATER; break;

                case 'k':  break;
                case '&':  break;
                case 'w': low = FEAT_WALL_EXTRA;
                          high = FEAT_WALL_SOLID;
                          break;
                case 'p': low = FEAT_PERM_EXTRA;
                          high = FEAT_PERM_SOLID;
                          break;

                default: low = high = 0x00; break;
            }

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
                    byte a = TERM_RED;

                    borg_grid *ag = &borg_grids[y][x];

                    /* show only those grids */
                    if (!(ag->feat >= low && ag->feat <= high)) continue;

                    /* known floor grids */
                    if (borg_cave_floor_bold(y, x)) a = TERM_YELLOW;

                    /* Arrary members*/
                    print_rel('*', TERM_RED, y, x);
                }
            }

            /* Get keypress */
            msg_print("Press any key.");
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

        /* Command: check "info" flags */
        case 'i':
        case 'I':
        {
            int x, y;

            u16b mask;

            /* Get a "Borg command", or abort */
            if (!get_com("Borg command: Show grids: ", &cmd)) return;

            /* Extract a flag */
            switch (cmd)
            {
                case '0': mask = 1 << 0; break;
                case '1': mask = 1 << 1; break;
                case '2': mask = 1 << 2; break;
                case '3': mask = 1 << 3; break;
                case '4': mask = 1 << 4; break;
                case '5': mask = 1 << 5; break;
                case '6': mask = 1 << 6; break;
                case '7': mask = 1 << 7; break;

                case 'm': mask = BORG_MARK; break;
                case 'g': mask = BORG_GLOW; break;
                case 'd': mask = BORG_DARK; break;
                case 'o': mask = BORG_OKAY; break;
                case 'l': mask = BORG_LITE; break;
                case 'v': mask = BORG_VIEW; break;
                case 't': mask = BORG_TEMP; break;
                case 'x': mask = BORG_XTRA; break;

                default: mask = 0x000; break;
            }

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
                    byte a = TERM_RED;

                    borg_grid *ag = &borg_grids[y][x];

                    /* Given mask, show only those grids */
                    if (mask && !(ag->info & mask)) continue;

                    /* Given no mask, show unknown grids */
                    if (!mask && (ag->info & BORG_MARK)) continue;

                    /* Known Floor */
                    if (borg_cave_floor_bold(y, x)) a = TERM_YELLOW;

                    /* Array Member */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg_print("Press any key.");
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

    /* Command: check "avoidances" */
        case 'a':
        case 'A':
        {
            int x, y, p;

            /* Scan map */
            for (y = c_y - MAX_RANGE; y < c_y + MAX_RANGE; y++)
            {
                for (x = c_x - MAX_RANGE; x < c_x + MAX_RANGE; x++)
                {
                    byte a = TERM_DARK;

					/* Bounds */
					if (!in_bounds(y,x)) continue;

					/* Obtain danger */
                    p = borg_danger(y, x, 1, TRUE);

                    /* Skip non-avoidances */
                    if (p < avoidance / 10) continue;

                    /* Scale the pain */
                    if (p > avoidance/8) a = TERM_SLATE;
                    if (p > avoidance/6) a = TERM_L_DARK;
                    if (p > avoidance/4) a = TERM_SLATE;
                    if (p > avoidance/2) a = TERM_L_WHITE;
                    if (p > avoidance) a = TERM_WHITE;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg_format("(%d,%d of %d,%d) Avoidance value %d.", c_y, c_x, panel_row_min / PANEL_HGT,panel_col_min / PANEL_WID, avoidance);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }


    /* Command: check previous steps */
        case 'y':
        {
            int x, y, i;

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
                    byte a = TERM_RED;
                    /* Check for an existing step */
                    for (i = 0; i < track_step_num; i++)
                    {
                        /*  */
                        if ((track_step_x[i] == x) && (track_step_y[i] == y))
                        /* Display */
                        print_rel('*', a, y, x);
                    }
                }
            }

            /* Get keypress */
            msg_format("(%d) Steps noted", track_step_num);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

        /* Command: show "monsters" */
        case 'k':
        case 'K':
        {
            int i, n = 0;

            /* Scan the monsters */
            for (i = 1; i < borg_kills_nxt; i++)
            {
                borg_kill *kill = &borg_kills[i];

                /* Still alive */
                if (kill->r_idx)
                {
                    int x = kill->x;
                    int y = kill->y;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);

                    /* Count */
                    n++;
                }
            }

            /* Get keypress */
            msg_format("There are %d known monsters.", n);
            msg_print(NULL);

			n = 0;
            /* Scan the monsters */
            for (i = 1; i < borg_kills_nxt; i++)
            {
                borg_kill *kill = &borg_kills[i];

                /* Still alive */
                if (kill->r_idx)
                {
                    int x = kill->x;
                    int y = kill->y;

					if (!kill->friendly && !kill->pet) continue;

                    /* Display */
                    print_rel('*', TERM_GREEN, y, x);

                    /* Count */
                    n++;
                }
            }
            /* Get keypress */
            msg_format("And %d friendly monsters.", n);
            msg_print(NULL);

			/* Redraw map */
            prt_map();
            break;
        }


        /* Command: show "objects" */
        case 't':
        {
            int i, n = 0;

            /* Scan the objects */
            for (i = 1; i < borg_takes_nxt; i++)
            {
                borg_take *take = &borg_takes[i];

                /* Still alive */
                if (take->k_idx)
                {
                    int x = take->x;
                    int y = take->y;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);

                    /* Count */
                    n++;
                }
            }

            /* Get keypress */
            msg_format("There are %d known objects.", n);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }


        /* Command: debug -- current flow */
        case '%':
        {
            int i, x, y;

            /* Flow */
            for (i = 0; i < 250; i++)
            {
                int n = 0;

                /* Scan map */
                for (y = w_y; y < w_y + SCREEN_HGT; y++)
                {
                    for (x = w_x; x < w_x + SCREEN_WID; x++)
                    {
                        byte a = TERM_RED;

                        /* Verify flow cost */
                        if (borg_data_flow->data[y][x] != i) continue;

                        /* Display */
                        print_rel('*', a, y, x);

                        /* Count */
                        n++;
                    }
                }

                /* Nothing */
                if (!n) break;

                /* Get keypress */
                msg_format("Flow depth %d.", i);
                msg_print(NULL);

                /* Redraw map */
                prt_map();
            }
            break;
        }
		/* Display the intended path to the flow */
		case '^':
		{
			    int x, y;
				int o;
				int false_y, false_x;
			    borg_grid *ag;

				false_y = c_y;
				false_x = c_x;

			    /* Continue */
	            for (o = 0; o < 50; o++)
			    {
			        int b_n = 0;

			        int i, b_i = -1;

			        int c, b_c;


			        /* Flow cost of current grid */
			        b_c = borg_data_flow->data[c_y][c_x] * 10;

			        /* Prevent loops */
			        b_c = b_c - 5;

			        /* Look around */
			        for (i = 0; i < 8; i++)
			        {
			            /* Grid in that direction */
						x = false_x + ddx_ddd[i];
			            y = false_y + ddy_ddd[i];

			            /* Access the grid */
			            ag = &borg_grids[y][x];

			            /* Flow cost at that grid */
			            c = borg_data_flow->data[y][x] * 10;

			            /* Never backtrack */
			            if (c > b_c) continue;

			            /* Notice new best value */
			            if (c < b_c) b_n = 0;

			            /* Apply the randomizer to equivalent values */
			            if ((++b_n >= 2) && (rand_int(b_n) != 0)) continue;

			            /* Track it */
			            b_i = i; b_c = c;
			        }

			        /* Try it */
			        if (b_i >= 0)
			        {
			            /* Access the location */
			            x = false_x + ddx_ddd[b_i];
			            y = false_y + ddy_ddd[b_i];

                        /* Display */
                        print_rel('*', TERM_RED, y, x);

						/* Simulate motion */
			            false_y = y;
						false_x = x;
			        }

				}
				msg_print("Possible Flow Path.  Cost ");
                msg_print(NULL);

                /* Redraw map */
                prt_map();
		break;
		}

		/* Command: Resurrect Borg */
		case 'R':
		{
           char cmd;

           /* Confirm it */
           get_com("Are you sure you want to Respawn this borg? (y or n)? ", &cmd);

           if (cmd =='y' || cmd =='Y' )
           {
			   resurrect_borg();
		   }

           break;
       }


#endif /* not BORG_TK */
        /* Command: debug -- danger of grid */
        case '#':
        {
            int n = 0;

            /* Turns */
            n = (command_arg ? command_arg : 1);

            /* Danger of grid */
            msg_format("Danger(%d,%d,%d) is %d",
                        target_col, target_row, n,
                        borg_danger(target_row, target_col, n, TRUE));
            break;
        }

        /* Command:  Regional Fear Info*/
        case '_':
        {
            int x, y, p;

            /* Examine the screen */
            borg_update();

			/* Scan map */
            for (y = 1; y <= AUTO_MAX_Y - 1; y++)
            {
                for (x = 1; x <= AUTO_MAX_X - 1; x++)
                {
                    byte a = TERM_RED;

                    /* Obtain danger */
                    p =  borg_fear_region[y/11][x/11];

                    /* Skip non-fears */
                    if (p < avoidance / 15) continue;

                    /* Use colors = less painful */
                    if (p < avoidance / 2) a = TERM_RED;
                    if (p < avoidance / 4) a = TERM_ORANGE;
                    if (p < avoidance / 6) a = TERM_YELLOW;
                    if (p < avoidance / 8) a = TERM_GREEN;
                    if (p < avoidance / 10)	a = TERM_BLUE;

					/* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg_format("(%d,%d) Regional Fear.", c_y, c_x);
            msg_print(NULL);

            /* Redraw map */
            prt_map();

            /* Scan map */
            for (y = 1; y <= AUTO_MAX_Y; y++)
            {
                for (x = 1; x <= AUTO_MAX_X; x++)
                {
                    byte a = TERM_BLUE;

                    /* Obtain danger */
                    p =  borg_fear_monsters[y][x];

                    /* Skip non-fears */
                    if (p <= 0) continue;

                    /* Color Defines */
                    if (p == 1)  a = TERM_L_BLUE;

                    /* Color Defines */
                    if (p < avoidance / 20 &&
                        p > 1) a = TERM_BLUE;

                    /* Color Defines */
                    if (p < avoidance / 10 &&
                        p > avoidance / 20) a = TERM_GREEN;

                    /* Color Defines */
                    if (p < avoidance / 4 &&
                        p > avoidance / 10) a = TERM_YELLOW;

                    /* Color Defines */
                    if (p < avoidance / 2 &&
                        p > avoidance / 4) a = TERM_ORANGE;

                    /* Color Defines */
                    if (p > avoidance / 2)  a = TERM_RED;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg_format("(%d,%d) Monster Fear.", c_y, c_x);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

        /* Command: debug -- Power */
        case 'p':
        case 'P':
        {
            s32b p;

            /* Examine the screen */
            borg_update_frame();

            /* Examine the screen */
            borg_update();

            /* Cheat the "equip" screen */
            borg_cheat_equip();

            /* Cheat the "inven" screen */
            borg_cheat_inven();

            /* Extract some "hidden" variables */
            borg_hidden();

            /* Examine the inventory */
            borg_object_star_id();
            borg_notice(TRUE);

            /* Evaluate */
            p = borg_power();

			borg_cheat_store(borg_skill[BI_TOWN_NUM],BORG_HOME);

            borg_notice_home(NULL, FALSE);

            /* Report it */
            msg_format("Current Borg Power %ld", p);
            msg_format("Current Home Power %ld", borg_power_home());

            break;
        }

        /* Command: Show time */
        case '!':
        {
            s32b time = borg_t - borg_began;
            msg_format("time: (%d) ", time);
            time = (borg_time_town + (borg_t - borg_began));
            msg_format("; from town (%d)", time);
            msg_format("; on this panel (%d)", time_this_panel);
            msg_format("; need inviso (%d)", need_see_inviso);
            break;
        }

        /* Command: my LOS */
        case '@':
        {
            int x, y;
			borg_grid *ag;

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
					ag = &borg_grids[y][x];

					/* What the game has determined to be projectable */
					if (cave[y][x].info & (CAVE_VIEW)) print_rel('*', TERM_YELLOW, y, x);

					/* list of grids in the view array */
					if (ag->info & BORG_VIEW) print_rel('*', TERM_ORANGE, y, x);

					/* Projectable/ LOS routine */
                    else if (borg_los(c_y, c_x, y, x)) print_rel('*', TERM_RED, y, x);
                }
            }

            /* Get keypress */
            msg_format("Borg has LOS to these places.");
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }
        /* Command: show landing spots of Meteor Swarm */
        case '*':
        {
            int x, y;
			int i;

			borg_grid *ag;
            byte a = TERM_RED;

			/** Simulate 15 meteors **/
			/* Pick a location */
			for (i = 0; i < 200; i++)
			{
				int count = 0;
				int dy, dx;
				int d;

				bool redo = FALSE;

				/* Pick a location */
				do
				{
					redo = FALSE;
					count++;
					if (count > 250) break;
					x = c_x - 5 + randint(10);
					y = c_y - 5 + randint(10);

					dx = (c_x > x) ? (c_x - x) : (x - c_x);
					dy = (c_y > y) ? (c_y - y) : (y - c_y);

					/* Approximate distance */
					d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

					/* checks for loop */
					if (!in_bounds(y, x)) redo = TRUE;
					if (!borg_los(y, x, c_y, c_x)) redo = TRUE;
					if (d > 6) redo = TRUE;
				}
				while (redo);

				/* display the grid --*/

				/* What the game has determined to be projectable */
				if (cave[y][x].info & (CAVE_VIEW)) print_rel('*', TERM_YELLOW, y, x);

                /* Obtain sight with other routine */
                if (borg_los(c_y, c_x, y, x)) print_rel('*', a, y, x);
			}


            /* Get keypress */
            msg_format("Borg thinks meteors could fall on red.");
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

		case 'm':
		{
            int x, y;
			int yy, xx;

			borg_grid *ag;
            byte a = TERM_RED;

            /* Scan map */
            for (y = 0; y < MAX_HGT; y++)
            {
                for (x = 0; x < MAX_WID; x++)
                {
                    /* Obtain */
                    if (borg_detect_wall[y / 11][x / 33] == FALSE) continue;

                    /* Display */
                    print_rel('*', a, y, x);
                    print_rel('*', a, c_y, c_x);
                }
            }

            /* Get keypress */
            msg_format("Borg has Detect Walls to these places.");
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;




		}

       /*  command: debug -- change max depth */
       case '1':
        {
           int new_borg_depth;
           /* Get the new max depth */
           new_borg_depth = get_quantity("Enter new Max Depth: ", MAX_DEPTH - 1);

           /* Allow user abort */
           if (new_borg_depth >= 0)
           {
               p_ptr->max_dlv = new_borg_depth;
               borg_skill[BI_MAXDEPTH] = new_borg_depth;
           }

           break;
       }
       /*  command: debug -- allow borg to stop */
       case 'q':
        {
           int new_borg_stop_dlevel = 127;
           int new_borg_stop_clevel = 51;
           char cmd;

           /* Get the new max depth */
           new_borg_stop_dlevel = get_quantity("Enter new auto-stop dlevel: ", MAX_DEPTH -1);
           new_borg_stop_clevel = get_quantity("Enter new auto-stop clevel: ", 51);
           get_com("Stop when Morgoth Dies? (y or n)? ", &cmd);

           borg_stop_dlevel = new_borg_stop_dlevel;
           borg_stop_clevel = new_borg_stop_clevel;
           if (cmd =='n' || cmd =='N' ) borg_stop_king = FALSE;

           break;
       }

        /* Command:  HACK debug -- preparation for level */
        case '2':
        {
          int i=0;

            /* Examine the screen */
            borg_update_frame();
            borg_update();

            /* Extract some "hidden" variables */
            borg_cheat_equip();
            borg_cheat_inven();
            borg_hidden();

            /* Examine the inventory */
            borg_object_star_id();
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);

            /* Dump prep codes */
            for (i = 1; i <= 127; i++)
            {
               /* Dump fear code*/
               if ((cptr)NULL != borg_prepared[i]) break;
            }

            borg_slow_return = TRUE;
            msg_format("Max Level: %d  Prep'd For: %d  Reason: %s", borg_skill[BI_MAXDEPTH], i-1, borg_prepared[i] );
            borg_slow_return = FALSE;
            if (borg_ready_morgoth == 1)
            {
                msg_format("You are ready for the big fight!!");
            }

            break;
        }
        /* Command: debug -- Feature of grid */
        case '3':
        {

            borg_grid *ag = &borg_grids[target_col][target_row];


            /* Examine the screen */
            borg_update_frame();
            borg_update();
            borg_hidden();

            /* Feature of grid */
            msg_format("Feature of (%d,%d) %d",
				target_col, target_row), ag->feat;
            break;
        }

        /* Command: List the swap weapon and armour */
        case 'w':
        case 'W':
        {

            borg_item *item;

            /* Cheat the "equip" screen */
            borg_cheat_equip();
            /* Cheat the "inven" screen */
            borg_cheat_inven();
            /* Examine the inventory */
			borg_object_star_id();
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            /* Check the power */
            borg_power();

            /* Examine the screen */
            borg_update_frame();

            /* note the swap items */
            item = &borg_items[weapon_swap];
            msg_format("Swap Weapon:  %s, value= %d", item->desc, weapon_swap_value);
            item = &borg_items[armour_swap];
            msg_format("Swap Armour:  %s, value= %d", item->desc, armour_swap_value);
            break;
        }
		/* List the Nasties on the level */
        case 'C':
		{
			int i;

			/* Log Header */
			borg_note("Borg Nasties Count");

			/* Find the numerous nasty in order of nastiness */
			for (i = 0; i < borg_nasties_num; i++)
			{
				borg_note(format("Nasty: [%c] Count: %d, limited: %d",borg_nasties[i],
					borg_nasties_count[i],borg_nasties_limit[i]));
			}

			/* Done */
			msg_format("Borg Nasty Dump Complete.  Examine Log.");
			break;
		}

        /* Command: Display all known info on item */
        case 'o':
        case 'O':
        {
			int n =0;

			object_type *item2;

			/* use this item */
            n = (command_arg ? command_arg : 1);

            /* Examine the screen */
            borg_update();

            /* Extract some "hidden" variables */
            borg_hidden();

            /* Cheat the "equip" screen */
            borg_cheat_equip();
            /* Cheat the "inven" screen */
            borg_cheat_inven();
            /* Examine the inventory */
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            /* Check the power */
            borg_power();

            /* Examine the screen */
            borg_update_frame();

			/* Save the screen */
			Term_save();

			/* get the item */
			item2 = &inventory[n];

			/* Display the special screen */
			borg_display_item(item2);

			/* pause for study */
            msg_format("Borg believes: ");
            msg_print(NULL);

			/* Restore the screen */
            Term_load();


            break;
        }

#ifndef BORG_TK
        /* Change borg delay factor */
        case 'D':
        {
			int n = 4;

			/* use this item */
	        n = (command_arg ? command_arg : 1);

            borg_delay_factor = n;
            msg_format("Borg delay factor set to %d.", borg_delay_factor);
            break;
        }
        case 'd':
        {
            int ii = 2, k;

            /* Save the screen */
            Term_save();

            /* Dump the first realm spells */
            for (k = 1; k < MAX_REALM+1; k++)
            {
            	int i = 0,j;

				/* skip wrong realms */
				if (k != borg_skill[BI_REALM1] &&
					k != borg_skill[BI_REALM2]) continue;


				/* Books */
            	for (i = 0; i < 4; i++ )
            	{

            	    /* Clear the screen */
            	    Term_clear();

            	    Term_putstr(1, ii++, -1, TERM_WHITE, format("[ Realm %s ]:", borg_magics[k][0][0].realm_name));
            	    for (j = 0; j < 8; j++)
            	    {
            	        borg_magic *as = &borg_magics[k][i][j];
            	        cptr legal = NULL;

            	        if (as->level < 99)
            	        {
           	                if (k == borg_skill[BI_REALM1]) legal = (borg_spell_legal(borg_skill[BI_REALM1], i, j) ? "legal" : "Not Legal ");
           	                if (k == borg_skill[BI_REALM2]) legal = (borg_spell_legal(borg_skill[BI_REALM2], i, j) ? "legal" : "Not Legal ");
           	            }
           	            Term_putstr(1, ii++, -1, TERM_WHITE, format("%-25s %-10s %-2d attempts.",as->name, legal, as->times));
            	    }
            	    Term_putstr(1, ii++, -1, TERM_WHITE, format("Book located at inventory slot %d", borg_book[k][i]));
	        	    get_com("Exam spell books.  Press any key for next book.", &cmd);

					/* Reset where books are displayed */
					ii = 2;

            	} /* Books */
           	} /* Realms */

			/* Mindcrafters */
			if (borg_class == CLASS_ORPHIC)
			{
				int i;

				/* reset line for dump */
				i = 2;

          	    /* Clear the screen */
           	    Term_clear();


				/* Prepare the dump */
           	    Term_putstr(1, ii++, -1, TERM_WHITE, "[ Mindcrafter ]");

				/* Realm */
            	for (i = 0; i < MAX_MINDCRAFT_POWERS; i++ )
            	{
			        borg_mind *as = &borg_minds[i];
           	        cptr legal = NULL;

           	        if (as->level < 99)
           	        {
      	                legal = (borg_mindcr_legal(i, borg_skill[BI_CLEVEL]) ? "legal" : "Not Legal ");
       	            }

       	            Term_putstr(1, ii++, -1, TERM_WHITE, format("%s, %s, attempted %d times",as->name, legal, as->times));
           	    }

        	    get_com("Examing powers.  Press any key to continue.", &cmd);
			}

         /* Restore the screen */
         Term_load();

         /* Done */
         return;
        }

#endif /* BORG_TK */
        /* dump borg 'has' information */
        case 'h':
        {
            char cmd;
            int item = 0, to = 0;

            /* Get a "Borg command", or abort */
            if (!get_com("Dynamic Borg Has What: ((i)nv/(w)orn/(a)rtifact/(s)kill) ", &cmd)) return;

            switch (cmd)
            {
                case 'i':
                case 'I':
                    item = 0;
                    to = max_k_idx;
                    break;
                case 'w':
                case 'W':
                    item = max_k_idx;
                    to = max_k_idx + max_k_idx;
                    break;
                case 'a':
                case 'A':
                    item = max_k_idx + max_k_idx;
                    to = max_k_idx + max_k_idx + max_a_idx;
                    break;
                default:
                    item = max_k_idx + max_k_idx + max_a_idx;
                    to = max_k_idx + max_k_idx + max_a_idx + BI_MAX;
                    break;
            }
            /* Examine the screen */
            borg_update_frame();

            /* Examine the screen */
            borg_update();

            /* Cheat the "equip" screen */
            borg_cheat_equip();

            /* Cheat the "inven" screen */
            borg_cheat_inven();

            /* Extract some "hidden" variables */
            borg_hidden();

            /* Examine the inventory */
            borg_object_star_id();
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            for (;item < to; item++)
            {
                switch (cmd)
                {
                    case 'i':
                    case 'I':
                        borg_note(format("Item%03d value= %d.", item, borg_has[item]));
                        break;
                    case 'w':
                    case 'W':
                        borg_note(format("WItem%03d value= %d.", item-max_k_idx, borg_has[item]));
                        break;
                    case 'a':
                    case 'A':
                        borg_note(format("Artifact%03d value= %d.", item-max_k_idx-max_k_idx, borg_has[item]));
                        break;
                    default:
                        borg_note(format("skill %d (%s) value= %d.",item,
                            prefix_pref[item -
                                        max_k_idx -
                                        max_k_idx -
                                        max_a_idx], borg_has[item]));
                        break;
                }
            }

            /* note the completion. */
            msg_format("Borg_has[] dump complete.  Examine Log. ");
            break;
        }

		/* Happy Grids */
		case 'H':
		{
		    int i, x, y;

			int x1, y1, x2, y2;


		    /* Local region */
			y1 = c_y - 20;
			x1 = c_x - 20;
			y2 = c_y + 20;
			x2 = c_x + 20;

			/* Restrict to "legal" grids */
			if (y1 < 1) y1 = 1;
			if (x1 < 1) x1 = 1;
			if (y2 > AUTO_MAX_Y - 2) y2 = AUTO_MAX_Y - 2;
			if (x2 > AUTO_MAX_X - 2) x2 = AUTO_MAX_X - 2;


			/* Nothing yet */
			borg_temp_n = 0;

			/* Examine the panel */
			for (y = y1; y <= y2; y++)
			{
				/* Examine the panel */
				for (x = x1; x <= x2; x++)
				{
					/* Is this grid a happy grid? */
					if (!borg_happy_grid_bold(y, x)) continue;

					/* Can I rest on that one? */
					if (!borg_check_rest(y, x)) continue;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);

					/* Careful -- Remember it */
					borg_temp_x[borg_temp_n] = x;
					borg_temp_y[borg_temp_n] = y;
					borg_temp_n++;
				}
			}


            /* Get keypress */
            msg_format("Grids that are Happy and pass borg_check_rest (%d).", borg_temp_n-1);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;

		}


		/* Version of the game */
        case 'v':
        case 'V':
        {
            msg_format(" ZBorg Version: %s",borg_engine_date);
            break;
        }

		/* Show the Interesting Grids for flow dark-4 */
		case '4':
		{
		    int i, x, y;

			int x1, y1, x2, y2;


		    /* Local region */
			y1 = c_y - 11;
			x1 = c_x - 11;
			y2 = c_y + 11;
			x2 = c_x + 11;

			/* Restrict to "legal" grids */
			if (y1 < 1) y1 = 1;
			if (x1 < 1) x1 = 1;
			if (y2 > AUTO_MAX_Y - 2) y2 = AUTO_MAX_Y - 2;
			if (x2 > AUTO_MAX_X - 2) x2 = AUTO_MAX_X - 2;


			/* Nothing yet */
			borg_temp_n = 0;

			/* Examine the panel */
			for (y = y1; y <= y2; y++)
			{
				/* Examine the panel */
				for (x = x1; x <= x2; x++)
				{
					/* Skip "boring" grids */
					if (!borg_flow_dark_interesting(y, x)) continue;

					/* Skip "unreachable" grids */
					if (!borg_flow_dark_reachable(y, x)) continue;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);
		            msg_format("Flow dark-4 interesting grids (%d).", borg_temp_n);
		            msg_print(NULL);

					/* Careful -- Remember it */
					borg_temp_x[borg_temp_n] = x;
					borg_temp_y[borg_temp_n] = y;
					borg_temp_n++;
				}
			}


            /* Get keypress */
            msg_format("Total Flow dark-4 interesting grids (%d).", borg_temp_n-1);
            msg_print(NULL);

            /* Redraw map */
            prt_map();
            break;


		break;
}

        /* Oops */
        default:
        {
            /* Message */
            msg_print("That is not a legal Borg command.");
            break;
        }
    }
}



#ifdef BORG_TK

byte borg_delay_factor = 0;

cptr BORG_DIR_ROOT = NULL;
cptr BORG_DIR_DATA = NULL;

/* SettingType */
struct
{
    cptr name; /* Keyword */
    cptr desc; /* Human-readable description */
    void (*proc)();
    long data;
} settings[] = {
    {"borg_flag_save",
        "Save the game when the dungeon level changes",
        NULL, (long) &borg_flag_save},
    {"borg_stop_king",
        "Stop the borg on win",
        NULL, (long) &borg_stop_king},
    {"borg_worships_damage",
        "Borg worships damage",
        NULL, (long) &borg_worships_damage},
    {"borg_worships_speed",
        "Borg worships speed",
        NULL, (long) &borg_worships_speed},
    {"borg_worships_hp",
        "Borg worshops hitpoints",
        NULL, (long) &borg_worships_hp},
    {"borg_worships_mana",
        "Borg worships mana",
        NULL, (long) &borg_worships_mana},
    {"borg_worships_gold",
        "Borg worships gold",
        NULL, (long) &borg_worships_gold},
    {"borg_worships_ac",
        "Borg worships armor class",
        NULL, (long) &borg_worships_ac},
    {"borg_plays_risky",
        "Borg plays risky",
        NULL, (long) &borg_plays_risky},
    {"borg_slow_optimizehome",
        "Slow home optimize",
        NULL, (long) &borg_slow_optimizehome},
    {"borg_uses_swaps",
        "Borg uses Swap Items",
        NULL, (long) &borg_uses_swaps},
    {"borg_uses_calcs",
        "Borg uses Dynamic Calculations (slow)",
        NULL, (long) &borg_uses_calcs},
    {"delay_factor",
        "xyz",
        NULL, (long) &borg_delay_factor},
    {NULL, NULL, NULL, 0L},
};

void *Borg_GetSettings(void)
{
    return settings;
}

#define TCL_OK 0
typedef struct Tcl_Interp Tcl_Interp;

int Borg_Init(Tcl_Interp *interp, char *path)
{
    char path2[1024];

    BORG_DIR_ROOT = string_make(path);

    path_build(path2, 1024, BORG_DIR_ROOT, "data");
    BORG_DIR_DATA = string_make(path2);

    return TCL_OK;
}

#endif /* BORG_TK */




#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
