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

Index Sink::getGPIOIndex(const std::string& key) const
{
  for (size_t i = 0; i < GPIOKeys.size(); i++) {
    if (GPIOKeys[i] == key) {
      return static_cast<Index>(i);
    }
  }
  return invalidIndex;
}

bool Sink::setGPIOPin(Index index, Index pin)
{
  if (index < GPIOPins.size()) {
    GPIOPins[index] = pin;
    return true;
  }
  return false;
}

Index Sink::getAxisIndex(const std::string& name) const
{
  auto it = axisKeys.find(name);
  if (it != axisKeys.end()) {
    return it->second;
  }
  return invalidIndex;
}

Index Sink::getButtonIndex(const std::string& name) const
{
  auto it = buttonKeys.find(name);
  if (it != buttonKeys.end()) {
    return it->second;
  }
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