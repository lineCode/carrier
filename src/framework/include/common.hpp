#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <algorithm>
#include <functional>
#include <system_error>

template <typename E>
inline void fail(const E& ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

#endif
