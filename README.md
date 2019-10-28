# luthien
Super ultra-fast fastq preprocessor, using parallel windowed adaptive trimming.

## Windowed Adaptive Trimming
This sofware uses [sickle](https://github.com/najoshi/sickle)'s algorithm for fastq trimming, in which a sliding window with 
adaptive size is used to determine where to cut the original sequence.

## Features
- Single End fastq processing;
- Paired End fastq processing;
- Parallel processing and IO through a producer-consumer model;
- Very low memory usage, can be used on any modern personal computer;

## Features we want to implement
- Automatic detection of fastq scoring scheme (sanger, illumina or solexa);
- Reading and writing gzip compressed fastq files;

## How to use

### Compile

Luthien requires a G++ compiler with C++17 enabled. If you've got one installed, simple enter the repository and type:

```sh
$ make
```

### Command Line Interface

Luthien requires one or two .fastq files as input and outputs the results as .fastq files.

```sh
$ ./luthien

usage: 

required:
  -i1	The first fastq input file.
  -i2	The second fastq input file, enables paired end mode.
  -o1	The first fastq output file.
  -o2	The second fastq output file, required only for paired end mode.

optional:
  -s	Output single reads.
  -t	Number of worker threads. Maximum: Total number of cores.
		  Default:3
  -c	Luthien reads the input files in chunks with this maximum length.
	  	Default:12MB
  -b	Maximum chars from fastq to store in memory.
  		Default:72MB
  -nc	The number of chunks to load for each thread.
  -q	Minimum quality score.
  		Default:20
  -qt	Quality score type: sanger(default), illumina or solexa.
  -h --help	Print this help.
```

For single end:

```sh
$ luthien -i1 test/test.fastq -o1 test.fastq -t 4
```

For paired end:

```sh
$ luthien -i1 test/test.f.fastq -i2 test/test.r.fastq -o1 test.f.fastq -o2 test.r.fastq -t 4
```
