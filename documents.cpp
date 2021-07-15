//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <fstream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>

#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/ucnv.h>
#include <unicode/regex.h>

#include <blaze/Math.h>
#include <openssl/ssl.h>

#include "bloomfilter.hpp"
#include "jch.hpp"
#include "documents.hpp"

using namespace icu_69;
using blaze::DynamicMatrix;

std::size_t path_to_vector(fs::path const& p, std::vector<fs::path> & paths) {

    for(auto& p : fs::recursive_directory_iterator(p)) {
        paths.push_back(p);
    }

    return paths.size();
}

static void read_file_content(fs::path const& p, std::vector<UnicodeString> & fcontent) {
    fs::directory_iterator di(p);

    for(auto & pth : di) {
        std::ifstream istrm(pth.path(), std::ios::in | std::ios::binary);
        const std::istream::pos_type curpos = istrm.tellg();
        istrm.seekg(0, std::ifstream::end);
        const std::istream::pos_type stream_sz = istrm.tellg() - curpos;
        istrm.seekg(curpos);

        std::string contents;
        contents.resize(static_cast<std::size_t>(stream_sz));
        istrm.read(&contents[0], stream_sz);
        fcontent.push_back( UnicodeString::fromUTF8(contents).toLower() );
        istrm.close();

        //UnicodeString content, rd;

        //while(istrm >> rd) {
        //    content += (" " + rd.toLower());
        //}
    }
}

void read_content(fs::path const& p, std::vector<UnicodeString> & fcontent) {
    std::ifstream istrm(p, std::ios::in | std::ios::binary);
    UnicodeString rd;

    while(istrm >> rd) {
        fcontent.push_back( rd.toLower() );
    }

    istrm.close();
}


static void read_file_content(std::vector<fs::path> const& p, std::vector<UnicodeString> & fcontent) {
    for(auto & pth : p) {
        std::ifstream istrm(pth, std::ios::in | std::ios::binary);
        const std::istream::pos_type curpos = istrm.tellg();
        istrm.seekg(0, std::ifstream::end);
        const std::istream::pos_type stream_sz = istrm.tellg() - curpos;
        istrm.seekg(curpos);

        std::string contents;
        contents.resize(static_cast<std::size_t>(stream_sz));
        istrm.read(&contents[0], stream_sz);
        fcontent.push_back( UnicodeString::fromUTF8(contents).toLower() );
        istrm.close();

        /*
        UnicodeString content, rd;

        while(istrm >> rd) {
            content += (" " + rd.toLower());
        }

        fcontent.push_back( std::move(content) );
        istrm.close();
        */
    }
}

static void read_file_content(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, std::vector<UnicodeString> & fcontent) {
    for(auto pth = beg; pth != end; ++pth) {
        std::ifstream istrm(*pth, std::ios::in | std::ios::binary);
        const std::istream::pos_type curpos = istrm.tellg();
        istrm.seekg(0, std::ifstream::end);
        const std::istream::pos_type stream_sz = istrm.tellg() - curpos;
        istrm.seekg(curpos);

        std::string contents;
        contents.resize(static_cast<std::size_t>(stream_sz));
        istrm.read(&contents[0], stream_sz);
        fcontent.push_back( UnicodeString::fromUTF8(contents).toLower() );

        /*
        UnicodeString content, rd;

        while(istrm >> rd) {
            content += (" " + rd.toLower());
        }

        fcontent.push_back( std::move(content) );
        istrm.close();
        */
    }
}

// https://unicode-org.github.io/icu-docs/apidoc/dev/icu4c/classicu_1_1UnicodeString.html
//
// https://github.com/jpakkane/cppunicode/blob/main/cppunicode.cpp
//
std::size_t create_inverted_index(inverted_index_t & index, fs::path const& p, const std::size_t localities_n, const std::size_t locality, const UnicodeString& regex_p) {
    std::vector<UnicodeString> files_content;
    read_file_content(p, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regex_p, 0, status) };
    std::size_t fc_count = 0;
    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);

            auto idx = index.find(matched_token);

            if( idx != index.end() ) {
                if( idx->second.find(fc_count) != idx->second.end() ) {
                    ++(idx->second[fc_count]);
                }            
                else {
                    idx->second[fc_count] = 1;
                }
            }
            else {
                index[matched_token][fc_count] = 1;
            }
        }

        ++fc_count;
   }

   return fc_count;
}

std::size_t create_inverted_index(inverted_index_t & index, fs::path const& p, const std::size_t localities_n, const std::size_t locality, const UnicodeString& regex_p, Bloomfilter & bf) {

    std::vector<UnicodeString> files_content;
    read_file_content(p, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regex_p, 0, status) };
    std::size_t fc_count = 0;
    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);

            if(!bf.contains(matched_token)) {
                continue;
            }

            auto idx = index.find(matched_token);

            if( idx != index.end() ) {
                if( idx->second.find(fc_count) != idx->second.end() ) {
                    ++(idx->second[fc_count]);
                }            
                else {
                    idx->second[fc_count] = 1;
                }
            }
            else {
                index[matched_token][fc_count] = 1;
            }
        }

        ++fc_count;
   }

   return fc_count;
}

void inverted_index_to_matrix(std::unordered_map<std::string, std::size_t> const& voc, inverted_index_t const& idx, const std::size_t ndocs, const std::size_t entry_count, CompressedMatrix<double> & mat, const bool debug) {
    const std::size_t nvoc = voc.size();
    mat.resize(nvoc, ndocs);
    const auto voc_end = voc.end();

    for(auto & w : idx) {
        const auto& ventry = voc.find(w.first);
        if(ventry != voc_end) {
            const std::size_t docsz = w.second.size();
            mat.reserve(ventry->second, docsz);
            for(const auto d : w.second) {
                mat.append(ventry->second, d.first, d.second);
            }
        }
        else if(debug) {
                std::cerr << "word in corpus but not dictionary\t" << w.first << std::endl;
        }
    }

    for(std::size_t i = 0; i < ndocs; i++) {
        mat.finalize(i);
    }
}

void document_path_to_matrix(fs::path & p, UnicodeString & regexp, CompressedMatrix<double> & mat, std::unordered_map<std::string, std::size_t> const& voc) {
    inverted_index_t ii;
    const std::size_t doc_count = create_inverted_index(ii, p, 1, 1, regexp);
    std::size_t ecount = 0;
    for(const auto & i : ii) {
        ecount += i.second.size();
    }
    inverted_index_to_matrix(voc, ii, doc_count, ecount, mat);
}

void document_path_to_matrix(fs::path & p, UnicodeString & regexp, Bloomfilter &bf, CompressedMatrix<double> & mat, std::unordered_map<std::string, std::size_t> const& voc) {
    inverted_index_t ii;
    const std::size_t doc_count = create_inverted_index(ii, p, 1, 1, regexp, bf);
    std::size_t ecount = 0;
    for(const auto & i : ii) {
        ecount += i.second.size();
    }
    inverted_index_to_matrix(voc, ii, doc_count, ecount, mat);
}

std::size_t document_path_to_inverted_indices(fs::path & p, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks) {
    std::vector<UnicodeString> files_content;
    read_file_content(p, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;

    const std::int32_t int_n_ranks = static_cast<std::int32_t>(n_ranks);

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);
            const std::int32_t tok_rank = JumpConsistentHash(std::hash<std::string>{}(matched_token), int_n_ranks);

            auto & n_index = ii[tok_rank];

            auto idx = n_index.find(matched_token);


            if( idx != n_index.end() ) {
                if( idx->second.find(fc_count) != idx->second.end() ) {
                    ++(idx->second[fc_count]);
                }            
                else {
                    idx->second[fc_count] = 1;
                    ++entry_count;
                }
            }
            else {
                n_index[matched_token][fc_count] = 1;
                ++entry_count;
            }
        }

        ++fc_count;
   }

   return fc_count;
}

std::size_t document_path_to_inverted_indices(std::vector<fs::path> const& p, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks) {
    std::vector<UnicodeString> files_content;
    read_file_content(p, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;

    const std::int32_t int_n_ranks = static_cast<std::int32_t>(n_ranks);

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);
            const std::int32_t tok_rank = JumpConsistentHash(std::hash<std::string>{}(matched_token), int_n_ranks);

            auto & n_index = ii[tok_rank];

            auto idx = n_index.find(matched_token);


            if( idx != n_index.end() ) {
                if( idx->second.find(fc_count) != idx->second.end() ) {
                    ++(idx->second[fc_count]);
                }            
                else {
                    idx->second[fc_count] = 1;
                    ++entry_count;
                }
            }
            else {
                n_index[matched_token][fc_count] = 1;
                ++entry_count;
            }
        }

        ++fc_count;
   }

   return entry_count;
}

std::size_t document_path_to_inverted_indices(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, std::vector<inverted_index_t> & ii, const std::size_t n_ranks) {
    std::vector<UnicodeString> files_content;
    read_file_content(beg, end, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;

    const std::int32_t int_n_ranks = static_cast<std::int32_t>(n_ranks);

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);
            const std::int32_t tok_rank = JumpConsistentHash(std::hash<std::string>{}(matched_token), int_n_ranks);

            auto & n_index = ii[tok_rank];

            auto idx = n_index.find(matched_token);


            if( idx != n_index.end() ) {
                if( idx->second.find(fc_count) != idx->second.end() ) {
                    ++(idx->second[fc_count]);
                }            
                else {
                    idx->second[fc_count] = 1;
                    ++entry_count;
                }
            }
            else {
                n_index[matched_token][fc_count] = 1;
                ++entry_count;
            }
        }

        ++fc_count;
   }

   return entry_count;
}

std::size_t document_path_to_inverted_indices(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, Bloomfilter & bf, std::vector<inverted_index_t> & ii, const std::size_t n_ranks) {
    std::vector<UnicodeString> files_content;
    read_file_content(beg, end, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;

    const std::int32_t int_n_ranks = static_cast<std::int32_t>(n_ranks);

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);

            if(bf.contains(matched_token)) {
                const std::int32_t tok_rank = JumpConsistentHash(std::hash<std::string>{}(matched_token), int_n_ranks);

                auto & n_index = ii[tok_rank];

                auto idx = n_index.find(matched_token);

                if( idx != n_index.end() ) {
                    if( idx->second.find(fc_count) != idx->second.end() ) {
                        ++(idx->second[fc_count]);
                    }            
                    else {
                        idx->second[fc_count] = 1;
                        ++entry_count;
                    }
                }
                else {
                    n_index[matched_token][fc_count] = 1;
                    ++entry_count;
                }
            }
        }

        ++fc_count;
   }

   return entry_count;
}

std::size_t document_path_to_inverted_index(std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii, std::unordered_map<std::string, std::size_t> const& voc) {
    std::vector<UnicodeString> files_content;
    read_file_content(beg, end, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;
    const auto ii_end = ii.end();
    const auto voc_end = voc.end();

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);

            const auto voc_find = voc.find(matched_token);

            if(voc_find != voc_end) {
                const auto idx = ii.find(matched_token);
                if( idx != ii_end ) {
                    if( idx->second.find(fc_count) != idx->second.end() ) {
                        ++(idx->second[fc_count]);
                    }            
                    else {
                        idx->second[fc_count] = 1;
                        ++entry_count;
                    }
                }
                else {
                    ii[matched_token][fc_count] = 1;
                    ++entry_count;
                }
            }
            else {
                std::cerr << "not found\t" << matched_token << std::endl;
            }
        }

        ++fc_count;
    }

    return entry_count;
}

void matrix_to_vector(CompressedMatrix<double> const& mat, std::vector<std::size_t> & tokens) {
    const std::size_t wcount = static_cast<std::size_t>(std::floor(blaze::sum(mat)));
    tokens.resize(wcount);
    std::fill(std::begin(tokens), std::end(tokens), 0);
}

std::size_t load_wordlist(fs::path const& pth, std::unordered_map<std::string, std::size_t> & vocab) {
    std::vector<UnicodeString> voc;
    read_content(pth, voc);
    const std::size_t vcz = voc.size();

    for(std::size_t i = 0; i < vcz; ++i) {
        std::string tok;
        voc[i].toUTF8String(tok);
        vocab[tok] = i;
    }

    return vcz;
}
