#include <SDL2/SDL.h>
#include <iostream>
#include <iomanip>
#include "ParseConfig.h"
#include "Source.h"



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

void printList()
{
  if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return;
  }

  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    auto * js = SDL_JoystickOpen(i);
    if (js == nullptr) continue;

    auto name = SDL_JoystickName(js);
    if (name == nullptr) name = "[unknown]";

    auto guid = SDL_JoystickGetGUID(js);

    std::cout << "Joystick " << i << ", name=\"" << name << "\", GUID="  << std::hex;
    for (int j = 0; j < 16; j++) {
      std::cout << std::setw(2) << std::setfill('0') << (int)guid.data[i];
    }
    std::cout
      << std::dec << ", axes=" << SDL_JoystickNumAxes(js)
      << ", hats=" << SDL_JoystickNumHats(js)
      << ", buttons=" << SDL_JoystickNumButtons(js) << "\n";

    SDL_JoystickClose(js);
  }
}

void printHelp(const char* argv0)
{
  std::cout << "Usage: " << argv0 << " [OPTON]\n\n";
  std::cout << "  -h, --help    This help.\n";
  std::cout << "  -l, --list    List connected devices along with axis names.\n";
}

int main(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++) {
    if ((strcmp("-l", argv[i]) == 0) || (strcmp("--list", argv[i]) == 0)) {
      printList();
      return EXIT_SUCCESS;
    }
    else if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0)) {
      printHelp(argv[0]);
      return EXIT_SUCCESS;
    }
  }


  Context context;

  if (!parseConfig(&context, "../../../data/defaultconf.json")) {
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
  return EXIT_SUCCESS;
}