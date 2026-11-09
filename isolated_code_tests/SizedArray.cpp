#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
template <typename Tp, std::size_t Nm>
struct SzArray {
    using size_type_t =
        std::conditional_t<
            (Nm <= std::numeric_limits<unsigned char>::max()),
            unsigned char,
            std::conditional_t<
                (Nm <= std::numeric_limits<unsigned short>::max()),
                unsigned short,
                std::conditional_t<
                    (Nm <= std::numeric_limits<unsigned int>::max()),
                    unsigned int,
                    std::size_t>>>;
    std::array<Tp, Nm> buffer = {};
    size_type_t size = 0;
    template <typename... U>
    constexpr SzArray(U &&...values)
        : buffer{std::forward<U>(values)...},
          size(sizeof...(U)) {
    }
    void push_back(Tp data) {
        if (size >= Nm) return;
        buffer[size] = data;
        size++;
    }
    void pop_back() {
        if (size > 0) size--;
    }
    Tp at(const size_type_t index) {
        if constexpr (index >= Nm) throw std::out_of_range("Index out of bounds");
        return buffer[index];
    }
    template <size_type_t index>
    Tp at() {
        static_assert(index < Nm, "Index out of bounds");
        return buffer[index];
    }
    Tp &operator[](std::size_t i) { return buffer[i]; }
    const Tp &operator[](std::size_t i) const { return buffer[i]; }
};
int main() {
    SzArray<int, 3> arr = {0,1,2};
    return 0;
}
