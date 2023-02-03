#pragma once
#include <list>
#include <memory>

class BvhNode {

public:
  std::list<std::shared_ptr<BvhNode>> Children;
  static std::shared_ptr<BvhNode> Create();
};

class BvhSolver {

  std::shared_ptr<BvhNode> root_;

public:
};
