# LoRa Secure Communication using Tree Parity Machine (TPM) Synchronization

## Overview

This project demonstrates secure wireless communication between two ESP32 nodes using LoRa without exchanging the encryption key over the communication channel.

Instead of transmitting a secret key, both nodes independently synchronize identical **Tree Parity Machines (TPMs)** through mutual learning. Once synchronization is achieved, both devices derive the same 256-bit cryptographic key using the SHA-256 hash of the synchronized TPM weights. This key is then used to encrypt and decrypt messages transmitted over LoRa.

The project combines concepts from:
- Machine Learning
- Neural Cryptography
- Embedded Systems
- LoRa Wireless Communication
- Cryptographic Hash Functions

---

## Features

- Secure key generation without key exchange
- Tree Parity Machine synchronization
- LoRa communication at 433 MHz
- Automatic synchronization detection
- SHA-256 based key derivation
- XOR-based message encryption/decryption
- Real-time serial monitoring
- Lightweight implementation suitable for ESP32

---

## Hardware Requirements

- 2 × ESP32 Development Boards
- 2 × SX1278 LoRa Modules (433 MHz)
- Jumper wires
- USB cables

---

## Software Requirements

- Arduino IDE
- ESP32 Board Package
- LoRa Library
- SPI Library
- mbedTLS (included with ESP32 Arduino Core)

---

## Pin Connections

| SX1278 | ESP32 |
|---------|--------|
| NSS | GPIO 5 |
| RESET | GPIO 4 |
| DIO0 | GPIO 2 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| SCK | GPIO 18 |
| VCC | 3.3V |
| GND | GND |

---

## Project Structure

```
.
├── Node_A.ino        // Initiates synchronization and sends encrypted messages
├── Node_B.ino        // Responds during synchronization and decrypts messages
└── README.md
```

---

## Working Principle

### Phase 1 – TPM Initialization

Both ESP32 nodes initialize their Tree Parity Machines with random weights.

Parameters used:

- K = 3 Hidden Neurons
- N = 16 Inputs per Neuron
- L = 3 Weight Range (-3 to +3)

Initially, the weight matrices of both nodes are completely different.

---

### Phase 2 – Mutual Learning

For every synchronization round:

1. A deterministic random input vector is generated using the current round number.
2. Both TPMs compute their output (τ).
3. One node transmits its τ over LoRa.
4. The other node compares it with its own τ.
5. If both outputs match, both TPMs update their weights using the Hebbian learning rule.
6. This process repeats until synchronization is achieved.

No secret weights are transmitted during this process.

Only a single output bit (τ) is exchanged each round.

---

### Phase 3 – Synchronization

When more than 50 consecutive matching outputs are obtained, synchronization is considered successful.

Both devices now possess identical weight matrices.

A synchronization confirmation packet is then transmitted.

---

### Phase 4 – Shared Key Generation

After synchronization, both devices compute:

```
Shared Key = SHA256(TPM Weights)
```

Since the weights are identical, both devices independently generate the exact same 256-bit key.

The key is **never transmitted** over LoRa.

---

### Phase 5 – Secure Communication

Messages entered through the serial monitor are encrypted using XOR encryption:

```
Cipher = Plaintext XOR SharedKey
```

The receiver performs:

```
Plaintext = Cipher XOR SharedKey
```

to recover the original message.

---

## Communication Protocol

Three packet types are used.

| Packet | Value | Purpose |
|---------|-------|----------|
| MSG_TAU | 0x01 | Exchange TPM outputs |
| MSG_SYNC_DONE | 0x02 | Indicates synchronization complete |
| MSG_DATA | 0x03 | Encrypted user data |

---

## Synchronization Flow

```
ESP32 A                     ESP32 B

Generate Input
      │
Compute τ
      │
------ τ -------------->
                    Compute τ
                    Compare τ
                    Update Weights
<------ τ --------------
Update Weights

Repeat...

Synchronization

Generate SHA256 Key

Secure Communication
```

---

## Encryption Example

```
Original Message

HELLO

↓

Shared Key

A4 C1 ...

↓

Encrypted Data

EC 84 ...

↓

Transmit over LoRa

↓

Receiver XORs using same key

↓

HELLO
```

---

## Example Output

```
LoRa ready

TPM initialized

Round 271 local=1 remote=1

Round 272 local=-1 remote=-1

Synchronization likely achieved!

KEY:
8A31D3F24A...

SYNC CONFIRMED → SECURE MODE

Encrypted Sent:
51 2A 8D 19

Received Encrypted:
51 2A 8D 19

Decrypted Message:
Hello
```

---

## Security Concept

Unlike conventional symmetric encryption where a secret key must first be exchanged, this implementation uses **Neural Cryptography**.

Advantages include:

- No transmission of secret keys
- Dynamic key generation
- Different key generated every synchronization
- Lightweight implementation for embedded systems

---

## Limitations

- XOR encryption is used only for demonstration purposes.
- TPM synchronization does not guarantee resistance against advanced cryptographic attacks.
- Suitable for educational and research purposes.
- Not intended for production-grade secure communication.

---

## Future Improvements

- Replace XOR with AES-256 encryption.
- Add message authentication (HMAC).
- Packet acknowledgment and retransmission.
- Support multiple LoRa nodes.
- Secure file transfer over LoRa.
- OLED display for synchronization status.
- Power optimization for battery-operated devices.

---

## Applications

- Secure IoT communication
- Wireless sensor networks
- Industrial monitoring
- Smart agriculture
- Remote telemetry
- Embedded security research
- Neural cryptography demonstrations

---

## Authors

**Shivam Gupta**

B.Tech Information Technology  
Maharaja Agrasen Institute of Technology (MAIT)

---

## License
This project is developed for educational and research purposes.
