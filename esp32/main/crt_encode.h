#pragma once

#include <stdint.h>

#include "esp_err.h"

/* Pairwise-coprime one-byte moduli chosen to maximise the smallest pairwise
 * product, since that product, not the product of all three, is what bounds
 * recovery from an arbitrary pair. */
#define CRT_MODULUS_0 253u
#define CRT_MODULUS_1 254u
#define CRT_MODULUS_2 255u
#define CRT_RESIDUE_COUNT 3u
/* Product of the three moduli.  Recovering a value from all three residues is
 * unique below this bound. */
#define CRT_MAX_VALUE 16386810u
/* Recovery from ANY TWO residues is only unique below the smallest pairwise
 * product, min(253*254, 253*255, 254*255) = 64262.  Because the transport
 * tolerates the loss of one residue, this is the bound the encoder must
 * enforce, not CRT_MAX_VALUE. */
#define CRT_SAFE_MAX_VALUE 64262u

typedef struct {
  uint8_t index;
  uint8_t value;
} crt_residue_t;

esp_err_t crt_encode(uint32_t reading,
                     crt_residue_t residues[CRT_RESIDUE_COUNT]);
esp_err_t crt_decode(const crt_residue_t residues[2], uint32_t *reading);
uint16_t crt_modulus_for_index(uint8_t index);
