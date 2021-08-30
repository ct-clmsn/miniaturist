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
#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include <unicode/unistr.h>

#include "documents.hpp"

namespace fs = std::experimental::filesystem;

int main(int argc, char ** argv) {

    UnicodeString regexp(u"[\\p{L}\\p{M}]+");
    fs::path pth{}; //("./corpus");

    {
        bool halt = false;
        while(!halt) {
            int option_index = 0;
            static struct option long_options[] =
            {
                {"corpus_dir", required_argument, NULL, 'c' },
                {"regex",  optional_argument,     NULL, 'r' },
                {NULL,      0,                    NULL,  0 }
            };

            int c = getopt_long(argc, argv, "c:r:", long_options, &option_index);
            if(c == -1) {
                halt = true;
            }
            else {
                switch(c) {
                    case 'c':
                    {
                        std::string carg{optarg};
                        pth = fs::path{carg};
			break;
		    }
                    case 'r':
                    {
                        regexp = UnicodeString::fromUTF8(std::string{optarg});
			break;
                    }
                }
            }
        }

        bool exit = false;

        if(pth.string().size() < 1) {
            std::cerr << "Please specify '--vocab_list=vocabulary-file'" << std::endl;
            exit = true;
        }

        if(exit) {
            return 1;
        }
    }

    std::unordered_map<std::string, std::size_t> vocabulary;

    std::vector< fs::path > paths;
    path_to_vector( pth, paths );

    inverted_index_t ii{};

    std::vector< fs::path >::iterator beg = paths.begin();
    std::vector< fs::path >::iterator end = paths.end();

    document_path_to_inverted_index(beg, end, regexp, ii, vocabulary);
    for(const auto& e : ii) {
        std::cout << e.first << std::endl;
    }
}
