#include "icamera.hpp"

void engine::ICamera::UpdateFrustum() {
  glm::mat4 m = projectionMatrix() * cameraMatrix();

  // See: http://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf
  // REMEMBER: m[i][j] is j-th row, i-th column (glm is column major)

  frustum_ = Frustum{{
    // left
   {m[0][3] + m[0][0],
    m[1][3] + m[1][0],
    m[2][3] + m[2][0],
    m[3][3] + m[3][0]},

    // right
   {m[0][3] - m[0][0],
    m[1][3] - m[1][0],
    m[2][3] - m[2][0],
    m[3][3] - m[3][0]},

    // bottom
   {m[0][3] + m[0][1],
    m[1][3] + m[1][1],
    m[2][3] + m[2][1],
    m[3][3] + m[3][1]},

    // top
   {m[0][3] - m[0][1],
    m[1][3] - m[1][1],
    m[2][3] - m[2][1],
    m[3][3] - m[3][1]},

    // near
   {m[0][3] + m[0][2],
    m[1][3] + m[1][2],
    m[2][3] + m[2][2],
    m[3][3] + m[3][2]},

    // far
   {m[0][3] - m[0][2],
    m[1][3] - m[1][2],
    m[2][3] - m[2][2],
    m[3][3] - m[3][2]}
  }}; // ctor normalizes the planes
}
