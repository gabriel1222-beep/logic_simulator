#pragma once
#include <cstddef>
#include <vector>
template<typename T,std::size_t N,std::size_t heap_reserve = 0>
struct StackFirstVector{
  static_assert(N>0,"N must be grather than 0");
  std::vector<T> heap_storage;
  std::array<T,N> stack_storage;
  std::size_t size = 0;
  StackFirstVector(){
    heap_storage.reserve(heap_reserve);
  }
  void push_back(const T& data){
    if(size >= N){
      heap_storage.push_back(data);
      return;
    }
    stack_storage[size++] = data;
  }
  void pop_back(){
    if(size>=N){
      T data = heap_storage[heap_storage.size()-1];
      heap_storage.pop_back();
    }else if(size > 0){
      T data = stack_storage[size-1];
      size--;
    }
  }
  T* at(std::size_t i){
    if(size >= N){
      return &heap_storage[i-N];
    }
    return &stack_storage[i];
  }
  T cpy(std::size_t i){
    if(size >= N){
      return heap_storage[i-N];
    }
    return stack_storage[i];
  }
  void replace(std::size_t i,T &data){
    if(size >= N) heap_storage[i-N] = data; else stack_storage[i] = data;
  }
};
