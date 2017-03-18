#include "Context.h"

#include "Source.h"
#include "Sink.h"
#include "Mapping.h"
#include "GPIOManager.h"

Context::Context()
{
  gpioManager.reset(new GPIOManager());
}

Context::~Context()
{

}