// SPDX-License-Identifier: GPL-3.0-or-later
#include "aisettingspage.h"
#include "aiprovider.h"
#include "settings.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDesktopServices>
#include <QEvent>
#include <QSignalBlocker>

AiSettingsPage::AiSettingsPage(QWidget *parent) : QWidget(parent), m_provider(partsAiProvider())
{
    setObjectName("aiSettingsPage");
    auto layout = new QVBoxLayout(this);
    m_intro = new QLabel(this); m_intro->setWordWrap(true); layout->addWidget(m_intro);
    auto form = new QFormLayout;
    m_pathLabel = new QLabel(this); m_modelLabel = new QLabel(this);
    m_pathLabel->setObjectName("aiExecutableLabel");
    m_executable = new QLineEdit(this); m_executable->setObjectName("aiExecutable");
    m_executable->setText(Settings::get()->AiCodexExecutable.isEmpty() ? findCodexExecutable() : Settings::get()->AiCodexExecutable);
    auto pathRow = new QHBoxLayout;
    pathRow->addWidget(m_executable);
    m_browse = new QPushButton(this); pathRow->addWidget(m_browse); form->addRow(m_pathLabel, pathRow);
    m_model = new QComboBox(this); m_model->setObjectName("aiModel");
    m_model->addItem(QString(), QString());
    if (!Settings::get()->AiModel.isEmpty()) { m_model->addItem(Settings::get()->AiModel, Settings::get()->AiModel); m_model->setCurrentIndex(1); }
    form->addRow(m_modelLabel, m_model); layout->addLayout(form);
    auto buttons = new QHBoxLayout;
    m_connect = new QPushButton(this); m_connect->setObjectName("aiConnect");
    m_login = new QPushButton(this); m_login->setObjectName("aiLogin");
    m_logout = new QPushButton(this); m_logout->setObjectName("aiLogout");
    m_cancel = new QPushButton(this);
    for (auto button : {m_connect, m_login, m_logout, m_cancel}) buttons->addWidget(button);
    buttons->addStretch(); layout->addLayout(buttons);
    m_status = new QLabel(this); m_status->setObjectName("aiStatus");
    m_status->setWordWrap(true); m_status->setTextFormat(Qt::PlainText); layout->addWidget(m_status);
    m_help = new QPushButton(this); layout->addWidget(m_help, 0, Qt::AlignLeft); layout->addStretch();
    connect(m_browse, &QPushButton::clicked, this, [this] {
        const auto path = QFileDialog::getOpenFileName(this, tr("Codex executable"), m_executable->text());
        if (!path.isEmpty()) m_executable->setText(path);
    });
    connect(m_connect, &QPushButton::clicked, this, [this] {
        Settings::get()->AiCodexExecutable = m_executable->text().trimmed();
        Settings::get()->save();
        m_provider->connectAccount(Settings::get()->AiCodexExecutable);
    });
    connect(m_login, &QPushButton::clicked, m_provider, &AiProvider::login);
    connect(m_logout, &QPushButton::clicked, m_provider, &AiProvider::logout);
    connect(m_cancel, &QPushButton::clicked, m_provider, &AiProvider::cancel);
    connect(m_provider, &AiProvider::changed, this, &AiSettingsPage::refresh);
    connect(m_help, &QPushButton::clicked, this, [] { QDesktopServices::openUrl(QUrl("https://developers.openai.com/codex/cli")); });
    retranslate(); refresh();
}

void AiSettingsPage::save()
{
    Settings::get()->AiCodexExecutable = m_executable->text().trimmed();
    Settings::get()->AiModel = m_model->currentData().toString();
}

void AiSettingsPage::refresh()
{
    m_connect->setEnabled(!m_provider->busy());
    m_login->setEnabled(m_provider->connected() && !m_provider->busy() && !m_provider->ready());
    m_logout->setEnabled(m_provider->ready() && !m_provider->busy());
    m_cancel->setEnabled(m_provider->busy());
    m_executable->setEnabled(!m_provider->busy()); m_browse->setEnabled(!m_provider->busy());
    m_status->setText(m_provider->status());
    const auto selected = m_model->currentData().toString();
    QSignalBlocker blocker(m_model);
    for (const auto &value : m_provider->models()) {
        const auto model = value.toObject();
        const auto id = model["model"].toString();
        if (!id.isEmpty() && m_model->findData(id) < 0)
            m_model->addItem(model["displayName"].toString(id), id);
    }
    m_model->setCurrentIndex(qMax(0, m_model->findData(selected)));
}

void AiSettingsPage::retranslate()
{
    m_intro->setText(tr("Use Codex with your own ChatGPT account. Type codex in the command panel to start; /exit returns to Parts commands. "
        "AI receives the active and working directory paths and reads files only through requested tools. Requested text and tool results are sent to OpenAI. "
        "File changes require your approval. Account limits apply. Connection and sign-in buttons act immediately."));
    m_pathLabel->setText(tr("Codex executable")); m_modelLabel->setText(tr("Model"));
    m_pathLabel->setMinimumWidth(m_pathLabel->sizeHint().width());
    m_status->setText(m_provider->status());
    m_model->setItemText(0, tr("Codex default"));
    m_browse->setText(tr("Browse...")); m_connect->setText(tr("Connect"));
    m_login->setText(tr("Sign in with ChatGPT")); m_logout->setText(tr("Sign out"));
    m_cancel->setText(tr("Cancel")); m_help->setText(tr("Get Codex / installation instructions"));
}

void AiSettingsPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) retranslate();
    QWidget::changeEvent(event);
}
