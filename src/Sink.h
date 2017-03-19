#pragma once
#include "Context.h"
#include <string>
#include <vector>
#include <map>

class Sink
{
public:

  static Sink* factory(const std::string& type, const std::string& name);

  virtual ~Sink();

  Index getGPIOIndex(const std::string& key) const;

  bool setGPIOPin(Index index, Index pin);

  const std::string& getName() const { return name; }

  const std::vector<Index>& getGPIOPins() const { return GPIOPins; }

  Index getAxisIndex(const std::string& name) const;

  Index getButtonIndex(const std::string& name) const;

protected:
  std::string name;
  std::vector<Index> GPIOPins;
  std::vector<std::string> GPIOKeys;
  std::map<std::string, Index> axisKeys;
  std::map<std::string, Index> buttonKeys;

};

class MetaSink : public Sink
{
  friend Sink* Sink::factory(const std::string&, const std::string&);;
protected:
  MetaSink();
};

class MouseSink : public Sink
{
  friend Sink* Sink::factory(const std::string&, const std::string&);;
protected:
  MouseSink();
};

class JoystickSink : public Sink
{
  friend Sink* Sink::factory(const std::string&, const std::string&);;
protected:
  JoystickSink();
};

