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

#include "frame_buffer.h"

void ecg_frame_buffer_init(struct ecg_frame_buffer *fb)
{
  fb->write_in=&(fb->frame1);
  fb->read_out=&(fb->frame2);
  fb->write_in->current = 0;
  fb->read_out->current = 0;
  fb->write_in_seq = 0;
  fb->write_in->seq = 0;
  fb->write_in->timestamp = clock_seconds();
}

void ppg_frame_buffer_init(struct ppg_frame_buffer *fb)
{
  fb->write_in=&(fb->frame1);
  fb->read_out=&(fb->frame2);
  fb->write_in->current = 0;
  fb->read_out->current = 0;
  fb->write_in_seq = 0;
  fb->write_in->seq = 0;
  fb->write_in->timestamp = clock_seconds();
}

void resp_frame_buffer_init(struct resp_frame_buffer *fb)
{
  fb->write_in=&(fb->frame1);
  fb->read_out=&(fb->frame2);
  fb->write_in->current = 0;
  fb->read_out->current = 0;
  fb->write_in_seq = 0;
  fb->write_in->seq = 0;
  fb->write_in->timestamp = clock_seconds();
}

void ecg_frame_buffer_swap(struct ecg_frame_buffer *fb)
{
  struct ecg_frame *temp;
  temp = fb->write_in;
  fb->write_in=fb->read_out;
  fb->read_out=temp;
  fb->write_in_seq++;
  fb->write_in->seq=fb->write_in_seq;
  fb->write_in->timestamp = clock_seconds();
}


void ppg_frame_buffer_swap(struct ppg_frame_buffer *fb)
{
  struct ppg_frame *temp;
  temp = fb->write_in;
  fb->write_in=fb->read_out;
  fb->read_out=temp;
  fb->write_in_seq++;
  fb->write_in->seq=fb->write_in_seq;
  fb->write_in->timestamp = clock_seconds();
}


void resp_frame_buffer_swap(struct resp_frame_buffer *fb)
{
  struct resp_frame *temp;
  temp = fb->write_in;
  fb->write_in=fb->read_out;
  fb->read_out=temp;
  fb->write_in_seq++;
  fb->write_in->seq=fb->write_in_seq;
  fb->write_in->timestamp = clock_seconds();
}
