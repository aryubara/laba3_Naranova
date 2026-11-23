#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "CompleteBinaryTree.h"
#include "DoublyLinkedList.h"
#include "HashTable.h"
#include "MyArray.h"
#include "Queue.h"
#include "SinglyLinkedList.h"
#include "Stack.h"

using namespace lr3;
using V = std::vector<std::string>;

// ---------- MyArray ----------
/*добавляет 10 элементов и проверяет, 
что массив сам расширился и правильно возвращает
 элемент по индексу*/
TEST(MyArray, PushGrowAndGet) {
    MyArray a(1);
    for (int i = 0; i < 10; ++i) a.pushBack(std::to_string(i));
    EXPECT_EQ(a.size(), 10U);
    EXPECT_GE(a.capacity(), 10U);
    EXPECT_EQ(a.getAt(7), "7");
    EXPECT_FALSE(a.empty());
}
/*проверяет, что при создании массива 
с нулевой вместимостью программа не падает,
 а выделяет память под 1 элемент.*/
TEST(MyArray, ZeroCapacityIsFixed) {
    MyArray a(0);
    a.pushBack("x");
    EXPECT_EQ(a.getAt(0), "x");
}
/*проверяет вставку в середину/начало (insertAt),
 замену (setAt) и удаление (removeAt) со сдвигом элементов.*/
TEST(MyArray, InsertSetRemove) {
    MyArray a;
    a.pushBack("a");
    a.pushBack("c");
    a.insertAt(1, "b");
    a.insertAt(3, "d");
    a.insertAt(0, "z");
    EXPECT_EQ(a.toVector(), (V{"z", "a", "b", "c", "d"}));
    a.setAt(0, "y");
    a.removeAt(1);
    a.removeAt(3);
    EXPECT_EQ(a.toVector(), (V{"y", "b", "c"}));
}
/*проверяет, что при обращении к несуществующему
 индексу выбрасывается ошибка out_of_range*/
TEST(MyArray, OutOfRangeThrows) {
    MyArray a;
    EXPECT_THROW(a.getAt(0), std::out_of_range);
    EXPECT_THROW(a.setAt(0, "x"), std::out_of_range);
    EXPECT_THROW(a.removeAt(0), std::out_of_range);
    EXPECT_THROW(a.insertAt(1, "x"), std::out_of_range);
}
/*проверяет глубокое копирование (копия не портит оригинал),
 самоприсваивание и быстрое перемещение (std::move).*/
TEST(MyArray, CopyAndMove) {
    MyArray a;
    a.pushBack("1");
    a.pushBack("2");
    MyArray b(a);
    b.pushBack("3");
    EXPECT_EQ(a.size(), 2U);
    EXPECT_EQ(b.size(), 3U);
    MyArray c;
    c = b;
    EXPECT_EQ(c.toVector(), b.toVector());
    MyArray& self = c;
    c = self;
    EXPECT_EQ(c.size(), 3U);
    MyArray d(std::move(c));
    EXPECT_EQ(d.size(), 3U);
    EXPECT_EQ(c.size(), 0U);
    MyArray e;
    e = std::move(d);
    EXPECT_EQ(e.size(), 3U);
    e.pushBack("again");  // после перемещения объект-источник и приёмник остаются рабочими
    d.pushBack("reuse");
    EXPECT_EQ(d.size(), 1U);
    { auto& same = e; e = std::move(same); }
    EXPECT_EQ(e.size(), 4U);
}

// ---------- SinglyLinkedList ----------
/* проверяет добавление и удаление с головы и хвоста,
 а также ошибку при попытке взять элемент из пустого списка.*/
TEST(SinglyList, PushAndPop) {
    SinglyLinkedList l;
    l.pushBack("b");
    l.pushFront("a");
    l.pushBack("c");
    EXPECT_EQ(l.toVector(), (V{"a", "b", "c"}));
    EXPECT_EQ(l.front(), "a");
    EXPECT_EQ(l.back(), "c");
    EXPECT_EQ(l.popBack(), "c");
    EXPECT_EQ(l.popFront(), "a");
    EXPECT_EQ(l.popBack(), "b");
    EXPECT_TRUE(l.empty());
    EXPECT_THROW(l.popFront(), std::out_of_range);
    EXPECT_THROW(l.popBack(), std::out_of_range);
    EXPECT_THROW(l.front(), std::out_of_range);
    EXPECT_THROW(l.back(), std::out_of_range);
}
/* проверяет вставку нового элемента до и после заданного значения.*/
TEST(SinglyList, InsertAfterBefore) {
    SinglyLinkedList l;
    EXPECT_FALSE(l.insertBefore("x", "y"));
    EXPECT_FALSE(l.insertAfter("x", "y"));
    l.pushBack("a");
    l.pushBack("c");
    EXPECT_TRUE(l.insertAfter("c", "d"));
    EXPECT_TRUE(l.insertAfter("a", "b"));
    EXPECT_TRUE(l.insertBefore("a", "0"));
    EXPECT_TRUE(l.insertBefore("c", "bb"));
    EXPECT_FALSE(l.insertBefore("none", "q"));
    EXPECT_EQ(l.toVector(), (V{"0", "a", "b", "bb", "c", "d"}));
    EXPECT_EQ(l.back(), "d");
}
/*проверяет все способы удаления:
 по значению, до элемента и после элемента (включая голову и хвост).*/
TEST(SinglyList, RemoveVariants) {
    SinglyLinkedList l;
    EXPECT_FALSE(l.removeByValue("a"));
    EXPECT_FALSE(l.removeBefore("a"));
    for (const char* s : {"a", "b", "c", "d", "e"}) l.pushBack(s);
    EXPECT_TRUE(l.removeByValue("a"));   // голова
    EXPECT_TRUE(l.removeByValue("e"));   // хвост
    EXPECT_FALSE(l.removeByValue("zz"));
    EXPECT_EQ(l.toVector(), (V{"b", "c", "d"}));
    EXPECT_EQ(l.back(), "d");
    EXPECT_TRUE(l.removeAfter("c"));     // удаляет хвост
    EXPECT_EQ(l.back(), "c");
    EXPECT_FALSE(l.removeAfter("c"));
    EXPECT_FALSE(l.removeAfter("none"));
    l.pushBack("d");
    l.pushBack("e");
    EXPECT_TRUE(l.removeBefore("c"));    // удаляет голову
    EXPECT_EQ(l.toVector(), (V{"c", "d", "e"}));
    EXPECT_TRUE(l.removeBefore("e"));    // удаляет из середины
    EXPECT_EQ(l.toVector(), (V{"c", "e"}));
    EXPECT_FALSE(l.removeBefore("c"));   // перед головой ничего нет
    EXPECT_FALSE(l.removeBefore("none"));
}
/*проверяет поиск по индексу,
 проверку наличия элемента (contains), 
 копирование и перемещение списка.*/
TEST(SinglyList, GetContainsCopyMove) {
    SinglyLinkedList l;
    l.pushBack("x");
    l.pushBack("y");
    EXPECT_EQ(l.getAt(1), "y");
    EXPECT_THROW(l.getAt(2), std::out_of_range);
    EXPECT_TRUE(l.contains("x"));
    EXPECT_FALSE(l.contains("q"));
    SinglyLinkedList c(l);
    c.pushBack("z");
    EXPECT_EQ(l.size(), 2U);
    SinglyLinkedList d;
    d = c;
    { auto& same = d; d = same; }
    EXPECT_EQ(d.size(), 3U);
    SinglyLinkedList m(std::move(d));
    EXPECT_EQ(m.size(), 3U);
    SinglyLinkedList n;
    n.pushBack("old");
    n = std::move(m);
    EXPECT_EQ(n.toVector(), (V{"x", "y", "z"}));
    { auto& same = n; n = std::move(same); }
    EXPECT_EQ(n.size(), 3U);
}

// ---------- DoublyLinkedList ----------
/*проверяет добавление/удаление с обоих концов 
и обратный вывод списка (toVectorReversed).*/
TEST(DoublyList, PushPop) {
    DoublyLinkedList l;
    l.pushBack("b");
    l.pushFront("a");
    l.pushBack("c");
    EXPECT_EQ(l.toVector(), (V{"a", "b", "c"}));
    EXPECT_EQ(l.toVectorReversed(), (V{"c", "b", "a"}));
    EXPECT_EQ(l.front(), "a");
    EXPECT_EQ(l.back(), "c");
    EXPECT_EQ(l.popFront(), "a");
    EXPECT_EQ(l.popBack(), "c");
    EXPECT_EQ(l.popBack(), "b");
    EXPECT_THROW(l.popFront(), std::out_of_range);
    EXPECT_THROW(l.popBack(), std::out_of_range);
    EXPECT_THROW(l.front(), std::out_of_range);
    EXPECT_THROW(l.back(), std::out_of_range);
}
/*проверяет вставку и удаление элементов 
до/после заданного узла с сохранением связей в обе стороны.*/
TEST(DoublyList, InsertAndRemove) {
    DoublyLinkedList l;
    EXPECT_FALSE(l.insertAfter("x", "y"));
    EXPECT_FALSE(l.insertBefore("x", "y"));
    l.pushBack("b");
    EXPECT_TRUE(l.insertBefore("b", "a"));
    EXPECT_TRUE(l.insertAfter("b", "d"));
    EXPECT_TRUE(l.insertAfter("b", "c"));
    EXPECT_TRUE(l.insertBefore("c", "bb"));
    EXPECT_EQ(l.toVector(), (V{"a", "b", "bb", "c", "d"}));
    EXPECT_EQ(l.toVectorReversed(), (V{"d", "c", "bb", "b", "a"}));
    EXPECT_TRUE(l.removeAfter("a"));
    EXPECT_FALSE(l.removeAfter("d"));
    EXPECT_FALSE(l.removeAfter("none"));
    EXPECT_TRUE(l.removeBefore("d"));
    EXPECT_FALSE(l.removeBefore("a"));
    EXPECT_FALSE(l.removeBefore("none"));
    EXPECT_EQ(l.toVector(), (V{"a", "bb", "d"}));
    EXPECT_TRUE(l.removeByValue("a"));
    EXPECT_FALSE(l.removeByValue("a"));
    EXPECT_EQ(l.size(), 2U);
}
/*проверяет поиск, копирование, 
самоприсваивание и перемещение двусвязного списка.*/
TEST(DoublyList, GetContainsCopyMove) {
    DoublyLinkedList l;
    l.pushBack("x");
    l.pushBack("y");
    EXPECT_EQ(l.getAt(0), "x");
    EXPECT_THROW(l.getAt(5), std::out_of_range);
    EXPECT_TRUE(l.contains("y"));
    EXPECT_FALSE(l.contains("w"));
    DoublyLinkedList c(l);
    DoublyLinkedList d;
    d = c;
    { auto& same = d; d = same; }
    EXPECT_EQ(d.toVectorReversed(), (V{"y", "x"}));
    DoublyLinkedList m(std::move(d));
    DoublyLinkedList n;
    n.pushBack("old");
    n = std::move(m);
    EXPECT_EQ(n.size(), 2U);
    { auto& same = n; n = std::move(same); }
    EXPECT_EQ(n.size(), 2U);
}

// ---------- Stack / Queue ----------
/*проверяет принцип стека (последним пришёл — первым ушёл)
 и правильный порядок выгрузки в вектор.*/
TEST(Stack, LifoOrder) {
    Stack s;
    EXPECT_THROW(s.pop(), std::out_of_range);
    EXPECT_THROW(s.top(), std::out_of_range);
    s.push("1");
    s.push("2");
    s.push("3");
    EXPECT_EQ(s.top(), "3");
    EXPECT_EQ(s.toVector(), (V{"1", "2", "3"}));  // от дна к вершине
    EXPECT_EQ(s.pop(), "3");
    EXPECT_EQ(s.pop(), "2");
    EXPECT_EQ(s.size(), 1U);
    s.clear();
    EXPECT_TRUE(s.empty());
}
/* проверяет принцип очереди (первым пришёл — первым ушёл)
 и повторное заполнение после полной очистки.*/
TEST(Queue, FifoOrder) {
    Queue q;
    EXPECT_THROW(q.pop(), std::out_of_range);
    EXPECT_THROW(q.front(), std::out_of_range);
    q.push("1");
    q.push("2");
    q.push("3");
    EXPECT_EQ(q.front(), "1");
    EXPECT_EQ(q.pop(), "1");
    EXPECT_EQ(q.toVector(), (V{"2", "3"}));
    q.pop();
    q.pop();
    q.push("again");  // после опустошения хвост должен быть корректен
    EXPECT_EQ(q.front(), "again");
    q.clear();
    EXPECT_TRUE(q.empty());
}

// ---------- HashTable ----------
/*проверяет добавление, запрет дубликатов, поиск и удаление ключей.*/
TEST(HashTable, InsertContainsErase) {
    HashTable h(0);
    EXPECT_TRUE(h.insert("a", "1"));
    EXPECT_FALSE(h.insert("a", "2"));  // ключ уже был — обновилось значение
    EXPECT_EQ(h.get("a"), "2");
    EXPECT_THROW(h.get("b"), std::out_of_range);
    EXPECT_TRUE(h.contains("a"));
    EXPECT_FALSE(h.contains("b"));
    EXPECT_TRUE(h.erase("a"));
    EXPECT_FALSE(h.erase("a"));
    EXPECT_TRUE(h.empty());
}
/*закидывает 500 ключей в таблицу из 2 ящичков и проверяет, 
что она сама расширилась, не потеряла данные и 
равномерно их распределила.*/
TEST(HashTable, RehashKeepsKeys) {
    HashTable h(2);
    const std::size_t before = h.bucketCount();
    for (int i = 0; i < 500; ++i) EXPECT_TRUE(h.insert("key" + std::to_string(i), "val" + std::to_string(i)));
    EXPECT_GT(h.bucketCount(), before);
    EXPECT_EQ(h.size(), 500U);
    for (int i = 0; i < 500; ++i) {
        EXPECT_TRUE(h.contains("key" + std::to_string(i)));
        EXPECT_EQ(h.get("key" + std::to_string(i)), "val" + std::to_string(i));
    }
    EXPECT_LE(h.maxChainLength(), 20U);
    V keys = h.toVector();
    EXPECT_EQ(keys.size(), 500U);
    h.clear();
    EXPECT_EQ(h.size(), 0U);
    EXPECT_FALSE(h.contains("key1"));
}
/*сверяет математический результат хеш-функции djb2 с эталонным значением.*/
TEST(HashTable, Djb2KnownValue) {
    EXPECT_EQ(djb2(""), 5381U);
    EXPECT_EQ(djb2("a"), 5381U * 33U + 'a');
}
TEST(HashTable, CopyMoveAndCollisions) {
    HashTable a(2);
    a.insert("k1", "v1");
    a.insert("k2", "v2");
    a.insert("k3", "v3");

    // 1. Проверка конструктора копирования и копирующего присваивания
    HashTable b(a);
    EXPECT_EQ(b.get("k1"), "v1");
    EXPECT_EQ(b.size(), 3U);

    HashTable c;
    c = b;
    { auto& same = c; c = same; }  // защита от самоприсваивания
    EXPECT_EQ(c.get("k2"), "v2");

    // 2. Проверка конструктора перемещения и перемещающего присваивания
    HashTable d(std::move(c));
    EXPECT_EQ(d.get("k3"), "v3");

    HashTable e;
    e.insert("old", "val");
    e = std::move(d);
    { auto& same = e; e = std::move(same); }  // самоперемещение
    EXPECT_EQ(e.size(), 3U);

    // 3. Проверка удаления из середины/конца цепочки коллизий (1 бакет)
    HashTable single(1);
    single.insert("first", "1");
    single.insert("second", "2");
    single.insert("third", "3");
    EXPECT_TRUE(single.erase("second"));  // удаление из середины цепочки (prev != nullptr)
    EXPECT_TRUE(single.erase("first"));   // удаление из хвоста цепочки
    EXPECT_EQ(single.size(), 1U);
    EXPECT_EQ(single.get("third"), "3");
}


// ---------- CompleteBinaryTree ----------
/*добавляет 7 чисел и проверяет высоту, 
полноту дерева и все 4 вида обхода (level, in, pre, post).*/
TEST(Tree, InsertKeepsComplete) {
    CompleteBinaryTree t;
    EXPECT_TRUE(t.isComplete());
    EXPECT_EQ(t.height(), -1);
    EXPECT_FALSE(t.find(1));
    for (int i = 1; i <= 7; ++i) {
        t.insert(i);
        EXPECT_TRUE(t.isComplete());
    }
    EXPECT_EQ(t.levelOrder(), (std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
    EXPECT_EQ(t.inOrder(), (std::vector<int>{4, 2, 5, 1, 6, 3, 7}));
    EXPECT_EQ(t.preOrder(), (std::vector<int>{1, 2, 4, 5, 3, 6, 7}));
    EXPECT_EQ(t.postOrder(), (std::vector<int>{4, 5, 2, 6, 7, 3, 1}));
    EXPECT_EQ(t.height(), 2);
    EXPECT_TRUE(t.find(7));
    EXPECT_FALSE(t.find(8));
    t.insert(8);
    EXPECT_EQ(t.height(), 3);
}
/*проверяет глубокое копирование, перемещение и очистку дерева в памяти.*/
TEST(Tree, CopyMoveClear) {
    CompleteBinaryTree t;
    for (int i = 0; i < 5; ++i) t.insert(i);
    CompleteBinaryTree c(t);
    c.insert(99);
    EXPECT_EQ(t.size(), 5U);
    CompleteBinaryTree d;
    d = c;
    { auto& same = d; d = same; }
    EXPECT_EQ(d.size(), 6U);
    CompleteBinaryTree m(std::move(d));
    CompleteBinaryTree n;
    n.insert(1);
    n = std::move(m);
    EXPECT_EQ(n.size(), 6U);
    { auto& same = n; n = std::move(same); }
    EXPECT_EQ(n.size(), 6U);
    n.clear();
    EXPECT_TRUE(n.empty());
}
/*проверяет, что дерево принимает числа в виде строк, 
но выбрасывает ошибку на буквы или слишком огромные числа*/
TEST(Tree, AppendValidation) {
    CompleteBinaryTree t;
    t.append("-5");
    EXPECT_TRUE(t.find(-5));
    EXPECT_THROW(t.append("abc"), std::invalid_argument);
    EXPECT_THROW(t.append("12x"), std::invalid_argument);
    EXPECT_THROW(t.append("99999999999999999999"), std::out_of_range);
}

// ---------- Полиморфизм через IContainer ----------
/*проверяет, что через ссылку на базовый класс 
IContainer вызываются правильные методы конкретного наследника (Queue).*/
TEST(Polymorphism, PrintThroughBase) {
    Queue q;
    q.push("a");
    q.push("b");
    const IContainer& base = q;
    std::ostringstream os;
    base.print(os);
    EXPECT_EQ(os.str(), "[a, b]\n");
    EXPECT_EQ(base.typeName(), "Queue");
}
