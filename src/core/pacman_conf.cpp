#include "pacman_conf.h"

namespace PacmanConf
{

bool isMultilibEnabled(const QString& contents)
{
    bool inMultilibSection = false;

    const QStringList lines = contents.split('\n');
    for (const QString& rawLine : lines)
    {
        const QString line = rawLine.trimmed();

        // Check for [multilib] section header
        if (line == "[multilib]")
        {
            inMultilibSection = true;
            continue;
        }

        // If we found [multilib] section, check if it's not commented
        if (inMultilibSection && !line.isEmpty() && !line.startsWith("#"))
        {
            // If we find Include directive, multilib is enabled
            if (line.startsWith("Include"))
            {
                return true;
            }
        }

        // If we hit another section, stop
        if (inMultilibSection && line.startsWith("[") && line != "[multilib]")
        {
            break;
        }
    }

    return false;
}

bool isChaoticAurEnabled(const QString& contents)
{
    bool inChaoticAurSection = false;

    const QStringList lines = contents.split('\n');
    for (const QString& rawLine : lines)
    {
        const QString line = rawLine.trimmed();

        // Check for [chaotic-aur] section header
        if (line == "[chaotic-aur]")
        {
            inChaoticAurSection = true;
            continue;
        }

        // If we found [chaotic-aur] section, check if it's not commented
        if (inChaoticAurSection && !line.isEmpty() && !line.startsWith("#"))
        {
            // If we find Include or Server directive, chaotic-aur is enabled
            if (line.startsWith("Include") || line.startsWith("Server"))
            {
                return true;
            }
        }

        // If we hit another section, stop
        if (inChaoticAurSection && line.startsWith("[") && line != "[chaotic-aur]")
        {
            break;
        }
    }

    return false;
}

}  // namespace PacmanConf
