#pragma once
#include <mutex>
#include <string>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	template<typename TEntity>
	class EntityTaskContext : public TaskContext
	{
	public:
		std::shared_ptr<TEntity> entity;
	};

	class BaseEntity : public IInitializationTaskSource, public INamed, public virtual std::enable_shared_from_this<BaseEntity>
	{
	protected:
		BaseEntity(std::string_view name);
		BaseEntity();
		~BaseEntity() override;

	public:

		BaseEntity(const BaseEntity&) = delete;
		BaseEntity(BaseEntity&&) noexcept = delete;
		BaseEntity& operator=(const BaseEntity&) = delete;
		BaseEntity& operator=(BaseEntity&&) noexcept = delete;
		std::shared_ptr<BaseTask> CreateInitializationTask() override;
		std::string_view GetName() const override;
		std::uint64_t GetId() const;
	private:
		std::uint64_t _id;
		std::string _name;
		static std::mutex _mutex;
		static std::uint64_t _idCounter;
	};
}
