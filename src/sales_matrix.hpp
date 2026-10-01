// FoodRush - 2D Array Matrix Operations
// [MODULE IV] 2D Arrays (matrix operations) | used by: SALES_MATRIX_AND_ZONE_DISTANCE
#ifndef SALES_MATRIX_HPP
#define SALES_MATRIX_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "models.hpp"

const int DAYS_PER_WEEK = 7;

// [MODULE IV] Neutral Zone names for 5 delivery zones
const char* const ZONE_NAMES[MAX_ZONES] = {
    "Central",
    "North",
    "South",
    "East",
    "West"
};

// [MODULE IV] 2D Array: Distance Matrix between delivery zones (in kilometers)
// Symmetric 5x5 matrix
const double ZONE_DISTANCE_MATRIX[MAX_ZONES][MAX_ZONES] = {
    // Cen    Nor    Sou    Eas    Wes
    { 0.0,   3.5,   4.2,   5.1,   3.8 }, // Central (Zone 0)
    { 3.5,   0.0,   7.4,   6.2,   5.9 }, // North   (Zone 1)
    { 4.2,   7.4,   0.0,   6.8,   6.5 }, // South   (Zone 2)
    { 5.1,   6.2,   6.8,   0.0,   8.2 }, // East    (Zone 3)
    { 3.8,   5.9,   6.5,   8.2,   0.0 }  // West    (Zone 4)
};

class SalesMatrixManager {
private:
    // [MODULE IV] 2D Array: Sales revenue per restaurant per day of the week
    // Rows: 6 Restaurants, Columns: 7 Days (Mon=0, Tue=1, Wed=2, Thu=3, Fri=4, Sat=5, Sun=6)
    double sales[MAX_RESTAURANTS][DAYS_PER_WEEK];

public:
    // [MODULE IV] Matrix Initialization with seed historical revenue in INR | used by: SALES_INIT
    SalesMatrixManager() {
        // Restaurant 0: Royal Dum Biryani
        sales[0][0] = 14200.0; sales[0][1] = 13800.0; sales[0][2] = 15900.0; sales[0][3] = 16400.0; sales[0][4] = 22500.0; sales[0][5] = 28900.0; sales[0][6] = 26400.0;
        // Restaurant 1: Sagar Dosa & Tiffin
        sales[1][0] = 9800.0;  sales[1][1] = 10400.0; sales[1][2] = 11200.0; sales[1][3] = 11800.0; sales[1][4] = 15600.0; sales[1][5] = 21400.0; sales[1][6] = 19800.0;
        // Restaurant 2: Bella Italia Trattoria
        sales[2][0] = 11500.0; sales[2][1] = 10900.0; sales[2][2] = 12600.0; sales[2][3] = 13100.0; sales[2][4] = 18900.0; sales[2][5] = 24800.0; sales[2][6] = 22300.0;
        // Restaurant 3: Tokyo Ramen & Robata
        sales[3][0] = 12800.0; sales[3][1] = 12100.0; sales[3][2] = 13400.0; sales[3][3] = 14200.0; sales[3][4] = 19700.0; sales[3][5] = 25900.0; sales[3][6] = 23700.0;
        // Restaurant 4: The Burger & Brews Co.
        sales[4][0] = 13600.0; sales[4][1] = 13100.0; sales[4][2] = 14800.0; sales[4][3] = 15500.0; sales[4][4] = 21200.0; sales[4][5] = 27800.0; sales[4][6] = 25100.0;
        // Restaurant 5: Sweet Tooth Patisserie
        sales[5][0] = 8400.0;  sales[5][1] = 7900.0;  sales[5][2] = 8900.0;  sales[5][3] = 9400.0;  sales[5][4] = 14200.0; sales[5][5] = 19500.0; sales[5][6] = 18100.0;
    }

    // [MODULE IV] Row Sum: Calculate total weekly sales for a specific restaurant | used by: RESTAURANT_TOTALS
    double getRestaurantWeeklyTotal(int restaurantIndex) const {
        if (restaurantIndex < 0 || restaurantIndex >= MAX_RESTAURANTS) return 0.0;
        double sum = 0.0;
        for (int day = 0; day < DAYS_PER_WEEK; ++day) {
            sum += sales[restaurantIndex][day]; // Row traversal
        }
        return sum;
    }

    // [MODULE IV] Column Sum: Calculate total sales on a given day across all restaurants | used by: DAILY_TOTALS
    double getDailyTotalSales(int dayIndex) const {
        if (dayIndex < 0 || dayIndex >= DAYS_PER_WEEK) return 0.0;
        double sum = 0.0;
        for (int rest = 0; rest < MAX_RESTAURANTS; ++rest) {
            sum += sales[rest][dayIndex]; // Column traversal
        }
        return sum;
    }

    // [MODULE IV] Grand Matrix Sum: Platform total revenue | used by: PLATFORM_TOTAL
    double getGrandTotalSales() const {
        double total = 0.0;
        for (int r = 0; r < MAX_RESTAURANTS; ++r) {
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                total += sales[r][d];
            }
        }
        return total;
    }

    // [MODULE IV] Matrix Max Element Search: Find the busiest restaurant and day | used by: SALES_PEAK_ANALYSIS
    void findBusiestSlot(int& outRestIndex, int& outDayIndex, double& outPeakAmount) const {
        outPeakAmount = -1.0;
        outRestIndex = 0;
        outDayIndex = 0;
        for (int r = 0; r < MAX_RESTAURANTS; ++r) {
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                if (sales[r][d] > outPeakAmount) {
                    outPeakAmount = sales[r][d];
                    outRestIndex = r;
                    outDayIndex = d;
                }
            }
        }
    }

    // [MODULE IV] Record new live order sale into matrix | used by: LIVE_SALES_RECORDING
    void recordSale(int restaurantIndex, int dayIndex, double amount) {
        if (restaurantIndex >= 0 && restaurantIndex < MAX_RESTAURANTS &&
            dayIndex >= 0 && dayIndex < DAYS_PER_WEEK) {
            sales[restaurantIndex][dayIndex] += amount;
        }
    }

    // [MODULE IV] 2D Zone Distance Lookup | used by: DELIVERY_FEE_CALCULATION
    static double getDistance(int fromZone, int toZone) {
        if (fromZone < 0 || fromZone >= MAX_ZONES || toZone < 0 || toZone >= MAX_ZONES) {
            return 4.0; // Default fallback distance
        }
        return ZONE_DISTANCE_MATRIX[fromZone][toZone];
    }

    // [MODULE I] & [MODULE IV] Calculate delivery fee and estimated delivery time
    static void calculateDelivery(int restaurantZone, int customerZone, bool isExpress, double& outFee, int& outEstimatedMinutes) {
        double distKm = getDistance(restaurantZone, customerZone);
        // Type conversion: static_cast and arithmetic operators
        outFee = BASE_DELIVERY_FEE + (distKm * PER_KM_FEE);
        if (isExpress) {
            outFee += EXPRESS_SURCHARGE;
        }

        // ETA calculation: 15 mins prep + 3 mins per km (Express shaves off 10 mins)
        int travelMins = static_cast<int>(distKm * 3.0);
        int prepMins = isExpress ? 12 : 20;
        outEstimatedMinutes = prepMins + travelMins;
    }

    // Serialize full 2D matrix to JSON for admin dashboard
    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"days\":[\"Mon\",\"Tue\",\"Wed\",\"Thu\",\"Fri\",\"Sat\",\"Sun\"],"
           << "\"zones\":[";
        for (int z = 0; z < MAX_ZONES; ++z) {
            ss << "\"" << ZONE_NAMES[z] << "\"";
            if (z < MAX_ZONES - 1) ss << ",";
        }
        ss << "],"
           << "\"distanceMatrix\":[";
        for (int i = 0; i < MAX_ZONES; ++i) {
            ss << "[";
            for (int j = 0; j < MAX_ZONES; ++j) {
                ss << ZONE_DISTANCE_MATRIX[i][j];
                if (j < MAX_ZONES - 1) ss << ",";
            }
            ss << "]";
            if (i < MAX_ZONES - 1) ss << ",";
        }
        ss << "],"
           << "\"sales\":[";
        for (int r = 0; r < MAX_RESTAURANTS; ++r) {
            ss << "[";
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                ss << sales[r][d];
                if (d < DAYS_PER_WEEK - 1) ss << ",";
            }
            ss << "]";
            if (r < MAX_RESTAURANTS - 1) ss << ",";
        }
        ss << "],"
           << "\"restaurantWeeklyTotals\":[";
        for (int r = 0; r < MAX_RESTAURANTS; ++r) {
            ss << getRestaurantWeeklyTotal(r);
            if (r < MAX_RESTAURANTS - 1) ss << ",";
        }
        ss << "],"
           << "\"dailyTotals\":[";
        for (int d = 0; d < DAYS_PER_WEEK; ++d) {
            ss << getDailyTotalSales(d);
            if (d < DAYS_PER_WEEK - 1) ss << ",";
        }
        ss << "],";

        int busiestRest = 0, busiestDay = 0;
        double peakAmount = 0.0;
        findBusiestSlot(busiestRest, busiestDay, peakAmount);

        ss << "\"grandTotal\":" << getGrandTotalSales() << ","
           << "\"busiestRestaurant\":" << busiestRest << ","
           << "\"busiestDay\":" << busiestDay << ","
           << "\"peakSalesAmount\":" << peakAmount
           << "}";
        return ss.str();
    }
};

#endif // SALES_MATRIX_HPP
