# luthien
Parallel windowed adaptive trimming for fastq files using quality score.

**status**: Still implementing basic functionality. Come see us later.

## Windowed Adaptive Trimming
This sofware uses [sickle](https://github.com/najoshi/sickle)'s algorithm for fastq trimming, in which a sliding window with 
adaptive size is used to determine where to cut the original sequence.

## Features we want to implement
- Single End fastq processing;
- Paired End fastq processing;
- Parallel processing through an producer-consumer model;
- Automatic detection of fastq scoring scheme (sanger, illumina or solexa);
- Reading and writing gzip compressed fastq files;
