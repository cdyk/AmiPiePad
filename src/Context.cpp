#include "Context.h"

#include "Source.h"
#include "Sink.h"
#include "Mapping.h"
#include "GPIOManager.h"

Context::Context()
{
  gpioManager.reset(new GPIOManager());

  std::unique_ptr<Sink> metaSink(Sink::factory("meta", "meta"));
  sinks.insert(std::make_pair("meta", std::move(metaSink)));
}

Context::~Context()
{

}