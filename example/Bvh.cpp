#include "Bvh.h"
#include <cctype>
#include <charconv>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdlib.h>

using It = std::string_view::iterator;
using Delimiter = std::function<std::optional<It>(It, It)>;

struct Offset {
  float x;
  float y;
  float z;
};

enum class ChannelTypes {
  None,
  Xposition,
  Yposition,
  Zposition,
  Zrotation,
  Xrotation,
  Yrotation,
};

struct Channels {
  ChannelTypes values[6] = {};
};

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
        m_pos = *found;
        break;
      }
    }

    return std::string_view(begin, end);
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

static std::optional<It> is_space(It it, It end) {
  if (!std::isspace(*it)) {
    return {};
  }
  ++it;
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return it;
}

static std::optional<It> eol(It it, It end) {
  if (*it != '\n') {
    return {};
  }
  ++it;
  return it;
}

static std::optional<It> get_name(It it, It end) {
  if (*it != '\n') {
    return {};
  }
  ++it;
  // head space
  for (; it != end; ++it) {
    if (!std::isspace(*it)) {
      break;
    }
  }
  return it;
}

struct BvhImpl {
  Tokenizer token_;

  BvhImpl(std::string_view src) : token_(src) {}

  int level_ = 0;

  bool Parse() {
    if (!token_.expect("HIERARCHY", is_space)) {
      return false;
    }

    return Joint();
  }

private:
  bool Joint() {
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

        for (int i = 0; i < level_; ++i) {
          std::cout << "  ";
        }
        std::cout << *name << std::endl;
        if (!token_.expect("{", is_space)) {
          return false;
        }
        ++level_;
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        auto channels = ParseChannels();
        if (!channels) {
          return false;
        }

        Joint();

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
        --level_;
        break;
      } else if (*token == "MOTION") {
        break;
      } else {
        throw std::runtime_error("unknown");
      }
    }
  }

  std::optional<Offset> ParseOffset() {
    if (!token_.expect("OFFSET", is_space)) {
      return {};
    }
    auto x = token_.token(is_space);
    auto y = token_.token(is_space);
    auto z = token_.token(is_space);

    return Offset{};
  }

  std::optional<Channels> ParseChannels() {
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

    return Channels{};
  }
};

Bvh::Bvh() {}
Bvh::~Bvh() {}
bool Bvh::Parse(std::string_view src) {
  BvhImpl parser(src);
  if (!parser.Parse()) {
    return false;
  }
  return true;
}
