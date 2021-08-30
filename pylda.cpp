//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <vector>
#include <unordered_map>
#include <string>
#include <experimental/filesystem>

#include <unicode/unistr.h>
#include <blaze/Math.h>
#include <pybind11/pybind11.h>

#include "inverted_index.hpp"
#include "documents.hpp"
#include "ldalib.hpp"

namespace fs = std::experimental::filesystem;

#ifdef ICU69
using icu_69::UnicodeString;
#else
using icu_66::UnicodeString;
#endif

using blaze::DynamicMatrix;
using blaze::CompressedMatrix;

namespace py = pybind11;

void lda(std::string path, std::vector<std::string> vocab, std::string regex, const std::size_t iterations, const std::size_t n_topics, const double alpha, const double beta) {
    fs::path pth{path};
    UnicodeString regexp = UnicodeString::fromUTF8(regex);

    std::unordered_map<std::string, std::size_t> vocabulary;

    const std::size_t vocab_sz = vocab.size();
    for(std::size_t i = 0; i < vocab_sz; ++i) {
        vocabulary.insert({vocab[i], i});
    }

    CompressedMatrix<double> dwcm;
    DynamicMatrix<double> tdcm, twcm;

    std::vector<std::size_t> tokens;

    std::vector< fs::path > paths;
    {
        path_to_vector( pth, paths );
        std::vector< fs::path >::iterator beg = paths.begin();
        std::vector< fs::path >::iterator end = paths.end();
        const std::size_t ndocs = static_cast<std::size_t>(end-beg);

        inverted_index_t ii;

        tdcm.resize( n_topics, ndocs );
        twcm.resize( n_topics, vocab_sz );
        tdcm = 0.0;
        twcm = 0.0;

        document_path_to_inverted_index(beg, end, regexp, ii, vocabulary);
        CompressedMatrix<double> wdcm;
        inverted_index_to_matrix(vocabulary, ii, ndocs, wdcm);
        dwcm = blaze::trans(wdcm);
        matrix_to_vector(dwcm, tokens);
    }

    train_lda(dwcm, tdcm, twcm, tokens, n_topics, iterations, alpha, beta);
}


void lda(std::string path, std::string vocab_path, std::string regex, const std::size_t iterations, const std::size_t n_topics, const double alpha, const double beta) {
    fs::path wpth{vocab_path};
    fs::path pth{path};
    UnicodeString regexp = UnicodeString::fromUTF8(regex);

    std::unordered_map<std::string, std::size_t> vocabulary;

    const std::size_t vocab_sz = load_wordlist(wpth, vocabulary);

    CompressedMatrix<double> dwcm;
    DynamicMatrix<double> tdcm, twcm;

    std::vector<std::size_t> tokens;

    std::vector< fs::path > paths;
    {
        path_to_vector( pth, paths );
        std::vector< fs::path >::iterator beg = paths.begin();
        std::vector< fs::path >::iterator end = paths.end();
        const std::size_t ndocs = static_cast<std::size_t>(end-beg);

        inverted_index_t ii;

        tdcm.resize( n_topics, ndocs );
        twcm.resize( n_topics, vocab_sz );
        tdcm = 0.0;
        twcm = 0.0;

        document_path_to_inverted_index(beg, end, regexp, ii, vocabulary);
        CompressedMatrix<double> wdcm;
        inverted_index_to_matrix(vocabulary, ii, ndocs, wdcm);
        dwcm = blaze::trans(wdcm);
        matrix_to_vector(dwcm, tokens);
    }

    train_lda(dwcm, tdcm, twcm, tokens, n_topics, iterations, alpha, beta);
}

PYBIND11_MODULE(pylda, m) {

    m.def("lda", [](std::string const& path, std::vector<std::string> const& vocab, std::string const& regex, const long iters, const long ntopics, const double alpha, const double beta) {
        lda(path, vocab, regex, static_cast<std::size_t>(iters), static_cast<std::size_t>(ntopics), alpha, beta);
    });

    m.def("lda", [](std::string const& path, std::string const& vocab, std::string const& regex, const long iters, const long ntopics, const double alpha, const double beta) {
        lda(path, vocab, regex, static_cast<std::size_t>(iters), static_cast<std::size_t>(ntopics), alpha, beta);
    });

}
