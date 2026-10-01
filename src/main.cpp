// ============================================================================
// FoodRush - High-Performance C++ Core Engine
// Full Implementation of CS Syllabus Modules I-X
// ============================================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <chrono>

#include "models.hpp"
#include "array_stack.hpp"
#include "array_queue.hpp"
#include "sales_matrix.hpp"
#include "algorithms.hpp"
#include "seed_data.hpp"

// Global System State
Restaurant gRestaurants[MAX_RESTAURANTS];
int gRestaurantCount = 0;

MenuItem gMenuItems[MAX_TOTAL_ITEMS];
int gMenuItemCount = 0;

Rider gRiders[MAX_RIDERS];
int gRiderCount = 0;

// [MODULE VIII] Custom Array-based Stack for Cart Undo History
ArrayStack<CartAction, 50> gUndoStack;

// [MODULE VIII] ArrayStack for Recently Viewed Dishes (LIFO)
ArrayStack<int, 20> gRecentlyViewedStack;

// [MODULE IX] Custom Array-based Circular Queue for Kitchen Orders (FIFO)
CircularQueue<int, 50> gKitchenQueue;

// [MODULE IX] Express vs Standard Priority Order Queue
PriorityOrderQueue<int, 50> gPriorityOrderQueue;

// [MODULE IX] Circular Queue for Rider Rotation
CircularQueue<int, MAX_RIDERS> gAvailableRiderQueue;

// [MODULE IV] 2D Array Manager for Sales Matrix and Distance Lookup
SalesMatrixManager gSalesMatrix;

// [MODULE X] STL Manager for Maps, Sets, Vectors, Deques, and STL Stacks/Queues
STLManager gSTLManager;

// Current Active Shopping Cart State
CartItem gCartItems[MAX_CART_ITEMS];
int gCartCount = 0;
std::string gActiveCouponCode = "";
double gActiveDiscountPercent = 0.0;
double gActiveDiscountFlat = 0.0;

// Orders Archive
Order gOrders[MAX_ORDERS_HISTORY];
int gOrderCount = 0;
int gNextOrderId = 1001;

// ============================================================================
// [MODULE II] HELPER FUNCTIONS (Pass-by-value, Pass-by-reference, Overloading)
// ============================================================================

// [MODULE II] Function Overloading: formatCurrency with double
inline std::string formatCurrency(double amount) {
    std::ostringstream ss;
    ss << CURRENCY_SYMBOL << std::fixed << std::setprecision(2) << amount;
    return ss.str();
}

// [MODULE II] Function Overloading: formatCurrency with integer paise (cents)
inline std::string formatCurrency(int paise) {
    // [MODULE I] Type conversion: static_cast from int paise to double rupees
    double rupees = static_cast<double>(paise) / 100.0;
    return formatCurrency(rupees);
}

const MenuItem* findMenuItemById(int itemId) {
    for (int i = 0; i < gMenuItemCount; ++i) {
        if (gMenuItems[i].id == itemId) {
            return &gMenuItems[i];
        }
    }
    return nullptr;
}

MenuItem* findMenuItemByIdMutable(int itemId) {
    for (int i = 0; i < gMenuItemCount; ++i) {
        if (gMenuItems[i].id == itemId) {
            return &gMenuItems[i];
        }
    }
    return nullptr;
}

const Restaurant* findRestaurantById(int restId) {
    for (int i = 0; i < gRestaurantCount; ++i) {
        if (gRestaurants[i].id == restId) {
            return &gRestaurants[i];
        }
    }
    return nullptr;
}

Order* findOrderById(int orderId) {
    for (int i = 0; i < gOrderCount; ++i) {
        if (gOrders[i].orderId == orderId) {
            return &gOrders[i];
        }
    }
    return nullptr;
}

// Calculate live position of an order in the kitchen circular queue
int getOrderQueuePosition(int orderId) {
    for (int i = 0; i < gKitchenQueue.size(); ++i) {
        int qId = 0;
        if (gKitchenQueue.getAtOffset(i, qId) && qId == orderId) {
            return i + 1; // 1-indexed queue position
        }
    }
    return 0; // Not currently waiting in queue (cooking, out for delivery, or delivered)
}

// [MODULE I] Bill Calculation using operators and type conversion
void calculateCartTotals(
    double& outSubtotal,
    double& outTax,
    double& outDeliveryFee,
    double& outDiscount,
    double& outTotal,
    int deliveryZoneId = 0,
    bool isExpress = false)
{
    outSubtotal = 0.0;
    int restaurantZoneId = 0;

    for (int i = 0; i < gCartCount; ++i) {
        outSubtotal += gCartItems[i].getLineTotal();
        const Restaurant* r = findRestaurantById(gCartItems[i].restaurantId);
        if (r) {
            restaurantZoneId = r->zoneId;
        }
    }

    if (gCartCount == 0) {
        outTax = 0.0;
        outDeliveryFee = 0.0;
        outDiscount = 0.0;
        outTotal = 0.0;
        return;
    }

    // [MODULE I] Arithmetic operators for tax & delivery calculation
    outTax = outSubtotal * TAX_RATE;

    int estimatedMins = 0;
    SalesMatrixManager::calculateDelivery(restaurantZoneId, deliveryZoneId, isExpress, outDeliveryFee, estimatedMins);

    // Apply coupon discount
    outDiscount = 0.0;
    if (gActiveDiscountPercent > 0.0) {
        outDiscount += (outSubtotal * (gActiveDiscountPercent / 100.0));
    }
    outDiscount += gActiveDiscountFlat;
    if (gActiveCouponCode.find("FREEDEL") != std::string::npos) {
        outDeliveryFee = 0.0; // Free delivery coupon
    }
    if (outDiscount > outSubtotal) {
        outDiscount = outSubtotal;
    }

    outTotal = (outSubtotal - outDiscount) + outTax + outDeliveryFee;
    if (outTotal < 0.0) outTotal = 0.0;
}

// Serialize current cart to JSON string
std::string cartToJSON(int deliveryZone = 0, bool isExpress = false) {
    double subtotal = 0.0, tax = 0.0, delFee = 0.0, discount = 0.0, total = 0.0;
    calculateCartTotals(subtotal, tax, delFee, discount, total, deliveryZone, isExpress);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{"
       << "\"itemCount\":" << gCartCount << ","
       << "\"items\":[";
    for (int i = 0; i < gCartCount; ++i) {
        ss << gCartItems[i].toJSON();
        if (i < gCartCount - 1) ss << ",";
    }
    ss << "],"
       << "\"coupon\":\"" << gActiveCouponCode << "\","
       << "\"discountPercent\":" << gActiveDiscountPercent << ","
       << "\"discountAmount\":" << discount << ","
       << "\"subtotal\":" << subtotal << ","
       << "\"tax\":" << tax << ","
       << "\"deliveryFee\":" << delFee << ","
       << "\"total\":" << total << ","
       << "\"undoStackDepth\":" << gUndoStack.size()
       << "}";
    return ss.str();
}

// Build standard JSON response supporting both "modules" and "handled_by"
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
// [MODULE VIII] CART ACTIONS & UNDO ENGINE
// ============================================================================

bool addToCartInternal(int itemId, int quantity, bool recordUndo = true) {
    if (quantity <= 0) return false;
    MenuItem* item = findMenuItemByIdMutable(itemId);
    if (!item || item->stock < quantity) {
        return false;
    }

    int existingIdx = -1;
    for (int i = 0; i < gCartCount; ++i) {
        if (gCartItems[i].itemId == itemId) {
            existingIdx = i;
            break;
        }
    }

    int prevQty = 0;
    int newQty = quantity;

    if (existingIdx != -1) {
        prevQty = gCartItems[existingIdx].quantity;
        newQty = prevQty + quantity;
        gCartItems[existingIdx].quantity = newQty;
    } else {
        if (gCartCount >= MAX_CART_ITEMS) return false;
        gCartItems[gCartCount] = {itemId, item->restaurantId, item->name, item->price, quantity};
        gCartCount++;
    }

    // [MODULE VIII] Stack push | used by: UNDO
    if (recordUndo) {
        CartAction action;
        action.type = (existingIdx == -1 ? ACTION_ADD_ITEM : ACTION_UPDATE_QTY);
        action.itemId = itemId;
        action.previousQuantity = prevQty;
        action.newQuantity = newQty;
        action.itemPrice = item->price;
        action.itemName = item->name;
        gUndoStack.push(action);

        gSTLManager.pushUndo(action);
    }

    return true;
}

bool removeFromCartInternal(int itemId, bool recordUndo = true) {
    int foundIdx = -1;
    for (int i = 0; i < gCartCount; ++i) {
        if (gCartItems[i].itemId == itemId) {
            foundIdx = i;
            break;
        }
    }
    if (foundIdx == -1) return false;

    CartItem itemBeingRemoved = gCartItems[foundIdx];

    // [MODULE VIII] Stack push | used by: UNDO
    if (recordUndo) {
        CartAction action;
        action.type = ACTION_REMOVE_ITEM;
        action.itemId = itemId;
        action.previousQuantity = itemBeingRemoved.quantity;
        action.newQuantity = 0;
        action.itemPrice = itemBeingRemoved.price;
        action.itemName = itemBeingRemoved.name;
        gUndoStack.push(action);

        gSTLManager.pushUndo(action);
    }

    // Shift 1D array to remove item
    for (int i = foundIdx; i < gCartCount - 1; ++i) {
        gCartItems[i] = gCartItems[i + 1];
    }
    gCartCount--;
    return true;
}

// [MODULE VIII] Stack pop & Reverse Action | used by: UNDO
bool performCartUndo(std::string& outActionDesc) {
    CartAction action;
    if (!gUndoStack.pop(action)) {
        outActionDesc = "Undo history is empty.";
        return false;
    }

    CartAction stlAction;
    gSTLManager.popUndo(stlAction);

    std::ostringstream ss;

    if (action.type == ACTION_ADD_ITEM) {
        if (action.previousQuantity == 0) {
            removeFromCartInternal(action.itemId, false);
            ss << "Removed '" << action.itemName << "' from order";
        } else {
            for (int i = 0; i < gCartCount; ++i) {
                if (gCartItems[i].itemId == action.itemId) {
                    gCartItems[i].quantity = action.previousQuantity;
                    break;
                }
            }
            ss << "Updated quantity of '" << action.itemName << "' to " << action.previousQuantity;
        }
    } else if (action.type == ACTION_UPDATE_QTY) {
        for (int i = 0; i < gCartCount; ++i) {
            if (gCartItems[i].itemId == action.itemId) {
                gCartItems[i].quantity = action.previousQuantity;
                break;
            }
        }
        ss << "Restored quantity of '" << action.itemName << "' to " << action.previousQuantity;
    } else if (action.type == ACTION_REMOVE_ITEM) {
        addToCartInternal(action.itemId, action.previousQuantity, false);
        ss << "Restored '" << action.itemName << "' to order";
    }

    outActionDesc = ss.str();
    return true;
}

// ============================================================================
// [MODULE II] COMMAND DISPATCHER & PROTOCOL HANDLER
// ============================================================================

std::string handleCommand(const std::string& rawLine) {
    std::vector<std::string> tokens = tokenizeString(rawLine, ' ');
    if (tokens.empty()) {
        return makeResponse(false, "Empty command", "{}", {"Module II (Control Statements)"});
    }

    std::string cmd = tokens[0];

    // ------------------------------------------------------------------------
    // PING
    // ------------------------------------------------------------------------
    if (cmd == "PING") {
        return makeResponse(true, "FoodRush Engine is active", "{\"status\":\"OK\"}", {"Module I (Basics)", "Module II (Control)"});
    }

    // ------------------------------------------------------------------------
    // GET_RESTAURANTS
    // ------------------------------------------------------------------------
    if (cmd == "GET_RESTAURANTS") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gRestaurantCount; ++i) {
            ss << gRestaurants[i].toJSON();
            if (i < gRestaurantCount - 1) ss << ",";
        }
        ss << "]";
        return makeResponse(true, "Fetched all restaurants", ss.str(), {
            "Module VI (Structures - Restaurant)",
            "Module III (1D Arrays - restaurants[])"
        });
    }

    // ------------------------------------------------------------------------
    // GET_MENU [restaurantId]
    // ------------------------------------------------------------------------
    if (cmd == "GET_MENU") {
        int filterRestId = (tokens.size() > 1) ? std::stoi(tokens[1]) : 0;
        std::ostringstream ss;
        ss << "[";
        bool first = true;
        for (int i = 0; i < gMenuItemCount; ++i) {
            if (filterRestId == 0 || gMenuItems[i].restaurantId == filterRestId) {
                if (!first) ss << ",";
                ss << gMenuItems[i].toJSON();
                first = false;
            }
        }
        ss << "]";
        return makeResponse(true, "Fetched menu items", ss.str(), {
            "Module VI (Structures - MenuItem)",
            "Module III (1D Arrays - menuItems[])",
            "Module I (Basics - Bitwise dietary flags)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE V] SEARCH_DISH <query>
    // ------------------------------------------------------------------------
    if (cmd == "SEARCH_DISH") {
        std::string query = "";
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (i > 1) query += " ";
            query += tokens[i];
        }

        std::ostringstream ss;
        ss << "{\"matches\":[";
        bool first = true;
        int matchCount = 0;

        for (int i = 0; i < gMenuItemCount; ++i) {
            if (stringContains(gMenuItems[i].name, query) ||
                stringContains(gMenuItems[i].category, query)) {
                if (!first) ss << ",";
                ss << gMenuItems[i].toJSON();
                first = false;
                matchCount++;

                gRecentlyViewedStack.push(gMenuItems[i].id);
            }
        }
        ss << "]";

        // [MODULE V] Did You Mean fuzzy suggestion using Levenshtein distance
        std::string didYouMean = "";
        if (matchCount == 0 && !query.empty()) {
            didYouMean = findDidYouMeanSuggestion(query, gMenuItems, gMenuItemCount);
        }

        ss << ",\"didYouMean\":\"" << escapeJSON(didYouMean) << "\""
           << ",\"matchCount\":" << matchCount
           << "}";

        return makeResponse(true, "Search completed", ss.str(), {
            "Module V (Strings - Traversal, toLower, containsSubstring, Levenshtein)",
            "Module VIII (Stack - Recently Viewed Dishes)",
            "Module III (1D Arrays - Linear scan)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VIII] CART_ADD <itemId> <quantity>
    // ------------------------------------------------------------------------
    if (cmd == "CART_ADD") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: CART_ADD <itemId> <quantity>", "{}", {"Module II (Control)"});
        }
        int itemId = std::stoi(tokens[1]);
        int quantity = std::stoi(tokens[2]);

        bool ok = addToCartInternal(itemId, quantity, true);
        if (!ok) {
            return makeResponse(false, "Could not add item: out of stock or cart limit reached.", "{}", {"Module VIII (Stack)", "Module I (Basics)"});
        }

        return makeResponse(true, "Item added to your order", cartToJSON(), {
            "Module VIII (Stack - push action for undo)",
            "Module VI (Structures - CartItem)",
            "Module I (Basics - price calculation)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VIII] CART_REMOVE <itemId>
    // ------------------------------------------------------------------------
    if (cmd == "CART_REMOVE") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: CART_REMOVE <itemId>", "{}", {"Module II (Control)"});
        }
        int itemId = std::stoi(tokens[1]);
        bool ok = removeFromCartInternal(itemId, true);
        if (!ok) {
            return makeResponse(false, "Item not found in order", "{}", {"Module II (Control)"});
        }
        return makeResponse(true, "Item removed from order", cartToJSON(), {
            "Module VIII (Stack - push removal action for undo)",
            "Module III (1D Arrays - array shifting)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VIII] CART_UNDO
    // ------------------------------------------------------------------------
    if (cmd == "CART_UNDO") {
        std::string actionDesc;
        bool ok = performCartUndo(actionDesc);
        if (!ok) {
            return makeResponse(false, actionDesc, cartToJSON(), {
                "Module VIII (Stack - pop underflow guard)"
            });
        }
        return makeResponse(true, actionDesc, cartToJSON(), {
            "Module VIII (Stack - pop & reverse action)",
            "Module VI (Structures - CartAction)",
            "Module X (STL - std::stack synchronizer)"
        });
    }

    // ------------------------------------------------------------------------
    // CART_VIEW [zoneId] [isExpress]
    // ------------------------------------------------------------------------
    if (cmd == "CART_VIEW") {
        int zone = (tokens.size() > 1) ? std::stoi(tokens[1]) : 0;
        bool express = (tokens.size() > 2) ? (tokens[2] == "1" || tokens[2] == "true") : false;
        return makeResponse(true, "Current order summary", cartToJSON(zone, express), {
            "Module I (Basics - tax, subtotal, delivery calculation)",
            "Module IV (2D Arrays - zone distance lookup)",
            "Module VI (Structures - CartItem)"
        });
    }

    // ------------------------------------------------------------------------
    // CART_CLEAR
    // ------------------------------------------------------------------------
    if (cmd == "CART_CLEAR") {
        gCartCount = 0;
        gUndoStack.clear();
        gActiveCouponCode = "";
        gActiveDiscountPercent = 0.0;
        gActiveDiscountFlat = 0.0;
        return makeResponse(true, "Cart cleared", cartToJSON(), {
            "Module VIII (Stack - clear)",
            "Module III (1D Arrays - reset count)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE V] & [MODULE X] APPLY_COUPON <code>
    // ------------------------------------------------------------------------
    if (cmd == "APPLY_COUPON") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Please provide a coupon code", "{}", {"Module II (Control)"});
        }
        std::string rawCode = tokens[1];
        std::string upperCode = "";
        for (char c : rawCode) upperCode += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        double discountPct = 0.0;
        bool found = gSTLManager.getCouponDiscount(upperCode, discountPct);

        // [MODULE V] String Reversal for Palindrome Promo Bonus
        std::string revCode = reverseString(upperCode);
        bool isPalindrome = (upperCode.length() >= 3 && upperCode == revCode);

        if (found || isPalindrome) {
            if (isPalindrome && !found) {
                discountPct = 15.0; // 15% VIP Palindrome bonus
                upperCode += " (Palindrome Bonus)";
            } else if (isPalindrome && found) {
                discountPct += 10.0; // Additional 10% bonus
                upperCode += " + Palindrome (+10%)";
            }
            gActiveCouponCode = upperCode;
            gActiveDiscountPercent = discountPct;

            std::ostringstream ss;
            ss << "{\"coupon\":\"" << upperCode << "\",\"discountPercent\":" << discountPct << "}";
            return makeResponse(true, "Promotion applied: " + std::to_string(static_cast<int>(discountPct)) + "% OFF", ss.str(), {
                "Module X (STL - std::map lookup)",
                "Module V (Strings - reverseString palindrome check)",
                "Module I (Basics - percentage discount)"
            });
        }

        return makeResponse(false, "Invalid coupon code. Try FIRST50, WELCOME10, or FREEDEL.", "{}", {
            "Module X (STL - std::map::find)",
            "Module V (Strings - reverseString)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VI], [MODULE IX], [MODULE IV], [MODULE V] CHECKOUT
    // ------------------------------------------------------------------------
    if (cmd == "CHECKOUT") {
        if (tokens.size() < 4) {
            return makeResponse(false, "Usage: CHECKOUT <name> <zoneId> <street> [isExpress]", "{}", {"Module II (Control)"});
        }
        if (gCartCount == 0) {
            return makeResponse(false, "Cannot checkout: your cart is empty", "{}", {"Module II (Control)"});
        }

        std::string customerName = tokens[1];
        int zoneId = std::stoi(tokens[2]);
        std::string street = tokens[3];
        bool isExpress = (tokens.size() > 4) ? (tokens[4] == "1" || tokens[4] == "true") : false;

        if (zoneId < 0 || zoneId >= MAX_ZONES) zoneId = 0;

        double subtotal = 0.0, tax = 0.0, delFee = 0.0, discount = 0.0, total = 0.0;
        calculateCartTotals(subtotal, tax, delFee, discount, total, zoneId, isExpress);

        int restId = gCartItems[0].restaurantId;
        const Restaurant* rest = findRestaurantById(restId);
        int restZone = rest ? rest->zoneId : 0;
        std::string restName = rest ? rest->name : "Partner Kitchen";
        int estimatedMins = 0;
        double dummyFee = 0.0;
        SalesMatrixManager::calculateDelivery(restZone, zoneId, isExpress, dummyFee, estimatedMins);

        int orderId = gNextOrderId++;

        // [MODULE V] Generate verified tracking code with check digit using string reversal
        std::string trackingCode = generateTrackingCode(orderId);

        // [MODULE VI] Build Order Struct
        Order newOrder;
        newOrder.orderId = orderId;
        newOrder.trackingCode = trackingCode;
        newOrder.restaurantId = restId;
        newOrder.restaurantName = restName;
        newOrder.customerName = customerName;
        newOrder.deliveryAddress = {street, zoneId, "", ""};
        newOrder.itemCount = gCartCount;
        for (int i = 0; i < gCartCount; ++i) {
            newOrder.items[i] = gCartItems[i];
            MenuItem* m = findMenuItemByIdMutable(gCartItems[i].itemId);
            if (m && m->stock >= gCartItems[i].quantity) {
                m->stock -= gCartItems[i].quantity;
            }
        }
        newOrder.subtotal = subtotal;
        newOrder.tax = tax;
        newOrder.deliveryFee = delFee;
        newOrder.discount = discount;
        newOrder.totalAmount = total;
        newOrder.isExpress = isExpress;
        newOrder.status = "PLACED";
        newOrder.assignedRiderId = -1;
        newOrder.riderName = "Assigning delivery partner";
        newOrder.estimatedMinutes = estimatedMins;

        // [MODULE IX] Circular Queue Enqueue for Kitchen Orders
        gKitchenQueue.enqueue(newOrder.orderId);
        gPriorityOrderQueue.enqueue(newOrder.orderId, isExpress);
        gSTLManager.enqueueOrder(newOrder.orderId);

        newOrder.queuePosition = getOrderQueuePosition(orderId);

        if (gOrderCount < MAX_ORDERS_HISTORY) {
            gOrders[gOrderCount++] = newOrder;
        }

        // [MODULE IV] Record live sale into 2D Sales Matrix
        int restaurantIndex = restId - 1;
        if (restaurantIndex >= 0 && restaurantIndex < MAX_RESTAURANTS) {
            gSalesMatrix.recordSale(restaurantIndex, 6, total);
        }

        // Reset cart after checkout
        gCartCount = 0;
        gUndoStack.clear();
        gActiveCouponCode = "";
        gActiveDiscountPercent = 0.0;
        gActiveDiscountFlat = 0.0;

        return makeResponse(true, "Order placed successfully! Tracking code: " + trackingCode, newOrder.toJSON(), {
            "Module VI (Structures - Nested Order & Address)",
            "Module IX (Queue - Circular & Priority Enqueue)",
            "Module V (Strings - Tracking code check-digit via string reversal)",
            "Module IV (2D Arrays - Sales Matrix update & Distance fee)",
            "Module I (Basics - Bill & tax calculation)",
            "Module III (1D Arrays - Stock deduction)"
        });
    }

    // ------------------------------------------------------------------------
    // TRACK_ORDER <orderIdOrCode>
    // ------------------------------------------------------------------------
    if (cmd == "TRACK_ORDER") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: TRACK_ORDER <orderIdOrCode>", "{}", {"Module II (Control)"});
        }
        std::string query = tokens[1];
        Order* target = nullptr;

        // Try lookup by numeric order ID
        try {
            int id = std::stoi(query);
            target = findOrderById(id);
        } catch (...) {
            // Lookup by tracking code
            for (int i = 0; i < gOrderCount; ++i) {
                if (gOrders[i].trackingCode == query) {
                    target = &gOrders[i];
                    break;
                }
            }
        }

        if (!target) {
            // Return most recent order if available
            if (gOrderCount > 0) {
                target = &gOrders[gOrderCount - 1];
            } else {
                return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
            }
        }

        target->queuePosition = getOrderQueuePosition(target->orderId);

        // [MODULE V] Verify tracking code check digit
        bool isTrackingValid = validateTrackingCode(target->trackingCode);

        std::ostringstream ss;
        ss << "{"
           << "\"order\":" << target->toJSON() << ","
           << "\"trackingCodeVerified\":" << (isTrackingValid ? "true" : "false")
           << "}";

        return makeResponse(true, "Tracking details for order #" + std::to_string(target->orderId), ss.str(), {
            "Module V (Strings - Tracking code verification via reversal)",
            "Module IX (Queue - Live circular queue position inspection)",
            "Module VI (Structures - Order)"
        });
    }

    // ------------------------------------------------------------------------
    // SIMULATE_NEXT_STAGE [orderId] (Step order to next stage for customer tracking)
    // ------------------------------------------------------------------------
    if (cmd == "SIMULATE_NEXT_STAGE") {
        Order* target = nullptr;
        if (tokens.size() > 1) {
            int ordId = std::stoi(tokens[1]);
            target = findOrderById(ordId);
        } else if (gOrderCount > 0) {
            target = &gOrders[gOrderCount - 1]; // Latest order
        }

        if (!target) {
            return makeResponse(false, "No active order to advance", "{}", {"Module II (Control)"});
        }

        std::string prevStatus = target->status;
        std::string newStatus = prevStatus;

        if (prevStatus == "PLACED") {
            // Move to PREPARING (kitchen begins)
            target->status = "PREPARING";
            target->riderName = "Chef is preparing your meal";
            newStatus = "PREPARING";
        } else if (prevStatus == "PREPARING") {
            // Kitchen finishes: dequeue from kitchen queue and assign rider
            int dummyId = 0;
            bool wasExpress = false;
            gPriorityOrderQueue.dequeue(dummyId, wasExpress);
            gKitchenQueue.dequeue(dummyId);
            gSTLManager.dequeueOrder(dummyId);

            // Assign rider
            int riderId = -1;
            if (!gAvailableRiderQueue.dequeue(riderId)) {
                for (int i = 0; i < gRiderCount; ++i) {
                    if (gRiders[i].isAvailable) {
                        riderId = gRiders[i].id;
                        break;
                    }
                }
            }

            Rider* r = nullptr;
            for (int i = 0; i < gRiderCount; ++i) {
                if (gRiders[i].id == riderId) {
                    r = &gRiders[i];
                    r->isAvailable = false;
                    break;
                }
            }

            target->status = "OUT_FOR_DELIVERY";
            target->assignedRiderId = riderId;
            target->riderName = r ? (r->name + " (" + r->vehicle + ")") : "Delivery Partner";
            target->estimatedMinutes = target->isExpress ? 10 : 18;
            newStatus = "OUT_FOR_DELIVERY";
        } else if (prevStatus == "OUT_FOR_DELIVERY") {
            // Deliver order: free up rider and return to circular queue
            target->status = "DELIVERED";
            target->estimatedMinutes = 0;

            if (target->assignedRiderId != -1) {
                for (int i = 0; i < gRiderCount; ++i) {
                    if (gRiders[i].id == target->assignedRiderId) {
                        gRiders[i].isAvailable = true;
                        gRiders[i].totalDeliveries++;
                        gAvailableRiderQueue.enqueue(gRiders[i].id); // Return to rotation
                        break;
                    }
                }
            }
            gSTLManager.recordCompletedOrder(target->orderId);
            newStatus = "DELIVERED";
        }

        target->queuePosition = getOrderQueuePosition(target->orderId);

        return makeResponse(true, "Order #" + std::to_string(target->orderId) + " transitioned to: " + newStatus, target->toJSON(), {
            "Module IX (Queue - Circular Queue dequeue & Rider rotation)",
            "Module VI (Structures - Order state transition)",
            "Module X (STL - std::deque completed orders)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE IX] GET_ORDERS
    // ------------------------------------------------------------------------
    if (cmd == "GET_ORDERS") {
        std::ostringstream ss;
        ss << "{"
           << "\"kitchenQueueSize\":" << gKitchenQueue.size() << ","
           << "\"priorityQueueSize\":" << gPriorityOrderQueue.totalSize() << ","
           << "\"expressQueueSize\":" << gPriorityOrderQueue.expressSize() << ","
           << "\"orders\":[";
        for (int i = gOrderCount - 1; i >= 0; --i) {
            ss << gOrders[i].toJSON();
            if (i > 0) ss << ",";
        }
        ss << "]}";
        return makeResponse(true, "Fetched orders", ss.str(), {
            "Module IX (Queue - CircularQueue size & peek)",
            "Module VI (Structures - Order[])"
        });
    }

    // ------------------------------------------------------------------------
    // COOK_ORDER
    // ------------------------------------------------------------------------
    if (cmd == "COOK_ORDER") {
        int orderId = -1;
        if (tokens.size() > 1) {
            orderId = std::stoi(tokens[1]);
        } else {
            bool wasExpress = false;
            if (!gPriorityOrderQueue.dequeue(orderId, wasExpress)) {
                return makeResponse(false, "Kitchen queue is empty", "{}", {"Module IX (Queue)"});
            }
            int circId = -1;
            gKitchenQueue.dequeue(circId);
            int stlId = -1;
            gSTLManager.dequeueOrder(stlId);
        }

        Order* targetOrder = findOrderById(orderId);
        if (!targetOrder) {
            return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
        }

        targetOrder->status = "READY_FOR_PICKUP";
        targetOrder->riderName = "Awaiting delivery partner pickup";

        return makeResponse(true, "Order #" + std::to_string(orderId) + " prepared by kitchen", targetOrder->toJSON(), {
            "Module IX (Queue - Circular Queue dequeue)",
            "Module VI (Structures - Order status update)"
        });
    }

    // ------------------------------------------------------------------------
    // ASSIGN_RIDER
    // ------------------------------------------------------------------------
    if (cmd == "ASSIGN_RIDER") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: ASSIGN_RIDER <orderId>", "{}", {"Module II (Control)"});
        }
        int orderId = std::stoi(tokens[1]);
        Order* targetOrder = findOrderById(orderId);
        if (!targetOrder) {
            return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
        }

        int riderId = -1;
        if (!gAvailableRiderQueue.dequeue(riderId)) {
            for (int i = 0; i < gRiderCount; ++i) {
                if (gRiders[i].isAvailable) {
                    riderId = gRiders[i].id;
                    break;
                }
            }
        }

        if (riderId == -1) {
            return makeResponse(false, "All delivery partners are currently busy", "{}", {"Module IX (Queue)"});
        }

        Rider* assignedRider = nullptr;
        for (int i = 0; i < gRiderCount; ++i) {
            if (gRiders[i].id == riderId) {
                assignedRider = &gRiders[i];
                break;
            }
        }

        if (assignedRider) {
            assignedRider->isAvailable = false;
            targetOrder->assignedRiderId = assignedRider->id;
            targetOrder->riderName = assignedRider->name + " (" + assignedRider->vehicle + ")";
            targetOrder->status = "OUT_FOR_DELIVERY";
        }

        return makeResponse(true, "Delivery partner assigned: " + targetOrder->riderName, targetOrder->toJSON(), {
            "Module IX (Queue - Circular Rider Queue rotation)",
            "Module VI (Structures - Rider & Order linkage)"
        });
    }

    // ------------------------------------------------------------------------
    // COMPLETE_ORDER
    // ------------------------------------------------------------------------
    if (cmd == "COMPLETE_ORDER") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: COMPLETE_ORDER <orderId>", "{}", {"Module II (Control)"});
        }
        int orderId = std::stoi(tokens[1]);
        Order* targetOrder = findOrderById(orderId);
        if (!targetOrder) {
            return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
        }

        targetOrder->status = "DELIVERED";
        targetOrder->estimatedMinutes = 0;

        if (targetOrder->assignedRiderId != -1) {
            for (int i = 0; i < gRiderCount; ++i) {
                if (gRiders[i].id == targetOrder->assignedRiderId) {
                    gRiders[i].isAvailable = true;
                    gRiders[i].totalDeliveries++;
                    gAvailableRiderQueue.enqueue(gRiders[i].id);
                    break;
                }
            }
        }

        gSTLManager.recordCompletedOrder(orderId);

        return makeResponse(true, "Order #" + std::to_string(orderId) + " delivered successfully", targetOrder->toJSON(), {
            "Module IX (Queue - Return rider to Circular Queue)",
            "Module X (STL - std::deque completed orders)",
            "Module VI (Structures - Status update)"
        });
    }

    // ------------------------------------------------------------------------
    // GET_RIDERS
    // ------------------------------------------------------------------------
    if (cmd == "GET_RIDERS") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gRiderCount; ++i) {
            ss << gRiders[i].toJSON();
            if (i < gRiderCount - 1) ss << ",";
        }
        ss << "]";
        return makeResponse(true, "Fetched delivery fleet", ss.str(), {
            "Module VI (Structures - Rider)",
            "Module III (1D Arrays - riders[])",
            "Module IX (Queue - available rider tracking)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE IV] GET_SALES_MATRIX
    // ------------------------------------------------------------------------
    if (cmd == "GET_SALES_MATRIX") {
        return makeResponse(true, "Weekly sales matrix and metrics", gSalesMatrix.toJSON(), {
            "Module IV (2D Arrays - sales[6][7] row/col sums)",
            "Module IV (2D Arrays - zone distance matrix[5][5])"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE III] GET_ARRAY_STATS
    // ------------------------------------------------------------------------
    if (cmd == "GET_ARRAY_STATS") {
        double ratings[MAX_RESTAURANTS];
        for (int i = 0; i < gRestaurantCount; ++i) {
            ratings[i] = gRestaurants[i].rating;
        }

        double prices[MAX_TOTAL_ITEMS];
        int stocks[MAX_TOTAL_ITEMS];
        for (int i = 0; i < gMenuItemCount; ++i) {
            prices[i] = gMenuItems[i].price;
            stocks[i] = gMenuItems[i].stock;
        }

        int maxRatingIdx = 0, minRatingIdx = 0;
        double maxRating = findArrayMax(ratings, gRestaurantCount, maxRatingIdx);
        double minRating = findArrayMin(ratings, gRestaurantCount, minRatingIdx);
        double avgRating = calculateArraySum(ratings, gRestaurantCount) / gRestaurantCount;

        int maxPriceIdx = 0, minPriceIdx = 0;
        double maxPrice = findArrayMax(prices, gMenuItemCount, maxPriceIdx);
        double minPrice = findArrayMin(prices, gMenuItemCount, minPriceIdx);
        double totalPriceSum = calculateArraySum(prices, gMenuItemCount);
        double avgPrice = totalPriceSum / gMenuItemCount;

        double sortedPrices[MAX_TOTAL_ITEMS];
        for (int i = 0; i < gMenuItemCount; ++i) sortedPrices[i] = prices[i];
        bubbleSortArray(sortedPrices, gMenuItemCount);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"ratings\":{"
           << "\"average\":" << avgRating << ","
           << "\"max\":" << maxRating << ",\"bestRestaurant\":\"" << gRestaurants[maxRatingIdx].name << "\","
           << "\"min\":" << minRating << ",\"lowestRestaurant\":\"" << gRestaurants[minRatingIdx].name << "\""
           << "},"
           << "\"prices\":{"
           << "\"average\":" << avgPrice << ","
           << "\"max\":" << maxPrice << ",\"priciestDish\":\"" << gMenuItems[maxPriceIdx].name << "\","
           << "\"min\":" << minPrice << ",\"cheapestDish\":\"" << gMenuItems[minPriceIdx].name << "\","
           << "\"sortedSample\":[" << sortedPrices[0] << "," << sortedPrices[1] << "," << sortedPrices[2] << ",\"... \","
           << sortedPrices[gMenuItemCount-1] << "]"
           << "}"
           << "}";
        return makeResponse(true, "Catalog statistical operations complete", ss.str(), {
            "Module III (1D Arrays - Sum, Min, Max, Traversal)",
            "Module III (1D Arrays - Bubble Sort on prices)",
            "Module I (Basics - floating point division)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VII] BENCHMARK
    // ------------------------------------------------------------------------
    if (cmd == "BENCHMARK") {
        std::string benchJSON = PerformanceBenchmark::runFullBenchmark();
        return makeResponse(true, "Performance benchmark completed", benchJSON, {
            "Module VII (Performance - O(N) vs O(log N) Search)",
            "Module VII (Performance - O(N^2) vs O(N log N) Sort)",
            "Module VII (Performance - Time/Space Asymptotic Complexity)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE X] COMPARE_DS
    // ------------------------------------------------------------------------
    if (cmd == "COMPARE_DS") {
        int iters = (tokens.size() > 1) ? std::stoi(tokens[1]) : 50000;
        std::string compJSON = gSTLManager.compareDataStructures(iters);
        return makeResponse(true, "Data structure comparison completed", compJSON, {
            "Module X (STL - std::stack, std::queue, std::deque)",
            "Module VIII (Stack - ArrayStack direct comparison)",
            "Module IX (Queue - CircularQueue direct comparison)",
            "Module VII (Performance - Microsecond stopwatch benchmarking)"
        });
    }

    // ------------------------------------------------------------------------
    // [MODULE VIII] & [MODULE IX] INSPECT_ENGINE
    // ------------------------------------------------------------------------
    if (cmd == "INSPECT_ENGINE") {
        std::ostringstream ss;
        ss << "{"
           << "\"stackInspector\":{"
           << "\"capacity\":" << gUndoStack.capacity() << ","
           << "\"size\":" << gUndoStack.size() << ","
           << "\"isEmpty\":" << (gUndoStack.isEmpty() ? "true" : "false") << ","
           << "\"elements\":[";
        for (int i = 0; i < gUndoStack.size(); ++i) {
            CartAction act;
            if (gUndoStack.getAtDepth(i, act)) {
                ss << act.toJSON();
                if (i < gUndoStack.size() - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"recentlyViewedStack\":{"
           << "\"size\":" << gRecentlyViewedStack.size() << ","
           << "\"items\":[";
        for (int i = 0; i < gRecentlyViewedStack.size(); ++i) {
            int dishId = 0;
            if (gRecentlyViewedStack.getAtDepth(i, dishId)) {
                const MenuItem* m = findMenuItemById(dishId);
                ss << "{\"id\":" << dishId << ",\"name\":\"" << (m ? m->name : "Unknown") << "\"}";
                if (i < gRecentlyViewedStack.size() - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"circularQueueInspector\":{"
           << "\"capacity\":" << gKitchenQueue.capacity() << ","
           << "\"count\":" << gKitchenQueue.size() << ","
           << "\"frontIndex\":" << gKitchenQueue.getFrontIndex() << ","
           << "\"rearIndex\":" << gKitchenQueue.getRearIndex() << ","
           << "\"elements\":[";
        for (int i = 0; i < gKitchenQueue.size(); ++i) {
            int ordId = 0;
            if (gKitchenQueue.getAtOffset(i, ordId)) {
                ss << ordId;
                if (i < gKitchenQueue.size() - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"riderQueueInspector\":{"
           << "\"count\":" << gAvailableRiderQueue.size() << ","
           << "\"capacity\":" << gAvailableRiderQueue.capacity()
           << "},"
           << "\"stlOverview\":{"
           << "\"couponsInMap\":" << gSTLManager.getCoupons().size() << ","
           << "\"cuisinesInSet\":" << gSTLManager.getCuisines().size() << ","
           << "\"dailySpecialPairs\":" << gSTLManager.getDailySpecials().size()
           << "}"
           << "}";
        return makeResponse(true, "Internal state snapshot generated", ss.str(), {
            "Module VIII (Stack - Raw array memory layout inspection)",
            "Module IX (Queue - Circular buffer front/rear pointers)",
            "Module X (STL - Map/Set/Pair inspection)"
        });
    }

    // ------------------------------------------------------------------------
    // HELP
    // ------------------------------------------------------------------------
    if (cmd == "HELP") {
        return makeResponse(true, "Available Commands: PING, GET_RESTAURANTS, GET_MENU [id], SEARCH_DISH <query>, CART_ADD <id> <qty>, CART_REMOVE <id>, CART_UNDO, CART_VIEW, CART_CLEAR, APPLY_COUPON <code>, CHECKOUT <name> <zoneId> <street> [express], TRACK_ORDER <idOrCode>, SIMULATE_NEXT_STAGE [id], GET_ORDERS, COOK_ORDER [id], ASSIGN_RIDER <id>, COMPLETE_ORDER <id>, GET_RIDERS, GET_SALES_MATRIX, GET_ARRAY_STATS, BENCHMARK, COMPARE_DS, INSPECT_ENGINE, HELP", "{}", {"Module II (Control)"});
    }

    return makeResponse(false, "Unknown command: " + cmd + ". Type HELP for list.", "{}", {"Module II (Control)"});
}

// ============================================================================
// INTERACTIVE CLI CONSOLE MENU (Terminal Examiner Mode)
// ============================================================================
void runInteractiveCLI() {
    std::cout << "\n======================================================\n";
    std::cout << "         FOODRUSH - C++ CORE DELIVERY SYSTEM          \n";
    std::cout << "         Academic & Technical Console Session         \n";
    std::cout << "======================================================\n";

    while (true) {
        std::cout << "\n--- MAIN CONSOLE MENU ---\n";
        std::cout << "1. Browse Restaurants & Menu      [Modules III, VI]\n";
        std::cout << "2. Search Dishes with Fuzzy Match [Module V]\n";
        std::cout << "3. Manage Cart & Test Stack Undo  [Module VIII]\n";
        std::cout << "4. Track Order & Verify Reversal  [Module V, IX]\n";
        std::cout << "5. Kitchen Queue & Rider Rotation [Module IX]\n";
        std::cout << "6. 6x7 Sales Matrix & Distances   [Module IV]\n";
        std::cout << "7. 1D Array Statistics & Sort     [Module III]\n";
        std::cout << "8. Run Asymptotic Benchmarks      [Module VII]\n";
        std::cout << "9. Compare Array DS vs STL        [Module X]\n";
        std::cout << "10. Live Memory Inspector         [Modules VIII, IX]\n";
        std::cout << "0. Exit Console\n";
        std::cout << "Select option (0-10): ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::string dummy;
            std::getline(std::cin, dummy);
            continue;
        }

        std::cin.ignore(1000, '\n');

        if (choice == 0) {
            std::cout << "Exiting FoodRush Console.\n";
            break;
        } else if (choice == 1) {
            std::cout << "\n--- RESTAURANTS ---\n";
            for (int i = 0; i < gRestaurantCount; ++i) {
                std::cout << "[" << gRestaurants[i].id << "] " << gRestaurants[i].name
                          << " (" << gRestaurants[i].cuisine << ") | Rating: " << gRestaurants[i].rating
                          << "/5.0 | Min: " << formatCurrency(gRestaurants[i].minOrder) << "\n";
            }
            std::cout << "\nEnter restaurant ID to view menu (0 for all): ";
            int rId = 0;
            std::cin >> rId;
            std::cout << "\n--- DISHES ---\n";
            for (int i = 0; i < gMenuItemCount; ++i) {
                if (rId == 0 || gMenuItems[i].restaurantId == rId) {
                    std::cout << "ID: " << gMenuItems[i].id << " | " << gMenuItems[i].name
                              << " | Price: " << formatCurrency(gMenuItems[i].price)
                              << " | Stock: " << gMenuItems[i].stock
                              << (gMenuItems[i].isVeg ? " [VEG]" : " [NON-VEG]") << "\n";
                }
            }
        } else if (choice == 2) {
            std::cout << "Enter search query: ";
            std::string q;
            std::getline(std::cin, q);
            std::cout << handleCommand("SEARCH_DISH " + q) << "\n";
        } else if (choice == 3) {
            std::cout << "\n1. Add item\n2. Remove item\n3. UNDO last action (Stack Pop)\n4. View cart\nChoose: ";
            int sub = 0;
            std::cin >> sub;
            if (sub == 1) {
                int id, qty;
                std::cout << "Item ID: "; std::cin >> id;
                std::cout << "Quantity: "; std::cin >> qty;
                std::cout << handleCommand("CART_ADD " + std::to_string(id) + " " + std::to_string(qty)) << "\n";
            } else if (sub == 2) {
                int id;
                std::cout << "Item ID: "; std::cin >> id;
                std::cout << handleCommand("CART_REMOVE " + std::to_string(id)) << "\n";
            } else if (sub == 3) {
                std::cout << handleCommand("CART_UNDO") << "\n";
            } else {
                std::cout << handleCommand("CART_VIEW") << "\n";
            }
        } else if (choice == 4) {
            std::cout << "Enter Order ID or Tracking Code: ";
            std::string code;
            std::getline(std::cin, code);
            std::cout << handleCommand("TRACK_ORDER " + code) << "\n";
        } else if (choice == 5) {
            std::cout << handleCommand("GET_ORDERS") << "\n";
            std::cout << "\n1. Cook Next Order (Queue Dequeue)\n2. Assign Rider\n3. Advance Latest Order\nChoose: ";
            int qsub = 0;
            std::cin >> qsub;
            if (qsub == 1) {
                std::cout << handleCommand("COOK_ORDER") << "\n";
            } else if (qsub == 2) {
                int ordId;
                std::cout << "Order ID: "; std::cin >> ordId;
                std::cout << handleCommand("ASSIGN_RIDER " + std::to_string(ordId)) << "\n";
            } else if (qsub == 3) {
                std::cout << handleCommand("SIMULATE_NEXT_STAGE") << "\n";
            }
        } else if (choice == 6) {
            std::cout << handleCommand("GET_SALES_MATRIX") << "\n";
        } else if (choice == 7) {
            std::cout << handleCommand("GET_ARRAY_STATS") << "\n";
        } else if (choice == 8) {
            std::cout << handleCommand("BENCHMARK") << "\n";
        } else if (choice == 9) {
            std::cout << handleCommand("COMPARE_DS 50000") << "\n";
        } else if (choice == 10) {
            std::cout << handleCommand("INSPECT_ENGINE") << "\n";
        }
    }
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    initializeSeedData(gRestaurants, gRestaurantCount, gMenuItems, gMenuItemCount, gRiders, gRiderCount);

    for (int i = 0; i < gRiderCount; ++i) {
        gAvailableRiderQueue.enqueue(gRiders[i].id);
    }

    for (int i = 0; i < gMenuItemCount; ++i) {
        gSTLManager.addMenuItem(gMenuItems[i]);
    }

    if (argc > 1 && (std::string(argv[1]) == "--cli" || std::string(argv[1]) == "-i" || std::string(argv[1]) == "menu")) {
        runInteractiveCLI();
        return 0;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line == "EXIT" || line == "QUIT") break;
        if (line == "MENU" || line == "CLI") {
            runInteractiveCLI();
            continue;
        }

        std::string response = handleCommand(line);
        std::cout << response << "\n" << std::flush;
    }

    return 0;
}
