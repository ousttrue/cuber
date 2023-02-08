#pragma once
#include <chrono>
#include <iostream>
#include <memory>
#include <ostream>
#include <span>
#include <vector>

struct BvhOffset {
  float x;
  float y;
  float z;

  BvhOffset &operator+=(const BvhOffset &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
};
inline std::ostream &operator<<(std::ostream &os, const BvhOffset &offset) {
  os << "[" << offset.x << ", " << offset.y << ", " << offset.z << "]";
  return os;
}

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
  uint16_t index;
  uint16_t parent;
  BvhOffset localOffset;
  BvhOffset worldOffset;
  BvhChannels channels;
};

inline std::ostream &operator<<(std::ostream &os, const BvhJoint &joint) {
  os << joint.name << ": " << joint.worldOffset << " " << joint.channels;
  return os;
}

using BvhTime = std::chrono::duration<float, std::ratio<1, 1>>;

struct BvhFrame {
  int index;
  BvhTime time;
  std::span<const float> values;
};

struct Bvh {
  std::vector<BvhJoint> joints;
  std::vector<BvhJoint> endsites;
  BvhTime frame_time = {};
  std::vector<float> frames;
  uint32_t frame_channel_count = 0;
  float max_height = 0;
  Bvh();
  ~Bvh();
  static std::shared_ptr<Bvh> ParseFile(std::string_view file);
  bool Parse(std::string_view src);
  uint32_t FrameCount() const { return frames.size() / frame_channel_count; }
  const BvhJoint *GetParent(int parent) const {
    for (auto &joint : joints) {
      if (joint.parent == parent) {
        return &joint;
      }
    }
    return nullptr;
  }
  int TimeToIndex(BvhTime time) const {
    auto div = time / frame_time;
    auto index = (int)div;
    if (index >= FrameCount()) {
      index = index % FrameCount();
    }
    return index;
  }
  BvhFrame GetFrame(int index) const {
    auto begin = frames.data() + index * frame_channel_count;
    return {
        .index = index,
        .time = frame_time * index,
        .values = {begin, begin + frame_channel_count},
    };
  }
  float GuessScaling() const {
    // guess bvh scale
    float scalingFactor = 1.0f;
    if (max_height < 2) {
      // maybe meter scale. do nothing
    } else if (max_height < 200) {
      // maybe cm scale
      scalingFactor = 0.01f;
    }
    return scalingFactor;
  }
};

inline std::ostream &operator<<(std::ostream &os, const Bvh &bvh) {
  int channel_count = 0;
  for (auto &joint : bvh.joints) {
    channel_count += joint.channels.size();
  }

  os << "<BVH: " << bvh.joints.size()
     << " joints: " << (bvh.frames.size() / channel_count) //
     << " frames/" << bvh.frame_time                       //
     << " max_height: " << bvh.max_height                  //
     << std::endl;
  for (auto joint : bvh.joints) {
    os << "  " << joint << std::endl;
  }
  os << ">" << std::endl;

  return os;
}
