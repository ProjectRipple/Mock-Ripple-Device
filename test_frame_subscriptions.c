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

#include "vitalprop.h"
#include "frame_buffer.h"
#include "frame_subscription.h"

#include "lib/random.h"

#include "dev/leds.h"

#include "sys/etimer.h"

#include <stdio.h>
#include <string.h>
#include "dev/button-sensor.h"
#include "net/rime.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "simple-udp.h"
#define UDP_PORT 5688
#define UDP_PORT2 5689

#define STREAM_FRAMES_PER_SECOND 2

//buffers should probably be declared volatile since an interrupt or
//other code will likely be making changes to it.
static struct ripplecomm_record current_vitals = {{0}};
static struct resp_frame_buffer fake_resp_buffer;
static struct ecg_frame_buffer fake_ecg_buffer;
static struct subscription_list fake_resp_sl;
static struct subscription_list fake_ecg_sl;
static struct subscription_list record_sl;

//const static struct vitalprop_callbacks vitalprop_call = {vp_recv};
static struct vitalprop_conn vp;

static struct simple_udp_connection vitalcast_connection;

//Fake Signal Event
static process_event_t buffer_flop_event;
static process_event_t vital_update_event;
/*---------------------------------------------------------------------------*/
//subscription callback routine -  update vplist with current vitals for vitalprop
static void vp_update(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  vp.vl.e[0].r = current_vitals;
}
/*---------------------------------------------------------------------------*/
//subscription callback routine -  UDP unicast data
static void vc_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_message rm;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type=VITALUCAST_RECORD;
  memcpy(&(rm.r_record),frame_ptr,sizeof(struct ripplecomm_record));
  rm.r_record.r_seqid = uip_htons(rm.r_record.r_seqid);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_record), (uip_ipaddr_t *)subscription_data);
}
/*---------------------------------------------------------------------------*/
//subscription callback routine -  UDP unicast data
static void resp_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_resp rm;
  struct resp_frame *rf = (struct resp_frame *)frame_ptr;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type=RESP_STREAM;
  memcpy(rm.store,rf->store,RESP_STREAM_SIZE*2);
  rm.seq = uip_htons(rf->seq);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_resp), (uip_ipaddr_t *)subscription_data);
}
/*---------------------------------------------------------------------------*/
static void ecg_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_ecg rm;
  struct ecg_frame *rf = (struct ecg_frame *)frame_ptr;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type=ECG_STREAM;
  memcpy(rm.store,rf->store,ECG_STREAM_SIZE*2);
  rm.seq = uip_htons(rf->seq);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_ecg), (uip_ipaddr_t *)subscription_data);
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
  struct ripplecomm_header h;
  //printf("Data received on port %d from port %d with length %d\n", receiver_port, sender_port, datalen);
  memcpy(&h, data, sizeof(struct ripplecomm_header));

  if (h.r_dispatch == RIPPLECOMM_DISPATCH && h.r_msg_type >= RIPPLECOMM_VERSION_MIN_COMPAT)
  {
    //printf("Ripplecomm message received\n");
    if (h.r_msg_type == RESP_STREAM_REQUEST)
    {
      struct ripplecomm_s_req sr;
      subscription_data_t sink = {{0}};
      memcpy(&sr, data, sizeof(struct ripplecomm_s_req));
      memcpy(&(sink),&sr.r_sink,sizeof(uip_ipaddr_t));
      //printf("creating respiration subscription\n");
      create_subscription(&fake_resp_sl, 1, sr.r_expiration, resp_send, sink);
    }
    if (h.r_msg_type == ECG_STREAM_REQUEST)
    {
      struct ripplecomm_s_req sr;
      subscription_data_t sink = {{0}};
      memcpy(&sr, data, sizeof(struct ripplecomm_s_req));
      memcpy(&(sink),&sr.r_sink,sizeof(uip_ipaddr_t));
      //printf("creating ecg subscription\n");
      create_subscription(&fake_resp_sl, 1, sr.r_expiration, ecg_send, sink);
    }
    if (h.r_msg_type == VITALUCAST_REQUEST)
    {
      struct ripplecomm_s_req sr;
      subscription_data_t sink = {{0}};
      memcpy(&sr, data, sizeof(struct ripplecomm_s_req));
      memcpy(&(sink),&sr.r_sink,sizeof(uip_ipaddr_t));
      //printf("creating record subscription\n");
      create_subscription(&record_sl,1,sr.r_expiration,vc_send,sink);
    }
  }
}
/*---------------------------------------------------------------------------*/
#if WITH_UIP6
static void set_global_address(void)
{
  uip_ipaddr_t ipaddr;
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
}
#endif//WITH_UIP6
/*---------------------------------------------------------------------------*/
static void
vp_recv(struct vitalprop_conn *c)
{
//  printf("%d.%d: vp message received\n",
//	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
}
/*---------------------------------------------------------------------------*/
PROCESS(test_subscription_process, "Subscription example");
PROCESS(fake_signal_process, "fake signal");
AUTOSTART_PROCESSES(&test_subscription_process);
#if WITH_UIP6

#else
static const struct unicast_callbacks unicast_callbacks = {recv};

#endif//WITH_UIP6
const static struct vitalprop_callbacks vitalprop_call = {vp_recv};
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(fake_signal_process, ev, data)
{
  static uint16_t resp_val;
  static uint16_t ecg_val;
  static struct etimer et;
  PROCESS_BEGIN();
  //printf("fake signal started!\n");

  while(1) {
    etimer_set(&et, CLOCK_SECOND/STREAM_FRAMES_PER_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //memset(fake_ecg_buffer.write_in->store,0,ECG_FRAME_CAPACITY);
    //memset(resp_frame_buffer.write_in->store,0,RESP_FRAME_CAPACITY);

    static int i;
    for (i=0;i<RESP_FRAME_CAPACITY;i++)
    {
      resp_val += 1;//increase respiration value
#if WITH_UIP6
      fake_resp_buffer.write_in->store[i]=uip_htons(resp_val);//uip_htons converts to network byte order, usually used before sending
#else
      fake_resp_buffer.write_in->store[i]=resp_val;
#endif//WITH_UIP6
    }
    for (i=0;i<ECG_FRAME_CAPACITY;i++)
    {
      ecg_val += 1;//increase respiration value
#if WITH_UIP6
      fake_ecg_buffer.write_in->store[i]=uip_htons(ecg_val);//uip_htons converts to network byte order, usually used before sending
#else
      fake_ecg_buffer.write_in->store[i]=ecg_val;
#endif//WITH_UIP6
    }

    //flop buffer, fire signal
    resp_frame_buffer_swap(&fake_resp_buffer);
    ecg_frame_buffer_swap(&fake_ecg_buffer);
    process_post(&test_subscription_process, buffer_flop_event, 0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static void current_vitals_update(void *ptr)
{
  current_vitals.r_seqid++;
  current_vitals.heart_rate++;
  current_vitals.temperature++;
  current_vitals.spo2++;
  execute_subscription_callbacks(&record_sl,&current_vitals,NULL);
  process_post(&test_subscription_process, vital_update_event, 0);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_subscription_process, ev, data)
{
  //static struct etimer vtimer;
  static struct ctimer vtimer;
  static int report_mode = 0;// 0 - broadcast, 1 -vitalprop, 2 - unicast to subscribers
#if WITH_UIP6
  uip_ipaddr_t bcast_addr;

#else
  //PROCESS_EXITHANDLER(unicast_close(&vitalcast_connection);)
#endif//WITH_UIP6
  PROCESS_EXITHANDLER(vitalprop_close(&vp);)
  PROCESS_BEGIN();
  resp_frame_buffer_init(&fake_resp_buffer);//this needs to come after process begin
  ecg_frame_buffer_init(&fake_ecg_buffer);
  init_subscription_list(&fake_resp_sl);
  init_subscription_list(&fake_ecg_sl);
  init_subscription_list(&record_sl);
  //Create some generic vital values:
  //current_vitals = {{0}};
  current_vitals.record_addr=rimeaddr_node_addr;
  current_vitals.r_seqid=0;
  current_vitals.heart_rate=1;
  current_vitals.temperature = 2;
  //printf("test process started\n");
  buffer_flop_event = process_alloc_event();
  vital_update_event = process_alloc_event();
#if WITH_UIP6
  set_global_address();
  simple_udp_register(&vitalcast_connection, UDP_PORT, NULL, UDP_PORT, receiver);
#else
  broadcast_open(&vitalcast_connection, VITALUCAST_CHANNEL, &unicast_callbacks);
#endif//WITH_UIP6
  //start the fake signal (like starting rtimer process)
  process_start(&fake_signal_process, 0);

  SENSORS_ACTIVATE(button_sensor);
  ctimer_set(&vtimer, CLOCK_SECOND,current_vitals_update,NULL);

  while(1)
  {
    //etimer_set(&vtimer, CLOCK_SECOND);

    PROCESS_YIELD();
    if(ev == buffer_flop_event)
    {
      execute_subscription_callbacks(&fake_resp_sl,fake_resp_buffer.read_out,NULL);
      execute_subscription_callbacks(&fake_ecg_sl,fake_ecg_buffer.read_out,NULL);
    }
    else if (ev == vital_update_event)
    {
      ctimer_set(&vtimer, CLOCK_SECOND,current_vitals_update,NULL);
    }
    else if (ev == sensors_event && data == &button_sensor)
    {
      if (report_mode == 3)
      {
        report_mode = 0;
      }
      report_mode++;
      printf("Report Mode %d\n", report_mode);
      //switch between unicast request only, broadcast, and vitalprop modes
      //on mode switch, clear all subscriptions, create new ones, this will even clear requested unicast subscriptions
      clear_subscriptions(&record_sl);

      if (report_mode == 2 )
      {
        //vitalprop mode
        struct ripplecomm_s_req sr;
        subscription_data_t sink = {{0}};
#if WITH_UIP6
        vitalprop_open(&vp, CLOCK_SECOND*10, UDP_PORT2, &vitalprop_call);
#else
        vitalprop_open(&vp, CLOCK_SECOND*10, VITALUCAST_CHANNEL2, &vitalprop_call);
#endif//WITH_UIP6
        //printf("creating vitalprop ubscription");
        create_subscription(&record_sl,0,sr.r_expiration,vp_update,sink);
      }

      else if (report_mode == 1 )
      {
        //broadcast mode -only with uip
        //vitalprop_close(&vp);
#if WITH_UIP6
        struct ripplecomm_s_req sr;
        subscription_data_t sink = {{0}};
        uip_create_linklocal_allnodes_mcast(&bcast_addr);
        memcpy(&(sink), &bcast_addr,sizeof(uip_ipaddr_t));
        //printf("creating broadcast ubscription");
        create_subscription(&record_sl,0,sr.r_expiration,vc_send,sink);
#endif//WITH_UIP6
        //report_mode++;
      }
      else if (report_mode ==3 )
      {
        //return to default request only mode
        vitalprop_close(&vp);
      }

    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
