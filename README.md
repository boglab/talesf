This directory contains the source code for the TAL Effector Site Finder.

# License

With the exception of the Kseq library (self-contained in the `kseq.h' file),
all source code included herein is available under an ISC license.

Copyright (c) 2011-2012, Daniel S. Standage <daniel.standage@gmail.com> and
Erin Doyle <edoyle@iastate.edu>.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# Description

TALESF is a C library for identifying potential binding sites for transcription
activator-like (TAL) effectors in a given genomic sequence. Compiling and
running this program requires a C compiler with OpenMP support (such as GCC 4.2
or higher). To compile TALESF, enter the following commands in the directory

containing this file.
```
  make
  make install
  make frontend
```
To run the program with default options:
```
  ./talesf -o test_output_file path_to_fasta_input_file "RVD SEQUENCE"
```
For more information on available options:
```
  ./talesf -h
```

By default, the output data is sorted by score. The 'sortfilter' script
is provided as a tool for (surprise!) sorting and filtering output from TALESF.
For a descriptive usage statement, enter the following command.

  ./sortfilter -h