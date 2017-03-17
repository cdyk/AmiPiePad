#include <iostream>
#include "Sink.h"

Sink* Sink::factory(const std::string& type, const std::string& name)
{
  Sink* rv = nullptr;
  if (type == "meta") {
    rv = new MetaSink();
  }
  else if (type == "mouse") {
    rv = new MouseSink();
  }
  else if (type == "joystick") {
    rv = new JoystickSink();
  }
  else {
    return nullptr;
  }
  rv->GPIOPins.resize(rv->GPIOKeys.size(), ~0u);
  rv->name = name;
  return rv;
}

Sink::~Sink()
{

}

bool Sink::addGPIOMapping(const std::string& key, uint32_t pin)
{
  for (size_t i = 0; i < GPIOKeys.size(); i++) {
    if (GPIOKeys[i] == "key") {
      if (GPIOPins[i] != ~0u) {
        std::cerr << name << '.' << key << " is already set.\n";
        return false;
      }
      GPIOPins[i] = pin;
      return true;
    }
  }
  std::cerr << name << '.' << key << " does not exist.\n";
  return false;
}

Index Sink::getAxisIndex(const std::string& name) const
{
  auto it = axisKeys.find(name);
  if (it != axisKeys.end()) {
    return it->second;
  }
  std::cerr << "Invalid axis name \"" << name << "\"\n";
  return invalidIndex;
}

Index Sink::getButtonIndex(const std::string& name) const
{
  auto it = buttonKeys.find(name);
  if (it != buttonKeys.end()) {
    return it->second;
  }
  std::cerr << "Invalid button name \"" << name << "\"\n";
  return invalidIndex;
}



MetaSink::MetaSink()
{
  buttonKeys = {
    {"next-mapping-button", 0}
  };
}

MouseSink::MouseSink()
{
  GPIOKeys = {
    "1-v-pulse",
    "2-h-pulse",
    "3-vq-pulse",
    "4-hq-pulse",
    "6-l-button",
    "9-r-button"
  };
  axisKeys = {
    { "h-axis", 0 },
    { "v-axis", 1 }
  };
  buttonKeys = {
    {"l-button", 0},
    {"r-button", 1}
  };
}

JoystickSink::JoystickSink()
{
  GPIOKeys = {
    "1-forward",
    "2-back",
    "3-left",
    "4-right",
    "6-fire-1",
  };
  axisKeys = {
    {"h-axis", 0},
    {"v-axis", 1}
  };
  buttonKeys = {
    { "fire-button", 0},
    { "fire-1-button", 0}
  };
}