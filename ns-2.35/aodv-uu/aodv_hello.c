/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
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

#ifdef NS_PORT
#include "ns-2/aodv-uu.h"
#else

#include <netinet/in.h>
#include "aodv_hello.h"
#include "aodv_timeout.h"
#include "aodv_rrep.h"
#include "aodv_rreq.h"
#include "routing_table.h"
#include "timer_queue.h"
#include "params.h"
#include "aodv_socket.h"
#include "defs.h"
#include "debug.h"

/* by gcy */
#include "../mac/mac-802_11.h"

/* end */

extern int unidir_hack, receive_n_hellos, hello_jittering, optimized_hellos;
static struct timer hello_timer;

#endif

/* #define DEBUG_HELLO */


long NS_CLASS hello_jitter() {
    if (hello_jittering) {
#ifdef NS_PORT
        return (long) (((float) Random::integer(RAND_MAX + 1) / RAND_MAX - 0.5)
                   * JITTER_INTERVAL);
#else
        return (long) (((float) random() / RAND_MAX - 0.5) * JITTER_INTERVAL);
#endif
    } else
        return 0;
}

void NS_CLASS hello_start() {
    if (GLO_OUT) printf("start hello\n");

    if (hello_timer.used)
        return;

    /* cyo & gcy */
    if (this_host.nb_optimized == 0) {
        this_host.hello_infos_timer = 0;
        this_host.neighbor_sum_init = 0;
        int i, j;
        for (i = 0; i < NUM_NODE; i++) {
            for (j = 0; j < 3; j++) {
                this_host.hello_infos[i][j].hello_send = 0;
                this_host.hello_infos[i][j].hello_received = 0;
                this_host.hello_infos[i][j].hello_send_nb = 0;
                this_host.hello_infos[i][j].hello_received_nb = 0;
                this_host.hello_infos[i][j].isValid = 0;
                //todo 未初始化ip
                //this_host.nb_tbl[i][j].cost = 0;
                //this_host.nb_tbl[i][j].isValid = 0;
                //todo 未初始化ip
                this_host.f_tbl[i][j].count = 0;
                for (int k = 0; k < NUM_NODE; k++) {
                    this_host.f_tbl[i][j].f_history_value[k] = 0;
                }
                //todo 未初始化ip
            }
        }
        for (i = 0; i < NUM_NODE; i++) {
            for (j = 0; j < NUM_STATES; j++) {
                this_host.sta_tbl[i][j].node_sta = 0;
                this_host.sta_tbl[i][j].isValid = 0;
                //todo 未初始化ip
            }
        }
        for (i = 0; i < NUM_STATES; i++) {
            this_host.sta_self[i].node_sta = 0;
            this_host.sta_self[i].isValid = 0;
            //todo 未初始化ip
        }
        this_host.nb_optimized = 1;
    }
    /* end */


    gettimeofday(&this_host.fwd_time, NULL);

    DEBUG(LOG_DEBUG, 0, "Starting to send HELLOs!");
    timer_init(&hello_timer, &NS_CLASS hello_send, NULL);

    hello_send(NULL);
}

void NS_CLASS hello_stop() {
    DEBUG(LOG_DEBUG, 0,
          "No active forwarding routes - stopped sending HELLOs!");
    timer_remove(&hello_timer);
}

void NS_CLASS hello_send(void *arg) {
    RREP *rrep;
    AODV_ext *ext = NULL;
    u_int8_t flags = 0;
    struct in_addr dest;
    long time_diff, jitter;
    struct timeval now;
    int msg_size = RREP_SIZE;
    int i;

    if (GLO_OUT) printf("send hello\n");

    gettimeofday(&now, NULL);

    if (optimized_hellos &&
        timeval_diff(&now, &this_host.fwd_time) > ACTIVE_ROUTE_TIMEOUT) {
        hello_stop();
        return;
    }

    time_diff = timeval_diff(&now, &this_host.bcast_time);
    jitter = hello_jitter();

    /* This check will ensure we don't send unnecessary hello msgs, in case
       we have sent other bcast msgs within HELLO_INTERVAL */
    if (time_diff >= HELLO_INTERVAL) {

        for (i = 0; i < MAX_NR_INTERFACES; i++) {
            if (!DEV_NR(i).enabled)
                continue;
#ifdef DEBUG_HELLO
            DEBUG(LOG_DEBUG, 0, "sending Hello to 255.255.255.255");
#endif
            rrep = rrep_create(flags, 0, 0, DEV_NR(i).ipaddr,
                               this_host.seqno,
                               DEV_NR(i).ipaddr,
                               ALLOWED_HELLO_LOSS * HELLO_INTERVAL);

            /* Assemble a RREP extension which contain our neighbor set... */
            if (unidir_hack) {
                int i;

                if (ext)
                    ext = AODV_EXT_NEXT(ext);
                else
                    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);

                ext->type = RREP_HELLO_NEIGHBOR_SET_EXT;
                ext->length = 0;

                for (i = 0; i < RT_TABLESIZE; i++) {
                    list_t *pos;
                    list_foreach(pos, &rt_tbl.tbl[i]) {
                        rt_table_t * rt = (rt_table_t *) pos;
                        /* If an entry has an active hello timer, we assume
                           that we are receiving hello messages from that
                           node... */
                        if (rt->hello_timer.used) {
#ifdef DEBUG_HELLO
                            DEBUG(LOG_INFO, 0,
                              "Adding %s to hello neighbor set ext",
                              ip_to_str(rt->dest_addr));
#endif
                            memcpy(AODV_EXT_DATA(ext), &rt->dest_addr,
                                   sizeof(struct in_addr));
                            ext->length += sizeof(struct in_addr);
                        }
                    }
                }
                if (ext->length)
                    msg_size = RREP_SIZE + AODV_EXT_SIZE(ext);
            }
            dest.s_addr = AODV_BROADCAST;
            aodv_socket_send((AODV_msg *) rrep, dest, msg_size, 1, &DEV_NR(i));
        }

        timer_set_timeout(&hello_timer, HELLO_INTERVAL + jitter);
        //by cyo
        hello_send_add();
        hello_infos_timer_add();
        //cyo_end
    } else {
        if (HELLO_INTERVAL - time_diff + jitter < 0)
            timer_set_timeout(&hello_timer,
                              HELLO_INTERVAL - time_diff - jitter);
        else
            timer_set_timeout(&hello_timer,
                              HELLO_INTERVAL - time_diff + jitter);
    }
}


/* Process a hello message */
void NS_CLASS hello_process(RREP *hello, int rreplen, unsigned int ifindex) {
    u_int32_t hello_seqno, timeout, hello_interval = HELLO_INTERVAL;
    u_int8_t state, flags = 0;
    struct in_addr ext_neighbor, hello_dest;
    rt_table_t * rt;
    AODV_ext *ext = NULL;
    int i;
    struct timeval now;
    //by cyo
    int channel;//todo
    //cyo_end
    gettimeofday(&now, NULL);
    //by cyo
    channel = hello->channel;
    for (i = 0; i < MAX_NR_INTERFACES; i++) {
        if (!DEV_NR(i).enabled)
            continue;
        else
            break;
    }
    for (int j = 0; j < NUM_NODE; j++) {
        // by fxj  3  lines
        if (hash_cmp(&(hello->union_data.hello_infos[j][channel].ipaddr), &(DEV_NR(i).ipaddr))) {
            hello_send_add_nb(hello_dest, hello->channel, hello->union_data.hello_infos[j][channel].hello_send);
            hello_received_add_nb(hello_dest, hello->channel, hello->union_data.hello_infos[j][channel].hello_received);
            break;
        }
    }
    hello_received_add(hello_dest, hello->channel, 1);
    hello_ip_add(hello_dest, channel);
    nb_add(hello_dest);
    //cyo_end

    hello_dest.s_addr = hello->dest_addr;
    hello_seqno = ntohl(hello->dest_seqno);

    rt = rt_table_find(hello_dest);

    if (rt)
        flags = rt->flags;

    if (unidir_hack)
        flags |= RT_UNIDIR;

    /* Check for hello interval extension: */
    ext = (AODV_ext *) ((char *) hello + RREP_SIZE);

    while (rreplen > (int) RREP_SIZE) {
        switch (ext->type) {
            case RREP_HELLO_INTERVAL_EXT:
                if (ext->length == 4) {
                    memcpy(&hello_interval, AODV_EXT_DATA(ext), 4);
                    hello_interval = ntohl(hello_interval);
#ifdef DEBUG_HELLO
                    DEBUG(LOG_INFO, 0, "Hello extension interval=%lu!",
                          hello_interval);
#endif

                } else
                    alog(LOG_WARNING, 0,
                         __FUNCTION__, "Bad hello interval extension!");
                break;
            case RREP_HELLO_NEIGHBOR_SET_EXT:

#ifdef DEBUG_HELLO
                DEBUG(LOG_INFO, 0, "RREP_HELLO_NEIGHBOR_SET_EXT");
#endif
                for (i = 0; i < ext->length; i = i + 4) {
                    ext_neighbor.s_addr =
                            *(in_addr_t * )((char *) AODV_EXT_DATA(ext) + i);

                    if (ext_neighbor.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
                        flags &= ~RT_UNIDIR;
                }
                break;
            default:
                alog(LOG_WARNING, 0, __FUNCTION__,
                     "Bad extension!! type=%d, length=%d", ext->type, ext->length);
                ext = NULL;
                break;
        }
        if (ext == NULL)
            break;

        rreplen -= AODV_EXT_SIZE(ext);
        ext = AODV_EXT_NEXT(ext);
    }

#ifdef DEBUG_HELLO
    DEBUG(LOG_DEBUG, 0, "rcvd HELLO from %s, seqno %lu",
      ip_to_str(hello_dest), hello_seqno);
#endif
    /* This neighbor should only be valid after receiving 3
       consecutive hello messages... */
    if (receive_n_hellos)
        state = INVALID;
    else
        state = VALID;

    timeout = ALLOWED_HELLO_LOSS * hello_interval + ROUTE_TIMEOUT_SLACK;

    if (!rt) {
        /* No active or expired route in the routing table. So we add a
           new entry... */

        rt = rt_table_insert(hello_dest, hello_dest, 1,
                             hello_seqno, timeout, state, flags, ifindex);

        if (flags & RT_UNIDIR) {
            DEBUG(LOG_INFO, 0, "%s new NEIGHBOR, link UNI-DIR",
                  ip_to_str(rt->dest_addr));
        } else {
            DEBUG(LOG_INFO, 0, "%s new NEIGHBOR!", ip_to_str(rt->dest_addr));
        }
        rt->hello_cnt = 1;

    } else {

        if ((flags & RT_UNIDIR) && rt->state == VALID && rt->hcnt > 1) {
            goto hello_update;
        }

        if (receive_n_hellos && rt->hello_cnt < (receive_n_hellos - 1)) {
            if (timeval_diff(&now, &rt->last_hello_time) <
                (long) (hello_interval + hello_interval / 2))
                rt->hello_cnt++;
            else
                rt->hello_cnt = 1;

            memcpy(&rt->last_hello_time, &now, sizeof(struct timeval));
            return;
        }
        rt_table_update(rt, hello_dest, 1, hello_seqno, timeout, VALID, flags);
    }

    hello_update:

    hello_update_timeout(rt, &now, ALLOWED_HELLO_LOSS * hello_interval);
    return;
}


#define HELLO_DELAY 50        /* The extra time we should allow an hello
				   message to take (due to processing) before
				   assuming lost . */

NS_INLINE void NS_CLASS hello_update_timeout(rt_table_t *rt,
                                             struct timeval *now, long time) {
    timer_set_timeout(&rt->hello_timer, time + HELLO_DELAY);
    memcpy(&rt->last_hello_time, now, sizeof(struct timeval));
}

/* by gcy */
void NS_CLASS update_stability() {
    int temp = this_host.stability.neighbor_sum;
    int node_sum = 0;
    int chan_sum = 0;
    for (int i = 0; i < NUM_NODE; i++) {
        int flag = 0;
        for (int j = 0; j < 3; j++) {
            if (this_host.nb_tbl[i][j].isValid) flag++;
        }
        if (flag > 0) {
            node_sum++;
        }
        chan_sum += flag;
    }
    int paraA = node_sum;
    int paraB = node_sum - temp;
    int paraC = chan_sum;
    double paraD = 0;
    int cur_stability = paraA + paraB + paraC + (int) paraD;
    this_host.stability.neighbor_sum = paraA;
    this_host.stability.neighbor_change = paraB;
    this_host.stability.available_channel_num = paraC;
    this_host.stability.best_info_noise_ratio = paraD;
    this_host.stability.stability = cur_stability;
    if (GCY_OUT) printf("A: %d, B: %d, C: %d, D: %f\n", paraA, paraB, paraC, paraD);
}
/* end */
//by cyo
void NS_CLASS nb_add(in_addr ip_temp) {
    for (int i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.nb_tbl[i][0].ipaddr), &ip_temp)) {
            nb_setIsValid(ip_temp,0,1);
            nb_setIsValid(ip_temp,1,1);
            nb_setIsValid(ip_temp,2,1);
            if (CYO_OUT) printf("exist ip_temp is %d\n", ip_temp.s_addr);
            return;
        }
    }
    if (CYO_OUT) printf("new ip_temp is %d\n", ip_temp.s_addr);
    for (int i = 0; i < 3; i++) {
        this_host.nb_tbl[this_host.neighbor_sum_init][i].ipaddr = ip_temp;
        this_host.nb_tbl[this_host.neighbor_sum_init][i].cost = INIT_COST;
        this_host.nb_tbl[this_host.neighbor_sum_init][i].isValid = 1;
    }
    this_host.neighbor_sum_init++;
}

void NS_CLASS nb_update_cost(struct in_addr ip_temp, int channel, float cost_value) {      //仅在邻居表中存有该ip的时候才可有用，注意用法
    if (CYO_OUT) printf("updateCost ip : %d\n", ip_temp.s_addr);
    for (int i = 0; i < this_host.stability.neighbor_sum; i++) {
        if (hash_cmp(&(this_host.nb_tbl[i][channel].ipaddr), &ip_temp)) {
            this_host.nb_tbl[i][channel].cost = cost_value;
            break;
        }
    }
}

void NS_CLASS nb_setIsValid(in_addr ip_temp, int channel, int isValid) {
    for (int i = 0; i < this_host.stability.neighbor_sum; i++) {
        if (hash_cmp(&(this_host.nb_tbl[i][channel].ipaddr), &ip_temp)) {
            this_host.nb_tbl[i][channel].isValid = isValid;
            break;
        }
    }
}

void NS_CLASS hello_received_add(in_addr ip_temp, int channel, int num) {
    int i;
    for (i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            this_host.hello_infos[i][channel].hello_received += num;
            break;
        }
    }
    if (i == NUM_NODE) {
        for (int j = 0; j < NUM_NODE; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipaddr = ip_temp;
                this_host.hello_infos[j][channel].hello_received += num;
                break;
            }
        }
    }
}

void NS_CLASS hello_ip_add(in_addr ip_temp, int channel) {
    int i;
    for (i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            break;
        }
    }
    if (i == NUM_NODE) {
        for (int j = 0; j < NUM_NODE; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipaddr = ip_temp;
                break;
            }
        }
    }
}

void NS_CLASS hello_send_add() {
    for (int i = 0; i < NUM_NODE; i++) {
        for (int j = 0; j < 3; j++) {
            this_host.hello_infos[i][j].hello_send += 1;
        }
    }
}

void NS_CLASS hello_received_add_nb(in_addr ip_temp, int channel, int num) {
    int i;
    for (i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            this_host.hello_infos[i][channel].hello_received_nb = num;
            break;
        }
    }
    if (i == NUM_NODE) {
        for (int j = 0; j < NUM_NODE; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipaddr = ip_temp;
                this_host.hello_infos[j][channel].hello_received_nb = num;
                break;
            }
        }
    }
}

void NS_CLASS hello_send_add_nb(in_addr ip_temp, int channel, int num) {
    int i;
    for (i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            this_host.hello_infos[i][channel].hello_send_nb = num;
            break;
        }
    }
    if (i == NUM_NODE) {
        for (int j = 0; j < NUM_NODE; j++) {
            if (this_host.hello_infos[j][channel].isValid == 0) {
                this_host.hello_infos[j][channel].isValid = 1;
                this_host.hello_infos[j][channel].ipaddr = ip_temp;
                this_host.hello_infos[j][channel].hello_send_nb = num;
                break;
            }
        }
    }
}

void NS_CLASS hello_infos_clear() {
    for (int i = 0; i < NUM_NODE; i++) {
        for (int j = 0; j < 3; j++) {
            this_host.hello_infos[i][j].hello_send = 0;
            this_host.hello_infos[i][j].hello_send_nb = 0;
            this_host.hello_infos[i][j].hello_received_nb = 0;
            this_host.hello_infos[i][j].hello_received = 0;
        }
    }
}

void NS_CLASS hello_infos_timer_add() {
    this_host.hello_infos_timer++;
    if (this_host.hello_infos_timer % TIMER_F == 0) {
        for (int i = 0; i < NUM_NODE; i++) {
            for (int j = 0; j < 3; j++) {
                if (this_host.hello_infos[i][j].isValid == 1) {
                    add_f_value((float) this_host.hello_infos[i][j].hello_received_nb
                                / (float) this_host.hello_infos[i][j].hello_send,
                                this_host.hello_infos[i][j].ipaddr, j);
                }
            }
        }
    }
    if (this_host.hello_infos_timer == TIMER_COST) {
        update_stability();
        this_host.hello_infos_timer = 0;
        for (int i = 0; i < NUM_NODE; i++) {
            for (int j = 0; j < 3; j++) {
                if (CYO_OUT) printf("nb_tbl[%d] is %d\n", i, this_host.nb_tbl[i][j].ipaddr.s_addr);
                updateCost(this_host.nb_tbl[i][j].ipaddr, j);
            }
        }
        hello_infos_clear();
    }
}

void NS_CLASS add_f_value(float f, in_addr ip_temp, int channel) {
    for (int i = 0; i < NUM_HISTORY_F; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            if (this_host.f_tbl[i][channel].count == NUM_HISTORY_F) {
                for (int j = 1; j <= NUM_HISTORY_F - 1; j++) {
                    this_host.f_tbl[i][channel].f_history_value[j - 1] = this_host.f_tbl[i][channel].f_history_value[j];
                }
                this_host.f_tbl[i][channel].f_history_value[NUM_HISTORY_F - 1] = f;
            } else {
                this_host.f_tbl[i][channel].f_history_value[this_host.f_tbl[i][channel].count] = f;
                this_host.f_tbl[i][channel].count++;
            }
        }
    }
}

float NS_CLASS getE(u_int8_t A_send, u_int8_t B_send, u_int8_t A_received, u_int8_t B_received) {
    if (A_send == 0 || B_send == 0) {
        return INIT_E;
    } else if (A_received == 0 || B_received == 0) {
        return 0;
    }
    return (float) (A_received * B_received) / (float) (A_send * B_send);
}

float NS_CLASS getF(struct in_addr ip_temp, int channel) {
    int node_id;
    for (int i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            node_id = i;
        }
    }
    if (node_id == NUM_NODE) {
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

float NS_CLASS getG(const struct node_info historyStab[], int neighbor_sum, int neighbor_change) {
    float result = 0;
    int neighbor_change1 = (neighbor_change > 0) ? neighbor_change : -neighbor_change;
    for (int i = 0; i < NUM_STATES; i++) {
        if (this_host.sta_self[i].isValid == 0 || historyStab[i].isValid == 0) {
            result += INIT_G;
            continue;
        }
        if (this_host.sta_self[i].node_sta == 0 && historyStab[i].node_sta == 0) {
            if (neighbor_sum == 0) {
                result += K00;
            } else {
                result += K00 * (float) (neighbor_sum - neighbor_change1) / (float) neighbor_sum;
            }
        } else if (this_host.sta_self[i].node_sta == 1 && historyStab[i].node_sta == 1) {
            if (neighbor_sum == 0) {
                result += K11;
            } else {
                result += K11 * (float) (neighbor_sum - neighbor_change1) / (float) neighbor_sum;
            }
        } else {
            if (neighbor_sum == 0) {
                result += K01;
            } else {
                result += K01 * (float) (neighbor_sum - neighbor_change1) / (float) neighbor_sum;
            }
        }
    }
    return (result > 0) ? result / (float) NUM_STATES : 0;
}

void NS_CLASS updateCost(in_addr ip_temp, int channel) {
    float E = 0, F = 0, G = 0;
    for (int i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_temp)) {
            E = getE(this_host.hello_infos[i][channel].hello_send, this_host.hello_infos[i][channel].hello_send_nb,
                     this_host.hello_infos[i][channel].hello_received,
                     this_host.hello_infos[i][channel].hello_received_nb);
            G = getG(this_host.sta_tbl[i], this_host.stability.neighbor_sum, this_host.stability.neighbor_change);
            break;
        }
    }
    F = getF(ip_temp, channel);
    float result = A1 * E + B1 * F + C1 * G - (A1 + B1) * E * F - (A1 + C1) * E * G - (B1 + C1) * F * G +
                   (A1 + B1 + C1) * E * F * G;
    if (CYO_OUT) printf("ip is %d, channel is %d\n",ip_temp.s_addr,channel);
    if (CYO_OUT) printf("updateCost %f\n", result);
    nb_update_cost(ip_temp, channel, result);
}

unsigned int hashing(struct in_addr *addr, hash_value *hash) {
    *hash = (hash_value) addr->s_addr;
    return (*hash & RT_TABLEMASK);
}

int NS_CLASS hash_cmp(struct in_addr *addr1, struct in_addr *addr2) {
    hash_value hash;
    return hashing(addr1, &hash) == hashing(addr2, &hash);
}
//cyo_end
