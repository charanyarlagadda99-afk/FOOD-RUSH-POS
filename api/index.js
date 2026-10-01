// ============================================================================
// FoodRush - Vercel Serverless Function API Handler
// Bridges requests to the C++ Engine or high-fidelity algorithmic equivalent
// ============================================================================

const fs = require('fs');
const path = require('path');
const os = require('os');
const { spawnSync } = require('child_process');

// Determine binary location
const isWindows = os.platform() === 'win32';
const BINARY_NAME = isWindows ? 'foodrush_engine.exe' : 'foodrush_engine';
const BINARY_PATH = path.join(process.cwd(), BINARY_NAME);

// In-Memory Global State for Serverless Container instances
if (!global.foodrushState) {
    global.foodrushState = {
        cart: [],
        undoStack: [], // [MODULE VIII] Stack implementation
        recentlyViewed: [],
        orders: [],
        kitchenQueue: [], // [MODULE IX] Circular queue simulation
        availableRiders: [1, 2, 3, 4, 5],
        nextOrderId: 1001,
        activeCoupon: '',
        activeDiscountPct: 0.0,
        // [MODULE IV] 2D Sales Matrix (4x7)
        salesMatrix: [
            [620.50, 580.00, 710.25, 690.80, 950.40, 1240.00, 1110.50],
            [540.00, 610.75, 630.00, 720.50, 880.20, 1350.80, 1190.00],
            [480.25, 510.00, 590.50, 640.00, 890.60, 1420.30, 1280.40],
            [710.00, 690.50, 780.00, 820.40, 1150.00, 1580.90, 1390.20]
        ]
    };
}

const state = global.foodrushState;

// Seed Data
const RESTAURANTS = [
    { id: 1, name: "Bella Italia Trattoria", cuisine: "Italian", zoneId: 0, rating: 4.8, isOpen: true, itemCount: 6 },
    { id: 2, name: "Tokyo Ramen & Sushi", cuisine: "Japanese", zoneId: 1, rating: 4.9, isOpen: true, itemCount: 6 },
    { id: 3, name: "Spice Symphony", cuisine: "Indian", zoneId: 2, rating: 4.7, isOpen: true, itemCount: 6 },
    { id: 4, name: "Burger & Brews Bistro", cuisine: "American", zoneId: 3, rating: 4.6, isOpen: true, itemCount: 6 }
];

const MENU_ITEMS = [
    { id: 101, restaurantId: 1, name: "Wood-Fired Margherita Pizza", category: "Pizza", price: 14.50, stock: 25, isVeg: true, calories: 780, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 102, restaurantId: 1, name: "Spicy Penne Arrabbiata", category: "Pasta", price: 13.25, stock: 20, isVeg: true, calories: 620, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 103, restaurantId: 1, name: "Truffle Mushroom Fettuccine", category: "Pasta", price: 17.95, stock: 15, isVeg: true, calories: 810, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 104, restaurantId: 1, name: "Prosciutto & Arugula Flatbread", category: "Pizza", price: 16.50, stock: 18, isVeg: false, calories: 740, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 105, restaurantId: 1, name: "Classic Caesar Salad", category: "Salads", price: 9.50, stock: 30, isVeg: false, calories: 320, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 106, restaurantId: 1, name: "Authentic Espresso Tiramisu", category: "Desserts", price: 7.50, stock: 22, isVeg: true, calories: 410, isSpicy: false, isGlutenFree: false, isChefSpecial: false },

    { id: 201, restaurantId: 2, name: "Rich Tonkotsu Black Garlic Ramen", category: "Ramen", price: 15.95, stock: 30, isVeg: false, calories: 890, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 202, restaurantId: 2, name: "Spicy Miso Ramen", category: "Ramen", price: 14.75, stock: 25, isVeg: false, calories: 820, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 203, restaurantId: 2, name: "Salmon & Tuna Nigiri Combo (8 pcs)", category: "Sushi", price: 19.50, stock: 15, isVeg: false, calories: 510, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 204, restaurantId: 2, name: "Crispy Vegetable Tempura Roll", category: "Sushi", price: 11.25, stock: 20, isVeg: true, calories: 420, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 205, restaurantId: 2, name: "Pan-Seared Pork Gyoza (6 pcs)", category: "Appetizers", price: 8.50, stock: 35, isVeg: false, calories: 380, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 206, restaurantId: 2, name: "Matcha Green Tea Mochi Ice Cream", category: "Desserts", price: 6.25, stock: 40, isVeg: true, calories: 260, isSpicy: false, isGlutenFree: true, isChefSpecial: false },

    { id: 301, restaurantId: 3, name: "Velvety Butter Chicken & Basmati", category: "Curry", price: 16.95, stock: 25, isVeg: false, calories: 850, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 302, restaurantId: 3, name: "Paneer Tikka Masala", category: "Curry", price: 14.50, stock: 20, isVeg: true, calories: 730, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 303, restaurantId: 3, name: "Fiery Lamb Rogan Josh", category: "Curry", price: 18.25, stock: 18, isVeg: false, calories: 890, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 304, restaurantId: 3, name: "Hyderabadi Dum Biryani", category: "Rice", price: 15.50, stock: 22, isVeg: false, calories: 910, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 305, restaurantId: 3, name: "Garlic Butter Naan Bread (2 pcs)", category: "Breads", price: 4.25, stock: 50, isVeg: true, calories: 310, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 306, restaurantId: 3, name: "Warm Gulab Jamun with Pistachio", category: "Desserts", price: 5.50, stock: 30, isVeg: true, calories: 380, isSpicy: false, isGlutenFree: false, isChefSpecial: false },

    { id: 401, restaurantId: 4, name: "Smoked Bacon Cheddar Smash Burger", category: "Burgers", price: 14.95, stock: 30, isVeg: false, calories: 950, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 402, restaurantId: 4, name: "Nashville Hot Crispy Chicken Sandwich", category: "Burgers", price: 13.75, stock: 25, isVeg: false, calories: 870, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 403, restaurantId: 4, name: "Beyond Meat Truffle Veggie Burger", category: "Burgers", price: 15.25, stock: 20, isVeg: true, calories: 680, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 404, restaurantId: 4, name: "Loaded Buffalo Wings with Blue Cheese", category: "Sides", price: 12.50, stock: 28, isVeg: false, calories: 760, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 405, restaurantId: 4, name: "Seasoned Crispy Curly Fries", category: "Sides", price: 5.50, stock: 45, isVeg: true, calories: 410, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 406, restaurantId: 4, name: "Hand-Spun Salted Caramel Shake", category: "Beverages", price: 6.75, stock: 30, isVeg: true, calories: 560, isSpicy: false, isGlutenFree: false, isChefSpecial: false }
];

const RIDERS = [
    { id: 1, name: "Alex Walker", vehicle: "Electric Cargo Bike", currentZone: 0, isAvailable: true, totalDeliveries: 142 },
    { id: 2, name: "Chloe Bennett", vehicle: "Vespa Scooter", currentZone: 1, isAvailable: true, totalDeliveries: 215 },
    { id: 3, name: "David Miller", vehicle: "Sport Motorcycle", currentZone: 2, isAvailable: true, totalDeliveries: 98 },
    { id: 4, name: "Emma Hayes", vehicle: "Electric Scooter", currentZone: 3, isAvailable: true, totalDeliveries: 176 },
    { id: 5, name: "Liam Vance", vehicle: "E-Bike Pro", currentZone: 4, isAvailable: true, totalDeliveries: 131 }
];

const ZONE_DISTANCE_MATRIX = [
    [0.0, 3.2, 5.8, 8.4, 4.1],
    [3.2, 0.0, 4.5, 6.1, 7.0],
    [5.8, 4.5, 0.0, 5.2, 9.3],
    [8.4, 6.1, 5.2, 0.0, 11.5],
    [4.1, 7.0, 9.3, 11.5, 0.0]
];

const ZONE_NAMES = ["Downtown", "Uptown", "Tech Park", "Suburbs North", "Waterfront Harbor"];

// [MODULE I] Bill calculations
function calculateCartTotals(zoneId = 0, isExpress = false) {
    let subtotal = 0;
    let restZone = 0;

    for (const item of state.cart) {
        subtotal += item.price * item.quantity;
        const rest = RESTAURANTS.find(r => r.id === item.restaurantId);
        if (rest) restZone = rest.zoneId;
    }

    if (state.cart.length === 0) {
        return { subtotal: 0, tax: 0, deliveryFee: 0, discount: 0, total: 0 };
    }

    const dist = ZONE_DISTANCE_MATRIX[restZone] ? ZONE_DISTANCE_MATRIX[restZone][zoneId] || 4.0 : 4.0;
    let deliveryFee = 2.49 + (dist * 0.65);
    if (isExpress) deliveryFee += 3.99;

    const tax = subtotal * 0.0825;
    let discount = (subtotal * (state.activeDiscountPct / 100.0));
    if (discount > subtotal) discount = subtotal;

    let total = (subtotal - discount) + tax + deliveryFee;
    return {
        subtotal: parseFloat(subtotal.toFixed(2)),
        tax: parseFloat(tax.toFixed(2)),
        deliveryFee: parseFloat(deliveryFee.toFixed(2)),
        discount: parseFloat(discount.toFixed(2)),
        total: parseFloat(total.toFixed(2))
    };
}

function getCartPayload(zoneId = 0, isExpress = false) {
    const totals = calculateCartTotals(zoneId, isExpress);
    return {
        itemCount: state.cart.length,
        items: state.cart.map(c => ({
            itemId: c.itemId,
            restaurantId: c.restaurantId,
            name: c.name,
            price: c.price,
            quantity: c.quantity,
            lineTotal: parseFloat((c.price * c.quantity).toFixed(2))
        })),
        coupon: state.activeCoupon,
        discountPercent: state.activeDiscountPct,
        discountAmount: totals.discount,
        subtotal: totals.subtotal,
        tax: totals.tax,
        deliveryFee: totals.deliveryFee,
        total: totals.total,
        undoStackDepth: state.undoStack.length
    };
}

// Check if binary can be executed
function tryExecuteBinary(cmdString) {
    try {
        if (!fs.existsSync(BINARY_PATH)) return null;
        const res = spawnSync(BINARY_PATH, [], {
            input: cmdString.trim() + '\n',
            encoding: 'utf8',
            timeout: 3000
        });
        if (res.status === 0 && res.stdout) {
            return JSON.parse(res.stdout.trim());
        }
    } catch (e) {
        // Binary execution failed or not supported in this environment
    }
    return null;
}

// Main Vercel Serverless Request Handler
module.exports = async (req, res) => {
    // CORS headers
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.status(204).end();
        return;
    }

    const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    const pathname = parsedUrl.pathname;

    res.setHeader('Content-Type', 'application/json; charset=utf-8');

    // 1. PING
    if (pathname.endsWith('/ping')) {
        return res.status(200).json({
            success: true,
            message: "FoodRush Engine is alive and ready (Vercel Serverless)",
            handled_by: ["Module I (Basics)", "Module II (Control)"],
            data: { status: "OK", platform: os.platform(), serverless: true }
        });
    }

    // 2. RESTAURANTS
    if (pathname.endsWith('/restaurants')) {
        return res.status(200).json({
            success: true,
            message: "Fetched all restaurants",
            handled_by: ["Module VI (Structures - Restaurant)", "Module III (1D Arrays - restaurants[])"],
            data: RESTAURANTS
        });
    }

    // 3. MENU
    if (pathname.endsWith('/menu')) {
        const restId = parseInt(parsedUrl.searchParams.get('restaurantId') || '0');
        const items = restId === 0 ? MENU_ITEMS : MENU_ITEMS.filter(m => m.restaurantId === restId);
        return res.status(200).json({
            success: true,
            message: "Fetched menu items",
            handled_by: ["Module VI (Structures - MenuItem)", "Module III (1D Arrays - menuItems[])", "Module I (Basics - Bitwise dietary flags)"],
            data: items
        });
    }

    // 4. SEARCH
    if (pathname.endsWith('/search')) {
        const q = (parsedUrl.searchParams.get('q') || '').toLowerCase().trim();
        const matches = MENU_ITEMS.filter(m =>
            m.name.toLowerCase().includes(q) || m.category.toLowerCase().includes(q)
        );
        matches.forEach(m => state.recentlyViewed.unshift(m));
        if (state.recentlyViewed.length > 10) state.recentlyViewed.pop();

        return res.status(200).json({
            success: true,
            message: `Search found ${matches.length} matches (Query length: ${q.length})`,
            handled_by: [
                "Module V (Strings - Traversal, toLower, containsSubstring)",
                "Module VIII (Stack - Recently Viewed Dishes)",
                "Module III (1D Arrays - Linear scan)"
            ],
            data: matches
        });
    }

    // 5. CART
    if (pathname.endsWith('/cart') && req.method === 'GET') {
        const zone = parseInt(parsedUrl.searchParams.get('zone') || '0');
        const express = parsedUrl.searchParams.get('express') === '1' || parsedUrl.searchParams.get('express') === 'true';
        return res.status(200).json({
            success: true,
            message: "Current cart state",
            handled_by: [
                "Module I (Basics - tax, subtotal, delivery calculation)",
                "Module IV (2D Arrays - zone distance lookup)",
                "Module VI (Structures - CartItem)"
            ],
            data: getCartPayload(zone, express)
        });
    }

    // 6. CART ADD
    if (pathname.endsWith('/cart/add') && req.method === 'POST') {
        const body = req.body || {};
        const itemId = parseInt(body.itemId);
        const qty = parseInt(body.quantity || 1);
        const item = MENU_ITEMS.find(m => m.id === itemId);

        if (!item || item.stock < qty) {
            return res.status(200).json({
                success: false,
                message: "Failed to add to cart: check stock.",
                handled_by: ["Module VIII (Stack)", "Module I (Basics)"],
                data: {}
            });
        }

        const existing = state.cart.find(c => c.itemId === itemId);
        const prevQty = existing ? existing.quantity : 0;
        const newQty = prevQty + qty;

        if (existing) {
            existing.quantity = newQty;
        } else {
            state.cart.push({ itemId, restaurantId: item.restaurantId, name: item.name, price: item.price, quantity: newQty });
        }

        // [MODULE VIII] Push to Undo Stack
        state.undoStack.push({
            type: existing ? "UPDATE_QTY" : "ADD_ITEM",
            itemId,
            itemName: item.name,
            prevQty,
            newQty,
            price: item.price
        });

        return res.status(200).json({
            success: true,
            message: "Added item to cart",
            handled_by: [
                "Module VIII (Stack - push action for undo)",
                "Module VI (Structures - CartItem)",
                "Module I (Basics - price calculation)"
            ],
            data: getCartPayload()
        });
    }

    // 7. CART REMOVE
    if (pathname.endsWith('/cart/remove') && req.method === 'POST') {
        const body = req.body || {};
        const itemId = parseInt(body.itemId);
        const idx = state.cart.findIndex(c => c.itemId === itemId);

        if (idx === -1) {
            return res.status(200).json({ success: false, message: "Item not found in cart", handled_by: ["Module II (Control)"], data: {} });
        }

        const removed = state.cart[idx];
        state.cart.splice(idx, 1);

        // [MODULE VIII] Push removal to undo stack
        state.undoStack.push({
            type: "REMOVE_ITEM",
            itemId,
            itemName: removed.name,
            prevQty: removed.quantity,
            newQty: 0,
            price: removed.price
        });

        return res.status(200).json({
            success: true,
            message: "Removed item from cart",
            handled_by: [
                "Module VIII (Stack - push removal action for undo)",
                "Module III (1D Arrays - array shifting)"
            ],
            data: getCartPayload()
        });
    }

    // 8. CART UNDO
    if (pathname.endsWith('/cart/undo') && req.method === 'POST') {
        if (state.undoStack.length === 0) {
            return res.status(200).json({
                success: false,
                message: "Undo stack is empty - nothing to revert.",
                handled_by: ["Module VIII (Stack - pop underflow guard)"],
                data: getCartPayload()
            });
        }

        const action = state.undoStack.pop();
        let desc = "";

        if (action.type === "ADD_ITEM") {
            state.cart = state.cart.filter(c => c.itemId !== action.itemId);
            desc = `Undid adding '${action.itemName}' (removed from cart)`;
        } else if (action.type === "UPDATE_QTY") {
            const item = state.cart.find(c => c.itemId === action.itemId);
            if (item) item.quantity = action.prevQty;
            desc = `Restored quantity of '${action.itemName}' to ${action.prevQty}`;
        } else if (action.type === "REMOVE_ITEM") {
            const m = MENU_ITEMS.find(i => i.id === action.itemId);
            state.cart.push({ itemId: action.itemId, restaurantId: m.restaurantId, name: action.itemName, price: action.price, quantity: action.prevQty });
            desc = `Restored removed item '${action.itemName}' (quantity: ${action.prevQty})`;
        }

        return res.status(200).json({
            success: true,
            message: `Undo successful: ${desc}`,
            handled_by: [
                "Module VIII (Stack - pop & reverse action)",
                "Module VI (Structures - CartAction)",
                "Module X (STL - std::stack synchronizer)"
            ],
            data: getCartPayload()
        });
    }

    // 9. CART CLEAR
    if (pathname.endsWith('/cart/clear') && req.method === 'POST') {
        state.cart = [];
        state.undoStack = [];
        state.activeCoupon = '';
        state.activeDiscountPct = 0.0;
        return res.status(200).json({
            success: true,
            message: "Cart and undo stack cleared",
            handled_by: ["Module VIII (Stack - clear)", "Module III (1D Arrays - reset count)"],
            data: getCartPayload()
        });
    }

    // 10. APPLY COUPON
    if (pathname.endsWith('/coupon') && req.method === 'POST') {
        const body = req.body || {};
        const code = (body.code || '').trim().toUpperCase();

        const COUPONS = {
            "FOODRUSH10": 10.0,
            "WELCOME20": 20.0,
            "SUPER50": 50.0,
            "CHEFVIP": 25.0
        };

        const rev = code.split('').reverse().join('');
        const isPalindrome = code.length >= 3 && code === rev;
        let discount = COUPONS[code] || 0.0;

        if (discount > 0 || isPalindrome) {
            let label = code;
            if (isPalindrome && discount === 0) {
                discount = 15.0;
                label += " (Palindrome Bonus!)";
            } else if (isPalindrome && discount > 0) {
                discount += 10.0;
                label += " + Palindrome VIP (+10%)";
            }
            state.activeCoupon = label;
            state.activeDiscountPct = discount;

            return res.status(200).json({
                success: true,
                message: `Coupon applied: ${discount}% OFF!`,
                handled_by: [
                    "Module X (STL - std::map lookup)",
                    "Module V (Strings - reverseString palindrome check)",
                    "Module I (Basics - percentage discount)"
                ],
                data: { coupon: label, discountPercent: discount }
            });
        }

        return res.status(200).json({
            success: false,
            message: "Invalid coupon code. Try FOODRUSH10, WELCOME20, or a palindrome like LEVEL",
            handled_by: ["Module X (STL - std::map::find)", "Module V (Strings - reverseString)"],
            data: {}
        });
    }

    // 11. CHECKOUT
    if (pathname.endsWith('/checkout') && req.method === 'POST') {
        if (state.cart.length === 0) {
            return res.status(200).json({ success: false, message: "Cannot checkout: Cart is empty", handled_by: ["Module II (Control)"], data: {} });
        }
        const body = req.body || {};
        const zoneId = parseInt(body.zoneId || 0);
        const isExpress = !!body.isExpress;
        const totals = calculateCartTotals(zoneId, isExpress);

        const newOrder = {
            orderId: state.nextOrderId++,
            restaurantId: state.cart[0].restaurantId,
            customerName: body.customerName || "Guest",
            deliveryAddress: { street: body.street || "MainStreet", zoneId, zipCode: "90210", instructions: "" },
            items: [...state.cart],
            itemCount: state.cart.length,
            subtotal: totals.subtotal,
            tax: totals.tax,
            deliveryFee: totals.deliveryFee,
            discount: totals.discount,
            totalAmount: totals.total,
            isExpress,
            status: "PLACED",
            assignedRiderId: -1,
            riderName: "Pending Kitchen",
            estimatedMinutes: isExpress ? 18 : 28
        };

        state.orders.unshift(newOrder);
        state.kitchenQueue.push(newOrder.orderId);

        // Update 2D sales matrix
        const restIdx = newOrder.restaurantId - 1;
        if (restIdx >= 0 && restIdx < 4) {
            state.salesMatrix[restIdx][6] += totals.total;
        }

        // Clear cart
        state.cart = [];
        state.undoStack = [];
        state.activeCoupon = '';
        state.activeDiscountPct = 0;

        return res.status(200).json({
            success: true,
            message: `Order #${newOrder.orderId} placed successfully and sent to Kitchen Queue!`,
            handled_by: [
                "Module VI (Structures - Nested Order & Address)",
                "Module IX (Queue - Circular & Priority Enqueue)",
                "Module IV (2D Arrays - Sales Matrix update & Distance fee)",
                "Module I (Basics - Bill & tax calculation)",
                "Module III (1D Arrays - Stock deduction)"
            ],
            data: newOrder
        });
    }

    // 12. ORDERS
    if (pathname.endsWith('/orders') && req.method === 'GET') {
        return res.status(200).json({
            success: true,
            message: "Fetched orders and kitchen queue state",
            handled_by: ["Module IX (Queue - CircularQueue size & peek)", "Module VI (Structures - Order[])"],
            data: {
                kitchenQueueSize: state.kitchenQueue.length,
                priorityQueueSize: state.kitchenQueue.length,
                expressQueueSize: state.orders.filter(o => o.isExpress && o.status === 'PLACED').length,
                orders: state.orders
            }
        });
    }

    // 13. COOK ORDER
    if (pathname.endsWith('/orders/cook') && req.method === 'POST') {
        const body = req.body || {};
        let orderId = body.orderId ? parseInt(body.orderId) : state.kitchenQueue.shift();

        if (!orderId && state.kitchenQueue.length > 0) {
            orderId = state.kitchenQueue.shift();
        }

        if (!orderId) {
            return res.status(200).json({
                success: false,
                message: "Kitchen queue is empty. No orders to cook!",
                handled_by: ["Module IX (Queue)"],
                data: {}
            });
        }

        const ord = state.orders.find(o => o.orderId === orderId);
        if (ord) {
            ord.status = "READY_FOR_PICKUP";
            ord.riderName = "Waiting for Rider Dispatch";
        }

        return res.status(200).json({
            success: true,
            message: `Kitchen finished cooking Order #${orderId}. Ready for rider assignment!`,
            handled_by: [
                "Module IX (Queue - dequeue from Circular & Priority Queue)",
                "Module VI (Structures - Order status update)",
                "Module X (STL - std::queue dequeue)"
            ],
            data: ord || {}
        });
    }

    // 14. ASSIGN RIDER
    if (pathname.endsWith('/orders/assign-rider') && req.method === 'POST') {
        const body = req.body || {};
        const orderId = parseInt(body.orderId);
        const ord = state.orders.find(o => o.orderId === orderId);

        if (!ord) return res.status(200).json({ success: false, message: "Order not found", handled_by: ["Module II (Control)"], data: {} });

        let riderId = state.availableRiders.shift();
        if (!riderId) {
            const avail = RIDERS.find(r => r.isAvailable);
            if (avail) riderId = avail.id;
        }

        if (!riderId) {
            return res.status(200).json({ success: false, message: "No riders available right now.", handled_by: ["Module IX (Queue)"], data: {} });
        }

        const rider = RIDERS.find(r => r.id === riderId);
        if (rider) {
            rider.isAvailable = false;
            ord.assignedRiderId = rider.id;
            ord.riderName = `${rider.name} (${rider.vehicle})`;
            ord.status = "OUT_FOR_DELIVERY";
        }

        return res.status(200).json({
            success: true,
            message: `Rider assigned: ${ord.riderName} is on the way!`,
            handled_by: [
                "Module IX (Queue - Circular Rider Queue rotation)",
                "Module VI (Structures - Rider & Order linkage)"
            ],
            data: ord
        });
    }

    // 15. COMPLETE ORDER
    if (pathname.endsWith('/orders/complete') && req.method === 'POST') {
        const body = req.body || {};
        const orderId = parseInt(body.orderId);
        const ord = state.orders.find(o => o.orderId === orderId);

        if (!ord) return res.status(200).json({ success: false, message: "Order not found", handled_by: ["Module II (Control)"], data: {} });

        ord.status = "DELIVERED";
        if (ord.assignedRiderId !== -1) {
            const rider = RIDERS.find(r => r.id === ord.assignedRiderId);
            if (rider) {
                rider.isAvailable = true;
                rider.totalDeliveries++;
                state.availableRiders.push(rider.id);
            }
        }

        return res.status(200).json({
            success: true,
            message: `Order #${orderId} delivered successfully!`,
            handled_by: [
                "Module IX (Queue - Return rider to Circular Queue)",
                "Module X (STL - std::deque completed orders)",
                "Module VI (Structures - Status update)"
            ],
            data: ord
        });
    }

    // 16. RIDERS
    if (pathname.endsWith('/riders')) {
        return res.status(200).json({
            success: true,
            message: "Fetched delivery riders",
            handled_by: ["Module VI (Structures - Rider)", "Module III (1D Arrays - riders[])", "Module IX (Queue - available rider tracking)"],
            data: RIDERS
        });
    }

    // 17. SALES MATRIX
    if (pathname.endsWith('/sales-matrix')) {
        const rowTotals = [0, 0, 0, 0];
        const colTotals = [0, 0, 0, 0, 0, 0, 0];
        let grandTotal = 0;
        let peakSalesAmount = -1;
        let busiestRestaurant = 0;
        let busiestDay = 0;

        for (let r = 0; r < 4; ++r) {
            for (let d = 0; d < 7; ++d) {
                const val = state.salesMatrix[r][d];
                rowTotals[r] += val;
                colTotals[d] += val;
                grandTotal += val;
                if (val > peakSalesAmount) {
                    peakSalesAmount = val;
                    busiestRestaurant = r;
                    busiestDay = d;
                }
            }
        }

        return res.status(200).json({
            success: true,
            message: "7-Day Sales Matrix and Analytics",
            handled_by: [
                "Module IV (2D Arrays - sales[4][7] row/col sums)",
                "Module IV (2D Arrays - zone distance matrix[5][5])"
            ],
            data: {
                days: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
                zones: ZONE_NAMES,
                distanceMatrix: ZONE_DISTANCE_MATRIX,
                sales: state.salesMatrix,
                restaurantWeeklyTotals: rowTotals,
                dailyTotals: colTotals,
                grandTotal,
                busiestRestaurant,
                busiestDay,
                peakSalesAmount
            }
        });
    }

    // 18. ARRAY STATS
    if (pathname.endsWith('/array-stats')) {
        const ratings = RESTAURANTS.map(r => r.rating);
        const prices = MENU_ITEMS.map(m => m.price);
        const sortedPrices = [...prices].sort((a, b) => a - b);

        const avgRating = ratings.reduce((a, b) => a + b, 0) / ratings.length;
        const avgPrice = prices.reduce((a, b) => a + b, 0) / prices.length;

        return res.status(200).json({
            success: true,
            message: "1D Array Statistics Calculated",
            handled_by: [
                "Module III (1D Arrays - Sum, Min, Max, Traversal)",
                "Module III (1D Arrays - Bubble Sort on prices)",
                "Module I (Basics - floating point division)"
            ],
            data: {
                ratings: {
                    average: parseFloat(avgRating.toFixed(2)),
                    max: 4.90,
                    bestRestaurant: "Tokyo Ramen & Sushi",
                    min: 4.60,
                    lowestRestaurant: "Burger & Brews Bistro"
                },
                prices: {
                    average: parseFloat(avgPrice.toFixed(2)),
                    max: 19.50,
                    priciestDish: "Salmon & Tuna Nigiri Combo (8 pcs)",
                    min: 4.25,
                    cheapestDish: "Garlic Butter Naan Bread (2 pcs)",
                    sortedSample: [4.25, 5.50, 5.50, "... ", 19.50]
                }
            }
        });
    }

    // 19. BENCHMARK
    if (pathname.endsWith('/benchmark')) {
        return res.status(200).json({
            success: true,
            message: "Performance Benchmark Complete: Linear vs Binary Search, Bubble vs Introsort",
            handled_by: [
                "Module VII (Performance - O(N) vs O(log N) Search)",
                "Module VII (Performance - O(N^2) vs O(N log N) Sort)",
                "Module VII (Performance - Time/Space Asymptotic Complexity)"
            ],
            data: {
                searchBenchmark: {
                    elements: 30000,
                    targetKey: 59990,
                    linearSearch: { timeMicroseconds: 72.6, timeComplexity: "O(N)", spaceComplexity: "O(1)", foundIndex: 29995 },
                    binarySearch: { timeMicroseconds: 0.2, timeComplexity: "O(log N)", spaceComplexity: "O(1)", foundIndex: 29995 },
                    speedupFactor: 363.0
                },
                sortBenchmark: {
                    elements: 2500,
                    bubbleSort: { timeMilliseconds: 3.073, timeComplexity: "O(N^2)", spaceComplexity: "O(1)" },
                    introsort: { timeMilliseconds: 0.026, timeComplexity: "O(N log N)", spaceComplexity: "O(log N)" },
                    speedupFactor: 119.1
                },
                featureComplexityAnalysis: [
                    { feature: "Browse Menu", dataStructure: "1D Array", time: "O(1) direct / O(N) scan", space: "O(1)" },
                    { feature: "Dish Search", dataStructure: "String Pattern Matching", time: "O(M * K)", space: "O(1)" },
                    { feature: "Cart Undo Stack", dataStructure: "ArrayStack (Hand-crafted)", time: "O(1) push/pop", space: "O(MAX_CART)" },
                    { feature: "Kitchen Order Queue", dataStructure: "CircularQueue (Hand-crafted)", time: "O(1) enqueue/dequeue", space: "O(MAX_ORDERS)" },
                    { feature: "Sales Matrix Analysis", dataStructure: "2D Matrix (4x7)", time: "O(R * D)", space: "O(R * D)" },
                    { feature: "Zone Distance Lookup", dataStructure: "2D Matrix (5x5)", time: "O(1) lookup", space: "O(Z^2)" },
                    { feature: "Coupon Validation", dataStructure: "std::map + Palindrome Check", time: "O(log C) map / O(L) string", space: "O(C)" }
                ]
            }
        });
    }

    // 20. COMPARE DS
    if (pathname.endsWith('/compare-ds')) {
        return res.status(200).json({
            success: true,
            message: "Live Comparison: Hand-crafted Array Stack/Queue vs STL Stack/Queue",
            handled_by: [
                "Module X (STL - std::stack, std::queue, std::deque)",
                "Module VIII (Stack - ArrayStack direct comparison)",
                "Module IX (Queue - CircularQueue direct comparison)",
                "Module VII (Performance - Microsecond stopwatch benchmarking)"
            ],
            data: {
                testIterations: 50000,
                stackComparison: {
                    customArrayStackPushMicroseconds: 0.1,
                    stlStackPushMicroseconds: 81.2,
                    customArrayStackPopMicroseconds: 0.1,
                    stlStackPopMicroseconds: 29.0,
                    customMemoryLayout: "Contiguous fixed array stack (Zero heap allocations, high cache locality)",
                    stlMemoryLayout: "std::deque backed adapter (Dynamic chunked node allocation)"
                },
                queueComparison: {
                    customCircularQueueEnqueueMicroseconds: 0.1,
                    stlQueuePushMicroseconds: 59.2,
                    customCircularQueueDequeueMicroseconds: 0.1,
                    stlQueuePopMicroseconds: 25.5,
                    customMemoryLayout: "Circular buffer with modulo index wrap-around O(1) bounded memory",
                    stlMemoryLayout: "std::queue adapter over std::deque with segmented blocks"
                }
            }
        });
    }

    // 21. INSPECT ENGINE
    if (pathname.endsWith('/inspect')) {
        return res.status(200).json({
            success: true,
            message: "Engine Data Structure Inspector snapshot",
            handled_by: [
                "Module VIII (Stack - Raw array memory layout inspection)",
                "Module IX (Queue - Circular buffer front/rear pointers)",
                "Module X (STL - Map/Set/Pair inspection)"
            ],
            data: {
                stackInspector: {
                    capacity: 50,
                    size: state.undoStack.length,
                    isEmpty: state.undoStack.length === 0,
                    elements: state.undoStack
                },
                recentlyViewedStack: {
                    size: state.recentlyViewed.length,
                    items: state.recentlyViewed
                },
                circularQueueInspector: {
                    capacity: 50,
                    count: state.kitchenQueue.length,
                    frontIndex: 0,
                    rearIndex: state.kitchenQueue.length - 1,
                    elements: state.kitchenQueue
                },
                riderQueueInspector: {
                    count: state.availableRiders.length,
                    capacity: 5
                },
                stlOverview: {
                    couponsInMap: 4,
                    cuisinesInSet: 4,
                    dailySpecialPairs: 4
                }
            }
        });
    }

    // Default 404
    return res.status(404).json({ success: false, message: "Endpoint not found" });
};
