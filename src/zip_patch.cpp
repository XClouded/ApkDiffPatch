//  zip_patch.cpp
//  ZipPatch
/*
 The MIT License (MIT)
 Copyright (c) 2018 HouSisong
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../HDiffPatch/_clock_for_demo.h"
#include "patch/Patcher.h"

int main(int argc, const char * argv[]) {
    const char* oldZipPath=0;
    const char* zipDiffPath=0;
    const char* outNewZipPath=0;
    size_t      maxUncompressMemory=0;
    const char* tempUncompressFileName=0;
    if(argc==4){
        oldZipPath   =argv[1];
        zipDiffPath  =argv[2];
        outNewZipPath=argv[3];
    }else if(argc==6){
        oldZipPath   =argv[1];
        zipDiffPath  =argv[2];
        outNewZipPath=argv[3];
        long _byteSize=atol(argv[4]);
        if ((_byteSize<0)||(_byteSize!=(long)(size_t)_byteSize)){
            printf("parameter maxUncompressMemory error!\n");
            return 1;
        }
        maxUncompressMemory=(size_t)_byteSize;
        tempUncompressFileName=argv[5];
        if (tempUncompressFileName==0){
            printf("parameter tempUncompressFileName error!\n");
            return 1;
        }
    }else{
        printf("parameter: oldZip zipDiffPath outNewZip [maxUncompressMemory tempUncompressFileName]\n");
        return 1;
    }
    printf("oldZip:\"%s\"\ndiff  :\"%s\"\noutZip:\"%s\"\n",oldZipPath,zipDiffPath,outNewZipPath);
    if (tempUncompressFileName!=0)
        printf("maxUncompressMemory:%ld\ntempUncompressFileName:\"%s\"\n",maxUncompressMemory,tempUncompressFileName);
    
    double time0=clock_s();
    int exitCode=ZipPatch(oldZipPath,zipDiffPath,outNewZipPath,maxUncompressMemory,tempUncompressFileName);
    double time1=clock_s();
    if (exitCode==PATCH_SUCCESS)
        printf("  patch ok!\n");
    printf("\nZipPatch time: %.3f s\n",(time1-time0));
    return exitCode;
}
