#pragma once
#include <Core/Mesh.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Buffer.hpp>
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

			const std::vector<VkVertexInputBindingDescription>& GetVertexBindingDescriptions() const;
			const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributeDescriptions() const;
			VkIndexType GetIndexType() const;
			std::shared_ptr<Vulkan::Buffer> GetIndexBuffer() const;
			const std::vector<VkBuffer>& GetVertexBuffers() const;
			const std::vector<VkDeviceSize>& GetVertexBuffersOffsets() const;

		protected:
			std::shared_ptr<Core::UniformBuffer<Data>> CreateUniformBuffer() override;
			std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		private:

			class InitTaskContext final : public Core::TaskContext
			{
			public:
				std::shared_ptr<Renderer> renderer;
			};

			class InitTask final : public Task<InitTaskContext>
			{
			public:
				InitTask(const std::shared_ptr<InitTaskContext>& ctx);
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			static VkFormat GetVertexBufferFormat(Core::VertexBufferPrototype::Format format);

			std::vector<VkVertexInputBindingDescription> _bindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;

			VkIndexType _indexType = VK_INDEX_TYPE_UINT16;
			std::shared_ptr<Vulkan::Buffer> _indexBuffer;
			std::vector<VkBuffer> _vertexBuffers;
			std::vector<VkDeviceSize> _vertexBufferOffsets;
		};


	protected:
		std::shared_ptr<Core::BaseTask> CreateInternalInitializationTask() override;
		std::shared_ptr<Core::VertexBuffer> CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype) override;
		std::shared_ptr<Core::IndexBuffer> CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype) override;

	};
}
