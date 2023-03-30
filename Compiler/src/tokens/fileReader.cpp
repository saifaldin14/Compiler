/*
 File: fileReader.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-27
*/

#include "../../include/tokens/fileReader.hpp"

/*
 Description:
    Constructor to read the contents of the file using Double Bufferring.
 
 Parameters:
    - fileName: The file to open (string)
 */
FileReader::FileReader(string fileName) {
    ifstream readFile(fileName);
    
    if (readFile) {
        string line;
        while (getline(readFile, line)) {
            // If the first buffer is empty, add the line to it
            if (buffer1.empty()) buffer1 = line;
            else {
                // If the first buffer is not empty, add the line to the second buffer
                buffer2 = line;
                processBuffer(buffer1);
                
                // Clear the first buffer
                buffer1.clear();
                // Swap the first buffer with the second buffer
                swap(buffer1, buffer2);
            }
        }
        // If there is any remaining data in the first buffer, process it
        if (!buffer1.empty()) processBuffer(buffer1);
    } else {
        cout << "Error: unable to open file " << fileName << std::endl;
    }
}

/*
 Description:
    Getter function to return the contents of the file.
 
 Parameters:
    NULL
 
 Returns:
    - The contents of the file as text (string)
 */
string FileReader::getFileContents() { return fileContents; }

/*
 Description:
    Do any processing of the buffer here.
    Add the buffer contents to the file contents string with a newline character.
 
 Parameters:
    - buffer: The string buffer with the file contents (string&)
 
 Returns:
    NULL
 */
void FileReader::processBuffer(string& buffer) { fileContents += buffer + "\n"; }

