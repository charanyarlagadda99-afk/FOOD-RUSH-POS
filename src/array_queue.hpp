// FoodRush - Custom Array-based Circular Queue Implementation
// [MODULE IX] Queue (array circular implementation + applications) | used by: KITCHEN_DISPATCH
#ifndef ARRAY_QUEUE_HPP
#define ARRAY_QUEUE_HPP

#include <stdexcept>
#include <sstream>
#include <string>

// Hand-crafted Circular Queue using a raw 1D array and modulo arithmetic
// Rule: Built by hand with arrays first before STL versions
template <typename T, int CAPACITY>
class CircularQueue {
private:
    T buffer[CAPACITY]; // Raw 1D array
    int frontIndex;     // Points to front element
    int rearIndex;      // Points to last inserted element
    int count;          // Current number of elements

public:
    // [MODULE IX] Circular Queue Constructor | used by: QUEUE_INIT
    CircularQueue() : frontIndex(0), rearIndex(-1), count(0) {}

    // [MODULE IX] Queue isFull check | used by: KITCHEN_CAPACITY_GUARD
    bool isFull() const {
        return count == CAPACITY;
    }

    // [MODULE IX] Queue isEmpty check | used by: DISPATCH_GUARD
    bool isEmpty() const {
        return count == 0;
    }

    // [MODULE IX] Queue size query | used by: KITCHEN_MONITOR
    int size() const {
        return count;
    }

    int capacity() const {
        return CAPACITY;
    }

    int getFrontIndex() const {
        return frontIndex;
    }

    int getRearIndex() const {
        return rearIndex;
    }

    // [MODULE IX] Circular Queue Enqueue | used by: NEW_ORDER_SUBMISSION
    bool enqueue(const T& item) {
        if (isFull()) {
            return false; // Queue overflow
        }
        // Modulo wrap-around for circular buffer efficiency
        rearIndex = (rearIndex + 1) % CAPACITY;
        buffer[rearIndex] = item;
        count++;
        return true;
    }

    // [MODULE IX] Circular Queue Dequeue | used by: KITCHEN_COOKING_DISPATCH
    bool dequeue(T& outItem) {
        if (isEmpty()) {
            return false; // Queue underflow
        }
        outItem = buffer[frontIndex];
        // Advance front with circular wrap-around
        frontIndex = (frontIndex + 1) % CAPACITY;
        count--;
        return true;
    }

    // [MODULE IX] Circular Queue Peek | used by: NEXT_ORDER_PREVIEW
    bool peek(T& outItem) const {
        if (isEmpty()) {
            return false;
        }
        outItem = buffer[frontIndex];
        return true;
    }

    // Access element at logical offset from front (0 = next in line)
    bool getAtOffset(int offset, T& outItem) const {
        if (offset < 0 || offset >= count) {
            return false;
        }
        int actualIdx = (frontIndex + offset) % CAPACITY;
        outItem = buffer[actualIdx];
        return true;
    }

    // [MODULE IX] Queue Clear | used by: RESET_QUEUE
    void clear() {
        frontIndex = 0;
        rearIndex = -1;
        count = 0;
    }
};

// Hand-crafted Priority Order Queue for Express Orders
// Express orders get served before standard orders!
template <typename T, int CAPACITY>
class PriorityOrderQueue {
private:
    CircularQueue<T, CAPACITY> expressQueue;  // High priority queue
    CircularQueue<T, CAPACITY> standardQueue; // Normal priority queue

public:
    PriorityOrderQueue() {}

    bool enqueue(const T& item, bool isExpress) {
        if (isExpress) {
            return expressQueue.enqueue(item);
        } else {
            return standardQueue.enqueue(item);
        }
    }

    // Dequeue serves express orders first, then standard orders
    bool dequeue(T& outItem, bool& wasExpress) {
        if (!expressQueue.isEmpty()) {
            wasExpress = true;
            return expressQueue.dequeue(outItem);
        }
        if (!standardQueue.isEmpty()) {
            wasExpress = false;
            return standardQueue.dequeue(outItem);
        }
        return false;
    }

    bool peek(T& outItem, bool& wasExpress) const {
        if (!expressQueue.isEmpty()) {
            wasExpress = true;
            return expressQueue.peek(outItem);
        }
        if (!standardQueue.isEmpty()) {
            wasExpress = false;
            return standardQueue.peek(outItem);
        }
        return false;
    }

    int totalSize() const {
        return expressQueue.size() + standardQueue.size();
    }

    int expressSize() const {
        return expressQueue.size();
    }

    int standardSize() const {
        return standardQueue.size();
    }

    const CircularQueue<T, CAPACITY>& getExpressQueue() const {
        return expressQueue;
    }

    const CircularQueue<T, CAPACITY>& getStandardQueue() const {
        return standardQueue;
    }
};

#endif // ARRAY_QUEUE_HPP
