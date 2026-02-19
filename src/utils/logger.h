#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace Logger
{
inline void
init()
{
    auto console_sink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();
    console_sink->set_level( spdlog::level::debug );

    auto file_sink = std::make_shared< spdlog::sinks::basic_file_sink_mt >( "logs/app.log", true );
    file_sink->set_level( spdlog::level::debug );

    std::vector< spdlog::sink_ptr > sinks { console_sink, file_sink };
    auto logger = std::make_shared< spdlog::logger >( "multi_sink", sinks.begin(), sinks.end() );

    spdlog::set_default_logger( logger );
    spdlog::set_level( spdlog::level::debug );
    spdlog::flush_on( spdlog::level::debug );
}

inline void
info( const QString& message )
{
    spdlog::info( message.toStdString() );
}

inline void
warning( const QString& message )
{
    spdlog::warn( message.toStdString() );
}

inline void
error( const QString& message )
{
    spdlog::error( message.toStdString() );
}

inline void
debug( const QString& message )
{
    spdlog::debug( message.toStdString() );
}
}  // namespace Logger

#endif  // LOGGER_H
