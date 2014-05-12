/**
 * \addtogroup ripplepropagation
 * @{
 */

/**
 * \file
 *         C file for simulating an ECG
 * \author
 *         Adam Renner
 */

 #include "dummy_ecg.h"
static uint16_t dummy_ecg_array[DUMMY_ECG_SIZE] = {15358,
  14665,
  14203,
  13741,
  13395,
  13164,
  12702,
  12702,
  12587,
  12125,
  12125,
  12240,
  12240,
  12009,
  11894,
  12009,
  12125,
  12009,
  11778,
  11778,
  11894,
  12356,
  12125,
  12240,
  12702,
  12471,
  12471,
  12702,
  12933,
  13048,
  12702,
  12702,
  13164,
  13164,
  13048,
  12702,
  13279,
  13164,
  12933,
  12818,
  13048,
  13279,
  12818,
  12471,
  12587,
  12702,
  12702,
  12471,
  12356,
  12471,
  12356,
  12009,
  12125,
  12240,
  12125,
  11778,
  11547,
  11663,
  11547,
  11547,
  11316,
  11316,
  11316,
  11201,
  10855,
  10970,
  11316,
  11085,
  10739,
  10739,
  10855,
  10970,
  10739,
  10855,
  11316,
  11894,
  11778,
  12125,
  12471,
  12587,
  12818,
  12587,
  12933,
  13279,
  13279,
  13048,
  13395,
  13510,
  13279,
  12587,
  12356,
  12240,
  12009,
  12009,
  12356,
  12471,
  11894,
  11085,
  10855,
  10739,
  10393,
  9700,
  9353,
  9469,
  9238,
  8776,
  8891,
  8891,
  9122,
  8891,
  8661,
  9007,
  9007,
  9007,
  8776,
  8891,
  8891,
  8661,
  7968,
  6467,
  5774,
  4965,
  6813,
  10508,
  18591,
  30831,
  42841,
  53464,
  59815,
  59931,
  50808,
  34527,
  18938,
  9469,
  6236,
  6582,
  8891,
  10970,
  10970,
  9815,
  9584,
  9469,
  9238,
  8545,
  8314,
  8199,
  8083,
  7621,
  7621,
  7852,
  7621,
  7275,
  7390,
  7621,
  7506,
  7390,
  7390,
  7275,
  7275,
  7506,
  7159,
  7621,
  7852,
  7737,
  7506,
  8199,
  8199,
  8199,
  8083,
  8545,
  8776,
  9238,
  9007,
  9353,
  10046,
  10393,
  10393,
  10855,
  11663,
  12125,
  12471,
  13279,
  14088,
  14896,
  15242,
  15704,
  16628,
  17321,
  17552,
  18245,
  18938,
  19630,
  19515,
  19861,
  20439,
  20670,
  20785,
  20554,
  20554,
  20439,
  20092,
  19400,
  18938,
  18360,
  17667,
  16513,
  15704};

static uint16_t *dummy_ecg_ptr = &dummy_ecg_array[0];
static uint16_t *dummy_ecg_head = &dummy_ecg_array[0];
static uint16_t *dummy_ecg_tail = &dummy_ecg_array[DUMMY_ECG_SIZE];

uint16_t dummy_ecg_next()
{
  if (dummy_ecg_ptr == dummy_ecg_tail) {
    dummy_ecg_ptr = dummy_ecg_head;
  }
  else {
    dummy_ecg_ptr++;
  }
  return *dummy_ecg_ptr;
}







