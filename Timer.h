#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include "uv.h"
class Timer
{
public:
    Timer(std::function<void()> func, uint32_t ms, uint32_t repeat);

    ~Timer();

    void callback();

    void stop();
private:
    uv_timer_t                      instance_;
    std::function<void()>           callback_;
    uint32_t                        status_;
};

Timer* setTimeout(std::function<void()> func, uint32_t ms);

Timer* setInterval(std::function<void()> func, uint32_t ms);

#endif //__TIMER_H__
