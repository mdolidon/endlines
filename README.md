endlines
========

Current version : **0.2.1**

Easy conversion between new-line conventions.

    endlines unix importedfiles/*

Three conventions are recognized : Unix, Windows, and legacy Mac. See `endlines --help` for details.

You'll find this somewhat similar to GNU's dos2unix and unix2dos utilities.

`endlines`'s advantages : 
- it attempts to detect binary files and skips them by default ; this can be overriden with `-b`
- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file (this may happen after patches/merges).
- it offers easy syntax for multiple files.
- it works easily on OSX (yet works the same on Linux as well) 

`endlines`'s (temporary) drawback :
- there isn't yet a way to preserve the files' timestamps. This is due to the unavailability of POSIX `utimensat` and `futimens` in OSX. I may put together a system-dependent work-around later on... or not.




Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory.

Linux and other POSIX users just download the repository and type `sudo make install`. 


Developers
----------

The code is very simple. Just remember to run `make test` before you come out.
