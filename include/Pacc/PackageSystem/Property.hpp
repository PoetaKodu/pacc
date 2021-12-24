#pragma once

#include PACC_PCH

namespace policies
{

////////////////////////////////////
class Join
{
public:
	template <typename TProp, typename TTransformPolicy>
	void operator()(TProp & left_, TProp const& right_, TTransformPolicy transformPolicy_ = {})
	{
		left_.content.reserve(right_.content.size());
		for (auto const& elem : right_.content)
		{
			left_.content.push_back( transformPolicy_(elem) );
		}
	}
};

////////////////////////////////////
class PassFurther
{
public:
	template <typename T>
	T operator()(T && toPass_) const
	{
		return std::forward<T>(toPass_);
	}	
};

////////////////////////////////////
class ResolvePath
{
public:
	fs::path basePath;

	template <typename T>
	T operator()(T && path_) const
	{
		if (path_.is_relative())
			return fsx::fwd(basePath / path_).string();
		else 
			return path_;
	}
}

}

////////////////////////////////////
template <
		typename TMergePolicy 		= policies::Join,
		typename TTransformPolicy 	= policies::PassFurther
	>
class ArrayProperty
{
	using ValueType 		= TValueType;
	using MergePolicy 		= TMergePolicy;
	using TransformPolicy 	= TTransformPolicy;

	MergePolicy* 		mergePolicy 	= nullptr;
	TransformPolicy* 	transformPolicy	= nullptr;

	void mergeWith(ArrayProperty const& other_)
	{
		auto doMerge = [&](auto const& merger) {
				merger(*this, other_, transformPolicy_ ? *transformPolicy_ : TransformPolicy{});
			};

		if (mergePolicy)
			doMerge(*mergePolicy);
		else
			doMerge(MergePolicy{});
	}
	
	std::vector<ValueType> content;
};