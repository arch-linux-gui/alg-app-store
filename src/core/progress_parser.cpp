#include "progress_parser.h"

#include <QRegularExpression>

ProgressParseResult parseOperationProgress(const QString& output)
{
    ProgressParseResult result;

    // Pattern: "downloading..." or "installing..."
    if (output.contains("downloading", Qt::CaseInsensitive))
    {
        result.statusText = "Downloading packages...";
    }
    else if (output.contains("installing", Qt::CaseInsensitive))
    {
        result.statusText = "Installing packages...";
    }
    else if (output.contains("building", Qt::CaseInsensitive))
    {
        result.statusText = "Building packages...";
    }
    else if (output.contains("checking", Qt::CaseInsensitive))
    {
        result.statusText = "Checking dependencies...";
    }
    else if (output.contains("resolving", Qt::CaseInsensitive))
    {
        result.statusText = "Resolving dependencies...";
    }

    // Pattern: "(1/5)" or "( 1/5)" to track package progress
    static const QRegularExpression packagePattern(R"(\(\s*(\d+)/(\d+)\))");
    auto match = packagePattern.match(output);
    if (match.hasMatch())
    {
        const int currentPackage = match.captured(1).toInt();
        const int totalPackages = match.captured(2).toInt();
        result.currentPackage = currentPackage;
        result.totalPackages = totalPackages;

        if (totalPackages > 0)
        {
            result.progressPercent = (currentPackage * 100) / totalPackages;
        }
    }

    // Pattern: "[##########] 100%" for download progress
    static const QRegularExpression percentPattern(R"(\s+(\d+)%\s*)");
    auto percentMatch = percentPattern.match(output);
    if (percentMatch.hasMatch())
    {
        result.progressPercent = percentMatch.captured(1).toInt();
    }

    return result;
}
