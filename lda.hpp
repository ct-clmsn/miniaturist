#pragma once
#ifndef __HPXLDA_HPP__
#define __HPXLDA_HPP__

#include <experimental/filesystem>
#include <cstdint>

#include <unicode/unistr.h>
#include <blaze/Math.h>

using namespace icu_69;
namespace fs = std::experimental::filesystem;
using blaze::DynamicMatrix;

void create_lda(DynamicMatrix<double> dwcm, fs::path const& p, const UnicodeString& regexp, std::vector<UnicodeString> & vocabulary, const std::size_t n_ranks, const std::size_t rank);
void train_lda(DynamicMatrix<double> const& dwcm, DynamicMatrix<double> & tdcm, DynamicMatrix<double> & twcm, const std::size_t n_topics, const std::size_t iterations, const double alpha, const double beta);

#endif
