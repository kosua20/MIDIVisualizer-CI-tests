#include <stdio.h>
#include <iostream>
#include <vector>

#include "ResourcesManager.h"
#include "ProgramUtilities.h"
// Resources headers.
#include "../resources/data.h"

 unsigned char* ResourcesManager::getDataForImage(const std::string & fileName, unsigned int & imwidth, unsigned int & imheight){
	
	if(imagesLibrary.count(fileName) > 0){
		
		auto size = imagesSize[fileName];
		imwidth = int(size[0]);
		imheight = int(size[1]);
		return imagesLibrary[fileName];
	}
	std::cerr << "[WARNING]: Unable to find ressource for image \"" << fileName << "\"." << std::endl;
	imwidth = 0;
	imheight = 0;
	return NULL;
}

std::string ResourcesManager::getStringForShader(const std::string & shaderName){
	
	if(shadersLibrary.count(shaderName) > 0){
		return shadersLibrary[shaderName];
	}
	
	std::cerr << "[WARNING]: Unable to find ressource for shader \"" << shaderName << "\"." << std::endl;
	return "";
}

void ResourcesManager::loadResources(){
	shadersLibrary.clear();
	imagesLibrary.clear();
	imagesSize.clear();
	
	shadersLibrary = shaders;
	
	imagesLibrary["font"] = font_image;
	imagesLibrary["flash"] = flash_image;
	imagesLibrary["particles"] = particles_image;
	imagesLibrary["noise"] = noise_image;
	imagesLibrary["pedal_side"] = pedal_side_image;
	imagesLibrary["pedal_center"] = pedal_center_image;
	imagesLibrary["pedal_top"] = pedal_top_image;

	imagesSize["font"] = font_size;
	imagesSize["flash"] = flash_size;
	imagesSize["particles"] = particles_size;
	imagesSize["noise"] = noise_size;
	imagesSize["pedal_side"] = pedal_side_size;
	imagesSize["pedal_center"] = pedal_center_size;
	imagesSize["pedal_top"] = pedal_top_size;
	
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("font", imwidth, imheight);
		textureLibrary["font"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("flash", imwidth, imheight);
		textureLibrary["flash"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("noise", imwidth, imheight);
		textureLibrary["noise"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("particles", imwidth, imheight);
		textureLibrary["particles"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("pedal_side", imwidth, imheight);
		textureLibrary["pedal_side"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("pedal_center", imwidth, imheight);
		textureLibrary["pedal_center"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	{
		unsigned int imwidth;
		unsigned int imheight;
		auto texture = ResourcesManager::getDataForImage("pedal_top", imwidth, imheight);
		textureLibrary["pedal_top"] = loadTexture(texture, imwidth, imheight, 4, false);
	}
	
	{
		unsigned int imwidth1 = 4;
		unsigned int imheight1 = 4;
		unsigned char blankTexture[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
		textureLibrary["blank"] = loadTexture(blankTexture, imwidth1, imheight1, 4, false);
	}

	{
		unsigned int imwidth1 = 4;
		unsigned int imheight1 = 4;
		unsigned char blankTexture[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
		textureLibrary["blankarray"] = loadTextureArray({ blankTexture }, { glm::ivec2(imwidth1, imheight1) }, 1, false);
	}
}

GLuint ResourcesManager::getTextureFor(const std::string & fileName){
	if(textureLibrary.count(fileName) > 0){
		return textureLibrary[fileName];
	}
	
	std::cerr << "[WARNING]: Unable to find texture for name \"" << fileName << "\"." << std::endl;
	return 0;
}

glm::vec2 ResourcesManager::getTextureSizeFor(const std::string & fileName){
	if(imagesSize.count(fileName) > 0){
		return imagesSize[fileName];
	}
	std::cerr << "[WARNING]: Unable to find texture size for name \"" << fileName << "\"." << std::endl;
	return glm::vec2(0.0f,0.0f);
}

std::unordered_map<std::string, std::string> ResourcesManager::shadersLibrary = {};

std::unordered_map<std::string,  unsigned char*> ResourcesManager::imagesLibrary = {};

std::unordered_map<std::string, glm::vec2> ResourcesManager::imagesSize = {};

std::unordered_map<std::string, GLuint> ResourcesManager::textureLibrary = {};
