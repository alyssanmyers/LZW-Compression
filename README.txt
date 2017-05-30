3460:435 Algorithms Project 2 — LZW 03/06/2017
Alyssa Myers

General Description
———————————————————
LZW compression & decompression of .txt files.

Execution
—————————
1) Go to the project directory via command line.
2) lzw435 & lzw435M is to be ran using C++11 compiler.
3) The first command line argument is either -c (compression) or -e (expansion).
4) The second command line argument is for the name of the file.

Part I: General LZW
————————————————————————————————————————————————————
o Algorithm will read 8-bit code word file to compress and write back to 12-bit code word file.
o Read back file to decompress.
 
Part II: Modified LZW to allow variable length codes
————————————————————————————————————————————————————
o Algorithm will vary the length of the code words from 9 to 16.

Files
—————
o Compressed files are saved with the extensions .lzw or .lzw2
o Expanded files are saved with the original extension with 2 or 2M
o Compare the original file with the expanded file using diff

i.e. 
case1.txt -> case1.txt.lzw -> case12.txt
or
case1.txt -> case1.txt.lzw2 -> case12M.txt
