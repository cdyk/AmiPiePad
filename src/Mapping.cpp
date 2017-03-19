#include <iostream>
#include "Mapping.h"
#include "Sink.h"
#include "Source.h"

void Mapping::addEntry(const Entry& entry)
{
  entries.push_back(entry);
}

