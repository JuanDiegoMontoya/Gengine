#pragma once
#include <camera.h>
#include <Frustum.h>
#include "block.h"
#include "light.h"
#include "misc_utils.h"
#include <mutex>
#include <concurrent_unordered_map.h> // TODO: temp solution to concurrent chunk access
#include <atomic>
#include <Shapes.h>

#include "ChunkHelpers.h"
#include "BlockStorage.h"
#include "ChunkMesh.h"

#include <cereal/archives/binary.hpp>

#define MARCHED_CUBES 0
#define ID3D(x, y, z, h, w) (x + h * (y + w * z))
#define ID2D(x, y, w) (w * y + x)

//typedef class Block;

class VAO;
class VBO;
class IBO;

//typedef std::pair<glm::ivec3, glm::ivec3> localpos;

/*
	0: -x-y+z
	1: +x-y+z
	2: +x-y-z
	3: -x-y-z
	4: -x+y+z
	5: +x+y+z
	6: +x+y-z
	7: -x+y-z
*/

// TODO: clean this up a lot
typedef struct Chunk
{
private:
public:
	Chunk();
	~Chunk();
	Chunk(const Chunk& other);
	Chunk& operator=(const Chunk& rhs);

	/*################################
						Global Chunk Info
	################################*/
	static constexpr int CHUNK_SIZE			  = 32;
	static constexpr int CHUNK_SIZE_SQRED = CHUNK_SIZE * CHUNK_SIZE;
	static constexpr int CHUNK_SIZE_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
	static constexpr int CHUNK_SIZE_LOG2  = 5; // log2(32) = 5


	/*################################
						Draw Functions
	################################*/
	void Update();


	/*################################
					Status Functions
	################################*/
	const glm::mat4& GetModel() const { return model_; }

	void SetPos(const glm::ivec3& pos)
	{
		pos_ = pos;
		model_ = glm::translate(glm::mat4(1.f), glm::vec3(pos_) * (float)CHUNK_SIZE);
		bounds.min = glm::vec3(pos_ * CHUNK_SIZE);
		bounds.max = glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE);
	}

	inline const glm::ivec3& GetPos() { return pos_; }

	inline bool IsVisible(Camera& cam) const
	{
		return cam.GetFrustum()->IsInside(bounds) >= Frustum::Visibility::Partial;
	}

	inline Block BlockAt(const glm::ivec3& p)
	{
		return storage.GetBlock(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
	}

	inline Block BlockAt(int index)
	{
		return storage.GetBlock(index);
	}

	inline BlockType BlockTypeAt(const glm::ivec3& p)
	{
		return storage.GetBlockType(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
	}

	inline BlockType BlockTypeAt(int index)
	{
		return storage.GetBlockType(index);
	}

	inline void SetBlockTypeAt(const glm::ivec3& lpos, BlockType type)
	{
		storage.SetBlock(
			ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE), type);
	}

	inline void SetLightAt(const glm::ivec3& lpos, Light light)
	{
		storage.SetLight(
			ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE), light);
	}

	inline Light LightAt(const glm::ivec3& p)
	{
		return storage.GetLight(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
	}

	inline Light LightAt(int index)
	{
		return storage.GetLight(index);
	}

	AABB GetAABB() const
	{
		return bounds;
	}


	void BuildMesh()
	{
		mesh.BuildMesh();
	}

	void BuildBuffers()
	{
		//mesh.BuildBuffers();
		mesh.BuildBuffers2();
	}

	void Render()
	{
		mesh.Render();
	}

	void RenderSplat()
	{
		mesh.RenderSplat();
	}

	ChunkMesh& GetMesh()
	{
		return mesh;
	}

	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(pos_, storage);
	}

private:
	glm::mat4 model_;
	glm::ivec3 pos_;	// position relative to other chunks (1 chunk = 1 index)
	bool visible_;		// used in frustum culling
	AABB bounds{};

	//ArrayBlockStorage<CHUNK_SIZE_CUBED> storage;
	PaletteBlockStorage<CHUNK_SIZE_CUBED> storage;
	ChunkMesh mesh;
}Chunk, *ChunkPtr;
