endlines
========

Current version : **0.4.0**

Easy conversion between new-line conventions.

    endlines unix -r an_imported_dir an_imported_file

Three conventions are recognized : Unix, Windows, and legacy Mac. File time stamps can be preserved with `-k`. See `endlines --help` for details.

`endlines`'s advantages over `dos2unix` : 
- it attempts to detect binary files and skips them by default ; this can be overriden with `-b`
- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file (this may happen after patches/merges).
- it offers a straightforward syntax for multiple files.
- it can recurse into directories ; see `-r` option.
- it works easily on OSX (yet works the same on Linux as well) 



Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory and save it to their `/usr/local/bin`.

Linux and other POSIX users just download the repository and type `sudo make install`. 



The help screen
---------------

     ------ Convert between line ending conventions. ------
    
     Use :
       endlines CONVENTION [OPTIONS] [FILES]
    
       Input conventions are determined automatically.
       Each input file may possibly use multiple conventions. 
       CONVENTION expresses the wished output convention.
       CONVENTION can be : lf unix linux osx crlf win windows dos cr oldmac 
       If no files are specified, endlines converts from stdin to stdout.
    
     Options :
       -q / --quiet    : silence all but the error messages.
       -v / --verbose  : print more about what's going on.
       -b / --binaries : don't skip binary files.
       -k / --keepdate : keep files' last modified and last access time stamps.
       -r / --recurse  : recurse into directories.
       -h / --hidden   : process hidden files (/directories) too.
       --version       : print version number.
    
     Example :
       endlines unix -k `find . -name "*.html"`


Developers
----------

The code is very simple. Just remember to run `make test` before you come out.
