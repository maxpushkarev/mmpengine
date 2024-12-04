#pragma once
#include <Core/Stream.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Stream : public Core::Stream<AppContext, StreamContext>
	{
	private:
		using Super = Core::Stream<AppContext, StreamContext>;
	public:
		Stream(const std::shared_ptr<AppContext>& appContext, const std::shared_ptr<StreamContext>& streamContext);
		Stream(const Stream&) = delete;
		Stream(Stream&&) noexcept = delete;
		Stream& operator=(const Stream&) = delete;
		Stream& operator=(Stream&&) noexcept = delete;
		~Stream() override;
	protected:
		void RestartInternal() override;
		void SubmitInternal() override;
		void SyncInternal() override;
	private:
		std::uint64_t _lastFenceValue;
		HANDLE _waitHandle;
	};
}