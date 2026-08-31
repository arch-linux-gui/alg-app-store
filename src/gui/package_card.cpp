#include "package_card.h"
#include "../core/alpm_wrapper.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>

PackageCard::PackageCard(const PackageInfo& info, QWidget* parent)
    : QWidget(parent)
    , m_info(info)
    , m_nameLabel(new QLabel(this))
    , m_descriptionLabel(new QLabel(this))
    , m_versionLabel(new QLabel(this))
    , m_repositoryLabel(new QLabel(this))
    , m_statusLabel(new QLabel(this))
{

    setupUi();
    checkInstallStatus();
}

void PackageCard::setupUi()
{
    setMinimumHeight(180);
    setMaximumHeight(220);
    setMinimumWidth(280);
    setCursor(Qt::PointingHandCursor);

    setProperty("class", "package-card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // Header with name and status
    auto* headerLayout = new QHBoxLayout();

    m_nameLabel->setText(m_info.name);
    m_nameLabel->setObjectName("card-name");
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    headerLayout->addWidget(m_nameLabel, 1);

    m_statusLabel->setProperty("class", "status-badge");
    m_statusLabel->hide();
    headerLayout->addWidget(m_statusLabel, 0);

    mainLayout->addLayout(headerLayout);

    // Description - with word wrap and proper sizing
    m_descriptionLabel->setText(m_info.description);
    m_descriptionLabel->setObjectName("card-description");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_descriptionLabel->setMinimumHeight(40);
    m_descriptionLabel->setMaximumHeight(70);
    mainLayout->addWidget(m_descriptionLabel, 1);

    // Version
    m_versionLabel->setText(QString("Version: %1").arg(m_info.version));
    m_versionLabel->setObjectName("card-version");
    mainLayout->addWidget(m_versionLabel, 0);

    // Repository badge
    m_repositoryLabel->setText(m_info.repository);
    m_repositoryLabel->setProperty("class", "repo-badge");
    m_repositoryLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_repositoryLabel, 0);

    setLayout(mainLayout);
}

void PackageCard::checkInstallStatus()
{
    m_isInstalled = AlpmWrapper::instance().isPackageInstalled(m_info.name);
    updateInstallStatus(m_isInstalled);
}

void PackageCard::updateInstallStatus(bool installed)
{
    m_isInstalled = installed;

    if (m_isInstalled)
    {
        m_statusLabel->setText("Installed");
        m_statusLabel->show();
    }
    else
    {
        m_statusLabel->hide();
    }
}

void PackageCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked(m_info);
    }
    QWidget::mousePressEvent(event);
}

void PackageCard::enterEvent(QEnterEvent* event)
{
    setProperty("hovered", true);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::enterEvent(event);
}

void PackageCard::leaveEvent(QEvent* event)
{
    setProperty("hovered", false);
    style()->unpolish(this);
    style()->polish(this);
    QWidget::leaveEvent(event);
}

void PackageCard::paintEvent(QPaintEvent*)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
