#pragma once

// no userdata specialization
struct Empty_ {};

// Generic GPU buffer that can store
//   up to 4GB (UINT_MAX) of data
template<typename UserT>
class BufferAllocator
{
public:

	BufferAllocator(GLuint size, GLuint alignment);
	~BufferAllocator();

	// Change data of the allocator
	uint64_t Allocate(void* data, GLuint size, UserT userdata = {});
	bool Free(uint64_t handle);
	bool FreeOldest();

	
	// Query information about the allocator
	const auto& GetAllocs() { return allocs_; }
	GLuint ActiveAllocs() { return numActiveAllocs_; }

	GLuint GetGPUHandle() { return gpuHandle; }
	GLuint GetAllocDataGPUHandle() { return allocDataGpuHandle_; }


	// Misc functions
	void Draw();
	void Update();

	const GLsizei align_; // allocation alignment

	template<typename UT>
	struct allocationData
	{
		allocationData() = default;
		allocationData(UT u) : userdata(u) {}

		uint64_t handle;// "pointer"
		double time;    // time of allocation
		uint32_t flags; // GPU flags
		uint32_t _pad;  // GPU padding
		GLuint offset;  // offset from beginning of this memory
		GLuint size;    // allocation size
		UT userdata;    // user-defined data
	};

	template<>
	struct allocationData<Empty_>
	{
		allocationData() = default;
		allocationData(Empty_) {}

		uint64_t handle;// "pointer"
		double time;    // time of allocation
		uint32_t flags; // GPU flags
		uint32_t _pad;  // GPU padding
		GLuint offset;  // offset from beginning of this memory
		GLuint size;    // allocation size
	};

	GLsizei AllocSize() const { return sizeof(allocationData<UserT>); }

	bool GetDirty() { return dirty_; }

private:
	std::vector<allocationData<UserT>> allocs_;
	using Iterator = decltype(allocs_.begin());

	// called whenever anything about the allocator changed
	void stateChanged();

	// merges adjacent null allocations to iterator
	void maybeMerge(Iterator it);

	// verifies the buffer has no errors, debug only
	void dbgVerify();

	void bufferAllocData();

	GLuint gpuHandle = 0;
	uint64_t nextHandle = 1;
	GLuint numActiveAllocs_ = 0;
	const GLuint capacity_; // for fixed size buffers

	GLuint allocDataGpuHandle_ = 0;

	bool dirty_ = false;

	// debug
	std::unique_ptr<class VAO> vao_;
	std::unique_ptr<class VBO> vbo_;
	void genDrawData();
};

#include "BufferAllocator.inl"