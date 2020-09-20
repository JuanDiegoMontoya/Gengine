#pragma once
#include "BufferAllocator.h"
#include <Graphics/vbo.h>
#include <Graphics/vao.h>
#include <Debug.h>


template<typename UserT>
BufferAllocator<UserT>::BufferAllocator<UserT>(GLuint size, GLuint alignment)
	: align_(alignment), capacity_(size)
{
	// add to size the distance to next aligned boundary
	// TODO: simplify (use align_ - 1 thing)
	size += (align_ - (size % align_)) % align_;

	// allocate uninitialized memory in VRAM
	// TODO: use immutable buffer storage for this (unless dynamic resizing is a thing)
	glCreateBuffers(1, &gpuHandle);
	glNamedBufferData(gpuHandle, size, NULL, GL_STATIC_DRAW);

	glCreateBuffers(1, &allocDataGpuHandle_);

	// make one big null allocation
	allocationData<UserT> phalloc;
	phalloc.handle = NULL;
	phalloc.offset = 0;
	phalloc.size = size;
	phalloc.time = 0;
	allocs_.push_back(phalloc);
}


template<typename UserT>
BufferAllocator<UserT>::~BufferAllocator<UserT>()
{
	glDeleteBuffers(1, &gpuHandle);
	glDeleteBuffers(1, &allocDataGpuHandle_);
}


template<typename UserT>
uint64_t BufferAllocator<UserT>::Allocate(void* data, GLuint size, UserT userdata)
{
	size += (align_ - (size % align_)) % align_;
	// find smallest NULL allocation that will fit
	Iterator small = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].handle == NULL && allocs_[i].size >= size) // potential allocation
		{
			if (small == allocs_.end())// initialize small
				small = allocs_.begin() + i;
			else if (allocs_[i].size < small->size)
				small = allocs_.begin() + i;
		}
	}
	// allocation failure
	if (small == allocs_.end())
		return NULL;

	// split free allocation
	allocationData<UserT> newAlloc(userdata);
	newAlloc.handle = nextHandle++;
	newAlloc.offset = small->offset;
	newAlloc.size = size;
	newAlloc.time = glfwGetTime();
	newAlloc.flags = 0;
	//newAlloc.userdata = userdata;

	small->offset += newAlloc.size;
	small->size -= newAlloc.size;

	// replace shrunk alloc if it would become degenerate
	if (small->size == 0)
		*small = newAlloc;
	else
		allocs_.insert(small, newAlloc);

	glNamedBufferSubData(gpuHandle, newAlloc.offset, newAlloc.size, data);
	++numActiveAllocs_;
	stateChanged();
	return newAlloc.handle;
}


template<typename UserT>
bool BufferAllocator<UserT>::Free(uint64_t handle)
{
	if (handle == NULL) return false;
	auto it = std::find_if(allocs_.begin(), allocs_.end(), [&](const auto& a) { return a.handle == handle; });
	if (it == allocs_.end()) // failed to free
		return false;

	it->handle = NULL;
	maybeMerge(it);
	--numActiveAllocs_;
	stateChanged();
	return true;
}


template<typename UserT>
bool BufferAllocator<UserT>::FreeOldest()
{
	// find and free the oldest allocation
	Iterator old = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].handle != NULL)
		{
			if (old == allocs_.end())
				old = allocs_.begin() + i;
			else if (allocs_[i].time < old->time)
				old = allocs_.begin() + i;
		}
	}

	// failed to find old node to free
	if (old == allocs_.end())
		return false;

	old->handle = NULL;
	maybeMerge(old);
	--numActiveAllocs_;
	stateChanged();
	return true;
}


template<typename UserT>
inline void BufferAllocator<UserT>::stateChanged()
{
	DEBUG_DO(dbgVerify());
	dirty_ = true;
}

template<typename UserT>
void BufferAllocator<UserT>::maybeMerge(Iterator it)
{
	bool removeIt = false;
	bool removeNext = false;

	// merge with next alloc
	if (it != allocs_.end() - 1)
	{
		Iterator next = it + 1;
		if (next->handle == NULL)
		{
			it->size += next->size;
			removeNext = true;
		}
	}

	// merge with previous alloc
	if (it != allocs_.begin())
	{
		Iterator prev = it - 1;
		if (prev->handle == NULL)
		{
			prev->size += it->size;
			removeIt = true;
		}
	}

	// erase merged allocations
	if (removeIt && removeNext)
		allocs_.erase(it, it + 2); // this and next
	else if (removeIt)
		allocs_.erase(it);         // just this
	else if (removeNext)
		allocs_.erase(it + 1);     // just next
}


template<typename UserT>
void BufferAllocator<UserT>::dbgVerify()
{
	uint64_t prevPtr = 1;
	GLsizei sumSize = 0;
	GLuint active = 0;
	for (const auto& alloc : allocs_)
	{
		if (alloc.handle != NULL)
			active++;
		// check there are never two null blocks in a row
		ASSERT_MSG(!(prevPtr == NULL && alloc.handle == NULL),
			"Verify failed: two null blocks in a row!");
		prevPtr = alloc.handle;

		// check offset is equal to total size so far
		ASSERT_MSG(alloc.offset == sumSize,
			"Verify failed: size/offset discrepancy!");
		sumSize += alloc.size;

		// check alignment
		ASSERT_MSG(alloc.offset % align_ == 0,
			"Verify failed: block alignment mismatch!");

		// check degenerate (0-size) allocation
		ASSERT_MSG(alloc.size != 0,
			"Verify failed: 0-size allocation!");
	}

	ASSERT_MSG(active == numActiveAllocs_,
		"Verify failed: active allocations mismatch!");
}


// invalidates existing data in buffer and copies the entire array of data to the GPU
// maybe not efficient, but it works
template<typename UserT>
inline void BufferAllocator<UserT>::bufferAllocData()
{
	glNamedBufferData(allocDataGpuHandle_, AllocSize() * allocs_.size(), allocs_.data(), GL_STATIC_COPY);
}


template<typename UserT>
inline void BufferAllocator<UserT>::genDrawData()
{
	const glm::vec3 free_color{ .2, 1, 1 };  // light cyan
	const glm::vec3 full_color{ 1, .3, .3 }; // light red
	bool alternator = true;

	vao_ = std::make_unique<VAO>();
	VBOlayout layout;
	layout.Push<float>(3); // position
	layout.Push<float>(3); // color

	std::vector<glm::vec3> data;
	for (const auto& alloc : allocs_)
	{
		glm::vec3 color = alloc.handle == NULL ? free_color : full_color;
		if (color == free_color)
			alternator = true;
		color *= (alternator ? 1.f : .5f);

		// position 1
		data.push_back({ (float)alloc.offset / (float)capacity_, 0, 0 });
		// color 1
		data.push_back(color);

		// position 2
		data.push_back({ (float)(alloc.offset + alloc.size) / (float)capacity_, 0, 0 });
		// color 2
		data.push_back(color);

		alternator = !alternator;
	}

	vbo_ = std::make_unique<VBO>(&data[0][0], sizeof(glm::vec3) * data.size(), GL_STREAM_DRAW);
	vao_->AddBuffer(*vbo_, layout);
}


//#pragma optimize("", off);
// length = entire screen by default, line width set beforehand by user
template<typename UserT>
inline void BufferAllocator<UserT>::Draw()
{
	if (vao_)
	{
		int dataSize = allocs_.size() * 4;
		vao_->Bind();
		glDrawArrays(GL_LINES, 0, dataSize / 2);
	}
}


template<typename UserT>
inline void BufferAllocator<UserT>::Update()
{
	if (dirty_)
	{
		genDrawData();
		bufferAllocData();
		dirty_ = false;
	}
}
//#pragma optimize("", on);