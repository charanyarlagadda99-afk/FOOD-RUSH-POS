// ============================================================================
// FoodRush - High-Performance C++ Core Engine
// Full Implementation of CS Syllabus Modules I-X
// + 3 Enterprise Backend Systems:
//   1. Smart Fleet Dispatch & Zone Routing (Nearest Available Rider Allocation)
//   2. Real-Time Inventory & Stock Lock Engine (Atomic Reservation & Restock)
//   3. Customer Rating System & Top-K Ranking Engine (Leaderboard via Partial Sort)
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

// [MODULE X] STL Manager for Maps, Sets, Vectors, Deques, and Stacks/Queues
STLManager gSTLManager;

// Active Shopping Cart State
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

// [MODULE II] Function Overloading: formatCurrency with integer paise
inline std::string formatCurrency(int paise) {
    // [MODULE I] Type conversion: static_cast from int paise to double rupees
    double rupees = static_cast<double>(paise) / 100.0;
    return formatCurrency(rupees);
}

const MenuItem* findMenuItemById(int itemId) {
    for (int i = 0; i < gMenuItemCount; ++i) {
        if (gMenuItems[i].id == itemId) return &gMenuItems[i];
    }
    return nullptr;
}

MenuItem* findMenuItemByIdMutable(int itemId) {
    for (int i = 0; i < gMenuItemCount; ++i) {
        if (gMenuItems[i].id == itemId) return &gMenuItems[i];
    }
    return nullptr;
}

const Restaurant* findRestaurantById(int restId) {
    for (int i = 0; i < gRestaurantCount; ++i) {
        if (gRestaurants[i].id == restId) return &gRestaurants[i];
    }
    return nullptr;
}

Order* findOrderById(int orderId) {
    for (int i = 0; i < gOrderCount; ++i) {
        if (gOrders[i].orderId == orderId) return &gOrders[i];
    }
    return nullptr;
}

int getOrderQueuePosition(int orderId) {
    for (int i = 0; i < gKitchenQueue.size(); ++i) {
        int qId = 0;
        if (gKitchenQueue.getAtOffset(i, qId) && qId == orderId) {
            return i + 1; // 1-indexed queue position
        }
    }
    return 0;
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
        if (r) restaurantZoneId = r->zoneId;
    }

    if (gCartCount == 0) {
        outTax = 0.0; outDeliveryFee = 0.0; outDiscount = 0.0; outTotal = 0.0;
        return;
    }

    outTax = outSubtotal * TAX_RATE;
    int estimatedMins = 0;
    SalesMatrixManager::calculateDelivery(restaurantZoneId, deliveryZoneId, isExpress, outDeliveryFee, estimatedMins);

    outDiscount = 0.0;
    if (gActiveDiscountPercent > 0.0) {
        outDiscount += (outSubtotal * (gActiveDiscountPercent / 100.0));
    }
    outDiscount += gActiveDiscountFlat;
    if (gActiveCouponCode.find("FREEDEL") != std::string::npos) {
        outDeliveryFee = 0.0;
    }
    if (outDiscount > outSubtotal) outDiscount = outSubtotal;

    outTotal = (outSubtotal - outDiscount) + outTax + outDeliveryFee;
    if (outTotal < 0.0) outTotal = 0.0;
}

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
// [MODULE VIII] CART ACTIONS & [FEATURE 2] INVENTORY LOCK ENGINE
// ============================================================================

bool addToCartInternal(int itemId, int quantity, bool recordUndo = true) {
    if (quantity <= 0) return false;
    MenuItem* item = findMenuItemByIdMutable(itemId);
    if (!item) return false;

    // [FEATURE 2: REAL-TIME INVENTORY LOCK]
    // Check available unreserved stock
    if ((item->stock - item->reservedStock) < quantity) {
        return false; // Out of stock or inventory already locked by active carts
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

    // Atomically lock inventory for the session
    item->reservedStock += quantity;

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
    MenuItem* item = findMenuItemByIdMutable(itemId);
    if (item) {
        // [FEATURE 2: REAL-TIME INVENTORY RELEASE]
        item->reservedStock -= itemBeingRemoved.quantity;
        if (item->reservedStock < 0) item->reservedStock = 0;
    }

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

    for (int i = foundIdx; i < gCartCount - 1; ++i) {
        gCartItems[i] = gCartItems[i + 1];
    }
    gCartCount--;
    return true;
}

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
                    int diff = gCartItems[i].quantity - action.previousQuantity;
                    gCartItems[i].quantity = action.previousQuantity;
                    MenuItem* m = findMenuItemByIdMutable(action.itemId);
                    if (m) {
                        m->reservedStock -= diff;
                        if (m->reservedStock < 0) m->reservedStock = 0;
                    }
                    break;
                }
            }
            ss << "Updated quantity of '" << action.itemName << "' to " << action.previousQuantity;
        }
    } else if (action.type == ACTION_UPDATE_QTY) {
        for (int i = 0; i < gCartCount; ++i) {
            if (gCartItems[i].itemId == action.itemId) {
                int diff = gCartItems[i].quantity - action.previousQuantity;
                gCartItems[i].quantity = action.previousQuantity;
                MenuItem* m = findMenuItemByIdMutable(action.itemId);
                if (m) {
                    m->reservedStock -= diff;
                    if (m->reservedStock < 0) m->reservedStock = 0;
                }
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
// [FEATURE 1: SMART FLEET DISPATCH & ZONE ROUTING ENGINE]
// ============================================================================

int dispatchNearestRider(int orderId) {
    Order* order = findOrderById(orderId);
    if (!order) return -1;
    const Restaurant* rest = findRestaurantById(order->restaurantId);
    if (!rest) return -1;

    double nearestDist = 999.0;
    int riderIdx = findNearestAvailableRider(rest->zoneId, gRiders, gRiderCount, ZONE_DISTANCE_MATRIX, nearestDist);
    if (riderIdx == -1) return -1; // No riders available

    Rider& rider = gRiders[riderIdx];
    rider.isAvailable = false;
    rider.activeOrderId = orderId;
    order->assignedRiderId = rider.id;
    order->riderName = rider.name + " (" + rider.vehicle + ")";
    order->dispatchDistance = nearestDist;
    order->status = "OUT_FOR_DELIVERY";

    // Dynamic ETA: kitchen prep + (distance / speed multiplier * 3.5 min/km)
    double speedMult = (rider.speedMultiplier > 0.1 ? rider.speedMultiplier : 1.0);
    order->estimatedMinutes = (order->isExpress ? 8 : 15) + static_cast<int>(std::round(nearestDist * 3.5 / speedMult));
    return rider.id;
}

// ============================================================================
// [MODULE II] COMMAND DISPATCHER & PROTOCOL HANDLER
// ============================================================================

std::string handleCommand(const std::string& line) {
    auto tokens = tokenizeString(line, ' ');
    if (tokens.empty()) {
        return makeResponse(false, "Empty command", "{}", {"Module II (Control)"});
    }

    std::string cmd = tokens[0];

    // PING
    if (cmd == "PING") {
        return makeResponse(true, "FoodRush Engine is alive and ready", "{\"status\":\"OK\"}", {
            "Module I (Basics)", "Module II (Control)"
        });
    }

    // GET_RESTAURANTS
    if (cmd == "GET_RESTAURANTS") {
        std::ostringstream ss;
        ss << "[";
        for (int i = 0; i < gRestaurantCount; ++i) {
            ss << gRestaurants[i].toJSON();
            if (i < gRestaurantCount - 1) ss << ",";
        }
        ss << "]";
        return makeResponse(true, "Fetched all partner restaurants", ss.str(), {
            "Module VI (Structures - Restaurant)",
            "Module III (1D Arrays - restaurants[])"
        });
    }

    // GET_MENU [restaurantId]
    if (cmd == "GET_MENU") {
        int targetRestId = 0;
        if (tokens.size() > 1) {
            targetRestId = std::stoi(tokens[1]);
        }

        std::ostringstream ss;
        ss << "[";
        bool first = true;
        for (int i = 0; i < gMenuItemCount; ++i) {
            if (targetRestId == 0 || gMenuItems[i].restaurantId == targetRestId) {
                if (!first) ss << ",";
                ss << gMenuItems[i].toJSON();
                first = false;
            }
        }
        ss << "]";
        return makeResponse(true, "Fetched catalog items", ss.str(), {
            "Module VI (Structures - MenuItem)",
            "Module III (1D Arrays - menuItems[])",
            "Module I (Basics - Bitwise dietary flags)"
        });
    }

    // [MODULE V] SEARCH_DISH <query>
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

        // [MODULE V] Levenshtein fuzzy suggestion
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

    // [FEATURE 3] GET_TOP_DISHES [k]
    if (cmd == "GET_TOP_DISHES") {
        int k = 5;
        if (tokens.size() > 1) {
            k = std::stoi(tokens[1]);
        }
        std::vector<MenuItem> topList = getTopKDishes(gMenuItems, gMenuItemCount, k);
        std::ostringstream ss;
        ss << "{\"topK\":" << topList.size() << ",\"dishes\":[";
        for (size_t i = 0; i < topList.size(); ++i) {
            ss << topList[i].toJSON();
            if (i < topList.size() - 1) ss << ",";
        }
        ss << "]}";
        return makeResponse(true, "Top-rated dishes leaderboard", ss.str(), {
            "Module VII (Performance - O(N log K) Sorting / Ranking)",
            "Feature 3 (Customer Rating System & Top-K Ranking)"
        });
    }

    // [FEATURE 3] RATE_DISH <dishId> <rating>
    if (cmd == "RATE_DISH") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: RATE_DISH <dishId> <rating_1_to_5>", "{}", {"Module II (Control)"});
        }
        int dishId = std::stoi(tokens[1]);
        double score = std::stod(tokens[2]);
        if (score < 1.0 || score > 5.0) {
            return makeResponse(false, "Rating must be between 1.0 and 5.0", "{}", {"Module II (Control)"});
        }
        MenuItem* item = findMenuItemByIdMutable(dishId);
        if (!item) return makeResponse(false, "Dish not found", "{}", {"Module II (Control)"});

        // Moving average recalculation
        item->rating = ((item->rating * item->ratingCount) + score) / (item->ratingCount + 1);
        item->ratingCount++;

        return makeResponse(true, "Rating submitted for '" + item->name + "'", item->toJSON(), {
            "Module I (Basics - Floating Point Calculation)",
            "Feature 3 (Customer Rating System & Top-K Ranking)"
        });
    }

    // [FEATURE 2] RESTOCK_ITEM <itemId> <quantity>
    if (cmd == "RESTOCK_ITEM") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: RESTOCK_ITEM <itemId> <quantity>", "{}", {"Module II (Control)"});
        }
        int itemId = std::stoi(tokens[1]);
        int qty = std::stoi(tokens[2]);
        if (qty <= 0) return makeResponse(false, "Quantity must be positive", "{}", {"Module II (Control)"});
        MenuItem* item = findMenuItemByIdMutable(itemId);
        if (!item) return makeResponse(false, "Dish not found", "{}", {"Module II (Control)"});
        item->stock += qty;
        return makeResponse(true, "Restocked '" + item->name + "' by " + std::to_string(qty) + " units", item->toJSON(), {
            "Module III (1D Arrays - stock update)",
            "Feature 2 (Real-Time Stock Lock & Inventory Reservation)"
        });
    }

    // CART_ADD <itemId> <quantity>
    if (cmd == "CART_ADD") {
        if (tokens.size() < 3) {
            return makeResponse(false, "Usage: CART_ADD <itemId> <quantity>", "{}", {"Module II (Control)"});
        }
        int itemId = std::stoi(tokens[1]);
        int quantity = std::stoi(tokens[2]);

        bool ok = addToCartInternal(itemId, quantity, true);
        if (!ok) {
            return makeResponse(false, "Could not add item: out of stock or inventory locked.", "{}", {
                "Module VIII (Stack)", "Feature 2 (Real-Time Inventory Lock)"
            });
        }

        return makeResponse(true, "Item added to your order", cartToJSON(), {
            "Module VIII (Stack - push action for undo)",
            "Module VI (Structures - CartItem)",
            "Feature 2 (Real-Time Inventory Lock)"
        });
    }

    // CART_REMOVE <itemId>
    if (cmd == "CART_REMOVE") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: CART_REMOVE <itemId>", "{}", {"Module II (Control)"});
        }
        int itemId = std::stoi(tokens[1]);
        bool ok = removeFromCartInternal(itemId, true);
        if (!ok) {
            return makeResponse(false, "Item not found in your order", "{}", {"Module VIII (Stack)"});
        }

        return makeResponse(true, "Item removed from order", cartToJSON(), {
            "Module VIII (Stack - push remove action)",
            "Module VI (Structures - CartItem)",
            "Feature 2 (Real-Time Inventory Release)"
        });
    }

    // CART_UNDO
    if (cmd == "CART_UNDO") {
        std::string desc = "";
        bool ok = performCartUndo(desc);
        if (!ok) {
            return makeResponse(false, desc, cartToJSON(), {"Module VIII (Stack - empty check)"});
        }

        return makeResponse(true, "Undid last action: " + desc, cartToJSON(), {
            "Module VIII (Stack - pop & reverse action)",
            "Module VI (Structures - CartAction)",
            "Feature 2 (Real-Time Inventory Lock Sync)"
        });
    }

    // CART_VIEW [deliveryZoneId] [isExpress]
    if (cmd == "CART_VIEW") {
        int zone = 0;
        bool express = false;
        if (tokens.size() > 1) zone = std::stoi(tokens[1]);
        if (tokens.size() > 2) express = (tokens[2] == "1" || tokens[2] == "true");

        return makeResponse(true, "Current order breakdown", cartToJSON(zone, express), {
            "Module I (Basics - tax, subtotal, delivery calculation)",
            "Module IV (2D Arrays - zone distance lookup)",
            "Module VI (Structures - CartItem)"
        });
    }

    // CART_CLEAR
    if (cmd == "CART_CLEAR") {
        // Release all reserved stock
        for (int i = 0; i < gCartCount; ++i) {
            MenuItem* m = findMenuItemByIdMutable(gCartItems[i].itemId);
            if (m) {
                m->reservedStock -= gCartItems[i].quantity;
                if (m->reservedStock < 0) m->reservedStock = 0;
            }
        }
        gCartCount = 0;
        gUndoStack.clear();
        gActiveCouponCode = "";
        gActiveDiscountPercent = 0.0;
        gActiveDiscountFlat = 0.0;

        return makeResponse(true, "Order cleared", cartToJSON(), {
            "Module VIII (Stack - clear)",
            "Feature 2 (Real-Time Inventory Release)"
        });
    }

    // APPLY_COUPON <code>
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

    // CHECKOUT <name> <zoneId> <street> [isExpress]
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
            if (m) {
                m->stock -= gCartItems[i].quantity;
                m->reservedStock -= gCartItems[i].quantity;
                if (m->reservedStock < 0) m->reservedStock = 0;
                if (m->stock < 0) m->stock = 0;
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
        newOrder.dispatchDistance = 0.0;

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
            "Feature 2 (Real-Time Stock Lock & Stock Deduction)"
        });
    }

    // TRACK_ORDER <orderIdOrCode>
    if (cmd == "TRACK_ORDER") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: TRACK_ORDER <orderIdOrCode>", "{}", {"Module II (Control)"});
        }
        std::string query = tokens[1];
        Order* target = nullptr;

        try {
            int id = std::stoi(query);
            target = findOrderById(id);
        } catch (...) {
            for (int i = 0; i < gOrderCount; ++i) {
                if (gOrders[i].trackingCode == query) {
                    target = &gOrders[i];
                    break;
                }
            }
        }

        if (!target) {
            if (gOrderCount > 0) target = &gOrders[gOrderCount - 1];
            else return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
        }

        target->queuePosition = getOrderQueuePosition(target->orderId);
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

    // SIMULATE_NEXT_STAGE [orderId]
    if (cmd == "SIMULATE_NEXT_STAGE") {
        Order* target = nullptr;
        if (tokens.size() > 1) {
            int ordId = std::stoi(tokens[1]);
            target = findOrderById(ordId);
        } else if (gOrderCount > 0) {
            target = &gOrders[gOrderCount - 1];
        }

        if (!target) {
            return makeResponse(false, "No active order to advance", "{}", {"Module II (Control)"});
        }

        std::string prevStatus = target->status;
        std::string newStatus = prevStatus;

        if (prevStatus == "PLACED") {
            target->status = "PREPARING";
            target->riderName = "Chef is preparing your meal";
            newStatus = "PREPARING";
        } else if (prevStatus == "PREPARING") {
            // Kitchen finishes: dequeue and dispatch nearest rider
            int dummyId = 0;
            bool wasExpress = false;
            gPriorityOrderQueue.dequeue(dummyId, wasExpress);
            gKitchenQueue.dequeue(dummyId);
            gSTLManager.dequeueOrder(dummyId);

            // [FEATURE 1: SMART FLEET DISPATCH] Greedily assign nearest available rider
            int riderId = dispatchNearestRider(target->orderId);
            if (riderId == -1) {
                target->riderName = "Awaiting available partner";
            }
            newStatus = "OUT_FOR_DELIVERY";
        } else if (prevStatus == "OUT_FOR_DELIVERY") {
            target->status = "DELIVERED";
            target->estimatedMinutes = 0;

            if (target->assignedRiderId != -1) {
                for (int i = 0; i < gRiderCount; ++i) {
                    if (gRiders[i].id == target->assignedRiderId) {
                        gRiders[i].isAvailable = true;
                        gRiders[i].activeOrderId = -1;
                        gRiders[i].totalDeliveries++;
                        gAvailableRiderQueue.enqueue(gRiders[i].id);
                        break;
                    }
                }
            }
            gSTLManager.recordCompletedOrder(target->orderId);
            newStatus = "DELIVERED";
        }

        target->queuePosition = getOrderQueuePosition(target->orderId);

        std::vector<std::string> tags = {
            "Module IX (Queue - Circular Queue dequeue & Rider rotation)",
            "Module VI (Structures - Order state transition)",
            "Feature 1 (Smart Fleet Dispatch & Zone Routing)"
        };
        if (newStatus == "DELIVERED") {
            tags.push_back("Module X (STL - std::deque completed orders)");
        }

        return makeResponse(true, "Order #" + std::to_string(target->orderId) + " transitioned to: " + newStatus, target->toJSON(), tags);
    }

    // GET_ORDERS
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

    // COOK_ORDER [orderId]
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

    // [FEATURE 1] ASSIGN_RIDER / DISPATCH_ORDER <orderId>
    if (cmd == "ASSIGN_RIDER" || cmd == "DISPATCH_ORDER") {
        if (tokens.size() < 2) {
            return makeResponse(false, "Usage: ASSIGN_RIDER <orderId>", "{}", {"Module II (Control)"});
        }
        int orderId = std::stoi(tokens[1]);
        Order* targetOrder = findOrderById(orderId);
        if (!targetOrder) {
            return makeResponse(false, "Order not found", "{}", {"Module II (Control)"});
        }

        int riderId = dispatchNearestRider(orderId);
        if (riderId == -1) {
            return makeResponse(false, "All delivery partners are currently busy", "{}", {
                "Module IX (Queue)", "Feature 1 (Smart Fleet Dispatch)"
            });
        }

        return makeResponse(true, "Delivery partner assigned via nearest-zone dispatch: " + targetOrder->riderName, targetOrder->toJSON(), {
            "Feature 1 (Smart Fleet Dispatch & Zone Routing)",
            "Module IV (2D Arrays - 5x5 Zone Distance Matrix)",
            "Module VI (Structures - Rider & Order linkage)"
        });
    }

    // COMPLETE_ORDER <orderId>
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
                    gRiders[i].activeOrderId = -1;
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
            "Feature 1 (Smart Fleet Dispatch - Rider Free)"
        });
    }

    // [FEATURE 1] GET_FLEET_STATUS / GET_RIDERS
    if (cmd == "GET_FLEET_STATUS" || cmd == "GET_RIDERS") {
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
            "Feature 1 (Smart Fleet Dispatch & Zone Routing)"
        });
    }

    // [MODULE IV] GET_SALES_MATRIX
    if (cmd == "GET_SALES_MATRIX") {
        return makeResponse(true, "Weekly sales matrix and metrics", gSalesMatrix.toJSON(), {
            "Module IV (2D Arrays - sales[6][7] row/col sums)",
            "Module IV (2D Arrays - zone distance matrix[5][5])"
        });
    }

    // [MODULE III] GET_ARRAY_STATS
    if (cmd == "GET_ARRAY_STATS") {
        double ratings[MAX_RESTAURANTS];
        for (int i = 0; i < gRestaurantCount; ++i) ratings[i] = gRestaurants[i].rating;

        double prices[MAX_TOTAL_ITEMS];
        int stocks[MAX_TOTAL_ITEMS];
        for (int i = 0; i < gMenuItemCount; ++i) {
            prices[i] = gMenuItems[i].price;
            stocks[i] = gMenuItems[i].stock;
        }

        double sumPrices = calculateArraySum(prices, gMenuItemCount);
        double avgPrice = gMenuItemCount > 0 ? (sumPrices / gMenuItemCount) : 0.0;

        int maxRatingIdx = -1;
        double maxRating = findArrayMax(ratings, gRestaurantCount, maxRatingIdx);

        int minPriceIdx = -1;
        double minPrice = findArrayMin(prices, gMenuItemCount, minPriceIdx);

        int maxPriceIdx = -1;
        double maxPrice = findArrayMax(prices, gMenuItemCount, maxPriceIdx);

        double sortedPrices[MAX_TOTAL_ITEMS];
        std::memcpy(sortedPrices, prices, sizeof(double) * gMenuItemCount);
        bubbleSortArray(sortedPrices, gMenuItemCount);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"ratings\":{"
           << "\"count\":" << gRestaurantCount << ","
           << "\"maxRating\":" << maxRating << ","
           << "\"bestRestaurantId\":" << (maxRatingIdx != -1 ? gRestaurants[maxRatingIdx].id : 0)
           << "},"
           << "\"prices\":{"
           << "\"count\":" << gMenuItemCount << ","
           << "\"sumTotal\":" << sumPrices << ","
           << "\"average\":" << avgPrice << ","
           << "\"minPrice\":" << minPrice << ","
           << "\"cheapestItemId\":" << (minPriceIdx != -1 ? gMenuItems[minPriceIdx].id : 0) << ","
           << "\"maxPrice\":" << maxPrice << ","
           << "\"bubbleSortedSample\":[";
        int sampleSize = std::min(8, gMenuItemCount);
        for (int i = 0; i < sampleSize; ++i) {
            ss << sortedPrices[i];
            if (i < sampleSize - 1) ss << ",";
        }
        ss << "]"
           << "}"
           << "}";

        return makeResponse(true, "1D Array statistics and bubble sort analysis", ss.str(), {
            "Module III (1D Arrays - Sum, Min, Max, Traversal)",
            "Module III (1D Arrays - Bubble Sort on prices)",
            "Module I (Basics - floating point division)"
        });
    }

    // [MODULE VII] BENCHMARK
    if (cmd == "BENCHMARK") {
        PerformanceBenchmark bench;
        return makeResponse(true, "Performance benchmark completed", bench.runFullBenchmark(), {
            "Module VII (Performance - O(N) vs O(log N) Search)",
            "Module VII (Performance - O(N^2) vs O(N log N) Sort)",
            "Module VII (Performance - Time/Space Asymptotic Complexity)"
        });
    }

    // [MODULE X] COMPARE_DS [operations]
    if (cmd == "COMPARE_DS") {
        int operations = 10000;
        if (tokens.size() > 1) operations = std::stoi(tokens[1]);

        ArrayStack<int, 10000> customStack;
        std::stack<int> stlStack;

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < operations; ++i) customStack.push(i);
        for (int i = 0; i < operations; ++i) { int val; customStack.pop(val); }
        auto t2 = std::chrono::high_resolution_clock::now();
        double customStackUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < operations; ++i) stlStack.push(i);
        for (int i = 0; i < operations; ++i) stlStack.pop();
        t2 = std::chrono::high_resolution_clock::now();
        double stlStackUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        CircularQueue<int, 10000> customQueue;
        std::queue<int> stlQueue;

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < operations; ++i) customQueue.enqueue(i);
        for (int i = 0; i < operations; ++i) { int val; customQueue.dequeue(val); }
        t2 = std::chrono::high_resolution_clock::now();
        double customQueueUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < operations; ++i) stlQueue.push(i);
        for (int i = 0; i < operations; ++i) stlQueue.pop();
        t2 = std::chrono::high_resolution_clock::now();
        double stlQueueUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"operations\":" << operations << ","
           << "\"stackComparison\":{"
           << "\"customArrayStackUs\":" << customStackUs << ","
           << "\"stlStackUs\":" << stlStackUs << ","
           << "\"winner\":\"" << (customStackUs < stlStackUs ? "ArrayStack (Contiguous Memory)" : "std::stack") << "\""
           << "},"
           << "\"queueComparison\":{"
           << "\"customCircularQueueUs\":" << customQueueUs << ","
           << "\"stlQueueUs\":" << stlQueueUs << ","
           << "\"winner\":\"" << (customQueueUs < stlQueueUs ? "CircularQueue (Zero Heap Overhead)" : "std::queue") << "\""
           << "}"
           << "}";

        return makeResponse(true, "Data Structure comparison completed", ss.str(), {
            "Module X (STL - std::stack, std::queue, std::deque)",
            "Module VIII (Stack - ArrayStack direct comparison)",
            "Module IX (Queue - CircularQueue direct comparison)",
            "Module VII (Performance - Microsecond stopwatch benchmarking)"
        });
    }

    // INSPECT_ENGINE
    if (cmd == "INSPECT_ENGINE") {
        std::ostringstream ss;
        ss << "{"
           << "\"undoStack\":{"
           << "\"capacity\":" << 50 << ","
           << "\"size\":" << gUndoStack.size() << ","
           << "\"topIndex\":" << gUndoStack.getTopIndex() << ","
           << "\"isEmpty\":" << (gUndoStack.isEmpty() ? "true" : "false") << ","
           << "\"isFull\":" << (gUndoStack.isFull() ? "true" : "false") << ","
           << "\"recentActions\":[";
        int stackViewCount = std::min(5, gUndoStack.size());
        for (int i = 0; i < stackViewCount; ++i) {
            CartAction a;
            if (gUndoStack.getAtOffset(i, a)) {
                ss << a.toJSON();
                if (i < stackViewCount - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"kitchenQueue\":{"
           << "\"capacity\":" << 50 << ","
           << "\"size\":" << gKitchenQueue.size() << ","
           << "\"frontIndex\":" << gKitchenQueue.getFront() << ","
           << "\"rearIndex\":" << gKitchenQueue.getRear() << ","
           << "\"isEmpty\":" << (gKitchenQueue.isEmpty() ? "true" : "false") << ","
           << "\"isFull\":" << (gKitchenQueue.isFull() ? "true" : "false") << ","
           << "\"waitingOrderIds\":[";
        for (int i = 0; i < gKitchenQueue.size(); ++i) {
            int qId = 0;
            if (gKitchenQueue.getAtOffset(i, qId)) {
                ss << qId;
                if (i < gKitchenQueue.size() - 1) ss << ",";
            }
        }
        ss << "]"
           << "},"
           << "\"stlContainers\":{"
           << "\"vectorMenuSize\":" << gSTLManager.getVectorSize() << ","
           << "\"setUniqueCuisines\":" << gSTLManager.getCuisinesCount() << ","
           << "\"mapCouponsCount\":" << gSTLManager.getCouponsCount() << ","
           << "\"dequeCompletedOrders\":" << gSTLManager.getCompletedCount()
           << "}"
           << "}";

        return makeResponse(true, "Engine internal memory inspected", ss.str(), {
            "Module VIII (Stack - Raw array memory layout inspection)",
            "Module IX (Queue - Circular buffer front/rear pointers)",
            "Module X (STL - Map/Set/Pair inspection)"
        });
    }

    // HELP
    if (cmd == "HELP") {
        return makeResponse(true, "Available Commands: PING, GET_RESTAURANTS, GET_MENU [id], SEARCH_DISH <query>, GET_TOP_DISHES [k], RATE_DISH <id> <score>, RESTOCK_ITEM <id> <qty>, CART_ADD <id> <qty>, CART_REMOVE <id>, CART_UNDO, CART_VIEW, CART_CLEAR, APPLY_COUPON <code>, CHECKOUT <name> <zoneId> <street> [express], TRACK_ORDER <idOrCode>, SIMULATE_NEXT_STAGE [id], GET_ORDERS, COOK_ORDER [id], ASSIGN_RIDER <id>, COMPLETE_ORDER <id>, GET_FLEET_STATUS, GET_SALES_MATRIX, GET_ARRAY_STATS, BENCHMARK, COMPARE_DS, INSPECT_ENGINE, HELP", "{}", {"Module II (Control)"});
    }

    return makeResponse(false, "Unknown command: " + cmd + ". Type HELP for list.", "{}", {"Module II (Control)"});
}

// ============================================================================
// INTERACTIVE CLI CONSOLE MENU (Terminal Examiner Mode)
// ============================================================================
void runInteractiveCLI() {
    std::cout << "\n======================================================\n";
    std::cout << "         FOODRUSH - C++ CORE BACKEND ENGINE           \n";
    std::cout << "         5-Person Capstone & Viva Demonstration       \n";
    std::cout << "======================================================\n";

    while (true) {
        std::cout << "\n--- MAIN CONSOLE MENU ---\n";
        std::cout << "1. Browse Restaurants & Menu      [Modules III, VI]\n";
        std::cout << "2. Search Dishes with Fuzzy Match [Module V]\n";
        std::cout << "3. Manage Cart & Test Stack Undo  [Module VIII, Feature 2]\n";
        std::cout << "4. Track Order & Verify Reversal  [Module V, IX]\n";
        std::cout << "5. Cook Order (Circular Queue)    [Module IX]\n";
        std::cout << "6. Smart Fleet Dispatch & Routing [Module IV, Feature 1]\n";
        std::cout << "7. View 6x7 Sales Revenue Matrix  [Module IV]\n";
        std::cout << "8. Top-Rated Leaderboard & Rate   [Module VII, Feature 3]\n";
        std::cout << "9. Algorithmic Benchmarks & DS    [Module VII, X]\n";
        std::cout << "10. Inspect Engine Memory (DS)    [Module VIII, IX, X]\n";
        std::cout << "0. Exit System\n";
        std::cout << "Choose option (0-10): ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::string discard;
            std::getline(std::cin, discard);
            continue;
        }
        std::string dummy;
        std::getline(std::cin, dummy);

        if (choice == 0) break;
        if (choice == 1) {
            std::cout << handleCommand("GET_RESTAURANTS") << "\n";
            std::cout << "Enter Restaurant ID for menu (or 0 for all): ";
            int id = 0; std::cin >> id;
            std::cout << handleCommand("GET_MENU " + std::to_string(id)) << "\n";
        } else if (choice == 2) {
            std::cout << "Enter search query: ";
            std::string q; std::getline(std::cin, q);
            std::cout << handleCommand("SEARCH_DISH " + q) << "\n";
        } else if (choice == 3) {
            std::cout << "\n1. Add item\n2. Remove item\n3. UNDO last action (Stack Pop)\n4. View cart\n5. Clear cart\nChoose: ";
            int sub = 0; std::cin >> sub;
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
            } else if (sub == 4) {
                std::cout << handleCommand("CART_VIEW") << "\n";
            } else {
                std::cout << handleCommand("CART_CLEAR") << "\n";
            }
        } else if (choice == 4) {
            std::cout << "Enter Order ID or Tracking Code: ";
            std::string code; std::getline(std::cin, code);
            std::cout << handleCommand("TRACK_ORDER " + code) << "\n";
        } else if (choice == 5) {
            std::cout << handleCommand("COOK_ORDER") << "\n";
        } else if (choice == 6) {
            std::cout << "\n1. View Fleet Status\n2. Dispatch Nearest Rider to Order\nChoose: ";
            int sub = 0; std::cin >> sub;
            if (sub == 1) {
                std::cout << handleCommand("GET_FLEET_STATUS") << "\n";
            } else {
                std::cout << "Enter Order ID: ";
                int ordId; std::cin >> ordId;
                std::cout << handleCommand("ASSIGN_RIDER " + std::to_string(ordId)) << "\n";
            }
        } else if (choice == 7) {
            std::cout << handleCommand("GET_SALES_MATRIX") << "\n";
        } else if (choice == 8) {
            std::cout << "\n1. View Top-K Rated Dishes\n2. Rate a Dish\nChoose: ";
            int sub = 0; std::cin >> sub;
            if (sub == 1) {
                std::cout << "How many top dishes (K)? ";
                int k; std::cin >> k;
                std::cout << handleCommand("GET_TOP_DISHES " + std::to_string(k)) << "\n";
            } else {
                int dId; double rating;
                std::cout << "Dish ID: "; std::cin >> dId;
                std::cout << "Rating (1.0 - 5.0): "; std::cin >> rating;
                std::cout << handleCommand("RATE_DISH " + std::to_string(dId) + " " + std::to_string(rating)) << "\n";
            }
        } else if (choice == 9) {
            std::cout << handleCommand("BENCHMARK") << "\n";
            std::cout << handleCommand("COMPARE_DS 10000") << "\n";
        } else if (choice == 10) {
            std::cout << handleCommand("INSPECT_ENGINE") << "\n";
        }
    }
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char* argv[]) {
    // [MODULE I] Console I/O optimization
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // Initialize all seed data into fixed memory arrays
    initializeSeedData(gRestaurants, gRestaurantCount, gMenuItems, gMenuItemCount, gRiders, gRiderCount);

    // Pre-populate STL structures for Module X
    for (int i = 0; i < gMenuItemCount; ++i) {
        gSTLManager.addMenuItem(gMenuItems[i]);
    }
    for (int i = 0; i < gRestaurantCount; ++i) {
        gSTLManager.addCuisine(gRestaurants[i].cuisine);
    }
    for (int i = 0; i < gRiderCount; ++i) {
        gAvailableRiderQueue.enqueue(gRiders[i].id);
    }

    // CLI mode check
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--cli") == 0 || std::strcmp(argv[i], "-c") == 0) {
            runInteractiveCLI();
            return 0;
        }
    }

    // Standard I/O Bridge mode: reads 1 command line -> writes 1 JSON line
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line == "EXIT" || line == "QUIT") break;
        std::string response = handleCommand(line);
        std::cout << response << "\n" << std::flush;
    }

    return 0;
}
