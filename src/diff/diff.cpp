//  diff.cpp
//  ZipDiff
/*
 The MIT License (MIT)
 Copyright (c) 2016-2018 HouSisong
 
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
#include "diff.h"
#include <iostream>
#include <stdio.h>
#include "../../HDiffPatch/libHDiffPatch/HDiff/diff.h"  //https://github.com/sisong/HDiffPatch
#include "../../HDiffPatch/libHDiffPatch/HPatch/patch.h"
#include "../../HDiffPatch/file_for_patch.h"
#include "../../HDiffPatch/_clock_for_demo.h"
#include "DiffData.h"

#include "../../zstd/lib/zstd.h" // https://github.com/facebook/zstd
#define _CompressPlugin_zstd
#define _IsNeedIncludeDefaultCompressHead 0
//#define _CompressPlugin_zlib
#include "../../HDiffPatch/compress_plugin_demo.h"
#include "../../HDiffPatch/decompress_plugin_demo.h"

bool HDiffZ(const std::vector<TByte>& oldData,const std::vector<TByte>& newData,std::vector<TByte>& out_diffData,
            hdiff_TCompress* compressPlugin,hpatch_TDecompress* decompressPlugin,int myBestMatchScore);

#define  check(value) { \
    if (!(value)){ printf(#value" ERROR!\n");  \
        result=false; if (!_isInClear){ goto clear; } } }



bool ZipDiff(const char* oldZipPath,const char* newZipPath,const char* outDiffFileName){
    const int           myBestMatchScore=1;
    UnZipper            oldZip;
    UnZipper            newZip;
    TFileStreamOutput   out_diffFile;
    std::vector<TByte>  newData;
    std::vector<TByte>  oldData;
    std::vector<TByte>  hdiffzData;
    std::vector<TByte>  out_diffData;
    std::vector<uint32_t> samePairList;
    std::vector<uint32_t> newRefList;
    std::vector<uint32_t> newReCompressList;
    std::vector<uint32_t> oldRefList;
    bool            result=true;
    bool            _isInClear=false;
    int             oldZipFileCount=0;
    
    zstd_compress_level=22; //0..22
    hdiff_TCompress* compressPlugin=&zstdCompressPlugin;
    hpatch_TDecompress* decompressPlugin=&zstdDecompressPlugin;
    
    UnZipper_init(&oldZip);
    UnZipper_init(&newZip);
    TFileStreamOutput_init(&out_diffFile);
    
    check(UnZipper_openRead(&oldZip,oldZipPath));
    check(UnZipper_openRead(&newZip,newZipPath));
    
    check(getSamePairList(&newZip,&oldZip,samePairList,newRefList,newReCompressList));
    
    //todo: get best oldZip refList
    oldZipFileCount=UnZipper_fileCount(&oldZip);
    for (int i=0; i<oldZipFileCount; ++i) {
        oldRefList.push_back(i);
    }
    std::cout<<"\nZipDiff same file count: "<<samePairList.size()/2<<"\n";
    std::cout<<"\      ref old file count: "<<oldRefList.size()<<"\n";
    
    check(readZipStreamData(&newZip,newRefList,newData));
    check(readZipStreamData(&oldZip,oldRefList,oldData));
    check(HDiffZ(oldData,newData,hdiffzData,compressPlugin,decompressPlugin,myBestMatchScore));
    oldData.clear();
    newData.clear();
    
    check(serializeZipDiffData(out_diffData,&newZip,&oldZip,newReCompressList,
                               samePairList,oldRefList,hdiffzData,compressPlugin));
    std::cout<<"\nZipDiff size: "<<out_diffData.size()<<"\n";

    check(TFileStreamOutput_open(&out_diffFile,outDiffFileName,out_diffData.size()));
    check(out_diffData.size()==out_diffFile.base.write(out_diffFile.base.streamHandle,
                                                       0,out_diffData.data(),out_diffData.data()+out_diffData.size()));
    check(TFileStreamOutput_close(&out_diffFile));
    std::cout<<"  out ZipDiff file ok!\n";
clear:
    _isInClear=true;
    check(TFileStreamOutput_close(&out_diffFile));
    check(UnZipper_close(&newZip));
    check(UnZipper_close(&oldZip));
    return result;
}


bool HDiffZ(const std::vector<TByte>& oldData,const std::vector<TByte>& newData,std::vector<TByte>& out_diffData,
            hdiff_TCompress* compressPlugin,hpatch_TDecompress* decompressPlugin,int myBestMatchScore){
    double time0=clock_s();
    const size_t oldDataSize=oldData.size();
    const size_t newDataSize=newData.size();
    std::cout<<"run HDiffZ:\n";
    std::cout<<"oldDataSize : "<<oldDataSize<<"\nnewDataSize : "<<newDataSize<<"\n";
    
    std::vector<TByte>& diffData=out_diffData;
    const TByte* newData0=newData.data();
    const TByte* oldData0=oldData.data();
    create_compressed_diff(newData0,newData0+newDataSize,oldData0,oldData0+oldDataSize,
                           diffData,compressPlugin);
    std::cout<<"diffDataSize: "<<diffData.size()<<"\n";
    double time1=clock_s();
    if (!check_compressed_diff(newData0,newData0+newDataSize,oldData0,oldData0+oldDataSize,
                               diffData.data(),diffData.data()+diffData.size(),decompressPlugin)){
        std::cout<<"\n  patch check HDiffZ data error!!!\n";
        return false;
    }else{
        double time2=clock_s();
        std::cout<<"  patch check HDiffZ data ok!\n";
        std::cout<<"  diff  time:"<<(time1-time0)<<" s\n";
        std::cout<<"  patch time:"<<(time2-time1)<<" s\n";
        return true;
    }
}
