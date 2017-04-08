/*
   This file is part of endlines' source code

   Copyright 2014-2017 Mathias Dolidon

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef _KNOWN_BINARY_EXTENSIONS_H_
#define _KNOWN_BINARY_EXTENSIONS_H_


// Only the extensions we're most likely to encounter inside the
// kind of project that also host a lot of text data : source code,
// web project etc.
const char
*known_binary_file_extensions[] = {
    // images
    "jpg", "jpeg", "tif", "tiff", "gif", "png", "tga", "bmp", "xcf", "raw", "pdf",
    "JPG", "JPEG", "TIF", "TIFF", "GIF", "PNG", "TGA", "BMP", "XCF", "RAW", "PDF",


    // sound
    "mp3", "flac", "3ga", "m4a", "wav", "aiff", "wma", "mka", "au", "ogg", "mid",
    "MP3", "FLAC", "3GA", "M4A", "WAV", "AIFF", "WMA", "MKA", "AU", "OGG", "MID",


    // video
    "flv", "avi", "mkv", "wmv", "m4v", "mp4", "vob",
    "FLV", "AVI", "MKV", "WMV", "M4V", "MP4", "VOB",


    // database
    "db", "fdb", "accdb", "gdb", "mdb", "wdb", "sqlite", "sqlite3", "db3", "dbf", 
    "DB", "FDB", "ACCDB", "GDB", "MDB", "WDB", "SQLITE", "SQLITE3", "DB3", "DBF", 

    "myd", "sdf", "s3db", "sdb", "odb", "t2d",
    "MYD", "SDF", "S3DB", "SDB", "ODB", "T2D",

    
    // office
    "doc", "docx", "xls", "xlsx", "xlsm", "ppt", "pptx", "pub", "pubx",
    "DOC", "DOCX", "XLS", "XLSX", "XLSM", "PPT", "PPTX", "PUB", "PUBX",

    "dotx", "odt", "sxw", "odp", "sxi", "stw", "sdd",
    "DOTX", "ODT", "SXW", "ODP", "SXI", "STW", "SDD",


    // archive
    "jar", "7z", "tgz", "gz", "tar", "zip", "dmg", "zlib", "pkg", "bz2", "iso", "rar",
    "JAR", "7Z", "TGZ", "GZ", "TAR", "ZIP", "DMG", "ZLIB", "PKG", "BZ2", "ISO", "RAR",


    // executable / object
    "class", "o", "exe", "swf", "swt", "swc", "dll", "so", "a", "la",
    "CLASS", "O", "EXE", "SWF", "SWT", "SWC", "DLL"  // no need to upcase for linux libraries .so, la., and .a
};
const int known_binary_file_extensions_count =
        (int)(sizeof(known_binary_file_extensions)/sizeof(known_binary_file_extensions[0]));

#endif
