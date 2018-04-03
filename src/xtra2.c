/* File: effects.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 *
 * James E. Wilson and Robert A. Koeneke and Ben Harrison have released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2),
 * or under the terms of the traditional Angband license.
 */

#include "angband.h"

extern void do_poly_self();
extern void do_poly_wounds();
extern bool curse_weapon();
extern bool curse_armour();
extern void random_resistance(object_type * q_ptr, bool is_scroll, int specific);

/* This is a hack apparently, I wouldnt know ;)  */
extern void do_cmd_wiz_cure_all(void);

/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_stun( int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	if (p_ptr->prace == GUARDIAN)
		v = 0;

	/* Knocked out */
	if (p_ptr->stun > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p_ptr->stun > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p_ptr->stun > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Knocked out */
	if (v > 100)
	{
		new_aux = 3;
	}

	/* Heavy stun */
	else if (v > 50)
	{
		new_aux = 2;
	}

	/* Stun */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Stun */
			case 1:
				msg_print("You have been stunned.");
				break;

				/* Heavy stun */
			case 2:
				msg_print("You have been heavily stunned.");
				break;

				/* Knocked out */
			case 3:
				msg_print("You have been knocked out.");
				break;
		}

		if (randint(1000)<v || randint(16)==1)
		{

			msg_print("A vicious blow hits your head.");
			if(randint(3)==1)
			{
				if (!p_ptr->sustain_int) { (void) do_dec_stat(A_INT); }
				if (!p_ptr->sustain_wis) { (void) do_dec_stat(A_WIS); }
			}
			else if (randint(2)==1)
			{
				if (!p_ptr->sustain_int) { (void) do_dec_stat(A_INT); }
			}
			else
			{
				if (!p_ptr->sustain_wis) { (void) do_dec_stat(A_WIS); }
			}
		}

		/* Notice */
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
				msg_print("You are no longer stunned.");
				if (disturb_state) disturb(0, 0);
					break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->stun = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_STUN);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_cut(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	if (p_ptr->prace == GUARDIAN || p_ptr->prace == SKELETON ||
		p_ptr->prace == SPECTRE )
		v = 0;
	else if (p_ptr->prace == MUMMY && p_ptr->lev > 11)
		v = 0;

	/* Mortal wound */
	if (p_ptr->cut > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p_ptr->cut > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p_ptr->cut > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p_ptr->cut > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p_ptr->cut > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p_ptr->cut > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p_ptr->cut > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Mortal wound */
	if (v > 1000)
	{
		new_aux = 7;
	}

	/* Deep gash */
	else if (v > 200)
	{
		new_aux = 6;
	}

	/* Severe cut */
	else if (v > 100)
	{
		new_aux = 5;
	}

	/* Nasty cut */
	else if (v > 50)
	{
		new_aux = 4;
	}

	/* Bad cut */
	else if (v > 25)
	{
		new_aux = 3;
	}

	/* Light cut */
	else if (v > 10)
	{
		new_aux = 2;
	}

	/* Graze */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Graze */
			case 1:
				msg_print("You have been given a graze.");
				break;

				/* Light cut */
			case 2:
				msg_print("You have been given a light cut.");
				break;

				/* Bad cut */
			case 3:
				msg_print("You have been given a bad cut.");
				break;

				/* Nasty cut */
			case 4:
				msg_print("You have been given a nasty cut.");
				break;

				/* Severe cut */
			case 5:
				msg_print("You have been given a severe cut.");
				break;

				/* Deep gash */
			case 6:
				msg_print("You have been given a deep gash.");
				break;

				/* Mortal wound */
			case 7:
				msg_print("You have been given a mortal wound.");
				break;
		}

		/* Notice */
		notice = TRUE;

		if (randint(1000)<v || randint(16)==1)
		{
			if(!p_ptr->sustain_cha)
			{
				msg_print("You have been horribly scarred.");

				do_dec_stat(A_CHA);
			}
		}

	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
				msg_print("You are no longer bleeding.");
				if (disturb_state) disturb(0, 0);
					break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->cut = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_CUT);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

bool set_timed_effect( byte effect , int v )
{
	/* Is there a noticable change ( nothing is noticed only if duration is increased */
	bool notice = FALSE;
	/* Access data of the timed effect */
	timed_type *te_ptr = &timed[effect];

	/*Some timed effects are not that easy*/
	if( effect == TIMED_CUT )
		return set_cut(v);
	if( effect == TIMED_STUN )
		return set_stun(v);
	if( effect == TIMED_OPPOSE_CONF )
		set_timed_effect( TIMED_CONFUSED , 0);
	if( effect == TIMED_OPPOSE_FEAR )
		set_timed_effect( TIMED_AFRAID , 0);

	/* paranoid , check the effect index  */
	if( effect >= TIMED_COUNT )
	{
		msg_note("Trying to access an unknown timed effect");
	}

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Start effect */
	if (v)
	{
		if (!(*( te_ptr->timer )))
		{
			msg_print( te_ptr->gain );
			notice = TRUE;
		}
	}

	/* Shut down effect */
	else
	{
		if ((*( te_ptr->timer )))
		{
			msg_print( te_ptr->lose );
			notice = TRUE;
		}
	}

	/* Use the value */
	*( te_ptr->timer ) = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Update stuff */
	p_ptr->update |= te_ptr->update;

	/* Redraw stuff */
	p_ptr->redraw |= te_ptr->redraw;

	/* Window stuff, Hack !! */
	if( effect == TIMED_BLIND || effect == TIMED_IMAGE || effect == TIMED_INVULN || effect == TIMED_WRAITH_FORM )
		p_ptr->window |= (PW_OVERHEAD);

    /* Window stuff, hack if we need to update monsters, we better update the visible monsters window*/
    if( te_ptr->update & PU_MONSTERS )
		p_ptr->window |= (PW_VISIBLE);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
* Set "p_ptr->food", notice observable changes
*
* The "p_ptr->food" variable can get as large as 20000, allowing the
* addition of the most "filling" item, Ambrosia, which adds
* 7500 food units, without overflowing the 32767 maximum limit.
*
* Perhaps we should disturb the player with various messages,
* especially messages about hunger status changes.  XXX XXX XXX
*
* Digestion of food is handled in "dungeon.c", in which, normally,
* the player digests about 20 food units per 100 game turns, more
* when "fast", more when "regenerating", less with "slow digestion",
* but when the player is "gorged", he digests 100 food units per 10
* game turns, or a full 1000 food units per 100 game turns.
*
* Note that the player's speed is reduced by 10 units while gorged,
* so if the player eats a single food ration (5000 food units) when
* full (15000 food units), he will be gorged for (5000/100)*10 = 500
* game turns, or 500/(100/5) = 25 player turns (if nothing else is
* affecting the player speed).
*/
bool set_food(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		old_aux = 0;
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		old_aux = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		old_aux = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		old_aux = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		old_aux = 4;
	}

	/* Gorged */
	else
	{
		old_aux = 5;
	}

	/* Fainting / Starving */
	if (v < PY_FOOD_FAINT)
	{
		new_aux = 0;
	}

	/* Weak */
	else if (v < PY_FOOD_WEAK)
	{
		new_aux = 1;
	}

	/* Hungry */
	else if (v < PY_FOOD_ALERT)
	{
		new_aux = 2;
	}

	/* Normal */
	else if (v < PY_FOOD_FULL)
	{
		new_aux = 3;
	}

	/* Full */
	else if (v < PY_FOOD_MAX)
	{
		new_aux = 4;
	}

	/* Gorged */
	else
	{
		new_aux = 5;
	}

	/* Food increase */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Weak */
		case 1:
			msg_print("You are still weak.");
			break;

			/* Hungry */
		case 2:
			msg_print("You are still hungry.");
			break;

			/* Normal */
		case 3:
			msg_print("You are no longer hungry.");
			break;

			/* Full */
		case 4:
			msg_print("You are full!");
			break;

			/* Bloated */
		case 5:
			msg_print("You have gorged yourself!");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Fainting / Starving */
		case 0:
			msg_print("You are getting faint from hunger!");
			break;

			/* Weak */
		case 1:
			msg_print("You are getting weak from hunger!");
			break;

			/* Hungry */
		case 2:
			msg_print("You are getting hungry.");
			break;

			/* Normal */
		case 3:
			msg_print("You are no longer full.");
			break;

			/* Full */
		case 4:
			msg_print("You are no longer gorged.");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_HUNGER);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
* Advance experience levels and print experience
*/
void check_experience(void)
{
	bool level_reward = FALSE;
	bool level_corruption = FALSE;

	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;


	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Handle stuff */
	handle_stuff();

	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
		(p_ptr->exp < (player_exp[p_ptr->lev-2] *
		p_ptr->expfact / 100L)))
	{
		/* Lose a level */
		p_ptr->lev--;
		lite_spot(py, px);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle stuff */
		handle_stuff();
	}

	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) &&
		(p_ptr->exp >= (player_exp[p_ptr->lev-1] *
		p_ptr->expfact / 100L)))
	{
		/* Gain a level */
		p_ptr->lev++;
		lite_spot(py, px);

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_plv)
		{
			p_ptr->max_plv = p_ptr->lev;
			if ((p_ptr->pclass == CLASS_HELL_KNIGHT) || (p_ptr->pclass == CLASS_WARLOCK))
			{
				level_reward = TRUE;
			}
			if (p_ptr->muta2 & COR2_PATRON)
			{
				level_reward = TRUE;
			}
			if (p_ptr->prace == DEVILSPAWN)
			{
				if (randint(5)==1) level_corruption = TRUE;
			}
		}

		/* Sound */
		sound(SOUND_LEVEL);

		/* Message */
		msg_format("Welcome to level %d.", p_ptr->lev);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->window |= (PW_SPELL);

		/* Handle stuff */
		handle_stuff();

		/* Give the player the good news */
		msg_format("You feel magically reinvigorated.");
		/* Cure the player completely */
		(void)do_cmd_wiz_cure_all();

		if(level_reward)
		{
			gain_level_reward(0);
			level_reward = FALSE;
		}
		if (level_corruption)
		{
			msg_print("You feel different...");
			(void)gain_corruption(0);
			level_corruption = FALSE;
		}
	}
}


/*
* Gain experience
*/
void gain_exp(s32b amount)
{
	/* Have the weapon gain some xp first */
	{
		object_type *o_ptr = &inventory[INVEN_WIELD];
		u32b f1, f2, f3;
		object_flags(o_ptr, &f1, &f2, &f3);
		if( o_ptr->k_idx && o_ptr->tval && (f3 & TR3_XP) )
		{
			o_ptr->exp += 2 * amount / 3;
			check_experience_obj( o_ptr );
		}
	}

	/* Gain some experience */
	p_ptr->exp += amount;

	/* Slowly recover from experience drainage */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Gain max experience (20%) (was 10%) */
		p_ptr->max_exp += amount / 5;
	}

	/* Check Experience */
	check_experience();
}


/*
* Lose experience
*/
void lose_exp(s32b amount)
{
	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience();
}




/*
* Hack -- Return the "automatic coin type" of a monster race
* Used to allocate proper treasure when "Creeping coins" die
*
* XXX XXX XXX Note the use of actual "monster names"
*/
static int get_coin_type(monster_race *r_ptr)
{
	cptr name = (r_name + r_ptr->name);

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (2);
		if (strstr(name, " silver ")) return (5);
		if (strstr(name, " gold ")) return (10);
		if (strstr(name, " bronze ")) return (16);
		if (strstr(name, " adamantite ")) return (17);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (2);
		if (strstr(name, "Silver ")) return (5);
		if (strstr(name, "Gold ")) return (10);
		if (strstr(name, "Bronze ")) return (16);
		if (strstr(name, "Adamantite ")) return (17);
	}

	/* Assume nothing */
	return (0);
}

void calculate_xp(monster_race *r_ptr, s32b *exp , s32b *exp_frac)
{
	s32b div;

	/* Start from zero to prevent all kinds of bugs */
	*exp = 0;
	*exp_frac = 0;

	/*No exp for reborns */
	if( (r_ptr->flags7 & RF7_REBORN) && ( r_ptr->r_pkills > 0 ) ) return;

	/* Maximum player level */
	div = 10*p_ptr->max_plv;

	if (r_ptr->r_pkills >= 19) div = div * 2; /* Half experience for common monsters */
	if (r_ptr->r_pkills ==  0) div = div / 3; /* Triple experience for first kill */
	if (r_ptr->r_pkills ==  1) div = div / 2; /* Double experience for third kill */
	if (r_ptr->r_pkills ==  2) div = div / 2; /* Double experience for second kill */

	/* don't divide by 0 */
	if (div < 1) div = 1;

	/* Give some experience for the kill */
	*exp = ((long)r_ptr->mexp * r_ptr->level*10) / div;

	/* Handle fractional experience */
	*exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % div) * 0x10000L / div);
}


/*
* Handle the "death" of a monster.
*
* Disperse treasures centered at the monster location based on the
* various flags contained in the monster flags fields.
*
* Check for "Quest" completion when a quest monster is killed.
*
* Note that only the player can induce "monster_death()" on Uniques.
* Thus (for now) all Quest monsters should be Uniques.
*
* Note that monsters can now carry objects, and when a monster dies,
* it drops all of its objects, which may disappear in crowded rooms.
*/
void monster_death(int m_idx)
{
	int i, j, y, x, ny, nx;

	int dump_item = 0;
	int dump_gold = 0;

	int number = 0;
	int total = 0;
	int q_idx = 0;

	bool quest = FALSE;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool visible = (m_ptr->ml || (r_ptr->flags1 & (RF1_UNIQUE)));

	bool good = (r_ptr->flags1 & (RF1_DROP_GOOD)) ? TRUE : FALSE;
	bool great = (r_ptr->flags1 & (RF1_DROP_GREAT)) ? TRUE : FALSE;

	bool do_gold = (!(r_ptr->flags1 & (RF1_ONLY_ITEM)));
	bool do_item = (!(r_ptr->flags1 & (RF1_ONLY_GOLD)));

	bool cloned = FALSE;

	int force_coin = get_coin_type(r_ptr);

	object_type forge;
	object_type *q_ptr;

    /* Update monster list window */
	p_ptr->window |= (PW_VISIBLE);
	p_ptr->window |= (PW_MONSTER);

	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	if (m_ptr->smart &(SM_CLONED))
		cloned = TRUE;

	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		q_ptr = &forge;

		/* Copy the object */
		object_copy(q_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Drop it */
		drop_near(q_ptr, -1, y, x);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;

	/* Mega^2-hack -- destroying the Stormbringer gives it us! */
	if (strstr((r_name + r_ptr->name),"Stormbringer"))
	{
		/* Get local object */
		q_ptr = &forge;

		/* Prepare to make the Stormbringer */
		object_prep(q_ptr, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));

		/* Mega-Hack -- Name the sword  */

		q_ptr->art_name = quark_add("'Stormbringer'");
		q_ptr->to_h = 16;
		q_ptr->to_d = 16;
		q_ptr->ds = 6;
		q_ptr->dd = 6;
		q_ptr->pval = 2;

		q_ptr->art_flags1 |= ( TR1_VAMPIRIC | TR1_STR | TR1_CON );
		q_ptr->art_flags2 |= ( TR2_FREE_ACT | TR2_HOLD_LIFE |
			TR2_RES_NEXUS | TR2_RES_CHAOS | TR2_RES_NETHER |
			TR2_RES_CONF ); /* No longer resist_disen */
		q_ptr->art_flags3 |= (TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
			TR3_IGNORE_FIRE | TR3_IGNORE_COLD);
		/* Just to be sure */

		q_ptr->art_flags3 |= TR3_NO_TELE; /* How's that for a downside? */

		/* For game balance... */
		q_ptr->art_flags3 |= (TR3_CURSED | TR3_HEAVY_CURSE);
		q_ptr->ident |= IDENT_CURSED;

		if (randint(2)==1) q_ptr->art_flags3 |= (TR3_DRAIN_EXP);
		q_ptr->art_flags3 |= (TR3_AGGRAVATE);

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, y, x);

	}

	/* Mega-Hack -- drop "winner" treasures */
	else if (r_ptr->flags1 & (RF1_DROP_CHOSEN))
	{
		if (strstr((r_name + r_ptr->name),"Lucifer"))
		{

			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Mighty Hammer of Worlds" */
			object_prep(q_ptr, lookup_kind(TV_HAFTED, SV_WORLDS));

			/* Mega-Hack -- Mark this item as "Mighty Hammer of Worlds" */
			q_ptr->name1 = ART_HAMMER_ABADDON;

			/* Mega-Hack -- Actually create "Mighty Hammer of Worlds" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);

			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Crown of the Universe" */
			object_prep(q_ptr, lookup_kind(TV_CROWN, SV_SEVENTH));

			/* Mega-Hack -- Mark this item as "Crown of the Universe" */
			q_ptr->name1 = ART_SEVENTH;

			/* Mega-Hack -- Actually create "Crown of the Universe" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
		}
		else
		{
			byte a_idx = 0;
			int chance = 0;
			int I_kind = 0;
#ifdef GO_GROO
            Alas , Groo is no more !
			if (strstr((r_name + r_ptr->name),"Groo"))
			{
				a_idx = ART_GROO; /* This is commented out  */
				chance = 75;
			}
#endif

			if ((a_idx > 0) && ((randint(99)<chance) || (debug_mode)))
			{
				if (a_info[a_idx].cur_num == 0)
				{
					artefact_type *a_ptr = &a_info[a_idx];

					/* Get local object */
					q_ptr = &forge;

					/* Wipe the object */
					object_wipe(q_ptr);

					/* Acquire the "kind" index */
					I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

					/* Create the artefact */
					object_prep(q_ptr, I_kind);

					/* Save the name */
					q_ptr->name1 = a_idx;

					/* Extract the fields */
					q_ptr->pval = a_ptr->pval;
					q_ptr->ac = a_ptr->ac;
					q_ptr->dd = a_ptr->dd;
					q_ptr->ds = a_ptr->ds;
					q_ptr->to_a = a_ptr->to_a;
					q_ptr->to_h = a_ptr->to_h;
					q_ptr->to_d = a_ptr->to_d;
					q_ptr->weight = a_ptr->weight;

					/* Hack -- acquire "cursed" flag */
					if (a_ptr->flags3 & (TR3_CURSED)) q_ptr->ident |= (IDENT_CURSED);

					random_artefact_resistance(q_ptr);

					a_info[a_idx].cur_num = 1;

					/* Drop the artefact from heaven */
					drop_near(q_ptr, -1, y, x);


				}
			}
		}
	}


	/* Determine how much we can drop */
	if ((r_ptr->flags1 & (RF1_DROP_60)) && (rand_int(100) < 60)) number++;
	if ((r_ptr->flags1 & (RF1_DROP_90)) && (rand_int(100) < 90)) number++;
	if (r_ptr->flags1 & (RF1_DROP_1D2)) number += damroll(1, 2);
	if (r_ptr->flags1 & (RF1_DROP_2D2)) number += damroll(2, 2);
	if (r_ptr->flags1 & (RF1_DROP_3D2)) number += damroll(3, 2);
	if (r_ptr->flags1 & (RF1_DROP_4D2)) number += damroll(4, 2);

	if (cloned) number = 0; /* Clones drop no stuff */
	if ((is_quest(dun_level)) && ((r_ptr->flags1 & RF1_GUARDIAN) || (r_ptr->flags1 & RF1_ALWAYS_GUARD)))
	{
		q_idx = get_quest_number ();
		q_list[q_idx].cur_num++;

		if (q_list[q_idx].cur_num == q_list[q_idx].max_num)
		{
			/* Drop at least 2 items (the stair will probably destroy one */
			number += 2;
			quest = TRUE;
			q_list[q_idx].level=0;
		}
	}

	/* Hack -- handle creeping coins */
	coin_type = force_coin;

	/* Average dungeon and monster levels */
	object_level = (dun_level + r_ptr->level) / 2;

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Get local object */
		q_ptr = &forge;

		/* Wipe the object */
		object_wipe(q_ptr);

		/* Make Gold, but not for first two quest items */
		if ((!quest || (j > 1)) && do_gold && (!do_item || (rand_int(100) < 50)))
		{
			/* Make some gold */
			if (!make_gold(q_ptr)) continue;

			/* Unique only golds should give loads of gold, make sure there is no overflow */
			if( ( r_ptr->flags1 & (RF1_UNIQUE) ) && (r_ptr->flags1 & (RF1_ONLY_GOLD)) )
				q_ptr->pval = (q_ptr->pval * q_ptr->pval > 25193 ) ? 25193 : q_ptr->pval * q_ptr->pval;

			/* XXX XXX XXX */
			dump_gold++;
		}

		/* Make Object */
		else
		{
			/* Make an object */
			if (!quest || (j>1))
			{
				if (!make_object(q_ptr, good, great)) continue;
			}
			else
			{
				/* The first two items for a quest monster are great */
				if (!make_object(q_ptr, TRUE, TRUE)) continue;
			}

			/* XXX XXX XXX */
			dump_item++;
		}

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, y, x);
	}

	/* Reset the object level */
	object_level = dun_level;

	/* Reset "coin" type */
	coin_type = 0;


	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

	/* Only process "Quest Monsters" */
	if (!((r_ptr->flags1 & RF1_GUARDIAN) || (r_ptr->flags1 & RF1_ALWAYS_GUARD))) return;

	/* Check if quest is complete (Heino Vander Sanden) */
	if (q_list[q_idx].cur_num != q_list[q_idx].max_num) return;

	/* No longer quest monster (Heino Vander Sanden) */
	r_ptr->flags1 ^= (RF1_GUARDIAN);

	/* Count incomplete quests (Heino Vander Sanden) */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		if (q_list[i].level || (q_list[i].cur_num != q_list[i].max_num)) total++;
	}

	/* Need some stairs */
	if (total)
	{
		/* Stagger around */
		while (!cave_valid_bold(y, x))
		{
			int d = 1;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d);

			/* Stagger */
			y = ny; x = nx;
		}

		/* XXX XXX XXX */
		delete_object(y, x);

		/* Explain the stairway */
		msg_print("A magical stairway appears...");

		/* Create stairs down */
		cave_set_feat(y, x, FEAT_MORE);

		/* Remember to update everything */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
	}
	/* Nothing left, game over... */
	else
	{
		/* Total winner */
		total_winner = TRUE;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_TITLE);

		/* Congratulations */
		msg_print("*** CONGRATULATIONS ***");
		msg_print("You have won the game!");
		msg_print("You may retire (commit suicide) when you are ready.");
	}
}




/*
* Decreases monsters hit points, handling monster death.
*
* We return TRUE if the monster has been killed (and deleted).
*
* We announce monster death (using an optional "death message"
* if given, and a otherwise a generic killed/destroyed message).
*
* Only "physical attacks" can induce the "You have slain" message.
* Missile and Spell attacks will induce the "dies" message, or
* various "specialized" messages.  Note that "You have destroyed"
* and "is destroyed" are synonyms for "You have slain" and "dies".
*
* Hack -- unseen monsters yield "You have killed it." message.
*
* Added fear (DGK) and check whether to print fear messages -CWS
*
* Genericized name, sex, and capitilization -BEN-
*
* As always, the "ghost" processing is a total hack.
*
* Hack -- we "delay" fear messages by passing around a "fear" flag.
*
* XXX XXX XXX Consider decreasing monster experience over time, say,
* by using "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))"
* instead of simply "(m_exp * m_lev) / (p_lev)", to make the first
* monster worth more than subsequent monsters.  This would also need
* to induce changes in the monster recall code.
*/
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	char tmp[1024];
	s32b new_exp, new_exp_frac;

	/* Redraw (later) if needed */
	if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];

		/* Extract monster name */
		monster_desc(m_name, m_ptr, 0);

		if (r_ptr->flags3 & (RF3_FALLEN_ANGEL))
		{
			msg_format("%^s discards this temporary mortal shell.",m_name);
			if (randint(5)==1 && dun_level > DIS_END )
			{
				u32b f1, f2, f3;
				object_type *weapon_ptr;
				char	o_name[80];
				int curses = 1 + randint(3);
				/* Scare the player */
				msg_format("Lucifer puts a terrible curse on you!", m_name);
				/* Weapons that are bane of Angels protect against the curse */
				weapon_ptr = &inventory[INVEN_WIELD];
				/* Extract the flags */
				object_flags(weapon_ptr, &f1, &f2, &f3);
				/* Angel bane ? */
				if( weapon_ptr->tval && ( f1 & ( TR1_KILL_ANGEL ) ) )
				{
					object_desc(o_name, weapon_ptr, TRUE, 3);
					msg_format("%^s throbs in your hands and protects you from the curse.", o_name);
				}
				else
				/* Oops, I feel bad for monks.. */
				{
					curse_equipment(100, 50);
					do { activate_ty_curse(); } while (--curses);\
				}
			}
		}

		/* Make a sound */
		sound(SOUND_KILL);

		/* Death by Missile/Spell attack */
		if (note)
		{
			msg_format("%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
			msg_format("You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags3 & (RF3_DEMON)) ||
			(r_ptr->flags3 & (RF3_UNDEAD)) ||
			(r_ptr->flags3 & (RF3_DEVIL)) ||
			(r_ptr->flags2 & (RF2_STUPID)) ||
			(r_ptr->flags3 & (RF3_NONLIVING)) ||
			(strchr("Evg", r_ptr->d_char)))
		{
			msg_format("You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			msg_format("You have slain %s.", m_name);
		}

		/* Get the experience for that monster with current player level */
		calculate_xp( r_ptr , &new_exp , &new_exp_frac );

		/*No exp or treasure for reborns */
		if(!((r_ptr->flags7 & (RF7_REBORN)) && r_ptr->r_pkills > 0 ))
		{

			/* Handle fractional experience */
			new_exp_frac += p_ptr->exp_frac;

			/* Keep track of experience, overflowing if needed */
			if (new_exp_frac >= 0x10000L)
			{
				new_exp++;
				p_ptr->exp_frac = (u16b)(new_exp_frac - 0x10000L);
			}
			else
			{
				p_ptr->exp_frac = (u16b)new_exp_frac;
			}

			/* Gain experience */
			gain_exp(new_exp);
			/* Generate treasure */
			monster_death(m_idx);
		}

		/* When the player kills a Unique, it stays dead */
		/* Except of course when it doesnt stay dead ;) */
		if (r_ptr->flags1 & (RF1_UNIQUE) &&
			!(r_ptr->flags7 & (RF7_REBORN))) r_ptr->max_num = 0;

		/* XXX XXX Mega-Hack -- allow another ghost later
		* Remove the slain bone file */
		if (m_ptr->r_idx == MAX_R_IDX-1)
		{
			r_ptr->max_num = 1;

			/* Delete the bones file */
			sprintf(tmp, "%s%sbone.%03d", ANGBAND_DIR_BONE, PATH_SEP, dun_level);

			fd_kill(tmp);
		}

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || (r_ptr->flags1 & (RF1_UNIQUE)))
		{
			/* Count kills this life */
			if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

			/* Count kills in all lives */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx,TRUE);

        /* Update monster list window */
        p_ptr->window |= (PW_VISIBLE);
		/* Dead monster, update potential xp decreases and stuff */
		p_ptr->window |= (PW_MONSTER);
		/* I hate sprinkling handle_stuffs until it works..*/
		handle_stuff();

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


#ifdef ALLOW_FEAR

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !(r_ptr->flags3 & (RF3_NO_FEAR)))
	{
		int		percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		* Run (sometimes) if at 10% or less of max hit points,
		* or (usually) when hit for half its current hit points
		*/
		if (((percentage <= 10) && (rand_int(10) < percentage)) ||
			((dam >= m_ptr->hp) && (rand_int(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (byte_hack)(randint(10) +
				(((dam >= m_ptr->hp) && (percentage > 7)) ?
				20 : ((11 - percentage) * 5)));
		}
	}

#endif

	/* Not dead yet */
	return (FALSE);
}


#define PANEL_SIZE 11
#define PANEL_SCROLL_WIDTH PANEL_SIZE *3
#define PANEL_SCROLL_HEIGHT PANEL_SIZE *2

/*
* Calculates current boundaries
* Called below and from "do_cmd_locate()".
*/
void panel_bounds(void)
{
	panel_row_min = panel_row * (SCREEN_HGT / 2);
	panel_row_max = panel_row_min + SCREEN_HGT - 1;
	panel_row_prt = panel_row_min - 1;

/* min used to be SCREEN_WID / 2, since SCREEN_WID used to be 66..  */
	panel_col_min = panel_col * (SCREEN_WID /2);
	panel_col_max = panel_col_min + SCREEN_WID - 1;
	panel_col_prt = panel_col_min /*- 13*/;
}

void panel_bounds_center(void)
{
	panel_row = panel_row_min / (SCREEN_HGT / 2);
	panel_row_max = panel_row_min + SCREEN_HGT - 1;
	panel_row_prt = panel_row_min - 1;

	panel_col = panel_col_min / (SCREEN_WID / 2);
	panel_col_max = panel_col_min + SCREEN_WID - 1;
	panel_col_prt = panel_col_min /*- 13*/;
}


/*
* Given an row (y) and col (x), this routine detects when a move
* off the screen has occurred and figures new borders. -RAK-
*
* "Update" forces a "full update" to take place.
*
* The map is reprinted if necessary, and "TRUE" is returned.
*/
void verify_panel(void)
{
	int y = py;
	int x = px;

	if ((centre_view) && !((no_centre_run && running)))
	{
		int prow_min;
		int pcol_min;

		int max_prow_min = max_panel_rows * (SCREEN_HGT / 2);
		int max_pcol_min = max_panel_cols * (SCREEN_WID / 2);

		/* Center vertically */
		prow_min = y - SCREEN_HGT / 2;
		if (prow_min > max_prow_min) prow_min = max_prow_min;
		else if (prow_min < 0) prow_min = 0;

		/* Center horizontally */
		pcol_min = x - SCREEN_WID / 2;
		if (pcol_min > max_pcol_min) pcol_min = max_pcol_min;
		else if (pcol_min < 0) pcol_min = 0;

		/* Check for "no change" */
		if ((prow_min == panel_row_min) && (pcol_min == panel_col_min)) return;

		/* Save the new panel info */
		panel_row_min = prow_min;
		panel_col_min = pcol_min;

		/* Recalculate the boundaries */
		panel_bounds_center();
	}

	else
	{

		int prow = panel_row;
		int pcol = panel_col;;

		/*
		* In the olden days, a 25 line screen used to scroll 2 spaces of the horizontal edges,
		* and a 80 column screen used to scroll 4 spaces of the vertical edges
		* These numbers seem stupid now, so why not use the player view distance?
		* Though that actually does not really work for the original size screen..
		* So we just scale the original values
		*/

		int row_trigger = SCREEN_HGT * 2 / 25;
		int col_trigger = SCREEN_WID * 4 / 80;

		/* Scroll screen when 2 grids from top/bottom edge */
		if ((y < panel_row_min + row_trigger) || (y > panel_row_max - row_trigger))
		{
			prow = ((y - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
			if (prow > max_panel_rows) prow = max_panel_rows;
			else if (prow < 0) prow = 0;
		}

		/* Scroll screen when 4 grids from left/right edge */
		if ((x < panel_col_min + col_trigger) || (x > panel_col_max - col_trigger))
		{
			pcol = ((x - SCREEN_WID / 4) / (SCREEN_WID / 2));
			if (pcol > max_panel_cols) pcol = max_panel_cols;
			else if (pcol < 0) pcol = 0;
		}

		/* Check for "no change" */
		if ((prow == panel_row) && (pcol == panel_col)) return;

		/* Hack -- optional disturb on "panel change" */
		if (disturb_panel) disturb(0, 0);

		/* Save the new panel info */
		panel_row = prow;
		panel_col = pcol;

		/* Recalculate the boundaries */
		panel_bounds();
	}

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
* Monster health description
*/
cptr look_mon_desc(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool          living = TRUE;
	int           perc;


	/* Determine if the monster is "living" (vs "undead") */
	if (r_ptr->flags3 & (RF3_UNDEAD)) living = FALSE;
	if (r_ptr->flags3 & (RF3_DEMON)) living = FALSE;
	if (r_ptr->flags3 & (RF3_DEVIL)) living = FALSE;
	if (r_ptr->flags3 & (RF3_NONLIVING)) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * m_ptr->hp / m_ptr->maxhp;

	if (perc >= 60)
	{
		return (living ? "somewhat wounded" : "somewhat damaged");
	}

	if (perc >= 25)
	{
		return (living ? "wounded" : "damaged");
	}

	if (perc >= 10)
	{
		return (living ? "badly wounded" : "badly damaged");
	}

	return (living ? "almost dead" : "almost destroyed");
}



/*
* Angband sorting algorithm -- quick sort in place
*
* Note that the details of the data we are sorting is hidden,
* and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
* function hooks to interact with the data, which is given as
* two pointers, and which may have any user-defined form.
*/
void ang_sort_aux(vptr u, vptr v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp)(u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp)(u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap)(u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(u, v, b+1, q);
}


/*
* Angband sorting algorithm -- quick sort in place
*
* Note that the details of the data we are sorting is hidden,
* and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
* function hooks to interact with the data, which is given as
* two pointers, and which may have any user-defined form.
*/
void ang_sort(vptr u, vptr v, int n)
{
	/* Sort the array */
	ang_sort_aux(u, v, 0, n-1);
}





/*** Targetting Code ***/


/*
* Determine is a monster makes a reasonable target
*
* The concept of "targetting" was stolen from "Morgul" (?)
*
* The player can target any location, or any "target-able" monster.
*
* Currently, a monster is "target_able" if it is visible, and if
* the player can hit it with a projection, and the player is not
* hallucinating.  This allows use of "use closest target" macros.
*
* Future versions may restrict the ability to target "trappers"
* and "mimics", but the semantics is a little bit weird.
*/
bool target_able(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	/* Monster must be alive */
	if (!m_ptr->r_idx) return (FALSE);

	/* Monster must be visible */
	if (!m_ptr->ml) return (FALSE);

	/* Monster must be projectable */
	if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) return (FALSE);

	/* Hack -- no targeting hallucinations */
	if (p_ptr->image) return (FALSE);

	/* XXX XXX XXX Hack -- Never target trappers */
	/* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

	/* Assume okay */
	return (TRUE);
}




/*
* Update (if necessary) and verify (if possible) the target.
*
* We return TRUE if the target is "okay" and FALSE otherwise.
*/
bool target_okay(void)
{
	/* Accept stationary targets */
	if (target_who < 0) return (TRUE);

	/* Check moving targets */
	if (target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(target_who))
		{
			monster_type *m_ptr = &m_list[target_who];

			/* Acquire monster location */
			target_row = m_ptr->fy;
			target_col = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}



/*
* Sorting hook -- comp function -- by "distance to player"
*
* We use "u" and "v" to point to arrays of "x" and "y" positions,
* and sort the arrays by double-distance to the player.
*/
static bool ang_sort_comp_distance(vptr u, vptr v, int a, int b)
{
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = x[a]; kx -= px; kx = ABS(kx);
	ky = y[a]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = x[b]; kx -= px; kx = ABS(kx);
	ky = y[b]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	return (da <= db);
}


/*
* Sorting hook -- swap function -- by "distance to player"
*
* We use "u" and "v" to point to arrays of "x" and "y" positions,
* and sort the arrays by distance to the player.
*/
static void ang_sort_swap_distance(vptr u, vptr v, int a, int b)
{
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	byte temp;

	/* Swap "x" */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;

	/* Swap "y" */
	temp = y[a];
	y[a] = y[b];
	y[b] = temp;
}



/*
* Hack -- help "select" a location (see below)
*/
static s16b target_pick(int y1, int x1, int dy, int dx)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < temp_n; i++)
	{
		/* Point 2 */
		x2 = temp_x[i];
		y2 = temp_y[i];

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* XXX XXX XXX Penalize location */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i; b_v = v;
	}

	/* Result */
	return (b_i);
}


/*
* Hack -- determine if a given location is "interesting"
*/
static bool target_set_accept(int y, int x)
{
	cave_type *c_ptr;

	s16b this_o_idx, next_o_idx = 0;

	/* More Paranoia */
	if( x >= MAX_WID ) return FALSE;
	if( y >= MAX_HGT ) return FALSE;

	/* Player grid is always interesting */
	if ((y == py) && (x == px)) return (TRUE);

	/* Handle hallucination */
	if (p_ptr->image) return (FALSE);

	/* Examine the grid */
	c_ptr = &cave[y][x];

	/* Visible monsters */
	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		/* Visible monsters */
		if (m_ptr->ml) return (TRUE);
	}

	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Memorized object */
		if (o_ptr->marked) return (TRUE);
	}

	/* Interesting memorized features */
	if (c_ptr->info & (CAVE_MARK))
	{
		/* Notice glyphs */
		if (c_ptr->feat == FEAT_GLYPH) return (TRUE);
		if (c_ptr->feat == FEAT_MINOR_GLYPH) return (TRUE);

		/* Notice doors */
		if (c_ptr->feat == FEAT_OPEN) return (TRUE);
		if (c_ptr->feat == FEAT_BROKEN) return (TRUE);

		/* Notice stairs */
		if (c_ptr->feat == FEAT_LESS) return (TRUE);
		if (c_ptr->feat == FEAT_MORE) return (TRUE);

		/* Notice shops */
		if ((c_ptr->feat >= FEAT_SHOP_HEAD) &&
			(c_ptr->feat <= FEAT_SHOP_TAIL)) return (TRUE);

		/* Notice traps */
		if ((c_ptr->feat >= FEAT_TRAP_HEAD) &&
			(c_ptr->feat <= FEAT_TRAP_TAIL)) return (TRUE);

		/* Notice doors */
		if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			(c_ptr->feat <= FEAT_DOOR_TAIL)) return (TRUE);

		/* Notice rubble */
		if (c_ptr->feat == FEAT_RUBBLE) return (TRUE);

		/* Notice veins with treasure */
		if (c_ptr->feat == FEAT_MAGMA_K) return (TRUE);
		if (c_ptr->feat == FEAT_QUARTZ_K) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}


/*
* Prepare the "temp" array for "target_set"
*
* Return the number of target_able monsters in the set.
*/
static void target_set_prepare(int mode)
{
	int y, x;

	/* Reset "temp" array */
	temp_n = 0;

	/* Scan the current panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			/* Require line of sight, unless "look" is "expanded" */
			if (!expand_look && !player_has_los_bold(y, x)) continue;

			/* Require "interesting" contents */
			if (!target_set_accept(y, x)) continue;

			/* Require target_able monsters for "TARGET_KILL" */
			if ((mode & (TARGET_KILL)) && !target_able(c_ptr->m_idx)) continue;

			/* Save the location */
			temp_x[temp_n] = x;
			temp_y[temp_n] = y;
			temp_n++;
		}
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_distance;
	ang_sort_swap = ang_sort_swap_distance;

	/* Sort the positions */
	ang_sort(temp_x, temp_y, temp_n);
}


/*
* Examine a grid, return a keypress.
*
* The "mode" argument contains the "TARGET_LOOK" bit flag, which
* indicates that the "space" key should scan through the contents
* of the grid, instead of simply returning immediately.  This lets
* the "look" command get complete information, without making the
* "target" command annoying.
*
* The "info" argument contains the "commands" which should be shown
* inside the "[xxx]" text.  This string must never be empty, or grids
* containing monsters will be displayed with an extra comma.
*
* Note that if a monster is in the grid, we update both the monster
* recall info and the health bar info to track that monster.
*
* Eventually, we may allow multiple objects per grid, or objects
* and terrain features in the same grid. XXX XXX XXX
*
* This function must handle blindness/hallucination.
*/
static int target_set_aux(int y, int x, int mode, cptr info)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;

	cptr s1, s2, s3;

	bool boring;

	int feat;

	int query;

	char out_val[160];


	/* Repeat forever */
	while (1)
	{
		/* Paranoia */
		query = ' ';

		/* Assume boring */
		boring = TRUE;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";

		/* Hack -- under the player */
		if ((y == py) && (x == px))
		{
			/* Description */
			s1 = "You are ";

			/* Preposition */
			s2 = "on ";
		}


		/* Hack -- hallucination */
		if (p_ptr->image)
		{
			cptr name = "something strange";

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;

			/* Repeat forever */
			continue;
		}


		/* Actual monsters */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Visible */
			if (m_ptr->ml)
			{
				bool recall = FALSE;

				char m_name[80];

				/* Not boring */
				boring = FALSE;

				/* Get the monster name ("a kobold") */
				monster_desc(m_name, m_ptr, 0x08);

				/* Hack -- track this monster race */
				monster_race_track(m_ptr->r_idx);

				/* Hack -- health bar for this monster */
				health_track(c_ptr->m_idx);

				/* Hack -- handle stuff */
				handle_stuff();

				/* Interact */
				while (1)
				{
					/* Recall */
					if (recall)
					{
						/* Save */
						Term_save();

						/* Recall on screen */
						screen_roff(m_ptr->r_idx);

						/* Hack -- Complete the prompt (again) */
						Term_addstr(-1, TERM_WHITE, format("  [r,%s]", info));

						/* Command */
						query = inkey();

						/* Restore */
						Term_load();
					}

					/* Normal */
					else
					{
						/* Describe, and prompt for recall */
						sprintf(out_val, "%s%s%s%s (%s)%s%s%s[r,%s]",
							s1, s2, s3, m_name, look_mon_desc(c_ptr->m_idx),
							(m_ptr->smart & SM_CLONED ? " (clone)": ""),
							(is_ally( m_ptr ) ? " (allied) " : " "),
							(m_ptr->ally & ALLY_SELF ? " (neutral) " : " "),
							info);

						prt(out_val, 0, 0);

						/* Place cursor */
						move_cursor_relative(y, x);

						/* Command */
						query = inkey();
					}

					/* Normal commands */
					if (query != 'r') break;

					/* Toggle recall */
					recall = !recall;
				}

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Hack -- take account of gender */
				if (r_ptr->flags1 & (RF1_FEMALE)) s1 = "She is ";
				else if (r_ptr->flags1 & (RF1_MALE)) s1 = "He is ";

				/* Use a preposition */
				s2 = "carrying ";

				/* Scan all objects being carried */
				for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
				{
					char o_name[80];

					object_type *o_ptr;

					/* Acquire object */
					o_ptr = &o_list[this_o_idx];

					/* Acquire next object */
					next_o_idx = o_ptr->next_o_idx;

					/* Obtain an object description */
					object_desc(o_name, o_ptr, TRUE, 3);

					/* Describe the object */
					sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
					prt(out_val, 0, 0);
					move_cursor_relative(y, x);
					query = inkey();

					/* Always stop at "normal" keys */
					if ((query != '\r') && (query != '\n') && (query != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

					/* Change the intro */
					s2 = "also carrying ";
				}

				/* Double break */
				if (this_o_idx) break;

				/* Use a preposition */
				s2 = "on ";
			}
		}

		if( c_ptr->o_idx )
		{
			term_show_most_expensive_item( x , y , "There is" );
			window_stuff();
		}

		/* Scan all objects in the grid */
		for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;

			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Describe it */
			if (o_ptr->marked)
			{
				char o_name[80];

				/* Not boring */
				boring = FALSE;

				/* Obtain an object description */
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Describe the object */
				sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
				prt(out_val, 0, 0);
				move_cursor_relative(y, x);
				query = inkey();

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Plurals */
				if (o_ptr->number != 1) s1 = "They are ";

				/* Preposition */
				s2 = "on ";
			}
		}

		/* Double break */
		if (this_o_idx) break;


		/* Feature (apply "mimic") */
		feat = f_info[c_ptr->feat].mimic;

		/* Require knowledge about grid, or ability to see grid */
		if (!(c_ptr->info & (CAVE_MARK)) && !player_can_see_bold(y,x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}

		/* Terrain feature if needed */
		if (boring || (feat > FEAT_INVIS))
		{
			cptr name = f_name + f_info[feat].name;

			/* Hack -- handle unknown grids */
			if (feat == FEAT_NONE) name = "unknown grid";

			/* Pick a prefix */
			if (*s2 && (feat == FEAT_MINOR_GLYPH || feat == FEAT_GLYPH)) s2 = "on ";
			else if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";

			/* Pick proper indefinite article */
			s3 = (is_a_vowel(name[0])) ? "an " : "a ";

			/* Hack -- special introduction for store doors */
			if ((feat >= FEAT_SHOP_HEAD) && (feat <= FEAT_SHOP_TAIL))
			{
				s3 = (is_a_vowel(name[0])) ? "the entrance to an " : "the entrance to a ";
			}

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Always stop at "normal" keys */
			if ((query != '\r') && (query != '\n') && (query != ' ')) break;
		}

		/* Stop on everything but "return" */
		if ((query != '\r') && (query != '\n')) break;
	}

	/* Keep going */
	return (query);
}

/*
* Handle "target" and "look".
*
* Note that this code can be called from "get_aim_dir()".
*
* All locations must be on the current panel.  Consider the use of
* "panel_bounds()" to allow "off-panel" targets, perhaps by using
* some form of "scrolling" the map around the cursor.  XXX XXX XXX
* That is, consider the possibility of "auto-scrolling" the screen
* while the cursor moves around.  This may require changes in the
* "update_mon()" code to allow "visibility" even if off panel, and
* may require dynamic recalculation of the "temp" grid set.
*
* Hack -- targetting/observing an "outer border grid" may induce
* problems, so this is not currently allowed.
*
* The player can use the direction keys to move among "interesting"
* grids in a heuristic manner, or the "space", "+", and "-" keys to
* move through the "interesting" grids in a sequential manner, or
* can enter "location" mode, and use the direction keys to move one
* grid at a time in any direction.  The "t" (set target) command will
* only target a monster (as opposed to a location) if the monster is
* target_able and the "interesting" mode is being used.
*
* The current grid is described using the "look" method above, and
* a new command may be entered at any time, but note that if the
* "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
* where "space" has no obvious meaning) then "space" will scan
* through the description of the current grid until done, instead
* of immediately jumping to the next "interesting" grid.  This
* allows the "target" command to retain its old semantics.
*
* The "*", "+", and "-" keys may always be used to jump immediately
* to the next (or previous) interesting grid, in the proper mode.
*
* The "return" key may always be used to scan through a complete
* grid description (forever).
*
* This command will cancel any old target, even if used from
* inside the "look" command.
*/
bool target_set(int mode)
{
	int		i, d, m;

	int		y = py;
	int		x = px;

	bool	done = FALSE;

	bool	flag = TRUE;

	char	query;

	char	info[80];

	cave_type		*c_ptr;


	/* Cancel target */
	target_who = 0;

	/* Cancel tracking */
	/* health_track(0); */

	/* Prepare the "temp" array */
	target_set_prepare(mode);

	/* Start near the player */
	m = 0;

	/* Interact */
	while (!done)
	{
		/* Interesting grids */
		if (flag && temp_n)
		{
			y = temp_y[m];
			x = temp_x[m];

			/* Access */
			c_ptr = &cave[y][x];

			/* Allow target */
			if (target_able(c_ptr->m_idx))
			{
				strcpy(info, "q,t,p,o,+,-,<dir>");
			}

			/* Dis-allow target */
			else
			{
				strcpy(info, "q,p,o,+,-,<dir>");
			}

			/* Describe and Prompt */
			query = target_set_aux(y, x, mode, info);

			/* Assume no "direction" */
			d = 0;

			/* Analyze */
			switch (query)
			{
			case ESCAPE:
			case 'q':
				{
					done = TRUE;
					break;
				}

			case 't':
			case '.':
			case '5':
			case '0':
				{
					if (target_able(c_ptr->m_idx))
					{
						health_track(c_ptr->m_idx);
						target_who = c_ptr->m_idx;
						target_row = y;
						target_col = x;
						done = TRUE;
					}
					else
					{
						bell();
					}
					break;
				}

			case ' ':
			case '*':
			case '+':
				{
					if (++m == temp_n)
					{
						m = 0;
						if (!expand_list) done = TRUE;
					}
					break;
				}

			case '-':
				{
					if (m-- == 0)
					{
						m = temp_n - 1;
						if (!expand_list) done = TRUE;
					}
					break;
				}

			case 'p':
				{
					y = py;
					x = px;
				}

			case 'o':
				{
					flag = !flag;
					break;
				}

			case 'm':
				{
					break;
				}

			default:
				{
					d = get_keymap_dir(query);
					if (!d) bell();
					break;
				}
			}

			/* Hack -- move around */
			if (d)
			{
				/* Find a new monster */
				i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

				/* Use that grid */
				if (i >= 0) m = i;
			}
		}

		/* Arbitrary grids */
		else
		{
			/* Access */
			c_ptr = &cave[y][x];

			/* Default prompt */
			strcpy(info, "q,t,p,m,+,-,<dir>");

			/* Describe and Prompt (enable "TARGET_LOOK") */
			query = target_set_aux(y, x, mode | TARGET_LOOK, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no direction */
			d = 0;

			/* Analyze the keypress */
			switch (query)
			{
			case ESCAPE:
			case 'q':
				{
					done = TRUE;
					break;
				}

			case 't':
			case '.':
			case '5':
			case '0':
				{
					target_who = -1;
					target_row = y;
					target_col = x;
					done = TRUE;
					break;
				}

			case ' ':
			case '*':
			case '+':
			case '-':
				{
					break;
				}

			case 'p':
				{
					y = py;
					x = px;
				}

			case 'o':
				{
					break;
				}

			case 'm':
				{
					flag = !flag;
					break;
				}

			default:
				{
					d = get_keymap_dir(query);
					if (!d) bell();
					break;
				}
			}

			/* Handle "direction" */
			if (d)
			{
				x += ddx[d];
				y += ddy[d];

				/* Hack -- Verify x */
				if ((x>=cur_wid-1) || (x>panel_col_max)) x--;
				else if ((x<=0) || (x<panel_col_min)) x++;

				/* Hack -- Verify y */
				if ((y>=cur_hgt-1) || (y>panel_row_max)) y--;
				else if ((y<=0) || (y<panel_row_min)) y++;
			}
		}
	}

	/* Forget */
	temp_n = 0;

	/* Clear the top line */
	prt("", 0, 0);

	/* Failure to set target */
	if (!target_who) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
* Get an "aiming direction" from the user.
*
* The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
* "0" for "current target", and "-1" for "entry aborted".
*
* Note that "Force Target", if set, will pre-empt user interaction,
* if there is a usable target already set.
*
* Note that confusion over-rides any (explicit?) user choice.
*/
bool get_aim_dir(int *dp)
{
	int		dir;

	char	command;

	cptr	p;

#ifdef ALLOW_REPEAT /* TNB */

	if (repeat_pull(dp)) {

		/* Confusion? */

		/* Verify */
		if (!(*dp == 5 && !target_okay())) {
			return (TRUE);
		}
	}

#endif /* ALLOW_REPEAT -- TNB */

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Hack -- auto-target if requested */
	if (use_old_target && target_okay()) dir = 5;

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;

		/* Convert various keys to "standard" keys */
		switch (command)
		{
			/* Use current target */
		case 'T':
		case 't':
		case '.':
		case '5':
		case '0':
			{
				dir = 5;
				break;
			}

			/* Set new target */
		case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}

		default:
			{
				dir = get_keymap_dir(command);
				break;
			}
		}

		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;

		/* Error */
		if (!dir) bell();
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	command_dir = dir;

	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[rand_int(8)];
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(dir);

#endif /* ALLOW_REPEAT -- TNB */


	/* A "valid" direction was entered */
	return (TRUE);
}



/*
* Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
* and place it into "command_dir", unless we already have one.
*
* This function should be used for all "repeatable" commands, such as
* run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
* as all commands which must reference a grid adjacent to the player,
* and which may not reference the grid under the player.  Note that,
* for example, it is no longer possible to "disarm" or "open" chests
* in the same grid as the player.
*
* Direction "5" is illegal and will (cleanly) abort the command.
*
* This function tracks and uses the "global direction", and uses
* that as the "desired direction", to which "confusion" is applied.
*/
bool get_rep_dir(int *dp)
{
	int dir;

#ifdef ALLOW_REPEAT /* TNB */

	if (repeat_pull(dp)) {
		return (TRUE);
	}

#endif /* ALLOW_REPEAT -- TNB */

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Get a direction */
	while (!dir)
	{
		char ch;

		/* Get a command (or Cancel) */
		if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

		/* Look up the direction */
		dir = get_keymap_dir(ch);

		/* Oops */
		if (!dir) bell();
	}

	/* Prevent weirdness */
	if (dir == 5) dir = 0;

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save desired direction */
	command_dir = dir;

	/* Apply "confusion" */
	if (p_ptr->confused)
	{
		/* Standard confusion */
		if (rand_int(100) < 75)
		{
			/* Random direction */
			dir = ddd[rand_int(8)];
		}
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;


#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(dir);

#endif /* ALLOW_REPEAT -- TNB */

	/* Success */
	return (TRUE);
}


int get_evil_patron()
{
	return (((p_ptr->age)+(p_ptr->sc))%MAX_PATRON);
}

int patron_poly_self()
{
		msg_format("The voice of %s booms out: 'Thou needst a new form!'", patrons[p_ptr->evil_patron].short_name);
		do_poly_self();
		return 1;
}

int patron_poly_gain_exp()
{
	msg_format("The voice of %s booms out: 'Well done! Lead on!'", patrons[p_ptr->evil_patron].short_name);
	if (p_ptr->lev < PY_MAX_LEVEL)
	{
		s32b ee = (p_ptr->exp / 2) + 10;
		if (ee > 100000L) ee = 100000L;
		msg_print("You feel more experienced.");
		gain_exp(ee);
		return 1;
	}
	return 0;
}

int patron_poly_loose_exp()
{
	msg_format("The voice of %s booms out: Thou does not deserve that power just yet!", patrons[p_ptr->evil_patron].short_name);
	/* Lose some experience */
	p_ptr->max_exp -= p_ptr->max_exp / 6;
	if( p_ptr->exp > p_ptr->max_exp )
		p_ptr->exp = p_ptr->max_exp;
	/* Check Experience */
	check_experience();
	return 1;
}

int patron_great_object()
{
	if (p_ptr->lev < PY_MAX_LEVEL - 10 )
	{
		msg_format("The voice of %s booms out: 'Use my gift wisely.'", patrons[p_ptr->evil_patron].short_name);
		acquirement(py, px, 1, TRUE);
		return 1;
	}
	return 0;
}

int patron_great_objects()
{
	if (p_ptr->lev < PY_MAX_LEVEL - 5 )
	{
		msg_format("The voice of %s booms out: 'Behold, how generously I reward thy loyalty.'",patrons[p_ptr->evil_patron].short_name);
		acquirement(py, px, randint(2) + 3, TRUE);
		return 1;
	}
	return 0;
}

int patron_ty_curse()
{
	msg_format("The voice of %s whispers: 'Power requires suffering, true power requires true suffering.'",patrons[p_ptr->evil_patron].short_name);
	activate_ty_curse();
	return 1;
}

static int patron_chaos_weapon()
{
	object_type *q_ptr;
	object_type forge;
	int dummy = 0, dummy2 = 0;

	msg_format("The voice of %s booms out: 'Thy deed hath earned thee a worthy blade.'",patrons[p_ptr->evil_patron].short_name);
	/* Get local object */
	q_ptr = &forge;
	dummy = TV_SWORD;
	switch(randint(p_ptr->lev))
	{
		case 1: case 2: case 0:    dummy2 = SV_DAGGER;             break;
		case 3: case 4:            dummy2 = SV_PARRYING_DAGGER;    break;
		case 5: case 6:            dummy2 = SV_RAPIER;             break;
		case 7: case 8:            dummy2 = SV_SMALL_SWORD;        break;
		case 9: case 10:           dummy2 = SV_SHORT_SWORD;        break;
		case 11: case 12: case 13: dummy2 = SV_SABRE;              break;
		case 14: case 15: case 16: dummy2 = SV_CUTLASS;            break;
		case 17: case 27:          dummy2 = SV_TULWAR;             break;
		case 18: case 19: case 20: dummy2 = SV_BROAD_SWORD;        break;
		case 21: case 22: case 23: dummy2 = SV_LONG_SWORD;         break;
		case 24: case 25: case 26: dummy2 = SV_SCIMITAR;           break;
		case 28: case 29:          dummy2 = SV_GLADIUS;            break;
		case 30: case 31:          dummy2 = SV_TWO_HANDED_SWORD;   break;
		case 32:                   dummy2 = SV_EXECUTIONERS_SWORD; break;
		default:                   dummy2 = SV_BLADE_OF_CHAOS;
	}
	object_prep(q_ptr, lookup_kind(dummy, dummy2));
	q_ptr->to_h = 3 + (randint(dun_level))%10;
	q_ptr->to_d = 3 + (randint(dun_level))%10;
	random_resistance(q_ptr, FALSE, ((randint(34))+4));

	if( randint(p_ptr->lev) > 32 ){
		create_artefact(q_ptr, FALSE);
		q_ptr->art_flags2 |= TR2_RES_CHAOS;
		q_ptr->art_flags2 |= TR2_RES_CONF;
		q_ptr->art_flags2 |= TR2_RES_DISEN;
	}else{
		q_ptr->name2 = EGO_CHAOTIC;
	}
	/* Drop it in the dungeon */
	drop_near(q_ptr, -1, py, px);
	return 1;
}

/* Yes, this has become _much_ nastier, you could die. */
int patron_summon_normal()
{
	int dummy;
	msg_format("The voice of %s booms out: 'You have grown soft, maybe this will keep you sharp.'", patrons[p_ptr->evil_patron].short_name);
	for (dummy = 0; dummy < p_ptr->lev / 3 + 1; dummy++)
		(void) summon_specific(py, px, dun_level + p_ptr->lev / 5, 0);
	return 1;
}

/* Yes, this has become _much_ nastier, you could die. */
int patron_summon_deep()
{
	int dummy;
	msg_format("The voice of %s booms out: 'You have grown soft, maybe this will keep you sharp.'", patrons[p_ptr->evil_patron].short_name);
	for (dummy = 0; dummy < p_ptr->lev / 15 + 1; dummy++)
		(void) summon_specific(py, px, dun_level + 15, 0);
	return 1;
}

int patron_havoc()
{
	msg_format("The voice of %s booms out: 'Death and destruction! This pleaseth me!'", patrons[p_ptr->evil_patron].short_name);
	fire_ball( GF_DISINTEGRATE, 0, 300, 8);
	call_chaos();
	return 1;
}

int patron_destruction()
{
		msg_format("The voice of %s booms out: 'Death and destruction! This pleaseth me!'", patrons[p_ptr->evil_patron].short_name);
		destroy_area(py, px, 25, TRUE);
		return 1;
}

int patron_curse_weapon()
{
	msg_format("The voice of %s booms out: 'Thou reliest too much on thy weapon.'", patrons[p_ptr->evil_patron].short_name);
	return curse_weapon();
}

int patron_curse_armor()
{
	msg_format("The voice of %s booms out: 'Thou reliest too much on thine equipment.'", patrons[p_ptr->evil_patron].short_name);
	return curse_armour();
}

int patron_hurt(){

	/* TODO: we need to make sure that this does never surpasses 32... */
	char wrath_reason[32];
	int damage    = 0;
	int treshold  = 0;
	sprintf(wrath_reason, "the Wrath of %s", patrons[p_ptr->evil_patron].short_name);

	/* Inform the player that someone is really unhappy */
	msg_format("The voice of %s thunders: 'You no longer serve my purposes!'" , patrons[p_ptr->evil_patron].short_name);

	/* Make sure that at least 10% of the hp are left, a fighting chance.. */
	damage   = p_ptr->lev * 4;
	treshold = p_ptr->mhp / 10;
	damage = ( p_ptr->chp - damage < treshold  )?(p_ptr->chp - treshold):damage;
	if( damage > 0 )
		take_hit( damage , format( "the Wrath of %s", patrons[p_ptr->evil_patron].short_name ) );

	/*Negative damage does not count*/
	return damage<0?0:damage;
}

int patron_gain_stat()
{
	int chosen_stat;
	msg_format("The voice of %s rings out: 'Stay and let me mold thee!'", patrons[p_ptr->evil_patron].short_name);
	/* 1 time in 3 the prefered stat is increased, otherwise a random stat is increased */
	chosen_stat = ( randint(3) == 1 ) ? patrons[p_ptr->evil_patron].stat : randint(6)-1;
	/* If one of the increases is usefull ( increases or restores ), TRUE is returned   */
	return ( do_inc_stat( chosen_stat ) + do_inc_stat( chosen_stat ) + do_inc_stat( chosen_stat ) );
}

int patron_loose_stat()
{
	int chosen_stat;
	msg_format("The voice of %s booms out: 'It takes pressure to become a diamond.'", patrons[p_ptr->evil_patron].short_name);
	/* 1 time in 3 the prefered stat is increased, otherwise a random stat is increased */
	chosen_stat = ( randint(3) == 1 ) ? patrons[p_ptr->evil_patron].stat : randint(6)-1;
	do_dec_stat( chosen_stat );
	do_dec_stat( chosen_stat );
	/* */
	if( p_ptr->stat_cur[chosen_stat] < p_ptr->stat_max[chosen_stat] )
	{
		/* Evil, drop max value as well, update bonus and let caller know */
		p_ptr->stat_max[chosen_stat] = p_ptr->stat_cur[chosen_stat];
		p_ptr->update |= (PU_BONUS);
		return 1;
	}
	return 0;
}


int patron_raise_stats()
{
	int dummy;
	int total = 0;
	msg_format("The voice of %s booms out: 'Receive this modest gift from me!'", patrons[p_ptr->evil_patron].short_name);
		for (dummy = 0; dummy < 6; dummy++)
			total += do_inc_stat(dummy);
	return total;
}

int patron_loose_stats()
{ 	int dummy;
	msg_format("The voice of %s thunders: 'Patience and humility!'", patrons[p_ptr->evil_patron].short_name);
	for (dummy = 0; dummy < 6; dummy++)
		dec_stat(dummy, 10 + randint(15), TRUE);
	return 1;
}

int patron_mass_gen()
{
		msg_format("The voice of %s booms out: 'Let me relieve thee of thine oppressors!'", patrons[p_ptr->evil_patron].short_name);
		return mass_genocide(FALSE);
}

int patron_ignore()
{
		msg_format("%s ignores you.", patrons[p_ptr->evil_patron].short_name);
		return 1;
}

int patron_wrath()
{
	int effect; /* Is there any effect, there might not be.. */
	/* These are guaranteed */
	effect = patron_hurt() + patron_loose_stats() + patron_summon_normal() + patron_ty_curse();
	/* These happen half of the time */
	if (randint(2)==1) effect += patron_curse_weapon();
	if (randint(2)==1) effect += patron_curse_armor();
	return effect;
}

int patron_permanent_walls()
{
	int x,y;

	msg_format("%s says 'I like what you've done with the place.'", patrons[p_ptr->evil_patron].short_name);
	/* Scan all normal grids */
	for (y = 1; y < cur_hgt-1; y++)
	{
		for (x = 1; x < cur_wid-1; x++)
		{
			cave_type *c_ptr = &cave[y][x];
			if( c_ptr->feat >= FEAT_MAGMA && c_ptr->feat <= FEAT_PERM_OUTER )
				c_ptr->feat = FEAT_PERM_SOLID;
		}
	}
	/* Mega-Hack -- Forget the view and lite */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	return 1;
}

int patron_mutation()
{
	msg_format("%^s corrupts you!", patrons[p_ptr->evil_patron].short_name);
	(void)gain_corruption(0);
	return 0;
}

/* I just know this can be done better, please dont post this on the WTF du jour ;] */
int patron_null(){ return 0; }

#define REWARD_GOOD   0
#define REWARD_BAD    1
#define REWARD_FICKLE 2

reward_type patron_rewards[] =
{
	{ REWARD_FICKLE , patron_poly_self       , "Poly self" },
	{ REWARD_GOOD   , patron_poly_gain_exp   , "Gain experience" },
	{ REWARD_BAD    , patron_poly_loose_exp  , "Loose experience" },
	{ REWARD_GOOD   , patron_great_object    , "Give great object" },
	{ REWARD_GOOD   , patron_great_objects   , "Give great objects" },
	{ REWARD_FICKLE , patron_chaos_weapon    , "Give chaos weapon" },
	{ REWARD_BAD    , patron_ty_curse        , "TY curse" },
	{ REWARD_BAD    , patron_summon_normal   , "Summon normal" },
	{ REWARD_GOOD   , patron_havoc           , "Havoc" },
	{ REWARD_GOOD   , patron_gain_stat       , "Increase stat" },
	{ REWARD_BAD    , patron_loose_stat      , "Loose stat" },
	{ REWARD_BAD    , patron_loose_stats     , "Loose stats" },
	{ REWARD_GOOD   , patron_raise_stats     , "Raise stats" },
	{ REWARD_FICKLE , patron_destruction     , "Destruction" },
	{ REWARD_BAD    , patron_wrath           , "Wrath" },
	{ REWARD_BAD    , patron_hurt            , "Hurt" },
	{ REWARD_BAD    , patron_curse_armor     , "Curse Armor" },
	{ REWARD_BAD    , patron_curse_weapon    , "Curse Weapon" },
	{ REWARD_BAD    , patron_summon_deep     , "Summon Deep" },
	{ REWARD_GOOD   , patron_mass_gen        , "Mass Genocide" },
	{ REWARD_FICKLE , patron_ignore          , "Ignore" },
	{ REWARD_GOOD   , patron_undead_servant  , "Undead Servant" },
	{ REWARD_GOOD   , patron_demon_servant   , "Demon Servant" },
	{ REWARD_GOOD   , patron_angel_servant   , "Angel Servant" },
	{ REWARD_BAD    , patron_undead_foe      , "Undead Foe" },
	{ REWARD_BAD    , patron_demon_foe       , "Demon Foe" },
	{ REWARD_BAD    , patron_angel_foe       , "Angel Foe" },
	{ REWARD_FICKLE , patron_permanent_walls , "Permanent Level" },
	{ REWARD_FICKLE , patron_mutation        , "Gain Mutation" },
	{ REWARD_FICKLE , patron_null            , NULL },
};

void gain_level_reward(int chosen_reward)
{
	int nasty_chance = 6;
	int nasty_ok = FALSE;
	int reward_count = 0;
	int reward_idx = 0;
	int tries = 0;

	/* Only 1 reward per level gain, thats kinda boring really..
	   TODO: Reconsider this */
	if (!chosen_reward)
	{
		if (multi_rew) return;
		else multi_rew = TRUE;
	}
	/* Level 13 and products of 13: BAD!, level 7 and 9 and their products should be fine */
	if (p_ptr->lev == 13)        nasty_chance = 2;
	else if (!(p_ptr->lev % 13)) nasty_chance = 3;
	else if (!(p_ptr->lev % 7 )) nasty_chance = 12;
	else if (!(p_ptr->lev % 9 )) nasty_chance = 12;

	/* Can bad things happen ? */
	if (randint(nasty_chance)==1)
		nasty_ok = TRUE;

	/* Count the possible effects */
	while( patron_rewards[reward_count].description != NULL )
		reward_count++;

	/* Give up after a while */
	while( tries++ < 100 )
	{
		reward_idx = rand_int( reward_count );
		if( //Not nasty and good reward
			( !nasty_ok && patron_rewards[reward_idx].nasty == REWARD_GOOD ) ||
			//Not nasty and fickle reward with type 0 which means abyssal patron
			( !nasty_ok && patron_rewards[reward_idx].nasty == REWARD_FICKLE && patrons[p_ptr->evil_patron].type == 0 ) ||
			//With nasty, anything goes
			( nasty_ok ) )
				tries = tries + patron_rewards[reward_idx].func() * 100;
	}
}

/*
* old -- from PsiAngband.
*/
bool tgt_pt(int *x,int *y)
{
	char ch = 0;
	int d,cu,cv;
	bool success = FALSE;

	*x = px;
	*y = py;

	cu = Term->scr->cu;
	cv = Term->scr->cv;
	Term->scr->cu = 0;
	Term->scr->cv = 1;
	msg_print("Select a point and press space.");

	while ((ch != 27) && (ch != ' '))
	{
		move_cursor_relative(*y,*x);
		ch = inkey();
		switch (ch)
		{
		case 27: break;
		case ' ': success = TRUE; break;
		default:
			{
				d = get_keymap_dir(ch);
				if (!d) break;
				*x += ddx[d];
				*y += ddy[d];

				/* Hack -- Verify x */
				if ((*x>=cur_wid-1) || (*x>=panel_col_min + SCREEN_WID)) (*x)--;
				else if ((*x<=0) || (*x<=panel_col_min)) (*x)++;

				/* Hack -- Verify y */
				if ((*y>=cur_hgt-1) || (*y>=panel_row_min + SCREEN_HGT)) (*y)--;
				else if ((*y<=0) || (*y<=panel_row_min)) (*y)++;

				break;
			}
		}
	}

	Term->scr->cu = cu;
	Term->scr->cv = cv;
	Term_fresh();
	return success;

}

/*Somewhat the same tactic as in Tyrant, add all the odds together*/
/*in order to find how many sides the die needs to get a random corruption*/
int count_corruption_options()
{
	int i;
	int count = 0;
	for( i = 0 ; i < COUNT_CORRUPTIONS ; i++ )
		count = count + corruptions[i].odds;
	return count;
}

/*Converts a dice throw for a corruption into the actual corruption*/
int get_corruption( int option)
{
	int i;
	int count = 0;
	for( i = 0 ; i < COUNT_CORRUPTIONS ; i++ )
	{
		count = count + corruptions[i].odds;
		if(count>=option)
			return i;
	}
	msg_format("Getting corruption option %d failed.", option);msg_print(NULL);
	return 0;
}

/*Converts an index to a muta pointer in the p_ptr struct*/
u32b *corruption_idx_to_u32b( byte b )
{
	switch( b )
	{
		case 1:
			return &(p_ptr->muta1);
		case 2:
			return &(p_ptr->muta2);
		case 3:
			return &(p_ptr->muta3);
	}
	msg_format("Getting corruption idx (%d) failed.", b );msg_print(NULL);
	/*Need to return a valid memory address anyway to avoid panick*/
	return &(p_ptr->muta1);
}


bool gain_corruption(int choose_mut)
{
	int attempts_left = 20;
	cptr muta_desc = "";
	bool muta_chosen = FALSE;
	u32b muta_which = 0;
	int opposed_muta_which = 0;
	u32b *muta_class = 0;
	u32b *opposed_muta_class = 0;
	int options = count_corruption_options();
	int die;
	int idx;
	int i;

	if (choose_mut) attempts_left = 1;

	while (attempts_left--)
	{
		/*Throw the die*/
		die = choose_mut?choose_mut:randint(options);
		/* Find out which corruption we get */
		idx = get_corruption( die );
		/* Get the class this corruption belongs to*/
		muta_class = corruption_idx_to_u32b( corruptions[idx].idx );
		/* Get the bitflag for it */
		muta_which = corruptions[idx].bitflag;
		/* Get the description for it */
		muta_desc = corruptions[idx].gain;
		/*Check if we got something, probably paranoid*/
		if (muta_class && muta_which)
		{
			/*Check if the corruption isnt present already*/
			if (!(*(muta_class) & muta_which))
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen == TRUE) break;
	} /* End of while to try and get a corruption */

	if (!muta_chosen)
	{
		msg_print("You feel normal.");
		return FALSE;
	}
	else
	{
		if ( ( p_ptr->prace == VAMPIRE || p_ptr->prace == WEREWOLF ) &&
			!(p_ptr->muta2 & COR2_POISON_FANGS) &&
			(randint(10)<7))
		{
			muta_class = &(p_ptr->muta2);
			muta_which = COR2_POISON_FANGS;
			muta_desc = "Your fangs start to drip poison...";
		}

		else if (p_ptr->prace == IMP &&
			!(p_ptr->muta2 & COR2_CLAWS) &&
			(randint(10)<7))
		{
			muta_class = &(p_ptr->muta2);
			muta_which = COR2_CLAWS;
			muta_desc = "Your fingernails grow into razor sharp claws!";
		}

		else if (p_ptr->prace == IMP &&
			!(p_ptr->muta1 & COR1_SHRIEK) &&
			(randint(10)<7))
		{
			muta_class = &(p_ptr->muta1);
			muta_which = COR1_SHRIEK;
			muta_desc = "Your vocal cords get much tougher.";
		}

		else if (p_ptr->prace == DEVILSPAWN &&
			!(p_ptr->muta1 & COR1_POLYMORPH) &&
			(randint(10)<2))
		{
			muta_class = &(p_ptr->muta1);
			muta_which = COR1_POLYMORPH;
			muta_desc = "Your body seems mutable.";
		}

		else if (p_ptr->psign == SIGN_MORUI &&
			!(p_ptr->muta2 & COR2_SPINES) &&
			(randint(10)<7))
		{
			muta_class = &(p_ptr->muta2);
			muta_which = COR2_SPINES;
			muta_desc = "Bony spines grow from your elbows and knees.";
		}

		msg_print("You change!");
		msg_print(muta_desc);
		*(muta_class) |= muta_which;

		/*
		 * Some corruptions are the opposite of others, gaining them
		 * will screw up/cure their opposites. Gaining supercomputer brain (+4int/wis)
		 * will fix idiocy(-4wis/int). The opposite is true of course as well ( for now ).
		 */

		/* Go over all the opposed corruptions, NULL record has message NULL */
		for( i = 0 ; opposed_corruptions[i].message ; i++ )
		{
			/*An if to prevent at least some useless computing cycles*/
			if( muta_which == opposed_corruptions[i].bitflag_gain   )
			{
				/* Check the mutation class */
				if( muta_class ==  corruption_idx_to_u32b( opposed_corruptions[i].idx_gain ) )
				{
					/* Get the opposed corruption class and corruption index*/
					opposed_muta_class = corruption_idx_to_u32b( opposed_corruptions[i].idx );
					opposed_muta_which =  opposed_corruptions[i].bitflag;
					/*Do we have that corruption already*/
					if( *opposed_muta_class & opposed_muta_which )
					{
						/*Inform player that the opposed corruption has been lost*/
						msg_print(  opposed_corruptions[i].message );
						/*Update bitflag ( get rid of unwanted corruption ) */
						*opposed_muta_class &= ~(opposed_muta_which);
					}
				}
			}
		}

		p_ptr->update |= PU_BONUS;
		handle_stuff();
		return TRUE;
	}
}

bool lose_corruption(int choose_mut)
{
	int attempts_left = 20;
	cptr muta_desc = "";
	bool muta_chosen = FALSE;
	int muta_which = 0;
	u32b * muta_class = 0;
	int options = count_corruption_options();
	int die;
	int idx;

	if (choose_mut) attempts_left = 1;

	while (attempts_left--)
	{
		/*Throw the die*/
		die = choose_mut?choose_mut:randint(options);
		/* Find out which corruption we lose */
		idx = get_corruption( die );
		/* Get the class this corruption belongs to*/
		muta_class = corruption_idx_to_u32b( corruptions[idx].idx );
		/* Get the bitflag for it */
		muta_which = corruptions[idx].bitflag;
		/* Get the description for it */
		muta_desc = corruptions[idx].lose;
		/*Check if we got something, probably paranoid*/
		if (muta_class && muta_which)
		{
			/*Check if the corruption is already present*/
			if (*(muta_class) & muta_which)
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen == TRUE) break;
	} /* End of while to try and lose a corruption */

	if (!muta_chosen)
	{
		msg_print("You feel oddly normal.");
		return FALSE;
	}
	else
	{
		msg_print(muta_desc);
		*(muta_class) &= ~(muta_which);

		p_ptr->update |= PU_BONUS;
		handle_stuff();
		return TRUE;
	}
}

bool get_hack_dir(int *dp)
{
	int		dir;
	cptr    p;
	char command;


	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = 0;

	/* (No auto-targetting */

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;

		/* Convert various keys to "standard" keys */
		switch (command)
		{
			/* Use current target */
		case 'T':
		case 't':
		case '.':
		case '5':
		case '0':
			{
				dir = 5;
				break;
			}

			/* Set new target */
		case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}

		default:
			{
				dir = get_keymap_dir(command);
				break;
			}
		}

		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;

		/* Error */
		if (!dir) bell();
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	command_dir = dir;

	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[rand_int(8)];
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(dir);

#endif /* ALLOW_REPEAT -- TNB */

	/* A "valid" direction was entered */
	return (TRUE);
}


void dump_corruptions(FILE * OutFile)
{
	int i;
	u32b *muta_class = 0;
	if (!OutFile) return;

	for( i = 0 ; i < COUNT_CORRUPTIONS ; i++ )
	{
		muta_class = corruption_idx_to_u32b( corruptions[i].idx );
		if (*(muta_class) & corruptions[i].bitflag )
		{
			/*Dump the description*/
			fprintf(OutFile, "%s", corruptions[i].description);
			/*And a new line*/
			fprintf(OutFile, "\n");
		}
	}
}
