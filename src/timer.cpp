#include "timer.hpp"

Timer::Timer() : _previousTime(0)
{
    _previousTime = glfwGetTime();
}

float Timer::elapsedTime() const
{
    return glfwGetTime() - _previousTime;
}

void Timer::restart()
{
    _previousTime = glfwGetTime();
}
