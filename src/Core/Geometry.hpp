#pragma once
#include <vector>

namespace MMPEngine::Core
{
	enum class VertexBufferFormat : std::uint8_t
	{
		Float1,
		Float2,
		Float3,
		Float4,
		Uint4,
	};

	enum class IndexBufferFormat : std::uint8_t
	{
		Uint16,
		Uint32
	};

	enum class VertexBufferPrototypeSemantics : std::uint8_t
	{
		Position,
		Color,
		Normal,
		BiNormal,
		Tangent,
		UV,
		BlendIndices,
		BlendWeight
	};

	class BaseGeometryBufferPrototype
	{
	public:
		struct Settings
		{
			//TODO: support some flags for vert/index buffers: UAV, raw...
		};
		virtual std::size_t GetStride() const = 0;
		virtual std::size_t GetByteLength() const = 0;
		virtual const void* GetDataPtr() const = 0;
		virtual std::size_t GetElementsCount() const = 0;
		const Settings& GetSettings() const;

		BaseGeometryBufferPrototype(const Settings& s);
		BaseGeometryBufferPrototype(const BaseGeometryBufferPrototype&) = delete;
		BaseGeometryBufferPrototype& operator=(const BaseGeometryBufferPrototype&) = delete;
		BaseGeometryBufferPrototype(BaseGeometryBufferPrototype&&) noexcept = default;
		BaseGeometryBufferPrototype& operator=(BaseGeometryBufferPrototype&&) noexcept = default;
		virtual ~BaseGeometryBufferPrototype();
	protected:
		Settings _settings;
	};
}