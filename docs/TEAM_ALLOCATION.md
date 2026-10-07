# FoodRush POS: 5-Person Capstone Engineering Team Allocation

This document outlines the architectural division of labor and technical ownership for presenting **FoodRush POS** (Restaurant, Hotel & Cafe Management System) as a 5-person university capstone engineering project. Every engineer owns dedicated algorithms, data structures, and features in the C++ backend engine.

---

## 👥 Summary Team Matrix

| Role & Member | Primary Architectural Domain | Syllabus Modules | Core C++ Data Structures & Algorithms | Key Engine Commands |
|---|---|---|---|---|
| **Member 1**<br>Catalog & Search Specialist | Multi-Outlet Catalog & Fuzzy Search Engine | **Module III, V** | 48-Item Menu Arrays, Substring Scanning, Levenshtein DP Matrix ($O(L_1 \cdot L_2)$), Case Normalization | `GET_MENU`, `SEARCH_DISHES`, `GET_OUTLETS` |
| **Member 2**<br>Table Order & State Specialist | Dine-In Table Management & LIFO Order Undo | **Module I, VI, VIII, X** | 12 Dining Tables Array, Custom `ArrayStack` (LIFO Action Undo), `std::map` Coupon Lookup | `GET_TABLES`, `SELECT_TABLE`, `ORDER_ADD`, `ORDER_REMOVE`, `ORDER_UNDO`, `ORDER_CLEAR` |
| **Member 3**<br>Kitchen Pipeline Specialist | Multi-Ticket Concurrent Kitchen KOT Queue | **Module II, IX, X** | Custom `CircularQueue` Ring Buffer, Priority Two-Queue Express Fast-Track, Stage Transitions | `SUBMIT_KOT`, `GET_ACTIVE_ORDERS`, `UPDATE_ORDER_STAGE` |
| **Member 4**<br>Invoicing & Printing Specialist | GST Tax Invoicing & Thermal Receipt Formatter | **Module I, V, X** | GST & Service Charge Calculation, String-Reversal Check-Digit Invoices, ASCII Receipt Formatter, `std::deque` Archive | `GENERATE_BILL`, `GET_BILLS`, `PRINT_BILL` |
| **Member 5**<br>Staff & Matrix Analytics Specialist | Staff Attendance Roster & 2D Sales Matrix | **Module III, IV, VII** | 1D Staff Struct Array, 2D Sales Revenue Matrix ($6 \times 7$), Table Seating Matrix ($4 \times 3$), Stopwatch Benchmarks | `GET_STAFF`, `MARK_ATTENDANCE`, `GET_SALES_MATRIX`, `GET_TOP_DISHES`, `BENCHMARK`, `COMPARE_DS` |

---

## 🔬 Detailed Individual Engineering Ownership

### 👤 Member 1: Catalog & Search Specialist
* **Responsibilities:**
  - Designed the unified multi-outlet catalog in C++ with 48 distinct dishes across 6 hotel & restaurant kitchen outlets (Grand Mughal, Dakshin Tiffin, Trattoria Bella Vista, Sakura Asian, Boulevard Grill, Royal Patisserie).
  - Implemented the string search algorithm (`SEARCH_DISHES`) utilizing case-insensitive substring searching via `toLowerString` and manual character matching.
  - Built the **Levenshtein Distance Dynamic Programming Matrix** ($O(L_1 \cdot L_2)$) to provide "Did You Mean" phonetic/spelling corrections when typos occur (e.g. `biryany` $\rightarrow$ `biryani`).
  - Integrated bitwise dietary flags (`1 = Spicy`, `2 = Vegetarian`, `4 = ChefSpecial`) for $O(1)$ dietary classification.
* **Viva Questions to Answer:**
  - *Q: Why use Levenshtein distance instead of a simple contains check?*  
    *A:* User typos (e.g., "biryany" vs "biryani") would cause an empty search result. Levenshtein dynamic programming calculates the minimum single-character edits (insertions, deletions, substitutions) to find the closest menu item under edit distance $\le 3$.
  - *Q: How does your search handle case-insensitivity without regex?*  
    *A:* We implement a manual loop converting characters to lowercase via ASCII manipulation (`tolower`), avoiding heavy regex libraries.

---

### 👤 Member 2: Table Order & State Management Specialist
* **Responsibilities:**
  - Engineered the hand-crafted `ArrayStack<CartAction, 50>` LIFO data structure from scratch with zero dynamic memory allocation.
  - Architected the table order undo mechanism: every addition, quantity alteration, or removal pushes a delta frame `(itemId, prevQty, newQty)` onto the stack. Clicking Undo executes a LIFO pop and runs the inverse operation.
  - Managed the 12 dining tables array (`gTables[12]`) across 4 sections (Main Dining, AC Family Lounge, Rooftop Terrace, Garden Lounge) with `VACANT` / `OCCUPIED` states.
  - Designed the $O(\log C)$ coupon registry using `std::map<std::string, double>` for percentage discount lookups.
* **Viva Questions to Answer:**
  - *Q: What are the time and space complexities of your undo stack?*  
    *A:* Push and pop operations are strictly $O(1)$ time complexity and operate within a bounded $O(\text{CAPACITY})$ contiguous array buffer, preventing stack overflows.
  - *Q: Why did you build `ArrayStack` with arrays before using `std::stack`?*  
    *A:* To demonstrate memory locality, raw stack pointer manipulation (`topIndex`), and avoid heap allocations during high-frequency cart operations.

---

### 👤 Member 3: Kitchen Pipeline & Multi-Ticket Queue Specialist
* **Responsibilities:**
  - Engineered the circular ring buffer `CircularQueue<int, 50>` with modular arithmetic `(front + 1) % CAPACITY` to eliminate the $O(N)$ element shift problem.
  - Built the `PriorityOrderQueue` multi-queue system that prioritizes express priority tickets over standard tickets.
  - Implemented the concurrent multi-order live kitchen board displaying active tickets from all occupied tables simultaneously.
  - Managed the order lifecycle: `ORDERED` $\rightarrow$ `PREPARING` $\rightarrow$ `SERVED` $\rightarrow$ `BILLED`.
* **Viva Questions to Answer:**
  - *Q: Why is a linear queue inefficient for a kitchen dispatch system?*  
    *A:* In a linear array queue, dequeuing an order requires shifting all remaining elements left ($O(N)$), or the front pointer drifts off the end. A circular queue reclaims vacated slots in $O(1)$ time using modulo indexing.
  - *Q: How does express priority order scheduling work?*  
    *A:* The scheduler queries the express queue first; only when the express queue is exhausted does it dequeue standard tickets, ensuring urgent orders cook first.

---

### 👤 Member 4: Tax Invoicing & Thermal Printing Specialist
* **Responsibilities:**
  - Formulated the itemized billing engine: 5% GST (SGST+CGST), 5% Service Charge, and promo discounts.
  - Developed the **Tamper-Proof Invoice Code Check-Digit Algorithm** using string reversal: reverses the bill ID string, computes weighted sum modulo 10, and appends the check digit (e.g. `INV-5001-1`).
  - Built the formatted monospaced ASCII thermal receipt formatter, printing clean tax receipts with GSTIN numbers, itemized breakdown, and totals.
  - Architected the settled past bills archive utilizing `std::deque<BillReceipt>` for $O(1)$ front insertion and retrieval.
* **Viva Questions to Answer:**
  - *Q: How is the invoice check digit computed?*  
    *A:* The bill ID digits are reversed via our string reversal algorithm [Module V]. Each reversed digit is multiplied by its position index (`sum += digit * (i + 1)`), and `sum % 10` forms the check digit.
  - *Q: Why use `std::deque` instead of `std::vector` for the past bills archive?*  
    *A:* Settled bills arrive in real-time and must be displayed newest-first. `std::deque::push_front` executes in $O(1)$ time without relocating existing elements, whereas `std::vector::insert(begin)` requires $O(N)$ element shifts.

---

### 👤 Member 5: Staff Attendance & Matrix Analytics Specialist
* **Responsibilities:**
  - Engineered the Staff Attendance Register managing 8 staff members across morning, evening, and full-day shifts with hours worked and duty status toggling.
  - Maintained the $6 \times 7$ weekly revenue matrix (`sales[outlet][day]`) in 2D contiguous memory: row sums calculate weekly revenue per outlet, column sums calculate daily revenue, and a max scan locates the peak sales slot.
  - Built the **Customer Rating Feedback System** (`RATE_DISH`) with running average formula and the **Top-K Leaderboard** algorithm (`getTopKDishes`) with $O(N \log K)$ sorting complexity.
  - Managed the stopwatch benchmarking suite (`BENCHMARK` & `COMPARE_DS`) measuring real CPU cycles for linear vs binary search, bubble sort vs introsort, and custom structures vs STL.
* **Viva Questions to Answer:**
  - *Q: What is the complexity of computing weekly sales analytics?*  
    *A:* $O(R \cdot D)$ where $R=6$ outlets and $D=7$ days. Both daily sums and outlet totals are computed in single linear passes.
  - *Q: What do your stopwatch benchmarks demonstrate to the examiners?*  
    *A:* On 30,000 elements, Binary Search ($O(\log N)$) finishes in ~0.1 µs while Linear Search ($O(N)$) takes ~12 µs (over 100x faster). Our custom contiguous array stack also outperforms STL due to zero heap allocations.

---

## 🎯 How to Conduct a Group Viva Demonstration

1. **Launch the Engine & Web UI:**
   - Run `node server/server.js` (or start `.\foodrush_engine.exe` directly).
   - Open `http://localhost:3000` in the browser.

2. **Step-by-Step Viva Run-Through:**
   - **Member 1**: Types `piza` in the search bar, explains the Levenshtein distance matrix suggestion and dietary bitwise flags.
   - **Member 2**: Selects Table 3, adds 2x Biryani, clicks **Undo** to demonstrate the hand-crafted `ArrayStack` LIFO reversion.
   - **Member 3**: Punches an Express KOT, opens **Kitchen KOT Board (`#kitchen`)**, demonstrates multi-order concurrency and advances order stages.
   - **Member 4**: Returns to Table 3, clicks **Settle & Print Bill**, displays the ASCII thermal receipt with GST, explains the reversal check-digit invoice code, and shows the archived bill in the past bills tab.
   - **Member 5**: Opens **Staff & Shifts (`#staff`)** to toggle attendance, opens **Sales Analytics (`#sales`)** to show row/col sums on the 6x7 matrix, and presses `V` to show microsecond stopwatch benchmarks.
