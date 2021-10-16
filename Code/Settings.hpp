#pragma once

namespace Big
{
	class Settings
	{
	public:
		std::atomic_bool m_LogWindow = true;
		std::atomic_bool m_LockMouse = false;
	};

	inline Settings g_Settings;
}
