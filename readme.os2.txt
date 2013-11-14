Exifren/2 is hereby released into the public domain.

This is a quick port from some source I found on-line.  The source was kinda
old, but the program works just fine with pictures from my digital camera
(fuji finepix 4700 zoom).  Your experinces may vary. The date format is
hard-coded to yyyy-mm-dd_xxhxxmxx.  The source code is included and you are
free to do what you wish with the code.  This software comes with no support,
no warentees and no gurantees.  I strongly suggest that you use a copy of
some files to test the program first.
If the program works with your camera, please send a quick e-mail to
kb0uov@access4less.net
Note to coders:  The source is kinda messy, but I decided not to clean it up.

Intall
--------------
Unzip program to directory of your choice.  Optionally add directory to path.
Optionally replace exifren.exe with packed\exifren.exe for warp 4 systems.

Uninstall
--------------
Delete files that were installed.

Usage
--------------
exifren [/a] f1.jpg f2.jpg      /a appends the old filename after the date/time
Note:
        exifren does NOT support wildcards, but it does support multiple
        filenames on the command line

eren.cmd [/a] wildcard.jpg      /a appends the old filename after the date/time
Note:
        eren.cmd is a cmd file that supports wildcards by using the for/do
        command, but it does NOT support multiple filemasks/filenames.

Suggested impovements for others
--------------------------------
1. Clean up code/rewrite as needed.
2. include support for wildcards.
3. Gui version?
