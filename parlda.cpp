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
#include <hpx/modules/collectives.hpp>
#include <hpx/numeric.hpp>
#include <hpx/program_options.hpp>
#include <hpx/algorithm.hpp>

#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdint>
//#include <cassert>

#include <unicode/unistr.h>
#include <blaze/Math.h>

#include "parldalib.hpp"
#include "gibbs.hpp"
#include "documents.hpp"
#include "results.hpp"

namespace fs = std::experimental::filesystem;
using namespace hpx::collectives;

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

int hpx_main(hpx::program_options::variables_map & vm) {

    {
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
    }

    UnicodeString regexp(UnicodeString::fromUTF8(vm["regex"].as<std::string>())); //u"[\\p{L}\\p{M}]+");

    fs::path pth{vm["corpus_dir"].as<std::string>()};

    const std::size_t n_threads = hpx::resource::get_num_threads("default");
    const std::size_t n_topics = vm["num_topics"].as<std::size_t>();
    const std::size_t iterations = vm["num_iters"].as<std::size_t>();
    const double alpha = vm["alpha"].as<double>();
    const double beta = vm["beta"].as<double>();

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
        const std::size_t n_paths = path_to_vector( pth, paths );
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

    par_train_lda(thread_idx, dwcm, tdcm, twcm, tokens, n_topics, iterations, alpha, beta);

    print_topics(vocabulary, twcm[0], n_topics);

    for(const std::size_t i : thread_idx) {
        const auto beg = std::get<0>(doc_chunks[i]);
        const auto end = std::get<1>(doc_chunks[i]);
        const std::size_t diff = static_cast<std::size_t>(end-beg);
        print_document_topics(tdcm[i], n_topics, 0, diff, 4);
    }

    return hpx::finalize();
}

int main(int argc, char ** argv) {
    // command line handling
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
