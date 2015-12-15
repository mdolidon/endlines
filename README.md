endlines
========

Current version : **1.0**

Easy conversion between new-line conventions.

    endlines unix -r an_imported_dir 


`endlines`'s advantages over `dos2unix` : 
- it attempts to detect binary files and skips them by default ; this can be overriden with `-b`
- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file (this may happen after patches/merges).
- it offers a straightforward syntax for multiple files.
- it can recurse into directories ; see `-r` option.
- it works easily on OSX (yet works the same on Linux as well) 


**Note about Unicode**
UTF-8 is supported. UCS-2 and UTF-16/32 are not. 



The help screen
---------------

     ------ Convert line endings  ------

     Use :
       endlines OUT_CONVENTION [OPTIONS] [FILES]

       Input conventions are determined automatically.
       Each input file may possibly use multiple conventions. 
       OUT_CONVENTION can be : lf unix linux osx crlf win windows dos cr oldmac 
       If no files are specified, endlines converts from stdin to stdout.

     General options :
       -q / --quiet    : silence all but the error messages.
       -v / --verbose  : print more about what's going on.
       --version       : print version number.

     File options :
       -b / --binaries : don't skip binary files.
       -k / --keepdate : keep files' last modified and last access time stamps.
       -r / --recurse  : recurse into directories.
       -h / --hidden   : process hidden files (/directories) too. 

     Examples :
       endlines unix *.txt
       endlines win -k -r a_folder another_folder


Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory and save it to their `/usr/local/bin`.

Linux and other POSIX users just download the repository and type `sudo make install`. 


Developers
----------

The code is very simple. Just remember to run `make test` before you come out.


Latest releases
---------------

1.0 : promoting 0.4.2, considered bug free

0.4.2 : bug fix : the program does not hang anymore on empty files.

0.4.1 : infers hidden file names correctly.

0.4.0 : introduces directory recursion and skipping hidden file ; known bug in hidden file name detection.
