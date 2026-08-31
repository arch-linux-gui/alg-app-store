#ifndef PACMAN_CONF_H
#define PACMAN_CONF_H

#include <QString>
#include <QStringList>

// Pure section-scanning helpers for pacman.conf content. Take the file
// contents as a QString rather than a path so callers (and tests) can feed
// in fixture text instead of hitting /etc/pacman.conf directly.
namespace PacmanConf
{

bool isMultilibEnabled(const QString& contents);
bool isChaoticAurEnabled(const QString& contents);

}  // namespace PacmanConf

#endif  // PACMAN_CONF_H
