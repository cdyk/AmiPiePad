#pragma once
#include <SDL2/SDL.h>

class Context;
class Source;

struct SourceInstance {
  const Source* source;
  SDL_Joystick* joystick;

};

class InstanceManager
{
public:

  InstanceManager();

  ~InstanceManager();

  void reset(Context* context);

  void clear(Context* context);

private:
  std::vector<SourceInstance> sourceInstances;


};