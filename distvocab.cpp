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
#include <hpx/algorithm.hpp>

#include <vector>
#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include <unicode/unistr.h>

#include "jch.hpp"
#include "documents.hpp"
#include "inverted_index_serialize.hpp"

namespace fs = std::experimental::filesystem;
using namespace hpx::collectives;

int hpx_main(hpx::program_options::variables_map & vm) {

    bool histogram = false;
    bool exit = false;
    std::size_t filterlb = 0;
    std::size_t filterub = std::numeric_limits<std::size_t>::max();

    if(vm.count("corpus_dir") == 0) {
        std::cerr << "Please specify '--corpus_dir=corpus-directory'" << std::endl;
        exit = true;
    }

    if(vm.count("histogram") > 0) {
        histogram = true;
    }

    if(vm.count("filterlb") > 0) {
        filterlb = vm["filterlb"].as<std::size_t>();
    }

    if(vm.count("filterub") > 0) {
        filterub = vm["filterub"].as<std::size_t>();
    }

    if(exit) {
        return hpx::finalize();
    }

    UnicodeString regexp(UnicodeString::fromUTF8(vm["regex"].as<std::string>()));
    fs::path pth{vm["corpus_dir"].as<std::string>()};

    const std::vector<hpx::id_type> localities = hpx::find_all_localities();
    const std::size_t n_locales = localities.size();
    const std::size_t locality_id = hpx::get_locality_id();

    std::vector<inverted_index_t> indices(n_locales);
    for(std::size_t i = 0; i < n_locales; ++i) {
        indices.emplace_back(inverted_index_t{});
    }

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
            const std::size_t locale_doc_diff = std::get<1>(locale_dp)-std::get<0>(locale_dp);
            paths.resize(locale_doc_diff);
            std::copy_n(std::begin(locale_paths)+std::get<0>(locale_dp), locale_doc_diff, std::begin(paths));
        }

        inverted_index_t ii{};

        std::vector< fs::path >::iterator beg = paths.begin();
        std::vector< fs::path >::iterator end = paths.end();

        document_path_to_inverted_index(beg, end, regexp, ii);

        std::hash<std::string> stdhash{};

        for(const auto& entry : ii) {
            const std::int32_t tok_rank = JumpConsistentHash(stdhash(entry.first), n_locales);
            auto um_rank_itr = indices[tok_rank].find(entry.first); // unordered_map<size_t, size_t>
            auto um_rank_itr_end = um_rank_itr->second.end();

            if(indices[tok_rank].end() != um_rank_itr) {
                for(auto entryval : entry.second) {
                    auto vitr = um_rank_itr->second.find(entryval.first);

                    if(vitr != um_rank_itr_end) {
                        vitr->second += entryval.second;
                    }
                    else {
                        um_rank_itr->second.insert({entryval.first, entryval.second});
                    }
                }
            }
            else {
                indices[tok_rank].insert({entry.first, entry.second});
            }
        }
    }


    {
        std::vector< inverted_index_t > fin_indices;

        const std::string all_gather_direct_basename = "all_gather_direct";
        auto all_gather_direct_client = create_communicator(
            all_gather_direct_basename.c_str(), num_sites_arg(n_locales), this_site_arg(locality_id)
        );

        for(std::size_t i = 0; i < n_locales; ++i) {

           if(locality_id == i) {
               hpx::future<std::vector<inverted_index_t>> f = hpx::collectives::gather_here(all_gather_direct_client, indices[locality_id], hpx::collectives::this_site_arg{locality_id}); 
               fin_indices = f.get();
           }
           else {
               hpx::future<void> f = hpx::collectives::gather_there(all_gather_direct_client, indices[i], hpx::collectives::this_site_arg{locality_id});
               f.get();
           }

        }

        std::copy(std::begin(fin_indices), std::end(fin_indices), std::begin(indices));
    }

    std::unordered_map<std::string, std::size_t> filter{};
    const auto filter_end = filter.end(); 
    std::plus<std::size_t> addr{};

    for(std::size_t i = 0; i < n_locales; ++i) {
        for(const auto& e : indices[i]) {
	    if(filter.find(e.first) != filter_end) {
                const std::size_t count = hpx::transform_reduce(hpx::execution::par,
                    std::begin(e.second), std::end(e.second), 0, addr,
		    [](const auto& entry){
		        return entry.second;
                });

		filter[e.first] += count;
            }
	    else {
                const std::size_t count = hpx::transform_reduce(hpx::execution::par,
                    std::begin(e.second), std::end(e.second), 0, addr,
		    [](const auto& entry){
		        return entry.second;
                });

		filter[e.first] = count;
            }
        }
    }

    if(!histogram) {
        if(filterlb != 0 && filterub != std::numeric_limits<std::size_t>::max()) {
            for(const auto& e : filter) {
                if(e.second >= filterlb && e.second <= filterub) {
                    std::cout << e.first << std::endl;
                }
            }
	}
        else {
	    for(const auto& e : filter) {
                std::cout << e.first << std::endl;
            }
	}
    }
    else {
	if(filterlb != 0 && filterub != std::numeric_limits<std::size_t>::max()) {
	    for(const auto& e : filter) {
		if(e.second >= filterlb && e.second <= filterub) {
                    std::cout << e.first << ',' << e.second << std::endl;
		}
            }
	}
        else {
	    for(const auto& e : filter) {
                std::cout << e.first << ',' << e.second << std::endl;
            }
	}
    }

    return hpx::finalize();
}

int main(int argc, char ** argv) {
    hpx::program_options::options_description desc("usage: distvocab [options]");
    desc.add_options()
	    ("regex,re", hpx::program_options::value<std::string>()->default_value("[\\p{L}\\p{M}]+"),"regex (default: [\\p{L}\\p{M}]+]")
	    ("corpus_dir,cd",hpx::program_options::value<std::string>(),"directory path containing the corpus to model")
	    ("filterlb,lb",hpx::program_options::value<std::size_t>(),"filter out terms with a frequency below this value")
	    ("filterub,ub",hpx::program_options::value<std::size_t>(),"filter out terms with a frequency above this value")
	    ("histogram,hg",hpx::program_options::value<bool>(),"print global counts");

    hpx::init_params params;
    params.desc_cmdline = desc;
    return hpx::init(argc, argv, params);
}

#endif
