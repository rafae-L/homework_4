#ifndef TRY_TO_GIT_TIMER_H
#define TRY_TO_GIT_TIMER_H

#include <chrono>
#include <iostream>

using namespace std::chrono;

template<typename T>
class Timer {
public:
    Timer() {}

//    ~Timer() {
//        steady_clock::time_point finish = steady_clock::now();
//
//        std::cout << duration_cast<T>( current_foray + finish - start).count()  << std::endl;
//    }

    void Pause(){
        steady_clock::time_point interval = steady_clock::now();

        if(is_clock_work) {
            current_foray += interval - start;
            is_clock_work = false;
        }
    }

    void Resume(){
        if(!is_clock_work)
        {
            is_clock_work = true;
            start = steady_clock::now();
        }
    }

    int Get(){
        steady_clock::time_point finish = steady_clock::now();

        return duration_cast<T>( current_foray + finish - start).count();
    }

private:
    std::chrono::nanoseconds current_foray = std::chrono::nanoseconds(0);
    steady_clock::time_point start = steady_clock::now();
    bool is_clock_work = true;
};

#endif //TRY_TO_GIT_TIMER_H
