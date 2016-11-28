// Copyright (c) 2016, Tamas Csala

#ifndef COMMON_SETTINGS_HPP_
#define COMMON_SETTINGS_HPP_

#include <memory>
#include <functional>

namespace Settings {

static constexpr double kEpsilon = 1e-5;

}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template<typename T>
std::unique_ptr<T> make_unique() {
  return std::unique_ptr<T>{new T{}};
}

template<typename T>
constexpr T Sqr(T v) {
  return v * v;
}

template<typename T>
constexpr T Cube(T v) {
  return v * v * v;
}

#endif
