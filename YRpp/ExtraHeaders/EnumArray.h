#pragma once

template <class IndexType, class ValueType>
class EnumArray
{
public:

	ValueType& operator[](IndexType i)
	{
		return array_[static_cast<int>(i)];
	}

	const ValueType& operator[](IndexType i) const
	{
		return array_[static_cast<int>(i)];
	}

	const auto begin() const { return std::begin(array_); }
	const auto end() const { return std::end(array_); }

	auto begin() { return std::begin(array_); }
	auto end() { return std::end(array_); }

	constexpr int size() const { return size_; }

private:
	ValueType array_[static_cast<int>(IndexType::kMaxValue) + 1];

	constexpr int size_ = static_cast<int>(IndexType::kMaxValue) + 1;
};
