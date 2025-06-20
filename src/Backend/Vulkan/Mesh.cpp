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
		const auto tc = std::make_shared<InitTaskContext>();
		tc->mesh = std::dynamic_pointer_cast<Mesh>(shared_from_this());
		return std::make_shared<InitTask>(tc);
	}

	const std::vector<VkVertexInputBindingDescription>& Mesh::GetVertexBindingDescriptions() const
	{
		return _bindingDescriptions;
	}

	VkIndexType Mesh::GetIndexType() const
	{
		return _indexType;	
	}

	std::shared_ptr<Vulkan::Buffer> Mesh::GetIndexBuffer() const
	{
		return _indexBuffer;
	}

	const std::vector<VkBuffer>& Mesh::GetVertexBuffers() const
	{
		return _vertexBuffers;
	}

	const std::vector<VkVertexInputAttributeDescription>& Mesh::GetVertexAttributeDescriptions() const
	{
		return _attributeDescriptions;
	}

	const std::vector<VkDeviceSize>& Mesh::GetVertexBuffersOffsets() const
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



	Mesh::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void Mesh::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto mesh = GetTaskContext()->mesh;
		assert(mesh);

		std::uint32_t bindingIndex = 0;
		for (const auto& vbInfos : mesh->_vertexBufferInfos)
		{
			for (std::size_t semanticIndex = 0; semanticIndex < vbInfos.second.size(); ++semanticIndex)
			{
				const auto& vbInfo = vbInfos.second.at(semanticIndex);
				const auto vb = std::dynamic_pointer_cast<Buffer>(vbInfo.ptr->GetUnderlyingBuffer());
				assert(vb);
				mesh->_vertexBuffers.push_back(vb->GetDescriptorBufferInfo().buffer);

				VkVertexInputBindingDescription binding{};
				binding.binding = bindingIndex++;
				binding.stride = static_cast<std::uint32_t>(vbInfo.stride);
				binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				VkVertexInputAttributeDescription attr {};

				attr.location = binding.binding;
				attr.binding = binding.binding;
				attr.offset = 0;
				attr.format = GetVertexBufferFormat(vbInfo.format);

				mesh->_bindingDescriptions.push_back(binding);
				mesh->_attributeDescriptions.push_back(attr);
			}
		}

		mesh->_vertexBufferOffsets.resize(mesh->_vertexBuffers.size(), 0);

		const auto& ibInfo = mesh->_indexBufferInfo;
		const auto ib = std::dynamic_pointer_cast<Buffer>(ibInfo.ptr->GetUnderlyingBuffer());
		assert(ib);

		mesh->_indexType = (ibInfo.format == Core::IndexBufferPrototype::Format::Uint16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		mesh->_indexBuffer = ib;
	}


	VkFormat Mesh::GetVertexBufferFormat(Core::VertexBufferPrototype::Format format)
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
