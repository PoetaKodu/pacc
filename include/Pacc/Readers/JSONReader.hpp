#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>
#include <Pacc/Helpers/Json.hpp>

class PackageJsonReader
	: public BasicJsonView<json> // non-const
{
public:
	using BasicJsonView::BasicJsonView;

	void makeConformant();
};
