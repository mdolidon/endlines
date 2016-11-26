| Version 1.8 | Apache License 2.0 |
| ----------------- | ------------------ |

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

Let the help screen say it all :


      endlines ACTION [OPTIONS] [FILES]
    
      ACTION can be :
        lf, unix, linux, osx    : convert all endings to LF.
        crlf, windows, win, dos : convert all endings to CR-LF.
        cr, oldmac              : convert all endings to CR.
        check                   : perform a dry run to check current conventions.
    
      If no files are specified, endlines converts from stdin to stdout.
      Supports UTF-8, UTF-16 with BOM, and all major single byte codesets.
    
      General   -q / --quiet    : silence all but the error messages.
                -v / --verbose  : print more about what's going on.
                --version       : print version and license.
    
      Files     -b / --binaries : don't skip binary files.
                -h / --hidden   : process hidden files (/directories) too.
                -k / --keepdate : keep last modified and last access times.
                -r / --recurse  : recurse into directories.
    
      Examples  endlines check *.txt
                endlines linux -k -r aFolder anotherFolder


## Install

### From binaries

The `binaries` folder contains precompiled binaries for Linux/AMD64 and OSX. You can rename the one that fits you to `endlines` and move it to `/usr/local/bin` or to your local path.


### From source

- Local install : `make; make test` ; if satisfied, move the `endlines` executable to your local path.
- Global install : `make; make test; sudo make install` should suffice.



## Operating systems

Endlines is known to have been compiled and run out of the box on Apple OSX, several Linux distributions and IBM AIX. I provide support for all POSIX compliant operating sytems.

Endlines *may* compile and run on Windows provided the proper POSIX header files are available. I won't provide any support for it, but pull requests helping with Windows support will be welcome.


## News

Version 1.8 is safe for concurrent launches.

Version 1.7 embeds the speed and robustness improvements of quickly paced previous versions. I strongly encourage you to upgrade. I removed the tags on purpose. Besides, I don't have a Mac anymore, and this will be the last version that I can compile for OSX by myself. If somebody is willing to provide support with OSX builds, I'll be glad to team up. Please just open an issue.

