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

///
/// Mat3 for bvh rotation
///
/// [0, 1, 2][x]    [0x + 1y + 2z]
/// [3, 4, 5][y] => [3x + 4y + 5z]
/// [6, 7, 8][z]    [6x + 7y + 8z]
///
struct BvhMat3 {
  float _0 = 1;
  float _1 = 0;
  float _2 = 0;
  float _3 = 0;
  float _4 = 1;
  float _5 = 0;
  float _6 = 0;
  float _7 = 0;
  float _8 = 1;
  static BvhMat3 RotateXDegrees(float degree);
  static BvhMat3 RotateYDegrees(float degree);
  static BvhMat3 RotateZDegrees(float degree);
  BvhMat3 operator*(const BvhMat3 &rhs);
  BvhMat3 Transpose() const {
    return {
        _0, _3, _6, //
        _1, _4, _7, //
        _2, _5, _8  //
    };
  }
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
  size_t startIndex;
  BvhChannelTypes values[6] = {};
  BvhChannelTypes operator[](size_t index) const { return values[index]; }
  BvhChannelTypes &operator[](size_t index) { return values[index]; }
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

inline std::ostream &operator<<(std::ostream &os, const BvhChannels channels) {
  for (int i = 0; i < 6; ++i) {
    if (channels[i] == BvhChannelTypes::None) {
      break;
    }
    if (i) {
      os << ", ";
    }
    os << to_str(channels[i]);
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

  std::tuple<BvhOffset, BvhMat3> Resolve(const BvhChannels &channels) const;
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
