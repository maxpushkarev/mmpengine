#include <cassert>
#include <Backend/Vulkan/Job.hpp>
#include <Backend/Vulkan/Buffer.hpp>


namespace MMPEngine::Backend::Vulkan
{
	BaseJob::BaseJob() = default;
	BaseJob::~BaseJob()
	{
		if (_device && _pipeline)
		{
			vkDestroyPipeline(_device->GetNativeLogical(), _pipeline, nullptr);
		}

		if (_device && _pipelineLayout)
		{
			vkDestroyPipelineLayout(_device->GetNativeLogical(), _pipelineLayout, nullptr);
		}
	};

	void BaseJob::PrepareMaterialParameters(const std::shared_ptr<GlobalContext>& globalContext, const Core::BaseMaterial::Parameters& params)
	{
		const auto& paramsVec = params.GetAll();
		assert(!paramsVec.empty());

		std::vector<Core::BaseMaterial::Parameters::EntryView> entryViewsInSet;
		std::vector<VkWriteDescriptorSet> writeSets;

		auto buildSets = [this, &entryViewsInSet, &writeSets, &globalContext]()
		{
				std::vector<VkDescriptorSetLayoutBinding> bindings;
				std::uint32_t bindingCounter = 0;

				for (const auto& ev : entryViewsInSet)
				{
					VkDescriptorSetLayoutBinding layoutBinding{};
					layoutBinding.binding = bindingCounter++;
					layoutBinding.descriptorCount = 1;
					layoutBinding.stageFlags = GetStageFlags();
					layoutBinding.pImmutableSamplers = nullptr;


					if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(ev.entryPtr->settings))
					{
						if (std::dynamic_pointer_cast<const Core::BaseUniformBuffer>(ev.entryPtr->entity))
						{
							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						}
						else
						{
							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						}
					}
					else
					{
						throw Core::UnsupportedException("unsupported entity type as parameter for Vulkan backend");
					}

					bindings.push_back(layoutBinding);

					if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(ev.entryPtr->settings))
					{
						if (std::dynamic_pointer_cast<const Core::CounteredUnorderedAccessBuffer>(ev.entryPtr->entity))
						{
							VkDescriptorSetLayoutBinding counterLb{};
							counterLb.binding = bindingCounter++;
							counterLb.descriptorCount = 1;
							counterLb.stageFlags = GetStageFlags();
							counterLb.pImmutableSamplers = nullptr;
							counterLb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

							bindings.push_back(counterLb);
						}
					}
				}

				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
				layoutInfo.pBindings = bindings.data();

				auto setAllocation = globalContext->descriptorPool->AllocateSet(layoutInfo);
				_setAllocations.push_back(std::move(setAllocation));


				bindingCounter = 0;
				for (const auto& ev : entryViewsInSet)
				{
					VkWriteDescriptorSet writeSet{};
					writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSet.pNext = nullptr;
					writeSet.dstSet = _setAllocations.back().GetDescriptorSet();
					writeSet.dstBinding = bindingCounter++;
					writeSet.dstArrayElement = 0;
					writeSet.descriptorCount = 1;

					VkAccessFlags srcAccess = 0;
					VkAccessFlags dstAccess = 0;

					if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(ev.entryPtr->settings))
					{
						if (std::dynamic_pointer_cast<const Core::BaseUniformBuffer>(ev.entryPtr->entity))
						{
							writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

							srcAccess = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
							dstAccess = VK_ACCESS_UNIFORM_READ_BIT;
						}
						else
						{
							writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

							if (std::dynamic_pointer_cast<const Core::BaseUnorderedAccessBuffer>(ev.entryPtr->entity))
							{
								srcAccess = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
								dstAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
							}
							else
							{
								srcAccess = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
								dstAccess = VK_ACCESS_SHADER_READ_BIT;
							}
						}

						const auto buffer = std::dynamic_pointer_cast<const Core::Buffer>(ev.entryPtr->entity);
						const auto castedBuffer = std::dynamic_pointer_cast<const Buffer>(buffer->GetUnderlyingBuffer());
						const auto& castedBufferInfo = castedBuffer->GetDescriptorBufferInfo();

						writeSet.pBufferInfo = &castedBufferInfo;
						_memoryBarrierTasks.push_back(const_cast<Buffer*>(castedBuffer.get())->CreateMemoryBarrierTask(srcAccess, dstAccess));
					}
					else
					{
						throw Core::UnsupportedException("unsupported entity type as parameter for Vulkan backend");
					}

					writeSets.push_back(writeSet);


					if (std::holds_alternative<Core::BaseMaterial::Parameters::Buffer>(ev.entryPtr->settings))
					{
						const auto buffer = std::dynamic_pointer_cast<const Core::Buffer>(ev.entryPtr->entity);
						if (const auto castedCounteredUaBuffer = std::dynamic_pointer_cast<const CounteredUnorderedAccessBuffer>(buffer->GetUnderlyingBuffer()))
						{
							VkWriteDescriptorSet counterWs{};
							counterWs.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
							counterWs.pNext = nullptr;
							counterWs.dstSet = _setAllocations.back().GetDescriptorSet();
							counterWs.dstBinding = bindingCounter++;
							counterWs.dstArrayElement = 0;
							counterWs.descriptorCount = 1;
							counterWs.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

							const auto& counterBufferDescInfo = castedCounteredUaBuffer->GetCounterBuffer()->GetDescriptorBufferInfo();
							counterWs.pBufferInfo = &counterBufferDescInfo;

							writeSets.push_back(counterWs);
						}
					}
				}
		};

		for (std::size_t i = 0; i < paramsVec.size(); ++i)
		{
			if (i > 0 && paramsVec[i - 1].tag != paramsVec[i].tag)
			{
				buildSets();
				entryViewsInSet.clear();
			}

			entryViewsInSet.push_back(params.TryGet(paramsVec[i].name).value());
		}

		if (!entryViewsInSet.empty())
		{
			buildSets();
		}


		vkUpdateDescriptorSets(
			_device->GetNativeLogical(), 
			static_cast<std::uint32_t>(writeSets.size()),
			writeSets.data(), 
			0, 
			nullptr
		);


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.flags = 0;

		std::vector<VkDescriptorSetLayout> setLayouts;

		setLayouts.reserve(_setAllocations.size());
		_sets.reserve(_setAllocations.size());

		std::transform(_setAllocations.cbegin(), _setAllocations.cend(), std::back_inserter(setLayouts), [](const auto& alloc)
		{
			return alloc.GetDescriptorSetLayout();
		});

		std::transform(_setAllocations.cbegin(), _setAllocations.cend(), std::back_inserter(_sets), [](const auto& alloc)
		{
			return alloc.GetDescriptorSet();
		});

		pipelineLayoutInfo.setLayoutCount = static_cast<std::uint32_t>(_setAllocations.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		vkCreatePipelineLayout(_device->GetNativeLogical(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
		assert(_pipelineLayout);
	}
	
	BaseJob::MemBarriersTask::MemBarriersTask(const std::shared_ptr<TaskContext>& context) : Task<TaskContext>(context)
	{
	}

	void BaseJob::MemBarriersTask::OnScheduled(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::OnScheduled(stream);

		if (const auto job = this->GetTaskContext()->job)
		{
			for (const auto& sst : job->_memoryBarrierTasks)
			{
				stream->Schedule(sst);
			}
		}
	}

	template<>
	VkShaderStageFlags Job<Core::ComputeMaterial>::GetStageFlags() const
	{
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}

	template<>
	VkShaderStageFlags Job<Core::MeshMaterial>::GetStageFlags() const
	{
		return VK_SHADER_STAGE_ALL_GRAPHICS;
	}
}
