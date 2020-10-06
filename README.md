NAME
====

endlines - convert text files line ending conventions

SYNOPSIS
========

endlines ACTION \[OPTIONS\] \[FILES\]

DESCRIPTION
===========

endlines smartly converts text files from and to the following line
ending conventions : Unix/Apple (LF), Windows (CRLF) and legacy Apple
(CR).

No need to know about the source files' convention. Hidden files and
directories are skipped by default. Binary files will be detected and
skipped by default, according to a filter based on both file extension
and file content. Files' last access and last modified time stamps can
be preserved. UTF-8 files, UTF-16 with BOM as well as all single byte
encodings will be treated well. However, UTF-32 is not supported (files
will be seen as binary and left untouched). Whether converting or
checking, a report is given on the original state of line endings that
were found.

OPTIONS
=======

ACTION
------

**lf, unix, linux, osx**  
convert all endings to LF.

**crlf, windows, win, dos**  
convert all endings to CR-LF.

**cr, oldmac**  
convert all endings to CR.

**check**  
perform a dry run to check current conventions.

\[OPTIONS\]
-----------

**-f, --final**  
add final EOL if none.

**-q, --quiet**  
silence all but the error messages.

**-v, --verbose**  
print more about what's going on.

**--version**  
print version and license.

\[FILES\]
---------

**-b, --binaries**  
don't skip binary files.

**-h, --hidden**  
process hidden files (/directories) too.

**-k, --keepdate**  
keep last modified and last access times.

**-r, --recurse**  
recurse into directories.

EXAMPLES
========

**endlines check \*.txt**  
Check current conventions of all text files.

**endlines linux -kr aFolder anotherFolder**  
Convert line endings to linux, keeping date, recursive, within aFolder
and anotherFolder.

AUTHOR
======

Mathias Dolidon

LICENSE
=======

Apache License 2.0

SEE ALSO
========

Full documentation &lt;https://github.com/mdolidon/endlines&gt;
