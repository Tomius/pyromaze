// Copyright (c) 2014, Tamas Csala

#ifndef ENGINE_MESH_MESH_RENDERER_H_
#define ENGINE_MESH_MESH_RENDERER_H_

#include <map>
#include <memory>
#include <climits>
#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <btBulletDynamicsCommon.h>

#include "engine/mesh/assimp.hpp"
#include "engine/collision/bounding_box.hpp"

namespace engine {

/// A class that can load in and draw meshes using assimp.
class MeshRenderer {
 public:
  enum AttribPosition {
    kPositionAttribLocation = 0,
    kTexcoordAttribLocation = 1,
    kNormalAttribLocation = 2,
    kTangentAttribLocation = 3,
    kModelMatrixAttributeLocation = 4
  };

 protected:
  struct MeshDataStorage {
    gl::VertexArray vao;
    gl::ArrayBuffer positions_buffer,
                    normals_buffer,
                    tangents_buffer,
                    texcoords_buffer,
                    model_matrix_buffer,
                    model_matrix_buffer_2;
    gl::IndexBuffer indices_buffer;

    size_t vertex_count = 0;
    size_t vertex_allocation = 0;
    size_t idx_count = 0;
    size_t idx_allocation = 0;
    size_t model_matrix_buffer_allocation = 0;
    bool first_model_matrix_buffer = true;

    void uploadVertexData(const std::vector<glm::vec3>& positions,
                          const std::vector<glm::vec3>& normals,
                          const std::vector<glm::vec3>& tangets,
                          const std::vector<glm::vec2>& texcoords);

    void uploadIndexData(const std::vector<GLuint>& indices);

    void uploadModelMatrices(const std::vector<glm::mat4>& matrices);

   private:
    template<typename T, typename Buffer>
    void uploadNewData(const std::vector<T>& data, Buffer& buffer,
                       size_t allocation, size_t count) {
      assert(allocation >= count + data.size());

      gl::Bind(buffer);
      buffer.subData(count * sizeof(T),
                     data.size() * sizeof(T),
                     data.data());
    }

    template<typename T, typename Buffer>
    void reallocUploadNewData(const std::vector<T>& data, Buffer& buffer,
                              size_t allocation, size_t count) {
      assert(allocation >= count + data.size());
      size_t prev_size = count*sizeof(T);
      size_t new_size = allocation*sizeof(T);

      // Alloc tmp buffer
      Buffer temp_buffer;
      gl::Bind(temp_buffer);
      temp_buffer.data(new_size, nullptr);

      // Copy old data to tmp buffer
      glBindBuffer(GL_COPY_READ_BUFFER, buffer.expose());
      glBindBuffer(GL_COPY_WRITE_BUFFER, temp_buffer.expose());
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, prev_size);

      // Upload new data
      temp_buffer.subData(prev_size, data.size()*sizeof(T), data.data());

      // Swap the buffers
      buffer = std::move(temp_buffer);
    }

    gl::ArrayBuffer& currentModelMatrixBuffer();
    void ensureModelMatrixBufferSize(size_t size);
    void setupModelMatrixAttrib();
  };

  MeshDataStorage& getMeshDataStorage() {
    static MeshDataStorage instace;
    return instace;
  }

  /**
   * @brief A class to store per mesh data (the class loads in a scene, that
   *        might contain multiply meshes).
   */
  struct MeshEntry {
    constexpr static unsigned kInvalidMaterial = unsigned(-1);
    unsigned material_index = kInvalidMaterial;

    unsigned base_idx = 0;
    unsigned idx_count = 0;
  };

  /// The assimp importer. The scene actually belongs to this.
  Assimp::Importer importer_;

  /// A pointer to the scene stored by the importer. But this is the working interface for it.
  const aiScene* scene_;

  /// The name of the file loaded in. It is stored to be able to print it out if an error happens.
  std::string filename_;

  /// The vao-s and buffers per mesh.
  std::vector<MeshEntry> entries_;

  /// The transformation that takes the model's world coordinates to the OpenGL style world coordinates.
  glm::mat4 world_transformation_;

  /// A struct containin the state and data of a material type.
  struct MaterialInfo {
    bool active;
    int tex_unit;
    std::vector<gl::Texture2D> textures;

    MaterialInfo() : active(false), tex_unit(0) {}
  };

  /// The materials.
  std::map<aiTextureType, MaterialInfo> materials_;

  /// Model-space bounding box of the mesh (without transformations applied)
  mutable BoundingBox model_space_bounding_box_;
  mutable bool is_setup_model_space_bounding_box_ = false;

  /// Stores if the setup function was called (it shouldn't be called more than once).
  bool is_setup_ = false;
  /// Textures can be disabled, and not used for rendering
  bool textures_enabled_ = true;

  unsigned triangle_count = 0;

  /// It shouldn't be copyable.
  MeshRenderer(const MeshRenderer& src) = delete;
  /// It shouldn't be copyable.
  void operator=(const MeshRenderer& rhs) = delete;

public:
  /// Loads in the mesh from a file, and does some post-processing on it.
  /** @param filename - The name of the file to load in.
    * @param flags - The assimp post-process flags. */
  MeshRenderer(const std::string& filename,
               gl::Bitfield<aiPostProcessSteps> flags);

  /// Sets up a btTriangleIndexVertexArray, and returns a vector of indices
  /// that should be stored throughout the lifetime of the bullet object
  std::vector<int> btTriangles(btTriangleIndexVertexArray* triangles);

public:
  void setup();

private:
  std::vector<GLuint>    getIndices(const aiMesh* mesh);
  std::vector<glm::vec3> getPositions(const aiMesh* mesh);
  std::vector<glm::vec3> getNormals(const aiMesh* mesh);
  std::vector<glm::vec3> getTangents(const aiMesh* mesh);
  std::vector<glm::vec2> getTexCoords(size_t index,
                                      unsigned char tex_coord_set = 0);

public:
  void uploadModelMatrices(const std::vector<glm::mat4>& matrices);

  /// Checks if every mesh in the scene has tex_coords
  /** Returns true if all of the meshes in the scene have texture
    * coordinates in the specified texture coordinate set.
    * @param tex_coord_set - Specifies the index of the texture coordinate set that should be inspected */
  bool hasTexCoords(unsigned char tex_coord_set = 0);

  /**
   * @brief Loads in a specified type of texture for every mesh. If no texture
   *        but a single color is specified, then sets up an 1x1 texture with
   *        that color (so you can use the same shader).
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
  void setupTextures(unsigned short texture_unit,
                     aiTextureType tex_type,
                     const char *pKey,
                     unsigned int type,
                     unsigned int idx,
                     bool srgb = true);

  /// Sets the diffuse textures up to a specified texture unit.
  /** Changes the currently active texture unit and Texture2D binding.
    * @param texture_unit - Specifies the texture unit to use for the diffuse textures. */
  void setupDiffuseTextures(unsigned short texture_unit, bool srbg = true);

  /// Sets the specular textures up to a specified texture unit.
  /** Changes the currently active texture unit and Texture2D binding.
    * @param texture_unit - Specifies the texture unit to use for the specular textures. */
  void setupSpecularTextures(unsigned short texture_unit);

  /// Renders the mesh.
  /** Changes the currently active VAO and may change the Texture2D binding */
  void render(size_t instance_count = 1);

private:
  /// Ensures that the model-space bounding box is calculated.
  void calculateModelSpaceBoundBox() const;

public:
  /// Gives information about the mesh's bounding cuboid.
  BoundingBox boundingBox(const glm::mat4& matrix = glm::mat4{}) const;

  /// Returns the transformation that takes the model's world coordinates to the OpenGL style world coordinates.
  /** i.e if you see that a character is laying on ground instead of standing, it is probably
    * because the character is defined in a space where Z is up not Y. Right multiplying your
    * model matrix with this matrix will solve that problem. */
  glm::mat4 worldTransform() const;

  /// Returns the bounding sphere from the bounding box
  glm::vec4 bSphere(const BoundingBox& bbox) const;

  /// Returns the center offseted by the model matrix (as xyz) and radius (as w) of the bounding sphere.
  glm::vec4 bSphere(const glm::mat4& modelMatrix = glm::mat4{}) const;

  /// Returns the center of the bounding sphere.
  glm::vec3 bSphereCenter() const;

  /// Returns the radius of the bounding sphere.
  float bSphereRadius() const;

  unsigned triangleCount() const { return triangle_count; }

  /// Enables the use of textures for rendering.
  void enableTextures() { textures_enabled_ = true; }

  /// Disables the use of textures for rendering.
  void disableTextures() { textures_enabled_ = true; }
};

}  // namespace engine

#endif  // ENGINE_MESH_MESH_RENDERER_H_
