// FoodRush - Custom Array-based Stack Implementation
// [MODULE VIII] Stack (array implementation + applications) | used by: CART_UNDO_AND_RECENT_ITEMS
#ifndef ARRAY_STACK_HPP
#define ARRAY_STACK_HPP

#include <stdexcept>
#include <sstream>
#include <string>

// Hand-crafted fixed-capacity Stack using a raw 1D array
// Rule: Built by hand with arrays first before STL versions
template <typename T, int CAPACITY>
class ArrayStack {
private:
    T elements[CAPACITY]; // Raw 1D array storage
    int topIndex;         // Index of topmost element (-1 when empty)

public:
    // [MODULE VIII] Stack Constructor | used by: STACK_INITIALIZATION
    ArrayStack() : topIndex(-1) {}

    // [MODULE VIII] Stack isFull check | used by: CAPACITY_GUARD
    bool isFull() const {
        return topIndex >= CAPACITY - 1;
    }

    // [MODULE VIII] Stack isEmpty check | used by: BOUNDS_GUARD
    bool isEmpty() const {
        return topIndex == -1;
    }

    // [MODULE VIII] Stack size | used by: INSPECTOR
    int size() const {
        return topIndex + 1;
    }

    int capacity() const {
        return CAPACITY;
    }

    // [MODULE VIII] Stack push | used by: UNDO
    bool push(const T& item) {
        if (isFull()) {
            return false; // Prevent stack overflow
        }
        elements[++topIndex] = item;
        return true;
    }

    // [MODULE VIII] Stack pop | used by: UNDO
    bool pop(T& outItem) {
        if (isEmpty()) {
            return false; // Prevent stack underflow
        }
        outItem = elements[topIndex--];
        return true;
    }

    // [MODULE VIII] Stack peek / top | used by: INSPECTOR
    bool peek(T& outItem) const {
        if (isEmpty()) {
            return false;
        }
        outItem = elements[topIndex];
        return true;
    }

    // [MODULE VIII] Stack clear | used by: CART_RESET
    void clear() {
        topIndex = -1;
    }

    // Access element at depth (0 = top, 1 = second from top...) for inspection
    bool getAtDepth(int depth, T& outItem) const {
        int idx = topIndex - depth;
        if (idx < 0 || idx > topIndex) {
            return false;
        }
        outItem = elements[idx];
        return true;
    }

    int getTopIndex() const {
        return topIndex;
    }

    bool getAtOffset(int offset, T& outItem) const {
        return getAtDepth(offset, outItem);
    }
};

#endif // ARRAY_STACK_HPP
