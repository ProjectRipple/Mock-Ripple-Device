/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *         Header file for common elements
 * \author
 *         Adam Renner
 */

// rimeaddr changed to linkaddr
//#include "net/rime/rimeaddr.h"
#include "net/linkaddr.h"
#include <stdint.h>

// uip moved to net/ip/uip.h
//#include "net/uip.h"
#include "net/ip/uip.h"

#ifndef RIPPLECOMM_DISPATCH
#define RIPPLECOMM_DISPATCH 0xD2
#endif

#ifndef RIPPLECOMM_VERSION
#define RIPPLECOMM_VERSION 0x10
#endif

#ifndef RIPPLECOMM_VERSION_MIN_COMPAT
#define RIPPLECOMM_VERSION_MIN_COMPAT RIPPLECOMM_VERSION
#endif

#ifndef RIPPLECOMM_VERSION_COMPATIBLE
#define RIPPLECOMM_VERSION_COMPATIBLE(x) (RIPPLECOMM_VERSION == x)
#endif

#ifndef ECG_STREAM_SIZE
#define ECG_STREAM_SIZE ECG_FRAME_CAPACITY
#endif
#ifndef PPG_STREAM_SIZE
#define PPG_STREAM_SIZE PPG_FRAME_CAPACITY
#endif
#ifndef RESP_STREAM_SIZE
#define RESP_STREAM_SIZE RESP_FRAME_CAPACITY
#endif

enum RIPPLECOMM_MSG_TYPES
{
  VITALUCAST_REQUEST = RIPPLECOMM_VERSION,
  VITALUCAST_RECORD,
  VITALPROP_RECORDS,
  ECG_STREAM_REQUEST,
  ECG_STREAM,
  RESP_STREAM_REQUEST,
  RESP_STREAM,
  PPG_STREAM_REQUEST,
  PPG_STREAM,
  SET_TRIAGE,
  SET_TRIAGE_VERIFY,
  PUT_FILE,
  PUT_FILE_VERIFY,
  GET_FILE,
  GET_FILE_VERIFY
};

struct ripplecomm_header
{
  uint8_t r_dispatch;
  uint8_t r_msg_type;// 4bits version, 4 bits message type
};

struct ripplecomm_record
{
  //rimeaddr_t record_addr;
  linkaddr_t record_addr;
  uip_ipaddr_t device_ipv6;
  uint16_t r_seqid;
  uint8_t heart_rate;
  uint8_t spo2;
  uint8_t bpm;
  uint16_t temperature;
  uint16_t device_status;//for storing battery or other data TBD
};

struct ripplecomm_message
{
  struct ripplecomm_header r_header;
  struct ripplecomm_record r_record;
};

//Allowing any IP to request a stream from any device can be exploited easily
//Perhaps the App should limit this by having a preset list of allowable sinks.
//or by requiring the request to come through a broker for authentication
struct ripplecomm_s_req
{
  struct ripplecomm_header r_header;
  uip_ipaddr_t r_sink;//address to send to
  uint8_t r_expiration;
};

struct ripplecomm_ecg
{
  struct ripplecomm_header r_header;
  uint16_t blank;
  uint32_t seq;
  uint16_t store[ECG_STREAM_SIZE];
};

struct ripplecomm_ppg
{
  struct ripplecomm_header r_header;
  uint32_t seq;
  uint16_t store[PPG_STREAM_SIZE];
};

struct ripplecomm_resp
{
  struct ripplecomm_header r_header;
  uint32_t seq;
  uint16_t store[RESP_STREAM_SIZE];
};

