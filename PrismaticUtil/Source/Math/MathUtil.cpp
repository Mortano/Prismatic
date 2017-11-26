#include "Math\MathUtil.h"

glm::vec4 pe::Forward(const glm::mat4 &mat) { return mat[2]; }

glm::vec4 pe::Right(const glm::mat4 &mat) { return mat[0]; }

glm::vec4 pe::Up(const glm::mat4 &mat) { return mat[1]; }

glm::vec4 pe::Position(const glm::mat4 &mat) { return mat[3]; }

glm::vec3 pe::ToVec3(const glm::vec4 &v) { return glm::vec3{v}; }

float pe::CosTheta(const glm::vec3 &v) { return v.z; }

float pe::AbsCosTheta(const glm::vec3 &v) { return std::fabsf(v.z); }

bool pe::SameHemisphere(const glm::vec3 &v1, const glm::vec3 &v2) {
  return v1.z * v2.z > 0.f;
}
