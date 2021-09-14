//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <unistd.h>
#include <getopt.h>
#include <iostream>

#include <unicode/unistr.h>
#include <blaze/Math.h>

#include "ldalib.hpp"
#include "gibbs.hpp"

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

void train_lda(CompressedMatrix<double> const& dwcm,
               DynamicMatrix<double> & tdcm,
               DynamicMatrix<double> & twcm,
               std::vector<std::size_t> & tokens,
               const std::size_t n_topics, const std::size_t iterations, const double alpha, const double beta) {

    // build randomized topic-document-count-matrix and topic-word-count-matrix
    //
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dis(0, n_topics-1);

        const std::size_t n_docs = dwcm.rows();

        std::vector<std::size_t>::iterator token_itr = tokens.begin();

        for(std::size_t d = 0; d < n_docs; ++d) {
            const auto dwcm_end = dwcm.end(d);
            for(CompressedMatrix<double, blaze::rowMajor>::ConstIterator it = dwcm.begin(d); it != dwcm_end; ++it) {
                const std::size_t w = it->index();
                const std::size_t k_max = static_cast<std::size_t>(std::floor(it->value()));
                for(std::size_t k = 0; k < k_max; k++) {
                    const std::size_t top = dis(gen);
                    (*token_itr) = top;
                    tdcm(top, d) += 1.0;
                    twcm(top, w) += 1.0;
                    ++token_itr;
                }
            }
        }
    }

    DynamicVector<double> ztot(n_topics, 0.0);
    DynamicVector<double> probs(n_topics, 0.0);
    const double N = static_cast<double>(tokens.size());
    drand dr{};

    for(std::size_t i = 0; i < iterations; ++i) {
        ztot = blaze::sum<blaze::rowwise>(twcm);
        gibbs(dwcm, tdcm, twcm, tokens, ztot, probs, dr, n_topics, N, alpha, beta);
    }
}
