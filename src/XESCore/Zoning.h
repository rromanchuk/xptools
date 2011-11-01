/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ZONING_H
#define ZONING_H

#include "AptDefs.h"
#include "MapDefs.h"
#include "ProgressUtils.h"

struct	DEMGeo;

/************************************************************************************************
 * LAND CLASS TABLES
 ************************************************************************************************/

struct LandClassInfo_t {
	int			category;								// 
	float		urban_density;
	float		veg_density;
};	

typedef hash_map<int, LandClassInfo_t>	LandClassInfoTable;
extern LandClassInfoTable gLandClassInfo;

/************************************************************************************************
 * ZOOOOOOOOOOOOOOOONING RULES
 ************************************************************************************************/

struct ZoningInfo_t {
	float		max_slope;								// Exclude areas that have slope > this value
	int			need_lu;								// Exclude areas that are not urban LU?
	int			fill_edge;								// Use rules to put AG along edges.
	int			fill_area;								// Use rules to put AG in interior areas
	int			fill_points;
	int			fill_veg;								// Use rules to put forests into remaining area
	int			terrain_type;							// Source terrain type required for this zoning.
};

struct ZoningRule_t {
	int			terrain;								// Required base terrain (or require natural)
	float		size_min,		size_max;				// Size of block in square meters
	float		slope_min,		slope_max;				// Max slope within block
	float		urban_avg_min,	urban_avg_max;			// Average urbanization level
	float		forest_avg_min,	forest_avg_max;			// Average forest level
	float		park_avg_min,	park_avg_max;			// Average park level
	float		bldg_min,		bldg_max;				// Maximum building height (0 if none)
	float		ang_min,		ang_max;				// Angle turn range.  Left turns are positive.
	int			sides_min,		sides_max;
	int			req_cat1;
	float		req_cat1_min;
	int			req_cat2;
	float		req_cat2_min;

	int			req_water;								// Are we adjacent to water (not counting holes/lakes)
	int			req_train;								// Train tracks on outer border?
	int			req_road;								// Roads with direct access (primary/secondary, not highway or ramp)

	float		min_side_len;							// Side length constraints
	float		max_side_len;
	float		block_err_max;							// Maximum acceptable block error
	float		min_side_major;							// Bounds range on the major and minor block axes
	float		max_side_major;
	float		min_side_minor;
	float		max_side_minor;

	set<int>	require_features;						// One of these features MUST be present or we can't use the rule.
	set<int>	consume_features;						// These features get consumed by the act of zoning.
	int			crud_ok;								// We can live with "stuff" in our zone that we haven't consumed.
	int			hole_ok;								// Is there a hole in the block?
	
	int			zoning;
};

struct LandFillRule_t {
	set<int>	required_zoning;
	int			color;
	int			terrain;
};

typedef vector<ZoningRule_t>	ZoningRuleTable;
typedef map<int, ZoningInfo_t>	ZoningInfoTable;
typedef vector<LandFillRule_t>	LandFillRuleTable;
extern ZoningRuleTable				gZoningRules;
extern ZoningInfoTable				gZoningInfo;
extern LandFillRuleTable			gLandFillRules;

/************************************************************************************************
 * 3-D FILL RULES
 ************************************************************************************************/

struct EdgeRule_t {
	int			zoning;
	int			variant;
	int			road_type;		
	int			resource_id;
	float		width;
};


struct FillRule_t {
	
	int			zoning;									// Base zoning we act upon
	
	int			road;
	
	int			variant;
	float		min_height;
	float		max_height;
	float		min_side_len;							// Side length constraints
	float		max_side_len;
	float		block_err_max;							// Maximum acceptable block error
	float		min_side_major;							// Bounds range on the major and minor block axes
	float		max_side_major;
	float		min_side_minor;
	float		max_side_minor;
	float		ang_min,ang_max;						// Angle turn range.  Left turns are positive.

//	float		slope_min,		slope_max;
//	
//	int			hole_ok;								// Okay to have holes or interruptions in our block?
	
	float		agb_min_width;
	float		agb_slop_width;
	float		agb_slop_depth;
	float		fac_width;
	float		fac_depth;								// 0 if no subdivide in the back
	float		fac_extra;								// add these to 
	
	int			agb_id;
	int			fac_id;
	int			ags_id;
	
	
//	float		bldg_min,		bldg_max;
//
//	int			adj_terrain;							// These locate an adjacent edge with a certain property.   We will key
//	int			adj_network;							// the feature to align to this edge.
//	int			adj_zoning;

};	

struct PointRule_t {
	int		zoning;
	int		feature;
	float	height_min;
	float	height_max;
	float	width;
	float	depth;
	int		fac_id;
};


typedef vector<EdgeRule_t>		EdgeRuleTable;
typedef vector<FillRule_t>		FillRuleTable;
typedef vector<PointRule_t>		PointRuleTable;
extern EdgeRuleTable			gEdgeRules;
extern FillRuleTable			gFillRules;
extern PointRuleTable			gPointRules;

void	LoadZoningRules(void);


/*
 * Given a map and various raster parameters, go through the map and
 * assign terrain types to all polygons as appropriate.  This includes
 * terrain type based on area features or terrain types based on
 * urban zoning.
 *
 */
void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
				const DEMGeo&		inPark,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				Pmwx::Face_handle	inDebug,
				ProgressFunc		inProg);
				
FillRule_t * GetFillRuleForBlock(Pmwx::Face_handle f);

PointRule_t * GetPointRuleForFeature(int zoning, const GISPointFeature_t& f);

#endif /* ZONING_H */