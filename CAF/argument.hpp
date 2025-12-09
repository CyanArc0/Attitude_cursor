#pragma once
#define $included$CAF$argment 2.3

#include<cstddef>
#include<string_view>
#include<compare>
#include<iterator>
#include"ctypes/cstring/cstring_t.hpp"
#include"argref/ar_t.hpp"

namespace CAF
{
	struct argument
	{
	private:
		int argc;
		cstring_t*argv;
	protected:
		static constexpr char space = ' ';
	public:
		using size_type = std::size_t;
		using value_type = std::string_view;

		struct iterator
		{
		private:
			cstring_t*_pointer=nullptr;
		public:
			using iterator_category = std::random_access_iterator_tag;
			
			using value_type = std::string_view;
			using difference_type = std::ptrdiff_t;

			struct pointer : protected std::string_view
			{
				using basic_string_view::basic_string_view;
				std::string_view* operator->()noexcept
				{
					return this;
				}
				std::string_view operator*()noexcept
				{
					return *this;
				}
			};
			using reference = value_type;
			
			constexpr iterator()noexcept = default;
			constexpr explicit iterator(cstring_t*p)noexcept : _pointer(p) {}
			iterator& operator=(ar_t<iterator>)&noexcept = default;

			iterator& operator++()&noexcept {return ++_pointer,*this;}
			iterator operator++(int)&noexcept {return iterator(_pointer++);}
			iterator& operator--()&noexcept {return --_pointer,*this;}
			iterator operator--(int)&noexcept {return iterator(_pointer--);}
			iterator& operator+=(difference_type n)&noexcept {return _pointer += n,*this;}
			iterator& operator-=(difference_type n)&noexcept {return _pointer -= n,*this;}

			friend iterator operator+(iterator i,difference_type n)noexcept {return iterator(i._pointer + n);}
			friend iterator operator+(difference_type n,iterator i)noexcept {return iterator(n + i._pointer);}
			friend iterator operator-(iterator i,difference_type n)noexcept {return iterator(i._pointer - n);}
			friend difference_type operator-(iterator b,iterator a)noexcept {return b._pointer - a._pointer;}
			
			friend bool operator==(iterator a,iterator b)noexcept = default;
			friend std::strong_ordering operator<=>(iterator a,iterator b)noexcept = default;

			reference operator*()const noexcept {return *_pointer;}
			pointer operator->()const noexcept {return *_pointer;}
			reference operator[](difference_type n)const noexcept {return _pointer[n];}
		};
		using reference = value_type;

		using const_reference = reference;
		using const_iterator = iterator;
		
		argument(int _argc,char *_argv[])noexcept:argc(_argc),argv(const_cast<cstring_t*>(_argv)){}
		argument& operator=(ar_t<argument>)&noexcept = default;

		constexpr size_type size()const noexcept
		{
			return argc;
		}
		constexpr iterator begin()const noexcept
		{
			return iterator(argv);
		}
		constexpr iterator end()const noexcept
		{
			return iterator(argv+argc);
		}

		reference operator[](size_type index)const noexcept
		{
			return argv[index];
		}
	};
}