// FoodRush - Core Algorithms, String Processing, 1D Array Stats, and Benchmarks
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

// [MODULE III] Sum of elements in a 1D array | used by: AVERAGE_RATINGS
inline double calculateArraySum(const double arr[], int size) {
    double total = 0.0;
    for (int i = 0; i < size; ++i) { // 1D array traversal
        total += arr[i];
    }
    return total;
}

// [MODULE III] Maximum element in a 1D array with index | used by: BEST_RATED_RESTAURANT
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

// [MODULE III] Bubble Sort on 1D array of prices | used by: PRICE_SORTING
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

// [MODULE V] Character Frequency Analysis | used by: COUPON_ENTROPY_CHECK
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

// [MODULE V] Levenshtein Edit Distance for "Did You Mean" Fuzzy Search Suggestions
inline int calculateLevenshteinDistance(const std::string& s1, const std::string& s2) {
    int m = static_cast<int>(s1.length());
    int n = static_cast<int>(s2.length());

    // Allocate 2D matrix for dynamic programming table
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

    // Check item names and categories
    for (int i = 0; i < itemCount; ++i) {
        // Compare with category
        std::string cat = toLowerString(items[i].category);
        int dCat = calculateLevenshteinDistance(cleanQuery, cat);
        if (dCat < minDistance && dCat <= 2 && dCat > 0) {
            minDistance = dCat;
            bestMatch = items[i].category;
        }

        // Compare with individual words in dish name
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

// [MODULE V] Tracking Code Check Digit Generator Using String Reversal
// Computes a tamper-proof check digit by reversing the order digits and calculating weighted modulo
inline std::string generateTrackingCode(int orderId) {
    std::string idStr = std::to_string(orderId);
    std::string rev = reverseString(idStr); // [MODULE V] String reversal

    int weightedSum = 0;
    for (size_t i = 0; i < rev.length(); ++i) {
        weightedSum += (rev[i] - '0') * (static_cast<int>(i) + 2);
    }
    int checkDigit = (weightedSum % 9) + 1; // 1 to 9

    return "TRK-" + idStr + "-" + std::to_string(checkDigit);
}

// [MODULE V] Validate Tracking Code Using String Reversal Verification
inline bool validateTrackingCode(const std::string& trackingCode) {
    auto tokens = tokenizeString(trackingCode, '-');
    if (tokens.size() != 3 || tokens[0] != "TRK") {
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

// ============================================================================
// [FEATURE 1: SMART FLEET DISPATCH & ZONE ROUTING]
// ============================================================================
inline int findNearestAvailableRider(
    int restaurantZone,
    const Rider riders[],
    int riderCount,
    const double distanceMatrix[MAX_ZONES][MAX_ZONES],
    double& outDistance)
{
    int bestIdx = -1;
    double minDistance = 9999.0;
    for (int i = 0; i < riderCount; ++i) {
        if (riders[i].isAvailable) {
            double dist = distanceMatrix[restaurantZone][riders[i].currentZone];
            if (dist < minDistance) {
                minDistance = dist;
                bestIdx = i;
            }
        }
    }
    outDistance = (bestIdx != -1) ? minDistance : -1.0;
    return bestIdx;
}

// ============================================================================
// [FEATURE 3: RATING SYSTEM & TOP-K RANKING ENGINE]
// ============================================================================
inline std::vector<MenuItem> getTopKDishes(const MenuItem items[], int itemCount, int k) {
    std::vector<MenuItem> sortedList;
    for (int i = 0; i < itemCount; ++i) {
        sortedList.push_back(items[i]);
    }
    std::sort(sortedList.begin(), sortedList.end(), [](const MenuItem& a, const MenuItem& b) {
        if (std::abs(a.rating - b.rating) > 0.001) return a.rating > b.rating;
        return a.ratingCount > b.ratingCount; // tie-breaker by total reviews
    });
    if (k > static_cast<int>(sortedList.size())) k = static_cast<int>(sortedList.size());
    if (k < 1) k = 1;
    return std::vector<MenuItem>(sortedList.begin(), sortedList.begin() + k);
}

// [MODULE V] JSON String Escaping | used by: PROTOCOL_SERIALIZER
inline std::string escapeJSON(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\b') ss << "\\b";
        else if (c == '\f') ss << "\\f";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

// ============================================================================
// [MODULE X] STL DATA STRUCTURES USAGE
// ============================================================================

class STLManager {
private:
    // [MODULE X] std::map for coupon discount lookup | used by: DISCOUNT_SYSTEM
    std::map<std::string, double> couponDiscounts;

    // [MODULE X] std::set for unique cuisine categories | used by: CATEGORY_REGISTRY
    std::set<std::string> availableCuisines;

    // [MODULE X] std::vector for dynamic menu item storage | used by: DYNAMIC_CATALOG
    std::vector<MenuItem> stlMenuItems;

    // [MODULE X] std::stack for STL cart undo stack | used by: STL_STACK_VERSION
    std::stack<CartAction> stlUndoStack;

    // [MODULE X] std::queue for STL order kitchen queue | used by: STL_QUEUE_VERSION
    std::queue<int> stlKitchenQueue;

    // [MODULE X] std::deque for recent completed order history | used by: RECENT_ORDERS_DEQUE
    std::deque<int> recentOrdersDeque;

    // [MODULE X] std::pair for promo pair (itemId, discountPercent) | used by: PROMO_PAIRS
    std::vector<std::pair<int, double>> dailySpecials;

public:
    STLManager() {
        // Product Coupons in std::map
        couponDiscounts["FIRST50"]   = 50.0; // 50% off (up to limit)
        couponDiscounts["WELCOME10"] = 10.0; // 10% off
        couponDiscounts["FREEDEL"]   = 100.0; // Free delivery flag
        couponDiscounts["FOODRUSH"]  = 20.0; // 20% off

        // Unique cuisines in std::set
        availableCuisines.insert("Biryani & Mughlai");
        availableCuisines.insert("South Indian");
        availableCuisines.insert("Artisan Italian");
        availableCuisines.insert("Japanese");
        availableCuisines.insert("American Gourmet");
        availableCuisines.insert("Desserts & Bakery");

        // Seed daily special pairs (itemId, discount%)
        dailySpecials.push_back(std::make_pair(101, 15.0)); // Hyderabadi Dum Biryani
        dailySpecials.push_back(std::make_pair(201, 20.0)); // Ghee Roast Dosa
        dailySpecials.push_back(std::make_pair(301, 10.0)); // Wood-Fired Margherita
        dailySpecials.push_back(std::make_pair(401, 12.5)); // Tonkotsu Ramen
        dailySpecials.push_back(std::make_pair(501, 15.0)); // Cheddar Smash Burger
        dailySpecials.push_back(std::make_pair(601, 10.0)); // Chocolate Ganache Pastry
    }

    // [MODULE X] Lookup coupon discount using std::map | used by: APPLY_COUPON
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

    // [MODULE X] Push to STL Undo Stack | used by: STL_UNDO
    void pushUndo(const CartAction& action) {
        stlUndoStack.push(action);
    }

    // [MODULE X] Pop from STL Undo Stack
    bool popUndo(CartAction& outAction) {
        if (stlUndoStack.empty()) return false;
        outAction = stlUndoStack.top();
        stlUndoStack.pop();
        return true;
    }

    // [MODULE X] Enqueue to STL Queue | used by: STL_KITCHEN
    void enqueueOrder(int orderId) {
        stlKitchenQueue.push(orderId);
    }

    // [MODULE X] Dequeue from STL Queue
    bool dequeueOrder(int& outOrderId) {
        if (stlKitchenQueue.empty()) return false;
        outOrderId = stlKitchenQueue.front();
        stlKitchenQueue.pop();
        return true;
    }

    // [MODULE X] Double-ended queue (deque) tracking of completed orders
    void recordCompletedOrder(int orderId) {
        recentOrdersDeque.push_back(orderId);
        if (recentOrdersDeque.size() > 25) {
            recentOrdersDeque.pop_front(); // Keep last 25
        }
    }

    const std::map<std::string, double>& getCoupons() const { return couponDiscounts; }
    const std::set<std::string>& getCuisines() const { return availableCuisines; }
    const std::vector<std::pair<int, double>>& getDailySpecials() const { return dailySpecials; }

    int getVectorSize() const { return static_cast<int>(stlMenuItems.size()); }
    int getCuisinesCount() const { return static_cast<int>(availableCuisines.size()); }
    int getCouponsCount() const { return static_cast<int>(couponDiscounts.size()); }
    int getCompletedCount() const { return static_cast<int>(recentOrdersDeque.size()); }
    void addCuisine(const std::string& cuisine) { availableCuisines.insert(cuisine); }

    // [MODULE X] Live Comparison: Hand-crafted Array Stack/Queue vs STL Stack/Queue
    std::string compareDataStructures(int testIterations = 50000) const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);

        // Benchmark 1: Custom ArrayStack vs std::stack
        ArrayStack<int, 60000> customStack;
        std::stack<int> stlStack;

        // Custom Stack Push
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            customStack.push(i);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        double customStackPushUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        // STL Stack Push
        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            stlStack.push(i);
        }
        t2 = std::chrono::high_resolution_clock::now();
        double stlStackPushUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        // Custom Stack Pop
        t1 = std::chrono::high_resolution_clock::now();
        int val = 0;
        for (int i = 0; i < testIterations; ++i) {
            customStack.pop(val);
        }
        t2 = std::chrono::high_resolution_clock::now();
        double customStackPopUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        // STL Stack Pop
        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            val = stlStack.top();
            stlStack.pop();
        }
        t2 = std::chrono::high_resolution_clock::now();
        double stlStackPopUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        // Benchmark 2: Custom CircularQueue vs std::queue
        CircularQueue<int, 60000> customQueue;
        std::queue<int> stlQueue;

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            customQueue.enqueue(i);
        }
        t2 = std::chrono::high_resolution_clock::now();
        double customQueueEnqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            stlQueue.push(i);
        }
        t2 = std::chrono::high_resolution_clock::now();
        double stlQueueEnqUs = std::chrono::duration<double, std::micro>(t2 - t1).count();

        t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < testIterations; ++i) {
            customQueue.dequeue(val);
        }
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
           << "\"customMemoryLayout\":\"Contiguous fixed array stack (Zero heap allocations, high cache locality)\","
           << "\"stlMemoryLayout\":\"std::deque backed adapter (Dynamic chunked node allocation)\""
           << "},"
           << "\"queueComparison\":{"
           << "\"customCircularQueueEnqueueMicroseconds\":" << customQueueEnqUs << ","
           << "\"stlQueuePushMicroseconds\":" << stlQueueEnqUs << ","
           << "\"customCircularQueueDequeueMicroseconds\":" << customQueueDeqUs << ","
           << "\"stlQueuePopMicroseconds\":" << stlQueueDeqUs << ","
           << "\"customMemoryLayout\":\"Circular buffer with modulo index wrap-around O(1) bounded memory\","
           << "\"stlMemoryLayout\":\"std::queue adapter over std::deque with segmented blocks\""
           << "}"
           << "}";
        return ss.str();
    }
};

// ============================================================================
// [MODULE VII] PERFORMANCE BENCHMARK (Time & Space Complexity)
// ============================================================================

class PerformanceBenchmark {
public:
    static std::string runFullBenchmark() {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);

        const int SEARCH_ELEMENTS = 30000;
        const int SORT_ELEMENTS = 2500;

        // 1. Prepare sorted array for Search test
        std::vector<int> searchData(SEARCH_ELEMENTS);
        for (int i = 0; i < SEARCH_ELEMENTS; ++i) {
            searchData[i] = i * 2; // Sorted even numbers
        }
        int target = (SEARCH_ELEMENTS - 5) * 2; // Near the end to stress Linear Search

        // Linear Search O(N)
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

        // Binary Search O(log N)
        tStart = std::chrono::high_resolution_clock::now();
        int low = 0, high = SEARCH_ELEMENTS - 1;
        int binaryFoundIdx = -1;
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

        // 2. Prepare randomized arrays for Sort test
        std::vector<int> bubbleData(SORT_ELEMENTS);
        for (int i = 0; i < SORT_ELEMENTS; ++i) {
            bubbleData[i] = SORT_ELEMENTS - i; // Worst-case reverse order
        }
        std::vector<int> introData = bubbleData;

        // Bubble Sort O(N^2)
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

        // std::sort O(N log N) - Introsort
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
           << "{\"feature\":\"Browse Menu\",\"dataStructure\":\"1D Array\",\"time\":\"O(1) direct / O(N) scan\",\"space\":\"O(1)\"},"
           << "{\"feature\":\"Dish Search\",\"dataStructure\":\"String Pattern Matching\",\"time\":\"O(M * K)\",\"space\":\"O(1)\"},"
           << "{\"feature\":\"Did You Mean Suggestion\",\"dataStructure\":\"Levenshtein Distance Matrix\",\"time\":\"O(L1 * L2)\",\"space\":\"O(L1 * L2)\"},"
           << "{\"feature\":\"Cart Undo Stack\",\"dataStructure\":\"ArrayStack (Hand-crafted)\",\"time\":\"O(1) push/pop\",\"space\":\"O(MAX_CART)\"},"
           << "{\"feature\":\"Kitchen Order Queue\",\"dataStructure\":\"CircularQueue (Hand-crafted)\",\"time\":\"O(1) enqueue/dequeue\",\"space\":\"O(MAX_ORDERS)\"},"
           << "{\"feature\":\"Sales Matrix Analysis\",\"dataStructure\":\"2D Matrix (6x7)\",\"time\":\"O(R * D)\",\"space\":\"O(R * D)\"},"
           << "{\"feature\":\"Zone Distance Lookup\",\"dataStructure\":\"2D Matrix (5x5)\",\"time\":\"O(1) lookup\",\"space\":\"O(Z^2)\"},"
           << "{\"feature\":\"Tracking Code Check Digit\",\"dataStructure\":\"String Reversal Algorithm\",\"time\":\"O(L)\",\"space\":\"O(L)\"},"
           << "{\"feature\":\"Coupon Validation\",\"dataStructure\":\"std::map (Red-Black Tree)\",\"time\":\"O(log C)\",\"space\":\"O(C)\"}"
           << "]"
           << "}";
        return ss.str();
    }
};

#endif // ALGORITHMS_HPP
