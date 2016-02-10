| Version 1.2 | Apache License 2.0 |
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


Operating systems
-----------------

I provide support for **Linux, all BSDs and OSX**. Any POSIX compliant system should be supported out of the box.

Endlines *may* compile and run on Windows provided the proper POSIX header files are available. I won't provide any support for it, but pull requests helping with Windows support will be welcome.
