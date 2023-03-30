/*
 File: fileReader.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-27
*/

#ifndef FILE_READER_H
#define FILE_READER_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

class FileReader {
public:
    FileReader(string fileName);
    string getFileContents();
private:
    string buffer1, buffer2;
    string fileContents;
    void processBuffer(string& buffer);
};


#endif /* fileReader_hpp */

