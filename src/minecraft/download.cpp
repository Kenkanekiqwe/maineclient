#include "download.hpp"
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <fstream>
#include <vector>
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"bcrypt.lib")
namespace maine::minecraft {
#ifdef _WIN32
extern "C" void MaineSetCrashStage(const char* stage);
#endif
static void stage(const char* s){
#ifdef _WIN32
  MaineSetCrashStage(s);
#else
  (void)s;
#endif
}
static bool urlParts(const std::wstring& u,std::wstring& host,std::wstring& path,bool& https){
  URL_COMPONENTS c{}; c.dwStructSize=sizeof(c); wchar_t h[512]{},p[32768]{};
  c.lpszHostName=h;c.dwHostNameLength=512;c.lpszUrlPath=p;c.dwUrlPathLength=32768;
  if(!WinHttpCrackUrl(u.c_str(),0,0,&c)) return false; host.assign(h,c.dwHostNameLength); path.assign(p,c.dwUrlPathLength); https=c.nScheme==INTERNET_SCHEME_HTTPS; return true;
}
std::string Downloader::sha1(const std::filesystem::path& f){
  std::ifstream in(f,std::ios::binary); if(!in)return{};
  BCRYPT_ALG_HANDLE alg=nullptr; BCRYPT_HASH_HANDLE hash=nullptr; DWORD cb=0,obj=0;
  if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA1_ALGORITHM,nullptr,0)!=0)return{};
  if(BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,(PUCHAR)&obj,sizeof(obj),&cb,0)!=0 || obj==0){
    BCryptCloseAlgorithmProvider(alg,0); return{};
  }
  std::vector<UCHAR> mem(obj),digest(20);
  if(BCryptCreateHash(alg,&hash,mem.data(),obj,nullptr,0,0)!=0 || !hash){
    BCryptCloseAlgorithmProvider(alg,0); return{};
  }
  bool good=true;
  std::vector<char> buf(1<<20);
  while(in){
    in.read(buf.data(),buf.size());
    const auto n=in.gcount();
    if(n && BCryptHashData(hash,(PUCHAR)buf.data(),(ULONG)n,0)!=0){good=false;break;}
  }
  if(!in.eof()) good=false;
  if(good && BCryptFinishHash(hash,digest.data(),20,0)!=0) good=false;
  BCryptDestroyHash(hash); BCryptCloseAlgorithmProvider(alg,0);
  if(!good)return{};
  static const char* hx="0123456789abcdef"; std::string s; s.reserve(40);
  for(auto b:digest){s+=hx[b>>4];s+=hx[b&15];} return s;
}
DownloadResult Downloader::file(const std::string& url,const std::filesystem::path& target,const std::string& expected){
  stage("Downloader::urlParts");
  std::wstring w(url.begin(),url.end()),host,path; bool https=false;
  if(!urlParts(w,host,path,https)) return{false,"Invalid URL"};

  stage("Downloader::WinHttpOpen");
  HINTERNET ses=WinHttpOpen(L"MaineClient/0.2",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,nullptr,nullptr,0);
  if(!ses) return{false,"WinHttpOpen failed: "+std::to_string(GetLastError())};

  stage("Downloader::WinHttpConnect");
  HINTERNET con=WinHttpConnect(ses,host.c_str(),
      https?INTERNET_DEFAULT_HTTPS_PORT:INTERNET_DEFAULT_HTTP_PORT,0);
  if(!con){
    const DWORD err=GetLastError();
    WinHttpCloseHandle(ses);
    return{false,"WinHttpConnect failed: "+std::to_string(err)};
  }

  stage("Downloader::WinHttpOpenRequest");
  DWORD flags=https?WINHTTP_FLAG_SECURE:0;
  HINTERNET req=WinHttpOpenRequest(con,L"GET",path.c_str(),nullptr,
      WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,flags);
  if(!req){
    const DWORD err=GetLastError();
    WinHttpCloseHandle(con); WinHttpCloseHandle(ses);
    return{false,"WinHttpOpenRequest failed: "+std::to_string(err)};
  }

  stage("Downloader::filesystem");
  std::error_code ec;
  std::filesystem::create_directories(target.parent_path(),ec);
  if(ec){
    WinHttpCloseHandle(req); WinHttpCloseHandle(con); WinHttpCloseHandle(ses);
    return{false,"Cannot create download directory: "+ec.message()};
  }

  const std::wstring targetW=target.wstring();
  const std::wstring tmpW=(std::filesystem::path(target.string()+".part")).wstring();

  stage("Downloader::WinHttpSendRequest");
  bool ok=false;
  if(WinHttpSendRequest(req,nullptr,0,nullptr,0,0,0) &&
     WinHttpReceiveResponse(req,nullptr)){
    DWORD status=0,sz=sizeof(status);
    if(WinHttpQueryHeaders(req,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,
                           nullptr,&status,&sz,nullptr) &&
       status>=200 && status<300){
      stage("Downloader::headers");
      HANDLE out=CreateFileW(tmpW.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,nullptr);
      if(out!=INVALID_HANDLE_VALUE){
        char buffer[1<<16];
        DWORD n=0;
        ok=true;
        for(;;){
          n=0;
          if(!WinHttpReadData(req,buffer,sizeof(buffer),&n)){ok=false;break;}
          if(n==0)break;
          DWORD written=0;
          if(!WriteFile(out,buffer,n,&written,nullptr) || written!=n){
            ok=false; break;
          }
        }
        FlushFileBuffers(out);
        CloseHandle(out);
      }else{
        return{false,"Cannot create temporary download file (Win32 error "+
                      std::to_string(GetLastError())+"): "+tmpW};
      }
    }else{
      return{false,"HTTP download failed (status "+std::to_string(status)+"): "+url};
    }
  }

  stage("Downloader::close");
  WinHttpCloseHandle(req); WinHttpCloseHandle(con); WinHttpCloseHandle(ses);

  if(!ok){
    DeleteFileW(tmpW.c_str());
    return{false,"Download failed: "+url};
  }

  stage("Downloader::sha1");
  if(!expected.empty()){
    const std::string actual=sha1(std::filesystem::path(tmpW));
    if(actual.empty()){
      DeleteFileW(tmpW.c_str());
      return{false,"Cannot calculate SHA-1: "+tmpW};
    }
    if(actual!=expected){
      DeleteFileW(tmpW.c_str());
      return{false,"SHA-1 mismatch: "+url};
    }
  }

  stage("Downloader::install_begin");
  if(GetFileAttributesW(tmpW.c_str())==INVALID_FILE_ATTRIBUTES){
    return{false,"Downloaded temporary file is missing: "+tmpW};
  }

  stage("Downloader::install_delete_old");
  DeleteFileW(targetW.c_str());

  stage("Downloader::install_copy");
  if(!CopyFileW(tmpW.c_str(),targetW.c_str(),FALSE)){
    const DWORD err=GetLastError();
    DeleteFileW(tmpW.c_str());
    return{false,"Cannot install downloaded file (Win32 error "+
                  std::to_string(err)+"): "+targetW};
  }

  stage("Downloader::install_verify");
  if(GetFileAttributesW(targetW.c_str())==INVALID_FILE_ATTRIBUTES){
    DeleteFileW(tmpW.c_str());
    return{false,"Downloaded file was copied but cannot be opened: "+targetW};
  }

  stage("Downloader::install_cleanup");
  DeleteFileW(tmpW.c_str());
  return{true,{}};
}
