# 🍽️ FoodRush POS — Restaurant, Hotel & Cafe Management System

> **High-Performance C++ Core Engine (CS Syllabus Modules I-X) + Web POS Interface**  
> Designed for university computer science coursework, 5-person capstone engineering demonstrations, and viva voce examinations.

---

## 📌 Overview

**FoodRush POS** is an enterprise-grade hospitality point-of-sale and operations backend system engineered in high-performance C++. Inspired by real-world platforms like **Toast POS, Petpooja, and Lightspeed**, it powers the complete dine-in workflow within a restaurant or hotel complex:

1. **Table Management & Dine-In Ordering**: 12 dining tables across 4 dining sections (`Main Dining Hall`, `AC Family Lounge`, `Rooftop Terrace`, `Garden Lounge`) with live `VACANT` / `OCCUPIED` states.
2. **Concurrent Multi-Order Kitchen KOT Board**: Real-time kitchen queue displaying multiple active dining table tickets simultaneously, powered by a custom FIFO `CircularQueue` and priority express lane.
3. **Itemized GST Invoicing & Thermal Receipt Printing**: Automatic computation of 5% GST (SGST+CGST), 5% Service Charge, coupon discounts, and generation of formatted monospaced thermal paper receipts (with string-reversal check-digit validation).
4. **Settled Past Bills Archive**: High-capacity historical invoice archive managed with `std::deque`, allowing immediate retrieval and thermal reprinting.
5. **Staff Attendance & Shift Register**: Daily clock-in/out duty register for 8 staff members across morning, evening, and full-day shifts.
6. **Weekly Sales Analytics Matrix**: $6 \text{ Kitchen Outlets} \times 7 \text{ Days}$ revenue matrix with row/column total aggregations and peak-sales slot detection.

---

## 🏗️ Architecture & Data Flow

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

## 📚 Complete CS Syllabus Mapping

| Module | Topic | File & Function | Real Feature in FoodRush POS |
| :--- | :--- | :--- | :--- |
| **Module I** | Basics & Financial Types | `src/models.hpp`<br>`src/main.cpp:calculateTableBillTotals()` | 5% GST & 5% Service Charge calculation, Express KOT surcharge (+₹50), bitwise dietary flags (`Spicy=1`, `GlutenFree=2`, `ChefSpecial=4`), `static_cast` |
| **Module II** | Control Statements & Functions | `src/main.cpp:handleCommand()`<br>`src/main.cpp:formatCurrency()` | Command dispatcher switch/if-else ladder, while/for loops, function overloading (`formatCurrency(double)` vs `formatCurrency(int)`), pass-by-reference guards |
| **Module III** | 1D Arrays | `src/algorithms.hpp:calculateArraySum()`<br>`findArrayMax()`, `findArrayMin()`<br>`linearSearchArray()`, `bubbleSortArray()`<br>`src/main.cpp:gStaff` | Staff Attendance Roster (8 members), dish price arrays, cheapest/priciest dish extraction, and Bubble Sorting of dish prices across 48 items |
| **Module IV** | 2D Arrays | `src/sales_matrix.hpp:SalesMatrixManager`<br>`sales[6][7]`<br>`TABLE_CAPACITY_MATRIX[4][3]` | 6 Outlets × 7 Days weekly sales revenue matrix (row sums = outlet weekly revenue, column sums = daily platform revenue), 4×3 dining sections seating matrix |
| **Module V** | Strings & Algorithms | `src/algorithms.hpp:calculateLevenshteinDistance()`<br>`generateInvoiceCode()`, `validateInvoiceCode()`<br>`toLowerString()`, `reverseString()` | Fuzzy search "Did you mean?" suggestions (Levenshtein), verified tax invoices (`INV-5001-1`) via string reversal check-digit algorithm |
| **Module VI** | Structures | `src/models.hpp:Table`<br>`StaffMember`, `MenuItem`<br>`OrderTicket`, `BillReceipt` | Domain entities: `Table` with occupancy status, `OrderTicket` with nested line items, `BillReceipt` with financial totals and invoice codes |
| **Module VII** | Asymptotic Complexity | `src/algorithms.hpp:PerformanceBenchmark`<br>`runFullBenchmark()` | Microsecond stopwatch comparison: Linear Search $O(N)$ vs Binary Search $O(\log N)$ on 30,000 items; Bubble Sort $O(N^2)$ vs Introsort $O(N \log N)$ on 2,500 elements |
| **Module VIII** | Stack (Hand-Crafted) | `src/array_stack.hpp:ArrayStack`<br>`src/main.cpp:addToTableOrderInternal()`<br>`src/main.cpp:undoLastTableOrderAction()` | **Table Order Action Undo Stack**: Every add/remove is pushed onto a hand-crafted array stack. Clicking **Undo** pops and reverses the action. **Recently Viewed Dishes Stack** |
| **Module IX** | Queue (Hand-Crafted) | `src/array_queue.hpp:CircularQueue`<br>`PriorityOrderQueue`<br>`src/main.cpp:gKitchenQueue` | **Kitchen FIFO Order Queue**: Circular buffer with modulo arithmetic. **Priority Order Queue**: Express KOT VIP orders jump ahead of standard tickets |
| **Module X** | Standard Template Library | `src/algorithms.hpp:STLManager`<br>`std::map` (coupons), `std::set` (cuisines)<br>`std::vector` (catalog), `std::deque` (past bills)<br>`std::pair` (promos) | Coupon dictionary lookup, unique cuisine registry, completed past bills archive (double-ended queue), and live comparison of Hand-crafted vs STL structures |

---

## 👥 5-Person Capstone Division of Labor

- **Member 1 (Catalog & Search Specialist)**: Unified 48-item catalog, case-insensitive substring search, Levenshtein distance matrix ($O(L_1 \cdot L_2)$) "Did You Mean" engine.
- **Member 2 (Table Order & State Specialist)**: 12 dining tables state management, hand-crafted `ArrayStack` LIFO undo history for table modifications, `std::map` discount promo engine.
- **Member 3 (Kitchen Pipeline Specialist)**: Concurrent multi-order kitchen board, custom `CircularQueue` ring buffer with modulo arithmetic, VIP/Express priority scheduling.
- **Member 4 (Invoicing & Printing Specialist)**: Itemized GST & Service Charge calculation, check-digit invoice generation using string reversal, formatted ASCII thermal receipt formatter, and `std::deque` past bills archive.
- **Member 5 (Staff & Matrix Analytics Specialist)**: Staff attendance register, $6 \times 7$ 2D weekly revenue matrix with row/column traversals, $O(N \log K)$ Top-K dish rankings, and stopwatch benchmarks.

---

## 🚀 Quick Start Guide

### Prerequisites
- `g++` (supporting C++17)
- `Node.js` (v18+)

### 1. Compile the C++ Engine
```bash
g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe
```

### 2. Run Integration Tests
```bash
node test/smoke_test.js
```
*Executes all 24 automated smoke tests covering all 10 syllabus modules and POS workflows.*

### 3. Start the Web POS Server
```bash
npm start
```
*Launches the persistent bridge server at **http://localhost:3000**.*

---

## 🧾 Sample Thermal Receipt Output

```
========================================
       THE GRAND REGENCY HOTEL & POS    
       Official Tax Invoice & Receipt   
========================================
Invoice No:  INV-5001-1
Bill ID:     #5001
Date & Time: 12:30 PM
Table:       Table 1 (Family) (Main Dining Hall)
Guest Name:  Rajesh Verma
Server:      Vikram Malhotra
----------------------------------------
ITEM                     QTY   PRICE   TOTAL
----------------------------------------
Hyderabadi Chicken..    2  320.00 640.00
Zafrani Matka Phirni    2  130.00 260.00
----------------------------------------
Item Subtotal:                    900.00
Promo Discount:                       -90.00
GST (5% SGST+CGST):                40.50
Service Charge (5%):               40.50
========================================
NET TOTAL PAYABLE:          INR   891.00
Payment Method: UPI
========================================
     THANK YOU FOR DINING WITH US!      
  GSTIN: 29AABCU9603R1ZM | Bangalore   
========================================
```

---

## 📄 License
MIT License. Built for Computer Science education and demonstration.
