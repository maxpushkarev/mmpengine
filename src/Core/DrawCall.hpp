#pragma once
#include <Core/Job.hpp>
#include <Core/Camera.hpp>
#include <Core/Geometry.hpp>

namespace MMPEngine::Core
{
	class Camera::DrawCallsJob : public Job<void>
	{
	protected:
		class SingleDrawCallJob : public Job<void>
		{
		};
	public:
		struct Item final
		{
			std::shared_ptr<IGeometryRenderer> renderer;
			std::shared_ptr<BaseMaterial> material;
		};
		DrawCallsJob(const std::shared_ptr<Camera>& camera, std::vector<Item>&& items);
	protected:

		std::shared_ptr<Camera> _camera;
		std::vector<Item> _items;
	};
}
