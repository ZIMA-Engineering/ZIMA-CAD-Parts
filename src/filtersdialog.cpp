#include "filtersdialog.h"
#include "ui_filtersdialog.h"

#include "file.h"
#include "settings.h"
#include "filefilters/filefilter.h"

#include <QMessageBox>
#include <QDir>
#include <QFont>
#include <QRegularExpression>
#include <QSet>
#include <QTreeWidgetItem>

#include <algorithm>
#include <utility>

FiltersDialog::FiltersDialog(const QString &directory, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FiltersDialog),
      m_directory(directory),
      m_config(FileFilterConfig::load(directory)),
      m_updating(false)
{
    ui->setupUi(this);
    ui->scopeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->inheritCheckBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainLayout->setStretch(0, 0);
    ui->mainLayout->setStretch(1, 0);
    ui->mainLayout->setStretch(2, 1);
    ui->mainLayout->setStretch(3, 0);
    ui->mainLayout->setStretch(4, 0);
    ui->splitter->setSizes({430, 370});
    ui->scopeLabel->setText(tr("Rules for: %1").arg(QDir::toNativeSeparators(directory)));
    ui->versionModeComboBox->addItem(tr("No version handling"), int(FileVersionMode::None));
    ui->versionModeComboBox->addItem(tr("Pro/E numbered versions"), int(FileVersionMode::ProE));
    ui->versionModeComboBox->addItem(tr("ZIMA-CAD current file and history"), int(FileVersionMode::ZimaCad));

    if (!m_config.configured)
        setupDefaults();
    ui->inheritCheckBox->setChecked(m_config.inherit);
    rebuildTree();

    connect(ui->treeWidget, &QTreeWidget::currentItemChanged,
            this, &FiltersDialog::currentItemChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged,
            this, &FiltersDialog::treeItemChanged);
    connect(ui->nameLineEdit, &QLineEdit::textEdited, this, &FiltersDialog::updateCurrentItem);
    connect(ui->patternLineEdit, &QLineEdit::textEdited, this, &FiltersDialog::updateCurrentItem);
    connect(ui->versionModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FiltersDialog::updateCurrentItem);
    connect(ui->showVersionsCheckBox, &QCheckBox::toggled,
            this, &FiltersDialog::updateCurrentItem);
    connect(ui->addGroupButton, &QPushButton::clicked, this, &FiltersDialog::addGroup);
    connect(ui->addFormatButton, &QPushButton::clicked, this, &FiltersDialog::addFormat);
    connect(ui->deleteButton, &QPushButton::clicked, this, &FiltersDialog::deleteCurrent);
    connect(ui->moveUpButton, &QPushButton::clicked, this, &FiltersDialog::moveCurrentUp);
    connect(ui->moveDownButton, &QPushButton::clicked, this, &FiltersDialog::moveCurrentDown);
}

FiltersDialog::~FiltersDialog()
{
    delete ui;
}

void FiltersDialog::setupDefaults()
{
    m_config.inherit = true;
    for (const FilterGroup &legacyGroup : Settings::get()->FilterGroups)
    {
        FileFilterGroupDefinition group;
        group.id = uniqueGroupId(legacyGroup.internalName.toLower());
        group.title = legacyGroup.label;
        group.enabled = legacyGroup.enabled;
        for (FileFilter *filter : legacyGroup.filters)
        {
            if (filter->filterType() == FileFilter::Extension)
            {
                const QStringList patterns = File::getPatternsForFileType(filter->type);
                for (const QString &pattern : patterns)
                {
                    if (pattern.startsWith("*."))
                        group.formats << FileFilterDefinition{pattern, filter->enabled};
                }
            }
            else if (filter->filterType() == FileFilter::Version)
            {
                group.versionMode = FileVersionMode::ProE;
                group.showVersions = filter->enabled;
            }
        }
        m_config.groups << group;
    }

    FileFilterGroupDefinition zima;
    zima.id = "zima-cad";
    zima.title = tr("ZIMA-CAD files");
    zima.versionMode = FileVersionMode::ZimaCad;
    zima.formats << FileFilterDefinition{"*.prtz", true}
                 << FileFilterDefinition{"*.asmz", true}
                 << FileFilterDefinition{"*.drwz", true};
    m_config.groups << zima;
}

QString FiltersDialog::uniqueGroupId(const QString &base) const
{
    QString id = base.toLower();
    id.replace(QRegularExpression("[^a-z0-9_-]+"), "-");
    id.remove(QRegularExpression("^-+|-+$"));
    if (id.isEmpty())
        id = "group";
    QString candidate = id;
    int number = 2;
    while (std::any_of(m_config.groups.cbegin(), m_config.groups.cend(),
                       [&](const FileFilterGroupDefinition &group) { return group.id == candidate; }))
        candidate = id + '-' + QString::number(number++);
    return candidate;
}

void FiltersDialog::rebuildTree(int selectedGroup, int selectedFormat)
{
    m_updating = true;
    ui->treeWidget->clear();
    QTreeWidgetItem *selection = nullptr;
    for (int groupIndex = 0; groupIndex < m_config.groups.size(); ++groupIndex)
    {
        const FileFilterGroupDefinition &group = m_config.groups.at(groupIndex);
        QTreeWidgetItem *groupItem = new QTreeWidgetItem(ui->treeWidget);
        groupItem->setText(0, group.title);
        groupItem->setData(0, GroupIndexRole, groupIndex);
        groupItem->setData(0, FormatIndexRole, -1);
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsUserCheckable);
        groupItem->setCheckState(0, group.enabled ? Qt::Checked : Qt::Unchecked);
        QFont font = groupItem->font(0);
        font.setBold(true);
        groupItem->setFont(0, font);

        for (int formatIndex = 0; formatIndex < group.formats.size(); ++formatIndex)
        {
            const FileFilterDefinition &format = group.formats.at(formatIndex);
            QTreeWidgetItem *formatItem = new QTreeWidgetItem(groupItem);
            formatItem->setText(0, format.pattern);
            formatItem->setData(0, GroupIndexRole, groupIndex);
            formatItem->setData(0, FormatIndexRole, formatIndex);
            formatItem->setFlags(formatItem->flags() | Qt::ItemIsUserCheckable);
            formatItem->setCheckState(0, format.enabled ? Qt::Checked : Qt::Unchecked);
            if (groupIndex == selectedGroup && formatIndex == selectedFormat)
                selection = formatItem;
        }
        if (groupIndex == selectedGroup && selectedFormat < 0)
            selection = groupItem;
    }
    ui->treeWidget->expandAll();
    ui->treeWidget->setCurrentItem(selection ? selection : ui->treeWidget->topLevelItem(0));
    m_updating = false;
    currentItemChanged(ui->treeWidget->currentItem(), nullptr);
}

void FiltersDialog::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    if (m_updating || !current)
        return;
    const int groupIndex = current->data(0, GroupIndexRole).toInt();
    const int formatIndex = current->data(0, FormatIndexRole).toInt();
    if (groupIndex < 0 || groupIndex >= m_config.groups.size())
        return;

    m_updating = true;
    const FileFilterGroupDefinition &group = m_config.groups.at(groupIndex);
    const bool isFormat = formatIndex >= 0;
    ui->nameLabel->setVisible(!isFormat);
    ui->nameLineEdit->setVisible(!isFormat);
    ui->nameLineEdit->setText(group.title);
    ui->patternLabel->setVisible(isFormat);
    ui->patternLineEdit->setVisible(isFormat);
    ui->patternLineEdit->setText(isFormat ? group.formats.at(formatIndex).pattern : QString());
    ui->versionModeLabel->setVisible(!isFormat);
    ui->versionModeComboBox->setVisible(!isFormat);
    ui->showVersionsCheckBox->setVisible(!isFormat && group.versionMode != FileVersionMode::None);
    ui->versionModeComboBox->setCurrentIndex(
                ui->versionModeComboBox->findData(int(group.versionMode)));
    ui->showVersionsCheckBox->setChecked(group.showVersions);
    ui->addFormatButton->setEnabled(true);
    m_updating = false;
}

void FiltersDialog::treeItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_updating || !item || column != 0
            || item->data(0, FormatIndexRole).toInt() >= 0)
        return;

    const Qt::CheckState state = item->checkState(0);
    if (state == Qt::PartiallyChecked)
        return;

    m_updating = true;
    for (int childIndex = 0; childIndex < item->childCount(); ++childIndex)
        item->child(childIndex)->setCheckState(0, state);
    m_updating = false;
}

void FiltersDialog::updateCurrentItem()
{
    if (m_updating || !ui->treeWidget->currentItem())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    const int groupIndex = item->data(0, GroupIndexRole).toInt();
    const int formatIndex = item->data(0, FormatIndexRole).toInt();
    FileFilterGroupDefinition &group = m_config.groups[groupIndex];
    if (formatIndex >= 0)
    {
        group.formats[formatIndex].pattern = ui->patternLineEdit->text().trimmed();
        item->setText(0, group.formats.at(formatIndex).pattern);
    }
    else
    {
        group.title = ui->nameLineEdit->text().trimmed();
        group.versionMode = FileVersionMode(ui->versionModeComboBox->currentData().toInt());
        group.showVersions = ui->showVersionsCheckBox->isChecked();
        item->setText(0, group.title);
        ui->showVersionsCheckBox->setVisible(group.versionMode != FileVersionMode::None);
    }
}

void FiltersDialog::commitEditor()
{
    updateCurrentItem();
    for (int groupIndex = 0; groupIndex < ui->treeWidget->topLevelItemCount(); ++groupIndex)
    {
        QTreeWidgetItem *groupItem = ui->treeWidget->topLevelItem(groupIndex);
        FileFilterGroupDefinition &group = m_config.groups[groupIndex];
        group.enabled = groupItem->checkState(0) == Qt::Checked;
        for (int formatIndex = 0; formatIndex < groupItem->childCount(); ++formatIndex)
            group.formats[formatIndex].enabled = groupItem->child(formatIndex)->checkState(0) == Qt::Checked;
    }
}

void FiltersDialog::addGroup()
{
    commitEditor();
    FileFilterGroupDefinition group;
    group.id = uniqueGroupId("new-group");
    group.title = tr("New group");
    m_config.groups << group;
    rebuildTree(m_config.groups.size() - 1);
}

void FiltersDialog::addFormat()
{
    if (!ui->treeWidget->currentItem())
        return;
    commitEditor();
    const int groupIndex = ui->treeWidget->currentItem()->data(0, GroupIndexRole).toInt();
    m_config.groups[groupIndex].formats << FileFilterDefinition{"*.*", true};
    rebuildTree(groupIndex, m_config.groups[groupIndex].formats.size() - 1);
}

void FiltersDialog::deleteCurrent()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item)
        return;
    commitEditor();
    const int groupIndex = item->data(0, GroupIndexRole).toInt();
    const int formatIndex = item->data(0, FormatIndexRole).toInt();
    if (formatIndex >= 0)
        m_config.groups[groupIndex].formats.removeAt(formatIndex);
    else
        m_config.groups.removeAt(groupIndex);
    rebuildTree(std::min(groupIndex, int(m_config.groups.size()) - 1));
}

void FiltersDialog::moveCurrentUp()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item)
        return;
    commitEditor();
    int groupIndex = item->data(0, GroupIndexRole).toInt();
    int formatIndex = item->data(0, FormatIndexRole).toInt();
    if (formatIndex > 0)
    {
        m_config.groups[groupIndex].formats.swapItemsAt(formatIndex, formatIndex - 1);
        rebuildTree(groupIndex, formatIndex - 1);
    }
    else if (formatIndex < 0 && groupIndex > 0)
    {
        m_config.groups.swapItemsAt(groupIndex, groupIndex - 1);
        rebuildTree(groupIndex - 1);
    }
}

void FiltersDialog::moveCurrentDown()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item)
        return;
    commitEditor();
    int groupIndex = item->data(0, GroupIndexRole).toInt();
    int formatIndex = item->data(0, FormatIndexRole).toInt();
    if (formatIndex >= 0 && formatIndex + 1 < m_config.groups[groupIndex].formats.size())
    {
        m_config.groups[groupIndex].formats.swapItemsAt(formatIndex, formatIndex + 1);
        rebuildTree(groupIndex, formatIndex + 1);
    }
    else if (formatIndex < 0 && groupIndex + 1 < m_config.groups.size())
    {
        m_config.groups.swapItemsAt(groupIndex, groupIndex + 1);
        rebuildTree(groupIndex + 1);
    }
}

void FiltersDialog::accept()
{
    commitEditor();
    QSet<QString> groupIds;
    for (const FileFilterGroupDefinition &group : std::as_const(m_config.groups))
    {
        if (group.title.trimmed().isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid filters"), tr("Every group must have a name."));
            return;
        }
        if (groupIds.contains(group.id.toLower()))
        {
            QMessageBox::warning(this, tr("Invalid filters"), tr("Group identifiers must be unique."));
            return;
        }
        groupIds.insert(group.id.toLower());

        QSet<QString> patterns;
        for (const FileFilterDefinition &format : group.formats)
        {
            const QString pattern = format.pattern.trimmed();
            if (pattern.isEmpty())
            {
                QMessageBox::warning(this, tr("Invalid filters"), tr("A file pattern cannot be empty."));
                return;
            }
            if (patterns.contains(pattern.toLower()))
            {
                QMessageBox::warning(this, tr("Invalid filters"),
                                     tr("The pattern %1 occurs more than once in a group.").arg(pattern));
                return;
            }
            patterns.insert(pattern.toLower());
        }
    }
    m_config.inherit = ui->inheritCheckBox->isChecked();
    QString error;
    if (!m_config.save(m_directory, &error))
    {
        QMessageBox::warning(this, tr("Unable to save filters"), error);
        return;
    }
    QDialog::accept();
}
