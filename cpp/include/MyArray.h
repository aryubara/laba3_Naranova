#ifndef LR3_MYARRAY_H
#define LR3_MYARRAY_H

#include <cstddef>
#include <string>
#include <vector>
#include "IContainer.h"

using namespace std;

namespace lr3 {

// Класс динамического массива строк.
// При нехватке места автоматически увеличивает выделенную память в 2 раза.
class MyArray : public IContainer {
public:
    // --- Конструкторы, присваивание и деструктор (Правило пяти) ---

    // Создаёт пустой массив с заданной начальной вместимостью (по умолчанию 2).
    explicit MyArray(size_t initialCapacity = 2);

    // Конструктор копирования (создаёт полную копию другого массива).
    MyArray(const MyArray& other);

    // Конструктор перемещения (забирает ресурсы у временного объекта без копирования).
    MyArray(MyArray&& other) noexcept;

    // Оператор копирующего присваивания.
    MyArray& operator=(const MyArray& other);

    // Оператор перемещающего присваивания.
    MyArray& operator=(MyArray&& other) noexcept;

    // Деструктор (освобождает выделенную динамическую память).
    ~MyArray() override;


    // --- Основные методы работы с массивом ---

    // Возвращает текущую вместимость (сколько элементов влезет без перевыделения памяти).
    size_t capacity() const { 
        return capacity_; 
    }

    // Добавляет новую строку в конец массива.
    void pushBack(const string& value);

    // Вставляет строку по указанному индексу, сдвигая остальные элементы вправо.
    // Допускается вставка в самый конец (когда index == size()).
    void insertAt(size_t index, const string& value);

    // Возвращает строку по индексу (только для чтения).
    const string& getAt(size_t index) const;

    // Заменяет существующую строку по указанному индексу на новую.
    void setAt(size_t index, const string& value);

    // Удаляет элемент по индексу, сдвигая последующие элементы влево.
    void removeAt(size_t index);


    // --- Методы интерфейса IContainer ---

    // Возвращает строковое название типа контейнера.
    string typeName() const override { 
        return "MyArray"; 
    }

    // Возвращает текущее количество элементов в массиве.
    size_t size() const override { 
        return size_; 
    }

    // Преобразует содержимое массива в стандартный vector.
    vector<string> toVector() const override;

    // Очищает массив (сбрасывает счётчик элементов, но не освобождает память).
    void clear() override { 
        size_ = 0; 
    }

    // Добавляет элемент в конец (реализация общего метода интерфейса).
    void append(const string& value) override { 
        pushBack(value); 
    }

private:
    // --- Вспомогательные внутренние методы ---

    // Увеличивает вместимость массива в 2 раза и переносит старые данные в новую память.
    void grow();

    // Проверяет, не выходит ли индекс за допустимые границы массива.
    void checkIndex(size_t index) const;


    // --- Поля класса ---

    string* data_;     // Указатель на динамический массив строк в памяти
    size_t capacity_;  // Максимальное количество элементов до следующего расширения памяти
    size_t size_;      // Текущее количество реально хранимых элементов
};

} // namespace lr3

#endif // LR3_MYARRAY_H
