// https://hamberg.no/erlend/posts/2015-03-20-jump-consistent-hash-in-haskell.html
//
#pragma once
#ifndef __JCH_HPP__
#define __JCH_HPP__

#include <cstdint>

std::int32_t JumpConsistentHash(std::uint64_t key, std::int32_t num_buckets);

#endif
