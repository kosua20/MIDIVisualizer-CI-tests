#ifndef ProgramUtilities_h
#define ProgramUtilities_h

#include <gl3w/gl3w.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

/// This macro is used to check for OpenGL errors with access to the file and line number where the error is detected.
#define checkGLError() _checkGLError(__FILE__, __LINE__)

/// Converts a GLenum error number into a human-readable string.
std::string getGLErrorString(GLenum error);

/// Check if any OpenGL error has been detected and log it.
int _checkGLError(const char *file, int line);

struct GPUHandle {
	GLint _handle{-1};

	operator GLint() const { return _handle; }
	GPUHandle& operator=(const GLint handle){ _handle = handle; return *this; }
};

class ShaderProgram {
public:

	void init(const std::string & vertexName, const std::string & fragmentName);

	void use();

	void clean();

	void texture(const std::string& name, GLuint texture, GLenum shape);

	template<typename T>
	void uniform(const std::string& name, const T& val);

	template<typename T>
	void uniforms(const std::string& name, size_t count, const T* vals);

private:

	/// Load a shader of the given type from a string
	static GLuint loadShader(const std::string & prog, GLuint type);

	static GLuint createGLProgramFromStrings(const std::string & vertexContent, const std::string & fragmentContent);

	std::unordered_map<std::string, GPUHandle> _uniforms;
	std::unordered_map<std::string, int> _textures;
	GLuint _id;
};

template<>
inline void ShaderProgram::uniform(const std::string& name, const float& val){
	glUniform1f(_uniforms[name], val);
}

template<>
inline void ShaderProgram::uniform(const std::string& name, const int& val){
	glUniform1i(_uniforms[name], val);
}

template<>
inline void ShaderProgram::uniform(const std::string& name, const bool& val){
	glUniform1i(_uniforms[name], val ? 1 : 0);
}

template<>
inline void ShaderProgram::uniform(const std::string& name, const glm::vec2& val){
	glUniform2fv(_uniforms[name], 1, &val[0]);
}

template<>
inline void ShaderProgram::uniform(const std::string& name, const glm::vec3& val){
	glUniform3fv(_uniforms[name], 1, &val[0]);
}

template<>
inline void ShaderProgram::uniform(const std::string& name, const glm::vec4& val){
	glUniform4fv(_uniforms[name], 1, &val[0]);
}

template<>
inline void ShaderProgram::uniforms(const std::string& name, size_t count, const int* vals){
	glUniform1iv(_uniforms[name], (unsigned int)count, vals);
}

template<>
inline void ShaderProgram::uniforms(const std::string& name, size_t count, const float* vals){
	glUniform1fv(_uniforms[name], (unsigned int)count, vals);
}

template<>
inline void ShaderProgram::uniforms(const std::string& name, size_t count, const glm::vec2* vals){
	glUniform2fv(_uniforms[name], (unsigned int)count, (GLfloat*)vals);
}

template<>
inline void ShaderProgram::uniforms(const std::string& name, size_t count, const glm::vec3* vals){
	glUniform3fv(_uniforms[name], (unsigned int)count, (GLfloat*)vals);
}

template<>
inline void ShaderProgram::uniforms(const std::string& name, size_t count, const glm::vec4* vals){
	glUniform4fv(_uniforms[name], (unsigned int)count, (GLfloat*)vals);
}

// Texture loading.

// 2D texture.

GLuint loadTexture(const std::string& path, unsigned int channels, bool sRGB);

//GLuint loadTexture(const std::string& path, const GLuint program, const GLuint textureSlot, const std::string& uniformName, bool sRGB = false);

GLuint loadTexture( unsigned char* image, unsigned imwidth, unsigned imheight, unsigned int channels, bool sRGB);

GLuint loadTextureArray(const std::vector<std::string>& paths, bool sRGB, int & layers);

GLuint loadTextureArray(const std::vector<unsigned char*>& images, const std::vector<glm::ivec2> & sizes, unsigned int channels, bool sRGB);

std::vector<GLuint> generate2DViewsOfArray(GLuint tex, unsigned int maxSize);

// Cubemap texture.
//GLuint loadTextureCubeMap(const std::string& pathBase, bool sRGB);

//GLuint loadTextureCubeMap(const std::string& pathBase, const GLuint program, const GLuint textureSlot, const std::string& uniformName, bool sRGB = false);


#endif
