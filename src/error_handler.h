#pragma once
#include <functional>
#include <string_view>

namespace cuber {
using ErrorHandler = std::function<void(std::string_view)>;
}
