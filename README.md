endlines
========

Current version : **0.3.1**

Easy conversion between new-line conventions.

    endlines unix importedfiles/*

Three conventions are recognized : Unix, Windows, and legacy Mac. File time stamps can be preserved with `-k`. See `endlines --help` for details.

`endlines`'s advantages over `dos2unix` : 
- it attempts to detect binary files and skips them by default ; this can be overriden with `-b`
- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file (this may happen after patches/merges).
- it offers a straightforward syntax for multiple files.
- it works easily on OSX (yet works the same on Linux as well) 



Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory and save it to their `/usr/local/bin`.

Linux and other POSIX users just download the repository and type `sudo make install`. 


Developers
----------

The code is very simple. Just remember to run `make test` before you come out.
