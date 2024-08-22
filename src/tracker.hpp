// SPDX-License-Identifier: MIT

#pragma once

#include "constants.hpp"
#include "log.hpp"
#include "message.hpp"
#include "message_serializer.hpp"
#include "network.hpp"
#include "response_router.hpp"
#include "routing_table.hpp"
#include "value_store.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

/**
 *
 */
template<typename RandomEngineType, typename NetworkType>
class tracker final
{
public:
  ///
  using network_type = NetworkType;

  ///
  using endpoint_type = typename network_type::endpoint_type;

  ///
  using random_engine_type = RandomEngineType;

public:
  /**
   *
   */
  tracker(boost::asio::io_service& io_service,
          id const& my_id,
          network_type& network,
          random_engine_type& random_engine)
    : response_router_(io_service)
    , message_serializer_(my_id)
    , network_(network)
    , random_engine_(random_engine)
  {
  }

  /**
   *
   */
  tracker(tracker const&) = delete;

  /**
   *
   */
  tracker& operator=(tracker const&) = delete;

  /**
   *
   */
  template<typename Request, typename OnResponseReceived, typename OnError>
  void send_request(Request const& request,
                    endpoint_type const& e,
                    timer::duration const& timeout,
                    OnResponseReceived const& on_response_received,
                    OnError const& on_error)
  {
    id const response_id(random_engine_);
    // Generate the request buffer.
    auto message = message_serializer_.serialize(request, response_id);

    // This lamba will keep the request message alive.
    auto on_request_sent =
        [this, response_id, on_response_received, on_error, timeout](
            std::error_code const& failure) {
          if (failure)
            on_error(failure);
          else
            response_router_.register_temporary_callback(
                response_id, timeout, on_response_received, on_error);
        };

    // Serialize the request and send it.
    network_.send(message, e, on_request_sent);
  }

  /**
   *
   */
  template<typename Request>
  void send_request(Request const& request, endpoint_type const& e)
  {
    id const response_id(random_engine_);
    send_response(response_id, request, e);
  }

  /**
   *
   */
  template<typename Response>
  void send_response(id const& response_id,
                     Response const& response,
                     endpoint_type const& e)
  {
    auto message = message_serializer_.serialize(response, response_id);

    auto on_response_sent = [](std::error_code const& /* failure */) {};

    network_.send(message, e, on_response_sent);
  }

  /**
   *
   */
  void handle_new_response(endpoint_type const& s,
                           header const& h,
                           buffer::const_iterator i,
                           buffer::const_iterator e)
  {
    response_router_.handle_new_response(s, h, i, e);
  }

private:
  ///
  response_router response_router_;
  ///
  message_serializer message_serializer_;
  ///
  network_type& network_;
  ///
  random_engine_type& random_engine_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
