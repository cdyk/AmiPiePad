#pragma once
#include <map>
#include <vector>
#include <memory>


class Source;
class Sink;
class Mapping;
class InstanceManager;
class GPIOManager;

typedef uint32_t Index;
static const uint32_t invalidIndex = ~0u;

struct Context
{
  Context();

  ~Context();

  std::unique_ptr<GPIOManager> gpioManager;
  std::unique_ptr<InstanceManager> instanceManager;
  std::map<std::string, std::unique_ptr<Source>> sources;
  std::map<std::string, std::unique_ptr<Sink>> sinks;
  std::vector<std::unique_ptr<Mapping>> mappings;

  unsigned int mapping = 0;
  bool quit = false;
  bool reset = true;
};