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
LIB_BLAS_DIR=
LIB_LAPACK_DIR=
LIB_OPENSSL_DIR=
LIB_DIR=$(LIB_BLAS_DIR) $(LIB_LAPACK_DIR) $(LIB_OPENSSL_DIR)

all: distparldalib.o parldalib.o ldalib.o gibbs.o documents.o results.o
	g++ -O3 -std=c++17 -mavx -o lda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 lda.cpp jch.o documents.o results.o gibbs.o ldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs`
	g++ -O3 -std=c++17 -mavx -o parlda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 parlda.cpp jch.o documents.o results.o gibbs.o parldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs`
	g++ -O3 -std=c++17 -mavx -o distparlda $(INC_DIR) $(LIB_DIR) `pkg-config icu-i18n icu-io icu-uc --cflags` `pkg-config hpx_application --cflags` -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 distparlda.cpp jch.o documents.o results.o gibbs.o distparldalib.o -lstdc++fs -lssl -lcrypto -lblas -llapack `pkg-config icu-i18n icu-io icu-uc --libs` `pkg-config hpx_application --libs` -lhpx_iostreams

distparldalib.o: gibbs.o distparldalib.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) `pkg-config hpx_application --cflags` distparldalib.cpp

parldalib.o: gibbs.o parldalib.cpp
	g++ -O3 -std=c++17 -mavx -c -DBLAZE_USE_VECTORIZATION=1 -DBLAZE_BLAS_MODE=1 $(INC_DIR) `pkg-config hpx_application --cflags` parldalib.cpp

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
