//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "results.hpp"

#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "unicode/unistr.h"
#include "unicode/ustream.h"

template<typename Vector>
inline void argsort(Vector const& v, std::vector<std::size_t> & args) {
    const std::size_t vsz = v.size();
    if(args.size() != vsz) {
        args.resize(vsz);
    }
    
    std::iota(std::begin(args), std::end(args), 0);
    std::sort(std::begin(args), std::end(args), [&v](const std::size_t i, const std::size_t j) {
        return v[i] < v[j];
    });
}

void print_topics(std::unordered_map<std::string, std::size_t> const& vocabulary, DynamicMatrix<double> const& twcm, const std::size_t n_topics, const std::size_t mxtokens) {
    const std::size_t ntopics = twcm.rows();
    assert(ntopics == n_topics);

    constexpr char locale_name[] = "";
    setlocale( LC_ALL, locale_name );
    std::locale::global(std::locale(locale_name));
    std::ios_base::sync_with_stdio(false);
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());

    DynamicVector<double> ztot = blaze::sum<blaze::rowwise>(twcm);
    ztot /= blaze::sum(ztot);

    std::vector<std::size_t> idx(vocabulary.size());
    std::fill(std::begin(idx), std::end(idx), 0);

    std::vector<std::string> voc_idx(vocabulary.size());
    for(const auto& v : vocabulary) {
        voc_idx[v.second] = v.first;
    }

    for(std::size_t t = 0; t < n_topics; ++t) {
        DynamicVector<double, blaze::rowVector> tw = blaze::row(twcm, t);
        tw = -tw;
        argsort(tw, idx);
        std::cout << "topic " << t << '\t' << ztot[t] << '\t';

        for(std::size_t m = 0; m < mxtokens; ++m) {
           if(m == (mxtokens-1)) {
               std::cout << voc_idx[idx[m]];
           }
           else {
               std::cout << voc_idx[idx[m]] << ' ';
           }
        }
        std::cout << std::endl;
    }
    std::cout << "prob sum\t" << blaze::sum(ztot) << std::endl;
}

void print_document_topics(DynamicMatrix<double> const& tdcm, const std::size_t n_topics, const std::size_t docbeg, const std::size_t docend, const std::size_t mxtopics) {
    const std::size_t ntopics = tdcm.rows();
    assert(ntopics == n_topics);

    std::size_t mxt = mxtopics;
    if(mxtopics == static_cast<std::size_t>(-1)) {
        mxt = ntopics;
    }

    std::vector<std::size_t> idx(n_topics);
    std::fill(std::begin(idx), std::end(idx), 0);

    //const std::size_t ndocs = tdcm.columns();
    for(std::size_t d = docbeg; d < docend; ++d) {
        DynamicVector<double, blaze::columnVector> td = blaze::column(tdcm, d);
        //std::cout << "document count\t" << td << '\t';

        const double ztot = blaze::sum(td);

        td *= (-1.0 / ztot);
        argsort(td, idx);
        std::cout << "document " << d << '\t'; 

        for(std::size_t m = 0; m < mxt; ++m) {
           if(m == (mxtopics-1)) {
               std::cout << '(' << (-td[idx[m]]) << ',' << idx[m] << ')';
           }
           else {
               std::cout << '(' << (-td[idx[m]]) << ',' << idx[m] << ')' << ' ';
           }
        }
        std::cout << std::endl;
    }
}
