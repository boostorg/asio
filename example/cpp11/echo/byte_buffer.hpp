///
/// Copyright (C) 2016 Xiaoshuang LU (luxiaoshuang at qiyi dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///

# ifndef _BYTE_BUFFER_HPP_
# define _BYTE_BUFFER_HPP_ 1

# include <cstdint>

///
/// Header Size
///     Only 1, 2, 4, and 8 are allowed.
///
///  Body Size
///     Arbitrary.
///
class byte_buffer
{
    public:
        uintmax_t header_length_;

        uintmax_t body_length_;

        // where header and body reside
        uint8_t * data_;
};

# endif
