#pragma once
#include <d3d12.h>
#include <Core/Mesh.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Core/Node.hpp>
#include <Core/Mesh.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Mesh final : public Core::Mesh
	{
	private:

		class InitTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<Mesh> mesh;
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

	public:
		Mesh(Core::GeometryPrototype&& proto);
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetVertexInputLayout() const;
		const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBufferViews() const;
		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const;

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
	private:
		static const char* GetSemanticsName(Core::VertexBufferPrototype::Semantics semantics);
		static DXGI_FORMAT GetVertexBufferFormat(Core::VertexBufferPrototype::Format format);

		std::vector<D3D12_INPUT_ELEMENT_DESC> _vertexInputLayout;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> _vertexBufferViews;
		D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	};
}
