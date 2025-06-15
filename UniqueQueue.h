#ifndef UNIQUEQUEUE_H
#define UNIQUEQUEUE_H
#include <queue>
#include <unordered_set>

/**
 * @class UniqueQueue
 * @brief A queue that ensures each element is inserted only once.
 *
 * This data structure behaves like a standard FIFO queue but prevents duplicate insertions.
 *
 * Internally, it uses a `std::queue` for ordering and a `std::unordered_set` to track seen elements.
 *
 * @tparam T The type of elements stored in the queue. Must be hashable and comparable.
 */
template<typename T>
class UniqueQueue {
public:
    /**
     * @brief Pushes an element to the queue if it hasn't been inserted before.
     * @param value The element to insert.
     */
    void push(const T& value) {
        if (seen_.insert(value).second) {
            queue_.push(value);
        }
    }

    /**
     * @brief Removes the front element from the queue.
     */
    void pop() {
        if (!queue_.empty()) {
            queue_.pop();
        }
    }

    /**
     * @brief Accesses the front element of the queue.
     * @return A reference to the front element.
     */
    const T& front() const {
        return queue_.front();
    }

    /**
     * @brief Checks whether the queue is empty.
     * @return true if the queue is empty; false otherwise.
     */
    bool empty() const {
        return queue_.empty();
    }

    /**
     * @brief Clears the queue and the set of seen elements.
     */
    void clear() {
        while(!queue_.empty()) queue_.pop();
        seen_.clear();
    }

private:
    std::queue<T> queue_;        ///< Underlying FIFO queue.
    std::unordered_set<T> seen_; ///< Set of already inserted elements.
};
#endif // UNIQUEQUEUE_H
