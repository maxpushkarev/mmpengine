#include <Core/Geometry.hpp>

namespace MMPEngine::Core
{
	BaseGeometryBufferPrototype::BaseGeometryBufferPrototype(const Settings& s) : _settings(s)
	{
	}

	BaseGeometryBufferPrototype::~BaseGeometryBufferPrototype() = default;

	const BaseGeometryBufferPrototype::Settings& BaseGeometryBufferPrototype::GetSettings() const
	{
		return _settings;
	}
}