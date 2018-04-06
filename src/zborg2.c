/* File: borg2.c */
/* Purpose: Low level dungeon mapping skills -BEN- */

#include "angband.h"


#ifdef ALLOW_BORG

#include "zborg1.h"
#include "zborg2.h"


/*
 * This file helps the Borg understand mapping the dungeon.
 *
 * Currently, this includes general routines involving dungeon grids,
 * including calculating "flow" values from place to place, determining
 * "line of sight", plus "field of view" and "torch-lit grids", setting
 * the target to a given location, and extracting the optimal direction
 * for "movement" from place to place.
 *
 * Note that the dungeon is assumed smaller than 256 by 256.
 *
 * This file also supplies the (compiled out) support for "automatic
 * room extraction".  This code will automatically group regions of
 * the dungeon into rooms, and do the "flow" navigation on those rooms
 * instead of on grids.  Often, this takes less space, and is faster,
 * howver, it is more complicated, and does not allow "specialized"
 * flow calculations that penalize grids by variable amounts.
 */

/*
 * This macro allows us to efficiently add a grid to the "view" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "view" array, and we are never
 * called when the "view" array is full.
 */
#define borg_cave_view_hack(A,Y,X) \
    (A)->info |= BORG_VIEW; \
    borg_view_y[borg_view_n] = (Y); \
    borg_view_x[borg_view_n] = (X); \
    borg_view_n++

/*
 * Attempt to improve viewable, projectable, and LOS based on
 * changing events.
 */
bool borg_dynamic_view(int y1, int x1, int dist)
{
	borg_grid *ag;

	/* Access the grid */
    ag = &borg_grids[y1][x1];

    /* Assume all unknown grids more than distance 10 from you
     * are walls--when I am wounded.
	 */
    if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
    {
        if ((dist > 10) && (ag->feat == FEAT_NONE)) return (FALSE);
    }
	else if (borg_fear_region[c_y/11][c_x/11] >= avoidance / 20)
	{
		/* If a non-LOS monster is attacking me, then it is probably has
			* LOS to me, so do not place walls on unknown grids.  This will allow
			* me the chance to attack monsters.
			*
			* This does not work if the non-LOS monster is invisible.
			* This helps in a case like this:
			*####################			1.  Player has ESP and can sense the priest.
			*......@......      p			2.  Priest has cast a spell at the player.
			*#############					3.  Unknown grids are between player and priest
			*								4.  Borg has created regional fear from non-LOS priest.
			*
			*/
        if ((dist > MAX_RANGE) && (ag->feat == FEAT_NONE)) return (FALSE);
	}
	else if (borg_detect_wall[y1 / PANEL_HGT][x1 / PANEL_WID] == TRUE)
	{
			/* This area has been magic mapped, so I should be able to see the unknown grids */
			return (TRUE);
	}
	else
    {
        /* Assume all unknown grids more than distance 3 from my light limit are walls. */
        if ((dist > borg_skill[BI_CUR_LITE] + 2) && (ag->feat == FEAT_NONE)) return (FALSE);
    }

	/* Otherwise, its OK to be considered viewable */
	return (TRUE);
}

/* If a monster is sitting in a darkened area and the borg has ESP, he may not realize that the monster
 * has full LOS to him.  If that monster casts a spell, then it surely has LOS from it's current location.
 * This routine will force the unknown grids between the borg and the monster to be floor grids and therefore
 * BORG_VIEW.
 */
void borg_force_grid_los(int y9, int x9)
{

    int dist, y, x;
	int y1, x1;


    borg_grid *ag;

    /* Start at the initial location */
    y = y1 = c_y; x = x1 = c_x;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
		/* Check bounds */
		if (!in_bounds(y,x)) continue;

        /* Get the grid */
        ag = &borg_grids[y][x];

		/* Skip the non-unknown grids */
		if (!(ag->info & BORG_VIEW))
		{
			/* Force floor */
			ag->feat = FEAT_FLOOR;

			/* Force viewable */
			borg_cave_view_hack(ag, y, x);
		}

		/* Check for arrival at "final target" */
        if ((x == x9) && (y == y9)) return;

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y9, x9);
    }

    /* Done */
    return;
}



/*
 * A simple, fast, integer-based line-of-sight algorithm.
 *
 * See "los()" in "cave.c" for complete documentation
 */
bool borg_los(int y1, int x1, int y2, int x2)
{

 if (borg_projectable(y1,x1,y2,x2, TRUE, TRUE)) return (TRUE);
 return (FALSE);

}

/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that there is no monster in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 * stop_door if true will effectively block vision, sometimes its nice to pretend the door
 * is not there. so we set it as false
 */
bool borg_projectable(int y1, int x1, int y2, int x2, bool stop_doors, bool stop_walls)
{
    int dist, y, x;
	bool borg_passwalling = FALSE;

    borg_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
		/* Check bounds */
		if (!in_bounds(y,x)) continue;

        /* Get the grid */
        ag = &borg_grids[y][x];

		/* Is the borg in this wall grid? */
		if (borg_skill[BI_PASSWALL] && y == c_y && x == c_x &&
			((ag->feat >= FEAT_SECRET && ag->feat <= FEAT_WALL_SOLID) ||
			(ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL))) borg_passwalling = TRUE;

		/* Apply some dynamic decisions on viewable grids based on a few factors */
		if (!borg_dynamic_view(y, x, dist)) break;

        /* Do not pass through doors if requested */
        if (dist && !borg_passwalling && stop_doors == TRUE &&
			(ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL)) break;


		/* Make sure I stop at walls */
		if (dist && (stop_walls || !borg_passwalling) &&
					(ag->feat == FEAT_RUBBLE ||
					 ag->feat == FEAT_MAGMA ||
					 ag->feat == FEAT_MAGMA_K ||
					 ag->feat == FEAT_WALL_EXTRA ||
					 ag->feat == FEAT_WALL_INNER ||
					 ag->feat == FEAT_WALL_OUTER ||
					 ag->feat == FEAT_WALL_SOLID ||
					 ag->feat == FEAT_PERM_SOLID ||
					 ag->feat == FEAT_PERM_BUILDING ||
					 ag->feat == FEAT_PERM_INNER ||
					 ag->feat == FEAT_PERM_OUTER ||
					 ag->feat == FEAT_PERM_INNER ||
					 ag->feat == FEAT_PERM_SOLID ||
					 ag->feat == FEAT_WALL_EXTRA)) break;

		/* Cheat the game code at the end.  Rarely the borg might not calculate the outside margin correctly. */
		if (!player_has_los_bold(y, x)) return (FALSE);

		/* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}


/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that there is no monster in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 * This is used by borg_offset()
 */
bool borg_offset_projectable(int y1, int x1, int y2, int x2)
{
    int dist, y, x;

    borg_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Assume all unknown grids are walls. */
        if ((dist) && (ag->feat == FEAT_NONE)) break;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}


/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that monsters in the way will stop the projection
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 */
bool borg_projectable_pure(int y1, int x1, int y2, int x2)
{
    int dist, y, x;
    borg_grid *ag;
	bool borg_passwalling = FALSE;


    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

		/* Is the borg in this wall grid? */
		if (borg_skill[BI_PASSWALL] && y == c_y && x == c_x &&
			((ag->feat >= FEAT_SECRET && ag->feat <= FEAT_WALL_SOLID) ||
			(ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL))) borg_passwalling = TRUE;

		/* Hack -- assume unknown grids are walls */
        if (dist && (ag->feat == FEAT_NONE)) return (FALSE);

        /* Do not pass through doors if requested */
        if (dist && !borg_passwalling &&
			(ag->feat >= FEAT_DOOR_HEAD && ag->feat <= FEAT_DOOR_TAIL)) return (FALSE);


		/* Make sure I stop at walls */
		if (dist && !borg_passwalling &&
					(ag->feat == FEAT_RUBBLE ||
					 ag->feat == FEAT_MAGMA ||
					 ag->feat == FEAT_MAGMA_K ||
					 ag->feat == FEAT_WALL_EXTRA ||
					 ag->feat == FEAT_WALL_INNER ||
					 ag->feat == FEAT_WALL_OUTER ||
					 ag->feat == FEAT_WALL_SOLID ||
					 ag->feat == FEAT_PERM_SOLID ||
					 ag->feat == FEAT_PERM_BUILDING ||
					 ag->feat == FEAT_PERM_INNER ||
					 ag->feat == FEAT_PERM_OUTER ||
					 ag->feat == FEAT_PERM_INNER ||
					 ag->feat == FEAT_PERM_SOLID ||
					 ag->feat == FEAT_WALL_EXTRA)) return (FALSE);

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Stop at other monsters */
        if (ag->kill && (x != x1 || y != y1)) return (FALSE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}


/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that monsters in the way will stop the projection.
 * Assume that an unknown grid is a floor grid.
 * We want at least one unknown grid.
 *
 * This routine is used mainly aiming beams of light and
 * shooting into darkness, testing the projection path.
 *
*/
bool borg_projectable_dark(int y1, int x1, int y2, int x2)
{
    int dist, y, x;
    int unknown = 0;
    borg_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* We want at least 1 unknown grid */
        if (dist && (ag->feat == FEAT_NONE)) unknown ++;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2) && unknown >= 1) return (TRUE);

        /* Stop at monsters */
        if (ag->kill && dist != 0) break;

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }
    /* Assume obstruction */
    return (FALSE);
}


/*
 * Clear the lite grids
 */
void borg_forget_lite(void)
{
    int i;

    /* None to forget */
    if (!borg_lite_n) return;

    /* Clear them all */
    for (i = 0; i < borg_lite_n; i++)
    {
        int y = borg_lite_y[i];
        int x = borg_lite_x[i];

        /* Forget that the grid is lit */
        borg_grids[y][x].info &= ~BORG_LITE;
    }

    /* None left */
    borg_lite_n = 0;
}

/*
 * This allows us to efficiently add a grid to the "lite" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "lite" array, and we are never
 * called when the "lite" array is full.
 */
void borg_cave_lite_hack(int Y, int X)
{
	if (Y >= 0 && Y < AUTO_MAX_Y &&
    	X >= 0 && X < AUTO_MAX_X)
    {
    	borg_grids[Y][X].info |= BORG_LITE;
    	borg_lite_y[borg_lite_n] = (Y);
    	borg_lite_x[borg_lite_n] = (X);
    	borg_lite_n++;
	}
}

bool borg_cave_floor_view(borg_grid *ag)
{
		if ((ag->feat == FEAT_NONE) ||
			(ag->feat == FEAT_INVIS) ||
			(ag->feat == FEAT_FLOOR) ||
			(ag->feat >= FEAT_TRAP_HEAD && ag->feat <= FEAT_TRAP_TAIL) ||
			(ag->feat == FEAT_LESS) ||
			(ag->feat == FEAT_MORE) ||
			(ag->feat == FEAT_BROKEN) ||
			(ag->feat == FEAT_OPEN) ||
			(ag->feat == FEAT_GLYPH) ||
			(ag->feat == FEAT_WATER)) return (TRUE);
			/*(ag->feat == FEAT_LAVA) ||*/
  return FALSE;
}

/*
 * Grid based version of "borg_cave_floor_bold()"
 */
bool borg_cave_floor_grid(borg_grid *ag)
{
		if ((ag->feat == FEAT_FLOOR) ||
			(ag->feat >= FEAT_TRAP_HEAD && ag->feat <= FEAT_TRAP_TAIL) ||
			(ag->feat == FEAT_LESS) ||
			(ag->feat == FEAT_MORE) ||
			(ag->feat == FEAT_BROKEN) ||
			(ag->feat == FEAT_OPEN) ||
			(ag->feat == FEAT_GLYPH) ||
			(ag->feat == FEAT_WATER)) return (TRUE);
			/*(ag->feat == FEAT_LAVA) */
	return FALSE;
}

/*
 * Determine if a grid is a floor grid or other grid I can see through
 */
bool borg_cave_floor_bold(int Y, int X)
{
	if (Y >= 0 && Y < AUTO_MAX_Y &&
		X >= 0 && X < AUTO_MAX_X)
	{
		if ((borg_grids[Y][X].feat == FEAT_FLOOR) ||
			(borg_grids[Y][X].feat >= FEAT_TRAP_HEAD && borg_grids[Y][X].feat <= FEAT_TRAP_TAIL) ||
			(borg_grids[Y][X].feat == FEAT_LESS) ||
			(borg_grids[Y][X].feat == FEAT_MORE) ||
			(borg_grids[Y][X].feat == FEAT_BROKEN) ||
			(borg_grids[Y][X].feat == FEAT_OPEN) ||
			(borg_grids[Y][X].feat == FEAT_GLYPH) ||
			(borg_grids[Y][X].feat == FEAT_WATER)) return (TRUE);
			/*(borg_grids[Y][X].feat == FEAT_DEEP_LAVA) ||*/
	}
	return FALSE;
}

/*
 * Determine if a "legal" grid is an "empty" floor grid
 *
 * Line 1 -- forbid doors, rubble, seams, walls
 * Line 2 -- forbid normal monsters
 * Line 3 -- forbid the player
 * Line 4 -- forbid an object
 */
#define borg_cave_empty_bold(Y,X) \
    (borg_cave_floor_bold(Y,X) && \
     !(borg_grids[Y][X].kill) && \
     !(((Y) == c_y) && ((X) == c_x)) && \
	 !(borg_grids[Y][X].take))


/*
 * Update the set of grids "illuminated" by the player's lite.
 *
 * See "update_lite" in "cave.c" for complete documentation
 *
 * It is very important that the "player grid" be the first grid in the
 * array of "BORG_LITE" grids, since this is assumed in several places.
 */
void borg_update_lite(void)
{
    int i, x, y, min_x, max_x, min_y, max_y;


    /*** Clear old grids ***/

    /* Clear them all */
    for (i = 0; i < borg_lite_n; i++)
    {
        y = borg_lite_y[i];
        x = borg_lite_x[i];

		/* bounds check */
		if (!in_bounds(y,x)) continue;

        /* Mark the grid as not "lite" */
        if (borg_grids[y][x].info & BORG_LITE) borg_grids[y][x].info &= ~BORG_LITE;
    }

    /* None left */
    borg_lite_n = 0;

    /* Hack -- Player has no lite */
    if (borg_skill[BI_CUR_LITE] <= 0) return;


    /*** Collect the new "lite" grids ***/

    /* Player grid */
    borg_cave_lite_hack(c_y, c_x);

    /* Radius 1 -- torch radius */
    if (borg_skill[BI_CUR_LITE] >= 1)
    {
        /* Adjacent grid */
        borg_cave_lite_hack(c_y+1, c_x);
        borg_cave_lite_hack(c_y-1, c_x);
        borg_cave_lite_hack(c_y, c_x+1);
        borg_cave_lite_hack(c_y, c_x-1);

        /* Diagonal grids */
        borg_cave_lite_hack(c_y+1, c_x+1);
        borg_cave_lite_hack(c_y+1, c_x-1);
        borg_cave_lite_hack(c_y-1, c_x+1);
        borg_cave_lite_hack(c_y-1, c_x-1);
    }

    /* Radius 2 -- lantern radius */
    if (borg_skill[BI_CUR_LITE] >= 2)
    {
        /* South of the player */
        if (borg_cave_floor_bold(c_y+1, c_x))
        {
            borg_cave_lite_hack(c_y+2, c_x);
            borg_cave_lite_hack(c_y+2, c_x+1);
            borg_cave_lite_hack(c_y+2, c_x-1);
        }

        /* North of the player */
        if (borg_cave_floor_bold(c_y-1, c_x))
        {
            borg_cave_lite_hack(c_y-2, c_x);
            borg_cave_lite_hack(c_y-2, c_x+1);
            borg_cave_lite_hack(c_y-2, c_x-1);
        }

        /* East of the player */
        if (borg_cave_floor_bold(c_y, c_x+1))
        {
            borg_cave_lite_hack(c_y, c_x+2);
            borg_cave_lite_hack(c_y+1, c_x+2);
            borg_cave_lite_hack(c_y-1, c_x+2);
        }

        /* West of the player */
        if (borg_cave_floor_bold(c_y, c_x-1))
        {
            borg_cave_lite_hack(c_y, c_x-2);
            borg_cave_lite_hack(c_y+1, c_x-2);
            borg_cave_lite_hack(c_y-1, c_x-2);
        }
    }

    /* Radius 3+ -- artifact radius */
    if (borg_skill[BI_CUR_LITE] >= 3)
    {
        int d, p;

        /* Maximal radius */
        p = borg_skill[BI_CUR_LITE];

        /* Paranoia -- see "LITE_MAX" */
        if (p > 5) p = 5;

        /* South-East of the player */
        if (borg_cave_floor_bold(c_y+1, c_x+1))
        {
            borg_cave_lite_hack(c_y+2, c_x+2);
        }

        /* South-West of the player */
        if (borg_cave_floor_bold(c_y+1, c_x-1))
        {
            borg_cave_lite_hack(c_y+2, c_x-2);
        }

        /* North-East of the player */
        if (borg_cave_floor_bold(c_y-1, c_x+1))
        {
            borg_cave_lite_hack(c_y-2, c_x+2);
        }

        /* North-West of the player */
        if (borg_cave_floor_bold(c_y-1, c_x-1))
        {
            borg_cave_lite_hack(c_y-2, c_x-2);
        }

        /* Maximal north */
        min_y = c_y - p;
        if (min_y < 0) min_y = 0;

        /* Maximal south */
        max_y = c_y + p;
        if (max_y > AUTO_MAX_Y-1) max_y = AUTO_MAX_Y-1;

        /* Maximal west */
        min_x = c_x - p;
        if (min_x < 0) min_x = 0;

        /* Maximal east */
        max_x = c_x + p;
        if (max_x > AUTO_MAX_X-1) max_x = AUTO_MAX_X-1;

        /* Scan the maximal box */
        for (y = min_y; y <= max_y; y++)
        {
            for (x = min_x; x <= max_x; x++)
            {
                int dy = (c_y > y) ? (c_y - y) : (y - c_y);
                int dx = (c_x > x) ? (c_x - x) : (x - c_x);

                /* Skip the "central" grids (above) */
                if ((dy <= 2) && (dx <= 2)) continue;

                /* Hack -- approximate the distance */
                d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

                /* Skip distant grids */
                if (d > p) continue;

                /* Viewable, nearby, grids get "torch lit" */
                if (borg_grids[y][x].info & BORG_VIEW)
                {
                    /* This grid is "torch lit" */
                    borg_cave_lite_hack(y, x);
                }
            }
        }
    }
}





/*
 * Clear the viewable space
 */
void borg_forget_view(void)
{
    int i;

    borg_grid *ag;

    /* None to forget */
    if (!borg_view_n) return;

    /* Clear them all */
    for (i = 0; i < borg_view_n; i++)
    {
        int y = borg_view_y[i];
        int x = borg_view_x[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Forget that the grid is viewable */
        ag->info &= ~BORG_VIEW;
    }

    /* None left */
    borg_view_n = 0;
}





/*
 * Helper function for "borg_update_view()" below
 *
 * See "update_view_aux()" in "cave.c" for complete documentation.
 */
static bool borg_update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, z1, z2;
	bool wall = TRUE;

    borg_grid *ag;

    borg_grid *g1_ag;
    borg_grid *g2_ag;

    /* Access the grids */
    g1_ag = &borg_grids[y1][x1];
    g2_ag = &borg_grids[y2][x2];


    /* Check for floors'ish */
    f1 = (borg_cave_floor_view(g1_ag));
    f2 = (borg_cave_floor_view(g2_ag));


    /* Totally blocked by physical walls */
    if (!f1 && !f2) return (TRUE);


    /* Check for visibility */
    v1 = (f1 && (g1_ag->info & BORG_VIEW));
    v2 = (f2 && (g2_ag->info & BORG_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Access the grid */
    ag = &borg_grids[y][x];

	/* Absolutely known non-floorish grids */
	if (!borg_cave_floor_bold(y,x))
	{
		if (distance(c_y, c_x, y, x) > 2 + borg_skill[BI_CUR_LITE]) wall = TRUE;
		else wall = FALSE;
	}
    /* Check for walls */
    else wall = (!borg_cave_floor_view(ag));


    /* Check the "ease" of visibility */
    z1 = (v1 && (g1_ag->info & BORG_XTRA));
    z2 = (v2 && (g2_ag->info & BORG_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2)
    {
        ag->info |= BORG_XTRA;

        borg_cave_view_hack(ag, y, x);

        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1)
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2)
    {
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Mega-Hack -- the "borg_los()" function works poorly on walls */
    if (wall)
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Hack -- check line of sight */
    if (borg_los(c_y, c_x, y, x))
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Assume no line of sight. */
    return (TRUE);
}



/*
 * Calculate the region currently "viewable" by the player
 *
 * See "update_view()" in "cave.c" for complete documentation
 *
 * It is very important that the "player grid" be the first grid in the
 * array of "BORG_VIEW" grids, since this is assumed in several places.
 */
void borg_update_view(void)
{
    int n, m, d, k, y, x, z;

    int se, sw, ne, nw, es, en, ws, wn;

    int full, over;

    borg_grid *ag;


    /*** Initialize ***/

    /* Normal */
    /* Full radius (20) */
    full = MAX_SIGHT;

    /* Octagon factor (30) */
    over = MAX_SIGHT * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < borg_view_n; n++)
    {
        y = borg_view_y[n];
        x = borg_view_x[n];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Mark the grid as not in "view" */
        ag->info &= ~(BORG_VIEW);
    }

    /* Start over with the "view" array */
    borg_view_n = 0;

    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    y = c_y;
    x = c_x;

	/* These locations Make Zborg crash in Wilderness */
	if (y == 1 || x == 1) return;
	if (y == 63 || x == 196) return;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Assume the player grid is easily viewable */
    ag->info |= BORG_XTRA;

    /* Assume the player grid is viewable */
    borg_cave_view_hack(ag, y, x);


    /*** Step 2 -- Major Diagonals ***/

    /* Hack -- Limit */
    z = full * 2 / 3;

    /* Scan south-east */
    for (d = 1; d <= z; d++)
    {
		if ((y + d > AUTO_MAX_Y-1) || (x + d > AUTO_MAX_X-1)) continue;
		ag = &borg_grids[y+d][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x+d);
		if (!borg_dynamic_view(y+d, x+d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= z; d++)
    {
		if ((y + d > AUTO_MAX_Y-1) || (x - d < 0)) continue;
        ag = &borg_grids[y+d][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x-d);
		if (!borg_dynamic_view(y+d, x-d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= z; d++)
    {
		if ((y - d < 0) || (x + d > AUTO_MAX_X-1)) continue;
        ag = &borg_grids[y-d][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x+d);
		if (!borg_dynamic_view(y-d, x+d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= z; d++)
    {
		if ((y - d < 0) || (x - d < 0)) continue;
        ag = &borg_grids[y-d][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x-d);
		if (!borg_dynamic_view(y-d, x-d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++)
    {
		if ((y + d > AUTO_MAX_Y-1) || (x > AUTO_MAX_X-1)) continue;
        ag = &borg_grids[y+d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x);
		if (!borg_dynamic_view(y+d, x, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++)
    {
		if ((y - d < 0) || (x > AUTO_MAX_X-1)) continue;
        ag = &borg_grids[y-d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x);
		if (!borg_dynamic_view(y-d, x, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++)
    {
		if ((y > AUTO_MAX_Y-1) || (x + d > AUTO_MAX_X-1)) continue;
        ag = &borg_grids[y][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x+d);
		if (!borg_dynamic_view(y, x+d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++)
    {
		if ((y > AUTO_MAX_Y-1) || (x - d < 0)) continue;
        ag = &borg_grids[y][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x-d);
		if (!borg_dynamic_view(y, x-d, d)) break;
        if (!borg_cave_floor_view(ag)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++)
    {
        int ypn, ymn, xpn, xmn;


        /* Acquire the "bounds" of the maximal circle */
        z = over - n - n;
        if (z > full - n) z = full - n;
        while ((z + n + (n>>1)) > full) z--;


        /* Access the four diagonal grids */
        ypn = y + n;
        ymn = y - n;
        xpn = x + n;
        xmn = x - n;


        /* South strip */
        if (ypn < AUTO_MAX_Y-1)
        {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_Y-1) - ypn);

            /* East side */
            if ((xpn <= AUTO_MAX_X-1) && (n < se))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn+d, xpn, ypn+d-1, xpn-1, ypn+d-1, xpn))
                    {
                        if (n + d >= se) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                se = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < sw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn+d, xmn, ypn+d-1, xmn+1, ypn+d-1, xmn))
                    {
                        if (n + d >= sw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                sw = k + 1;
            }
        }


        /* North strip */
        if (ymn > 0)
        {
            /* Maximum distance */
            m = MIN(z, ymn);

            /* East side */
            if ((xpn <= AUTO_MAX_X-1) && (n < ne))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn-d, xpn, ymn-d+1, xpn-1, ymn-d+1, xpn))
                    {
                        if (n + d >= ne) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ne = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < nw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn-d, xmn, ymn-d+1, xmn+1, ymn-d+1, xmn))
                    {
                        if (n + d >= nw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                nw = k + 1;
            }
        }


        /* East strip */
        if (xpn < AUTO_MAX_X-1)
        {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_X-1) - xpn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y-1) && (n < es))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xpn+d, ypn-1, xpn+d-1, ypn, xpn+d-1))
                    {
                        if (n + d >= es) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                es = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < en))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xpn+d, ymn+1, xpn+d-1, ymn, xpn+d-1))
                    {
                        if (n + d >= en) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                en = k + 1;
            }
        }


        /* West strip */
        if (xmn > 0)
        {
            /* Maximum distance */
            m = MIN(z, xmn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y-1) && (n < ws))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xmn-d, ypn-1, xmn-d+1, ypn, xmn-d+1))
                    {
                        if (n + d >= ws) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ws = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < wn))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xmn-d, ymn+1, xmn-d+1, ymn, xmn-d+1))
                    {
                        if (n + d >= wn) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                wn = k + 1;
            }
        }
    }


    /*** Step 5 -- Complete the algorithm ***/

    /* Update all the new grids */
    for (n = 0; n < borg_view_n; n++)
    {
        y = borg_view_y[n];
        x = borg_view_x[n];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Clear the "BORG_XTRA" flag */
        ag->info &= ~BORG_XTRA;
    }
}

sint borg_project_path(coord *gp, int range, int y1, int x1, int y2, int x2, u16b flg)
{
	int y, x;

	int n = 0;
	int k = 0;

	/* Absolute */
	int ay, ax;

	/* Offsets */
	int sy, sx;

	/* Fractions */
	int frac;

	/* Scale factors */
	int full, half;

	/* Slope */
	int m;


	/* No path necessary (or allowed) */
	if ((x1 == x2) && (y1 == y2)) return (0);


	/* Analyze "dy" */
	if (y2 < y1)
	{
		ay = (y1 - y2);
		sy = -1;
	}
	else
	{
		ay = (y2 - y1);
		sy = 1;
	}

	/* Analyze "dx" */
	if (x2 < x1)
	{
		ax = (x1 - x2);
		sx = -1;
	}
	else
	{
		ax = (x2 - x1);
		sx = 1;
	}


	/* Number of "units" in one "half" grid */
	half = (ay * ax);

	/* Number of "units" in one "full" grid */
	full = half << 1;


	/* Vertical */
	if (ay > ax)
	{
		/* Start at tile edge */
		frac = ax * ax;

		/* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = y1 + sy;
		x = x1;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n].x = x;
			gp[n].y = y;
			n++;

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && !borg_cave_floor_bold(y, x)) break;

			/* Sometimes stop at non-initial monsters/players */
			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (borg_grids[y][x].kill != 0)) break;
			}

			/* Slant */
			if (m)
			{
				/* Advance (X) part 1 */
				frac += m;

				/* Horizontal change */
				if (frac >= half)
				{
					/* Advance (X) part 2 */
					x += sx;

					/* Advance (X) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (Y) */
			y += sy;
		}
	}

	/* Horizontal */
	else if (ax > ay)
	{
		/* Start at tile edge */
		frac = ay * ay;

		/* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = y1;
		x = x1 + sx;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n].x = x;
			gp[n].y = y;
			n++;

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && !borg_cave_floor_bold(y, x)) break;

			/* Sometimes stop at non-initial monsters/players */
			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (borg_grids[y][x].kill != 0)) break;
			}

			/* Slant */
			if (m)
			{
				/* Advance (Y) part 1 */
				frac += m;

				/* Vertical change */
				if (frac >= half)
				{
					/* Advance (Y) part 2 */
					y += sy;

					/* Advance (Y) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (X) */
			x += sx;
		}
	}

	/* Diagonal */
	else
	{
		/* Start */
		y = y1 + sy;
		x = x1 + sx;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n].x = x;
			gp[n].y = y;
			n++;

			/* Hack -- Check maximum range */
			if ((n + (n >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && !borg_cave_floor_bold(y, x)) break;

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (borg_grids[y][x].kill != 0)) break;
			}

			/* Advance (Y) */
			y += sy;

			/* Advance (X) */
			x += sx;
		}
	}


	/* Length */
	return (n);
}

/*
 * Init this file.
 */
void borg_init_2(void)
{
    /* Nothing */
}

#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
