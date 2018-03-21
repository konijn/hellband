/* File: mon-desc.c */

/* Purpose: describe monsters (using monster memory) */

/*
 * Copyright (c) 1989 James E. Wilson, Christopher J. Stuart
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 *
 * James E. Wilson and Christopher J. Stuart released all changes to the Angband code under the terms of the GNU General Public License (version 2),
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
* Pronoun arrays, by gender.
*/
static cptr wd_he[3] =
{ "it", "he", "she" };
static cptr wd_his[3] =
{ "its", "his", "her" };


/*
* Pluralizer.  Args(count, singular, plural)
*/
#define plural(c,s,p) \
	(((c) == 1) ? (s) : (p))






/*
* Determine if the "armour" is known
* The higher the level, the fewer kills needed.
*/
static bool know_armour(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b kills = r_ptr->r_tkills;

	/* Normal monsters */
	if (kills > 304 / (4 + level)) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags1 & (RF1_UNIQUE))) return (FALSE);

	/* Unique monsters */
	if (kills > 304 / (38 + (5*level) / 4)) return (TRUE);

	/* Assume false */
	return (FALSE);
}


/*
* Determine if the "damage" of the given attack is known
* the higher the level of the monster, the fewer the attacks you need,
* the more damage an attack does, the more attacks you need
*/
static bool know_damage(int r_idx, int i)
{
	monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b a = r_ptr->r_blows[i];

	s32b d1 = r_ptr->blow[i].d_dice;
	s32b d2 = r_ptr->blow[i].d_side;

	s32b d = d1 * d2;

	/* Normal monsters */
	if ((4 + level) * a > 80 * d) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags1 & (RF1_UNIQUE))) return (FALSE);

	/* Unique monsters */
	if ((4 + level) * (2 * a) > 80 * d) return (TRUE);

	/* Assume false */
	return (FALSE);
}


/*
* Hack -- display monster information using "roff()"
*
* Note that there is now a compiler option to only read the monster
* descriptions from the raw file when they are actually needed, which
* saves about 60K of memory at the cost of disk access during monster
* recall, which is optional to the user.
*
* This function should only be called with the cursor placed at the
* left edge of the screen, on a cleared line, in which the recall is
* to take place.  One extra blank line is left after the recall.
*/
static void roff_aux(int r_idx)
{
	monster_race	*r_ptr;

	bool		old = FALSE;
	bool		sin = FALSE;

	int			m, n, r;

	cptr		p, q;

	int			msex = 0;

	bool		breath = FALSE;
	bool		magic = FALSE;

	u32b		flags1;
	u32b		flags2;
	u32b		flags3;
	u32b		flags4;
	u32b		flags5;
	u32b		flags6;
	u32b		flags7;	

	int			vn = 0;
	cptr		vp[64];

	monster_race        save_mem;



	/* Access the race and lore */
	r_ptr = &r_info[r_idx];


	/* Debug -- Know everything */
	if (debug_know)
	{
		/* XXX XXX XXX */

		/* Save the "old" memory */
		save_mem = *r_ptr;

		/* Hack -- Maximal kills */
		r_ptr->r_tkills = MAX_SHORT;

		/* Hack -- Maximal info */
		r_ptr->r_wake = r_ptr->r_ignore = MAX_UCHAR;

		/* Observe "maximal" attacks */
		for (m = 0; m < 4; m++)
		{
			/* Examine "actual" blows */
			if (r_ptr->blow[m].effect || r_ptr->blow[m].method)
			{
				/* Hack -- maximal observations */
				r_ptr->r_blows[m] = MAX_UCHAR;
			}
		}

		/* Hack -- maximal drops */
		r_ptr->r_drop_gold = r_ptr->r_drop_item =
			(((r_ptr->flags1 & (RF1_DROP_4D2)) ? 8 : 0) +
			((r_ptr->flags1 & (RF1_DROP_3D2)) ? 6 : 0) +
			((r_ptr->flags1 & (RF1_DROP_2D2)) ? 4 : 0) +
			((r_ptr->flags1 & (RF1_DROP_1D2)) ? 2 : 0) +
			((r_ptr->flags1 & (RF1_DROP_90))  ? 1 : 0) +
			((r_ptr->flags1 & (RF1_DROP_60))  ? 1 : 0));

		/* Hack -- but only "valid" drops */
		if (r_ptr->flags1 & (RF1_ONLY_GOLD)) r_ptr->r_drop_item = 0;
		if (r_ptr->flags1 & (RF1_ONLY_ITEM)) r_ptr->r_drop_gold = 0;

		/* Hack -- observe many spells */
		r_ptr->r_cast_inate = MAX_UCHAR;
		r_ptr->r_cast_spell = MAX_UCHAR;

		/* Hack -- know all the flags */
		r_ptr->r_flags1 = r_ptr->flags1;
		r_ptr->r_flags2 = r_ptr->flags2;
		r_ptr->r_flags3 = r_ptr->flags3;
		r_ptr->r_flags4 = r_ptr->flags4;
		r_ptr->r_flags5 = r_ptr->flags5;
		r_ptr->r_flags6 = r_ptr->flags6;
		r_ptr->r_flags7 = r_ptr->flags7;		
	}


	/* Extract a gender (if applicable) */
	if (r_ptr->flags1 & (RF1_FEMALE)) msex = 2;
	else if (r_ptr->flags1 & (RF1_MALE)) msex = 1;


	/* Obtain a copy of the "known" flags */
	flags1 = (r_ptr->flags1 & r_ptr->r_flags1);
	flags2 = (r_ptr->flags2 & r_ptr->r_flags2);
	flags3 = (r_ptr->flags3 & r_ptr->r_flags3);
	flags4 = (r_ptr->flags4 & r_ptr->r_flags4);
	flags5 = (r_ptr->flags5 & r_ptr->r_flags5);
	flags6 = (r_ptr->flags6 & r_ptr->r_flags6);
	flags7 = (r_ptr->flags7 & r_ptr->r_flags7);

	/* Assume some "obvious" flags */
	if (r_ptr->flags1 & (RF1_UNIQUE)) flags1 |= (RF1_UNIQUE);
	if ((r_ptr->flags1 & RF1_GUARDIAN) || (r_ptr->flags1 & RF1_ALWAYS_GUARD)) flags1 |= (RF1_GUARDIAN);
	if (r_ptr->flags1 & (RF1_MALE)) flags1 |= (RF1_MALE);
	if (r_ptr->flags1 & (RF1_FEMALE)) flags1 |= (RF1_FEMALE);
    if (r_ptr->flags7 & (RF7_ANNOYED)) flags7 |= (RF7_ANNOYED);

	/* Assume some "creation" flags */
	if (r_ptr->flags1 & (RF1_FRIEND)) flags1 |= (RF1_FRIEND);
	if (r_ptr->flags1 & (RF1_FRIENDS)) flags1 |= (RF1_FRIENDS);
	if (r_ptr->flags1 & (RF1_ESCORT)) flags1 |= (RF1_ESCORT);
	if (r_ptr->flags1 & (RF1_ESCORTS)) flags1 |= (RF1_ESCORTS);

	/* Killing a monster reveals some properties */
	if (r_ptr->r_tkills)
	{
		/* Know "race" flags */
		if (r_ptr->flags3 & (RF3_ORC)) flags3 |= (RF3_ORC);
		if (r_ptr->flags3 & (RF3_TROLL)) flags3 |= (RF3_TROLL);
		if (r_ptr->flags3 & (RF3_GIANT)) flags3 |= (RF3_GIANT);
		if (r_ptr->flags3 & (RF3_DRAGON)) flags3 |= (RF3_DRAGON);
		if (r_ptr->flags3 & (RF3_DEMON)) flags3 |= (RF3_DEMON);
		if (r_ptr->flags3 & (RF3_DEVIL)) flags3 |= (RF3_DEVIL);
		if (r_ptr->flags3 & (RF3_UNDEAD)) flags3 |= (RF3_UNDEAD);
		if (r_ptr->flags3 & (RF3_EVIL)) flags3 |= (RF3_EVIL);
		if (r_ptr->flags3 & (RF3_GOOD)) flags3 |= (RF3_GOOD);
		if (r_ptr->flags3 & (RF3_ANIMAL)) flags3 |= (RF3_ANIMAL);
		if (r_ptr->flags3 & (RF3_FALLEN_ANGEL)) flags3 |= (RF3_FALLEN_ANGEL);

		/* Know "forced" flags */
		if (r_ptr->flags1 & (RF1_FORCE_MAXHP)) flags1 |= (RF1_FORCE_MAXHP);
        
        /* Know reborn flag */
        if (r_ptr->flags7 & (RF7_REBORN)) flags7 |= (RF7_REBORN);
	}


	/* Require a flag to show kills */
	if (!(show_details))
	{
		/* nothing */
	}

	/* Treat uniques differently */
	else if (flags1 & (RF1_UNIQUE))
	{
		/* Hack -- Determine if the unique is "dead" */
		bool dead = (r_ptr->max_num == 0) ? TRUE : FALSE;

		/* We've been killed... */
		if (r_ptr->r_deaths)
		{
			/* Killed ancestors */
			roff(format("%^s has slain %d of your ancestors",
				wd_he[msex], r_ptr->r_deaths));

			/* But we've also killed it */
			if (dead)
			{
				roff(format(", but you have avenged %s!  ",
					plural(r_ptr->r_deaths, "him", "them")));
			}

			/* Unavenged (ever) */
			else
			{
				roff(format(", who %s unavenged.  ",
					plural(r_ptr->r_deaths, "remains", "remain")));
			}
		}

		/* Dead unique who never killed us */
		else if (dead)
		{
			roff("You have slain this foe");
			if(flags7 & (RF7_REBORN))
			{
				roff(" at least once");
			}
			roff(".  ");
		}
	}

	/* Not unique, but killed us */
	else if (r_ptr->r_deaths)
	{
		/* Dead ancestors */
		roff(format("%d of your ancestors %s been killed by this creature, ",
			r_ptr->r_deaths, plural(r_ptr->r_deaths, "has", "have")));

		/* Some kills this life */
		if (r_ptr->r_pkills)
		{
			roff(format("and you have exterminated at least %d of the creatures.  ",
				r_ptr->r_pkills));
		}

		/* Some kills past lives */
		else if (r_ptr->r_tkills)
		{
			roff(format("and %s have exterminated at least %d of the creatures.  ",
				"your ancestors", r_ptr->r_tkills));
		}

		/* No kills */
		else
		{
			roff(format("and %s is not ever known to have been defeated.  ",
				wd_he[msex]));
		}
	}

	/* Normal monsters */
	else
	{
		/* Killed some this life */
		if (r_ptr->r_pkills)
		{
			roff(format("You have killed at least %d of these creatures.  ",
				r_ptr->r_pkills));
		}

		/* Killed some last life */
		else if (r_ptr->r_tkills)
		{
			roff(format("Your ancestors have killed at least %d of these creatures.  ",
				r_ptr->r_tkills));
		}

		/* Killed none */
		else
		{
			roff("No battles to the death are recalled.  ");
		}
	}


	/* Descriptions */
	if (show_details)
	{
		char buf[2048];

#ifdef DELAY_LOAD_R_TEXT

		int fd;

		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

		/* Open the "raw" file */
		fd = fd_open(buf, O_RDONLY);

		/* Use file */
		if (fd >= 0)
		{
			huge pos;

			/* Starting position */
			pos = r_ptr->text;

			/* Additional offsets */
			pos += r_head->head_size;
			pos += r_head->info_size;
			pos += r_head->name_size;

			/* Seek */
			(void)fd_seek(fd, pos);

			/* Read a chunk of data */
			(void)fd_read(fd, buf, 2048);

			/* Close it */
			(void)fd_close(fd);
		}

#else

		/* Simple method */
		strcpy(buf, r_text + r_ptr->text);

#endif

		/* Dump it */
		roff(buf);
		roff(" ");
	}


	/* Nothing yet */
	old = FALSE;

	/* Describe location */
	if (r_ptr->level == 0)
	{
		c_roff(TERM_YELLOW, format("%^s lives in the town", wd_he[msex]),-1,-1,0);
		old = TRUE;
	}
	else if (r_ptr->r_tkills)
	{
		if (depth_in_feet)
		{
			roff(format("%^s is normally found at depths of %d feet",
				wd_he[msex], r_ptr->level * 50));
		}
		else
		{
			roff(format("%^s is normally found on dungeon level %d",
				wd_he[msex], r_ptr->level));
		}
		old = TRUE;
	}


	/* Describe movement */
	if (TRUE)
	{
		/* Introduction */
		if (old)
		{
			roff(", and ");
		}
		else
		{
			roff(format("%^s ", wd_he[msex]));
			old = TRUE;
		}
		roff("moves");

		/* Random-ness */
		if ((flags1 & (RF1_RAND_50)) || (flags1 & (RF1_RAND_25)))
		{
			/* Adverb */
			if ((flags1 & (RF1_RAND_50)) && (flags1 & (RF1_RAND_25)))
			{
				roff(" extremely");
			}
			else if (flags1 & (RF1_RAND_50))
			{
				roff(" somewhat");
			}
			else if (flags1 & (RF1_RAND_25))
			{
				roff(" a bit");
			}

			/* Adjective */
			roff(" erratically");

			/* Hack -- Occasional conjunction */
			if (r_ptr->speed != 110) roff(", and");
		}

		/* Speed */
		if (r_ptr->speed > 110)
		{
			if (r_ptr->speed > 130) roff(" incredibly");
			else if (r_ptr->speed > 120) roff(" very");
			roff(" quickly");
		}
		else if (r_ptr->speed < 110)
		{
			if (r_ptr->speed < 90) roff(" incredibly");
			else if (r_ptr->speed < 100) roff(" very");
			roff(" slowly");
		}
		else
		{
			roff(" at normal speed");
		}
	}

	/* The code above includes "attack speed" */
	if (flags1 & (RF1_NEVER_MOVE))
	{
		/* Introduce */
		if (old)
		{
			roff(", but ");
		}
		else
		{
			roff(format("%^s ", wd_he[msex]));
			old = TRUE;
		}

		/* Describe */
		roff("does not deign to chase intruders");
	}

	/* End this sentence */
	if (old)
	{
		roff(".  ");
		old = FALSE;
	}


	/* Describe experience if known */
	if (r_ptr->r_tkills)
	{
		/* Introduction */
		if (flags1 & (RF1_UNIQUE))
		{
			roff("Killing this");
		}
		else
		{
			roff("A kill of this");
		}

		/* Describe the "quality" */
		if (flags3 & (RF3_ANIMAL)) roff(" natural");
		if (flags3 & (RF3_EVIL)) roff(" evil");
		if (flags3 & (RF3_GOOD)) roff (" good");
		if (flags3 & (RF3_UNDEAD)) roff(" undead");
		if (flags3 & (RF3_DEVIL)) roff(" infernal");

		/* Describe the "race" */
		if (flags3 & (RF3_DRAGON)) roff(" dragon");
		else if (flags3 & (RF3_DEMON)) roff(" demon");
		else if (flags3 & (RF3_GIANT)) roff(" giant");
		else if (flags3 & (RF3_TROLL)) roff(" troll");
		else if (flags3 & (RF3_ORC)) roff(" orc");
		else if (flags3 & (RF3_FALLEN_ANGEL)) roff (" fallen angel");
		else roff(" creature");

		if(flags7 & (RF7_REBORN) && r_ptr->r_pkills>0)
		{
			roff(" again");
		}
		
		/* Group some variables */
		if (TRUE)
		{
			s32b i, j;

			/* How much xp would we get ? */
			calculate_xp( r_ptr, &i , &j );
			
			/* Convert the fraction into a percentage */
			j = j * 100 / 0x10000L ;

			/* Mention the experience */
			roff(format(" is worth %ld.%02ld point%s",
				(long)i, (long)j,
				(((i == 1) && (j == 0)) ? "" : "s")));

			/* Take account of annoying English */
			p = "th";
			i = p_ptr->lev % 10;
			if ((p_ptr->lev / 10) == 1) /* nothing */;
			else if (i == 1) p = "st";
			else if (i == 2) p = "nd";
			else if (i == 3) p = "rd";

			/* Take account of "leading vowels" in numbers */
			q = "";
			i = p_ptr->lev;
			if ((i == 8) || (i == 11) || (i == 18)) q = "n";

			/* Mention the dependance on the player's level */
			roff(format(" for a%s %lu%s level character.  ",
				q, (long)i, p));
		}
	}

	if ((flags2 & (RF2_AURA_FIRE)) && (flags2 & (RF2_AURA_ELEC)))
	{
		roff(format("%^s is surrounded by flames and electricity.  ", wd_he[msex]));
	}
	else if (flags2 & (RF2_AURA_FIRE))
	{
		roff(format("%^s is surrounded by flames.  ", wd_he[msex]));
	}
	else if (flags2 & (RF2_AURA_ELEC))
	{
		roff(format("%^s is surrounded by electricity.  ", wd_he[msex]));
	}

	if (flags2 & (RF2_REFLECTING))
	{
		roff(format("%^s reflects bolt spells.  ", wd_he[msex]));
	}


	/* Describe escorts */
	if ((flags1 & (RF1_ESCORT)) || (flags1 & (RF1_ESCORTS)))
	{
		roff(format("%^s usually appears with escorts.  ",
			wd_he[msex]));
	}

	/* Describe friends */
	else if ((flags1 & (RF1_FRIEND)) || (flags1 & (RF1_FRIENDS)))
	{
		roff(format("%^s usually appears in groups.  ",
			wd_he[msex]));
	}


	/* Collect inate attacks */
	vn = 0;
	if (flags4 & (RF4_SHRIEK))		vp[vn++] = "shriek for help";
	if (flags4 & (RF4_CUSTOM))		vp[vn++] = "be the only of its kind";
	if (flags4 & (RF4_XXX3))		vp[vn++] = "do something";
	if (flags4 & (RF4_SHARD))       vp[vn++] = "produce shard balls";
	if (flags4 & (RF4_ARROW_1))		vp[vn++] = "fire an arrow";
	if (flags4 & (RF4_ARROW_2))		vp[vn++] = "fire arrows";
	if (flags4 & (RF4_ARROW_3))		vp[vn++] = "fire a missile";
	if (flags4 & (RF4_ARROW_4))		vp[vn++] = "fire missiles";

	/* Describe inate attacks */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" may ");
			else if (n < vn-1) roff(", ");
			else roff(" or ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}


	/* Collect breaths */
	vn = 0;
	if (flags4 & (RF4_BR_ACID))		vp[vn++] = "acid";
	if (flags4 & (RF4_BR_ELEC))		vp[vn++] = "lightning";
	if (flags4 & (RF4_BR_FIRE))		vp[vn++] = "fire";
	if (flags4 & (RF4_BR_COLD))		vp[vn++] = "frost";
	if (flags4 & (RF4_BR_POIS))		vp[vn++] = "poison";
	if (flags4 & (RF4_BR_NETH))		vp[vn++] = "nether";
	if (flags4 & (RF4_BR_LITE))		vp[vn++] = "light";
	if (flags4 & (RF4_BR_DARK))		vp[vn++] = "darkness";
	if (flags4 & (RF4_BR_CONF))		vp[vn++] = "confusion";
	if (flags4 & (RF4_BR_SOUN))		vp[vn++] = "sound";
	if (flags4 & (RF4_BR_CHAO))		vp[vn++] = "chaos";
	if (flags4 & (RF4_BR_DISE))		vp[vn++] = "disenchantment";
	if (flags4 & (RF4_BR_NEXU))		vp[vn++] = "nexus";
	if (flags4 & (RF4_BR_TIME))		vp[vn++] = "time";
	if (flags4 & (RF4_BR_INER))		vp[vn++] = "inertia";
	if (flags4 & (RF4_BR_GRAV))		vp[vn++] = "gravity";
	if (flags4 & (RF4_BR_SHAR))		vp[vn++] = "shards";
	if (flags4 & (RF4_BR_PLAS))		vp[vn++] = "plasma";
	if (flags4 & (RF4_BR_WALL))		vp[vn++] = "force";
	if (flags4 & (RF4_BR_MANA))		vp[vn++] = "mana";
	if (flags4 & (RF4_BR_SLIM))     vp[vn++] = "hellslime";
	if (flags4 & (RF4_BR_DISI))     vp[vn++] = "disintegration";

	/* Describe breaths */
	if (vn)
	{
		/* Note breath */
		breath = TRUE;

		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" may breathe ");
			else if (n < vn-1) roff(", ");
			else roff(" or ");

			/* Dump */
			roff(vp[n]);
		}
	}


	/* Collect spells */
	vn = 0;
	if (flags5 & (RF5_BA_ACID))		vp[vn++] = "produce acid balls";
	if (flags5 & (RF5_BA_ELEC))		vp[vn++] = "produce lightning balls";
	if (flags5 & (RF5_BA_FIRE))		vp[vn++] = "produce fire balls";
	if (flags5 & (RF5_BA_COLD))		vp[vn++] = "produce frost balls";
	if (flags5 & (RF5_BA_POIS))		vp[vn++] = "produce poison balls";
	if (flags5 & (RF5_BA_NETH))		vp[vn++] = "produce nether balls";
	if (flags5 & (RF5_BA_WATE))		vp[vn++] = "produce water balls";
	if (flags4 & (RF4_BA_SLIM))     vp[vn++] = "produce balls of hellslime";
	if (flags5 & (RF5_BA_MANA))		vp[vn++] = "invoke mana storms";
	if (flags5 & (RF5_BA_DARK))		vp[vn++] = "invoke darkness storms";
	if (flags4 & (RF4_BA_CHAO))     vp[vn++] = "invoke raw chaos";
	if (flags6 & (RF6_MARK_CAIN))   vp[vn++] = "invoke the mark of Cain";
	if (flags5 & (RF5_DRAIN_MANA))	vp[vn++] = "drain mana";
	if (flags5 & (RF5_MIND_BLAST))	vp[vn++] = "cause mind blasting";
	if (flags5 & (RF5_BRAIN_SMASH))	vp[vn++] = "cause brain smashing";
	if (flags5 & (RF5_CAUSE_1))     vp[vn++] = "cause light wounds and cursing";
	if (flags5 & (RF5_CAUSE_2))     vp[vn++] = "cause serious wounds and cursing";
	if (flags5 & (RF5_CAUSE_3))     vp[vn++] = "cause critical wounds and cursing";
	if (flags5 & (RF5_CAUSE_4))		vp[vn++] = "cause mortal wounds";
	if (flags5 & (RF5_BO_ACID))		vp[vn++] = "produce acid bolts";
	if (flags5 & (RF5_BO_ELEC))		vp[vn++] = "produce lightning bolts";
	if (flags5 & (RF5_BO_FIRE))		vp[vn++] = "produce fire bolts";
	if (flags5 & (RF5_BO_COLD))		vp[vn++] = "produce frost bolts";
	if (flags5 & (RF5_BO_POIS))		vp[vn++] = "produce poison bolts";
	if (flags5 & (RF5_BO_NETH))		vp[vn++] = "produce nether bolts";
	if (flags5 & (RF5_BO_WATE))		vp[vn++] = "produce water bolts";
	if (flags5 & (RF5_BO_MANA))		vp[vn++] = "produce mana bolts";
	if (flags5 & (RF5_BO_PLAS))		vp[vn++] = "produce plasma bolts";
	if (flags5 & (RF5_BO_ICEE))		vp[vn++] = "produce ice bolts";
	if (flags5 & (RF5_MISSILE))		vp[vn++] = "produce magic missiles";
	if (flags5 & (RF5_SCARE))		vp[vn++] = "terrify";
	if (flags5 & (RF5_BLIND))		vp[vn++] = "blind";
	if (flags5 & (RF5_CONF))		vp[vn++] = "confuse";
	if (flags5 & (RF5_SLOW))		vp[vn++] = "slow";
	if (flags5 & (RF5_HOLD))		vp[vn++] = "paralyze";
	if (flags6 & (RF6_HASTE))		vp[vn++] = "haste-self";

	if (flags6 & (RF6_HEAL))		vp[vn++] = "heal-self";
	if (flags6 & (RF6_XXX2))		vp[vn++] = "do something";
	if (flags6 & (RF6_BLINK))		vp[vn++] = "blink-self";
	if (flags6 & (RF6_TPORT))		vp[vn++] = "teleport-self";
	if (flags6 & (RF6_XXX3))		vp[vn++] = "do something";
	if (flags6 & (RF6_XXX4))		vp[vn++] = "do something";
	if (flags6 & (RF6_TELE_TO))		vp[vn++] = "teleport to";
	if (flags6 & (RF6_TELE_AWAY))		vp[vn++] = "teleport away";
	if (flags6 & (RF6_TELE_LEVEL))	vp[vn++] = "teleport level";
	if (flags6 & (RF6_XXX5))		vp[vn++] = "do something";
	if (flags6 & (RF6_DARKNESS))		vp[vn++] = "create darkness";
	if (flags6 & (RF6_TRAPS))		vp[vn++] = "create traps";
	if (flags6 & (RF6_FORGET))		vp[vn++] = "cause amnesia";
	if (flags6 & (RF6_XXX6))		vp[vn++] = "do something";
	if (flags6 & (RF6_S_MONSTER))       vp[vn++] = "summon a monster";
	if (flags6 & (RF6_S_MONSTERS))	vp[vn++] = "summon monsters";
	if (flags6 & (RF6_S_KIN))       vp[vn++] = "summon aid";
	if (flags6 & (RF6_S_ANT))		vp[vn++] = "summon ants";
	if (flags6 & (RF6_S_SPIDER))		vp[vn++] = "summon spiders";
	if (flags6 & (RF6_S_HOUND))		vp[vn++] = "summon hounds";
	if (flags6 & (RF6_S_HYDRA))		vp[vn++] = "summon hydras";
	if (flags6 & (RF6_S_DEVIL))		vp[vn++] = "summon a devil";
	if (flags6 & (RF6_S_DEMON))		vp[vn++] = "summon a demon";
	if (flags6 & (RF6_S_UNDEAD))		vp[vn++] = "summon an undead";
	if (flags6 & (RF6_S_DRAGON))		vp[vn++] = "summon a dragon";
	if (flags6 & (RF6_S_HI_UNDEAD))	vp[vn++] = "summon Greater Undead";
	if (flags6 & (RF6_S_HI_DRAGON))	vp[vn++] = "summon Ancient Dragons";
	if (flags6 & (RF6_S_REAVER))     vp[vn++] = "summon Black Reavers";
	if (flags6 & (RF6_S_FALLEN))        vp[vn++] = "summon fallen angels";
	if (flags6 & (RF6_S_UNIQUE))		vp[vn++] = "summon Unique Monsters";

	/* Describe spells */
	if (vn)
	{
		/* Note magic */
		magic = TRUE;

		/* Intro */
		if (breath)
		{
			roff(", and is also");
		}
		else
		{
			roff(format("%^s is", wd_he[msex]));
		}

		/* Verb Phrase */
		roff(" magical, casting spells");

		/* Adverb */
		if (flags2 & (RF2_SMART)) roff(" intelligently");

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" which ");
			else if (n < vn-1) roff(", ");
			else roff(" or ");

			/* Dump */
			roff(vp[n]);
		}
	}


	/* End the sentence about inate/other spells */
	if (breath || magic)
	{
		/* Total casting */
		m = r_ptr->r_cast_inate + r_ptr->r_cast_spell;

		/* Average frequency */
		n = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

		/* Describe the spell frequency */
		if (m > 100)
		{
			roff(format("; 1 time in %d", 100 / n));
		}

		/* Guess at the frequency */
		else if (m)
		{
			n = ((n + 9) / 10) * 10;
			roff(format("; about 1 time in %d", 100 / n));
		}

		/* End this sentence */
		roff(".  ");
	}


	/* Describe monster "toughness" */
	if (know_armour(r_idx))
	{
		/* Armor */
		roff(format("%^s has an armour rating of %d",
			wd_he[msex], r_ptr->ac));

		/* Maximized hitpoints */
		if (flags1 & (RF1_FORCE_MAXHP))
		{
			roff(format(" and a life rating of %d.  ",
				r_ptr->hdice * r_ptr->hside));
		}

		/* Variable hitpoints */
		else
		{
			roff(format(" and a life rating of %dd%d.  ",
				r_ptr->hdice, r_ptr->hside));
		}
	}



	/* Collect special abilities. */
	vn = 0;
	if (flags2 & (RF2_OPEN_DOOR)) vp[vn++] = "open doors";
	if (flags2 & (RF2_BASH_DOOR)) vp[vn++] = "bash down doors";
	if (flags2 & (RF2_PASS_WALL)) vp[vn++] = "pass through walls";
	if (flags2 & (RF2_KILL_WALL)) vp[vn++] = "bore through walls";
	if (flags2 & (RF2_MOVE_BODY)) vp[vn++] = "push past weaker monsters";
	if (flags2 & (RF2_KILL_BODY)) vp[vn++] = "destroy weaker monsters";
	if (flags2 & (RF2_TAKE_ITEM)) vp[vn++] = "pick up objects";
	if (flags2 & (RF2_KILL_ITEM)) vp[vn++] = "destroy objects";
	if (flags7 & (RF7_AQUATIC)) vp[vn++] = "swim";
	if (flags7 & (RF7_FLIGHT)) vp[vn++] = "fly";	

	/* Describe special abilities. */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" can ");
			else if (n < vn-1) roff(", ");
			else roff(" and ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}


	/* Describe special abilities. */
	if (flags2 & (RF2_INVISIBLE))
	{
		roff(format("%^s is invisible.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_COLD_BLOOD))
	{
		roff(format("%^s is cold blooded.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_EMPTY_MIND))
	{
		roff(format("%^s is not detected by telepathy.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_WEIRD_MIND))
	{
		roff(format("%^s is rarely detected by telepathy.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_MULTIPLY))
	{
		roff(format("%^s breeds explosively.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_REGENERATE))
	{
		roff(format("%^s regenerates quickly.  ", wd_he[msex]));
	}


	/* Collect susceptibilities */
	vn = 0;
	if (flags3 & (RF3_HURT_ROCK)) vp[vn++] = "rock remover";
	if (flags3 & (RF3_HURT_LITE)) vp[vn++] = "bright light";
	if (flags3 & (RF3_HURT_FIRE)) vp[vn++] = "fire";
	if (flags3 & (RF3_HURT_COLD)) vp[vn++] = "cold";
	if (flags7 & (RF7_HURT_DARK)) vp[vn++] = "darkness";	

	/* Describe susceptibilities */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" is hurt by ");
			else if (n < vn-1) roff(", ");
			else roff(" and ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}


	/* Collect immunities */
	vn = 0;
	if (flags3 & (RF3_IM_ACID)) vp[vn++] = "acid";
	if (flags3 & (RF3_IM_ELEC)) vp[vn++] = "lightning";
	if (flags3 & (RF3_IM_FIRE)) vp[vn++] = "fire";
	if (flags3 & (RF3_IM_COLD)) vp[vn++] = "cold";
	if (flags3 & (RF3_IM_POIS)) vp[vn++] = "poison";
	if (flags7 & (RF7_IM_LITE)) vp[vn++] = "light";	
	if (flags7 & (RF7_IM_DARK)) vp[vn++] = "darkness";

	/* Describe immunities */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" is immune to ");
			else if (n < vn-1) roff(", ");
			else roff(" and ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}


	/* Collect resistances */
	vn = 0;
	if (flags7 & (RF7_RES_FIRE)) vp[vn++] = "fire";
	if (flags7 & (RF7_RES_COLD)) vp[vn++] = "cold";	
	if (flags7 & (RF7_RES_ELEC)) vp[vn++] = "electricity";
	if (flags7 & (RF7_RES_ACID)) vp[vn++] = "acid";
	if (flags7 & (RF7_RES_POIS)) vp[vn++] = "poison";	
	if (flags7 & (RF7_RES_LITE)) vp[vn++] = "light";
	if (flags7 & (RF7_RES_DARK)) vp[vn++] = "darkness";	
	if (flags3 & (RF3_RES_NETH)) vp[vn++] = "nether";
	if (flags3 & (RF3_RES_WATE)) vp[vn++] = "water";
	if (flags3 & (RF3_RES_PLAS)) vp[vn++] = "plasma";
	if (flags3 & (RF3_RES_NEXU)) vp[vn++] = "nexus";
	if (flags3 & (RF3_RES_DISE)) vp[vn++] = "disenchantment";
	if (flags3 & (RF3_RES_TELE)) vp[vn++] = "teleportation";

	/* Describe resistances */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" resists ");
			else if (n < vn-1) roff(", ");
			else roff(" and ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}

	/* Collect healing elements */
	vn = 0;
	if (flags7 & (RF7_HEAL_FIRE)) vp[vn++] = "fire";
	if (flags7 & (RF7_HEAL_COLD)) vp[vn++] = "cold";	
	if (flags7 & (RF7_HEAL_ELEC)) vp[vn++] = "electricity";
	if (flags7 & (RF7_HEAL_LITE)) vp[vn++] = "light";
	if (flags7 & (RF7_HEAL_DARK)) vp[vn++] = "darkness";	
	if (flags7 & (RF7_HEAL_NETH)) vp[vn++] = "nether";
	
	/* Describe resistances */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));
		
		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" is healed by ");
			else if (n < vn-1) roff(", ");
			else roff(" and ");
			
			/* Dump */
			roff(vp[n]);
		}
		
		/* End */
		roff(".  ");
	}
	
	
	/* Collect non-effects */
	vn = 0;
	if (flags3 & (RF3_NO_STUN)) vp[vn++] = "stunned";
	if (flags3 & (RF3_NO_FEAR)) vp[vn++] = "frightened";
	if (flags3 & (RF3_NO_CONF)) vp[vn++] = "confused";
	if (flags3 & (RF3_NO_SLEEP)) vp[vn++] = "slept";

	/* Describe non-effects */
	if (vn)
	{
		/* Intro */
		roff(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) roff(" cannot be ");
			else if (n < vn-1) roff(", ");
			else roff(" or ");

			/* Dump */
			roff(vp[n]);
		}

		/* End */
		roff(".  ");
	}


	/* Do we know how aware it is? */
	if ((((int)r_ptr->r_wake * (int)r_ptr->r_wake) > r_ptr->sleep) ||
		(r_ptr->r_ignore == MAX_UCHAR) ||
		((r_ptr->sleep == 0) && (r_ptr->r_tkills >= 10)))
	{
		cptr act;

		if (r_ptr->sleep > 200)
		{
			act = "prefers to ignore";
		}
		else if (r_ptr->sleep > 95)
		{
			act = "pays very little attention to";
		}
		else if (r_ptr->sleep > 75)
		{
			act = "pays little attention to";
		}
		else if (r_ptr->sleep > 45)
		{
			act = "tends to overlook";
		}
		else if (r_ptr->sleep > 25)
		{
			act = "takes quite a while to see";
		}
		else if (r_ptr->sleep > 10)
		{
			act = "takes a while to see";
		}
		else if (r_ptr->sleep > 5)
		{
			act = "is fairly observant of";
		}
		else if (r_ptr->sleep > 3)
		{
			act = "is observant of";
		}
		else if (r_ptr->sleep > 1)
		{
			act = "is very observant of";
		}
		else if (r_ptr->sleep > 0)
		{
			act = "is vigilant for";
		}
		else
		{
			act = "is ever vigilant for";
		}

		roff(format("%^s %s intruders, which %s may notice from %d feet.  ",
			wd_he[msex], act, wd_he[msex], 10 * r_ptr->aaf));
	}


	/* Drops gold and/or items */
	if (r_ptr->r_drop_gold || r_ptr->r_drop_item)
	{
		/* No "n" needed */
		sin = FALSE;

		/* Intro */
		roff(format("%^s may carry", wd_he[msex]));

		/* Count maximum drop */
		n = MAX(r_ptr->r_drop_gold, r_ptr->r_drop_item);

		/* One drop (may need an "n") */
		if (n == 1)
		{
			roff(" a");
			sin = TRUE;
		}

		/* Two drops */
		else if (n == 2)
		{
			roff(" one or two");
		}

		/* Many drops */
		else
		{
			roff(format(" up to %d", n));
		}


		/* Great */
		if (flags1 & (RF1_DROP_GREAT))
		{
			p = " exceptional";
		}

		/* Good (no "n" needed) */
		else if (flags1 & (RF1_DROP_GOOD))
		{
			p = " good";
			sin = FALSE;
		}

		/* Okay */
		else
		{
			p = NULL;
		}


		/* Objects */
		if (r_ptr->r_drop_item)
		{
			/* Handle singular "an" */
			if (sin) roff("n");
			sin = FALSE;

			/* Dump "object(s)" */
			if (p) roff(p);
			roff(" object");
			if (n != 1) roff("s");

			/* Conjunction replaces variety, if needed for "gold" below */
			p = " or";
		}

		/* Treasures */
		if (r_ptr->r_drop_gold)
		{
			/* Cancel prefix */
			if (!p) sin = FALSE;

			/* Handle singular "an" */
			if (sin) roff("n");
			sin = FALSE;

			/* Dump "treasure(s)" */
			if (p) roff(p);
			roff(" treasure");
			if (n != 1) roff("s");
		}

		/* End this sentence */
		roff(".  ");
	}


	/* Count the number of "known" attacks */
	for (n = 0, m = 0; m < 4; m++)
	{
		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Count known attacks */
		if (r_ptr->r_blows[m]) n++;
	}

	/* Examine (and count) the actual attacks */
	for (r = 0, m = 0; m < 4; m++)
	{
		int method, effect, d1, d2;

		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Skip unknown attacks */
		if (!r_ptr->r_blows[m]) continue;


		/* Extract the attack info */
		method = r_ptr->blow[m].method;
		effect = r_ptr->blow[m].effect;
		d1 = r_ptr->blow[m].d_dice;
		d2 = r_ptr->blow[m].d_side;


		/* No method yet */
		p = NULL;

		/* Acquire the method */
		switch (method)
		{
		case RBM_HIT:	p = "hit"; break;
		case RBM_TOUCH:	p = "touch"; break;
		case RBM_PUNCH:	p = "punch"; break;
		case RBM_KICK:	p = "kick"; break;
		case RBM_CLAW:	p = "claw"; break;
		case RBM_BITE:	p = "bite"; break;
		case RBM_STING:	p = "sting"; break;
		case RBM_SLASH:	p = "slash"; break;
		case RBM_BUTT:	p = "butt"; break;
		case RBM_CRUSH:	p = "crush"; break;
		case RBM_ENGULF:	p = "engulf"; break;
		case RBM_CHARGE: p = "charge";   break;
		case RBM_CRAWL:	p = "crawl on you"; break;
		case RBM_DROOL:	p = "drool on you"; break;
		case RBM_SPIT:	p = "spit"; break;
		case RBM_XXX3:	break;
		case RBM_GAZE:	p = "gaze"; break;
		case RBM_WAIL:	p = "wail"; break;
		case RBM_SPORE:	p = "release spores"; break;
		case RBM_WORSHIP: p = "hero worship";	break;
		case RBM_BEG:	p = "beg"; break;
		case RBM_INSULT:	p = "insult"; break;
		case RBM_MOAN:	p = "moan"; break;
		case RBM_SHOW:  p = "sing"; break;
		case RBM_SWIPE: p = "swipe"; break;
		}


		/* Default effect */
		q = NULL;

		/* Acquire the effect */
		switch (effect)
		{
		case RBE_HURT:	q = "attack"; break;
		case RBE_POISON:	q = "poison"; break;
		case RBE_UN_BONUS:	q = "disenchant"; break;
		case RBE_UN_POWER:	q = "drain charges"; break;
		case RBE_EAT_GOLD:	q = "steal gold"; break;
		case RBE_EAT_ITEM:	q = "steal items"; break;
		case RBE_EAT_FOOD:	q = "eat your food"; break;
		case RBE_EAT_LITE:	q = "absorb light"; break;
		case RBE_ACID:	q = "shoot acid"; break;
		case RBE_ELEC:  q = "electrocute"; break;
		case RBE_FIRE:	q = "burn"; break;
		case RBE_COLD:	q = "freeze"; break;
		case RBE_BLIND:	q = "blind"; break;
		case RBE_CONFUSE:	q = "confuse"; break;
		case RBE_TERRIFY:	q = "terrify"; break;
		case RBE_PARALYZE:	q = "paralyze"; break;
		case RBE_LOSE_STR:	q = "reduce strength"; break;
		case RBE_LOSE_INT:	q = "reduce intelligence"; break;
		case RBE_LOSE_WIS:	q = "reduce wisdom"; break;
		case RBE_LOSE_DEX:	q = "reduce dexterity"; break;
		case RBE_LOSE_CON:	q = "reduce constitution"; break;
		case RBE_LOSE_CHA:	q = "reduce charisma"; break;
		case RBE_LOSE_ALL:	q = "reduce all stats"; break;
		case RBE_SHATTER:	q = "shatter"; break;
		case RBE_EXP_10:	q = "lower experience (by 10d6+)"; break;
		case RBE_EXP_20:	q = "lower experience (by 20d6+)"; break;
		case RBE_EXP_40:	q = "lower experience (by 40d6+)"; break;
		case RBE_EXP_80:	q = "lower experience (by 80d6+)"; break;
		}


		/* Introduce the attack description */
		if (!r)
		{
			roff(format("%^s can ", wd_he[msex]));
		}
		else if (r < n-1)
		{
			roff(", ");
		}
		else
		{
			roff(", and ");
		}


		/* Hack -- force a method */
		if (!p) p = "do something weird";

		/* Describe the method */
		roff(p);


		/* Describe the effect (if any) */
		if (q)
		{
			/* Describe the attack type */
			roff(" to ");
			roff(q);

			/* Describe damage (if known) */
			if (d1 && d2 && know_damage(r_idx, m))
			{
				/* Display the damage */
				roff(" with damage");
				roff(format(" %dd%d", d1, d2));
			}
		}


		/* Count the attacks as printed */
		r++;
	}


	if (r)
	{
		/* Now mention attack speed */
		switch (r) /* Use correct grammar */
		{
		case 1:
			{
				roff(format(", which %s does",wd_he[msex]));
				break;
			}
		case 2:
			{
				roff(format(" - both of which %s does",wd_he[msex]));
				break;
			}
		default:
			{
				roff(format(" - all of which %s does",wd_he[msex]));
				break;
			}
		}

		if(r_ptr->num_blows > 8)
		{
			roff(" with incredible speed");
		}
		else if(r_ptr->num_blows > 5)
		{
			roff(" extremely quickly");
		}
		else if(r_ptr->num_blows > 3)
		{
			roff(" very quickly");
		}
		else if(r_ptr->num_blows > 2)
		{
			roff(" quickly");
		}
		else if(r_ptr->num_blows > 1)
		{
			roff(" at a moderate speed");
		}
		else
		{
			roff(" slowly");
		}

		/* Finish sentence above */
		roff(".  ");
	}

	/* Notice lack of attacks */
	else if (flags1 & (RF1_NEVER_BLOW))
	{
		roff(format("%^s has no physical attacks.  ", wd_he[msex]));
	}

	/* Or describe the lack of knowledge */
	else
	{
		roff(format("Nothing is known about %s attack.  ", wd_his[msex]));
	}


	/* Notice "Quest" monsters */
	if (flags1 & (RF1_GUARDIAN))
	{
		roff("You feel an intense desire to kill this monster...  ");
	}
	/* Notice monsters that _have_ to be avoided */
	if (flags7 & (RF7_ANNOYED))
	{
		c_roff(TERM_RED, format("%^s considers you an annoyance.", wd_he[msex]),-1,-1,0);
	}
	/* Notice monsters that are in the grip of the Lover's Storm */
	if (flags7 & (RF7_STORM))
	{
		roff( format("%^s is blown about to and fro by a violent storm.", wd_he[msex]));
	}	 
	/* Notice monsters that never die permanently */
	if (flags7 & (RF7_REBORN))
	{
		roff( format("%^s recovers from fatal wounds.", wd_he[msex]));
	}	     
	
	/* All done */
	roff("\n");


	/* Hack -- Restore monster memory */
	if (debug_know)
	{
		/* Restore memory */
		*r_ptr = save_mem;
	}
}





/*
* Hack -- Display the "name" and "attr/chars" of a monster race
*/
static void roff_top(int r_idx)
{
	monster_race	*r_ptr = &r_info[r_idx];

	byte		a1, a2;
	char		c1, c2;


	/* Access the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Access the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;

	/* Hack -- fake monochrome */
	if (!use_colour) a1 = TERM_WHITE;
	if (!use_colour) a2 = TERM_WHITE;


	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		Term_addstr(-1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

	/* Append the "standard" attr/char info */
	Term_addstr(-1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr(-1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr(-1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	Term_addstr(-1, TERM_WHITE, "'):");
}



/*
* Hack -- describe the given monster race at the top of the screen
*/
void screen_roff(int r_idx)
{
	/* Flush messages */
	msg_print(NULL);

	/* Begin recall */
	Term_erase(0, 1, 255);

	/* Recall monster */
	roff_aux(r_idx);

	/* Describe monster */
	roff_top(r_idx);
}




/*
* Hack -- describe the given monster race in the current "term" window
*/
void display_roff(int r_idx)
{
	int y;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Recall monster */
	roff_aux(r_idx);

	/* Describe monster */
	roff_top(r_idx);
}

/*
 * Hack -- show a list of the visible monsters in the current "term" window
 */
void display_visible_items(void)
{
	int i, j, gap, len, count = 0;
	int items = 0;

	item_list_entry *what;
	item_list_entry temp_item;
    
	/* Clear */
	Term_clear();

	/* Allocate the "who" array */
	C_MAKE(what, o_max, item_list_entry);

	/* Scan objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects and held objects*/
		if (!o_ptr->k_idx || o_ptr->held_m_idx) continue;

		/* Location, which we don't us any more right now
		y = o_ptr->iy;
		x = o_ptr->ix;
		*/

		/* Detect "real" objects */
		if (o_ptr->marked)
		{
			if(items)
			{
				int counter = 0;
				int found   = 0;
				for( counter = 0; counter < items ; counter++){
					object_type *q_ptr = &o_list[what[counter].o_idx];
					if( o_ptr->tval >= TV_MIRACLES_BOOK && q_ptr->tval == o_ptr->tval && q_ptr->sval == o_ptr->sval  )
					{
						what[counter].count += o_ptr->number;
						count = count + o_ptr->number;
						found = 1;
						break;
					}
					if( q_ptr->tval == o_ptr->tval && q_ptr->sval == o_ptr->sval && !object_known_p( q_ptr ) && !object_known_p( o_ptr ) )
					{
						what[counter].count += o_ptr->number;
						count = count + o_ptr->number;
						found = 1;
						break;
					}
				}
				if(found)
					continue;
			}

			/* Print it out, baby */
			what[items].o_idx = i;
			what[items].object_value = object_value( o_ptr );
			what[items].count = o_ptr->number;
			count = count + o_ptr->number;
			items++;
		}
	}

	if( items )
	{
		int w, h, num = 0;

		/* Sort the items according to cost */
		len = items;
		for (gap = len / 2; gap > 0; gap /= 2)
			for (i = gap; i < len; i++)
				for (j = i-gap; j >= 0 && what[j].object_value < what[j + gap].object_value ; j -= gap) {
					temp_item = what[j];
					what[j] =  what[j + gap];
					what[j+gap] = temp_item;
				}
		/* Print them */
		(void)Term_get_size(&w, &h);

		c_prt(TERM_WHITE,format("You can see %d item%s", count, (count != 1 ? "s:" : ":")), 0, 0);

		for (i = 0; i < items && i < h-1 ; i++){
			object_type *o_ptr = &o_list[ what[i].o_idx ];
			char o_name[80];
			byte number_swap;

			/* Default Colour */
			byte attr = TERM_WHITE;

			/* Naughty, naughty, we swap the count ;] */
			number_swap = o_ptr->number;
			/* Just in case, make sure large counts do not mess up Hellband */
			o_ptr->number = (what[i].count>255)?255:(byte_hack)what[i].count;

			/* Get a description */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Naughty, naughty, we swap the count back ;] */
			o_ptr->number = number_swap;

			c_prt( attr, o_name , (num % (h - 1)) + 1, (num / (h - 1) * 26));
			num++;
		}
	}
	else
	{
		c_prt(TERM_WHITE,"You see no items.",0,0);
	}
	FREE(what);
}

/*
 * Hack -- show a list of the visible monsters in the current "term" window
 */
void display_visible(void)
{
	int i, j;
	int c = 0;
	int items = 0;
	
	monster_list_entry *who;

	byte mflag_show = MFLAG_VIEW | MFLAG_MARK | MFLAG_SHOW;
    
	/* Clear */
	Term_clear();
    
	/* XXX Hallucination - no monster list */
	if (p_ptr->image)
	{		
		c_prt(TERM_WHITE,"You see a lot of pretty colours.",0,0);
		return;
	}
    
	/* Allocate the "who" array */
	C_MAKE(who, m_max, monster_list_entry);
    
	/* Count up the number visible in each race, consider using m_cnt */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
        
		bool found = FALSE;
        
		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;
        
		/* Skip unseen monsters */
		if (!(m_ptr->ml || (m_ptr->mflag & mflag_show )) && m_ptr->hp > 0) continue;
        
		/* Increase for this race */
  		if (items)
		{
			for (j = 0; j < items; j++)
			{
				if ( who[j].r_idx == m_ptr->r_idx)
				{
					who[j].amount++;
					found = TRUE;
					break;
				}
			}
		}
		
		if (!found)
		{
			who[items].r_idx = m_ptr->r_idx;
			who[items].amount = 1;
			items++;
		}
        
		/* Increase total Count */
		c++;
	}
    
	/* Are monsters visible? */
	if (items)
	{
		int w, h, num = 0;
		u16b why = 1;
        
		/* First, sort the monsters by expereince*/
		ang_sort_comp = ang_sort_comp_visible_hook;
		ang_sort_swap = ang_sort_swap_visible_hook;
        
		/* Sort the array */
		ang_sort(who, &why, items);
        
		/* Then, display them */
		(void)Term_get_size(&w, &h);
        
		c_prt(TERM_WHITE,format("You can see %d monster%s", c, (c > 1 ? "s:" : ":")), 0, 0);
        
		/* Print the monsters in reverse order */
		for (i = items - 1; i >= 0; i--)
		{
			monster_race *r_ptr =  &r_info[who[i].r_idx];
            char m_name[80];
            
			/* Default Colour */
			byte attr = TERM_WHITE;
            
			/* Uniques are red */
			if  (r_ptr->flags1 & (RF1_UNIQUE)) 
                attr = TERM_L_RED;
            
			/* Have we ever killed one? */
			else if (r_ptr->r_tkills)
			{
				if (r_ptr->level > p_ptr->max_dun_level  )
				{
					attr = TERM_VIOLET;
				}
			}
			else
			{
                attr = TERM_SLATE;
			}			
            
            strcpy(m_name, (r_name + r_ptr->name));
                        
			/* Dump the monster symbol and name */
			c_prt(r_ptr->x_attr, format("%c ",r_ptr->x_char), (num % (h - 1)) + 1, (num / (h - 1) * 26));
			if (who[i].amount == 1)
			{
                 c_prt(attr, m_name, (num % (h - 1)) + 1,  (num / (h - 1) * 26) + 2);
			}
			else
			{
				c_prt(attr, format("%s (x%d)", m_name, who[i].amount), (num % (h - 1)) + 1, ((num / (h - 1)) * 26) + 2);
			}
			num++;
		}
	}
    
	else
	{
		c_prt(TERM_WHITE,"You see no monsters.",0,0);
	}
    
	/* XXX XXX Free the "who" array */
	FREE(who);
}


