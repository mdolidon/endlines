| Version 1.9.1 | Apache License 2.0 |
| ------------- | ------------------ |

Did your source code get polluted by scattered `^M`'s ?

Endlines smartly converts text files from and to the following line ending conventions : Unix/Apple (LF), Windows (CRLF) and legacy Apple (CR).

    endlines unix *

- No need to know about the source files' convention. Multiple conventions can be mixed within a single file.
- Straightforward syntax for multiple files and recursion into directories. Hidden files and directories are skipped by default (you don't want to mess with your `.git`, do you ?)
- Binary files will be detected and skipped by default, according to a filter based on both file extension and file content.
- Files' last access and last modified time stamps can be preserved.
- UTF-8 files, UTF-16 with BOM as well as all single byte encodings will be treated well. However, UTF-32 is not supported (files will be seen as binary and left untouched).
- Whether converting or checking, a report is given on the original state of line endings that were found.

```
    $ endlines check -r .
    endlines : dry run, scanning files

    endlines : 6431 files checked ; found :
                  - 142 No line ending
                  - 1 Legacy Mac (CR)
                  - 6250 Unix (LF)
                  - 37 Windows (CR-LF)
                  - 1 Mixed endings
               2403 binaries skipped
               480 hidden files skipped
```


## Install

### From Debian package

For Linux/AMD64, you'll find the `endlines-1.9.1-amd64.deb` file in the `binaries` folder. You can install it with a GUI installer (like gdebi) or run `sudo dpkg --install endlines-1.9.1-amd64.deb` from the command line.


### From binaries

The `binaries` folder contains precompiled binaries for Linux/AMD64 and OSX. You can rename the one that fits you to `endlines` and move it to `/usr/local/bin` or to your local path.


### From source

- Local install : `make; make test` ; if satisfied, move the `endlines` executable to your local path.
- Global install : `make; make test; sudo make install` will put an `endlines` executable in `/usr/local/bin`.

Endlines is known to have been compiled and run out of the box on Apple OSX, several Linux distributions and IBM AIX. I provide support for all POSIX compliant operating sytems. I won't provide any support for Windows, but pull requests dealing with it will be welcome.
