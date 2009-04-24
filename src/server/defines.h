/* File: defines.h */

/* Purpose: server-side global constants and macro definitions */


#define	OPT_MAX 			37
#define	MAX_OPTION_GROUPS   4

/*
 * Available Options
 *
 * It is very important that all options stay together
 * without any gaps, OPT_MAX must not exceed them too.
 * Order, on the other hand, is not important.
 *
 *  *** WRONG ***			*** CORRECT ***
 * OPT_MAX       64			OPT_MAX       2
 * OPT_NO_GHOST  5          OPT_NO_GHOST  0
 * OPT_SUN_LIGHT 9          OPT_SUN_LIGHT 1
 */
#define OPT_NO_GHOST    		0
#define OPT_DEPTH_IN_FEET   	1
#define OPT_DUNGEON_ALIGN   	2
#define OPT_AVOID_OTHER 		3
#define OPT_USE_COLOR   		4
#define OPT_AUTO_SCUM       	5
#define OPT_EXPAND_LOOK 		6
#define OPT_SHOW_DETAILS    	7
#define OPT_CARRY_QUERY_FLAG	8
#define OPT_ALWAYS_PICKUP   	9
#define OPT_USE_OLD_TARGET  	10
#define OPT_STACK_FORCE_NOTES	11
#define OPT_STACK_FORCE_COSTS	12
#define OPT_STACK_ALLOW_ITEMS	13
#define OPT_STACK_ALLOW_WANDS	14
#define OPT_FIND_IGNORE_STAIRS	15
#define OPT_FIND_IGNORE_DOORS	16
#define OPT_FIND_CUT        	17
#define OPT_FIND_EXAMINE    	18
#define OPT_DISTURB_MOVE    	19
#define OPT_DISTURB_NEAR    	20
#define OPT_DISTURB_PANEL   	21
#define OPT_DISTURB_STATE   	22
#define OPT_DISTURB_MINOR   	23
#define OPT_DISTURB_OTHER   	24
#define OPT_VIEW_PERMA_GRIDS	25 
#define OPT_VIEW_TORCH_GRIDS	26 
#define OPT_VIEW_REDUCE_LITE	27
#define OPT_VIEW_REDUCE_VIEW	28
#define OPT_VIEW_YELLOW_LITE	29
#define OPT_VIEW_BRIGHT_LITE	30
#define OPT_VIEW_GRANITE_LITE	31
#define OPT_VIEW_SPECIAL_LITE	32
#define OPT_VIEW_ORANGE_LITE	33
#define OPT_EASY_ALTER			34
#define OPT_ALERT_HITPOINT		35
#define OPT_HILITE_LEADER   	36
#define option_p(A,B) (A->options[OPT_ ## B])


/*
 * item_tester_hook defines 
 */
#define ITH_WEAR        	1
#define ITH_WEAPON      	2
#define ITH_ARMOR       	3
#define ITH_AMMO        	4
#define ITH_RECHARGE       	5
#define ITH_ACTIVATE    	6
#define ITH_REFILL			7
#define item_test(A) (TV_MAX + (ITH_ ## A))
#define ITEM_ANY        	0

/*
 * item_tester_flag defines 
 */
#define ITF_WEAR	0x01 /* Can wear/wield item */
#define ITF_ACT 	0x02 /* Can activate item */
#define ITF_FUEL 	0x04 /* Can be used as fuel */
#define ITF_FLAG_0 	0x08 

#define ITF_FLAG_1 	ITEM_ASK_AIM  /* Reserved */ 
#define ITF_FLAG_2 	ITEM_ASK_ITEM /* Reserved */
#define ITF_FLAG_3 	0x40 /* Variant-specific */
#define ITF_FLAG_4 	0x80 /* Variant-specific */