// This file defines the `Logger` class.
#pragma once

#include "_EngineIncludes.hpp"

// This namespace is polluted with code for the game engine
namespace Engine
{
	// The `Logger` class responsible for simplification of tracing the 
	// program errors. It provides the API for writing formatted log 
	// messages to the files(every logger logs into the specific file,
	// for instance `application_logs.log` and the global `general.log`
	// file).
	class Logger
	{
	public:
		// Initialize the memory that is used by the logger sinks and loggers itself
		static void initialize();

		// Declare different types of loggers.
		static std::shared_ptr<spdlog::logger> m_ApplicationLogger;
		static std::shared_ptr<spdlog::logger> m_GraphicsLogger;
		static std::shared_ptr<spdlog::logger> m_ResourceLogger;
		static std::shared_ptr<spdlog::logger> m_GameLogger;

	private:
		// Declare different types of logger sinks.
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_ApplicationSink;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_GraphicsSink;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_GeneralSink;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_GameSink;
	};
}