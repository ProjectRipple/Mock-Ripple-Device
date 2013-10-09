/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *        dispatch controls collection
 *        and management of streaming buffers
 * \author
 *         Adam Renner
 */

#include frame_buffer.h
#include frame_subscription.h

/*It seems more practical t run all of the
 * devices on a single real time clock and collect
 * all of the data continuously - otherwise, management of the
 * the device could become complicated and it seems better
 *to avoid multiple interrupt routines accessing the SPI bus
 * simultaneously.
 */


struct frame_dispatch
{
  //framebuffer
  //subscription list
};

struct measurement_dispatch
{
  //function to read sensor
  //subscription list
};



//placeholder in case we need our own rtimer implementation
//for the econotag
void frame_rtimer();
