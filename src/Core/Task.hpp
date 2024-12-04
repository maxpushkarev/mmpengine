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
		virtual void Run(const std::shared_ptr<BaseStream>& stream);
		virtual void Finalize(const std::shared_ptr<BaseStream>& stream);

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
		void Run(const std::shared_ptr<BaseStream>& stream) override;
		void Finalize(const std::shared_ptr<BaseStream>& stream) override;
	private:
		void UpdateCache(const std::shared_ptr<BaseStream>& stream);
	protected:
		std::weak_ptr<BaseStream> _cachedStream;
		std::weak_ptr<TAppContext> _specificAppContext;
		std::weak_ptr<TStreamContext> _specificStreamContext;
	};


	template<typename TTaskContext>
	class TaskWithInternalContext : public virtual BaseTask
	{
		static_assert(std::is_base_of_v<TaskContext, TTaskContext>, "TTaskContext must be derived from TaskContext");
	protected:
		TaskWithInternalContext(const std::shared_ptr<TTaskContext>& innerContext);
		std::shared_ptr<TTaskContext> _internalTaskContext;
	};


	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::Run(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Run(stream);
		UpdateCache(stream);
	}

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::Finalize(const std::shared_ptr<BaseStream>& stream)
	{
		BaseTask::Finalize(stream);
		UpdateCache(stream);
	}

	template<typename TAppContext, typename TStreamContext>
	inline void ExternalContextSpecificTask<TAppContext, TStreamContext>::UpdateCache(const std::shared_ptr<BaseStream>& stream)
	{
		const auto cachedStream = _cachedStream.lock();
		if(!cachedStream || cachedStream != stream)
		{
			_specificStreamContext.reset();
			_specificAppContext.reset();

			_cachedStream = stream;
			_specificAppContext = std::dynamic_pointer_cast<TAppContext>(stream->GetAppContext());
			_specificStreamContext = std::dynamic_pointer_cast<TStreamContext>(stream->GetStreamContext());

			assert(!_specificStreamContext.expired());
			assert(!_specificAppContext.expired());
		}
	}

	template<typename TInnerContext>
	inline TaskWithInternalContext<TInnerContext>::TaskWithInternalContext(const std::shared_ptr<TInnerContext>& innerContext) :
		_internalTaskContext(innerContext)
	{
	}
}
