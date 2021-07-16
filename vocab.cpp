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

#include <unicode/unistr.h>

#include "documents.hpp"

namespace fs = std::experimental::filesystem;

int main(int argc, char ** argv) {

    UnicodeString regexp(u"[\\p{L}\\p{M}]+");
    fs::path pth("./corpus");

    const std::size_t n_threads = 4;
    const std::size_t n_topics = 16;
    const std::size_t iterations = 200;
    const double alpha = 0.1;
    const double beta = 0.01;

    std::unordered_map<std::string, std::size_t> vocabulary;

    std::vector< fs::path > paths;
    const std::size_t n_paths = path_to_vector( pth, paths );

    inverted_index_t ii{};

    std::vector< fs::path >::iterator beg = paths.begin();
    std::vector< fs::path >::iterator end = paths.end();

    const std::size_t nentries = document_path_to_inverted_index(beg, end, regexp, ii, vocabulary);
    for(const auto& e : ii) {
        std::cout << e.first << std::endl;
    }
}
