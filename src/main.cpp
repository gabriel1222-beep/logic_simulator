#include "circuit.hpp"
#include "gates.hpp"
#include "helper.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_error.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <format>
#include <iostream>
#include <sys/types.h>
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
        times.reserve(iterations + 1);
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
template <std::size_t reserve>
struct Ploter {
    std::vector<Measurements> measures = [] { std::vector<Measurements> v; v.reserve(reserve); return v; }();
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
        if (length >= 100) {
            length /= 2;
        }
        str.reserve(length * c.size());

        for (size_t i = 0; i < length; i++) {
            str += c;
        }
        return str;
    }
    template <typename T>
    void plot(std::string name, T value) {
        std::string character = "▀";
        std::cout
            << name << " "
            << "\x1b[91m" << std::format("{:.6f}", value)
            << " \x1b[93m"
            << std::format("{}", line(value, character))
            << "\x1b[0m\n";
    }
    void showAverages() {

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
};
bool SIMULATING_MODE = false;
int main() {
    Circuit circuit;
    auto andA = circuit.data.addComponent<Enums::Components_enum::And>();
    auto andB = circuit.data.addComponent<Enums::Components_enum::And>();
    auto orA = circuit.data.addComponent<Enums::Components_enum::Or>();
    auto notA = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto notB = circuit.data.addComponent<Enums::Components_enum::Not>();
    auto inA = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto inB = circuit.data.addComponent<Enums::Components_enum::ON_IN>();
    auto out = circuit.data.addComponent<Enums::Components_enum::Out>();
    circuit.data.connect(notA, 0, inA);
    circuit.data.connect(andA, 0, notA);
    circuit.data.connect(andA, 1, inB); // AndA = notA and B
    circuit.data.connect(notB, 0, inB);
    circuit.data.connect(andB, 0, notB);
    circuit.data.connect(andB, 1, inA); // AndB = notB and A
    circuit.data.connect(orA, 0, andA);
    circuit.data.connect(orA, 1, andB);
    circuit.data.connect(out, 0, orA);
    auto steps = 100000;
    std::printf("Number of steps per test -> %d\n\n", steps);
    std::vector<double> TableAvrgtime;
    TableAvrgtime.reserve(1000);
    std::vector<double> SwitchAvrgtime;
    SwitchAvrgtime.reserve(1000);
    Benchmarker benchmarker;
    std::vector<std::size_t> table_times;
    while (1) {
        std::cout << "\x1b[2J\x1b[H";
        auto table_time = benchmarker.averageExecutionTime("Table version", steps, [&] {
            circuit.stepSimulationTable();
        });
        table_times.push_back(table_time.average_t);
        Ploter<2> ploter;
        ploter.add(table_time);
        ploter.showAverages();
        std::cout << "\n";
        ploter.plot("Table average time          ", average(table_times));
    }

    return 0;
}
