#pragma once
#include <Core/Mesh.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Core/Node.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Mesh final : public Core::Mesh
	{
	public:
		Mesh(Core::GeometryPrototype&& proto);

		class Renderer final : public Core::Mesh::Renderer
		{
		public:
			Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node);
		protected:
			std::shared_ptr<Core::UniformBuffer<Data>> CreateUniformBuffer() override;
		};


	protected:
		std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
		std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;

	};
}
