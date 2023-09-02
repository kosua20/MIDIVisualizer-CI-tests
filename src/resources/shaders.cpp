#include "data.h"
const std::unordered_map<std::string, std::string> shaders = {
{ "flashes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in int onChan;\n uniform float time;\n uniform vec2 inverseScreenSize;\n uniform float userScale;\n uniform float keyboardHeight;\n uniform int minNote;\n uniform float notesCount;\n uniform bool horizontalMode = false;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n const float shifts[128] = float[](\n 	0,0.5,1,1.5,2,3,3.5,4,4.5,5,5.5,6,7,7.5,8,8.5,9,10,10.5,11,11.5,12,12.5,13,14,14.5,15,15.5,16,17,17.5,18,18.5,19,19.5,20,21,21.5,22,22.5,23,24,24.5,25,25.5,26,26.5,27,28,28.5,29,29.5,30,31,31.5,32,32.5,33,33.5,34,35,35.5,36,36.5,37,38,38.5,39,39.5,40,40.5,41,42,42.5,43,43.5,44,45,45.5,46,46.5,47,47.5,48,49,49.5,50,50.5,51,52,52.5,53,53.5,54,54.5,55,56,56.5,57,57.5,58,59,59.5,60,60.5,61,61.5,62,63,63.5,64,64.5,65,66,66.5,67,67.5,68,68.5,69,70,70.5,71,71.5,72,73,73.5,74\n );\n const vec2 scale = 0.9*vec2(3.5,3.0);\n out INTERFACE {\n 	vec2 uv;\n 	float onChannel;\n 	float id;\n } Out;\n void main(){\n 	\n 	// Scale quad, keep the square ratio.\n 	float screenRatio = inverseScreenSize.y/inverseScreenSize.x;\n 	vec2 scalingFactor = vec2(1.0, horizontalMode ? (1.0/screenRatio) : screenRatio);\n 	vec2 scaledPosition = v * 2.0 * scale * userScale/notesCount * scalingFactor;\n 	// Shift based on note/flash id.\n 	vec2 globalShift = vec2(-1.0 + ((shifts[gl_InstanceID] - shifts[minNote]) * 2.0 + 1.0) / notesCount, 2.0 * keyboardHeight - 1.0);\n 	\n 	gl_Position = vec4(flipIfNeeded(scaledPosition + globalShift), 0.0 , 1.0) ;\n 	\n 	// Pass infos to the fragment shader.\n 	Out.uv = v;\n 	Out.onChannel = float(onChan);\n 	Out.id = float(gl_InstanceID);\n 	\n }\n "}, 
{ "flashes_frag", "#version 330\n #define SETS_COUNT 12\n in INTERFACE {\n 	vec2 uv;\n 	float onChannel;\n 	float id;\n } In;\n uniform sampler2D textureFlash;\n uniform float time;\n uniform vec3 baseColor[SETS_COUNT];\n uniform float haloIntensity;\n uniform float haloInnerRadius;\n uniform float haloOuterRadius;\n uniform int texRowCount;\n uniform int texColCount;\n out vec4 fragColor;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	\n 	// If not on, discard flash immediatly.\n 	int cid = int(In.onChannel);\n 	float mask = 0.0;\n 	const float atlasSpeed = 15.0;\n 	const float safetyMargin = 0.05;\n 	// If up half, read from texture atlas.\n 	if(In.uv.y > 0.0){\n 		// Select a sprite, depending on time and flash id.\n 		int atlasShift = int(floor(mod(atlasSpeed * time, texRowCount*texColCount)) + floor(rand(In.id * vec2(time,1.0))));\n 		ivec2 atlasIndex = ivec2(atlasShift % texColCount, atlasShift / texColCount);\n 		// Scale UV to fit in one sprite from atlas.\n 		vec2 localUV = clamp(In.uv * vec2(1.0, 2.0) + vec2(0.5, 0.0), safetyMargin, 1.0-safetyMargin);\n 		\n 		// Read in black and white texture do determine opacity (mask).\n 		vec2 finalUV = (vec2(atlasIndex) + localUV)/vec2(texColCount, texRowCount);\n 		mask = texture(textureFlash,finalUV).r;\n 	}\n 	if(cid < 0){\n 		discard;\n 	}\n 	\n 	// Colored sprite.\n 	vec4 spriteColor = vec4(baseColor[cid], mask);\n 	\n 	// Circular halo effect.\n 	float haloAlpha = 1.0 - smoothstep(haloInnerRadius, haloOuterRadius, length(In.uv) / 0.5);\n 	vec4 haloColor;\n 	haloColor.rgb = baseColor[cid] + haloIntensity * vec3(1.0);\n 	haloColor.a = haloAlpha * 0.92;\n 	\n 	// Mix the sprite color and the halo effect.\n 	fragColor = mix(spriteColor, haloColor, haloColor.a);\n 	\n 	// Boost intensity.\n 	fragColor *= 1.1;\n 	// Premultiplied alpha.\n 	fragColor.rgb *= fragColor.a;\n }\n "},
{ "notes_vert", "#version 330\n layout(location = 0) in vec2 v;\n layout(location = 1) in vec4 id; //note id, start, duration, is minor\n layout(location = 2) in float channel; //note id, start, duration, is minor\n uniform float time;\n uniform float mainSpeed;\n uniform float minorsWidth = 1.0;\n uniform float keyboardHeight = 0.25;\n uniform bool reverseMode;\n uniform bool horizontalMode;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n uniform int minNoteMajor;\n uniform float notesCount;\n out INTERFACE {\n 	vec2 uv;\n 	vec2 noteSize;\n 	flat vec2 noteCorner;\n 	float isMinor;\n 	float channel;\n } Out;\n #define MAJOR_COUNT 75\n const int minorIds[MAJOR_COUNT] = int[](1, 3, 0, 6, 8, 10, 0, 13, 15, 0, 18, 20, 22, 0, 25, 27, 0, 30, 32, 34, 0, 37, 39, 0, 42, 44, 46, 0, 49, 51, 0, 54, 56, 58, 0, 61, 63, 0, 66, 68, 70, 0, 73, 75, 0, 78, 80, 82, 0, 85, 87, 0, 90, 92, 94, 0, 97, 99, 0, 102, 104, 106, 0, 109, 111, 0, 114, 116, 118, 0, 121, 123, 0, 126, 0);\n float minorShift(int id){\n    if(id == 1 || id == 6){\n 	   return -0.1;\n    }\n    if(id == 3 || id == 10){\n 	   return 0.1;\n    }\n    return 0.0;\n }\n void main(){\n 	\n 	float scalingFactor = id.w != 0.0 ? minorsWidth : 1.0;\n 	// Size of the note : width, height based on duration and current speed.\n 	Out.noteSize = vec2(0.9*2.0/notesCount * scalingFactor, id.z*mainSpeed);\n 	\n 	// Compute note shift.\n 	// Horizontal shift based on note id, width of keyboard, and if the note is minor or not.\n 	// Vertical shift based on note start time, current time, speed, and height of the note quad.\n 	//float a = (1.0/(notesCount-1.0)) * (2.0 - 2.0/notesCount);\n 	//float b = -1.0 + 1.0/notesCount;\n 	// This should be in -1.0, 1.0.\n 	// input: id.x is in [0 MAJOR_COUNT]\n 	// we want minNote to -1+1/c, maxNote to 1-1/c\n 	float a = 2.0;\n 	float b = -notesCount + 1.0 - 2.0 * float(minNoteMajor);\n 	float horizLoc = (id.x * a + b + id.w) / notesCount;\n 	float vertLoc = 2.0 * keyboardHeight - 1.0;\n 	vertLoc += (reverseMode ? -1.0 : 1.0) * (Out.noteSize.y * 0.5 + mainSpeed * (id.y - time));\n 	vec2 noteShift = vec2(horizLoc, vertLoc);\n 	noteShift.x += id.w * minorShift(minorIds[int(id.x)] % 12) * Out.noteSize.x;\n 	// Scale uv.\n 	Out.uv = Out.noteSize * v;\n 	Out.isMinor = id.w;\n 	Out.channel = channel;\n 	// Output position.\n 	gl_Position = vec4(flipIfNeeded(Out.noteSize * v + noteShift), 0.0 , 1.0);\n 	// Offset of bottom left corner.\n 	Out.noteCorner = flipIfNeeded(Out.noteSize * vec2(-0.5, reverseMode ? 0.5 : -0.5) + noteShift);\n }\n "}, 
{ "notes_frag", "#version 330\n #define SETS_COUNT 12\n in INTERFACE {\n 	vec2 uv;\n 	vec2 noteSize;\n 	flat vec2 noteCorner;\n 	float isMinor;\n 	float channel;\n } In;\n uniform vec3 baseColor[SETS_COUNT];\n uniform vec3 minorColor[SETS_COUNT];\n uniform vec2 inverseScreenSize;\n uniform float time;\n uniform float mainSpeed;\n uniform float colorScale;\n uniform float keyboardHeight = 0.25;\n uniform float fadeOut;\n uniform float edgeWidth;\n uniform float edgeBrightness;\n uniform float cornerRadius;\n uniform bool horizontalMode;\n uniform bool reverseMode;\n uniform sampler2D majorTexture;\n uniform sampler2D minorTexture;\n uniform bool useMajorTexture;\n uniform bool useMinorTexture;\n uniform vec2 texturesScale;\n uniform vec2 texturesStrength;\n uniform bool scrollMajorTexture;\n uniform bool scrollMinorTexture;\n out vec4 fragColor;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n void main(){\n 	\n 	vec2 normalizedCoord = vec2(gl_FragCoord.xy) * inverseScreenSize;\n 	vec3 tinting = vec3(1.0);\n 	vec2 tintingUV = 2.0 * normalizedCoord - 1.0;\n 	// Preserve screen pixel density, corrected for aspect ratio (on X so that preserving scrolling speed is easier).\n 	vec2 aspectRatio = vec2(inverseScreenSize.y / inverseScreenSize.x, 1.0);\n 	vec2 tintingScale = aspectRatio;\n 	tintingScale.x *= (horizontalMode && !reverseMode ? -1.0 : 1.0);\n 	tintingScale.y *= (!horizontalMode && reverseMode ? -1.0 : 1.0);\n 	if(useMajorTexture){\n 		vec2 texUVOffset = scrollMajorTexture ? In.noteCorner : vec2(0.0);\n 		vec2 texUV = texturesScale.x * tintingScale * (tintingUV - texUVOffset);\n 		texUV = flipIfNeeded(texUV);\n 		// Only on major notes.\n 		float intensity = (1.0 - In.isMinor) * texturesStrength.x;\n 		tinting = mix(tinting, texture(majorTexture, texUV).rgb, intensity);\n 	}\n 	if(useMinorTexture){\n 		vec2 texUVOffset = scrollMinorTexture ? In.noteCorner : vec2(0.0);\n 		vec2 texUV = texturesScale.y * tintingScale * (tintingUV - texUVOffset);\n 		texUV = flipIfNeeded(texUV);\n 		// Only on minor notes.\n 		float intensity = In.isMinor * texturesStrength.y;\n 		tinting = mix(tinting, texture(minorTexture, texUV).rgb, intensity);\n 	}\n 	\n 	\n 	// Rounded corner (super-ellipse equation).\n 	vec2 ellipseCoords = abs(In.uv / (0.5 * In.noteSize));\n 	vec2 ellipseExps = In.noteSize / max(cornerRadius, 1e-3);\n 	float radiusPosition = pow(ellipseCoords.x, ellipseExps.x) + pow(ellipseCoords.y, ellipseExps.y);\n 	// Fragment color.\n 	int cid = int(In.channel);\n 	fragColor.rgb = tinting * colorScale * mix(baseColor[cid], minorColor[cid], In.isMinor);\n 	// Apply scaling factor to edge.\n 	float deltaPix = fwidth(In.uv.x) * 4.0;\n 	float edgeIntensity = smoothstep(1.0 - edgeWidth - deltaPix, 1.0 - edgeWidth + deltaPix, radiusPosition);\n 	fragColor.rgb *= (1.0f + (edgeBrightness - 1.0f) * edgeIntensity);\n 	// If lower area of the screen, discard fragment as it should be hidden behind the keyboard.\n 	if((horizontalMode ? normalizedCoord.x : normalizedCoord.y) < keyboardHeight){\n 		discard;\n 	}\n 	if(	radiusPosition > 1.0){\n 		discard;\n 	}\n 	\n 	float distFromBottom = horizontalMode ? normalizedCoord.x : normalizedCoord.y;\n 	float fadeOutFinal = min(fadeOut, 0.9999);\n 	distFromBottom = max(distFromBottom - fadeOutFinal, 0.0) / (1.0 - fadeOutFinal);\n 	float alpha = 1.0 - distFromBottom;\n 	fragColor.a = alpha;\n }\n "},
{ "particles_vert", "#version 330\n #define SETS_COUNT 12\n layout(location = 0) in vec2 v;\n uniform float time;\n uniform float scale;\n uniform vec3 baseColor[SETS_COUNT];\n uniform vec2 inverseScreenSize;\n uniform sampler2D textureParticles;\n uniform vec2 inverseTextureSize;\n uniform sampler2D textureNoise;\n uniform int globalId;\n uniform float duration;\n uniform int channel;\n uniform int texCount;\n uniform float colorScale;\n uniform float expansionFactor = 1.0;\n uniform float speedScaling = 0.2;\n uniform float keyboardHeight = 0.25;\n uniform float turbulenceStrength;\n uniform float turbulenceScale;\n uniform int minNote;\n uniform float notesCount;\n uniform bool horizontalMode = false;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n const float shifts[128] = float[](\n 0,0.5,1,1.5,2,3,3.5,4,4.5,5,5.5,6,7,7.5,8,8.5,9,10,10.5,11,11.5,12,12.5,13,14,14.5,15,15.5,16,17,17.5,18,18.5,19,19.5,20,21,21.5,22,22.5,23,24,24.5,25,25.5,26,26.5,27,28,28.5,29,29.5,30,31,31.5,32,32.5,33,33.5,34,35,35.5,36,36.5,37,38,38.5,39,39.5,40,40.5,41,42,42.5,43,43.5,44,45,45.5,46,46.5,47,47.5,48,49,49.5,50,50.5,51,52,52.5,53,53.5,54,54.5,55,56,56.5,57,57.5,58,59,59.5,60,60.5,61,61.5,62,63,63.5,64,64.5,65,66,66.5,67,67.5,68,68.5,69,70,70.5,71,71.5,72,73,73.5,74\n );\n out INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } Out;\n float rand(vec2 co){\n 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n }\n void main(){\n 	Out.id = float(gl_InstanceID % texCount);\n 	Out.uv = v + 0.5;\n 	// Fade color based on time.\n 	Out.color = vec4(colorScale * baseColor[channel], 1.0-time*time);\n 	\n 	float localTime = speedScaling * time * duration;\n 	float particlesCount = 1.0/inverseTextureSize.y;\n 	\n 	// Pick particle id at random.\n 	float particleId = float(gl_InstanceID) + floor(particlesCount * 10.0 * rand(vec2(globalId,globalId)));\n 	float textureId = mod(particleId,particlesCount);\n 	float particleShift = floor(particleId/particlesCount);\n 	\n 	// Particle uv, in pixels.\n 	vec2 particleUV = vec2(localTime / inverseTextureSize.x + 10.0 * particleShift, textureId);\n 	// UV in [0,1]\n 	particleUV = (particleUV+0.5)*vec2(1.0,-1.0)*inverseTextureSize;\n 	// Avoid wrapping.\n 	particleUV.x = clamp(particleUV.x,0.0,1.0);\n 	// We want to skip reading from the very beginning of the trajectories because they are identical.\n 	// particleUV.x = 0.95 * particleUV.x + 0.05;\n 	// Read corresponding trajectory to get particle current position.\n 	vec3 position = texture(textureParticles, particleUV).xyz;\n 	// Center position (from [0,1] to [-0.5,0.5] on x axis.\n 	position.x -= 0.5;\n 	\n 	// Compute shift, randomly disturb it.\n 	vec2 shift = 0.5*position.xy;\n 	float random = rand(vec2(particleId + float(globalId),time*0.000002+100.0*float(globalId)));\n 	shift += vec2(0.0,0.1*random);\n 	\n 	// Scale shift with time (expansion effect).\n 	shift = shift*time*expansionFactor;\n 	// and with altitude of the particle (ditto).\n 	shift.x *= max(0.5, pow(shift.y,0.3));\n 	\n 	// Horizontal shift is based on the note ID.\n 	float xshift = -1.0 + ((shifts[globalId] - shifts[int(minNote)]) * 2.0 + 1.0) / notesCount;\n 	//  Combine global shift (due to note id) and local shift (based on read position).\n 	vec2 globalShift = vec2(xshift, (2.0 * keyboardHeight - 1.0)-0.02);\n 	vec2 localShift = shift * duration * vec2(1.0,0.5);\n 	vec2 vertexShift = 0.003 * scale * v;\n 	float screenRatio = inverseScreenSize.y/inverseScreenSize.x;\n 	vec2 screenScaling = vec2(1.0, horizontalMode ? (1.0/screenRatio) : screenRatio);\n 	vec2 particlePos = globalShift + screenScaling * localShift;\n 	vec2 curlNoise = textureLod(textureNoise, turbulenceScale * particlePos.xy / screenScaling, 0).gb;\n 	curlNoise.x = 2.0 * curlNoise.x - 1.0;\n 	vec2 curlShift = 0.01 * turbulenceStrength * time * curlNoise;\n 	vec2 finalPos = particlePos + screenScaling * (vertexShift + curlShift);\n 	// Discard particles that reached the end of their trajectories by putting them off-screen.\n 	finalPos = mix(vec2(-200.0),finalPos, position.z);\n 	// Output final particle position.\n 	gl_Position = vec4(flipIfNeeded(finalPos), 0.0, 1.0);\n 	\n 	\n }\n "}, 
{ "particles_frag", "#version 330\n in INTERFACE {\n 	vec4 color;\n 	vec2 uv;\n 	float id;\n } In;\n uniform sampler2DArray lookParticles;\n out vec4 fragColor;\n void main(){\n 	float alpha = texture(lookParticles, vec3(In.uv, In.id)).r;\n 	fragColor = In.color;\n 	fragColor.a *= alpha;\n }\n "},
{ "particlesblur_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "particlesblur_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n uniform vec3 backgroundColor = vec3(0.0);\n uniform float attenuationFactor = 0.99;\n uniform float time;\n out vec4 fragColor;\n vec4 blur(vec2 uv, bool vert){\n 	vec4 color = 0.2270270270 * texture(screenTexture, uv);\n 	vec2 pixelOffset = vert ? vec2(0.0, inverseScreenSize.y) : vec2(inverseScreenSize.x, 0.0);\n 	vec2 texCoordOffset0 = 1.3846153846 * pixelOffset;\n 	vec4 col0 = texture(screenTexture, uv + texCoordOffset0) + texture(screenTexture, uv - texCoordOffset0);\n 	color += 0.3162162162 * col0;\n 	vec2 texCoordOffset1 = 3.2307692308 * pixelOffset;\n 	vec4 col1 = texture(screenTexture, uv + texCoordOffset1) + texture(screenTexture, uv - texCoordOffset1);\n 	color += 0.0702702703 * col1;\n 	return color;\n }\n void main(){\n 	\n 	// Gaussian blur separated in two 1D convolutions, relying on bilinear interpolation to\n 	// sample multiple pixels at once with the proper weights.\n 	vec4 color = blur(In.uv, time > 0.5);\n 	// Include decay for fade out.\n 	fragColor = mix(vec4(backgroundColor, 0.0), color, attenuationFactor);\n 	\n }\n "},
{ "screenquad_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "screenquad_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n out vec4 fragColor;\n void main(){\n 	\n 	fragColor = texture(screenTexture,In.uv);\n 	\n }\n "},
{ "majorKeys_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n uniform bool horizontalMode = false;\n uniform float keyboardHeight = 0.25;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n void main(){\n 	// Input are in -0.5,0.5\n 	// We directly output the position.\n 	// [-0.5, 0.5] to [-1, -1+2.0*keyboardHeight]\n 	float yShift = keyboardHeight * (2.0 * v.y + 1.0) - 1.0;\n 	vec2 pos2D = vec2(v.x * 2.0, yShift);\n 	gl_Position.xy = flipIfNeeded(pos2D);\n 	gl_Position.zw = vec2(0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy + 0.5;\n 	\n }\n "}, 
{ "majorKeys_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n #define SETS_COUNT 12\n #define MAJOR_COUNT 75\n uniform bool highlightKeys;\n uniform bool horizontalMode = false;\n uniform float notesCount; // (maxNoteMajor - minNoteMajor + 1)\n uniform int actives[128];\n uniform int minNoteMajor;\n uniform vec2 inverseScreenSize;\n uniform vec3 edgeColor = vec3(0.0);\n uniform vec3 majorColor[SETS_COUNT];\n const int majorIds[MAJOR_COUNT] = int[](0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31, 33, 35, 36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84, 86, 88, 89, 91, 93, 95, 96, 98, 100, 101, 103, 105, 107, 108, 110, 112, 113, 115, 117, 119, 120, 122, 124, 125, 127);\n out vec4 fragColor;\n void main(){\n 	// White keys, and separators.\n 	float widthScaling = horizontalMode ? inverseScreenSize.y : inverseScreenSize.x;\n 	float majorUvX = fract(In.uv.x * notesCount);\n 	// Edges on the sides\n 	float centerIntensity = 1.0 - step( 1.0 - 2.0 * notesCount * widthScaling, abs(majorUvX * 2.0 - 1.0));\n 	// If the current major key is active, the majorColor is specific.\n 	int majorId = majorIds[clamp(int(In.uv.x * notesCount) + minNoteMajor, 0, 74)];\n 	int cidMajor = actives[majorId];\n 	vec3 frontColor = (highlightKeys && cidMajor >= 0) ? majorColor[cidMajor] : vec3(1.0);\n 	// Mix.\n 	fragColor.rgb = mix(edgeColor, frontColor, centerIntensity);\n 	fragColor.a = 1.0;\n }\n "},
{ "minorKeys_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n 	float id;\n } Out ;\n uniform bool horizontalMode = false;\n uniform float keyboardHeight = 0.25;\n uniform float notesCount; // (maxNoteMajor - minNoteMajor + 1)\n uniform int actives[128];\n uniform int minNoteMajor;\n uniform float minorsHeight = 0.6;\n uniform float minorsWidth = 1.0;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n #define MINOR_COUNT 53\n const int minorIds[MINOR_COUNT] = int[](1, 3, 6, 8, 10, 13, 15, 18, 20, 22, 25, 27, 30, 32, 34, 37, 39, 42, 44, 46, 49, 51, 54, 56, 58, 61, 63, 66, 68, 70, 73, 75, 78, 80, 82, 85, 87, 90, 92, 94,  97, 99,  102, 104, 106, 109, 111, 114, 116, 118, 121, 123, 126);\n const int noteDelta[12] = int[](0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6);\n float minorShift(int id){\n    if(id == 1 || id == 6){\n 	   return -0.1;\n    }\n    if(id == 3 || id == 10){\n 	   return 0.1;\n    }\n    return 0.0;\n }\n void main(){\n 	float noteWidth = 2.0 / notesCount;\n 	// Size of the note : width, height based on duration and current speed.\n 	vec2 noteSize = vec2(0.9 * noteWidth * minorsWidth, 2.0 * minorsHeight * keyboardHeight);\n 	// Compute note shift.\n 	// Horizontal shift based on note id, width of keyboard, and if the note is minor or not.\n 	//float a = (1.0/(notesCount-1.0)) * (2.0 - 2.0/notesCount);\n 	//float b = -1.0 + 1.0/notesCount;\n 	// This should be in -1.0, 1.0.\n 	// input: noteId is in [0 MAJOR_COUNT]\n 	// we want minNote to -1+1/c, maxNote to 1-1/c\n 	float a = 2.0;\n 	float b = -notesCount + 1.0 - 2.0 * float(minNoteMajor);\n 	int minorId = minorIds[gl_InstanceID];\n 	int noteId = (minorId/12) * 7 + noteDelta[minorId % 12];\n 	float horizLoc = (float(noteId) * a + b + 1.0) / notesCount;\n 	float vertLoc = 2.0 * (1.0 - minorsHeight) * keyboardHeight - 1.0 + noteSize.y * 0.5;\n 	vec2 noteShift = vec2(horizLoc, vertLoc);\n 	noteShift.x += minorShift(minorId % 12) * noteSize.x;\n 	// Output position.\n 	gl_Position = vec4(flipIfNeeded(noteSize * v + noteShift), 0.0 , 1.0) ;\n 	// Discard keys that are too close to the screen edges.\n 	if(abs(noteShift.x) >= 1.0 - 0.5 * noteWidth){\n 		gl_Position = vec4(-40000.0);\n 	}\n 	\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy + 0.5;\n 	// And the active channel if present.\n 	Out.id = float(actives[minorId]);\n 	\n }\n "}, 
{ "minorKeys_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n 	float id;\n } In ;\n #define SETS_COUNT 12\n uniform bool highlightKeys;\n uniform bool horizontalMode = false;\n uniform float keyboardHeight = 0.25;\n uniform float notesCount; // (maxNoteMajor - minNoteMajor + 1)\n uniform int minNoteMajor;\n uniform vec2 inverseScreenSize;\n uniform vec3 edgeColor = vec3(0.0);\n uniform vec3 minorColor[SETS_COUNT];\n uniform bool edgeOnMinors;\n uniform float minorsHeight = 0.6;\n uniform float minorsWidth = 1.0;\n vec2 minorShift(int id){\n 	if(id == 1 || id == 6){\n 		return vec2(0.0, 0.3);\n 	}\n 	if(id == 3 || id == 10){\n 		return vec2(0.3, 0.0);\n 	}\n 	return vec2(0.1,0.1);\n }\n out vec4 fragColor;\n void main(){\n 	// Center uvs\n 	vec2 ndc = 2.0 * In.uv - 1.0;\n 	float widthScaling = horizontalMode ? inverseScreenSize.y : inverseScreenSize.x;\n 	float heightScaling = horizontalMode ? inverseScreenSize.x : inverseScreenSize.y;\n 	float uvWidth = minorsWidth / float(notesCount) * 0.5;\n 	float uvHeight = minorsHeight * keyboardHeight * 0.5;\n 	// Edges on the sides and bottom.\n 	float xStep = step(1.0 - 2.0 * widthScaling / uvWidth, abs(ndc.x));\n 	float yStep = step(1.0 - 2.0 * heightScaling / uvHeight, -ndc.y);\n 	float centerIntensity = edgeOnMinors ? ((1.0 - xStep) * (1.0 - yStep)) : 1.0;\n 	// Key color changes when active.\n 	int cidMinor = int(In.id);\n 	vec3 frontColor = (highlightKeys && cidMinor >= 0) ? minorColor[cidMinor] : edgeColor;\n 	// Mix\n 	fragColor.rgb = mix(edgeColor, frontColor, centerIntensity);\n 	fragColor.a = 1.0;\n }\n "},
{ "backgroundtexture_vert", "#version 330\n layout(location = 0) in vec2 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n uniform bool behindKeyboard;\n uniform float keyboardHeight = 0.25;\n void main(){\n 	vec2 pos = v;\n 	if(!behindKeyboard){\n 		pos.y = (1.0-keyboardHeight) * pos.y + keyboardHeight;\n 	}\n 	// We directly output the position.\n 	gl_Position = vec4(pos, 0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "backgroundtexture_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform float textureAlpha;\n uniform bool behindKeyboard;\n out vec4 fragColor;\n void main(){\n 	fragColor = texture(screenTexture, In.uv);\n 	fragColor.a *= textureAlpha;\n }\n "},
{ "pedal_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform vec2 shift;\n uniform vec2 scale;\n uniform bool mirror;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	// Translate to put on top of the keyboard.\n 	gl_Position = vec4(v.xy * scale + shift, 0.5, 1.0);\n 	Out.uv = v.xy + 0.5;\n 	if(mirror){\n 		Out.uv.x = 1.0 - Out.uv.x;\n 	}\n }\n "}, 
{ "pedal_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform vec2 inverseScreenSize;\n uniform vec3 pedalColor;\n uniform float pedalOpacity;\n uniform sampler2D pedalTexture;\n uniform float pedalFlag;\n out vec4 fragColor;\n void main(){\n 	vec2 uv = In.uv;\n 	float pedalMask = texture(pedalTexture, uv).r;\n 	float finalOpacity = pedalMask * mix(pedalOpacity, 1.0, pedalFlag);\n 	fragColor = vec4(pedalColor, finalOpacity);\n }\n "},
{ "wave_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform float amplitude;\n uniform float keyboardSize;\n uniform float freq;\n uniform float phase;\n uniform float spread;\n uniform bool horizontalMode = false;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n out INTERFACE {\n 	float grad;\n } Out ;\n void main(){\n 	// Rescale as a thin line.\n 	vec2 pos = vec2(1.0, spread*0.02) * v.xy;\n 	// Sin perturbation.\n 	float waveShift = amplitude * sin(freq * v.x + phase);\n 	// Apply wave and translate to put on top of the keyboard.\n 	pos += vec2(0.0, waveShift + (-1.0 + 2.0 * keyboardSize));\n 	gl_Position = vec4(flipIfNeeded(pos), 0.5, 1.0);\n 	Out.grad = v.y;\n }\n "}, 
{ "wave_frag", "#version 330\n in INTERFACE {\n 	float grad;\n } In ;\n uniform vec3 waveColor;\n uniform float waveOpacity;\n out vec4 fragColor;\n void main(){\n 	// Fade out on the edges.\n 	float intensity = (1.0-abs(In.grad));\n 	// Premultiplied alpha.\n 	fragColor = waveOpacity * intensity * vec4(waveColor, 1.0);\n }\n "},
{ "fxaa_vert", "#version 330\n layout(location = 0) in vec3 v;\n out INTERFACE {\n 	vec2 uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	gl_Position = vec4(v, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = v.xy * 0.5 + 0.5;\n 	\n }\n "}, 
{ "fxaa_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n } In ;\n uniform sampler2D screenTexture;\n uniform vec2 inverseScreenSize;\n out vec4 fragColor;\n // Settings for FXAA.\n #define EDGE_THRESHOLD_MIN 0.0312\n #define EDGE_THRESHOLD_MAX 0.125\n #define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))\n #define ITERATIONS 12\n #define SUBPIXEL_QUALITY 0.75\n float rgb2luma(vec3 rgb){\n 	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));\n }\n /** Performs FXAA post-process anti-aliasing as described in the Nvidia FXAA white paper and the associated shader code.\n */\n void main(){\n 	vec4 colorCenter = texture(screenTexture,In.uv);\n 	// Luma at the current fragment\n 	float lumaCenter = rgb2luma(colorCenter.rgb);\n 	// Luma at the four direct neighbours of the current fragment.\n 	float lumaDown 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2( 0,-1)).rgb);\n 	float lumaUp 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2( 0, 1)).rgb);\n 	float lumaLeft 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2(-1, 0)).rgb);\n 	float lumaRight = rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2( 1, 0)).rgb);\n 	// Find the maximum and minimum luma around the current fragment.\n 	float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));\n 	float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));\n 	// Compute the delta.\n 	float lumaRange = lumaMax - lumaMin;\n 	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.\n 	if(lumaRange < max(EDGE_THRESHOLD_MIN,lumaMax*EDGE_THRESHOLD_MAX)){\n 		fragColor = colorCenter;\n 		return;\n 	}\n 	// Query the 4 remaining corners lumas.\n 	float lumaDownLeft 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2(-1,-1)).rgb);\n 	float lumaUpRight 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2( 1, 1)).rgb);\n 	float lumaUpLeft 	= rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2(-1, 1)).rgb);\n 	float lumaDownRight = rgb2luma(textureLodOffset(screenTexture,In.uv, 0.0,ivec2( 1,-1)).rgb);\n 	// Combine the four edges lumas (using intermediary variables for future computations with the same values).\n 	float lumaDownUp = lumaDown + lumaUp;\n 	float lumaLeftRight = lumaLeft + lumaRight;\n 	// Same for corners\n 	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;\n 	float lumaDownCorners = lumaDownLeft + lumaDownRight;\n 	float lumaRightCorners = lumaDownRight + lumaUpRight;\n 	float lumaUpCorners = lumaUpRight + lumaUpLeft;\n 	// Compute an estimation of the gradient along the horizontal and vertical axis.\n 	float edgeHorizontal =	abs(-2.0 * lumaLeft + lumaLeftCorners)	+ abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0	+ abs(-2.0 * lumaRight + lumaRightCorners);\n 	float edgeVertical =	abs(-2.0 * lumaUp + lumaUpCorners)		+ abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0	+ abs(-2.0 * lumaDown + lumaDownCorners);\n 	// Is the local edge horizontal or vertical ?\n 	bool isHorizontal = (edgeHorizontal >= edgeVertical);\n 	// Choose the step size (one pixel) accordingly.\n 	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;\n 	// Select the two neighboring texels lumas in the opposite direction to the local edge.\n 	float luma1 = isHorizontal ? lumaDown : lumaLeft;\n 	float luma2 = isHorizontal ? lumaUp : lumaRight;\n 	// Compute gradients in this direction.\n 	float gradient1 = luma1 - lumaCenter;\n 	float gradient2 = luma2 - lumaCenter;\n 	// Which direction is the steepest ?\n 	bool is1Steepest = abs(gradient1) >= abs(gradient2);\n 	// Gradient in the corresponding direction, normalized.\n 	float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));\n 	// Average luma in the correct direction.\n 	float lumaLocalAverage = 0.0;\n 	if(is1Steepest){\n 		// Switch the direction\n 		stepLength = - stepLength;\n 		lumaLocalAverage = 0.5*(luma1 + lumaCenter);\n 	} else {\n 		lumaLocalAverage = 0.5*(luma2 + lumaCenter);\n 	}\n 	// Shift UV in the correct direction by half a pixel.\n 	vec2 currentUv = In.uv;\n 	if(isHorizontal){\n 		currentUv.y += stepLength * 0.5;\n 	} else {\n 		currentUv.x += stepLength * 0.5;\n 	}\n 	// Compute offset (for each iteration step) in the right direction.\n 	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);\n 	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.\n 	vec2 uv1 = currentUv - offset * QUALITY(0);\n 	vec2 uv2 = currentUv + offset * QUALITY(0);\n 	// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.\n 	float lumaEnd1 = rgb2luma(textureLod(screenTexture,uv1, 0.0).rgb);\n 	float lumaEnd2 = rgb2luma(textureLod(screenTexture,uv2, 0.0).rgb);\n 	lumaEnd1 -= lumaLocalAverage;\n 	lumaEnd2 -= lumaLocalAverage;\n 	// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.\n 	bool reached1 = abs(lumaEnd1) >= gradientScaled;\n 	bool reached2 = abs(lumaEnd2) >= gradientScaled;\n 	bool reachedBoth = reached1 && reached2;\n 	// If the side is not reached, we continue to explore in this direction.\n 	if(!reached1){\n 		uv1 -= offset * QUALITY(1);\n 	}\n 	if(!reached2){\n 		uv2 += offset * QUALITY(1);\n 	}\n 	// If both sides have not been reached, continue to explore.\n 	if(!reachedBoth){\n 		for(int i = 2; i < ITERATIONS; i++){\n 			// If needed, read luma in 1st direction, compute delta.\n 			if(!reached1){\n 				lumaEnd1 = rgb2luma(textureLod(screenTexture, uv1, 0.0).rgb);\n 				lumaEnd1 = lumaEnd1 - lumaLocalAverage;\n 			}\n 			// If needed, read luma in opposite direction, compute delta.\n 			if(!reached2){\n 				lumaEnd2 = rgb2luma(textureLod(screenTexture, uv2, 0.0).rgb);\n 				lumaEnd2 = lumaEnd2 - lumaLocalAverage;\n 			}\n 			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.\n 			reached1 = abs(lumaEnd1) >= gradientScaled;\n 			reached2 = abs(lumaEnd2) >= gradientScaled;\n 			reachedBoth = reached1 && reached2;\n 			// If the side is not reached, we continue to explore in this direction, with a variable quality.\n 			if(!reached1){\n 				uv1 -= offset * QUALITY(i);\n 			}\n 			if(!reached2){\n 				uv2 += offset * QUALITY(i);\n 			}\n 			// If both sides have been reached, stop the exploration.\n 			if(reachedBoth){ break;}\n 		}\n 	}\n 	// Compute the distances to each side edge of the edge (!).\n 	float distance1 = isHorizontal ? (In.uv.x - uv1.x) : (In.uv.y - uv1.y);\n 	float distance2 = isHorizontal ? (uv2.x - In.uv.x) : (uv2.y - In.uv.y);\n 	// In which direction is the side of the edge closer ?\n 	bool isDirection1 = distance1 < distance2;\n 	float distanceFinal = min(distance1, distance2);\n 	// Thickness of the edge.\n 	float edgeThickness = (distance1 + distance2);\n 	// Is the luma at center smaller than the local average ?\n 	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;\n 	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).\n 	bool correctVariation1 = (lumaEnd1 < 0.0) != isLumaCenterSmaller;\n 	bool correctVariation2 = (lumaEnd2 < 0.0) != isLumaCenterSmaller;\n 	// Only keep the result in the direction of the closer side of the edge.\n 	bool correctVariation = isDirection1 ? correctVariation1 : correctVariation2;\n 	// UV offset: read in the direction of the closest side of the edge.\n 	float pixelOffset = - distanceFinal / edgeThickness + 0.5;\n 	// If the luma variation is incorrect, do not offset.\n 	float finalOffset = correctVariation ? pixelOffset : 0.0;\n 	// Sub-pixel shifting\n 	// Full weighted average of the luma over the 3x3 neighborhood.\n 	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);\n 	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.\n 	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);\n 	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;\n 	// Compute a sub-pixel offset based on this delta.\n 	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;\n 	// Pick the biggest of the two offsets.\n 	finalOffset = max(finalOffset,subPixelOffsetFinal);\n 	// Compute the final UV coordinates.\n 	vec2 finalUv = In.uv;\n 	if(isHorizontal){\n 		finalUv.y += finalOffset * stepLength;\n 	} else {\n 		finalUv.x += finalOffset * stepLength;\n 	}\n 	// Read the color at the new UV coordinates, and use it.\n 	vec4 finalColor = textureLod(screenTexture,finalUv, 0.0);\n 	fragColor = finalColor;\n }\n "},
{ "wave_noise_vert", "#version 330\n layout(location = 0) in vec2 v;\n uniform float keyboardSize;\n uniform float scale;\n uniform bool horizontalMode;\n uniform float noiseScale;\n uniform vec2 inverseScreenSize;\n uniform float offset;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n out INTERFACE {\n 	vec2 uv;\n 	float grad;\n } Out ;\n void main(){\n 	// Rescale as a thin quad\n 	vec2 pos = 2.0 * v.xy * vec2(1.0, 2.0 * scale);\n 	// Apply amplitude and translate to put on top of the keyboard.\n 	pos.y +=  -1.0 + 2.0 * keyboardSize + 2.0 * scale;\n 	gl_Position = vec4(flipIfNeeded(pos), 0.8, 1.0);\n 	float ratio = horizontalMode ? (inverseScreenSize.y / inverseScreenSize.x) : (inverseScreenSize.x / inverseScreenSize.y);\n 	Out.uv = (v.xy + 0.5) * vec2(1.0, scale) * noiseScale * vec2(1.0, ratio);\n 	Out.uv.y -= offset;\n 	Out.grad = 0.5-v.y;\n }\n "}, 
{ "wave_noise_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n 	float grad;\n } In ;\n uniform vec3 waveColor;\n uniform float waveOpacity;\n uniform sampler2D textureNoise;\n out vec4 fragColor;\n void main(){\n 	// Premultiplied alpha.\n 	vec2 uv = In.uv;\n 	float noise = texture(textureNoise, uv).r;\n 	fragColor = noise * waveOpacity * vec4(waveColor, 1.0) * In.grad * 4.0;\n }\n "},
{ "score_bars_vert", "#version 330\n layout(location = 0) in vec3 v;\n uniform vec2 baseOffset;\n uniform vec2 nextOffset;\n uniform vec2 scale;\n uniform bool horizontalMode;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n out INTERFACE {\n 	float uv;\n } Out ;\n void main(){\n 	\n 	// We directly output the position.\n 	vec2 pos = v.xy * scale * 2.0f + float(gl_InstanceID) * nextOffset + baseOffset;\n 	gl_Position = vec4(flipIfNeeded(pos), 0.0, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	Out.uv = 2.0 * (scale.x > 0.99 ? v.y : v.x);\n }\n "}, 
{ "score_bars_frag", "#version 330\n in INTERFACE {\n 	float uv;\n } In ;\n uniform vec3 color;\n out vec4 fragColor;\n void main(){\n 	float uv = 1.0 - abs(In.uv);\n 	float gradU = fwidth(uv);\n 	float alpha = smoothstep(0.1 - gradU, 0.1 + gradU, uv);\n 	fragColor = vec4(color, alpha);\n }\n "},
{ "score_labels_vert", "#version 330\n layout(location = 0) in vec3 v;\n uniform vec2 baseOffset;\n uniform vec2 nextOffset;\n uniform vec2 scale;\n uniform bool horizontalMode;\n uniform int maxDigitCount;\n uniform int firstMeasure;\n vec2 flipIfNeeded(vec2 inPos){\n 	return horizontalMode ? vec2(inPos.y, -inPos.x) : inPos;\n }\n out INTERFACE {\n 	vec2 uv;\n 	float id;\n } Out ;\n void main(){\n 	int fullID = firstMeasure + gl_InstanceID;\n 	// floor(log(fullId)/log(10)) + 1, but accuracy issues...\n 	int powTen = 1;\n 	int digitCount = 0;\n 	for( ; digitCount <= maxDigitCount; ++digitCount){\n 		if(powTen > fullID){\n 			break;\n 		}\n 		powTen *= 10;\n 	}\n 	float currentDigitCount = float(max(1, digitCount));\n 	vec2 size = vec2(currentDigitCount, 1.0);\n 	vec2 maxSize = vec2(float(maxDigitCount), 1.0);\n 	vec2 finalScale = 2.f * scale * size;\n 	vec2 maxFinalScale = 2.f * scale * maxSize;\n 	// We directly output the position.\n 	vec2 fullOffset = float(gl_InstanceID) * nextOffset + baseOffset;// + 0.5 * finalScale;\n 	if(horizontalMode){\n 		// Nothing to do on Y (which will become X) (already centered)\n 		// just center on X (which becomes Y)\n 		//Cheat to avoid digit touching the edge of the screen with offset 0.\n 		fullOffset.x += 0.6 * maxFinalScale.y;\n 	} else {\n 		// We want to right align, so use the max scale.\n 		fullOffset.x += maxFinalScale.x - 0.5 * finalScale.x;\n 		// Nothing to do on the Y axis.\n 	}\n 	vec2 pos = v.xy * finalScale + flipIfNeeded(fullOffset);\n 	gl_Position = vec4(pos, 0.1, 1.0);\n 	// Output the UV coordinates computed from the positions.\n 	// We flip the UVs so that the integer part corresponds to a power of ten\n 	Out.uv = (-v.xy + 0.5) * size;\n 	Out.id = float(fullID);\n }\n "}, 
{ "score_labels_frag", "#version 330\n in INTERFACE {\n 	vec2 uv;\n 	float id;\n } In ;\n uniform sampler2D font;\n uniform vec3 color;\n uniform int maxDigitCount;\n out vec4 fragColor;\n void main(){\n 	vec2 globalUV = In.uv;\n 	int digitLoc = int(floor(globalUV.x));\n 	int measure = int(floor(In.id));\n 	int powTen = 1;\n 	for(int i = 0; i <= maxDigitCount; ++i){\n 		if(i >= digitLoc ){\n 			continue;\n 		}\n 		powTen *= 10;\n 	}\n 	int number = (measure / powTen) % 10;\n 	// Remap 0,1 to 0.0, 1.0-padding on X,\n 	const float paddingInTexture = 0.12;\n 	// Flip UVs back.\n 	vec2 localUV = (1.0-fract(globalUV)) * vec2(1.0-paddingInTexture, 1.0);\n 	vec2 pixelSize = 0.5 / vec2(200.0, 256.0);\n 	localUV = clamp(localUV, pixelSize, 1.0 - pixelSize);\n 	localUV += vec2(number % 5, 1 - (number / 5));\n 	localUV *= vec2(0.2, 0.5);\n 	vec2 distancesFont = textureLod(font, localUV, 0).rg;\n 	float sdfFont = distancesFont.r - distancesFont.g;\n 	// Clamp to a reasonable value here to avoid edge artifacts?\n 	float deltaStep = min(fwidth(sdfFont), 0.1);\n 	float alpha = 1.0 - smoothstep(0.01 - deltaStep, 0.01 + deltaStep, sdfFont);\n 	if(measure < 0){\n 		discard;\n 	}\n 	if((measure < powTen) && (digitLoc != 0)){\n 		discard;\n 	}\n 	\n 	fragColor = vec4(color, alpha);\n }\n "}
};
