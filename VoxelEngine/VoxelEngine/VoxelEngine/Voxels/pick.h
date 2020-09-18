#pragma once

void raycast(glm::vec3 origin, glm::vec3 direction, float radius, 
	std::function<bool(glm::vec3, Block, glm::vec3)> callback);