/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.h"

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
#include <tinystl/string.h>
namespace stl = tinystl;

#include <bgfx/bgfx.h>
#include <bx/commandline.h>
#include <bx/endian.h>
#include <bx/math.h>
#include <bx/readerwriter.h>
#include <bx/string.h>
#include "entry/entry.h"
#include <meshoptimizer/src/meshoptimizer.h>

#include "bgfx_utils.h"

#include <bimg/decode.h>

#include <tuple>

//namespace bgfx {
//  void submit(ViewId _id, ProgramHandle _program, uint32_t _depth, uint8_t _preserveState)
//  {
//
//  }
//}

void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
	if (bx::open(_reader, _filePath) )
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		void* data = BX_ALLOC(_allocator, size);
		bx::read(_reader, data, size);
		bx::close(_reader);
		if (NULL != _size)
		{
			*_size = size;
		}
		return data;
	}
	else
	{
		DBG("Failed to open: %s.", _filePath);
	}

	if (NULL != _size)
	{
		*_size = 0;
	}

	return NULL;
}

void* load(const char* _filePath, uint32_t* _size)
{
	return load(entry::getFileReader(), entry::getAllocator(), _filePath, _size);
}

void unload(void* _ptr)
{
	BX_FREE(entry::getAllocator(), _ptr);
}

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
	if (bx::open(_reader, _filePath) )
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		bx::read(_reader, mem->data, size);
		bx::close(_reader);
		mem->data[mem->size-1] = '\0';
		return mem;
	}

	DBG("Failed to load %s.", _filePath);
	return NULL;
}

static void* loadMem(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
	if (bx::open(_reader, _filePath) )
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		void* data = BX_ALLOC(_allocator, size);
		bx::read(_reader, data, size);
		bx::close(_reader);

		if (NULL != _size)
		{
			*_size = size;
		}
		return data;
	}

	DBG("Failed to load %s.", _filePath);
	return NULL;
}

static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
	char filePath[512];

	const char* shaderPath = "???";

	switch (bgfx::getRendererType() )
	{
	case bgfx::RendererType::Noop:
	case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
	case bgfx::RendererType::Gnm:        shaderPath = "shaders/pssl/";  break;
	case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
	case bgfx::RendererType::Nvn:        shaderPath = "shaders/nvn/";   break;
	case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
	case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
	case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;

	case bgfx::RendererType::Count:
		BX_CHECK(false, "You should not be here!");
		break;
	}

	bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
	bx::strCat(filePath, BX_COUNTOF(filePath), _name);
	bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

	bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath) );
	bgfx::setName(handle, _name);

	return handle;
}

bgfx::ShaderHandle loadShader(const char* _name)
{
	return loadShader(entry::getFileReader(), _name);
}

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
	bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
	bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
	if (NULL != _fsName)
	{
		fsh = loadShader(_reader, _fsName);
	}

	return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
{
	return loadProgram(entry::getFileReader(), _vsName, _fsName);
}

static void imageReleaseCb(void* _ptr, void* _userData)
{
	BX_UNUSED(_ptr);
	bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
	bimg::imageFree(imageContainer);
}

bgfx::TextureHandle loadTexture(bx::FileReaderI* _reader, const char* _filePath, uint64_t _flags, uint8_t _skip, bgfx::TextureInfo* _info, bimg::Orientation::Enum* _orientation)
{
	BX_UNUSED(_skip);
	bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;

	uint32_t size;
	void* data = load(_reader, entry::getAllocator(), _filePath, &size);
	if (NULL != data)
	{
		bimg::ImageContainer* imageContainer = bimg::imageParse(entry::getAllocator(), data, size);

		if (NULL != imageContainer)
		{
			if (NULL != _orientation)
			{
				*_orientation = imageContainer->m_orientation;
			}

			const bgfx::Memory* mem = bgfx::makeRef(
					  imageContainer->m_data
					, imageContainer->m_size
					, imageReleaseCb
					, imageContainer
					);
			unload(data);

			if (imageContainer->m_cubeMap)
			{
				handle = bgfx::createTextureCube(
					  uint16_t(imageContainer->m_width)
					, 1 < imageContainer->m_numMips
					, imageContainer->m_numLayers
					, bgfx::TextureFormat::Enum(imageContainer->m_format)
					, _flags
					, mem
					);
			}
			else if (1 < imageContainer->m_depth)
			{
				handle = bgfx::createTexture3D(
					  uint16_t(imageContainer->m_width)
					, uint16_t(imageContainer->m_height)
					, uint16_t(imageContainer->m_depth)
					, 1 < imageContainer->m_numMips
					, bgfx::TextureFormat::Enum(imageContainer->m_format)
					, _flags
					, mem
					);
			}
			else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), _flags) )
			{
				handle = bgfx::createTexture2D(
					  uint16_t(imageContainer->m_width)
					, uint16_t(imageContainer->m_height)
					, 1 < imageContainer->m_numMips
					, imageContainer->m_numLayers
					, bgfx::TextureFormat::Enum(imageContainer->m_format)
					, _flags
					, mem
					);
			}

			if (bgfx::isValid(handle) )
			{
				bgfx::setName(handle, _filePath);
			}

			if (NULL != _info)
			{
				bgfx::calcTextureSize(
					  *_info
					, uint16_t(imageContainer->m_width)
					, uint16_t(imageContainer->m_height)
					, uint16_t(imageContainer->m_depth)
					, imageContainer->m_cubeMap
					, 1 < imageContainer->m_numMips
					, imageContainer->m_numLayers
					, bgfx::TextureFormat::Enum(imageContainer->m_format)
					);
			}
		}
	}

	return handle;
}

bgfx::TextureHandle loadTexture(const char* _name, uint64_t _flags, uint8_t _skip, bgfx::TextureInfo* _info, bimg::Orientation::Enum* _orientation)
{
	return loadTexture(entry::getFileReader(), _name, _flags, _skip, _info, _orientation);
}

bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat)
{
	uint32_t size = 0;
	void* data = loadMem(entry::getFileReader(), entry::getAllocator(), _filePath, &size);

	return bimg::imageParse(entry::getAllocator(), data, size, bimg::TextureFormat::Enum(_dstFormat) );
}

void calcTangents(void* _vertices, uint16_t _numVertices, bgfx::VertexLayout _layout, const uint16_t* _indices, uint32_t _numIndices)
{
	struct PosTexcoord
	{
		float m_x;
		float m_y;
		float m_z;
		float m_pad0;
		float m_u;
		float m_v;
		float m_pad1;
		float m_pad2;
	};

	float* tangents = new float[6*_numVertices];
	bx::memSet(tangents, 0, 6*_numVertices*sizeof(float) );

	PosTexcoord v0;
	PosTexcoord v1;
	PosTexcoord v2;

	for (uint32_t ii = 0, num = _numIndices/3; ii < num; ++ii)
	{
		const uint16_t* indices = &_indices[ii*3];
		uint32_t i0 = indices[0];
		uint32_t i1 = indices[1];
		uint32_t i2 = indices[2];

		bgfx::vertexUnpack(&v0.m_x, bgfx::Attrib::Position, _layout, _vertices, i0);
		bgfx::vertexUnpack(&v0.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i0);

		bgfx::vertexUnpack(&v1.m_x, bgfx::Attrib::Position, _layout, _vertices, i1);
		bgfx::vertexUnpack(&v1.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i1);

		bgfx::vertexUnpack(&v2.m_x, bgfx::Attrib::Position, _layout, _vertices, i2);
		bgfx::vertexUnpack(&v2.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i2);

		const float bax = v1.m_x - v0.m_x;
		const float bay = v1.m_y - v0.m_y;
		const float baz = v1.m_z - v0.m_z;
		const float bau = v1.m_u - v0.m_u;
		const float bav = v1.m_v - v0.m_v;

		const float cax = v2.m_x - v0.m_x;
		const float cay = v2.m_y - v0.m_y;
		const float caz = v2.m_z - v0.m_z;
		const float cau = v2.m_u - v0.m_u;
		const float cav = v2.m_v - v0.m_v;

		const float det = (bau * cav - bav * cau);
		const float invDet = 1.0f / det;

		const float tx = (bax * cav - cax * bav) * invDet;
		const float ty = (bay * cav - cay * bav) * invDet;
		const float tz = (baz * cav - caz * bav) * invDet;

		const float bx = (cax * bau - bax * cau) * invDet;
		const float by = (cay * bau - bay * cau) * invDet;
		const float bz = (caz * bau - baz * cau) * invDet;

		for (uint32_t jj = 0; jj < 3; ++jj)
		{
			float* tanu = &tangents[indices[jj]*6];
			float* tanv = &tanu[3];
			tanu[0] += tx;
			tanu[1] += ty;
			tanu[2] += tz;

			tanv[0] += bx;
			tanv[1] += by;
			tanv[2] += bz;
		}
	}

	for (uint32_t ii = 0; ii < _numVertices; ++ii)
	{
		const bx::Vec3 tanu = bx::load<bx::Vec3>(&tangents[ii*6]);
		const bx::Vec3 tanv = bx::load<bx::Vec3>(&tangents[ii*6 + 3]);

		float nxyzw[4];
		bgfx::vertexUnpack(nxyzw, bgfx::Attrib::Normal, _layout, _vertices, ii);

		const bx::Vec3 normal  = bx::load<bx::Vec3>(nxyzw);
		const float    ndt     = bx::dot(normal, tanu);
		const bx::Vec3 nxt     = bx::cross(normal, tanu);
		const bx::Vec3 tmp     = bx::sub(tanu, bx::mul(normal, ndt) );

		float tangent[4];
		bx::store(tangent, bx::normalize(tmp) );
		tangent[3] = bx::dot(nxt, tanv) < 0.0f ? -1.0f : 1.0f;

		bgfx::vertexPack(tangent, true, bgfx::Attrib::Tangent, _layout, _vertices, ii);
	}

	delete [] tangents;
}



Group::Group()
{
	reset();
}

void Group::reset()
{
	m_vbh.idx = bgfx::kInvalidHandle;
	m_ibh.idx = bgfx::kInvalidHandle;
	m_numVertices = 0;
	m_vertices = NULL;
	m_numIndices = 0;
	m_indices = NULL;
	m_prims.clear();
}

namespace bgfx
{
	int32_t read(bx::ReaderI* _reader, bgfx::VertexLayout& _layout, bx::Error* _err = NULL);
}



static float AreaCube(float width, float height, float depth)
{
	return 2.f * (width * height + height * depth + depth * width);
}
stl::vector<glm::vec3> Mesh::vertices;

//Real Time Collision Detection shtuff
//////////////////////////////////
struct glmSphere {
  glm::vec3 c; // glmSphere center
  float r; // glmSphere radius
};
void ExtremePointsAlongDirection(glm::vec3 dir, glm::vec3 pt[], int n, int* imin, int* imax)
{
  float minproj = FLT_MAX, maxproj = -FLT_MAX;
  for (int i = 0; i < n; i++) {
    // Project vector from origin to point onto direction vector
    float proj = glm::dot(pt[i], dir);
    // Keep track of least distant point along direction vector
    if (proj < minproj) {
      minproj = proj;
      *imin = i;
    }
    // Keep track of most distant point along direction vector
    if (proj > maxproj) {
      maxproj = proj;
      *imax = i;
    }
  }
}
int TestSphereSphere(glmSphere a, glmSphere b)
{
  // Calculate squared distance between centers
  glm::vec3 d = a.c - b.c;
  float dist2 = glm::dot(d, d);
  // Spheres intersect if squared distance is less than squared sum of radii
  float radiusSum = a.r + b.r;
  return dist2 <= radiusSum * radiusSum;
}
void MostSeparatedPointsOnAABB(int& min, int& max, glm::vec3 pt[], int numPts)
{
  // First find most extreme points along principal axes
  int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
  for (int i = 1; i < numPts; i++) {
    if (pt[i].x < pt[minx].x) minx = i;
    if (pt[i].x > pt[maxx].x) maxx = i;
    if (pt[i].y < pt[miny].y) miny = i;
    if (pt[i].y > pt[maxy].y) maxy = i;
    if (pt[i].z < pt[minz].z) minz = i;
    if (pt[i].z > pt[maxz].z) maxz = i;
  }
  // Compute the squared distances for the three pairs of points
  float dist2x = glm::dot(pt[maxx] - pt[minx], pt[maxx] - pt[minx]);
  float dist2y = glm::dot(pt[maxy] - pt[miny], pt[maxy] - pt[miny]);
  float dist2z = glm::dot(pt[maxz] - pt[minz], pt[maxz] - pt[minz]);
  // Pick the pair (min,max) of points most distant
  min = minx;
  max = maxx;
  if (dist2y > dist2x && dist2y > dist2z) 
  {
    max = maxy;
    min = miny;
  }
  if (dist2z > dist2x && dist2z > dist2y) 
  {
    max = maxz;
    min = minz;
  }
}
void SphereFromDistantPoints(glmSphere &s, glm::vec3 pt[], int numPts)
{
// Find the most separated point pair defining the encompassing AABB
int min, max;
MostSeparatedPointsOnAABB(min, max, pt, numPts);
// Set up sphere to just encompass these two points
s.c = (pt[min] + pt[max]) * 0.5f;
s.r = glm::dot(pt[max] - s.c, pt[max] - s.c);
s.r = glm::sqrt(s.r);
}
// Given glmSphere s and glm::vec3 p, update s (if needed) to just encompass p
void SphereOfSphereAndPt(glmSphere& s, glm::vec3& p)
{
  // Compute squared distance between point and sphere center
  glm::vec3 d = p - s.c;
  float dist2 = glm::dot(d, d);
  // Only update s if point p is outside it
  if (dist2 > s.r * s.r) {
    float dist = glm::sqrt(dist2);
    float newRadius = (s.r + dist) * 0.5f;
    float k = (newRadius - s.r) / dist;
    s.r = newRadius;
    s.c += d * k;
  }
}
void RitterSphere(glmSphere& s, glm::vec3 pt[], int numPts)
{
  // Get sphere encompassing two approximately most distant points
  SphereFromDistantPoints(s, pt, numPts);
  // Grow sphere to include all points
  for (int i = 0; i < numPts; i++)
    SphereOfSphereAndPt(s, pt[i]);
}
void CovarianceMatrix(glm::mat3& cov, glm::vec3 pt[], int numPts)
{
  float oon = 1.0f / (float)numPts;
  glm::vec3 c = glm::vec3(0.0f, 0.0f, 0.0f);
  float e00, e11, e22, e01, e02, e12;
  // Compute the center of mass (centroid) of the points
  for (int i = 0; i < numPts; i++)
    c += pt[i];
  c *= oon;
  // Compute covariance elements
  e00 = e11 = e22 = e01 = e02 = e12 = 0.0f;
  for (int i = 0; i < numPts; i++) {
    // Translate points so center of mass is at origin
    glm::vec3 p = pt[i] - c;
    // Compute covariance of translated points
    e00 += p.x * p.x;
    e11 += p.y * p.y;
    e22 += p.z * p.z;
    e01 += p.x * p.y;
    e02 += p.x * p.z;
    e12 += p.y * p.z;
  }
  // Fill in the covariance matrix elements
  cov[0][0] = e00 * oon;
  cov[1][1] = e11 * oon;
  cov[2][2] = e22 * oon;
  cov[0][1] = cov[1][0] = e01 * oon;
  cov[0][2] = cov[2][0] = e02 * oon;
  cov[1][2] = cov[2][1] = e12 * oon;
}
void SymSchur2(glm::mat3& a, int p, int q, float& c, float& s)
{
  if (glm::abs(a[p][q]) > 0.0001f) {
    float r = (a[q][q] - a[p][p]) / (2.0f * a[p][q]);
    float t;
    if (r >= 0.0f)
      t = 1.0f / (r + glm::sqrt(1.0f + r * r));
    else
      t = -1.0f / (-r + glm::sqrt(1.0f + r * r));
    c = 1.0f / glm::sqrt(1.0f + t * t);
    s = t * c;
  }
  else {
    c = 1.0f;
    s = 0.0f;
  }
}
void Jacobi(glm::mat3& a, glm::mat3& v)
{
  int i, j, n, p, q;
  float prevoff, c, s;
  glm::mat3 J, b, t;
  // Initialize v to identify matrix
  for (i = 0; i < 3; i++) {
    v[i][0] = v[i][1] = v[i][2] = 0.0f;
    v[i][i] = 1.0f;
  }
  // Repeat for some maximum number of iterations
  const int MAX_ITERATIONS = 50;
  for (n = 0; n < MAX_ITERATIONS; n++) {
    // Find largest off-diagonal absolute element a[p][q]
    p = 0; q = 1;
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        if (i == j) continue;
        if (glm::abs(a[i][j]) > glm::abs(a[p][q])) {
          p = i;
          q = j;
        }
      }
    }
    // Compute the Jacobi rotation matrix J(p, q, theta)
    // (This code can be optimized for the three different cases of rotation)
    SymSchur2(a, p, q, c, s);
    for (i = 0; i < 3; i++) {
      J[i][0] = J[i][1] = J[i][2] = 0.0f;
      J[i][i] = 1.0f;
    }
    J[p][p] = c; J[p][q] = s;
    J[q][p] = -s; J[q][q] = c;
    // Cumulate rotations into what will contain the eigenvectors
    v = v * J;
    // Make ’a’ more diagonal, until just eigenvalues remain on diagonal
    a = (glm::transpose(J) * a) * J;
    // Compute "norm" of off-diagonal elements
    float off = 0.0f;
    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        if (i == j) continue;
        off += a[i][j] * a[i][j];
      }
    }
    /* off = sqrt(off); not needed for norm comparison */
    // Stop when norm no longer decreasing
    if (n > 2 && off >= prevoff)
      return;
    prevoff = off;
  }
}
void EigenSphere(glmSphere& eigSphere, glm::vec3 pt[], int numPts)
{
  glm::mat3 m, v;
  // Compute the covariance matrix m
  CovarianceMatrix(m, pt, numPts);
  // Decompose it into eigenvectors (in v) and eigenvalues (in m)
  Jacobi(m, v);
  // Find the component with largest magnitude eigenvalue (largest spread)
  glm::vec3 e;
  int maxc = 0;
  float maxf, maxe = glm::abs(m[0][0]);
  if ((maxf = glm::abs(m[1][1])) > maxe) maxc = 1, maxe = maxf;
  if ((maxf = glm::abs(m[2][2])) > maxe) maxc = 2, maxe = maxf;
  e[0] = v[0][maxc];
  e[1] = v[1][maxc];
  e[2] = v[2][maxc];
  int imin, imax;
  ExtremePointsAlongDirection(e, pt, numPts, &imin, &imax);
  glm::vec3 minpt = pt[imin];
  glm::vec3 maxpt = pt[imax];
  float dist = glm::sqrt(glm::dot(maxpt - minpt, maxpt - minpt));
  eigSphere.r = dist * 0.5f;
  eigSphere.c = (minpt + maxpt) * 0.5f;
}
void RitterEigenSphere(glmSphere& s, glm::vec3 pt[], int numPts)
{
  // Start with sphere from maximum spread
  EigenSphere(s, pt, numPts);
  // Grow sphere to include all points
  for (int i = 0; i < numPts; i++)
    SphereOfSphereAndPt(s, pt[i]);
}
void RitterIterative(glmSphere& s, glm::vec3 pt[], int numPts)
{
  const int NUM_ITER = 8;
  RitterSphere(s, pt, numPts);
  glmSphere s2 = s;
  for (int k = 0; k < NUM_ITER; k++) {
    // Shrink sphere somewhat to make it an underestimate (not bound)
    s2.r = s2.r * 0.95f;
    // Make sphere bound data again
    for (int i = 0; i < numPts; i++) {
      // Swap pt[i] with pt[j], where j randomly from interval [i+1,numPts-1]
      std::swap(pt[i], pt[i + (std::rand() % (numPts - i))]); //<- this is my code and could be wrong
      SphereOfSphereAndPt(s2, pt[i]);
    }
    // Update s whenever a tighter sphere is found
    if (s2.r < s.r) s = s2;
  }
}
//////////////////////////////////

static void CalculateColliderInfo(Mesh* mesh)
{
  if (false)
  {
    static Aabb testAabb;
    //int TEST = mesh->verts.size() / 4 / 3;
    //toAabb(test, mesh->verts.data(), TEST, 12);
    toAabb(testAabb, &(mesh->vertices[0][0]), mesh->vertices.size(), 12);

    mesh->max = glm::vec3(testAabb.max.x, testAabb.max.y, testAabb.max.z);
    mesh->min = glm::vec3(testAabb.min.x, testAabb.min.y, testAabb.min.z);

    mesh->pos = glm::mix(mesh->max, mesh->min, 0.5);

    static Sphere testSphere;

    static Obb testObb;

    calcObb(testObb, &(mesh->vertices[0][0]), mesh->vertices.size(), 12, 4);

    auto& o = testObb.mtx;

    mesh->OBB = 
    {
      o[0 ], o[1 ], o[2 ], o[3 ],
      o[4 ], o[5 ], o[6 ], o[7 ],
      o[8 ], o[9 ], o[10], o[11],
      o[12], o[13], o[14], o[15]
    };
  }
  else
  {
    //AABB
    mesh->min = mesh->max = mesh->vertices[0];
    for(unsigned i = 0; i < mesh->vertices.size(); ++i)
    {
      glm::vec3 current = mesh->vertices[i];
      mesh->min = glm::min(mesh->min, current);
      mesh->max = glm::max(mesh->max, current);
    }

    //ritters
    glmSphere placeholder;
    RitterIterative(placeholder, mesh->vertices.data(), mesh->vertices.size());
    mesh->radiusRitters = placeholder.r;
    mesh->posRitters = placeholder.c;

    //larsson
    //skipped this crap too, don't know what to do about normals

    //PCA
    RitterEigenSphere(placeholder, mesh->vertices.data(), mesh->vertices.size());
    mesh->radiusPCA = placeholder.r;
    mesh->posPCA = placeholder.c;

    //PCA Ellipsoid
    //Screw this noise, there's no guidence for this

    //centroid
    mesh->radiusCentroid = glm::distance(mesh->max, mesh->min) * .5f;

		//OBB
		constexpr unsigned iterations = 4;
		constexpr float deltaAngle = float(bx::kPiHalf / iterations);

		float area = AreaCube(
			mesh->max.x - mesh->min.x, 
			mesh->max.y - mesh->min.y,
			mesh->max.z - mesh->min.z
		);

		glm::mat4 m;
		mesh->OBB = glm::mat4(0);
		mesh->OBB[0][0] = (mesh->max.x - mesh->min.x) * .5f;
		mesh->OBB[1][1] = (mesh->max.y - mesh->min.y) * .5f;
		mesh->OBB[2][2] = (mesh->max.z - mesh->min.z) * .5f;
		mesh->OBB[3][0] = (mesh->min.x + mesh->max.x) * .5f;
		mesh->OBB[3][1] = (mesh->min.y + mesh->max.y) * .5f;
		mesh->OBB[3][2] = (mesh->min.z + mesh->max.z) * .5f;
		mesh->OBB[3][3] = 1.0f;

		glm::mat4 mT;

		for (auto [i, xAngle] = std::tuple{ 0, 0.f }; i < iterations; ++i, xAngle += deltaAngle)
			for (auto [j, yAngle] = std::tuple{ 0, 0.f }; j < iterations; ++j, yAngle += deltaAngle)
				for (auto [k, zAngle] = std::tuple{ 0, 0.f }; k < iterations; ++k, zAngle += deltaAngle)
				{
					m = glm::orientate4(glm::vec3(xAngle, yAngle, zAngle));
					mT = glm::transpose(m);

					glm::vec3 min, max, current;
					min = max = mT * glm::vec4(mesh->vertices[0], 1.f);
					for (unsigned i = 1; i < mesh->vertices.size(); ++i)
					{
						current = mT * glm::vec4(mesh->vertices[i], 1.f);
						min = glm::min(min, current);
						max = glm::max(max, current);
					}
					//Calculate area
					float newArea = AreaCube(
						max.x - min.x,
						max.y - min.y,
						max.z - min.z);

					if (newArea < area)
					{
						area = newArea;
						glm::mat4 newOBB = glm::mat4(0);
						newOBB[0][0] = (max.x - min.x) * .5f;
						newOBB[1][1] = (max.y - min.y) * .5f;
						newOBB[2][2] = (max.z - min.z) * .5f;
						newOBB[3][0] = (min.x + max.x) * .5f;
						newOBB[3][1] = (min.y + max.y) * .5f;
						newOBB[3][2] = (min.z + max.z) * .5f;
						newOBB[3][3] = 1.0f;
						mesh->OBB = m * newOBB;
					}
				}


    mesh->pos = glm::mix(mesh->max, mesh->min, 0.5);
  }

	mesh->vertices.clear();

}

void Mesh::load(bx::ReaderSeekerI* _reader, bool _ramcopy)
{
#define BGFX_CHUNK_MAGIC_VB  BX_MAKEFOURCC('V', 'B', ' ', 0x1)
#define BGFX_CHUNK_MAGIC_VBC BX_MAKEFOURCC('V', 'B', 'C', 0x0)
#define BGFX_CHUNK_MAGIC_IB  BX_MAKEFOURCC('I', 'B', ' ', 0x0)
#define BGFX_CHUNK_MAGIC_IBC BX_MAKEFOURCC('I', 'B', 'C', 0x1)
#define BGFX_CHUNK_MAGIC_PRI BX_MAKEFOURCC('P', 'R', 'I', 0x0)

	using namespace bx;
	using namespace bgfx;
	
	Group group;
	
	bx::AllocatorI* allocator = entry::getAllocator();
	
	uint32_t chunk;
	bx::Error err;
	while (4 == bx::read(_reader, chunk, &err)
		   &&     err.isOk() )
	{
		switch (chunk)
		{
			case BGFX_CHUNK_MAGIC_VB:
			{
				read(_reader, group.m_sphere);
				read(_reader, group.m_aabb);
				read(_reader, group.m_obb);
				
				read(_reader, m_layout);
				
				uint16_t stride = m_layout.getStride();
				
				read(_reader, group.m_numVertices);
				const bgfx::Memory* mem = bgfx::alloc(group.m_numVertices*stride);
				read(_reader, mem->data, mem->size);
				if ( _ramcopy )
				{
					group.m_vertices = (uint8_t*)BX_ALLOC(allocator, group.m_numVertices*stride);
					bx::memCopy(group.m_vertices, mem->data, mem->size);
					int i = 0;
          vertices.reserve(group.m_numVertices * stride);
					while (i < group.m_numVertices * stride)
					{
            vertices.push_back(glm::vec3(
              *((float*)(group.m_vertices + i + 0)),
              *((float*)(group.m_vertices + i + 4)),
              *((float*)(group.m_vertices + i + 8))
            ));
						i += stride;
					}
          CalculateColliderInfo(this);

				}
				group.m_vbh = bgfx::createVertexBuffer(mem, m_layout);
			}
				break;
				
			case BGFX_CHUNK_MAGIC_VBC:
			{
				read(_reader, group.m_sphere);
				read(_reader, group.m_aabb);
				read(_reader, group.m_obb);
				
				read(_reader, m_layout);
				
				uint16_t stride = m_layout.getStride();
				
				read(_reader, group.m_numVertices);
				
				const bgfx::Memory* mem = bgfx::alloc(group.m_numVertices*stride);
				
				uint32_t compressedSize;
				bx::read(_reader, compressedSize);
				
				void* compressedVertices = BX_ALLOC(allocator, compressedSize);
				bx::read(_reader, compressedVertices, compressedSize);
				
				meshopt_decodeVertexBuffer(mem->data, group.m_numVertices, stride, (uint8_t*)compressedVertices, compressedSize);
				
				BX_FREE(allocator, compressedVertices);

				if ( _ramcopy )
				{
					group.m_vertices = (uint8_t*)BX_ALLOC(allocator, group.m_numVertices*stride);
					bx::memCopy(group.m_vertices, mem->data, mem->size);
				}
				
				group.m_vbh = bgfx::createVertexBuffer(mem, m_layout);
			}
				break;
				
			case BGFX_CHUNK_MAGIC_IB:
			{
				read(_reader, group.m_numIndices);
				const bgfx::Memory* mem = bgfx::alloc(group.m_numIndices*2);
				read(_reader, mem->data, mem->size);
				if ( _ramcopy )
				{
					group.m_indices = (uint16_t*)BX_ALLOC(allocator, group.m_numIndices*2);
					bx::memCopy(group.m_indices, mem->data, mem->size);
				}

				group.m_ibh = bgfx::createIndexBuffer(mem);
			}
				break;
				
			case BGFX_CHUNK_MAGIC_IBC:
			{
				bx::read(_reader, group.m_numIndices);
				
				const bgfx::Memory* mem = bgfx::alloc(group.m_numIndices*2);
				
				uint32_t compressedSize;
				bx::read(_reader, compressedSize);
				
				void* compressedIndices = BX_ALLOC(allocator, compressedSize);
				
				bx::read(_reader, compressedIndices, compressedSize);
				
				meshopt_decodeIndexBuffer(mem->data, group.m_numIndices, 2, (uint8_t*)compressedIndices, compressedSize);
				
				BX_FREE(allocator, compressedIndices);
				
				if ( _ramcopy )
				{
					group.m_indices = (uint16_t*)BX_ALLOC(allocator, group.m_numIndices*2);
					bx::memCopy(group.m_indices, mem->data, mem->size);
				}
				
				group.m_ibh = bgfx::createIndexBuffer(mem);
			}
				break;
				
			case BGFX_CHUNK_MAGIC_PRI:
			{
				uint16_t len;
				read(_reader, len);
				
				stl::string material;
				material.resize(len);
				read(_reader, const_cast<char*>(material.c_str() ), len);
				
				uint16_t num;
				read(_reader, num);
				
				for (uint32_t ii = 0; ii < num; ++ii)
				{
					read(_reader, len);
					
					stl::string name;
					name.resize(len);
					read(_reader, const_cast<char*>(name.c_str() ), len);
					
					Primitive prim;
					read(_reader, prim.m_startIndex);
					read(_reader, prim.m_numIndices);
					read(_reader, prim.m_startVertex);
					read(_reader, prim.m_numVertices);
					read(_reader, prim.m_sphere);
					read(_reader, prim.m_aabb);
					read(_reader, prim.m_obb);
					
					group.m_prims.push_back(prim);
				}
				
				m_groups.push_back(group);
				group.reset();
			}
				break;
				
			default:
				DBG("%08x at %d", chunk, bx::skip(_reader, 0) );
				break;
		}
	}
}

void Mesh::unload()
{
	bx::AllocatorI* allocator = entry::getAllocator();
	
	for (GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it)
	{
		const Group& group = *it;
		bgfx::destroy(group.m_vbh);
		
		if (bgfx::isValid(group.m_ibh) )
		{
			bgfx::destroy(group.m_ibh);
		}
		
		if ( NULL != group.m_vertices )
		{
			BX_FREE(allocator, group.m_vertices);
		}

		if ( NULL != group.m_indices )
		{
			BX_FREE(allocator, group.m_indices);
		}
	}
	m_groups.clear();
}

void Mesh::submit(bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state) const
{
	if (BGFX_STATE_MASK == _state)
	{
		_state = 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		;
	}
	
	bgfx::setTransform(_mtx);
	bgfx::setState(_state);
	
	for (GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it)
	{
		const Group& group = *it;
		
		bgfx::setIndexBuffer(group.m_ibh);
		bgfx::setVertexBuffer(0, group.m_vbh);
		bgfx::submit(_id, _program, 0, 
      #ifdef __ANDROID_API__ 
      uint8_t
      #endif
      (it != itEnd-1));
	}
}

void Mesh::submit(const MeshState*const* _state, uint8_t _numPasses, const float* _mtx, uint16_t _numMatrices) const
{
	uint32_t cached = bgfx::setTransform(_mtx, _numMatrices);
	
	for (uint32_t pass = 0; pass < _numPasses; ++pass)
	{
		bgfx::setTransform(cached, _numMatrices);
		
		const MeshState& state = *_state[pass];
		bgfx::setState(state.m_state);
		
		for (uint8_t tex = 0; tex < state.m_numTextures; ++tex)
		{
			const MeshState::Texture& texture = state.m_textures[tex];
			bgfx::setTexture(texture.m_stage
							 , texture.m_sampler
							 , texture.m_texture
							 , texture.m_flags
							 );
		}
		
		for (GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd; ++it)
		{
			const Group& group = *it;
			
			bgfx::setIndexBuffer(group.m_ibh);
			bgfx::setVertexBuffer(0, group.m_vbh);
			bgfx::submit(state.m_viewId, state.m_program, 0,
        #ifdef __ANDROID_API__ 
        uint8_t
        #endif
        (it != itEnd-1));
		}
	}
}

Mesh* meshLoad(bx::ReaderSeekerI* _reader, bool _ramcopy)
{
	Mesh* mesh = new Mesh;
	mesh->load(_reader, _ramcopy);
	return mesh;
}

Mesh* meshLoad(const char* _filePath, bool _ramcopy)
{
	bx::FileReaderI* reader = entry::getFileReader();
	if (bx::open(reader, _filePath) )
	{
		Mesh* mesh = meshLoad(reader, _ramcopy);
		bx::close(reader);
		return mesh;
	}

	return NULL;
}

void meshUnload(Mesh* _mesh)
{
	_mesh->unload();
	delete _mesh;
}

MeshState* meshStateCreate()
{
	MeshState* state = (MeshState*)BX_ALLOC(entry::getAllocator(), sizeof(MeshState) );
	return state;
}

void meshStateDestroy(MeshState* _meshState)
{
	BX_FREE(entry::getAllocator(), _meshState);
}

void meshSubmit(const Mesh* _mesh, bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state)
{
	_mesh->submit(_id, _program, _mtx, _state);
}

void meshSubmit(const Mesh* _mesh, const MeshState*const* _state, uint8_t _numPasses, const float* _mtx, uint16_t _numMatrices)
{
	_mesh->submit(_state, _numPasses, _mtx, _numMatrices);
}

Args::Args(int _argc, const char* const* _argv)
	: m_type(bgfx::RendererType::Count)
	, m_pciId(BGFX_PCI_ID_NONE)
{
	bx::CommandLine cmdLine(_argc, (const char**)_argv);

	if (cmdLine.hasArg("gl") )
	{
		m_type = bgfx::RendererType::OpenGL;
	}
	else if (cmdLine.hasArg("vk") )
	{
		m_type = bgfx::RendererType::Vulkan;
	}
	else if (cmdLine.hasArg("noop") )
	{
		m_type = bgfx::RendererType::Noop;
	}
	else if (BX_ENABLED(BX_PLATFORM_WINDOWS|BX_PLATFORM_WINRT|BX_PLATFORM_XBOXONE) )
	{
		if (cmdLine.hasArg("d3d9") )
		{
			m_type = bgfx::RendererType::Direct3D9;
		}
		else if (cmdLine.hasArg("d3d11") )
		{
			m_type = bgfx::RendererType::Direct3D11;
		}
		else if (cmdLine.hasArg("d3d12") )
		{
			m_type = bgfx::RendererType::Direct3D12;
		}
	}
	else if (BX_ENABLED(BX_PLATFORM_OSX) )
	{
		if (cmdLine.hasArg("mtl") )
		{
			m_type = bgfx::RendererType::Metal;
		}
	}

	if (cmdLine.hasArg("amd") )
	{
		m_pciId = BGFX_PCI_ID_AMD;
	}
	else if (cmdLine.hasArg("nvidia") )
	{
		m_pciId = BGFX_PCI_ID_NVIDIA;
	}
	else if (cmdLine.hasArg("intel") )
	{
		m_pciId = BGFX_PCI_ID_INTEL;
	}
	else if (cmdLine.hasArg("sw") )
	{
		m_pciId = BGFX_PCI_ID_SOFTWARE_RASTERIZER;
	}
}
