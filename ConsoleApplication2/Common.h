// Common constants, reading UI, Error handling
#define __COMMON__H__

#include <iostream>
#include <fstream>
#include <sstream> 
#include <string>
#include <assert.h>
#include <boost/filesystem.hpp>

enum ClassConstants
{
    // Defaults
    DEFAULT_BUFFER_SIZE = 1024,
    BYTES_IN_KILOBYTE = 1024,
  
    // Multithread
    MAX_THREADS = 16,
};

enum Exeptions
{
    NO_ERRORS = 0,
    EX_LOW_ARGUMENTS = 1,
    EX_INPUT_FILE_MISSING = 2,
    EX_READ_ERROR = 3,
    EX_WRITE_ERROR = 4,
    EX_ZERO_BUFF = 5,
    EX_UNKNOWN_ERROR = 6,
    EX_ZERO_SIZE = 7,
    // Range
    EX_COUNT = 8,
    EX_INVALID = 9,
};


const char* ExeptionStrings[] = {
  "No Error",
  "Not enough data. Program needs at least input filename and output file name",
  "Unable to locate input file",
  "Can not read from input file",
  "Can not write to the output file",
  "Buffer size can not be 0",
  "Unknown error happend",
  "Input file is zero size",
};

class Common
{
public:
    char*         inFileName;
    char*         outFileName;
    unsigned int  bufferSize;
    uintmax_t     fileSize;
    uintmax_t     lastBlockNumber;

    std::chrono::system_clock::time_point timeStarted;

    inline int ValidateParams(int argc, char *argv[]);
    inline int ExeptionHandler(int ex) const;
};

inline int
Common::ValidateParams(int argc, char *argv[]) 
{
    
    if (argc < 3)
    {
        return ExeptionHandler(EX_LOW_ARGUMENTS);
    }
    else
    {
        // Input is ok           
        inFileName = argv[1];
        outFileName = argv[2];

        if (argc >= 4)
        {
            std::stringstream convert(argv[3]);

            if (!(convert >> bufferSize)) // Converting to the number
                bufferSize = DEFAULT_BUFFER_SIZE;

            if ((bufferSize==0)) // Buffer is Zero
                return ExeptionHandler(EX_ZERO_BUFF);  

            bufferSize = bufferSize* BYTES_IN_KILOBYTE; //Assuming the block size in kilos
        }

        if (!boost::filesystem::exists(inFileName)) // Checking if the input file exists
            return ExeptionHandler(EX_INPUT_FILE_MISSING); 
    }
    
    return 0;
}

inline int 
Common::ExeptionHandler(int ex) const
{
    try
    {
        if (ex > NO_ERROR)
        {
            std::cout << "An exception occurred.\n Exception Nr. " << ex << " : " << ExeptionStrings[ex] << std::endl;
            throw ex;
        }
    }
    catch (int ex)
    {        
        std::cout << "System exception: " << ex<< std::endl;
    }
    return ex;
}