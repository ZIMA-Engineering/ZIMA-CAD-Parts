#include "updatespage.h"
#include "updateservice.h"
#include "update/installationclient.h"
#include "settings.h"
#include "zima-cad-parts.h"
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QDateTime>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QSettings>
#include <QVBoxLayout>

UpdatesPage::UpdatesPage(QWidget *parent) : QWidget(parent)
{
    setObjectName("updatesPage");
    auto layout = new QVBoxLayout(this);
    m_automatic = new QCheckBox(this); m_automatic->setChecked(Settings::get()->UpdatesAutomatic);
    m_status = new QLabel(this); m_status->setObjectName("updateStatus"); m_status->setWordWrap(true);
    m_versions = new QLabel(this); m_versions->setWordWrap(true);
    m_detail = new QLabel(this); m_detail->setWordWrap(true); m_detail->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_notes = new QPlainTextEdit(this); m_notes->setReadOnly(true); m_notes->setMaximumBlockCount(500);
    layout->addWidget(m_automatic); layout->addWidget(m_versions); layout->addWidget(m_status);
    m_progress = new QProgressBar(this); m_progress->setObjectName("updateProgress");
    layout->addWidget(m_progress);
    layout->addWidget(m_notes, 1); layout->addWidget(m_detail);
    auto buttons = new QHBoxLayout;
    m_check = new QPushButton(this); m_check->setObjectName("checkForUpdates");
    m_install = new QPushButton(this); m_install->setObjectName("installUpdate");
    m_rollback = new QPushButton(this); m_cancel = new QPushButton(this);
    for (auto button : {m_check, m_install, m_rollback, m_cancel}) buttons->addWidget(button);
    layout->addLayout(buttons);
    connect(m_check, &QPushButton::clicked, UpdateService::get(), &UpdateService::check);
    connect(m_cancel, &QPushButton::clicked, UpdateService::get(), &UpdateService::cancel);
    connect(m_install, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, tr("Install update"), tr("Download and install the update, then restart Parts? Your project files will be preserved.")) == QMessageBox::Yes)
            UpdateService::get()->install();
    });
    connect(m_rollback, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, tr("Return to previous version"), tr("Restart Parts with the previous verified version? This does not restore older project data.")) == QMessageBox::Yes)
            UpdateService::get()->rollback();
    });
    connect(UpdateService::get(), &UpdateService::changed, this, &UpdatesPage::refresh);
    retranslate(); refresh();
}
void UpdatesPage::save() { Settings::get()->UpdatesAutomatic = m_automatic->isChecked(); }
void UpdatesPage::retranslate()
{
    m_automatic->setText(tr("Check for updates silently at startup"));
    m_check->setText(tr("Check for updates")); m_install->setText(tr("Install update"));
    m_rollback->setText(tr("Return to previous version")); m_cancel->setText(tr("Cancel download"));
    m_notes->setPlaceholderText(tr("Release notes will appear here."));
}
void UpdatesPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) { retranslate(); refresh(); }
    QWidget::changeEvent(event);
}
void UpdatesPage::refresh()
{
    const auto service = UpdateService::get();
    const auto offer = service->offer();
    m_versions->setText(tr("Installed version: %1").arg(VERSION) + (offer.isEmpty() ? QString() : '\n' + tr("Available version: %1").arg(offer["availableVersion"].toString())));
    QString status;
    if (!service->error().isEmpty()) status = tr("Update check or installation failed. See details below.");
    else if (service->phase() == "checking") status = tr("Checking for updates...");
    else if (service->phase() == "downloading") status = tr("Downloading update...");
    else if (service->phase() == "verifying") status = tr("Verifying the downloaded update...");
    else if (service->phase() == "waiting") status = tr("Waiting for Parts to close...");
    else if (!offer.isEmpty()) status = tr("A new version is available.");
    else if (service->phase() == "current") status = tr("No newer compatible release is available.");
    else status = tr("Updates have not been checked yet.");
    m_status->setText(status);
    m_progress->setVisible(service->busy());
    const bool known = service->phase() == "downloading" && service->totalBytes() > 0;
    m_progress->setRange(0, known ? 100 : 0);
    if (known) m_progress->setValue(int(100 * service->receivedBytes() / service->totalBytes()));
    if (m_notes->toPlainText() != offer["notes"].toString()) m_notes->setPlainText(offer["notes"].toString());
    QSettings settings;
    QString detail = tr("Last successful check: %1").arg(settings.value("Updates/LastCheck", tr("Never")).toString());
    if (!offer.isEmpty()) detail += '\n' + tr("Download size: %1 MiB").arg(offer["manifest"].toObject()["archive"].toObject()["size"].toDouble() / 1048576.0, 0, 'f', 1);
    if (partsInstallationRoot().isEmpty()) detail += '\n' + tr("This is a development or custom copy. Use the distribution launcher to install updates.");
    if (!service->error().isEmpty()) detail += '\n' + service->error();
    if (!offer["reason"].toString().isEmpty()) detail += '\n' + offer["reason"].toString();
    for (auto label : {m_versions, m_status, m_detail}) label->setTextFormat(Qt::PlainText);
    m_detail->setText(detail);
    m_check->setEnabled(!service->busy()); m_install->setEnabled(!service->busy() && offer["installable"].toBool());
    m_cancel->setEnabled(service->busy() && service->phase() != "waiting");
    bool previous = false;
    const auto root = partsInstallationRoot();
    if (!root.isEmpty()) {
        QFile journal(root + "/.updates/" +
#ifdef Q_OS_WIN
            "windows-x64"
#else
            "debian-13-x86_64"
#endif
            + "-journal.json");
        if (journal.open(QIODevice::ReadOnly)) {
            const auto record = QJsonDocument::fromJson(journal.readAll()).object();
            previous = record["phase"] == "committed" && !record["previous"].toString().isEmpty();
        }
    }
    m_rollback->setEnabled(!service->busy() && previous);
}
