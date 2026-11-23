#ifndef LR3_HASHTABLE_H
#define LR3_HASHTABLE_H

#include <cstddef>
#include <string>
#include <vector>

#include "IContainer.h"

namespace lr3 {

// Хеш-функция DJB2 (h = h * 33 + c).
std::size_t djb2(const std::string& key);

// Ассоциативный массив (хеш-таблица) пар "ключ — значение" с методом цепочек.
class HashTable : public IContainer {
public:
    explicit HashTable(std::size_t buckets = 8);
    HashTable(const HashTable& other);
    HashTable(HashTable&& other) noexcept;
    HashTable& operator=(const HashTable& other);
    HashTable& operator=(HashTable&& other) noexcept;
    ~HashTable() override;

    // Вставляет пару (key, value). Если ключ уже есть — обновляет значение и возвращает false.
    bool insert(const std::string& key, const std::string& value);
    bool erase(const std::string& key);
    bool contains(const std::string& key) const;
    const std::string& get(const std::string& key) const;  // бросает std::out_of_range, если ключа нет

    std::size_t bucketCount() const { return buckets_.size(); }
    std::size_t maxChainLength() const;

    std::string typeName() const override { return "HashTable"; }
    std::size_t size() const override { return size_; }
    std::vector<std::string> toVector() const override;  // формат "key=value"
    void clear() override;
    void append(const std::string& entry) override;  // парсит "key=value" (или "key" с пустым значением)

private:
    struct Node {
        std::string key;
        std::string value;
        Node* next;
        Node(std::string k, std::string v, Node* n = nullptr)
            : key(std::move(k)), value(std::move(v)), next(n) {}
    };

    void rehash(std::size_t newCount);
    std::vector<Node*> buckets_;
    std::size_t size_ = 0;
};

}  // namespace lr3

#endif
