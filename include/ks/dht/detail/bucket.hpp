// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace ks::dht::inline abiv1::detail {

template<typename T, std::size_t N>
class bucket final
{
public:
  using value_type = T;

public:
  bucket() noexcept = default;

  bucket(bucket&& other) = default;

  bucket(bucket const& other) = delete;

  bucket& operator=(bucket&& other) = default;

  bucket& operator=(bucket const& other) = delete;

  template<typename Self>
  [[nodiscard]] auto begin(this Self&& self) noexcept
  {
    return std::forward<Self>(self).data_.begin();
  }

  template<typename Self>
  [[nodiscard]] auto end(this Self&& self) noexcept
  {
    return std::forward<Self>(self).data_.end();
  }

  template<typename Value>
  [[nodiscard]] T* try_push_back(Value&& value)
  {
    if (data_.size() == N)
      return nullptr;

    return &data_.emplace_back(std::forward<Value>(value));
  }

  auto erase(auto pos)
  {
    assert(pos != end());

    return data_.erase(pos);
  }

  template<typename Self>
  [[nodiscard]] auto data(this Self&& self) noexcept
  {
    return std::forward<Self>(self).data_.data();
  }

  template<typename Value>
  void push_back(Value&& value)
  {
    if (not try_push_back(std::forward<Value>(value))) [[unlikely]]
      throw std::bad_alloc{};
  }

  [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

private:
  std::vector<T> data_;
};

} // namespace ks::dht::abiv1::detail
