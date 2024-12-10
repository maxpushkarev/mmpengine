#pragma once
#include <cassert>
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

	class StreamBarrierTask final : public BaseTask
	{
	protected:
		void Run(const std::shared_ptr<BaseStream>& stream) override;
	public:
		static std::shared_ptr<StreamBarrierTask> kInstance;
	};

	template<typename TAppContext, typename TStreamContext>
	class ExternalContextSpecificTask : public virtual BaseTask
	{
		static_assert(std::is_base_of_v<AppContext, TAppContext>, "TAppContext must be derived from AppContext");
		static_assert(std::is_base_of_v<StreamContext, TStreamContext>, "TStreamContext must be derived from StreamContext");
	public:
		void OnScheduled(const std::shared_ptr<BaseStream>& stream) override;
		void Run(const std::shared_ptr<BaseStream>& stream) override;
		void OnComplete(const std::shared_ptr<BaseStream>& stream) override;
	private:
		void UpdateCache(const std::shared_ptr<BaseStream>& stream);
	protected:
		std::shared_ptr<TAppContext> _specificAppContext;
		std::shared_ptr<TStreamContext> _specificStreamContext;
	};


	template<typename TTaskContext>
	class ContextualTask : public virtual BaseTask
	{
		static_assert(std::is_base_of_v<TaskContext, TTaskContext>, "TTaskContext must be derived from TaskContext");
	protected:
		ContextualTask(const std::shared_ptr<TTaskContext>& innerContext);
		std::shared_ptr<TTaskContext> _internalTaskContext;
	public:
		std::shared_ptr<TTaskContext> GetContext() const;
	};

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::OnScheduled(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnScheduled(stream);
		UpdateCache(stream);
	}

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);
		UpdateCache(stream);
	}

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::OnComplete(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::OnComplete(stream);
		UpdateCache(stream);
	}

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::UpdateCache(const std::shared_ptr<BaseStream>& stream)
	{
		_specificAppContext = std::static_pointer_cast<TAppContext>(stream->GetAppContext());
		_specificStreamContext = std::static_pointer_cast<TStreamContext>(stream->GetStreamContext());
	}

	template<typename TInnerContext>
	inline ContextualTask<TInnerContext>::ContextualTask(const std::shared_ptr<TInnerContext>& innerContext) :
		_internalTaskContext(innerContext)
	{
	}

	template<typename TTaskContext>
	inline std::shared_ptr<TTaskContext> ContextualTask<TTaskContext>::GetContext() const
	{
		return _internalTaskContext;
	}
}
