endlines
========

Easy conversion between new-line conventions.

    endlines unix importedfiles/*

Three conventions are recognized : Unix, Windows, and legacy Mac. See `endlines --help` for details.

You'll find this somewhat similar to GNU's dos2unix and unix2dos utilities.

`endlines`'s advantages : 
- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file (this may happen after patches/merges).
- works easily on OSX ; works the same on Linux too.
- offers easy syntax for multiple files.

`endlines`'s (temporary) drawback :
- there isn't yet a way to preserve the files' timestamps. This is due to the unavailability of POSIX `utimensat` and `futimens` in OSX. I may put together a system-dependent work-around later on... or not.




Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory.

Linux and other POSIX users just download the repository and type `sudo make install`. 


Developers
----------

The code is very simple. Just remember to run `make test` before you come out.
