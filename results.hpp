//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __HPXLDA_RESULTS_HPP__
#define __HPXLDA_RESULTS_HPP__

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

#include <unicode/unistr.h>
#include <blaze/Math.h>

#ifdef ICU69
using icu_69::UnicodeString;
#else
using icu_66::UnicodeString;
#endif

using blaze::DynamicMatrix;

using namespace blaze;

void print_topics(std::unordered_map<std::string, std::size_t> const& vocabulary, DynamicMatrix<double> const& tdcm, const std::size_t n_topics, const std::size_t mxtokens=8);

void print_document_topics(DynamicMatrix<double> const& tdcm, const std::size_t n_topics, const std::size_t docbeg, const std::size_t docend, const std::size_t mxtopics=-1);

#endif
