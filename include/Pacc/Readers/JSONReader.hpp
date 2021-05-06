#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>
#include <Pacc/Helpers/Json.hpp>

class PackageJsonReader
	: public BasicJsonView<json> // non-const
{
public:
	using BasicJsonView::BasicJsonView;

	/// <summary>
	/// 	Makes sure that provided json can be further processed,
	/// 	f.e.: ensures that projects are inside a workspace.
	/// </summary>
	void makeConformant();
};
