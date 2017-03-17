#pragma once
#include <map>
#include <vector>
#include <memory>


class Source;
class Sink;
class Mapping;

typedef uint32_t Index;
static const uint32_t invalidIndex = ~0u;

struct Context
{
  std::map<std::string, std::unique_ptr<Source>> sources;
  std::map<std::string, std::unique_ptr<Sink>> sinks;
  std::vector<std::unique_ptr<Mapping>> mappings;

  bool quit = false;
  bool reset = true;
};