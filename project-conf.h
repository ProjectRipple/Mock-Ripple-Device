/*
 * Copyright (c) 2010, Loughborough University - Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *
 *
 * \author
 *
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

//when streaming, the number of frames that will be sent per second
#define STREAM_FRAMES_PER_SECOND 2
//the number of 16 bit samples in each frame
#define ECG_FRAME_CAPACITY 100
#define PPG_FRAME_CAPACITY 15
#define RESP_FRAME_CAPACITY 15
//Note: the desire frames per second should be the product of frame capacity and frames per second

#define  VP_LIST_SIZE 20
#define  VP_MSG_RCD_CNT 9

//turn off TCP in order to reduce ROM/RAM
#define UIP_CONF_TCP 0

//Ports for UDP
#define UDP_PORT 5688
#define UDP_PORT2 5689
//Channels if using Rime
#define VITALUCAST_CHANNEL 225
#define VITALUCAST_CHANNEL2 226

//ripplecomm messages are identified by a dispatch byte value and a version in the header
#define RIPPLECOMM_DISPATCH 0xD2

#define RIPPLECOMM_VERSION 0
#define RIPPLECOMM_VERSION_COMPATIBLE(x) (RIPPLECOMM_VERSION == x)

#ifdef UIP_CONF_BUFFER_SIZE
#undef UIP_CONF_BUFFER_SIZE
#endif
#define UIP_CONF_BUFFER_SIZE    350

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  200
#endif

#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif
#ifdef NETSTACK_CONF_MAC
#undef NETSTACK_CONF_MAC
#endif

#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver


//#define NODE_TYPE_COLLECTOR 1

//#define NETSTACK_CONF_NETWORK rime_driver
//#define NETSTACK_CONF_MAC     csma_driver
//#define NETSTACK_CONF_MAC     nullmac_driver
//#define NETSTACK_CONF_RDC     sicslowmac_driver
//#define NETSTACK_CONF_RDC     nullrdc_driver
//#define NETSTACK_CONF_RADIO   contiki_maca_driver
//#define NETSTACK_CONF_RADIO   cc2420_driver
//#define NETSTACK_CONF_FRAMER  framer_nullmac
#endif /* PROJECT_CONF_H_ */
