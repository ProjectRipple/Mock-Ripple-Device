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

#if WITH_UIP6
#include "simple-udp.h"
#include "string.h"
#else
#include "net/rime/broadcast.h"
#endif//WITH_UIP6

struct vitalprop_conn;

struct vitalprop_callbacks {
  void (* recv)(struct vitalprop_conn *c);
};

struct vitalprop_conn {
#if WITH_UIP6
  struct simple_udp_connection cu;
#else
  struct broadcast_conn c;
#endif//WITH_UIP6
  const struct vitalprop_callbacks *cb;
  struct pt pt;
  struct vp_list vl;
  struct ctimer t;
  clock_time_t interval;
};

#if WITH_UIP6
void vitalprop_open(struct vitalprop_conn *c, clock_time_t interval,
		  uint16_t port, const struct vitalprop_callbacks *cb);
#else
void vitalprop_open(struct vitalprop_conn *c, clock_time_t interval,
		  uint16_t channel, const struct vitalprop_callbacks *cb);
#endif//WITH_UIP6

void vitalprop_close(struct vitalprop_conn *c);

void vitalprop_send(struct vitalprop_conn *c);

void vitalprop_update(struct vitalprop_conn *c, struct ripplecomm_record r);


#endif /* __VITALPROP_H__ */
/** @} */
/** @} */

