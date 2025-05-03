#ifndef UNIQUEQUEUE_H
#define UNIQUEQUEUE_H
#include <queue>
#include <unordered_set>

template<typename T>
class UniqueQueue {
public:
    void push(const T& value) {
        if (seen_.insert(value).second) {
            queue_.push(value);
        }
    }

    void pop() {
        if (!queue_.empty()) {
            queue_.pop();
        }
    }

    const T& front() const {
        return queue_.front();
    }

    bool empty() const {
        return queue_.empty();
    }

    void clear() {
        while(!queue_.empty()) queue_.pop();
    }

private:
    std::queue<T> queue_;
    std::unordered_set<T> seen_;
};
#endif // UNIQUEQUEUE_H
