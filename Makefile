all: mock_ripple_device
PROJECT_SOURCEFILES += frame_buffer.c
PROJECT_SOURCEFILES += frame_subscription.c
PROJECT_SOURCEFILES += dummy_ecg.c

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

ifeq ($(TARGET),econotag)
  CFLAGS+= -DREAL_SENSORS
endif

#CONTIKI=/home/user/contiki
include $(CONTIKI)/Makefile.include
