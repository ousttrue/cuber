#pragma once
#include <stdint.h>

//
// SingleRootHierarchicalTransformation
//

union PackQuat {
  uint32_t value;
};

struct JointDefinition {
  // parentBone
  uint16_t parentBoneIndex;
  // HumanBones or any number
  uint16_t boneType;
  float x;
  float y;
  float z;
};
static_assert(sizeof(JointDefinition) == 16, "JointDefintion");

struct SkeletonHeader {
  char magic[8] = {'S', 'R', 'H', 'T', 'S', 'K', 'L', '1'};
  uint32_t skeletonId = 0;
  uint16_t jointCount = 0;
  uint16_t hasInitialRotation = 0;
};
// continue JointDefinition X jointCount
// if hasInitialRotation PackQuat X jointCount for InitialRotation
static_assert(sizeof(SkeletonHeader) == 16, "Skeleton");

struct FrameHeader {
  char magic[8] = {'S', 'R', 'H', 'T', 'F', 'R', 'M', '1'};
  // std::chrono::nanoseconds
  uint64_t time;
  uint32_t skeletonId = 0;
  // root position
  float x;
  float y;
  float z;
};
// continue PackQuat x SkeletonHeader::JointCount
static_assert(sizeof(FrameHeader) == 32, "FrameSize");
