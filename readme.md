# 2021无线网络课设

## 设计方案

### 第三单元

#### 寻路过程

- 当节点$n_j$接收到$n_s$发送的、上一条为$n_i$的RREQ寻路消息 `未过期 && 未被读过 && 传递的const值小于稳定概率阈值`时，认为这是$n_s$到$n_j$的不稳定的路由，暂时连接继续向目标节点$n_o$发送RREQ消息，并将$n_j$的路由表中$(n_s, n_i, n_j)$这一项的状态标志位`volatile`置1，记为不稳定

- 当上述节点$n_j$即为目标节点$n_o$时，照常发送RREP回复，并将路由表状态项置1，无需特殊处理

<img src="https://s2.loli.net/2021/12/06/XxDnI4NeFGE7Kbl.png" alt="image-20211206225447921" style="zoom: 67%;" />

- 当节点$n_j$对应的路由表中$(n_s, n_i, n_j)$有`volatile=1`，且收到`相同序列号 && 相同源节点 && const > Pmin`的RREQ消息时，认为找到了一条跳数较短的稳定路由，将$n_j$路由表中$n_s$对应路由表项的上一条$n_j$、跳数$ttp$进行更改，并将标志位`volatile`置0

<img src="https://s2.loli.net/2021/12/06/mXb5skH7v24VjiR.png" alt="image-20211206225447921" style="zoom: 67%;" />

- 当上述节点$n_j$即为目标节点$n_o$时，将序列号+1，发送一条特殊的RREP消息，根据更新的上一跳节点(反向路由)更新路由表中的下一跳节点(正向路由)。由于序列号更新过，被舍弃的节点的相应路由表项会自动失效，不需要再进行处理。

<img src="https://s2.loli.net/2021/12/06/chJKaqg6A4YLMWb.png" alt="image-20211206225447921" style="zoom: 67%;" />

#### 源码更改

```c++
/*** defs.h ***/
/* cost寻路和快速修复的开关 */
/* 用来跑tcl验证时区分原AODV协议和我们的策略 */
#define USE_YRB 1
#define USE_FXJ 1

/* 稳定概率阈值 */
#define COST_MIN 0.8


/*** routing_table.h ***/
/* 用于标记该路由是否稳定 */
struct rt_table
{
    u_int8_t volatile;  /* 不稳定为1 */
}
/* volatile参数 */
rt_table_t *rt_table_insert(struct in_addr dest, struct in_addr next,
			    u_int8_t hops, u_int32_t seqno, u_int32_t life,
			    u_int8_t state, u_int16_t flags,
			    unsigned int ifindex,
                u_int8_t volatile);
rt_table_t *rt_table_update(rt_table_t * rt, struct in_addr next, u_int8_t hops,
			    u_int32_t seqno, u_int32_t lifetime, u_int8_t state,
			    u_int16_t flags
                u_int8_t volatile);


/*** routing_table.c ***/
/* volatile参数 */
rt_table_t *rt_table_insert(struct in_addr dest, struct in_addr next,
			    u_int8_t hops, u_int32_t seqno, u_int32_t life,
			    u_int8_t state, u_int16_t flags,
			    unsigned int ifindex,
                u_int8_t volatile=0)
{
    rt->volatile = volatile;
}
rt_table_t *rt_table_update(rt_table_t * rt, struct in_addr next, u_int8_t hops,
			    u_int32_t seqno, u_int32_t lifetime, u_int8_t state,
			    u_int16_t flags
                u_int8_t volatile=0)
{
    
	rt->volatile = volatile;
}

/* 函数rt_table_insert */
    /* 添加参数u_int8_t volatile */
	rt->volatile = volatile;

/* TODO: 邻居节点的invalidate(?) */


/*** aodv_rreq.h ***/
/* 回路稳定概率，为链路稳定性概率乘积 */
typedef struct {
    float cost;
} RREQ;


/*** aodv_rreq.c ***/
/* rreq_process */
    /* 计算当前链路的cost值 */
    float cost = rreq->cost; 
    int i;
    for (i = 0; i < NUM_NODE; i++) {
        if (hash_cmp(&(this_host.hello_infos[i][channel].ipaddr), &ip_src)) {
            break;
        }
    }
    if (i < NUM_NODE) {
        cost *= this_host.hello_infos[i][channel].cost;
    }
	int volatile = 0;
	if (cost < COST_MIN) volatile = 1;

    /* 前面路由不稳定的节点收到一条稳定的RREQ需要继续转发 */
    /* Check if this RREQ has been processed */
    if (rreq_record_find(rreq_orig, rreq_id)) {
        fwd_rt = rt_table_find(rreq_dest);
        /* Ignore already processed RREQs. */
        if (!(fwd_rt && fwd_rt->state == VALID && fwd_rt->volatile && volatile))
        return;
    }

	/* 建立反向路由添加volatile */
	rt_table_insert(..., volatile);
	rt_table_update(..., volatile);


/*** aodv_rrep.h ***/
/* 对不起我发现我不需要update_next_hop了 */


/*** aodv_rrep.c ***/
/* 函数rrep_process */
    /* 更新正向路由 */
    /* 传递volatile值 */
	if (fwd_rt->dest_seqno == 0 ||
	       (int32_t) rrep_seqno > (int32_t) fwd_rt->dest_seqno ||
		   (rrep_seqno == fwd_rt->dest_seqno &&
		   (fwd_rt->state == INVALID || fwd_rt->flags & RT_UNIDIR || rrep_new_hcnt < fwd_rt->hcnt
		   || (fwd_rt->volatile && !rev_rt->volatile)))) 
	{
		pre_repair_hcnt = fwd_rt->hcnt;
		pre_repair_flags = fwd_rt->flags;

		fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
					rrep_lifetime, VALID,
					rt_flags | fwd_rt->flags,
					rev_rt->volatile); 
    } 
```


### fxj_part
#### RREP 消息变更
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |     Type      |R|A|N|   Reserved    |Prefix Sz|   Hop Count   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                     Destination IP address                    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                  Destination Sequence Number                  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                    Originator IP address                      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           Lifetime                            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           Channel                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |  Updt_nxt_hop |          Padding ( 24 empty bits )            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                          Union (data)               			|
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   新增位 N：仅能在 Hello 消息中置 1，表示向所有邻居索要它们的邻居表。每个接收到这个广播的邻居都通过消息发回自己的邻居表

