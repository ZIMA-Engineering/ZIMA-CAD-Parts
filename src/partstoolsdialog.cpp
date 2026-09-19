#include "partstoolsdialog.h"
#include "settings.h"
#include "partcache.h"
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
#include <QTimer>
#include <QSet>
#include <QHash>
#include <memory>

QString PartsToolsDialog::title(const QString &tool)
{
    if (tool == "ps2pdf") return tr("PostScript to PDF");
    if (tool == "ptc-clean") return tr("Clean PTC files");
    if (tool == "zima-clean") return tr("Clean ZIMA-CAD files");
    return tr("Edit STEP header");
}
PartsToolsDialog::PartsToolsDialog(const QString &tool, const QString &path, QWidget *parent)
    : QDialog(parent), m_tool(tool), m_sourcePath(path)
{
    setWindowTitle(title(tool));
    setObjectName("partsToolsDialog");
    resize(850, 620);
    auto layout = new QVBoxLayout(this);
    m_options = new QWidget(this);
    auto form = new QFormLayout(m_options);
    const bool automaticTool = tool != "step-edit";
    if (!automaticTool) {
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
    }
    if (tool != "ps2pdf") {
        m_recursive = new QCheckBox(tr("Include subdirectories"), this);
        m_recursive->setObjectName("toolRecursive");
        m_recursive->setChecked(Settings::get()->ToolsRecursive);
        form->addRow(m_recursive);
    }
    if (tool == "ps2pdf") {
        m_output = new QLineEdit("pdf", this);
        m_output->setObjectName("toolOutputDirectory");
        m_output->setPlaceholderText(tr("Relative to the source directory"));
        auto outputRow = new QHBoxLayout;
        outputRow->addWidget(m_output);
        auto browse = new QPushButton(tr("Directory..."), this); outputRow->addWidget(browse);
        connect(browse, &QPushButton::clicked, this, [this] {
            const auto value = QFileDialog::getExistingDirectory(this, tr("Output directory"), m_sourcePath);
            if (!value.isEmpty()) m_output->setText(value);
        });
        form->addRow(tr("Output directory"), outputRow);
        m_deleteSources = new QCheckBox(tr("Delete PS source files after creating PDF"), this);
        m_deleteSources->setObjectName("toolDeleteSources");
        m_deleteSources->setChecked(true);
        form->addRow(m_deleteSources);
        form->addRow(new QLabel(tr("Existing PDFs are replaced. PLT input must contain PostScript."), this));
        if (PartsCore::ghostscriptExecutable().isEmpty())
            form->addRow(new QLabel(tr("Ghostscript is missing. Repair the Parts runtime package."), this));
    } else if (tool == "ptc-clean") {
        m_old = new QCheckBox(tr("Remove old numbered versions; keep the highest number"), this);
        m_old->setObjectName("toolCleanOld");
        m_old->setChecked(Settings::get()->ToolsCleanOld);
        form->addRow(m_old);
        m_masks = new QLineEdit(Settings::get()->ToolsCleanMasks.join(';'), this);
        m_masks->setObjectName("toolCleanMasks");
        m_masks->setPlaceholderText("*.log;trail.txt.*");
        form->addRow(tr("Additional removal masks (semicolon separated)"), m_masks);
        form->addRow(new QLabel(tr("Selected files go to the trash. Directory locks are respected."), this));
    } else if (tool == "zima-clean") {
        form->addRow(new QLabel(tr("Remove all numbered ZIMA-CAD archives (.1, .2, ...). Current documents are kept."), this));
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
    if (!automaticTool) layout->addWidget(m_options);
    m_files = new QTreeWidget(this);
    m_files->setObjectName("toolFiles");
    if (automaticTool) {
        m_files->setColumnCount(1);
        m_files->setHeaderHidden(true);
        m_files->setRootIsDecorated(false);
    } else {
        m_files->setHeaderLabels({tr("File"), tr("Result / reason")});
    }
    m_files->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_files, 1);
    if (automaticTool) layout->addWidget(m_options);
    m_status = new QLabel(automaticTool
        ? (tool != "ps2pdf" ? tr("Choose files to clean. The list updates automatically.")
            : tr("Choose files to convert. The list updates automatically."))
        : tr("Preview the operation, then choose files to apply."), this);
    m_status->setWordWrap(true);
    m_status->setTextFormat(Qt::PlainText);
    m_status->setObjectName("toolStatus");
    layout->addWidget(m_status);
    auto buttons = new QHBoxLayout;
    auto selectAll = new QPushButton(tr("Select all"), this);
    auto selectNone = new QPushButton(tr("Select none"), this);
    buttons->addWidget(selectAll); buttons->addWidget(selectNone); buttons->addStretch();
    if (!automaticTool) {
        m_preview = new QPushButton(tr("Preview"), this);
        m_preview->setObjectName("toolPreview");
        buttons->addWidget(m_preview);
    }
    m_apply = new QPushButton(tool == "ptc-clean" || tool == "zima-clean" ? tr("Clean")
        : tool == "ps2pdf" ? tr("Create PDF") : tr("Apply selected"), this);
    if (automaticTool) m_apply->setAutoDefault(false);
    m_apply->setObjectName("toolApply"); m_apply->setEnabled(false);
    m_cancel = new QPushButton(tr("Close"), this);
    buttons->addWidget(m_apply); buttons->addWidget(m_cancel);
    layout->addLayout(buttons);
    if (automaticTool) {
        m_autoPreview = new QTimer(this);
        m_autoPreview->setSingleShot(true);
        m_autoPreview->setInterval(250);
        connect(m_autoPreview, &QTimer::timeout, this, [this] {
            if (!m_prepared) start(false);
        });
        m_autoPreview->start(0);
    }
    for (auto edit : m_options->findChildren<QLineEdit *>()) connect(edit, &QLineEdit::textChanged, this, &PartsToolsDialog::schedulePreview);
    for (auto check : m_options->findChildren<QCheckBox *>()) connect(check, &QCheckBox::toggled, this, &PartsToolsDialog::schedulePreview);
    const auto select = [this](Qt::CheckState state) {
        if (m_worker) return;
        for (int i = 0; i < m_plan.items.size(); ++i) m_files->topLevelItem(i)->setCheckState(0, state);
    };
    connect(selectAll, &QPushButton::clicked, this, [select] { select(Qt::Checked); });
    connect(selectNone, &QPushButton::clicked, this, [select] { select(Qt::Unchecked); });
    if (m_preview) connect(m_preview, &QPushButton::clicked, this, [this] { start(false); });
    connect(m_apply, &QPushButton::clicked, this, [this] { start(true); });
    connect(m_cancel, &QPushButton::clicked, this, &PartsToolsDialog::reject);
}
PartsToolsDialog::~PartsToolsDialog()
{
    if (m_worker) { m_worker->requestInterruption(); m_worker->wait(); }
    finishFileChanges();
}
void PartsToolsDialog::finishFileChanges()
{
    if (m_fileChangesPath.isEmpty()) return;
    const auto path = m_fileChangesPath;
    m_fileChangesPath.clear();
    PartCache::get()->endFileChanges(path);
}
void PartsToolsDialog::reject()
{
    if (m_autoPreview) m_autoPreview->stop();
    if (m_worker) { m_worker->requestInterruption(); m_status->setText(tr("Cancelling...")); }
    else QDialog::reject();
}
void PartsToolsDialog::invalidate()
{
    m_apply->setEnabled(false);
    m_plan = {};
    m_files->clear();
}
void PartsToolsDialog::schedulePreview()
{
    if (m_prepared) return;
    invalidate();
    if (m_autoPreview) {
        ++m_previewRevision;
        m_autoPreview->start(250);
        if (m_worker) m_worker->requestInterruption();
    }
}
void PartsToolsDialog::setPreparedPlan(const PartsCore::ToolPlan &plan)
{
    if (m_worker || plan.request.tool != m_tool) return;
    m_prepared = true;
    if (m_autoPreview) m_autoPreview->stop();
    m_sourcePath = plan.request.path;
    if (m_path) m_path->setText(plan.request.path);
    if (m_recursive) m_recursive->setChecked(plan.request.recursive);
    if (m_output) m_output->setText(plan.request.outputDirectory);
    if (m_deleteSources) m_deleteSources->setChecked(plan.request.deleteSourcesAfterConversion);
    if (m_old) m_old->setChecked(plan.request.oldVersions);
    if (m_masks) m_masks->setText(plan.request.patterns.join(';'));
    for (auto it = m_fields.begin(); it != m_fields.end(); ++it) {
        it.value().first->setChecked(plan.request.fields.contains(it.key()));
        it.value().second->setText(plan.request.fields[it.key()].toString());
    }
    m_plan = plan;
    m_options->setEnabled(false);
    // An AI review shows the captured operation, not a second editing form.
    for (auto button : m_options->findChildren<QPushButton *>()) button->hide();
    auto form = qobject_cast<QFormLayout *>(m_options->layout());
    if (m_recursive) form->setRowVisible(m_recursive, plan.request.recursive);
    else if (plan.request.recursive) {
        // CLI/AI plans retain their explicit scope even though the ordinary
        // PDF dialog operates only in its source directory.
        auto scope = new QLabel(tr("Include subdirectories"), m_options);
        scope->setObjectName("toolPreparedRecursive");
        form->addRow(scope);
    }
    for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
        form->setRowVisible(it.value().first, plan.request.fields.contains(it.key()));
    if (m_masks) form->setRowVisible(m_masks, !plan.request.patterns.isEmpty());
    m_files->setMinimumHeight(100);
    m_files->setMaximumHeight(180);
    if (m_preview) m_preview->hide();
    m_apply->setText(tr("Allow selected"));
    m_apply->setAutoDefault(false);
    m_apply->setDefault(false);
    m_cancel->setText(tr("Deny"));
    m_cancel->setObjectName("denyPartsOperation");
    m_cancel->setDefault(true);
    showPlan();
}

void PartsToolsDialog::showPlan()
{
    m_files->clear();
    for (const auto &item : m_plan.items) {
        QString detail = !item.output.isEmpty() ? item.output : !item.keeper.isEmpty() ? tr("Keep: %1").arg(item.keeper) : QString();
        if (!item.fields.isEmpty()) detail = QString::fromUtf8(QJsonDocument(item.fields).toJson(QJsonDocument::Compact));
        auto row = new QTreeWidgetItem(m_files, m_tool == "step-edit"
            ? QStringList{item.path, detail} : QStringList{item.path});
        row->setToolTip(0, detail);
        row->setCheckState(0, Qt::Checked);
    }
    QStringList skippedDetails;
    for (const auto &value : m_plan.skipped) {
        const auto item = value.toObject();
        skippedDetails.append(item["path"].toString() + ": " + item["reason"].toString());
        if (m_tool == "step-edit") {
            auto row = new QTreeWidgetItem(m_files, {item["path"].toString(), item["reason"].toString()});
            row->setDisabled(true);
        }
    }
    m_status->setText(tr("Ready: %1. Skipped: %2.").arg(m_plan.items.size()).arg(m_plan.skipped.size()));
    m_status->setToolTip(skippedDetails.join('\n'));
    if (!m_plan.skipped.isEmpty()) {
        const auto first = m_plan.skipped.first().toObject();
        m_status->setText(m_status->text() + '\n' + QFileInfo(first["path"].toString()).fileName()
            + ": " + first["reason"].toString().simplified().left(240));
    }
    m_apply->setEnabled(!m_plan.items.isEmpty() && (m_tool != "step-edit" || !m_plan.request.fields.isEmpty())
        && (m_tool != "ps2pdf" || !PartsCore::ghostscriptExecutable().isEmpty()));
}
void PartsToolsDialog::start(bool apply)
{
    if (m_worker) return;
    auto result = std::make_shared<PartsCore::ToolPlan>();
    auto applied = std::make_shared<QJsonObject>();
    auto error = std::make_shared<QString>();
    PartsCore::ToolRequest request;
    request.tool = m_tool;
    request.path = m_path ? m_path->text() : m_sourcePath;
    request.recursive = m_recursive && m_recursive->isChecked();
    if (m_output) request.outputDirectory = m_output->text();
    if (m_deleteSources) request.deleteSourcesAfterConversion = m_deleteSources->isChecked();
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
        const auto confirmation = m_tool == "ptc-clean" || m_tool == "zima-clean"
            ? tr("Move %1 selected files to the trash?").arg(selected.items.size())
            : m_tool == "ps2pdf"
                ? (request.deleteSourcesAfterConversion
                    ? tr("Create or replace %1 selected PDFs and delete their PS source files?").arg(selected.items.size())
                    : tr("Create or replace %1 selected PDFs?").arg(selected.items.size()))
                : tr("Apply this operation to %1 selected files?").arg(selected.items.size());
        if (!m_prepared && QMessageBox::question(this, title(m_tool), confirmation,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    }
    QHash<QString, int> rows;
    if (apply && m_tool != "step-edit")
        for (int i = 0; i < m_plan.items.size(); ++i) rows.insert(m_plan.items[i].path, i);
    auto worker = QThread::create([this, request, selected, apply, result, applied, error, rows] {
        try {
            if (apply) {
                *applied = PartsCore::applyTool(selected, [this, rows](const QString &path) {
                    const int row = rows.value(path, -1);
                    if (row < 0) return;
                    QMetaObject::invokeMethod(this, [this, row] {
                        // Keep indices stable for the captured plan until the
                        // final result removes completed items from it.
                        if (auto item = m_files->topLevelItem(row)) item->setHidden(true);
                    }, Qt::QueuedConnection);
                });
            } else *result = PartsCore::planTool(request);
        }
        catch (const QString &e) { *error = e; }
    });
    m_worker = worker;
    if (apply && m_tool != "step-edit") {
        const QFileInfo source(request.path);
        m_fileChangesPath = source.isDir() ? source.absoluteFilePath() : source.absolutePath();
        PartCache::get()->beginFileChanges(m_fileChangesPath);
    }
    m_options->setEnabled(false);
    if (m_preview) m_preview->setEnabled(false);
    m_apply->setEnabled(false); m_files->setEnabled(false);
    m_cancel->setText(tr("Cancel")); m_status->setText(tr("Working..."));
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    const auto previewRevision = m_previewRevision;
    const auto originalPlan = m_plan;
    connect(worker, &QThread::finished, this, [this, result, applied, error, apply, previewRevision, originalPlan, selected] {
        m_worker = nullptr;
        finishFileChanges();
        m_options->setEnabled(!m_prepared);
        if (m_preview) m_preview->setEnabled(true);
        m_files->setEnabled(true); m_cancel->setText(tr("Close"));
        invalidate();
        if (!apply && m_autoPreview && previewRevision != m_previewRevision) {
            m_autoPreview->start(0);
            return;
        }
        if (!error->isEmpty()) {
            if (apply) {
                m_plan = originalPlan;
                showPlan();
                QSet<QString> checked;
                for (const auto &item : selected.items) checked.insert(item.path);
                for (int i = 0; i < m_plan.items.size(); ++i)
                    m_files->topLevelItem(i)->setCheckState(0, checked.contains(m_plan.items[i].path)
                        ? Qt::Checked : Qt::Unchecked);
            }
            m_status->setText(*error);
            if (m_prepared) { m_operationResult = {{"error", *error}}; QDialog::accept(); }
            return;
        }
        if (apply) {
            if (m_tool != "step-edit") {
                QSet<QString> completed, checked;
                QMap<QString, QString> failures;
                for (const auto &value : (*applied)["completed"].toArray())
                    completed.insert(value.toObject()["path"].toString());
                for (const auto &item : selected.items) checked.insert(item.path);
                for (const auto &value : (*applied)["failed"].toArray()) {
                    const auto failure = value.toObject();
                    failures.insert(failure["path"].toString(), failure["reason"].toString());
                }
                m_plan = originalPlan;
                m_plan.items.removeIf([&completed](const auto &item) { return completed.contains(item.path); });
                showPlan();
                for (int i = 0; i < m_plan.items.size(); ++i) {
                    auto row = m_files->topLevelItem(i);
                    const auto &path = m_plan.items[i].path;
                    row->setCheckState(0, checked.contains(path) ? Qt::Checked : Qt::Unchecked);
                    if (failures.contains(path)) row->setToolTip(0, failures.value(path));
                }
            } else {
                for (const auto &kind : {"completed", "failed", "skipped"})
                    for (const auto &value : (*applied)[kind].toArray()) {
                        const auto item = value.toObject();
                        new QTreeWidgetItem(m_files, {item["path"].toString(), QString::fromUtf8(QJsonDocument(item).toJson(QJsonDocument::Compact))});
                    }
            }
            m_status->setText(tr("Completed: %1. Failed: %2. Skipped: %3.")
                .arg((*applied)["completed"].toArray().size()).arg((*applied)["failed"].toArray().size()).arg((*applied)["skipped"].toArray().size()));
            const auto failures = (*applied)["failed"].toArray();
            if (!failures.isEmpty()) {
                const auto first = failures.first().toObject();
                m_status->setText(m_status->text() + '\n' + QFileInfo(first["path"].toString()).fileName()
                    + ": " + first["reason"].toString().simplified().left(240));
            }
            if (!(*applied)["completed"].toArray().isEmpty()) emit filesChanged();
            if (m_prepared) { m_operationResult = *applied; QDialog::accept(); }
        } else {
            m_plan = *result;
            showPlan();
        }
    });
    worker->start();
}
