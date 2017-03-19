#pragma once
#include "Context.h"

class Sink;
class Source;

class Mapping
{
public:
  struct Entry {
    enum struct Type {
      Axis,
      Button
    } type = Type::Axis;
    Sink* sink = nullptr;
    Index sinkIndex = invalidIndex;

    Source* source = nullptr;
    std::vector<Index> sourceIndices;
  };

  void addEntry(const Entry& entry);

private:
  std::vector<Entry> entries;

};