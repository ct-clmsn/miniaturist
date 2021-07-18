<!-- Copyright (c) 2021 Christopher Taylor                                          -->
<!--                                                                                -->
<!--   Distributed under the Boost Software License, Version 1.0. (See accompanying -->
<!--   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)        -->
# [miniaturist](https://github.com/ct-clmsn/miniaturist)

This project provides implementations of a Latent Dirichlet
Allocation algorithm found [here](https://www.ics.uci.edu/~asuncion/software/fast.htm).
Latent Dirichlet Allocation is colloquially called "topic modeling". This
project offers topic modeling capabilities that work as a sequential program,
a parallel (threaded) program, and a distributed program that can be executed
in a Cloud or on High Performance Computing (HPC)/Supercomputing systems.

The following implementations are provided:

* lda - the sequential implementation
* parlda - the parallel implementation
* distparlda - the parallel, distributed memory implementation

Optional extensions provided:

* A plugin for the [Phylanx](https://github.com/STEllAR-GROUP/phylanx) distributed array toolkit
* [Python](https://www.python.org/) bindings
* [HDFS](https://hadoop.apache.org/docs/r1.2.1/hdfs_design.html) filesystem support (the Cloud support)

The following tools are provided:

* vocab - a sequential program to compute the vocabulary set found in all
documents of a corpus
* distvocab - a distributed memory program to compute the vocabulary set
found in all documents of a corpus

The implementation uses a modified version of the collapsed gibbs
sampler as defined by Newman, Asuncion, Smyth, and Welling.

Significant modifications have been made to the Newman, et al
treatment in an effort to utilize term-document-matrices as
the storage structure for document histograms.

This implementation only scales in the direction of documents.
Users are required to provide a vocabulary list in order to make
use of this implementation.

The vocabulary building tools print out a set of words encountered during
1 linear traversal of the documents. All vocabulary building tools print
results to stdout (the terminal).

The distributed vocabulary building tool prints to the stdout of each
machine it is running upon; it is suggested that users pipe the output
of the distributed vocabulary building tool to a distributed filesystem
using a filename that is: unique to the locality identifier (integer) of
the program instance, or in a /tmp directory that is remotely
accessible for a file copy (scp).

## How To Build

To build this software, users are provided 2 options: a handrolled
makefile or cmake.

1) The makefile will need to be modified. There are comments in the
file that explain what needs to be modified.

2) cmake will require creating a directory called 'build'. Users will
change directory into 'build'. At this point the user will need to type
something like the following:

`cmake -Dblaze_DIR=<PATH_TO_BLAZE_CMAKEFILE> -DHPX_DIR=<PATH_TO_HPX_CMAKEFILE> ..`

These are possible directories where the blaze and hpx cmakefiles can
be found:

* `PATH_TO_BLAZE_CMAKEFILE=/usr/share/blaze/cmake`
* `PATH_TO_HPX_CMAKEFILE=/usr/lib/cmake/HPX`

## How To Use

Topic Modeling Program names:

* lda, single node, sequential (no threads), implementation
* parlda, single node, parallel, implementation
* distparlda, distributed (multi-node), parallel, implementation

Command line arguments for all topic modeling programs:

* --num_topics=[enter an unsigned integer value for number of topics], required
* --vocab_list=[enter a valid path to the file containing the vocabulary list], required
* --corpus_dir=[enter a valid path to the directory containing the training corpus], required
* --regex=[enter a regular expression], default [\p{L}\p{M}]+
* --num_iters=[enter an unsigned integer value for iterations], default 1000
* --alpha=[enter a floating point number for alpha prior], default 0.1
* --beta=[enter a floating point number for beta prior], default 0.01

Additional command line arguments for parlda:

* --hpx:threads=[enter an unsigned integer value for number of threads], optional

Additional command line arguments for distparlda:

* --hpx:threads=[enter an unsigned integer value for number of threads], optional
* --hpx:nodes=[enter an unsigned integer value for number of threads], optional

Vocabulary Building Program names:

* vocab, single node, sequential (no thread), vocabulary builder
* distvocab, distributed, sequential (no thread), vocabulary builder

Command line arguments for all vocabulary programs:

* --corpus_dir=[enter a valid path to the directory containing the training corpus], required
* --regex=[enter a regular expression], default [\p{L}\p{M}]+

## Implementation Notes

This implementation loads the corpus into an inverted index. The inverted index
is converted into a sparse matrix that is transposed into a document-term matrix.
The implementation then processes subsets of the document-term matrix in parallel
chunks. A sparse matrix is used to store the document-term matrix to minimize a
required O(N^2) algorithmic cost spent traversing the matrix.

The most time-consuming portion of each implementation is the creation of the sparse
matrix which stores the document-term matrix. To accelerate the creaton of the matrix,
it is strongly encouraged that a user spends a fair amount of time studying the corpus
to identify a reasonably sized vocabulary set. The larger the vocabulary set the sparser
the document-term matrix becomes. There is a direct correlation between each implementation's 
performance and the size of the vocabulary set.

## Usage Notes

If you use the vocabulary building tools with a specific regular expression in mind, make
sure to use the same regular expression when invoking lda, parlda, or distparlda. Consistent
use of regular expressions parsing text when building the vocabulary and when modeling is
important for successful program execution.

## HPX Compilation Flags

Take time to review the following build options for HPX [here](https://hpx-docs.stellar-group.org/latest/html/manual/building_hpx.html).
Below are a short list of recommended options.

For Cloud Environments:

* HPX_WITH_PARCELPORT_TCP=ON
* HPX_WITH_FAULT_TOLERANCE=ON

For HPC Environments w/MPI:

* HPX_WITH_PARCELPORT_MPI=ON

For HPC Environments w/libfabric:

* HPX_WITH_PARCELPORT_LIBFABRIC=ON
* HPX_WITH_PARCELPORT_LIBFABRIC_PROVIDER=[gni,verbs,psm2,etc]

## Licenses

This implementation uses a Jump Consistency Hash and a Bloom Filter
implementation provided by 3rd parties.

* The Jump Consistency Hash source code was taken from a blog post
with no license provided. The blog post and author are referenced
in a comment at the top of the file 'jch.hpp'.

* The Bloom Filter implementation is MIT licensed and the license
terms can be found in the file 'MIT_LICENSE'. The author is referenced
in a comment at the top of the file 'bloomfilter.hpp'.

* The remainder of the source code in this project is [Boost Licensed](https://www.boost.org/users/license.html)
and the license terms can be found in the file 'LICENSE'.

## Dependencies

* C++17
* STE||AR HPX
* Blaze
* ICU
* LAPACK
* BLAS
* OpenSSL
* pkg-config
* cmake >= 3.17

## Optional Dependencies

* Phylanx
* pybind11 (Python support)
* libhdfs3

## Special Thanks

* D. Newman, A. Asuncion, P. Smyth, M. Welling. "Distributed Algorithms for Topic Models." JMLR 2009.
* STE||AR Group ([STE||AR HPX](https://github.com/STEllAR-GROUP/hpx) & [Phylanx](https://github.com/STEllAR-GROUP/phylanx))
* Blaze C++ Library ([Blaze](https://bitbucket.org/blaze-lib/blaze/src/master/))
* Erlend Hamberg (Jump Consistency Hash)
* Daan Kolthof (Bloom Filter)

## Author

Christopher Taylor

## Date

07/03/2021
