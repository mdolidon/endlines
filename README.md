| Version 1.4 | Apache License 2.0 |
| ----------- | ------------------ |

Did your source code get polluted by scattered `^M`'s ? 

Endlines smartly converts text files from and to the following line ending conventions : Unix/Apple (LF), Windows (CRLF) and legacy Apple (CR).

    endlines unix * 

- No need to know about the source files' convention. Multiple conventions can be mixed within a single file.
- Straightforward syntax for multiple files and recursion into directories. Hidden files and directories can either be avoided or processed (you don't want to mess with your `.git`, do you ?)
- Binary files will be detected and skipped by default. This can be overriden of course.
- Files' last access and last modified time stamps can be preserved.
- UTF-8 files as well as all single byte encodings will be treated well. However, UCS-2 and UTF-16/32 are not supported.


Let the help screen say it all :


       endlines ACTION [OPTIONS] [FILES]
     
       If no files are specified, endlines converts from stdin to stdout.
       ACTION can be :
         lf, unix, linux, osx    : convert all endings to LF
         crlf, windows, win, dos : convert all endings to CR-LF
         cr, oldmac              : convert all endings to CR
         check                   : perform a dry run to check current conventions
     
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
         endlines check *.txt
         endlines linux -k -r a_folder another_folder



Install
-------

Apple OSX users may get the binary from the `apple_osx_binary` directory and save it to their `/usr/local/bin`.

Linux and other POSIX users just download the repository and type `sudo make install`. 


Operating systems
-----------------

I provide support for **Linux, all BSDs and OSX**. Any POSIX compliant system should be supported out of the box.

Endlines *may* compile and run on Windows provided the proper POSIX header files are available. I won't provide any support for it, but pull requests helping with Windows support will be welcome.


Version history
--------------

New in version 1.4 :
- some very common binary file extensions (77 of them as of now), most likely to be found inside a source tree, will be considered binary files without needing to check the contents ; this allows for an even more conservative approach, in the seldom case where some binary files don't contain any non text characters

New in version 1.3 :
- Endlines reports on the original conventions it found
- `check` mode allows you to perform a dry run


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


Version 1.2 :
- earliest version without any known bug
