all: mock_ripple_device
PROJECT_SOURCEFILES += vitalprop.c
PROJECT_SOURCEFILES += vp_list.c
PROJECT_SOURCEFILES += frame_buffer.c
PROJECT_SOURCEFILES += frame_subscription.c

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

#CONTIKI=/home/user/contiki
include $(CONTIKI)/Makefile.include
