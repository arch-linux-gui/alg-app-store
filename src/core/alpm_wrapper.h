#ifndef ALPM_WRAPPER_H
#define ALPM_WRAPPER_H

#include <alpm.h>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <mutex>
#include "../utils/types.h"

class AlpmWrapper {
public:
    static AlpmWrapper& instance();
    
    ~AlpmWrapper();
    
    // Disable copy and move
    AlpmWrapper(const AlpmWrapper&) = delete;
    AlpmWrapper& operator=(const AlpmWrapper&) = delete;
    AlpmWrapper(AlpmWrapper&&) = delete;
    AlpmWrapper& operator=(AlpmWrapper&&) = delete;
    
    bool initialize();
    void release();
    
    QVector<PackageInfo> searchPackages(const QString& query);
    QVector<PackageInfo> getInstalledPackages();
    bool isPackageInstalled(const QString& packageName);
    PackageInfo getPackageInfo(const QString& packageName);
    QVector<UpdateInfo> getAvailableUpdates();
    
private:
    AlpmWrapper();
    
    alpm_handle_t* m_handle;
    alpm_list_t* m_syncDbs;
    std::mutex m_mutex;
    bool m_initialized;
    
    QStringList convertDependList(alpm_list_t* deps);
    void searchInDatabase(alpm_db_t* db, const QString& query, 
                         QVector<PackageInfo>& results);
};

#endif // ALPM_WRAPPER_H
