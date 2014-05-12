/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *         Header file for simulating an ECG
 * \author
 *         Adam Renner
 */
#define DUMMY_ECG_SIZE 207;
static uint16_t dummy_ecg_array[DUMMY_ECG_SIZE];
static uint16_t *dummy_ecg_ptr;
static uint16_t *dummy_ecg_head;
static uint16_t *dummy_ecg_tail;
uint16_t dummy_ecg_next();
