#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <QObject>
#include <QByteArray>

class QProcess;
class QWidget;

/**
 * @brief Singleton class for managing sudo authentication.
 *
 * Prompts for the user's password once at startup and reuses it
 * for all subsequent privileged operations via sudo -S.
 *
 * Memory Management:
 * - m_password: Zeroed out in destructor for security
 */
class AuthManager : public QObject {
    Q_OBJECT

public:
    static AuthManager& instance();

    ~AuthManager() override;

    // Disable copy and move
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;
    AuthManager(AuthManager&&) = delete;
    AuthManager& operator=(AuthManager&&) = delete;

    /**
     * @brief Show password dialog and validate credentials.
     * @param parent Parent widget for the dialog
     * @return true if authentication succeeded
     */
    bool authenticate(QWidget* parent = nullptr);

    /**
     * @brief Check if user has been authenticated this session.
     */
    bool isAuthenticated() const;

    /**
     * @brief Write the stored password to a QProcess's stdin.
     *
     * Call this after QProcess::start() for any process that uses
     * sudo -S to read the password from stdin.
     */
    void writePasswordToProcess(QProcess* process);

private:
    AuthManager();

    bool validatePassword(const QByteArray& password);

    QByteArray m_password;
    bool m_authenticated = false;
};

#endif // AUTH_MANAGER_H
