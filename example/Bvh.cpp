#include "Bvh.h"
#include <cctype>
#include <charconv>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdlib.h>

template <typename T> std::optional<T> to_num(std::string_view view) {
  T value;
  auto [ptr, ec] =
      std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec == std::errc{}) {
    return value;
  } else {
    return {};
  }
}

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

  template <typename T> std::optional<T> number(const Delimiter &delimiter) {
    auto n = token(delimiter);
    if (!n) {
      return {};
    }
    if (auto value = to_num<T>(*n)) {
      return *value;
    } else {
      return {};
    }
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
  std::vector<BvhJoint> &endsites_;

  BvhImpl(std::vector<BvhJoint> &joints, std::vector<BvhJoint> &endsites,
          std::string_view src)
      : token_(src), joints_(joints), endsites_(endsites) {}

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
        auto offset = ParseOffset();
        if (!offset) {
          return false;
        }
        auto channels = ParseChannels();
        if (!channels) {
          return false;
        }

        joints_.push_back(BvhJoint{
            .name = {name->begin(), name->end()},
            .parent = stack_.empty() ? -1 : stack_.back(),
            .offset = *offset,
            .channels = *channels,
        });
        stack_.push_back(index);

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
        endsites_.push_back(BvhJoint{
            .name = "End Site",
            .parent = stack_.empty() ? -1 : stack_.back(),
            .offset = *offset,
        });

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
    auto x = token_.number<float>(is_space);
    if (!x) {
      return {};
    }
    auto y = token_.number<float>(is_space);
    if (!y) {
      return {};
    }
    auto z = token_.number<float>(is_space);
    if (!z) {
      return {};
    }

    return BvhOffset{*x, *y, *z};
  }

  std::optional<BvhChannels> ParseChannels() {
    if (!token_.expect("CHANNELS", is_space)) {
      return {};
    }

    auto n = token_.number<int>(is_space);
    if (!n) {
      return {};
    }
    auto channel_count = *n;
    auto channels = BvhChannels{};
    for (int i = 0; i < channel_count; ++i) {
      if (auto channel = token_.token(is_space)) {
        if (*channel == "Xposition") {
          channels.values[i] = BvhChannelTypes::Xposition;
        } else if (*channel == "Yposition") {
          channels.values[i] = BvhChannelTypes::Yposition;
        } else if (*channel == "Zposition") {
          channels.values[i] = BvhChannelTypes::Zposition;
        } else if (*channel == "Xrotation") {
          channels.values[i] = BvhChannelTypes::Xrotation;
        } else if (*channel == "Yrotation") {
          channels.values[i] = BvhChannelTypes::Yrotation;
        } else if (*channel == "Zrotation") {
          channels.values[i] = BvhChannelTypes::Zrotation;
        } else {
          throw std::runtime_error("unknown");
        }
      }
    }
    return channels;
  }
};

Bvh::Bvh() {}
Bvh::~Bvh() {}
bool Bvh::Parse(std::string_view src) {
  BvhImpl parser(joints, endsites, src);
  if (!parser.Parse()) {
    return false;
  }
  return true;
}
