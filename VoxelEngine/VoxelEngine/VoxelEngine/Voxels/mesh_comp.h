#pragma once
#include "component.h"
#include "mesh.h"

// wrapper around Mesh that allows it to be a component
typedef class MeshComp : public Component
{
public:
	MeshComp(const std::vector<Vertex>& vertices, 
		const std::vector<unsigned int>& indices, 
		const std::vector<Texture>& textures);
	~MeshComp();

	Component* Clone() const;

	static const ComponentType ctype = ComponentType::cMesh;
private:
	MeshComp() = delete;
	Mesh* mesh;
}MeshComp, *MeshCompPtr;