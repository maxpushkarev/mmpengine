#pragma once
#include <vector>
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	class BaseGeometryBufferPrototype
	{
	public:
		struct Settings final
		{
			//TODO: support some flags for vert/index buffers: unordered access, raw access...
		};
		virtual std::size_t GetStride() const = 0;
		std::size_t GetByteLength() const;
		virtual const void* GetDataPtr() const = 0;
		virtual std::size_t GetElementsCount() const = 0;
		const Settings& GetSettings() const;

		BaseGeometryBufferPrototype(const Settings& s);
		BaseGeometryBufferPrototype(const BaseGeometryBufferPrototype&) = delete;
		BaseGeometryBufferPrototype& operator=(const BaseGeometryBufferPrototype&) = delete;
		BaseGeometryBufferPrototype(BaseGeometryBufferPrototype&&) noexcept;
		BaseGeometryBufferPrototype& operator=(BaseGeometryBufferPrototype&&) noexcept;
		virtual ~BaseGeometryBufferPrototype();
	protected:
		Settings _settings;
	};

	template<class TElement>
	class GeometryBufferPrototype : public virtual BaseGeometryBufferPrototype
	{
		static_assert(std::is_pod_v<TElement>);
	public:

		GeometryBufferPrototype(const Settings& s);
		GeometryBufferPrototype(const GeometryBufferPrototype&) = delete;
		GeometryBufferPrototype& operator=(const GeometryBufferPrototype&) = delete;
		GeometryBufferPrototype(GeometryBufferPrototype&&) noexcept;
		GeometryBufferPrototype& operator=(GeometryBufferPrototype&&) noexcept;
		~GeometryBufferPrototype() override;

		const void* GetDataPtr() const override;
		std::size_t GetStride() const override;
		std::size_t GetElementsCount() const override;

		std::vector<TElement> data;
	};

	class VertexBufferPrototype : public virtual BaseGeometryBufferPrototype
	{
	public:
		enum class Format : std::uint8_t
		{
			Float1,
			Float2,
			Float3,
			Float4,
			Uint4,
		};
		enum class Semantics : std::uint8_t
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
		struct VBSettings final
		{
			Semantics semantics = Semantics::Position;
		};
		struct Settings final
		{
			VBSettings vb {};
			BaseGeometryBufferPrototype::Settings base {};
		};

		VertexBufferPrototype(const VBSettings& vb);
		virtual Format GetFormat() const = 0;
		const VBSettings& GetVBSettings() const;


	private:
		VBSettings _vb;
	};

	class IndexBufferPrototype : public virtual BaseGeometryBufferPrototype
	{
	public:
		enum class Format : std::uint8_t
		{
			Uint16,
			Uint32
		};
		virtual Format GetFormat() const = 0;
	};

	class IndexBufferPrototype16 final : public IndexBufferPrototype, public GeometryBufferPrototype<std::uint16_t>
	{
	public:
		IndexBufferPrototype16(const Settings& s);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class IndexBufferPrototype32 final : public IndexBufferPrototype, public GeometryBufferPrototype<std::uint32_t>
	{
	public:
		IndexBufferPrototype32(const Settings& s);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class VertexBufferPrototypeFloat1 final : public VertexBufferPrototype, public GeometryBufferPrototype<std::float_t>
	{
	public:
		explicit VertexBufferPrototypeFloat1(const Settings& settings);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class VertexBufferPrototypeFloat2 final : public VertexBufferPrototype, public GeometryBufferPrototype<Vector2Float>
	{
	public:
		explicit VertexBufferPrototypeFloat2(const Settings& settings);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class VertexBufferPrototypeFloat3 final : public VertexBufferPrototype, public GeometryBufferPrototype<Vector3Float>
	{
	public:
		explicit VertexBufferPrototypeFloat3(const Settings& settings);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class VertexBufferPrototypeFloat4 final : public VertexBufferPrototype, public GeometryBufferPrototype<Vector4Float>
	{
	public:
		explicit VertexBufferPrototypeFloat4(const Settings& settings);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class VertexBufferPrototypeUint4 final : public VertexBufferPrototype, public GeometryBufferPrototype<Vector4Uint>
	{
	public:
		explicit VertexBufferPrototypeUint4(const Settings& settings);
		Format GetFormat() const override;

		const void* GetDataPtr() const override;
		std::size_t GetElementsCount() const override;
		std::size_t GetStride() const override;
	};

	class GeometryPrototype final
	{
	public:
		struct Subset final
		{
			std::uint32_t indexCount = 0;
			std::uint32_t indexStart = 0;
			std::uint32_t baseVertex = 0;
		};

		enum class Topology : std::uint8_t
		{
			Triangles
		};

		std::vector<std::unique_ptr<VertexBufferPrototype>> vertexBuffers;
		std::unique_ptr<IndexBufferPrototype> indexBuffer;
		std::vector<Subset> subsets;
		Topology topology = Topology::Triangles;

		GeometryPrototype();
		~GeometryPrototype();
		GeometryPrototype(const GeometryPrototype&) = delete;
		GeometryPrototype& operator=(const GeometryPrototype&) = delete;
		GeometryPrototype(GeometryPrototype&&) noexcept;
		GeometryPrototype& operator=(GeometryPrototype&&) noexcept;
	};

	template<class TElement>
	inline GeometryBufferPrototype<TElement>::GeometryBufferPrototype(const Settings& s) : BaseGeometryBufferPrototype(s)
	{
	}

	template<class TElement>
	inline GeometryBufferPrototype<TElement>& GeometryBufferPrototype<TElement>::operator=(GeometryBufferPrototype&&) noexcept = default;

	template<class TElement>
	inline GeometryBufferPrototype<TElement>::GeometryBufferPrototype(GeometryBufferPrototype&&) noexcept = default;

	template<class TElement>
	inline GeometryBufferPrototype<TElement>::~GeometryBufferPrototype() = default;

	template<class TElement>
	inline const void* GeometryBufferPrototype<TElement>::GetDataPtr() const
	{
		return data.data();
	}

	template<class TElement>
	inline std::size_t GeometryBufferPrototype<TElement>::GetStride() const
	{
		return sizeof(TElement);
	}

	template<class TElement>
	inline std::size_t GeometryBufferPrototype<TElement>::GetElementsCount() const
	{
		return data.size();
	}
}