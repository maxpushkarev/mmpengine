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

		template<typename TCoreMaterial>
		class IterationJob final : public Iteration, public Vulkan::Job<TCoreMaterial>
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
				std::vector<std::shared_ptr<Core::BaseTask>> _barrierTasks;
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


	template<typename TCoreMaterial>
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
	}

	template <typename TCoreMaterial>
	Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::ExecutionTask(const std::shared_ptr<TaskContext>& ctx) : Task<TaskContext>(ctx)
	{
		const auto baseJobCtx = std::make_shared<Vulkan::BaseJob::TaskContext>();
		baseJobCtx->job = std::dynamic_pointer_cast<Vulkan::BaseJob>(ctx->job);

		//_applyMaterial = std::make_shared<Vulkan::BaseJob::ApplyParametersTask>(baseJobCtx);
		//_setPipelineState = std::make_shared<typename Vulkan::Job<TCoreMaterial>::SetPipelineStateTask>(baseJobCtx);

		if constexpr (std::is_base_of_v<Core::MeshMaterial, TCoreMaterial>)
		{
			/*const auto& ibInfo = ctx->mesh->GetIndexBufferInfo();
			_switchStateTasks.push_back(std::dynamic_pointer_cast<Vulkan::BaseEntity>(ibInfo.ptr->GetUnderlyingBuffer())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));

			const auto& allBufferInfos = ctx->mesh->GetAllVertexBufferInfos();

			for (const auto& s2vbs : allBufferInfos)
			{
				for (const auto& vbInfo : s2vbs.second)
				{
					_switchStateTasks.push_back(std::dynamic_pointer_cast<Vulkan::BaseEntity>(vbInfo.ptr->GetUnderlyingBuffer())->CreateSwitchStateTask(D3D12_RESOURCE_STATE_GENERIC_READ));
				}
			}*/

			_impl = std::make_shared<Impl>(ctx);
		}
	}

	template <typename TCoreMaterial>
	void Camera::DrawCallsJob::IterationJob<TCoreMaterial>::ExecutionTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task<TaskContext>::OnScheduled(stream);

		//stream->Schedule(_setPipelineState);
		//stream->Schedule(_applyMaterial);

		for (const auto& bt : _barrierTasks)
		{
			stream->Schedule(bt);
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
