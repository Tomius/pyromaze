// Copyright (c) Tamas Csala

#include <vector>
#include <lodepng.h>
#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include "engine/mesh/mesh_renderer.hpp"
#include "settings.hpp"

namespace engine {

void MeshRenderer::MeshDataStorage::uploadVertexData(
          const std::vector<glm::vec3>& positions,
          const std::vector<glm::vec3>& normals,
          const std::vector<glm::vec3>& tangets,
          const std::vector<glm::vec2>& texcoords) {
  size_t vertex_count_to_upload = positions.size();
  assert(normals.size() == vertex_count_to_upload);
  assert(tangets.size() == vertex_count_to_upload);
  assert(texcoords.size() == vertex_count_to_upload);

  gl::Bind(vao);
  if (vertex_allocation < vertex_count + vertex_count_to_upload) {
    if (vertex_allocation == 0) {
      vertex_allocation = vertex_count_to_upload;
    } else {
      vertex_allocation = 2 * (vertex_count + vertex_count_to_upload);
    }

    reallocUploadNewData(positions, positions_buffer, vertex_allocation, vertex_count);
    reallocUploadNewData(normals, normals_buffer, vertex_allocation, vertex_count);
    reallocUploadNewData(tangets, tangents_buffer, vertex_allocation, vertex_count);
    reallocUploadNewData(texcoords, texcoords_buffer, vertex_allocation, vertex_count);

    gl::Bind(positions_buffer);
    gl::VertexAttribObject(kPositionAttribLocation).setup<glm::vec3>().enable();

    gl::Bind(normals_buffer);
    gl::VertexAttribObject(kNormalAttribLocation).setup<glm::vec3>().enable();

    gl::Bind(tangents_buffer);
    gl::VertexAttribObject(kTangentAttribLocation).setup<glm::vec3>().enable();

    gl::Bind(texcoords_buffer);
    gl::VertexAttribObject(kTexcoordAttribLocation).setup<glm::vec2>().enable();

  } else {
    uploadNewData(positions, positions_buffer, vertex_allocation, vertex_count);
    uploadNewData(normals, normals_buffer, vertex_allocation, vertex_count);
    uploadNewData(tangets, tangents_buffer, vertex_allocation, vertex_count);
    uploadNewData(texcoords, texcoords_buffer, vertex_allocation, vertex_count);
  }

  vertex_count += vertex_count_to_upload;
  gl::Unbind(gl::kArrayBuffer);
  gl::Unbind(gl::kVertexArray);
}

void MeshRenderer::MeshDataStorage::uploadIndexData(const std::vector<GLuint>& indices) {
  gl::Bind(vao);
  if (idx_allocation < idx_count + indices.size()) {
    if (idx_allocation == 0) {
      idx_allocation = indices.size();
    } else {
      idx_allocation = 2 * (idx_count + indices.size());
    }

    reallocUploadNewData(indices, indices_buffer, idx_allocation, idx_count);

  } else {
    uploadNewData(indices, indices_buffer, idx_allocation, idx_count);
  }

  idx_count += indices.size();
  gl::Bind(indices_buffer);
  gl::Unbind(gl::kVertexArray);
}

void MeshRenderer::MeshDataStorage::uploadModelMatrices(const std::vector<glm::mat4>& matrices) {
  gl::Bind(vao);
  gl::Bind(model_matrix_buffer);
  ensureModelMatrixBufferSize(matrices.size());
  model_matrix_buffer.subData(0, matrices.size() * sizeof(glm::mat4), matrices.data());
  gl::Unbind(model_matrix_buffer);
  gl::Unbind(vao);
}

void MeshRenderer::MeshDataStorage::ensureModelMatrixBufferSize(size_t size) {
  if (model_matrix_buffer_allocation < size) {
    model_matrix_buffer_allocation = 2*size;
    model_matrix_buffer.data(model_matrix_buffer_allocation * sizeof(glm::mat4), nullptr, gl::kDynamicDraw);
    setupModelMatrixAttrib();
  }
}

void MeshRenderer::MeshDataStorage::setupModelMatrixAttrib() {
  for (int i = 0; i < 4; ++i) {
    auto attrib = gl::VertexAttribObject(kModelMatrixAttributeLocation + i);
    attrib.pointer(4, gl::kFloat, false, sizeof(glm::mat4), (void*)(i*sizeof(glm::vec4)));
    attrib.divisor(1);
    attrib.enable();
  }
}

/// Loads in the mesh from a file, and does some post-processing on it.
/** @param filename - The name of the file to load in.
  * @param flags - The assimp post-process flags. */
MeshRenderer::MeshRenderer(const std::string& filename,
                           gl::Bitfield<aiPostProcessSteps> flags)
    : scene_(importer_.ReadFile(filename.c_str(), flags|aiProcess_Triangulate))
    , filename_(filename)
    , entries_(scene_ ? scene_->mNumMeshes : 0) {
  if (!scene_) {
    throw std::runtime_error("Error parsing " + filename_ + " : " +
                             importer_.GetErrorString());
  }

  // The world transform is the transform that takes the root node to it's
  // parent's space, which is the OpenGL style world space. The inverse of this
  // is stored as an attribute of the scene's root node.
  world_transformation_ =
    glm::inverse(engine::convertMatrix(scene_->mRootNode->mTransformation));
}

/// Sets up a btTriangleIndexVertexArray, and returns a vector of indices
/// that should be stored throughout the lifetime of the bullet object
std::vector<int> MeshRenderer::btTriangles(btTriangleIndexVertexArray* triangles) {
  std::vector<int> indices_vector;

  for (unsigned mesh_idx = 0; mesh_idx < scene_->mNumMeshes; ++mesh_idx) {
    const aiMesh* mesh = scene_->mMeshes[mesh_idx];
    btIndexedMesh btMesh;
    btMesh.m_numVertices = mesh->mNumVertices;
    btMesh.m_vertexBase = (const unsigned char*)mesh->mVertices;
    btMesh.m_vertexStride = sizeof(aiVector3D);
    btMesh.m_vertexType = PHY_FLOAT;

    auto indices_begin_idx = indices_vector.size();
    indices_vector.reserve(indices_vector.size() + mesh->mNumFaces * 3);
    for (size_t face_idx = 0; face_idx < mesh->mNumFaces; face_idx++) {
      const aiFace& face = mesh->mFaces[face_idx];
      if (face.mNumIndices == 3) {  // The invalid faces are just ignored.
        indices_vector.push_back(face.mIndices[0]);
        indices_vector.push_back(face.mIndices[1]);
        indices_vector.push_back(face.mIndices[2]);
      }
    }
    btMesh.m_numTriangles = (indices_vector.size()-indices_begin_idx)/3;
    btMesh.m_triangleIndexBase = (const unsigned char*)(indices_vector.data() + indices_begin_idx);
    btMesh.m_triangleIndexStride = 3*sizeof(int);
    btMesh.m_indexType = PHY_INTEGER;

    triangles->addIndexedMesh(btMesh, PHY_INTEGER);
  }

  return indices_vector;
}

void MeshRenderer::setup()
{
  if (!is_setup_) {
    is_setup_ = true;
  } else {
    std::cerr << "MeshRenderer::setup is called multiple times on the "
                 "same object. If the two calls want to set positions up into "
                 "the same attribute position, then the second call is "
                 "unneccesary. If they want to set the positions to different "
                 "attribute positions then the second call would make the "
                 "first call not work anymore. Either way, calling "
                 "setup multiply times is a design error, that should "
                 "be avoided.";
    std::terminate();
  }

  MeshDataStorage& mesh_data_storage = getMeshDataStorage();
  gl::Bind(mesh_data_storage.vao);

  for (size_t i = 0; i < entries_.size(); i++) {
    entries_[i].base_idx = mesh_data_storage.idx_count;

    const aiMesh* mesh = scene_->mMeshes[i];
    std::vector<GLuint> indices = getIndices(mesh);
    std::vector<glm::vec3> positions = getPositions(mesh);
    std::vector<glm::vec3> normals = getNormals(mesh);
    std::vector<glm::vec3> tangents = getTangents(mesh);
    std::vector<glm::vec2> texcoords = getTexCoords(i);

    mesh_data_storage.uploadVertexData(positions, normals, tangents, texcoords);
    mesh_data_storage.uploadIndexData(indices);

    entries_[i].idx_count = mesh_data_storage.idx_count - entries_[i].base_idx;
  }
}

std::vector<GLuint> MeshRenderer::getIndices(const aiMesh* mesh) {
  std::vector<GLuint> indices_vector;
  indices_vector.reserve(mesh->mNumFaces * 3);
  bool invalid_triangles = false;

  size_t idx_offset = getMeshDataStorage().vertex_count;
  for (size_t i = 0; i < mesh->mNumFaces; i++) {
    const aiFace& face = mesh->mFaces[i];
    if (face.mNumIndices == 3) {  // The invalid faces are just ignored.
      indices_vector.push_back(idx_offset + face.mIndices[0]);
      indices_vector.push_back(idx_offset + face.mIndices[1]);
      indices_vector.push_back(idx_offset + face.mIndices[2]);
    } else {
      invalid_triangles = true;
    }
  }

  if (invalid_triangles) {
    std::cerr << "Mesh '" << filename_ << "' contains non-triangle faces. "
                 "This might result in rendering artifacts." << std::endl;
  }

  return indices_vector;
}

std::vector<glm::vec3> MeshRenderer::getPositions(const aiMesh* mesh) {
  std::vector<glm::vec3> positions_vector;
  positions_vector.reserve(mesh->mNumVertices);
  for (int i = 0; i < mesh->mNumVertices; ++i) {
    const aiVector3D& pos = mesh->mVertices[i];
    positions_vector.emplace_back(pos.x, pos.y, pos.z);
  }

  return positions_vector;
}

std::vector<glm::vec3> MeshRenderer::getNormals(const aiMesh* mesh) {
  std::vector<glm::vec3> normals_vector;
  size_t vert_num = mesh->mNumVertices;
  if (mesh->HasNormals()) {
    normals_vector.reserve(vert_num);
    for (int i = 0; i < vert_num; ++i) {
      const aiVector3D& normal = mesh->mNormals[i];
      normals_vector.emplace_back(normal.x, normal.y, normal.z);
    }
  } else {
    normals_vector.resize(vert_num);
  }

  return normals_vector;
}

std::vector<glm::vec3> MeshRenderer::getTangents(const aiMesh* mesh) {
  std::vector<glm::vec3> tangents_vector;
  size_t vert_num = mesh->mNumVertices;
  if (mesh->HasTangentsAndBitangents()) {
    tangents_vector.reserve(vert_num);
    for (int i = 0; i < vert_num; ++i) {
      const aiVector3D& tangent = mesh->mTangents[i];
      tangents_vector.emplace_back(tangent.x, tangent.y, tangent.z);
    }
  } else {
    tangents_vector.resize(vert_num);
  }

  return tangents_vector;
}

std::vector<glm::vec2> MeshRenderer::getTexCoords(size_t index,
                                                  unsigned char tex_coord_set) {
  const aiMesh* mesh = scene_->mMeshes[index];
  entries_[index].material_index = mesh->mMaterialIndex;

  std::vector<glm::vec2> tex_coords_vector;

  size_t vert_num = mesh->mNumVertices;
  if (mesh->HasTextureCoords(tex_coord_set)) {
    tex_coords_vector.reserve(vert_num);
    for (size_t i = 0; i < vert_num; i++) {
      const aiVector3D& tex_coord = mesh->mTextureCoords[tex_coord_set][i];
      tex_coords_vector.emplace_back(tex_coord.x, tex_coord.y);
    }
  } else {
    tex_coords_vector.resize(vert_num);
  }

  return tex_coords_vector;
}

void MeshRenderer::uploadModelMatrices(const std::vector<glm::mat4>& matrices) {
  getMeshDataStorage().uploadModelMatrices(matrices);
}

/// Checks if every mesh in the scene has tex_coords
/** Returns true if all of the meshes in the scene have texture
  * coordinates in the specified texture coordinate set.
  * @param tex_coord_set  Specifies the index of the texture coordinate
  *                     set that should be inspected */
bool MeshRenderer::hasTexCoords(unsigned char tex_coord_set) {
  for (size_t i = 0; i < entries_.size(); i++) {
    if (!scene_->mMeshes[i]->HasTextureCoords(tex_coord_set)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Loads in a specified type of texture for every mesh. If no texture but
 *        a single color is specified, then sets up an 1x1 texture with that
 *        color (so you can use the same shader).
 *
 * Changes the currently active texture unit and Texture2D binding.
 * @param texture_unit      Specifies the texture unit to use for the textures.
 * @param tex_type          The type of the texture to load in. For ex
 *                          aiTextureType_DIFFUSE.
 * @param pKey, type, idx   These parameters identify the color parameter to
 *                          load in case there isn't any texture specified.
 *                          Use the assimp macros to fill these 3 parameters
 *                          all at once, for ex: AI_MATKEY_COLOR_DIFFUSE
 * @param srgb              Specifies weather the image is in srgb colorspace
 */
void MeshRenderer::setupTextures(unsigned short texture_unit,
                                 aiTextureType tex_type,
                                 const char *pKey,
                                 unsigned int type,
                                 unsigned int idx,
                                 bool srgb) {
  gl::ActiveTexture(texture_unit);

  materials_[tex_type].active = true;
  materials_[tex_type].tex_unit = texture_unit;

  if (scene_->mNumMaterials) {
    // Extract the directory part from the file name
    std::string::size_type slash_idx = filename_.find_last_of("/");
    std::string dir;

    if (slash_idx == std::string::npos) {
      dir = "./";
    } else if (slash_idx == 0) {
      dir = "/";
    } else {
      dir = filename_.substr(0, slash_idx + 1);
    }

    // Initialize the materials
    for (unsigned int i = 0; i < scene_->mNumMaterials; ++i) {
      const aiMaterial* mat = scene_->mMaterials[i];
      materials_[tex_type].textures.push_back(gl::Texture2D{});

      aiString filepath;
      if (mat->GetTexture(tex_type, 0, &filepath) == AI_SUCCESS) {
        gl::Bind(materials_[tex_type].textures[i]);
        unsigned width, height;
        std::vector<unsigned char> data;
        std::string path = dir + filepath.data;
        unsigned error = lodepng::decode(data, width, height, path, LCT_RGBA, 8);
        if (error) {
          std::cerr << "Image decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
          throw std::runtime_error("Image decoder error");
        }
        materials_[tex_type].textures[i].upload(gl::kSrgb8Alpha8, width, height,
            gl::kRgba, gl::kUnsignedByte, data.data());
        materials_[tex_type].textures[i].minFilter(gl::kLinear);
        materials_[tex_type].textures[i].magFilter(gl::kLinear);
      } else {
        aiColor4D color(0.f, 0.f, 0.f, 1.0f);
        mat->Get(pKey, type, idx, color);

        gl::Bind(materials_[tex_type].textures[i]);
        materials_[tex_type].textures[i].upload(gl::kRgba32F, 1, 1, gl::kRgba,
                                                gl::kFloat, &color.r);
        materials_[tex_type].textures[i].minFilter(gl::kNearest);
        materials_[tex_type].textures[i].magFilter(gl::kNearest);
      }
    }
  }

  gl::Unbind(gl::kTexture2D);
}

/// Sets the diffuse textures up to a specified texture unit.
/** Changes the currently active texture unit and Texture2D binding.
  * @param texture_unit Specifies the texture unit to use for the diffuse textures. */
void MeshRenderer::setupDiffuseTextures(unsigned short texture_unit, bool srbg) {
  setupTextures(texture_unit, aiTextureType_DIFFUSE,
                AI_MATKEY_COLOR_DIFFUSE, srbg);
}

/// Sets the specular textures up to a specified texture unit.
/** Changes the currently active texture unit and Texture2D binding.
  * @param texture_unit Specifies the texture unit to use for the specular textures. */
void MeshRenderer::setupSpecularTextures(unsigned short texture_unit) {
  setupTextures(texture_unit, aiTextureType_SPECULAR,
                AI_MATKEY_COLOR_SPECULAR, false);
}

/// Renders the mesh.
/** Changes the currently active VAO and may change the Texture2D binding */
void MeshRenderer::render(size_t instance_count) {
  if (!is_setup_) {
    return;  // we can't render the mesh, if we don't have any vertex.
  }
  for (size_t i = 0 ; i < entries_.size(); i++) {
    gl::Bind(getMeshDataStorage().vao);

    const size_t material_index = entries_[i].material_index;

    if (textures_enabled_) {
      for (auto iter = materials_.begin(); iter != materials_.end(); iter++) {
        auto& material = iter->second;
        if (material.active == true && material_index < scene_->mNumMaterials) {
          gl::ActiveTexture(material.tex_unit);
        }
        gl::Bind(material.textures[material_index]);
      }
    }

    if (Optimizations::kInstancing) {
      glDrawElementsInstanced(GL_TRIANGLES,
                              entries_[i].idx_count,
                              GL_UNSIGNED_INT,
                              (void*)(entries_[i].base_idx * sizeof(unsigned)),
                              instance_count);
    } else {
      for (int instance = 0; instance < instance_count; ++instance) {
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES,
                                            entries_[i].idx_count,
                                            GL_UNSIGNED_INT,
                                            (void*)(entries_[i].base_idx * sizeof(unsigned)),
                                            1,
                                            instance);
      }
    }


    if (textures_enabled_) {
      for (auto iter = materials_.begin(); iter != materials_.end(); iter++) {
        auto& material = iter->second;
        if (material.active == true && material_index < scene_->mNumMaterials) {
          gl::ActiveTexture(material.tex_unit);
        }
        gl::Unbind(material.textures[material_index]);
      }
    }
  }

  gl::Unbind(gl::kVertexArray);
}

/// The transformation that takes the model's world coordinates to the OpenGL style world coordinates.
/** i.e if you see that a character is laying on ground instead of standing, it is probably
  * because the character is defined in a space where XY is flat, and Z is up. Right
  * multiplying your model matrix with this matrix will solve that problem. */
glm::mat4 MeshRenderer::worldTransform() const {
  return world_transformation_;
}

/// Ensures that the model-space bounding box is calculated.
void MeshRenderer::calculateModelSpaceBoundBox() const {
  assert(is_setup_);

  if (!is_setup_model_space_bounding_box_) {
    float zero = 0.0f;  // This is needed to bypass a visual c++ compile error
    float infty = 1.0f / zero;
    glm::vec3 mins{infty, infty, infty}, maxes{-infty, -infty, -infty};
    for (size_t entry = 0; entry < entries_.size(); entry++) {
      const aiMesh* mesh = scene_->mMeshes[entry];

      for (size_t i = 0; i < mesh->mNumVertices; i++) {
        float x = mesh->mVertices[i].x;
        float y = mesh->mVertices[i].y;
        float z = mesh->mVertices[i].z;

        if (x < mins.x) {
          mins.x = x;
        }
        if (y < mins.y) {
          mins.y = y;
        }
        if (z < mins.z) {
          mins.z = z;
        }

        if (maxes.x < x) {
          maxes.x = x;
        }
        if (maxes.y < y) {
          maxes.y = y;
        }
        if (maxes.z < z) {
          maxes.z = z;
        }
      }
    }

    model_space_bounding_box_ = BoundingBox{mins, maxes};
    is_setup_model_space_bounding_box_ = true;
  }
}

/// Gives information about the mesh's bounding cuboid.
BoundingBox MeshRenderer::boundingBox(const glm::mat4& matrix) const {
  calculateModelSpaceBoundBox();

  glm::vec3 trasformed_mins {matrix * glm::vec4{model_space_bounding_box_.mins(), 1}};
  glm::vec3 trasformed_maxes {matrix * glm::vec4{model_space_bounding_box_.maxes(), 1}};

  return BoundingBox{glm::min(trasformed_mins, trasformed_maxes), glm::max(trasformed_mins, trasformed_maxes)};
}

glm::vec4 MeshRenderer::bSphere(const BoundingBox& bbox) const {
  glm::vec3 center = bbox.center(), extent = bbox.extent();
  return glm::vec4(center, sqrt(glm::dot(extent, extent)) / 2);  // Pythagoras.
}

/// Returns the center offseted by the model matrix (as xyz) and radius (as w) of the bounding sphere.
/** @param model_matrix - The matrix to use to offset the center of the bounding sphere. */
glm::vec4 MeshRenderer::bSphere(const glm::mat4& model_matrix) const {
  return bSphere(boundingBox(model_matrix));
}

/// Returns the center of the bounding sphere.
glm::vec3 MeshRenderer::bSphereCenter() const {
  return boundingBox().center();
}

/// Returns the radius of the bounding sphere.
float MeshRenderer::bSphereRadius() const {
  glm::vec3 extent = boundingBox().extent();
  return sqrt(glm::dot(extent, extent)) / 2;  // Pythagoras.
}

}  // namespace engine
