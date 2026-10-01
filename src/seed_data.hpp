// FoodRush - Realistic Seed Dataset
// 6 Restaurants, 8 Items Each (48 total dishes), 5 Neutral Delivery Zones, 5 Delivery Riders
#ifndef SEED_DATA_HPP
#define SEED_DATA_HPP

#include "models.hpp"
#include <string>

inline void initializeSeedData(
    Restaurant restaurants[], int& restCount,
    MenuItem items[], int& itemCount,
    Rider riders[], int& riderCount)
{
    // =========================================================================
    // 6 RESTAURANTS (Central, North, South, East, West)
    // =========================================================================
    restCount = 6;

    // Restaurant 1: Royal Dum Biryani
    restaurants[0] = {
        1, "Royal Dum Biryani", "Biryani & Mughlai", 0, 4.9, true, 8,
        {}, 30, 200.00
    };

    // Restaurant 2: Sagar Dosa & Tiffin
    restaurants[1] = {
        2, "Sagar Dosa & Tiffin", "South Indian", 2, 4.8, true, 8,
        {}, 25, 120.00
    };

    // Restaurant 3: Bella Italia Trattoria
    restaurants[2] = {
        3, "Bella Italia Trattoria", "Artisan Italian", 1, 4.7, true, 8,
        {}, 35, 250.00
    };

    // Restaurant 4: Tokyo Ramen & Robata
    restaurants[3] = {
        4, "Tokyo Ramen & Robata", "Japanese", 3, 4.9, true, 8,
        {}, 32, 280.00
    };

    // Restaurant 5: The Burger & Brews Co.
    restaurants[4] = {
        5, "The Burger & Brews Co.", "American Gourmet", 4, 4.6, true, 8,
        {}, 28, 180.00
    };

    // Restaurant 6: Sweet Tooth Patisserie
    restaurants[5] = {
        6, "Sweet Tooth Patisserie", "Desserts & Bakery", 0, 4.8, true, 8,
        {}, 20, 150.00
    };

    // =========================================================================
    // 48 MENU ITEMS (8 per restaurant)
    // =========================================================================
    itemCount = 48;

    // --- Royal Dum Biryani (IDs 101 - 108) ---
    items[0] = {101, 1, "Hyderabadi Chicken Dum Biryani", "Biryani", 320.00, 30, false, 840, 5}; // ChefSpecial + Spicy
    items[1] = {102, 1, "Awadhi Mutton Dum Biryani", "Biryani", 440.00, 20, false, 960, 4}; // ChefSpecial
    items[2] = {103, 1, "Nawabi Paneer Tikka Biryani", "Biryani", 280.00, 25, true, 720, 2}; // GlutenFree
    items[3] = {104, 1, "Murgh Malai Chicken Tikka", "Kebabs", 290.00, 25, false, 580, 0};
    items[4] = {105, 1, "Fiery Andhra Chicken Fry", "Starters", 260.00, 22, false, 620, 1}; // Spicy
    items[5] = {106, 1, "Shahi Mirchi Ka Salan", "Sides", 90.00, 40, true, 210, 1}; // Spicy
    items[6] = {107, 1, "Burani Garlic Raita", "Sides", 60.00, 50, true, 140, 2}; // GlutenFree
    items[7] = {108, 1, "Zafrani Matka Phirni", "Desserts", 130.00, 35, true, 340, 0};

    // --- Sagar Dosa & Tiffin (IDs 201 - 208) ---
    items[8]  = {201, 2, "Ghee Roast Masala Dosa", "Dosa", 160.00, 40, true, 420, 4}; // ChefSpecial
    items[9]  = {202, 2, "Mysore Onion Rava Dosa", "Dosa", 175.00, 35, true, 460, 1}; // Spicy
    items[10] = {203, 2, "Steamed Button Idli (4 pcs)", "Tiffin", 95.00, 50, true, 240, 2}; // GlutenFree
    items[11] = {204, 2, "Crispy Medu Vada (2 pcs)", "Tiffin", 90.00, 45, true, 310, 0};
    items[12] = {205, 2, "Chettinad Spicy Paneer Dosa", "Dosa", 195.00, 30, true, 510, 1}; // Spicy
    items[13] = {206, 2, "Bisi Bele Bath with Boondi", "Rice", 140.00, 30, true, 480, 0};
    items[14] = {207, 2, "Pineapple Kesari Halwa", "Desserts", 110.00, 35, true, 390, 0};
    items[15] = {208, 2, "Filter Degree Coffee", "Beverages", 50.00, 60, true, 120, 0};

    // --- Bella Italia Trattoria (IDs 301 - 308) ---
    items[16] = {301, 3, "Wood-Fired Margherita Pizza", "Pizza", 390.00, 25, true, 780, 4}; // ChefSpecial
    items[17] = {302, 3, "Spicy Penne Arrabbiata", "Pasta", 340.00, 20, true, 610, 1}; // Spicy
    items[18] = {303, 3, "Truffle Wild Mushroom Fettuccine", "Pasta", 460.00, 15, true, 790, 4}; // ChefSpecial
    items[19] = {304, 3, "Smoked Chicken & Jalapeno Pizza", "Pizza", 440.00, 20, false, 860, 1}; // Spicy
    items[20] = {305, 3, "Pesto Genovese Gnocchi", "Pasta", 380.00, 18, true, 640, 2}; // GlutenFree
    items[21] = {306, 3, "Rosemary Garlic Focaccia", "Breads", 140.00, 30, true, 320, 0};
    items[22] = {307, 3, "Burrata Caprese Salad", "Salads", 310.00, 20, true, 390, 2}; // GlutenFree
    items[23] = {308, 3, "Classic Espresso Tiramisu", "Desserts", 240.00, 25, true, 410, 0};

    // --- Tokyo Ramen & Robata (IDs 401 - 408) ---
    items[24] = {401, 4, "Rich Tonkotsu Black Garlic Ramen", "Ramen", 480.00, 25, false, 890, 4}; // ChefSpecial
    items[25] = {402, 4, "Spicy Miso Tofu Ramen", "Ramen", 410.00, 25, true, 720, 1}; // Spicy
    items[26] = {403, 4, "Crispy Vegetable Tempura Roll", "Sushi", 350.00, 20, true, 410, 0};
    items[27] = {404, 4, "Salmon & Avocado Maki (8 pcs)", "Sushi", 490.00, 15, false, 480, 2}; // GlutenFree
    items[28] = {405, 4, "Pan-Seared Chicken Gyoza (6 pcs)", "Dim Sum", 280.00, 30, false, 360, 0};
    items[29] = {406, 4, "Steamed Truffle Edamame", "Appetizers", 220.00, 35, true, 190, 2}; // GlutenFree
    items[30] = {407, 4, "Karaage Japanese Fried Chicken", "Starters", 320.00, 25, false, 610, 1}; // Spicy
    items[31] = {408, 4, "Matcha Green Tea Mochi (3 pcs)", "Desserts", 180.00, 30, true, 260, 2}; // GlutenFree

    // --- The Burger & Brews Co. (IDs 501 - 508) ---
    items[32] = {501, 5, "Smoked Bacon Cheddar Smash Burger", "Burgers", 360.00, 30, false, 920, 4}; // ChefSpecial
    items[33] = {502, 5, "Nashville Hot Crispy Chicken Burger", "Burgers", 330.00, 25, false, 840, 1}; // Spicy
    items[34] = {503, 5, "Truffle Wild Mushroom Veggie Burger", "Burgers", 290.00, 25, true, 670, 0};
    items[35] = {504, 5, "Peri-Peri Seasoned Curly Fries", "Sides", 140.00, 50, true, 390, 1}; // Spicy
    items[36] = {505, 5, "Smoky BBQ Glazed Wings (6 pcs)", "Starters", 310.00, 25, false, 680, 0};
    items[37] = {506, 5, "Crispy Onion Rings with Garlic Dip", "Sides", 150.00, 40, true, 340, 0};
    items[38] = {507, 5, "Hand-Spun Salted Caramel Shake", "Beverages", 190.00, 35, true, 510, 0};
    items[39] = {508, 5, "Molten Chocolate Lava Cake", "Desserts", 210.00, 30, true, 540, 4}; // ChefSpecial

    // --- Sweet Tooth Patisserie (IDs 601 - 608) ---
    items[40] = {601, 6, "Belgian Dark Chocolate Ganache Pastry", "Pastries", 180.00, 30, true, 440, 4}; // ChefSpecial
    items[41] = {602, 6, "Blueberry Baked New York Cheesecake", "Cakes", 240.00, 25, true, 490, 0};
    items[42] = {603, 6, "Almond French Croissant", "Bakery", 150.00, 35, true, 380, 0};
    items[43] = {604, 6, "Pistachio Raspberry Macarons (4 pcs)", "Desserts", 220.00, 20, true, 310, 2}; // GlutenFree
    items[44] = {605, 6, "Warm Cinnamon Sugar Churros (4 pcs)", "Desserts", 170.00, 25, true, 410, 0};
    items[45] = {606, 6, "Red Velvet Cream Cheese Slice", "Cakes", 190.00, 30, true, 460, 0};
    items[46] = {607, 6, "Double Chocolate Walnut Fudge Brownie", "Pastries", 160.00, 40, true, 480, 0};
    items[47] = {608, 6, "Artisanal Iced Cold Brew", "Beverages", 140.00, 50, true, 40, 2}; // GlutenFree

    // Map item IDs to parent restaurants
    for (int r = 0; r < restCount; ++r) {
        for (int i = 0; i < 8; ++i) {
            restaurants[r].itemIds[i] = items[r * 8 + i].id;
        }
    }

    // =========================================================================
    // 5 RIDERS (With contact details and vehicle types)
    // =========================================================================
    riderCount = 5;

    riders[0] = {1, "Rahul Sharma", "Electric Cargo Bike", 0, true, 184, "+91 98765 43210"};
    riders[1] = {2, "Priya Nair", "Ather Electric Scooter", 1, true, 241, "+91 98765 43211"};
    riders[2] = {3, "Vikram Malhotra", "Hero Splendor EV", 2, true, 119, "+91 98765 43212"};
    riders[3] = {4, "Amitav Sen", "TVS iQube Scooter", 3, true, 195, "+91 98765 43213"};
    riders[4] = {5, "Kavita Rao", "Ola S1 Pro Electric", 4, true, 162, "+91 98765 43214"};
}

#endif // SEED_DATA_HPP
