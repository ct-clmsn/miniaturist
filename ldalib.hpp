//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __HPXLDA_HPP__
#define __HPXLDA_HPP__

#include <cstdint>
#include <blaze/Math.h>

using blaze::CompressedMatrix;
using blaze::DynamicMatrix;

void train_lda(
    CompressedMatrix<double> const& dwcm,
    DynamicMatrix<double> & tdcm,
    DynamicMatrix<double> & twcm,
    std::vector<std::size_t> &tokens,
    const std::size_t n_topics, const std::size_t iterations, const double alpha, const double beta);

#endif
