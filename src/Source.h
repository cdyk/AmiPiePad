#pragma once
#include <string>
#include <cstdint>

class Source
{
public:

  Source(const std::string& name) : name(name) {}

  const std::string& getName() { return name; }

  void setDeviceName(const std::string& n);
  
  void setGUID(const uint8_t* g);

  void setSDLIndex(uint32_t ix);

private:
  std::string name;
  std::string deviceName;
  bool deviceNameSet = false;
  uint8_t guid[16];
  bool guidSet = false;
  uint32_t index;
  bool indexSet = false;


};