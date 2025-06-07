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
			IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const std::shared_ptr<Core::Camera>& camera, const Item& item);
			std::shared_ptr<Core::Camera> _camera;
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
			public:
				ExecutionTask(const std::shared_ptr<TaskContext>& ctx);
			protected:
				void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
			};

		public:
			IterationJob(const std::shared_ptr<DrawCallsJob>& job, const std::shared_ptr<Core::Camera>& camera, const Item& item);
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
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<DrawCallsJob>& job, const std::shared_ptr<Core::Camera>& camera, const Item& item)
		: IterationImpl(job, camera, item)
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
			const auto& ibInfo = ctx->mesh->GetIndexBufferInfo();
			drawCallsJob->GetMemoryBarrierTasks(pc).push_back(std::dynamic_pointer_cast<Vulkan::Buffer>(ibInfo.ptr->GetUnderlyingBuffer())->CreateMemoryBarrierTask(
				VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_INDEX_READ_BIT
			));

			const auto& allBufferInfos = ctx->mesh->GetAllVertexBufferInfos();

			for (const auto& s2vbs : allBufferInfos)
			{
				for (const auto& vbInfo : s2vbs.second)
				{
					drawCallsJob->GetMemoryBarrierTasks(pc).push_back(std::dynamic_pointer_cast<Vulkan::Buffer>(vbInfo.ptr->GetUnderlyingBuffer())->CreateMemoryBarrierTask(
						VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
						VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
					));
				}
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


			VkPipelineShaderStageCreateInfo vertStage{};
			vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertStage.pNext = nullptr;
			vertStage.flags = 0;
			vertStage.pSpecializationInfo = VK_NULL_HANDLE;
			vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertStage.module = vertexShader;
			vertStage.pName = Core::Shader::ENTRY_POINT_NAME;

			VkPipelineShaderStageCreateInfo pixelStage{};
			pixelStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pixelStage.pNext = nullptr;
			pixelStage.flags = 0;
			pixelStage.pSpecializationInfo = VK_NULL_HANDLE;
			pixelStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pixelStage.module = pixelShader;
			pixelStage.pName = Core::Shader::ENTRY_POINT_NAME;

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, pixelStage };


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

		vkCmdNextSubpass(this->_specificStreamContext->PopulateCommandsInBuffer()->GetNative(), VK_SUBPASS_CONTENTS_INLINE);
	}

	template<typename TCoreMaterial>
	inline Camera::DrawCallsJob::IterationJob<TCoreMaterial>::TaskContext::TaskContext(const std::shared_ptr<IterationJob>& job) : job(job)
	{
		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			meshRenderer = std::dynamic_pointer_cast<Core::Mesh::Renderer>(job->_item.renderer);
			mesh = std::dynamic_pointer_cast<const Vulkan::Mesh>(meshRenderer->GetMesh()->GetUnderlyingMesh());
		}
	}
}
