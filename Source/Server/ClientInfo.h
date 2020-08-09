#pragma once

struct ClientInfo
{
	int clientID = -1;
	double time_since_heard = 0;

	struct
	{
		glm::vec3 pos{ 0, 0, 0 };
		glm::vec3 front{ 1, 0, 0 };
	}playerData;
};