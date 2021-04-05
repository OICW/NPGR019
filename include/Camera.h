/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glm/glm.hpp>

// Movement directions bitfield
enum class MovementDirections : int
{
  None     = 0x0000,
  Forward  = 0x0001,
  Backward = 0x0002,
  Left     = 0x0004,
  Right    = 0x0008,
  Up       = 0x0010,
  Down     = 0x0020
};

// General camera class
class Camera
{
public:
  Camera();

  // Sets the movement speed
  void SetMovementSpeed(float speed) { _movementSpeed = speed; }
  // Sets sensitivity
  void SetSensitivity(float sensitivity) { _sensitivity = sensitivity; }
  // Sets transformation using eye, look at point and up vector
  void SetTransformation(const glm::vec3& eye, const glm::vec3& lookAt, const glm::vec3& up);
  // Returns const reference to the internal camera transformation
  const glm::mat4x4& GetWorldToView() const { return _worldToView; }
  // Returns const reference to the internal camera transformation inverse
  const glm::mat4x4& GetViewToWorld() const { return _viewToWorld; }
  // Sets camera projection using field of view and aspect ratio
  void SetProjection(float fov, float aspect, float near, float far);
  // Returns the camera projection matrix
  const glm::mat4x4& GetProjection() const { return _projection; }
  // Moves camera along designated directions and orients it using mouse
  void Move(MovementDirections direction, const glm::vec2& mouseMove, float dt);

protected:
  // World to view transformation matrix
  glm::mat4x4 _worldToView;
  // View to world transformation matrix
  glm::mat4x4 _viewToWorld;
  // Projection matrix
  glm::mat4x4 _projection;
  // Camera movement speed
  float _movementSpeed;
  // Camera sensitivity for mouse movement
  float _sensitivity;

};
