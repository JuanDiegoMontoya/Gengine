#pragma once
#include <functional>
#include <Graphics/GraphicsIncludes.h>

class VAO;
class IBO;
class Shader;
class DirLight;
class Sun;
class ChunkManager;
class Pipeline;
typedef struct Chunk* ChunkPtr;
typedef std::function<void()> DrawCB;
typedef std::function<void(const glm::mat4&)> ModelCB;

// responsible for making stuff appear on the screen
namespace Renderer
{
	void Init();

	// interation
	void DrawAll();
	void Clear();
	void ClearCSM();
	void ClearGGuffer();

	void SetDirLight(DirLight* d);
	void SetSun(Sun* s);

	void DrawCube();


	// broad-phase rendering
	void drawSky();
	void drawPostProcessing(); // apply post processing effects

	// narrow-phase rendering
	void drawBillboard(VAO* vao, size_t count, DrawCB uniform_cb);
	void drawQuad();

	// debug
	void drawDepthMapsDebug();
	void drawAxisIndicators();

	// deferred rendering
	void initDeferredBuffers();
	void lightingPass();

	// post processing
	void initPPBuffers();
	void postProcess();

	Pipeline* GetPipeline();


	/*###################################################
									"public" variables
	###################################################*/
	inline bool renderShadows = true;
	inline bool doGeometryPass = true; // for ssr currently
	//bool renderSSR = true;
	inline bool nvUsageEnabled = false;

	// post processing
	inline bool ppSharpenFilter = false;
	inline bool ppBlurFilter = false;
	inline bool ppEdgeDetection = false;
	inline bool ppChromaticAberration = false;

	//inline ChunkManager* chunkManager_;


	/*###################################################
									"private" variables
	###################################################*/
	inline DirLight* activeDirLight_;
	inline Sun* activeSun_;

	// ssr & deferred rendering
	inline unsigned gBuffer; // framebuffer
	inline unsigned gPosition, gNormal, gAlbedoSpec, gDepth;
	inline unsigned rboDepth; // depth renderbuffer

	// pp
	inline unsigned pBuffer;
	inline unsigned pColor;
	inline unsigned pDepth;


	// CSM (temp?)
	inline glm::mat4 view;
	inline glm::mat4 projection;
	inline glm::mat4 vView[3];             // 
	inline glm::vec3 LitDir;               // direction of light
	inline glm::vec3 right;                // light view right
	inline glm::vec3 up;                   // light view up
	inline glm::mat4 LitViewFam;           // light space view matrix
	inline glm::vec3 ratios;               // unused (exponential shadows)
	inline glm::vec4 cascadEnds;           // light space or something
	inline glm::vec3 cascadeEndsClipSpace; // clip space
}