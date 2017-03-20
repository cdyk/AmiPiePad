#pragma once
#include <string>
#include <cstdint>

class Source
{
public:

  Source(const std::string& name) : name(name) {}

  const std::string& getName() { return name; }

  void setDeviceName(const std::string& n);
  
  bool hasDeviceName() const { return deviceNameSet; }

  const std::string& getDeviceName() const { return deviceName; }

  void setGUID(const uint8_t* g);

  bool hasGUID() const { return guidSet;  }

  const uint8_t* getGUID() const { return guid; }

  void setSDLIndex(unsigned ix);

  bool hasSDLIndex() const { return indexSet; }

  unsigned getSDLIndex() const { return index; }

private:
  std::string name;
  std::string deviceName;
  bool deviceNameSet = false;
  uint8_t guid[16];
  bool guidSet = false;
  unsigned index;
  bool indexSet = false;


};