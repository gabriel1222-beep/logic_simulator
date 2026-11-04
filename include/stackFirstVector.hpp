#pragma once
#include <vector>
template<typename T,std::size_t N>
struct StackFistArray{

  std::vector<T> heap_storage;
  std::array<T,N> stack_storage;
  std::size_t size = 0;
  
  void push_back(T data){
    if(size >= N){
      heap_storage.push_back(data);
      return;
    }
    stack_storage[size++] = data;
  }

};
