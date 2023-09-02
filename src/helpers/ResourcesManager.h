#ifndef ResourcesManager_h
#define ResourcesManager_h

#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <gl3w/gl3w.h>

#include "../resources/meshes.h"

class ResourcesManager {

public:
	
	static std::string getStringForShader(const std::string & shaderName);
	
	static void loadResources();
	
	static GLuint getTextureFor(const std::string & fileName);
	
	static glm::vec2 getTextureSizeFor(const std::string & fileName);
	
private:
	
	static  unsigned char* getDataForImage(const std::string & fileName, unsigned int & imwidth, unsigned int & imheight);
	
	static std::unordered_map<std::string, std::string> shadersLibrary;
	
	static std::unordered_map<std::string, unsigned char*> imagesLibrary;
	
	static std::unordered_map<std::string, glm::vec2> imagesSize;
	
	static std::unordered_map<std::string, GLuint> textureLibrary;
};

#endif
