#pragma once
#include <functional>
#include <memory>
#include <Core/Context.hpp>
#include <Core/Stream.hpp>

namespace MMPEngine::Core
{
	class BaseStream;

	class BaseTask
	{
		friend class BaseStream;
	public:
		BaseTask();
		virtual ~BaseTask();
	protected:
		virtual void OnScheduled(const std::shared_ptr<BaseStream>& stream);
		virtual void Run(const std::shared_ptr<BaseStream>& stream);
		virtual void OnComplete(const std::shared_ptr<BaseStream>& stream);

	public:
		BaseTask(const BaseTask&) = delete;
		BaseTask(BaseTask&&) noexcept = default;
		BaseTask& operator=(const BaseTask&) = delete;
		BaseTask& operator=(BaseTask&&) noexcept = default;

		static std::shared_ptr<BaseTask> kEmpty;
	};

	class BatchTask final : public BaseTask
	{
	public:
		BatchTask(std::initializer_list<std::shared_ptr<BaseTask>> tasks);
		BatchTask(std::vector<std::shared_ptr<BaseTask>>&& tasks);
	protected:
		void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
	private:
		std::vector<std::shared_ptr<BaseTask>> _tasks;
	};

	class StreamBarrierTask final : public BaseTask
	{
	protected:
		void Run(const std::shared_ptr<BaseStream>& stream) override;
	public:
		static std::shared_ptr<StreamBarrierTask> kInstance;
	};

	class StreamFlushTask final : public BaseTask
	{
	protected:
		void Run(const std::shared_ptr<BaseStream>& stream) override;
	public:
		static std::shared_ptr<StreamFlushTask> kInstance;
	};

	class FunctionalTask final : public BaseTask
	{
	public:
		using Handler = std::function<void(const std::shared_ptr<BaseStream>& stream)>;
		FunctionalTask(Handler&& onScheduleFn, Handler&& runFn, Handler&& onCompleteFn);
	private:
		Handler _onScheduleFn;
		Handler _runFn;
		Handler _onOnCompleteFn;
	protected:
		void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		void Run(const std::shared_ptr<BaseStream>& stream) override;
		void OnComplete(const std::shared_ptr<BaseStream>& stream) override;
	};

	template<typename TTaskContext>
	class ContextualTask : public BaseTask, public TaskContextHolder<TTaskContext>
	{
	protected:
		ContextualTask(const std::shared_ptr<TTaskContext>& taskContext);
	};

	template<typename TTaskContext>
	inline ContextualTask<TTaskContext>::ContextualTask(const std::shared_ptr<TTaskContext>& taskContext) : TaskContextHolder<TTaskContext>(taskContext)
	{
	}

	template<typename TGlobalContext, typename TStreamContext, typename TTaskContext>
	class ContextHolderTask : public AppStreamContextCache<TGlobalContext, TStreamContext>, public ContextualTask<TTaskContext>
	{
	protected:
		ContextHolderTask(const std::shared_ptr<TTaskContext>& taskContext);
		void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		void Run(const std::shared_ptr<BaseStream>& stream) override;
		void OnComplete(const std::shared_ptr<BaseStream>& stream) override;
	};

	template<typename TGlobalContext, typename TStreamContext, typename TTaskContext>
	inline ContextHolderTask<TGlobalContext, TStreamContext, TTaskContext>::ContextHolderTask(const std::shared_ptr<TTaskContext>& taskContext) : ContextualTask<TTaskContext>(taskContext)
	{
	}

	template<typename TGlobalContext, typename TStreamContext, typename TTaskContext>
	inline void ContextHolderTask<TGlobalContext, TStreamContext, TTaskContext>::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnScheduled(stream);
		this->UpdateCache(stream);
	}

	template<typename TGlobalContext, typename TStreamContext, typename TTaskContext>
	inline void ContextHolderTask<TGlobalContext, TStreamContext, TTaskContext>::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);
		this->UpdateCache(stream);
	}

	template<typename TGlobalContext, typename TStreamContext, typename TTaskContext>
	inline void ContextHolderTask<TGlobalContext, TStreamContext, TTaskContext>::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnComplete(stream);
		this->UpdateCache(stream);
	}


	template<typename TGlobalContext, typename TStreamContext>
	class ContextHolderTask<TGlobalContext, TStreamContext, void> : public AppStreamContextCache<TGlobalContext, TStreamContext>, public BaseTask
	{
	protected:
		void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		void Run(const std::shared_ptr<BaseStream>& stream) override;
		void OnComplete(const std::shared_ptr<BaseStream>& stream) override;
	};

	template <typename TGlobalContext, typename TStreamContext>
	void ContextHolderTask<TGlobalContext, TStreamContext, void>::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnScheduled(stream);
		this->UpdateCache(stream);
	}

	template <typename TGlobalContext, typename TStreamContext>
	void ContextHolderTask<TGlobalContext, TStreamContext, void>::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);
		this->UpdateCache(stream);
	}

	template <typename TGlobalContext, typename TStreamContext>
	void ContextHolderTask<TGlobalContext, TStreamContext, void>::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnComplete(stream);
		this->UpdateCache(stream);
	}
}
