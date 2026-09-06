"""The gateway must verify the bytes the firmware actually signed.

An earlier revision verified an ASCII rendering of the decoded value while the
firmware signed a packed binary struct, so verification could never succeed and
therefore never constrained anything.  These tests pin the two together.

A second, previously undetected mismatch lived one layer deeper: the firmware
(esp32/main/signing.c) SHA-256-hashes the struct and signs the 32-byte digest
with Ed25519, but the gateway verified the Ed25519 signature over the raw
struct directly.  A signature computed the way this test file's own helper
below now does -- SHA-256 first, then sign the digest, matching the firmware
exactly -- used to fail verification here even though the message-layout fix
above was correct.  ``firmware_sign`` exists so every test in this file
produces a genuine firmware-style signature rather than one that happens to
round-trip through this file's own (previously mismatched) assumptions.
"""

import hashlib
import struct

import pytest
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ed25519

from gateway import READING_SOIL_SHIFT, READING_TEMP_BUCKETS, _signature_payload
from lora_rx import SensorReading


def firmware_payload(device_id, reading_id, soil_raw, temp_centi, timestamp):
    """Byte-for-byte what esp32/main/signing.c signs."""
    return struct.pack("<IHHhq", device_id, reading_id, soil_raw,
                       temp_centi, timestamp)


def firmware_pack(soil_raw, temp_centi):
    """Byte-for-byte what esp32/main/main.c residue-encodes."""
    bucket = max(0, min(READING_TEMP_BUCKETS - 1, (temp_centi + 5500) // 100))
    return ((soil_raw >> READING_SOIL_SHIFT) * READING_TEMP_BUCKETS) + bucket


def reading_for(value, device_id=1234, reading_id=7, timestamp=99,
                signature=None):
    return SensorReading(device_id=str(device_id), reading_id=str(reading_id),
                         value=value, zone="North", timestamp=timestamp,
                         signature=signature)


def firmware_sign(key: ed25519.Ed25519PrivateKey, reading: SensorReading) -> bytes:
    """Sign exactly as esp32/main/signing.c does: SHA-256 the payload, then
    Ed25519-sign the 32-byte digest -- not the payload itself."""
    digest = hashlib.sha256(_signature_payload(reading)).digest()
    return key.sign(digest)


@pytest.mark.parametrize("soil_raw", [0, 1024, 2048, 4095])
@pytest.mark.parametrize("temp_centi", [-500, 0, 2500, 3950])
def test_gateway_reconstructs_the_signed_bytes(soil_raw, temp_centi):
    quantised_soil = (soil_raw >> READING_SOIL_SHIFT) << READING_SOIL_SHIFT
    quantised_temp = ((temp_centi + 5500) // 100) * 100 - 5500
    packed = firmware_pack(soil_raw, temp_centi)

    expected = firmware_payload(1234, 7, quantised_soil, quantised_temp, 99)
    assert _signature_payload(reading_for(packed)) == expected


def test_misdecoded_value_fails_verification():
    """A value one ambiguity period away must not verify.

    This is the property the deployment lacked: the residue transport could
    hand the gateway a wrong value and nothing downstream noticed.
    """
    from crt_decode import SAFE_MAX_VALUE
    from gateway import verify_signature

    key = ed25519.Ed25519PrivateKey.generate()
    public_key = key.public_key().public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )

    unsigned = reading_for(firmware_pack(2048, 2500))
    signature = firmware_sign(key, unsigned)

    good = reading_for(unsigned.value, signature=signature)
    wrong = reading_for(unsigned.value + SAFE_MAX_VALUE, signature=signature)

    assert _signature_payload(good) != _signature_payload(wrong)
    assert verify_signature(good, public_key) is True
    assert verify_signature(wrong, public_key) is False


def test_unsigned_readings_are_rejected_by_default():
    from gateway import verify_signature

    unsigned = reading_for(firmware_pack(1024, 2000))
    assert verify_signature(unsigned) is False
    assert verify_signature(unsigned, allow_unsigned=True) is True


# Canonical test vector (fixed seed, reproducible), also given in the
# manuscript's supplement so an independent reimplementation in another
# language can check byte-for-byte agreement without running this file.
_VECTOR_SEED = bytes(range(32))
_VECTOR_PUBKEY = bytes.fromhex(
    "03a107bff3ce10be1d70dd18e74bc09967e4d6309ba50d5f1ddc8664125531b8")
_VECTOR_PAYLOAD = bytes.fromhex("2a00000007000008c4098085746700000000")
_VECTOR_DIGEST = bytes.fromhex(
    "29cf4c16f38f0a5bc8cef0da69ed7c9e4d570579fcd2d347a8d063895b49599a")
_VECTOR_SIGNATURE = bytes.fromhex(
    "5c59e0f3fef48a23c09ae90c1ac638d179e1058569f2469323f43907b4ab8c4"
    "d701931da85a762a019cda7174e5a6e418326990f5b51207119e7bd83f124fc08")


def test_canonical_vector_matches_firmware_format():
    """Fixed byte-for-byte vector: device_id=42, reading_id=7,
    soil_raw=2048, temperature=25.00 C, timestamp=1735689600 (2025-01-01T00:00:00Z).
    Regenerating this vector from a from-scratch reimplementation of the
    firmware's pack-hash-sign path and getting the same digest and signature
    is the interoperability check Section "Signatures were verified over the
    wrong message" asks for.
    """
    key = ed25519.Ed25519PrivateKey.from_private_bytes(_VECTOR_SEED)
    assert key.public_key().public_bytes(
        serialization.Encoding.Raw, serialization.PublicFormat.Raw
    ) == _VECTOR_PUBKEY

    reading = reading_for(firmware_pack(2048, 2500), device_id=42, reading_id=7,
                         timestamp=1735689600)
    payload = _signature_payload(reading)
    assert payload == _VECTOR_PAYLOAD
    import hashlib as _hashlib
    assert _hashlib.sha256(payload).digest() == _VECTOR_DIGEST

    key.public_key().verify(_VECTOR_SIGNATURE, _VECTOR_DIGEST)  # raises if wrong

    signed = reading_for(reading.value, device_id=42, reading_id=7,
                        timestamp=1735689600, signature=_VECTOR_SIGNATURE.hex())
    pub_pem = key.public_key().public_bytes(
        serialization.Encoding.PEM, serialization.PublicFormat.SubjectPublicKeyInfo)
    from gateway import verify_signature
    assert verify_signature(signed, pub_pem) is True


def test_canonical_vector_rejects_single_byte_modification():
    key = ed25519.Ed25519PrivateKey.from_private_bytes(_VECTOR_SEED)
    pub_pem = key.public_key().public_bytes(
        serialization.Encoding.PEM, serialization.PublicFormat.SubjectPublicKeyInfo)
    from gateway import verify_signature

    tampered = reading_for(firmware_pack(2048, 2500) + 1, device_id=42,
                          reading_id=7, timestamp=1735689600,
                          signature=_VECTOR_SIGNATURE.hex())
    assert verify_signature(tampered, pub_pem) is False
