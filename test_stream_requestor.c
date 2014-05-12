
/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *         Test Program to show a subscription interface
 * \author
 *         Adam Renner
 */


#include "contiki.h"

#include "common.h"

#include "lib/random.h"

#include "dev/leds.h"



#include "sys/etimer.h"


#include <stdio.h>
#include <string.h>
#include "dev/button-sensor.h"

#if WITH_UIP6
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "simple-udp.h"

#include "net/uip-debug.h"
#include "net/rpl/rpl.h"
#define UDP_PORT 5688
#else
#endif//WITH_UIP6


//simple udp
static struct simple_udp_connection requestor_connection;

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
  //printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf("\n");
  //printf(" on port %d from port %d with length %d: '%s'\n",
   //      receiver_port, sender_port, datalen, data);
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(test_requestor_process, "Subscription example");
AUTOSTART_PROCESSES(&test_requestor_process);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
static void
create_rpl_dag(uip_ipaddr_t *ipaddr)
{
  struct uip_ds6_addr *root_if;

  root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    uip_ipaddr_t prefix;

    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &prefix, 64);
    //printf("created a new RPL dag\n");
  } else {
    //printf("failed to create a new RPL DAG\n");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_requestor_process, ev, data)
{
  //Still need to work on how to send the address to the subscription, using static for now
  static uip_ipaddr_t addr;
  uip_ipaddr_t *ipaddr;
  static uip_ipaddr_t myaddr;
  struct ripplecomm_s_req m;
  static int device_mode = 0;

  PROCESS_BEGIN();
  ipaddr = set_global_address();
  uip_ipaddr_copy(&myaddr,ipaddr);

  create_rpl_dag(ipaddr);


  simple_udp_register(&requestor_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);
  SENSORS_ACTIVATE(button_sensor);

  while(1)
  {
    PROCESS_YIELD();
    if (ev == sensors_event && data == &button_sensor)
    {

      if (device_mode == 3)
      {
        device_mode = 0;
      }
      device_mode++;
      printf("Device Mode %d\n", device_mode);
      if (device_mode == 1)
      {
        //device_mode++;
        //printf("Requesting Respiration Subscription\n");
        m.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
        m.r_header.r_msg_type=RESP_STREAM_REQUEST;
        //m.r_sink = *ipaddr;
        uip_ipaddr_copy((&m.r_sink),(&myaddr));
        m.r_expiration=0x0F;
        uip_ip6addr(&addr, 0xff02, 0, 0, 0, 0, 0, 0, 1);
        simple_udp_sendto(&requestor_connection, &m, sizeof(struct ripplecomm_s_req), &addr);
      }
      else if (device_mode == 2)
      {
        //device_mode++;
        //printf("Requesting ECG Subscription\n");
        m.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
        m.r_header.r_msg_type=ECG_STREAM_REQUEST;
        uip_ipaddr_copy((&m.r_sink),(&myaddr));
        m.r_expiration=0x0F;
        uip_ip6addr(&addr, 0xff02, 0, 0, 0, 0, 0, 0, 1);
        simple_udp_sendto(&requestor_connection, &m, sizeof(struct ripplecomm_s_req), &addr);
      }
      else if (device_mode == 3)
      {
        //device_mode = 0;
        //printf("Requesting RippleMessage Subscription\n");
        m.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
        m.r_header.r_msg_type=VITALUCAST_REQUEST;
        uip_ipaddr_copy((&m.r_sink),(&myaddr));
        m.r_expiration=0x0F;
        uip_ip6addr(&addr, 0xff02, 0, 0, 0, 0, 0, 0, 1);
        simple_udp_sendto(&requestor_connection, &m, sizeof(struct ripplecomm_s_req), &addr);
      }


    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
