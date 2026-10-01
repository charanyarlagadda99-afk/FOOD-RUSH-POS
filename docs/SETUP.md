# FoodRush - Setup & Execution Guide

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

## 1. Quick Launch (One Command)

From the project root (`d:\c++_year_2`):
```bash
node server/server.js
```
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
.\foodrush_engine.exe --cli
```
Features available in CLI mode:
- Option 1: Browse Restaurants & Menus
- Option 2: Search Dishes
- Option 3: Add to Cart
- Option 4: View Cart & Totals
- Option 5: Undo Last Cart Action (Stack LIFO)
- Option 6: Checkout & Place Order
- Option 7: Cook Next Order (Circular FIFO Queue)
- Option 8: View 6×7 Sales Matrix & Statistics
- Option 9: Run Algorithmic Performance Benchmarks

### Step C: Launch the Web Bridge Server
```bash
npm start
# or: node server/server.js
```

---

## 3. Running the Automated Smoke Test Suite

To verify all 21 engine commands, data structures, and response schemas:
```bash
node test/smoke_test.js
```
Expected output:
```
==================================================
  FoodRush C++ Engine - Comprehensive Smoke Test
==================================================
[PASS] GET_RESTAURANTS returned 6 restaurants
[PASS] GET_MENU returned 48 dishes
[PASS] SEARCH_DISH exact match 'Biryani' found 4 items
[PASS] SEARCH_DISH fuzzy 'piza' suggested 'pizza'
...
Summary: 21 Passed, 0 Failed
All 10 Syllabus Modules Verified and Operational!
```

---

## 4. Architecture Note: Why Local Persistent Server Over Serverless
FoodRush relies on a **stateful, long-lived C++ engine** process that maintains:
1. Contiguous in-memory arrays for the 6×7 sales matrix and 48 menu items.
2. The active `ArrayStack` frame pointer (`topIndex`) for cart undo.
3. The `CircularQueue` front and rear indices for real-time kitchen order dispatching.

Serverless deployment environments (such as Vercel Functions or AWS Lambda) spin down and freeze processes between HTTP invocations, destroying persistent in-memory data structures. Therefore, the application uses a local persistent Node.js bridge server (`server/server.js`) that communicates with `foodrush_engine.exe` via high-speed stdin/stdout IPC streams (measured round-trip time: **~1.85 ms**).
