#include "Bvh.h"
#include <cctype>
#include <charconv>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdlib.h>

using It = std::string_view::iterator;
struct Result {
  It end;
  It next;
};
using Delimiter = std::function<std::optional<Result>(It, It)>;

class Tokenizer {
  std::string_view m_data;
  std::string_view::iterator m_pos;

public:
  Tokenizer(std::string_view data) : m_data(data) { m_pos = m_data.begin(); }

  std::optional<std::string_view> token(const Delimiter &delimiter) {
    auto begin = m_pos;

    auto end = begin;

    for (; end != m_data.end(); ++end) {
      if (auto found = delimiter(end, m_data.end())) {
        auto [tail, next] = *found;
        m_pos = next;
        return std::string_view(begin, tail);
      }
    }

    return {};
  }

  bool expect(std::string_view expected, const Delimiter &delimiter) {
    if (auto line = token(delimiter)) {
      if (*line == expected) {
        return true;
      }
    }
    return false;
  }
};

static std::optional<Result> is_space(It it, It end) {
  if (!std::isspace(*it)) {
    return {};
  }
  auto tail = it;
  ++it;
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return Result{tail, it};
}

static std::optional<Result> get_name(It it, It end) {
  if (*it != '\n') {
    return {};
  }
  auto tail = it;
  ++it;
  // head space
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return Result{tail, it};
}

struct BvhImpl {
  Tokenizer token_;
  std::vector<BvhJoint> &joints_;

  BvhImpl(std::vector<BvhJoint> &joints, std::string_view src)
      : token_(src), joints_(joints) {}

  std::vector<int> stack_;

  bool Parse() {
    if (!token_.expect("HIERARCHY", is_space)) {
      return false;
    }

    return ParseJoint();
  }

private:
  bool ParseJoint() {
    while (true) {
      auto token = token_.token(is_space);
      if (!token) {
        return false;
      }

      if (*token == "ROOT" || *token == "JOINT") {
        // name
        // {
        // OFFSET x y z
        // CHANNELS 6
        // X {
        // }
        // }
        auto name = token_.token(get_name);
        if (!name) {
          return false;
        }

        // for (size_t i = 0; i < stack_.size(); ++i) {
        //   std::cout << "  ";
        // }
        // std::cout << *name << std::endl;

        if (!token_.expect("{", is_space)) {
          return false;
        }

        auto index = joints_.size();
        joints_.push_back(BvhJoint{
            .name = {name->begin(), name->end()},
            .parent = stack_.empty() ? -1 : stack_.back(),
        });
        stack_.push_back(index);
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        auto channels = ParseChannels();
        if (!channels) {
          return false;
        }

        ParseJoint();

      } else if (*token == "End") {
        // End Site
        // {
        // OFFSET x y z
        // }
        if (!token_.expect("Site", get_name)) {
          return false;
        }

        if (!token_.expect("{", is_space)) {
          return false;
        }
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        if (!token_.expect("}", is_space)) {
          return false;
        }
      } else if (*token == "}") {
        stack_.pop_back();
        break;
      } else if (*token == "MOTION") {
        break;
      } else {
        throw std::runtime_error("unknown");
      }
    }
  }

  std::optional<BvhOffset> ParseOffset() {
    if (!token_.expect("OFFSET", is_space)) {
      return {};
    }
    auto x = token_.token(is_space);
    auto y = token_.token(is_space);
    auto z = token_.token(is_space);

    return BvhOffset{};
  }

  std::optional<BvhChannels> ParseChannels() {
    if (!token_.expect("CHANNELS", is_space)) {
      return {};
    }

    auto n = token_.token(is_space);
    if (!n) {
      return {};
    }
    int channel_count;
    auto [ptr, ec] =
        std::from_chars(n->data(), n->data() + n->size(), channel_count);
    for (int i = 0; i < channel_count; ++i) {
      auto channel = token_.token(is_space);
    }

    return BvhChannels{};
  }
};

Bvh::Bvh() {}
Bvh::~Bvh() {}
bool Bvh::Parse(std::string_view src) {
  BvhImpl parser(joints, src);
  if (!parser.Parse()) {
    return false;
  }
  return true;
}
