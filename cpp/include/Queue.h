#ifndef LR3_QUEUE_H
#define LR3_QUEUE_H

#include <cstddef>
#include <string>
#include <vector>

#include "IContainer.h"
#include "SinglyLinkedList.h"

namespace lr3 {

// Очередь FIFO на основе односвязного списка с хвостом: push и pop за O(1).
class Queue : public IContainer {
public:
    void push(const std::string& value);
    std::string pop();
    const std::string& front() const;

    std::string typeName() const override;
    std::size_t size() const override;
    std::vector<std::string> toVector() const override;
    void clear() override;
    void append(const std::string& value) override;

private:
    SinglyLinkedList list_;
};

}  // namespace lr3

#endif
