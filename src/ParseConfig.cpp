#include <iostream>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <memory>

#include "ParseConfig.h"
#include "Source.h"
#include "Sink.h"
#include "Mapping.h"

#include <filereadstream.h>
#include <rapidjson.h>
#include <document.h>
#include <schema.h>
#include <stringbuffer.h>
#include <cassert>

namespace {

  bool parseSource(Context * context, const std::string & name, const rapidjson::Value & root)
  {
    std::unique_ptr<Source> src(new Source(name));

    for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {

      if (std::strcmp("device-name", it->name.GetString()) == 0) {
        src->setDeviceName(it->value.GetString());
      }
      else if (std::strcmp("device-name", it->name.GetString()) == 0) {
        assert(it->value.GetStringLength() == 32);

        uint8_t guid[16];

        auto * str = it->value.GetString();
        for (unsigned j = 0; j < 16; j++) {
          uint8_t value = 0u;
          for (unsigned i = 0; i < 2; i++) {
            auto c = str[2 * j + i];
            if (('0' <= c) && (c <= '9')) {
              value += static_cast<uint8_t>(c - '0');
            }
            else if (('a' <= c) && (c <= 'f')) {
              value += static_cast<uint8_t>(c - 'a') + 10u;
            }
            else if (('A' <= c) && (c <= 'F')) {
              value += static_cast<uint8_t>(c - 'A') + 10u;
            }
            else {
              assert("invalid character" && 0);
            }
            value = value * 16;
          }
          guid[j] = value;
        }

        src->setGUID(guid);
      }
      else if (std::strcmp("index", it->name.GetString()) == 0) {
        src->setSDLIndex(std::atoi(it->value.GetString()));
      }
    }

    context->sources.insert(std::make_pair(name, std::move(src)));

    return true;
  }

  bool parseSink(Context* context, const std::string& name, const rapidjson::Value& root)
  {
    auto type = root.FindMember("type");

    std::unique_ptr<Sink> sink(Sink::factory(type->value.GetString(), name));
    if (!sink) { std::cerr << "Failed to create sink type " << type->value.GetString() << "\n"; return false; }

    auto gpios = root.FindMember("gpio");
    for (auto it = gpios->value.MemberBegin(); it != gpios->value.MemberEnd(); ++it) {
      auto index = sink->getGPIOIndex(it->name.GetString());
      if (index == invalidIndex) {
        std::cerr << "Failed to get index of " << it->name.GetString() << "\n";
        return false;
      }
      
      if (!sink->setGPIOPin(index, static_cast<Index>(it->value.GetInt()))) {
        std::cerr << "Failed to set index " << index << " to pin " << it->value.GetInt() << "\n";
        return false;
      }
    }

    context->sinks.insert(std::make_pair(name, std::move(sink)));
    return true;
  }

  bool parseMapping(Context* context, const rapidjson::Value& root)
  {

    std::unique_ptr<Mapping> mapping(new Mapping());

    for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {

      std::string sinkName(it->name.GetString());
      auto sinkIt = context->sinks.find(sinkName);
      if (sinkIt == context->sinks.end()) {
        std::cerr << "Sink '" << sinkName << "' not defined.\n";
        return false;
      }
      auto * sink = sinkIt->second.get();

      std::string sourceName(it->value.FindMember("source")->value.GetString());
      auto sourceIt = context->sources.find(sourceName);
      if (sourceIt == context->sources.end()) {
        std::cerr << "Source '" << sourceName << "' not defined.\n";
        return false;
      }
      auto * source = sourceIt->second.get();

      for (auto jt = it->value.MemberBegin(); jt != it->value.MemberEnd(); ++jt) {
        Mapping::Entry entry;
        entry.sink = sink;

        std::string sinkIndexName(jt->name.GetString());
        if(sinkIndexName == "source") continue;

        entry.sinkIndex = sink->getAxisIndex(sinkIndexName);
        if (entry.sinkIndex != invalidIndex) {
          entry.type = Mapping::Entry::Type::Axis;
        }
        else {
          entry.sinkIndex = sink->getButtonIndex(sinkIndexName);
          if (entry.sinkIndex != invalidIndex) {
            entry.type = Mapping::Entry::Type::Button;
          }
          else {
            std::cerr << "Invalid sink name '" << sinkIndexName << "' on sink " << it->name.GetString() << "\n";
          }
        }

        entry.source = source;
        if (jt->value.IsInt()) {
          entry.sourceIndices.push_back(static_cast<Index>(jt->value.GetInt()));
        }
        else {
          if (entry.type != Mapping::Entry::Type::Button) {
            std::cerr << "On " << sinkIndexName << " of sink " << sinkName << ", currently only buttons can have multiple triggers\n";
            return false;
          }

          for (auto & v : jt->value.GetArray()) {
            entry.sourceIndices.push_back(static_cast<Index>(v.GetInt()));
          }
        }

        mapping->addEntry(entry);
      }
    }

    context->mappings.push_back(std::move(mapping));
    return true;
  }


}

bool parseConfig(Context * context, const std::string& schemaPath, const std::string& configPath)
{
  char buffer[65536];
  FILE* fp;

  rapidjson::Document schemaDoc;
  fp = fopen(schemaPath.c_str(), "r");
  if (fp == nullptr) { std::cerr << "Failed to read " << schemaPath << "\n"; return false; }
  rapidjson::FileReadStream schemaStream(fp, buffer, sizeof(buffer));
  schemaDoc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(schemaStream);
  fclose(fp);

  if (schemaDoc.HasParseError()) {
    std::cerr << "Failed to parse " << schemaPath << ": ";
    auto error = schemaDoc.GetParseError();
    std::cerr << error << "\n";
    return false;
  }

  rapidjson::Document configDoc;
  fp = fopen(configPath.c_str(), "r");
  if (fp == nullptr) { std::cerr << "Failed to read " << configPath << "\n"; return false; }
  rapidjson::FileReadStream configStream(fp, buffer, sizeof(buffer));
  configDoc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(configStream);
  fclose(fp);

  if (configDoc.HasParseError()) {
    std::cerr << "Failed to parse " << configPath << "\n";
    auto error = configDoc.GetParseError();
    std::cerr << error << "\n";
    return false;
  }

  rapidjson::SchemaDocument schema(schemaDoc);
  rapidjson::SchemaValidator validator(schema);
  if (!configDoc.Accept(validator)) {
    rapidjson::StringBuffer sb;
    validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
    std::cerr << "Invalid schema: " << std::string(sb.GetString()) << "\n";
    std::cerr << "Invalid keyword: " << validator.GetInvalidSchemaKeyword() << "\n";
    sb.Clear();
    validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
    std::cerr << "Invalid document: " << std::string(sb.GetString()) << "\n";
    return false;
  }

  if (!configDoc.IsObject()) { std::cerr << "config root is not an object\n"; return false; }

  auto sources = configDoc.FindMember("sources");
  for (auto it = sources->value.MemberBegin(); it != sources->value.MemberEnd(); ++it) {
    if (!parseSource(context, it->name.GetString(), it->value)) return false;
  }

  auto sinks = configDoc.FindMember("sinks");
  for (auto it = sinks->value.MemberBegin(); it != sinks->value.MemberEnd(); ++it) {
    if (!parseSink(context, it->name.GetString(), it->value)) return false;
  }

  auto mappings = configDoc.FindMember("mappings");
  for (auto & v : mappings->value.GetArray()) {
    if (!parseMapping(context, v.GetObject())) return false;
  }

  return true;
}
