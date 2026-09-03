#pragma once

#include <chrono>

namespace Timing
{
    using Clock = std::chrono::steady_clock;

    inline double ElapsedSeconds(Clock::time_point start,
                                 Clock::time_point end)
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
                   end - start)
            .count();
    }
}
