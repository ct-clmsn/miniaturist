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

#include "lda.hpp"
#include "documents.hpp"
#include "results.hpp"

namespace fs = std::experimental::filesystem;

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

inline void gibbs(CompressedMatrix<double> const& dwcm, DynamicMatrix<double> & tdcm, DynamicMatrix<double> & twcm, std::vector<std::size_t> & tokens, DynamicVector<double> & ztot, DynamicVector<double> & probs, const std::size_t n_topics, const double N, const double alpha, const double beta) {

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
        const std::size_t n_words = dwcm.columns();

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

    for(std::size_t i = 0; i < iterations; ++i) {
        ztot = blaze::sum<blaze::rowwise>(twcm);
        gibbs(dwcm, tdcm, twcm, tokens, ztot, probs, n_topics, N, alpha, beta);
    }
}

int main(int argc, char ** argv) {

    fs::path wpth{}; //"./decorpus-wordlist.txt");
    UnicodeString regexp(u"[\\p{L}\\p{M}]+");
    fs::path pth{}; //("./corpus");

    std::size_t n_topics = -1;
    std::size_t iterations = 1000;
    double alpha = 0.1;
    double beta = 0.01;

    {
        bool halt = false;
        while(!halt) {
            int option_index = 0;
            static struct option long_options[] = 
            {
                {"vocab_list", required_argument, NULL, 'v' },
                {"corpus_dir", required_argument, NULL, 'c' },
                {"num_topics", required_argument, NULL, 't' },
                {"regex",  optional_argument,     NULL, 'r' },
                {"num_iters",  optional_argument, NULL, 'i' },
                {"alpha",  optional_argument,     NULL, 'a' },
                {"beta",  optional_argument,      NULL, 'b' },
                {NULL,      0,                    NULL,  0 }
            };

            int c = getopt_long(argc, argv, "v:c:t:", long_options, &option_index);
            if(c == -1) {
                halt = true;
            }
            else {
                switch(c) {
                    case 'v':
                    {
                        std::string varg{optarg};
                        wpth = fs::path{varg};
                        break;
                    }
                    case 'c':
                    {
                        std::string carg{optarg};
                        pth = fs::path{carg};
                        break;
                    }
                    case 't':
                    {
                        n_topics = static_cast<std::size_t>(std::stol(optarg));
                        break;
                    }
                    case 'r':
                    {
                        regexp = UnicodeString::fromUTF8(std::string{optarg});
                        break;
                    }
                    case 'i':
                    {
                        iterations = static_cast<std::size_t>(std::stol(optarg));
                        break;
                    }
                    case 'a':
                    {
                        alpha = std::stod(optarg);
                        break;
                    }
                    case 'b':
                    {
                        beta = std::stod(optarg);
                        break;
                    }
                }
            }
        }

        bool exit = false;

        if(n_topics == -1) {
            std::cerr << "Please specify '--num_topics=unsigned-integer-value'" << std::endl;
            exit = true;
        }
        if(pth.string().size() < 1) {
            std::cerr << "Please specify '--vocab_list=vocabulary-file'" << std::endl;
            exit = true;
        }
        if(wpth.string().size() < 1) {
            std::cerr << "Please specify '--corpus_dir=corpus-directory'" << std::endl;
            exit = true;
        }

        if(exit) {
            return 1;
        }
    }


    std::unordered_map<std::string, std::size_t> vocabulary;

    //fs::path wpth("./wordlist-german.txt");

    const std::size_t vocab_sz = load_wordlist(wpth, vocabulary);

    CompressedMatrix<double> dwcm;
    DynamicMatrix<double> tdcm, twcm;

    std::vector<std::size_t> tokens;

    std::vector< fs::path > paths;
    {
        const std::size_t n_paths = path_to_vector( pth, paths );
        std::vector< fs::path >::iterator beg = paths.begin();
        std::vector< fs::path >::iterator end = paths.end();
        const std::size_t ndocs = static_cast<std::size_t>(end-beg);

        inverted_index_t ii;

        tdcm.resize( n_topics, ndocs );
        twcm.resize( n_topics, vocab_sz );
        tdcm = 0.0;
        twcm = 0.0;

        const std::size_t nentries = document_path_to_inverted_index(beg, end, regexp, ii, vocabulary);
        CompressedMatrix<double> wdcm;
        inverted_index_to_matrix(vocabulary, ii, ndocs, nentries, wdcm);
        dwcm = blaze::trans(wdcm);
        matrix_to_vector(dwcm, tokens);
    }

    train_lda(dwcm, tdcm, twcm, tokens, n_topics, iterations, alpha, beta);

    print_topics(vocabulary, twcm, n_topics);

    print_document_topics(tdcm, n_topics, 0, paths.end()-paths.begin(), 4);
}
