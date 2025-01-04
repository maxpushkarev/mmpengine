#include <Core/DrawCall.hpp>

namespace MMPEngine::Core
{
	Camera::DrawCallsJob::DrawCallsJob(const std::shared_ptr<Camera>& camera, std::vector<Item>&& items) : _camera(camera), _items(std::move(items))
	{
	}

}