// FoodRush - C++ Delivery System Models
// [MODULE VI] Structures (declaration, nested, arrays of structs, struct methods) | used by: RESTAURANT_AND_ORDER_MODELS
#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <sstream>
#include <iomanip>

// [MODULE I] Named Constants for fixed-size memory structures
const int MAX_ITEMS_PER_RESTAURANT = 10;
const int MAX_TOTAL_ITEMS = 40;
const int MAX_RESTAURANTS = 4;
const int MAX_ZONES = 5;
const int MAX_CART_ITEMS = 15;
const int MAX_ORDERS_HISTORY = 50;
const int MAX_RIDERS = 5;

// [MODULE I] Financial and System Constants
const double TAX_RATE = 0.0825;           // 8.25% Sales tax
const double BASE_DELIVERY_FEE = 2.49;    // Base fee
const double PER_KM_FEE = 0.65;           // Per km rate
const double EXPRESS_SURCHARGE = 3.99;    // Express order priority surcharge

// [MODULE VI] Nested Structure: Address
struct Address {
    std::string street;
    int zoneId; // 0 to MAX_ZONES-1
    std::string zipCode;
    std::string instructions;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"street\":\"" << street << "\","
           << "\"zoneId\":" << zoneId << ","
           << "\"zipCode\":\"" << zipCode << "\","
           << "\"instructions\":\"" << instructions << "\""
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: MenuItem
struct MenuItem {
    int id;
    int restaurantId;
    std::string name;
    std::string category;
    double price;
    int stock;
    bool isVeg;
    int calories;
    unsigned int dietaryFlags; // [MODULE I] Bitwise operator flags: 1=Spicy, 2=GlutenFree, 4=ChefSpecial

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"id\":" << id << ","
           << "\"restaurantId\":" << restaurantId << ","
           << "\"name\":\"" << name << "\","
           << "\"category\":\"" << category << "\","
           << "\"price\":" << price << ","
           << "\"stock\":" << stock << ","
           << "\"isVeg\":" << (isVeg ? "true" : "false") << ","
           << "\"calories\":" << calories << ","
           << "\"isSpicy\":" << ((dietaryFlags & 1) ? "true" : "false") << ","
           << "\"isGlutenFree\":" << ((dietaryFlags & 2) ? "true" : "false") << ","
           << "\"isChefSpecial\":" << ((dietaryFlags & 4) ? "true" : "false")
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: Restaurant
struct Restaurant {
    int id;
    std::string name;
    std::string cuisine;
    int zoneId;
    double rating; // e.g. 4.8
    bool isOpen;
    int itemCount;
    int itemIds[MAX_ITEMS_PER_RESTAURANT]; // Array within struct

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"cuisine\":\"" << cuisine << "\","
           << "\"zoneId\":" << zoneId << ","
           << "\"rating\":" << rating << ","
           << "\"isOpen\":" << (isOpen ? "true" : "false") << ","
           << "\"itemCount\":" << itemCount
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: CartItem
struct CartItem {
    int itemId;
    int restaurantId;
    std::string name;
    double price;
    int quantity;

    double getLineTotal() const {
        return price * quantity;
    }

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"itemId\":" << itemId << ","
           << "\"restaurantId\":" << restaurantId << ","
           << "\"name\":\"" << name << "\","
           << "\"price\":" << price << ","
           << "\"quantity\":" << quantity << ","
           << "\"lineTotal\":" << getLineTotal()
           << "}";
        return ss.str();
    }
};

// Cart Action representation for the Undo Stack
enum CartActionType {
    ACTION_ADD_ITEM = 1,
    ACTION_REMOVE_ITEM = 2,
    ACTION_UPDATE_QTY = 3
};

struct CartAction {
    CartActionType type;
    int itemId;
    int previousQuantity;
    int newQuantity;
    double itemPrice;
    std::string itemName;

    std::string toJSON() const {
        std::ostringstream ss;
        std::string typeStr = (type == ACTION_ADD_ITEM ? "ADD" : (type == ACTION_REMOVE_ITEM ? "REMOVE" : "UPDATE_QTY"));
        ss << "{"
           << "\"type\":\"" << typeStr << "\","
           << "\"itemId\":" << itemId << ","
           << "\"itemName\":\"" << itemName << "\","
           << "\"prevQty\":" << previousQuantity << ","
           << "\"newQty\":" << newQuantity
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: Order (Nested structs Address + array of CartItem)
struct Order {
    int orderId;
    int restaurantId;
    std::string customerName;
    Address deliveryAddress;           // Nested struct
    CartItem items[MAX_CART_ITEMS];    // Array of structs
    int itemCount;
    double subtotal;
    double tax;
    double deliveryFee;
    double discount;
    double totalAmount;
    bool isExpress;
    std::string status;                // "PLACED", "PREPARING", "OUT_FOR_DELIVERY", "DELIVERED"
    int assignedRiderId;               // -1 if none yet
    std::string riderName;
    int estimatedMinutes;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"orderId\":" << orderId << ","
           << "\"restaurantId\":" << restaurantId << ","
           << "\"customerName\":\"" << customerName << "\","
           << "\"address\":" << deliveryAddress.toJSON() << ","
           << "\"itemCount\":" << itemCount << ","
           << "\"items\":[";
        for (int i = 0; i < itemCount; ++i) {
            ss << items[i].toJSON();
            if (i < itemCount - 1) ss << ",";
        }
        ss << "],"
           << "\"subtotal\":" << subtotal << ","
           << "\"tax\":" << tax << ","
           << "\"deliveryFee\":" << deliveryFee << ","
           << "\"discount\":" << discount << ","
           << "\"totalAmount\":" << totalAmount << ","
           << "\"isExpress\":" << (isExpress ? "true" : "false") << ","
           << "\"status\":\"" << status << "\","
           << "\"assignedRiderId\":" << assignedRiderId << ","
           << "\"riderName\":\"" << riderName << "\","
           << "\"estimatedMinutes\":" << estimatedMinutes
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: Rider
struct Rider {
    int id;
    std::string name;
    std::string vehicle; // "E-Bike", "Scooter", "Motorcycle"
    int currentZone;
    bool isAvailable;
    int totalDeliveries;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"vehicle\":\"" << vehicle << "\","
           << "\"currentZone\":" << currentZone << ","
           << "\"isAvailable\":" << (isAvailable ? "true" : "false") << ","
           << "\"totalDeliveries\":" << totalDeliveries
           << "}";
        return ss.str();
    }
};

#endif // MODELS_HPP
