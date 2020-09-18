#include "stdafx.h"
#include "NetPlayerState.h"
#include <NetDefines.h>

namespace Net
{
	VisiblePlayerState PlayerObject::GetVisibleState()
	{
		//ASSERT(states.size() > 0);
		if (states.size() == 0)
			return VisiblePlayerState();

		if (states.size() == 1)
		{
			bufferTimer.reset();
			return states[0];
		}

		// interpolate if > 1 state
		VisiblePlayerState state;
		state.pos = glm::mix(states[0].pos, states[1].pos, keyframe);
		state.front = glm::mix(states[0].front, states[1].front, keyframe);
		return state;
	}


	void PlayerObject::Update(float dt)
	{
		if (bufferTimer.elapsed() > bufferTime)
		{
			keyframe += dt * SERVER_NET_TICKS_PER_SECOND;
			while (keyframe >= 1.0f)
			{
				keyframe -= 1.0f;
				if (states.size() > 1) // don't pop if only state
					states.pop_front();
			}
		}
	}


	std::optional<PlayerObject> Net::PlayerWorld::GetObj(int id)
	{
		std::shared_lock lk(mtx);
		if (auto f = objects.find(id); f != objects.end())
			return f->second;
		return std::nullopt;
	}


	void PlayerWorld::RemoveObject(int id)
	{
		std::lock_guard lk(mtx);
		objects.erase(id);
	}


	void PlayerWorld::PushState(int id, VisiblePlayerState state)
	{
		std::lock_guard lk(mtx);
		auto& obj = objects[id];
		if (obj.states.size() < MAX_STATES_PER_OBJ)
			obj.states.push_back(state);
	}


	void PlayerWorld::PopState(int id)
	{
		std::lock_guard lk(mtx);
		objects[id].states.pop_front();
	}


	void PlayerWorld::UpdateStates(float dt)
	{
		std::lock_guard lk(mtx); // we locking it ALL down
		for (auto& [key, obj] : objects)
			obj.Update(dt);
	}
}
