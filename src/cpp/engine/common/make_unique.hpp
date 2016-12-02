// Copyright (c) Tamas Csala

#ifndef COMMON_MAKE_UNIQUE_HPP_
#define COMMON_MAKE_UNIQUE_HPP_

namespace engine {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template<typename T>
std::unique_ptr<T> make_unique() {
  return std::unique_ptr<T>{new T{}};
}

}

#endif
