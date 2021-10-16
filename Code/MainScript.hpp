#pragma once
#include "Script.hpp"

namespace Big
{
	class MainScript : public Script
	{
	public:
		explicit MainScript() = default;
		~MainScript() noexcept = default;

		bool IsInitialized() override;
		ScriptType GetType() override;
		void Initialize() override;
		void Destroy() override;
		void Tick() override;
	private:
		bool m_Initialized{};
	};

	inline std::shared_ptr<MainScript> g_MainScript;
}
