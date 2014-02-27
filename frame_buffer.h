/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *         Header file for creating frames and buffering streaming
 *        sensor data
 * \author
 *         Adam Renner
 */

#include <sys/clock.h>

/*The capacity of the buffers in number of samples (less than 256)*/
#ifndef ECG_FRAME_CAPACITY
#define ECG_FRAME_CAPACITY 125
#endif
#ifndef PPG_FRAME_CAPACITY
#define PPG_FRAME_CAPACITY 15
#endif
#ifndef RESP_FRAME_CAPACITY
#define RESP_FRAME_CAPACITY 15
#endif

struct ecg_frame
{
  uint32_t timestamp;
  uint32_t seq;
  uint16_t current;//index of the current array value prior to writing
  uint16_t store[ECG_FRAME_CAPACITY];
};

struct ppg_frame
{
  uint32_t timestamp;
  uint32_t seq;
  uint16_t current;
  uint16_t store[PPG_FRAME_CAPACITY];
};

struct resp_frame
{
  uint32_t timestamp;
  uint32_t seq;
  uint16_t current;
  uint16_t store[RESP_FRAME_CAPACITY];
};

struct ecg_frame_buffer
{
  struct ecg_frame frame1;
  struct ecg_frame frame2;
  struct ecg_frame *write_in;
  struct ecg_frame *read_out;
  uint32_t write_in_seq;
};

struct ppg_frame_buffer
{
  struct ppg_frame frame1;
  struct ppg_frame frame2;
  struct ppg_frame *write_in;
  struct ppg_frame *read_out;
  uint32_t write_in_seq;
};

struct resp_frame_buffer
{
  struct resp_frame frame1;
  struct resp_frame frame2;
  struct resp_frame *write_in;
  struct resp_frame *read_out;
  uint32_t write_in_seq;
};

void ecg_frame_buffer_init(struct ecg_frame_buffer *fb);
void ppg_frame_buffer_init(struct ppg_frame_buffer *fb);
void resp_frame_buffer_init(struct resp_frame_buffer *fb);

void ecg_frame_buffer_swap(struct ecg_frame_buffer *fb);
void ppg_frame_buffer_swap(struct ppg_frame_buffer *fb);
void resp_frame_buffer_swap(struct resp_frame_buffer *fb);
