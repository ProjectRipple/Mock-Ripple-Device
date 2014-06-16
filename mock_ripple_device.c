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
#include "frame_buffer.h"
#include "frame_subscription.h"
#include "dummy_ecg.h"
#include "lib/random.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>
#include "dev/button-sensor.h"

/*
#include "net/rime.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
*/
#include "net/rime/rime.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "simple-udp.h"

#ifdef REAL_SENSORS
#include "uart.h"
#include "math.h"
#else
#include "lib/random.h"
#endif



//buffers could probably be declared volatile since an interrupt
//service routine or
//other code will likely be making changes to it in real hardware.
static struct ripplecomm_record current_vitals = {{0}};
static struct resp_frame_buffer fake_resp_buffer;
static struct ecg_frame_buffer fake_ecg_buffer;
static struct subscription_list fake_resp_sl;
static struct subscription_list fake_ecg_sl;
static struct subscription_list record_sl;

static char device_id[] = "0000000000000000";//64 bit unique id

static const double TEMPERATURE_RESISTOR = 2200.0;

static struct simple_udp_connection vitalcast_connection;

//Fake Signal Event
static process_event_t buffer_flop_event;
static process_event_t vital_update_event;

/*---------------------------------------------------------------------------*/
//subscription callback routine -  UDP unicast data
static void vc_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_message rm;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = VITALUCAST_RECORD;
  memcpy(&(rm.r_record),frame_ptr,sizeof(struct ripplecomm_record));
  rm.r_record.r_seqid = uip_htons(rm.r_record.r_seqid);
  rm.r_record.temperature = uip_htons(rm.r_record.temperature);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_message), (uip_ipaddr_t *)subscription_data);
}
/*---------------------------------------------------------------------------*/
//subscription callback routine -  UDP unicast data
static void resp_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_resp rm;
  struct resp_frame *rf = (struct resp_frame *)frame_ptr;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = RESP_STREAM;
  memcpy(rm.store,rf->store,RESP_STREAM_SIZE*2);
  rm.seq = uip_htonl(rf->seq);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_resp), (uip_ipaddr_t *)subscription_data);

}
/*---------------------------------------------------------------------------*/
static void ecg_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_ecg rm;
  struct ecg_frame *rf = (struct ecg_frame *)frame_ptr;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = ECG_STREAM;
  rm.blank = 0x1111; // set buffer bytes so they are easily identified
  memcpy(rm.store,rf->store,ECG_STREAM_SIZE*2);
  rm.seq = uip_htonl(rf->seq);
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
    printf("Ripplecomm message received\n");
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
      printf("creating ecg subscription\n");
      create_subscription(&fake_ecg_sl, 1, sr.r_expiration, ecg_send, sink);
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

static void set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  //uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  //uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  //uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

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

/*---------------------------------------------------------------------------*/
PROCESS(test_subscription_process, "Subscription example");
PROCESS(fake_signal_process, "fake signal");
AUTOSTART_PROCESSES(&test_subscription_process);
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(fake_signal_process, ev, data)
{
  static uint16_t resp_val;
  static uint16_t ecg_val;
  static struct etimer et;
  static int i,j;
  PROCESS_BEGIN();
  //printf("fake signal started!\n");


  while(1) {
    etimer_set(&et, CLOCK_SECOND/(CLOCK_CONF_SECOND));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //memset(fake_ecg_buffer.write_in->store,0,ECG_FRAME_CAPACITY);
    //memset(resp_frame_buffer.write_in->store,0,RESP_FRAME_CAPACITY);
#ifdef REAL_SENSORS
    adc_service();
    ecg_val = adc_reading[2];
    fake_ecg_buffer.write_in->store[j]=uip_htons(ecg_val);
    //printf("%d\n",ecg_val);
#else
    ecg_val = dummy_ecg_next();
    fake_ecg_buffer.write_in->store[j]=uip_htons(ecg_val);
#endif
    j++;
    if (j == ECG_FRAME_CAPACITY) {
      j = 0;
      ecg_frame_buffer_swap(&fake_ecg_buffer);
      process_post(&test_subscription_process, buffer_flop_event, 0);
    }

    /*int i,j;
    for (i=0;i<RESP_FRAME_CAPACITY;i++)
    {
      resp_val += 1;//increase respiration value
      fake_resp_buffer.write_in->store[i]=uip_htons(resp_val);//uip_htons converts to network byte order, usually used before sending
    }
    for (j=0;j<ECG_FRAME_CAPACITY;j++)
    {
      ecg_val = dummy_ecg_next();
      fake_ecg_buffer.write_in->store[j]=uip_htons(ecg_val);//uip_htons converts to network byte order, usually used before sending
    }*/

    //flop buffer, fire signal
    //resp_frame_buffer_swap(&fake_resp_buffer);
    
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static void current_vitals_update(void *ptr)
{
  uint8_t status = 0;
  double resistance = 0;
  double temperatureD = 0;
  uint16_t temp_hr;
#ifdef REAL_SENSORS
  adc_service();
  resistance = TEMPERATURE_RESISTOR / (((double)(adc_vbatt)/(double)adc_voltage(4)) - 1.0);
  resistance /= 1000;  //Next line function requires resistance to be in KOhms
  temperatureD = (5.7513*pow(resistance, 2) - 34.018*(resistance) + 72.661);
  current_vitals.temperature = (uint8_t)(temperatureD * 1.8) + 32;
  status=0;
  while(uart2_can_get())
  {
    status = uart2_getc();
    if((status & 0x80))
    {
      if(uart2_can_get())
      {
        temp_hr = uart2_getc();
        temp_hr = (((status & 0x03) << 7) | temp_hr);
        if(temp_hr == 0x01FF)
        {
            //printf("No heart rate reported\n");
        }
        if (temp_hr > 255) {
          temp_hr = 255;
        }
        current_vitals.heart_rate = temp_hr;
      }
      if(uart2_can_get())
      {
        current_vitals.spo2 = uart2_getc();
        if(current_vitals.spo2 == 0x7F)
        {
            //printf("No Spo2\n");
        }
      }
    //printf("%x %x %x\n", status, current_vitals.heart_rate, current_vitals.spo2);
    }  
    else {
    //printf("not status byte\n");
    }  
  }
  printf("%x %d %d %d\n", status, current_vitals.heart_rate, current_vitals.spo2, current_vitals.temperature);
#else
  current_vitals.temperature = random_rand()%11 + 90;
  current_vitals.heart_rate = random_rand()%41 + 60;
  current_vitals.spo2 = random_rand()%11 + 90;
  current_vitals.bpm = random_rand()%21 + 30;
#endif
  current_vitals.r_seqid++;
  execute_subscription_callbacks(&record_sl,&current_vitals,NULL);
  uip_ipaddr_copy(&(current_vitals.device_ipv6),&uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
  process_post(&test_subscription_process, vital_update_event, 0);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_subscription_process, ev, data)
{
  //static struct etimer vtimer;
  static struct ctimer vtimer;
  static int report_mode = 0;// 0 - broadcast, 1 -vitalprop, 2 - unicast to subscribers
  uip_ipaddr_t sink_addr;
  PROCESS_BEGIN();


#ifdef REAL_SENSORS
  // start adc
  adc_init();
  adc_setup_chan(0);
  adc_setup_chan(2);
  // temperature channel
  adc_setup_chan(4);
  // initialize uart2
  uart_init(UART2, 9600);
#endif
  
  
  resp_frame_buffer_init(&fake_resp_buffer);//this needs to come after process begin
  ecg_frame_buffer_init(&fake_ecg_buffer);
  init_subscription_list(&fake_resp_sl);
  init_subscription_list(&fake_ecg_sl);
  init_subscription_list(&record_sl);
  //Create some generic vital values:
  //current_vitals = {{0}};
  current_vitals.record_addr=linkaddr_node_addr;
  current_vitals.r_seqid=0;
  current_vitals.heart_rate=1;
  current_vitals.temperature = 2;
  //printf("test process started\n");
  buffer_flop_event = process_alloc_event();
  vital_update_event = process_alloc_event();
  set_global_address();

  sprintf(device_id,"%02X%02X%02X%02X%02X%02X%02X%02X",linkaddr_node_addr.u8[0],
          linkaddr_node_addr.u8[1],linkaddr_node_addr.u8[2],linkaddr_node_addr.u8[3],
          linkaddr_node_addr.u8[4],linkaddr_node_addr.u8[5],linkaddr_node_addr.u8[6],
          linkaddr_node_addr.u8[7]);

  uip_ipaddr_copy(&(current_vitals.device_ipv6),&uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
  simple_udp_register(&vitalcast_connection, UDP_PORT, NULL, UDP_PORT, receiver);
  //start the fake signal (like starting rtimer process)
  process_start(&fake_signal_process, 0);

  SENSORS_ACTIVATE(button_sensor);
  ctimer_set(&vtimer, CLOCK_SECOND*5,current_vitals_update,NULL);

  while(1)
  {

    PROCESS_YIELD();
    if(ev == buffer_flop_event)
    {
      execute_subscription_callbacks(&fake_resp_sl,fake_resp_buffer.read_out,NULL);
      execute_subscription_callbacks(&fake_ecg_sl,fake_ecg_buffer.read_out,NULL);
    }
    else if (ev == vital_update_event)
    {
      ctimer_set(&vtimer, CLOCK_SECOND*5,current_vitals_update,NULL);
    }
    else if (ev == sensors_event && data == &button_sensor)
    {
      report_mode++;
      printf("Report Mode %d\n", report_mode);
      clear_subscriptions(&record_sl);

      if (report_mode == 1 )
      {
        //send to aaaa::1
        subscription_data_t sink = {{0}};
        uip_ip6addr(&sink_addr, 0xabcd, 0, 0, 0, 0xba27, 0xebff, 0xfe79, 0xaf4b);
	//uip_ip6addr(&sink_addr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
        memcpy(&(sink), &sink_addr,sizeof(uip_ipaddr_t));
        create_subscription(&record_sl,0,0,vc_send,sink);
      }
      else if (report_mode ==2 )
      {
        //return to default request only mode
        report_mode = 0;
      }

    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
