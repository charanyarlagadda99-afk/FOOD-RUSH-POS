// FoodRush - Domain Models and System Constants
// [MODULE VI] Structures (declaration, nested, arrays of structs, struct methods) | used by: RESTAURANT_AND_ORDER_MODELS
#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <sstream>
#include <iomanip>

// [MODULE I] Named Constants for Fixed-Capacity Memory Structures
const int MAX_RESTAURANTS = 6;
const int MAX_ITEMS_PER_RESTAURANT = 10;
const int MAX_TOTAL_ITEMS = 60;
const int MAX_ZONES = 5;
const int MAX_CART_ITEMS = 20;
const int MAX_ORDERS_HISTORY = 100;
const int MAX_RIDERS = 5;

// [MODULE I] Financial and Operational Constants (Currency: INR ₹)
const double TAX_RATE = 0.05;              // 5% Goods and Services Tax (GST)
const double BASE_DELIVERY_FEE = 35.00;    // Base delivery fee in ₹
const double PER_KM_FEE = 8.50;            // Per km rate in ₹
const double EXPRESS_SURCHARGE = 45.00;    // Express priority kitchen surcharge in ₹
const char CURRENCY_SYMBOL[] = "\xE2\x82\xB9"; // UTF-8 byte sequence for ₹

// [MODULE VI] Nested Structure: Address
struct Address {
    std::string street;
    int zoneId; // 0=Central, 1=North, 2=South, 3=East, 4=West
    std::string landmark;
    std::string instructions;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"street\":\"" << street << "\","
           << "\"zoneId\":" << zoneId << ","
           << "\"landmark\":\"" << landmark << "\","
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
    unsigned int dietaryFlags; // [MODULE I] Bitwise flags: 1=Spicy, 2=GlutenFree, 4=ChefSpecial
    double rating;             // [FEATURE 3] Moving average rating (1.0 to 5.0)
    int ratingCount;           // [FEATURE 3] Total ratings received
    int reservedStock;         // [FEATURE 2] Real-time stock locked in active carts

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
           << "\"reservedStock\":" << reservedStock << ","
           << "\"availableStock\":" << (stock - reservedStock) << ","
           << "\"rating\":" << std::setprecision(1) << rating << ","
           << "\"ratingCount\":" << ratingCount << ","
           << std::setprecision(2)
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
    double rating;        // e.g. 4.8
    bool isOpen;
    int itemCount;
    int itemIds[MAX_ITEMS_PER_RESTAURANT]; // Fixed 1D array within struct
    int etaMinutes;
    double minOrder;

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
           << "\"itemCount\":" << itemCount << ","
           << "\"etaMinutes\":" << etaMinutes << ","
           << "\"minOrder\":" << std::setprecision(2) << minOrder
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

// Cart Action types for LIFO Undo Stack
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
    std::string trackingCode;          // e.g. "TRK-1001-6" (verified via string reversal check-digit)
    int restaurantId;
    std::string restaurantName;
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
    int queuePosition;                 // Live position in kitchen circular queue
    double dispatchDistance;           // [FEATURE 1] Nearest rider spatial distance in km

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"orderId\":" << orderId << ","
           << "\"trackingCode\":\"" << trackingCode << "\","
           << "\"restaurantId\":" << restaurantId << ","
           << "\"restaurantName\":\"" << restaurantName << "\","
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
           << "\"estimatedMinutes\":" << estimatedMinutes << ","
           << "\"queuePosition\":" << queuePosition << ","
           << "\"dispatchDistance\":" << dispatchDistance
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: Rider
struct Rider {
    int id;
    std::string name;
    std::string vehicle; // "Electric Cargo Bike", "Hero Electric Scooter", etc.
    int currentZone;
    bool isAvailable;
    int totalDeliveries;
    std::string phone;
    int activeOrderId;         // [FEATURE 1] -1 if idle, otherwise currently assigned order
    double speedMultiplier;    // [FEATURE 1] Vehicle speed coefficient (e.g. 1.25x for EV)

    std::string toJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"vehicle\":\"" << vehicle << "\","
           << "\"currentZone\":" << currentZone << ","
           << "\"isAvailable\":" << (isAvailable ? "true" : "false") << ","
           << "\"totalDeliveries\":" << totalDeliveries << ","
           << "\"phone\":\"" << phone << "\","
           << "\"activeOrderId\":" << activeOrderId << ","
           << "\"speedMultiplier\":" << std::fixed << std::setprecision(2) << speedMultiplier
           << "}";
        return ss.str();
    }
};

#endif // MODELS_HPP
