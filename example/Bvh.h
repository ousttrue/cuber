#pragma once
#include <memory>
#include <ostream>
#include <span>

class Bvh {
public:
  Bvh();
  ~Bvh();
  bool Parse(std::string_view src);
};

inline std::ostream &operator<<(std::ostream &os, const Bvh &bvh) { return os; }
