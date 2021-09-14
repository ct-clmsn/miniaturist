//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/modules/collectives.hpp>
#include <hpx/numeric.hpp>
#include <hpx/algorithm.hpp>

#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include <blaze/Math.h>

#include "distparldalib.hpp"
#include "gibbs.hpp"
#include "serialize.hpp"

using namespace hpx::collectives;

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

void distpar_train_lda(const std::size_t n_locales, 
                   const std::size_t locality_id,
                   const std::vector<std::size_t> & thread_idx,
                   std::vector< CompressedMatrix<double> > const& dwcm,
                   std::vector< DynamicMatrix<double> > & tdcm,
                   std::vector< DynamicMatrix<double> > & twcm,
                   std::vector< std::vector<std::size_t> > & tokens,
                   const std::size_t n_topics, const std::size_t iterations, const double alpha, const double beta) {

    const std::string all_reduce_direct_basename = "all_reduce_direct";
    auto all_reduce_direct_client = create_communicator(
        all_reduce_direct_basename.c_str(), num_sites_arg(n_locales), this_site_arg(locality_id)
    );

    const std::size_t n_threads = thread_idx.size();

    // build randomized topic-document-count-matrix and topic-word-count-matrix
    //
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dis(0, n_topics-1);

        for(const std::size_t i : thread_idx) {
            const std::size_t n_docs = dwcm[i].rows();

            std::vector<std::size_t>::iterator token_itr = tokens[i].begin();

            for(std::size_t d = 0; d < n_docs; ++d) {
                const auto dwcm_end = dwcm[i].end(d);
                for(CompressedMatrix<double, blaze::rowMajor>::ConstIterator it = dwcm[i].begin(d); it != dwcm_end; ++it) {
                    const std::size_t w = it->index();
                    const std::size_t k_max = static_cast<std::size_t>(std::floor(it->value()));
                    for(std::size_t k = 0; k < k_max; k++) {
                        const std::size_t top = dis(gen);
                        (*token_itr) = top;
                        tdcm[i](top, d) += 1.0;
                        twcm[i](top, w) += 1.0;
                        ++token_itr;
                    }
                }
            }
        }
    }

    std::plus< DynamicMatrix<double> > adder{};
    std::vector< DynamicVector<double> > ztot(n_threads);
    std::vector< DynamicVector<double> > probs(n_threads);
    std::vector< drand > drands(n_threads);

    DynamicMatrix<double> twcm_base(twcm[0].rows(), twcm[0].columns(), 0.0);
    DynamicMatrix<double> twcm_tmp(twcm[0].rows(), twcm[0].columns(), 0.0);

    twcm_base = hpx::reduce(std::begin(twcm), std::end(twcm), twcm_base, adder);

    // accumulate and distribute the global topic-word-count-matrix
    //
    {
        hpx::future< DynamicMatrix<double> > overall_result =
            hpx::collectives::all_reduce(all_reduce_direct_client, twcm_base, adder);

        twcm_base = overall_result.get();
    }

    for(const std::size_t ti : thread_idx) {
        twcm[ti] = twcm_base;
        ztot[ti].resize(n_topics);
        probs[ti].resize(n_topics);
        ztot[ti] = 0.0;
        probs[ti] = 0.0;
    }

    double N = 0.0;
    {
        std::size_t Nsz = 0;
        for(const auto& t : tokens) {
            Nsz += t.size();
        }

        N = static_cast<double>(Nsz);
    }

    for(std::size_t i = 0; i < iterations; ++i) {
        ztot[0] = blaze::sum<blaze::rowwise>(twcm_base);
        std::fill(std::begin(ztot), std::end(ztot), ztot[0]);

        hpx::for_each(hpx::execution::par, std::begin(thread_idx), std::end(thread_idx), [&tokens, &dwcm, &tdcm, &twcm, &twcm_base, &ztot, &probs, &drands, n_topics, alpha, beta, N](const std::size_t i) {
            gibbs(dwcm[i], tdcm[i], twcm[i], tokens[i], ztot[i], probs[i], drands[i], n_topics, N, alpha, beta);
            twcm[i] -= twcm_base;
        });

        // store local differences
        //
        twcm_tmp = hpx::reduce(std::begin(twcm), std::end(twcm), twcm_tmp, adder);

        // combine global differences
        //
        hpx::future< DynamicMatrix<double> > overall_result =
            hpx::collectives::all_reduce(all_reduce_direct_client, twcm_tmp, adder);

        twcm_tmp = overall_result.get();

        // add totall differences into local base value
        //
        twcm_base += twcm_tmp;

        // update all threads
        //
        std::fill(std::begin(twcm), std::end(twcm), twcm_base);

        twcm_tmp = 0.0;
    }
}
