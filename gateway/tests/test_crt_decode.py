import pytest

from crt_decode import MODULI, SAFE_MAX_VALUE, decode, encode


def test_crt_round_trip():
    for value in [0, 1, 42, 1234, 9000, 51455, SAFE_MAX_VALUE - 1]:
        assert decode(encode(value)) == value


def test_crt_missing_residue_reconstruction():
    """Any one residue may be lost, which is the point of the scheme."""
    value = 1234
    for dropped in MODULI:
        residues = encode(value)
        residues.pop(dropped)
        assert decode(residues) == value


def test_crt_invalid_input():
    with pytest.raises(ValueError):
        encode(-1)
    with pytest.raises(ValueError):
        # Two residues cannot separate values a full pairwise period apart,
        # so the encoder must reject at SAFE_MAX_VALUE, not at the product of
        # all three moduli.
        encode(SAFE_MAX_VALUE)
    with pytest.raises(ValueError):
        decode({MODULI[0]: 1})
    with pytest.raises(ValueError):
        decode({MODULI[0]: MODULI[0], MODULI[1]: 1})
