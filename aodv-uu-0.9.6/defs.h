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
#define USE_OPTIMIZATION true

#ifdef NS_PORT
/* NS_PORT: Log filename split into prefix and suffix. */
#define AODV_LOG_PATH_PREFIX "aodv-uu-"
#define AODV_RT_LOG_PATH_SUFFIX ".rtlog"
#define AODV_LOG_PATH_SUFFIX ".log"
#else
#define AODV_LOG_PATH "/var/log/aodvd.log"
#define AODV_RT_LOG_PATH "/var/log/aodvd.rtlog"
#endif                /* NS_PORT */

//#undef Max(A,B) ( (A) > (B) ? (A):(B))
#ifndef Max
#define Max(A, B) ( (A) > (B) ? (A):(B))
#endif

#define MINTTL 1        /* min TTL in the packets sent locally */

#define MAX_NR_INTERFACES 10
#define MAX_IFINDEX (MAX_NR_INTERFACES - 1)

#if !defined(IFNAMSIZ)
#define IFNAMSIZ 16
#endif
//by cyo
#define A1 0
#define B1 0
#define C1 0
#define K11 1.0//稳定时一步转移概率
#define K01 0.5//不稳定(0,1)时一步转移概率
#define K00 0.0//不稳定(0,0)时一步转移概率
#define INIT_E 0.5
#define INIT_G 0.5
#define INIT_F 0.5
#define INIT_COST 0.8
#if !defined(NUM_STATES)
#define NUM_STATES 5
#endif
// cyo_end
/* Data for a network device */
struct dev_info {
    int enabled;        /* 1 if struct is used, else 0 */
    int sock;            /* AODV socket associated with this device */
#ifdef CONFIG_GATEWAY
    int psock;			/* Socket to send buffered data packets. */
#endif
    unsigned int ifindex;
    char ifname[IFNAMSIZ];
    struct in_addr ipaddr;    /* The local IP address */
    struct in_addr netmask;    /* The netmask we use */
    struct in_addr broadcast;
};
//by cyo
struct nb_info {
    float cost;
    int isValid;
    struct in_addr ipAddr;
};
struct node_info {
    int node_sta;
    int isValid;
};
struct hello_info {
    in_addr ipAddr;
    int hello_send;
    int hello_received;
    int isValid;
};
struct f_value {
    in_addr ipAddr;
    int count;
    float f_history_value[20];
};
//cyo_end
struct host_info {
    u_int32_t seqno;        /* Sequence number */
    struct timeval bcast_time;    /* The time of the last broadcast msg sent */
    struct timeval fwd_time;    /* The time a data packet was last forwarded */
    u_int32_t rreq_id;        /* RREQ id */
    int nif;            /* Number of interfaces to broadcast on */
    struct dev_info devs[MAX_NR_INTERFACES + 1]; /* Add +1 for returning as "error" in ifindex2devindex. */
    //by cyo
    struct node_info sta_tbl[20][NUM_STATES];   //所有邻居结点的历史稳定性值
    struct node_info sta_self[NUM_STATES];//自身的稳定性
    int neighbor_sum;
    int neighbor_change;
    struct hello_info hello_infos[20][3];
    struct nb_info nb_tbl_2[20][3];//邻居表
    struct f_value f_tbl[20][3];
    //cyo_end
};

//by cyo
void nb_add(in_addr ip_temp) {
    for (int i = 0; i < 20; i++) {
        if (nb_tbl_2[i][0].isValid == ip_temp) {
            return;
        }
    }
    for (int i = 0; i < 3; i++) {
        nb_tbl_2[neighbor_sum][0].ipAddr = ip_temp;
        nb_tbl_2[neighbor_sum][1].ipAddr = ip_temp;
        nb_tbl_2[neighbor_sum][2].ipAddr = ip_temp;
        nb_tbl_2[neighbor_sum][0].cost = INIT_COST;
        nb_tbl_2[neighbor_sum][1].cost = INIT_COST;
        nb_tbl_2[neighbor_sum][2].cost = INIT_COST;
        nb_tbl_2[neighbor_sum][0].isValid = 1;
        nb_tbl_2[neighbor_sum][1].isValid = 1;
        nb_tbl_2[neighbor_sum][2].isValid = 1;
    }
    this_host.neighbor_sum++;
}
void nb_update(in_addr ip_temp,int channel,float cost_value){
    for(int i = 0;i<neighbor_sum;i++){
        if(nb_tbl_2[i][channel].ipAddr == ip_temp){
            nb_tbl_2[i][channel].cost = cost_value;
            break;
        }
    }
}
void nb_setIsValid(in_addr ip_temp,int channel,int isValid){
    for(int i = 0;i<neighbor_sum;i++){
        if(nb_tbl_2[i][channel].ipAddr == ip_temp){
            nb_tbl_2[i][channel].isValid = isValid;
            break;
        }
    }
}
void hello_infos_clear() {
    for (int i = 0; i < 20; i++) {
        this_host.hello_infos[i][0].hello_received = 0;
        this_host.hello_infos[i][1].hello_received = 0;
        this_host.hello_infos[i][2].hello_received = 0;
        this_host.hello_infos[i][0].hello_send = 0;
        this_host.hello_infos[i][1].hello_send = 0;
        this_host.hello_infos[i][2].hello_send = 0;
    }
}

void hello_received_add(in_addr ip_temp, int channel, int num) {
    int i;
    for (i = 0; i < 20; i++) {
        if (this_host.hello_infos[i][channel].ipAddr == ip_temp) {
            this_host.hello_infos[i][channel].hello_received += num;
            break;
        }
    }
    if (i == 20) {
        for (int j = 0; j < 20; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipAddr = ip_temp;
                this_host.hello_infos[j][channel].hello_received += num;
                break;
            }
        }
    }
}

void hello_send_add(in_addr ip_temp, int channel, int num) {
    int i;
    for (i = 0; i < 20; i++) {
        if (this_host.hello_infos[i][channel].ipAddr == ip_temp) {
            this_host.hello_infos[i][channel].hello_send += num;
            break;
        }
    }
    if (i == 20) {
        for (int j = 0; j < 20; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipAddr = ip_temp;
                this_host.hello_infos[j][channel].hello_send += num;
                break;
            }
        }
    }
}

void add_f_value(float f, in_addr ip_temp, int channel) {
    for (int i = 0; i < 20; i++) {
        if (this_host.f_tbl[i][channel].ipAddr == ip_temp) {
            if (this_host.f_tbl[i][channel].count == 20) {
                for (int j = 18; j >= 0; j--) {
                    this_host.f_tbl[i][channel].f_history_value[j + 1] = this_host.f_tbl[i][channel].history_value[j];
                }
                this_host.f_tbl[i][channel].f_history_value[0] = f;
            } else {
                this_host.f_tbl[i][channel].f_history_value[this_host.f_tbl[i][channel].count] = f;
                this_host.f_tbl[i][channel].count++;
            }
        }
    }
}

float getE(int A_send, int B_send, int A_recieved, int B_recieved) {
    if (A_send == 0 or B_send == 0) {
        return INIT_E;
    } else if (A_recieved == 0 or B_recieved == 0) {
        return 0;
    }
    return (float) (A_send * B_send) / (float) (A_recieved * B_recieved);
}

float getF(in_addr ip_temp, int channel) {
    int node_id;
    for (int i = 0; i < 20; i++) {
        if (this_host.f_tbl[i][channel].ipAddr == ip_temp) {
            node_id = i;
        }
    }
    if (node_id == 20) {
        return 0;
    }
    if (this_host.f_tbl[node_id][channel].count == 0) {
        return INIT_F;
    } else {
        float result;
        for (int i = 0; i < this_host.f_tbl[node_id][channel].count; i++) {
            result += this_host.f_tbl[node_id][channel].f_history_value[i];
        }
        result /= this_host.f_tbl[node_id][channel].count;
        return result;
    }
}

float getG(const struct node_info historyStab[], int neighbor_sum, int neighbor_change) {
    float result = 0;
    neighbor_change1 = (neighbor_change > 0) ? neighbor_change : -neighbor_change;
    for (int i = 0; i < NUM_STATES; i++) {
        if (this_host.sta_self[i].isValid == 0 || historyStab[i].isValid == 0) {
            result += INIT_G;
            continue;
        }
        if (this_host.sta_self[i].node_sta == 0 && historyStab[i].node_sta == 0) {
            if (neighbor_sum == 0) {
                result += K00;
            } else {
                result += K00 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
            }
        } else if (this_host.sta_self[i].node_sta == 1 && historyStab[i].node_sta == 1) {
            if (neighbor_sum == 0) {
                result += K11;
            } else {
                result += K11 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
            }
        } else {
            if (neighbor_sum == 0) {
                result += K01;
            } else {
                result += K01 * (float) (neighbor_sum - neighbor_change) / (float) neighbor_sum;
            }
        }
    }
    return (result > 0) ? result / (float) NUM_STATES : 0;
}

float updateCost(int E, int F, int G) {
    float result = A1 * E + B1 * F + C1 * G - (A1 + B1) * E * F - (A1 + C1) * E * G - (B1 + C1) * F * G +
                   (A1 + B1 + C1) * E * F * G;
    hello_send_clear();
    hello_received_clear();
    return result;
}
//cyo_end
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
static inline unsigned int ifindex2devindex(unsigned int ifindex) {
    int i;

    for (i = 0; i < this_host.nif; i++)
        if (dev_indices[i] == ifindex)
            return i;

    return MAX_NR_INTERFACES;
}

static inline struct dev_info *devfromsock(int sock) {
    int i;

    for (i = 0; i < this_host.nif; i++) {
        if (this_host.devs[i].sock == sock)
            return &this_host.devs[i];
    }
    return NULL;
}

static inline int name2index(char *name) {
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

/* Broadcast address according to draft (255.255.255.255) */
#define AODV_BROADCAST ((in_addr_t) 0xFFFFFFFF)

#define AODV_PORT 654

/* AODV Message types */
#define AODV_HELLO    0        /* Really never used as a separate type... */
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
typedef void (*callback_func_t)(int);

extern int attach_callback_func(int fd, callback_func_t func);

#endif

#endif                /* DEFS_H */
