// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <asio/associator.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

// This wrapper is used to add logic around a completion token
// while exposing its associated characteristics.
template<typename Wrapper, typename Token>
struct completion_wrapper final
{
  template<typename Self, typename... Args>
  decltype(auto) operator()(this Self&& self, Args&&... args)
  {
    return std::forward<Self>(self).wrapper_(
        std::forward<Args>(args)..., std::forward<Self>(self).completion_);
  }

  [[no_unique_address]] Wrapper wrapper_;
  [[no_unique_address]] Token completion_;
};

template<typename Wrapper, typename Token>
auto
wrap_completion(Wrapper&& wrapper, Token&& completion)
{
  return completion_wrapper{ std::forward<Wrapper>(wrapper),
                             std::forward<Token>(completion) };
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht

template<template<typename, typename> class Associator,
         typename Wrapper,
         typename Token,
         typename DefaultCandidate>
struct asio::associator<Associator,
                        ks::dht::detail::completion_wrapper<Wrapper, Token>,
                        DefaultCandidate> : Associator<Token, DefaultCandidate>
{
  static auto get(
      ks::dht::detail::completion_wrapper<Wrapper, Token> const& b) noexcept
  {
    return Associator<Token, DefaultCandidate>::get(b.completion_);
  }

  static auto get(ks::dht::detail::completion_wrapper<Wrapper, Token> const& b,
                  DefaultCandidate const& c) noexcept
  {
    return Associator<Token, DefaultCandidate>::get(b.completion_, c);
  }
};
