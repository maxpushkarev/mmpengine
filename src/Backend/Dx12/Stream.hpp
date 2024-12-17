#pragma once
#include <Core/Stream.hpp>
#include <Backend/Dx12/Context.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Stream : public Core::Stream<GlobalContext, StreamContext>
	{
	private:
		using Super = Core::Stream<GlobalContext, StreamContext>;
	public:
		Stream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
		Stream(const Stream&) = delete;
		Stream(Stream&&) noexcept = delete;
		Stream& operator=(const Stream&) = delete;
		Stream& operator=(Stream&&) noexcept = delete;
		~Stream() override;
		bool IsSyncCounterValueCompleted(std::uint64_t counterValue) const override;
	protected:
		void RestartInternal() override;
		void SubmitInternal() override;
		void SyncInternal() override;
	private:
		std::uint64_t _fenceSignalValue = 0;
		HANDLE _waitHandle;
		std::vector<ID3D12DescriptorHeap*> _heaps;
	};
}