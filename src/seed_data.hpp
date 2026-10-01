// FoodRush - Seed Data Initializer
// Provides realistic initial datasets for restaurants, dishes, zones, and riders
#ifndef SEED_DATA_HPP
#define SEED_DATA_HPP

#include "models.hpp"
#include <string>

// Populates initial restaurants and dishes into fixed-size arrays
inline void initializeSeedData(
    Restaurant restaurants[], int& restCount,
    MenuItem items[], int& itemCount,
    Rider riders[], int& riderCount)
{
    // =========================================================================
    // RESTAURANTS (4 Total)
    // =========================================================================
    restCount = 4;

    restaurants[0].id = 1;
    restaurants[0].name = "Bella Italia Trattoria";
    restaurants[0].cuisine = "Italian";
    restaurants[0].zoneId = 0; // Downtown
    restaurants[0].rating = 4.8;
    restaurants[0].isOpen = true;
    restaurants[0].itemCount = 6;

    restaurants[1].id = 2;
    restaurants[1].name = "Tokyo Ramen & Sushi";
    restaurants[1].cuisine = "Japanese";
    restaurants[1].zoneId = 1; // Uptown
    restaurants[1].rating = 4.9;
    restaurants[1].isOpen = true;
    restaurants[1].itemCount = 6;

    restaurants[2].id = 3;
    restaurants[2].name = "Spice Symphony";
    restaurants[2].cuisine = "Indian";
    restaurants[2].zoneId = 2; // Tech Park
    restaurants[2].rating = 4.7;
    restaurants[2].isOpen = true;
    restaurants[2].itemCount = 6;

    restaurants[3].id = 4;
    restaurants[3].name = "Burger & Brews Bistro";
    restaurants[3].cuisine = "American";
    restaurants[3].zoneId = 3; // Suburbs North
    restaurants[3].rating = 4.6;
    restaurants[3].isOpen = true;
    restaurants[3].itemCount = 6;

    // =========================================================================
    // MENU ITEMS (24 Total, 6 per restaurant)
    // =========================================================================
    itemCount = 24;

    // Bella Italia (IDs: 101-106)
    items[0] = {101, 1, "Wood-Fired Margherita Pizza", "Pizza", 14.50, 25, true, 780, 4}; // Chef Special
    items[1] = {102, 1, "Spicy Penne Arrabbiata", "Pasta", 13.25, 20, true, 620, 1}; // Spicy
    items[2] = {103, 1, "Truffle Mushroom Fettuccine", "Pasta", 17.95, 15, true, 810, 4}; // Chef Special
    items[3] = {104, 1, "Prosciutto & Arugula Flatbread", "Pizza", 16.50, 18, false, 740, 0};
    items[4] = {105, 1, "Classic Caesar Salad", "Salads", 9.50, 30, false, 320, 2}; // GlutenFree
    items[5] = {106, 1, "Authentic Espresso Tiramisu", "Desserts", 7.50, 22, true, 410, 0};

    // Tokyo Ramen & Sushi (IDs: 201-206)
    items[6] = {201, 2, "Rich Tonkotsu Black Garlic Ramen", "Ramen", 15.95, 30, false, 890, 4}; // Chef Special
    items[7] = {202, 2, "Spicy Miso Ramen", "Ramen", 14.75, 25, false, 820, 1}; // Spicy
    items[8] = {203, 2, "Salmon & Tuna Nigiri Combo (8 pcs)", "Sushi", 19.50, 15, false, 510, 2}; // GlutenFree
    items[9] = {204, 2, "Crispy Vegetable Tempura Roll", "Sushi", 11.25, 20, true, 420, 0};
    items[10] = {205, 2, "Pan-Seared Pork Gyoza (6 pcs)", "Appetizers", 8.50, 35, false, 380, 0};
    items[11] = {206, 2, "Matcha Green Tea Mochi Ice Cream", "Desserts", 6.25, 40, true, 260, 2}; // GlutenFree

    // Spice Symphony (IDs: 301-306)
    items[12] = {301, 3, "Velvety Butter Chicken & Basmati", "Curry", 16.95, 25, false, 850, 4}; // Chef Special
    items[13] = {302, 3, "Paneer Tikka Masala", "Curry", 14.50, 20, true, 730, 2}; // GlutenFree
    items[14] = {303, 3, "Fiery Lamb Rogan Josh", "Curry", 18.25, 18, false, 890, 1}; // Spicy
    items[15] = {304, 3, "Hyderabadi Dum Biryani", "Rice", 15.50, 22, false, 910, 1}; // Spicy
    items[16] = {305, 3, "Garlic Butter Naan Bread (2 pcs)", "Breads", 4.25, 50, true, 310, 0};
    items[17] = {306, 3, "Warm Gulab Jamun with Pistachio", "Desserts", 5.50, 30, true, 380, 0};

    // Burger & Brews Bistro (IDs: 401-406)
    items[18] = {401, 4, "Smoked Bacon Cheddar Smash Burger", "Burgers", 14.95, 30, false, 950, 4}; // Chef Special
    items[19] = {402, 4, "Nashville Hot Crispy Chicken Sandwich", "Burgers", 13.75, 25, false, 870, 1}; // Spicy
    items[20] = {403, 4, "Beyond Meat Truffle Veggie Burger", "Burgers", 15.25, 20, true, 680, 0};
    items[21] = {404, 4, "Loaded Buffalo Wings with Blue Cheese", "Sides", 12.50, 28, false, 760, 1}; // Spicy
    items[22] = {405, 4, "Seasoned Crispy Curly Fries", "Sides", 5.50, 45, true, 410, 2}; // GlutenFree
    items[23] = {406, 4, "Hand-Spun Salted Caramel Shake", "Beverages", 6.75, 30, true, 560, 0};

    // Assign restaurant item IDs mapping
    for (int r = 0; r < restCount; ++r) {
        for (int i = 0; i < 6; ++i) {
            restaurants[r].itemIds[i] = items[r * 6 + i].id;
        }
    }

    // =========================================================================
    // RIDERS (5 Total)
    // =========================================================================
    riderCount = 5;

    riders[0] = {1, "Alex Walker", "Electric Cargo Bike", 0, true, 142};
    riders[1] = {2, "Chloe Bennett", "Vespa Scooter", 1, true, 215};
    riders[2] = {3, "David Miller", "Sport Motorcycle", 2, true, 98};
    riders[3] = {4, "Emma Hayes", "Electric Scooter", 3, true, 176};
    riders[4] = {5, "Liam Vance", "E-Bike Pro", 4, true, 131};
}

#endif // SEED_DATA_HPP
