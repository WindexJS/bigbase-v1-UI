#pragma once
#include "Script.hpp"

namespace Big
{
	class ScriptManager
	{
	public:
		explicit ScriptManager();
		~ScriptManager() noexcept;
		ScriptManager(Logger const&) = delete;
		ScriptManager(Logger&&) = delete;
		ScriptManager& operator=(Logger const&) = delete;
		ScriptManager& operator=(Logger&&) = delete;

		/**
		 * \brief Registers a script
		 * \param script The script to be registered
		 */
		void AddScript(std::shared_ptr<Script> script);

		/**
		 * \brief Checks if the script is initialized
		 * \param script The script to be unregistered
		 */
		void RemoveScript(Script* script);

		/**
		 * \brief Runs a tick on the script thread
		 */
		void OnGameTick();

		/**
		 * \brief Runs a tick on the D3D thread
		 */
		void OnD3DTick();
	private:
		std::mutex m_Mutex;
		std::vector<std::shared_ptr<Script>> m_Scripts;
	};

	inline std::unique_ptr<ScriptManager> g_ScriptManager;
}
