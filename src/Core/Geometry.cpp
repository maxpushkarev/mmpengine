#include <Core/Geometry.hpp>

namespace MMPEngine::Core
{
	BaseGeometryBufferPrototype::BaseGeometryBufferPrototype(const Settings& s) : _settings(s)
	{
	}

	BaseGeometryBufferPrototype::~BaseGeometryBufferPrototype() = default;
	BaseGeometryBufferPrototype::BaseGeometryBufferPrototype(BaseGeometryBufferPrototype&&) noexcept = default;
	BaseGeometryBufferPrototype& BaseGeometryBufferPrototype::operator=(BaseGeometryBufferPrototype&&) noexcept = default;


	std::size_t BaseGeometryBufferPrototype::GetByteLength() const
	{
		return GetElementsCount() * GetStride();
	}

	const BaseGeometryBufferPrototype::Settings& BaseGeometryBufferPrototype::GetSettings() const
	{
		return _settings;
	}

	VertexBufferPrototype::VertexBufferPrototype(const VBSettings& vb) : _vb(vb)
	{
	}

	const VertexBufferPrototype::VBSettings& VertexBufferPrototype::GetVBSettings() const
	{
		return _vb;
	}

	VertexBufferPrototypeFloat1::VertexBufferPrototypeFloat1(const Settings& settings) :
		BaseGeometryBufferPrototype(settings.base),
		VertexBufferPrototype(settings.vb),
		GeometryBufferPrototype(settings.base)
	{
	}

	VertexBufferPrototype::Format VertexBufferPrototypeFloat1::GetFormat() const
	{
		return Format::Float1;
	}

	const void* VertexBufferPrototypeFloat1::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t VertexBufferPrototypeFloat1::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t VertexBufferPrototypeFloat1::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	VertexBufferPrototypeFloat2::VertexBufferPrototypeFloat2(const Settings& settings) :
		BaseGeometryBufferPrototype(settings.base),
		VertexBufferPrototype(settings.vb),
		GeometryBufferPrototype(settings.base)
	{
	}

	VertexBufferPrototype::Format VertexBufferPrototypeFloat2::GetFormat() const
	{
		return Format::Float2;
	}

	const void* VertexBufferPrototypeFloat2::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t VertexBufferPrototypeFloat2::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t VertexBufferPrototypeFloat2::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	VertexBufferPrototypeFloat3::VertexBufferPrototypeFloat3(const Settings& settings) :
		BaseGeometryBufferPrototype(settings.base),
		VertexBufferPrototype(settings.vb),
		GeometryBufferPrototype(settings.base)
	{
	}

	VertexBufferPrototype::Format VertexBufferPrototypeFloat3::GetFormat() const
	{
		return Format::Float3;
	}

	const void* VertexBufferPrototypeFloat3::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t VertexBufferPrototypeFloat3::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t VertexBufferPrototypeFloat3::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	VertexBufferPrototypeFloat4::VertexBufferPrototypeFloat4(const Settings& settings) :
		BaseGeometryBufferPrototype(settings.base),
		VertexBufferPrototype(settings.vb),
		GeometryBufferPrototype(settings.base)
	{
	}

	VertexBufferPrototype::Format VertexBufferPrototypeFloat4::GetFormat() const
	{
		return Format::Float4;
	}

	const void* VertexBufferPrototypeFloat4::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t VertexBufferPrototypeFloat4::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t VertexBufferPrototypeFloat4::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	VertexBufferPrototypeUint4::VertexBufferPrototypeUint4(const Settings& settings) :
		BaseGeometryBufferPrototype(settings.base),
		VertexBufferPrototype(settings.vb),
		GeometryBufferPrototype(settings.base)
	{
	}

	VertexBufferPrototype::Format VertexBufferPrototypeUint4::GetFormat() const
	{
		return Format::Uint4;
	}

	const void* VertexBufferPrototypeUint4::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t VertexBufferPrototypeUint4::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t VertexBufferPrototypeUint4::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	IndexBufferPrototype16::IndexBufferPrototype16(const Settings& settings) :
		BaseGeometryBufferPrototype(settings),
		IndexBufferPrototype(),
		GeometryBufferPrototype(settings)
	{
	}

	IndexBufferPrototype::Format IndexBufferPrototype16::GetFormat() const
	{
		return Format::Uint16;
	}

	const void* IndexBufferPrototype16::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t IndexBufferPrototype16::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t IndexBufferPrototype16::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	IndexBufferPrototype32::IndexBufferPrototype32(const Settings& settings) :
		BaseGeometryBufferPrototype(settings),
		IndexBufferPrototype(),
		GeometryBufferPrototype(settings)
	{
	}

	IndexBufferPrototype::Format IndexBufferPrototype32::GetFormat() const
	{
		return Format::Uint32;
	}

	const void* IndexBufferPrototype32::GetDataPtr() const
	{
		return GeometryBufferPrototype::GetDataPtr();
	}

	std::size_t IndexBufferPrototype32::GetElementsCount() const
	{
		return GeometryBufferPrototype::GetElementsCount();
	}

	std::size_t IndexBufferPrototype32::GetStride() const
	{
		return GeometryBufferPrototype::GetStride();
	}

	GeometryPrototype::GeometryPrototype() = default;
	GeometryPrototype::~GeometryPrototype() = default;
	GeometryPrototype::GeometryPrototype(GeometryPrototype&&) noexcept = default;
	GeometryPrototype& GeometryPrototype::operator=(GeometryPrototype&&) noexcept = default;

	IGeometryRenderer::IGeometryRenderer() = default;
	IGeometryRenderer::~IGeometryRenderer() = default;


}