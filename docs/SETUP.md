# FoodRush POS - Setup & Execution Guide

## Prerequisites
- **C++ Compiler**: `g++` with C++17 support (MinGW-w64 on Windows, or GCC / Clang on Linux/macOS).
- **Node.js**: Node.js LTS (v18.0 or higher).
- **Terminal**: PowerShell, Command Prompt, or bash.

To verify your tools:
```bash
g++ --version
node --version
```

---

## 1. Quick Launch (One Click / One Command)

### Option A: Windows Batch File (Easiest)
From the project root (`FOOD-RUSH-POS`):
Simply double-click:
```
run.bat
```
*(This automatically compiles `foodrush_engine.exe` if not already built, starts the Node server, and points your browser to `http://localhost:3000`.)*

### Option B: Node.js Command
```bash
npm start
```
*(Runs `node server/server.js` on `http://localhost:3000`)*

Then open your browser and navigate to:
```
http://localhost:3000
```
*(If port 3000 is occupied, the bridge server automatically tries ports 3001, 3002, etc., and logs the active address.)*

---

## 2. Manual Build & Step-by-Step Execution

### Step A: Compile the C++ Engine
To recompile the C++ engine binary manually:
```bash
g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe
```
> [!NOTE]
> On Windows, if `ld.exe: cannot open output file foodrush_engine.exe: Permission denied` appears, stop any running `node server/server.js` or `foodrush_engine.exe` processes before compiling:
> ```powershell
> Stop-Process -Name "foodrush_engine" -ErrorAction SilentlyContinue
> ```

### Step B: Interactive CLI Mode (For Terminal Viva / Offline Mode)
To test and demonstrate all features directly in the console without a browser:
```bash
.\foodrush_engine.exe --interactive
```
Available interactive commands:
- `GET_INITIAL_STATE` - View POS boot state
- `GET_TABLES` - View 12 dining tables status
- `SELECT_TABLE <id>` - Select active table (1-12)
- `ORDER_ADD <id> <qty>` - Add dish to table order (ArrayStack Push)
- `ORDER_UNDO` - Undo last table modification (ArrayStack Pop)
- `SUBMIT_KOT <guest> <0|1>` - Dispatch Kitchen Order Ticket (Circular Queue)
- `GET_ACTIVE_ORDERS` - View live kitchen KOT queue
- `GENERATE_BILL <tableId>` - Settle bill with GST & receipt
- `GET_BILLS` - View past settled bills archive (STL deque)
- `PRINT_BILL <billId>` - Print formatted thermal receipt (Check-digit)
- `GET_STAFF` - View staff attendance & shifts
- `GET_SALES_MATRIX` - 6 Outlets x 7 Days weekly sales matrix
- `BENCHMARK` - Run stopwatch performance test
- `COMPARE_DS` - Custom ArrayStack/Queue vs STL test
- `EXIT` - Quit interactive console

---

## 3. Running Automated Integration Tests

To run the complete 24-step smoke test suite covering all 10 syllabus modules and POS workflows:
```bash
npm test
```
*(or `node test/smoke_test.js`)*

Expected output:
```
Smoke Test Results: 24 Passed, 0 Failed (100% Green)
All 10 Syllabus Modules & POS Subsystems Verified Operational!
```
