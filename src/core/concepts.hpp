#pragma once
#include <concepts>

namespace core
{
template <typename T>
concept int_or_float = std::integral<T> || std::floating_point<T>;
}
