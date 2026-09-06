#include "crt_encode.h"

#include <stddef.h>

_Static_assert(CRT_MODULUS_0 == 253u, "unexpected CRT modulus 0");
_Static_assert(CRT_MODULUS_1 == 254u, "unexpected CRT modulus 1");
_Static_assert(CRT_MODULUS_2 == 255u, "unexpected CRT modulus 2");
_Static_assert(CRT_MODULUS_0 *CRT_MODULUS_1 *CRT_MODULUS_2 == CRT_MAX_VALUE,
               "unexpected CRT maximum");
_Static_assert(CRT_SAFE_MAX_VALUE == CRT_MODULUS_0 * CRT_MODULUS_1,
               "safe bound must be the smallest pairwise product");
_Static_assert(CRT_MODULUS_2 <= UINT8_MAX, "residue must fit in one byte");

static const uint16_t s_moduli[CRT_RESIDUE_COUNT] = {
    CRT_MODULUS_0,
    CRT_MODULUS_1,
    CRT_MODULUS_2,
};

uint16_t crt_modulus_for_index(uint8_t index) {
  return (index < CRT_RESIDUE_COUNT) ? s_moduli[index] : 0;
}

esp_err_t crt_encode(uint32_t reading,
                     crt_residue_t residues[CRT_RESIDUE_COUNT]) {
  if (residues == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  /* Encoding against CRT_MAX_VALUE would admit values that two residues
   * cannot distinguish: v and v + 64262 share their residues modulo 253 and
   * 254.  Since the transport is designed to survive losing one residue, the
   * encoder must respect the two-of-three bound. */
  if (reading >= CRT_SAFE_MAX_VALUE) {
    return ESP_ERR_INVALID_SIZE;
  }

  for (uint8_t i = 0; i < CRT_RESIDUE_COUNT; ++i) {
    residues[i].index = i;
    residues[i].value = (uint8_t)(reading % s_moduli[i]);
  }
  return ESP_OK;
}

esp_err_t crt_decode(const crt_residue_t residues[2], uint32_t *reading) {
  if (residues == NULL || reading == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  if (residues[0].index >= CRT_RESIDUE_COUNT ||
      residues[1].index >= CRT_RESIDUE_COUNT ||
      residues[0].index == residues[1].index) {
    return ESP_ERR_INVALID_ARG;
  }

  const uint16_t m0 = s_moduli[residues[0].index];
  const uint16_t m1 = s_moduli[residues[1].index];
  if (residues[0].value >= m0 || residues[1].value >= m1) {
    return ESP_ERR_INVALID_ARG;
  }

  for (uint32_t candidate = residues[0].value;
       candidate < (uint32_t)m0 * (uint32_t)m1; candidate += m0) {
    if ((candidate % m1) == residues[1].value) {
      *reading = candidate;
      return ESP_OK;
    }
  }
  return ESP_ERR_NOT_FOUND;
}
