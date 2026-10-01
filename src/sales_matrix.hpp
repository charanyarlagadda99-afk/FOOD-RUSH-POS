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

// Zone names for delivery matrix lookup
const char* const ZONE_NAMES[MAX_ZONES] = {
    "Downtown",
    "Uptown",
    "Tech Park",
    "Suburbs North",
    "Waterfront Harbor"
};

// [MODULE IV] 2D Array: Distance Matrix between delivery zones (in kilometers)
// Symmetric 5x5 matrix
const double ZONE_DISTANCE_MATRIX[MAX_ZONES][MAX_ZONES] = {
    // DT    UP    TP    SN    WF
    { 0.0,  3.2,  5.8,  8.4,  4.1 }, // Downtown
    { 3.2,  0.0,  4.5,  6.1,  7.0 }, // Uptown
    { 5.8,  4.5,  0.0,  5.2,  9.3 }, // Tech Park
    { 8.4,  6.1,  5.2,  0.0, 11.5 }, // Suburbs North
    { 4.1,  7.0,  9.3, 11.5,  0.0 }  // Waterfront Harbor
};

class SalesMatrixManager {
private:
    // [MODULE IV] 2D Array: Sales revenue per restaurant per day of the week
    // Rows: 4 Restaurants, Columns: 7 Days (Mon=0, Tue=1, Wed=2, Thu=3, Fri=4, Sat=5, Sun=6)
    double sales[MAX_RESTAURANTS][DAYS_PER_WEEK];

public:
    // [MODULE IV] Matrix Initialization with seed historical data | used by: SALES_INIT
    SalesMatrixManager() {
        // Restaurant 0: Bella Italia
        sales[0][0] = 620.50; sales[0][1] = 580.00; sales[0][2] = 710.25; sales[0][3] = 690.80; sales[0][4] = 950.40; sales[0][5] = 1240.00; sales[0][6] = 1110.50;
        // Restaurant 1: Tokyo Ramen & Sushi
        sales[1][0] = 540.00; sales[1][1] = 610.75; sales[1][2] = 630.00; sales[1][3] = 720.50; sales[1][4] = 880.20; sales[1][5] = 1350.80; sales[1][6] = 1190.00;
        // Restaurant 2: Spice Symphony
        sales[2][0] = 480.25; sales[2][1] = 510.00; sales[2][2] = 590.50; sales[2][3] = 640.00; sales[2][4] = 890.60; sales[2][5] = 1420.30; sales[2][6] = 1280.40;
        // Restaurant 3: Burger & Brews
        sales[3][0] = 710.00; sales[3][1] = 690.50; sales[3][2] = 780.00; sales[3][3] = 820.40; sales[3][4] = 1150.00; sales[3][5] = 1580.90; sales[3][6] = 1390.20;
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
            return 5.0; // Default fallback distance
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

        // ETA calculation: 15 mins prep + 3.5 mins per km (Express is 10 mins faster)
        int travelMins = static_cast<int>(distKm * 3.5);
        int prepMins = isExpress ? 10 : 18;
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
