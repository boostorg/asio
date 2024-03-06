//
// line_reader.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef LINE_READER_HPP
#define LINE_READER_HPP

#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/error.hpp>
#include <string>

class line_reader
{
public:
  virtual ~line_reader() {}

  template <typename CompletionToken>
  auto async_read_line(std::string prompt, CompletionToken&& token)
  {
    return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code, std::string)>(
        [](auto handler, line_reader* self, std::string prompt)
        {
          self->async_read_line_impl(std::move(prompt), std::move(handler));
        }, token, this, prompt);
  }

private:
  virtual void async_read_line_impl(std::string prompt,
      boost::asio::any_completion_handler<void(boost::system::error_code, std::string)> handler) = 0;
};

#endif // LINE_READER_HPP
