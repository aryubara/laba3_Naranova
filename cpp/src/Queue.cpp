#include "Queue.h"

namespace lr3 {

void Queue::push(const std::string& value) {
    list_.pushBack(value);
}

std::string Queue::pop() {
    return list_.popFront();
}

const std::string& Queue::front() const {
    return list_.front();
}

std::string Queue::typeName() const {
    return "Queue";
}

std::size_t Queue::size() const {
    return list_.size();
}

std::vector<std::string> Queue::toVector() const {
    return list_.toVector();
}

void Queue::clear() {
    list_.clear();
}

void Queue::append(const std::string& value) {
    push(value);
}

}  // namespace lr3
