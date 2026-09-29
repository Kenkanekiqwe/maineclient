#pragma once
#include "paths.hpp"
#include "manifest.hpp"
#include <string>
namespace maine::minecraft {
class VersionDownloader {
  maine::core::Paths paths_;
public:
 explicit VersionDownloader(maine::core::Paths p):paths_(std::move(p)){}
 bool install(const std::string& version,std::string& error);
};
}
