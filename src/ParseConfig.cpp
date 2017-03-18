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

namespace {

  bool parseSource(Context * context, const std::string & name, const rapidjson::Value & root)
  {
    std::unique_ptr<Source> src(new Source(name));

    for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {

      if (std::strcmp("device-name", it->name.GetString()) == 0) {
        src->setDeviceName(it->value.GetString());
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
      // TODO: Parse GPIO
    }

    context->sinks.insert(std::make_pair(name, std::move(sink)));
    return true;
  }

  bool parseMapping(Context* context, const rapidjson::Value& root)
  {


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
    if (!parseMapping(context, v)) return false;
  }

  return true;
}
