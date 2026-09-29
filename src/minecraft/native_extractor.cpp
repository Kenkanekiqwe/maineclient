#include "native_extractor.hpp"
#include <windows.h>
#include <fstream>
#include <vector>
#include <algorithm>
namespace maine::minecraft {
static unsigned short le16(const unsigned char* p){return (unsigned short)p[0]|((unsigned short)p[1]<<8);}
static unsigned int le32(const unsigned char* p){return (unsigned int)le16(p)|((unsigned int)le16(p+2)<<16);}
bool NativeExtractor::extractJar(const std::filesystem::path& archive,const std::filesystem::path& dest,std::string& error){
 std::ifstream f(archive,std::ios::binary); if(!f){error="Cannot open native archive: "+archive.string();return false;}
 f.seekg(0,std::ios::end); auto sz=f.tellg(); if(sz<=0){error="Empty native archive";return false;} f.seekg(0);
 std::vector<unsigned char> data((size_t)sz); f.read((char*)data.data(),sz);
 if(data.size()<22){error="Invalid ZIP archive";return false;}
 size_t eocd=data.size()-22;
 while(eocd>0 && !(data[eocd]==0x50&&data[eocd+1]==0x4b&&data[eocd+2]==0x05&&data[eocd+3]==0x06)) --eocd;
 if(!(data[eocd]==0x50&&data[eocd+1]==0x4b&&data[eocd+2]==0x05&&data[eocd+3]==0x06)){error="ZIP end record not found";return false;}
 unsigned short count=le16(data.data()+eocd+10); unsigned int cdSize=le32(data.data()+eocd+12), cdOff=le32(data.data()+eocd+16);
 if((size_t)cdOff+cdSize>data.size()){error="Invalid ZIP directory";return false;}
 std::filesystem::create_directories(dest);
 size_t p=cdOff;
 for(unsigned short i=0;i<count;i++){
   if(p+46>data.size()||le32(data.data()+p)!=0x02014b50){error="Invalid ZIP entry";return false;}
   unsigned short method=le16(data.data()+p+10), n=le16(data.data()+p+28), extra=le16(data.data()+p+30), comment=le16(data.data()+p+32);
   unsigned int comp=le32(data.data()+p+20), local=le32(data.data()+p+42);
   std::string name((char*)data.data()+p+46,n); p+=46+n+extra+comment;
   if(name.empty()||name.back()=='/'||name.find("META-INF/")==0) continue;
   if((size_t)local+30>data.size()||le32(data.data()+local)!=0x04034b50){error="Invalid local ZIP entry";return false;}
   unsigned short ln=le16(data.data()+local+26), le=le16(data.data()+local+28);
   size_t src=(size_t)local+30+ln+le;
   if(src+comp>data.size()){error="ZIP entry exceeds archive";return false;}
   if(method!=0){error="Native archive uses unsupported compression; install minizip support";return false;}
   auto out=dest/name; std::filesystem::create_directories(out.parent_path());
   std::ofstream o(out,std::ios::binary); o.write((char*)data.data()+src,comp);
 }
 return true;
}
}
