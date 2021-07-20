//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __HPXLDA_II_HPP__
#define __HPXLDA_II_HPP__

#include <string>
#include <unordered_map>
#include <cstdint>

using inverted_index_t = std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t> >;

#endif
