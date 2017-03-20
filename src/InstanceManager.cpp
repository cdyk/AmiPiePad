#include <SDL2/SDL.h>
#include <iostream>
#include <set>
#include <cstring>
#include "Context.h"
#include "InstanceManager.h"
#include "Mapping.h"
#include "Source.h"


InstanceManager::InstanceManager()
{

}

InstanceManager::~InstanceManager()
{

}

void InstanceManager::reset(Context* context)
{
  clear(context);
  std::cerr << "InstanceManager::reset\n";
  if (context->mappings.empty()) return;

  context->mapping = context->mapping % static_cast<unsigned int>(context->mappings.size());
  const auto * mapping = context->mappings[context->mapping].get();

  std::set<const Source*> candidateSources;
  for (const auto & e : mapping->getEntries()) {
    candidateSources.insert(e.source);
  }

  // Plow through joysticks and try to find matches.
  int n = SDL_NumJoysticks();
  for (int i = 0; i < n; i++) {

    const std::string name(SDL_JoystickNameForIndex(i) ? SDL_JoystickNameForIndex(i) : "");

    auto guid = SDL_JoystickGetDeviceGUID(i);

    const Source * candidate = nullptr;
    for (const auto * c : candidateSources) {

      if (c->hasDeviceName() && (name.find(c->getDeviceName()) == std::string::npos)) continue;

      if(c->hasGUID() && (std::memcmp(guid.data, c->getGUID(), 16) != 0)) continue;

      if(c->hasSDLIndex() && (c->getSDLIndex() != static_cast<unsigned int>(i))) continue;

      candidate = c;
      break;
    }

    if (candidate) {
      candidateSources.erase(candidate);

      SourceInstance si;
      si.source = candidate;
      si.joystick = SDL_JoystickOpen(i);
      if (si.joystick == nullptr) {
        std::cerr << "Failed top open joystick: " << SDL_GetError() << "\n";
      }
      else {
        sourceInstances.push_back(si);
      }
    }
  }
  std::cerr << sourceInstances.size() << " active sources.\n";
}

void InstanceManager::clear(Context* context)
{
  std::cerr << "InstanceManager::clear\n";

  for (auto & si : sourceInstances) {
    SDL_JoystickClose(si.joystick);
  }
  sourceInstances.clear();

}