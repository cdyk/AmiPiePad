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

  bool addAxisEntry(Context* context, const std::string& sink, const std::string& sinkAxis, const std::string& source, Index sourceAxis);

  bool addButtonEntry(Context* context, const std::string& sink, const std::string& sinkButton, const std::string& source, const std::vector<Index>& sourceButtons);

private:
  std::vector<Entry> entries;

};