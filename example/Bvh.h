#pragma once
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
  Zrotation,
  Xrotation,
  Yrotation,
};

struct BvhChannels {
  BvhChannelTypes values[6] = {};
};

struct BvhJoint {
  std::string name;
  int parent;
  BvhOffset offset;
  BvhChannels channels;
};

inline std::ostream &operator<<(std::ostream &os, const BvhJoint &joint) {
  os << joint.name;
  return os;
}

class Bvh {
public:
  std::vector<BvhJoint> joints;
  Bvh();
  ~Bvh();
  bool Parse(std::string_view src);
};

inline std::ostream &operator<<(std::ostream &os, const Bvh &bvh) {
  os << "<BVH: " << bvh.joints.size() << "joints" << std::endl;
  for (auto joint : bvh.joints) {
    os << "  " << joint << std::endl;
  }
  os << ">" << std::endl;

  return os;
}
