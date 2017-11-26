#pragma once

#include "peUtilDefs.h"
#include <glm\mat4x4.hpp>

namespace pe {

glm::vec4 PE_UTIL_API Forward(const glm::mat4 &mat);
glm::vec4 PE_UTIL_API Right(const glm::mat4 &mat);
glm::vec4 PE_UTIL_API Up(const glm::mat4 &mat);
glm::vec4 PE_UTIL_API Position(const glm::mat4 &mat);

glm::vec3 PE_UTIL_API ToVec3(const glm::vec4 &v);

float PE_UTIL_API CosTheta(const glm::vec3 &v);
float PE_UTIL_API AbsCosTheta(const glm::vec3 &v);
bool PE_UTIL_API SameHemisphere(const glm::vec3 &v1, const glm::vec3 &v2);

template <typename T> T Clamp(T val, T min, T max) {
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

template <typename T> T Lerp(T factor, T min, T max) {
  return min + factor * (max - min);
}

} // namespace pe
