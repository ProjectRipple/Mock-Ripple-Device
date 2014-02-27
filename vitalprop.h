/**
 * \addtogroup vitalprop
 * @{
 */

/**
 * \file
 *         Header file for VitalProp (trickle based DTN for vital sign propogation)
 * \author
 *         Adam Renner
 */

#ifndef __VITALPROP_H__
#define __VITALPROP_H__

#include "vp_list.h"

#include "sys/ctimer.h"
#include "net/queuebuf.h"
#include "simple-udp.h"
#include "string.h"

struct vitalprop_conn;

struct vitalprop_callbacks {
  void (* recv)(struct vitalprop_conn *c);
};

struct vitalprop_conn {
  struct simple_udp_connection cu;
  const struct vitalprop_callbacks *cb;
  struct pt pt;
  struct vp_list vl;
  struct ctimer t;
  clock_time_t interval;
};


void vitalprop_open(struct vitalprop_conn *c, clock_time_t interval,
		  uint16_t port, const struct vitalprop_callbacks *cb);

void vitalprop_close(struct vitalprop_conn *c);

void vitalprop_send(struct vitalprop_conn *c);

void vitalprop_update(struct vitalprop_conn *c, struct ripplecomm_record r);


#endif /* __VITALPROP_H__ */
/** @} */
/** @} */

