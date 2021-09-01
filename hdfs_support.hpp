//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __HDFS_SUPPORT_HPP__
#define __HDFS_SUPPORT_HPP__

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <experimental/filesystem>

#include <unicode/unistr.h>
#include <blaze/Math.h>
#include <hdfs/hdfs.h>

#include "inverted_index.hpp"

namespace fs = std::experimental::filesystem;
using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::CompressedMatrix;

#ifdef ICU69
using namespace icu_69;
#else
using namespace icu_66;
#endif

struct hdfs_context {
    hdfsBuilder * builder;
    hdfsFS filesystem; 
    std::size_t buffer_size;
    std::size_t block_size;

    hdfs_context() : builder(nullptr), filesystem(), buffer_size(-1), block_size(-1) {}

    ~hdfs_context() {
        hdfsDisconnect(filesystem);
        hdfsFreeBuilder(builder);
    }
};

void init_hdfs_context(hdfs_context & ctx, std::string const& namenode, const std::size_t namenode_port, const std::size_t buffer_sz, const std::size_t block_sz);

std::size_t load_wordlist(hdfs_context & ctx, fs::path const& pth, std::unordered_map<std::string, std::size_t> & vocab);

std::size_t path_to_vector(hdfs_context & ctx, std::string const& p, std::vector<fs::path> & paths);

std::size_t document_path_to_inverted_index(hdfs_context & ctx, std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii, std::unordered_map<std::string, std::size_t> const& voc);

std::size_t document_path_to_inverted_index(hdfs_context & ctx, std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii);

void json_topic_matrices(hdfs_context & ctx, std::string const& prefix, std::vector<CompressedMatrix<double>> const& dwcm, std::vector<DynamicMatrix<double>> const& tdcm, std::vector<DynamicMatrix<double>> const& twcm);

void json_topic_matrices(hdfs_context & ctx, const std::size_t locality, std::string const& prefix, std::vector<CompressedMatrix<double>> const& dwcm, std::vector<DynamicMatrix<double>> const& tdcm, std::vector<DynamicMatrix<double>> const& twcm);

#endif
