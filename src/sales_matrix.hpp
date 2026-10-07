// FoodRush / RestoRush - 2D Array Matrix Operations for Hotel & Restaurant POS
// [MODULE IV] 2D Arrays (matrix operations) | used by: SALES_REPORTS_AND_TABLE_LAYOUT
#ifndef SALES_MATRIX_HPP
#define SALES_MATRIX_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "models.hpp"

const int DAYS_PER_WEEK = 7;
const int SECTIONS_COUNT = 4;
const int TABLES_PER_SECTION = 3;

// [MODULE IV] Hotel / Restaurant Dining Sections
const char* const SECTION_NAMES[SECTIONS_COUNT] = {
    "Main Dining Hall",
    "AC Family Lounge",
    "Rooftop Terrace",
    "Garden Lounge & Banquet"
};

// [MODULE IV] 2D Array: Table Seating Capacity Matrix (4 Dining Sections x 3 Tables each)
const int TABLE_CAPACITY_MATRIX[SECTIONS_COUNT][TABLES_PER_SECTION] = {
    { 4, 4, 6 }, // Section 0: Main Dining Hall (Tables 1, 2, 3)
    { 6, 8, 4 }, // Section 1: AC Family Lounge (Tables 4, 5, 6)
    { 2, 4, 4 }, // Section 2: Rooftop Terrace (Tables 7, 8, 9)
    { 4, 8, 10 } // Section 3: Garden Lounge & Banquet (Tables 10, 11, 12)
};

class SalesMatrixManager {
private:
    // [MODULE IV] 2D Array: Weekly sales revenue per outlet per day of the week
    // Rows: 6 Kitchen Outlets, Columns: 7 Days (Mon=0, Tue=1, Wed=2, Thu=3, Fri=4, Sat=5, Sun=6)
    double sales[MAX_OUTLETS][DAYS_PER_WEEK];

public:
    // [MODULE IV] Matrix Initialization with seed historical revenue in INR | used by: SALES_INIT
    SalesMatrixManager() {
        // Outlet 0: Grand Mughal Dining (Biryani & Mughlai)
        sales[0][0] = 14200.0; sales[0][1] = 13800.0; sales[0][2] = 15900.0; sales[0][3] = 16400.0; sales[0][4] = 22500.0; sales[0][5] = 28900.0; sales[0][6] = 26400.0;
        // Outlet 1: Dakshin Tiffin & Cafe (South Indian)
        sales[1][0] = 9800.0;  sales[1][1] = 10400.0; sales[1][2] = 11200.0; sales[1][3] = 11800.0; sales[1][4] = 15600.0; sales[1][5] = 21400.0; sales[1][6] = 19800.0;
        // Outlet 2: Trattoria Bella Vista (Artisan Italian)
        sales[2][0] = 11500.0; sales[2][1] = 10900.0; sales[2][2] = 12600.0; sales[2][3] = 13100.0; sales[2][4] = 18900.0; sales[2][5] = 24800.0; sales[2][6] = 22300.0;
        // Outlet 3: Sakura Asian Bistro (Japanese & Asian)
        sales[3][0] = 12800.0; sales[3][1] = 12100.0; sales[3][2] = 13400.0; sales[3][3] = 14200.0; sales[3][4] = 19700.0; sales[3][5] = 25900.0; sales[3][6] = 23700.0;
        // Outlet 4: The Boulevard Grill & Burgers (American & Cafe)
        sales[4][0] = 13600.0; sales[4][1] = 13100.0; sales[4][2] = 14800.0; sales[4][3] = 15500.0; sales[4][4] = 21200.0; sales[4][5] = 27800.0; sales[4][6] = 25100.0;
        // Outlet 5: The Royal Patisserie (Desserts & Confectionery)
        sales[5][0] = 8400.0;  sales[5][1] = 7900.0;  sales[5][2] = 8900.0;  sales[5][3] = 9400.0;  sales[5][4] = 14200.0; sales[5][5] = 19500.0; sales[5][6] = 18100.0;
    }

    // [MODULE IV] Row Sum: Calculate total weekly sales for a specific outlet | used by: OUTLET_TOTALS
    double getRestaurantWeeklyTotal(int outletIndex) const {
        if (outletIndex < 0 || outletIndex >= MAX_OUTLETS) return 0.0;
        double sum = 0.0;
        for (int day = 0; day < DAYS_PER_WEEK; ++day) {
            sum += sales[outletIndex][day]; // Row traversal
        }
        return sum;
    }

    // [MODULE IV] Column Sum: Calculate total sales on a given day across all outlets | used by: DAILY_TOTALS
    double getDailyTotalSales(int dayIndex) const {
        if (dayIndex < 0 || dayIndex >= DAYS_PER_WEEK) return 0.0;
        double sum = 0.0;
        for (int outlet = 0; outlet < MAX_OUTLETS; ++outlet) {
            sum += sales[outlet][dayIndex]; // Column traversal
        }
        return sum;
    }

    // [MODULE IV] Grand Matrix Sum: Platform total revenue | used by: HOTEL_GRAND_TOTAL
    double getGrandTotalSales() const {
        double total = 0.0;
        for (int r = 0; r < MAX_OUTLETS; ++r) {
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                total += sales[r][d];
            }
        }
        return total;
    }

    // [MODULE IV] Matrix Max Element Search: Find the busiest outlet and day | used by: SALES_PEAK_ANALYSIS
    void findBusiestSlot(int& outOutletIndex, int& outDayIndex, double& outPeakAmount) const {
        outPeakAmount = -1.0;
        outOutletIndex = 0;
        outDayIndex = 0;
        for (int r = 0; r < MAX_OUTLETS; ++r) {
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                if (sales[r][d] > outPeakAmount) {
                    outPeakAmount = sales[r][d];
                    outOutletIndex = r;
                    outDayIndex = d;
                }
            }
        }
    }

    // [MODULE IV] Record new live settled bill sale into matrix | used by: LIVE_BILL_RECORDING
    void recordSale(int outletIndex, int dayIndex, double amount) {
        if (outletIndex >= 0 && outletIndex < MAX_OUTLETS &&
            dayIndex >= 0 && dayIndex < DAYS_PER_WEEK) {
            sales[outletIndex][dayIndex] += amount;
        }
    }

    // Serialize full 2D matrix to JSON for admin sales report
    std::string toJSON() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{"
           << "\"days\":[\"Mon\",\"Tue\",\"Wed\",\"Thu\",\"Fri\",\"Sat\",\"Sun\"],"
           << "\"sections\":[";
        for (int s = 0; s < SECTIONS_COUNT; ++s) {
            ss << "\"" << SECTION_NAMES[s] << "\"";
            if (s < SECTIONS_COUNT - 1) ss << ",";
        }
        ss << "],"
           << "\"tableLayoutMatrix\":[";
        for (int i = 0; i < SECTIONS_COUNT; ++i) {
            ss << "[";
            for (int j = 0; j < TABLES_PER_SECTION; ++j) {
                ss << TABLE_CAPACITY_MATRIX[i][j];
                if (j < TABLES_PER_SECTION - 1) ss << ",";
            }
            ss << "]";
            if (i < SECTIONS_COUNT - 1) ss << ",";
        }
        ss << "],"
           << "\"sales\":[";
        for (int r = 0; r < MAX_OUTLETS; ++r) {
            ss << "[";
            for (int d = 0; d < DAYS_PER_WEEK; ++d) {
                ss << sales[r][d];
                if (d < DAYS_PER_WEEK - 1) ss << ",";
            }
            ss << "]";
            if (r < MAX_OUTLETS - 1) ss << ",";
        }
        ss << "],"
           << "\"outletWeeklyTotals\":[";
        for (int r = 0; r < MAX_OUTLETS; ++r) {
            ss << getRestaurantWeeklyTotal(r);
            if (r < MAX_OUTLETS - 1) ss << ",";
        }
        ss << "],"
           << "\"dailyTotals\":[";
        for (int d = 0; d < DAYS_PER_WEEK; ++d) {
            ss << getDailyTotalSales(d);
            if (d < DAYS_PER_WEEK - 1) ss << ",";
        }
        ss << "],";

        int busiestOutlet = 0, busiestDay = 0;
        double peakAmount = 0.0;
        findBusiestSlot(busiestOutlet, busiestDay, peakAmount);

        ss << "\"grandTotal\":" << getGrandTotalSales() << ","
           << "\"busiestOutlet\":" << busiestOutlet << ","
           << "\"busiestDay\":" << busiestDay << ","
           << "\"peakSalesAmount\":" << peakAmount
           << "}";
        return ss.str();
    }
};

#endif // SALES_MATRIX_HPP
