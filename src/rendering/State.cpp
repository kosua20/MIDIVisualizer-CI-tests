
#include "State.h"
#include "MIDIScene.h"
#include "../helpers/ResourcesManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

std::map<std::string, State::OptionInfos> State::_sharedInfos;

// Quality names.
const std::map<std::string, Quality::Level> Quality::names = {
	{ "LOW_RES", Quality::LOW_RES },
	{ "LOW", Quality::LOW },
	{ "MEDIUM", Quality::MEDIUM },
	{ "HIGH", Quality::HIGH },
	{ "HIGH_RES", Quality::HIGH_RES }
};

const std::map<Quality::Level, Quality> Quality::availables = {
	{ Quality::LOW_RES, { Quality::LOW_RES, 0.5f, 0.5f, 0.5f}},
	{ Quality::LOW, { Quality::LOW, 0.5f, 0.5f, 1.0f}},
	{ Quality::MEDIUM, { Quality::MEDIUM, 0.5f, 1.0f, 1.0f}},
	{ Quality::HIGH, { Quality::HIGH, 1.0f, 1.0f, 1.0f}},
	{ Quality::HIGH_RES, { Quality::HIGH_RES, 1.0f, 2.0f, 2.0f}}
};

Quality::Quality(const Quality::Level & alevel, const float partRes, const float blurRes, const float finRes){
	for(const auto & kv : names){
		if(kv.second == alevel){
			name = kv.first;
			break;
		}
	}
	particlesResolution = partRes; blurResolution = blurRes; finalResolution = finRes;
}

State::OptionInfos::OptionInfos(){
	description = "";
	type = Type::OTHER;
	range = {0.0f, 0.0f};
	values = "";
}

State::OptionInfos::OptionInfos(const std::string & adesc, State::OptionInfos::Type atype, const std::array<float, 2> & arange){
	description = adesc;
	type = atype;
	range = arange;
	// Generate value description based on type and range.
	switch(type){
		case Type::BOOLEAN:
			values = "1 or 0 to enable/disable";
			break;
		case Type::INTEGER:
			values = "integer";
			if(range[0] != range[1]){
				values.append(" in [" + std::to_string(int(range[0])) + "," + std::to_string(int(range[1])) + "]");
			}
			break;
		case Type::FLOAT:
			values = "number";
			if(range[0] != range[1]){
				values.append(" in [" + std::to_string(range[0]) + "," + std::to_string(range[1]) + "]");
			}
			break;
		case Type::COLOR:
			values = "R G B in [0.0, 1.0]";
			break;
		default:
			break;
	}
}

void State::defineOptions(){
	// Integers.
	_sharedInfos["particles-count"] = {"Particles count", OptionInfos::Type::INTEGER, {1.0f, 512.0f}};

	// Booleans.
	_sharedInfos["show-particles"] = {"Should particles be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-flashes"] = {"Should flashes be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-blur"] = {"Should the blur be visible", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-blur-notes"] = {"Should the notes be part of the blur", OptionInfos::Type::BOOLEAN};
	_sharedInfos["lock-colors"] = {"Should all effects use the same color", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-horiz-lines"] = {"Should horizontal score lines be showed", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-vert-lines"] = {"Should vertical score lines be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-numbers"] = {"Should measure numbers be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-keyboard"] = {"Should the keyboard be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["bg-img-behind-keyboard"] = {"Should the background image extend behind the keyboard", OptionInfos::Type::BOOLEAN};
	_sharedInfos["keyboard-highlight"] = {"Should the pressed keys be highlighted", OptionInfos::Type::BOOLEAN};
	_sharedInfos["keyboard-custom-colors"] = {"Override notes color for pressed keys", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-score"] = {"Should the score (lines, numbers) be shown", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-bg-img"] = {"Use a background texture", OptionInfos::Type::BOOLEAN};
	_sharedInfos["show-notes"] = {"Should the notes be shown", OptionInfos::Type::BOOLEAN};

	// Floats.
	_sharedInfos["time-scale"] = {"Vertical display scale", OptionInfos::Type::FLOAT};
	_sharedInfos["minor-size"] = {"Relative size of the minor keys and notes", OptionInfos::Type::FLOAT, {0.1f, 1.0f}};
	_sharedInfos["particles-size"] = {"Size of the particles", OptionInfos::Type::FLOAT, {1.0f, 10.0f}};
	_sharedInfos["particles-speed"] = {"Particles speed", OptionInfos::Type::FLOAT};
	_sharedInfos["particles-expansion"] = {"Particles expansion factor", OptionInfos::Type::FLOAT};
	_sharedInfos["blur-attenuation"] = {"Blur attenuation speed", OptionInfos::Type::FLOAT, {0.0f, 1.0f}};
	_sharedInfos["flashes-size"] = {"Flash effect size", OptionInfos::Type::FLOAT, {0.1f, 3.0f}};
	_sharedInfos["preroll"] = {"Preroll time in seconds before starting to play", OptionInfos::Type::FLOAT};
	_sharedInfos["bg-img-opacity"] = {"Background opacity", OptionInfos::Type::FLOAT, {0.0f, 1.0f}};

	// Colors.
	_sharedInfos["color-major"] = {"Major notes color", OptionInfos::Type::COLOR};
	_sharedInfos["color-bg"] = {"Background color", OptionInfos::Type::COLOR};
	_sharedInfos["color-particles"] = {"Particles color", OptionInfos::Type::COLOR};
	_sharedInfos["color-keyboard-major"] = {"Custom color for pressed major keys", OptionInfos::Type::COLOR};
	_sharedInfos["color-keyboard-minor"] = {"Custom color for pressed minor keys", OptionInfos::Type::COLOR};
	_sharedInfos["color-lines"] = {"Score lines color", OptionInfos::Type::COLOR};
	_sharedInfos["color-numbers"] = {"Score measure numbers color", OptionInfos::Type::COLOR};
	_sharedInfos["color-keyboard"] = {"Keyboard black keys color", OptionInfos::Type::COLOR};
	_sharedInfos["color-minor"] = {"Minor notes color", OptionInfos::Type::COLOR};
	_sharedInfos["color-flashes"] = {"Flash effect color", OptionInfos::Type::COLOR};

	// Others.
	_sharedInfos["quality"] = {"Rendering quality", OptionInfos::Type::OTHER};
	_sharedInfos["quality"].values = "values: LOW_RES, LOW, MEDIUM, HIGH, HIGH_RES";
	_sharedInfos["layers"] = {"Active layers indices, from background to foreground", OptionInfos::Type::OTHER};
	_sharedInfos["layers"].values = "values: bg-color: 0, bg-texture: 1, blur: 2, score: 3, keyboard: 4, particles: 5, notes: 6, flashes: 7";
}

std::string State::helpText(size_t & alignSize){
	if(_sharedInfos.empty()){
		defineOptions();
	}
	size_t maxLength = 0;
	for(const auto & param : _sharedInfos){
		maxLength = std::max(maxLength, param.first.size());
	}
	alignSize = maxLength + 4;

	std::stringstream msg;
	for(const auto & param : _sharedInfos){
		const std::string padString(alignSize - param.first.size(), ' ');
		msg << "--" << param.first << padString << param.second.description << " (" << param.second.values << ")" << std::endl;
	}
	return msg.str();
}

State::State(){
	reset();
	updateOptions();
}

void State::updateOptions(){
	if(_sharedInfos.empty()){
		defineOptions();
	}

	_intInfos["particles-count"] = &particles.count;
	_boolInfos["show-particles"] = &showParticles;
	_boolInfos["show-flashes"] = &showFlashes;
	_boolInfos["show-blur"] = &showBlur;
	_boolInfos["show-blur-notes"] = &showBlurNotes;
	_boolInfos["lock-colors"] = &lockParticleColor;
	_boolInfos["show-horiz-lines"] = &background.hLines;
	_boolInfos["show-vert-lines"] = &background.vLines;
	_boolInfos["show-numbers"] = &background.digits;
	_boolInfos["show-keyboard"] = &showKeyboard;
	_boolInfos["bg-img-behind-keyboard"] = &background.imageBehindKeyboard;
	_boolInfos["keyboard-highlight"] = &keyboard.highlightKeys;
	_boolInfos["keyboard-custom-colors"] = &keyboard.customKeyColors;
	_boolInfos["show-score"] = &showScore;
	_boolInfos["show-bg-img"] = &background.image;
	_boolInfos["show-notes"] = &showNotes;
	_floatInfos["time-scale"] = &scale;
	_floatInfos["minor-size"] = &background.minorsWidth;
	_floatInfos["particles-size"] = &particles.scale;
	_floatInfos["particles-speed"] = &particles.speed;
	_floatInfos["particles-expansion"] = &particles.expansion;
	_floatInfos["blur-attenuation"] = &attenuation;
	_floatInfos["flashes-size"] = &flashSize;
	_floatInfos["preroll"] = &prerollTime;
	_floatInfos["bg-img-opacity"] = &background.imageAlpha;
	_vecInfos["color-major"] = &baseColor;
	_vecInfos["color-bg"] = &background.color;
	_vecInfos["color-particles"] = &particles.color;
	_vecInfos["color-keyboard-major"] = &keyboard.majorColor;
	_vecInfos["color-keyboard-minor"] = &keyboard.minorColor;
	_vecInfos["color-lines"] = &background.linesColor;
	_vecInfos["color-numbers"] = &background.textColor;
	_vecInfos["color-keyboard"] = &background.keysColor;
	_vecInfos["color-minor"] = &minorColor;
	_vecInfos["color-flashes"] = &flashColor;
}


void State::save(const std::string & path){
	std::string outputPath = path;
	if (path.size() > 4 && path.substr(path.size() - 4, 4) != ".ini") {
		outputPath += ".ini";
	}
	std::ofstream configFile(outputPath);
	if(!configFile.is_open()){
		std::cerr << "Unable to save state to file at path " << outputPath << std::endl;
		return;
	}
	// Make sure the parameter pointers are up to date.
	updateOptions();

	configFile << "# MIDIVisualizer configuration file." << std::endl;
	configFile << "# Version (do not modify)" << std::endl;
	configFile << MIDIVIZ_VERSION_MAJOR << " " << MIDIVIZ_VERSION_MINOR << std::endl;

	// Write all options.
	for(const auto & param : _floatInfos){
		configFile << std::endl << "# " << _sharedInfos[param.first].description << " (";
		configFile << _sharedInfos[param.first].values << ")" << std::endl;
		configFile << param.first << ": " << *param.second << std::endl;
	}

	for(const auto & param : _intInfos){
		configFile << std::endl << "# " << _sharedInfos[param.first].description << " (";
		configFile << _sharedInfos[param.first].values << ")" << std::endl;
		configFile << param.first << ": " << *param.second << std::endl;
	}

	for(const auto & param : _boolInfos){
		configFile << std::endl << "# " << _sharedInfos[param.first].description << " (";
		configFile << _sharedInfos[param.first].values << ")" << std::endl;
		configFile << param.first << ": " << (*(param.second) ? 1 : 0) << std::endl;
	}

	for(const auto & param : _vecInfos){
		configFile << std::endl << "# " << _sharedInfos[param.first].description << " (";
		configFile << _sharedInfos[param.first].values << ")" << std::endl;
		const glm::vec3 & val = *param.second;
		configFile << param.first << ": " << val[0] << " " << val[1] << " " << val[2] << std::endl;
	}

	configFile << std::endl << "# " << _sharedInfos["quality"].description << " (";
	configFile << _sharedInfos["quality"].values << ")" << std::endl;
	configFile << "quality: " << Quality::availables.at(quality).name << std::endl;

	configFile << std::endl << "# " << _sharedInfos["layers"].description << " (";
	configFile << _sharedInfos["layers"].values << ")" << std::endl;
	configFile << "layers: ";
	for (int i = 0; i < layersMap.size(); ++i) {
		configFile << layersMap[i] << (i != (layersMap.size() - 1) ? " " : "");
	}
	configFile << std::endl;

	configFile.close();
}

void State::load(const std::string & path){
	std::ifstream configFileRaw(path);
	if(!configFileRaw.is_open()){
		std::cerr << "Unable to load state from file at path " << path << std::endl;
		return;
	}

	// Now that we support comments we need to be able to skip them without large code modifications below.
	// Do a first parse of the file, filtering comments, and build a new stream from it.
	// Not the most efficient thing, but we are talking about a config file of < 100 lines.
	std::stringstream configFile;
	std::string line;
	while(std::getline(configFileRaw, line)){
		line = trim(line, "\r\t ");
		if(line.empty() || line[0] == '#'){
			continue;
		}
		configFile << line << std::endl;
	}
	configFileRaw.close();

	// Check the version number.
	int majVersion = 0; int minVersion = 0;
	configFile >> majVersion >> minVersion;
	
	if(majVersion > MIDIVIZ_VERSION_MAJOR || (majVersion == MIDIVIZ_VERSION_MAJOR && minVersion > MIDIVIZ_VERSION_MINOR)){
		std::cout << "The config is more recent, some settings might be ignored." << std::endl;
	}
	if(majVersion < MIDIVIZ_VERSION_MAJOR || (majVersion == MIDIVIZ_VERSION_MAJOR && minVersion < MIDIVIZ_VERSION_MINOR)){
		std::cout << "The config is older, some newer settings will be left as-is." << std::endl;
	}

	// Two options: if we are < 5.0, we use the old positional format.
	// Else we use the key-value format, similar to command line.
	if(majVersion < 5){
		load(configFile, majVersion, minVersion);
	} else {
		// Build arguments list.
		const Arguments args = Configuration::parseArguments(configFile);
		load(args);
	}

}

void State::load(const Arguments & configArgs){
	// Make sure the parameter pointers are up to date.
	updateOptions();

	for(const auto & arg : configArgs){
		const auto & key = arg.first;
		if(arg.second.empty()){
			std::cerr << "Missing values for key " << key << "." << std::endl;
			continue;
		}

		if(key == "quality"){
			const std::string & qualityName = arg.second[0];
			if(Quality::names.count(qualityName) > 0){
				quality = Quality::names.at(qualityName);
			}
			continue;
		}

		if(key == "layers"){
			const size_t bound = std::min(layersMap.size(), arg.second.size());
			for(size_t id = 0; id < bound; ++id){
				layersMap[id] = Configuration::parseInt(arg.second[id]);
			}
			continue;
		}

		const auto itf = _floatInfos.find(key);
		if(itf != _floatInfos.end()){
			const auto & opt = itf->second;
			*opt = Configuration::parseFloat(arg.second[0]);
			const auto & range = _sharedInfos.at(key).range;
			if(range[0] != range[1]){
				*opt = glm::clamp(*opt, range[0], range[1]);
			}
			continue;
		}

		const auto iti = _intInfos.find(key);
		if(iti != _intInfos.end()){
			const auto & opt = iti->second;
			*opt = Configuration::parseInt(arg.second[0]);
			const auto & range = _sharedInfos.at(key).range;
			if(range[0] != range[1]){
				*opt = glm::clamp(*opt, int(range[0]), int(range[1]));
			}
			continue;
		}

		const auto itb = _boolInfos.find(key);
		if(itb != _boolInfos.end()){
			const auto & opt = itb->second;
			*opt = Configuration::parseBool(arg.second[0]);
			continue;
		}

		const auto itv = _vecInfos.find(key);
		if(itv != _vecInfos.end()){
			const auto & opt = itv->second;
			*opt = Configuration::parseVec3(arg.second);
			continue;
		}
	}
}

void State::reset(){
	baseColor = 1.35f*glm::vec3(0.57f,0.19f,0.98f);
	minorColor = 0.8f*baseColor;
	flashColor = baseColor;
	background.color = glm::vec3(0.0f, 0.0f, 0.0f) ;
	background.linesColor = glm::vec3(1.0f, 1.0f, 1.0f);
	background.textColor = glm::vec3(1.0f, 1.0f, 1.0f);
	background.keysColor = glm::vec3(0.0f, 0.0f, 0.0f);
	particles.color = baseColor;
	
	scale = 0.5f ;
	attenuation = 0.99f;
	showParticles = true ;
	showFlashes = true ;
	showBlur = true ;
	showBlurNotes = false ;
	lockParticleColor = true ;
	showNotes = true;
	showScore = true;
	showKeyboard = true;
	flashSize = 1.0f;
	
	background.minorsWidth = 0.8f;
	background.hLines = true;
	background.vLines = true ;
	background.digits = true ;
	background.image = false;
	background.imageAlpha = 1.0f;
	background.tex = 0;
	background.imageBehindKeyboard = false;

	particles.speed = 0.2f;
	particles.expansion = 1.0f;
	particles.scale = 1.0f;
	particles.count = 256;
	const GLuint blankID = ResourcesManager::getTextureFor("blankarray");
	particles.tex = blankID;
	particles.texCount = 1;
	
	quality = Quality::MEDIUM;
	prerollTime = 1.0f;
	keyboard.highlightKeys = true;
	keyboard.customKeyColors = false;
	keyboard.majorColor = baseColor;
	keyboard.minorColor = minorColor;

	for (int i = 0; i < layersMap.size(); ++i) {
		layersMap[i] = i;
	}
}

void State::load(std::istream & configFile, int majVersion, int minVersion){

	// MIDIVIZ_VERSION_MAJOR == 3, MIDIVIZ_VERSION_MINOR == 0
	// This part is always valid, as it was present when the saving system was introduced.
	// Note: we don't restore the texture IDs and scale.
	{
		configFile >> baseColor[0] >> baseColor[1] >> baseColor[2] ;
		configFile >> background.color[0] >> background.color[1] >> background.color[2] ;
		configFile >> particles.color[0] >> particles.color[1] >> particles.color[2] ;

		configFile >> scale ;
		configFile >> showParticles ;
		configFile >> showFlashes ;
		configFile >> showBlur ;
		configFile >> showBlurNotes ;
		configFile >> lockParticleColor ;

		configFile >> background.minorsWidth ;
		configFile >> background.hLines ;
		configFile >> background.vLines ;
		configFile >> background.digits ;
		configFile >> showKeyboard ;

		configFile >> particles.speed ;
		configFile >> particles.expansion ;
		configFile >> particles.count ;
	}

	// MIDIVIZ_VERSION_MAJOR == 3, MIDIVIZ_VERSION_MINOR == 1
	// Added quality level.
	if(majVersion > 3 || (majVersion == 3 && minVersion >= 1)){
		std::string qualityName;;
		configFile >> qualityName;
		if(Quality::names.count(qualityName) > 0){
			quality = Quality::names.at(qualityName);
		}
	}

	// MIDIVIZ_VERSION_MAJOR == 3, MIDIVIZ_VERSION_MINOR == 2
	// Added attenuation factor.
	if(majVersion > 3 || (majVersion == 3 && minVersion >= 2)){
		configFile >> attenuation ;
	}

	// MIDIVIZ_VERSION_MAJOR == 3, MIDIVIZ_VERSION_MINOR == 3
	/* minorColor = 0.8f*baseColor;
	flashColor = baseColor; */
	if(majVersion > 3 || (majVersion == 3 && minVersion >= 3)){
		configFile >> showNotes;
		configFile >> background.linesColor[0] >> background.linesColor[1] >> background.linesColor[2] ;
		configFile >> background.textColor[0] >> background.textColor[1] >> background.textColor[2] ;
		configFile >> background.keysColor[0] >> background.keysColor[1] >> background.keysColor[2] ;
		configFile >> minorColor[0] >> minorColor[1] >> minorColor[2] ;
		configFile >> flashColor[0] >> flashColor[1] >> flashColor[2] ;
		configFile >> flashSize;
	}

	// MIDIVIZ_VERSION_MAJOR == 3, MIDIVIZ_VERSION_MINOR == 5
	if (majVersion > 3 || (majVersion == 3 && minVersion >= 5)) {
		configFile >> prerollTime;
		configFile >> showScore;
		std::string layersList;
		// Get the next line.
		// Skip newline.
		std::getline(configFile, layersList);
		std::getline(configFile, layersList);
		std::stringstream sstr(layersList);
		int id = 0;
		while (!sstr.eof() && id < layersMap.size()) {
			sstr >> layersMap[id];
			++id;
		}
	}

	// MIDIVIZ_VERSION_MAJOR == 4, MIDIVIZ_VERSION_MINOR == 0
	if (majVersion > 4 || (majVersion == 4 && minVersion >= 0)) {
		configFile >> background.image;
		configFile >> background.imageAlpha;
		configFile >> background.imageBehindKeyboard;
		configFile >> keyboard.highlightKeys;
	}

	// MIDIVIZ_VERSION_MAJOR == 4, MIDIVIZ_VERSION_MINOR == 1
	if (majVersion > 4 || (majVersion == 4 && minVersion >= 1)) {
		configFile >> keyboard.customKeyColors;
		configFile >> keyboard.majorColor[0] >> keyboard.majorColor[1] >> keyboard.majorColor[2];
		configFile >> keyboard.minorColor[0] >> keyboard.minorColor[1] >> keyboard.minorColor[2];
	}
}
