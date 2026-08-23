#pragma once
#include <chrono>
namespace Learner
{
class Clock
{
private:
    std::chrono::high_resolution_clock::time_point startTime;

public:
    Clock() = default;
    void start()
    {
        startTime = std::chrono::high_resolution_clock::now();
    }
    void reset()
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    double elapsed() const
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = currentTime - startTime;
        return duration.count();
    }
};
}