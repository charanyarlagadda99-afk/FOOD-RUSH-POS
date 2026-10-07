# FoodRush - System Overview & Architecture

## 1. What FoodRush Is
**FoodRush** is a full-stack, high-performance food delivery platform designed to showcase real-world systems programming, data structures, and algorithms for second-year Computer Science coursework and university viva voce examinations.

The platform provides a complete end-to-end food ordering workflow:
- **Customers**: Browse partner restaurants across culinary traditions, search dishes with fuzzy "did you mean" corrections, view Top-Rated dishes, customize their cart, undo accidental additions with a single click, submit verified dish ratings, and checkout with zone-based delivery calculation and express priority.
- **Order Tracking**: Track orders through a live 4-stage pipeline (Confirmed ➔ In Kitchen ➔ Out for Delivery ➔ Delivered) using tamper-proof tracking codes secured with string-reversal check digits.
- **Kitchens**: Receive incoming orders into a circular priority queue where express orders jump ahead of standard orders without starving the FIFO queue, with atomic real-time inventory locking and restocking.
- **Delivery Fleet**: Manage delivery riders with greedy nearest-neighbor spatial dispatch across a 5×5 distance matrix with vehicle speed multipliers.
- **Admin Operations**: View platform-wide metrics, a 6×7 restaurant revenue matrix, daily sales volume, delivery fleet availability, inventory restock tools, and 1D price/rating analytics.

---

## 2. Five-Person Capstone Project Architecture
FoodRush is partitioned cleanly across five specialized engineering modules:
1. **Catalog & Search Specialist (Member 1)**: Unified 48-item catalog, case-insensitive string search, $O(L_1 \cdot L_2)$ Levenshtein distance Did You Mean suggestion.
2. **Cart & State Specialist (Member 2)**: Hand-crafted `ArrayStack` LIFO undo/redo mechanism, two-pointer palindrome coupon validator, `std::map` promo discount engine.
3. **Kitchen Pipeline Specialist (Member 3)**: Ring buffer `CircularQueue` order FIFO, express priority scheduling, 4-stage order lifecycle, tracking code check-digit generator.
4. **Fleet Dispatch Specialist (Member 4 - Feature 1)**: Inter-zone spatial routing, greedy nearest-rider selection via $5 \times 5$ distance matrix, dynamic vehicle speed multipliers.
5. **Inventory Lock & Feedback Specialist (Member 5 - Features 2 & 3)**: Two-phase stock reservation (`reservedStock`), atomic restock, customer rating feedback with running average, $O(N \log K)$ Top-K dish leaderboard.

---

## 2. The Dual-Surface Design Concept
FoodRush is built with two distinct, purposefully separated surfaces:

### Surface 1: The Commercial Storefront & Admin Portal (Default)
To any customer or restaurant manager, FoodRush looks, feels, and operates like a polished, modern food delivery service:
- **Editorial Design System**: Warm paper canvas (`#FBF9F6`), clean card elevations, deep obsidian typography with Fraunces serif headings and Inter body text, terracotta accents (`#D9531E`), and sage green status badges (`#2F7D5B`).
- **Zero Academic Jargon**: The customer experience contains **zero leaks** of internal implementation details. The words "C++", "engine", "Module", "array", "stack", "queue", or "STL" never appear anywhere on the customer or admin interface.
- **Instant Micro-interactions**: Slide-over cart drawer with quantity steppers, undo toast notifications, live order tracking stepper, and responsive restaurant tabs.

### Surface 2: The Viva Mode (For Examiners & Students)
Designed exclusively for university professors and technical demonstrations:
- **Discreet Activation**: Opened only by pressing the keyboard shortcut `V` or clicking the subtle "For examiners" link in the footer. Off by default.
- **Live CS Inspection**:
  - **Module Badges**: Every user action and API response highlights which of the 10 CS syllabus modules executed the command.
  - **Memory & DS Inspector**: Visualizes internal C++ memory state—hand-crafted `ArrayStack` frames with `topIndex`, `CircularQueue` front/rear ring buffer indices, and STL container sizes.
  - **Real Stopwatch Benchmarks**: Live microsecond execution comparisons ($O(N)$ vs $O(\log N)$ search, $O(N^2)$ vs $O(N \log N)$ sort).
  - **Interactive Module Guide**: Full mapping of all 10 syllabus modules directly linked to source code and demonstration steps.

---

## 3. How FoodRush Works (The Architecture Flow)

```
[ Customer / Admin Browser ]
       │
       ▼  HTTP / REST API (JSON)
[ Node.js Bridge Server ] (server/server.js)
       │  - Spawns C++ child process once on startup
       │  - Line-oriented protocol over persistent stdin/stdout
       ▼
[ C++ High-Performance Engine ] (foodrush_engine.exe)
   ├── Module I    : Basics (Constants, Type Conversion, Bitwise Dietary Flags)
   ├── Module II   : Control Statements & Functions (Command Dispatcher)
   ├── Module III  : 1D Arrays (Ratings, Prices, Stock, Sum/Min/Max/Bubble Sort)
   ├── Module IV   : 2D Arrays (6x7 Sales Matrix & 5x5 Zone Distance Matrix)
   ├── Module V    : Strings (Levenshtein Distance, Tokenizer, Reversal Check Digit)
   ├── Module VI   : Structures (Address, MenuItem, Restaurant, Order, Rider)
   ├── Module VII  : Performance (Timed Stopwatch Benchmarks, Big-O Comparisons)
   ├── Module VIII : Hand-Crafted Array Stack (Cart Undo & Recently Viewed)
   ├── Module IX   : Hand-Crafted Circular Queue (Kitchen Dispatch & Rider Rotation)
   └── Module X    : Standard Template Library (std::map, set, vector, deque, stack, queue)
```

### End-to-End Request Pipeline
1. **User Action**: The customer clicks "+ Add" on a dish, adjusts quantities, or applies a coupon.
2. **Web Bridge**: The browser issues a `POST /api/cart/add` or `POST /api/checkout` request.
3. **IPC Command Execution**: `server/server.js` formats the request into a single newline-delimited command string (e.g. `CART_ADD 101 2`) and writes it to the C++ process `stdin`.
4. **Deterministic Engine Logic**: The C++ engine processes the command in-memory using low-level arrays, structs, and algorithms, serializes the response to a single JSON line, tags it with the active syllabus modules (e.g. `["I", "VI", "VIII"]`), and writes it to `stdout`.
5. **UI Update**: The web UI receives the JSON response in under **2 milliseconds**, updates the UI state, and (if Viva Mode is active) flashes the corresponding module badges.

---

## 4. Why C++ is the Backend Engine
1. **Deterministic Memory & Zero Garbage Collection**: Real-world dispatch and matching engines require predictable latency. Contiguous arrays and struct buffers in C++ execute without runtime garbage collection pauses.
2. **Pedagogical Integrity**: Rather than hiding data structures behind high-level language wrappers, students write raw `ArrayStack` and `CircularQueue` implementations with pointer bounds checks and modulo wrap-around arithmetic.
3. **High-Efficiency IPC**: The native C++ binary communicates with Node.js via lightweight local pipes, completing end-to-end round-trips in ~1.85 ms while consuming negligible system resources.
