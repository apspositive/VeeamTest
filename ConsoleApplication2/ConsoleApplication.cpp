// ConsoleApplication2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "common.h"
#include "HashWriter.h"
#include "HugeFileReader.h"
#include <iostream>

int main(int argc, char *argv[])
{
    int exeption;
    Common CommonProc;
  
    exeption = CommonProc.ValidateParams(argc, argv); // Fill arguments

    if (!exeption)
    {
      HashWriter outPutJob(CommonProc.outFileName);
      HugeFileReader ReadProcessor(&CommonProc, &outPutJob);
   // exeption = 
    }

    return exeption;
}

