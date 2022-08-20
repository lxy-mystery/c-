#include "Timer.h"
#include "uv.h"
static void timer_cb(uv_timer_t* handle) {
    Timer* obj = (Timer*) handle->data;
    obj->callback();
}

Timer* setTimeout(std::function<void()> func, uint32_t ms)
{
    return new Timer(func, ms, 0);
}

Timer* setInterval(std::function<void()> func, uint32_t ms)
{
    return new Timer(func, ms, ms);
}

Timer::Timer(std::function<void()> func, uint32_t ms, uint32_t repeat):callback_(func), status_(0)
{
    uv_timer_init(uv_default_loop(), &instance_);
    instance_.data = this;
    uv_timer_start(&instance_, timer_cb, ms, repeat);
}

Timer::~Timer()
{
    if (status_ == 0) {
        uv_timer_stop(&instance_);
    }
}

void Timer::callback()
{
    callback_();
    // if only run once kill myself
    if (uv_timer_get_repeat(&instance_) == 0) {
        delete this;
    }
}

void Timer::stop()
{
    if (status_ == 0) {
        uv_timer_stop(&instance_);
        status_ = 1;
    }
}
