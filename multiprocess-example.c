#include <stdbool.h>
#include <stdio.h>

#include <contiki.h>
#include <lib/assert.h>

PROCESS(process1, "Process 1");
PROCESS(process2, "Process 2");
PROCESS(process3, "Process 3");

AUTOSTART_PROCESSES(&process1, &process2);

static process_event_t event1;
static process_event_t event2;

void shutdown()
{
  printf("Somehow we escaped the endless loop...");
}

PROCESS_THREAD(process1, ev, data)
{
  static struct etimer process1_timer;

  event1 = process_alloc_event();
  event2 = process_alloc_event();

  PROCESS_EXITHANDLER(shutdown());
  PROCESS_BEGIN();

  process_start(&process3, 0);

  etimer_set(&process1_timer, CLOCK_SECOND);

  while(true)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&process1_timer));
    etimer_reset(&process1_timer);

    // run process3 immediately
    assert(process_is_running(&process3));
    process_post_synch(&process3, event1, 0);
  }

  PROCESS_END();
}

PROCESS_THREAD(process2, ev, data)
{
  static struct etimer process2_timer;

  PROCESS_BEGIN();

  printf("process2\n");

  etimer_set(&process2_timer, 5*CLOCK_SECOND);

  while(true)
  {
    PROCESS_YIELD_UNTIL(etimer_expired(&process2_timer));

    etimer_reset(&process2_timer);

    // enqueue process3
    assert(process_is_running(&process3));
    process_post(&process3, event2, 0);
  }

  PROCESS_END();
}

PROCESS_THREAD(process3, ev, data)
{
  PROCESS_BEGIN();

  printf("process3\n");

  while(true)
  {
    PROCESS_WAIT_EVENT();

    if(ev == event1)
    {
      printf("event1\n");
    }
    else if(ev == event2)
    {
      printf("event2\n");
    }
    else
    {
      printf("unknown event\n");
    }
  }

  PROCESS_END();
}
