#include "native_extractor.hpp"
#include <windows.h>
#include <fstream>
#include <vector>
#include <algorithm>
namespace maine::minecraft {
static unsigned short le16(const unsigned char* p){return (unsigned short)p[0]|((unsigned short)p[1]<<8);}
static unsigned int le32(const unsigned char* p){return (unsigned int)le16(p)|((unsigned int)le16(p+2)<<16);}
#include <vector>\nbool NativeExtractor::extractJar(const std::filesystem::path& archive,const std::filesystem::path& dest,std::string& error){
 std::filesystem::create_directories(dest);
 std::string a=archive.string(), d=dest.string();
 auto quote=[](std::string s){std::string r="\""; for(char ch:s){if(ch=='\"') r+="\\\""; else r+=ch;} return r+"\"";};
 std::string cmd="tar -xf "+quote(a)+" -C "+quote(d);
 STARTUPINFOA si{}; si.cb=sizeof(si); PROCESS_INFORMATION pi{};
 std::vector<char> buf(cmd.begin(),cmd.end()); buf.push_back('\0');
 if(!CreateProcessA(nullptr,buf.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi)){
   error="Could not start Windows tar: "+std::to_string(GetLastError()); return false;
 }
 WaitForSingleObject(pi.hProcess,INFINITE);
 DWORD code=1; GetExitCodeProcess(pi.hProcess,&code); CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
 if(code!=0){error="Native archive extraction failed with code "+std::to_string(code);return false;}
 auto meta=dest/"META-INF"; std::error_code ec; std::filesystem::remove_all(meta,ec);
 return true;
}
}
