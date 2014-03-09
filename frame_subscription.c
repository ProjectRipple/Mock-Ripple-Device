/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *        used by dispatch to manage stream subscriptions
 *
 * \author
 *         Adam Renner
 */

#include <stdlib.h>
#include "contiki-net.h"
#include <string.h>
#include "frame_subscription.h"

MEMB(subslist, struct frame_subscription, TOTAL_SUBSCRIPTION_LIST_LIMIT);

/*remove subscription will remove whatever is pointed to by sl->iter, as long as prev points to previous node
* Careful when using!
*/
void remove_subscription(struct subscription_list *sl,struct frame_subscription *prev);

void remove_subscription(struct subscription_list *sl,struct frame_subscription *prev)
{
  //link previous node or list head to next node
  if (prev==NULL)//if this is first node
  {
    sl->head = (sl->iter)->next;
    memb_free(&subslist, sl->iter);
    sl->iter=sl->head;
  }
  else
  {
    prev->next = (sl->iter)->next;
    memb_free(&subslist, sl->iter);
    sl->iter=prev->next;
  }
  //free memory
  //free(sl->iter);
  //sl->iter = prev;
  return;
}

void init_subscription_list(struct subscription_list *sl)
{
  sl->head=NULL;
  sl->iter=sl->head;
}

void end_subscription(struct subscription_list *sl, frame_subscription_cb_t cb, subscription_data_t subscription_data)
{
  sl->iter =sl->head;
  struct frame_subscription *prev = NULL;//need to keep track of previous to repair list
  while (sl->iter != NULL)
  {
    if (((sl->iter)->cb == cb) && (!memcmp(&(sl->iter->subscription_data), &subscription_data,sizeof(subscription_data_t))))//identify subscription by callback and data
    {
      remove_subscription(sl,prev);
      return;
    }
    else
    {
      prev = sl->iter;
      sl->iter = (sl->iter)->next;
    }
  }
}

void create_subscription(struct subscription_list *sl,uint8_t expires, uint8_t expiration, frame_subscription_cb_t cb, subscription_data_t subscription_data)
{
  sl->iter =sl->head;
  // go through the list to see if subscription already exist or find the last node
  while (sl->iter != NULL)
  {
    if (((sl->iter)->cb == cb) && !memcmp(&(sl->iter->subscription_data), &subscription_data,sizeof(subscription_data_t)))//identify subscription by callback and data memory locations
    {
      //renew subscription
      (sl->iter)->expires = expires;
      (sl->iter)->expiration = expiration;
      return;
    }
    sl->iter = (sl->iter)->next;
  }
  //subscription not found, add new subscription at top of list
  sl->iter = sl->head;
  sl->head = (struct frame_subscription *)memb_alloc(&subslist);
  (sl->head)->next = sl->iter;
  (sl->head)->expires = expires;
  (sl->head)->expiration = expiration;
  (sl->head)->cb=cb;
  (sl->head)->subscription_data=subscription_data;
  return;
}

void find_and_remove_expired_subscriptions(struct subscription_list *sl)
{
  sl->iter =sl->head;
  struct frame_subscription *prev = NULL;//need to keep track of previous to repair list
  while (sl->iter != NULL)
  {
    if (((sl->iter)->expires == 1) && ((sl->iter)->expiration ==0))//identify subscription by callback and data
    {
      remove_subscription(sl,prev);
    }
    else
    {
      prev = sl->iter;
      sl->iter = (sl->iter)->next;
    }
  }
}

void execute_subscription_callbacks(struct subscription_list *sl,void *frame_ptr, void *data_ptr)
{
  sl->iter =sl->head;
  struct frame_subscription *prev = NULL;

  while (sl->iter != NULL)
  {
    sl->iter->cb(frame_ptr,data_ptr, &(sl->iter->subscription_data));
    if (sl->iter->expires == 1)
    {
      if (sl->iter->expiration > 1)
      {
        sl->iter->expiration--;
        prev = sl->iter;
        sl->iter = (sl->iter)->next;
      }
      else
      {
        remove_subscription(sl,prev);
      }
    }
    else
    {
      prev = sl->iter;
      sl->iter = (sl->iter)->next;
    }

  }
}

void clear_subscriptions(struct subscription_list *sl)
{
 sl->iter =sl->head;
struct frame_subscription *prev = NULL;
while (sl->iter != NULL)
  {
    remove_subscription(sl,prev);
    //prev = sl->iter;//unnecessary, prev is not modified in this instance
    //sl->iter = (sl->iter)->next
    //sl->iter =sl->head;
  }
}
