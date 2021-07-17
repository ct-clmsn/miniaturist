//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __HPXLDA_DOCUMENTS_HPP__
#define __HPXLDA_DOCUMENTS_HPP__

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include <experimental/filesystem>

#include <unicode/unistr.h>
#include <blaze/Math.h>
#include "bloomfilter.hpp"

namespace fs = std::experimental::filesystem;
using icu_69::UnicodeString;
using blaze::DynamicMatrix;
using blaze::CompressedMatrix;

using inverted_index_t = std::unordered_map<std::string, std::unordered_map<std::size_t, std::size_t> >;

void read_content(fs::path const& p, std::vector<UnicodeString> & fcontent);

std::size_t path_to_vector(fs::path const& p, std::vector<fs::path> & paths);

void document_path_to_matrix(fs::path & p, UnicodeString & regexp, DynamicMatrix<double> & mat, std::unordered_map<std::string, std::size_t> const& voc);

void document_path_to_matrix(fs::path & p, UnicodeString & regexp, Bloomfilter &bf, DynamicMatrix<double> & mat, std::unordered_map<std::string, std::size_t> const& voc);

std::size_t document_path_to_inverted_indices(fs::path & p, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks);

std::size_t document_path_to_inverted_indices(std::vector<fs::path> const& p, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks);

std::size_t document_path_to_inverted_indices(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks);

std::size_t document_path_to_inverted_indices(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, Bloomfilter & bf, std::vector<inverted_index_t> & ii, const std::size_t n_ranks);

std::size_t document_path_to_inverted_index(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii, std::unordered_map<std::string, std::size_t> const& voc);

std::size_t document_path_to_inverted_index(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii);

void inverted_index_to_matrix(std::unordered_map<std::string, std::size_t> const & vocab, inverted_index_t const& idx, const std::size_t doc_count, const std::size_t entry_count, CompressedMatrix<double> & mat, const bool debug=false);

void matrix_to_vector(CompressedMatrix<double> const& mat, std::vector<std::size_t> & tokens);

std::size_t load_wordlist(fs::path const& pth, std::unordered_map<std::string, std::size_t> & vocab);

#endif
