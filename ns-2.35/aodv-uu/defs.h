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
 *****************************************************************************/
#ifndef _DEFS_H
#define _DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef NS_PORT
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#endif

#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>


#ifndef NS_PORT
#include "timer_queue.h"
#endif

#ifdef NS_PORT
#define NS_CLASS AODVUU::
#define NS_OUTSIDE_CLASS ::
#define NS_STATIC
#define NS_INLINE
/* NS_PORT: Using network device 0, with interface index 0. */
#define NS_DEV_NR 0
#define NS_IFINDEX NS_DEV_NR
#else
#define NS_CLASS
#define NS_OUTSIDE_CLASS
#define NS_STATIC static
#define NS_INLINE inline
#endif

#define AODV_UU_VERSION "0.9.6"
#define DRAFT_VERSION "rfc3561"

#ifdef NS_PORT
/* NS_PORT: Log filename split into prefix and suffix. */
#define AODV_LOG_PATH_PREFIX "aodv-uu-"
#define AODV_RT_LOG_PATH_SUFFIX ".rtlog"
#define AODV_LOG_PATH_SUFFIX ".log"
#else
#define AODV_LOG_PATH "/var/log/aodvd.log"
#define AODV_RT_LOG_PATH "/var/log/aodvd.rtlog"
#endif				/* NS_PORT */

#ifndef Max
#define Max(A,B) ( (A) > (B) ? (A):(B))
#endif

#define MINTTL 1		/* min TTL in the packets sent locally */

#define MAX_NR_INTERFACES 10
#define MAX_IFINDEX (MAX_NR_INTERFACES - 1)
#define CHANNEL_NUM 3 

#if !defined(IFNAMSIZ)
#define IFNAMSIZ 16
#endif
//by cyo
/*#define A1 0.3
#define B1 0.3
#define C1 0.3
#define K11 1.0//稳定时一步转移概率
#define K01 0.5//不稳定(0,1)时一步转移概率
#define K00 0.0//不稳定(0,0)时一步转移概率*/
#define A1 1
#define B1 1
#define C1 1
#define MK 1
#define INIT_E 0.5
#define INIT_G 0.5
#define INIT_F 0.5
#define INIT_COST 0.8
#define TIMER_F 2
#define TIMER_COST 4
#define TIMER_STA 2
//#define HELLO_INFOS_TIMER 10
#if !defined(NUM_STATES)
#define NUM_STATES 8
#define NUM_NODE 20
#define NUM_HISTORY_F 20
#endif
// cyo_end

/* added by cyo & gcy */
/*是否打印printf内容 */
#define GCY_OUT 1
#define CYO_OUT 1
#define YRB_OUT 1
#define FXJ_OUT 0
#define GLO_OUT 1
/* end */


/* added by yrb */

/* 稳定概率阈值 */
#define COST_MIN 0.8

/* cost寻路和快速修复的开关 */
/* 用来跑tcl验证时区分原AODV协议和我们的策略 */
#define USE_YRB 1
#define USE_FXJ 1
/* end yrb */

/* Data for a network device */
struct dev_info {
    int enabled;		/* 1 if struct is used, else 0 */
    int sock;			/* AODV socket associated with this device */
#ifdef CONFIG_GATEWAY
    int psock;			/* Socket to send buffered data packets. */
#endif
    unsigned int ifindex;
    char ifname[IFNAMSIZ];
    struct in_addr ipaddr;	/* The local IP address */
    struct in_addr netmask;	/* The netmask we use */
    struct in_addr broadcast;
};

/* by gcy */
struct stability_info {
    int neighbor_sum;               // para A
    int neighbor_change;            // para B
    int available_channel_num;      // para C
    double best_info_noise_ratio;   // para D
    int stability;                  // result
};
/* end */
//by cyo
struct nb_info {
    float cost;
    int isValid;
    int isVisited;    
    struct in_addr ipaddr;
};
struct node_info {
    struct in_addr ipaddr;
    int node_sta[NUM_STATES];
    int isValid;
    int count;
};
struct hello_info {
    struct in_addr ipaddr;
    u_int8_t hello_send_nb;//邻居结点发送给该节点的消息数量
    u_int8_t hello_received_nb;//邻居节点接受到的从该节点发送的消息数量
    u_int8_t hello_send;//该节点发送给邻居节点的消息的数量
    u_int8_t hello_received;//该节点收到的从邻居结点发送来的消息数量
    u_int8_t isValid;
};
struct f_value {
    struct in_addr ipaddr;
    int count;
    float f_history_value[NUM_HISTORY_F];
};
//cyo_end
struct host_info {
    u_int32_t seqno;		/* Sequence number */
    struct timeval bcast_time;	/* The time of the last broadcast msg sent */
    struct timeval fwd_time;	/* The time a data packet was last forwarded */
    u_int32_t rreq_id;		/* RREQ id */
    int nif;			/* Number of interfaces to broadcast on */
    struct dev_info devs[MAX_NR_INTERFACES+1]; /* Add +1 for returning as "error" in ifindex2devindex. */
    /* by gcy & cyo */
    struct stability_info stability;
    struct node_info sta_tbl[NUM_NODE];//所有邻居结点的历史稳定性值
    struct node_info sta_self;             //自身的稳定性
    int neighbor_sum_init;                              //仅用于nb_tbl初始加入ip地址时计数，防止neighbor_sum减小时发生错误
    struct hello_info hello_infos[NUM_NODE][CHANNEL_NUM];
    u_int8_t hello_received_history[NUM_NODE][CHANNEL_NUM][11];//每个邻居每个信道十个值,[i][j][10]存入当前值
    u_int8_t hello_received_queue_top[NUM_NODE][3];
    u_int8_t hello_send;
    u_int32_t self_addr;    
    struct nb_info nb_tbl[NUM_NODE][3];                       //邻居表
    struct f_value f_tbl[NUM_NODE][3];
    int hello_infos_timer;
    int nb_optimized;                                         //optimized nb
    /* end */
};

/*
  NS_PORT: TEMPORARY SOLUTION: Moved the two variables into the AODVUU class,
  and placed the function definition after the AODVUU class definition.

  (This is to avoid running several passes through defs.h during the source
  code extraction performed by the AODVUU class.)

  TODO: Find some smarter way to accomplish this.
*/
#ifndef NS_PORT
/* This will point to a struct containing information about the host */
struct host_info this_host;

/* Array of interface indexes */
unsigned int dev_indices[MAX_NR_INTERFACES];

/* Given a network interface index, return the index into the
   devs array, Necessary because ifindex is not always 0, 1,
   2... */
static inline unsigned int ifindex2devindex(unsigned int ifindex)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
	if (dev_indices[i] == ifindex)
	    return i;

    return MAX_NR_INTERFACES;
}

static inline struct dev_info *devfromsock(int sock)
{
    int i;

    for (i = 0; i < this_host.nif; i++) {
	if (this_host.devs[i].sock == sock)
	    return &this_host.devs[i];
    }
    return NULL;
}

static inline int name2index(char *name)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
	if (strcmp(name, this_host.devs[i].ifname) == 0)
	    return this_host.devs[i].ifindex;

    return -1;
}
#endif


/* Two macros to simplify retriving of a dev_info struct. Either using
   an ifindex or a device number (index into devs array). */
#define DEV_IFINDEX(ifindex) (this_host.devs[ifindex2devindex(ifindex)])
#define DEV_NR(n) (this_host.devs[n])
//by cyo
#define DEBUG_IP(i) (this_host.self_addr == i)
// cyo_end
 /* Broadcast address according to draft (255.255.255.255) */
#define AODV_BROADCAST ((in_addr_t) 0xFFFFFFFF)

#define AODV_PORT 654

/* AODV Message types */
#define AODV_HELLO    0		/* Really never used as a separate type... */
#define AODV_RREQ     1
#define AODV_RREP     2
#define AODV_RERR     3
#define AODV_RREP_ACK 4

/* A generic AODV packet header struct... */
#ifdef NS_PORT
struct AODV_msg {
#else
typedef struct {
#endif
    u_int8_t type;

/* NS_PORT: Additions for the AODVUU packet type in ns-2 */
#ifdef NS_PORT
    static int offset_;		// Required by PacketHeaderManager

    inline static int &offset() {
	return offset_;
    }
    inline static AODV_msg *access(const Packet * p) {
	return (AODV_msg *) p->access(offset_);
    }

    int size();
};

typedef AODV_msg hdr_aodvuu;	// Name convention for headers
#define HDR_AODVUU(p) ((hdr_aodvuu *) hdr_aodvuu::access(p))
#else
} AODV_msg;
#endif

/* AODV Extension types */
#define RREQ_EXT 1
#define RREP_EXT 1
#define RREP_HELLO_INTERVAL_EXT 2
#define RREP_HELLO_NEIGHBOR_SET_EXT 3
#define RREP_INET_DEST_EXT 4

/* An generic AODV extensions header */
typedef struct {
    u_int8_t type;
    u_int8_t length;
    /* Type specific data follows here */
} AODV_ext;

/* MACROS to access AODV extensions... */
#define AODV_EXT_HDR_SIZE sizeof(AODV_ext)
#define AODV_EXT_DATA(ext) ((char *)((char *)ext + AODV_EXT_HDR_SIZE))
#define AODV_EXT_NEXT(ext) ((AODV_ext *)((char *)ext + AODV_EXT_HDR_SIZE + ext->length))
#define AODV_EXT_SIZE(ext) (AODV_EXT_HDR_SIZE + ext->length)

#ifndef NS_PORT
/* The callback function */
typedef void (*callback_func_t) (int);
extern int attach_callback_func(int fd, callback_func_t func);
#endif

#endif				/* DEFS_H */
