# Battleship

A terminal-based Battleship game written in C where you place ships on a 10x10 grid and take turns firing at coordinates to sink your opponent's fleet, playable solo against a random-firing CPU or head-to-head over a TCP network connection.

## Building

```bash
gcc -o battleship battleship.c
```

## Running

### Single Player (vs. CPU)

```bash
./battleship
```

The CPU opponent places its ships randomly. You and the CPU alternate turns automatically.

### Two Player (Networked)

One player acts as the server and the other connects as a client. The server takes the first turn.

**Server** (host the game on a port):
```bash
./battleship <port>
# Example:
./battleship 8080
```

**Client** (connect to the server):
```bash
./battleship <server-ip> <port>
# Example:
./battleship 192.168.1.10 8080
```

## Gameplay

### Ship Placement

At the start of the game you place 5 ships on a 10x10 grid. Rows are labeled `A`–`J` and columns `0`–`9`.

| Ship       | Size |
|------------|------|
| Carrier    | 5    |
| Battleship | 4    |
| Cruiser    | 3    |
| Submarine  | 2    |
| Destroyer  | 1    |

**Input formats:**

- **Horizontal** — `C37` places a ship across row C from column 3 to column 7.
- **Vertical** — `CG4` places a ship down column 4 from row C to row G.

Input is case-insensitive. The game rejects placements that go out of bounds or overlap existing ships.

### Taking Shots

Each turn you enter a row letter (`A`–`J`) and a column number (`0`–`9`). Enter `Q` for the row prompt to quit.

### Grid Display

**Your ships grid:**

| Symbol | Meaning             |
|--------|---------------------|
| `.`    | Empty water         |
| `C`    | Carrier             |
| `B`    | Battleship          |
| `R`    | Cruiser             |
| `S`    | Submarine           |
| `D`    | Destroyer           |
| `X`    | Destroyed ship cell |

**Your shots grid:**

| Symbol | Meaning   |
|--------|-----------|
| `.`    | Untried   |
| `H`    | Hit       |
| `M`    | Miss      |

### Winning

Sink all 5 enemy ships (15 total ship cells) to win. In single-player mode the CPU fires back each turn with a random untried coordinate.
