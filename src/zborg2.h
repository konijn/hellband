/* File: zborg2.h */
/* Purpose: Header file for "borg2.c" -BEN- */

#ifndef INCLUDED_BORG2_H
#define INCLUDED_BORG2_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg2.c".
 */

#include "zborg1.h"

/*
 * Used to see if square is transparent.
 * Trees now block sight only half the time.
 */
#define borg_cave_half_bold(Y,X) \
	((cave[Y][X].feat == FEAT_TREES) && (quick_rand()))

/*
 * Determine if a "legal" grid is a "clean" floor grid
 *
 * Line 1 -- forbid non-floors
 * Line 2 -- forbid deep water -KMW-
 * Line 3 -- forbid deep lava -KMW-
 * Line 4 -- forbid normal objects
 */
#define borg_cave_clean_bold(Y,X) \
	(((cave[Y][X].feat == FEAT_FLOOR) || \
	  (cave[Y][X].feat == FEAT_SHAL_WATER) || \
	  (cave[Y][X].feat == FEAT_SHAL_LAVA) || \
	  (cave[Y][X].feat == FEAT_GRASS) || \
	  (cave[Y][X].feat == FEAT_DIRT)) && \
	  (cave[Y][X].o_idx == 0))


extern bool borg_cave_floor_grid(borg_grid *ag);
extern bool borg_cave_floor_view(borg_grid *ag);
extern bool borg_cave_floor_bold(int Y, int X);
extern void borg_force_grid_los(int y, int x);

/*
 * Check a path for line of sight
 */
extern bool borg_los(int y1, int x1, int y2, int x2);

/*
 * Check the projection from (x1,y1) to (x2,y2)
 */
extern bool borg_projectable(int y1, int x1, int y2, int x2, bool stop_doors, bool stop_walls);
extern bool borg_offset_projectable(int y1, int x1, int y2, int x2);
extern sint borg_project_path(coord *gp, int range, int y1, int x1, int y2, int x2, u16b flg);
/*
 * Check the projection from (x1,y1) to (x2,y2).
 */
extern bool borg_projectable_pure(int y1, int x1, int y2, int x2);
extern bool borg_projectable_dark(int y1, int x1, int y2, int x2);

/*
 * Forget the "lite"
 */
extern void borg_forget_lite(void);

/*
 * Update the "lite"
 */
extern void borg_update_lite(void);

/*
 * Forget the "view"
 */
extern void borg_forget_view(void);

/*
 * Update the "view"
 */
extern void borg_update_view(void);


/*
 * Initialize this file
 */
extern void borg_init_2(void);


#endif


#endif
