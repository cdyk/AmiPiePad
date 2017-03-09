#pragma once

class Source
{
public:

  Source(const std::string& name) : name(name) {}

  const std::string& getName() { return name; }

  void setDeviceName(const std::string& n) { deviceName = n; deviceNameSet = true; }
  

private:
  std::string name;
  std::string deviceName;
  bool deviceNameSet = false;



};