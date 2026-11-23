#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
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

namespace {

V sorted(const IContainer& c) {
    V v = c.toVector();
    std::sort(v.begin(), v.end());
    return v;
}

template <typename T>
class SerializationTest : public ::testing::Test {};

using AllTypes = ::testing::Types<MyArray, SinglyLinkedList, DoublyLinkedList, Queue, Stack, HashTable, CompleteBinaryTree>;
TYPED_TEST_SUITE(SerializationTest, AllTypes);

}  // namespace
/*сохраняет каждый из 7 контейнеров в бинарный формат и проверяет, 
что после загрузки данные полностью совпали.*/
TYPED_TEST(SerializationTest, BinaryRoundTrip) {
    TypeParam src;
    for (const char* v : {"5", "-3", "42", "7", "0"}) src.append(v);
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    src.saveBinary(ss);
    TypeParam dst;
    dst.append("999");  // старое содержимое должно быть заменено
    dst.loadBinary(ss);
    EXPECT_EQ(sorted(dst), sorted(src));
    EXPECT_EQ(dst.size(), src.size());
}
/*то же самое, но для текстового формата.*/
TYPED_TEST(SerializationTest, TextRoundTrip) {
    TypeParam src;
    for (const char* v : {"1", "2", "3", "4"}) src.append(v);
    std::stringstream ss;
    src.saveText(ss);
    TypeParam dst;
    dst.loadText(ss);
    EXPECT_EQ(sorted(dst), sorted(src));
}
/*проверяет, что пустые контейнеры корректно 
сохраняются и загружаются, очищая старые данные.*/
TYPED_TEST(SerializationTest, EmptyRoundTrip) {
    TypeParam src;
    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);
    TypeParam a;
    TypeParam b;
    a.append("1");
    b.append("1");
    a.loadBinary(bin);
    b.loadText(txt);
    EXPECT_TRUE(a.empty());
    EXPECT_TRUE(b.empty());
}
/*обрезает бинарный файл на середине и проверяет,
 что загрузка выдаёт ошибку, а старые данные в контейнере не стираются.*/
TYPED_TEST(SerializationTest, TruncatedBinaryFailsAndKeepsData) {
    TypeParam src;
    src.append("10");
    src.append("20");
    std::stringstream full(std::ios::in | std::ios::out | std::ios::binary);
    src.saveBinary(full);
    const std::string bytes = full.str();
    for (std::size_t cut : {std::size_t{0}, std::size_t{2}, std::size_t{5}, bytes.size() - 1}) {
        std::stringstream bad(bytes.substr(0, cut), std::ios::in | std::ios::binary);
        TypeParam dst;
        dst.append("77");
        EXPECT_THROW(dst.loadBinary(bad), std::runtime_error) << "cut=" << cut;
        EXPECT_EQ(dst.size(), 1U);  // содержимое не пострадало
    }
}
/*проверяет, что обрезанный текстовый файл вызывает ошибку при загрузке.*/
TYPED_TEST(SerializationTest, TruncatedTextFails) {
    TypeParam src;
    src.append("10");
    src.append("20");
    std::stringstream txt;
    src.saveText(txt);
    std::string s = txt.str();
    s.resize(s.size() - 3);  // обрезаем последнюю строку и перевод строки
    std::stringstream bad(s.substr(0, s.rfind('\n')));
    TypeParam dst;
    EXPECT_THROW(dst.loadText(bad), std::runtime_error);
}
/*пытается загрузить сохранённый Stack в Queue и проверяет,
 что программа блокирует подмену типов.*/
TEST(Serialization, TypeMismatchRejected) {
    Stack s;
    s.append("1");
    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    s.saveBinary(bin);
    s.saveText(txt);
    Queue q;
    EXPECT_THROW(q.loadBinary(bin), std::runtime_error);
    EXPECT_THROW(q.loadText(txt), std::runtime_error);
}
/*подсовывает файлы с неправильным паролем (не LR3B/LR3T)
 или пустой файл и проверяет, что они отклоняются*/
TEST(Serialization, BadMagicRejected) {
    Queue q;
    std::stringstream bin("XXXX1234", std::ios::in | std::ios::binary);
    EXPECT_THROW(q.loadBinary(bin), std::runtime_error);
    std::stringstream txt("HELLO Queue 0\n");
    EXPECT_THROW(q.loadText(txt), std::runtime_error);
    std::stringstream empty;
    EXPECT_THROW(q.loadText(empty), std::runtime_error);
}
/*подсовывает файлы с неправильным паролем (не LR3B/LR3T) 
или пустой файл и проверяет, что они отклоняются*/
TEST(Serialization, BadEscapeAndHugeLengthRejected) {
    MyArray a;
    std::stringstream t1("LR3T MyArray 1\nabc\\");
    EXPECT_THROW(a.loadText(t1), std::runtime_error);
    std::stringstream t2("LR3T MyArray 1\nab\\q\n");
    EXPECT_THROW(a.loadText(t2), std::runtime_error);

    std::string huge("LR3B", 4);
    huge += static_cast<char>(7);
    huge += "MyArray";
    huge += std::string("\x01\x00\x00\x00", 4);  // один элемент
    huge += std::string("\xff\xff\xff\xff", 4);  // длина 4 ГБ
    std::stringstream h(huge, std::ios::in | std::ios::binary);
    EXPECT_THROW(a.loadBinary(h), std::runtime_error);
}
/*проверяет, что строки с переносами \n, слэшами \, 
пробелами и русскими буквами сохраняются и читаются без искажений.*/
TEST(Serialization, SpecialCharactersSurvive) {
    MyArray a;
    a.append("hello world");
    a.append("line1\nline2");
    a.append("back\\slash");
    a.append("cr\rhere");
    a.append("");
    a.append("юникод ✓");
    std::stringstream txt;
    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    a.saveText(txt);
    a.saveBinary(bin);
    MyArray t;
    MyArray b;
    t.loadText(txt);
    b.loadBinary(bin);
    EXPECT_EQ(t.toVector(), a.toVector());
    EXPECT_EQ(b.toVector(), a.toVector());
}
/*проверяет, что после сохранения и загрузки в стеке, 
очереди и списке сохранился точный порядок элементов.*/
TEST(Serialization, OrderPreservedForOrderedContainers) {
    Stack s;
    Queue q;
    DoublyLinkedList d;
    for (const char* v : {"c", "a", "b"}) {
        s.push(v);
        q.push(v);
        d.pushBack(v);
    }
    std::stringstream ss;
    std::stringstream qs;
    std::stringstream ds;
    s.saveText(ss);
    q.saveText(qs);
    d.saveText(ds);
    Stack s2;
    Queue q2;
    DoublyLinkedList d2;
    s2.loadText(ss);
    q2.loadText(qs);
    d2.loadText(ds);
    EXPECT_EQ(s2.pop(), "b");
    EXPECT_EQ(q2.pop(), "c");
    EXPECT_EQ(d2.toVector(), (V{"c", "a", "b"}));
}
/*проверяет, что двоичное дерево после бинарной загрузки
 восстанавливается с точно такой же структурой и обходами.*/
TEST(Serialization, TreeStructureRestored) {
    CompleteBinaryTree t;
    for (int i = 1; i <= 10; ++i) t.insert(i * 3);
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    t.saveBinary(ss);
    CompleteBinaryTree r;
    r.loadBinary(ss);
    EXPECT_EQ(r.levelOrder(), t.levelOrder());
    EXPECT_EQ(r.inOrder(), t.inOrder());
    EXPECT_TRUE(r.isComplete());
}
/* проверяет, что дерево отказывается загружать 
текстовый файл, если внутри записаны буквы вместо чисел.
*/
TEST(Serialization, TreeRejectsNonNumericText) {
    CompleteBinaryTree t;
    std::stringstream ss("LR3T CompleteBinaryTree 1\nabc\n");
    EXPECT_THROW(t.loadText(ss), std::invalid_argument);
}
// ---------- тесты сериализации для структур ----------

TEST(ExplicitSerialization, MyArrayBinaryAndText) {
    MyArray src;
    src.pushBack("alpha");
    src.pushBack("beta");
    src.pushBack("gamma");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    MyArray fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.toVector(), (V{"alpha", "beta", "gamma"}));
    EXPECT_EQ(fromTxt.toVector(), (V{"alpha", "beta", "gamma"}));
}

TEST(ExplicitSerialization, SinglyLinkedListBinaryAndText) {
    SinglyLinkedList src;
    src.pushBack("first");
    src.pushBack("second");
    src.pushFront("zero");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    SinglyLinkedList fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.toVector(), (V{"zero", "first", "second"}));
    EXPECT_EQ(fromTxt.toVector(), (V{"zero", "first", "second"}));
}

TEST(ExplicitSerialization, DoublyLinkedListBinaryAndText) {
    DoublyLinkedList src;
    src.pushBack("one");
    src.pushBack("two");
    src.pushBack("three");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    DoublyLinkedList fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.toVector(), (V{"one", "two", "three"}));
    EXPECT_EQ(fromTxt.toVectorReversed(), (V{"three", "two", "one"}));
}

TEST(ExplicitSerialization, StackBinaryAndText) {
    Stack src;
    src.push("bottom");
    src.push("middle");
    src.push("top");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    Stack fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.pop(), "top");
    EXPECT_EQ(fromBin.pop(), "middle");
    EXPECT_EQ(fromTxt.top(), "top");
}

TEST(ExplicitSerialization, QueueBinaryAndText) {
    Queue src;
    src.push("client1");
    src.push("client2");
    src.push("client3");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    Queue fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.pop(), "client1");
    EXPECT_EQ(fromBin.pop(), "client2");
    EXPECT_EQ(fromTxt.front(), "client1");
}

TEST(ExplicitSerialization, HashTableKeyValueBinaryAndText) {
    HashTable src;
    src.insert("name", "Alice");
    src.insert("city", "Novosibirsk");
    src.insert("formula", "a=b+c");

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    HashTable fromBin, fromTxt;
    fromBin.insert("old", "data");
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_EQ(fromBin.size(), 3U);
    EXPECT_FALSE(fromBin.contains("old"));
    EXPECT_EQ(fromBin.get("name"), "Alice");
    EXPECT_EQ(fromBin.get("city"), "Novosibirsk");
    EXPECT_EQ(fromBin.get("formula"), "a=b+c");

    EXPECT_EQ(fromTxt.size(), 3U);
    EXPECT_EQ(fromTxt.get("name"), "Alice");
    EXPECT_EQ(fromTxt.get("city"), "Novosibirsk");
    EXPECT_EQ(fromTxt.get("formula"), "a=b+c");
}

TEST(ExplicitSerialization, CompleteBinaryTreeBinaryAndText) {
    CompleteBinaryTree src;
    for (int v : {10, 20, 30, 40, 50}) src.insert(v);

    std::stringstream bin(std::ios::in | std::ios::out | std::ios::binary);
    std::stringstream txt;
    src.saveBinary(bin);
    src.saveText(txt);

    CompleteBinaryTree fromBin, fromTxt;
    fromBin.loadBinary(bin);
    fromTxt.loadText(txt);

    EXPECT_TRUE(fromBin.isComplete());
    EXPECT_EQ(fromBin.levelOrder(), (std::vector<int>{10, 20, 30, 40, 50}));
    EXPECT_EQ(fromTxt.levelOrder(), (std::vector<int>{10, 20, 30, 40, 50}));
}

