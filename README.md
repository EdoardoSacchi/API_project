# API Project – Hexagonal Grid Navigation

Final project for the **Algorithms and Principles of Computer Science** course (Politecnico di Milano).

This program manages a map based on a hexagonal grid, supporting the creation of direct air routes, dynamic cost modification, and the calculation of the minimum cost path using **Dijkstra's** algorithm.

The full project specification is available in the [`project-specifications-24-25.pdf`](project-specifications-24-25.pdf) file.

---

## Table of Contents

- [Description](#description)
- [Commands](#commands)
- [Data Structures and Algorithms](#data-structures-and-algorithms)
- [Compilation and Execution](#compilation-and-execution)
- [Testing](#testing)
- [Repository Structure](#repository-structure)

---

## Description

The program operates on a hexagonal map where:

- Each hexagon has a **transit cost** between 0 and 100 (initialized to 1).
- It is possible to create direct **air routes** between hexagons (maximum 5 per source hexagon).
- Costs can be **dynamically modified** over a circular area with attenuation proportional to the distance.
- The **minimum cost path** between two points is calculated using Dijkstra's algorithm, considering both grid adjacencies and air routes.

Navigation on the hexagonal grid uses the **odd-r offset** coordinate system, converted internally into **axial coordinates** to manage the 6 directions of adjacency.

---

## Commands

The program reads commands from `stdin`, one per line. Available commands are:

### `init <columns> <rows>`

Initializes (or re-initializes) the hexagonal map. All hexagons are created with a cost of 1 and no air routes. The path cache is cleared.

- **Output:** `OK` if parameters are valid (both > 0), `KO` otherwise.

### `change_cost <x> <y> <v> <radius>`

Modifies the cost of hexagons in a circular area centered at `(x, y)`. The applied delta decreases from the center toward the edge:

> delta = floor(v × (radius - distance) / radius)

- `v` ∈ [-10, +10], `radius` > 0
- Costs remain constrained within the [0, 100] range.
- The costs of air routes for the involved hexagons are also updated.
- **Output:** `OK` or `KO`

### `toggle_air_route <xs> <ys> <xd> <yd>`

Activates or deactivates a direct air route from the hexagon `(xs, ys)` to `(xd, yd)`.

- If the route already exists, it is removed.
- If it doesn't exist, it is added with a cost calculated as an average: `(sum of existing route costs + source hexagon cost) / (num. routes + 1)`.
- Maximum 5 routes per source hexagon.
- **Output:** `OK` or `KO`

### `travel_cost <xs> <ys> <xd> <yd>`

Calculates the minimum path cost from `(xs, ys)` to `(xd, yd)`, considering both hexagonal adjacencies and air routes. Hexagons with a cost of 0 are impassable.

- **Output:** the minimum cost, or `-1` if no path exists.

---

## Data Structures and Algorithms

| Structure / Algorithm | Usage |
|---|---|
| **Binary Min-heap** | Priority queue for Dijkstra's algorithm |
| **Circular Queue (BFS)** | Level-order traversal in `change_cost` |
| **FIFO Cache** | Stores the last 10,000 calculated paths; invalidated on every map modification |
| **Dijkstra** | Calculation of the shortest path in `travel_cost` — O((V+E) log V) |
| **BFS** | Cost propagation in `change_cost` — O(r²) |
| **Axial Coordinates** | Efficient navigation across the 6 hexagonal grid directions |

---

## Compilation and Execution

### Requirements

- C compiler with **C11** standard support (e.g., GCC)
- **CMake** ≥ 3.31

### Build

To build the project, run the following commands in your terminal:

    mkdir build && cd build
    cmake ..
    make

### Execution

You can run the program using an input file or interactively:

    # From an input file
    ./progettoAPI < input.txt

    # Interactive (terminate with Ctrl+D)
    ./progettoAPI

### Example

    $ ./progettoAPI
    init 100 100
    OK
    change_cost 10 20 -10 5
    OK
    travel_cost 0 0 20 0
    20
    travel_cost 30 95 30 97
    12

---

## Testing

The `test_pubblici/` folder contains test cases with their respective expected outputs in the `Results/` sub-folder.

    # Run a test and compare with the expected output
    ./progettoAPI < test_pubblici/example.txt | diff - test_pubblici/Results/example.txt.result

| Test | Description |
|---|---|
| `example.txt` | Basic example |
| `edge_cases.txt` | Edge cases |
| `large.txt` | Large-scale map |
| `long.txt` | Long sequence of operations |
| `empty.txt` | Map with no costs |
| `stress_cache.txt` | Cache stress test |
| `stress_cache2.txt` | Cache stress test (variant) |

---

## Repository Structure

    progetto-API/
    ├── main.c                    # Source code
    ├── CMakeLists.txt            # Build configuration
    ├── specifica-progetto-24-25.pdf    # Project specification
    ├── test_pubblici/            # Public test cases
    │   ├── example.txt
    │   ├── edge_cases.txt
    │   ├── large.txt
    │   ├── long.txt
    │   ├── empty.txt
    │   ├── stress_cache.txt
    │   ├── stress_cache2.txt
    │   └── Results/              # Expected outputs
    │       ├── example.txt.result
    │       ├── edge_cases.txt.result
    │       └── ...
    └── README.md

## Evaluation

> This project received a final grade of **30/30**.
