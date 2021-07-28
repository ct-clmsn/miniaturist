<!-- Copyright (c) 2021 Christopher Taylor                                          -->
<!--                                                                                -->
<!--   Distributed under the Boost Software License, Version 1.0. (See accompanying -->
<!--   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)        -->
<!--                                                                                -->
# [miniaturist](https://github.com/ct-clmsn/miniaturist)

This project provides implementations of a Latent Dirichlet
Allocation algorithm found [here](https://www.ics.uci.edu/~asuncion/software/fast.htm).
Latent Dirichlet Allocation is colloquially called "topic modeling". This
project offers topic modeling capabilities that work as a sequential program,
a parallel (threaded) program, and a distributed parallel program that can be
executed on High Performance Computing (HPC)/Supercomputing systems or a Cloud.

The following implementations are provided:

* lda - the sequential implementation
* parlda - the parallel implementation for multi-core systems
* distparlda - distributed, parallel, implementation for High Performance Computers (HPC)/Supercomputers
* distparldahdfs - distributed, parallel, implementation for Clouds ([Hadoop filesystem, HDFS](https://hadoop.apache.org/docs/r1.2.1/hdfs_design.html))

Optional extensions provided:

* A plugin for the [Phylanx](https://github.com/STEllAR-GROUP/phylanx) distributed array toolkit
* [Python](https://www.python.org/) bindings (modules: pylda, pyparlda, pydistparlda)

The following tools are provided:

* vocab - a sequential program to compute the vocabulary set found in all
documents of a corpus
* distvocab - a distributed memory program to compute the vocabulary set
found in all documents of a corpus
* distvocabhdfs - a distributed memory program to compute the vocabulary set
found in all documents of a corpus on HDFS

The implementation uses a modified version of the collapsed gibbs
sampler as defined by Newman, Asuncion, Smyth, and Welling.

Significant modifications have been made to the Newman, et al
treatment in an effort to utilize term-document-matrices as
the storage structure for document histograms.

This implementation only scales in the direction of documents. Users
are required to provide a vocabulary list in order to make use of this
implementation. Vocabulary lists can be populated programmatically or
loaded into a modeling program with a file containing a 'new-line'
delimited list of words.

The vocabulary building tools print out a set of words encountered during
1 linear traversal of the documents. All vocabulary building tools print
results to stdout (the terminal).

The distributed vocabulary building tool prints to the stdout of each
machine it is running upon; it is suggested that users pipe the output
of the distributed vocabulary building tool to a distributed filesystem
using a filename that is: unique to the locality identifier (integer) of
the program instance, or into a remote /tmp directory that is accessible
for a file copy (scp).

## How To Build

This project requires using cmake. cmake requires creating a directory
called 'build'. Users will change directory into 'build'. At this point
the user will need to type something like the following:

`cmake -Dblaze_DIR=<PATH_TO_BLAZE_CMAKEFILE> -DHPX_DIR=<PATH_TO_HPX_CMAKEFILE> ..`

These are possible directories where the blaze and hpx cmakefiles can
be found:

* `PATH_TO_BLAZE_CMAKEFILE=/usr/share/blaze/cmake`
* `PATH_TO_HPX_CMAKEFILE=/usr/lib/cmake/HPX`

Add the following for Cloud (Hadoop File System - HDFS) support:

`cmake -Dblaze_DIR=<PATH_TO_BLAZE_CMAKEFILE> -DHPX_DIR=<PATH_TO_HPX_CMAKEFILE> -Dlibhdfs3_DIR=<PATH_TO_LIBHDFS3_INSTALL> ..`

This is a possible location for libhdfs3.so and hdfs/hdfs.h:

* `PATH_TO_LIBHDFS3_INSTALL=/usr/`

Add the following for Python bindings to be built. To inform cmake where
pybind11 is installed, use the `-Dpybind11_DIR=<PATH_TO_PYBIND11_CMAKEFILE>`
flag.

`cmake -Dblaze_DIR=<PATH_TO_BLAZE_CMAKEFILE> -DHPX_DIR=<PATH_TO_HPX_CMAKEFILE> -Dpybind11_DIR=<PATH_TO_PYBIND11_CMAKEFILE>`

Here is a possible directory where the pybind11 cmakefiles are located:

* `PATH_TO_PYBIND11_CMAKEFILE=/usr/share/cmake/pybind11`

## How To Use

Topic Modeling Program names:

* lda, single node, sequential (no threads), implementation
* parlda, single node, parallel, implementation
* distparlda, distributed (multi-node), parallel, implementation
* distparldahdfs, distributed (multi-node), parallel, HDFS implementation

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

Additional command line arguments for distparldahdfs:

* --hdfs_namenode_address=[enter string], required
* --hdfs_namenode_port=[unsigned integer for hdfs namenode port], required
* --hdfs_buffer_size=[unsigned integer buffer size for file reads from hdfs], default 1024

Vocabulary Building Program names:

* vocab, single node, sequential (no thread), vocabulary builder
* distvocab, distributed, sequential (no thread), vocabulary builder
* distvocabhdfs, distributed, sequential (no thread), vocabulary builder for HDFS

Command line arguments for all vocabulary programs:

* --corpus_dir=[enter a valid path to the directory containing the training corpus], required
* --regex=[enter a regular expression], default [\p{L}\p{M}]+

Additional command line arguments for distvocabhdfs:

* --hdfs_namenode_address=[enter string], required
* --hdfs_namenode_port=[unsigned integer for hdfs namenode port], required
* --hdfs_buffer_size=[unsigned integer buffer size for file reads from hdfs], default 1024

Topic Modeling Libraries:

* libldalib.a, ldalib.hpp single node, sequential (no threads), implementation
* libparldalib.a, parldalib.hpp single node, parallel, implementation
* libdistparldalib.a, distparldalib.hpp distributed (multi-node), parallel, implementation

The Python bindings requires users to type the following in python3.8:

`from pylda import lda`

## Implementation Notes

This implementation loads the corpus into an inverted index. The inverted index
is converted into a sparse matrix that is transposed into a document-term matrix.
This step is required to permit the parallel processing of the corpus by document.
The implementation processes subsets of the document-term matrix in parallel chunks.
A sparse matrix is used to store the document-term matrix to minimize a
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

Dynamic runtime instrumentation for Cloud and/or HPC:

* HPX_WITH_APEX=ON (requires [APEX](https://github.com/khuck/xpress-apex) installation)

For OpenMP enabled LAPACK, BLAS, Blaze implementations:

* [hpxMP](https://github.com/STEllAR-GROUP/hpxMP) - provides HPX/APEX the ability to manage OpenMP

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

* APEX
* hpxMP
* Phylanx
* pybind11 (Python support)
* libhdfs3 (Hadoop Filesystem/HDFS support)

## Special Thanks

* D. Newman, A. Asuncion, P. Smyth, M. Welling. "Distributed Algorithms for Topic Models." JMLR 2009.
* STE||AR Group ([STE||AR HPX](https://github.com/STEllAR-GROUP/hpx) & [Phylanx](https://github.com/STEllAR-GROUP/phylanx))
* Blaze C++ Library ([Blaze](https://bitbucket.org/blaze-lib/blaze/src/master/))
* Erlend Hamberg (Jump Consistency Hash)
* Daan Kolthof (Bloom Filter)
* Erik Muttersbach ([libhdfs3](https://github.com/erikmuttersbach/libhdfs3))
* Kevin A. Huck, Allan Porterfield, Nick Chaimov, Hartmut Kaiser, Allen D. Malony, Thomas Sterling, Rob Fowler. "An Autonomic Performance Environment for eXascale"; [APEX](https://github.com/khuck/xpress-apex)
* Jeremy Kemp; Tianyi Zhang; Shahrzad Shirzad; Bryce Adelstein Lelbach aka wash; Hartmut Kaiser; Bibek Wagle; Parsa Amini; Alireza Kheirkhahan. "[hpxMP](https://github.com/STEllAR-GROUP/hpxMP) v0.3.0: An OpenMP runtime implemented using HPX"

## Author

Christopher Taylor

## Date

07/03/2021
