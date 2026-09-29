#include "launcher.hpp"
namespace maine::core {
bool Launcher::validate(std::string& e) const {
  if(config_.selectedVersion.empty()){e="No Minecraft version selected.";return false;}
  if(!std::filesystem::exists(paths_.instances)){e="Maine data directory is not initialized.";return false;}
  return true;
}
bool Launcher::launch(std::string& e) const { if(!validate(e)) return false; e="Launch service is pending."; return false; }
}
