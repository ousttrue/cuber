#pragma once
#include <chrono>
#include <memory>
#include <ostream>
#include <span>
#include <vector>

struct BvhOffset {
  float x;
  float y;
  float z;
};

enum class BvhChannelTypes {
  None,
  Xposition,
  Yposition,
  Zposition,
  Xrotation,
  Yrotation,
  Zrotation,
};

inline std::string_view to_str(BvhChannelTypes channelType) {
  switch (channelType) {
  case BvhChannelTypes::None:
    return "None";
  case BvhChannelTypes::Xposition:
    return "Xp";
  case BvhChannelTypes::Yposition:
    return "Yp";
  case BvhChannelTypes::Zposition:
    return "Zp";
  case BvhChannelTypes::Xrotation:
    return "Xr";
  case BvhChannelTypes::Yrotation:
    return "Yr";
  case BvhChannelTypes::Zrotation:
    return "Zr";
  default:
    throw std::runtime_error("unknown");
  }
}

struct BvhChannels {
  BvhChannelTypes values[6] = {};

  size_t size() const {
    size_t i = 0;
    for (; i < 6; ++i) {
      if (values[i] == BvhChannelTypes::None) {
        break;
      }
    }
    return i;
  }
};

inline std::ostream &operator<<(std::ostream &os, const BvhChannels channel) {
  for (int i = 0; i < 6; ++i) {
    if (channel.values[i] == BvhChannelTypes::None) {
      break;
    }
    if (i) {
      os << ", ";
    }
    os << to_str(channel.values[i]);
  }
  return os;
}

struct BvhJoint {
  std::string name;
  int parent;
  BvhOffset offset;
  BvhChannels channels;
};

inline std::ostream &operator<<(std::ostream &os, const BvhJoint &joint) {
  os << joint.name << ": [" << joint.offset.x << ", " << joint.offset.y << ", "
     << joint.offset.z << "]: " << joint.channels;
  return os;
}

using BvhTime = std::chrono::duration<float, std::ratio<1, 1>>;

struct Bvh {
public:
  std::vector<BvhJoint> joints;
  std::vector<BvhJoint> endsites;
  BvhTime frame_time = {};
  std::vector<float> frames;
  Bvh();
  ~Bvh();
  bool Parse(std::string_view src);
};

inline std::ostream &operator<<(std::ostream &os, const Bvh &bvh) {
  int channel_count = 0;
  for (auto &joint : bvh.joints) {
    channel_count += joint.channels.size();
  }

  os << "<BVH: " << bvh.joints.size()
     << "joints: " << (bvh.frames.size() / channel_count) << "frames/"
     << bvh.frame_time << std::endl;
  for (auto joint : bvh.joints) {
    os << "  " << joint << std::endl;
  }
  os << ">" << std::endl;

  return os;
}
