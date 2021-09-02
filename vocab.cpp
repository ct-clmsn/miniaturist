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
#include <limits>
#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include <unicode/unistr.h>

#include "documents.hpp"

namespace fs = std::experimental::filesystem;

int main(int argc, char ** argv) {

    bool histogram = false;
    std::size_t filterlb = 0;
    std::size_t filterub = std::numeric_limits<std::size_t>::max();
    UnicodeString regexp(u"[\\p{L}\\p{M}]+");
    fs::path pth{};

    {
        bool halt = false;
        while(!halt) {
            int option_index = 0;
            static struct option long_options[] =
            {
                {"corpus_dir", required_argument, NULL, 'c' },
                {"regex",      optional_argument, NULL, 'r' },
                {"histogram",  optional_argument, NULL, 'h' },
                {"filterlb",     optional_argument, NULL, 'l' },
                {"filterub",     optional_argument, NULL, 'u' },
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
		    case 'h':
		    {
                        histogram = true;
			break;
	            }
                    case 'l':
		    {
			std::string sval{optarg};
			char * svalend = nullptr;
			filterlb = static_cast<std::size_t>(std::strtoul(sval.c_str(), &svalend, 10));
			break;
	            }
                    case 'u':
		    {
			std::string sval{optarg};
			char * svalend = nullptr;
			filterub = static_cast<std::size_t>(std::strtoul(sval.c_str(), &svalend, 10));
			break;
	            }

                }
            }
        }

        bool exit = false;

        if(pth.string().size() < 1) {
            std::cerr << "Please specify '--corpus_dir=<path> (required), --regex=<string> (optional), --histogram (optional) --filterlb=integer (optional) --filterub=integer (optional)'" << std::endl;
            exit = true;
        }

        if(exit) {
            return 1;
        }
    }

    std::vector< fs::path > paths;
    path_to_vector( pth, paths );

    inverted_index_t ii{};

    std::vector< fs::path >::iterator beg = paths.begin();
    std::vector< fs::path >::iterator end = paths.end();

    document_path_to_inverted_index(beg, end, regexp, ii);

    if(!histogram) {
	if(filterlb == 0 && filterub == std::numeric_limits<std::size_t>::max()) {
            for(const auto& e : ii) {
                std::cout << e.first << std::endl;
            }
	}
	else {
    	    std::plus<std::size_t> addr{};
            for(const auto& e : ii) {
                const std::size_t count = std::transform_reduce(e.second.begin(), e.second.end(), 0, addr, [](const auto& entry){ return entry.second; });
		if(count >= filterlb && count <= filterub) {
                    std::cout << e.first << std::endl;
		}
            }
        }
    }
    else {
	if(filterlb == 0 && filterub == std::numeric_limits<std::size_t>::max()) {
    	    std::plus<std::size_t> addr{};
            for(const auto& e : ii) {
                const std::size_t count = std::transform_reduce(e.second.begin(), e.second.end(), 0, addr, [](const auto& entry){ return entry.second; });
                std::cout << e.first << ',' << count << std::endl;
            }
	}
	else {
    	    std::plus<std::size_t> addr{};
            for(const auto& e : ii) {
                const std::size_t count = std::transform_reduce(e.second.begin(), e.second.end(), 0, addr, [](const auto& entry){ return entry.second; });
		if(count >= filterlb && count <= filterub) {
                    std::cout << e.first << ',' << count << std::endl;
		}
            }
        }
    }
}
