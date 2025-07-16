/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UNIQUEQUEUE_H
#define UNIQUEQUEUE_H
#include <queue>
#include <unordered_set>

/**
 * @class UniqueQueue
 * @brief A queue that ensures each element is inserted only once.
 *
 * This data structure behaves like a standard FIFO queue but prevents duplicate
 * insertions.
 *
 * Internally, it uses a `std::queue` for ordering and a `std::unordered_set` to
 * track seen elements.
 *
 * @tparam T The type of elements stored in the queue. Must be hashable and
 * comparable.
 */
template <typename T> class UniqueQueue {
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
    const T& front() const { return queue_.front(); }

    /**
     * @brief Checks whether the queue is empty.
     * @return true if the queue is empty; false otherwise.
     */
    bool empty() const { return queue_.empty(); }

    /**
     * @brief Clears the queue and the set of seen elements.
     */
    void clear() {
        while (!queue_.empty())
            queue_.pop();
        seen_.clear();
    }

  private:
    std::queue<T>         queue_; ///< Underlying FIFO queue.
    std::unordered_set<T> seen_;  ///< Set of already inserted elements.
};
#endif // UNIQUEQUEUE_H
