//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __GIBBS_HPP__

#include <vector>
#include <cmath>
#include <cstdint>

#include <blaze/Math.h>

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

void gibbs(
    CompressedMatrix<double> const& dwcm,
    DynamicMatrix<double> & tdcm,
    DynamicMatrix<double> & twcm,
    std::vector<std::size_t> & tokens, DynamicVector<double> & ztot,
    DynamicVector<double> & probs,
    const std::size_t n_topics, const double N, const double alpha, const double beta);

#endif
