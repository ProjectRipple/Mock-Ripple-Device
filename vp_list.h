/**
 * \addtogroup vitalprop
 * @{
 */

/**
 * \file
 *         Header file for vp_list a linked list queue for managin vital sign records
 * \author
 *         Adam Renner
 */

#ifndef __VP_LIST_H__
#define __VP_LIST_H__

#include "common.h"
#include "net/packetbuf.h"

#ifndef  VP_LIST_SIZE
#define  VP_LIST_SIZE 20
#endif

#ifndef  VP_MSG_RCD_CNT
#define  VP_MSG_RCD_CNT 9
#endif


/*#define NODE_TYPE_COLLECTOR*/

/*
 * Rank is determined by the formula R = A*old_data_heard + B*repetitions_over_interval + C*intervals_since_last_send + D
 * where D is determined based on the age of the record in the list
 * Current Setting:
 * A=10;
 * B=2;
 * C=-1;
 * D = 0 if age >= AGE_REL, +3 if age is not
 * AGE_REL =2
 *
 */
static const int rank_A = 20;
static const int rank_B = -2;
static const int rank_C = 3;
static const int rank_D = 6;
static const int age_rel = 3;


/**
 *    VitalProp will differ from trickle in that it will send out a new packet every tau because its own sensor values change
 *    In addition it will include a series of other mote records that it has recieved and stored in an struct with array (vp_list)
 *    A ranking mechanism will be used for the vp_list to determine which packets should be sent at each interval
 *    The catalague will have a defined maximum size. The lowest ranked records are dropped off when the max vp_list size is reached.
 *    Initially this should start as a small circular buffer where the oldest record gets removed when new records are added.
 */

struct ripplecomm_vp_message
{
  struct ripplecomm_header header;
  struct ripplecomm_record records[VP_MSG_RCD_CNT];
};

struct vp_list_element
{
  int rank;
  int old_data_heard;
  int repetitions_over_interval;
  int intervals_since_last_send;
  int in_message;
  struct ripplecomm_record r;
};

struct vp_list
{
    struct vp_list_element e[VP_LIST_SIZE];
};



/**
 * \brief      Initialize the list with zeroized values except the first element which must be the address of this node
 * \param l    A pointer to a struct vp_list
 * \param personal_addr the rime address of this node
 *
 *             This function must be called after creating a vp_list, otherwise the other vp_list functions will not function
 *              correctly
 */
void vp_list_init(struct vp_list *l, rimeaddr_t personal_addr);


/**
 * \brief      Intended to update the data on this node
 * \param l    A pointer to a struct vp_list
 * \param r    a single ripplecomm record
 *
 *             Updates the record for this node. Will fail if the first element of l does not have
 *             the same rime address as intended
 */
void vp_list_update_node(struct vp_list *l, struct ripplecomm_record r);



/**
 * \brief      Intended to update the data on this node
 * \param l    A pointer to a struct vp_list
 *
 *             copies the message data from the vp_list into a message
 */
void vp_list_to_msg(struct vp_list *l, struct ripplecomm_vp_message *m);


/**
 * \brief      Intended to update the data on this node
 * \param l    A pointer to a struct vp_list
 * \param m    The received message to extract record data from
 *
 *             copies the message data to the vp_list from a ripplecomm vp message
 */
void vp_list_from_msg(struct vp_list *l, struct ripplecomm_vp_message m);


/**
 * \brief      Increases the age of all the records by 1
 * \param l    A pointer to a struct vp_list
 *
 *            Typically performed after sending a message
 */
void age_vp_list(struct vp_list *l);


/**
 * \brief      Updates the rank of a single vp_list_element
 * \param e    A pointer to a struct vp_list_element
 *
 *            Typically performed when a new element is added, or when updating all list elements
 */
void update_element_rank(struct vp_list_element *e);


/**
 * \brief      Updates the rank of all vp_list elements
 * \param l    A pointer to a struct vp_list
 *
 *            Update the rank all list elements
 */
void update_all_ranks(struct vp_list *l);


/**
 * \brief      Returns the iterator of the lowestranked record in vp_list
 * \param l    A pointer to a struct vp_list
 *
 *            Returns the iterator of the lowestranked record in vp_list
 */
int lowest_rank_from_vp_list(struct vp_list *l);





#endif /* __VP_LIST_H__ */
/** @} */
/** @} */

