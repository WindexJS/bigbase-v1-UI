#pragma once
#include "Game.hpp"

namespace Big
{
	class ScriptGlobal
	{
	public:
		constexpr explicit ScriptGlobal(std::size_t index):
			m_Index(index)
		{}

		constexpr ScriptGlobal Add(std::size_t index)
		{
			return ScriptGlobal(m_Index + index);
		}

		constexpr ScriptGlobal Add(std::size_t index, std::size_t x)
		{
			return Add(1 + (index * x));
		}

		template <typename T>
		std::enable_if_t<std::is_pointer<T>::value, T> As() const
		{
			return reinterpret_cast<T>(IndexToPtr(m_Index));
		}

		template <typename T>
		std::enable_if_t<std::is_lvalue_reference<T>::value, T> As() const
		{
			return *reinterpret_cast<std::add_pointer_t<std::remove_reference_t<T>>>(IndexToPtr(m_Index));
		}

		template <typename T>
		std::enable_if_t<std::is_same<T, std::uintptr_t>::value, T> As() const
		{
			return reinterpret_cast<std::uintptr_t>(As<void*>());
		}
	private:
		static void* IndexToPtr(std::size_t index)
		{
			return (g_GameVariables->m_GlobalBase[index >> 0x12 & 0x3F]) + (index & 0x3FFFF);
		}

		std::uintptr_t m_Index;
	};
}
