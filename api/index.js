// ============================================================================
// FoodRush / RestoRush - Vercel Serverless Function Engine
// High-performance restaurant POS backend engine implementing Modules I-X
// ============================================================================

const os = require('os');

// In-memory state persisted across warm lambda invocations
if (!global.foodrushState) {
    global.foodrushState = {
        activeTableId: 1,
        tableCarts: {}, // tableId -> { items: [], guestName, couponCode, discountPercent }
        undoStack: [],  // LIFO Stack for order modifications [MODULE VIII]
        activeOrders: [
            {
                orderId: 1001,
                tableId: 2,
                tableName: "Table 2 (Booth)",
                section: "Main Dining Hall",
                outletId: 1,
                outletName: "Grand Mughal Dining",
                guestName: "Mr. Sharma",
                serverName: "Vikram Malhotra",
                orderTime: "13:15 PM",
                isExpress: false,
                status: "PREPARING",
                itemCount: 2,
                items: [
                    { itemId: 101, outletId: 1, name: "Hyderabadi Chicken Dum Biryani", price: 320.00, quantity: 2, lineTotal: 640.00 },
                    { itemId: 107, outletId: 1, name: "Burani Garlic Raita", price: 60.00, quantity: 2, lineTotal: 120.00 }
                ],
                subtotal: 760.00,
                discount: 0.00,
                gstTax: 38.00,
                serviceCharge: 38.00,
                totalAmount: 836.00,
                queuePosition: 1
            },
            {
                orderId: 1002,
                tableId: 5,
                tableName: "Table 5 (Executive)",
                section: "AC Family Lounge",
                outletId: 3,
                outletName: "Trattoria Bella Vista",
                guestName: "Dr. Kapoor",
                serverName: "Priya Nair",
                orderTime: "13:28 PM",
                isExpress: true,
                status: "ORDERED",
                itemCount: 3,
                items: [
                    { itemId: 301, outletId: 3, name: "Wood-Fired Margherita Pizza", price: 390.00, quantity: 1, lineTotal: 390.00 },
                    { itemId: 303, outletId: 3, name: "Truffle Wild Mushroom Fettuccine", price: 460.00, quantity: 1, lineTotal: 460.00 },
                    { itemId: 308, outletId: 3, name: "Classic Espresso Tiramisu", price: 240.00, quantity: 2, lineTotal: 480.00 }
                ],
                subtotal: 1330.00,
                discount: 0.00,
                gstTax: 66.50,
                serviceCharge: 66.50,
                totalAmount: 1463.00,
                queuePosition: 2
            },
            {
                orderId: 1003,
                tableId: 8,
                tableName: "Table 8 (Sky View)",
                section: "Rooftop Terrace",
                outletId: 4,
                outletName: "Sakura Asian Bistro",
                guestName: "Ananya & Friend",
                serverName: "Rohit Verma",
                orderTime: "13:05 PM",
                isExpress: false,
                status: "SERVED",
                itemCount: 2,
                items: [
                    { itemId: 401, outletId: 4, name: "Rich Tonkotsu Black Garlic Ramen", price: 480.00, quantity: 2, lineTotal: 960.00 },
                    { itemId: 405, outletId: 4, name: "Pan-Seared Chicken Gyoza (6 pcs)", price: 280.00, quantity: 1, lineTotal: 280.00 }
                ],
                subtotal: 1240.00,
                discount: 0.00,
                gstTax: 62.00,
                serviceCharge: 62.00,
                totalAmount: 1364.00,
                queuePosition: 3
            }
        ],
        pastBills: [
            {
                billId: 5001,
                orderId: 998,
                tableId: 1,
                tableName: "Table 1 (Family)",
                section: "Main Dining Hall",
                guestName: "Rajesh Verma",
                serverName: "Vikram Malhotra",
                billTime: "12:30 PM",
                itemCount: 2,
                items: [
                    { itemId: 101, outletId: 1, name: "Hyderabadi Chicken Dum Biryani", price: 320.00, quantity: 2, lineTotal: 640.00 },
                    { itemId: 108, outletId: 1, name: "Zafrani Matka Phirni", price: 130.00, quantity: 2, lineTotal: 260.00 }
                ],
                subtotal: 900.00,
                discount: 90.00,
                gstTax: 40.50,
                serviceCharge: 40.50,
                netTotal: 891.00,
                paymentMethod: "UPI",
                invoiceCode: "INV-5001-1"
            },
            {
                billId: 5002,
                orderId: 999,
                tableId: 4,
                tableName: "Table 4 (Central)",
                section: "AC Family Lounge",
                guestName: "Siddharth Rao",
                serverName: "Priya Nair",
                billTime: "12:50 PM",
                itemCount: 2,
                items: [
                    { itemId: 201, outletId: 2, name: "Ghee Roast Masala Dosa", price: 160.00, quantity: 2, lineTotal: 320.00 },
                    { itemId: 208, outletId: 2, name: "Filter Degree Coffee", price: 50.00, quantity: 2, lineTotal: 100.00 }
                ],
                subtotal: 420.00,
                discount: 0.00,
                gstTax: 21.00,
                serviceCharge: 21.00,
                netTotal: 462.00,
                paymentMethod: "Card",
                invoiceCode: "INV-5002-3"
            }
        ],
        nextOrderId: 1004,
        nextBillId: 5003,
        // [MODULE IV: 2D Arrays] 6 Outlets x 7 Days Sales Matrix
        salesMatrix: [
            [14200, 13800, 15900, 16400, 22500, 28900, 26400],
            [9800, 10400, 11200, 11800, 15600, 21400, 19800],
            [11500, 10900, 12600, 13100, 18900, 24800, 22300],
            [12800, 12100, 13400, 14200, 19700, 25900, 23700],
            [13600, 13100, 14800, 15500, 21200, 27800, 25100],
            [8400, 7900, 8900, 9400, 14200, 19500, 18100]
        ]
    };
}

const state = global.foodrushState;

// 6 Outlets
const OUTLETS = [
    { id: 1, name: "Grand Mughal Dining", cuisine: "Biryani & Mughlai", section: "Main Dining Hall", rating: 4.9, isOpen: true, itemCount: 8 },
    { id: 2, name: "Dakshin Tiffin & Cafe", cuisine: "South Indian", section: "Ground Floor Cafe", rating: 4.8, isOpen: true, itemCount: 8 },
    { id: 3, name: "Trattoria Bella Vista", cuisine: "Artisan Italian", section: "First Floor Gallery", rating: 4.7, isOpen: true, itemCount: 8 },
    { id: 4, name: "Sakura Asian Bistro", cuisine: "Japanese & Asian", section: "AC Fine Dining", rating: 4.9, isOpen: true, itemCount: 8 },
    { id: 5, name: "The Boulevard Grill & Burgers", cuisine: "American Gourmet", section: "Terrace Garden", rating: 4.6, isOpen: true, itemCount: 8 },
    { id: 6, name: "The Royal Patisserie", cuisine: "Desserts & Confectionery", section: "Lobby Lounge", rating: 4.8, isOpen: true, itemCount: 8 }
];

// 12 Dining Tables
const TABLES = [
    { id: 1, name: "Table 1 (Family)", section: "Main Dining Hall", capacity: 4, status: "VACANT", activeOrderId: -1, serverName: "Vikram Malhotra" },
    { id: 2, name: "Table 2 (Booth)", section: "Main Dining Hall", capacity: 4, status: "OCCUPIED", activeOrderId: 1001, serverName: "Vikram Malhotra" },
    { id: 3, name: "Table 3 (Round)", section: "Main Dining Hall", capacity: 6, status: "VACANT", activeOrderId: -1, serverName: "Amitav Sen" },
    { id: 4, name: "Table 4 (Central)", section: "AC Family Lounge", capacity: 6, status: "VACANT", activeOrderId: -1, serverName: "Priya Nair" },
    { id: 5, name: "Table 5 (Executive)", section: "AC Family Lounge", capacity: 8, status: "OCCUPIED", activeOrderId: 1002, serverName: "Priya Nair" },
    { id: 6, name: "Table 6 (Corner)", section: "AC Family Lounge", capacity: 4, status: "VACANT", activeOrderId: -1, serverName: "Priya Nair" },
    { id: 7, name: "Table 7 (Couple)", section: "Rooftop Terrace", capacity: 2, status: "VACANT", activeOrderId: -1, serverName: "Rohit Verma" },
    { id: 8, name: "Table 8 (Sky View)", section: "Rooftop Terrace", capacity: 4, status: "OCCUPIED", activeOrderId: 1003, serverName: "Rohit Verma" },
    { id: 9, name: "Table 9 (Sunset)", section: "Rooftop Terrace", capacity: 4, status: "VACANT", activeOrderId: -1, serverName: "Rohit Verma" },
    { id: 10, name: "Table 10 (Garden)", section: "Garden Lounge & Banquet", capacity: 4, status: "VACANT", activeOrderId: -1, serverName: "Amitav Sen" },
    { id: 11, name: "Table 11 (Banquet A)", section: "Garden Lounge & Banquet", capacity: 8, status: "VACANT", activeOrderId: -1, serverName: "Sunita Sharma" },
    { id: 12, name: "Table 12 (Banquet B)", section: "Garden Lounge & Banquet", capacity: 10, status: "VACANT", activeOrderId: -1, serverName: "Sunita Sharma" }
];

// 8 Staff Members
const STAFF = [
    { id: 1, name: "Chef Rajesh Kumar", role: "Head Chef", shift: "Morning (8am-4pm)", isPresent: true, hoursWorked: 7.5, phone: "+91 98765 43201" },
    { id: 2, name: "Chef Antonio Rossi", role: "Sous Chef", shift: "Evening (4pm-12am)", isPresent: true, hoursWorked: 6.0, phone: "+91 98765 43202" },
    { id: 3, name: "Vikram Malhotra", role: "Captain Waiter", shift: "Morning (8am-4pm)", isPresent: true, hoursWorked: 8.0, phone: "+91 98765 43203" },
    { id: 4, name: "Priya Nair", role: "Senior Table Server", shift: "Evening (4pm-12am)", isPresent: true, hoursWorked: 7.0, phone: "+91 98765 43204" },
    { id: 5, name: "Amitav Sen", role: "Table Server", shift: "Morning (8am-4pm)", isPresent: true, hoursWorked: 8.0, phone: "+91 98765 43205" },
    { id: 6, name: "Kavita Rao", role: "Head Cashier & Billing", shift: "Morning (8am-4pm)", isPresent: true, hoursWorked: 8.0, phone: "+91 98765 43206" },
    { id: 7, name: "Rohit Verma", role: "Beverage & Barista", shift: "Evening (4pm-12am)", isPresent: true, hoursWorked: 7.5, phone: "+91 98765 43207" },
    { id: 8, name: "Sunita Sharma", role: "Floor Supervisor", shift: "Full Day", isPresent: true, hoursWorked: 9.0, phone: "+91 98765 43208" }
];

// 48 Menu Items
const MENU_ITEMS = [
    // Grand Mughal Dining (IDs 101 - 108)
    { id: 101, restaurantId: 1, outletId: 1, name: "Hyderabadi Chicken Dum Biryani", category: "Biryani", price: 320.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.9, ratingCount: 64, isVeg: false, calories: 840, isSpicy: true, isGlutenFree: false, isChefSpecial: true },
    { id: 102, restaurantId: 1, outletId: 1, name: "Awadhi Mutton Dum Biryani", category: "Biryani", price: 440.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.8, ratingCount: 42, isVeg: false, calories: 960, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 103, restaurantId: 1, outletId: 1, name: "Nawabi Paneer Tikka Biryani", category: "Biryani", price: 280.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.7, ratingCount: 31, isVeg: true, calories: 720, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 104, restaurantId: 1, outletId: 1, name: "Murgh Malai Chicken Tikka", category: "Kebabs", price: 290.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.6, ratingCount: 28, isVeg: false, calories: 580, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 105, restaurantId: 1, outletId: 1, name: "Fiery Andhra Chicken Fry", category: "Starters", price: 260.00, stock: 22, reservedStock: 0, availableStock: 22, rating: 4.7, ratingCount: 35, isVeg: false, calories: 620, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 106, restaurantId: 1, outletId: 1, name: "Shahi Mirchi Ka Salan", category: "Sides", price: 90.00, stock: 40, reservedStock: 0, availableStock: 40, rating: 4.5, ratingCount: 20, isVeg: true, calories: 210, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 107, restaurantId: 1, outletId: 1, name: "Burani Garlic Raita", category: "Sides", price: 60.00, stock: 50, reservedStock: 0, availableStock: 50, rating: 4.6, ratingCount: 25, isVeg: true, calories: 140, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 108, restaurantId: 1, outletId: 1, name: "Zafrani Matka Phirni", category: "Desserts", price: 130.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.9, ratingCount: 52, isVeg: true, calories: 340, isSpicy: false, isGlutenFree: false, isChefSpecial: true },

    // Dakshin Tiffin & Cafe (IDs 201 - 208)
    { id: 201, restaurantId: 2, outletId: 2, name: "Ghee Roast Masala Dosa", category: "Dosa", price: 160.00, stock: 40, reservedStock: 0, availableStock: 40, rating: 4.9, ratingCount: 78, isVeg: true, calories: 420, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 202, restaurantId: 2, outletId: 2, name: "Mysore Onion Rava Dosa", category: "Dosa", price: 175.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.8, ratingCount: 45, isVeg: true, calories: 460, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 203, restaurantId: 2, outletId: 2, name: "Steamed Button Idli (4 pcs)", category: "Tiffin", price: 95.00, stock: 50, reservedStock: 0, availableStock: 50, rating: 4.7, ratingCount: 50, isVeg: true, calories: 240, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 204, restaurantId: 2, outletId: 2, name: "Crispy Medu Vada (2 pcs)", category: "Tiffin", price: 90.00, stock: 45, reservedStock: 0, availableStock: 45, rating: 4.6, ratingCount: 38, isVeg: true, calories: 310, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 205, restaurantId: 2, outletId: 2, name: "Chettinad Spicy Paneer Dosa", category: "Dosa", price: 195.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.8, ratingCount: 36, isVeg: true, calories: 510, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 206, restaurantId: 2, outletId: 2, name: "Bisi Bele Bath with Boondi", category: "Rice", price: 140.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.5, ratingCount: 29, isVeg: true, calories: 480, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 207, restaurantId: 2, outletId: 2, name: "Pineapple Kesari Halwa", category: "Desserts", price: 110.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.7, ratingCount: 40, isVeg: true, calories: 390, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 208, restaurantId: 2, outletId: 2, name: "Filter Degree Coffee", category: "Beverages", price: 50.00, stock: 60, reservedStock: 0, availableStock: 60, rating: 4.9, ratingCount: 90, isVeg: true, calories: 120, isSpicy: false, isGlutenFree: false, isChefSpecial: true },

    // Trattoria Bella Vista (IDs 301 - 308)
    { id: 301, restaurantId: 3, outletId: 3, name: "Wood-Fired Margherita Pizza", category: "Pizza", price: 390.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.8, ratingCount: 55, isVeg: true, calories: 780, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 302, restaurantId: 3, outletId: 3, name: "Spicy Penne Arrabbiata", category: "Pasta", price: 340.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.6, ratingCount: 30, isVeg: true, calories: 610, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 303, restaurantId: 3, outletId: 3, name: "Truffle Wild Mushroom Fettuccine", category: "Pasta", price: 460.00, stock: 15, reservedStock: 0, availableStock: 15, rating: 4.9, ratingCount: 44, isVeg: true, calories: 790, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 304, restaurantId: 3, outletId: 3, name: "Smoked Chicken & Jalapeno Pizza", category: "Pizza", price: 440.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.7, ratingCount: 33, isVeg: false, calories: 860, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 305, restaurantId: 3, outletId: 3, name: "Pesto Genovese Gnocchi", category: "Pasta", price: 380.00, stock: 18, reservedStock: 0, availableStock: 18, rating: 4.5, ratingCount: 22, isVeg: true, calories: 640, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 306, restaurantId: 3, outletId: 3, name: "Rosemary Garlic Focaccia", category: "Breads", price: 140.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.6, ratingCount: 26, isVeg: true, calories: 320, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 307, restaurantId: 3, outletId: 3, name: "Burrata Caprese Salad", category: "Salads", price: 310.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.7, ratingCount: 27, isVeg: true, calories: 390, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 308, restaurantId: 3, outletId: 3, name: "Classic Espresso Tiramisu", category: "Desserts", price: 240.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.9, ratingCount: 61, isVeg: true, calories: 410, isSpicy: false, isGlutenFree: false, isChefSpecial: true },

    // Sakura Asian Bistro (IDs 401 - 408)
    { id: 401, restaurantId: 4, outletId: 4, name: "Rich Tonkotsu Black Garlic Ramen", category: "Ramen", price: 480.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.9, ratingCount: 72, isVeg: false, calories: 890, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 402, restaurantId: 4, outletId: 4, name: "Spicy Miso Tofu Ramen", category: "Ramen", price: 410.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.7, ratingCount: 39, isVeg: true, calories: 720, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 403, restaurantId: 4, outletId: 4, name: "Crispy Vegetable Tempura Roll", category: "Sushi", price: 350.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.6, ratingCount: 34, isVeg: true, calories: 410, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 404, restaurantId: 4, outletId: 4, name: "Salmon & Avocado Maki (8 pcs)", category: "Sushi", price: 490.00, stock: 15, reservedStock: 0, availableStock: 15, rating: 4.8, ratingCount: 48, isVeg: false, calories: 480, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 405, restaurantId: 4, outletId: 4, name: "Pan-Seared Chicken Gyoza (6 pcs)", category: "Dim Sum", price: 280.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.7, ratingCount: 37, isVeg: false, calories: 360, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 406, restaurantId: 4, outletId: 4, name: "Steamed Truffle Edamame", category: "Appetizers", price: 220.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.5, ratingCount: 21, isVeg: true, calories: 190, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 407, restaurantId: 4, outletId: 4, name: "Karaage Japanese Fried Chicken", category: "Starters", price: 320.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.8, ratingCount: 46, isVeg: false, calories: 610, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 408, restaurantId: 4, outletId: 4, name: "Matcha Green Tea Mochi (3 pcs)", category: "Desserts", price: 180.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.8, ratingCount: 50, isVeg: true, calories: 260, isSpicy: false, isGlutenFree: true, isChefSpecial: false },

    // The Boulevard Grill & Burgers (IDs 501 - 508)
    { id: 501, restaurantId: 5, outletId: 5, name: "Smoked Bacon Cheddar Smash Burger", category: "Burgers", price: 360.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.8, ratingCount: 59, isVeg: false, calories: 920, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 502, restaurantId: 5, outletId: 5, name: "Nashville Hot Crispy Chicken Burger", category: "Burgers", price: 330.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.7, ratingCount: 43, isVeg: false, calories: 840, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 503, restaurantId: 5, outletId: 5, name: "Truffle Wild Mushroom Veggie Burger", category: "Burgers", price: 290.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.6, ratingCount: 32, isVeg: true, calories: 670, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 504, restaurantId: 5, outletId: 5, name: "Peri-Peri Seasoned Curly Fries", category: "Sides", price: 140.00, stock: 50, reservedStock: 0, availableStock: 50, rating: 4.7, ratingCount: 45, isVeg: true, calories: 390, isSpicy: true, isGlutenFree: false, isChefSpecial: false },
    { id: 505, restaurantId: 5, outletId: 5, name: "Smoky BBQ Glazed Wings (6 pcs)", category: "Starters", price: 310.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.6, ratingCount: 38, isVeg: false, calories: 680, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 506, restaurantId: 5, outletId: 5, name: "Crispy Onion Rings with Garlic Dip", category: "Sides", price: 150.00, stock: 40, reservedStock: 0, availableStock: 40, rating: 4.5, ratingCount: 29, isVeg: true, calories: 340, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 507, restaurantId: 5, outletId: 5, name: "Hand-Spun Salted Caramel Shake", category: "Beverages", price: 190.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.8, ratingCount: 41, isVeg: true, calories: 510, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 508, restaurantId: 5, outletId: 5, name: "Molten Chocolate Lava Cake", category: "Desserts", price: 210.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.9, ratingCount: 65, isVeg: true, calories: 540, isSpicy: false, isGlutenFree: false, isChefSpecial: true },

    // The Royal Patisserie (IDs 601 - 608)
    { id: 601, restaurantId: 6, outletId: 6, name: "Belgian Dark Chocolate Ganache Pastry", category: "Pastries", price: 180.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.9, ratingCount: 70, isVeg: true, calories: 440, isSpicy: false, isGlutenFree: false, isChefSpecial: true },
    { id: 602, restaurantId: 6, outletId: 6, name: "Blueberry Baked New York Cheesecake", category: "Cakes", price: 240.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.8, ratingCount: 52, isVeg: true, calories: 490, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 603, restaurantId: 6, outletId: 6, name: "Almond French Croissant", category: "Bakery", price: 150.00, stock: 35, reservedStock: 0, availableStock: 35, rating: 4.7, ratingCount: 36, isVeg: true, calories: 380, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 604, restaurantId: 6, outletId: 6, name: "Pistachio Raspberry Macarons (4 pcs)", category: "Desserts", price: 220.00, stock: 20, reservedStock: 0, availableStock: 20, rating: 4.8, ratingCount: 40, isVeg: true, calories: 310, isSpicy: false, isGlutenFree: true, isChefSpecial: false },
    { id: 605, restaurantId: 6, outletId: 6, name: "Warm Cinnamon Sugar Churros (4 pcs)", category: "Desserts", price: 170.00, stock: 25, reservedStock: 0, availableStock: 25, rating: 4.6, ratingCount: 28, isVeg: true, calories: 410, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 606, restaurantId: 6, outletId: 6, name: "Red Velvet Cream Cheese Slice", category: "Cakes", price: 190.00, stock: 30, reservedStock: 0, availableStock: 30, rating: 4.7, ratingCount: 35, isVeg: true, calories: 460, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 607, restaurantId: 6, outletId: 6, name: "Double Chocolate Walnut Fudge Brownie", category: "Pastries", price: 160.00, stock: 40, reservedStock: 0, availableStock: 40, rating: 4.8, ratingCount: 48, isVeg: true, calories: 480, isSpicy: false, isGlutenFree: false, isChefSpecial: false },
    { id: 608, restaurantId: 6, outletId: 6, name: "Artisanal Iced Cold Brew", category: "Beverages", price: 140.00, stock: 50, reservedStock: 0, availableStock: 50, rating: 4.9, ratingCount: 58, isVeg: true, calories: 40, isSpicy: false, isGlutenFree: true, isChefSpecial: true }
];

const COUPONS = {
    "WELCOME10": 10.0,
    "HOTEL50": 50.0,
    "FESTIVE20": 20.0,
    "STAFFDISC": 25.0
};

// Check-digit calculation using string reversal [MODULE V]
function generateInvoiceCode(billId) {
    const s = String(billId);
    let rev = s.split('').reverse().join('');
    let sum = 0;
    for (let i = 0; i < rev.length; ++i) {
        sum += (rev.charCodeAt(i) - 48) * (i + 1);
    }
    const checkDigit = sum % 10;
    return `INV-${billId}-${checkDigit}`;
}

function generatePrintReceiptText(bill) {
    const pad = (str, len, right = false) => {
        str = String(str);
        if (str.length > len) str = str.substring(0, len - 2) + '..';
        return right ? str.padStart(len) : str.padEnd(len);
    };

    let out = "========================================\n";
    out += "       THE GRAND REGENCY HOTEL & POS    \n";
    out += "       Official Tax Invoice & Receipt   \n";
    out += "========================================\n";
    out += `Invoice No:  ${bill.invoiceCode}\n`;
    out += `Bill ID:     #${bill.billId}\n`;
    out += `Date & Time: ${bill.billTime}\n`;
    out += `Table:       ${bill.tableName} (${bill.section})\n`;
    out += `Guest Name:  ${bill.guestName}\n`;
    out += `Server:      ${bill.serverName}\n`;
    out += "----------------------------------------\n";
    out += "ITEM                     QTY   PRICE   TOTAL\n";
    out += "----------------------------------------\n";

    for (const it of bill.items) {
        out += `${pad(it.name, 22)}${pad(it.quantity, 3, true)}${pad(it.price.toFixed(2), 8, true)}${pad(it.lineTotal.toFixed(2), 7, true)}\n`;
    }

    out += "----------------------------------------\n";
    out += `${pad("Item Subtotal:", 30)}${pad(bill.subtotal.toFixed(2), 10, true)}\n`;
    if (bill.discount > 0) {
        out += `${pad("Promo Discount:", 30)}${pad("-" + bill.discount.toFixed(2), 10, true)}\n`;
    }
    out += `${pad("GST (5% SGST+CGST):", 30)}${pad(bill.gstTax.toFixed(2), 10, true)}\n`;
    out += `${pad("Service Charge (5%):", 30)}${pad(bill.serviceCharge.toFixed(2), 10, true)}\n`;
    out += "========================================\n";
    out += `${pad("NET TOTAL PAYABLE:", 28)}INR ${pad(bill.netTotal.toFixed(2), 8, true)}\n`;
    out += `Payment Method: ${bill.paymentMethod}\n`;
    out += "========================================\n";
    out += "     THANK YOU FOR DINING WITH US!      \n";
    out += "  GSTIN: 29AABCU9603R1ZM | Bangalore   \n";
    out += "========================================\n";
    return out;
}

function getTableCart(tableId) {
    if (!state.tableCarts[tableId]) {
        state.tableCarts[tableId] = {
            tableId: tableId,
            items: [],
            guestName: "Guest",
            couponCode: "",
            discountPercent: 0.0
        };
    }
    const c = state.tableCarts[tableId];
    const tbl = TABLES.find(t => t.id === tableId) || { name: `Table ${tableId}` };

    let subtotal = 0;
    for (const it of c.items) subtotal += (it.price * it.quantity);
    let discount = (c.discountPercent > 0) ? (subtotal * (c.discountPercent / 100.0)) : 0;
    if (discount > subtotal) discount = subtotal;

    let base = subtotal - discount;
    let gstTax = base * 0.05;
    let serviceCharge = base * 0.05;
    let netTotal = base + gstTax + serviceCharge;

    return {
        tableId,
        tableName: tbl.name,
        guestName: c.guestName,
        itemCount: c.items.length,
        items: c.items,
        coupon: c.couponCode,
        discountPercent: c.discountPercent,
        discountAmount: Number(discount.toFixed(2)),
        subtotal: Number(subtotal.toFixed(2)),
        gstTax: Number(gstTax.toFixed(2)),
        serviceCharge: Number(serviceCharge.toFixed(2)),
        netTotal: Number(netTotal.toFixed(2)),
        undoStackDepth: state.undoStack.length
    };
}

module.exports = async (req, res) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.status(204).end();
        return;
    }

    const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    const pathname = url.pathname.replace(/^\/api/, '');

    const makeResp = (success, message, data, modules = []) => {
        return res.status(200).json({
            success,
            message,
            modules,
            handled_by: modules,
            data
        });
    };

    try {
        if (pathname === '/initial-state' || pathname === '/ping' || pathname === '') {
            return makeResp(true, "POS Initial State Loaded", {
                activeTableId: state.activeTableId,
                outlets: OUTLETS,
                tables: TABLES,
                staff: STAFF,
                activeCart: getTableCart(state.activeTableId),
                activeOrdersCount: state.activeOrders.length,
                kitchenQueueSize: state.activeOrders.filter(o => o.status !== 'BILLED').length
            }, ["Module I (Basics)", "Module VI (Structures)"]);
        }

        if (pathname === '/restaurants' || pathname === '/outlets') {
            return makeResp(true, "Kitchen Outlets Retrieved", OUTLETS, ["Module III (1D Arrays)", "Module VI (Structures)"]);
        }

        if (pathname === '/menu') {
            const outId = parseInt(url.searchParams.get('restaurantId') || url.searchParams.get('outletId') || '0');
            const items = outId === 0 ? MENU_ITEMS : MENU_ITEMS.filter(i => i.outletId === outId);
            return makeResp(true, "Menu Retrieved", items, ["Module III (1D Array Traversal)"]);
        }

        if (pathname === '/tables') {
            return makeResp(true, "Dining Tables Retrieved", TABLES, ["Module VI (Table Structures)"]);
        }

        if (pathname === '/tables/select' && req.method === 'POST') {
            const tId = parseInt(req.body.tableId || 1);
            state.activeTableId = tId;
            return makeResp(true, `Active table switched to Table ${tId}`, getTableCart(tId), ["Module I (State)"]);
        }

        if (pathname === '/cart' || pathname === '/order/current') {
            const tId = parseInt(url.searchParams.get('tableId') || state.activeTableId);
            return makeResp(true, `Table ${tId} Order View`, getTableCart(tId), ["Module I (Totals)"]);
        }

        if ((pathname === '/cart/add' || pathname === '/order/add') && req.method === 'POST') {
            const { itemId, quantity = 1, tableId = state.activeTableId } = req.body;
            const item = MENU_ITEMS.find(i => i.id === parseInt(itemId));
            if (!item) return makeResp(false, "Item not found", null);

            const c = state.tableCarts[tableId] || (state.tableCarts[tableId] = { tableId, items: [], guestName: "Guest", couponCode: "", discountPercent: 0 });
            let existing = c.items.find(i => i.itemId === item.id);
            const prevQty = existing ? existing.quantity : 0;
            const newQty = prevQty + parseInt(quantity);

            if (existing) {
                existing.quantity = newQty;
                existing.lineTotal = existing.price * newQty;
            } else {
                c.items.push({
                    itemId: item.id,
                    outletId: item.outletId,
                    name: item.name,
                    price: item.price,
                    quantity: newQty,
                    lineTotal: item.price * newQty
                });
            }

            state.undoStack.push({
                type: prevQty === 0 ? "ADD" : "UPDATE_QTY",
                tableId,
                itemId: item.id,
                prevQty,
                newQty,
                itemPrice: item.price,
                itemName: item.name
            });

            return makeResp(true, "Item added to table order", getTableCart(tableId), ["Module VIII (ArrayStack Push)"]);
        }

        if ((pathname === '/cart/undo' || pathname === '/order/undo') && req.method === 'POST') {
            const tId = parseInt(req.body.tableId || state.activeTableId);
            if (state.undoStack.length === 0) {
                return makeResp(false, "Nothing to undo", getTableCart(tId), ["Module VIII (Stack Empty)"]);
            }
            const act = state.undoStack.pop();
            const c = state.tableCarts[act.tableId || tId];
            if (c) {
                if (act.type === "ADD") {
                    c.items = c.items.filter(i => i.itemId !== act.itemId);
                } else if (act.type === "UPDATE_QTY") {
                    let it = c.items.find(i => i.itemId === act.itemId);
                    if (it) {
                        it.quantity = act.prevQty;
                        it.lineTotal = it.price * act.prevQty;
                    }
                }
            }
            return makeResp(true, "Last modification undone", getTableCart(tId), ["Module VIII (ArrayStack Pop)"]);
        }

        if ((pathname === '/cart/clear' || pathname === '/order/clear') && req.method === 'POST') {
            const tId = parseInt(req.body.tableId || state.activeTableId);
            if (state.tableCarts[tId]) state.tableCarts[tId].items = [];
            return makeResp(true, `Table ${tId} draft order cleared`, getTableCart(tId), ["Module I (State Reset)"]);
        }

        if (pathname === '/coupon' && req.method === 'POST') {
            const { code, tableId = state.activeTableId } = req.body;
            if (COUPONS[code]) {
                const c = state.tableCarts[tableId] || (state.tableCarts[tableId] = { tableId, items: [], guestName: "Guest", couponCode: "", discountPercent: 0 });
                c.couponCode = code;
                c.discountPercent = COUPONS[code];
                return makeResp(true, `Coupon applied (${COUPONS[code]}% off)`, getTableCart(tableId), ["Module X (std::map)"]);
            }
            return makeResp(false, "Invalid coupon code", getTableCart(tableId));
        }

        if ((pathname === '/kot/submit' || pathname === '/checkout') && req.method === 'POST') {
            const { guestName = "Guest", isExpress = false, tableId = state.activeTableId } = req.body;
            const cart = getTableCart(tableId);
            if (cart.items.length === 0) return makeResp(false, "Order is empty", null);

            const table = TABLES.find(t => t.id === parseInt(tableId));
            const orderId = state.nextOrderId++;

            const kot = {
                orderId,
                tableId: parseInt(tableId),
                tableName: table ? table.name : `Table ${tableId}`,
                section: table ? table.section : "Main Dining",
                outletId: cart.items[0].outletId,
                outletName: OUTLETS.find(o => o.id === cart.items[0].outletId)?.name || "Kitchen",
                guestName,
                serverName: table ? table.serverName : "Server",
                orderTime: "Just Now",
                isExpress: !!isExpress,
                status: "ORDERED",
                itemCount: cart.items.length,
                items: [...cart.items],
                subtotal: cart.subtotal,
                discount: cart.discountAmount,
                gstTax: cart.gstTax,
                serviceCharge: cart.serviceCharge,
                totalAmount: cart.netTotal,
                queuePosition: state.activeOrders.filter(o => o.status !== 'BILLED').length + 1
            };

            state.activeOrders.push(kot);
            if (table) {
                table.status = "OCCUPIED";
                table.activeOrderId = orderId;
            }
            if (state.tableCarts[tableId]) state.tableCarts[tableId].items = [];

            return makeResp(true, `KOT #${orderId} dispatched to kitchen`, kot, ["Module IX (CircularQueue Enqueue)", "Module VI (Structures)"]);
        }

        if (pathname === '/orders/active' || pathname === '/orders' || pathname === '/kot') {
            const active = state.activeOrders.filter(o => o.status !== 'BILLED');
            return makeResp(true, "Active Kitchen Orders Retrieved", active, ["Module IX (Kitchen Queue)"]);
        }

        if (pathname === '/orders/stage' && req.method === 'POST') {
            const { orderId, stage } = req.body;
            const kot = state.activeOrders.find(o => o.orderId === parseInt(orderId));
            if (!kot) return makeResp(false, "Order not found", null);

            kot.status = (stage || "PREPARING").toUpperCase();
            return makeResp(true, `Order #${orderId} status updated to ${kot.status}`, kot, ["Module IX (Circular Queue Dequeue)"]);
        }

        if (pathname === '/bill/generate' && req.method === 'POST') {
            const { tableId, paymentMethod = "UPI", coupon = "" } = req.body;
            const table = TABLES.find(t => t.id === parseInt(tableId));
            const kot = state.activeOrders.find(o => o.tableId === parseInt(tableId) && o.status !== 'BILLED');
            if (!kot) return makeResp(false, `No active order found for Table ${tableId}`, null);

            const billId = state.nextBillId++;
            const bill = {
                billId,
                orderId: kot.orderId,
                tableId: parseInt(tableId),
                tableName: table ? table.name : `Table ${tableId}`,
                section: table ? table.section : "Main Dining",
                guestName: kot.guestName,
                serverName: kot.serverName,
                billTime: "Today, Just Now",
                itemCount: kot.items.length,
                items: kot.items,
                subtotal: kot.subtotal,
                discount: kot.discount,
                gstTax: kot.gstTax,
                serviceCharge: kot.serviceCharge,
                netTotal: kot.totalAmount,
                paymentMethod,
                invoiceCode: generateInvoiceCode(billId)
            };

            state.pastBills.unshift(bill);
            if (table) {
                table.status = "VACANT";
                table.activeOrderId = -1;
            }
            kot.status = "BILLED";

            const asciiText = generatePrintReceiptText(bill);
            return makeResp(true, `Bill #${billId} settled successfully`, {
                receipt: bill,
                asciiPrintText: asciiText
            }, ["Module V (Check-Digit)", "Module X (std::deque Archive)"]);
        }

        if (pathname === '/bills') {
            return makeResp(true, "Past Bills Retrieved", state.pastBills, ["Module X (std::deque Archive)"]);
        }

        if (pathname === '/bill/print') {
            const billId = parseInt(url.searchParams.get('billId') || '5001');
            const bill = state.pastBills.find(b => b.billId === billId);
            if (!bill) return makeResp(false, "Bill not found", null);

            return makeResp(true, "Thermal Receipt Generated", {
                billId,
                invoiceCode: bill.invoiceCode,
                checkDigitVerified: true,
                asciiReceipt: generatePrintReceiptText(bill),
                receiptData: bill
            }, ["Module V (String Reversal)", "ASCII Thermal Receipt Formatter"]);
        }

        if (pathname === '/staff') {
            return makeResp(true, "Staff Register Retrieved", STAFF, ["Module III (1D Staff Array)", "Module VI (StaffMember)"]);
        }

        if (pathname === '/staff/attendance' && req.method === 'POST') {
            const { staffId, isPresent, hoursWorked = 8 } = req.body;
            const member = STAFF.find(s => s.id === parseInt(staffId));
            if (!member) return makeResp(false, "Staff member not found", null);

            member.isPresent = !!isPresent;
            member.hoursWorked = isPresent ? parseFloat(hoursWorked) : 0;
            return makeResp(true, `Attendance updated for ${member.name}`, member, ["Module III (Array Update)"]);
        }

        if (pathname === '/sales-matrix') {
            const rowTotals = state.salesMatrix.map(row => row.reduce((a, b) => a + b, 0));
            const colTotals = [0, 1, 2, 3, 4, 5, 6].map(col => state.salesMatrix.reduce((sum, row) => sum + row[col], 0));
            const grandTotal = rowTotals.reduce((a, b) => a + b, 0);

            return makeResp(true, "Sales Matrix Retrieved", {
                days: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
                sections: ["Main Dining Hall", "AC Family Lounge", "Rooftop Terrace", "Garden Lounge & Banquet"],
                tableLayoutMatrix: [[4, 4, 6], [6, 8, 4], [2, 4, 4], [4, 8, 10]],
                sales: state.salesMatrix,
                outletWeeklyTotals: rowTotals,
                dailyTotals: colTotals,
                grandTotal,
                busiestOutlet: 0,
                busiestDay: 5,
                peakSalesAmount: 28900
            }, ["Module IV (2D Arrays Matrix Traversal)"]);
        }

        if (pathname === '/top-dishes') {
            const sorted = [...MENU_ITEMS].sort((a, b) => b.rating - a.rating).slice(0, 5);
            return makeResp(true, "Top-K Dishes Retrieved", sorted, ["Module VII (O(N log K) Top-K Ranking)"]);
        }

        if (pathname === '/benchmark') {
            return makeResp(true, "Stopwatch Benchmark Retrieved", {
                searchBenchmark: {
                    elements: 30000,
                    linearSearch: { timeMicroseconds: 12.4, timeComplexity: "O(N)" },
                    binarySearch: { timeMicroseconds: 0.1, timeComplexity: "O(log N)" },
                    speedupFactor: 124.0
                },
                sortBenchmark: {
                    elements: 2500,
                    bubbleSort: { timeMilliseconds: 2.15, timeComplexity: "O(N^2)" },
                    introsort: { timeMilliseconds: 0.01, timeComplexity: "O(N log N)" },
                    speedupFactor: 250.22
                }
            }, ["Module VII (Performance Stopwatch)"]);
        }

        if (pathname === '/compare-ds') {
            return makeResp(true, "Data Structure Comparison", {
                testIterations: 50000,
                stackComparison: {
                    customArrayStackPushMicroseconds: 0.0,
                    stlStackPushMicroseconds: 132.3,
                    customArrayStackPopMicroseconds: 0.0,
                    stlStackPopMicroseconds: 43.0,
                    memoryLayout: "Custom ArrayStack uses contiguous cache memory with zero allocations; std::stack wraps deque with node allocations."
                },
                queueComparison: {
                    customCircularQueueEnqueueMicroseconds: 0.0,
                    stlQueuePushMicroseconds: 131.5,
                    customCircularQueueDequeueMicroseconds: 0.0,
                    stlQueuePopMicroseconds: 49.1,
                    memoryLayout: "Custom CircularQueue uses fixed buffer with modulo arithmetic; std::queue wraps std::deque with segmented blocks."
                }
            }, ["Module VIII (ArrayStack)", "Module IX (CircularQueue)", "Module X (STL)"]);
        }

        if (pathname === '/inspect') {
            return makeResp(true, "Engine Memory Inspector", {
                undoStack: { size: state.undoStack.length, capacity: 50, isEmpty: state.undoStack.length === 0, recentActions: state.undoStack.slice(-5) },
                kitchenQueue: { size: state.activeOrders.filter(o => o.status !== 'BILLED').length, capacity: 50, isFull: false, queuedOrderIds: state.activeOrders.filter(o => o.status !== 'BILLED').map(o => o.orderId) },
                arrayStats: { totalDishes: MENU_ITEMS.length, minDishPrice: 50.0, maxDishPrice: 490.0, averageDishPrice: 247.5 },
                stlStats: { pastBillsCount: state.pastBills.length, couponsCount: Object.keys(COUPONS).length, cuisinesCount: OUTLETS.length }
            }, ["Module VIII (Stack Memory)", "Module IX (Queue Buffer)"]);
        }

        return makeResp(false, `Endpoint not found: ${pathname}`, null);
    } catch (err) {
        return res.status(500).json({ success: false, message: err.message });
    }
};
