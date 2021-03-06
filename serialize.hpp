//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#if !defined(PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP)
#define PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP

//#include <phylanx/config.hpp>
#include <hpx/serialization/array.hpp>
#include <hpx/include/util.hpp>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

#include <array>
#include <cstddef>

namespace hpx { namespace serialization
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool TF>
    void load(
        input_archive& archive, blaze::DynamicVector<T, TF>& target, unsigned)
    {
        // De-serialize vector
        std::size_t count = 0UL;
        std::size_t spacing = 0UL;
        archive >> count >> spacing;

        target.resize(count, false);
        archive >>
            hpx::serialization::make_array(target.data(), spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void load(input_archive& archive, blaze::DynamicMatrix<T, true>& target,
        unsigned)
    {
        // De-serialize matrix
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> rows >> columns >> spacing;

        target.resize(rows, columns, false);
        archive >>
            hpx::serialization::make_array(target.data(), spacing * columns);
    }

    template <typename T>
    void load(input_archive& archive, blaze::DynamicMatrix<T, false>& target,
        unsigned)
    {
        // De-serialize matrix
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> rows >> columns >> spacing;

        target.resize(rows, columns, false);
        archive >>
            hpx::serialization::make_array(target.data(), rows * spacing);
    }

    template <typename T>
    void load(input_archive& archive, blaze::DynamicTensor<T>& target, unsigned)
    {
        // De-serialize tensor
        std::size_t pages = 0UL;
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> pages >> rows >> columns >> spacing;

        target.resize(pages, rows, columns, false);
        archive >> hpx::serialization::make_array(
                       target.data(), rows * spacing * pages);
    }

    template <typename T>
    void load(
        input_archive& archive, blaze::DynamicArray<4UL, T>& target, unsigned)
    {
        // De-serialize 4d array
        std::size_t quats = 0UL;
        std::size_t pages = 0UL;
        std::size_t rows  = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> quats >> pages >> rows >> columns >> spacing;

        target.resize(
            std::array<std::size_t, 4UL>{columns, rows, pages, quats}, false);
        archive >> hpx::serialization::make_array(
                       target.data(), rows * spacing * pages * quats);
    }
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        bool TF, typename RT>
    void load(input_archive& archive,
        blaze::CustomVector<T, AF, PF, TF, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        bool SO, typename RT>
    void load(input_archive& archive,
        blaze::CustomMatrix<T, AF, PF, SO, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void load(input_archive& archive,
        blaze::CustomTensor<T, AF, PF, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void load(input_archive& archive,
        blaze::CustomArray<4UL, T, AF, PF, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool TF>
    void save(output_archive& archive,
        blaze::DynamicVector<T, TF> const& target, unsigned)
    {
        // Serialize vector
        std::size_t count = target.size();
        std::size_t spacing = target.spacing();
        archive << count << spacing;

        archive << hpx::serialization::make_array(target.data(), spacing);
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T, true> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), spacing * columns);
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T, false> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), rows * spacing);
    }

    template <typename T>
    void save(output_archive& archive, blaze::DynamicTensor<T> const& target,
        unsigned)
    {
        // Serialize tensor
        std::size_t pages = target.pages();
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << pages << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), pages * rows * spacing);
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicArray<4UL, T> const& target, unsigned)
    {
        // Serialize 4d array
        std::size_t quats = target.quats();
        std::size_t pages = target.pages();
        std::size_t rows  = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << quats << pages << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), quats * pages * rows * spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        bool TF, typename RT>
    void save(output_archive& archive,
        blaze::CustomVector<T, AF, PF, TF, RT> const& target, unsigned)
    {
        // Serialize vector
        std::size_t count = target.size();
        std::size_t spacing = target.spacing();
        archive << count << spacing;

        archive << hpx::serialization::make_array(target.data(), spacing);
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, true, RT> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), spacing * columns);
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, false, RT> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), rows * spacing);
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void save(output_archive& archive,
        blaze::CustomTensor<T, AF, PF, RT> const& target, unsigned)
    {
        // Serialize tensor
        std::size_t pages = target.pages();
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << pages << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), pages * rows * spacing);
    }

    template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
        typename RT>
    void save(output_archive& archive,
        blaze::CustomArray<4UL, T, AF, PF, RT> const& target, unsigned)
    {
        // Serialize 4d array
        std::size_t quats = target.quats();
        std::size_t pages = target.pages();
        std::size_t rows  = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << quats << pages << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), quats * pages * rows * spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool TF>), (blaze::DynamicVector<T, TF>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool SO>), (blaze::DynamicMatrix<T, SO>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
            bool TF, typename RT>),
        (blaze::CustomVector<T, AF, PF, TF, RT>) );

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
            bool SO, typename RT>),
        (blaze::CustomMatrix<T, AF, PF, SO, RT>) );

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (blaze::DynamicTensor<T>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (blaze::DynamicArray<4UL, T>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
            typename RT>),
        (blaze::CustomTensor<T, AF, PF, RT>) );

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, blaze::AlignmentFlag AF, blaze::PaddingFlag PF,
            typename RT>),
        (blaze::CustomArray<4UL, T, AF, PF, RT>) );
}}

#endif

