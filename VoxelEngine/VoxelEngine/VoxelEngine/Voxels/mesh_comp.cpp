#include "stdafx.h"
#include "mesh_comp.h"

using namespace std;

MeshComp::MeshComp(const std::vector<Vertex>& vertices,
	const std::vector<unsigned int>& indices,
	const std::vector<Texture>& textures)
{
	mesh = new Mesh(vertices, indices, textures);
}

MeshComp::~MeshComp()
{
	delete mesh;
}

Component* MeshComp::Clone() const
{
	return new MeshComp(mesh->GetVertices(), mesh->GetIndices(), mesh->GetTextures());
}
