# FoodRush - Project Limitations (Stated Honestly)

To maintain clarity, reproducibility, and pedagogical focus on core Computer Science data structures and systems programming, FoodRush deliberately scopes out certain commercial infrastructure layers. Here is an honest declaration of design boundaries and limitations:

---

### 1. In-Memory Storage vs. Persistent Relational Database
- **Current Behavior**: All restaurant catalogs (6 restaurants, 48 dishes), live orders, the 6×7 sales matrix, and stack/queue buffers reside directly in C++ program memory.
- **Limitation**: Terminating or restarting the C++ engine resets active cart sessions and new orders back to the initial seed state.
- **Rationale**: The focus of this second-year coursework project is low-level memory layout, pointer manipulation, contiguous cache locality, and algorithmic time complexity—not SQL database administration, migrations, or ORM overhead.

---

### 2. Mock Payments vs. Real Banking Gateways
- **Current Behavior**: Checkout computes exact line totals, taxes (8.25%), zone-based delivery surcharges, and coupon discounts, instantly generating confirmed orders.
- **Limitation**: No external payment gateways (Stripe, Razorpay, PayPal) or PCI-DSS tokenization pipelines are connected.
- **Rationale**: Payment gateways require external webhook listeners, merchant credentials, and internet connectivity, which distract from evaluating deterministic C++ data structure logic during an offline academic viva.

---

### 3. Symmetric 2D Distance Matrix vs. Live GPS / GIS Map Routing
- **Current Behavior**: Delivery distances and fees are calculated using a 5×5 symmetric lookup matrix between five urban zones (`ZONE_DISTANCE_MATRIX[5][5]`).
- **Limitation**: The system does not query live GPS coordinates, Google Maps Directions API, or OpenStreetMap road graph turn-by-turn routing.
- **Rationale**: Demonstrates 2D matrix indexing and $O(1)$ constant time lookup in C++ without external API rate limits or network latency.

---

### 4. Single-Active Session vs. Multi-Tenant User Authentication
- **Current Behavior**: The web bridge connects to a single active cart and session state on the C++ engine.
- **Limitation**: No user registration, password hashing (bcrypt), JWT sessions, or role-based multi-tenant isolation.
- **Rationale**: Avoids authentication boilerplate, keeping the codebase completely transparent, readable, and easy to explain line-by-line during university viva examinations.

---

### 5. Local Persistent Process vs. Serverless Cloud Architecture
- **Current Behavior**: Node.js manages one long-lived C++ engine child process via synchronized line-oriented pipes.
- **Limitation**: Not designed for stateless serverless deployment (e.g. AWS Lambda / Vercel Serverless Functions) where execution containers freeze between requests, which would wipe in-memory stack and circular queue state.
- **Rationale**: Maintaining a persistent native process provides ultra-fast IPC round-trip latency (~1.85 ms) and preserves deterministic queue states throughout the entire session.
