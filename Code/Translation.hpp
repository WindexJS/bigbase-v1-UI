#pragma once
#include <JSON/json.hpp>

namespace Big
{
	class TranslationManager
	{
	public:
		explicit TranslationManager()
		{
			std::snprintf(&m_TranslationDir[0], sizeof(m_TranslationDir) - 1, "%s\\" BIGBASE_NAME "\\Translations\\", std::getenv("appdata"));
			try
			{
				if (!std::filesystem::exists(&m_TranslationDir[0]))
					std::filesystem::create_directory(&m_TranslationDir[0]);
			}
			catch (fs::filesystem_error const&)
			{
			}
		}

		/**
		 * \brief Loads a translation file
		 * \param name The stem of the translation file
		 */
		void LoadTranslations(const char* name)
		{
			m_Translations.clear();
			g_Logger->Info("Loading %s translations.", name);

			try
			{
				char filename[100] = {};
				std::snprintf(&filename[0], sizeof(filename) - 1, "%s.big", name);
				auto filePath = std::filesystem::path(m_TranslationDir).append(filename);

				std::ifstream file(filePath);
				if (file.good())
				{
					std::stringstream str;
					std::string line;
					while (std::getline(file, line))
					{
						str << line << '\n';
					}

					nlohmann::json json = nlohmann::json::parse(str, nullptr, false);
					auto objects = json.get<std::unordered_map<std::string, nlohmann::json>>();

					for (auto&&[key, value] : objects)
					{
						m_Translations.emplace(Joaat(key.c_str()), value.get<std::string>());
					}
				}
			}
			catch (nlohmann::detail::exception const&)
			{
				//g_Logger->Info("Exception while parsing %s translations: %s", name, ex.what());
				g_Logger->Info("Failed to parse %s translations.", name);
			}
		}

		/**
		 * \brief Loads a translation file
		 * \param label The hash of the label
		 * \return The translation string, or a placeholder string
		 */
		const char* GetTranslation(std::uint32_t label)
		{
			if (auto it = m_Translations.find(label); it != m_Translations.end())
			{
				return &(it->second)[0];
			}
			else
			{
				static char buf[64];

				std::fill(std::begin(buf), std::end(buf), '\0');
				std::snprintf(&buf[0], sizeof(buf) - 1, "Unknown (0x%08X)", label);
				
				return &buf[0];
			}
		}

		/**
		 * \brief Gets the directory of the translations
		 * \return const char*
		 */
		const char* GetTranslationDirectory()
		{
			return &m_TranslationDir[0];
		}

		~TranslationManager() noexcept = default;
		TranslationManager(TranslationManager const&) = delete;
		TranslationManager(TranslationManager&&) = delete;
		TranslationManager& operator=(TranslationManager const&) = delete;
		TranslationManager& operator=(TranslationManager&&) = delete;
	private:
		std::unordered_map<std::uint32_t, std::string> m_Translations;
		char m_TranslationDir[1024] = {};
	};

	inline std::unique_ptr<TranslationManager> g_TranslationManager;
}

#define BIG_TRANSLATE_IMPL(label) (::Big::g_TranslationManager->GetTranslation(CONSTEXPR_JOAAT(label)))
#define BIG_TRANSLATE(label) BIG_TRANSLATE_IMPL(label)
