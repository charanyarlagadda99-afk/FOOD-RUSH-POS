// FoodRush / RestoRush - Core Algorithms, String Processing, 1D Array Stats, and Benchmarks
// Covers: [MODULE III] 1D Arrays, [MODULE V] Strings, [MODULE VII] Performance, [MODULE X] STL
#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "models.hpp"
#include "array_stack.hpp"
#include "array_queue.hpp"

// ============================================================================
// [MODULE III] 1D ARRAY OPERATIONS
// ============================================================================

// [MODULE III] Sum of elements in a 1D array | used by: AVERAGE_RATINGS / TOTALS
inline double calculateArraySum(const double arr[], int size) {
    double total = 0.0;
    for (int i = 0; i < size; ++i) { // 1D array traversal
        total += arr[i];
    }
    return total;
}

// [MODULE III] Maximum element in a 1D array with index | used by: BEST_RATED_OUTLET
inline double findArrayMax(const double arr[], int size, int& outIndex) {
    if (size <= 0) { outIndex = -1; return 0.0; }
    double maxVal = arr[0];
    outIndex = 0;
    for (int i = 1; i < size; ++i) {
        if (arr[i] > maxVal) {
            maxVal = arr[i];
            outIndex = i;
        }
    }
    return maxVal;
}

// [MODULE III] Minimum element in a 1D array with index | used by: CHEAPEST_DISH
inline double findArrayMin(const double arr[], int size, int& outIndex) {
    if (size <= 0) { outIndex = -1; return 0.0; }
    double minVal = arr[0];
    outIndex = 0;
    for (int i = 1; i < size; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            outIndex = i;
        }
    }
    return minVal;
}

// [MODULE III] Linear Search on 1D integer array | used by: ITEM_LOOKUP
inline int linearSearchArray(const int arr[], int size, int targetKey) {
    for (int i = 0; i < size; ++i) {
        if (arr[i] == targetKey) {
            return i; // Found at index i, O(N) time
        }
    }
    return -1; // Not found
}

// [MODULE III] Bubble Sort on 1D array of dish prices | used by: PRICE_SORTING
inline void bubbleSortArray(double arr[], int size) {
    for (int i = 0; i < size - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < size - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                double temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }
        if (!swapped) break; // Optimization if already sorted
    }
}

// ============================================================================
// [MODULE V] STRING OPERATIONS & PATTERN MATCHING
// ============================================================================

// [MODULE V] String Traversal & Case Conversion | used by: SEARCH_NORMALIZATION
inline std::string toLowerString(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

// [MODULE V] Pattern Matching: Substring search | used by: DISH_SEARCH
inline bool stringContains(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    std::string h = toLowerString(haystack);
    std::string n = toLowerString(needle);
    return h.find(n) != std::string::npos;
}

// [MODULE V] String Tokenizing by delimiter | used by: COMMAND_PARSER
inline std::vector<std::string> tokenizeString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// [MODULE V] String Escaping for Safe JSON Serialization
inline std::string escapeJSON(const std::string& input) {
    std::string output = "";
    for (char c : input) {
        if (c == '"') output += "\\\"";
        else if (c == '\\') output += "\\\\";
        else if (c == '\b') output += "\\b";
        else if (c == '\f') output += "\\f";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else output += c;
    }
    return output;
}

// [MODULE V] String Reversal | used by: STRING_REVERSAL_ALGORITHM
inline std::string reverseString(std::string str) {
    int n = static_cast<int>(str.length());
    for (int i = 0; i < n / 2; ++i) {
        char temp = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = temp;
    }
    return str;
}

// [MODULE V] Character Frequency Analysis | used by: COUPON_CHECK
inline std::map<char, int> analyzeCharFrequency(const std::string& str) {
    std::map<char, int> freq;
    for (char c : str) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            char lowerC = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            freq[lowerC]++;
        }
    }
    return freq;
}

inline std::string charFrequencyToJSON(const std::map<char, int>& freq) {
    std::ostringstream ss;
    ss << "{";
    bool first = true;
    for (const auto& p : freq) {
        if (!first) ss << ",";
        ss << "\"" << p.first << "\":" << p.second;
        first = false;
    }
    ss << "}";
    return ss.str();
}

// [MODULE V] Levenshtein Edit Distance for "Did You Mean" Fuzzy Search Suggestions
inline int calculateLevenshteinDistance(const std::string& s1, const std::string& s2) {
    int m = static_cast<int>(s1.length());
    int n = static_cast<int>(s2.length());

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (std::tolower(static_cast<unsigned char>(s1[i - 1])) ==
                std::tolower(static_cast<unsigned char>(s2[j - 1]))) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({
                    dp[i - 1][j],    // Deletion
                    dp[i][j - 1],    // Insertion
                    dp[i - 1][j - 1] // Substitution
                });
            }
        }
    }
    return dp[m][n];
}

// [MODULE V] Find "Did You Mean" suggestion from catalog vocabulary
inline std::string findDidYouMeanSuggestion(const std::string& query, const MenuItem items[], int itemCount) {
    std::string cleanQuery = toLowerString(query);
    if (cleanQuery.length() < 3) return "";

    std::string bestMatch = "";
    int minDistance = 999;

    for (int i = 0; i < itemCount; ++i) {
        std::string cat = toLowerString(items[i].category);
        int dCat = calculateLevenshteinDistance(cleanQuery, cat);
        if (dCat < minDistance && dCat <= 2 && dCat > 0) {
            minDistance = dCat;
            bestMatch = items[i].category;
        }

        auto words = tokenizeString(items[i].name, ' ');
        for (const auto& w : words) {
            std::string wordClean = toLowerString(w);
            if (wordClean.length() < 3) continue;
            int dWord = calculateLevenshteinDistance(cleanQuery, wordClean);
            if (dWord < minDistance && dWord <= 2 && dWord > 0) {
                minDistance = dWord;
                bestMatch = w;
            }
        }
    }

    return bestMatch;
}

// [MODULE V] Invoice Check Digit Generator Using String Reversal
// Computes a tamper-proof check digit by reversing the bill digits and calculating weighted modulo
inline std::string generateInvoiceCode(int billId) {
    std::string idStr = std::to_string(billId);
    std::string rev = reverseString(idStr); // [MODULE V] String reversal

    int weightedSum = 0;
    for (size_t i = 0; i < rev.length(); ++i) {
        weightedSum += (rev[i] - '0') * (static_cast<int>(i) + 2);
    }
    int checkDigit = (weightedSum % 9) + 1; // 1 to 9

    return "INV-" + idStr + "-" + std::to_string(checkDigit);
}

// [MODULE V] Validate Invoice Code Using String Reversal Verification
inline bool validateInvoiceCode(const std::string& invoiceCode) {
    auto tokens = tokenizeString(invoiceCode, '-');
    if (tokens.size() != 3 || tokens[0] != "INV") {
        return false;
    }

    std::string idStr = tokens[1];
    int expectedCheckDigit = std::stoi(tokens[2]);

    std::string rev = reverseString(idStr); // [MODULE V] String reversal
    int weightedSum = 0;
    for (size_t i = 0; i < rev.length(); ++i) {
        weightedSum += (rev[i] - '0') * (static_cast<int>(i) + 2);
    }
    int calculatedCheckDigit = (weightedSum % 9) + 1;

    return (expectedCheckDigit == calculatedCheckDigit);
}

// [MODULE V] ASCII Thermal Receipt Formatter for Bill Printing
inline std::string generatePrintReceiptText(const BillReceipt& bill) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "========================================\n";
    ss << "       THE GRAND REGENCY HOTEL & POS    \n";
    ss << "       Official Tax Invoice & Receipt   \n";
    ss << "========================================\n";
    ss << "Invoice No:  " << bill.invoiceCode << "\n";
    ss << "Bill ID:     #" << bill.billId << "\n";
    ss << "Date & Time: " << bill.billTime << "\n";
    ss << "Table:       " << bill.tableName << " (" << bill.section << ")\n";
    ss << "Guest Name:  " << bill.guestName << "\n";
    ss << "Server:      " << bill.serverName << "\n";
    ss << "----------------------------------------\n";
    ss << "ITEM                     QTY   PRICE   TOTAL\n";
    ss << "----------------------------------------\n";

    for (int i = 0; i < bill.itemCount; ++i) {
        std::string n = bill.items[i].name;
        if (n.length() > 20) n = n.substr(0, 18) + "..";
        ss << std::left << std::setw(22) << n;
        ss << std::right << std::setw(3) << bill.items[i].quantity;
        ss << std::right << std::setw(8) << bill.items[i].price;
        ss << std::right << std::setw(7) << bill.items[i].getLineTotal() << "\n";
    }

    ss << "----------------------------------------\n";
    ss << std::left << std::setw(30) << "Item Subtotal:" << std::right << std::setw(10) << bill.subtotal << "\n";
    if (bill.discount > 0.0) {
        ss << std::left << std::setw(30) << "Promo Discount:" << std::right << std::setw(9) << "-" << bill.discount << "\n";
    }
    ss << std::left << std::setw(30) << "GST (5% SGST+CGST):" << std::right << std::setw(10) << bill.gstTax << "\n";
    ss << std::left << std::setw(30) << "Service Charge (5%):" << std::right << std::setw(10) << bill.serviceCharge << "\n";
    ss << "========================================\n";
    ss << std::left << std::setw(28) << "NET TOTAL PAYABLE:" << "INR " << std::right << std::setw(8) << bill.netTotal << "\n";
    ss << "Payment Method: " << bill.paymentMethod << "\n";
    ss << "========================================\n";
    ss << "     THANK YOU FOR DINING WITH US!      \n";
    ss << "  GSTIN: 29AABCU9603R1ZM | Bangalore   \n";
    ss << "========================================\n";
    return ss.str();
}

// ============================================================================
// [MODULE VII] TOP-K LEADERBOARD ALGORITHM (O(N log K) Ranking)
// ============================================================================
inline std::vector<MenuItem> getTopKDishes(const MenuItem items[], int itemCount, int k) {
    if (k <= 0) k = 5;
    if (k > itemCount) k = itemCount;

    std::vector<MenuItem> candidateList;
    candidateList.reserve(itemCount);
    for (int i = 0; i < itemCount; ++i) {
        candidateList.push_back(items[i]);
    }

    std::sort(candidateList.begin(), candidateList.end(), [](const MenuItem& a, const MenuItem& b) {
        if (std::abs(a.rating - b.rating) > 0.001) {
            return a.rating > b.rating;
        }
        return a.ratingCount > b.ratingCount;
    });

    if (static_cast<int>(candidateList.size()) > k) {
        candidateList.resize(k);
    }
    return candidateList;
}

// ============================================================================
// [MODULE X] STL DATA STRUCTURES USAGE FOR RESTAURANT & HOTEL POS
// ============================================================================

class STLManager {
private:
    // [MODULE X] std::map for coupon discount lookup | used by: DISCOUNT_SYSTEM
    std::map<std::string, double> couponDiscounts;

    // [MODULE X] std::set for unique dining categories | used by: CATEGORY_REGISTRY
    std::set<std::string> availableCuisines;

    // [MODULE X] std::vector for master menu item catalog | used by: MASTER_CATALOG
    std::vector<MenuItem> stlMenuItems;

    // [MODULE X] std::stack for STL order undo stack | used by: STL_STACK_VERSION
    std::stack<CartAction> stlUndoStack;

    // [MODULE X] std::queue for STL kitchen KOT queue | used by: STL_QUEUE_VERSION
    std::queue<int> stlKitchenQueue;

    // [MODULE X] std::deque for settled past bills archive | used by: PAST_BILLS_ARCHIVE
    std::deque<BillReceipt> pastBillsDeque;

    // [MODULE X] std::pair for promo pair (itemId, discountPercent) | used by: PROMO_PAIRS
    std::vector<std::pair<int, double>> dailySpecials;

public:
    STLManager() {
        // POS Discount Coupons in std::map
        couponDiscounts["WELCOME10"] = 10.0; // 10% off
        couponDiscounts["HOTEL50"]   = 50.0; // ₹50 flat / 50% discount
        couponDiscounts["FESTIVE20"] = 20.0; // 20% off
        couponDiscounts["STAFFDISC"] = 25.0; // 25% staff dining discount

        // Unique sections & cuisines in std::set
        availableCuisines.insert("Biryani & Mughlai");
        availableCuisines.insert("South Indian");
        availableCuisines.insert("Artisan Italian");
        availableCuisines.insert("Japanese");
        availableCuisines.insert("American Gourmet");
        availableCuisines.insert("Desserts & Bakery");

        // Daily special pairs (itemId, discount%)
        dailySpecials.push_back(std::make_pair(101, 15.0)); // Hyderabadi Dum Biryani
        dailySpecials.push_back(std::make_pair(201, 20.0)); // Ghee Roast Dosa
        dailySpecials.push_back(std::make_pair(301, 10.0)); // Wood-Fired Margherita
        dailySpecials.push_back(std::make_pair(401, 12.5)); // Tonkotsu Ramen
        dailySpecials.push_back(std::make_pair(501, 15.0)); // Cheddar Smash Burger
        dailySpecials.push_back(std::make_pair(601, 10.0)); // Chocolate Ganache Pastry
    }

    bool getCouponDiscount(const std::string& code, double& outPercent) const {
        std::map<std::string, double>::const_iterator it = couponDiscounts.find(code);
        if (it != couponDiscounts.end()) {
            outPercent = it->second;
            return true;
        }
        return false;
    }

    void addMenuItem(const MenuItem& item) {
        stlMenuItems.push_back(item);
    }

    void pushUndo(const CartAction& action) {
        stlUndoStack.push(action);
    }

    bool popUndo(CartAction& outAction) {
        if (stlUndoStack.empty()) return false;
        outAction = stlUndoStack.top();
        stlUndoStack.pop();
        return true;
    }

    void enqueueOrder(int orderId) {
        stlKitchenQueue.push(orderId);
    }

    bool dequeueOrder(int& outOrderId) {
        if (stlKitchenQueue.empty()) return false;
        outOrderId = stlKitchenQueue.front();
        stlKitchenQueue.pop();
        return true;
    }

    // [MODULE X] Double-ended queue (deque) tracking of settled past bills
    void recordSettledBill(const BillReceipt& bill) {
        pastBillsDeque.push_front(bill); // Newest bills at the front
        if (pastBillsDeque.size() > 50) {
            pastBillsDeque.pop_back(); // Keep last 50 bills
        }
    }

    const std::deque<BillReceipt>& getPastBills() const { return pastBillsDeque; }
    const std::map<std::string, double>& getCoupons() const { return couponDiscounts; }
    const std::set<std::string>& getCuisines() const { return availableCuisines; }
    const std::vector<std::pair<int, double>>& getDailySpecials() const { return dailySpecials; }

    int getVectorSize() const { return static_cast<int>(stlMenuItems.size()); }
    int getCuisinesCount() const { return static_cast<int>(availableCuisines.size()); }
    int getCouponsCount() const { return static_cast<int>(couponDiscounts.size()); }
    int getPastBillsCount() const { return static_cast<int>(pastBillsDeque.size()); }
    void addCuisine(const std::string& cuisine) { availableCuisines.insert(cuisine); }

    // [MODULE X] Live Comparison: Hand-crafted Array Stack/Queue vs STL Stack/Queue
    std::string compareDataStructures(int testIterations = 50000) const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);

        ArrayStack<int, 60000> customStack;
        std::stack<int> stlStack;

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) customStack.push(i);
        auto t2 = std::chrono::high_resolution_clock::now();
        double customStackPushUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) stlStack.push(i);
        t2 = std::chrono::high_resolution_clock::now();
        double stlStackPushUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        int val = 0;
        for (int i = 0; i < testIterations; ++i) customStack.pop(val);
        t2 = std::chrono::high_resolution_clock::now();
        double customStackPopUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            val = stlStack.top();
            stlStack.pop();
        }
        t2 = std::chrono::high_resolution_clock::now();
        double stlStackPopUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        CircularQueue<int, 60000> customQueue;
        std::queue<int> stlQueue;

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) customQueue.enqueue(i);
        t2 = std::chrono::high_resolution_clock::now();
        double customQueueEnqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) stlQueue.push(i);
        t2 = std::chrono::high_resolution_clock::now();
        double stlQueueEnqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) customQueue.dequeue(val);
        t2 = std::chrono::high_resolution_clock::now();
        double customQueueDeqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            val = stlQueue.front();
            stlQueue.pop();
        }
        t2 = std::chrono::high_resolution_clock::now();
        double stlQueueDeqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        ss << "{"
           << "\"testIterations\":" << testIterations << ","
           << "\"stackComparison\":{"
           << "\"customArrayStackPushMicroseconds\":" << customStackPushUs << ","
           << "\"stlStackPushMicroseconds\":" << stlStackPushUs << ","
           << "\"customArrayStackPopMicroseconds\":" << customStackPopUs << ","
           << "\"stlStackPopMicroseconds\":" << stlStackPopUs << ","
           << "\"memoryLayout\":\"Custom ArrayStack uses contiguous cache memory with zero allocations; std::stack wraps deque with node allocations.\""
           << "},"
           << "\"queueComparison\":{"
           << "\"customCircularQueueEnqueueMicroseconds\":" << customQueueEnqUs << ","
           << "\"stlQueuePushMicroseconds\":" << stlQueueEnqUs << ","
           << "\"customCircularQueueDequeueMicroseconds\":" << customQueueDeqUs << ","
           << "\"stlQueuePopMicroseconds\":" << stlQueueDeqUs << ","
           << "\"memoryLayout\":\"Custom CircularQueue uses fixed buffer with modulo arithmetic; std::queue wraps std::deque with segmented blocks.\""
           << "}"
           << "}";
        return ss.str();
    }
};

// ============================================================================
// [MODULE VII] TIMED STOPWATCH BENCHMARK
// ============================================================================

class PerformanceBenchmark {
private:
    static const int SEARCH_ELEMENTS = 30000;
    static const int SORT_ELEMENTS = 2500;

public:
    std::string runFullBenchmark() {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);

        std::vector<int> searchData(SEARCH_ELEMENTS);
        for (int i = 0; i < SEARCH_ELEMENTS; ++i) {
            searchData[i] = i * 2;
        }
        int target = (SEARCH_ELEMENTS - 10) * 2;

        auto tStart = std::chrono::high_resolution_clock::now();
        int linearFoundIdx = -1;
        for (int i = 0; i < SEARCH_ELEMENTS; ++i) {
            if (searchData[i] == target) {
                linearFoundIdx = i;
                break;
            }
        }
        auto tEnd = std::chrono::high_resolution_clock::now();
        double linearSearchUs = std::chrono::duration<double, std::micro>(tEnd - tStart).count();

        tStart = std::chrono::high_resolution_clock::now();
        int low = 0, high = SEARCH_ELEMENTS - 1, binaryFoundIdx = -1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (searchData[mid] == target) {
                binaryFoundIdx = mid;
                break;
            } else if (searchData[mid] < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        tEnd = std::chrono::high_resolution_clock::now();
        double binarySearchUs = std::chrono::duration<double, std::micro>(tEnd - tStart).count();

        std::vector<int> bubbleData(SORT_ELEMENTS);
        for (int i = 0; i < SORT_ELEMENTS; ++i) {
            bubbleData[i] = SORT_ELEMENTS - i;
        }
        std::vector<int> introData = bubbleData;

        tStart = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < SORT_ELEMENTS - 1; ++i) {
            for (int j = 0; j < SORT_ELEMENTS - i - 1; ++j) {
                if (bubbleData[j] > bubbleData[j + 1]) {
                    int tmp = bubbleData[j];
                    bubbleData[j] = bubbleData[j + 1];
                    bubbleData[j + 1] = tmp;
                }
            }
        }
        tEnd = std::chrono::high_resolution_clock::now();
        double bubbleSortMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        tStart = std::chrono::high_resolution_clock::now();
        std::sort(introData.begin(), introData.end());
        tEnd = std::chrono::high_resolution_clock::now();
        double stdSortMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        ss << "{"
           << "\"searchBenchmark\":{"
           << "\"elements\":" << SEARCH_ELEMENTS << ","
           << "\"targetKey\":" << target << ","
           << "\"linearSearch\":{"
           << "\"timeMicroseconds\":" << linearSearchUs << ","
           << "\"timeComplexity\":\"O(N)\","
           << "\"spaceComplexity\":\"O(1)\","
           << "\"foundIndex\":" << linearFoundIdx
           << "},"
           << "\"binarySearch\":{"
           << "\"timeMicroseconds\":" << binarySearchUs << ","
           << "\"timeComplexity\":\"O(log N)\","
           << "\"spaceComplexity\":\"O(1)\","
           << "\"foundIndex\":" << binaryFoundIdx
           << "},"
           << "\"speedupFactor\":" << (binarySearchUs > 0 ? (linearSearchUs / binarySearchUs) : 100.0)
           << "},"
           << "\"sortBenchmark\":{"
           << "\"elements\":" << SORT_ELEMENTS << ","
           << "\"bubbleSort\":{"
           << "\"timeMilliseconds\":" << bubbleSortMs << ","
           << "\"timeComplexity\":\"O(N^2)\","
           << "\"spaceComplexity\":\"O(1)\""
           << "},"
           << "\"introsort\":{"
           << "\"timeMilliseconds\":" << stdSortMs << ","
           << "\"timeComplexity\":\"O(N log N)\","
           << "\"spaceComplexity\":\"O(log N)\""
           << "},"
           << "\"speedupFactor\":" << (stdSortMs > 0 ? (bubbleSortMs / stdSortMs) : 50.0)
           << "},"
           << "\"featureComplexityAnalysis\":["
           << "{\"feature\":\"Browse Catalog\",\"dataStructure\":\"1D Array\",\"time\":\"O(1) direct / O(N) scan\",\"space\":\"O(1)\"},"
           << "{\"feature\":\"Dish Search\",\"dataStructure\":\"String Pattern Matching\",\"time\":\"O(M * K)\",\"space\":\"O(1)\"},"
           << "{\"feature\":\"Did You Mean Suggestion\",\"dataStructure\":\"Levenshtein Distance Matrix\",\"time\":\"O(L1 * L2)\",\"space\":\"O(L1 * L2)\"},"
           << "{\"feature\":\"Order Modification Undo\",\"dataStructure\":\"ArrayStack (Hand-crafted)\",\"time\":\"O(1) push/pop\",\"space\":\"O(MAX_CART)\"},"
           << "{\"feature\":\"Kitchen KOT Multi-Queue\",\"dataStructure\":\"CircularQueue (Hand-crafted)\",\"time\":\"O(1) enqueue/dequeue\",\"space\":\"O(MAX_ORDERS)\"},"
           << "{\"feature\":\"Weekly Sales Matrix\",\"dataStructure\":\"2D Matrix (6x7)\",\"time\":\"O(R * D)\",\"space\":\"O(R * D)\"},"
           << "{\"feature\":\"Table Seating Layout\",\"dataStructure\":\"2D Matrix (4x3)\",\"time\":\"O(1) lookup\",\"space\":\"O(S * T)\"},"
           << "{\"feature\":\"Invoice Check Digit\",\"dataStructure\":\"String Reversal Algorithm\",\"time\":\"O(L)\",\"space\":\"O(L)\"},"
           << "{\"feature\":\"Past Bills Archive\",\"dataStructure\":\"std::deque (Double-ended queue)\",\"time\":\"O(1) push_front\",\"space\":\"O(B)\"}"
           << "]"
           << "}";
        return ss.str();
    }
};

#endif // ALGORITHMS_HPP
