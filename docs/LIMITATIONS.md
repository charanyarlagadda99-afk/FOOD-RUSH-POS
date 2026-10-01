# FoodRush - Project Limitations (Stated Honestly)

To maintain clarity and pedagogical focus on core Computer Science data structures and algorithms, FoodRush deliberately scopes out certain enterprise production systems. Here is an honest declaration of what the project does not do:

### 1. In-Memory Storage vs. Persistent Relational Database
- **Current Behavior:** All seed restaurants, dishes, live orders, sales matrices, and stack/queue buffers reside in C++ program memory.
- **Limitation:** Restarting the C++ engine resets active cart sessions and new orders back to the initial seed state.
- **Rationale:** Focuses on pointer manipulation, array memory layouts, and algorithmic time complexity rather than database ORMs or SQL syntax.

### 2. Mock Payments vs. Real Banking Gateways
- **Current Behavior:** Checkout computes exact line totals, taxes (8.25%), zone-based delivery surcharges, and coupon discounts, instantly validating orders.
- **Limitation:** No real payment gateway integration (Stripe, PayPal, credit card tokenization) or PCI-DSS compliance mechanisms are implemented.
- **Rationale:** Financial payment processing requires external webhooks, merchant accounts, and SSL infrastructure irrelevant to data structure coursework.

### 3. Symmetric 2D Distance Matrix vs. Live GPS / GIS Map Routing
- **Current Behavior:** Delivery distances and fees are calculated using a 5x5 symmetric lookup matrix between five predefined urban zones (`ZONE_DISTANCE_MATRIX[5][5]`).
- **Limitation:** Does not query live GPS coordinates, Google Maps API, or OpenStreetMap road graph turn-by-turn routing.
- **Rationale:** Demonstrates 2D matrix indexing and $O(1)$ constant time lookup in C++ without external API keys or network latency.

### 4. Single-User Session vs. Multi-Tenant User Authentication
- **Current Behavior:** The frontend interacts with a single active cart and session state on the C++ engine.
- **Limitation:** No user registration, password hashing (bcrypt), JWT sessions, or concurrent multi-tenant role-based access control.
- **Rationale:** Avoids security and session management boilerplate, keeping the codebase transparent, readable, and easy to explain line-by-line during professor viva exams.

### 5. Single-Process Concurrency vs. Distributed Microservices
- **Current Behavior:** Node.js manages one persistent C++ engine child process via synchronized line-oriented pipes.
- **Limitation:** Not designed for multi-server horizontal load balancing or distributed message brokers (Kafka/RabbitMQ).
- **Rationale:** Guarantees deterministic order execution and makes debugging straightforward for students.
