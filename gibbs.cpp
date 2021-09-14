//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "gibbs.hpp"

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
    drand & dr,
    const std::size_t n_topics, const double N, const double alpha, const double beta) {

    const std::size_t n_docs = dwcm.rows();
    const double wbeta = N * beta;

    std::vector<std::size_t>::iterator token_itr = tokens.begin();

    for(std::size_t d = 0; d < n_docs; ++d) {
        const auto dwcm_end = dwcm.end(d);
        for(CompressedMatrix<double, blaze::rowMajor>::ConstIterator it = dwcm.begin(d); it != dwcm_end; ++it) {
            const std::size_t w = it->index();
            const std::size_t k_max = static_cast<std::size_t>(std::floor(it->value()));
            for(std::size_t k = 0; k < k_max; k++) {
                // decrement twcm, tdcm, ztot
                //
                //assert(token_itr != token_end);
                std::size_t t = (*token_itr);

                //assert(twcm(t, w) >= 0.0);
                //assert(tdcm(t, d) >= 0.0);
                //assert(ztot[t] >= 0.0);

                ztot[t] -= 1.0;
                twcm(t, w) -= 1.0;
                tdcm(t, d) -= 1.0;

                //double totprob = 0.0;
                //for(std::size_t i = 0; i < n_topics; ++i) {
                //    probs[i] = ((twcm(i,w) + beta) * (tdcm(i,d) + alpha)) / (ztot[i] + wbeta);
                //    totprob += probs[i];
                //}

                probs = ((blaze::column(twcm, w) + beta) * (blaze::column(tdcm,d) + alpha)) / (ztot + wbeta);
                const double totprob = blaze::sum(probs);
                const double maxprob = totprob * dr();
                std::size_t nt = 0;
                double curprob = probs[nt];

                while(curprob < maxprob) {
                    ++nt;
                    curprob += probs[nt];
                }

                nt = (nt >= n_topics) ? (nt % n_topics) : nt;

                (*token_itr) = nt;
                ztot[nt] += 1.0;
                twcm(nt, w) += 1.0;
                tdcm(nt, d) += 1.0;
                ++token_itr;
            }
        }
    }
}
