#include "IContainer.h"

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>

using namespace std;

namespace lr3 {

namespace {

const char kMagicBin[4] = {'L', 'R', '3', 'B'}; //пароль для бин файла
const char kMagicText[] = "LR3T";//пароль для текст файла
const uint32_t kMaxLen = 64U * 1024U * 1024U;  // одна строка не может быть больше 64мб

//берет больш число и режет его на 4 кусочка по 1 байту
void writeU32(ostream& os, uint32_t v) {
    const char b[4] = {static_cast<char>(v & 0xFFU), static_cast<char>((v >> 8) & 0xFFU),
                       static_cast<char>((v >> 16) & 0xFFU), static_cast<char>((v >> 24) & 0xFFU)};
    os.write(b, 4);//запись этих 4 кусков в файл
}
//читает эти 4 куска и склеивает обратно в одно число
uint32_t readU32(istream& is) {
    unsigned char b[4];
    is.read(reinterpret_cast<char*>(b), 4);
    if (!is) throw runtime_error("unexpected end of binary data");
    return static_cast<uint32_t>(b[0]) | (static_cast<uint32_t>(b[1]) << 8) |
           (static_cast<uint32_t>(b[2]) << 16) | (static_cast<uint32_t>(b[3]) << 24);
}
//один эл контенера=одна строчка в текстовом файле
//убирает все символы переноса перед записью в файл
string escape(const string& s) {
    string out;
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}
//возвращает символы обратно при чтении из файла
string unescape(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] != '\\') {
            out += s[i];
            continue;
        }
        if (++i >= s.size()) throw runtime_error("dangling escape in text data");
        if (s[i] == '\\') out += '\\';
        else if (s[i] == 'n') out += '\n';
        else if (s[i] == 'r') out += '\r';
        else throw runtime_error("unknown escape in text data");
    }
    return out;
}

} 
//печать элементов
void IContainer::print(ostream& os) const {
    const vector<string> items = toVector();
    os << '[';
    for (size_t i = 0; i < items.size(); ++i) {
        if (i != 0) os << ", ";
        os << items[i];
    }
    os << "]\n";
}
//функция сохрания бин файла
void IContainer::saveBinary(ostream& os) const {
    const string name = typeName();
    const vector<string> items = toVector();
    os.write(kMagicBin, 4);
    os.put(static_cast<char>(name.size()));
    os.write(name.data(), static_cast<streamsize>(name.size()));
    writeU32(os, static_cast<uint32_t>(items.size()));
    for (const string& s : items) {
        writeU32(os, static_cast<uint32_t>(s.size()));
        os.write(s.data(), static_cast<streamsize>(s.size()));
    }
    if (!os) throw runtime_error("write error");
}
//функция загрузки бин файла
void IContainer::loadBinary(istream& is) {
    char magic[4];
    is.read(magic, 4);
    if (!is || string(magic, 4) != string(kMagicBin, 4)) throw runtime_error("bad binary magic");
    const int nameLen = is.get();
    if (nameLen == istream::traits_type::eof()) throw runtime_error("unexpected end of binary data");
    string name(static_cast<size_t>(nameLen), '\0');
    is.read(&name[0], nameLen);
    if (!is) throw runtime_error("unexpected end of binary data");
    if (name != typeName()) throw runtime_error("type mismatch: file has " + name + ", expected " + typeName());

    const uint32_t count = readU32(is);
    vector<string> items;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t len = readU32(is);
        if (len > kMaxLen) throw runtime_error("element too large");
        string s(len, '\0');
        is.read(&s[0], static_cast<streamsize>(len));
        if (!is) throw runtime_error("unexpected end of binary data");
        items.push_back(std::move(s));
    }
    clear();  // контейнер меняется только после успешного чтения всего файла
    for (const string& s : items) append(s);
}
//сохрание текст файла
void IContainer::saveText(ostream& os) const {
    const vector<string> items = toVector();
    os << kMagicText << ' ' << typeName() << ' ' << items.size() << '\n';
    for (const string& s : items) os << escape(s) << '\n';
    if (!os) throw runtime_error("write error");
}
//загрузка текст файла
void IContainer::loadText(istream& is) {
    string magic;
    string name;
    size_t count = 0;
    if (!(is >> magic >> name >> count) || magic != kMagicText) throw runtime_error("bad text header");
    if (name != typeName()) throw runtime_error("type mismatch: file has " + name + ", expected " + typeName());
    string line;
    getline(is, line);  // остаток строки заголовка
    vector<string> items;
    for (size_t i = 0; i < count; ++i) {
        if (!getline(is, line)) throw runtime_error("unexpected end of text data");
        items.push_back(unescape(line));
    }
    clear();
    for (const string& s : items) append(s);
}

} 
