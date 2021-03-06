/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

#ifndef __PROJECT_ROUTER_CONF_H__
#define __PROJECT_ROUTER_CONF_H__

//forward a collection port to the tun interface
#define COLLECTION_UDP_PORT 5689
#define FORWARDING_UDP_PORT 5690

//define the TUN address
#define TUN_ADD0 0xaaaa
#define TUN_ADD1 0
#define TUN_ADD2 0
#define TUN_ADD3 0
#define TUN_ADD4 0
#define TUN_ADD5 0
#define TUN_ADD6 0
#define TUN_ADD7 1


#ifndef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#endif

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4
#endif

#ifdef UIP_CONF_BUFFER_SIZE
#undef UIP_CONF_BUFFER_SIZE
#endif
#define UIP_CONF_BUFFER_SIZE		400

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  200
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif


#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif
#ifdef NETSTACK_CONF_MAC
#undef NETSTACK_CONF_MAC
#endif

#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver


#endif /* __PROJECT_ROUTER_CONF_H__ */
