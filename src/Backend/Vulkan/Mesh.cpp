#include <cassert>
#include <Backend/Vulkan/Mesh.hpp>
#include <Backend/Vulkan/Buffer.hpp>

namespace MMPEngine::Backend::Vulkan
{
	Mesh::Mesh(Core::GeometryPrototype&& proto) : Core::Mesh(std::move(proto))
	{
	}

	std::shared_ptr<Core::VertexBuffer> Mesh::CreateVertexBuffer(const Core::VertexBufferPrototype* vbPrototype)
	{
		return std::make_shared<VertexBuffer>(Core::InputAssemblerBuffer::Settings{
		{vbPrototype->GetDataPtr()}, {vbPrototype->GetByteLength()}
			});
	}

	std::shared_ptr<Core::IndexBuffer> Mesh::CreateIndexBuffer(const Core::IndexBufferPrototype* ibPrototype)
	{
		return std::make_shared<IndexBuffer>(Core::InputAssemblerBuffer::Settings{
			{ibPrototype->GetDataPtr()}, { ibPrototype->GetByteLength() }
			});
	}

	std::shared_ptr<Core::BaseTask> Mesh::CreateInternalInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	std::shared_ptr<Core::BaseTask> Mesh::Renderer::CreateInternalInitializationTask()
	{
		const auto tc = std::make_shared<InitTaskContext>();
		tc->renderer = std::dynamic_pointer_cast<Renderer>(shared_from_this());
		return std::make_shared<InitTask>(tc);
	}

	const std::vector<VkVertexInputBindingDescription>& Mesh::Renderer::GetVertexBindingDescriptions() const
	{
		return _bindingDescriptions;
	}

	VkIndexType Mesh::Renderer::GetIndexType() const
	{
		return _indexType;	
	}

	std::shared_ptr<Vulkan::Buffer> Mesh::Renderer::GetIndexBufferPointer() const
	{
		return _indexBuffer;
	}

	const std::vector<VkBuffer>& Mesh::Renderer::GetVertexBuffers() const
	{
		return _vertexBuffers;
	}

	const std::vector<std::shared_ptr<Vulkan::Buffer>>& Mesh::Renderer::GetVertexBufferPointers() const
	{
		return _vertexBufferPointers;
	}

	const std::vector<VkVertexInputAttributeDescription>& Mesh::Renderer::GetVertexAttributeDescriptions() const
	{
		return _attributeDescriptions;
	}

	const std::vector<VkDeviceSize>& Mesh::Renderer::GetVertexBuffersOffsets() const
	{
		return _vertexBufferOffsets;
	}

	Mesh::Renderer::Renderer(const Settings& settings, const std::shared_ptr<Core::Mesh>& mesh, const std::shared_ptr<Core::Node>& node) : Core::Mesh::Renderer(settings, mesh, node)
	{
	}

	std::shared_ptr<Core::UniformBuffer<Core::Mesh::Renderer::Data>> Mesh::Renderer::CreateUniformBuffer()
	{
		return std::make_shared<UniformBuffer<Core::Mesh::Renderer::Data>>();
	}



	Mesh::Renderer::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void Mesh::Renderer::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto renderer = GetTaskContext()->renderer;
		assert(renderer);


		std::uint32_t bindingIndex = 0;

		renderer->ForEachAvailableVertexAttributes([&bindingIndex, &renderer](const auto& vbInfo, const auto&)
		{
				const auto vb = std::dynamic_pointer_cast<Buffer>(vbInfo.ptr->GetUnderlyingBuffer());
				assert(vb);
				renderer->_vertexBuffers.push_back(vb->GetDescriptorBufferInfo().buffer);
				renderer->_vertexBufferPointers.push_back(vb);

				VkVertexInputBindingDescription binding{};
				binding.binding = bindingIndex++;
				binding.stride = static_cast<std::uint32_t>(vbInfo.stride);
				binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				VkVertexInputAttributeDescription attrDesc{};

				attrDesc.location = binding.binding;
				attrDesc.binding = binding.binding;
				attrDesc.offset = 0;
				attrDesc.format = GetVertexBufferFormat(vbInfo.format);

				renderer->_bindingDescriptions.push_back(binding);
				renderer->_attributeDescriptions.push_back(attrDesc);
		});

		renderer->_vertexBufferOffsets.resize(renderer->_vertexBuffers.size(), 0);

		const auto& ibInfo = renderer->GetMesh()->GetIndexBufferInfo();
		const auto ib = std::dynamic_pointer_cast<Buffer>(ibInfo.ptr->GetUnderlyingBuffer());
		assert(ib);

		renderer->_indexType = (ibInfo.format == Core::IndexBufferPrototype::Format::Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		renderer->_indexBuffer = ib;
	}


	VkFormat Mesh::Renderer::GetVertexBufferFormat(Core::VertexBufferPrototype::Format format)
	{
		switch (format)
		{
		case Core::VertexBufferPrototype::Format::Float1:
			return VK_FORMAT_R32_SFLOAT;
		case Core::VertexBufferPrototype::Format::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case Core::VertexBufferPrototype::Format::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Core::VertexBufferPrototype::Format::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Core::VertexBufferPrototype::Format::Uint4:
			return VK_FORMAT_R32G32B32A32_UINT;
		default:
			throw Core::UnsupportedException("unsupported Vulkan vertex buffer format");
		}
	}

}
