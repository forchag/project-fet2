from crt_decode import MODULI, encode
from lora_rx import LoRaReceiver


def test_lora_reassembly_discards_third_residue():
    receiver = LoRaReceiver(clock=lambda: 0)
    residues = encode(4321)
    assert receiver.process_packet({"device_id": "s1", "reading_id": "r1", "modulus": MODULI[0], "residue": residues[MODULI[0]], "zone": "North"}) is None
    reading = receiver.process_packet({"device_id": "s1", "reading_id": "r1", "modulus": MODULI[1], "residue": residues[MODULI[1]], "zone": "North"})
    assert reading is not None
    assert reading.value == 4321
    assert reading.device_id == "s1"
    assert receiver.process_packet({"device_id": "s1", "reading_id": "r1", "modulus": MODULI[2], "residue": residues[MODULI[2]]}) is None


def test_lora_incomplete_timeout():
    now = [0.0]
    receiver = LoRaReceiver(clock=lambda: now[0], timeout_seconds=60)
    residues = encode(12)
    receiver.process_packet({"device_id": "s1", "reading_id": "r2", "modulus": MODULI[0], "residue": residues[MODULI[0]]})
    now[0] = 61.0
    receiver.expire_incomplete()
    assert receiver.process_packet({"device_id": "s1", "reading_id": "r2", "modulus": MODULI[1], "residue": residues[MODULI[1]]}) is None
