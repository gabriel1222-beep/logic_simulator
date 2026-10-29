#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
struct Measurements {
    double average_t;
    std::string name;
    std::size_t smallest_t;
    std::size_t biggest_t;
    std::string getAverage_t() {
        std::string str;
        if (average_t >= 1000000) {
            str = std::format("{:.6f} ms", average_t / 1000000);
        } else if (average_t >= 1000) {
            str = std::format("{:.6f} µs", average_t / 1000);
        } else {
            str = std::format("{:.6f} ns", average_t);
        }
        return str;
    }
    void print() {
        std::cout << "Benchmark: \x1b[93m" << name << "\x1b[0m\n\n";
        if (smallest_t >= 1000000) {
            std::cout << "Smallest time: " << smallest_t / 1000000 << " ms" << "\n";
        } else {
            std::cout << "Smallest time: " << smallest_t << " ns" << "\n";
        }
        if (biggest_t >= 1000000) {
            std::cout << "Biggest time: " << biggest_t / 1000000 << " ms" << "\n";
        } else {
            std::cout << "Biggest time: " << biggest_t << " ns" << "\n";
        }
        if (average_t >= 1000000) {
            std::cout << "Average time: " << average_t / 1000000 << " ms" << "\n";
        } else {
            std::cout << "Average time: " << average_t << " ns" << "\n";
        }
    }
};
struct Benchmarker {
    template <class F>
    std::size_t executionTimeMS(F &&f) {
        const auto start = std::chrono::steady_clock::now();
        std::forward<F>(f)();
        const auto stop = std::chrono::steady_clock::now();
        auto time = stop - start;
        return static_cast<std::size_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
    }
    template <class F>
    std::size_t executionTimeNS(F &&f) {
        const auto start = std::chrono::steady_clock::now();
        std::forward<F>(f)();
        const auto stop = std::chrono::steady_clock::now();
        const auto time = stop - start;
        return static_cast<std::size_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(time).count());
    }
    template <class F>
    Measurements averageExecutionTime(std::string bench_name, std::size_t iterations, F &&f) {
        std::vector<std::size_t> times;
        times.reserve(iterations);
        double time_sum = 0;
        for (std::size_t i = 0; i < iterations; i++) {
            auto time = executionTimeNS(f);
            time_sum += time;
            times.push_back(time);
        }
        Measurements measures;
        measures.name = bench_name;
        measures.average_t = time_sum / times.size();
        measures.biggest_t = *std::max_element(times.begin(), times.end());
        measures.smallest_t = *std::min_element(times.begin(), times.end());
        return measures;
    }
};
struct Ploter {
    std::vector<Measurements> measures;
    void add(Measurements &measure) {
        measures.push_back(measure);
    }
    std::size_t getLongestNameSize() {
        auto longest = std::max_element(
            measures.begin(), measures.end(),
            [](auto &a, auto &b) {
                return a.name.size() < b.name.size();
            });
        return longest == measures.end() ? 0 : longest->name.size();
    }
    std::size_t getbiggestAvrgValueStrSize() {
        auto biggest = std::max_element(
            measures.begin(), measures.end(),
            [](auto &a, auto &b) {
                return a.average_t < b.average_t;
            });
        return biggest == measures.end() ? 0 : std::format("{}", biggest->average_t).size();
    }
    std::string line(std::size_t length, std::string &c) {
        std::string str;
        str.reserve(length * c.size());

        for (size_t i = 0; i < length; i++) {
            str += c;
        }
        return str;
    }
    void showAveragesA() {

        std::string character = "▀";
        std::size_t longest_name_length = getLongestNameSize();
        std::size_t longest_value_length = getbiggestAvrgValueStrSize();
        std::size_t w = longest_name_length + longest_value_length;
        for (auto &measure : measures) {
            std::cout
                << std::format("{:<{}}", measure.name, longest_name_length + 1)
                << "\x1b[91m" << measure.getAverage_t()
                << " \x1b[93m"
                << std::format("{:>{}}", line(measure.average_t, character), w)
                << "\x1b[0m\n";
        }
    }
    void showAveragesB() {
        std::string character = "▀";
        std::size_t longest_name_length = getLongestNameSize();
        std::size_t longest_value_length = getbiggestAvrgValueStrSize();
        for (auto &measure : measures) {
            std::cout
                << std::format("{:<{}}", measure.name, longest_name_length + 1)
                << "\x1b[91m" << measure.getAverage_t()
                << " \x1b[93m"
                << line(measure.average_t, character)
                << "\x1b[0m\n";
        }
    }
};
template<typename T>
T add(T A, T B) {
    return A + B;
}
template<typename T>
void add(T *res,T *A, T *B) {
    *res = *A + *B;
}
int main() {
    Benchmarker benchmkr;
    int tests = 10000000;
    while (1) {
        std::cout<<"\x1b[2J\x1b[H";
        auto time1 = benchmkr.averageExecutionTime("SumA", tests, [&] {
            auto n = add(1, 1);
        });
        int a = 4;
        int b = 4;
        int res;
        auto time2 = benchmkr.averageExecutionTime("SumB", tests, [&] {
            add(&res,&a, &b);
        });
        Ploter ploter;
        ploter.add(time1);
        ploter.add(time2);
        ploter.showAveragesA();
    }

    return 0;
}
