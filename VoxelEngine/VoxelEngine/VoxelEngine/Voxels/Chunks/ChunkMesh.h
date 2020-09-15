#pragma once
#include <block.h>
//#include <Chunks/Chunk.h>
#include <Rendering/NuRenderer.h>
#include <Graphics/dib.h>
#include <Graphics/vbo.h>
#include <Graphics/vao.h>

//class VAO;
//class VBO;
//class DIB;
struct Chunk;

class ChunkMesh
{
public:
	ChunkMesh(Chunk* p) : parent(p) {}
	~ChunkMesh();

	void Render();
	void RenderSplat();
	void BuildBuffers();
	void BuildBuffers2();
	void BuildMesh();

	GLsizei GetVertexCount() { return vertexCount_; }
	GLsizei GetPointCount() { return pointCount_; }

	// debug
	static inline bool debug_ignore_light_level = false;
	static inline std::atomic<double> accumtime = 0;
	static inline std::atomic<unsigned> accumcount = 0;

private:

	void buildBlockFace(
		int face,
		const glm::ivec3& blockPos,
		BlockType block);
	void addQuad(const glm::ivec3& lpos, BlockType block, int face, Chunk* nearChunk, Light light);
	int vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm);


	enum
	{
		Far,
		Near,
		Left,
		Right,
		Top,
		Bottom,

		fCount
	};

	Chunk* parent = nullptr;
	Chunk* nearChunks[6]{ nullptr };

	std::unique_ptr<VAO> vao_;
	std::unique_ptr<VBO> encodedStuffVbo_;
	std::unique_ptr<VBO> lightingVbo_;
	std::unique_ptr<VBO> posVbo_;

	// vertex data (held until buffers are sent to GPU)
	std::vector<GLint> encodedStuffArr;
	std::vector<GLint> lightingArr;
	std::vector<GLint> interleavedArr;

	GLsizei vertexCount_ = 0; // number of block vertices
	uint64_t bufferHandle = NULL;
	uint64_t bufferHandleSplat = NULL;

	// SPLATTING STUFF
	std::unique_ptr<VAO> svao_;
	std::unique_ptr<VBO> svbo_;
	std::vector<GLint> sPosArr; // point positions
	GLsizei pointCount_ = 0;
	bool voxelReady_ = true; // hack to prevent same voxel from being added multiple times for splatting (I think)

	// indirect drawing stuff
	std::unique_ptr<DIB> dib_;

	std::shared_mutex mtx;
};


