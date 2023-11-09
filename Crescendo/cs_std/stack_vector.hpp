#pragma once

#include <array>

namespace cs_std
{
	// Vector-like class with a maximum size
	// Vector-like interface
	template<typename T, size_t N>
	class stack_vector
	{
	private:
		std::array<T, N> array;
		size_t curSize;
	public:
		// Constructors
		constexpr stack_vector() : array(), curSize(0) {}
		// Copy constructors
		constexpr stack_vector(const stack_vector& other) : array(other.array), curSize(other.curSize) {}
		constexpr stack_vector& operator=(stack_vector&& other)
		{
			if (this != &other)
			{
				this->array = std::move(other.array);
				this->curSize = other.curSize;
			}
			return *this;
		}
		// Initlizer list constructors
		constexpr stack_vector(std::initializer_list<T> init) : array(), curSize(0)
		{
			for (auto& item : init)
			{
				this->array[this->curSize] = item;
				this->curSize++;
			}
		}
		// Range constructors
		template<typename InputIt>
		constexpr stack_vector(InputIt first, InputIt last) : array(), curSize(0)
		{
			for (auto it = first; it != last; it++)
			{
				this->array[this->curSize] = *it;
				this->curSize++;
			}
		}
		// Destructors
		~stack_vector() = default;
		// Element access
		constexpr T& at(size_t pos)
		{
			if (pos >= this->curSize) throw std::out_of_range("stack_vector::at");
			return this->array[pos];
		}
		constexpr const T& at(size_t pos) const
		{
			if (pos >= this->curSize) throw std::out_of_range("stack_vector::at");
			return this->array[pos];
		}
		constexpr T& operator[](size_t pos) { return this->array[pos]; }
		constexpr const T& operator[](size_t pos) const { return this->array[pos]; }
		constexpr T& front() { return this->array[0]; }
		constexpr const T& front() const { return this->array[0]; }
		constexpr T& back() { return this->array[this->curSize - 1]; }
		constexpr const T& back() const { return this->array[this->curSize - 1]; }
		constexpr T* data() { return this->array.data(); }
		constexpr const T* data() const { return this->array.data(); }
		// Iterators
		constexpr auto begin() noexcept { return array.begin(); }
		constexpr auto end() noexcept { return array.begin() + curSize; }
		constexpr auto begin() const noexcept { return array.begin(); }
		constexpr auto end() const noexcept { return array.begin() + curSize; }
		// Capacity
		constexpr bool empty() const { return this->curSize == 0; }
		constexpr size_t size() const { return this->curSize; }
		constexpr size_t capacity() const { return N }

		// Modifiers
		constexpr void clear() { this->curSize = 0; }
		template<typename... Args>
		constexpr void emplace_back(Args&&... args )
		{
			this->array[this->curSize] = T(std::forward<Args>(args)...);
			this->curSize++;
		}
		constexpr void push_back(T&& value)
		{
			this->array[this->curSize] = std::move(value);
			this->curSize++;
		}
		constexpr void push_back(const T& value)
		{
			this->array[this->curSize] = value;
			this->curSize++;
		}
		constexpr void pop_back() { this->curSize--; }
	};
}