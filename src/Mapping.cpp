#include <iostream>
#include "Mapping.h"
#include "Sink.h"
#include "Source.h"

namespace {

  bool getSourceAndSinkPointers(Context* context, Mapping::Entry& entry, const std::string& source, const std::string& sink)
  {
    auto sourceIt = context->sources.find(source);
    if (sourceIt == context->sources.end()) {
      std::cerr << "getSourceAndSinkPointers: Failed to find source \"" << source << "\"\n";
      return false;
    }
    entry.source = sourceIt->second.get();

    auto sinkIt = context->sinks.find(sink);
    if (sinkIt == context->sinks.end()) {
      std::cerr << "getSourceAndSinkPointers: Failed to find sink \"" << sink << "\"\n";
      return false;
    }
    entry.sink = sinkIt->second.get();
    return true;
  }


}


bool Mapping::addAxisEntry(Context* context, const std::string& sink, const std::string& sinkAxis, const std::string& source, Index sourceAxis)
{
  Entry entry;
  entry.type = Entry::Type::Axis;
  if (!getSourceAndSinkPointers(context, entry, source, sink)) return false;

  entry.sinkIndex = entry.sink->getAxisIndex(sink);
  if (entry.sinkIndex == invalidIndex) return false;

  entry.sourceIndices = { sourceAxis };

  return true;
}

bool Mapping::addButtonEntry(Context* context, const std::string& sink, const std::string& sinkButton, const std::string& source, const std::vector<Index>& sourceButtons)
{
  Entry entry;
  entry.type = Entry::Type::Axis;
  if (!getSourceAndSinkPointers(context, entry, source, sink)) return false;

  entry.sinkIndex = entry.sink->getButtonIndex(sink);
  if (entry.sinkIndex == invalidIndex) return false;

  entry.sourceIndices = sourceButtons;

  return true;
}
