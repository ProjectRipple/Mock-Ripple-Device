/**
 * \addtogroup vitalprop
 * @{
 */

/**
 * \file
 *         VitalProp (vitalprop based DTN for vital sign propogation)
 * \author
 *         Adam Renner
 */

//#define WITH_UIP6 1

#if WITH_UIP6
#include "net/uip.h"
#include "net/uip-ds6.h"
//#include "simple-udp.h"
#include "string.h"
#else
#include "net/rime.h"
#endif//WITH_UIP6

#if WITH_UIP6
#else
#endif//WITH_UIP6

#include "vitalprop.h"
#include <string.h>
#include "lib/random.h"

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int run_vitalprop(struct vitalprop_conn *c);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
timer_callback(void *ptr)
{
  struct vitalprop_conn *c = ptr;
  vitalprop_send(c);
  c->vl.e[0].r.r_seqid++;
  c->vl.e[0].r.r_hops = 0;
  run_vitalprop(c);
}
/*---------------------------------------------------------------------------*/
static void
reset_interval(struct vitalprop_conn *c)
{
  PT_INIT(&c->pt);
  run_vitalprop(c);
}
/*---------------------------------------------------------------------------*/
static void
set_timer(struct vitalprop_conn *c, struct ctimer *t, clock_time_t i)
{
  ctimer_set(t, i, timer_callback, c);
}
/*---------------------------------------------------------------------------*/
static int
run_vitalprop(struct vitalprop_conn *c)
{
  clock_time_t interval;
  PT_BEGIN(&c->pt);

  while(1) {
    interval = c->interval;
    set_timer(c, &c->t, interval / 2 + (random_rand() % (interval/2 )));
    PT_YIELD(&c->pt); /* Wait until interval timer expired. */
  }
  PT_END(&c->pt);
  return 1;
}
/*---------------------------------------------------------------------------*/
#if WITH_UIP6
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  struct ripplecomm_vp_message m;
  struct vitalprop_conn *vpc = (struct vitalprop_conn *)c;
  //printf("Data received on port %d from port %d with length %d\n",
  //       receiver_port, sender_port, datalen);

  memcpy(&m, data, datalen);
  if (m.header.r_dispatch == RIPPLECOMM_DISPATCH && m.header.r_msg_type == VITALPROP_RECORDS && RIPPLECOMM_VERSION_COMPATIBLE(m.header.r_version))
  {
    vp_list_from_msg(&(vpc->vl),m);
    vpc->cb->recv(vpc);//need to send this a vitalprop_conneciton struct, not simple_udp_connection
  }
}
#else
static void
recv(struct broadcast_conn *bc, const rimeaddr_t *from)
{
  struct vitalprop_conn *c = (struct vitalprop_conn *)bc;
  struct ripplecomm_vp_message m;

  PRINTF("%d.%d: vitalprop recv seqno %d from %d.%d our %d data len %d channel %d\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 7,
	 from->u8[0], from->u8[1],
	 8,
	 packetbuf_datalen(),
	 packetbuf_attr(PACKETBUF_ATTR_CHANNEL));

  packetbuf_copyto(&m);
  if (m.header.r_dispatch == RIPPLECOMM_DISPATCH && m.header.r_msg_type == VITALPROP_RECORDS && RIPPLECOMM_VERSION_COMPATIBLE(m.header.r_version))
  {
    vp_list_from_msg(&(c->vl),m);
    c->cb->recv(c);
  }
}
#endif//WITH_UIP6
/*---------------------------------------------------------------------------*/
#if WITH_UIP6
#else
static CC_CONST_FUNCTION struct broadcast_callbacks bc = { recv };
#endif//WITH_UIP6
/*---------------------------------------------------------------------------*/
#if WITH_UIP6
void vitalprop_open(struct vitalprop_conn *c, clock_time_t interval,
	     uint16_t udp_port, const struct vitalprop_callbacks *cb)
{
  simple_udp_register(&(c->cu), udp_port,
                      NULL, udp_port,
                      receiver);
  c->cb = cb;
  //Need to verify that rimeaddr_nod_addr reliably duplicated iid
  vp_list_init(&(c->vl), rimeaddr_node_addr);
  c->interval = interval;
  reset_interval(c);
}
#else
void
vitalprop_open(struct vitalprop_conn *c, clock_time_t interval,
	     uint16_t channel, const struct vitalprop_callbacks *cb)
{
  broadcast_open(&c->c, channel, &bc);
  c->cb = cb;
  vp_list_init(&(c->vl), rimeaddr_node_addr);
  c->interval = interval;
  reset_interval(c);
}
#endif//WITH_UIP6
/*---------------------------------------------------------------------------*/
void
vitalprop_close(struct vitalprop_conn *c)
{
#if WITH_UIP6
//stopping the timer may be enough
#else
  broadcast_close(&c->c);
#endif//WITH_UIP6
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
void
vitalprop_send(struct vitalprop_conn *c)
{
  struct ripplecomm_vp_message m;
#if WITH_UIP6
  uip_ipaddr_t addr;
#endif//WITH_UIP6
  update_all_ranks(&(c->vl));
  vp_list_to_msg(&(c->vl),&m);
#if WITH_UIP6
  uip_create_linklocal_allnodes_mcast(&addr);
  simple_udp_sendto(&(c->cu), &m, sizeof(struct ripplecomm_vp_message), &addr);
#else
  packetbuf_copyfrom(&m, sizeof(struct ripplecomm_vp_message));
  broadcast_send(&c->c);
#endif//WITH_UIP6
  age_vp_list(&(c->vl));
}
/*---------------------------------------------------------------------------*/
/** @} */
