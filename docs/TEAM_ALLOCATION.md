# FoodRush: 5-Person Capstone Engineering Team Allocation

This document outlines the architectural division of labor and technical ownership for presenting **FoodRush** as a 5-person university capstone engineering project. Every engineer owns dedicated algorithms, data structures, and features in the C++ backend engine.

---

## 👥 Summary Team Matrix

| Role & Member | Primary Architectural Domain | Syllabus Modules | Core C++ Data Structures & Algorithms | Key Engine Commands |
|---|---|---|---|---|
| **Member 1**<br>Catalog & Search Specialist | Product Catalog & String Search Engine | **Module III, V** | 1D Menu Arrays, Substring Scanning, Levenshtein Distance ($O(L_1 \cdot L_2)$), Case Normalization | `GET_MENU`, `SEARCH_DISH` |
| **Member 2**<br>Cart & State Specialist | LIFO Cart Transactions & Promotions | **Module I, VI, VIII, X** | Custom `ArrayStack` (LIFO Undo/Redo), Two-Pointer Palindrome Checker, `std::map` Coupon Lookup | `CART_ADD`, `CART_REMOVE`, `CART_UNDO`, `CART_CLEAR`, `APPLY_COUPON` |
| **Member 3**<br>Kitchen Pipeline Specialist | Circular Kitchen Queue & Express Priority | **Module II, IX, X** | Custom `CircularQueue` Ring Buffer, Two-Queue Express Priority, `std::deque` History | `CHECKOUT`, `COOK_ORDER`, `SIMULATE_NEXT_STAGE`, `TRACK_ORDER` |
| **Member 4**<br>Fleet Dispatch & Spatial Specialist *(Feature 1)* | Spatial Routing & Fleet Dispatch | **Module IV, VI** | 2D Distance Matrix ($5 \times 5$), Greedy Nearest-Neighbor Spatial Search, Speed Multipliers | `ASSIGN_RIDER`, `DISPATCH_ORDER`, `GET_FLEET_STATUS`, `GET_SALES_MATRIX` |
| **Member 5**<br>Inventory Lock & Feedback Specialist *(Features 2 & 3)* | Real-Time Inventory & Top-K Leaderboard | **Module I, III, VII** | Two-Phase Stock Reservation/Locking, Atomic Restock, Heap/IntroSort $O(N \log K)$ Ranking | `RESTOCK_ITEM`, `RATE_DISH`, `GET_TOP_DISHES`, `GET_ARRAY_STATS`, `BENCHMARK` |

---

## 🔬 Detailed Individual Engineering Ownership

### 👤 Member 1: Catalog & Search Specialist
* **Responsibilities:**
  - Designed the unified catalog data pipeline in C++ with 48 distinct items spanning 6 partner restaurants.
  - Implemented the custom string search algorithm (`SEARCH_DISH`) utilizing case-insensitive substring searching via `toLowerString` and manual character matching ($O(M \cdot K)$).
  - Built the **Levenshtein Distance Dynamic Programming Matrix** ($O(L_1 \cdot L_2)$) to provide "Did You Mean" phonetic/spelling corrections when zero direct substring matches are found.
  - Integrated bitwise dietary flags (`1 = Spicy`, `2 = Vegetarian`, `4 = Gluten-Free`) for $O(1)$ dietary classification.
* **Viva Questions to Answer:**
  - *Q: Why use Levenshtein distance instead of a simple contains check?*  
    *A:* User typos (e.g., "biryany" vs "biryani") would cause an empty search result. Levenshtein dynamic programming calculates the minimum single-character edits (insertions, deletions, substitutions) to find the closest menu item under edit distance $\le 3$.
  - *Q: How does your search handle case-insensitivity without regex?*  
    *A:* We implement a manual loop converting characters to lowercase via ASCII manipulation (`c >= 'A' && c <= 'Z' ? c + 32 : c`), avoiding heavy regex libraries.

---

### 👤 Member 2: Cart & State Management Specialist
* **Responsibilities:**
  - Engineered the hand-crafted `ArrayStack<CartAction, 50>` LIFO data structure from scratch with zero dynamic memory allocation.
  - Architected the cart undo/redo mechanism: every addition, quantity alteration, or removal pushes a delta frame `(itemId, prevQty, newQty)` onto the stack.
  - Implemented the two-pointer palindrome bonus coupon algorithm (`isPalindromeCoupon`) using string reversal ($O(N)$ time, $O(1)$ space).
  - Designed the $O(\log C)$ coupon registry using `std::map<std::string, double>` for percentage discount lookups.
* **Viva Questions to Answer:**
  - *Q: What are the time and space complexities of your undo stack?*  
    *A:* Push and pop operations are strictly $O(1)$ time complexity and operate within a bounded $O(\text{CAPACITY})$ contiguous array buffer, preventing stack overflows.
  - *Q: Why did you build `ArrayStack` with arrays before using `std::stack`?*  
    *A:* To demonstrate memory locality, raw stack pointer manipulation (`topIndex`), and avoid heap allocations during high-frequency cart operations.

---

### 👤 Member 3: Kitchen Pipeline & Order Lifecycle Specialist
* **Responsibilities:**
  - Engineered the circular ring buffer `CircularQueue<int, 50>` with modular arithmetic `(front + 1) % CAPACITY` to eliminate the $O(N)$ element shift problem.
  - Built the `PriorityOrderQueue` multi-queue system that prioritizes express priority orders over standard delivery tickets.
  - Implemented the 4-stage order lifecycle: `PLACED` $\rightarrow$ `PREPARING` $\rightarrow$ `OUT_FOR_DELIVERY` $\rightarrow$ `DELIVERED`.
  - Created the tracking code check-digit generator and validator using weighted ASCII character summation and string reversal.
* **Viva Questions to Answer:**
  - *Q: Why is a linear queue inefficient for a kitchen dispatch system?*  
    *A:* In a linear array queue, dequeuing an order requires shifting all remaining elements left ($O(N)$), or the front pointer drifts off the end. A circular queue reclaims vacated slots in $O(1)$ time using modulo indexing.
  - *Q: How does express priority order scheduling work?*  
    *A:* The scheduler queries the express queue first; only when the express queue is exhausted does it dequeue standard tickets, ensuring urgent orders cook first.

---

### 👤 Member 4: Fleet Dispatch & Spatial Specialist (Feature 1)
* **Responsibilities:**
  - Architected the inter-zone spatial routing system using a symmetric $5 \times 5$ distance matrix in 2D contiguous memory.
  - Designed the **Greedy Nearest-Available Rider Dispatch Algorithm** (`dispatchNearestRider`): scans available couriers and picks the one minimizing distance to the restaurant zone.
  - Modeled rider mobility attributes: vehicle type (cargo bike, electric scooter, EV), zone location, and speed multiplier ($1.00\times$ to $1.35\times$) for dynamic transit ETA calculation.
  - Maintained the $6 \times 7$ platform revenue matrix (`sales[restaurant][day]`) with row totals, column totals, and peak revenue detection.
* **Viva Questions to Answer:**
  - *Q: How is rider selection computed?*  
    *A:* We execute a spatial scan across `gRiders[]`: filtering `isAvailable == true`, looking up `distanceMatrix[rider.currentZone][restaurant.zone]`, and picking $\min(\text{distance})$. If tied, we select based on courier speed multiplier.
  - *Q: What is the complexity of computing weekly sales analytics?*  
    *A:* $O(R \cdot D)$ where $R=6$ restaurants and $D=7$ days. Both daily sums and restaurant totals are computed in single linear passes.

---

### 👤 Member 5: Real-Time Inventory & Ranking Specialist (Features 2 & 3)
* **Responsibilities:**
  - Engineered the **Two-Phase Inventory Lock & Reservation System**:
    - Adding to cart immediately locks stock (`reservedStock += qty`), preventing race conditions and cart collisions.
    - Cart removal or undo atomically releases stock (`reservedStock -= qty`).
    - Checkout permanently deducts stock (`stock -= qty; reservedStock -= qty;`).
  - Created the atomic `RESTOCK_ITEM` command for kitchen inventory management.
  - Built the **Customer Rating Feedback System** (`RATE_DISH`) with running average formula:  
    $$\bar{R}_{new} = \frac{\bar{R}_{old} \cdot C_{old} + \text{score}}{C_{old} + 1}$$
  - Engineered the **Top-K Leaderboard** algorithm (`getTopKDishes`) with $O(N \log K)$ sorting complexity to surface highest-rated dishes.
  - Managed the stopwatch benchmarking suite (`BENCHMARK` & `COMPARE_DS`) comparing custom structures against the C++ STL.
* **Viva Questions to Answer:**
  - *Q: How do you prevent out-of-stock items from being ordered by multiple users?*  
    *A:* We maintain both `stock` and `reservedStock`. An item's available stock is $\max(0, \text{stock} - \text{reservedStock})$. If available stock is 0, subsequent `CART_ADD` requests are rejected at the engine level.
  - *Q: Why is your Top-K dish ranking efficient?*  
    *A:* Instead of sorting all $N=48$ dishes completely, we extract candidate dishes and perform a bounded ranking sort to return the top $K$ items in $O(N \log K)$ time.

---

## 🎯 How to Conduct a Group Viva Demonstration

1. **Launch the Engine & Web UI:**
   - Run `node server/server.js` (or start `.\foodrush_engine.exe` in interactive CLI mode).
   - Open `http://localhost:3000` in the browser.
2. **Member 1 Demos:**
   - Type `biryani` in the search bar $\rightarrow$ instant matches.
   - Type `biryany` (typo) $\rightarrow$ "Did you mean: Biryani" chip appears (Levenshtein distance in action!).
3. **Member 2 Demos:**
   - Add a dish $\rightarrow$ open cart $\rightarrow$ click **Undo** $\rightarrow$ items revert via `ArrayStack`.
   - Apply coupon `FIRST50` (std::map discount) or palindrome coupon `LEVEL` (string reversal check).
4. **Member 3 Demos:**
   - Place an order $\rightarrow$ check Live Order Tracker.
   - Click "Simulate Next Stage" $\rightarrow$ order moves from Circular Kitchen Queue to Rider Dispatch.
5. **Member 4 Demos:**
   - Open **Admin** tab $\rightarrow$ inspect **Delivery Fleet Availability** (riders with zones and speed multipliers).
   - Point to the $6 \times 7$ Weekly Revenue Matrix and $5 \times 5$ Zone Distance Matrix.
6. **Member 5 Demos:**
   - In Storefront, click **★ Top Rated** filter $\rightarrow$ observe Top-K Leaderboard ($O(N \log K)$ ranking).
   - Click any dish's rating badge (e.g. `★ 4.9`) $\rightarrow$ submit a 5★ review $\rightarrow$ observe moving average recalculation live!
   - In Admin tab, use **Inventory Restock Control** to add units to any low-stock dish.
   - Press <kbd>V</kbd> to open Examiner Workbench $\rightarrow$ run **Stopwatch Benchmarks** comparing hand-crafted arrays against STL!
