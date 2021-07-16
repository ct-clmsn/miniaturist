//  Copyright (c) 2021 Christopher Taylor 
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include <unicode/unistr.h>
#include <blaze/Math.h>

#include "ldalib.hpp"
#include "results.hpp"
#include "documents.hpp"

using namespace icu_69;

using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

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
