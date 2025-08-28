#pragma once
#include <cassert>
#include <Core/DrawCall.hpp>
#include <Backend/Vulkan/Mesh.hpp>
#include <Backend/Vulkan/Job.hpp>
#include <Backend/Vulkan/Camera.hpp>
#include <Backend/Vulkan/Task.hpp>
#include <Backend/Vulkan/Texture.hpp>
#include <Backend/Vulkan/Buffer.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob, public Vulkan::Job<Core::MeshMaterial>
	{
	private:
		class InternalTaskContext;

		class IterationImpl
		{
		protected:
			IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const Item& item);
			Item _item;
			DrawCallsJob* _drawCallsJob;
			std::vector<VkShaderModule> _shaderModules;
		};

		class Pass final
		{
		public:
			explicit Pass(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
			Pass(const Pass&) = delete;
			Pass(Pass&&) noexcept = delete;
			Pass& operator=(const Pass&) = delete;
			Pass& operator=(Pass&&) noexcept = delete;
			~Pass();
			VkRenderPass GetRenderPass() const;
			const std::vector<VkAttachmentDescription>& GetAttachmentDescriptions() const;

		private:
			VkRenderPass _renderPass = VK_NULL_HANDLE;
			std::shared_ptr<Wrapper::Device> _device;
			std::vector<VkAttachmentDescription> _attachmentDescriptions;
		};

		class FrameBuffer final
		{
		public:
			explicit FrameBuffer(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
			FrameBuffer(const FrameBuffer&) = delete;
			FrameBuffer(FrameBuffer&&) noexcept;
			FrameBuffer& operator=(const FrameBuffer&) = delete;
			FrameBuffer& operator=(FrameBuffer&&) noexcept = delete;
			~FrameBuffer();

			VkFramebuffer GetFrameBuffer() const;
		private:
			VkFramebuffer _frameBuffer = VK_NULL_HANDLE;
			std::shared_ptr<Wrapper::Device> _device;
		};

		class InternalTaskContext final : public Core::TaskContext, public std::enable_shared_from_this<InternalTaskContext>
		{
		public:
			std::shared_ptr<DrawCallsJob> job;
			std::vector<std::shared_ptr<IColorTargetTexture>> colorRenderTargets;
			std::vector<VkImageView> attachments;
			std::shared_ptr<IDepthStencilTexture> depthStencil;
			std::vector<VkClearValue> clearValues;
		};

		class InitInternalTask final : public Task<InternalTaskContext>
		{
		public:
			InitInternalTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class BeginPass final : public Task<InternalTaskContext>
		{
		public:
			BeginPass(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		class StartTask final : public Task<InternalTaskContext>
		{
		public:
			StartTask(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void OnScheduled(const std::shared_ptr<Core::BaseStream>& stream) override;
		private:
			std::shared_ptr<Core::BaseTask> _memBarriersTasks;
			std::shared_ptr<Core::BaseTask> _beginPass;
		};

		class EndPass final : public Task<InternalTaskContext>
		{
		public:
			EndPass(const std::shared_ptr<InternalTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};

		template<typename TCoreMaterial>
		class IterationJob final : public Iteration, public IterationImpl, public Vulkan::Job<TCoreMaterial>
		{
		private:
			class TaskContext final : public Core::TaskContext
			{
			public:
				TaskContext(const std::shared_ptr<IterationJob>& job);
				std::shared_ptr<IterationJob> job;
				std::shared_ptr<const Vulkan::Mesh::Renderer> renderer;
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
			public:
				ExecutionTask(const std::shared_ptr<TaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			IterationJob(const std::shared_ptr<DrawCallsJob>& job, const Item& item);
			IterationJob(const IterationJob&) = delete;
			IterationJob(IterationJob&&) noexcept = delete;
			IterationJob& operator=(const IterationJob&) = delete;
			IterationJob& operator=(IterationJob&&) noexcept = delete;
			~IterationJob() override;
			std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
			std::shared_ptr<Core::BaseTask> CreateExecutionTask() override;
		};


	public:
		DrawCallsJob(const std::shared_ptr<Core::Camera>& camera, std::vector<Item>&& items);
		~DrawCallsJob() override;
		DrawCallsJob(const DrawCallsJob&) = delete;
		DrawCallsJob(DrawCallsJob&&) noexcept = delete;
		DrawCallsJob& operator=(const DrawCallsJob&) = delete;
		DrawCallsJob& operator=(DrawCallsJob&&) noexcept = delete;
		std::vector<std::shared_ptr<Core::BaseTask>>& GetMemoryBarrierTasks(Core::PassControl<true, IterationImpl>);
		std::shared_ptr<Pass> GetPass(Core::PassControl<true, IterationImpl>) const;
		const std::vector<std::shared_ptr<Core::Camera::DrawCallsJob::Iteration>>& GetIterations(Core::PassControl<true, IterationImpl>) const;
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsFinish() override;
	private:
		std::shared_ptr<InternalTaskContext> BuildInternalContext();
		const FrameBuffer* GetOrCreateFrameBuffer(const std::shared_ptr<InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
		std::shared_ptr<Pass> _pass;
		std::vector<std::tuple<std::vector<VkImageView>, FrameBuffer>> _cachedFrameBuffers;
	};


	template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<DrawCallsJob>& job, const Item& item)
		: IterationImpl(job, item)
	{
	}

	template <typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::~IterationJob<TCoreMaterial>()
	{
		if(this->_device)
		{
			for (const auto& sm : this->_shaderModules)
			{
				vkDestroyShaderModule(this->_device->GetNativeLogical(), sm, nullptr);
			}
		}
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
		const auto iteration = ctx->job;
		const auto drawCallsJob = iteration->_drawCallsJob;
		const auto pc = Core::PassKey {iteration.get()};
		const auto material = std::dynamic_pointer_cast<TCoreMaterial>(iteration->_item.material);

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			const auto& ibInfo = ctx->renderer->GetIndexBufferPointer();
			drawCallsJob->GetMemoryBarrierTasks(pc).push_back(ibInfo->CreateMemoryBarrierTask(
				VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_INDEX_READ_BIT
			));

			const auto& allVertexBuffers = ctx->renderer->GetVertexBufferPointers();

			for (const auto& vb : allVertexBuffers)
			{
				drawCallsJob->GetMemoryBarrierTasks(pc).push_back(vb->CreateMemoryBarrierTask(
					VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
					VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
				));
			}
		}

		iteration->_device = this->_specificGlobalContext->device;
		iteration->PrepareMaterialParameters(this->_specificGlobalContext, iteration->_item.material->GetParameters());

		for (const auto& mbt : iteration->_memoryBarrierTasks)
		{
			drawCallsJob->GetMemoryBarrierTasks(pc).push_back(mbt);
		}


		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			VkShaderModule vertexShader;
			VkShaderModule pixelShader;

			VkShaderModuleCreateInfo shaderModelInfo{};
			shaderModelInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModelInfo.pNext = nullptr;
			shaderModelInfo.flags = 0;


			shaderModelInfo.codeSize = material->GetVertexShader()->GetCompiledBinaryLength();
			shaderModelInfo.pCode = static_cast<const std::uint32_t*>(material->GetVertexShader()->GetCompiledBinaryData());

			vkCreateShaderModule(this->_specificGlobalContext->device->GetNativeLogical(), &shaderModelInfo, nullptr, &vertexShader);
			assert(vertexShader);
			iteration->_shaderModules.push_back(vertexShader);

			shaderModelInfo.codeSize = material->GetPixelShader()->GetCompiledBinaryLength();
			shaderModelInfo.pCode = static_cast<const std::uint32_t*>(material->GetPixelShader()->GetCompiledBinaryData());

			vkCreateShaderModule(this->_specificGlobalContext->device->GetNativeLogical(), &shaderModelInfo, nullptr, &pixelShader);
			assert(pixelShader);
			iteration->_shaderModules.push_back(pixelShader);

			std::vector<VkPipelineShaderStageCreateInfo> shaderStages {};

			VkPipelineShaderStageCreateInfo vertStage{};
			vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertStage.pNext = nullptr;
			vertStage.flags = 0;
			vertStage.pSpecializationInfo = VK_NULL_HANDLE;
			vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertStage.module = vertexShader;
			vertStage.pName = material->GetVertexShader()->GetInfo().entryPointName.c_str();

			VkPipelineShaderStageCreateInfo pixelStage{};
			pixelStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pixelStage.pNext = nullptr;
			pixelStage.flags = 0;
			pixelStage.pSpecializationInfo = VK_NULL_HANDLE;
			pixelStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pixelStage.module = pixelShader;
			pixelStage.pName = material->GetPixelShader()->GetInfo().entryPointName.c_str();;

			shaderStages.push_back(vertStage);
			shaderStages.push_back(pixelStage);

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.flags = 0;
			inputAssembly.pNext = nullptr;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			switch (const auto topology = ctx->renderer->GetMesh()->GetTopology())
			{
			case Core::GeometryPrototype::Topology::Triangles:
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				break;
			default:
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
				break;
			}


			VkPipelineVertexInputStateCreateInfo vertexInput{};
			vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInput.pNext = nullptr;
			vertexInput.flags = 0;
			vertexInput.vertexBindingDescriptionCount = static_cast<std::uint32_t>(ctx->renderer->GetVertexBindingDescriptions().size());
			vertexInput.pVertexBindingDescriptions = ctx->renderer->GetVertexBindingDescriptions().data();
			vertexInput.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(ctx->renderer->GetVertexAttributeDescriptions().size());
			vertexInput.pVertexAttributeDescriptions = ctx->renderer->GetVertexAttributeDescriptions().data();

			const auto& settings = material->GetSettings();

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.pNext = nullptr;
			rasterizer.flags = 0;
			rasterizer.polygonMode = (settings.fillMode == Core::RenderingMaterial::Settings::FillMode::Solid ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE);
			rasterizer.cullMode = (settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Back
				? VK_CULL_MODE_BACK_BIT :
				(settings.cullMode == Core::RenderingMaterial::Settings::CullMode::Front ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_NONE)),
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.lineWidth = 1.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasSlopeFactor = 0.0f;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;


			const auto getCmpFunc = [](Core::RenderingMaterial::Settings::Comparision comparision) -> auto
				{
					switch (comparision)
					{
					case Core::RenderingMaterial::Settings::Comparision::Always:
						return VK_COMPARE_OP_ALWAYS;
					case Core::RenderingMaterial::Settings::Comparision::Equal:
						return VK_COMPARE_OP_EQUAL;
					case Core::RenderingMaterial::Settings::Comparision::Greater:
						return VK_COMPARE_OP_GREATER;
					case Core::RenderingMaterial::Settings::Comparision::GreaterEqual:
						return VK_COMPARE_OP_GREATER_OR_EQUAL;
					case Core::RenderingMaterial::Settings::Comparision::Less:
						return VK_COMPARE_OP_LESS;
					case Core::RenderingMaterial::Settings::Comparision::LessEqual:
						return VK_COMPARE_OP_LESS_OR_EQUAL;
					case Core::RenderingMaterial::Settings::Comparision::Never:
						return VK_COMPARE_OP_NEVER;
					case Core::RenderingMaterial::Settings::Comparision::NotEqual:
						return VK_COMPARE_OP_NOT_EQUAL;
					default:
						return VK_COMPARE_OP_ALWAYS;
					}
				};

			const auto getStencilOpFunc = [](Core::RenderingMaterial::Settings::Stencil::Op func) -> auto
				{
					switch (func)
					{
					case Core::RenderingMaterial::Settings::Stencil::Op::Keep:
						return VK_STENCIL_OP_KEEP;
					case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndSaturate:
						return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
					case Core::RenderingMaterial::Settings::Stencil::Op::DecrementAndWrap:
						return VK_STENCIL_OP_DECREMENT_AND_WRAP;
					case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndSaturate:
						return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
					case Core::RenderingMaterial::Settings::Stencil::Op::IncrementAndWrap:
						return VK_STENCIL_OP_INCREMENT_AND_WRAP;
					case Core::RenderingMaterial::Settings::Stencil::Op::Invert:
						return VK_STENCIL_OP_INVERT;
					case Core::RenderingMaterial::Settings::Stencil::Op::Replace:
						return VK_STENCIL_OP_REPLACE;
					case Core::RenderingMaterial::Settings::Stencil::Op::Zero:
						return VK_STENCIL_OP_ZERO;
					default:
						return VK_STENCIL_OP_KEEP;
					}
				};


			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.pNext = nullptr;
			depthStencil.flags = 0;

			depthStencil.depthTestEnable = static_cast<VkBool32>(settings.depth.test == Core::RenderingMaterial::Settings::Depth::Test::On);
			depthStencil.depthWriteEnable = static_cast<VkBool32>(settings.depth.write == Core::RenderingMaterial::Settings::Depth::Write::On);
			depthStencil.depthCompareOp = getCmpFunc(settings.depth.comparision);

			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {};
			depthStencil.back = {};

			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;

			if (settings.stencil.has_value())
			{
				const auto& stencil = settings.stencil.value();
				depthStencil.stencilTestEnable = VK_TRUE;

				const auto allParamEntries = material->GetParameters().GetAll();

				const auto stencilRefIt = std::find_if(allParamEntries.cbegin(), allParamEntries.cend(), [](const auto& e) {
					return std::holds_alternative<Core::BaseMaterial::Parameters::StencilRef>(e.settings);
				});

				assert(stencilRefIt != allParamEntries.cend());
				const auto reference = static_cast<std::uint32_t>(std::dynamic_pointer_cast<const Core::StencilRef>(stencilRefIt->entity)->value);

				depthStencil.front.compareOp = getCmpFunc(stencil.front.comparision);
				depthStencil.front.depthFailOp = getStencilOpFunc(stencil.front.depthFail);
				depthStencil.front.failOp = getStencilOpFunc(stencil.front.stencilFail);
				depthStencil.front.passOp = getStencilOpFunc(stencil.front.stencilPass);
				depthStencil.front.compareMask = 0xff;
				depthStencil.front.writeMask = 0xff;
				depthStencil.front.reference = reference;

				depthStencil.back.compareOp = getCmpFunc(stencil.back.comparision);
				depthStencil.back.depthFailOp = getStencilOpFunc(stencil.back.depthFail);
				depthStencil.back.failOp = getStencilOpFunc(stencil.back.stencilFail);
				depthStencil.back.passOp = getStencilOpFunc(stencil.back.stencilPass);
				depthStencil.back.compareMask = 0xff;
				depthStencil.back.writeMask = 0xff;
				depthStencil.back.reference = reference;
			}

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.pNext = nullptr;
			multisampling.flags = 0;
			multisampling.rasterizationSamples = std::dynamic_pointer_cast<IColorTargetTexture>(iteration->_drawCallsJob->_camera->GetTarget().color.front().tex->GetUnderlyingTexture())->GetSamplesCount();
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = static_cast<VkBool32>(settings.alphaToCoverage == Core::RenderingMaterial::Settings::AlphaToCoverage::On);
			multisampling.alphaToOneEnable = VK_FALSE;

			const auto getBlendOp = [](Core::RenderingMaterial::Settings::Blend::Op op) -> auto
				{
					switch (op)
					{
					case Core::RenderingMaterial::Settings::Blend::Op::Add:
						return VK_BLEND_OP_ADD;
					case Core::RenderingMaterial::Settings::Blend::Op::Max:
						return VK_BLEND_OP_MAX;
					case Core::RenderingMaterial::Settings::Blend::Op::Min:
						return VK_BLEND_OP_MIN;
					case Core::RenderingMaterial::Settings::Blend::Op::RevSub:
						return VK_BLEND_OP_REVERSE_SUBTRACT;
					case Core::RenderingMaterial::Settings::Blend::Op::Sub:
						return VK_BLEND_OP_SUBTRACT;
					default:
						return VK_BLEND_OP_ADD;
					}
				};

			const auto getBlendFactor = [](Core::RenderingMaterial::Settings::Blend::Factor factor) -> auto
				{
					switch (factor)
					{
					case Core::RenderingMaterial::Settings::Blend::Factor::Zero:
						return VK_BLEND_FACTOR_ZERO;
					case Core::RenderingMaterial::Settings::Blend::Factor::DstAlpha:
						return VK_BLEND_FACTOR_DST_ALPHA;
					case Core::RenderingMaterial::Settings::Blend::Factor::DstColor:
						return VK_BLEND_FACTOR_DST_COLOR;
					case Core::RenderingMaterial::Settings::Blend::Factor::One:
						return VK_BLEND_FACTOR_ONE;
					case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcAlpha:
						return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstAlpha:
						return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
					case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusDstColor:
						return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
					case Core::RenderingMaterial::Settings::Blend::Factor::OneMinusSrcColor:
						return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
					case Core::RenderingMaterial::Settings::Blend::Factor::SrcAlpha:
						return VK_BLEND_FACTOR_SRC_ALPHA;
					case Core::RenderingMaterial::Settings::Blend::Factor::SrcColor:
						return VK_BLEND_FACTOR_SRC_COLOR;
					default:
						return VK_BLEND_FACTOR_ONE;
					}
				};

			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

			for (std::size_t i = 0; i < settings.blend.targets.size(); ++i)
			{
				const auto& bt = settings.blend.targets.at(i);
				VkPipelineColorBlendAttachmentState blendAttachment {};

				blendAttachment.blendEnable = static_cast<VkBool32>(bt.op != Core::RenderingMaterial::Settings::Blend::Op::None);

				blendAttachment.colorBlendOp = getBlendOp(bt.op);
				blendAttachment.srcColorBlendFactor = getBlendFactor(bt.src);
				blendAttachment.dstColorBlendFactor = getBlendFactor(bt.dst);

				blendAttachment.alphaBlendOp = blendAttachment.colorBlendOp;
				blendAttachment.srcAlphaBlendFactor = blendAttachment.srcColorBlendFactor;
				blendAttachment.dstAlphaBlendFactor = blendAttachment.dstColorBlendFactor;

				blendAttachment.colorWriteMask = 0;

				if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Red))
				{
					blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
				}

				if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Green))
				{
					blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
				}

				if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Blue))
				{
					blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
				}

				if (static_cast<std::uint8_t>(bt.colorMask) & static_cast<std::uint8_t>(Core::RenderingMaterial::Settings::Blend::ColorMask::Alpha))
				{
					blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
				}


				blendAttachments.push_back(blendAttachment);
			}

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.pNext = nullptr;
			colorBlending.flags = 0;

			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;

			colorBlending.attachmentCount = static_cast<std::uint32_t>(iteration->_drawCallsJob->_camera->GetTarget().color.size());
			colorBlending.pAttachments = blendAttachments.data();

			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;


			const auto size = iteration->_drawCallsJob->_camera->GetTarget().color.front().tex->GetSettings().base.size;
			VkViewport viewport = { 0.0f, 0.0f, static_cast<std::float_t>(size.x), static_cast<std::float_t>(size.y), 0.0f, 1.0f };
			VkRect2D scissor = { {0, 0}, { size.x, size.y } };

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;
			viewportState.flags = 0;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkGraphicsPipelineCreateInfo pipelineInfo{};

			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = nullptr;
			pipelineInfo.flags = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;

			pipelineInfo.stageCount = static_cast<std::uint32_t>(shaderStages.size());
			pipelineInfo.pStages = shaderStages.data();

			pipelineInfo.pVertexInputState = &vertexInput;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pDynamicState = VK_NULL_HANDLE;
			pipelineInfo.pTessellationState = VK_NULL_HANDLE;
			pipelineInfo.layout = iteration->_pipelineLayout;
			pipelineInfo.renderPass = drawCallsJob->GetPass(pc)->GetRenderPass();
			pipelineInfo.subpass = 0;

			const auto pipelineRes = vkCreateGraphicsPipelines(iteration->_device->GetNativeLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &iteration->_pipeline);
			assert(pipelineRes == VK_SUCCESS);
			assert(iteration->_pipeline);
		}
	}

	template <typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::ExecutionTask(const std::shared_ptr<TaskContext>& ctx) : Task<TaskContext>(ctx)
	{
	}

	template <typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<TaskContext>::Run(stream);

		const auto tc = this->GetTaskContext();

		vkCmdBindPipeline(this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, tc->job->_pipeline);

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			vkCmdBindVertexBuffers(
				this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
				0, 
				static_cast<std::uint32_t>(tc->renderer->GetVertexBuffers().size()), 
				tc->renderer->GetVertexBuffers().data(),
				tc->renderer->GetVertexBuffersOffsets().data()
			);

			vkCmdBindIndexBuffer(
				this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), 
				tc->renderer->GetIndexBufferPointer()->GetDescriptorBufferInfo().buffer,
				0, 
				tc->renderer->GetIndexType()
			);

			vkCmdBindDescriptorSets(
				this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(),
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				tc->job->_pipelineLayout,
				0,
				static_cast<std::uint32_t>(tc->job->_sets.size()),
				tc->job->_sets.data(),
				0,
				nullptr
			);

			const auto& subsets = tc->renderer->GetMesh()->GetSubsets();

			for (const auto& ss : subsets)
			{
				vkCmdDrawIndexed(
					this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), 
					ss.indexCount,
					static_cast<std::uint32_t>(tc->renderer->GetSettings().dynamicData.instancesCount),
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
			renderer = std::dynamic_pointer_cast<Vulkan::Mesh::Renderer>(
				std::dynamic_pointer_cast<Core::Mesh::Renderer>(job->_item.renderer)->GetUnderlyingRenderer()
			);
		}
	}
}
