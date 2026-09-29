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
  if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA1_ALGORITHM,nullptr,0))return{};
  BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,(PUCHAR)&obj,sizeof(obj),&cb,0);
  std::vector<UCHAR> mem(obj),digest(20); BCryptCreateHash(alg,&hash,mem.data(),obj,nullptr,0,0);
  std::vector<char> buf(1<<20); while(in){in.read(buf.data(),buf.size()); auto n=in.gcount(); if(n) BCryptHashData(hash,(PUCHAR)buf.data(),(ULONG)n,0);}
  BCryptFinishHash(hash,digest.data(),20,0); BCryptDestroyHash(hash); BCryptCloseAlgorithmProvider(alg,0);
  static const char* hx="0123456789abcdef"; std::string s; for(auto b:digest){s+=hx[b>>4];s+=hx[b&15];} return s;
}
DownloadResult Downloader::file(const std::string& url,const std::filesystem::path& target,const std::string& expected){
  stage("Downloader::urlParts");
  std::wstring w(url.begin(),url.end()),host,path; bool https=false; if(!urlParts(w,host,path,https))return{false,"Invalid URL"};
  stage("Downloader::WinHttpOpen");
  HINTERNET ses=WinHttpOpen(L"MaineClient/0.1",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,nullptr,nullptr,0); if(!ses)return{false,"WinHttpOpen failed"};
  stage("Downloader::WinHttpConnect");
  HINTERNET con=WinHttpConnect(ses,host.c_str(),https?INTERNET_DEFAULT_HTTPS_PORT:INTERNET_DEFAULT_HTTP_PORT,0); if(!con){WinHttpCloseHandle(ses);return{false,"WinHttpConnect failed"};}
  stage("Downloader::WinHttpOpenRequest");
  DWORD flags=https?WINHTTP_FLAG_SECURE:0; HINTERNET req=WinHttpOpenRequest(con,L"GET",path.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,flags);
  if(!req){WinHttpCloseHandle(con);WinHttpCloseHandle(ses);return{false,"WinHttpOpenRequest failed"};}
  stage("Downloader::filesystem");
  bool ok=false; std::filesystem::create_directories(target.parent_path()); auto tmp=target; tmp+=".part";
  stage("Downloader::WinHttpSendRequest");
  if(WinHttpSendRequest(req,nullptr,0,nullptr,0,0,0)&&WinHttpReceiveResponse(req,nullptr)){
    DWORD status=0,sz=sizeof(status); WinHttpQueryHeaders(req,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,nullptr,&status,&sz,nullptr);
    stage("Downloader::headers");
    if(status>=200&&status<300){std::ofstream out(tmp,std::ios::binary|std::ios::trunc); char b[1<<16]; DWORD n=0; while(WinHttpReadData(req,b,sizeof(b),&n)&&n)out.write(b,n); out.close(); ok=bool(out);}
  }
  stage("Downloader::close");
  WinHttpCloseHandle(req);WinHttpCloseHandle(con);WinHttpCloseHandle(ses);
  if(!ok){std::error_code ec;std::filesystem::remove(tmp,ec);return{false,"Download failed: "+url};}
  stage("Downloader::sha1");
  if(!expected.empty() && sha1(tmp)!=expected){std::error_code ec;std::filesystem::remove(tmp,ec);return{false,"SHA-1 mismatch: "+url};}
  stage("Downloader::install");
  std::error_code ec;std::filesystem::remove(target,ec);std::filesystem::rename(tmp,target,ec); if(ec)return{false,"Cannot install downloaded file: "+ec.message()}; return{true,{}};
}
}
