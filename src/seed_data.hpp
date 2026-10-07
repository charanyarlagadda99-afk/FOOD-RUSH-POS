// FoodRush / RestoRush - Realistic Seed Dataset for Hotel & Restaurant POS
// 6 Outlets, 48 Dishes, 12 Dining Tables, 8 Staff Members
#ifndef SEED_DATA_HPP
#define SEED_DATA_HPP

#include "models.hpp"
#include <string>

inline void initializeSeedData(
    RestaurantOutlet outlets[], int& outletCount,
    MenuItem items[], int& itemCount,
    Table tables[], int& tableCount,
    StaffMember staff[], int& staffCount)
{
    // =========================================================================
    // 6 RESTAURANT & HOTEL KITCHEN OUTLETS
    // =========================================================================
    outletCount = 6;

    outlets[0] = {1, "Grand Mughal Dining", "Biryani & Mughlai", "Main Dining Hall", 4.9, true, 8, {}};
    outlets[1] = {2, "Dakshin Tiffin & Cafe", "South Indian", "Ground Floor Cafe", 4.8, true, 8, {}};
    outlets[2] = {3, "Trattoria Bella Vista", "Artisan Italian", "First Floor Gallery", 4.7, true, 8, {}};
    outlets[3] = {4, "Sakura Asian Bistro", "Japanese & Asian", "AC Fine Dining", 4.9, true, 8, {}};
    outlets[4] = {5, "The Boulevard Grill & Burgers", "American Gourmet", "Terrace Garden", 4.6, true, 8, {}};
    outlets[5] = {6, "The Royal Patisserie", "Desserts & Confectionery", "Lobby Lounge", 4.8, true, 8, {}};

    // =========================================================================
    // 48 MENU ITEMS (8 per outlet)
    // =========================================================================
    itemCount = 48;

    // --- Grand Mughal Dining (IDs 101 - 108) ---
    items[0] = {101, 1, "Hyderabadi Chicken Dum Biryani", "Biryani", 320.00, 30, false, 840, 5, 4.9, 64, 0};
    items[1] = {102, 1, "Awadhi Mutton Dum Biryani", "Biryani", 440.00, 20, false, 960, 4, 4.8, 42, 0};
    items[2] = {103, 1, "Nawabi Paneer Tikka Biryani", "Biryani", 280.00, 25, true, 720, 2, 4.7, 31, 0};
    items[3] = {104, 1, "Murgh Malai Chicken Tikka", "Kebabs", 290.00, 25, false, 580, 0, 4.6, 28, 0};
    items[4] = {105, 1, "Fiery Andhra Chicken Fry", "Starters", 260.00, 22, false, 620, 1, 4.7, 35, 0};
    items[5] = {106, 1, "Shahi Mirchi Ka Salan", "Sides", 90.00, 40, true, 210, 1, 4.5, 20, 0};
    items[6] = {107, 1, "Burani Garlic Raita", "Sides", 60.00, 50, true, 140, 2, 4.6, 25, 0};
    items[7] = {108, 1, "Zafrani Matka Phirni", "Desserts", 130.00, 35, true, 340, 0, 4.9, 52, 0};

    // --- Dakshin Tiffin & Cafe (IDs 201 - 208) ---
    items[8]  = {201, 2, "Ghee Roast Masala Dosa", "Dosa", 160.00, 40, true, 420, 4, 4.9, 78, 0};
    items[9]  = {202, 2, "Mysore Onion Rava Dosa", "Dosa", 175.00, 35, true, 460, 1, 4.8, 45, 0};
    items[10] = {203, 2, "Steamed Button Idli (4 pcs)", "Tiffin", 95.00, 50, true, 240, 2, 4.7, 50, 0};
    items[11] = {204, 2, "Crispy Medu Vada (2 pcs)", "Tiffin", 90.00, 45, true, 310, 0, 4.6, 38, 0};
    items[12] = {205, 2, "Chettinad Spicy Paneer Dosa", "Dosa", 195.00, 30, true, 510, 1, 4.8, 36, 0};
    items[13] = {206, 2, "Bisi Bele Bath with Boondi", "Rice", 140.00, 30, true, 480, 0, 4.5, 29, 0};
    items[14] = {207, 2, "Pineapple Kesari Halwa", "Desserts", 110.00, 35, true, 390, 0, 4.7, 40, 0};
    items[15] = {208, 2, "Filter Degree Coffee", "Beverages", 50.00, 60, true, 120, 0, 4.9, 90, 0};

    // --- Trattoria Bella Vista (IDs 301 - 308) ---
    items[16] = {301, 3, "Wood-Fired Margherita Pizza", "Pizza", 390.00, 25, true, 780, 4, 4.8, 55, 0};
    items[17] = {302, 3, "Spicy Penne Arrabbiata", "Pasta", 340.00, 20, true, 610, 1, 4.6, 30, 0};
    items[18] = {303, 3, "Truffle Wild Mushroom Fettuccine", "Pasta", 460.00, 15, true, 790, 4, 4.9, 44, 0};
    items[19] = {304, 3, "Smoked Chicken & Jalapeno Pizza", "Pizza", 440.00, 20, false, 860, 1, 4.7, 33, 0};
    items[20] = {305, 3, "Pesto Genovese Gnocchi", "Pasta", 380.00, 18, true, 640, 2, 4.5, 22, 0};
    items[21] = {306, 3, "Rosemary Garlic Focaccia", "Breads", 140.00, 30, true, 320, 0, 4.6, 26, 0};
    items[22] = {307, 3, "Burrata Caprese Salad", "Salads", 310.00, 20, true, 390, 2, 4.7, 27, 0};
    items[23] = {308, 3, "Classic Espresso Tiramisu", "Desserts", 240.00, 25, true, 410, 0, 4.9, 61, 0};

    // --- Sakura Asian Bistro (IDs 401 - 408) ---
    items[24] = {401, 4, "Rich Tonkotsu Black Garlic Ramen", "Ramen", 480.00, 25, false, 890, 4, 4.9, 72, 0};
    items[25] = {402, 4, "Spicy Miso Tofu Ramen", "Ramen", 410.00, 25, true, 720, 1, 4.7, 39, 0};
    items[26] = {403, 4, "Crispy Vegetable Tempura Roll", "Sushi", 350.00, 20, true, 410, 0, 4.6, 34, 0};
    items[27] = {404, 4, "Salmon & Avocado Maki (8 pcs)", "Sushi", 490.00, 15, false, 480, 2, 4.8, 48, 0};
    items[28] = {405, 4, "Pan-Seared Chicken Gyoza (6 pcs)", "Dim Sum", 280.00, 30, false, 360, 0, 4.7, 37, 0};
    items[29] = {406, 4, "Steamed Truffle Edamame", "Appetizers", 220.00, 35, true, 190, 2, 4.5, 21, 0};
    items[30] = {407, 4, "Karaage Japanese Fried Chicken", "Starters", 320.00, 25, false, 610, 1, 4.8, 46, 0};
    items[31] = {408, 4, "Matcha Green Tea Mochi (3 pcs)", "Desserts", 180.00, 30, true, 260, 2, 4.8, 50, 0};

    // --- The Boulevard Grill & Burgers (IDs 501 - 508) ---
    items[32] = {501, 5, "Smoked Bacon Cheddar Smash Burger", "Burgers", 360.00, 30, false, 920, 4, 4.8, 59, 0};
    items[33] = {502, 5, "Nashville Hot Crispy Chicken Burger", "Burgers", 330.00, 25, false, 840, 1, 4.7, 43, 0};
    items[34] = {503, 5, "Truffle Wild Mushroom Veggie Burger", "Burgers", 290.00, 25, true, 670, 0, 4.6, 32, 0};
    items[35] = {504, 5, "Peri-Peri Seasoned Curly Fries", "Sides", 140.00, 50, true, 390, 1, 4.7, 45, 0};
    items[36] = {505, 5, "Smoky BBQ Glazed Wings (6 pcs)", "Starters", 310.00, 25, false, 680, 0, 4.6, 38, 0};
    items[37] = {506, 5, "Crispy Onion Rings with Garlic Dip", "Sides", 150.00, 40, true, 340, 0, 4.5, 29, 0};
    items[38] = {507, 5, "Hand-Spun Salted Caramel Shake", "Beverages", 190.00, 35, true, 510, 0, 4.8, 41, 0};
    items[39] = {508, 5, "Molten Chocolate Lava Cake", "Desserts", 210.00, 30, true, 540, 4, 4.9, 65, 0};

    // --- The Royal Patisserie (IDs 601 - 608) ---
    items[40] = {601, 6, "Belgian Dark Chocolate Ganache Pastry", "Pastries", 180.00, 30, true, 440, 4, 4.9, 70, 0};
    items[41] = {602, 6, "Blueberry Baked New York Cheesecake", "Cakes", 240.00, 25, true, 490, 0, 4.8, 52, 0};
    items[42] = {603, 6, "Almond French Croissant", "Bakery", 150.00, 35, true, 380, 0, 4.7, 36, 0};
    items[43] = {604, 6, "Pistachio Raspberry Macarons (4 pcs)", "Desserts", 220.00, 20, true, 310, 2, 4.8, 40, 0};
    items[44] = {605, 6, "Warm Cinnamon Sugar Churros (4 pcs)", "Desserts", 170.00, 25, true, 410, 0, 4.6, 28, 0};
    items[45] = {606, 6, "Red Velvet Cream Cheese Slice", "Cakes", 190.00, 30, true, 460, 0, 4.7, 35, 0};
    items[46] = {607, 6, "Double Chocolate Walnut Fudge Brownie", "Pastries", 160.00, 40, true, 480, 0, 4.8, 48, 0};
    items[47] = {608, 6, "Artisanal Iced Cold Brew", "Beverages", 140.00, 50, true, 40, 2, 4.9, 58, 0};

    // =========================================================================
    // 12 DINING TABLES (Across 4 Hotel / Restaurant Sections)
    // =========================================================================
    tableCount = 12;

    // Section 1: Main Dining Hall
    tables[0] = {1, "Table 1 (Family)", "Main Dining Hall", 4, "VACANT", -1, "Vikram Malhotra"};
    tables[1] = {2, "Table 2 (Booth)", "Main Dining Hall", 4, "OCCUPIED", 1001, "Vikram Malhotra"};
    tables[2] = {3, "Table 3 (Round)", "Main Dining Hall", 6, "VACANT", -1, "Amitav Sen"};

    // Section 2: AC Family Lounge
    tables[3] = {4, "Table 4 (Central)", "AC Family Lounge", 6, "VACANT", -1, "Priya Nair"};
    tables[4] = {5, "Table 5 (Executive)", "AC Family Lounge", 8, "OCCUPIED", 1002, "Priya Nair"};
    tables[5] = {6, "Table 6 (Corner)", "AC Family Lounge", 4, "VACANT", -1, "Priya Nair"};

    // Section 3: Rooftop Terrace
    tables[6] = {7, "Table 7 (Couple)", "Rooftop Terrace", 2, "VACANT", -1, "Rohit Verma"};
    tables[7] = {8, "Table 8 (Sky View)", "Rooftop Terrace", 4, "OCCUPIED", 1003, "Rohit Verma"};
    tables[8] = {9, "Table 9 (Sunset)", "Rooftop Terrace", 4, "VACANT", -1, "Rohit Verma"};

    // Section 4: Garden Lounge & Banquet
    tables[9]  = {10, "Table 10 (Garden)", "Garden Lounge & Banquet", 4, "VACANT", -1, "Amitav Sen"};
    tables[10] = {11, "Table 11 (Banquet A)", "Garden Lounge & Banquet", 8, "VACANT", -1, "Sunita Sharma"};
    tables[11] = {12, "Table 12 (Banquet B)", "Garden Lounge & Banquet", 10, "VACANT", -1, "Sunita Sharma"};

    // =========================================================================
    // 8 STAFF MEMBERS (Chefs, Waiters, Captains, Cashiers)
    // =========================================================================
    staffCount = 8;

    staff[0] = {1, "Chef Rajesh Kumar", "Head Chef", "Morning (8am-4pm)", true, 7.5, "+91 98765 43201"};
    staff[1] = {2, "Chef Antonio Rossi", "Sous Chef", "Evening (4pm-12am)", true, 6.0, "+91 98765 43202"};
    staff[2] = {3, "Vikram Malhotra", "Captain Waiter", "Morning (8am-4pm)", true, 8.0, "+91 98765 43203"};
    staff[3] = {4, "Priya Nair", "Senior Table Server", "Evening (4pm-12am)", true, 7.0, "+91 98765 43204"};
    staff[4] = {5, "Amitav Sen", "Table Server", "Morning (8am-4pm)", true, 8.0, "+91 98765 43205"};
    staff[5] = {6, "Kavita Rao", "Head Cashier & Billing", "Morning (8am-4pm)", true, 8.0, "+91 98765 43206"};
    staff[6] = {7, "Rohit Verma", "Beverage & Barista", "Evening (4pm-12am)", true, 7.5, "+91 98765 43207"};
    staff[7] = {8, "Sunita Sharma", "Floor Supervisor", "Full Day", true, 9.0, "+91 98765 43208"};
}

#endif // SEED_DATA_HPP
