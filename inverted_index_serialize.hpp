//  Copyright (c) 2021 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#pragma once
#ifndef INVERTED_INDEX_SERIALIZATION_HPP
#define INVERTED_INDEX_SERIALIZATION_HPP

#include <hpx/serialization/unordered_map.hpp>
#include <hpx/include/util.hpp>
#include <cstddef>

#include "inverted_index.hpp"

namespace hpx { namespace serialization
{
    template <typename T, bool TF>
    void load(
        input_archive& archive, inverted_index_t& target, unsigned)
    {
        archive >> target;
    }

    template <typename T, bool TF>
    void save(output_archive& archive,
        inverted_index_t const& target, unsigned)
    {
        archive << target;
    }
}}

#endif
