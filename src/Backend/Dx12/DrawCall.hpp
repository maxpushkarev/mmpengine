#pragma once
#include <cassert>
#include <Core/DrawCall.hpp>
#include <Backend/Dx12/Mesh.hpp>
#include <Backend/Dx12/d3dx12.h>
#include <Backend/Dx12/Job.hpp>
#include <Backend/Dx12/Camera.hpp>
#include <Backend/Dx12/Task.hpp>
#include <Backend/Dx12/Texture.hpp>


namespace MMPEngine::Backend::Dx12
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	private:
		class InternalTaskContext final : public Core::TaskContext
		{
		public:
			std::shared_ptr<DrawCallsJob> job;
			std::vector<std::shared_ptr<BaseEntity>> colorRenderTargets;
			std::shared_ptr<BaseEntity> depthStencil;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorRenderTargetHandles;
		};

		class PrepareRenderTargetsTask final : public Task<InternalTaskContext>
		{
		public:
			PrepareRenderTargetsTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class PrepareTask final : public Task<InternalTaskContext>
		{
		public:
			PrepareTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<PrepareRenderTargetsTask> _prepareRenderTargets;
			std::shared_ptr<BindDescriptorPoolsTask> _bindDescriptorHeaps;
			std::vector<std::shared_ptr<Core::BaseTask>> _switchStateTasks;
		};

		template<typename TCoreMaterial>
		class IterationJob final : public Iteration, public Dx12::Job<TCoreMaterial>
		{
		private:

			using PsoDescType = std::conditional_t<std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>, D3D12_GRAPHICS_PIPELINE_STATE_DESC, void>;
			using PsoStreamType = std::conditional_t<std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>, CD3DX12_PIPELINE_STATE_STREAM, void>;

			class TaskContext final : public Core::TaskContext
			{
			public:
				std::shared_ptr<IterationJob> job;
			};

			class InitTask final : public Task<TaskContext>
			{
			public:
				InitTask(const std::shared_ptr<TaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			IterationJob(const std::shared_ptr<Core::Camera>& camera, const Item& item);
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
			std::shared_ptr<Core::BaseTask> CreateExecutionTask() override;
		private:
			std::shared_ptr<Core::Camera> _camera;
			Item _item;
		};


	public:
		DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
	};


	template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<Core::Camera>& camera, const Item& item)
		: _camera(camera), _item(item)
	{
	}

	template<typename TCoreMaterial>
	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<TaskContext>();
		ctx->job = std::dynamic_pointer_cast<IterationJob>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	template<typename TCoreMaterial>
	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateExecutionTask()
	{
		return Core::BaseTask::kEmpty;
	}

	template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::InitTask(const std::shared_ptr<TaskContext>& ctx) : Task<typename Dx12::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>(ctx)
	{
	}

	template<typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<typename Dx12::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>::Run(stream);

		const auto job = this->GetTaskContext()->job;
		assert(job);

		const auto renderer = job->_item.renderer;
		const auto material = std::dynamic_pointer_cast<TCoreMaterial>(job->_item.material);
		const auto camera = job->_camera;

		job->BakeMaterialParameters(this->_specificGlobalContext, material->GetParameters());

		PsoDescType psoDesc = {};
		psoDesc.pRootSignature = job->_rootSignature.Get();
		psoDesc.SampleDesc = std::dynamic_pointer_cast<IColorTargetTexture>(camera->GetTarget().color.begin()->tex->GetUnderlyingTexture())->GetSampleDesc();

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			const auto vs = material->GetVertexShader();
			const auto ps = material->GetPixelShader();

			psoDesc.VS =
			{
				vs->GetCompiledBinaryData(),
				vs->GetCompiledBinaryLength()
			};
			psoDesc.PS =
			{
				ps->GetCompiledBinaryData(),
				ps->GetCompiledBinaryLength()
			};

			const auto meshRenderer = std::dynamic_pointer_cast<Core::Mesh::Renderer>(renderer);
			const auto mesh = std::dynamic_pointer_cast<const Dx12::Mesh>(meshRenderer->GetMesh()->GetUnderlyingMesh());

			switch (const auto topology = mesh->GetTopology())
			{
			case Core::GeometryPrototype::Topology::Triangles:
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				break;
			default:
				break;
			}

			const auto vertexLayout = mesh->GetVertexInputLayout();
			psoDesc.InputLayout = { vertexLayout.data(), static_cast<std::uint32_t>(vertexLayout.size()) };
		}

		const auto& settings = material->GetSettings();

		psoDesc.RasterizerState = {
			(settings.fillMode == Core::RenderingMaterial::Settings::FillMode::Solid ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME),
			(settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Back 
				? D3D12_CULL_MODE_BACK : 
				(settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Front ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE )),
			false,
			D3D12_DEFAULT_DEPTH_BIAS,
			D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			true,
			psoDesc.SampleDesc.Count > 1,
			false,
			0,
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};

		const auto getCmpFunc = [](Core::RenderingMaterial::Settings::Comparision comparision) -> auto
		{
			switch (comparision)
			{
			case Core::RenderingMaterial::Settings::Comparision::Always:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			case Core::RenderingMaterial::Settings::Comparision::Equal:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case Core::RenderingMaterial::Settings::Comparision::Greater:
				return D3D12_COMPARISON_FUNC_GREATER;
			case Core::RenderingMaterial::Settings::Comparision::GreaterEqual:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case Core::RenderingMaterial::Settings::Comparision::Less:
				return D3D12_COMPARISON_FUNC_LESS;
			case Core::RenderingMaterial::Settings::Comparision::LessEqual:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case Core::RenderingMaterial::Settings::Comparision::Never:
				return D3D12_COMPARISON_FUNC_NEVER;
			case Core::RenderingMaterial::Settings::Comparision::NotEqual:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			default:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			}
		};

		const auto getStencilOpFunc = [](Core::RenderingMaterial::Settings::Stencil::Op func) -> auto
		{
			switch(func)
			{
			case Core::RenderingMaterial::Settings::Stencil::Op::Keep:
				return D3D12_STENCIL_OP_KEEP;
			case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndSaturate:
				return D3D12_STENCIL_OP_DECR_SAT;
			case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndWrap:
				return D3D12_STENCIL_OP_DECR;
			case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndSaturate:
				return D3D12_STENCIL_OP_INCR_SAT;
			case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndWrap:
				return D3D12_STENCIL_OP_INCR;
			case Core::RenderingMaterial::Settings::Stencil::Op::Invert:
				return D3D12_STENCIL_OP_INVERT;
			case Core::RenderingMaterial::Settings::Stencil::Op::Replace:
				return D3D12_STENCIL_OP_REPLACE;
			case Core::RenderingMaterial::Settings::Stencil::Op::Zero:
				return D3D12_STENCIL_OP_ZERO;
			default:
				return D3D12_STENCIL_OP_KEEP;
			}
		};

		psoDesc.DepthStencilState = {
			(settings.depth.test == Core::RenderingMaterial::Settings::Depth::Test::On || settings.depth.write == Core::RenderingMaterial::Settings::Depth::Write::On),
			(settings.depth.write == Core::RenderingMaterial::Settings::Depth::Write::On ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO),
			getCmpFunc(settings.depth.comparision),
			false,
			D3D12_DEFAULT_STENCIL_READ_MASK,
			D3D12_DEFAULT_STENCIL_WRITE_MASK,
			{
				D3D12_STENCIL_OP_KEEP,
				D3D12_STENCIL_OP_KEEP,
				D3D12_STENCIL_OP_KEEP,
				D3D12_COMPARISON_FUNC_ALWAYS
			},
			{
				D3D12_STENCIL_OP_KEEP,
				D3D12_STENCIL_OP_KEEP,
				D3D12_STENCIL_OP_KEEP,
				D3D12_COMPARISON_FUNC_ALWAYS
			}
		};

		if(settings.stencil.has_value())
		{
			const auto& stencil = settings.stencil.value();
			psoDesc.DepthStencilState.StencilEnable = true;

			psoDesc.DepthStencilState.FrontFace.StencilFunc = getCmpFunc(stencil.front.comparision);
			psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = getStencilOpFunc(stencil.front.depthFail);
			psoDesc.DepthStencilState.FrontFace.StencilFailOp = getStencilOpFunc(stencil.front.stencilFail);
			psoDesc.DepthStencilState.FrontFace.StencilPassOp = getStencilOpFunc(stencil.front.stencilPass);

			psoDesc.DepthStencilState.BackFace.StencilFunc = getCmpFunc(stencil.back.comparision);
			psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = getStencilOpFunc(stencil.back.depthFail);
			psoDesc.DepthStencilState.BackFace.StencilFailOp = getStencilOpFunc(stencil.back.stencilFail);
			psoDesc.DepthStencilState.BackFace.StencilPassOp = getStencilOpFunc(stencil.back.stencilPass);
		}

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.NumRenderTargets = static_cast<decltype(psoDesc.NumRenderTargets)>(camera->GetTarget().color.size());

		for(std::size_t i = 0; i < camera->GetTarget().color.size(); ++i)
		{
			const auto rt = camera->GetTarget().color.at(i);
			psoDesc.RTVFormats[i] = std::dynamic_pointer_cast<IColorTargetTexture>(rt.tex->GetUnderlyingTexture())->GetRTVFormat();
		}

		psoDesc.DSVFormat = camera->GetTarget().depthStencil.tex
			? std::dynamic_pointer_cast<IDepthStencilTexture>(camera->GetTarget().depthStencil.tex->GetUnderlyingTexture())->GetDSVFormat()
			: DXGI_FORMAT_UNKNOWN;

		auto psoStream = PsoStreamType{ psoDesc };
		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
		streamDesc.pPipelineStateSubobjectStream = &psoStream;
		streamDesc.SizeInBytes = sizeof(psoStream);

		/*Microsoft::WRL::ComPtr<ID3D12Device2> castedDevice = nullptr;
		this->_specificGlobalContext->device->QueryInterface(IID_PPV_ARGS(castedDevice.GetAddressOf()));
		assert(castedDevice != nullptr);

		castedDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(job->_pipelineState.GetAddressOf()));*/
	}
}
