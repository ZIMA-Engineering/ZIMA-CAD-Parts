#include "partstoolsdialog.h"
#include "settings.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <memory>

QString PartsToolsDialog::title(const QString &tool)
{
    if (tool == "ps2pdf") return tr("PostScript to PDF");
    if (tool == "ptc-clean") return tr("Clean PTC files");
    return tr("Edit STEP header");
}
PartsToolsDialog::PartsToolsDialog(const QString &tool, const QString &path, QWidget *parent)
    : QDialog(parent), m_tool(tool)
{
    setWindowTitle(title(tool));
    setObjectName("partsToolsDialog");
    resize(850, 620);
    auto layout = new QVBoxLayout(this);
    m_options = new QWidget(this);
    auto form = new QFormLayout(m_options);
    auto sourceRow = new QHBoxLayout;
    m_path = new QLineEdit(path, this);
    m_path->setObjectName("toolPath");
    sourceRow->addWidget(m_path, 1);
    auto file = new QPushButton(tr("File..."), this);
    auto folder = new QPushButton(tr("Directory..."), this);
    sourceRow->addWidget(file); sourceRow->addWidget(folder);
    form->addRow(tr("Source"), sourceRow);
    connect(file, &QPushButton::clicked, this, [this] {
        const auto value = QFileDialog::getOpenFileName(this, title(m_tool), m_path->text(),
            m_tool == "ps2pdf" ? "PostScript (*.ps *.eps *.plt)" : m_tool == "step-edit" ? "STEP (*.stp *.step)" : QString());
        if (!value.isEmpty()) m_path->setText(value);
    });
    connect(folder, &QPushButton::clicked, this, [this] {
        const auto value = QFileDialog::getExistingDirectory(this, title(m_tool), m_path->text());
        if (!value.isEmpty()) m_path->setText(value);
    });
    m_recursive = new QCheckBox(tr("Include subdirectories"), this);
    m_recursive->setChecked(Settings::get()->ToolsRecursive);
    form->addRow(m_recursive);
    if (tool == "ps2pdf") {
        m_output = new QLineEdit(this);
        m_output->setPlaceholderText(tr("Beside each source file"));
        auto outputRow = new QHBoxLayout;
        outputRow->addWidget(m_output);
        auto browse = new QPushButton(tr("Directory..."), this); outputRow->addWidget(browse);
        connect(browse, &QPushButton::clicked, this, [this] {
            const auto value = QFileDialog::getExistingDirectory(this, tr("Output directory"), m_path->text());
            if (!value.isEmpty()) m_output->setText(value);
        });
        form->addRow(tr("Output directory"), outputRow);
        form->addRow(new QLabel(tr("Existing PDFs are kept. PLT input must contain PostScript."), this));
        if (PartsCore::ghostscriptExecutable().isEmpty())
            form->addRow(new QLabel(tr("Ghostscript is missing. Repair the Parts runtime package."), this));
    } else if (tool == "ptc-clean") {
        m_old = new QCheckBox(tr("Remove old numbered versions; keep the highest number"), this);
        m_old->setChecked(Settings::get()->ToolsCleanOld);
        form->addRow(m_old);
        m_masks = new QLineEdit(Settings::get()->ToolsCleanMasks.join(';'), this);
        m_masks->setPlaceholderText("*.log;trail.txt.*");
        form->addRow(tr("Additional removal masks (semicolon separated)"), m_masks);
        form->addRow(new QLabel(tr("Selected files go to the trash. Directory locks are respected."), this));
    } else {
        const QStringList names{"name", "date", "author", "organization", "preprocessor", "system", "authorization"};
        const QStringList labels{tr("File name"), tr("Date"), tr("Author"), tr("Organization"),
            tr("Preprocessor"), tr("Originating system"), tr("Authorization")};
        for (int i = 0; i < names.size(); ++i) {
            auto check = new QCheckBox(labels[i], this);
            auto edit = new QLineEdit(this);
            edit->setEnabled(false);
            form->addRow(check, edit);
            m_fields[names[i]] = {check, edit};
            connect(check, &QCheckBox::toggled, edit, &QLineEdit::setEnabled);
        }
        form->addRow(new QLabel(tr("Check only fields to change. Originals are backed up in 0000-index/tool-backups."), this));
    }
    layout->addWidget(m_options);
    m_files = new QTreeWidget(this);
    m_files->setObjectName("toolFiles");
    m_files->setHeaderLabels({tr("File"), tr("Result / reason")});
    m_files->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_files, 1);
    m_status = new QLabel(tr("Preview the operation, then choose files to apply."), this);
    m_status->setWordWrap(true);
    layout->addWidget(m_status);
    auto buttons = new QHBoxLayout;
    auto selectAll = new QPushButton(tr("Select all"), this);
    auto selectNone = new QPushButton(tr("Select none"), this);
    buttons->addWidget(selectAll); buttons->addWidget(selectNone); buttons->addStretch();
    m_preview = new QPushButton(tr("Preview"), this);
    m_preview->setObjectName("toolPreview");
    m_apply = new QPushButton(tr("Apply selected"), this);
    m_apply->setObjectName("toolApply"); m_apply->setEnabled(false);
    m_cancel = new QPushButton(tr("Close"), this);
    buttons->addWidget(m_preview); buttons->addWidget(m_apply); buttons->addWidget(m_cancel);
    layout->addLayout(buttons);
    for (auto edit : m_options->findChildren<QLineEdit *>()) connect(edit, &QLineEdit::textChanged, this, &PartsToolsDialog::invalidate);
    for (auto check : m_options->findChildren<QCheckBox *>()) connect(check, &QCheckBox::toggled, this, &PartsToolsDialog::invalidate);
    const auto select = [this](Qt::CheckState state) {
        if (m_worker) return;
        for (int i = 0; i < m_plan.items.size(); ++i) m_files->topLevelItem(i)->setCheckState(0, state);
    };
    connect(selectAll, &QPushButton::clicked, this, [select] { select(Qt::Checked); });
    connect(selectNone, &QPushButton::clicked, this, [select] { select(Qt::Unchecked); });
    connect(m_preview, &QPushButton::clicked, this, [this] { start(false); });
    connect(m_apply, &QPushButton::clicked, this, [this] { start(true); });
    connect(m_cancel, &QPushButton::clicked, this, &PartsToolsDialog::reject);
}
PartsToolsDialog::~PartsToolsDialog()
{
    if (m_worker) { m_worker->requestInterruption(); m_worker->wait(); }
}
void PartsToolsDialog::reject()
{
    if (m_worker) { m_worker->requestInterruption(); m_status->setText(tr("Cancelling...")); }
    else QDialog::reject();
}
void PartsToolsDialog::invalidate()
{
    m_apply->setEnabled(false);
    m_plan = {};
    m_files->clear();
}
void PartsToolsDialog::start(bool apply)
{
    if (m_worker) return;
    auto result = std::make_shared<PartsCore::ToolPlan>();
    auto applied = std::make_shared<QJsonObject>();
    auto error = std::make_shared<QString>();
    PartsCore::ToolRequest request;
    request.tool = m_tool; request.path = m_path->text(); request.recursive = m_recursive->isChecked();
    if (m_output) request.outputDirectory = m_output->text();
    if (m_old) request.oldVersions = m_old->isChecked();
    if (m_masks) {
        for (const auto &mask : m_masks->text().split(';', Qt::SkipEmptyParts))
            if (!mask.trimmed().isEmpty()) request.patterns.append(mask.trimmed());
    }
    for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
        if (it.value().first->isChecked()) request.fields[it.key()] = it.value().second->text();
    PartsCore::ToolPlan selected = m_plan;
    if (apply) {
        selected.items.clear();
        for (int i = 0; i < m_plan.items.size(); ++i)
            if (m_files->topLevelItem(i)->checkState(0) == Qt::Checked) selected.items.append(m_plan.items[i]);
        if (selected.items.isEmpty()) return;
        if (QMessageBox::question(this, title(m_tool), tr("Apply this operation to %1 selected files?").arg(selected.items.size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    }
    auto worker = QThread::create([request, selected, apply, result, applied, error] {
        try { if (apply) *applied = PartsCore::applyTool(selected); else *result = PartsCore::planTool(request); }
        catch (const QString &e) { *error = e; }
    });
    m_worker = worker;
    m_options->setEnabled(false); m_preview->setEnabled(false); m_apply->setEnabled(false); m_files->setEnabled(false);
    m_cancel->setText(tr("Cancel")); m_status->setText(tr("Working..."));
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::finished, this, [this, result, applied, error, apply] {
        m_worker = nullptr;
        m_options->setEnabled(true); m_preview->setEnabled(true); m_files->setEnabled(true); m_cancel->setText(tr("Close"));
        invalidate();
        if (!error->isEmpty()) { m_status->setText(*error); return; }
        if (apply) {
            for (const auto &kind : {"completed", "failed", "skipped"})
                for (const auto &value : (*applied)[kind].toArray()) {
                    const auto item = value.toObject();
                    new QTreeWidgetItem(m_files, {item["path"].toString(), QString::fromUtf8(QJsonDocument(item).toJson(QJsonDocument::Compact))});
                }
            m_status->setText(tr("Completed: %1. Failed: %2. Skipped: %3.")
                .arg((*applied)["completed"].toArray().size()).arg((*applied)["failed"].toArray().size()).arg((*applied)["skipped"].toArray().size()));
            if (!(*applied)["completed"].toArray().isEmpty()) emit filesChanged();
        } else {
            m_plan = *result;
            for (const auto &item : m_plan.items) {
                QString detail = !item.output.isEmpty() ? item.output : !item.keeper.isEmpty() ? tr("Keep: %1").arg(item.keeper) : QString();
                if (!item.fields.isEmpty()) detail = QString::fromUtf8(QJsonDocument(item.fields).toJson(QJsonDocument::Compact));
                auto row = new QTreeWidgetItem(m_files, {item.path, detail});
                row->setCheckState(0, Qt::Checked);
            }
            for (const auto &value : m_plan.skipped) {
                const auto item = value.toObject();
                auto row = new QTreeWidgetItem(m_files, {item["path"].toString(), item["reason"].toString()});
                row->setDisabled(true);
            }
            m_status->setText(tr("Ready: %1. Skipped: %2.").arg(m_plan.items.size()).arg(m_plan.skipped.size()));
            m_apply->setEnabled(!m_plan.items.isEmpty() && (m_tool != "step-edit" || !m_plan.request.fields.isEmpty())
                && (m_tool != "ps2pdf" || !PartsCore::ghostscriptExecutable().isEmpty()));
        }
    });
    worker->start();
}
