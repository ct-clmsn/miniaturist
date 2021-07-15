//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
#include <hpx/modules/collectives.hpp>
#include <hpx/numeric.hpp>
#include <hpx/algorithm.hpp>

#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>
//#include <cassert>

#include <unicode/unistr.h>
#include <blaze/Math.h>

#include "lda.hpp"
#include "documents.hpp"
#include "results.hpp"
#include "serialize.hpp"

namespace fs = std::experimental::filesystem;
using namespace hpx::collectives;

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

inline void distpar_gibbs(CompressedMatrix<double> const& dwcm, DynamicMatrix<double> & tdcm, DynamicMatrix<double> & twcm, std::vector<std::size_t> & tokens, DynamicVector<double> & ztot, DynamicVector<double> & probs, const std::size_t n_topics, const double N, const double alpha, const double beta) {

    const std::size_t n_docs = dwcm.rows();
    //const std::size_t n_tokens = tokens.size(); //dwcm.columns();
    const double wbeta = N * beta;

    std::vector<std::size_t>::iterator token_itr = tokens.begin();
    //const std::vector<std::size_t>::iterator token_end = tokens.end();

    for(std::size_t d = 0; d < n_docs; ++d) {
        //for(std::size_t w = 0; w < n_words; w++) {
        //    const std::size_t k_max = static_cast<std::size_t>(std::floor(dwcm(d,w)));
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
                const double maxprob = totprob * drand48();
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
            //const std::size_t n_words = dwcm[i].columns();

            std::vector<std::size_t>::iterator token_itr = tokens[i].begin();

            for(std::size_t d = 0; d < n_docs; ++d) {
                //for(std::size_t w = 0; w < n_words; w++) {
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

    DynamicMatrix<double> twcm_base(twcm[0].rows(), twcm[0].columns(), 0.0); 
    //twcm_base = hpx::reduce(hpx::execution::par, std::begin(twcm), std::end(twcm), twcm_base, adder);
    twcm_base = hpx::reduce(std::begin(twcm), std::end(twcm), twcm_base, adder);

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

        hpx::for_each(hpx::execution::par, std::begin(thread_idx), std::end(thread_idx), [&tokens, &dwcm, &tdcm, &twcm, &twcm_base, &ztot, &probs, n_topics, alpha, beta, N](const std::size_t i) {
        //std::for_each(std::begin(thread_idx), std::end(thread_idx), [&tokens, &dwcm, &tdcm, &twcm, &twcm_base, &ztot, &probs, n_topics, alpha, beta](const std::size_t i) {
            distpar_gibbs(dwcm[i], tdcm[i], twcm[i], tokens[i], ztot[i], probs[i], n_topics, N, alpha, beta);
            twcm[i] -= twcm_base;
        });

        //twcm_base = hpx::reduce(hpx::execution::par, std::begin(twcm), std::end(twcm), twcm_base, adder);
        twcm_base = hpx::reduce(std::begin(twcm), std::end(twcm), twcm_base, adder);

        hpx::future< DynamicMatrix<double> > overall_result =
            all_reduce(all_reduce_direct_client, twcm_base, std::plus<DynamicMatrix<double>>{});

        twcm_base = overall_result.get();

        std::fill(std::begin(twcm), std::end(twcm), twcm_base);
    }
}

int hpx_main(hpx::program_options::variables_map & vm) {

    bool exit = false;
    if(vm.count("vocab_list") == 0) {
        std::cerr << "Please specify '--vocab_list=vocabulary-file'" << std::endl;
        exit = true;
    }

    if(vm.count("corpus_dir") == 0) {
        std::cerr << "Please specify '--corpus_dir=corpus-directory'" << std::endl;
        exit = true;
    }

    if(vm.count("num_topics") == 0) {
        std::cerr << "Please specify '--num_topics=unsigned-integer-value'" << std::endl;
        exit = true;
    }

    if(exit) {
        return hpx::finalize();
    }

    UnicodeString regexp(UnicodeString::fromUTF8(vm["regex"].as<std::string>())); //u"[\\p{L}\\p{M}]+");

    fs::path pth{vm["corpus_dir"].as<std::string>()};

    const std::size_t n_threads = hpx::resource::get_num_threads("default");
    const std::size_t n_topics = vm["num_topics"].as<std::size_t>();
    const std::size_t iterations = vm["num_iters"].as<std::size_t>();
    const double alpha = vm["alpha"].as<double>();
    const double beta = vm["beta"].as<double>();

    const std::vector<hpx::id_type> localities = hpx::find_all_localities();
    const size_t n_locales = localities.size();
    const std::size_t locality_id = hpx::get_locality_id();

    std::unordered_map<std::string, std::size_t> vocabulary;

    fs::path wpth{vm["vocab_list"].as<std::string>()};

    const std::size_t vocab_sz = load_wordlist(wpth, vocabulary);

    std::vector< CompressedMatrix<double> > dwcm(n_threads);
    std::vector< DynamicMatrix<double> > tdcm(n_threads), twcm(n_threads);

    std::vector< std::size_t > thread_idx(n_threads);
    std::iota(std::begin(thread_idx), std::end(thread_idx), 0);


    std::vector< std::vector<std::size_t> > tokens(n_threads);
    std::vector< std::tuple<std::size_t, std::size_t> > doc_chunks(n_threads);

    {
        std::vector< fs::path > paths;

        // sort out locale file portion
        //
        {
            std::vector< fs::path > locale_paths;
            const std::size_t n_paths = path_to_vector( pth, locale_paths );
            const std::size_t chunk_sz = n_paths / n_locales;
            const std::size_t base = locality_id * chunk_sz;
            const std::tuple<std::size_t, std::size_t> locale_dp{base, ( locality_id != (n_locales-1) ) ? (base + chunk_sz) : n_paths};
            //const std::tuple<std::size_t, std::size_t> locale_dp{base, base + chunk_sz + (( locality_id != (n_locales-1) ) ? 0 : (n_paths % n_locales))};
            const std::size_t locale_doc_diff = std::get<1>(locale_dp)-std::get<0>(locale_dp);
            paths.resize(locale_doc_diff);
            std::copy_n(std::begin(locale_paths)+std::get<0>(locale_dp), locale_doc_diff, std::begin(paths));
        }

        const std::size_t n_paths = paths.size(); //path_to_vector( pth, paths );
        const std::size_t chunk_sz = n_paths / n_threads;

        std::vector< fs::path >::iterator paths_itr = paths.begin();
        std::vector< inverted_index_t > ii(n_threads);

        for(const std::size_t i : thread_idx) {
            const std::size_t base = i * chunk_sz;
            std::tuple<std::size_t, std::size_t> dp{base,  ( i != (n_threads-1) ) ? (base + chunk_sz) : n_paths};

            const std::size_t doc_diff = std::get<1>(dp)-std::get<0>(dp);
            tdcm[i].resize( n_topics, doc_diff );
            twcm[i].resize( n_topics, vocab_sz );
            tdcm[i] = 0.0;
            twcm[i] = 0.0;

            doc_chunks[i] = std::move(dp);

            auto beg = paths_itr+std::get<0>(doc_chunks[i]);
            auto end = paths_itr+std::get<1>(doc_chunks[i]);
            const std::size_t nentries = document_path_to_inverted_index(beg, end, regexp, ii[i], vocabulary);
            const std::size_t ndocs = static_cast<std::size_t>(end-beg);
            CompressedMatrix<double> wdcm;
            inverted_index_to_matrix(vocabulary, ii[i], ndocs, nentries, wdcm);
            dwcm[i] = blaze::trans(wdcm);
            matrix_to_vector(dwcm[i], tokens[i]);
        }
    }

    distpar_train_lda(n_locales, locality_id, thread_idx, dwcm, tdcm, twcm, tokens, n_topics, iterations, alpha, beta);

    print_topics(vocabulary, twcm[0], n_topics);

    for(const std::size_t i : thread_idx) {
        const auto beg = std::get<0>(doc_chunks[i]);
        const auto end = std::get<1>(doc_chunks[i]);
        const std::size_t diff = static_cast<std::size_t>(end-beg);
        print_document_topics(tdcm[i], n_topics, 0, diff, 4);
    }

    //for(const auto& tc : tdcm) {
    //    std::cout << tc << std::endl;
    //}
    return hpx::finalize();
}

int main(int argc, char ** argv) {
    hpx::program_options::options_description desc("usage: parlda [options]");
    desc.add_options()("regex,re",
        hpx::program_options::value<std::string>()->default_value("[\\p{L}\\p{M}]+"),
        "regex (default: [\\p{L}\\p{M}]+]")("num_iters,n",
        hpx::program_options::value<std::size_t>()->default_value(1000),
        "number of iterations (default: 1000)")("alpha,a",
        hpx::program_options::value<double>()->default_value(0.1),
        "alpha parameter")("beta,b",
        hpx::program_options::value<double>()->default_value(0.011),
        "beta parameter")("num_topics,nt",
        hpx::program_options::value<std::size_t>(),
        "number of topics")("vocab_list,vl",
        hpx::program_options::value<std::string>(),
        "file path containing the vocabulary list")("corpus_dir,cd",
        hpx::program_options::value<std::string>(),
        "directory path containing the corpus to model");

    hpx::init_params params;
    params.desc_cmdline = desc;
    return hpx::init(argc, argv, params);
}

#endif
