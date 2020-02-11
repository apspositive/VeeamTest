#include <boost\filesystem.hpp>
#include <boost\filesystem\fstream.hpp>
#include <boost\thread\thread.hpp>
#include <boost\filesystem\operations.hpp>
#include <boost\iostreams\device\mapped_file.hpp>
#include <boost\crc.hpp>
#include <mutex>


using namespace std;
namespace fs = boost::filesystem;



class HugeFileReader
{
public:
    fs::ifstream  inputFile;
    uintmax_t     bufferSize;    

    std::mutex m;

    HugeFileReader(Common* nProps, HashWriter* outPutObj) {

       // Prepare variables
       inputFile.open(nProps->inFileName, std::ios_base::binary);
       fs::path p(nProps->inFileName, fs::native);
       nProps->fileSize = fs::file_size(p);        

       if (nProps->fileSize < 1) // Zero size input file
       {
           nProps->ExeptionHandler(EX_ZERO_SIZE);
       }

       bufferSize = nProps->bufferSize;
       
       nProps->lastBlockNumber = (nProps->fileSize-1) / bufferSize;
       uintmax_t blocksInfile = nProps->lastBlockNumber + 1;
       
       nProps->timeStarted = std::chrono::system_clock::now(); /// Remember the time
       // Then start
       startJob(nProps, outPutObj);
    };

    void startJob(Common* nProps, HashWriter* outPutObj);
   


private:
    std::vector<boost::thread> threads;
 };

void
readLastBlock(std::mutex* m, fs::ifstream* inputFile, Common* nProps, HashWriter* outPutObj) {
    m->lock();
    uintmax_t pos = outPutObj->currentPos;
    m->unlock();

    uintmax_t  appendix = nProps->fileSize - pos;

    char* buffer = new char[nProps->bufferSize];
    char *begin = buffer + appendix;
    char *end = begin + nProps->bufferSize;
    std::fill(begin, end, 0);

    inputFile->seekg(pos); // Move to the current pos
    inputFile->read(buffer, appendix);

    boost::crc_32_type result;
    result.process_bytes(buffer, nProps->bufferSize);
    delete[] buffer;

    m->lock();
    outPutObj->putValue(pos / nProps->bufferSize, result.checksum());
    outPutObj->onUpdateHashes(nProps, inputFile);   
    m->unlock();
}

void 
processor(std::mutex* m, fs::ifstream* inputFile, Common* nProps, HashWriter* outPutObj)
{
    while (outPutObj->currentPos < nProps->fileSize)
    {
        m->lock();
        uintmax_t pos = outPutObj->currentPos;
        outPutObj->currentPos += nProps->bufferSize;
        m->unlock();

        uintmax_t blockNumber = pos / nProps->bufferSize;

        if (pos < nProps->fileSize)
        {
            bool isLastBlock = blockNumber == nProps->lastBlockNumber;

            uintmax_t  readSize = isLastBlock ?  nProps->fileSize - pos : nProps->bufferSize;
            char* buffer = new char[nProps->bufferSize];

            if (isLastBlock) // Fill the rest by 0s
            {
                char *begin = buffer + readSize + 1;
                char *end = begin + nProps->bufferSize - readSize - 1;
                std::fill(begin, end, 0);
            }

            try
            {
                inputFile->seekg(pos); // Move to the current pos
                inputFile->read(buffer, readSize);
            }
            catch (exception& e)
            {
                nProps->ExeptionHandler(EX_READ_ERROR);
                cout << "System exception: " << e.what() << endl;
            }

            boost::crc_32_type result;
            result.process_bytes(buffer, nProps->bufferSize);
            delete[] buffer;

            m->lock();
            outPutObj->putValue(blockNumber, result.checksum());
            outPutObj->onUpdateHashes(nProps, inputFile);
            m->unlock();         
        }      
    }
  
}


void
HugeFileReader::startJob(Common* nProps, HashWriter* outPutObj) {
    //  inFile.open(inFileName, std::ios_base::binary);

    for (unsigned int i = 0; i < MAX_THREADS; i++) {
        threads.push_back(boost::thread(processor, &m, &inputFile, nProps, outPutObj));
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    system("pause");
}
