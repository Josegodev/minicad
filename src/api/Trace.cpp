#include "langcad/api/Trace.hpp"

#include <atomic>
#include <chrono>
#include <sstream>

namespace langcad::api {

std::string makeTraceId() {
    static std::atomic<unsigned long long> counter{0};

    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    std::ostringstream stream;
    stream << "trace-" << millis << "-" << counter.fetch_add(1);
    return stream.str();
}

} // namespace langcad::api
