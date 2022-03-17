#include <iostream>

#include <thread>
#include <numeric>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include "Timer.h"

static std::mt19937_64 gen{static_cast<unsigned long long>(std::chrono::system_clock::now().time_since_epoch().count())};

template <typename Iterator, typename T>
void accumulate_block(Iterator first, Iterator last, T init, T& result) {
    result = std::accumulate(first, last, init);
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init, size_t num_workers) {
    auto length = std::distance(first, last);
    if (length < 32) return std::accumulate(first, last, init);

    auto length_per_thread = length / num_workers;

    std::vector<std::thread> threads;
    std::vector<T> results(num_workers - 1);

    auto left_border = first;
    auto right_border = std::next(first, length%num_workers + length_per_thread);
    //начальный сдвиг, что бы только первому потоку достался больший диапозон

    for(auto i = 0u; i < num_workers - 1; i++) {
        threads.push_back(std::thread(
                accumulate_block<Iterator, T>,
                left_border, right_border, 0, std::ref(results[i])));

        left_border = right_border;
        right_border = std::next(right_border, length_per_thread);
    }
    auto main_result = std::accumulate(left_border, last, init);
    // std::mem_fun_ref -- для оборачивания join().
    std::for_each(std::begin(threads), std::end(threads), std::mem_fun_ref(&std::thread::join));
    // 5. Собираем результаты
    return std::accumulate(std::begin(results), std::end(results), main_result);
}

//создает рандомный вектор, сумма любого промежутка которoго, по модулю меньше заданого числа
std::vector<int> create_special(int n, int amplitude, int roof = 1'000'000){
    int sum_from_start = 0; //будем держать это значение в отрезке [-roof/2, roof/2]

    //проверка на адекватность данных
    roof = (abs(roof) < 2*amplitude) ? 2*amplitude : abs(roof);

    //чтобы шаг был равен amplitude: |amplitude * rnd| <= roof/2
    std::uniform_int_distribution<int> rnd(-roof/2/amplitude, roof/2/amplitude);

    std::vector<int> result(n);

    for(int i = 0; i < n; i++){
        int chose = amplitude * rnd(gen);

        if(2*(sum_from_start + chose) < -roof || roof < 2*(sum_from_start + chose)) chose *= -1;

        result[i] = chose;
        sum_from_start += chose;
    }

    return result;
}


using namespace std;
int main() {
    int N = 50;
    int count_experements = 100;
    int size = 1'000'000;
    int amplitude = 5;
    int roof = 1'000'000;

    vector<int> data(N, 0); //будут записана сумарное время всех count_experements

    for(size_t e = 0; e < count_experements; e++){
        auto subject = create_special(size, amplitude, roof);
        int expect = accumulate(subject.begin(), subject.end(), 0);

        for(size_t core = 1; core <= N; core++){
            Timer<microseconds> t;
            int calculate = parallel_accumulate(subject.begin(), subject.end(), 0, core);
            data[core-1] += t.Get();

            if(calculate != expect) throw runtime_error(to_string(calculate) + " != " + to_string(expect));
        }
    }

    for(auto res : data) cout << res << " ";

    return 0;
}

