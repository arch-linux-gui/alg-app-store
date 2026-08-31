#ifndef AUR_HELPER_H
#define AUR_HELPER_H

#include "../utils/types.h"
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVector>
#include <memory>

/**
 * @brief Helper class for interacting with the Arch User Repository (AUR).
 * 
 * Memory Management:
 * - m_networkManager: Owned by std::unique_ptr for RAII-style cleanup
 * - Network replies are managed via Qt parent-child and deleteLater()
 */
class AurHelper : public QObject
{
    Q_OBJECT

public:
    explicit AurHelper(QObject* parent = nullptr);
    ~AurHelper() override;

    void searchPackages(const QString& query);
    void getPackageInfo(const QString& packageName);
    QVector<UpdateInfo> checkAurUpdates();

    // Pure JSON -> PackageInfo mapping, exposed as a static so it can be unit
    // tested without a QNetworkAccessManager or a live AUR request.
    static PackageInfo parseAurPackage(const QJsonObject& obj);

signals:
    void searchCompleted(const QVector<PackageInfo>& results);
    void packageInfoReceived(const PackageInfo& info);
    void error(const QString& message);

private slots:
    void onSearchFinished();
    void onPackageInfoFinished();

private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
};

#endif  // AUR_HELPER_H
