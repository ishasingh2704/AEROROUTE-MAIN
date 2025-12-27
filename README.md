
---

```markdown
# ✈️ AeroRoute – Flight Routing & Simulation System (DSA Project)

A **C++ console-based flight routing and simulation system** built to demonstrate **graph algorithms, pathfinding techniques, and real-world decision making** using Data Structures & Algorithms.

This project models airports as nodes and flight routes as weighted edges, allowing users to:
- Book flights
- Analyze weather conditions
- Compute optimal flight paths
- Simulate rerouting under adverse conditions

---

## 🎯 Problem Statement

Given a network of airports connected by flight routes with different distances, costs, and travel times, determine the **optimal path** between a source and destination airport.

The system must:
- Handle multiple optimization criteria (distance, cost, time)
- Adapt routes based on **weather conditions**
- Support multiple shortest-path algorithms
- Simulate real-world flight booking and routing logic

---

## 🧠 Core DSA Concepts Used

- **Graph Representation**
  - Airports → Nodes
  - Flight routes → Weighted edges
- **Adjacency List**
- **Shortest Path Algorithms**
  - Dijkstra’s Algorithm
  - A* (A-Star) Algorithm
  - Bellman–Ford Algorithm
- **Priority Queue (Min Heap)**
- **Greedy Algorithms**
- **Command-Line Based Simulation**

---

## 🧮 Algorithms Implemented

| Algorithm | Purpose |
|---------|--------|
| Dijkstra | Fast shortest path (non-negative weights) |
| A* (A-Star) | Heuristic-based optimized routing |
| Bellman–Ford | Handles negative weights |
| Greedy Selection | Cheapest / fastest path selection |

---

## ⏱️ Time & Space Complexity

- **Dijkstra:** `O(E log V)`
- **A*:** `O(E)` (heuristic-dependent)
- **Bellman–Ford:** `O(VE)`
- **Space Complexity:** `O(V + E)`

Where:
- `V` = number of airports
- `E` = number of flight routes

---

## 🖥️ Features

- 📍 Airport selection using **index or airport code**
- 🛫 Flight booking with seat allocation
- 🌦️ Real-time weather integration (simulated)
- 🔁 Automatic rerouting due to bad weather
- 📊 Comparison of:
  - Shortest path (distance)
  - Cheapest path (cost)
  - Fastest path (time)
- 🎥 Flight path visualization (video demo)

---

## 📂 Project Structure

```

AEROROUTE-MAIN/
├── src/
│   ├── flight_booking.cpp
│   ├── flight_simulator.cpp
│
├── assets/
├── screenshots/
│   ├── 1.png
│   ├── 2.png
│   ├── 3.png
│   └── 4.png
│
├── demo/
│   └── flight_visualization.mp4
│
├── compile.bat
├── run.bat
└── README.md

````

---

## 🔢 Sample Input / Output

### Input
- Departure airport: `AUG`
- Arrival airport: `PHX`
- Path preference: Cheapest
- Algorithm: A*

### Output
- Selected route: `AUG → STL → PHX`
- Cost: `$217`
- Flight time: `288.1 minutes`
- Weather-based rerouting applied successfully

---

## ▶️ How to Run the Project

### Option 1: Using batch files (Windows)
```bash
compile.bat
run.bat
````

### Option 2: Manual compilation

```bash
g++ src/flight_booking.cpp src/flight_simulator.cpp -o aeroroute
./aeroroute
```

---

## 🎥 Demo & Screenshots

* 📹 **Flight Simulation Video:** `demo/flight_visualization.mp4`
* 🖼️ Screenshots available in `/screenshots` folder showing:

  * Airport selection
  * Flight booking
  * Weather analysis
  * Pathfinding results

---

## 💡 Key Learnings

* Practical implementation of graph algorithms
* Handling real-world constraints using DSA
* Designing modular C++ programs
* Comparing multiple shortest-path strategies
* Applying algorithmic thinking to simulations

---

## 🔮 Future Enhancements

* Add real-time API-based weather data
* GUI-based visualization
* Support for dynamic graph updates
* Multi-user booking simulation
* Performance benchmarking of algorithms

---

## 👩‍💻 Author

**Isha Singh**
CSE Undergraduate (5th Semester)
Jaypee Institute of Information Technology, Noida

---

