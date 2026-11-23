#include "Stack.h"

#include <algorithm>

namespace lr3 {

std::vector<std::string> Stack::toVector() const {
    if (list_.empty()) return {};
    std::vector<std::string> out = list_.toVector();
    std::reverse(out.begin(), out.end());
    return out;
}

}  // namespace lr3
