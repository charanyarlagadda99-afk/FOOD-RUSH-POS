// ============================================================================
// FoodRush / RestoRush - Restaurant, Hotel & Cafe POS Core Engine
// Full Implementation of CS Syllabus Modules I-X
// + 3 Enterprise POS Subsystems:
//   1. Multi-Table Dine-In Order Management & Table Turnover Lifecycle
//   2. Real-Time Kitchen KOT Pipeline with Priority Express Lane
//   3. Staff Attendance Register & Shift Roster Management
// + Itemized GST Invoice & Thermal Receipt Print Generation
// ============================================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <chrono>
#include <algorithm>

#include "models.hpp"
#include "array_stack.hpp"
#include "array_queue.hpp"
#include "sales_matrix.hpp"
#include "algorithms.hpp"
#include "seed_data.hpp"

// ============================================================================
// GLOBAL SYSTEM STATE & MEMORY BUFFERS
// ============================================================================

// [MODULE VI] Arrays of Structures
RestaurantOutlet gOutlets[MAX_OUTLETS];
int gOutletCount = 0;

MenuItem gMenuItems[MAX_TOTAL_ITEMS];
int gMenuItemCount = 0;

Table gTables[MAX_TABLES];
int gTableCount = 0;

StaffMember gStaff[MAX_STAFF];
int gStaffCount = 0;

// Active ordering table (default: Table 1)
int gActiveTableId = 1;

// Draft order items per table (1-indexed for tables 1 to 12)
struct TableCart {
    OrderItem items[MAX_ORDER_ITEMS];
    int itemCount;
    std::string guestName;
    std::string couponCode;
    double discountPercent;
    double discountFlat;

    TableCart() : itemCount(0), guestName("Walk-in Guest"), couponCode(""), discountPercent(0.0), discountFlat(0.0) {}
};

TableCart gTableCarts[MAX_TABLES + 1];

// [MODULE VIII] Array-based Stack for Order Action Undo (LIFO)
ArrayStack<CartAction, 50> gUndoStack;

// [MODULE VIII] ArrayStack for Recently Viewed Dishes (LIFO)
ArrayStack<int, 20> gRecentlyViewedStack;

// [MODULE IX] Custom Array-based Circular Queue for Kitchen Orders (FIFO)
CircularQueue<int, 50> gKitchenQueue;

// [MODULE IX] Priority Order Queue (VIP / Express Fast-Track)
PriorityOrderQueue<int, 50> gPriorityOrderQueue;

// [MODULE IV] 2D Array Sales Matrix Manager (6 Outlets x 7 Days)
SalesMatrixManager gSalesMatrix;

// [MODULE X] STL Manager (vector, deque, map, set, stack, queue, pair)
STLManager gSTLManager;

// Live Active KOT Orders
OrderTicket gActiveOrders[MAX_ORDERS_HISTORY];
int gActiveOrderCount = 0;
int gNextOrderId = 1004; // 1001, 1002, 1003 seeded
int gNextBillId = 5003;  // 5001, 5002 seeded

// ============================================================================
// [MODULE II] HELPER FUNCTIONS & LOOKUPS
// ============================================================================

// [MODULE II] Function Overloading: formatCurrency with double rupees
inline std::string formatCurrency(double amount) {
    std::ostringstream ss;
    ss << CURRENCY_SYMBOL << std::fixed << std::setprecision(2) << amount;
    return ss.str();
}

// [MODULE II] Function Overloading: formatCurrency with integer paise
inline std::string formatCurrency(int paise) {
    // [MODULE I] Type conversion: static_cast from int to double
    double rupees = static_cast<double>(paise) / 100.0;
    return formatCurrency(rupees);
}

MenuItem* findMenuItemByIdMutable(int itemId) {
    for (int i = 0; i < gMenuItemCount; ++i) {
        if (gMenuItems[i].id == itemId) return &gMenuItems[i];
    }
    return nullptr;
}

const MenuItem* findMenuItemById(int itemId) {
    return findMenuItemByIdMutable(itemId);
}

Table* findTableByIdMutable(int tableId) {
    for (int i = 0; i < gTableCount; ++i) {
        if (gTables[i].id == tableId) return &gTables[i];
    }
    return nullptr;
}

const Table* findTableById(int tableId) {
    return findTableByIdMutable(tableId);
}

const RestaurantOutlet* findOutletById(int outletId) {
    for (int i = 0; i < gOutletCount; ++i) {
        if (gOutlets[i].id == outletId) return &gOutlets[i];
    }
    return nullptr;
}

OrderTicket* findActiveOrderById(int orderId) {
    for (int i = 0; i < gActiveOrderCount; ++i) {
        if (gActiveOrders[i].orderId == orderId) return &gActiveOrders[i];
    }
    return nullptr;
}

// Calculate active order position in circular kitchen queue
int getOrderQueuePosition(int orderId) {
    for (int i = 0; i < gKitchenQueue.size(); ++i) {
        int qId = 0;
        if (gKitchenQueue.getAtOffset(i, qId) && qId == orderId) {
            return i + 1; // 1-indexed position
        }
    }
    return 0;
}

// [MODULE I] Bill Calculation using operators and financial rates
void calculateTableBillTotals(
    int tableId,
    double& outSubtotal,
    double& outDiscount,
    double& outGstTax,
    double& outServiceCharge,
    double& outNetTotal)
{
    outSubtotal = 0.0;
    outDiscount = 0.0;
    outGstTax = 0.0;
    outServiceCharge = 0.0;
    outNetTotal = 0.0;

    if (tableId < 1 || tableId > MAX_TABLES) return;
    const TableCart& cart = gTableCarts[tableId];

    for (int i = 0; i < cart.itemCount; ++i) {
        outSubtotal += cart.items[i].getLineTotal();
    }

    if (cart.discountPercent > 0.0) {
        outDiscount += (outSubtotal * (cart.discountPercent / 100.0));
    }
    outDiscount += cart.discountFlat;
    if (outDiscount > outSubtotal) outDiscount = outSubtotal;

    double taxableBase = outSubtotal - outDiscount;
    if (taxableBase < 0.0) taxableBase = 0.0;

    outGstTax = taxableBase * GST_TAX_RATE;
    outServiceCharge = taxableBase * SERVICE_CHARGE_RATE;
    outNetTotal = taxableBase + outGstTax + outServiceCharge;
}

// JSON Serialization of Table Draft Order
std::string tableCartToJSON(int tableId) {
    if (tableId < 1 || tableId > MAX_TABLES) tableId = gActiveTableId;
    const TableCart& cart = gTableCarts[tableId];
    const Table* tbl = findTableById(tableId);

    double subtotal = 0.0, discount = 0.0, gstTax = 0.0, serviceCharge = 0.0, netTotal = 0.0;
    calculateTableBillTotals(tableId, subtotal, discount, gstTax, serviceCharge, netTotal);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{"
       << "\"tableId\":" << tableId << ","
       << "\"tableName\":\"" << (tbl ? tbl->name : "Table") << "\","
       << "\"guestName\":\"" << cart.guestName << "\","
       << "\"itemCount\":" << cart.itemCount << ","
       << "\"items\":[";
    for (int i = 0; i < cart.itemCount; ++i) {
        ss << cart.items[i].toJSON();
        if (i < cart.itemCount - 1) ss << ",";
    }
    ss << "],"
       << "\"coupon\":\"" << cart.couponCode << "\","
       << "\"discountPercent\":" << cart.discountPercent << ","
       << "\"discountAmount\":" << discount << ","
       << "\"subtotal\":" << subtotal << ","
       << "\"gstTax\":" << gstTax << ","
       << "\"serviceCharge\":" << serviceCharge << ","
       << "\"netTotal\":" << netTotal << ","
       << "\"undoStackDepth\":" << gUndoStack.size()
       << "}";
    return ss.str();
}

// JSON Response Wrapper
std::string makeResponse(bool success, const std::string& message, const std::string& dataJSON, const std::vector<std::string>& modules) {
    std::ostringstream ss;
    ss << "{"
       << "\"success\":" << (success ? "true" : "false") << ","
       << "\"message\":\"" << escapeJSON(message) << "\","
       << "\"modules\":[";
    for (size_t i = 0; i < modules.size(); ++i) {
        ss << "\"" << escapeJSON(modules[i]) << "\"";
        if (i < modules.size() - 1) ss << ",";
    }
    ss << "],"
       << "\"handled_by\":[";
    for (size_t i = 0; i < modules.size(); ++i) {
        ss << "\"" << escapeJSON(modules[i]) << "\"";
        if (i < modules.size() - 1) ss << ",";
    }
    ss << "],"
       << "\"data\":" << (dataJSON.empty() ? "{}" : dataJSON)
       << "}";
    return ss.str();
}

// ============================================================================
// [MODULE VIII] TABLE ORDER ACTIONS & INVENTORY RESERVATION ENGINE
// ============================================================================

bool addToTableOrderInternal(int tableId, int itemId, int quantity, bool recordUndo = true) {
    if (quantity <= 0 || tableId < 1 || tableId > MAX_TABLES) return false;
    MenuItem* item = findMenuItemByIdMutable(itemId);
    if (!item) return false;

    // Check available unreserved stock
    if ((item->stock - item->reservedStock) < quantity) {
        return false; // Insufficient kitchen stock
    }

    TableCart& cart = gTableCarts[tableId];
    int existingIdx = -1;
    for (int i = 0; i < cart.itemCount; ++i) {
        if (cart.items[i].itemId == itemId) {
            existingIdx = i;
            break;
        }
    }

    int prevQty = 0;
    int newQty = quantity;

    if (existingIdx != -1) {
        prevQty = cart.items[existingIdx].quantity;
        newQty = prevQty + quantity;
        cart.items[existingIdx].quantity = newQty;
    } else {
        if (cart.itemCount >= MAX_ORDER_ITEMS) return false;
        cart.items[cart.itemCount] = {itemId, item->outletId, item->name, item->price, quantity};
        cart.itemCount++;
    }

    // Atomically reserve kitchen stock
    item->reservedStock += quantity;

    // [MODULE VIII] Push to Custom Array-based Undo Stack
    if (recordUndo) {
        CartAction action;
        action.type = (prevQty == 0) ? ACTION_ADD_ITEM : ACTION_UPDATE_QTY;
        action.itemId = itemId;
        action.previousQuantity = prevQty;
        action.newQuantity = newQty;
        action.itemPrice = item->price;
        action.itemName = item->name;
        gUndoStack.push(action);
        gSTLManager.pushUndo(action); // Mirror into STL stack
    }

    return true;
}

bool removeFromTableOrderInternal(int tableId, int itemId, bool recordUndo = true) {
    if (tableId < 1 || tableId > MAX_TABLES) return false;
    TableCart& cart = gTableCarts[tableId];

    int targetIdx = -1;
    for (int i = 0; i < cart.itemCount; ++i) {
        if (cart.items[i].itemId == itemId) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx == -1) return false;

    int removedQty = cart.items[targetIdx].quantity;
    std::string itemName = cart.items[targetIdx].name;
    double itemPrice = cart.items[targetIdx].price;

    // Release reserved inventory
    MenuItem* item = findMenuItemByIdMutable(itemId);
    if (item) {
        item->reservedStock -= removedQty;
        if (item->reservedStock < 0) item->reservedStock = 0;
    }

    // Shift remaining array elements
    for (int i = targetIdx; i < cart.itemCount - 1; ++i) {
        cart.items[i] = cart.items[i + 1];
    }
    cart.itemCount--;

    if (recordUndo) {
        CartAction action;
        action.type = ACTION_REMOVE_ITEM;
        action.itemId = itemId;
        action.previousQuantity = removedQty;
        action.newQuantity = 0;
        action.itemPrice = itemPrice;
        action.itemName = itemName;
        gUndoStack.push(action);
        gSTLManager.pushUndo(action);
    }

    return true;
}

// [MODULE VIII] Undo Last Table Order Action using ArrayStack
bool undoLastTableOrderAction(int tableId) {
    if (gUndoStack.isEmpty()) return false;
    if (tableId < 1 || tableId > MAX_TABLES) tableId = gActiveTableId;

    CartAction action;
    if (!gUndoStack.pop(action)) return false;

    CartAction stlAction;
    gSTLManager.popUndo(stlAction); // Keep STL in sync

    TableCart& cart = gTableCarts[tableId];

    if (action.type == ACTION_ADD_ITEM) {
        // Was newly added: remove it without pushing to undo
        removeFromTableOrderInternal(tableId, action.itemId, false);
    } else if (action.type == ACTION_UPDATE_QTY) {
        // Was updated: restore previous quantity
        int targetIdx = -1;
        for (int i = 0; i < cart.itemCount; ++i) {
            if (cart.items[i].itemId == action.itemId) {
                targetIdx = i;
                break;
            }
        }
        if (targetIdx != -1) {
            int delta = action.newQuantity - action.previousQuantity;
            cart.items[targetIdx].quantity = action.previousQuantity;
            MenuItem* item = findMenuItemByIdMutable(action.itemId);
            if (item) {
                item->reservedStock -= delta;
                if (item->reservedStock < 0) item->reservedStock = 0;
            }
        }
    } else if (action.type == ACTION_REMOVE_ITEM) {
        // Was removed: re-add previous quantity without pushing to undo
        addToTableOrderInternal(tableId, action.itemId, action.previousQuantity, false);
    }

    return true;
}

void clearTableOrderInternal(int tableId) {
    if (tableId < 1 || tableId > MAX_TABLES) return;
    TableCart& cart = gTableCarts[tableId];

    for (int i = 0; i < cart.itemCount; ++i) {
        MenuItem* item = findMenuItemByIdMutable(cart.items[i].itemId);
        if (item) {
            item->reservedStock -= cart.items[i].quantity;
            if (item->reservedStock < 0) item->reservedStock = 0;
        }
    }
    cart.itemCount = 0;
    cart.couponCode = "";
    cart.discountPercent = 0.0;
    cart.discountFlat = 0.0;
}

// ============================================================================
// SEED INITIAL ACTIVE KOT ORDERS & PAST BILLS
// ============================================================================

void initializeActiveOrdersAndBills() {
    // 1. Seed Active Order 1001 for Table 2 (Main Dining Hall)
    OrderTicket kot1;
    kot1.orderId = 1001;
    kot1.tableId = 2;
    kot1.tableName = "Table 2 (Booth)";
    kot1.section = "Main Dining Hall";
    kot1.outletId = 1;
    kot1.outletName = "Grand Mughal Dining";
    kot1.guestName = "Mr. Sharma";
    kot1.serverName = "Vikram Malhotra";
    kot1.orderTime = "13:15 PM";
    kot1.isExpressKOT = false;
    kot1.status = "PREPARING";
    kot1.itemCount = 2;
    kot1.items[0] = {101, 1, "Hyderabadi Chicken Dum Biryani", 320.00, 2};
    kot1.items[1] = {107, 1, "Burani Garlic Raita", 60.00, 2};
    kot1.subtotal = 760.00;
    kot1.discount = 0.00;
    kot1.gstTax = 38.00;
    kot1.serviceCharge = 38.00;
    kot1.totalAmount = 836.00;
    kot1.queuePosition = 1;
    gActiveOrders[gActiveOrderCount++] = kot1;
    gKitchenQueue.enqueue(1001);
    gSTLManager.enqueueOrder(1001);

    // 2. Seed Active Order 1002 for Table 5 (AC Family Lounge)
    OrderTicket kot2;
    kot2.orderId = 1002;
    kot2.tableId = 5;
    kot2.tableName = "Table 5 (Executive)";
    kot2.section = "AC Family Lounge";
    kot2.outletId = 3;
    kot2.outletName = "Trattoria Bella Vista";
    kot2.guestName = "Dr. Kapoor";
    kot2.serverName = "Priya Nair";
    kot2.orderTime = "13:28 PM";
    kot2.isExpressKOT = true;
    kot2.status = "ORDERED";
    kot2.itemCount = 3;
    kot2.items[0] = {301, 3, "Wood-Fired Margherita Pizza", 390.00, 1};
    kot2.items[1] = {303, 3, "Truffle Wild Mushroom Fettuccine", 460.00, 1};
    kot2.items[2] = {308, 3, "Classic Espresso Tiramisu", 240.00, 2};
    kot2.subtotal = 1330.00;
    kot2.discount = 0.00;
    kot2.gstTax = 66.50;
    kot2.serviceCharge = 66.50;
    kot2.totalAmount = 1463.00;
    kot2.queuePosition = 2;
    gActiveOrders[gActiveOrderCount++] = kot2;
    gKitchenQueue.enqueue(1002);
    gPriorityOrderQueue.enqueue(1002, true);
    gSTLManager.enqueueOrder(1002);

    // 3. Seed Active Order 1003 for Table 8 (Rooftop Terrace)
    OrderTicket kot3;
    kot3.orderId = 1003;
    kot3.tableId = 8;
    kot3.tableName = "Table 8 (Sky View)";
    kot3.section = "Rooftop Terrace";
    kot3.outletId = 4;
    kot3.outletName = "Sakura Asian Bistro";
    kot3.guestName = "Ananya & Friend";
    kot3.serverName = "Rohit Verma";
    kot3.orderTime = "13:05 PM";
    kot3.isExpressKOT = false;
    kot3.status = "SERVED";
    kot3.itemCount = 2;
    kot3.items[0] = {401, 4, "Rich Tonkotsu Black Garlic Ramen", 480.00, 2};
    kot3.items[1] = {405, 4, "Pan-Seared Chicken Gyoza (6 pcs)", 280.00, 1};
    kot3.subtotal = 1240.00;
    kot3.discount = 0.00;
    kot3.gstTax = 62.00;
    kot3.serviceCharge = 62.00;
    kot3.totalAmount = 1364.00;
    kot3.queuePosition = 3;
    gActiveOrders[gActiveOrderCount++] = kot3;
    gKitchenQueue.enqueue(1003);
    gSTLManager.enqueueOrder(1003);

    // 4. Seed 2 Settled Past Bills into STL Deque
    BillReceipt b1;
    b1.billId = 5001;
    b1.orderId = 998;
    b1.tableId = 1;
    b1.tableName = "Table 1 (Family)";
    b1.section = "Main Dining Hall";
    b1.guestName = "Rajesh Verma";
    b1.serverName = "Vikram Malhotra";
    b1.billTime = "12:30 PM";
    b1.itemCount = 2;
    b1.items[0] = {101, 1, "Hyderabadi Chicken Dum Biryani", 320.00, 2};
    b1.items[1] = {108, 1, "Zafrani Matka Phirni", 130.00, 2};
    b1.subtotal = 900.00;
    b1.discount = 90.00; // 10% coupon
    b1.gstTax = 40.50;
    b1.serviceCharge = 40.50;
    b1.netTotal = 891.00;
    b1.paymentMethod = "UPI";
    b1.invoiceCode = generateInvoiceCode(5001);
    gSTLManager.recordSettledBill(b1);

    BillReceipt b2;
    b2.billId = 5002;
    b2.orderId = 999;
    b2.tableId = 4;
    b2.tableName = "Table 4 (Central)";
    b2.section = "AC Family Lounge";
    b2.guestName = "Siddharth Rao";
    b2.serverName = "Priya Nair";
    b2.billTime = "12:50 PM";
    b2.itemCount = 2;
    b2.items[0] = {201, 2, "Ghee Roast Masala Dosa", 160.00, 2};
    b2.items[1] = {208, 2, "Filter Degree Coffee", 50.00, 2};
    b2.subtotal = 420.00;
    b2.discount = 0.00;
    b2.gstTax = 21.00;
    b2.serviceCharge = 21.00;
    b2.netTotal = 462.00;
    b2.paymentMethod = "Card";
    b2.invoiceCode = generateInvoiceCode(5002);
    gSTLManager.recordSettledBill(b2);
}

// ============================================================================
// [MODULE II] COMMAND DISPATCHER & EXECUTION ENGINE
// ============================================================================

std::string handleCommand(const std::string& line) {
    if (line.empty()) return "";

    std::vector<std::string> tokens = tokenizeString(line, ' ');
    if (tokens.empty()) return "";

    std::string cmd = tokens[0];
    // Transform command to uppercase
    for (size_t i = 0; i < cmd.length(); ++i) {
        cmd[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd[i])));
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_INITIAL_STATE / INIT
    // ------------------------------------------------------------------------
    if (cmd == "GET_INITIAL_STATE" || cmd == "INIT" || cmd == "STATUS") {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"activeTableId\":" << gActiveTableId << ","
           << "\"outlets\":[";
        for (int i = 0; i < gOutletCount; ++i) {
            ss << gOutlets[i].toJSON();
            if (i < gOutletCount - 1) ss << ",";
        }
        ss << "],"
           << "\"tables\":[";
        for (int i = 0; i < gTableCount; ++i) {
            ss << gTables[i].toJSON();
            if (i < gTableCount - 1) ss << ",";
        }
        ss << "],"
           << "\"staff\":[";
        for (int i = 0; i < gStaffCount; ++i) {
            ss << gStaff[i].toJSON();
            if (i < gStaffCount - 1) ss << ",";
        }
        ss << "],"
           << "\"activeCart\":" << tableCartToJSON(gActiveTableId) << ","
           << "\"activeOrdersCount\":" << gActiveOrderCount << ","
           << "\"kitchenQueueSize\":" << gKitchenQueue.size()
           << "}";

        return makeResponse(true, "POS Initial State Loaded Successfully", ss.str(),
            {"Module I (Basics)", "Module II (Functions)", "Module VI (Structures)", "Module IX (Kitchen Queue)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_OUTLETS / GET_RESTAURANTS
    // ------------------------------------------------------------------------
    if (cmd == "GET_OUTLETS" || cmd == "GET_RESTAURANTS") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gOutletCount; ++i) {
            ss << gOutlets[i].toJSON();
            if (i < gOutletCount - 1) ss << ",";
        }
        ss << "]";
        return makeResponse(true, "Kitchen Outlets Retrieved", ss.str(),
            {"Module III (1D Arrays)", "Module VI (Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_MENU [outletId]
    // ------------------------------------------------------------------------
    if (cmd == "GET_MENU") {
        int filterOutlet = 0;
        if (tokens.size() > 1) {
            try { filterOutlet = std::stoi(tokens[1]); } catch (...) { filterOutlet = 0; }
        }

        std::ostringstream ss;
        ss << "[";
        bool first = true;
        for (int i = 0; i < gMenuItemCount; ++i) {
            if (filterOutlet == 0 || gMenuItems[i].outletId == filterOutlet) {
                if (!first) ss << ",";
                ss << gMenuItems[i].toJSON();
                first = false;
            }
        }
        ss << "]";

        return makeResponse(true, "Menu Catalog Retrieved", ss.str(),
            {"Module III (1D Array Traversal)", "Module VI (Structures)", "Module X (Vector Registry)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_TABLES
    // ------------------------------------------------------------------------
    if (cmd == "GET_TABLES") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gTableCount; ++i) {
            ss << gTables[i].toJSON();
            if (i < gTableCount - 1) ss << ",";
        }
        ss << "]";
        return makeResponse(true, "Dining Tables Status Retrieved", ss.str(),
            {"Module III (1D Arrays)", "Module VI (Table Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: SELECT_TABLE <tableId>
    // ------------------------------------------------------------------------
    if (cmd == "SELECT_TABLE") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: SELECT_TABLE <tableId>", "{}", {"Module II (Control)"});
        int tId = 1;
        try { tId = std::stoi(tokens[1]); } catch (...) { return makeResponse(false, "Invalid tableId", "{}", {}); }

        if (tId < 1 || tId > MAX_TABLES) {
            return makeResponse(false, "Table ID out of range (1-12)", "{}", {});
        }

        gActiveTableId = tId;
        std::string cartJson = tableCartToJSON(gActiveTableId);
        return makeResponse(true, "Active dining table switched to Table " + std::to_string(tId), cartJson,
            {"Module I (State)", "Module II (Functions)", "Module VI (Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: ORDER_ADD / ADD_TO_CART <itemId> [qty] [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "ORDER_ADD" || cmd == "ADD_TO_CART") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: ORDER_ADD <itemId> [qty] [tableId]", "{}", {});
        int itemId = 0, qty = 1, tId = gActiveTableId;
        try {
            itemId = std::stoi(tokens[1]);
            if (tokens.size() > 2) qty = std::stoi(tokens[2]);
            if (tokens.size() > 3) tId = std::stoi(tokens[3]);
        } catch (...) {
            return makeResponse(false, "Invalid parameters", "{}", {});
        }

        if (tId < 1 || tId > MAX_TABLES) tId = gActiveTableId;
        bool ok = addToTableOrderInternal(tId, itemId, qty, true);
        if (!ok) {
            return makeResponse(false, "Failed to add dish (insufficient stock or invalid item)", "{}",
                {"Inventory Lock Engine", "Module VIII (Stack)"});
        }

        // Track recently viewed / ordered in LIFO stack [MODULE VIII]
        gRecentlyViewedStack.push(itemId);

        return makeResponse(true, "Item added to Table " + std::to_string(tId) + " order", tableCartToJSON(tId),
            {"Module VIII (ArrayStack Push)", "Module VI (Order Structures)", "Module I (Totals)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: ORDER_REMOVE / REMOVE_FROM_CART <itemId> [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "ORDER_REMOVE" || cmd == "REMOVE_FROM_CART") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: ORDER_REMOVE <itemId> [tableId]", "{}", {});
        int itemId = 0, tId = gActiveTableId;
        try {
            itemId = std::stoi(tokens[1]);
            if (tokens.size() > 2) tId = std::stoi(tokens[2]);
        } catch (...) {
            return makeResponse(false, "Invalid parameters", "{}", {});
        }

        if (tId < 1 || tId > MAX_TABLES) tId = gActiveTableId;
        bool ok = removeFromTableOrderInternal(tId, itemId, true);
        if (!ok) {
            return makeResponse(false, "Item not found in table order", "{}", {});
        }

        return makeResponse(true, "Item removed from Table " + std::to_string(tId) + " order", tableCartToJSON(tId),
            {"Module VIII (ArrayStack Push)", "Module III (Array Shift)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: ORDER_UNDO / UNDO [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "ORDER_UNDO" || cmd == "UNDO") {
        int tId = gActiveTableId;
        if (tokens.size() > 1) {
            try { tId = std::stoi(tokens[1]); } catch (...) { tId = gActiveTableId; }
        }

        bool ok = undoLastTableOrderAction(tId);
        if (!ok) {
            return makeResponse(false, "Nothing to undo in order history", tableCartToJSON(tId),
                {"Module VIII (ArrayStack Pop Empty Check)"});
        }

        return makeResponse(true, "Last order modification reverted (LIFO Undo)", tableCartToJSON(tId),
            {"Module VIII (ArrayStack Pop)", "Module X (STL Stack Mirror)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: ORDER_CLEAR / CLEAR_CART [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "ORDER_CLEAR" || cmd == "CLEAR_CART") {
        int tId = gActiveTableId;
        if (tokens.size() > 1) {
            try { tId = std::stoi(tokens[1]); } catch (...) { tId = gActiveTableId; }
        }

        clearTableOrderInternal(tId);
        return makeResponse(true, "Table " + std::to_string(tId) + " draft order cleared", tableCartToJSON(tId),
            {"Module I (State Reset)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: ORDER_VIEW / GET_CART [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "ORDER_VIEW" || cmd == "GET_CART") {
        int tId = gActiveTableId;
        if (tokens.size() > 1) {
            try { tId = std::stoi(tokens[1]); } catch (...) { tId = gActiveTableId; }
        }
        return makeResponse(true, "Table " + std::to_string(tId) + " Order View", tableCartToJSON(tId),
            {"Module I (Bill Calculation)", "Module VI (Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: APPLY_COUPON <code> [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "APPLY_COUPON") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: APPLY_COUPON <code> [tableId]", "{}", {});
        std::string code = tokens[1];
        int tId = gActiveTableId;
        if (tokens.size() > 2) {
            try { tId = std::stoi(tokens[2]); } catch (...) { tId = gActiveTableId; }
        }

        // [MODULE X] Map lookup for coupon discount
        const auto& coupons = gSTLManager.getCoupons();
        auto it = coupons.find(code);
        if (it != coupons.end()) {
            gTableCarts[tId].couponCode = code;
            gTableCarts[tId].discountPercent = it->second;
            return makeResponse(true, "Coupon code applied (" + std::to_string(static_cast<int>(it->second)) + "% off)",
                tableCartToJSON(tId), {"Module X (std::map lookup)", "Module I (Discount Calculation)"});
        }

        return makeResponse(false, "Invalid coupon code", tableCartToJSON(tId), {"Module X (std::map miss)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: SUBMIT_KOT / CHECKOUT [guestName] [isExpress] [tableId]
    // ------------------------------------------------------------------------
    if (cmd == "SUBMIT_KOT" || cmd == "CHECKOUT") {
        int tId = gActiveTableId;
        std::string guestName = "Guest";
        bool isExpress = false;

        if (tokens.size() > 1 && !tokens[1].empty()) guestName = tokens[1];
        if (tokens.size() > 2) isExpress = (tokens[2] == "1" || tokens[2] == "true" || tokens[2] == "express");
        if (tokens.size() > 3) {
            try { tId = std::stoi(tokens[3]); } catch (...) { tId = gActiveTableId; }
        }

        TableCart& cart = gTableCarts[tId];
        if (cart.itemCount == 0) {
            return makeResponse(false, "Cannot submit empty order for Table " + std::to_string(tId), "{}", {});
        }

        Table* table = findTableByIdMutable(tId);
        if (!table) return makeResponse(false, "Table not found", "{}", {});

        // Compute financial totals
        double subtotal = 0.0, discount = 0.0, gstTax = 0.0, serviceCharge = 0.0, netTotal = 0.0;
        calculateTableBillTotals(tId, subtotal, discount, gstTax, serviceCharge, netTotal);
        if (isExpress) netTotal += EXPRESS_KOT_SURCHARGE;

        // Create new KOT OrderTicket
        int orderId = gNextOrderId++;
        OrderTicket kot;
        kot.orderId = orderId;
        kot.tableId = tId;
        kot.tableName = table->name;
        kot.section = table->section;
        kot.guestName = guestName;
        kot.serverName = table->serverName;
        kot.orderTime = "Just Now";
        kot.itemCount = cart.itemCount;
        kot.subtotal = subtotal;
        kot.discount = discount;
        kot.gstTax = gstTax;
        kot.serviceCharge = serviceCharge;
        kot.totalAmount = netTotal;
        kot.isExpressKOT = isExpress;
        kot.status = "ORDERED";

        int firstOutletId = 1;
        for (int i = 0; i < cart.itemCount; ++i) {
            kot.items[i] = cart.items[i];
            firstOutletId = cart.items[i].outletId;
        }
        kot.outletId = firstOutletId;
        const RestaurantOutlet* outlet = findOutletById(firstOutletId);
        kot.outletName = outlet ? outlet->name : "Kitchen";

        // [MODULE IX] Custom Array Circular Queue Enqueue
        gPriorityOrderQueue.enqueue(orderId, isExpress);
        gKitchenQueue.enqueue(orderId);
        gSTLManager.enqueueOrder(orderId); // Mirror to STL queue

        kot.queuePosition = gKitchenQueue.size();

        // Update table status to OCCUPIED
        table->status = "OCCUPIED";
        table->activeOrderId = orderId;

        // Store into Active Orders array
        if (gActiveOrderCount < MAX_ORDERS_HISTORY) {
            gActiveOrders[gActiveOrderCount++] = kot;
        }

        // Clear table's draft cart
        clearTableOrderInternal(tId);

        return makeResponse(true, "KOT #" + std::to_string(orderId) + " dispatched to kitchen for " + table->name,
            kot.toJSON(),
            {"Module IX (CircularQueue Enqueue)", "Module VI (OrderTicket Struct)", "Module I (GST/Service Charge Calculation)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_ACTIVE_ORDERS / GET_KITCHEN_QUEUE
    // ------------------------------------------------------------------------
    if (cmd == "GET_ACTIVE_ORDERS" || cmd == "GET_KITCHEN_QUEUE") {
        std::ostringstream ss;
        ss << "[";
        bool first = true;
        for (int i = 0; i < gActiveOrderCount; ++i) {
            if (gActiveOrders[i].status != "BILLED") {
                if (!first) ss << ",";
                gActiveOrders[i].queuePosition = getOrderQueuePosition(gActiveOrders[i].orderId);
                ss << gActiveOrders[i].toJSON();
                first = false;
            }
        }
        ss << "]";

        return makeResponse(true, "Active Kitchen Orders Retrieved", ss.str(),
            {"Module IX (Circular Queue Buffer)", "Module VI (OrderTicket Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: UPDATE_ORDER_STAGE <orderId> <stage>
    // ------------------------------------------------------------------------
    if (cmd == "UPDATE_ORDER_STAGE") {
        if (tokens.size() < 3) return makeResponse(false, "Usage: UPDATE_ORDER_STAGE <orderId> <ORDERED|PREPARING|SERVED|BILLED>", "{}", {});
        int orderId = 0;
        try { orderId = std::stoi(tokens[1]); } catch (...) { return makeResponse(false, "Invalid orderId", "{}", {}); }
        std::string stage = tokens[2];
        for (size_t i = 0; i < stage.length(); ++i) {
            stage[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(stage[i])));
        }

        OrderTicket* kot = findActiveOrderById(orderId);
        if (!kot) return makeResponse(false, "Order #" + std::to_string(orderId) + " not found", "{}", {});

        kot->status = stage;
        if (stage == "SERVED") {
            // Once served, dequeue from kitchen circular queue
            int deqVal = 0;
            gKitchenQueue.dequeue(deqVal);
            gSTLManager.dequeueOrder(deqVal);
        }

        return makeResponse(true, "Order #" + std::to_string(orderId) + " status updated to " + stage, kot->toJSON(),
            {"Module IX (Circular Queue Dequeue)", "Module VI (Order Lifecycle)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GENERATE_BILL <tableId> [paymentMethod] [coupon]
    // ------------------------------------------------------------------------
    if (cmd == "GENERATE_BILL") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: GENERATE_BILL <tableId> [paymentMethod] [coupon]", "{}", {});
        int tId = 1;
        try { tId = std::stoi(tokens[1]); } catch (...) { return makeResponse(false, "Invalid tableId", "{}", {}); }
        std::string payment = (tokens.size() > 2) ? tokens[2] : "UPI";
        std::string coupon = (tokens.size() > 3) ? tokens[3] : "";

        Table* table = findTableByIdMutable(tId);
        if (!table) return makeResponse(false, "Table not found", "{}", {});

        // Find active order for this table
        OrderTicket* activeKOT = nullptr;
        for (int i = 0; i < gActiveOrderCount; ++i) {
            if (gActiveOrders[i].tableId == tId && gActiveOrders[i].status != "BILLED") {
                activeKOT = &gActiveOrders[i];
                break;
            }
        }

        if (!activeKOT) {
            return makeResponse(false, "No active unbilled order found for Table " + std::to_string(tId), "{}", {});
        }

        // Apply discount if coupon provided
        if (!coupon.empty()) {
            const auto& coupons = gSTLManager.getCoupons();
            auto it = coupons.find(coupon);
            if (it != coupons.end()) {
                activeKOT->discount = activeKOT->subtotal * (it->second / 100.0);
                double base = activeKOT->subtotal - activeKOT->discount;
                activeKOT->gstTax = base * GST_TAX_RATE;
                activeKOT->serviceCharge = base * SERVICE_CHARGE_RATE;
                activeKOT->totalAmount = base + activeKOT->gstTax + activeKOT->serviceCharge;
            }
        }

        // Create settled BillReceipt
        int billId = gNextBillId++;
        BillReceipt bill;
        bill.billId = billId;
        bill.orderId = activeKOT->orderId;
        bill.tableId = tId;
        bill.tableName = table->name;
        bill.section = table->section;
        bill.guestName = activeKOT->guestName;
        bill.serverName = activeKOT->serverName;
        bill.billTime = "Today, Just Now";
        bill.itemCount = activeKOT->itemCount;
        for (int i = 0; i < activeKOT->itemCount; ++i) {
            bill.items[i] = activeKOT->items[i];
        }
        bill.subtotal = activeKOT->subtotal;
        bill.discount = activeKOT->discount;
        bill.gstTax = activeKOT->gstTax;
        bill.serviceCharge = activeKOT->serviceCharge;
        bill.netTotal = activeKOT->totalAmount;
        bill.paymentMethod = payment;

        // [MODULE V] Generate verified Invoice Code using String Reversal Check-Digit
        bill.invoiceCode = generateInvoiceCode(billId);

        // [MODULE X] Archive settled bill into STL deque
        gSTLManager.recordSettledBill(bill);

        // [MODULE IV] Record settled revenue into 2D Sales Matrix
        int outletIdx = (activeKOT->outletId >= 1 && activeKOT->outletId <= MAX_OUTLETS) ? (activeKOT->outletId - 1) : 0;
        gSalesMatrix.recordSale(outletIdx, 6, bill.netTotal); // Sunday / Today column

        // Mark table VACANT and KOT as BILLED
        table->status = "VACANT";
        table->activeOrderId = -1;
        activeKOT->status = "BILLED";

        std::string asciiReceipt = generatePrintReceiptText(bill);
        std::ostringstream ss;
        ss << "{"
           << "\"receipt\":" << bill.toJSON() << ","
           << "\"asciiPrintText\":\"" << escapeJSON(asciiReceipt) << "\""
           << "}";

        return makeResponse(true, "Bill #" + std::to_string(billId) + " settled successfully. Table " + std::to_string(tId) + " is now VACANT.",
            ss.str(),
            {"Module V (Reversal Check-Digit)", "Module IV (Sales Matrix Recording)", "Module X (STL Deque Archive)", "Module I (Tax Invoicing)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_BILLS (Past Bills Archive)
    // ------------------------------------------------------------------------
    if (cmd == "GET_BILLS") {
        const auto& pastBills = gSTLManager.getPastBills();
        std::ostringstream ss;
        ss << "[";
        for (size_t i = 0; i < pastBills.size(); ++i) {
            ss << pastBills[i].toJSON();
            if (i < pastBills.size() - 1) ss << ",";
        }
        ss << "]";

        return makeResponse(true, "Settled Past Bills Archive Retrieved", ss.str(),
            {"Module X (std::deque Past Bills Archive)", "Module VI (BillReceipt Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: PRINT_BILL <billId>
    // ------------------------------------------------------------------------
    if (cmd == "PRINT_BILL") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: PRINT_BILL <billId>", "{}", {});
        int billId = 0;
        try { billId = std::stoi(tokens[1]); } catch (...) { return makeResponse(false, "Invalid billId", "{}", {}); }

        const auto& pastBills = gSTLManager.getPastBills();
        const BillReceipt* targetBill = nullptr;
        for (const auto& b : pastBills) {
            if (b.billId == billId) {
                targetBill = &b;
                break;
            }
        }

        if (!targetBill) {
            return makeResponse(false, "Bill #" + std::to_string(billId) + " not found in archive", "{}", {});
        }

        // [MODULE V] Verify check-digit on invoice code
        bool isCodeValid = validateInvoiceCode(targetBill->invoiceCode);
        std::string ascii = generatePrintReceiptText(*targetBill);

        std::ostringstream ss;
        ss << "{"
           << "\"billId\":" << billId << ","
           << "\"invoiceCode\":\"" << targetBill->invoiceCode << "\","
           << "\"checkDigitVerified\":" << (isCodeValid ? "true" : "false") << ","
           << "\"asciiReceipt\":\"" << escapeJSON(ascii) << "\","
           << "\"receiptData\":" << targetBill->toJSON()
           << "}";

        return makeResponse(true, "Formatted Thermal Bill Receipt Generated", ss.str(),
            {"Module V (String Reversal & Check-Digit)", "Thermal Invoice ASCII Formatter", "Module VI (Bill Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_STAFF (Workers Attendance & Shifts)
    // ------------------------------------------------------------------------
    if (cmd == "GET_STAFF") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gStaffCount; ++i) {
            ss << gStaff[i].toJSON();
            if (i < gStaffCount - 1) ss << ",";
        }
        ss << "]";

        return makeResponse(true, "Staff Attendance Register Retrieved", ss.str(),
            {"Module III (1D Staff Array)", "Module VI (StaffMember Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: MARK_ATTENDANCE <staffId> <present:0|1> [hoursWorked]
    // ------------------------------------------------------------------------
    if (cmd == "MARK_ATTENDANCE") {
        if (tokens.size() < 3) return makeResponse(false, "Usage: MARK_ATTENDANCE <staffId> <0|1> [hours]", "{}", {});
        int staffId = 0;
        bool isPresent = false;
        double hours = 8.0;

        try {
            staffId = std::stoi(tokens[1]);
            isPresent = (tokens[2] == "1" || tokens[2] == "true");
            if (tokens.size() > 3) hours = std::stod(tokens[3]);
        } catch (...) {
            return makeResponse(false, "Invalid parameters", "{}", {});
        }

        StaffMember* member = nullptr;
        for (int i = 0; i < gStaffCount; ++i) {
            if (gStaff[i].id == staffId) {
                member = &gStaff[i];
                break;
            }
        }

        if (!member) return makeResponse(false, "Staff member not found", "{}", {});

        member->isPresent = isPresent;
        member->hoursWorked = isPresent ? hours : 0.0;

        return makeResponse(true, "Attendance marked for " + member->name, member->toJSON(),
            {"Module III (Staff Roster Update)", "Module VI (Structures)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: SEARCH_DISHES <query>
    // ------------------------------------------------------------------------
    if (cmd == "SEARCH_DISHES") {
        if (tokens.size() < 2) return makeResponse(false, "Usage: SEARCH_DISHES <query>", "[]", {});
        std::string query = line.substr(line.find(' ') + 1);

        std::string queryLower = toLowerString(query);
        std::vector<std::string> queryTokens = tokenizeString(queryLower, ' ');

        std::vector<const MenuItem*> matchingDishes;
        for (int i = 0; i < gMenuItemCount; ++i) {
            std::string itemNameLower = toLowerString(gMenuItems[i].name);
            std::string catLower = toLowerString(gMenuItems[i].category);

            bool matches = false;
            for (const auto& token : queryTokens) {
                if (stringContains(itemNameLower, token) || stringContains(catLower, token)) {
                    matches = true;
                    break;
                }
            }
            if (matches) {
                matchingDishes.push_back(&gMenuItems[i]);
            }
        }

        // Levenshtein "Did You Mean"
        std::string didYouMean = "";
        int bestDistance = 999;
        for (int i = 0; i < gMenuItemCount; ++i) {
            int dist = calculateLevenshteinDistance(queryLower, toLowerString(gMenuItems[i].name));
            if (dist > 0 && dist < 4 && dist < bestDistance) {
                bestDistance = dist;
                didYouMean = gMenuItems[i].name;
            }
        }

        std::ostringstream ss;
        ss << "{"
           << "\"query\":\"" << escapeJSON(query) << "\","
           << "\"matchCount\":" << matchingDishes.size() << ","
           << "\"didYouMean\":\"" << escapeJSON(didYouMean) << "\","
           << "\"charFrequency\":" << charFrequencyToJSON(analyzeCharFrequency(queryLower)) << ","
           << "\"results\":[";
        for (size_t i = 0; i < matchingDishes.size(); ++i) {
            ss << matchingDishes[i]->toJSON();
            if (i < matchingDishes.size() - 1) ss << ",";
        }
        ss << "]}";

        return makeResponse(true, "Dish search completed", ss.str(),
            {"Module V (Tokenizing, Levenshtein DP Matrix, Char Frequency)", "Module III (Linear Search)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_SALES_MATRIX / GET_SALES_REPORT
    // ------------------------------------------------------------------------
    if (cmd == "GET_SALES_MATRIX" || cmd == "GET_SALES_REPORT") {
        return makeResponse(true, "Weekly Sales Matrix & Seating Layout Retrieved", gSalesMatrix.toJSON(),
            {"Module IV (2D Array Matrix Traversal, Row/Col Sums, Max Element)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: GET_TOP_DISHES [k]
    // ------------------------------------------------------------------------
    if (cmd == "GET_TOP_DISHES") {
        int k = 5;
        if (tokens.size() > 1) {
            try { k = std::stoi(tokens[1]); } catch (...) { k = 5; }
        }

        std::vector<MenuItem> topK = getTopKDishes(gMenuItems, gMenuItemCount, k);
        std::ostringstream ss;
        ss << "[";
        for (size_t i = 0; i < topK.size(); ++i) {
            ss << topK[i].toJSON();
            if (i < topK.size() - 1) ss << ",";
        }
        ss << "]";

        return makeResponse(true, "Top-K Highest Rated Dishes", ss.str(),
            {"Module VII (O(N log K) Top-K Ranking)", "Module III (1D Array Sort)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: RATE_DISH <itemId> <rating>
    // ------------------------------------------------------------------------
    if (cmd == "RATE_DISH") {
        if (tokens.size() < 3) return makeResponse(false, "Usage: RATE_DISH <itemId> <1.0-5.0>", "{}", {});
        int itemId = 0;
        double ratingVal = 0.0;
        try {
            itemId = std::stoi(tokens[1]);
            ratingVal = std::stod(tokens[2]);
        } catch (...) {
            return makeResponse(false, "Invalid parameters", "{}", {});
        }

        MenuItem* item = findMenuItemByIdMutable(itemId);
        if (!item) return makeResponse(false, "Dish not found", "{}", {});

        if (ratingVal < 1.0) ratingVal = 1.0;
        if (ratingVal > 5.0) ratingVal = 5.0;

        // Cumulative moving average
        double totalScore = (item->rating * item->ratingCount) + ratingVal;
        item->ratingCount++;
        item->rating = totalScore / item->ratingCount;

        return makeResponse(true, "Dish rating submitted successfully", item->toJSON(),
            {"Module I (Arithmetic Operations)", "Module VI (Structures Update)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: RESTOCK_ITEM <itemId> <quantity>
    // ------------------------------------------------------------------------
    if (cmd == "RESTOCK_ITEM") {
        if (tokens.size() < 3) return makeResponse(false, "Usage: RESTOCK_ITEM <itemId> <quantity>", "{}", {});
        int itemId = 0, addQty = 0;
        try {
            itemId = std::stoi(tokens[1]);
            addQty = std::stoi(tokens[2]);
        } catch (...) {
            return makeResponse(false, "Invalid parameters", "{}", {});
        }

        MenuItem* item = findMenuItemByIdMutable(itemId);
        if (!item) return makeResponse(false, "Dish not found", "{}", {});

        item->stock += addQty;
        return makeResponse(true, "Inventory restocked for " + item->name, item->toJSON(),
            {"Inventory Control Engine", "Module I (Compound Operators)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: BENCHMARK (Stopwatch Search & Sort)
    // ------------------------------------------------------------------------
    if (cmd == "BENCHMARK") {
        PerformanceBenchmark bench;
        std::string benchJSON = bench.runFullBenchmark();
        return makeResponse(true, "Stopwatch Performance Benchmark Complete", benchJSON,
            {"Module VII (Big-O Empirical Stopwatch)", "Module III (Linear vs Binary Search, Bubble vs Quick Sort)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: COMPARE_DS (Custom ArrayStack/Queue vs STL)
    // ------------------------------------------------------------------------
    if (cmd == "COMPARE_DS") {
        std::string compJSON = gSTLManager.compareDataStructures(50000);
        return makeResponse(true, "Hand-Crafted Data Structures vs STL Live Benchmark", compJSON,
            {"Module VIII (ArrayStack)", "Module IX (CircularQueue)", "Module X (STL std::stack & std::queue)"});
    }

    // ------------------------------------------------------------------------
    // COMMAND: INSPECT_ENGINE (Memory & Buffer Inspector)
    // ------------------------------------------------------------------------
    if (cmd == "INSPECT_ENGINE") {
        // Collect dish prices for 1D stats
        double prices[MAX_TOTAL_ITEMS];
        for (int i = 0; i < gMenuItemCount; ++i) {
            prices[i] = gMenuItems[i].price;
        }

        int maxIdx = 0, minIdx = 0;
        double maxPrice = findArrayMax(prices, gMenuItemCount, maxIdx);
        double minPrice = findArrayMin(prices, gMenuItemCount, minIdx);
        double sumPrices = calculateArraySum(prices, gMenuItemCount);
        double avgPrice = (gMenuItemCount > 0) ? (sumPrices / gMenuItemCount) : 0.0;

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"undoStack\":{"
           << "\"size\":" << gUndoStack.size() << ","
           << "\"capacity\":50,"
           << "\"isEmpty\":" << (gUndoStack.isEmpty() ? "true" : "false") << ","
           << "\"recentActions\":[";
        int undoCount = gUndoStack.size();
        for (int i = 0; i < undoCount && i < 5; ++i) {
            CartAction a;
            if (gUndoStack.getAtOffset(i, a)) {
                ss << a.toJSON();
                if (i < undoCount - 1 && i < 4) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"kitchenQueue\":{"
           << "\"size\":" << gKitchenQueue.size() << ","
           << "\"capacity\":50,"
           << "\"isFull\":" << (gKitchenQueue.isFull() ? "true" : "false") << ","
           << "\"queuedOrderIds\":[";
        for (int i = 0; i < gKitchenQueue.size(); ++i) {
            int qId = 0;
            if (gKitchenQueue.getAtOffset(i, qId)) {
                ss << qId;
                if (i < gKitchenQueue.size() - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"arrayStats\":{"
           << "\"totalDishes\":" << gMenuItemCount << ","
           << "\"minDishPrice\":" << minPrice << ","
           << "\"maxDishPrice\":" << maxPrice << ","
           << "\"averageDishPrice\":" << avgPrice
           << "},"
           << "\"stlStats\":{"
           << "\"pastBillsCount\":" << gSTLManager.getPastBillsCount() << ","
           << "\"couponsCount\":" << gSTLManager.getCouponsCount() << ","
           << "\"cuisinesCount\":" << gSTLManager.getCuisinesCount()
           << "}"
           << "}";

        return makeResponse(true, "Engine Internal Memory Inspector", ss.str(),
            {"Module VIII (ArrayStack Memory)", "Module IX (CircularQueue Buffer)", "Module III (1D Array Stats)", "Module X (STL Containers)"});
    }

    return makeResponse(false, "Unknown command: " + cmd, "{}", {});
}

// ============================================================================
// CONSOLE INTERACTIVE MODE
// ============================================================================
void runInteractiveConsole() {
    std::cout << "\n======================================================\n";
    std::cout << "  FOODRUSH / RESTORUSH - RESTAURANT & HOTEL POS ENGINE\n";
    std::cout << "  High-Performance C++ Core Engine (CS Syllabus I-X)\n";
    std::cout << "======================================================\n";
    std::cout << "Available Commands:\n";
    std::cout << "  GET_INITIAL_STATE          - View POS boot state\n";
    std::cout << "  GET_TABLES                 - View 12 dining tables\n";
    std::cout << "  SELECT_TABLE <id>          - Select active table (1-12)\n";
    std::cout << "  ORDER_ADD <id> <qty>       - Add dish to table order\n";
    std::cout << "  ORDER_UNDO                 - Undo last table modification\n";
    std::cout << "  SUBMIT_KOT <guest> <0|1>   - Dispatch Kitchen Order Ticket\n";
    std::cout << "  GET_ACTIVE_ORDERS          - View live kitchen KOT queue\n";
    std::cout << "  GENERATE_BILL <tableId>    - Settle bill with GST & receipt\n";
    std::cout << "  GET_BILLS                  - View past settled bills archive\n";
    std::cout << "  PRINT_BILL <billId>        - Print formatted thermal receipt\n";
    std::cout << "  GET_STAFF                  - View staff attendance & shifts\n";
    std::cout << "  GET_SALES_MATRIX           - 6 Outlets x 7 Days weekly sales\n";
    std::cout << "  BENCHMARK                  - Run stopwatch performance test\n";
    std::cout << "  COMPARE_DS                 - Custom ArrayStack vs STL test\n";
    std::cout << "  EXIT                       - Quit interactive console\n";
    std::cout << "======================================================\n\n";

    std::string line;
    while (true) {
        std::cout << "FoodRush POS [Table " << gActiveTableId << "]> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "EXIT" || line == "quit") break;
        if (line.empty()) continue;

        std::string response = handleCommand(line);
        std::cout << response << "\n\n";
    }
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char* argv[]) {
    // 1. Initialize Seed Data (6 Outlets, 48 Dishes, 12 Tables, 8 Staff)
    initializeSeedData(gOutlets, gOutletCount, gMenuItems, gMenuItemCount, gTables, gTableCount, gStaff, gStaffCount);

    // 2. Populate STL Manager collections
    for (int i = 0; i < gMenuItemCount; ++i) {
        gSTLManager.addMenuItem(gMenuItems[i]);
    }
    for (int i = 0; i < gOutletCount; ++i) {
        gSTLManager.addCuisine(gOutlets[i].cuisine);
    }

    // 3. Seed Concurrent Active KOT Orders & Past Bills
    initializeActiveOrdersAndBills();

    // 4. Interactive Console Mode Check
    if (argc > 1 && std::string(argv[1]) == "--interactive") {
        runInteractiveConsole();
        return 0;
    }

    // 5. Line-by-Line Standard I/O Pipe Mode (Used by Node.js Server Bridge)
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::string res = handleCommand(line);
        std::cout << res << std::endl; // Flush stdout per command line
    }

    return 0;
}
