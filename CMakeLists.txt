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
pkg_check_modules(ICUIO REQUIRED  icu-io)
pkg_check_modules(ICUUC REQUIRED icu-uc)

include_directories(${BLAS_INCLUDE_DIR})

find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
if(NOT blaze_FOUND)
  message("Blaze could not be found. Please specify blaze_DIR to assist locating it.")
endif()
include("${blaze_DIR}/blaze-config-version.cmake")
message("Blaze version: " "${PACKAGE_VERSION}")

if(DEFINED libhdfs3_DIR)
    message("-- libhdfs3_DIR defined: " "${libhdfs3_DIR}")
    if(EXISTS "${libhdfs3_DIR}/include/hdfs/hdfs.h")
        message("-- Found libhdfs3.h")
        if(EXISTS "${libhdfs3_DIR}/lib/libhdfs3.so")
            message("-- Found libhdfs3.so")
            add_executable(distparldahdfs jch.cpp documents.cpp hdfs_support.cpp results.cpp gibbs.cpp distparldalib.cpp distparldahdfs.cpp)

            target_compile_options(distparldahdfs PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
            target_compile_definitions(distparldahdfs PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_SHARED_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
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

        else()
            message("-- libhdfs3.so not found; distparldahdfs will not be built")
        endif()
    else()
            message("-- libhdfs3.h not found; distparldahdfs will not be built.")
    endif()
else()
    message("-- libhdfs3_DIR not defined; distparldahdfs will not be built.")
endif()

add_executable(lda jch.cpp documents.cpp results.cpp gibbs.cpp ldalib.cpp lda.cpp)

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

add_executable(parlda jch.cpp documents.cpp results.cpp gibbs.cpp parldalib.cpp parlda.cpp)

target_compile_options(parlda PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(parlda PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_SHARED_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
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

add_executable(distparlda jch.cpp documents.cpp results.cpp gibbs.cpp distparldalib.cpp distparlda.cpp)

target_compile_options(distparlda PUBLIC ${DISTPARLDA_CONFIG_DEFINITIONS})
target_compile_definitions(distparlda PUBLIC BLAZE_USE_HPX_THREADS BLAZE_USE_SHARED_MEMORY_PARALLELIZATION BLAZE_USE_HPX_THREADS)
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
