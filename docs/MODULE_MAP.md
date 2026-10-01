# FoodRush - Complete Module & Syllabus Map

This document maps all 10 Computer Science syllabus modules to their exact code locations, functions, real features, and step-by-step instructions for demonstrating them to professors during viva voce exams.

---

## Complete Module Mapping Table

| Module | Topic | File & Function | Real Feature in FoodRush | How to Demo in Viva Mode |
| :--- | :--- | :--- | :--- | :--- |
| **Module I** | Basics: I/O, Constants, Data Types, Operators, Type Conversion | `src/models.hpp`<br>`src/main.cpp:calculateCartTotals()` | Bill computation, tax calculation (8.25%), bitwise dietary flags (Spicy=1, GlutenFree=2, ChefSpecial=4), distance multipliers, `static_cast` | Add items to cart. Observe subtotal, tax breakdown, and express surcharge. Notice bitwise tags (SPICY, GLUTEN FREE, CHEF SPECIAL) on dish cards. |
| **Module II** | Control Statements & Functions | `src/main.cpp:handleCommand()`<br>`src/main.cpp:formatCurrency()` | Command dispatcher switch/if-else ladder, while/for loops, function overloading (`formatCurrency(double)` vs `formatCurrency(int)`), pass-by-reference guards | Run any command in Web or CLI menu. Notice the command dispatcher resolving commands and formatting currency in ₹. |
| **Module III** | 1D Arrays: Declare, Initialize, Pass to Functions, Sum, Min, Max, Search, Bubble Sort | `src/algorithms.hpp:calculateArraySum()`<br>`findArrayMax()`, `findArrayMin()`<br>`linearSearchArray()`, `bubbleSortArray()` | Ratings analysis, stock management, cheapest/priciest dish extraction, and Bubble Sorting of dish prices across 48 items | Open Viva Mode (`V`) ➔ View Performance Benchmarks. Observe 1D array operations and price-sorted menu samples. |
| **Module IV** | 2D Arrays: Matrix Operations, Row Sums, Column Sums, Symmetric Lookups | `src/sales_matrix.hpp:SalesMatrixManager`<br>`ZONE_DISTANCE_MATRIX[5][5]`<br>`sales[6][7]` | 6×7 restaurant sales matrix (row sums = restaurant weekly revenue, column sums = daily platform revenue), 5×5 zone distance matrix for delivery fees | Open Admin View (`#admin`). View the 6×7 revenue grid with row/col sums and peak sales slot. In Cart, switch delivery zones to see $O(1)$ matrix distance fee recalculate. |
| **Module V** | Strings: Levenshtein Distance, Substring Matching, Tokenizing, Reversal Check Digit | `src/algorithms.hpp:levenshteinDistance()`<br>`generateTrackingCode()`, `verifyTrackingCheckDigit()`<br>`toLowerString()`, `reverseString()` | Fuzzy search "Did you mean?" suggestions (Levenshtein), tamper-proof tracking codes (`TRK-1001-8`) via string reversal weighted check digit, palindrome coupon check | Type a typo like `biryani` or `piza` in the search bar to see the "Did you mean?" suggestion chip. Place an order to see the check-digit tracking code. In Cart, enter coupon `LEVEL` or `RACECAR`. |
| **Module VI** | Structures: Declarations, Arrays of Structs, Nested Structs, Struct Methods | `src/models.hpp:Address`<br>`MenuItem`, `Restaurant`<br>`CartItem`, `Order`, `Rider` | Domain entities: `Order` contains nested `Address` and array of `CartItem`. JSON serialization methods on structs. | Browse restaurants and dishes. Checkout to generate a full nested `Order` record with delivery address and line items. |
| **Module VII** | Performance: Asymptotic Complexity, Timed Stopwatch Benchmarks | `src/algorithms.hpp:PerformanceBenchmark`<br>`runFullBenchmark()` | Microsecond stopwatch comparison: Linear Search $O(N)$ vs Binary Search $O(\log N)$ on 30,000 items; Bubble Sort $O(N^2)$ vs Introsort $O(N \log N)$ | Press `V` to open Viva Mode. Click **Run Benchmarks**. Show measured microsecond execution times and 300x+ speedup multipliers. |
| **Module VIII** | Stack: Array-based Implementation, Push, Pop, Peek, LIFO Applications | `src/array_stack.hpp:ArrayStack`<br>`src/main.cpp:addToCartInternal()`<br>`src/main.cpp:performCartUndo()` | **Cart Action Undo Stack**: Every add/remove is pushed onto a hand-crafted array stack. Clicking **Undo** pops and reverses the action. **Recently Viewed Dishes Stack**. | Add dishes to the cart. Click **Undo** to pop the last action. Open Viva Mode (`V`) to view the live **Stack Frames** inspector showing `topIndex` and stored actions. |
| **Module IX** | Queue: Array-based Circular Queue, Front/Rear Pointers, Priority Queue | `src/array_queue.hpp:CircularQueue`<br>`PriorityOrderQueue`<br>`src/main.cpp:gKitchenQueue` | **Kitchen FIFO Order Queue**: Circular buffer with modulo arithmetic. **Express Priority Queue**: Express orders jump ahead. **Rider Rotation Queue**: Dequeues and assigns available riders. | Place an order and track it. Click "Simulate Next Stage" to see it move through the kitchen circular queue. In Viva Mode (`V`), inspect front/rear ring buffer indices. |
| **Module X** | STL: pair, vector, iterators, deque, stack, queue, set, map | `src/algorithms.hpp:STLManager`<br>`std::map` (coupons), `std::set` (cuisines)<br>`std::vector` (catalog), `std::deque` (orders)<br>`std::pair` (promos) | Coupon dictionary lookup, unique cuisine registry, completed order tracking, and live comparison of Hand-crafted vs STL structures | In Viva Mode (`V`), click **Compare Hand-Crafted vs STL** to view side-by-side push/pop and allocation benchmarks across 50,000 operations. |

---

## Live Demonstration Script for Viva / Exam

1. **Step 1 - Customer Experience & Architecture [Modules I, VI]**:
   - Open `http://localhost:3000`. Show the clean, editorial food delivery storefront.
   - Point to the restaurant cards and dish catalog across 6 culinary styles (Biryani, Dosa, Italian, Ramen, Burgers, Desserts).
   - Explain: *"Dishes are stored in a fixed array of `MenuItem` structs [Module VI]. Notice the dietary badges computed via bitwise operators on `dietaryFlags` [Module I]."*

2. **Step 2 - Fuzzy Dish Search & Strings [Module V]**:
   - Type `piza` or `biryani` in the search bar.
   - Click the "Did you mean pizza?" suggestion chip.
   - Explain: *"Search uses custom string normalization, substring search, and the Levenshtein edit-distance dynamic programming algorithm [Module V] to find close matches."*

3. **Step 3 - Cart & Hand-Crafted Stack Undo [Module VIII]**:
   - Add "Dum Handi Mutton Biryani", then add "Mirchi Ka Salan".
   - Open the slide-over cart drawer. Click the **Undo** button.
   - Explain: *"Every mutation is encapsulated as a `CartAction` and pushed onto our hand-crafted `ArrayStack` [Module VIII]. Clicking Undo executes a LIFO pop and runs the inverse operation."*
   - Press `V` on the keyboard to open **Viva Mode**. Point to the **Memory & Data Structure Inspector** showing the live `ArrayStack` frame count and `topIndex`.

4. **Step 4 - 2D Distance Matrix & String Reversal Check Digit [Modules IV, V, VI]**:
   - Select delivery zone "East" and enable "Express Priority Delivery".
   - Point out the delivery fee calculated using the 5×5 symmetric zone distance matrix (`ZONE_DISTANCE_MATRIX[restaurantZone][customerZone]`) [Module IV].
   - Enter coupon `LEVEL` or `RACECAR`. The engine validates the coupon and reverses the string to detect the palindrome bonus!
   - Click **Complete Order**.
   - Show the generated tracking code (e.g. `TRK-1001-8`).
   - Explain: *"To ensure tamper-proof order identification, the check digit is calculated by reversing the order digits using our string reversal algorithm [Module V] with weighted modulo-9 verification."*

5. **Step 5 - Kitchen Circular Queue & Priority Dispatch [Module IX]**:
   - On the Order Tracking page, point to the live 4-stage pipeline.
   - Click **Simulate Next Stage**. The kitchen circular queue dequeues the order. If express was selected, it jumps ahead of standard orders in `PriorityOrderQueue` [Module IX].
   - In Viva Mode, point to the Circular Queue front and rear indices wrapping around using modulo arithmetic.

6. **Step 6 - Admin 6×7 Revenue Matrix [Module IV]**:
   - Navigate to `#admin` in the browser.
   - Point to the 6×7 sales matrix: 6 rows (restaurants) × 7 columns (days of the week).
   - Explain: *"Row sums yield total weekly restaurant revenue; column sums yield total daily platform revenue. The peak sales cell is identified via 2D matrix traversal."*

7. **Step 7 - Microsecond Performance Benchmarks & STL Comparison [Modules VII, X]**:
   - In Viva Mode, click **Run Benchmarks**.
   - Show the professor the live stopwatch comparison:
     - Linear Search ($O(N)$) vs Binary Search ($O(\log N)$) on 30,000 items showing a **300x+ speedup**.
     - Bubble Sort ($O(N^2)$) vs Introsort ($O(N \log N)$).
   - Click **Compare Hand-Crafted vs STL**.
   - Show that our hand-crafted `ArrayStack` and `CircularQueue` execute with zero dynamic heap allocations and higher cache locality compared to `std::stack` and `std::queue`.
