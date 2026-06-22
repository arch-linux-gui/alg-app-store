#include "auth_manager.h"
#include "../utils/logger.h"
#include <QProcess>
#include <QInputDialog>
#include <QMessageBox>

AuthManager& AuthManager::instance() {
    static AuthManager instance;
    return instance;
}

AuthManager::AuthManager()
    : QObject(nullptr) {
}

AuthManager::~AuthManager() {
    // Zero out the password in memory
    m_password.fill('\0');
    m_password.clear();
}

bool AuthManager::authenticate(QWidget* parent) {
    if (m_authenticated) {
        return true;
    }

    const int maxAttempts = 3;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        bool ok = false;
        QString prompt = "Enter your password to continue:";
        if (attempt > 0) {
            prompt = QString("Incorrect password. Attempt %1 of %2.\nEnter your password:")
                         .arg(attempt + 1)
                         .arg(maxAttempts);
        }

        QString password = QInputDialog::getText(
            parent,
            "Authentication Required",
            prompt,
            QLineEdit::Password,
            QString(),
            &ok
        );

        if (!ok) {
            // User cancelled
            Logger::info("Authentication cancelled by user");
            return false;
        }

        QByteArray passwordBytes = password.toUtf8();

        if (validatePassword(passwordBytes)) {
            m_password = passwordBytes;
            m_authenticated = true;
            Logger::info("Authentication successful");
            return true;
        }

        Logger::warning(QString("Authentication failed (attempt %1/%2)")
                            .arg(attempt + 1)
                            .arg(maxAttempts));
    }

    QMessageBox::critical(parent, "Authentication Failed",
        "Maximum authentication attempts exceeded.\n"
        "The application will now exit.");

    return false;
}

bool AuthManager::isAuthenticated() const {
    return m_authenticated;
}

void AuthManager::writePasswordToProcess(QProcess* process) {
    if (!process || !m_authenticated) {
        return;
    }

    process->write(m_password + "\n");
    process->closeWriteChannel();
}

bool AuthManager::validatePassword(const QByteArray& password) {
    QProcess process;
    process.start("sudo", QStringList() << "-S" << "-p" << "" << "true");

    if (!process.waitForStarted(3000)) {
        Logger::error("Failed to start sudo validation process");
        return false;
    }

    process.write(password + "\n");
    process.closeWriteChannel();

    process.waitForFinished(10000);

    return process.exitCode() == 0;
}
