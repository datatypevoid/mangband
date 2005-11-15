

/* Purpose: Angband game engine */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"
#include "externs.h"



/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
static cptr value_check_aux1(object_type *o_ptr)
{
	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "terrible";

		/* Normal */
		return "special";
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return "worthless";

		/* Normal */
		return "excellent";
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items */
	if (broken_p(o_ptr)) return "broken";

	/* Good "armor" bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/* Default to "average" */
	return "average";
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static cptr value_check_aux2(object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return "cursed";

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return "broken";

	/* Artifacts -- except cursed/broken ones */
	if (artifact_p(o_ptr)) return "good";

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return "good";

	/* Good armor bonus */
	if (o_ptr->to_a > 0) return "good";

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return "good";

	/* No feeling */
	return (NULL);
}




/*
 * Sense the inventory
 *
 *   Class 0 = Warrior --> fast and heavy
 *   Class 1 = Mage    --> slow and light
 *   Class 2 = Priest  --> fast but light
 *   Class 3 = Rogue   --> okay and heavy
 *   Class 4 = Ranger  --> okay and heavy
 *   Class 5 = Paladin --> slow but heavy
 */
static void sense_inventory(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i;

	int		plev = p_ptr->lev;

	bool	heavy = FALSE;

	cptr	feel;

	object_type *o_ptr;

	char o_name[80];


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (p_ptr->confused) return;

	/* Analyze the class */
	switch (p_ptr->pclass)
	{
		case CLASS_WARRIOR:
		{
			/* Good sensing */
			if (0 != rand_int(9000L / (plev * plev + 40))) return;

			/* Heavy sensing */
			heavy = TRUE;

			/* Done */
			break;
		}

		case CLASS_MAGE:
		{
			/* Very bad (light) sensing */
			if (0 != rand_int(240000L / (plev + 5))) return;

			/* Done */
			break;
		}

		case CLASS_PRIEST:
		{
			/* Good (light) sensing */
			if (0 != rand_int(10000L / (plev * plev + 40))) return;

			/* Done */
			break;
		}

		case CLASS_ROGUE:
		{
			/* Okay sensing */
			if (0 != rand_int(20000L / (plev * plev + 40))) return;

			/* Heavy sensing */
			heavy = TRUE;

			/* Done */
			break;
		}

		case CLASS_RANGER:
		{
			if (0 != rand_int(20000L / (plev * plev + 40))) return;

			/* Heavy sensing */
			heavy = TRUE;

			/* Done */
			break;
		}

		case CLASS_PALADIN:
		{
			/* Bad sensing */
			if (0 != rand_int(80000L / (plev * plev + 40))) return;

			/* Heavy sensing */
			heavy = TRUE;

			/* Done */
			break;
		}
	}


	/*** Sense everything ***/

	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool okay = FALSE;

		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
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
			{
				okay = TRUE;
				break;
			}
		}

		/* Skip non-sense machines */
		if (!okay) continue;

		/* We know about it already, do not tell us again */
		if (o_ptr->ident & ID_SENSE) continue;

		/* It is fully known, no information needed */
		if (object_known_p(Ind, o_ptr)) continue;

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

		/* Check for a feeling */
		feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

		/* Skip non-feelings */
		if (!feel) continue;

		/* Stop everything */
		if (p_ptr->disturb_minor) disturb(Ind, 0, 0);

		/* Get an object description */
		object_desc(Ind, o_name, o_ptr, FALSE, 0);

		/* Message (equipment) */
		if (i >= INVEN_WIELD)
		{
			msg_format(Ind, "You feel the %s (%c) you are %s %s %s...",
			           o_name, index_to_label(i), describe_use(Ind, i),
			           ((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* Message (inventory) */
		else
		{
			msg_format(Ind, "You feel the %s (%c) in your pack %s %s...",
			           o_name, index_to_label(i),
			           ((o_ptr->number == 1) ? "is" : "are"), feel);
		}

		/* We have "felt" it */
		o_ptr->ident |= (ID_SENSE);

		/* Inscribe it textually */
		if (!o_ptr->note) o_ptr->note = quark_add(feel);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * Regenerate hit points				-RAK-
 */
static void regenhp(int Ind, int percent)
{
	player_type *p_ptr = Players[Ind];

	s32b        new_chp, new_chp_frac;
	int                   old_chp;

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
	/* Apply the healing */
	hp_player_quiet(Ind, new_chp >> 16);
	//p_ptr->chp += new_chp >> 16;   /* div 65536 */

	/* check for overflow -- this is probably unneccecary */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;

	/* handle fractional hit point healing */
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		hp_player_quiet(Ind, 1);
		p_ptr->chp_frac = new_chp_frac - 0x10000L;
	}
	else
	{
		p_ptr->chp_frac = new_chp_frac;
	}
}


/*
 * Regenerate mana points				-RAK-
 */
static void regenmana(int Ind, int percent)
{
	player_type *p_ptr = Players[Ind];

	s32b        new_mana, new_mana_frac;
	int                   old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += new_mana >> 16;	/* div 65536 */
	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = new_mana_frac - 0x10000L;
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}






/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
/* Note that since this is done in real time, monsters will regenerate
 * faster in game time the deeper they are in the dungeon.
 */
static void regen_monsters(void)
{
	int i, frac;

	/* Regenerate everyone */
	for (i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags2 & RF2_REGENERATE) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Update health bars */
			update_health(i);
		}
	}
}



/*
 * Handle certain things once every 50 game turns
 */

static void process_world(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		x, y;

	cave_type		*c_ptr;
	byte			*w_ptr;


	/* Every 50 game turns */
	if (turn % 50) return;


	/*** Check the Time and Load ***/
	/* The server will never quit --KLJ-- */

	/*** Handle the "town" (stores and sunshine) ***/

	/* While in town or wilderness */
	if (p_ptr->dun_depth <= 0)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if (!(turn % ((10L * TOWN_DAWN) / 2)))
		{
			int Depth = p_ptr->dun_depth;
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn % (10L * TOWN_DAWN)));

			/* Day breaks */
			if (dawn)
			{
				/* Message */
				msg_print(Ind, "The sun has risen.");
	
				/* Hack -- Scan the level */
				for (y = 0; y < MAX_HGT; y++)
				{
					for (x = 0; x < MAX_WID; x++)
					{
						/* Get the cave grid */
						c_ptr = &cave[Depth][y][x];
						w_ptr = &p_ptr->cave_flag[y][x];

						/* Assume lit */
						c_ptr->info |= CAVE_GLOW;

						/* Hack -- Memorize lit grids if allowed */
						if ((!Depth) && (p_ptr->view_perma_grids)) *w_ptr |= CAVE_MARK;

						/* Hack -- Notice spot */
						note_spot(Ind, y, x);						
					}			
				}
			}

			/* Night falls */
			else
			{
				/* Message  */
				msg_print(Ind, "The sun has fallen.");

				 /* Hack -- Scan the level */
				for (y = 0; y < MAX_HGT; y++)
				{					
					for (x = 0; x < MAX_WID; x++)
					{
						/*  Get the cave grid */
						c_ptr = &cave[Depth][y][x];
						w_ptr = &p_ptr->cave_flag[y][x];

						/*  Darken "boring" features */
						if (c_ptr->feat <= FEAT_INVIS && !(c_ptr->info & CAVE_ROOM))
						{
							  /* Forget the grid */ 
							c_ptr->info &= ~CAVE_GLOW;
							*w_ptr &= ~CAVE_MARK;

							  /* Hack -- Notice spot */
							note_spot(Ind, y, x);
						}						
					}
					
				}  				
			}

			/* Update the monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw map */
			p_ptr->redraw |= (PR_MAP);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);
		}
	}

	/* While in the dungeon */
	else
	{
		/*** Update the Stores ***/
		/*  Don't do this for each player.  In fact, this might be */
		/*  taken out entirely for now --KLJ-- */
	}


	/*** Process the monsters ***/
	/* Note : since monsters are added at a constant rate in real time,
	 * this corresponds in game time to them appearing at faster rates
	 * deeper in the dungeon.
	 */

	/* Check for creature generation */
	if (rand_int(MAX_M_ALLOC_CHANCE) == 0)
	{
		/* Set the monster generation depth */
		if (p_ptr->dun_depth >= 0)		
			monster_level = p_ptr->dun_depth;
		
		else monster_level = 2 + (wild_info[p_ptr->dun_depth].radius / 2);

		/* Make a new monster */
		if (p_ptr->dun_depth >= 0)
			(void)alloc_monster(p_ptr->dun_depth, MAX_SIGHT + 5, FALSE);
		else wild_add_monster(p_ptr->dun_depth);
	}
}



/*
 * Verify use of "wizard" mode
 */
#if 0
static bool enter_wizard_mode(void)
{
#ifdef ALLOW_WIZARD
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0002))
	{
		/* Mention effects */
		msg_print("Wizard mode is for debugging and experimenting.");
		msg_print("The game will not be scored if you enter wizard mode.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0002;
	}

	/* Success */
	return (TRUE);
#endif /* ALLOW_WIZARD */

	/* XXX XXX XXX Return FALSE if wizard mode is compiled out --KLJ-- */
	return (FALSE);
}
#endif


#ifdef ALLOW_WIZARD

/*
 * Verify use of "debug" commands
 */
static bool enter_debug_mode(void)
{
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0008))
	{
		/* Mention effects */
		msg_print("The debug commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use debug commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use debug commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0008;
	}

	/* Success */
	return (TRUE);
}

/*
 * Hack -- Declare the Wizard Routines
 */
extern int do_cmd_wizard(void);

#endif


#ifdef ALLOW_BORG

/*
 * Verify use of "borg" commands
 */
static bool enter_borg_mode(void)
{
	/* No permission */
	if (!can_be_wizard) return (FALSE);

	/* Ask first time */
	if (!(noscore & 0x0010))
	{
		/* Mention effects */
		msg_print("The borg commands are for debugging and experimenting.");
		msg_print("The game will not be scored if you use borg commands.");
		msg_print(NULL);

		/* Verify request */
		if (!get_check("Are you sure you want to use borg commands? "))
		{
			return (FALSE);
		}

		/* Mark savefile */
		noscore |= 0x0010;
	}

	/* Success */
	return (TRUE);
}

/*
 * Hack -- Declare the Ben Borg
 */
extern void do_cmd_borg(void);

#endif



/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 *
 * XXX XXX XXX Make some "blocks"
 *
 * This all happens "automagically" by the Input() function in netserver.c
 */
#if 0
static void process_command(void)
{
}
#endif

/*
 * Check for nearby players or monsters and attempt to do something useful.
 *
 * This function should only be called if the player is "lagging" and helpless
 * to do anything about the situation.  This is not intended to be incredibly
 * useful, merely to prevent deaths due to extreme lag.
 */
 /* This function returns a 0 if no attack has been performed, a 1 if an attack
  * has been performed and there are still monsters around, and a 2 if an attack
  * has been performed and all of the surrounding monsters are dead.
  */
  /* We now intelligently try to decide which monster to autoattack.  Our current
   * algorithm is to fight first Q's, then any monster that is 20 levels higher
   * than its peers, then the most proportionatly wounded monster, then the highest
   * level monster, then the monster with the least hit points.
   */ 
static int auto_retaliate(int Ind)
{
	player_type *p_ptr = Players[Ind], *q_ptr, *p_target_ptr = NULL, *prev_p_target_ptr = NULL;
	int i;
	monster_type *m_ptr, *m_target_ptr = NULL, *prev_m_target_ptr = NULL;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];

#if 0
		/* Skip non-connected players */
		if (q_ptr->conn == NOT_CONNECTED) continue;
#endif

		/* Skip players not at this depth */
		if (p_ptr->dun_depth != q_ptr->dun_depth) continue;

		/* Skip ourselves */
		if (Ind == i) continue;

		/* Skip players we aren't hostile to */
		if (!check_hostile(Ind, i)) continue;

		/* Skip players we cannot see */
		if (!p_ptr->play_vis[i]) continue;

		/* A hostile player is next to us */
		if (distance(p_ptr->py, p_ptr->px, q_ptr->py, q_ptr->px) == 1)
		{
			/* Figure out if this is the best target so far */
			if (p_target_ptr)
			{
				/* If we are 15 levels over the old target, make this
				 * player our new target.
				 */
				if ((q_ptr->lev - 15) >= p_target_ptr->lev)
				{
					prev_p_target_ptr = p_target_ptr;
					p_target_ptr = q_ptr;
				}
				/* Otherwise attack this player if he is more proportionatly
				 * wounded than our old target.
				 */
				else if (q_ptr->chp * p_target_ptr->mhp < p_target_ptr->chp * q_ptr->mhp)
				{
					prev_p_target_ptr = p_target_ptr;
					p_target_ptr = q_ptr;
				}
				/* If it is a tie attack the higher level player */
				else if (q_ptr->chp * p_target_ptr->mhp == p_target_ptr->chp * q_ptr->mhp)
				{
					if (q_ptr->lev > p_target_ptr->lev)
					{
						prev_p_target_ptr = p_target_ptr;
						p_target_ptr = q_ptr;
					}
					/* If it is a tie attack the player with less hit points */
					else if (q_ptr->lev == p_target_ptr->lev)
					{
						if (q_ptr->chp < p_target_ptr->chp)
						{
							prev_p_target_ptr = p_target_ptr;
							p_target_ptr = q_ptr;
						}
					}
				}
			}
			else
			{
				prev_p_target_ptr = p_target_ptr;
				p_target_ptr = q_ptr;
			}
		}
	}


	/* Check each monster */
	for (i = 1; i < m_max; i++)
	{
		m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters that aren't at this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Make sure that the player can see this monster */
		if (!p_ptr->mon_vis[i]) continue;

		/* A monster is next to us */
		if (distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx) == 1)
		{
			/* Figure out if this is the best target so far */
			if (m_target_ptr)
			{
				/* If it is a Q, then make it our new target. */
				/* We don't handle the case of choosing between two
				 * Q's because if the player is standing next to two Q's
				 * he deserves whatever punishment he gets.
				 */
				if (r_info[m_ptr->r_idx].d_char == 'Q')
				{
					prev_m_target_ptr = m_target_ptr;
					m_target_ptr = m_ptr;
				}
				/* Otherwise if it is 20 levels higher than everything
				 * else attack it.
				 */
				else if ((r_info[m_ptr->r_idx].level - 20) >=
						r_info[m_target_ptr->r_idx].level)
				{
					prev_m_target_ptr = m_target_ptr;
					m_target_ptr = m_ptr;
				}
				/* Otherwise if it is the most proportionatly wounded monster
				 * attack it.
				 */
				else if (m_ptr->hp * m_target_ptr->maxhp < m_target_ptr->hp * m_ptr->maxhp)
				{
					prev_m_target_ptr = m_target_ptr;
					m_target_ptr = m_ptr;
				}
				/* If it is a tie attack the higher level monster */
				else if (m_ptr->hp * m_target_ptr->maxhp == m_target_ptr->hp * m_ptr->maxhp)
				{
					if (r_info[m_ptr->r_idx].level > r_info[m_target_ptr->r_idx].level)
					{
						prev_m_target_ptr = m_target_ptr;
						m_target_ptr = m_ptr;
					}
					/* If it is a tie attack the monster with less hit points */
					else if (r_info[m_ptr->r_idx].level == r_info[m_target_ptr->r_idx].level)
					{
						if (m_ptr->hp < m_target_ptr->hp)
						{
							prev_m_target_ptr = m_target_ptr;
							m_target_ptr = m_ptr;
						}
					}
				}
			}
			else
			{
				prev_m_target_ptr = m_target_ptr;
				m_target_ptr = m_ptr;
			}
		}
	}

	/* If we have a player target, attack him. */
	if (p_target_ptr)
	{
		/* Attack him */
		py_attack(Ind, p_target_ptr->py, p_target_ptr->px);

		/* Check if he is still alive or another targets exists */
		if ((!p_target_ptr->death) || (prev_p_target_ptr) || (m_target_ptr))
		{
			/* We attacked something */
			return 1;
		}
		else
		{
			/* Otherwise return 2 to indicate we are no longer
			 * autoattacking anything.
			 */
			return 2;
		}
	}

	/* The dungeon master does not fight his or her offspring */
	if (!strcmp(p_ptr->name, cfg_dungeon_master)) return FALSE;

	/* If we have a target to attack, attack it! */
	if (m_target_ptr)
	{
		/* Attack it */
		py_attack(Ind, m_target_ptr->fy, m_target_ptr->fx);

		/* Check if it is still alive or another targets exists */
		if ((m_target_ptr->r_idx) || (prev_m_target_ptr) || (p_target_ptr))
		{
			/* We attacked something */
			return 1;
		}
		else
		{
			/* Otherwise return 2 to indicate we are no longer
			 * autoattacking anything.
			 */
			return 2;
		}
	}

	/* Nothing was attacked. */
	return 0;
}

/*
 * Player processing that occurs at the beginning of a new turn
 */
static void process_player_begin(int Ind)
{
	player_type *p_ptr = Players[Ind];


	/* Give the player some energy */
	p_ptr->energy += extract_energy[p_ptr->pspeed];

	/* Make sure they don't have too much */
	/* But let them store up some extra */
	/* Storing up extra energy lets us perform actions while we are running */
	//if (p_ptr->energy > (level_speed(p_ptr->dun_depth)*6)/5)
	//	p_ptr->energy = (level_speed(p_ptr->dun_depth)*6)/5;
	if (p_ptr->energy > (level_speed(p_ptr->dun_depth)*2) - 1)
		p_ptr->energy = (level_speed(p_ptr->dun_depth)*2) - 1;

	/* Check "resting" status */
	if (p_ptr->resting)
	{
		/* No energy availiable while resting */
		// This prevents us from instantly waking up.
		p_ptr->energy = 0;
	}

	/* Handle paralysis here */
	if (p_ptr->paralyzed || p_ptr->stun >= 100)
		p_ptr->energy = 0;

	/* Hack -- semi-constant hallucination */
	if (p_ptr->image && (randint(10) < 1)) p_ptr->redraw |= (PR_MAP);

	/* Mega-Hack -- Random teleportation XXX XXX XXX */
	if ((p_ptr->teleport) && (rand_int(100) < 1))
	{
		/* Teleport player */
		teleport_player(Ind, 40);
	}

}


/*
 * Player processing that occurs at the end of a turn
 */
static void process_player_end(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int	i, j, new_depth, new_world_x, new_world_y;
	int	regen_amount, NumPlayers_old=NumPlayers;
	char	attackstatus;

	object_type		*o_ptr;

	/* Try to execute any commands on the command queue. */
	/* NB: process_pending may have deleted the connection! */
	if(process_pending_commands(p_ptr->conn)) return;

	/* Check for auto-retaliate */
	if ((p_ptr->energy >= level_speed(p_ptr->dun_depth)) && !p_ptr->confused)
	{
		/* Check for nearby monsters and try to kill them */
		/* If auto_retaliate returns nonzero than we attacked
		 * something and so should use energy.
		 */
		if ((attackstatus = auto_retaliate(Ind)))
		{
			/* Use energy */
			p_ptr->energy -= level_speed(p_ptr->dun_depth);
		}
	}

	/* Handle running -- 5 times the speed of walking */
	while (p_ptr->running && p_ptr->energy >= (level_speed(p_ptr->dun_depth)*6)/5)
	{
		run_step(Ind, 0);
		p_ptr->energy -= level_speed(p_ptr->dun_depth) / 5;
	}


	/* Notice stuff */
	if (p_ptr->notice) notice_stuff(Ind);

	/* XXX XXX XXX Pack Overflow */
	if (p_ptr->inventory[INVEN_PACK].k_idx)
	{
		int		amt;

		char	o_name[80];


		/* Choose an item to spill */
		i = INVEN_PACK;

		/* Access the slot to be dropped */
		o_ptr = &p_ptr->inventory[i];

		/* Drop all of that item */
		amt = o_ptr->number;

		/* Disturbing */
		disturb(Ind, 0, 0);

		/* Warning */
		msg_print(Ind, "Your pack overflows!");

		/* Describe */
		object_desc(Ind, o_name, o_ptr, TRUE, 3);

		/* Message */
		msg_format(Ind, "You drop %s.", o_name);

		/* Drop it (carefully) near the player */
		drop_near(o_ptr, 0, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Decrease the item, optimize. */
		inven_item_increase(Ind, i, -amt);
		inven_item_optimize(Ind, i);
	}


	/* Process things such as regeneration. */
	/* This used to be processed every 10 turns, but I am changing it to be
	 * processed once every 5/6 of a "dungeon turn". This will make healing
	 * and poison faster with respect to real time < 1750 feet and slower >
	 * 1750 feet.
	 */
	if (!(turn%(level_speed(p_ptr->dun_depth)/12)))
	{
		/*** Damage over Time ***/

		/* Take damage from poison */
		if (p_ptr->poisoned)
		{
			/* Take damage */
			take_hit(Ind, 1, "poison");
		}

		/* Take damage from cuts */
		if (p_ptr->cut)
		{
			/* Mortal wound or Deep Gash */
			if (p_ptr->cut > 200)
			{
				i = 3;
			}

			/* Severe cut */
			else if (p_ptr->cut > 100)
			{
				i = 2;
			}

			/* Other cuts */
			else
			{
				i = 1;
			}

			/* Take damage */
			take_hit(Ind, i, "a fatal wound");
		}

		/*** Check the Food, and Regenerate ***/

		/* Ghosts don't need food */
		/* don't use food in town */

		if ((!p_ptr->ghost) && (p_ptr->dun_depth>0))
		{
			/* Digest normally */
			if (p_ptr->food < PY_FOOD_MAX)
			{
				/* Every 50/6 level turns */
			        if (!(turn%((level_speed(p_ptr->dun_depth)/12)*10)))
				{
					/* Basic digestion rate based on speed */
					i = extract_energy[p_ptr->pspeed] * 2;

					/* Regeneration takes more food */
					if (p_ptr->regenerate) i += 30;

					/* Slow digestion takes less food */
					if (p_ptr->slow_digest) i -= 10;

					/* Digest some food */
					(void)set_food(Ind, p_ptr->food - i);

					/* Hack -- check to see if we have been kicked off
					 * due to starvation 
					 */

					if (NumPlayers != NumPlayers_old) return;
				}
			}

			/* Digest quickly when gorged */
			else
			{
				/* Digest a lot of food */
				(void)set_food(Ind, p_ptr->food - 100);
			}

			/* Starve to death (slowly) */
			if (p_ptr->food < PY_FOOD_STARVE)
			{
				/* Calculate damage */
				i = (PY_FOOD_STARVE - p_ptr->food) / 10;

				/* Take damage */
				take_hit(Ind, i, "starvation");
			}
		}

		/* Default regeneration */
		regen_amount = PY_REGEN_NORMAL;

		/* Getting Weak */
		if (p_ptr->food < PY_FOOD_WEAK)
		{
			/* Lower regeneration */
			if (p_ptr->food < PY_FOOD_STARVE)
			{
				regen_amount = 0;
			}
			else if (p_ptr->food < PY_FOOD_FAINT)
			{
				regen_amount = PY_REGEN_FAINT;
			}
			else
			{
				regen_amount = PY_REGEN_WEAK;
			}

			/* Getting Faint */
			if (!p_ptr->ghost && p_ptr->food < PY_FOOD_FAINT)
			{
				/* Faint occasionally */
				if (!p_ptr->paralyzed && (rand_int(100) < 10))
				{
					/* Message */
					msg_print(Ind, "You faint from the lack of food.");
					disturb(Ind, 1, 0);

					/* Hack -- faint (bypass free action) */
					(void)set_paralyzed(Ind, p_ptr->paralyzed + 1 + rand_int(5));
				}
			}
		}

		/* Regeneration ability */
		if (p_ptr->regenerate)
		{
			regen_amount = regen_amount * 2;
		}

		/* Resting */
		if (p_ptr->resting && !p_ptr->searching)
		{
			regen_amount = regen_amount * 3;
		}

		/* Regenerate the mana */
		/* Hack -- regenerate mana 5/3 times faster */
		if (p_ptr->csp < p_ptr->msp)
		{
			regenmana(Ind, (regen_amount * 5) / 3 );
		}

		/* Poisoned or cut yields no healing */
		if (p_ptr->poisoned) regen_amount = 0;
		if (p_ptr->cut) regen_amount = 0;

		/* Regenerate Hit Points if needed */
		if (p_ptr->chp < p_ptr->mhp)
		{
			regenhp(Ind, regen_amount);
		}

		/* Disturb if we are done resting */
		if ((p_ptr->resting) && (p_ptr->chp == p_ptr->mhp) && (p_ptr->csp == p_ptr->msp))
		{
			disturb(Ind, 0, 0);
		}

		/* Finally, at the end of our turn, update certain counters. */
		/*** Timeout Various Things ***/

		/* Hack -- Hallucinating */
		if (p_ptr->image)
		{
			(void)set_image(Ind, p_ptr->image - 1);
		}

		/* Blindness */
		if (p_ptr->blind)
		{
			(void)set_blind(Ind, p_ptr->blind - 1);
		}

		/* Times see-invisible */
		if (p_ptr->tim_invis)
		{
			(void)set_tim_invis(Ind, p_ptr->tim_invis - 1);
		}

		/* Timed infra-vision */
		if (p_ptr->tim_infra)
		{
			(void)set_tim_infra(Ind, p_ptr->tim_infra - 1);
		}

		/* Paralysis */
		if (p_ptr->paralyzed)
		{
			(void)set_paralyzed(Ind, p_ptr->paralyzed - 1);
		}

		/* Confusion */
		if (p_ptr->confused)
		{
			(void)set_confused(Ind, p_ptr->confused - 1);
		}

		/* Afraid */
		if (p_ptr->afraid)
		{
			(void)set_afraid(Ind, p_ptr->afraid - 1);
		}

		/* Fast */
		if (p_ptr->fast)
		{
			(void)set_fast(Ind, p_ptr->fast - 1);
		}

		/* Slow */
		if (p_ptr->slow)
		{
			(void)set_slow(Ind, p_ptr->slow - 1);
		}

		/* Protection from evil */
		if (p_ptr->protevil)
		{
			(void)set_protevil(Ind, p_ptr->protevil - 1);
		}

		/* Invulnerability */
		/* Hack -- make -1 permanent invulnerability */
		if (p_ptr->invuln)
		{
			if (p_ptr->invuln > 0)
				(void)set_invuln(Ind, p_ptr->invuln - 1);
		}

		/* Heroism */
		if (p_ptr->hero)
		{
			(void)set_hero(Ind, p_ptr->hero - 1);
		}

		/* Super Heroism */
		if (p_ptr->shero)
		{
			(void)set_shero(Ind, p_ptr->shero - 1);
		}

		/* Blessed */
		if (p_ptr->blessed)
		{
			(void)set_blessed(Ind, p_ptr->blessed - 1);
		}

		/* Shield */
		if (p_ptr->shield)
		{
			(void)set_shield(Ind, p_ptr->shield - 1);
		}

		/* Oppose Acid */
		if (p_ptr->oppose_acid)
		{
			(void)set_oppose_acid(Ind, p_ptr->oppose_acid - 1);
		}

		/* Oppose Lightning */
		if (p_ptr->oppose_elec)
		{
			(void)set_oppose_elec(Ind, p_ptr->oppose_elec - 1);
		}

		/* Oppose Fire */
		if (p_ptr->oppose_fire)
		{
			(void)set_oppose_fire(Ind, p_ptr->oppose_fire - 1);
		}

		/* Oppose Cold */
		if (p_ptr->oppose_cold)
		{
			(void)set_oppose_cold(Ind, p_ptr->oppose_cold - 1);
		}

		/* Oppose Poison */
		if (p_ptr->oppose_pois)
		{
			(void)set_oppose_pois(Ind, p_ptr->oppose_pois - 1);
		}

		/*** Poison and Stun and Cut ***/

		/* Poison */
		if (p_ptr->poisoned)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Apply some healing */
			(void)set_poisoned(Ind, p_ptr->poisoned - adjust);
		}

		/* Stun */
		if (p_ptr->stun)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Apply some healing */
			(void)set_stun(Ind, p_ptr->stun - adjust);
		}

		/* Cut */
		if (p_ptr->cut)
		{
			int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

			/* Hack -- Truly "mortal" wound */
			if (p_ptr->cut > 1000) adjust = 0;

			/* Apply some healing */
			(void)set_cut(Ind, p_ptr->cut - adjust);
		}

		/*** Process Light ***/

		/* Check for light being wielded */
		o_ptr = &p_ptr->inventory[INVEN_LITE];

		/* Burn some fuel in the current lite */
		if (o_ptr->tval == TV_LITE)
		{
			/* Hack -- Use some fuel (sometimes) */
			if (!artifact_p(o_ptr) && !(o_ptr->sval == SV_LITE_DWARVEN)
				&& !(o_ptr->sval == SV_LITE_FEANOR) && (o_ptr->pval > 0))
			{
				/* Decrease life-span */
				o_ptr->pval--;

				/* Hack -- notice interesting fuel steps */
				if ((o_ptr->pval < 100) || (!(o_ptr->pval % 100)))
				{
					/* Window stuff */
					p_ptr->window |= (PW_INVEN | PW_EQUIP);
				}

				/* Hack -- Special treatment when blind */
				if (p_ptr->blind)
				{
					/* Hack -- save some light for later */
					if (o_ptr->pval == 0) o_ptr->pval++;
				}

				/* The light is now out */
				else if (o_ptr->pval == 0)
				{
					disturb(Ind, 0, 0);
					msg_print(Ind, "Your light has gone out!");
				}

				/* The light is getting dim */
				else if ((o_ptr->pval < 100) && (!(o_ptr->pval % 10)))
				{
					if (p_ptr->disturb_minor) disturb(Ind, 0, 0);
					msg_print(Ind, "Your light is growing faint.");
				}
			}
		}

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);

		/*** Process Inventory ***/

		/* Handle experience draining */
		if (p_ptr->exp_drain)
		{
			if ((rand_int(100) < 10) && (p_ptr->exp > 0))
			{
				p_ptr->exp--;
				p_ptr->max_exp--;
				check_experience(Ind);
			}
		}

		/* Note changes */
		j = 0;

		/* Process equipment */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			/* Get the object */
			o_ptr = &p_ptr->inventory[i];

			/* Skip non-objects */
			if (!o_ptr->k_idx) continue;

			/* Recharge activatable objects */
			if (o_ptr->timeout > 0)
			{
				/* Recharge */
				o_ptr->timeout--;

				/* Notice changes */
				if (!(o_ptr->timeout)) j++;
			}
		}

		/* Recharge rods */
		for (i = 0; i < INVEN_PACK; i++)
		{
			o_ptr = &p_ptr->inventory[i];

			/* Examine all charging rods */
			if ((o_ptr->tval == TV_ROD) && (o_ptr->pval))
			{
				/* Charge it */
				o_ptr->pval--;

				/* Notice changes */
				if (!(o_ptr->pval)) j++;
			}
		}

		/* Notice changes */
		if (j)
		{
			/* Combine pack */
			p_ptr->notice |= (PN_COMBINE);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);
		}

		/* Feel the inventory */
		sense_inventory(Ind);

		/*** Involuntary Movement ***/

		/* Delayed Word-of-Recall */
		if (p_ptr->word_recall)
		{
			/* Count down towards recall */
			p_ptr->word_recall--;

		       /* MEGA HACK: no recall if icky, or in a shop */
			if( ! p_ptr->word_recall ) 
			{
				if( character_icky || (p_ptr->store_num > 0)) {
				    p_ptr->word_recall++;
				}
			}

			/* Activate the recall */
			
			if (!p_ptr->word_recall)
			{
				/* Disturbing! */
				disturb(Ind, 0, 0);

				/* Determine the level */
				if (p_ptr->dun_depth > 0)
				{
					/* Messages */
					msg_print(Ind, "You feel yourself yanked upwards!");
					msg_format_near(Ind, "%s is yanked upwards!", p_ptr->name);
					
					/* New location */
					new_depth = 0;
					new_world_x = p_ptr->world_x;
					new_world_y = p_ptr->world_y;
					
					p_ptr->new_level_method = LEVEL_RAND;
				}
				else if ((p_ptr->dun_depth < 0) || (p_ptr->recall_depth < 0))
				{
					/* Messages */
					msg_print(Ind, "You feel yourself yanked sideways!");
					msg_format_near(Ind, "%s is yanked sideways!", p_ptr->name);
					
					/* New location */
					if (p_ptr->dun_depth < 0) 
					{
						new_depth = 0;
						new_world_x = 0;
						new_world_y = 0;										
					}
					else 
					{ 
						new_depth = p_ptr->recall_depth;
						new_world_x = wild_info[new_depth].world_x;
						new_world_y = wild_info[new_depth].world_y;
					}
					p_ptr->new_level_method = LEVEL_OUTSIDE_RAND;												
				}
				else
				{
					/* Messages */
					msg_print(Ind, "You feel yourself yanked downwards!");
					msg_format_near(Ind, "%s is yanked downwards!", p_ptr->name);
					new_depth = p_ptr->recall_depth;
					new_world_x = p_ptr->world_x;
					new_world_y = p_ptr->world_y;
					p_ptr->new_level_method = LEVEL_RAND;
				}
				
				/* One less person here */
				players_on_depth[p_ptr->dun_depth]--;
				
				/* paranoia, required for adding old wilderness saves to new servers */
				if (players_on_depth[p_ptr->dun_depth] < 0) players_on_depth[p_ptr->dun_depth] = 0;

				/* Remove the player */
				cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].m_idx = 0;
					
				/* Show everyone that he's left */
				everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

				/* Forget his lite and view */
				forget_lite(Ind);
				forget_view(Ind);

				p_ptr->dun_depth = new_depth;
				p_ptr->world_x = new_world_x;
				p_ptr->world_y = new_world_y;

				/* One more person here */
				players_on_depth[p_ptr->dun_depth]++;

				p_ptr->new_level_flag = TRUE;
			}
		}
	}


	/* HACK -- redraw stuff a lot, this should reduce perceived latency. */
	/* This might not do anything, I may have been silly when I added this. -APD */
	/* Notice stuff (if needed) */
	if (p_ptr->notice) notice_stuff(Ind);

	/* Update stuff (if needed) */
	if (p_ptr->update) update_stuff(Ind);
	
	/* Redraw stuff (if needed) */
	if (p_ptr->redraw) redraw_stuff(Ind);

	/* Redraw stuff (if needed) */
	if (p_ptr->window) window_stuff(Ind);
}




/*
 * This function handles "global" things such as the stores,
 * day/night in the town, etc.
 */
 
/* Added the japanese unique respawn patch -APD- 
   It appears that each moment of time is equal to 10 minutes?
*/
static void process_various(void)
{
	int i, j, y, x, num_on_depth;
	cave_type *c_ptr;
	player_type *p_ptr;

	char buf[1024];

	/* Save the server state occasionally */
	if (!(turn % (cfg_fps * 60 * SERVER_SAVE)))
	{
		save_server_info();

		/* Save each player */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Save this player */
			save_player(i);
		}
	}

	/* Handle certain things once a minute */
	if (!(turn % (cfg_fps * 60)))
	{
		/* Update the player retirement timers */
		for (i = 1; i <= NumPlayers; i++)
		{
			p_ptr = Players[i];

			// If our retirement timer is set
			if (p_ptr->retire_timer > 0)
			{
				// Decrement our retire timer
				p_ptr->retire_timer--;

				// If the timer runs out, forcibly retire
				// this character.
				if (!p_ptr->retire_timer)
				{
					do_cmd_suicide(i);
				}
			}

		}


		/* Update the unique respawn timers */
		for (i = 1; i < MAX_R_IDX-1; i++)
		{
			monster_race *r_ptr = &r_info[i];
	                                                
			/* Make sure we are looking at a dead unique */
			if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
			if (r_ptr->max_num > 0) continue;

			/* Hack -- Initially set a newly killed uniques respawn timer */
			/* -1 denotes that the respawn timer is unset */
   			if (r_ptr->respawn_timer < 0) 
   			{
				r_ptr->respawn_timer = cfg_unique_respawn_time * (r_ptr->level + 1);
				if (r_ptr->respawn_timer > cfg_unique_max_respawn_time)
  					r_ptr->respawn_timer = cfg_unique_max_respawn_time;
  			}
			// Decrament the counter 
			else r_ptr->respawn_timer--; 
			// Once the timer hits 0, ressurect the unique.
			if (!r_ptr->respawn_timer)
    			{
				/* "Ressurect" the unique */
    				r_ptr->max_num = 1;
				r_ptr->respawn_timer = -1;

    				/* Tell every player */
    				sprintf(buf,"%s rises from the dead!",(r_name + r_ptr->name));
    				msg_broadcast(0,buf); 
    			}	    			
 		}

		// If the level unstaticer is not disabled
		if (cfg_level_unstatic_chance > 0)
		{
			// For each dungeon level
			for (i = 1; i < MAX_DEPTH; i++)
			{
				// If this depth is static
				if (players_on_depth[i])
				{
					num_on_depth = 0;
					// Count the number of players actually in game on this depth
					for (j = 1; j < NumPlayers + 1; j++)
					{
						p_ptr = Players[j];
						if (p_ptr->dun_depth == i) num_on_depth++;
					}
					// If this level is static and no one is actually on it
					if (!num_on_depth)
					{
						// random chance of the level unstaticing
						// the chance is one in (base_chance * depth)/250 feet.
						if (!rand_int(((cfg_level_unstatic_chance * (i+5))/5)-1))
						{
							// unstatic the level
							players_on_depth[i] = 0;
						}
					}
				}
			}
		}
	}


	/* Grow trees very occasionally */
	if (!(turn % (10L * GROW_TREE)))
	{
		/* Find a suitable location */
		for (i = 1; i < 1000; i++)
		{
			cave_type *c_ptr;

			/* Pick a location */
			y = rand_range(1, MAX_HGT - 1);
			x = rand_range(1, MAX_WID - 1);

			/* Acquire pointer */
			c_ptr = &cave[0][y][x];

			/* Only allow "dirt" */
			if (c_ptr->feat != FEAT_DIRT) continue;

			/* Never grow on top of objects or monsters */
			if (c_ptr->m_idx) continue;
			if (c_ptr->o_idx) continue;

			/* Grow a tree here */
			c_ptr->feat = FEAT_TREE;

			/* Show it */
			everyone_lite_spot(0, y, x);

			/* Done */
			break;
		}
	}

	/* Update the stores */
	if (!(turn % (10L * STORE_TURNS)))
	{
		int n;

		/* Maintain each shop (except home and auction house) */
		for (n = 0; n < MAX_STORES - 2; n++)
		{
			/* Maintain */
			store_maint(n);
		}

		/* Sometimes, shuffle the shopkeepers */
		if (rand_int(STORE_SHUFFLE) == 0)
		{
			/* Shuffle a random shop (except home and auction house) */
			store_shuffle(rand_int(MAX_STORES - 2));
		}
	}

	/* Hack -- Daybreak/Nightfall outside the dungeon */
	if (!(turn % ((10L * TOWN_DAWN) / 2)))
	{
		bool dawn;

		/* Check for dawn */
		dawn = (!(turn % (10L * TOWN_DAWN)));
		/* Day breaks */
		if (dawn)
		{
			/* Mega-Hack -- erase all wilderness monsters.
			 * This should prevent wilderness monster "buildup",
			 * massive worm infestations, and uniques getting
			 * lost out there.
			 */
			for (i = 1; i < MAX_WILD; i++) 
				/* if no one is here the monsters 'migrate'.*/
				if (!players_on_depth[-i]) wipe_m_list(-i);
			/* another day, more stuff to kill... */
			for (i = 1; i < MAX_WILD; i++) wild_info[-i].flags &= ~(WILD_F_INHABITED);
		
			/* Hack -- Scan the town */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					 /* Get the cave grid */
					c_ptr = &cave[0][y][x];

					 /* Assume lit */
					c_ptr->info |= CAVE_GLOW;

					 /* Hack -- Notice spot */
					note_spot_depth(0, y, x);
				}
			} 
		}	
		else
		{
			/* Hack -- Scan the town */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					 /* Get the cave grid */
					c_ptr = &cave[0][y][x];

					 /* Darken "boring" features */
					if (c_ptr->feat <= FEAT_INVIS && !(c_ptr->info & CAVE_ROOM))
					{
						 /* Darken the grid */
						c_ptr->info &= ~CAVE_GLOW;
					}
				}
			}
			/* hack -- make fruit bat wear off */
			for (x = 1; x < NumPlayers + 1; x++)
			{
				if (Players[x]->fruit_bat)
				{
					Players[x]->fruit_bat--;
					if (!Players[x]->fruit_bat)
					msg_print(x, "Your form feels much more familliar.");
				}
			}
		}
	}
}
			
		

/*
 * Main loop --KLJ--
 *
 * This is main loop; it is called every 1/FPS seconds.  Usually FPS is about
 * 50, so that a normal unhasted unburdened character gets 1 player turn per
 * second.  Note that we process every player and the monsters, then quit.
 * The "scheduling" code (see sched.c) is the REAL main loop, which handles
 * various inputs and timings.
 */

void dungeon(void)
{
	int i, d;
	byte *w_ptr;
	cave_type *c_ptr;

	/* Return if no one is playing */
	/* if (!NumPlayers) return; */

	/* Check for death.  Go backwards (very important!) */
	for (i = NumPlayers; i > 0; i--)
	{
#if 0
		/* Check connection first */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;
#endif

		/* Check for death */
		if (Players[i]->death)
		{
			/* Kill him */
			player_death(i);
		}
	}

	/* Check player's depth info */
	for (i = 1; i < NumPlayers + 1; i++)
	{
		player_type *p_ptr = Players[i];
		int Depth = p_ptr->dun_depth;
		int j, x, y, startx, starty;

		if (players_on_depth[Depth] == 0) continue;
#if 0
		if (p_ptr->conn == NOT_CONNECTED)
			continue;
#endif

		if (!p_ptr->new_level_flag)
			continue;

		/* Check "maximum depth" to make sure it's still correct */
		if (Depth > p_ptr->max_dlv)
			p_ptr->max_dlv = Depth;

		/* Make sure the server doesn't think the player is in a store */
		p_ptr->store_num = -1;

		/* Check to see which if the level needs generation or destruction */
		/* Note that "town" is excluded */

		for (j = -MAX_WILD+1; j < MAX_DEPTH; j++)
		{
			/* Everybody has left a level that is still generated */
			if (players_on_depth[j] == 0 && cave[j])
			{						
				/* Destroy the level */
				/* Hack -- don't dealloc the town */
				if (j)
					dealloc_dungeon_level(j);
			}

		}


		/* Somebody has entered an ungenerated level */
		if (players_on_depth[Depth] && !cave[Depth])
		{
			/* Allocate space for it */
			alloc_dungeon_level(Depth);

			/* Generate a dungeon level there */
			/* option 29 is auto_scum */
			generate_cave(Depth,p_ptr->options[29]);
		}

		/* Clear the "marked" and "lit" flags for each cave grid */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				w_ptr = &p_ptr->cave_flag[y][x];

				*w_ptr = 0;
			}
		}

		/* hack -- update night/day in wilderness levels */
		if ((Depth < 0) && (IS_DAY)) wild_apply_day(Depth); 
		if ((Depth < 0) && (IS_NIGHT)) wild_apply_night(Depth);

		/* Memorize the town and all wilderness levels close to town */
		if (Depth <= 0 ? (wild_info[Depth].radius <= 2) : 0)
		{
			bool dawn = ((turn % (10L * TOWN_DAWN)) < (10L * TOWN_DAWN / 2)); 

			p_ptr->max_panel_rows = (MAX_HGT / SCREEN_HGT) * 2 - 2;
			p_ptr->max_panel_cols = (MAX_WID / SCREEN_WID) * 2 - 2;

			p_ptr->cur_hgt = MAX_HGT;
			p_ptr->cur_wid = MAX_WID;

			/* Memorize the town for this player (if daytime) */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					w_ptr = &p_ptr->cave_flag[y][x];
					c_ptr = &cave[Depth][y][x];

					/* Memorize if daytime or "interesting" */
					if (dawn || c_ptr->feat > FEAT_INVIS || c_ptr->info & CAVE_ROOM)
						*w_ptr |= CAVE_MARK;
				}
			}
		}
		else
		{
			p_ptr->max_panel_rows = (MAX_HGT / SCREEN_HGT) * 2 - 2;
			p_ptr->max_panel_cols = (MAX_WID / SCREEN_WID) * 2 - 2;

			p_ptr->cur_hgt = MAX_HGT;
			p_ptr->cur_wid = MAX_WID;
		}

		/* Determine starting location */
		switch (p_ptr->new_level_method)
		{
			/* Climbed down */
			case LEVEL_DOWN:  starty = level_down_y[Depth];
					  startx = level_down_x[Depth];
					  break;

			/* Climbed up */
			case LEVEL_UP:    starty = level_up_y[Depth];
					  startx = level_up_x[Depth];
					  break;
			
			/* Teleported level */
			case LEVEL_RAND:  starty = level_rand_y[Depth];
					  startx = level_rand_x[Depth];
					  break;
			
			/* Used ghostly travel */
			case LEVEL_GHOST: starty = p_ptr->py;
					  startx = p_ptr->px;
					  break;
					  
			/* Over the river and through the woods */			  
			case LEVEL_OUTSIDE: starty = p_ptr->py;
				            startx = p_ptr->px;
				            break;
			/* this is used instead of extending the level_rand_y/x
			   into the negative direction to prevent us from
			   alocing so many starting locations.  Although this does
			   not make players teleport to simmilar locations, this
			   could be achieved by seeding the RNG with the depth.
			*/
			case LEVEL_OUTSIDE_RAND: 
			
				/* make sure we aren't in an "icky" location */
				do
				{
					starty = rand_int(MAX_HGT-3)+1;
					startx = rand_int(MAX_WID-3)+1;
				}
				while (  (cave[Depth][starty][startx].info & CAVE_ICKY)
				      || (!cave_floor_bold(Depth, starty, startx)) );
				break;
		}

		/* Place the player in an empty space */
		for (j = 0; j < 1500; ++j)
		{
			/* Increasing distance */
			d = (j + 149) / 150;

			/* Pick a location */
			scatter(Depth, &y, &x, starty, startx, d, 1);

			/* Must have an "empty" grid */
			if(cave[Depth]) if (!cave_empty_bold(Depth, y, x)) continue;

			/* Not allowed to go onto a icky location (house) if Depth <= 0 */
			if ((Depth <= 0) && (cave[Depth][y][x].info & CAVE_ICKY))
				continue;

			break;
		}

#if 0
		while (TRUE)
		{
			y = rand_range(1, ((Depth) ? (MAX_HGT - 2) : (SCREEN_HGT - 2)));
			x = rand_range(1, ((Depth) ? (MAX_WID - 2) : (SCREEN_WID - 2)));

			/* Must be a "naked" floor grid */
			if (!cave_naked_bold(Depth, y, x)) continue;

			/* Refuse to start on anti-teleport grids */
			if (cave[Depth][y][x].info & CAVE_ICKY) continue;

			break;
		}
#endif

		p_ptr->py = y;
		p_ptr->px = x;

		/* Update the player location */
		cave[Depth][y][x].m_idx = 0 - i;

		/* Recalculate panel */
		p_ptr->panel_row = ((p_ptr->py - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
		if (p_ptr->panel_row > p_ptr->max_panel_rows) p_ptr->panel_row = p_ptr->max_panel_rows;
		else if (p_ptr->panel_row < 0) p_ptr->panel_row = 0;

		p_ptr->panel_col = ((p_ptr->px - SCREEN_WID / 4) / (SCREEN_WID / 2));
		if (p_ptr->panel_col > p_ptr->max_panel_cols) p_ptr->panel_col = p_ptr->max_panel_cols;
		else if (p_ptr->panel_col < 0) p_ptr->panel_col = 0;
	
		p_ptr->redraw |= (PR_MAP);
		p_ptr->redraw |= (PR_DEPTH);

		panel_bounds(i);
		forget_view(i);
		forget_lite(i);
		update_view(i);
		update_lite(i);
		update_monsters(TRUE);

		/* Clear the flag */
		p_ptr->new_level_flag = FALSE;
	}

	/* Handle any network stuff */
	Net_input();

	/* Hack -- Compact the object list occasionally */
	if (o_top + 16 > MAX_O_IDX) compact_objects(32);

	/* Hack -- Compact the monster list occasionally */
	if (m_top + 32 > MAX_M_IDX) compact_monsters(64);


	// Note -- this is the END of the last turn

	/* Do final end of turn processing for each player */
	for (i = 1; i < NumPlayers + 1; i++)
	{
#if 0
		if (Players[i]->conn == NOT_CONNECTED)
			continue;
#endif

		/* Actually process that player */
		process_player_end(i);
	}

	/* Check for death.  Go backwards (very important!) */
	for (i = NumPlayers; i > 0; i--)
	{
#if 0
		/* Check connection first */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;
#endif

		/* Check for death */
		if (Players[i]->death)
		{
			/* Kill him */
			player_death(i);
		}
	}



	///*** BEGIN NEW TURN ***///
	turn++;

	/* Do some beginning of turn processing for each player */
	for (i = 1; i < NumPlayers + 1; i++)
	{
#if 0
		if (Players[i]->conn == NOT_CONNECTED)
			continue;
#endif

		/* Actually process that player */
		process_player_begin(i);
	}

	/* Process all of the monsters */
	process_monsters();

	/* Process all of the objects */
	process_objects();

	/* Probess the world */
	for (i = 1; i < NumPlayers + 1; i++)
	{
#if 0
		if (Players[i]->conn == NOT_CONNECTED)
			continue;
#endif

		/* Process the world of that player */
		process_world(i);
	}

	/* Process everything else */
	process_various();

	/* Hack -- Regenerate the monsters every hundred game turns */
	if (!(turn % 100)) regen_monsters();

	/* Refresh everybody's displays */
	for (i = 1; i < NumPlayers + 1; i++)
	{
		player_type *p_ptr = Players[i];
#if 0
		if (p_ptr->conn == NOT_CONNECTED)
			continue;
#endif

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff(i);

		/* Update stuff */
		if (p_ptr->update) update_stuff(i);

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff(i);

		/* Window stuff */
		if (p_ptr->window) window_stuff(i);
	}

	/* Send any information over the network */
	Net_output();
}

		
/*
 * Load the various "user pref files"
 */
static void load_all_pref_files(void)
{
	char buf[1024];


	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "user.prf");

	/* Process that file */
	process_pref_file(buf);



}


/*
 * Actually play a game
 *
 * If the "new_game" parameter is true, then, after loading the
 * server-specific savefiles, we will start anew.
 */
void play_game(bool new_game)
{
	int i, n;


	/* Hack -- Character is "icky" */
	/*character_icky = TRUE;*/


	/* Hack -- turn off the cursor */
	/*(void)Term_set_cursor(0);*/


	/* Attempt to load the server state information */
	if (!load_server_info())
	{
		/* Oops */
		quit("broken server savefile(s)");
	}

	/* UltraHack -- clear each wilderness levels inhabited flag, so
	   monsters will respawn.
	   hack -- clear the wild_f_in_memory flag, so house objects are added
	   once and only once.
	
	   I believe this is no longer neccecary.
	for (i = 1; i < MAX_WILD; i++) wild_info[-i].flags &= ~(WILD_F_IN_MEMORY);
	*/

	/* Nothing loaded */
	if (!server_state_loaded)
	{
		/* Make server state info */
		new_game = TRUE;

		/* Create a new dungeon */
		server_dungeon = FALSE;
	}

	/* Process old character */
	if (!new_game)
	{
		/* Process the player name */
		/*process_player_name(FALSE);*/
	}

	/* Init the RNG */
	if (Rand_quick)
	{
		u32b seed;

		/* Basic seed */
		seed = (time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() << 1));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Extract the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_set;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (option_info[i].o_var)
		{
			/* Set */
			if (option_flag[os] & (1L << ob))
			{
				/* Set */
				(*option_info[i].o_var) = TRUE;
			}
			
			/* Clear */
			else
			{
				/* Clear */
				(*option_info[i].o_var) = FALSE;
			}
		}
	}

	/* Roll new town */
	if (new_game)
	{
		/* Ignore the dungeon */
		server_dungeon = FALSE;

		/* Start in town */
		/*dun_level = 0;*/

		/* Hack -- seed for flavors */
		seed_flavor = rand_int(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = rand_int(0x10000000);

		/* Initialize server state information */
		/*player_birth();*/
		server_birth();

		/* Hack -- enter the world */
		turn = 1;

		/* Initialize the stores */
		for (n = 0; n < MAX_STORES; n++)
		{
			/* Initialize */
			store_init(n);
	
			/* Ignore home and auction house */
			if ((n == MAX_STORES - 2) || (n == MAX_STORES - 1)) continue;
	
			/* Maintain the shop */
			for (i = 0; i < 10; i++) store_maint(n);
		}
	}


	/* Flash a message */
	s_printf("Please wait...\n");

	/* Flush the message */
	/*Term_fresh();*/


	/* Hack -- Enter wizard mode */
	/*if (arg_wizard && enter_wizard_mode()) wizard = TRUE;*/


	/* Flavor the objects */
	flavor_init();

	s_printf("Object flavors initialized...\n");

	/* Reset the visual mappings */
	reset_visuals();

	/* Window stuff */
	/*p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER);*/

	/* Window stuff */
	/*p_ptr->window |= (PW_MONSTER);*/

	/* Window stuff */
	/*window_stuff();*/


	/* Load the "pref" files */
	load_all_pref_files();

	/* Set or clear "rogue_like_commands" if requested */
	/*if (arg_force_original) rogue_like_commands = FALSE;
	if (arg_force_roguelike) rogue_like_commands = TRUE;*/

	/* Verify the keymap */
	/*keymap_init();*/

	/* React to changes */
	/*Term_xtra(TERM_XTRA_REACT, 0);*/


	/* Make a town if necessary */
	if (!server_dungeon)
	{
		/* Allocate space for it */
		alloc_dungeon_level(0);

		/* Actually generate the town */
		generate_cave(0,0);
	}

	/* Finish initializing dungeon monsters */
	setup_monsters();

	/* Finish initializing dungeon objects */
	setup_objects();

	/* Server initialization is now "complete" */
	server_generated = TRUE;


	/* Hack -- Character is no longer "icky" */
	/*character_icky = FALSE;*/


	/* Start game */
	/*alive = TRUE;*/

	/* Hack -- Enforce "delayed death" */
	/*if (p_ptr->chp < 0) death = TRUE;*/

	/* Set up the contact socket, so we can allow players to connect */
	setup_contact_socket();

	/* Set up the network server */
	if (Setup_net_server() == -1)
		quit("Couldn't set up net server");

	/* Set up the main loop */
	install_timer_tick(dungeon, cfg_fps);

	/* Loop forever */
	sched();

	/* This should never, ever happen */
	s_printf("sched returned!!!\n");

	/* Close stuff */
	close_game();

	/* Quit */
	quit(NULL);
}


void shutdown_server(void)
{
	int i;

	s_printf("Shutting down.\n");

	/* Kick every player out and save his game */
	while(NumPlayers > 0)
	{
		/* Note the we always save the first player */
		player_type *p_ptr = Players[1];

		/* Indicate cause */
		strcpy(p_ptr->died_from, "server shutdown");

		/* Try to save */
		if (!save_player(1)) Destroy_connection(p_ptr->conn, "Server shutdown (save failed)");

		/* Successful save */
		Destroy_connection(p_ptr->conn, "Server shutdown (save succeeded)");
	}

	/* Now wipe every object, to preserve artifacts on the ground */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Wipe this depth */
		wipe_o_list(i);
	}

	/* Save the server state */
	if (!save_server_info()) quit("Server state save failed!");

	/* Tell the metaserver that we're gone */
	Report_to_meta(META_DIE);

	quit("Server state saved");
}