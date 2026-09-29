#include "core/paths.hpp"
#include "core/config.hpp"
#include "ui/main_window.hpp"
int main() {
  auto paths=maine::core::Paths::create(); paths.ensure();
  auto config=maine::core::Config::load(paths.config/"settings.json");
  return maine::ui::runMainWindow(paths,config);
}
