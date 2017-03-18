#include <SDL2/SDL.h>
#include <iostream>
#include <iomanip>
#include "ParseConfig.h"


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

bool printList()
{
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    auto * js = SDL_JoystickOpen(i);
    if (js == nullptr) continue;

    auto name = SDL_JoystickName(js);
    if (name == nullptr) name = "[unknown]";

    auto guid = SDL_JoystickGetGUID(js);

    std::cout << "Joystick " << i << ", name=\"" << name << "\", GUID="  << std::hex;
    for (int j = 0; j < 16; j++) {
      std::cout << std::setw(2) << std::setfill('0') << (int)guid.data[j];
    }
    std::cout
      << std::dec << ", axes=" << (SDL_JoystickNumAxes(js) + 2* SDL_JoystickNumHats(js))
      << ", buttons=" << SDL_JoystickNumButtons(js) << "\n";

    SDL_JoystickClose(js);
  }

  return true;
}

bool dumpJoystick(int index)
{
  if (SDL_JoystickEventState(SDL_ENABLE) < 0) return false;

  auto * js = SDL_JoystickOpen(index);
  if (js == nullptr) return false;

  auto name = SDL_JoystickName(js);
  if (name == nullptr) name = "[unknown]";

  auto guid = SDL_JoystickGetGUID(js);

  std::cout << "Joystick " << index << ", name=\"" << name << "\", GUID=" << std::hex;
  for (int j = 0; j < 16; j++) {
    std::cout << std::setw(2) << std::setfill('0') << (int)guid.data[j];
  }
  std::cout << std::dec << "\n\n";


  bool quit = false;
  while (!quit) {
    SDL_Event e;
    if (SDL_WaitEvent(&e)) {
      switch (e.type)
      {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_JOYAXISMOTION:
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        std::cout << "\033[1A";
        for (int j = 0; j < SDL_JoystickNumAxes(js); j++) {
          std::cout << 'A' << j << ": " << std::setw(6) << std::setfill(' ') << SDL_JoystickGetAxis(js, j) << " ";
        }
        for (int j = 0; j < SDL_JoystickNumHats(js); j++) {
          auto mask = SDL_JoystickGetHat(js, j);
          int values[2] = {
            ((mask & SDL_HAT_LEFT) ? -32768 : 0) + ((mask & SDL_HAT_RIGHT) ? 32767 : 0),
            ((mask & SDL_HAT_UP) ? -32768 : 0) + ((mask & SDL_HAT_DOWN) ? 32767 : 0)
          };
          for (int k = 0; k < 2; k++) {
            std::cout << 'A' << (SDL_JoystickNumAxes(js) + 2*j + k) << ": " << std::setw(6) << std::setfill(' ') << values[k] << " ";
          }
        }
        for (int j = 0; j < SDL_JoystickNumButtons(js); j++) {
          std::cout << 'B' << j << ": " << (SDL_JoystickGetButton(js, j) ? "X " : "  ");
        }
        std::cout << "\n";
        break;
      }
    }
    else {
      std::cerr << "SDL_WaitEvent failed: " << SDL_GetError() << std::endl;
      quit = true;
    }
  }


  SDL_JoystickClose(js);
  return true;
}


void printHelp(const char* argv0)
{
  std::cout << "Usage: " << argv0 << " [OPTON]\n\n";
  std::cout << "  -h, --help     This help.\n";
  std::cout << "  -l, --list     List connected devices along with axis names.\n";
  std::cout << "  -d, --dump <i> Dump events from a given joystick.\n";
}

int main(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++) {
    if ((strcmp("-l", argv[i]) == 0) || (strcmp("--list", argv[i]) == 0)) {
      if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
      }
      auto rv = printList();
      SDL_Quit();
      return rv ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0)) {
      printHelp(argv[0]);
      return EXIT_SUCCESS;
    }
    else if ((i+1 < argc) && ((strcmp("-d", argv[i]) == 0) || (strcmp("--dump", argv[i]) == 0))) {
      if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
      }
      auto rv = dumpJoystick(atoi(argv[i + 1]));
      SDL_Quit();
      return rv ? EXIT_SUCCESS : EXIT_FAILURE;
    }
  }

  Context context;

  if (!parseConfig(&context, "../../../data/confschema.json", "../../../data/defaultconf.json")) {
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