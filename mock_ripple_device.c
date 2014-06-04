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
#include "net/rime.h"

#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "simple-udp.h"

#include "mqtt-sn.h"

//buffers could probably be declared volatile since an interrupt
//service routine or
//other code will likely be making changes to it in real hardware.
static struct ripplecomm_record current_vitals = {{0}};
static struct resp_frame_buffer fake_resp_buffer;
static struct ecg_frame_buffer fake_ecg_buffer;
static struct subscription_list fake_resp_sl;
static struct subscription_list fake_ecg_sl;
static struct subscription_list record_sl;

static struct mqtt_sn_connection mqtt_sn_c;
static uip_ipaddr_t sink_addr;
static char device_id[] = "0000000000000000";//64 bit unique id - 16 character hex
static char pub_topic[] = "P_Stats/0000000000000000/vitalcast";
static uint16_t publisher_topic_id;
static uint16_t reg_topic_msg_id;
static mqtt_sn_register_request regreq;
//uint8_t debug = FALSE;

static enum mqttsn_connection_status connection_state = MQTTSN_DISCONNECTED;

/*A few events for managing device state*/
static process_event_t mqttsn_connack_event;

static struct simple_udp_connection vitalcast_connection;

//Fake Signal Event
static process_event_t buffer_flop_event;
static process_event_t vital_update_event;


PROCESS(example_mqttsn_process, "Configure Connection and Topic Registration");
PROCESS(publish_process, "register topic and publish data");
/*---------------------------------------------------------------------------*/
//subscription callback routine -  UDP unicast data
static void vc_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_message rm;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = VITALUCAST_RECORD;
  memcpy(&(rm.r_record),frame_ptr,sizeof(struct ripplecomm_record));
  rm.r_record.r_seqid = uip_htons(rm.r_record.r_seqid);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_record), (uip_ipaddr_t *)subscription_data);
}

/*---------------------------------------------------------------------------*/
//mqtt-sn callback routine
static void vc_mqttsn(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_message rm;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = VITALUCAST_RECORD;
  memcpy(&(rm.r_record),frame_ptr,sizeof(struct ripplecomm_record));
  rm.r_record.r_seqid = uip_htons(rm.r_record.r_seqid);
  rm.r_record.temperature = uip_htons(rm.r_record.temperature);
  printf("publishing \n ");
  mqtt_sn_send_publish(&mqtt_sn_c, publisher_topic_id,MQTT_SN_TOPIC_TYPE_NORMAL,&(rm.r_record), sizeof(struct ripplecomm_record),MQTT_QOS,MQTT_RETAIN);
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
  rm.seq = uip_htons(rf->seq);
  simple_udp_sendto(&vitalcast_connection, &rm, sizeof(struct ripplecomm_resp), (uip_ipaddr_t *)subscription_data);

}
/*---------------------------------------------------------------------------*/
static void ecg_send(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data)
{
  struct ripplecomm_ecg rm;
  struct ecg_frame *rf = (struct ecg_frame *)frame_ptr;
  rm.r_header.r_dispatch=RIPPLECOMM_DISPATCH;
  rm.r_header.r_msg_type = ECG_STREAM;
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

  if (h.r_dispatch == RIPPLECOMM_DISPATCH)
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

/*---------------------------------------------------------------------------*/
static void
puback_receiver(struct mqtt_sn_connection *mqc, const uip_ipaddr_t *source_addr, const uint8_t *data, uint16_t datalen)
{
  printf("Puback received\n");
}
/*---------------------------------------------------------------------------*/
static void
connack_receiver(struct mqtt_sn_connection *mqc, const uip_ipaddr_t *source_addr, const uint8_t *data, uint16_t datalen)
{
  connack_packet_t incoming_connack;
  memcpy(&incoming_connack, data, datalen);
  printf("Connack received\n");
  if (incoming_connack.return_code == ACCEPTED) {
    process_post(&example_mqttsn_process, mqttsn_connack_event, NULL);
  } else {
    printf("Connack error: %s\n", mqtt_sn_return_code_string(incoming_connack.return_code));
  }
}
/*---------------------------------------------------------------------------*/
static void
regack_receiver(struct mqtt_sn_connection *mqc, const uip_ipaddr_t *source_addr, const uint8_t *data, uint16_t datalen)
{
  regack_packet_t incoming_regack;
  memcpy(&incoming_regack, data, datalen);
  printf("Regack received\n");
  if (incoming_regack.message_id == reg_topic_msg_id) {
    if (incoming_regack.return_code == ACCEPTED) {
      publisher_topic_id = uip_htons(incoming_regack.topic_id);
    } else {
      printf("Regack error: %s\n", mqtt_sn_return_code_string(incoming_regack.return_code));
    }
  }
}
/*---------------------------------------------------------------------------*/
/*Add callbacks here if we make them*/
static const struct mqtt_sn_callbacks mqtt_sn_call = {
  NULL,
  NULL,
  NULL,
  connack_receiver,
  regack_receiver,
  puback_receiver,
  NULL,
  NULL,
  NULL
  };

/*---------------------------------------------------------------------------*/
PROCESS(mock_device_process, "Subscription example");
PROCESS(fake_signal_process, "fake signal");
AUTOSTART_PROCESSES(&mock_device_process);
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

    int i,j;
    for (i=0;i<RESP_FRAME_CAPACITY;i++)
    {
      resp_val += 1;//increase respiration value
      fake_resp_buffer.write_in->store[i]=uip_htons(resp_val);//uip_htons converts to network byte order, usually used before sending
    }
    for (j=0;j<ECG_FRAME_CAPACITY;j++)
    {
      ecg_val = dummy_ecg_next();
      fake_ecg_buffer.write_in->store[j]=uip_htons(ecg_val);//uip_htons converts to network byte order, usually used before sending
    }

    //flop buffer, fire signal
    resp_frame_buffer_swap(&fake_resp_buffer);
    ecg_frame_buffer_swap(&fake_ecg_buffer);
    process_post(&mock_device_process, buffer_flop_event, 0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*this process will publish data at regular intervals*/
PROCESS_THREAD(publish_process, ev, data)
{
  static uint8_t registration_tries;
  static mqtt_sn_register_request *rreq = &regreq;

  PROCESS_BEGIN();

  sprintf(pub_topic,"P_Stats/%s/vitalcast",device_id);
  //memcpy(&(pub_topic[8]),device_id,16);

  printf("registering topic\n");
  registration_tries =0;
  while (registration_tries < MQTT_REQUEST_RETRIES)
  {
    reg_topic_msg_id = mqtt_sn_register_try(rreq,&mqtt_sn_c,pub_topic,MQTT_REPLY_TIMEOUT);
    PROCESS_WAIT_EVENT_UNTIL(mqtt_sn_request_returned(rreq));
    if (mqtt_sn_request_success(rreq)) {
      registration_tries = MQTT_REQUEST_RETRIES;
      printf("registration acked\n");
    }
    else {
      registration_tries++;
      if (rreq->state == MQTTSN_REQUEST_FAILED) {
          printf("Regack error: %s\n", mqtt_sn_return_code_string(rreq->return_code));
      }
    }
  }
  if (mqtt_sn_request_success(rreq)){
    //start topic publishing to topic at regular intervals
    subscription_data_t sink = {{0}};
    create_subscription(&record_sl,0,0,vc_mqttsn,sink);
  } else {
    printf("unable to register topic\n");
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/*this main mqtt process will create connection and register topics*/
/*---------------------------------------------------------------------------*/
static struct ctimer connection_timer;
static process_event_t connection_timeout_event;

static void connection_timer_callback(void *mqc)
{
  process_post(&example_mqttsn_process, connection_timeout_event, NULL);
}

PROCESS_THREAD(example_mqttsn_process, ev, data)
{

  static uint8_t connection_retries = 0;

  PROCESS_BEGIN();

  mqttsn_connack_event = process_alloc_event();

  mqtt_sn_set_debug(0);
  uip_ip6addr(&sink_addr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
  mqtt_sn_create_socket(&mqtt_sn_c,MQTT_UDP_PORT, &sink_addr, MQTT_UDP_PORT);
  (&mqtt_sn_c)->mc = &mqtt_sn_call;

  sprintf(device_id,"%02X%02X%02X%02X%02X%02X%02X%02X",rimeaddr_node_addr.u8[0],
          rimeaddr_node_addr.u8[1],rimeaddr_node_addr.u8[2],rimeaddr_node_addr.u8[3],
          rimeaddr_node_addr.u8[4],rimeaddr_node_addr.u8[5],rimeaddr_node_addr.u8[6],
          rimeaddr_node_addr.u8[7]);

  /*Request a connection and wait for connack*/
  printf("requesting connection \n ");
  connection_timeout_event = process_alloc_event();
  ctimer_set( &connection_timer, MQTT_REPLY_TIMEOUT, connection_timer_callback, NULL);
  mqtt_sn_send_connect(&mqtt_sn_c,device_id,MQTT_KEEP_ALIVE);
  connection_state = MQTTSN_WAITING_CONNACK;
  while (connection_retries < MQTT_REQUEST_RETRIES)
  {
    PROCESS_WAIT_EVENT();
    if (ev == mqttsn_connack_event) {
      //if success
      printf("connection acked\n");
      ctimer_stop(&connection_timer);
      connection_state = MQTTSN_CONNECTED;
      connection_retries = MQTT_REQUEST_RETRIES;//using break here may mess up switch statement of proces
    }
    if (ev == connection_timeout_event) {
      connection_state = MQTTSN_CONNECTION_FAILED;
      connection_retries++;
      printf("connection timeout\n");
      ctimer_restart(&connection_timer);
      if (connection_retries < MQTT_REQUEST_RETRIES) {
        mqtt_sn_send_connect(&mqtt_sn_c,device_id,MQTT_KEEP_ALIVE);
        connection_state = MQTTSN_WAITING_CONNACK;
      }
    }
  }
  ctimer_stop(&connection_timer);
  if (connection_state == MQTTSN_CONNECTED){
    process_start(&publish_process, 0);
    //monitor connection
    while(1)
    {
      PROCESS_WAIT_EVENT();
    }
  } else {
    printf("unable to connect\n");
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
  uip_ipaddr_copy(&(current_vitals.device_ipv6),&uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
  execute_subscription_callbacks(&record_sl,&current_vitals,NULL);
  process_post(&mock_device_process, vital_update_event, 0);
}

PROCESS_THREAD(mock_device_process, ev, data)
{
  static struct ctimer vtimer;
  static struct etimer startup_timer;

  PROCESS_BEGIN();
  mqtt_sn_set_debug(0);

  resp_frame_buffer_init(&fake_resp_buffer);//this needs to come after process begin
  ecg_frame_buffer_init(&fake_ecg_buffer);
  init_subscription_list(&fake_resp_sl);
  init_subscription_list(&fake_ecg_sl);
  init_subscription_list(&record_sl);
  //give node unique id
  current_vitals.record_addr=rimeaddr_node_addr;
  current_vitals.r_seqid=0;
  current_vitals.heart_rate=1;
  current_vitals.temperature = 2;
  //printf("test process started\n");
  buffer_flop_event = process_alloc_event();
  vital_update_event = process_alloc_event();
  //set_global_address();
  uip_ipaddr_copy(&(current_vitals.device_ipv6),&uip_ds6_get_global(ADDR_PREFERRED)->ipaddr);
  simple_udp_register(&vitalcast_connection, UDP_PORT, NULL, UDP_PORT, receiver);

  sprintf(device_id,"%02X%02X%02X%02X%02X%02X%02X%02X",rimeaddr_node_addr.u8[0],
          rimeaddr_node_addr.u8[1],rimeaddr_node_addr.u8[2],rimeaddr_node_addr.u8[3],
          rimeaddr_node_addr.u8[4],rimeaddr_node_addr.u8[5],rimeaddr_node_addr.u8[6],
          rimeaddr_node_addr.u8[7]);

  //start the fake signal (like starting rtimer process)
  process_start(&fake_signal_process, 0);

  SENSORS_ACTIVATE(button_sensor);
  //wait for RPL to provide us with host address
  /*Wait a little to let system get set*/
  etimer_set(&startup_timer, STARTUP_DELAY);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&startup_timer));

  ctimer_set(&vtimer, PERIODIC_DELAY,current_vitals_update,NULL);
  process_start(&example_mqttsn_process, 0);

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
      ctimer_set(&vtimer, PERIODIC_DELAY,current_vitals_update,NULL);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
