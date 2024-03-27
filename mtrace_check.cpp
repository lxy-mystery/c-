#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <format>
#include <functional>
#include <set>

namespace fs = std::filesystem;

static std::vector<std::tuple<uint64_t, uint64_t, std::string>>           g_process_maps;
static std::map<std::string, std::map<uint64_t, uint64_t>>                g_caller_stastics;

struct FieldParseProtocol {
  size_t begin = 0;
  size_t count = 0;
};

auto parse_header(const std::string& header_line) {
  std::vector<FieldParseProtocol> result;
  enum class State : uint8_t {
    FiledBegin = 0,
    FiledEnd,
  }state = State::FiledBegin;
  // [begin, end]
  FieldParseProtocol proto;
  
  for (int i = 0; i < header_line.size(); i++) {
    const char ch = header_line.at(i);
    switch (state)
    {
    case State::FiledBegin: {
      if (ch == ' ') {
        state = State::FiledEnd;
        //std::cout << "decode header field: " << header_line.substr(begin, count);
        std::cout << std::format("decode header field FiledBegin -> FiledEnd: [{}] begin {}, count {}", header_line.substr(proto.begin, proto.count), proto.begin, proto.count) << std::endl;
      }
      proto.count++;
      break;
    }
    case State::FiledEnd: {
      if (ch != ' ') {
        result.push_back(proto);
        std::cout << std::format("decode header field FiledEnd -> FiledBegin: [{}] begin {}, count {}", header_line.substr(proto.begin, proto.count), proto.begin, proto.count) << std::endl;
        proto.begin = proto.begin + proto.count;
        proto.count = 0;
        state = State::FiledBegin;
      }
      proto.count++;
      break;
    }
    default:
      break;
    }
  }
  proto.count = std::string::npos;
  result.push_back(proto);
  std::cout << std::format("decode header field last: [{}] begin {}, count {}", header_line.substr(proto.begin, proto.count), proto.begin, proto.count) << std::endl;
  return result;
}

auto parse_line(const auto& proto, const std::string& line) {
  std::vector<std::string> fileds;
  for (const auto& filed: proto) {
    fileds.emplace_back(line.substr(filed.begin, filed.count));
  }
  return fileds;
}

auto hex_to_decimal(const std::string& hex_string) {
  enum class State: uint8_t {
    Begin = 0,
    NumberBegin,
    End
  } state = State::Begin;

  uint64_t res = 0;
  for (const auto & ch : hex_string) {
    switch (state)
    {
    case State::Begin: {
      // strip blank and zero
      if (ch != ' ' && ch != '0') {
        state = State::NumberBegin;
        char base = 'a' - 10;
        if (ch >= '0' && ch <= '9') {
          base = '0';
        }
        res = (res << 4) | (ch - base);
      }
      break;
    }
    case State::NumberBegin: {
      if (ch == ' ') {
        state = State::End;
        break;
      }
      char base = 'a' - 10;
      if (ch >= '0' && ch <= '9') {
        base = '0';
      }
      res = (res << 4) | (ch - base);
      break;
    }
    default:
      break;
    }
  }
  return res;
}

int decode_pmap_file(const std::string& file_name) {
  // check pmap result is exist "./345.map"
  fs::path pmap_file_path = fs::absolute(file_name);
  if (!fs::exists(pmap_file_path)) {
    std::cout << "ERROR: file not exist. " << pmap_file_path << std::endl;
    return -1;
  }

  std::fstream pmap_file(pmap_file_path);
  std::string line;
  int line_number = 0;
  std::vector<FieldParseProtocol> proto;
  while (std::getline(pmap_file, line)) {
    line_number++;
    if (line_number == 1) {
      // skip first line
      continue;
    }
    if (line_number == 2) {
      proto = parse_header(line);
      for (const auto& filed_proto : proto) {
        std::cout << std::format("-- start :{}, count: {}", filed_proto.begin, filed_proto.count) << std::endl;
      }
      continue;
    }
    // maybe latest line
    if (line.starts_with("mapped")) {
      break;
    }
    const auto&& fields = parse_line(proto, line);
    if (fields.size() < 6) {
      std::cout << std::format("decode line fields size wrong: {}", fields.size()) << std::endl;
      continue;
    }
    auto base_address = hex_to_decimal(fields[0]);
    auto kbytes = hex_to_decimal(fields[1]);
    auto offset = hex_to_decimal(fields[3]);
    auto mapping = fields[5];
    g_process_maps.emplace_back(std::make_tuple(base_address, offset, mapping));
    std::cout << std::format("base_address:{}, offset:{}, size:{}, mapping:{}", base_address, offset, kbytes, mapping) << std::endl;
  }
  pmap_file.close();
  return 0;
}

void traverse_path(const fs::path& path, bool recursion, std::function<bool (const fs::path&)> visitor) {
  if (!fs::exists(path)) {
    std::cerr << "Path does not exist: " << path << std::endl;
    return;
  }

  if (fs::is_directory(path) && recursion) {
    for (const auto& entry : fs::directory_iterator(path)) {
      traverse_path(entry.path(), recursion, visitor);
    }
  }
  else if (fs::is_regular_file(path)) {
    if (visitor && !visitor(path)) {
      return;
    }
  }
  else {
    std::cout << "Special file or directory: " << path << std::endl;
  }
}

auto process_trace_file(const std::set<fs::path> file_list) {
  std::vector<FieldParseProtocol> proto;
  for (const auto& memory_file : file_list) {
    enum class State : uint8_t {
      FileStart = 0,
      HeadStart = 1,
      Content
    }state = State::FileStart;
    g_caller_stastics[memory_file.generic_string()] = {};
    std::map<uint64_t, uint64_t> &caller_map = g_caller_stastics[memory_file.generic_string()];
    std::cout << "process file " << memory_file << std::endl;
    std::fstream trace(memory_file);
    std::string line;
    while (std::getline(trace, line)) {
      switch (state)
      {
      case State::FileStart: {
        if (line.starts_with("------")) {
          state = State::HeadStart;
        }
        break;
      }
      case State::HeadStart: {
        //proto = parse_header(line);
        proto = { {0, 18}, {19, 9}, {32, 14} };
        state = State::Content;
        break;
      }
      case State::Content: {
        const auto&& fields = parse_line(proto, line);
        auto caller = hex_to_decimal(fields[2].substr(2));
        if (caller_map.find(caller) == caller_map.end()) {
          caller_map[caller] = 0;
        }
        caller_map[caller]++;
        // std::cout << std::format("address:{}, size:{}, caller:{}", fields[0], fields[1], fields[2]) << std::endl;
        break;
      }
      default:
        break;
      }
    }
  }
}

auto find_process_segment(uint64_t caller) {
  int i = 0;
  for (; i < g_process_maps.size(); i++) {
    auto [base, position, mapping] = g_process_maps[i];
    if (base > caller) {
      return g_process_maps[i - 1];
    }
  }
  return g_process_maps[g_process_maps.size() - 1];
}

int main()
{
  decode_pmap_file("./345.map");
  std::set<fs::path> file_list;

  traverse_path(fs::path("./mtrace/"), true, [&](const fs::path& dir) -> bool {
    std::cout << "visitor " << dir << std::endl;
    file_list.insert(dir);
    return true;
    });

  process_trace_file(file_list);
  for (const auto& item: g_caller_stastics) {
    std::cout << " ======================= " << item.first << " ======================" << std::endl;
    for (const auto &caller: item.second) {
      auto [base, position, mapping] = find_process_segment(caller.first);
      std::cout << std::format("==== caller:{}, count:{}, seg base:{}, seg pos:{}, seg:{}", caller.first, caller.second, base, position, mapping) << std::endl;
    }
  }
  return 0;
}
