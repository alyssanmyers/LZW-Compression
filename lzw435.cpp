/***********************************************/
// Alyssa Myers
// 3460:435 Algorithms
// Project 2 — LZW
// March 6, 2017
//
// LZW compression & decompression of files.
/***********************************************/

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <cmath>

#define DICT_BITS 12

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
template <typename Iterator>
Iterator compress(std::vector<char> &uncompressed, Iterator result)
{
    // 2^8
    int dictionary_base = std::pow(2,8);
    
    // 2^12
    int dictionary_limit = std::pow(2,DICT_BITS);
    
    // current size
    int dictionary_size = dictionary_base;
    
    // build dictionary
    std::map<std::string,int> dictionary;
    for (int i = 0; i < dictionary_base; i++)
        dictionary[std::string(1, i)] = i;
    
    std::string word;
    for(std::string::const_iterator it = uncompressed.begin();
        it != uncompressed.end(); it++)
    {
        // retrieve next character
        char value = *it;
        
        // build prefix
        std::string prefix = word + std::string(1,value);
    
        // if prefix exists, build on
        if(dictionary.count(prefix))
        {
            word = prefix;
        }
        // add to the dictionary
        else
        {
            // add to the returned hash
            *result++ = dictionary[word];
            
            if(dictionary_size < dictionary_limit)
            {
                // add prefix to the dictionary
                dictionary[prefix] = dictionary_size++;
            }
            
            // reset word to base
            word = std::string(1, value);
        }
    }
    
    // Output the code for word
    if (!word.empty())
    {
        *result++ = dictionary[word];
    }
    
    return result;
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end)
{
    // 2^8
    int dictionary_base = std::pow(2,8);
    
    // 2^12
    int dictionary_limit = std::pow(2,DICT_BITS);
    
    // current size
    int dictionary_size = dictionary_base;
    
    // build dictionary
    std::map<int,std::string> dictionary;
    for (int i = 0; i < dictionary_base; i++)
        dictionary[i] = std::string(1, i);
    
    std::string word(1, *begin++);
    std::string result = word;
    std::string entry;
    
    for (; begin != end; begin++)
    {
        try
        {
            // retrieve
            int value = *begin;
            
            // if it exists, retrieve it
            if (dictionary.count(value))
            {
                entry = dictionary[value];
            }
            // build entry to create
            else if (value == dictionary_size)
            {
                entry = word + word[0];
            }
            else throw "Bad compression.";
            
            result += entry;

            // Add w+entry[0] to the dictionary.
            dictionary[dictionary_size++] = word + entry[0];
            
            word = entry;
        }
        catch (const char* e)
        {
            std::cout << e << std::endl;
            break;
        }
    }
    return result;
}

std::string int2BinaryString(int c, int cl)
{
    //a binary code string with code length = cl
    std::string p = "";
    
    // converting number into binary
    while (c > 0)
    {
        if(c % 2 == 0) p = "0" + p;
        else p = "1" + p;
        c = c >> 1;
    }
    int zeros = cl - p.size();
    if (zeros < 0)
    {
        std::cout << "\nWarning: Overflow. code is too big to be coded by " << cl <<" bits!\n";
        p = p.substr(p.size() - cl);
    }
    else
    {
        //pad 0s to left of the binary code if needed
        for (int i = 0; i < zeros; i++) p = "0" + p;
    }
    return p;
}

int binaryString2Int(std::string p)
{
    int code = 0;
    if (p.size()>0)
    {
        if (p.at(0)=='1') code = 1;
        p = p.substr(1);
        while (p.size()>0)
        {
            code = code << 1; 
            if (p.at(0)=='1') code++;
            p = p.substr(1);
        }
    }
    return code;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "You must provide two arguments, (c/e) (filename)." << std::endl;
        return 1;
    }
    else if(!strcmp(argv[1], "c"))
    {
        std::string file(argv[2]);
        std::vector<int> compressed;
        std::vector<char> content;
        std::string zeros = "00000000";
        long fsize, osize;
        struct stat filestatus, outputstatus;
        
        std::cout << "Compressing " << file << std::endl;
        
        try
        {
            // read the file
            std::ifstream in(file.c_str(), std::ios::binary);
        
            // get the size of the file in bytes
            stat(file.c_str(), &filestatus);
            fsize = filestatus.st_size;
        
            auto const start_pos = in.tellg();
            in.ignore(std::numeric_limits<std::streamsize>::max());
            auto char_count = in.gcount();
            in.seekg(start_pos);
            content = std::vector<char>(char_count);
            in.read(&content[0], content.size());

            in.close();
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // compress
        compress(content, std::back_inserter(compressed));

        // 8 bit binary string to 12 bit binary string
        std::string binary_code, p;
        for (std::vector<int>::iterator it = compressed.begin() ; it != compressed.end(); ++it)
        {
            p = int2BinaryString(*it, DICT_BITS);
            binary_code += p;
        }
        
        // store to file in 8 bytes
        try
        {
            // make sure the length of the binary string is a multiple of 8
            std::ofstream out((file + ".lzw").c_str(), std::ios::binary);
            if(binary_code.size() % 8 != 0)
                binary_code += zeros.substr(0, 8 - binary_code.size() % 8);
        
            int b;
            for(int i = 0; i < binary_code.size(); i += 8)
            {
                b = 1;
                for(int j = 0; j < 8; j++)
                {
                    b = b << 1;
                    if(binary_code.at(i+j) == '1')
                        b += 1;
                }
                
                // save the string byte by byte (8 bits)
                char c = (char) (b & 255);
                out.write(&c, 1);
        }
        out.close();
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // get the size of the file in bytes;
        stat((file+".lzw").c_str(), &outputstatus);
        osize = outputstatus.st_size;
        
        std::cout << "File bytes : " << fsize << " -> " << osize << std::endl;
    }
    else if(!strcmp(argv[1], "e"))
    {
        std::string file(argv[2]);
        std::vector<int> compressed;
        std::vector<char> content;
        std::string buffer;
        long fsize, osize;
        struct stat filestatus, outputstatus;
        
        std::cout << "Expanding " << file << std::endl;
        
        try
        {
            // read the file
            std::ifstream in(file.c_str(), std::ios::binary);
        
            // get the size of the file in bytes
            stat(file.c_str(), &filestatus);
            fsize = filestatus.st_size;
        
            auto const start_pos = in.tellg();
            in.ignore(std::numeric_limits<std::streamsize>::max());
            auto char_count = in.gcount();
            in.seekg(start_pos);
            content = std::vector<char>(char_count);
            in.read(&content[0], content.size());
        
            std::string zeros = "000000000000";
            long count = 0;
            while(count < fsize)
            {
                unsigned char uc = (unsigned char) content[count];
                std::string p = "";
                for(int j = 0; j < 8 && uc > 0; j++)
                {
                    if(uc % 2 == 0) p = "0" + p;
                    else p = "1" + p;
                    uc = uc >> 1;
                }
                p = zeros.substr(0, 8-p.size()) + p;
                buffer += p;
                count++;
            }
            in.close();
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // 12 to 8 bits, string to int
        std::string p;
        int word;
        for (int i = 0; i < buffer.length(); i += DICT_BITS)
        {
            p = "";
            p = buffer.substr(i, DICT_BITS);
            word = binaryString2Int(p);
            compressed.push_back(word);
        }
       
        // decompress
        std::string decompressed = decompress(compressed.begin(), compressed.end());
        
        // to char
        long dsize = decompressed.length() + 1;
        char d[dsize];
        strcpy(d, decompressed.c_str());

        // creating file string
        std::string name = file.substr(0, file.find('.')) + "2";
        std::string ext = file.substr(file.find('.'), file.length());
        ext.erase(ext.length()-4);
        std::string output = name + ext;

        try
        {
            std::ofstream out(output.c_str(), std::ios::binary);
            out << d;
            out.close();
        }
        catch(std::exception &e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // get the size of the file in bytes
        stat((output).c_str(), &outputstatus);
        osize = outputstatus.st_size;
        
        std::cout << "File bytes : " << fsize << " -> " << osize << std::endl;
    }
    else
    {
        std::cout << "Error: invalid arguments (" << argv[1] << ") (" << argv[2] << ")." << std::endl;
        return 1;
    }
    return 0;
}


