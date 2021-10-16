#pragma once
#include "BaseSubmenu.hpp"
#include "../Natives.hpp"

namespace Big::UserInterface
{
	class PlayerSubmenu : public BaseSubmenu<PlayerSubmenu>
	{
	public:
		explicit PlayerSubmenu() = default;
		~PlayerSubmenu() noexcept = default;
		explicit PlayerSubmenu(std::uint32_t* selectedPlayer, std::uint32_t id, std::function<void(PlayerSubmenu*)> action):
			m_SelectedPlayer(selectedPlayer)
		{
			SetId(id);
			SetAction(std::move(action));
		}

		PlayerSubmenu(PlayerSubmenu const&) = default;
		PlayerSubmenu& operator=(PlayerSubmenu const&) = default;
		PlayerSubmenu(PlayerSubmenu&&) = default;
		PlayerSubmenu& operator=(PlayerSubmenu&&) = default;

		const char* GetName() override
		{
			return PLAYER::GET_PLAYER_NAME(*m_SelectedPlayer);
		}
	private:
		std::uint32_t* m_SelectedPlayer;
	};
}
