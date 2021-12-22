/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *          
 *
 *****************************************************************************/
#ifndef _AODV_NEIGHBOR_H
#define _AODV_NEIGHBOR_H

#ifndef NS_NO_GLOBALS
#include "defs.h"
#include "routing_table.h"
#endif				/* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

void neighbor_add(AODV_msg * aodv_msg, struct in_addr source,
		  unsigned int ifindex);
void neighbor_link_break(rt_table_t * rt);

// fxj
#ifdef USE_FXJ
int nb_table_find(in_addr nb_addr); //  return index in nb_tbl, -1 when not nb or all channels down
int nb_best_channel(in_addr nb_addr);  //  return best channel to bnr, -1 when link break
int nb_table_find(int nb_addr); //  return index in nb_tbl, -1 when not nb or all channels down
int nb_best_channel(int nb_addr);  //  return best channel to bnr, -1 when link break
#endif
// fxj_end

#endif				/* NS_NO_DECLARATIONS */

#endif				/* AODV_NEIGHBOR_H */
