#ifndef PROGRESS_PARSER_H
#define PROGRESS_PARSER_H

#include <QString>
#include <optional>

// Pure parsing of pacman/yay/paru operation output, extracted so the
// progress-message and progress-bar logic can be unit tested without a
// QProgressBar/QLabel-backed dialog.
struct ProgressParseResult
{
    std::optional<QString> statusText;
    std::optional<int> currentPackage;
    std::optional<int> totalPackages;

    // Final percentage to apply to the progress bar, if any. When both the
    // "(n/total)" and a bare "NN%" pattern match the same output, the "NN%"
    // match takes precedence, mirroring the original inline parsing order.
    std::optional<int> progressPercent;
};

ProgressParseResult parseOperationProgress(const QString& output);

#endif  // PROGRESS_PARSER_H
