

#include "contiki.h"
#include "vitalprop.h"

#include "lib/random.h"

#include "dev/leds.h"

#include <stdio.h>

#if WITH_UIP6
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "simple-udp.h"
#define UDP_PORT 1234
#else
#endif//WITH_UIP6
/*---------------------------------------------------------------------------*/
PROCESS(example_vp_process, "VP example");
AUTOSTART_PROCESSES(&example_vp_process);
/*---------------------------------------------------------------------------*/
static void
vp_recv(struct vitalprop_conn *c)
{
  printf("%d.%d: vp message received\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
}
const static struct vitalprop_callbacks vitalprop_call = {vp_recv};
static struct vitalprop_conn vp;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_vp_process, ev, data)
{
  static struct etimer et;
#if WITH_UIP6
  //uip_ipaddr_t addr;
#endif//WITH_UIP6
  PROCESS_EXITHANDLER(vitalprop_close(&vp);)
  PROCESS_BEGIN();
#if WITH_UIP6
  vitalprop_open(&vp, CLOCK_SECOND*10, UDP_PORT, &vitalprop_call);
#else
  vitalprop_open(&vp, CLOCK_SECOND*10, 145, &vitalprop_call);
#endif//WITH_UIP6

  while(1) {

    etimer_set(&et, CLOCK_SECOND * 5 + random_rand() % (CLOCK_SECOND * 2));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    //packetbuf_copyfrom("Hello, world", 13);
    //vp.vl.e[0].r.r_seqid++;
    vp.vl.e[0].r.heart_rate++;
    //vitalprop_send(&vp);

  }
  PROCESS_END();
}
