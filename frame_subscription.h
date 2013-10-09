/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *        used by dispatch to manage stream subscriptions with a linked list
 *
 * \author
 *         Adam Renner
 */
#include <stdint.h>

//Need some sort of flexible datatype that can hold ip addresses, filenames etc.
//ip6 address is 16 bytes
//ip4 address is 4 bytes
//filenames can be 31 characters
typedef union subscription_data_t {
  uint8_t  u8[32];			/* Initializer, must come first. */
  uint16_t u16[16];
  char c8[32];
} subscription_data_t;


//subscriptions are will contain a pointer to a function to be run to execute the subscription
//This allows us to accomodate different type of actions like memory logging and sending
//for the different signals and frames
typedef void (*frame_subscription_cb_t)(void *frame_ptr, void *data_ptr, subscription_data_t *subscription_data);



struct frame_subscription
{
  uint8_t expires;//0=no, 1=yes
  uint8_t expiration;//countdown timer to expiration (256 should be enough cycles between renewals)
  frame_subscription_cb_t cb;
  void *data_ptr;
  subscription_data_t subscription_data;
  struct frame_subscription *next;
};


struct subscription_list
{
  struct frame_subscription *head;//pointer to the first list node
  struct frame_subscription *iter;//for iterating through list nodes
};

void init_subscription_list(struct subscription_list *sl);

/**
 * \brief      Create a subscription for the signals subscription list
 * \param sl    A pointer to a struct subscription list
 * \param expires    1 or 0, is it an expiring subscription
 * \param expiration    countdowner for expiring subscriptions
 * \param cb    callback function to handle the signal frame
 * \param subscription_data  data needed by the callback fct.
 *
 *             This function requires an existing subscription list for the signal frames.
 *            It is called to both create new and renew subscriptions
 *
 */
void create_subscription(struct subscription_list *sl,uint8_t expires, uint8_t expiration, frame_subscription_cb_t cb, subscription_data_t subscription_data);

/**
 * \brief      End or cancel a subscription
 * \param sl    A pointer to a struct subscrpiption list
 * \param cb    callback function to handle the signal frame
 * \param subscription_data  data needed by the callback fct.
 *
 *             This function requires an existing subscription list for the signal frames.
 *            It is called to both create new and renew subscriptions
 *
 */
void end_subscription(struct subscription_list *sl, frame_subscription_cb_t cb, subscription_data_t subscription_data);

/**
 * \brief      Removes any expired subscriptions from the sl - this could potentially be handled prior to dispatching frames
 * \param sl    A pointer to a struct subscription list
 *
 */
void find_and_remove_expired_subscriptions(struct subscription_list *sl);

/**
 * \brief      Execute the callback for each subscription in the list
 * \param sl    A pointer to a struct subscription list
 * \param frame_ptr frame to use in the callback
 * \param data_ptr A pointer to any data that is needed for this frame
 */
void execute_subscription_callbacks(struct subscription_list *sl,void *frame_ptr, void *data_ptr);

/**
 * \brief      Removes all subscriptions from list
 * \param sl    A pointer to a struct subscription list, even non-expiring
 */
void clear_subscriptions(struct subscription_list *sl);
