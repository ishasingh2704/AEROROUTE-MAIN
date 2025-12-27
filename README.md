# ✈️ Skyways: DSA Flight Simulator

**Skyways: DSA Flight Simulator** is a comprehensive C++ project that merges the power of **Data Structures and Algorithms (DSA)** with real-world aviation systems. It visually demonstrates how classic pathfinding techniques— **Dijkstra’s**, **A\***, and **Bellman-Ford** —can be applied to dynamic flight routing, complete with live weather-based rerouting via the **OpenWeatherMap API**. With a sleek **SFML** interface and automated **HTML/CSS** boarding pass generation, Skyways transforms algorithm education into an engaging, interactive experience.

---

## 🚀 Why Skyways Matters

- 🔁 **Realistic Simulation :** Emulates flight rerouting under adverse weather conditions—just like in real-world aviation.
- 🧑‍🏫 **Educational Value :** Demonstrates advanced algorithms in a visual and interactive format.
- 🔗 **API Integration :** Uses real-time weather data via the OpenWeatherMap API to affect routing decisions.
- ⚙️ **Complete Tech Stack :** Combines C++, SFML, web development (HTML/CSS), and RESTful APIs.
- 🧳 **Portfolio-Ready :** A standout addition for students or developers showcasing algorithmic problem-solving and UI integration.

---

## 🛫 Project Overview

Skyways is composed of two main modules:

### 1. **Flight Booking System** (`flight_booking.cpp`)
- Select departure and arrival airports by code or index.
- View available flights over 5 days with dynamic pricing.
- Enter passenger details and confirm booking.
- Generates a modern **HTML/CSS** boarding pass stored in `output/`.
- Launches the **Flight Simulator** module with selected route.

### 2. **Flight Simulator** (`flight_simulator.cpp`)
- Animates the chosen route with real-time flight path visualization.
- Fetches and processes **live weather data**.
- Automatically reroutes flights for safety.
- Offers choice of **Shortest**, **Cheapest**, or **Fastest** path.
- Supports all three algorithms: **Dijkstra**, **A\***, and **Bellman-Ford**.
- Built using **SFML** for dynamic rendering and animation.

---

## 🧠 DSA in Action

This project demonstrates how advanced algorithmic logic and data structures can solve real-world problems:

- **Graph Theory :** Airports and routes modeled as a weighted graph.
- **Pathfinding Algorithms :** Implements Dijkstra’s, A*, and Bellman-Ford algorithms to compute the optimal path.
- **Efficient Data Structures :** Uses adjacency lists, edge structures, STL containers (`vector`, `map`, `set`), and priority queues.
- **Real-Time Adaptation :** Reacts dynamically to weather data and re-routes flights accordingly.

---

## ✨ Key Features

- 🧾 Terminal-based flight booking
- 📅 Dynamic pricing engine
- 🌩️ Real-time weather-driven path changes
- 🧭 Visualized pathfinding (Dijkstra, A*, Bellman-Ford)
- ✈️ Animated aircraft movement on map
- 🧾 HTML/CSS boarding pass for every booking
- 🧰 Modular and modern C++17 codebase
- 🖼️ SFML interface for simulation and UI
- 💻 Command-line and graphical launch support

---

## 📁 Project Structure

Skyways-DSA-Flight-Simulator/
- `src/` — C++ source files (`flight_simulator.cpp`, `flight_booking.cpp`, `config.h`)
- `assets/` — Runtime assets (Images, Maps)
- `dll/` — Required DLLs (not committed to Git)
- `output/` — Generated files (Boarding passes)
- `screenshots/` — Demo Images and documentation visuals
- `demo/` — Animated demo (MP4)
- `compile.bat` — Build script (Windows)
- `run.bat` — Run script (sets up DLL path)
- `.vscode/` — VS Code settings (optional)
- `.gitignore` — Git ignore rules
- `README.md` — Project documentation

---

## 📦 Dependencies

- **SFML 2.6.2**                  [Graphics rendering and real-time animation]
- **CPR**                         [HTTP requests (for OpenWeatherMap API)]
- **nlohmann/json**               [JSON parsing and processing]  
- **C++ Compiler (C++17+)**       [Required to compile the project]
- **Windows OS**                  [Current build supports Windows by default]
- **OpenWeatherMap API key**      [Access to live weather data]

---

## 🛠️ How to Build and Run

1. **Clone the Repository** and install Dependencies (see above).
2. Run `compile.bat` to build the project. This will :
   - Compile all source files from `src/`
   - Copy all required DLLs into the `dll/` folder
3. Use `run.bat` to launch the booking system. This will:
   - Temporarily add `dll/` to your PATH so all DLLs are found
   - Run `flight_booking.exe` (or modify to run `flight_simulator.exe` as needed)

---

## 🎬 Demo

You can preview key outputs of the project below:

🗺️ Flight Path Visualization :  ![Flight Path](screenshots/)

🎟️ Boarding Pass Sample :  ![Boarding Pass](output/boarding_pass.png)

🎞️ Application Demo :  ![Demo](demo/flight_visualization.mp4)

---

## 🔑 API Key

- The OpenWeatherMap API key is stored securely in a configuration file for better security and flexibility.
- Before running the project, make sure to insert your own API key into the config file (e.g., `config.h` or `.env`).
- In production or public use, consider encrypting or hiding your config via environment variables or a secure key vault mechanism.

---

## 🧩 Troubleshooting

- **Missing DLLs :** Make sure to use `run.bat` so all DLLs in `dll/` are found.
- **API Key Issues :**  Update the default OpenWeatherMap API key in the source code with your own key, or store it securely via a configuration file or environment variable.

---

## 📚 License

This project is licensed under the **MIT** License and is intended solely for educational and academic purposes.

---

## 🙏 Acknowledgments

- **OpenWeatherMap** — For providing real-time weather data.
- **SFML** — For powering the visual simulation.

---

## 👤 Author

Teesha Dhiman
(Developer of Skyways: DSA Flight Simulator)