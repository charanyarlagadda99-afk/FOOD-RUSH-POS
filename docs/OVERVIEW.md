# FoodRush / RestoRush - Restaurant, Hotel & Cafe POS Engine

## 1. What FoodRush POS Is
**FoodRush POS** is an enterprise-grade Restaurant, Hotel & Cafe Point-of-Sale (POS) and operations backend engine engineered in high-performance C++ (CS Syllabus Modules I-X). Inspired by commercial POS architectures (such as Toast POS, Petpooja, and Lightspeed), it serves as a central hospitality management platform for dine-in operations within a hotel or restaurant building.

### Core Capabilities:
1. **Multi-Table Dine-In Management**: 12 tables mapped across 4 dining sections (Main Dining Hall, AC Family Lounge, Rooftop Terrace, Garden Lounge & Banquet) with real-time status tracking (`VACANT` vs `OCCUPIED`).
2. **Concurrent Multi-Order Kitchen KOT Board**: Real-time kitchen queue displaying multiple active dining table tickets simultaneously, powered by a custom FIFO `CircularQueue` and priority express lane.
3. **Itemized GST Invoicing & Thermal Receipt Printing**: Automatic computation of 5% GST (SGST+CGST), 5% Service Charge, coupon discounts, and generation of formatted monospaced thermal paper receipts (with string-reversal check-digit validation).
4. **Settled Past Bills Archive**: High-capacity historical invoice archive managed with `std::deque`, allowing immediate retrieval and thermal reprinting.
5. **Staff Attendance & Shift Register**: Daily clock-in/out duty register for 8 staff members across morning, evening, and full-day shifts.
6. **Weekly Sales Analytics Matrix**: $6 \text{ Kitchen Outlets} \times 7 \text{ Days}$ revenue matrix with row/column total aggregations and peak-sales slot detection.

---

## 2. Five-Person Capstone Architecture
FoodRush POS is partitioned cleanly across five specialized engineering modules:
1. **Catalog & Search Specialist (Member 1)**: 48-item multi-outlet catalog, tokenized case-insensitive search, and $O(L_1 \cdot L_2)$ dynamic programming Levenshtein distance "Did You Mean" engine.
2. **Table Order & Billing Specialist (Member 2)**: Dine-in table lifecycle, hand-crafted `ArrayStack` LIFO undo history for table modifications, and `std::map` discount promo engine.
3. **Kitchen KOT Pipeline Specialist (Member 3)**: Multi-ticket concurrent kitchen board, custom `CircularQueue` ring buffer with modulo arithmetic, and VIP/Express priority scheduling.
4. **Invoicing & Past Bills Archive Specialist (Member 4)**: Itemized GST & Service Charge calculation, check-digit invoice generation using string reversal, formatted ASCII thermal receipt formatter, and `std::deque` past bills archive.
5. **Staff Register & Matrix Analytics Specialist (Member 5)**: Staff duty attendance register, $6 \times 7$ 2D weekly revenue matrix with row/column traversals, and $O(N \log K)$ Top-K dish rankings.

---

## 3. The Dual-Surface Design Concept
FoodRush POS implements a strict dual-surface architectural pattern:
- **Commercial POS Surface (Default)**: Clean, editorial hospitality interface for cashiers, waitstaff, and chefs. Never exposes academic jargon or implementation details.
- **Examiner Viva Mode (Hot-key: 'V')**: Revealed only for examiners and university professors. Displays live internal engine memory buffers (`ArrayStack` capacity, `CircularQueue` ring pointers, 1D array stats), stopwatch benchmarks, and module badges on every action.

---

## 4. End-to-End Architectural Flow
```
[ Cashier / Waiter / Kitchen Browser ]
       │
       ▼  HTTP / REST API (JSON)
[ Node.js Bridge Server ] (server/server.js)
       │  - Persistent process supervisor
       │  - Zero-latency line protocol over stdin/stdout
       ▼
[ C++ POS Core Engine ] (foodrush_engine.exe)
   ├── Module I    : Financial Constants, GST/Service Rates, Type Conversion
   ├── Module II   : Command Dispatcher, Function Overloading
   ├── Module III  : 1D Arrays (Prices, Stock, Ratings, Staff Roster)
   ├── Module IV   : 2D Arrays (6x7 Weekly Sales Matrix, Table Layout)
   ├── Module V    : Strings (Levenshtein Fuzzy Match, Check-Digit Reversal)
   ├── Module VI   : Structures (Table, StaffMember, OrderTicket, BillReceipt)
   ├── Module VII  : Performance (Stopwatch Benchmarks, O(N log K) Ranking)
   ├── Module VIII : Hand-Crafted ArrayStack (Table Order Action Undo)
   ├── Module IX   : Hand-Crafted CircularQueue (Multi-Ticket Kitchen KOT)
   └── Module X    : STL Containers (std::deque, std::map, std::set, std::vector)
```

---

## 5. Why C++ is the Engine
1. **Deterministic Latency & Cache Locality**: Hotel and restaurant POS systems require sub-millisecond deterministic response times without garbage collection spikes. Contiguous memory arrays and fixed-capacity structs ensure instant throughput.
2. **Pedagogical Purity**: Directly implements foundational data structures (`ArrayStack`, `CircularQueue`, 2D Matrices) without hiding logic behind black-box libraries, making it effortless to defend in university viva voce examinations.
