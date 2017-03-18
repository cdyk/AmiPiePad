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
    if (context->sources.find(name) != context->sources.end()) {
      std::cerr << name << " is already a defined source.\n";
      return false;
    }
    if (!root.IsObject()) { std::cerr << name << " is not a JSON object.\n"; return false; }


    std::unique_ptr<Source> src(new Source(name));

    for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {

      if (std::strcmp("device-name", it->name.GetString()) == 0) {
        if (!it->value.IsString()) { std::cerr << "Source '" << name << "' device-name is not a string\n"; return false; }
        src->setDeviceName(it->value.GetString());
      }
    }

    context->sources.insert(std::make_pair(name, std::move(src)));

    return true;
  }

  bool parseSink(Context* context, const std::string& name, const rapidjson::Value& root)
  {
    if (!root.IsObject()) { std::cerr << name << " is not a JSON object.\n"; return false; }

    if (context->sinks.find(name) != context->sinks.end()) { std::cerr << name << " is already a defined sink.\n"; }

    auto type = root.FindMember("type");
    if (type != root.MemberEnd()) { std::cerr << name << " is missing compulsory type member.\n"; return false; }
    if (!type->value.IsString()) { std::cerr << name << ", 'type' is not a string.\n"; }

    std::unique_ptr<Sink> sink(Sink::factory(type->value.GetString(), name));
    if (!sink) { std::cerr << "Failed to create sink type " << type->value.GetString() << "\n"; return false; }

    auto gpios = root.FindMember("gpio");
    if(gpios == root.MemberEnd()) { std::cerr << name << " is missing compulsory gpio member.\n"; return false; }
    if (!gpios->value.IsObject()) { std::cerr << name << ", gpio member is not of object type.\n"; return false; }


    context->sinks.insert(std::make_pair(name, std::move(sink)));

    return true;
  }

  bool parseMapping(Context* context, const std::string& name, const rapidjson::Value& root)
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
  if (sources != configDoc.MemberEnd()) {
    if (!sources->value.IsObject()) { std::cerr << "'sources' is not an object\n"; return false; }
    for (auto it = sources->value.MemberBegin(); it != sources->value.MemberEnd(); ++it) {
      if (!parseSource(context, it->name.GetString(), it->value)) return false;
    }
  }

  auto sinks = configDoc.FindMember("sinks");
  if (sinks != configDoc.MemberEnd()) {
    if (!sinks->value.IsObject()) { std::cerr << "'sinks' is not an object\n"; return false; }
    for (auto it = sinks->value.MemberBegin(); it != sinks->value.MemberEnd(); ++it) {
      if (!parseSink(context, it->name.GetString(), it->value)) return false;
    }
  }

  auto mappings = configDoc.FindMember("mappings");
  if (mappings != configDoc.MemberEnd()) {
    if (!mappings->value.IsObject()) { std::cerr << "'mappings' is not an object\n"; return false; }
    for (auto it = mappings->value.MemberBegin(); it != mappings->value.MemberEnd(); ++it) {
      if (!parseMapping(context, it->name.GetString(), it->value)) return false;
    }
  }

  return true;
}
