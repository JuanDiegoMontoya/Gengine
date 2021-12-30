#pragma once
#include <string_view>
#include <utility/HashedString.h>
#include "../Mesh.h"

namespace GFX
{
	namespace MeshManager
	{
		MeshID CreateMeshBatched(std::string_view filename, hashed_string name);
		MeshID GetMeshBatched(hashed_string name);

		// TODO: function(s) to load skeletal meshes
	};
}