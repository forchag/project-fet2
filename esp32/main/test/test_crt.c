#include "unity.h"

#include "crt_encode.h"

static void assert_round_trip_from_pair(uint32_t reading, uint8_t a,
                                        uint8_t b) {
  crt_residue_t residues[CRT_RESIDUE_COUNT];
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(reading, residues));

  crt_residue_t pair[2] = {residues[a], residues[b]};
  uint32_t decoded = UINT32_MAX;
  TEST_ASSERT_EQUAL(ESP_OK, crt_decode(pair, &decoded));
  TEST_ASSERT_EQUAL_UINT32(reading, decoded);
}

TEST_CASE("CRT encodes residues for all moduli", "[crt]") {
  crt_residue_t residues[CRT_RESIDUE_COUNT];
  const uint32_t reading = 51455; /* packed full scale: soil 255, bucket 200 */
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(reading, residues));
  TEST_ASSERT_EQUAL_UINT8(0, residues[0].index);
  TEST_ASSERT_EQUAL_UINT8(reading % CRT_MODULUS_0, residues[0].value);
  TEST_ASSERT_EQUAL_UINT8(1, residues[1].index);
  TEST_ASSERT_EQUAL_UINT8(reading % CRT_MODULUS_1, residues[1].value);
  TEST_ASSERT_EQUAL_UINT8(2, residues[2].index);
  TEST_ASSERT_EQUAL_UINT8(reading % CRT_MODULUS_2, residues[2].value);
}

TEST_CASE("CRT rejects values two residues cannot disambiguate", "[crt]") {
  crt_residue_t residues[CRT_RESIDUE_COUNT];
  /* The encoder must reject at the two-of-three bound, not at the product of
   * all three moduli: with only two residues, v and v + 9797 are
   * indistinguishable. */
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_SIZE,
                    crt_encode(CRT_SAFE_MAX_VALUE, residues));
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_SIZE, crt_encode(CRT_MAX_VALUE, residues));
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(CRT_SAFE_MAX_VALUE - 1, residues));
}

TEST_CASE("two residues cannot separate values a full period apart", "[crt]") {
  /* Regression test for the domain error: 5 and 5 + 253*254 share residues
   * modulo 253 and 254, so admitting both would let the transport silently
   * decode one as the other. */
  crt_residue_t low[CRT_RESIDUE_COUNT];
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(5, low));
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_SIZE,
                    crt_encode(5 + CRT_MODULUS_0 * CRT_MODULUS_1, low));
}

TEST_CASE("the packed sensor range fits the two-of-three bound", "[crt]") {
  /* The encoder is only sound if every value the sensor path can produce is
   * accepted.  This is the check whose absence let a guard be introduced that
   * rejected most of the ADC range. */
  const uint32_t temp_buckets = 201u;
  const uint32_t max_soil_quantised = 4095u >> 4u;
  const uint32_t max_packed = max_soil_quantised * temp_buckets +
                              (temp_buckets - 1u);
  TEST_ASSERT_LESS_THAN_UINT32(CRT_SAFE_MAX_VALUE, max_packed);

  crt_residue_t residues[CRT_RESIDUE_COUNT];
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(max_packed, residues));
  TEST_ASSERT_EQUAL(ESP_OK, crt_encode(0, residues));
}

TEST_CASE("CRT decodes zero from any two residues", "[crt]") {
  assert_round_trip_from_pair(0, 0, 1);
  assert_round_trip_from_pair(0, 0, 2);
  assert_round_trip_from_pair(0, 1, 2);
}

TEST_CASE("CRT decodes mid-range reading from any two residues", "[crt]") {
  assert_round_trip_from_pair(4095, 0, 1); /* mid-range packed values */
  assert_round_trip_from_pair(4095, 0, 2);
  assert_round_trip_from_pair(4095, 1, 2);
  assert_round_trip_from_pair(51455, 0, 1); /* packed full scale */
  assert_round_trip_from_pair(51455, 1, 2);
}

TEST_CASE("CRT decodes highest supported reading from any two residues",
          "[crt]") {
  assert_round_trip_from_pair(CRT_SAFE_MAX_VALUE - 1, 0, 1);
  assert_round_trip_from_pair(CRT_SAFE_MAX_VALUE - 1, 0, 2);
  assert_round_trip_from_pair(CRT_SAFE_MAX_VALUE - 1, 1, 2);
}

TEST_CASE("CRT rejects invalid decode pairs", "[crt]") {
  uint32_t decoded = 0;
  crt_residue_t duplicate[2] = {{.index = 0, .value = 1},
                                {.index = 0, .value = 2}};
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, crt_decode(duplicate, &decoded));

  crt_residue_t invalid_index[2] = {{.index = 3, .value = 1},
                                    {.index = 1, .value = 2}};
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, crt_decode(invalid_index, &decoded));

  crt_residue_t invalid_value[2] = {{.index = 0, .value = CRT_MODULUS_0},
                                    {.index = 1, .value = 2}};
  TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, crt_decode(invalid_value, &decoded));
}
