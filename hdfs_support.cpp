//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "hdfs_support.hpp"

#include <sstream>

#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/ucnv.h>
#include <unicode/regex.h>

#include <cassert>

#ifdef ICU69
using namespace icu_69;
#else
using namespace icu_66;
#endif

void init_hdfs_context(hdfs_context & ctx, std::string const& namenode, const std::size_t namenode_port, const std::size_t buffer_sz, const std::size_t block_sz) {
    ctx.builder = hdfsNewBuilder();
    hdfsBuilderSetNameNode(ctx.builder, namenode.c_str());
    hdfsBuilderSetNameNodePort(ctx.builder, namenode_port);
    ctx.filesystem = hdfsBuilderConnect(ctx.builder);
    ctx.buffer_size = buffer_sz;
    ctx.block_size = block_sz;
}

static void read_file_content(hdfs_context & ctx, fs::path const& pth, std::vector<UnicodeString> & fcontent) {
    std::uint8_t buffer[ctx.buffer_size];

    hdfsFileInfo *fileInfo = hdfsGetPathInfo(ctx.filesystem, pth.c_str());
    char ***hosts = hdfsGetHosts(ctx.filesystem, pth.c_str(), 0, fileInfo->mSize);

    std::string contents;
    contents.reserve(static_cast<std::size_t>(fileInfo->mSize));

    for(std::uint64_t block = 0; hosts[block]; ++block) {
        for(std::uint64_t j = 0; hosts[block][j]; ++j) { 
            const char * hostname = hosts[block][j];

            hdfsFile file = hdfsOpenFile2(ctx.filesystem, hostname, pth.c_str(), O_RDONLY, ctx.buffer_size, 0, 0);

            const std::int64_t seek = fileInfo->mBlockSize*block;
            const std::int32_t r = hdfsSeek(ctx.filesystem, file, seek);

            assert(r >= 0);

            std::int32_t rd = 0;
            std::int32_t totalrd = 0;

            do {
                rd = hdfsRead(ctx.filesystem, file, buffer, ctx.buffer_size);

                if(rd > 0) {
                    contents.append(reinterpret_cast<char *>(buffer), rd);
                }

                totalrd += rd;

            } while(rd > 0 && totalrd < fileInfo->mBlockSize);

            hdfsCloseFile(ctx.filesystem, file);
        }
    }

    if(contents.size() > 0) {
        fcontent.push_back( UnicodeString::fromUTF8(contents).toLower() );
    }

    hdfsFreeFileInfo(fileInfo, 1);
}

static void read_file_content(hdfs_context & ctx, std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, std::vector<UnicodeString> & fcontent) {
    std::uint8_t buffer[ctx.buffer_size];

    for(auto pth = beg; pth != end; ++pth) {
        hdfsFileInfo *fileInfo = hdfsGetPathInfo(ctx.filesystem, pth->c_str());
        char ***hosts = hdfsGetHosts(ctx.filesystem, pth->c_str(), 0, fileInfo->mSize);

        std::string contents;
        contents.reserve(static_cast<std::size_t>(fileInfo->mSize));

        for(std::uint64_t block = 0; hosts[block]; ++block) {
            for(std::uint64_t j = 0; hosts[block][j]; ++j) { 
                const char * hostname = hosts[block][j];

                hdfsFile file = hdfsOpenFile2(ctx.filesystem, hostname, pth->c_str(), O_RDONLY, ctx.buffer_size, 0, 0);

                const std::int64_t seek = fileInfo->mBlockSize*block;
                const std::int32_t r = hdfsSeek(ctx.filesystem, file, seek);

                assert(r >= 0);

                std::int32_t rd = 0;
                std::int32_t totalrd = 0;

                do {
                    rd = hdfsRead(ctx.filesystem, file, buffer, ctx.buffer_size);

                    if(rd > 0) {
                        contents.append(reinterpret_cast<char *>(buffer), rd);
                    }

                    totalrd += rd;

                } while(rd > 0 && totalrd < fileInfo->mBlockSize);

                hdfsCloseFile(ctx.filesystem, file);
            }
        }

        if(contents.size() > 0) {
            fcontent.push_back( UnicodeString::fromUTF8(contents).toLower() );
        }

        hdfsFreeFileInfo(fileInfo, 1);
    }
}

std::size_t load_wordlist(hdfs_context & ctx, fs::path const& pth, std::unordered_map<std::string, std::size_t> & vocab) {
    std::vector<UnicodeString> voc;
    read_file_content(ctx, pth, voc);
    const std::size_t vcz = voc.size();

    for(std::size_t i = 0; i < vcz; ++i) {
        std::string tok;
        voc[i].toUTF8String(tok);
        vocab[tok] = i;
    }

    return vcz;
}

std::size_t document_path_to_inverted_index(hdfs_context & ctx, std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii, std::unordered_map<std::string, std::size_t> const& voc) {

    std::vector<UnicodeString> files_content;
    read_file_content(ctx, beg, end, files_content);

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

std::size_t path_to_vector(hdfs_context & ctx, std::string const& p, std::vector<fs::path> & paths) {
    if(hdfsExists(ctx.filesystem, p.c_str())) {

       std::int32_t count = 0;
       hdfsFileInfo * fi = hdfsListDirectory(ctx.filesystem, p.c_str(), &count);

       if(fi != nullptr) {
           for(std::int32_t i = 0; i < count; ++i) {

               if((fi+i)->mKind == kObjectKindDirectory) {
                   const std::string dirnom{(fi+i)->mName};
                   path_to_vector(ctx, dirnom, paths);
               }
               else {
                   paths.emplace_back(fs::path{std::string{(fi+i)->mName}});
               }

               hdfsFreeFileInfo(fi+i, 1);
           }
       }
    }

    return paths.size();
}

std::size_t document_path_to_inverted_index(hdfs_context & ctx, std::vector<fs::path>::iterator & beg, std::vector<fs::path>::iterator & end, UnicodeString & regexp, inverted_index_t & ii) {
    std::vector<UnicodeString> files_content;
    read_file_content(ctx, beg, end, files_content);

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<RegexMatcher> matcher{ new RegexMatcher(regexp, 0, status) };
    std::size_t fc_count = 0;
    std::size_t entry_count = 0;
    const auto ii_end = ii.end();

    for(auto & fc : files_content) {
        matcher->reset(fc);

        while(matcher->find()) {
            const auto beg = matcher->start(status);
            const auto end = matcher->end(status);

            auto token = fc.tempSubString(beg, end-beg);

            std::string matched_token;
            token.toUTF8String(matched_token);

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

        ++fc_count;
    }

    return entry_count;
}

void json_topic_matrices(hdfs_context & ctx, std::string const& prefix, std::vector<CompressedMatrix<double>> const& dwcm, std::vector<DynamicMatrix<double>> const& tdcm, std::vector<DynamicMatrix<double>> const& twcm) {

    std::string content{};
    {
        std::stringstream fs{};
        fs << "[ { 'name' : 'dwcm', " << std::endl
           << " 'data_size' : " << dwcm.size() << ", " << std::endl
           << " 'data' : [" << std::endl;

        std::size_t sz = dwcm.size()-1, i = 0;
        for(const auto& dwcm_m : dwcm) {
            fs << dwcm_m << std::endl;
            if(i == sz) { fs << ',' << std::endl; }    
        }

        fs << "] }, " << std::endl
           << "{ 'name' : 'tdcm', " << std::endl
           << " 'data_size' : " << tdcm.size() << ", " << std::endl
           << " 'data' : [" << std::endl;

        sz = tdcm.size()-1, i = 0;
        for(const auto& tdcm_m : tdcm) {
            fs << tdcm_m << std::endl;
            if(i == sz) { fs << ',' << std::endl; }
        }

        fs << "] }, " << std::endl
           << "{ 'name' : 'twcm', " << std::endl
           << " 'data_size' : " << twcm.size() << ", " << std::endl
           << " 'data' : [" << std::endl;

        sz = twcm.size()-1, i = 0;
        for(const auto& twcm_m : twcm) {
            fs << twcm_m << std::endl;
            if(i == sz) { fs << ',' << std::endl; }
        }

        fs << "] } ]" << std::endl;

        content = fs.str();
        fs.flush();
    }

    hdfsFile out;
    std::string fn{prefix + ".json"};
    if (nullptr == (out = hdfsOpenFile(ctx.filesystem, fn.c_str(), O_WRONLY, 0, 0, ctx.block_size))) {
	std::cerr << "json_topic_matrices hdfsOpenFile error" << std::endl;
	std::cerr.flush();
	return;
    }

    std::uint8_t buffer[ctx.buffer_size];
    std::size_t content_sz = content.size();
    std::size_t batch = 0;
    std::size_t offset = 0;
    int rc = -1;
    
    while(content_sz > 0) {
	batch = content_sz < ctx.buffer_size ? content_sz : ctx.buffer_size;
        std::copy_n(std::begin(content)+offset, batch, buffer);
	if(0 > (rc = hdfsWrite(ctx.filesystem, out, buffer, batch))) {
	    std::cerr << "json_topic_matrices hdfsWrite error" << std::endl;
            std::cerr.flush();
            rc = hdfsCloseFile(ctx.filesystem, out);
	    return;
	}

	content_sz -= rc;
	offset += rc;
    }

    rc = hdfsCloseFile(ctx.filesystem, out);
}

void json_topic_matrices(hdfs_context & ctx, const std::size_t locality, std::string const& prefix, std::vector<CompressedMatrix<double>> const& dwcm, std::vector<DynamicMatrix<double>> const& tdcm, std::vector<DynamicMatrix<double>> const& twcm) {
    std::string tprefix{ prefix + "_" + std::to_string(locality) };
    json_topic_matrices(ctx, tprefix, dwcm, tdcm, twcm);
}
