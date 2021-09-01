# Copyright (c) 2021 Christopher Taylor
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Require a recent version of cmake
cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# This project is C++ based.
project(miniaturist CXX)

# Instruct cmake to find the HPX settings
include(FindPkgConfig REQUIRED)
find_package(HPX REQUIRED)
find_package(LAPACK REQUIRED)
find_package(BLAS REQUIRED)
find_package(OpenSSL REQUIRED)

pkg_check_modules(ICU18N REQUIRED icu-i18n)
pkg_check_modules(ICUIO REQUIRED icu-io)
pkg_check_modules(ICUUC REQUIRED icu-uc)

if(NOT ICUUC_FOUND)
   message("icu could not be found.")
else()
   message("-- icu version: " "${ICUUC_VERSION}")
   if(ICUUC_VERSION EQUAL "69.1")
      add_definitions(-DICU69=1)
   endif()
endif()


include_directories(${BLAS_INCLUDE_DIR})

find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
if(NOT blaze_FOUND)
  message("Blaze could not be found. Please specify blaze_DIR to assist locating it.")
endif()

include("${blaze_DIR}/blaze-config-version.cmake")
message("-- Blaze version: " "${PACKAGE_VERSION}")

find_package(pybind11 NO_CMAKE_PACKAGE_REGISTRY)
if(NOT pybind11_FOUND)
  message("-- pybind11 could not be found. Please specify pybind11_DIR to assist locating it.")
endif()

message("-- pybind11_DIR defined: " "${pybind11_DIR}")
#include_directories(${pybind11_INCLUDE_DIR})

#pybind11_add_module(pyparlda pyparlda.cpp)

add_library(ldaobj OBJECT jch.cpp documents.cpp results.cpp gibbs.cpp)
target_include_directories(ldaobj PUBLIC ${LAPACK_INCLUDE_DIRS})
target_include_directories(ldaobj PUBLIC ${BLAS_INCLUDE_DIRS})
target_include_directories(ldaobj PUBLIC ${ICU18N_INCLUDE_DIRS})
target_include_directories(ldaobj PUBLIC ${ICUIO_INCLUDE_DIRS})
target_include_directories(ldaobj PUBLIC ${ICUUC_INCLUDE_DIRS})
target_include_directories(ldaobj PUBLIC ${OPENSSL_INCLUDE_DIRS})
target_compile_options(ldaobj PUBLIC ${ICU18N_CFLAGS_OTHER})
target_compile_options(ldaobj PUBLIC ${ICUIO_CFLAGS_OTHER})
target_compile_options(ldaobj PUBLIC ${ICUUC_CFLAGS_OTHER})
target_link_directories(ldaobj PUBLIC ${OPENSSL_LIBRARY_DIRS})

if(DEFINED libhdfs3_DIR)
    message("-- libhdfs3_DIR defined: " "${libhdfs3_DIR}")
    if(EXISTS "${libhdfs3_DIR}/include/hdfs/hdfs.h")
        message("-- Found libhdfs3.h")
        if(EXISTS "${libhdfs3_DIR}/lib/libhdfs3.so")
            message("-- Found libhdfs3.so")

            add_executable(distparldahdfs hdfs_support.cpp distparldahdfs.cpp)

            target_link_libraries(distparldahdfs ldaobj distparldalib)

            target_compile_options(distparldahdfs PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
            target_compile_definitions(distparldahdfs PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_STATIC_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
            target_link_libraries(distparldahdfs -lstdc++fs)

            target_link_libraries(distparldahdfs ${HPX_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${HPX_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${HPX_INCLUDE_DIRS})

            target_link_libraries(distparldahdfs ${LAPACK_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${LAPACK_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${LAPACK_INCLUDE_DIRS})

            target_link_libraries(distparldahdfs ${BLAS_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${BLAS_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${BLAS_INCLUDE_DIRS})

            target_link_libraries(distparldahdfs ${ICU18N_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${ICU18N_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${ICU18N_INCLUDE_DIRS})
            target_compile_options(distparldahdfs PUBLIC ${ICU18N_CFLAGS_OTHER})

            target_link_libraries(distparldahdfs ${ICUIO_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${ICUIO_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${ICUIO_INCLUDE_DIRS})
            target_compile_options(distparldahdfs PUBLIC ${ICUIO_CFLAGS_OTHER})

            target_link_libraries(distparldahdfs ${ICUUC_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${ICUUC_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${ICUUC_INCLUDE_DIRS})
            target_compile_options(distparldahdfs PUBLIC ${ICUUC_CFLAGS_OTHER})

            target_link_libraries(distparldahdfs ${OPENSSL_LIBRARIES})
            target_link_directories(distparldahdfs PUBLIC ${OPENSSL_LIBRARY_DIRS})
            target_include_directories(distparldahdfs PUBLIC ${OPENSSL_INCLUDE_DIRS})

            target_link_libraries(distparldahdfs -lhdfs3)
            target_link_directories(distparldahdfs PUBLIC ${libhdfs3_DIR}/lib)
            target_include_directories(distparldahdfs PUBLIC ${libhdfs3_DIR}/include)

            add_executable(distvocabhdfs jch.cpp hdfs_support.cpp distvocabhdfs.cpp)

            target_compile_options(distvocabhdfs PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
            target_link_libraries(distvocabhdfs -lstdc++fs)

            target_link_libraries(distvocabhdfs ${HPX_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${HPX_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${HPX_INCLUDE_DIRS})

            target_link_libraries(distvocabhdfs ${LAPACK_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${LAPACK_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${LAPACK_INCLUDE_DIRS})

            target_link_libraries(distvocabhdfs ${BLAS_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${BLAS_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${BLAS_INCLUDE_DIRS})

            target_link_libraries(distvocabhdfs ${ICU18N_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${ICU18N_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${ICU18N_INCLUDE_DIRS})
            target_compile_options(distvocabhdfs PUBLIC ${ICU18N_CFLAGS_OTHER})

            target_link_libraries(distvocabhdfs ${ICUIO_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${ICUIO_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${ICUIO_INCLUDE_DIRS})
            target_compile_options(distvocabhdfs PUBLIC ${ICUIO_CFLAGS_OTHER})

            target_link_libraries(distvocabhdfs ${ICUUC_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${ICUUC_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${ICUUC_INCLUDE_DIRS})
            target_compile_options(distvocabhdfs PUBLIC ${ICUUC_CFLAGS_OTHER})

            target_link_libraries(distvocabhdfs ${OPENSSL_LIBRARIES})
            target_link_directories(distvocabhdfs PUBLIC ${OPENSSL_LIBRARY_DIRS})
            target_include_directories(distvocabhdfs PUBLIC ${OPENSSL_INCLUDE_DIRS})

            target_link_libraries(distvocabhdfs -lhdfs3)
            target_link_directories(distvocabhdfs PUBLIC ${libhdfs3_DIR}/lib)
            target_include_directories(distvocabhdfs PUBLIC ${libhdfs3_DIR}/include)

            install(
                # install all miniaturist hdfs header files
		FILES ${PROJECT_SOURCE_DIR}/hdfs_support.hpp
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/miniaturist
            )

            install(
                # install all miniaturist hdfs program files
		FILES ${CMAKE_CURRENT_BINARY_DIR}/distparldahdfs ${CMAKE_CURRENT_BINARY_DIR}/distvocabhdfs
                DESTINATION ${CMAKE_INSTALL_BINDIR}
            )

        else()
            message("-- libhdfs3.so not found; distparldahdfs will not be built")
        endif()
    else()
            message("-- libhdfs3.h not found; distparldahdfs will not be built.")
    endif()
else()
    message("-- libhdfs3_DIR not defined; distparldahdfs will not be built.")
endif()

add_library(ldalib STATIC ldalib.cpp)
target_link_libraries(ldalib ldaobj)

target_link_libraries(ldalib ${LAPACK_LIBRARIES})
target_link_directories(ldalib PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(ldalib PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(ldalib ${BLAS_LIBRARIES})
target_link_directories(ldalib PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(ldalib PUBLIC ${BLAS_INCLUDE_DIRS})

add_executable(lda lda.cpp)

target_link_libraries(lda ldalib)

target_compile_options(lda PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_link_libraries(lda -lstdc++fs)

target_link_libraries(lda ${LAPACK_LIBRARIES})
target_link_directories(lda PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(lda ${BLAS_LIBRARIES})
target_link_directories(lda PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${BLAS_INCLUDE_DIRS})

target_link_libraries(lda ${ICU18N_LIBRARIES})
target_link_directories(lda PUBLIC ${ICU18N_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${ICU18N_INCLUDE_DIRS})
target_compile_options(lda PUBLIC ${ICU18N_CFLAGS_OTHER})

target_link_libraries(lda ${ICUIO_LIBRARIES})
target_link_directories(lda PUBLIC ${ICUIO_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${ICUIO_INCLUDE_DIRS})
target_compile_options(lda PUBLIC ${ICUIO_CFLAGS_OTHER})

target_link_libraries(lda ${ICUUC_LIBRARIES})
target_link_directories(lda PUBLIC ${ICUUC_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${ICUUC_INCLUDE_DIRS})
target_compile_options(lda PUBLIC ${ICUUC_CFLAGS_OTHER})

target_link_libraries(lda ${OPENSSL_LIBRARIES})
target_link_directories(lda PUBLIC ${OPENSSL_LIBRARY_DIRS})
target_include_directories(lda PUBLIC ${OPENSSL_INCLUDE_DIRS})

add_library(parldalib STATIC parldalib.cpp)
target_link_libraries(parldalib ldaobj)

target_compile_options(parldalib PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(parldalib PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_STATIC_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)

target_link_libraries(parldalib ${HPX_LIBRARIES})
target_link_directories(parldalib PUBLIC ${HPX_LIBRARY_DIRS})
target_include_directories(parldalib PUBLIC ${HPX_INCLUDE_DIRS})

target_link_libraries(parldalib ${LAPACK_LIBRARIES})
target_link_directories(parldalib PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(parldalib PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(parldalib ${BLAS_LIBRARIES})
target_link_directories(parldalib PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(parldalib PUBLIC ${BLAS_INCLUDE_DIRS})

add_executable(parlda parlda.cpp)

target_link_libraries(parlda parldalib)

target_compile_options(parlda PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(parlda PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_STATIC_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
target_link_libraries(parlda -lstdc++fs)

target_link_libraries(parlda ${HPX_LIBRARIES})
target_link_directories(parlda PUBLIC ${HPX_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${HPX_INCLUDE_DIRS})

target_link_libraries(parlda ${LAPACK_LIBRARIES})
target_link_directories(parlda PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(parlda ${BLAS_LIBRARIES})
target_link_directories(parlda PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${BLAS_INCLUDE_DIRS})

target_link_libraries(parlda ${ICU18N_LIBRARIES})
target_link_directories(parlda PUBLIC ${ICU18N_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${ICU18N_INCLUDE_DIRS})
target_compile_options(parlda PUBLIC ${ICU18N_CFLAGS_OTHER})

target_link_libraries(parlda ${ICUIO_LIBRARIES})
target_link_directories(parlda PUBLIC ${ICUIO_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${ICUIO_INCLUDE_DIRS})
target_compile_options(parlda PUBLIC ${ICUIO_CFLAGS_OTHER})

target_link_libraries(parlda ${ICUUC_LIBRARIES})
target_link_directories(parlda PUBLIC ${ICUUC_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${ICUUC_INCLUDE_DIRS})
target_compile_options(parlda PUBLIC ${ICUUC_CFLAGS_OTHER})

target_link_libraries(parlda ${OPENSSL_LIBRARIES})
target_link_directories(parlda PUBLIC ${OPENSSL_LIBRARY_DIRS})
target_include_directories(parlda PUBLIC ${OPENSSL_INCLUDE_DIRS})

add_library(distparldalib STATIC distparldalib.cpp)
target_link_libraries(distparldalib ldaobj)

target_compile_options(distparldalib PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(distparldalib PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_STATIC_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)

target_link_libraries(distparldalib ${HPX_LIBRARIES})
target_link_directories(distparldalib PUBLIC ${HPX_LIBRARY_DIRS})
target_include_directories(distparldalib PUBLIC ${HPX_INCLUDE_DIRS})

target_link_libraries(distparldalib ${LAPACK_LIBRARIES})
target_link_directories(distparldalib PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(distparldalib PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(distparldalib ${BLAS_LIBRARIES})
target_link_directories(distparldalib PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(distparldalib PUBLIC ${BLAS_INCLUDE_DIRS})

add_executable(distparlda distparlda.cpp)

target_link_libraries(distparlda distparldalib)

target_compile_options(distparlda PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(distparlda PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_STATIC_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
target_link_libraries(distparlda -lstdc++fs)

target_link_libraries(distparlda ${HPX_LIBRARIES})
target_link_directories(distparlda PUBLIC ${HPX_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${HPX_INCLUDE_DIRS})

target_link_libraries(distparlda ${LAPACK_LIBRARIES})
target_link_directories(distparlda PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(distparlda ${BLAS_LIBRARIES})
target_link_directories(distparlda PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${BLAS_INCLUDE_DIRS})

target_link_libraries(distparlda ${ICU18N_LIBRARIES})
target_link_directories(distparlda PUBLIC ${ICU18N_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${ICU18N_INCLUDE_DIRS})
target_compile_options(distparlda PUBLIC ${ICU18N_CFLAGS_OTHER})

target_link_libraries(distparlda ${ICUIO_LIBRARIES})
target_link_directories(distparlda PUBLIC ${ICUIO_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${ICUIO_INCLUDE_DIRS})
target_compile_options(distparlda PUBLIC ${ICUIO_CFLAGS_OTHER})

target_link_libraries(distparlda ${ICUUC_LIBRARIES})
target_link_directories(distparlda PUBLIC ${ICUUC_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${ICUUC_INCLUDE_DIRS})
target_compile_options(distparlda PUBLIC ${ICUUC_CFLAGS_OTHER})

target_link_libraries(distparlda ${OPENSSL_LIBRARIES})
target_link_directories(distparlda PUBLIC ${OPENSSL_LIBRARY_DIRS})
target_include_directories(distparlda PUBLIC ${OPENSSL_INCLUDE_DIRS})

add_executable(vocab jch.cpp documents.cpp vocab.cpp)

target_compile_options(vocab PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_link_libraries(vocab -lstdc++fs)

target_link_libraries(vocab ${LAPACK_LIBRARIES})
target_link_directories(vocab PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(vocab ${BLAS_LIBRARIES})
target_link_directories(vocab PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${BLAS_INCLUDE_DIRS})

target_link_libraries(vocab ${ICU18N_LIBRARIES})
target_link_directories(vocab PUBLIC ${ICU18N_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${ICU18N_INCLUDE_DIRS})
target_compile_options(vocab PUBLIC ${ICU18N_CFLAGS_OTHER})

target_link_libraries(vocab ${ICUIO_LIBRARIES})
target_link_directories(vocab PUBLIC ${ICUIO_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${ICUIO_INCLUDE_DIRS})
target_compile_options(vocab PUBLIC ${ICUIO_CFLAGS_OTHER})

target_link_libraries(vocab ${ICUUC_LIBRARIES})
target_link_directories(vocab PUBLIC ${ICUUC_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${ICUUC_INCLUDE_DIRS})
target_compile_options(vocab PUBLIC ${ICUUC_CFLAGS_OTHER})

target_link_libraries(vocab ${OPENSSL_LIBRARIES})
target_link_directories(vocab PUBLIC ${OPENSSL_LIBRARY_DIRS})
target_include_directories(vocab PUBLIC ${OPENSSL_INCLUDE_DIRS})

add_executable(distvocab jch.cpp documents.cpp distvocab.cpp)

target_compile_options(distvocab PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_link_libraries(distvocab -lstdc++fs)

target_link_libraries(distvocab ${HPX_LIBRARIES})
target_link_directories(distvocab PUBLIC ${HPX_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${HPX_INCLUDE_DIRS})

target_link_libraries(distvocab ${LAPACK_LIBRARIES})
target_link_directories(distvocab PUBLIC ${LAPACK_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${LAPACK_INCLUDE_DIRS})

target_link_libraries(distvocab ${BLAS_LIBRARIES})
target_link_directories(distvocab PUBLIC ${BLAS_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${BLAS_INCLUDE_DIRS})

target_link_libraries(distvocab ${ICU18N_LIBRARIES})
target_link_directories(distvocab PUBLIC ${ICU18N_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${ICU18N_INCLUDE_DIRS})
target_compile_options(distvocab PUBLIC ${ICU18N_CFLAGS_OTHER})

target_link_libraries(distvocab ${ICUIO_LIBRARIES})
target_link_directories(distvocab PUBLIC ${ICUIO_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${ICUIO_INCLUDE_DIRS})
target_compile_options(distvocab PUBLIC ${ICUIO_CFLAGS_OTHER})

target_link_libraries(distvocab ${ICUUC_LIBRARIES})
target_link_directories(distvocab PUBLIC ${ICUUC_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${ICUUC_INCLUDE_DIRS})
target_compile_options(distvocab PUBLIC ${ICUUC_CFLAGS_OTHER})

target_link_libraries(distvocab ${OPENSSL_LIBRARIES})
target_link_directories(distvocab PUBLIC ${OPENSSL_LIBRARY_DIRS})
target_include_directories(distvocab PUBLIC ${OPENSSL_INCLUDE_DIRS})

install(
    # install all miniaturist header files
    FILES ${PROJECT_SOURCE_DIR}/gibbs.hpp ${PROJECT_SOURCE_DIR}/inverted_index.hpp ${PROJECT_SOURCE_DIR}/jch.hpp ${PROJECT_SOURCE_DIR}/parldalib.hpp ${PROJECT_SOURCE_DIR}/results.hpp ${PROJECT_SOURCE_DIR}/distparldalib.hpp ${PROJECT_SOURCE_DIR}/documents.hpp ${PROJECT_SOURCE_DIR}/hdfs_support.hpp ${PROJECT_SOURCE_DIR}/inverted_index_serialize.hpp ${PROJECT_SOURCE_DIR}/ldalib.hpp ${PROJECT_SOURCE_DIR}/serialize.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/miniaturist
)

install(
    # install all miniaturist library files
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libdistparldalib.a ${CMAKE_CURRENT_BINARY_DIR}/libldalib.a ${CMAKE_CURRENT_BINARY_DIR}/libparldalib.a
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/miniaturist
)

install(
    # install all miniaturist program files
    FILES ${CMAKE_CURRENT_BINARY_DIR}/distparlda ${CMAKE_CURRENT_BINARY_DIR}/lda ${CMAKE_CURRENT_BINARY_DIR}/parlda ${CMAKE_CURRENT_BINARY_DIR}/distvocab ${CMAKE_CURRENT_BINARY_DIR}/vocab
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(pybind11_FOUND)

    pybind11_add_module(pylda jch.cpp documents.cpp results.cpp gibbs.cpp ldalib.cpp pylda.cpp)
    target_link_libraries(pylda PRIVATE -lstdc++fs)

    target_link_libraries(pylda PRIVATE ${LAPACK_LIBRARIES})
    target_link_directories(pylda PRIVATE ${LAPACK_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${LAPACK_INCLUDE_DIRS})

    target_link_libraries(pylda PRIVATE ${BLAS_LIBRARIES})
    target_link_directories(pylda PRIVATE ${BLAS_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${BLAS_INCLUDE_DIRS})

    target_link_libraries(pylda PRIVATE ${ICU18N_LIBRARIES})
    target_link_directories(pylda PRIVATE ${ICU18N_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${ICU18N_INCLUDE_DIRS})
    target_compile_options(pylda PRIVATE ${ICU18N_CFLAGS_OTHER})

    target_link_libraries(pylda PRIVATE ${ICUIO_LIBRARIES})
    target_link_directories(pylda PRIVATE ${ICUIO_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${ICUIO_INCLUDE_DIRS})
    target_compile_options(pylda PRIVATE ${ICUIO_CFLAGS_OTHER})

    target_link_libraries(pylda PRIVATE ${ICUUC_LIBRARIES})
    target_link_directories(pylda PRIVATE ${ICUUC_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${ICUUC_INCLUDE_DIRS})
    target_compile_options(pylda PRIVATE ${ICUUC_CFLAGS_OTHER})

    target_link_libraries(pylda PRIVATE ${OPENSSL_LIBRARIES})
    target_link_directories(pylda PRIVATE ${OPENSSL_LIBRARY_DIRS})
    target_include_directories(pylda PRIVATE ${OPENSSL_INCLUDE_DIRS})

    target_include_directories(pylda PRIVATE ${pybind11_INCLUDE_DIR})

endif()
