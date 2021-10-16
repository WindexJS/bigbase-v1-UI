#pragma once

namespace Big
{
	enum class ScriptType
	{
		Game,
		D3D
	};

	enum class ScriptAction
	{
		D3DResizeBuffersPre,
		D3DResizeBuffersPost,
		GetLabelText
	};

	class Script
	{
	public:
		virtual ~Script() noexcept = default;
		Script(Script const&) = delete;
		Script(Script&&) = delete;
		Script& operator=(Script const&) = delete;
		Script& operator=(Script&&) = delete;

		/**
		 * \brief Checks if the script is initialized
		 * \return bool
		 */
		virtual bool IsInitialized() = 0;

		/**
		 * \brief Gets the type of the script
		 * \return ScriptType
		 */
		virtual ScriptType GetType() = 0;

		/**
		 * \brief Initializes the script
		 */
		virtual void Initialize() = 0;

		/**
		 * \brief Destroys the script
		 */
		virtual void Destroy() = 0;

		/**
		 * \brief Executes the script for a tick
		 */
		virtual void Tick() = 0;
	protected:
		Script() = default;
	};
}
