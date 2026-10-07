# FoodRush POS - Complete Module & Syllabus Map

This document maps all 10 Computer Science syllabus modules and the 3 Enterprise POS Subsystems to their exact code locations, functions, real POS features, and step-by-step viva voce examination talking points.

---

## Complete Module Mapping Table

| Module | Topic | File & Function | Real Feature in FoodRush POS | How to Demo in Viva Mode |
| :--- | :--- | :--- | :--- | :--- |
| **Module I** | Basics: I/O, Constants, Data Types, Operators, Type Conversion | `src/models.hpp`<br>`src/main.cpp:calculateTableBillTotals()` | 5% GST & 5% Service Charge calculation, Express KOT surcharge (+₹50), bitwise dietary flags (`Spicy=1`, `GlutenFree=2`, `ChefSpecial=4`), `static_cast` paise to rupees conversion | Add dishes to a table order. Observe subtotal, 5% GST, 5% Service Charge, and net total. Notice bitwise badges (VEG, NON-VEG, CHEF SPECIAL) on dish cards. |
| **Module II** | Control Statements & Functions | `src/main.cpp:handleCommand()`<br>`src/main.cpp:formatCurrency()` | Command dispatcher switch/if-else ladder, while/for loops, function overloading (`formatCurrency(double)` vs `formatCurrency(int)`), pass-by-reference guards | Select tables and punch orders. Observe the C++ engine resolving commands and formatting currency in ₹. |
| **Module III** | 1D Arrays: Declare, Initialize, Pass to Functions, Sum, Min, Max, Search, Bubble Sort | `src/algorithms.hpp:calculateArraySum()`<br>`findArrayMax()`, `findArrayMin()`<br>`linearSearchArray()`, `bubbleSortArray()`<br>`src/main.cpp:gStaff` | Staff Attendance Roster (8 members), dish price arrays, cheapest/priciest dish extraction, and Bubble Sorting of dish prices across 48 items | Open Staff View (`#staff`). Toggle clock-in/out for staff members. In Viva Mode (`V`), view 1D array price statistics (min, max, average). |
| **Module IV** | 2D Arrays: Matrix Operations, Row Sums, Column Sums, Peak Slot Search | `src/sales_matrix.hpp:SalesMatrixManager`<br>`sales[6][7]`<br>`TABLE_CAPACITY_MATRIX[4][3]` | 6 Outlets × 7 Days weekly sales revenue matrix (row sums = outlet weekly revenue, column sums = daily platform revenue), 4×3 dining sections seating matrix | Open Sales Analytics (`#sales`). View the 6×7 revenue matrix with row/col sums and peak sales slot. |
| **Module V** | Strings: Levenshtein Distance, Substring Matching, Tokenizing, Reversal Check Digit | `src/algorithms.hpp:calculateLevenshteinDistance()`<br>`generateInvoiceCode()`, `validateInvoiceCode()`<br>`toLowerString()`, `reverseString()` | Fuzzy search "Did you mean?" suggestions (Levenshtein), verified tax invoices (`INV-5001-1`) via string reversal check-digit algorithm, coupon parsing | Type a typo like `biryani` or `piza` in the search bar to see the "Did you mean" suggestion chip. Settle a bill to see the check-digit verified invoice code. |
| **Module VI** | Structures: Declarations, Arrays of Structs, Nested Structs, Struct Methods | `src/models.hpp:Table`<br>`StaffMember`, `MenuItem`<br>`OrderTicket`, `BillReceipt` | Domain entities: `Table` with occupancy status, `OrderTicket` with nested line items, `BillReceipt` with financial totals and invoice codes. | Switch tables, punch KOT orders, settle bills, and view staff roster. Inspect structured JSON output. |
| **Module VII** | Performance: Asymptotic Complexity, Timed Stopwatch Benchmarks | `src/algorithms.hpp:PerformanceBenchmark`<br>`runFullBenchmark()` | Microsecond stopwatch comparison: Linear Search $O(N)$ vs Binary Search $O(\log N)$ on 30,000 items; Bubble Sort $O(N^2)$ vs Introsort $O(N \log N)$ on 2,500 elements | Press `V` to open Viva Mode. Click **Benchmarks**. Show measured microsecond execution times and 100x+ speedup multipliers. |
| **Module VIII** | Stack: Array-based Implementation, Push, Pop, Peek, LIFO Applications | `src/array_stack.hpp:ArrayStack`<br>`src/main.cpp:addToTableOrderInternal()`<br>`src/main.cpp:undoLastTableOrderAction()` | **Table Order Action Undo Stack**: Every add/remove is pushed onto a hand-crafted array stack. Clicking **Undo** pops and reverses the action. **Recently Viewed Dishes Stack**. | Add dishes to a table order. Click **Undo** to pop the last action. In Viva Mode (`V`), inspect the live **Memory Inspector** showing stack capacity and stored frames. |
| **Module IX** | Queue: Array-based Circular Queue, Front/Rear Pointers, Priority Queue | `src/array_queue.hpp:CircularQueue`<br>`PriorityOrderQueue`<br>`src/main.cpp:gKitchenQueue` | **Kitchen FIFO Order Queue**: Circular buffer with modulo arithmetic. **Priority Order Queue**: Express KOT VIP orders jump ahead of standard tickets. | Open Kitchen KOT Board (`#kitchen`). View concurrent active tickets. Advance stages (`ORDERED` ➔ `PREPARING` ➔ `SERVED`). In Viva Mode (`V`), inspect circular queue ring buffer indices. |
| **Module X** | STL: pair, vector, iterators, deque, stack, queue, set, map | `src/algorithms.hpp:STLManager`<br>`std::map` (coupons), `std::set` (cuisines)<br>`std::vector` (catalog), `std::deque` (past bills)<br>`std::pair` (promos) | Coupon dictionary lookup, unique cuisine registry, completed past bills archive (double-ended queue), and live comparison of Hand-crafted vs STL structures | In Viva Mode (`V`), click **Data Structures** to view side-by-side push/pop and enqueue/dequeue benchmarks across 50,000 operations. |
| **Feature 1** | Multi-Table Dine-In Management *(Member 2)* | `src/models.hpp:Table`<br>`src/main.cpp:gTables` | 12 dining tables across 4 sections, real-time `VACANT` / `OCCUPIED` states, table turnover lifecycle | Select tables 1-12, add dishes, settle bill to free table back to vacant. |
| **Feature 2** | Live Kitchen KOT Board *(Member 3)* | `src/array_queue.hpp:PriorityOrderQueue`<br>`src/main.cpp:gActiveOrders` | Multi-ticket concurrent kitchen display with live stage progression (`ORDERED` ➔ `PREPARING` ➔ `SERVED`) | View multiple concurrent orders simultaneously on the kitchen board. |
| **Feature 3** | Staff Attendance Register *(Member 5)* | `src/models.hpp:StaffMember`<br>`src/main.cpp:gStaff` | Daily duty roster for 8 staff members across morning, evening, and full-day shifts with hours tracking | Toggle staff attendance on duty / off duty. |

---

## Live Demonstration Script for Viva / Exam

1. **Step 1 - Dining Tables & Catalog [Modules I, VI]**:
   - Open `http://localhost:3000`. Show the POS Tables & Billing screen.
   - Point to the 12 dining table cards (Tables 2, 5, 8 occupied; others vacant).
   - Point to the 6 kitchen outlets (Grand Mughal, Dakshin, Trattoria Bella, Sakura Asian, Boulevard Grill, Royal Patisserie) and 48 menu items.
   - Explain: *"Dishes and tables are stored in fixed arrays of structs [Module VI]. Notice financial rates and bitwise dietary flags [Module I]."*

2. **Step 2 - Fuzzy Dish Search & Levenshtein [Module V]**:
   - Type `piza` or `biryany` in the search bar.
   - Click the "Did you mean pizza?" suggestion chip.
   - Explain: *"Search uses custom string tokenizing, substring search, and the Levenshtein distance dynamic programming matrix [Module V] to identify close matches."*

3. **Step 3 - Table Order & Hand-Crafted ArrayStack Undo [Module VIII]**:
   - Select Table 3. Add "Hyderabadi Chicken Dum Biryani", then add "Burani Garlic Raita".
   - Click the **Undo** button.
   - Explain: *"Every table modification is pushed onto our hand-crafted `ArrayStack` [Module VIII]. Clicking Undo executes a LIFO pop and reverts the exact state change."*
   - Press `V` to open **Viva Mode**. Point to the **Memory Inspector** showing the live `ArrayStack` frame count.

4. **Step 4 - Punch KOT & Circular Kitchen Queue [Module IX]**:
   - Check "Express KOT (+₹50)" and click **Punch KOT**.
   - Show that Table 3 is now `OCCUPIED` with an active order ID.
   - Navigate to **Kitchen KOT Board (`#kitchen`)**.
   - Point to the active tickets running simultaneously (Tables 2, 5, 8, 3).
   - Advance stage: click **Start Prep**, then **Mark Served**.
   - Explain: *"The kitchen queue uses a custom array-based `CircularQueue` with modulo arithmetic [Module IX] and priority fast-track for express orders."*

5. **Step 5 - Bill Generation & Thermal Receipt Printing [Modules I, V, X]**:
   - Return to Table 3. Click **Settle & Print Bill**.
   - Show the generated Tax Invoice & Thermal Receipt modal:
     - 5% GST (SGST+CGST) + 5% Service Charge calculated.
     - Verified invoice code (e.g. `INV-5003-7`) generated using string reversal check-digit [Module V].
     - Formatted ASCII thermal paper receipt layout.
     - Table 3 is automatically returned to `VACANT`.
   - Navigate to **Past Bills (`#bills`)** to show the invoice safely stored in the `std::deque` archive [Module X].

6. **Step 6 - Staff Attendance Register [Module III]**:
   - Navigate to **Staff & Shifts (`#staff`)**.
   - Point to the 8 staff members, roles, assigned shifts, and hours clocked today.
   - Toggle attendance for a staff member to show dynamic array updates [Module III].

7. **Step 7 - 6x7 Sales Revenue Matrix [Module IV]**:
   - Navigate to **Sales Analytics (`#sales`)**.
   - Point to the 6 Outlets × 7 Days weekly sales matrix.
   - Explain: *"Row sums calculate weekly revenue per outlet; column sums calculate daily revenue across the hotel/restaurant complex. Peak sales slot is located via 2D matrix max scan [Module IV]."*

8. **Step 8 - Microsecond Stopwatch Benchmarks & STL Comparison [Modules VII, X]**:
   - In Viva Mode (`V`), click **Benchmarks** to show measured linear search vs binary search ($O(N)$ vs $O(\log N)$) and bubble sort vs introsort ($O(N^2)$ vs $O(N \log N)$).
   - Click **Data Structures** to show that our hand-crafted `ArrayStack` and `CircularQueue` achieve zero allocation overhead and high cache locality compared to `std::stack` and `std::queue`.
