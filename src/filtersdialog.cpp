#include "filtersdialog.h"
#include "localfilters.h"
#include "settings.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

FiltersDialog::FiltersDialog(const QString &directory, QWidget *parent)
    : QDialog(parent), m_directory(directory)
{
    setWindowTitle(tr("Filters"));
    resize(520, 360);
    auto layout = new QVBoxLayout(this);
    auto explanation = new QLabel(tr("Show all files except the names or patterns below.\n"
        "One pattern per line, for example *.bak or Thumbs.db.\n"
        "These settings apply only to this folder; subfolders do not inherit them."), this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    LocalFilters filters;
    filters.load(directory, Settings::get()->ShowProeVersions);
    m_hidden = new QPlainTextEdit(this);
    m_hidden->setPlainText(filters.hidden.join('\n'));
    layout->addWidget(m_hidden);
    m_versions = new QCheckBox(tr("Show all Pro/E versions (otherwise only the newest)"), this);
    m_versions->setChecked(filters.showVersions);
    layout->addWidget(m_versions);
    m_zimaVersions = new QCheckBox(tr("Hide ZIMA-CAD archive versions (.1, .2, ...)"), this);
    m_zimaVersions->setObjectName("hideZimaVersions");
    m_zimaVersions->setChecked(!filters.showZimaVersions);
    layout->addWidget(m_zimaVersions);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &FiltersDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
void FiltersDialog::accept()
{
    const QString path = LocalFilters::filePath(m_directory);
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        QMessageBox::warning(this, tr("Filters"), tr("Cannot create the settings folder."));
        return;
    }
    QStringList hidden;
    for (const QString &line : m_hidden->toPlainText().split('\n')) {
        if (!line.trimmed().isEmpty())
            hidden.append(line.trimmed());
    }
    hidden.removeDuplicates();
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("Filters/Hide", hidden);
    settings.setValue("Filters/ShowVersions", m_versions->isChecked());
    settings.setValue("Filters/ShowZimaVersions", !m_zimaVersions->isChecked());
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        QMessageBox::warning(this, tr("Filters"), tr("Cannot save the filters."));
        return;
    }
    QDialog::accept();
}
