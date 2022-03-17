#include <iostream>
#include <thread>
#include <numeric>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <future>

template <typename Iterator, typename Func>
void for_each_block(Iterator first, Iterator last, Func f) {
    std::for_each(first, last, f);
}

template<typename Iterator, typename Func>
void parallel_for_each_without_order(Iterator first, Iterator last, Func f) {
    auto length = std::distance(first, last);
    if (length < 32){
        std::for_each(first, last, f);
        return;
    }

    size_t num_workers = std::thread::hardware_concurrency();
    auto length_per_thread = length / num_workers;

    std::vector<std::future<void>> futures;

    auto left_border = first;
    auto right_border = std::next(first, length%num_workers + length_per_thread);
    //начальный сдвиг, что бы только первому потоку достался больший диапозон


    for(auto i = 0u; i < num_workers-1; i++) {
        futures.push_back(std::async(std::launch::async,
                for_each_block<Iterator, Func>,
                left_border, right_border, f));

        left_border = right_border;
        right_border = std::next(right_border, length_per_thread);
    }

    std::for_each(left_border, last, f);
}


using namespace std;
int main(){
    vector<int> x = {1, 3, -1, 0, 2};
    parallel_for_each_without_order(x.begin(), x.end(), [](auto& a){a *= 2;});
    for(auto el : x) cout << el << " "; cout << endl;

    parallel_for_each_without_order(x.begin(), x.end(), [](auto& a){a = a/2 + 10;});
    for(auto el : x) cout << el << " "; cout << endl;

    return 0;
}
