#pragma once
#include <map>
#include <memory>

class Source;

struct Context
{
  std::map<std::string, std::unique_ptr<Source>> sources;

};