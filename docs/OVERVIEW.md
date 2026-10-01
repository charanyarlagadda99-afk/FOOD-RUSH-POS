# FoodRush - Project Overview

## 1. What FoodRush Is
**FoodRush** is a full-stack, high-performance food delivery system engineered for second-year Computer Science students to demonstrate core data structures, algorithms, and systems programming principles to university professors.

It simulates a real-world multi-restaurant ordering platform:
- **Customers** browse partner restaurants, search dishes in real-time, customize their cart, apply promotional coupons with palindrome bonuses, and checkout with express or standard delivery.
- **Kitchens** receive orders into a circular priority queue where express orders jump ahead. Chefs prepare orders in FIFO sequence.
- **Delivery Fleet** manages a circular rotation queue of delivery riders, assigning available drivers based on geographic zone distances and vehicle types.
- **Admin Dashboard** provides live 2D sales matrix analytics (row sums, column sums, peak slots) and 1D array statistics (sum, min, max, average, and bubble-sorted samples).
- **Engine Inspector** offers a live window into internal C++ memory structures (array stack frames, circular queue front/rear indices, and STL containers).

---

## 2. How FoodRush Works (The Architecture Flow)

```
[ Browser UI ]  (HTML5 / CSS3 / Vanilla JavaScript)
       │
       ▼  HTTP Fetch / REST API (JSON)
[ Node.js Bridge Server ]  (server/server.js)
       │
       ▼  stdin / stdout Line Protocol (1 Command Line In ➔ 1 JSON Line Out)
[ C++ High-Performance Engine ]  (foodrush_engine.exe)
  ├── Module I    : Basics (Constants, Type Conversion, Bitwise Flags)
  ├── Module II   : Control Statements & Functions (Command Dispatcher)
  ├── Module III  : 1D Arrays (Ratings, Prices, Stock, Sum/Min/Max/Sort)
  ├── Module IV   : 2D Arrays (4x7 Sales Matrix & 5x5 Zone Distance Matrix)
  ├── Module V    : Strings (Traversal, Pattern Match, Palindrome Reversal)
  ├── Module VI   : Structures (Address, MenuItem, Restaurant, Order, Rider)
  ├── Module VII  : Performance (Benchmarked O(N) vs O(log N), O(N²) vs O(N log N))
  ├── Module VIII : Hand-crafted Array Stack (Cart Undo & Recently Viewed)
  ├── Module IX   : Hand-crafted Circular Queue (Kitchen & Rider Dispatch)
  └── Module X    : STL Containers (std::map, set, vector, deque, stack, queue)
```

1. **User Action**: A user clicks "+ Add to Cart", "Undo", or "Cook Order" on the web page.
2. **Server Routing**: The Node.js server receives an HTTP request and formats it into a single-line engine command (e.g. `CART_ADD 101 2`).
3. **C++ Processing**: The persistent C++ engine reads the line from `stdin`, runs the deterministic data structure or algorithm logic, serializes the result to JSON, and writes it to `stdout`.
4. **Badge Tracking**: Every JSON line includes a `handled_by` field identifying which CS syllabus modules executed the command.
5. **UI Rendering**: The web frontend updates state and highlights the active module badges in real time.

---

## 3. Why C++ is the Backend Engine
1. **Deterministic Memory & Zero Garbage Collection**: Food delivery systems handle high-frequency concurrent events. In C++, memory layouts are predictable, contiguous, and free from garbage collection latency spikes.
2. **Pedagogical Purity**: Instead of relying on abstract black-box libraries, students implement the foundational data structures (**ArrayStack** and **CircularQueue**) from first principles using raw pointers and arrays.
3. **Microsecond Performance**: Benchmarks show hand-crafted contiguous array structures operate with zero heap overhead, beating general-purpose dynamic adapters while demonstrating real time and space trade-offs.
