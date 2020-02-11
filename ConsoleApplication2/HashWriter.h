#pragma once
#include <vector>
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

class HashWriter
{    
public:
    class pair
    {
    public:
        unsigned int value;
        uintmax_t    position;

        pair(uintmax_t nPosition, unsigned int nValue)
        {           
            position = nPosition;
            value = nValue;
        }
    };

    HashWriter(char* outFileName) {
        outputFile.open(outFileName, std::ios::out | std::ios::binary);
    }

    fs::ofstream       outputFile;
    std::vector<pair>  outputBuff;
    uintmax_t          storedPosition = 0;
    uintmax_t          currentPos = 0;


    inline void onUpdateHashes(Common* nProps, fs::ifstream* inputFile);
    inline void putValue(uintmax_t nPosition, unsigned int nValue);
};

inline void
HashWriter::putValue(uintmax_t nPosition, unsigned int nValue)
{
    //std::cout << "Added " << nValue << " : " << nPosition << '\n';
    pair tempValue(nPosition, nValue);
    for (unsigned int i = 0; i < outputBuff.size(); i++)
    {
               //pair check = outputBuff.at(i);
        if (outputBuff.at(i).position > nPosition)
        {
            outputBuff.emplace(outputBuff.begin() + i , tempValue);
            return;
        }
    }

    outputBuff.push_back(tempValue);
}



void
HashWriter::onUpdateHashes(Common* nProps, fs::ifstream* inputFile)
{
    bool readyToWrite = (outputBuff.at(0).position == storedPosition);
    std::string outputData("");
    
    if (readyToWrite)
    {   
        bool haveTodone = false;
        int length = 0;
        do 
        {
        outputData += (char)outputBuff.at(0).value; //byte 1
        outputBuff.at(0).value >>= 8;
        outputData += (char)outputBuff.at(0).value; // byte 2
        outputBuff.at(0).value >>= 8;
        outputData += (char)outputBuff.at(0).value; // byte 3
        outputBuff.at(0).value >>= 8;
        outputData += (char)outputBuff.at(0).value; // byte 4        
        
        if (outputBuff.at(0).position == nProps->lastBlockNumber)
         {
            haveTodone = true;
         }

        outputBuff.erase(outputBuff.begin()); // Clear the first element
        storedPosition++; // expect +1 (linked chain)    

        length += 4; // 32bits = 4 Bytes
        } while (outputBuff.size() > 0 && (outputBuff.at(0).position == storedPosition));

        try
        {
            outputFile.write(&outputData[0], length);
        }
        catch (std::exception& e)
        {
            nProps->ExeptionHandler(EX_WRITE_ERROR);
            std::cout << "System exception: " << e.what() << std::endl;
        }

        std::cout << "Delivered " << (length >>2) << " block" << (length > 4 ? 's ':' ') << "! (next is: " << storedPosition  << "/ " << nProps->lastBlockNumber  << ") \n";

        if (haveTodone)
        {
            std::chrono::system_clock::time_point timeFinished = std::chrono::system_clock::now();

            auto  duration = std::chrono::duration_cast<std::chrono::milliseconds>(timeFinished - nProps->timeStarted);

            
            std::cout << "Job is done Sir in " << duration.count() << " miliseconds! \n";
            outputFile.close();
            inputFile->close();
        }
        
    }
    outputData = ""; // CleanUp the buffer
}
