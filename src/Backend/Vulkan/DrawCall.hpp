#pragma once
#include <cassert>
#include <Core/DrawCall.hpp>
#include <Backend/Vulkan/Mesh.hpp>
#include <Backend/Vulkan/Job.hpp>
#include <Backend/Vulkan/Camera.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Texture.hpp>


namespace MMPEngine::Backend::Vulkan
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	private:
		class InternalTaskContext;

		class Pass final
		{
		public:
			explicit Pass(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
			Pass(const Pass&) = delete;
			Pass(Pass&&) noexcept;
			Pass& operator=(const Pass&) = delete;
			Pass& operator=(Pass&&) noexcept = delete;
			~Pass();

			const std::vector<VkAttachmentDescription>& GetAttachmentDescriptions() const;
		private:
			VkRenderPass _renderPass = VK_NULL_HANDLE;
			VkFramebuffer _frameBuffer = VK_NULL_HANDLE;
			std::shared_ptr<Wrapper::Device> _device;

			std::vector<VkAttachmentDescription> _attachmentDescriptions;
		};

		class InternalTaskContext final : public Core::TaskContext, public std::enable_shared_from_this<InternalTaskContext>
		{
		public:
			std::shared_ptr<DrawCallsJob> job;
			std::vector<std::shared_ptr<IColorTargetTexture>> colorRenderTargets;
			std::vector<VkImageView> attachments;
			std::shared_ptr<IDepthStencilTexture> depthStencil;
		};

		class BeginPass final : public Task<InternalTaskContext>
		{
		public:
			BeginPass(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class EndPass final : public Task<InternalTaskContext>
		{
		public:
			EndPass(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class Start final : public Task<InternalTaskContext>
		{
		public:
			Start(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<BeginPass> _beginPassTask;
			std::vector<std::shared_ptr<Core::BaseTask>> _memoryBarrierTasks;
		};

		/*template<typename TCoreMaterial>
		class IterationJob final : public Iteration, public Vulkan::Job<TCoreMaterial>
		{
		private:

			using PsoDescType = std::conditional_t<std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>, D3D12_GRAPHICS_PIPELINE_STATE_DESC, void>;
			using PsoStreamType = std::conditional_t<std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>, CD3Vulkan_PIPELINE_STATE_STREAM, void>;

			class TaskContext final : public Core::TaskContext
			{
			public:
				TaskContext(const std::shared_ptr<IterationJob>& job);
				std::shared_ptr<IterationJob> job;
				std::shared_ptr<Core::Mesh::Renderer> meshRenderer;
				std::shared_ptr<const Vulkan::Mesh> mesh;
			};

			class InitTask final : public Task<TaskContext>
			{
			public:
				InitTask(const std::shared_ptr<TaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

			class ExecutionTask final : public Task<TaskContext>
			{
			private:
				class Impl final : public Task<TaskContext>
				{
				public:
					Impl(const std::shared_ptr<TaskContext>& ctx);
					void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
				};
			public:
				ExecutionTask(const std::shared_ptr<TaskContext>& ctx);
				void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
			private:
				std::shared_ptr<Core::BaseTask> _applyMaterial;
				std::shared_ptr<Core::BaseTask> _setPipelineState;
				std::shared_ptr<Core::BaseTask> _impl;
				std::vector<std::shared_ptr<Core::BaseTask>> _switchStateTasks;
			};

		public:
			IterationJob(const std::shared_ptr<Core::Camera>& camera, const Item& item);
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
			std::shared_ptr<Core::BaseTask> CreateExecutionTask() override;
		private:
			std::shared_ptr<Core::Camera> _camera;
			Item _item;
		};*/


	public:
		DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
		~DrawCallsJob() override;
		DrawCallsJob(const DrawCallsJob&) = delete;
		DrawCallsJob(DrawCallsJob&&) noexcept = delete;
		DrawCallsJob& operator=(const DrawCallsJob&) = delete;
		DrawCallsJob& operator=(DrawCallsJob&&) noexcept = delete;
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsFinish() override;
	private:
		std::shared_ptr<InternalTaskContext> BuildInternalContext();
		const Pass* GetOrCreatePass(const std::shared_ptr<InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
		std::vector<std::tuple<std::vector<VkImageView>, Pass>> _cachedPasses;
	};


	/*template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<Core::Camera>& camera, const Item& item)
		: _camera(camera), _item(item)
	{
	}

	template<typename TCoreMaterial>
	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<TaskContext>(std::dynamic_pointer_cast<IterationJob>(shared_from_this()));
		return std::make_shared<InitTask>(ctx);
	}

	template<typename TCoreMaterial>
	std::shared_ptr<Core::BaseTask> Camera::DrawCallsJob::IterationJob<TCoreMaterial>::CreateExecutionTask()
	{
		const auto ctx = std::make_shared<TaskContext>(std::dynamic_pointer_cast<IterationJob>(shared_from_this()));
		return std::make_shared<ExecutionTask>(ctx);
	}

	template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::InitTask(const std::shared_ptr<TaskContext>& ctx) : Task<typename Vulkan::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>(ctx)
	{
	}

	template<typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<typename Vulkan::Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext>::Run(stream);

		const auto ctx = this->GetTaskContext();
		const auto job = ctx->job;
		assert(job);

		const auto renderer = job->_item.renderer;
		const auto material = std::dynamic_pointer_cast<TCoreMaterial>(job->_item.material);
		const auto camera = job->_camera;

		job->BakeMaterialParameters(this->_specificGlobalContext, material->GetParameters(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		PsoDescType psoDesc = {};
		psoDesc.pRootSignature = job->_rootSignature.Get();
		psoDesc.SampleDesc = std::dynamic_pointer_cast<IColorTargetTexture>(camera->GetTarget().color.begin()->tex->GetUnderlyingTexture())->GetSampleDesc();
		psoDesc.SampleMask = UINT_MAX;

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

			switch (const auto topology = ctx->mesh->GetTopology())
			{
			case Core::GeometryPrototype::Topology::Triangles:
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				break;
			default:
				break;
			}

			const auto& vertexLayout = ctx->mesh->GetVertexInputLayout();
			psoDesc.InputLayout = { vertexLayout.data(), static_cast<std::uint32_t>(vertexLayout.size()) };
		}

		const auto& settings = material->GetSettings();

		psoDesc.RasterizerState = {
			(settings.fillMode == Core::RenderingMaterial::Settings::FillMode::Solid ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME),
			(settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Back
				? D3D12_CULL_MODE_BACK :
				(settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Front ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE)),
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
				switch (func)
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

		const auto getBlendOp = [](Core::RenderingMaterial::Settings::Blend::Op op) -> auto
			{
				switch (op)
				{
				case Core::RenderingMaterial::Settings::Blend::Op::Add:
					return D3D12_BLEND_OP_ADD;
				case Core::RenderingMaterial::Settings::Blend::Op::Max:
					return D3D12_BLEND_OP_MAX;
				case Core::RenderingMaterial::Settings::Blend::Op::Min:
					return D3D12_BLEND_OP_MIN;
				case Core::RenderingMaterial::Settings::Blend::Op::RevSub:
					return D3D12_BLEND_OP_REV_SUBTRACT;
				case Core::RenderingMaterial::Settings::Blend::Op::Sub:
					return D3D12_BLEND_OP_SUBTRACT;
				default:
					return D3D12_BLEND_OP_ADD;
				}
			};

		const auto getBlendFactor = [](Core::RenderingMaterial::Settings::Blend::Factor factor) -> auto
			{
				switch (factor)
				{
				case Core::RenderingMaterial::Settings::Blend::Factor::Zero:
					return D3D12_BLEND_ZERO;
				case Core::RenderingMaterial::Settings::Blend::Factor::DstAlpha:
					return D3D12_BLEND_DEST_ALPHA;
				case Core::RenderingMaterial::Settings::Blend::Factor::DstColor:
					return D3D12_BLEND_DEST_COLOR;
				case Core::RenderingMaterial::Settings::Blend::Factor::One:
					return D3D12_BLEND_ONE;
				case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcAlpha:
					return D3D12_BLEND_INV_SRC_ALPHA;
				case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstAlpha:
					return D3D12_BLEND_INV_DEST_ALPHA;
				case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstColor:
					return D3D12_BLEND_INV_DEST_COLOR;
				case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcColor:
					return D3D12_BLEND_INV_SRC_COLOR;
				case Core::RenderingMaterial::Settings::Blend::Factor::SrcAlpha:
					return D3D12_BLEND_SRC_ALPHA;
				case Core::RenderingMaterial::Settings::Blend::Factor::SrcColor:
					return D3D12_BLEND_SRC_COLOR;
				default:
					return D3D12_BLEND_ONE;
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

		if (settings.stencil.has_value())
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

		psoDesc.BlendState = CD3Vulkan_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.AlphaToCoverageEnable = settings.alphaToCoverage == Core::RenderingMaterial::Settings::AlphaToCoverage::On;
		psoDesc.BlendState.IndependentBlendEnable = !std::equal(settings.blend.targets.begin() + 1, settings.blend.targets.end(), settings.blend.targets.begin());

		for (std::size_t i = 0; i < settings.blend.targets.size(); ++i)
		{
			const auto& bt = settings.blend.targets.at(i);
			psoDesc.BlendState.RenderTarget[i].BlendEnable = bt.op != Core::RenderingMaterial::Settings::Blend::Op::None;
			psoDesc.BlendState.RenderTarget[i].BlendOp = getBlendOp(bt.op);
			psoDesc.BlendState.RenderTarget[i].SrcBlend = getBlendFactor(bt.src);
			psoDesc.BlendState.RenderTarget[i].DestBlend = getBlendFactor(bt.dst);

			psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = psoDesc.BlendState.RenderTarget[i].BlendOp;
			psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = psoDesc.BlendState.RenderTarget[i].SrcBlend;
			psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = psoDesc.BlendState.RenderTarget[i].DestBlend;

			psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = 0;

			if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Red))
			{
				psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;
			}

			if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Green))
			{
				psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
			}

			if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Blue))
			{
				psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
			}

			if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Alpha))
			{
				psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
			}
		}

		psoDesc.NumRenderTargets = static_cast<decltype(psoDesc.NumRenderTargets)>(camera->GetTarget().color.size());

		for (std::size_t i = 0; i < camera->GetTarget().color.size(); ++i)
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

		Microsoft::WRL::ComPtr<ID3D12Device2> castedDevice = nullptr;
		this->_specificGlobalContext->device->QueryInterface(IID_PPV_ARGS(castedDevice.GetAddressOf()));
		assert(castedDevice != nullptr);

		castedDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&job->_pipelineState));
		assert(job->_pipelineState);
	}

	template <typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::ExecutionTask(const std::shared_ptr<TaskContext>& ctx) : Task<TaskContext>(ctx)
	{
		const auto baseJobCtx = std::make_shared<Vulkan::BaseJob::TaskContext>();
		baseJobCtx->job = std::dynamic_pointer_cast<Vulkan::BaseJob>(ctx->job);

		_applyMaterial = std::make_shared<Vulkan::BaseJob::ApplyParametersTask>(baseJobCtx);
		_setPipelineState = std::make_shared<typename Vulkan::Job<TCoreMaterial>::SetPipelineStateTask>(baseJobCtx);

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			const auto& ibInfo = ctx->mesh->GetIndexBufferInfo();
			_switchStateTasks.push_back(std::dynamic_pointer_cast<Vulkan::BaseEntity>(ibInfo.ptr->GetUnderlyingBuffer())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));

			const auto& allBufferInfos = ctx->mesh->GetAllVertexBufferInfos();

			for (const auto& s2vbs : allBufferInfos)
			{
				for (const auto& vbInfo : s2vbs.second)
				{
					_switchStateTasks.push_back(std::dynamic_pointer_cast<Vulkan::BaseEntity>(vbInfo.ptr->GetUnderlyingBuffer())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
				}
			}

			_impl = std::make_shared<Impl>(ctx);
		}
	}

	template <typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<TaskContext>::OnScheduled(stream);

		stream->Schedule(_setPipelineState);
		stream->Schedule(_applyMaterial);

		for (const auto& sst : _switchStateTasks)
		{
			stream->Schedule(sst);
		}

		stream->Schedule(_impl);
	}

	template <typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::Impl::Impl(const std::shared_ptr<TaskContext>& ctx) : Task<TaskContext>(ctx)
	{
	}

	template <typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::Impl::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<TaskContext>::Run(stream);

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			const auto ctx = this->GetTaskContext();
			const auto& ibv = ctx->mesh->GetIndexBufferView();

			this->_specificStreamContext->PopulateCommandsInBuffer()->IASetIndexBuffer(&ibv);

			const auto& vbvs = ctx->mesh->GetVertexBufferViews();
			this->_specificStreamContext->PopulateCommandsInBuffer()->IASetVertexBuffers(
				0,
				static_cast<std::uint32_t>(vbvs.size()), vbvs.data());

			const auto topology = ctx->mesh->GetTopology();
			D3D12_PRIMITIVE_TOPOLOGY d3d12Topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			switch (topology)
			{
			case Core::GeometryPrototype::Topology::Triangles:
				d3d12Topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			default:
				break;
			}

			this->_specificStreamContext->PopulateCommandsInBuffer()->IASetPrimitiveTopology(d3d12Topology);

			const auto& subsets = ctx->mesh->GetSubsets();

			for (const auto& ss : subsets)
			{
				this->_specificStreamContext->PopulateCommandsInBuffer()->DrawIndexedInstanced(
					ss.indexCount,
					static_cast<std::uint32_t>(ctx->meshRenderer->GetSettings().dynamicData.instancesCount),
					ss.indexStart,
					ss.baseVertex,
					0
				);
			}
		}
	}

	template<typename TCoreMaterial>
	inline Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext::TaskContext(const std::shared_ptr<IterationJob>& job) : job(job)
	{
		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			meshRenderer = std::dynamic_pointer_cast<Core::Mesh::Renderer>(job->_item.renderer);
			mesh = std::dynamic_pointer_cast<const Vulkan::Mesh>(meshRenderer->GetMesh()->GetUnderlyingMesh());
		}
	}*/
}
