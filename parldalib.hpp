//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __PARLDALIB_HPP__
#include <vector>
#include <cmath>
#include <cstdint>

#include <unicode/unistr.h>
#include <blaze/Math.h>

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

void par_train_lda(const std::vector<std::size_t> & thread_idx,
                   std::vector< CompressedMatrix<double> > const& dwcm,
                   std::vector< DynamicMatrix<double> > & tdcm,
                   std::vector< DynamicMatrix<double> > & twcm,
                   std::vector< std::vector<std::size_t> > & tokens,
                   const std::size_t n_topics, const std::size_t iterations, const double alpha, const double beta);
#endif
