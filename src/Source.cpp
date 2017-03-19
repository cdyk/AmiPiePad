#include <cstring>
#include "Source.h"


void Source::setDeviceName(const std::string& n) 
{
  deviceName = n;
  deviceNameSet = true;
}

void Source::setGUID(const uint8_t* g)
{
  std::memcpy(guid, g, 16);
  guidSet = true;
}

void Source::setSDLIndex(uint32_t ix)
{
  index = ix;
  indexSet = true;
}
