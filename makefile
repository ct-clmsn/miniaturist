#  Copyright (c) 2021 Christopher Taylor
#
#  SPDX-License-Identifier: BSL-1.0
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

# MODIFY THESE VARIABLES!
#
INC_BLAS_DIR=
INC_LAPACK_DIR=
INC_OPENSSL_DIR=
INC_DIR=$(INC_BLAS_DIR) $(INC_LAPACK_DIR) $(INC_OPENSSL_DIR)
INC_HDFS_DIR=

LIB_BLAS_DIR=
LIB_LAPACK_DIR=
LIB_OPENSSL_DIR=
LIB_DIR=$(LIB_BLAS_DIR) $(LIB_LAPACK_DIR) $(LIB_OPENSSL_DIR)
LIB_HDFS_DIR=

all: distvocab vocab distparldalib.o parldalib.o ldalib.o gibbs.o documents.o results.o
	g++ -O3 -std=c++17 -mavx -o lda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 lda.cpp jch.o documents.o results.o gibbs.o ldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs`
	g++ -O3 -std=c++17 -mavx -o parlda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 parlda.cpp jch.o documents.o results.o gibbs.o parldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs`
	g++ -O3 -std=c++17 -mavx -o distparlda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 distparlda.cpp jch.o documents.o results.o gibbs.o distparldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs` -lhpx_iostreams

hdfs_support.o: hdfs_support.cpp
	g++ -O3 -std=c++17 -c $(INC_DIR) $(LIB_DIR) $(INC_HDFS_DIR) $(LIB_HDFS_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` hdfs_support.cpp -lstdc++fs `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs` -lhdfs3

distparldahdfs: hdfs_support.o
	g++ -O3 -std=c++17 -mavx -o distparlda $(INC_DIR) $(LIB_DIR) $(INC_HDFS_DIR) $(LIB_HDFS_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 distparlda.cpp jch.o documents.o hdfs_support.o results.o gibbs.o distparldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs` -lhdfs3 -lhpx_iostreams

distvocabhdfs: distvocabhdfs.cpp hdfs_support.o jch.o
	g++ -O3 -std=c++17 -mavx -o distvocabhdfs $(INC_DIR) $(LIB_DIR) $(INC_HDFS_DIR) $(LIB_HDFS_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 distvocabhdfs.cpp jch.o hdfs_support.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs` -lhdfs3 -lhpx_iostreams

distvocab: distvocab.cpp documents.o results.o
	g++ -O3 -std=c++17 -o distvocab `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` jch.o documents.o -lstdc++fs `pkg-config icu-i18n icu-io icu-uc --libs` -lssl -lcrypto `pkg-config hpx_application --libs` distvocab.cpp

vocab: vocab.cpp documents.o results.o
	g++ -O3 -std=c++17 -o vocab `pkg-config icu-i18n icu-io icu-uc --cflags` jch.o documents.o -lstdc++fs `pkg-config icu-i18n icu-io icu-uc --libs` -lssl -lcrypto vocab.cpp

distparldalib.o: gibbs.o distparldalib.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) `pkg-config hpx_application --cflags` distparldalib.cpp

parldalib.o: gibbs.o parldalib.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_USE_HPX_THREADS=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) `pkg-config hpx_application --cflags` parldalib.cpp

ldalib.o: gibbs.o ldalib.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) `pkg-config hpx_application --cflags` ldalib.cpp

gibbs.o: gibbs.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) gibbs.cpp

documents.o: documents.cpp jch.o
	g++ -O3 -std=c++17 -c `pkg-config icu-i18n icu-io icu-uc --cflags` documents.cpp

results.o: results.cpp
	g++ -O3 -std=c++17 -c `pkg-config icu-i18n icu-io icu-uc --cflags` $(INC_DIR) -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 results.cpp

jch.o: jch.cpp
	g++ -O3 -std=c++17 -c jch.cpp

clean:
	rm jch.o
	rm documents.o
	rm results.o
	rm ldalib.o
	rm parldalib.o
	rm distparldalib.o
	rm parlda
	rm lda
	rm distparlda
	rm vocab
	rm distvocab
