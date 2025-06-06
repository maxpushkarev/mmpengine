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
	class Camera::DrawCallsJob final : public Core::Camera::DrawCallsJob
	{
	private:
		class InternalTaskContext;

		class IterationImpl
		{
		public:
			IterationImpl(const std::shared_ptr<DrawCallsJob>& job, const std::shared_ptr<Core::Camera>& camera, const Item& item);
			std::shared_ptr<Core::Camera> _camera;
			Item _item;
			std::shared_ptr<DrawCallsJob> _drawCallsJob;
		};

		class Pass final
		{
		public:
			explicit Pass(const std::shared_ptr<const InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
			Pass(const Pass&) = delete;
			Pass(Pass&&) noexcept;
			Pass& operator=(const Pass&) = delete;
			Pass& operator=(Pass&&) noexcept = delete;
			~Pass();

			VkFramebuffer GetFrameBuffer() const;
			VkRenderPass GetRenderPass() const;
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
	protected:
		std::shared_ptr<Iteration> BuildIteration(const Item& item) const override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsStart() override;
		std::shared_ptr<Core::BaseTask> CreateInitializationTaskInternal() override;
		std::shared_ptr<Core::BaseTask> CreateTaskForIterationsFinish() override;
	private:
		std::shared_ptr<InternalTaskContext> BuildInternalContext();
		const Pass* GetOrCreatePass(const std::shared_ptr<InternalTaskContext>& ctx, const std::shared_ptr<Wrapper::Device>& device);
		std::vector<std::tuple<std::vector<VkImageView>, Pass>> _cachedPasses;
		std::vector<std::shared_ptr<Core::BaseTask>> _memoryBarrierTasks;
	};


	template<typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::IterationJob(const std::shared_ptr<DrawCallsJob>& job, const std::shared_ptr<Core::Camera>& camera, const Item& item)
		: IterationImpl(job, camera, item)
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
		const auto iteration = ctx->job;

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			const auto& ibInfo = ctx->mesh->GetIndexBufferInfo();
			iteration->_drawCallsJob->_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<Vulkan::Buffer>(ibInfo.ptr->GetUnderlyingBuffer())->CreateMemoryBarrierTask(
				VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_INDEX_READ_BIT
			));

			const auto& allBufferInfos = ctx->mesh->GetAllVertexBufferInfos();

			for (const auto& s2vbs : allBufferInfos)
			{
				for (const auto& vbInfo : s2vbs.second)
				{
					iteration->_drawCallsJob->_memoryBarrierTasks.push_back(std::dynamic_pointer_cast<Vulkan::Buffer>(vbInfo.ptr->GetUnderlyingBuffer())->CreateMemoryBarrierTask(
						VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
						VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
					));
				}
			}
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
