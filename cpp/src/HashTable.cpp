#include "HashTable.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace lr3 {

std::size_t djb2(const std::string& key) {
    std::size_t h = 5381;
    for (unsigned char c : key) h = h * 33 + c;
    return h;
}

HashTable::HashTable(std::size_t buckets) : buckets_(std::max<std::size_t>(buckets, 1), nullptr) {}

HashTable::HashTable(const HashTable& other) : buckets_(other.buckets_.size(), nullptr) {
    for (Node* head : other.buckets_) {
        for (Node* n = head; n != nullptr; n = n->next) insert(n->key, n->value);
    }
}

HashTable::HashTable(HashTable&& other) noexcept : buckets_(std::move(other.buckets_)), size_(other.size_) {
    other.buckets_.assign(1, nullptr);
    other.size_ = 0;
}

HashTable& HashTable::operator=(const HashTable& other) {
    if (this == &other) return *this;
    HashTable copy(other);
    *this = std::move(copy);
    return *this;
}

HashTable& HashTable::operator=(HashTable&& other) noexcept {
    if (this == &other) return *this;
    HashTable::clear();
    buckets_ = std::move(other.buckets_);
    size_ = other.size_;
    other.buckets_.assign(1, nullptr);
    other.size_ = 0;
    return *this;
}

HashTable::~HashTable() { HashTable::clear(); }

bool HashTable::contains(const std::string& key) const {
    const std::size_t idx = djb2(key) % buckets_.size();
    for (Node* n = buckets_[idx]; n != nullptr; n = n->next) {
        if (n->key == key) return true;
    }
    return false;
}

const std::string& HashTable::get(const std::string& key) const {
    const std::size_t idx = djb2(key) % buckets_.size();
    for (Node* n = buckets_[idx]; n != nullptr; n = n->next) {
        if (n->key == key) return n->value;
    }
    throw std::out_of_range("HashTable: key not found");
}

bool HashTable::insert(const std::string& key, const std::string& value) {
    const std::size_t idx = djb2(key) % buckets_.size();
    for (Node* n = buckets_[idx]; n != nullptr; n = n->next) {
        if (n->key == key) {
            n->value = value;  // обновляем значение существующего ключа
            return false;
        }
    }
    if ((size_ + 1) * 4 > buckets_.size() * 3) rehash(buckets_.size() * 2);
    const std::size_t newIdx = djb2(key) % buckets_.size();
    buckets_[newIdx] = new Node(key, value, buckets_[newIdx]);
    ++size_;
    return true;
}

bool HashTable::erase(const std::string& key) {
    const std::size_t idx = djb2(key) % buckets_.size();
    Node* prev = nullptr;
    for (Node* n = buckets_[idx]; n != nullptr; prev = n, n = n->next) {
        if (n->key == key) {
            if (prev == nullptr) buckets_[idx] = n->next;
            else prev->next = n->next;
            delete n;
            --size_;
            return true;
        }
    }
    return false;
}

void HashTable::rehash(std::size_t newCount) {
    std::vector<Node*> fresh(newCount, nullptr);
    for (Node* head : buckets_) {
        Node* n = head;
        while (n != nullptr) {
            Node* next = n->next;
            const std::size_t idx = djb2(n->key) % newCount;
            n->next = fresh[idx];
            fresh[idx] = n;
            n = next;
        }
    }
    buckets_ = std::move(fresh);
}

std::size_t HashTable::maxChainLength() const {
    std::size_t mx = 0;
    for (Node* head : buckets_) {
        std::size_t len = 0;
        for (Node* n = head; n != nullptr; n = n->next) ++len;
        mx = std::max(mx, len);
    }
    return mx;
}

std::vector<std::string> HashTable::toVector() const {
    std::vector<std::string> out;
    out.reserve(size_);
    for (Node* head : buckets_) {
        for (Node* n = head; n != nullptr; n = n->next) {
            out.push_back(n->key + "=" + n->value);
        }
    }
    return out;
}

void HashTable::clear() {
    for (Node*& head : buckets_) {
        while (head != nullptr) {
            Node* next = head->next;
            delete head;
            head = next;
        }
    }
    size_ = 0;
}

void HashTable::append(const std::string& entry) {
    const std::size_t pos = entry.find('=');
    if (pos == std::string::npos) insert(entry, "");
    else insert(entry.substr(0, pos), entry.substr(pos + 1));
}

}  