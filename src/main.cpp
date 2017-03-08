#include <SDL2/SDL.h>
#include <iostream>





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


int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return 1;
  }


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