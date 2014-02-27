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

#include "net/rime/rimeaddr.h"
#include <stdint.h>

#include "net/uip.h"

#ifndef RIPPLECOMM_DISPATCH
#define RIPPLECOMM_DISPATCH 0xD2
#endif

#ifndef RIPPLECOMM_VERSION
#define RIPPLECOMM_VERSION 1
#endif

#ifndef RIPPLECOMM_VERSION_MIN_COMPAT
#define RIPPLECOMM_VERSION_MIN_COMPAT 1
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
  VITALUCAST_REQUEST,
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
  uint8_t r_version : 4;
  uint8_t r_msg_type : 4;
};

struct ripplecomm_record
{
  rimeaddr_t record_addr;
  uint16_t r_seqid;
  //struct split_byte age_hops;//age in interval counts, hops in hop counts
  uint8_t r_est_age;//age in interval counts
  uint8_t r_hops;//hop counts
  uint8_t heart_rate;
  uint8_t spo2;
  uint8_t bpm;
  uint8_t temperature;
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

