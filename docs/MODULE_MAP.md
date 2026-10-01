# FoodRush - Complete Module & Syllabus Map

This document maps all 10 Computer Science syllabus modules to their exact code locations, functions, real features, and step-by-step instructions for demonstrating them to professors during viva exams.

---

## Complete Module Mapping Table

| Module | Topic | File & Function | Real Feature in FoodRush | How to Demo Live in App |
| :--- | :--- | :--- | :--- | :--- |
| **Module I** | Basics: I/O, Constants, Data Types, Operators, Type Conversion | `src/models.hpp`<br>`src/main.cpp:calculateCartTotals()` | Bill computation, tax calculation (8.25%), bitwise dietary flags (Spicy=1, GlutenFree=2, ChefSpecial=4), distance multipliers, `static_cast` | Add items to cart. Observe subtotal, tax breakdown, and express surcharge recalculating. Notice bitwise tags (SPICY, GLUTEN FREE, CHEF SPECIAL) on dish cards. |
| **Module II** | Control Statements & Functions | `src/main.cpp:handleCommand()`<br>`src/main.cpp:formatCurrency()` | Command dispatcher switch/if-else ladder, while/for loops, function overloading (dollars vs cents), pass-by-reference guards | Run any command in Web or CLI menu. Notice the command dispatcher resolving commands and formatting currency strings. |
| **Module III** | 1D Arrays: Declare, Initialize, Pass to Functions, Sum, Min, Max, Search, Bubble Sort | `src/algorithms.hpp:calculateArraySum()`<br>`findArrayMax()`, `findArrayMin()`<br>`linearSearchArray()`, `bubbleSortArray()` | Ratings analysis, stock management, cheapest/priciest dish extraction, and Bubble Sorting of dish prices | Click **Sales Matrix & Stats** tab ➔ Click **Compute 1D Stats**. Watch average/min/max ratings computed along with Bubble Sorted price samples. |
| **Module IV** | 2D Arrays: Matrix Operations, Row Sums, Column Sums, Symmetric Lookups | `src/sales_matrix.hpp:SalesMatrixManager`<br>`ZONE_DISTANCE_MATRIX[5][5]`<br>`sales[4][7]` | 7-day restaurant sales matrix (row sums = restaurant weekly revenue, column sums = daily platform revenue), 5x5 zone distance lookup | Open **Sales Matrix & Stats** tab. View the full 4x7 grid with color-coded row/col sums and peak slot highlighted. Select different zones at checkout to see distance fees change. |
| **Module V** | Strings: Traversal, toLower, Contains Substring, Tokenizing, Frequency, Reversal | `src/algorithms.hpp:toLowerString()`<br>`stringContains()`, `tokenizeString()`<br>`reverseString()`, `analyzeCharFrequency()` | Real-time dish search (case-insensitive substring search), command parser tokenizing, palindrome coupon bonus verification | Open **Dish Search** tab. Type `ramen` or `burger`. In Cart, enter coupon `LEVEL` or `RACECAR` to trigger the palindrome string reversal bonus! |
| **Module VI** | Structures: Declarations, Arrays of Structs, Nested Structs, Struct Methods | `src/models.hpp:Address`<br>`MenuItem`, `Restaurant`<br>`CartItem`, `Order`, `Rider` | Domain entities: `Order` contains nested `Address` and array of `CartItem`. JSON serialization methods on structs. | Browse restaurants and dishes in **Restaurants & Menu** tab. Checkout to generate a full nested `Order` record visible in the **Kitchen & Riders** tab. |
| **Module VII** | Performance: Asymptotic Complexity, Timed Stopwatch Benchmarks | `src/algorithms.hpp:PerformanceBenchmark`<br>`runFullBenchmark()` | Microsecond stopwatch comparison: Linear Search $O(N)$ vs Binary Search $O(\log N)$ on 30,000 items; Bubble Sort $O(N^2)$ vs Introsort $O(N \log N)$ | Open **Benchmarks & STL** tab. Click **Run Benchmark**. View real microsecond timings and live speedup multipliers (e.g. 300x+ speedup). |
| **Module VIII** | Stack: Array-based Implementation, Push, Pop, Peek, LIFO Applications | `src/array_stack.hpp:ArrayStack`<br>`src/main.cpp:addToCartInternal()`<br>`src/main.cpp:performCartUndo()` | **Cart Action Undo Stack**: Every add/remove is pushed onto a hand-crafted array stack. Clicking **Undo** pops and reverses the action. **Recently Viewed Dishes Stack**. | In **Cart & Undo** tab: Add 2 items. Notice "Stack: 2". Click **UNDO ACTION**. Watch item pop off the stack and cart state revert. Open **Engine Inspector** to view stack frames! |
| **Module IX** | Queue: Array-based Circular Queue, Front/Rear Pointers, Priority Queue | `src/array_queue.hpp:CircularQueue`<br>`PriorityOrderQueue`<br>`src/main.cpp:gKitchenQueue` | **Kitchen FIFO Order Queue**: Orders enter circular buffer. **Express Priority Queue**: Express orders served first. **Rider Rotation Queue**: Dequeues and assigns available riders. | Checkout an order. Open **Kitchen & Riders** tab. Notice pending order in queue. Click **Cook Next Order** to dequeue. Assign and complete rider delivery. Inspect circular buffer slots in **Engine Inspector**. |
| **Module X** | STL: pair, vector, iterators, deque, stack, queue, set, map | `src/algorithms.hpp:STLManager`<br>`std::map` (coupons), `std::set` (cuisines)<br>`std::vector` (catalog), `std::deque` (orders)<br>`std::pair` (promos) | Coupon dictionary lookup, unique cuisine registry, completed order tracking, and live comparison of Hand-crafted vs STL structures | Open **Benchmarks & STL** tab. Click **Compare Hand-crafted vs STL**. View side-by-side push/pop and memory allocation comparisons over 50,000 operations! |

---

## Live Demonstration Script for Viva / Exam

1. **Step 1 - Restaurants & Menu [Modules I, III, VI]**:
   - Open `http://localhost:3050`. Point to the restaurant cards and dish list.
   - Explain: *"Dishes are stored in a fixed array of `MenuItem` structs [Module VI]. Notice the dietary badges computed via bitwise operators on `dietaryFlags` [Module I]."*

2. **Step 2 - Dish Search [Module V]**:
   - Navigate to **Dish Search**. Type `ramen` and press Enter.
   - Explain: *"Search uses custom string traversal, lowercase normalization, and substring pattern matching [Module V] to filter through all 24 items in $O(M \cdot K)$ time."*

3. **Step 3 - Cart & Stack Undo [Module VIII]**:
   - Add "Wood-Fired Margherita Pizza", then add "Authentic Espresso Tiramisu".
   - Switch to **Cart & Undo** tab. Point to the **UNDO ACTION** button showing `Stack: 2`.
   - Click **UNDO ACTION**. The tiramisu is popped from the hand-crafted `ArrayStack` and removed from the cart!
   - Navigate to **Engine Inspector** tab to show the professor the actual stack frames with `topIndex`.

4. **Step 4 - Couponding with Palindrome Bonus [Modules V, X]**:
   - In Cart, enter `LEVEL` or `RACECAR`. Click **Apply**.
   - Explain: *"The coupon algorithm checks `std::map` [Module X], then runs `reverseString()` [Module V]. Because 'LEVEL' is a palindrome, the C++ engine awards a bonus VIP discount!"*

5. **Step 5 - Checkout & Kitchen Circular Queue [Modules IV, IX]**:
   - Select Zone 2, enable **Express Priority**, and click **Complete Checkout**.
   - Explain: *"Delivery fee is calculated using the 5x5 zone distance matrix [Module IV]. The order struct is enqueued into our hand-crafted circular queue and priority queue [Module IX]."*
   - Switch to **Kitchen & Riders** tab. Click **Cook Next Order** (demonstrates circular queue dequeue), then **Assign Rider** (rotates circular rider fleet).

6. **Step 6 - 2D Sales Matrix [Module IV]**:
   - Open **Sales Matrix & Stats**. Point to the 4x7 table.
   - Explain: *"Each row sum calculates weekly restaurant revenue; each column sum calculates total daily platform revenue. The peak sales cell is identified via 2D matrix traversal."*

7. **Step 7 - Benchmarks & STL Comparison [Modules VII, X]**:
   - Open **Benchmarks & STL** tab. Click **Run Benchmark**. Show the 300x+ speedup of Binary Search over Linear Search, and Introsort over Bubble Sort.
   - Click **Compare Hand-crafted vs STL**. Show that our hand-crafted `ArrayStack` and `CircularQueue` execute in sub-microsecond time with zero heap allocations compared to dynamic deque adapters.
