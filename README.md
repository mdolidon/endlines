Current version : **1.2**

Source code got polluted by `^M`'s ? Endlines smartly converts text files from and to the following line ending conventions : Unix/Apple (LF), Windows (CRLF) and legacy Apple (CR).

    endlines unix * 

- it doesn't need to know about the source files' convention. Multiple conventions can even be intermixed within a single file.
- it attempts to detect binary files and skips them by default ; this can be overriden of course.
- it offers a straightforward syntax for multiple files and can recurse into directories, excluding or including hidden files and directories (you don't want to mess with your `.git`, do you ?)
- UTF-8 files as well as all single byte encodings will be treated well. However, UCS-2 and UTF-16/32 are not supported.
- it can preserve your files' timestamps.
- I provide support for Linux, all BSDs and OSX. Any POSIX compliant system should be supported out of the box.
- it may compile and run on Windows provided the proper POSIX header files are available. I won't provide any support for it, but pull requests helping with Windows support will be welcome.

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


Recent versions
---------------

**1.2** : fixed hidden files option

**1.1** : preserves file permissions and ownership



Developers
----------

The code is very simple. Just remember to run `make test` before you come out.
