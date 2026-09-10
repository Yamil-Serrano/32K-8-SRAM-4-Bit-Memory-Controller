# 32K x 8 SRAM 4-Bit Memory Controller

A memory controller module that interfaces a 4-bit CPU with a standard 8-bit SRAM chip, enabling individual nibble read and write operations without wasting the unused half of each memory byte. The design implements a transparent read-modify-write cycle within a single clock period using discrete 74HC-series logic, making it suitable for breadboard prototyping and custom PCB integration.

## Overview

This RAM module bridges the gap between 4-bit data paths and 8-bit memory organization. While the SRAM stores 8-bit bytes, the CPU operates on 4-bit nibbles. This controller allows the CPU to access either the upper or lower nibble of any byte independently, treating the memory space as individually addressable nibble locations when combined with the appropriate control signals.

The core challenge addressed by this design is the inability of standard SRAM to perform partial-byte writes. A 4-bit CPU attempting to modify only four bits of an eight-bit memory location would normally corrupt the adjacent nibble. This module solves this problem by performing an automatic read-modify-write operation entirely within a single clock cycle, completely transparent to the programmer.

## Architecture

The memory module consists of two primary data paths: a read path that delivers the selected nibble to the CPU, and a write path that reconstructs a complete byte from the old byte and the new nibble before writing it back to SRAM.

### Read Path

During read operations, the SRAM outputs a full byte through the 74HC245 bidirectional transceiver. That byte is then fed into a 74HC157 2:1 multiplexer, which selects either the upper or lower nibble based on the SEL signal and presents the chosen nibble to the CPU's 4-bit data bus.

### Write Path

Write operations reconstruct a full byte before it ever reaches the SRAM. First, the existing byte is read out of SRAM through the 74HC245 and captured by a 74HC573 transparent latch. That latched byte is then combined with the new nibble coming from the CPU through two 74HC257 multiplexers, which merge the old byte and the new nibble into a complete byte. Finally, that reconstructed byte is driven back through the 74HC245, now switched to the opposite direction, and written into SRAM. The whole sequence is coordinated by the clock signal, which splits the operation into a rising-edge phase for reading the old byte and a falling-edge phase for writing the modified byte back.

## Schematic

<img width="4961" height="3508" alt="32K×8 SRAM 4-Bit Memory Controller" src="https://github.com/user-attachments/assets/0a630646-5ade-4a86-b956-91b0a51d4e1a" />


## Components Used

| IC | Function | Quantity | Purpose |
|---|---|---|---|
| CY62256LL-70 | SRAM | 1 | 32K x 8 memory storage |
| 74HC245 | Octal Bus Transceiver | 1 | Bidirectional data bus control |
| 74HC573 | Octal Transparent Latch | 1 | Captures and holds old byte during write operations |
| 74HC257 | Quad 2:1 Multiplexer | 2 | Byte reconstruction from old byte and new nibble |
| 74HC157 | Quad 2:1 Multiplexer | 1 | Nibble selection during read operations |
| 74HC00 | Quad NAND Gate | 1 | Control signal generation |

## Specifications

### Memory Characteristics

- **Memory Device:** CY62256LL-70 (32K x 8 SRAM)
- **Address Lines:** All 15 address pins (A0-A14) are broken out and usable, giving access to the full 32K x 8 SRAM; each pin has a pull-down resistor, so any unconnected pin defaults to 0
- **Organization:** 8-bit bytes, each containing two 4-bit nibbles

### Interface Signals

| Signal | Direction | Description |
|---|---|---|
| SW | Input | Write enable (1 = write, 0 = read) |
| CLK | Input | System clock, controls phase timing |
| SEL | Input | Nibble select (0 = low nibble, 1 = high nibble) |
| ADDR | Input | Memory address (up to 15 lines, A0-A14) |

### Performance

| Parameter | Value |
|---|---|
| Access Time (Read) | 88 ns typical, 98 ns worst case |
| Write Cycle Time | 90 ns typical, 98 ns worst case |
| Theoretical Maximum Frequency | 10-11 MHz |
| Practical Conservative Target | 8-10 MHz |
| Validated Frequency | 1 MHz (Arduino Mega test platform) |

## Control Logic

The 74HC00 generates all required control signals from the SW and CLK inputs. The core logic equations are:

```
NAND1 = NOT(CLK)
NAND2 = NOT(SW AND NAND1)
CTRL = NAND2
DIR = NOT(CTRL)
```

The CTRL signal controls SRAM write enable, latch enable, and multiplexer output enable. The DIR signal controls the direction of the bidirectional transceiver. On the clock's rising edge, CTRL is high and DIR is low: the SRAM stays in read mode, the 74HC245 points from SRAM to the circuit, and the 74HC573 latch is transparent, capturing the old byte. On the falling edge, CTRL goes low and DIR goes high: the SRAM switches to write mode, the 74HC245 reverses direction toward the SRAM, the latch holds the captured byte, and the 74HC257 multiplexers become active to merge in the new nibble.

## Operation Details

### Write Operation (SW = 1)

When the CPU initiates a write operation, the module executes the following sequence within a single clock cycle:

**Phase 1: Rising Edge - Read Old Byte**
- SRAM remains in read mode (WE = 1)
- 74HC245 configured for SRAM -> Circuit direction
- 74HC573 latch is transparent and captures the full byte from SRAM

**Phase 2: Falling Edge - Write Modified Byte**
- SRAM transitions to write mode (WE = 0)
- 74HC245 configured for Circuit -> SRAM direction
- 74HC573 holds the old byte value
- 74HC257 multiplexers reconstruct the new byte

### Read Operation (SW = 0)

Read operations are straightforward and do not require the read-modify-write process:

1. SRAM outputs the full byte to the 74HC245
2. 74HC245 passes the byte to the 74HC157 multiplexer
3. The multiplexer selects either the high or low nibble based on SEL
4. The selected nibble appears on the CPU data bus

## Verification

The module was validated using a firmware written for an Arduino Mega, which exercised the module's read and write logic to confirm it was working correctly. The test fills the entire RAM with `1111` as the main test pattern, then fills it again with `0000` as a cleanup step.

## Educational Value

This project demonstrates several important concepts in digital design:

- **Memory Interfacing:** Connecting a CPU to memory with different data widths
- **Read-Modify-Write:** Performing atomic operations on memory
- **Transparent Hardware:** Implementing complex operations inside a single clock cycle
- **Bidirectional Data Buses:** Managing data flow direction on shared buses
- **Timing Analysis:** Understanding propagation delays and critical paths
- **Signal Generation:** Deriving multiple control signals from basic logic gates

## Known Limitations

- **Maximum Frequency:** Ripple-through propagation limits top speed to approximately 10 MHz
- **No Wait States:** The module assumes the CPU can complete the operation in one cycle

## License

Licensed under the Solderpad Hardware License v2.1.
See the LICENSE file for full terms.

## Contact

If you have any questions or suggestions, feel free to reach out:

- GitHub: [Neowizen](https://github.com/Neowizen)
