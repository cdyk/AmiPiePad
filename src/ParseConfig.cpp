#include <iostream>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ParseConfig.h"

#include <rapidjson.h>
#include <document.h>


namespace {

  bool snarf(std::vector<char>& content, const std::string path)
  {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) == -1) return false;

    content.resize(st.st_size);

    if (read(fd, content.data(), st.st_size) != st.st_size) return false;
    close(fd);

    return true;
  }

  bool parseSource(Context * context, const std::string & name, const rapidjson::Value & root)
  {
    if (context->sources.find(name) != context->sources.end()) {
      std::cerr << "source '" << name << "' already defined.\n";
      return false;
    }

    std::cerr << name << "\n";

    return true;
  }

}

bool parseConfig(Context * context, const std::string& path)
{

  std::vector<char> content;
  if (!snarf(content, path)) {
    std::cerr << "Failed to read " << path << std::endl;
    return false;
  }
  content.push_back('\0');

  rapidjson::Document d;
  d.ParseInsitu(content.data());
  if (d.HasParseError()) {
    std::cerr << "Failed to parse " << path << std::endl;
    return false;
  }
  if (!d.IsObject()) { std::cerr << "config root is not an object\n"; return false; }

  auto sources = d.FindMember("sources");
  if (sources != d.MemberEnd()) {
    if (!sources->value.IsObject()) { std::cerr << "'sources' is not an object\n"; return false; }
    for (auto it = sources->value.MemberBegin(); it != sources->value.MemberEnd(); ++it) {
      if (!parseSource(context, it->name.GetString(), it->value)) return false;
    }
  }

  return true;
}
