// FoodRush / RestoRush - Restaurant, Hotel & Cafe POS Domain Models
// [MODULE VI] Structures (declaration, nested, arrays of structs, struct methods) | used by: POS_MODELS
#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <sstream>
#include <iomanip>

// [MODULE I] Named Constants for Fixed-Capacity Memory Structures
const int MAX_OUTLETS = 6;
const int MAX_ITEMS_PER_OUTLET = 10;
const int MAX_TOTAL_ITEMS = 60;
const int MAX_TABLES = 12;
const int MAX_STAFF = 8;
const int MAX_ORDER_ITEMS = 20;
const int MAX_ORDERS_HISTORY = 100;
const int MAX_BILLS_HISTORY = 100;

// [MODULE I] Financial and Operational Constants (Currency: INR ₹)
const double GST_TAX_RATE = 0.05;         // 5% Goods and Services Tax (GST)
const double SERVICE_CHARGE_RATE = 0.05;  // 5% Restaurant Service Charge
const double EXPRESS_KOT_SURCHARGE = 50.00; // Urgent KOT Priority Surcharge in ₹
const char CURRENCY_SYMBOL[] = "\xE2\x82\xB9"; // UTF-8 byte sequence for ₹

// [MODULE VI] Structure: Table (Dine-In Hotel / Restaurant Tables)
struct Table {
    int id;                    // 1 to 12
    std::string name;          // "Table 1", "T4 - Window Booth", etc.
    std::string section;       // "Main Dining Hall", "AC Family Lounge", "Rooftop Terrace", "Garden Lounge"
    int capacity;              // 2, 4, 6, 8, 10 persons
    std::string status;        // "VACANT", "OCCUPIED", "BILLED"
    int activeOrderId;         // -1 if vacant, otherwise active orderId
    std::string serverName;    // Assigned waiter / server

    std::string toJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"section\":\"" << section << "\","
           << "\"capacity\":" << capacity << ","
           << "\"status\":\"" << status << "\","
           << "\"activeOrderId\":" << activeOrderId << ","
           << "\"serverName\":\"" << serverName << "\""
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: StaffMember (Hotel & Restaurant Staff Register)
struct StaffMember {
    int id;                    // 1 to 8
    std::string name;          // "Chef Rajesh", "Vikram Malhotra", etc.
    std::string role;          // "Head Chef", "Sous Chef", "Captain Waiter", "Table Server", "Cashier", "Barista"
    std::string shift;         // "Morning (8am-4pm)", "Evening (4pm-12am)", "Full Day"
    bool isPresent;            // Attendance status
    double hoursWorked;        // Hours clocked today
    std::string phone;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"role\":\"" << role << "\","
           << "\"shift\":\"" << shift << "\","
           << "\"isPresent\":" << (isPresent ? "true" : "false") << ","
           << "\"hoursWorked\":" << hoursWorked << ","
           << "\"phone\":\"" << phone << "\""
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: MenuItem
struct MenuItem {
    int id;
    int outletId;              // Kitchen outlet / section (1 to 6)
    std::string name;
    std::string category;      // "Biryani", "Dosa", "Pizza", "Ramen", "Burgers", "Desserts", "Beverages"
    double price;
    int stock;
    bool isVeg;
    int calories;
    unsigned int dietaryFlags; // [MODULE I] Bitwise flags: 1=Spicy, 2=GlutenFree, 4=ChefSpecial
    double rating;             // Moving average customer rating (1.0 to 5.0)
    int ratingCount;           // Total ratings received
    int reservedStock;         // Stock reserved in active unbilled tables

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"id\":" << id << ","
           << "\"restaurantId\":" << outletId << ","
           << "\"outletId\":" << outletId << ","
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

// [MODULE VI] Structure: RestaurantOutlet (Kitchens / Outlets within Hotel/Complex)
struct RestaurantOutlet {
    int id;
    std::string name;
    std::string cuisine;
    std::string section;
    double rating;
    bool isOpen;
    int itemCount;
    int itemIds[MAX_ITEMS_PER_OUTLET];

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"cuisine\":\"" << cuisine << "\","
           << "\"section\":\"" << section << "\","
           << "\"rating\":" << rating << ","
           << "\"isOpen\":" << (isOpen ? "true" : "false") << ","
           << "\"itemCount\":" << itemCount
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: OrderItem / CartItem
struct OrderItem {
    int itemId;
    int outletId;
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
           << "\"outletId\":" << outletId << ","
           << "\"name\":\"" << name << "\","
           << "\"price\":" << price << ","
           << "\"quantity\":" << quantity << ","
           << "\"lineTotal\":" << getLineTotal()
           << "}";
        return ss.str();
    }
};

// Cart / Order Action types for LIFO Undo Stack [MODULE VIII]
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

// [MODULE VI] Structure: OrderTicket (Live KOT - Kitchen Order Ticket across Tables)
struct OrderTicket {
    int orderId;
    int tableId;
    std::string tableName;
    std::string section;
    int outletId;
    std::string outletName;
    std::string guestName;
    OrderItem items[MAX_ORDER_ITEMS];
    int itemCount;
    double subtotal;
    double gstTax;
    double serviceCharge;
    double discount;
    double totalAmount;
    bool isExpressKOT;         // Priority express kitchen ticket
    std::string status;        // "ORDERED", "PREPARING", "SERVED", "BILLED"
    std::string serverName;
    int queuePosition;         // Live position in kitchen circular queue
    std::string orderTime;

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"orderId\":" << orderId << ","
           << "\"tableId\":" << tableId << ","
           << "\"tableName\":\"" << tableName << "\","
           << "\"section\":\"" << section << "\","
           << "\"outletId\":" << outletId << ","
           << "\"outletName\":\"" << outletName << "\","
           << "\"guestName\":\"" << guestName << "\","
           << "\"serverName\":\"" << serverName << "\","
           << "\"orderTime\":\"" << orderTime << "\","
           << "\"itemCount\":" << itemCount << ","
           << "\"items\":[";
        for (int i = 0; i < itemCount; ++i) {
            ss << items[i].toJSON();
            if (i < itemCount - 1) ss << ",";
        }
        ss << "],"
           << "\"subtotal\":" << subtotal << ","
           << "\"gstTax\":" << gstTax << ","
           << "\"serviceCharge\":" << serviceCharge << ","
           << "\"discount\":" << discount << ","
           << "\"totalAmount\":" << totalAmount << ","
           << "\"isExpress\":" << (isExpressKOT ? "true" : "false") << ","
           << "\"status\":\"" << status << "\","
           << "\"queuePosition\":" << queuePosition
           << "}";
        return ss.str();
    }
};

// [MODULE VI] Structure: BillReceipt (Settled Customer Bill & Invoice for Printing)
struct BillReceipt {
    int billId;                // e.g. 5001
    int orderId;
    int tableId;
    std::string tableName;
    std::string section;
    std::string guestName;
    std::string serverName;
    std::string billTime;
    OrderItem items[MAX_ORDER_ITEMS];
    int itemCount;
    double subtotal;
    double gstTax;
    double serviceCharge;
    double discount;
    double netTotal;
    std::string paymentMethod; // "UPI", "Cash", "Card"
    std::string invoiceCode;   // e.g. "INV-5001-8" (verified via string reversal check-digit)

    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"billId\":" << billId << ","
           << "\"orderId\":" << orderId << ","
           << "\"tableId\":" << tableId << ","
           << "\"tableName\":\"" << tableName << "\","
           << "\"section\":\"" << section << "\","
           << "\"guestName\":\"" << guestName << "\","
           << "\"serverName\":\"" << serverName << "\","
           << "\"billTime\":\"" << billTime << "\","
           << "\"itemCount\":" << itemCount << ","
           << "\"items\":[";
        for (int i = 0; i < itemCount; ++i) {
            ss << items[i].toJSON();
            if (i < itemCount - 1) ss << ",";
        }
        ss << "],"
           << "\"subtotal\":" << subtotal << ","
           << "\"gstTax\":" << gstTax << ","
           << "\"serviceCharge\":" << serviceCharge << ","
           << "\"discount\":" << discount << ","
           << "\"netTotal\":" << netTotal << ","
           << "\"paymentMethod\":\"" << paymentMethod << "\","
           << "\"invoiceCode\":\"" << invoiceCode << "\""
           << "}";
        return ss.str();
    }
};

// Alias for backwards compatibility
typedef RestaurantOutlet Restaurant;
typedef OrderItem CartItem;
typedef OrderTicket Order;

#endif // MODELS_HPP
