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


#include "net/uip.h"
#include "net/uip-ds6.h"
#include "string.h"

#include "vitalprop.h"
#include <string.h>
#include "lib/random.h"

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
/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
void
vitalprop_close(struct vitalprop_conn *c)
{
  ctimer_stop(&c->t);
}
/*---------------------------------------------------------------------------*/
void
vitalprop_send(struct vitalprop_conn *c)
{
  struct ripplecomm_vp_message m;

  uip_ipaddr_t addr;
  update_all_ranks(&(c->vl));
  vp_list_to_msg(&(c->vl),&m);
  uip_create_linklocal_allnodes_mcast(&addr);
  simple_udp_sendto(&(c->cu), &m, sizeof(struct ripplecomm_vp_message), &addr);

  age_vp_list(&(c->vl));
}
/*---------------------------------------------------------------------------*/
/** @} */
