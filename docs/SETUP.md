## Live Production URL (Vercel)
FoodRush is deployed live and globally accessible at:
- **Production URL:** [https://foodrush-kappa.vercel.app](https://foodrush-kappa.vercel.app)
- **Deployment URL:** [https://foodrush-6utq7thox-foraitools28-9900s-projects.vercel.app](https://foodrush-6utq7thox-foraitools28-9900s-projects.vercel.app)

---

## Prerequisites (For Local Execution)

---

## 1. Fast Launch (Windows One-Click)
Simply double-click:
```cmd
run.bat
```
This batch script automatically compiles the C++ engine if needed and launches the Web Bridge Server on `http://localhost:3050`.

---

## 2. Manual Build & Execution

### Step A: Compile the C++ Engine
From the project root (`d:\c++_year_2`):
```bash
g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe
```

### Step B: Run the Interactive Terminal Mode (For Offline / Terminal Viva)
To run FoodRush purely in the terminal console without starting the web server:
```bash
.\foodrush_engine.exe --cli
```
This opens the interactive text menu (Options 1–9) to browse restaurants, test the stack undo, simulate kitchen cooking, view the 7-day sales matrix, and run live benchmarks.

### Step C: Start the Web Application
```bash
npm start
# or: node server/server.js
```
Open your browser and navigate to:
```
http://localhost:3050
```

---

## 3. Running Automated Tests
Run the comprehensive 19-command smoke test suite:
```bash
npm test
# or: node test/smoke_test.js
```
Expected output:
```
Test Summary: 19 Passed, 0 Failed
All 10 Modules Verified and Confirmed Operational!
```

---

## 4. Troubleshooting
- **Port already in use (`EADDRINUSE`)**: The server automatically detects if port 3050 is occupied and shifts to 3051 or 3052. Look at the terminal output for the active URL.
- **Compiler not recognized**: Ensure `g++` is in your system `PATH`.
