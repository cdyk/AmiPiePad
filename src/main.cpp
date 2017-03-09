#include <SDL2/SDL.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <rapidjson.h>
#include <document.h>



void tryAddControllers()
{
  int n = SDL_NumJoysticks();
  for (int i = 0; i < n; i++) {
    auto name = SDL_JoystickNameForIndex(i);
    if (name == nullptr) name = "[unknown]";

    std::cerr << '[' << i << "]  \"" << name << '"' << std::endl;

  }
}

void purgeControllers(SDL_JoystickID joyId)
{
}

bool snarf(std::vector<char>& content, const std::string path)
{
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) return false;

  struct stat st;
  if (fstat(fd, &st) == -1) return false;

  content.resize(st.st_size);

  if (read(fd, content.data(), st.st_size) != st.st_size) return false;
  close(fd);

  return true;
}

bool parseConfig(const std::string& path)
{
  std::vector<char> content;
  if (!snarf(content, path)) {
    std::cerr << "Failed to read " << path << std::endl;
    return false;
  }
  content.push_back('\0');

  rapidjson::Document d;
  d.ParseInsitu(content.data());
  if (d.HasParseError()) {
    std::cerr << "Failed to parse " << path << std::endl;
    return false;
  }



  return true;
}


int main(int argc, char* argv[])
{

  if (!parseConfig("../../../data/defaultconf.json")) {
    std::cerr << "Failed to parse config file." << std::endl;
    return 1;
  }

  if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  tryAddControllers();

  bool quit = false;
  while (!quit) {
    SDL_Event e;
    if (!SDL_WaitEvent(&e)) {
      std::cerr << "SDL_WaitEvent failed: " << SDL_GetError() << std::endl;
      break;
    }

    switch (e.type)
    {
    case SDL_QUIT:
      quit = true;
      break;

    case SDL_CONTROLLERDEVICEADDED:
      tryAddControllers();
      break;

    case SDL_CONTROLLERDEVICEREMAPPED:
      purgeControllers(e.cdevice.which);
      tryAddControllers();
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
      purgeControllers(e.cdevice.which);
      break;

    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
      // Handled by SDL_CONTROLLERDEVICE...
      break;

    default:
      std::cerr << "Unhandled event, type=" << std::hex << e.type << std::endl;
    }
  }

  SDL_Quit();
  return 0;
}