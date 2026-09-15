#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtTest>
#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTemporaryDir>
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"
#include "localfilters.h"
#include "datasourcewidget.h"
#include "datasourceview.h"
#include "directorywidget.h"
#include "createdirectorydialog.h"
#include "directoryeditordialog.h"
#include "filerenamer.h"
#include "filemover.h"
#include "fileview.h"
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include "mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QSplashScreen>
#include <QTranslator>
#include <QStandardPaths>
#include "browserprofilemanager.h"
#include <QPushButton>
#include <QDialogButtonBox>
#include "applicationlanguage.h"
#include "languageflagswidget.h"
#include "maintoolbar.h"
#include "filtersdialog.h"
#include "fileeditdialog.h"
#include "partcache.h"
#include "prtreader.h"
#include "directoryprotection.h"
#include "partselector.h"
#include "directoryremover.h"
#include "filecopier.h"
#ifdef HAVE_OCCT
#include "extensions/productview/occtimportworker.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#endif

class PartsIntegrationTest : public QObject
{
    Q_OBJECT
    QTemporaryDir settingsDir;
private:
    static void touch(const QString &directory, const QString &name)
    {
        QFile file(QDir(directory).filePath(name));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("fixture");
    }
private slots:
    void initTestCase()
    {
        QCoreApplication::setOrganizationName("ZimaPartsTests");
        QCoreApplication::setApplicationName("PartsIntegration");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    }

    void cliMatchesGuiReadResults()
    {
        const QString cli = qEnvironmentVariable("PARTS_CLI_EXE");
        if (cli.isEmpty())
            QSKIP("Set PARTS_CLI_EXE to run the real CLI parity test");
        QTemporaryDir directory;
        for (const auto &name : {"xxx.pdf", "xxx.prt.9", "xxx.prt.10", "xxx.prtz", "xxx.prtz.1", "hidden.log", ".directory"})
            touch(directory.path(), name);
        QDir().mkpath(directory.filePath("0000-index"));
        {
            QSettings metadata(directory.filePath("0000-index/metadata.ini"), QSettings::IniFormat);
            metadata.setValue("Directory/Version", 2);
            metadata.setValue("Directory/Parameters", QStringList{"description"});
            metadata.setValue("Parameters/description/Label/en", "Description");
            metadata.setValue("Parts/xxx/description/en", "Shared value");
            QSettings filters(directory.filePath("0000-index/filters.ini"), QSettings::IniFormat);
            filters.setValue("Filters/ShowVersions", false);
            filters.setValue("Filters/ShowZimaVersions", false);
            filters.setValue("Filters/Hide", QStringList{"*.log"});
        }
        Settings::get()->LanguageMetadata = "en";
        FileModel model;
        model.setDirectory(directory.path());
        FileFilterModel proxy;
        proxy.setSourceModel(&model);
        QProcess process;
        process.start(cli, {"list", directory.path(), "--language", "en", "--json"});
        QVERIFY(process.waitForFinished(10000));
        QCOMPARE(process.exitCode(), 0);
        const auto rows = QJsonDocument::fromJson(process.readAllStandardOutput()).object().value("parts").toArray();
        QCOMPARE(rows.size(), proxy.rowCount());
        QMap<QString, QString> gui;
        for (int row = 0; row < proxy.rowCount(); ++row)
            gui.insert(proxy.index(row, 0).data().toString(), proxy.index(row, 2).data().toString());
        for (const auto &row : rows) {
            const auto item = row.toObject();
            const auto name = item.value("name").toString();
            QVERIFY(gui.contains(name));
            QCOMPARE(item.value("parameters").toObject().value("description").toString(), gui.value(name));
        }
        QVERIFY(gui.contains("xxx.prt.10"));
        QVERIFY(!gui.contains("xxx.prt.9"));
        QVERIFY(!gui.contains("xxx.prtz.1"));
    }

    void languageSwitching()
    {
        auto settings = Settings::get();
        QCOMPARE(settings->Languages, QStringList({"en_US", "cs_CZ", "de_DE", "fr_FR", "ru_RU"}));
        settings->setCurrentLanguageCode("unsupported");
        QCOMPARE(settings->getCurrentLanguageCode(), QString("en_US"));
        settings->setCurrentLanguageCode("fr_CA");
        QCOMPARE(settings->getCurrentLanguageCode(), QString("fr_FR"));
        settings->setCurrentLanguageCode("ru_BY");
        QCOMPARE(settings->getCurrentLanguageCode(), QString("ru_RU"));
        settings->setCurrentLanguageCode("en_US");
        MainToolBar toolbar;
        toolbar.show();
        auto flags = toolbar.findChild<LanguageFlagsWidget *>();
        QVERIFY(flags);
        const auto buttons = flags->findChildren<QPushButton *>();
        QCOMPARE(buttons.size(), 5);
        QVERIFY(!buttons.last()->icon().pixmap(24, 16).isNull());
        QTemporaryDir directory;
        const QStringList labels = {"Settings", QString::fromUtf8("Nastavení"),
                                    "Einstellungen", QString::fromUtf8("Paramètres"),
                                    QString::fromUtf8("Настройки")};
        for (int i : {4, 3, 1, 2, 0, 4, 0}) {
            buttons.at(i)->click();
            QCoreApplication::processEvents();
            QCOMPARE(settings->getCurrentLanguageCode(), settings->Languages.at(i));
            QCOMPARE(toolbar.findChild<QAction *>("actionSettings")->text(), labels.at(i));
            FiltersDialog dialog(directory.path());
            QVERIFY(!dialog.windowTitle().isEmpty());
            if (i == 3) {
                QCOMPARE(dialog.windowTitle(), QString("Filtres"));
                QDialogButtonBox standardButtons(QDialogButtonBox::Cancel);
                QCOMPARE(standardButtons.button(QDialogButtonBox::Cancel)->text().remove('&'), QString("Annuler"));
            }
            if (i == 4) {
                QCOMPARE(dialog.windowTitle(), QString::fromUtf8("Фильтры"));
                QDialogButtonBox standardButtons(QDialogButtonBox::Cancel);
                QCOMPARE(standardButtons.button(QDialogButtonBox::Cancel)->text().remove('&'),
                         QString::fromUtf8("Отмена"));
                for (int count : {1, 2, 5, 21}) {
                    const auto plural = QCoreApplication::translate("UnusedThumbnailsDialog",
                        "The following %n unused thumbnails were found. Delete them?", "", count);
                    QVERIFY(plural.contains(QString::number(count)));
                    QVERIFY(plural.startsWith(QString::fromUtf8("Найден")));
                }
            }
            QCOMPARE(QSettings().value("Language").toString(), settings->Languages.at(i));
        }
    }

#ifdef HAVE_OCCT
    void cadImportSmoke()
    {
        QTemporaryDir directory;
        const TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();
        const QByteArray step = QFile::encodeName(directory.filePath("fixture.step"));
        STEPControl_Writer stepWriter;
        QCOMPARE(stepWriter.Transfer(box, STEPControl_AsIs), IFSelect_RetDone);
        QCOMPARE(stepWriter.Write(step.constData()), IFSelect_RetDone);
        const QByteArray iges = QFile::encodeName(directory.filePath("fixture.iges"));
        IGESControl_Writer igesWriter;
        QVERIFY(igesWriter.AddShape(box));
        QVERIFY(igesWriter.Write(iges.constData()));
        const QByteArray stl = QFile::encodeName(directory.filePath("fixture.stl"));
        BRepMesh_IncrementalMesh mesh(box, 0.1);
        StlAPI_Writer stlWriter;
        QVERIFY(stlWriter.Write(box, stl.constData()));
        const QList<FileType::FileType> formats = {FileType::STEP, FileType::IGES, FileType::STL};
        const QList<QByteArray> paths = {step, iges, stl};
        for (int i = 0; i < formats.size(); ++i) {
            OcctImportWorker worker(QFile::decodeName(paths.at(i)), formats.at(i), i + 1);
            QSignalSpy imported(&worker, &OcctImportWorker::imported);
            QSignalSpy failed(&worker, &OcctImportWorker::failed);
            worker.run();
            QCOMPARE(failed.size(), 0);
            QCOMPARE(imported.size(), 1);
            const auto result = qvariant_cast<OcctImportResultPtr>(imported.first().at(1));
            QVERIFY(result);
            QVERIFY(!result->bbox.IsVoid());
            QVERIFY(result->vertices.size() >= 36);
            QCOMPARE(result->vertices.size(), result->normals.size());
        }
    }
#endif

    void zimaIconsRender()
    {
        FileIconProvider icons;
        for (const QString &ext : {"prtz", "asmz", "drwz", "frmz", "tblz"}) {
            const auto current = icons.icon(QFileInfo("part." + ext)).pixmap(32, 32);
            const auto archive = icons.icon(QFileInfo("part." + ext + ".10")).pixmap(32, 32);
            QVERIFY(!current.isNull());
            QVERIFY(!archive.isNull());
            QCOMPARE(current.toImage(), archive.toImage());
        }
    }


    void hiddenTabsDeferTheirContent()
    {
        QTemporaryDir directory;
        auto settings = Settings::get();
        const auto previous = settings->DataSources;
        DataSource source("Fixture", directory.path());
        settings->DataSources = {&source};
        {
            DataSourceWidget widget(directory.path());
            QCOMPARE(widget.dsList->count(), 0);
            widget.settingsChanged();
            QCOMPARE(widget.dsList->count(), 0);
            QCOMPARE(widget.currentDir(), directory.path());
            widget.show();
            QTRY_COMPARE(widget.dsList->count(), 1);
        }
        settings->DataSources = previous;
    }

    void deletePreservesOtherTreeBranches()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("work/remove");
        QDir(directory.path()).mkpath("other/keep");
        DataSourceView view(directory.path());
        auto proxy = qobject_cast<QSortFilterProxyModel *>(view.model());
        auto source = qobject_cast<QFileSystemModel *>(proxy->sourceModel());
        view.show();
        auto proxyIndex = [&](const QString &path) {
            return proxy->mapFromSource(source->index(path));
        };
        QTRY_VERIFY(proxyIndex(directory.filePath("other")).isValid());
        const QPersistentModelIndex other(proxyIndex(directory.filePath("other")));
        view.setExpanded(other, true);
        QTRY_VERIFY(proxyIndex(directory.filePath("work/remove")).isValid());
        QVERIFY(view.navigateToDirectory(directory.filePath("work/remove")));
        const QPersistentModelIndex root(view.rootIndex());
        QSignalSpy resets(source, &QAbstractItemModel::modelReset);
        QSignalSpy selected(&view, &DataSourceView::directorySelected);
        QTimer confirm;
        connect(&confirm, &QTimer::timeout, this, [] {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                if (auto box = qobject_cast<QMessageBox *>(widget)) {
                    if (box->standardButtons().testFlag(QMessageBox::Yes))
                        box->button(QMessageBox::Yes)->click();
                }
            }
        });
        confirm.start(10);
        QVERIFY(QMetaObject::invokeMethod(&view, "deleteDirectory", Qt::DirectConnection));
        confirm.stop();
        QVERIFY(!QFileInfo::exists(directory.filePath("work/remove")));
        QCOMPARE(resets.size(), 0);
        QCOMPARE(view.rootIndex(), QModelIndex(root));
        QVERIFY(other.isValid());
        QVERIFY(view.isExpanded(other));
        QCOMPARE(selected.size(), 1);
        QCOMPARE(selected.first().first().toString(), directory.filePath("work"));
    }

    void zimaArchivesAreVisibleUntilExplicitlyHidden()
    {
        QTemporaryDir directory;
        FiltersDialog dialog(directory.path());
        auto showArchives = dialog.findChild<QCheckBox *>("showZimaVersions");
        QVERIFY(showArchives);
        QVERIFY(showArchives->isChecked());
        dialog.accept();
        LocalFilters filters;
        filters.load(directory.path(), true);
        QVERIFY(filters.showZimaVersions);
        const QFileInfoList files{QFileInfo(directory.filePath("part.prtz")),
                                 QFileInfo(directory.filePath("part.prtz.1"))};
        QCOMPARE(filters.accepted(files, true).count(true), 2);
        showArchives->setChecked(false);
        dialog.accept();
        filters.load(directory.path(), true);
        QVERIFY(!filters.showZimaVersions);
        QCOMPARE(filters.accepted(files, true).count(true), 1);
        FiltersDialog reopened(directory.path());
        QVERIFY(!reopened.findChild<QCheckBox *>("showZimaVersions")->isChecked());
    }

    void applyLockToAllSubdirectoriesIsExplicitAndPreservesMetadata()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("a/nested");
        QDir(directory.path()).mkpath("b");
        QDir(directory.path()).mkpath("0000-index/internal");
        const QString a = directory.filePath("a");
        auto meta = MetadataCache::get()->metadata(a);
        meta->setParameterHandles({"description"});
        meta->setPartParam("part", "description", "Keep this value");
        auto result = DirectoryProtection::applyToSubdirectories(directory.path(), true);
        QCOMPARE(result.updated, 3);
        QVERIFY(result.failed.isEmpty());
        QVERIFY(!result.canceled);
        QVERIFY(DirectoryProtection::isLocked(a));
        QVERIFY(DirectoryProtection::isLocked(a + "/nested"));
        QVERIFY(DirectoryProtection::isLocked(directory.filePath("b")));
        QVERIFY(!DirectoryProtection::isLocked(directory.path()));
        QVERIFY(!QFileInfo::exists(directory.filePath("0000-index/internal/0000-index")));
        QVERIFY(meta->removalLocked());
        QCOMPARE(meta->partParam("part", "description"), QString("Keep this value"));
        QDir(directory.path()).mkpath("created-later");
        QVERIFY(!DirectoryProtection::isLocked(directory.filePath("created-later")));
        result = DirectoryProtection::applyToSubdirectories(directory.path(), false);
        QCOMPARE(result.updated, 4);
        QVERIFY(result.failed.isEmpty());
        QVERIFY(!DirectoryProtection::isLocked(a + "/nested"));
        QVERIFY(!meta->removalLocked());
        QCOMPARE(meta->partParam("part", "description"), QString("Keep this value"));
        DirectoryEditorDialog dialog{QFileInfo(directory.path())};
        auto button = dialog.findChild<QPushButton *>("applyLockToSubdirectoriesButton");
        QVERIFY(button);
        QVERIFY(!button->autoDefault());
        auto lock = dialog.findChild<QCheckBox *>("removalLockCheckBox");
        QVERIFY(lock);
        lock->setChecked(true);
        QTimer closeReport;
        closeReport.setInterval(10);
        connect(&closeReport, &QTimer::timeout, [] {
            for (auto widget : QApplication::topLevelWidgets())
                if (auto message = qobject_cast<QMessageBox *>(widget))
                    message->accept();
        });
        closeReport.start();
        button->click();
        closeReport.stop();
        dialog.reject();
        QVERIFY(DirectoryProtection::isLocked(a));
        QVERIFY(DirectoryProtection::isLocked(a + "/nested"));
        QVERIFY(!DirectoryProtection::isLocked(directory.path()));
    }

    void applyingDirectoryLocksReportsFailuresAndCancellation()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("blocked");
        QDir(directory.path()).mkpath("valid");
        touch(directory.filePath("blocked"), "0000-index");
        auto result = DirectoryProtection::applyToSubdirectories(directory.path(), true);
        QCOMPARE(result.updated, 1);
        QCOMPARE(result.failed, QStringList{directory.filePath("blocked")});
        QVERIFY(DirectoryProtection::isLocked(directory.filePath("valid")));
        result = DirectoryProtection::applyToSubdirectories(directory.path(), false,
                                                            [](const QString &) { return false; });
        QVERIFY(result.canceled);
        QCOMPARE(result.updated, 0);
        QVERIFY(DirectoryProtection::isLocked(directory.filePath("valid")));
    }

    void lockDisablesButtonsAndUpdatesAfterUnlock()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("library/child");
        const QString library = directory.filePath("library");
        const QString child = library + "/child";
        touch(library, "locked.pdf");
        touch(child, "free.pdf");
        auto selector = PartSelector::get();
        selector->clear();
        DirectoryWidget widget;
        widget.setDirectory(library);
        auto remove = widget.findChild<QPushButton *>("btnDelete");
        auto move = widget.findChild<QPushButton *>("moveButton");
        auto copy = widget.findChild<QPushButton *>("copyToWorkingDirButton");
        QVERIFY(remove && move && copy);
        QVERIFY(remove->isEnabled());
        auto meta = MetadataCache::get()->metadata(library);
        meta->setRemovalLocked(true);
        QTRY_VERIFY(!remove->isEnabled());
        QVERIFY(!move->isEnabled());
        QVERIFY(copy->isEnabled());
        QVERIFY(!remove->toolTip().isEmpty());
        widget.setDirectory(child);
        QVERIFY(remove->isEnabled());
        selector->select(library, library + "/locked.pdf");
        QTRY_VERIFY(!remove->isEnabled());
        QVERIFY(!move->isEnabled());
        selector->clear();
        QTRY_VERIFY(remove->isEnabled());
        widget.setDirectory(library);
        QVERIFY(!remove->isEnabled());
        meta->setRemovalLocked(false);
        QTRY_VERIFY(remove->isEnabled());
        QVERIFY(move->isEnabled());
        QVERIFY(remove->toolTip().isEmpty());
    }

    void directoryLockIsLocalAndPersists()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("library/child");
        const QString library = directory.filePath("library");
        touch(library, "part.pdf");
        touch(library + "/child", "free.pdf");
        QVERIFY(!DirectoryProtection::isLocked(library));
        {
            DirectoryEditorDialog dialog{QFileInfo(library)};
            auto lock = dialog.findChild<QCheckBox *>("removalLockCheckBox");
            QVERIFY(lock);
            QVERIFY(!lock->isChecked());
            lock->setChecked(true);
            dialog.apply();
        }
        QVERIFY(DirectoryProtection::isLocked(library));
        QCOMPARE(DirectoryProtection::removalLock(QFileInfo(library + "/part.pdf")), library);
        QVERIFY(DirectoryProtection::removalLock(QFileInfo(library + "/child/free.pdf")).isEmpty());
        QVERIFY(DirectoryProtection::removalLock(QFileInfo(library + "/child")).isEmpty());
        QCOMPARE(DirectoryProtection::removalLock(QFileInfo(directory.path())), library);
        MetadataCache::get()->clear(library);
        DirectoryEditorDialog reopened{QFileInfo(library)};
        auto lock = reopened.findChild<QCheckBox *>("removalLockCheckBox");
        QVERIFY(lock->isChecked());
        lock->setChecked(false);
        reopened.apply();
        QVERIFY(!DirectoryProtection::isLocked(library));
    }

    void lockedDeletionPreflightsWholeBatch()
    {
        QTemporaryDir directory;
        QDir(directory.path()).mkpath("library/child");
        const QString library = directory.filePath("library");
        touch(directory.path(), "unlocked.pdf");
        touch(library, "part.pdf");
        touch(library + "/child", "free.pdf");
        MetadataCache::get()->metadata(library)->setRemovalLocked(true);
        DirectoryRemoverWorker worker;
        worker.setStopOnError(true);
        worker.setFileInfos({QFileInfo(directory.filePath("unlocked.pdf")), QFileInfo(library + "/part.pdf")});
        QSignalSpy errors(&worker, &ThreadWorker::errorOccured);
        worker.run();
        QCOMPARE(errors.count(), 1);
        QVERIFY(QFileInfo::exists(directory.filePath("unlocked.pdf")));
        QVERIFY(QFileInfo::exists(library + "/part.pdf"));
        DirectoryRemoverWorker parentRemoval;
        parentRemoval.setStopOnError(true);
        parentRemoval.setFileInfos({QFileInfo(directory.path())});
        QSignalSpy parentErrors(&parentRemoval, &ThreadWorker::errorOccured);
        parentRemoval.run();
        QCOMPARE(parentErrors.count(), 1);
        QVERIFY(DirectoryProtection::isLocked(library));
        DirectoryRemoverWorker childRemoval;
        childRemoval.setStopOnError(true);
        childRemoval.setFileInfos({QFileInfo(library + "/child/free.pdf")});
        childRemoval.run();
        QVERIFY(!QFileInfo::exists(library + "/child/free.pdf"));
        MetadataCache::get()->metadata(library)->setRemovalLocked(false);
        DirectoryRemoverWorker unlocked;
        unlocked.setStopOnError(true);
        unlocked.setFileInfos({QFileInfo(library + "/part.pdf")});
        unlocked.run();
        QVERIFY(!QFileInfo::exists(library + "/part.pdf"));
    }

    void lockBlocksMovesAndReplacementButAllowsCopyOut()
    {
        QTemporaryDir directory;
        for (const auto &name : {"library", "destination", "source"})
            QDir(directory.path()).mkpath(name);
        const QString library = directory.filePath("library");
        const QString destination = directory.filePath("destination");
        touch(library, "part.pdf");
        MetadataCache::get()->metadata(library)->setRemovalLocked(true);
        FileMoverWorker move;
        move.setSourceFiles({qMakePair(QFileInfo(library + "/part.pdf"), QString())}, destination);
        QSignalSpy errors(&move, &ThreadWorker::errorOccured);
        move.run();
        QCOMPARE(errors.count(), 1);
        QVERIFY(QFileInfo::exists(library + "/part.pdf"));
        QVERIFY(!QFileInfo::exists(destination + "/part.pdf"));
        FileMoverWorker moveFolder;
        moveFolder.setSourceFiles({qMakePair(QFileInfo(library), QString())}, destination);
        QSignalSpy folderErrors(&moveFolder, &ThreadWorker::errorOccured);
        moveFolder.run();
        QCOMPARE(folderErrors.count(), 1);
        FileCopierWorker copy;
        copy.setStopOnError(true);
        copy.setSourceFiles({FileCopyIntent(QFileInfo(library + "/part.pdf"))}, destination);
        copy.run();
        QVERIFY(QFileInfo::exists(destination + "/part.pdf"));
        touch(directory.filePath("source"), "part.pdf");
        FileMoverWorker replace;
        replace.setSourceFiles({qMakePair(QFileInfo(directory.filePath("source/part.pdf")), QString())}, library);
        QSignalSpy replaceErrors(&replace, &ThreadWorker::errorOccured);
        replace.run();
        QCOMPARE(replaceErrors.count(), 1);
        QVERIFY(QFileInfo::exists(directory.filePath("source/part.pdf")));
        FileCopierWorker overwrite;
        overwrite.setStopOnError(true);
        overwrite.setSourceFiles({FileCopyIntent(QFileInfo(directory.filePath("source/part.pdf")))}, library);
        QSignalSpy overwriteErrors(&overwrite, &ThreadWorker::errorOccured);
        overwrite.run();
        QCOMPARE(overwriteErrors.count(), 1);
        QVERIFY(QFileInfo::exists(library + "/part.pdf"));
    }

    void sharedPartParametersSurviveRefresh()
    {
        QTemporaryDir directory;
        for (const auto &name : {"xxx.pdf", "xxx.prt.1", "xxx.prt.10", "xxx.prtz",
                                 "xxx.prtz.2", "xxx.revA.pdf", "xxx.revA.prt.1"})
            touch(directory.path(), name);
        auto meta = MetadataCache::get()->metadata(directory.path());
        meta->setParameterHandles({"description"});
        meta->setParameterLabel("description", Settings::get()->LanguageMetadata, "Description");
        FileModel model;
        model.setDirectory(directory.path());
        auto cell = [&](const QString &name) {
            for (int row = 0; row < model.rowCount(); ++row)
                if (model.fileInfo(model.index(row, 0)).fileName() == name)
                    return model.index(row, 2);
            return QModelIndex();
        };
        QVERIFY(cell("xxx.prt.1").isValid());
        QSignalSpy changes(&model, &QAbstractItemModel::dataChanged);
        QVERIFY(model.setData(cell("xxx.prt.1"), "Shared value"));
        QCOMPARE(changes.count(), 5);
        for (const auto &name : {"xxx.pdf", "xxx.prt.1", "xxx.prt.10", "xxx.prtz", "xxx.prtz.2"})
            QCOMPARE(model.data(cell(name), Qt::DisplayRole).toString(), QString("Shared value"));
        QVERIFY(model.setData(cell("xxx.revA.pdf"), "Dotted name"));
        QCOMPARE(model.data(cell("xxx.revA.prt.1"), Qt::DisplayRole).toString(), QString("Dotted name"));
        {
            FileEditDialog dialog(QFileInfo(directory.filePath("xxx.pdf")), &model);
            const auto edits = dialog.findChildren<QLineEdit *>();
            QCOMPARE(edits.size(), 1);
            QCOMPARE(edits.first()->text(), QString("Shared value"));
            edits.first()->setText("Dialog value");
            dialog.save();
        }
        model.reloadParts();
        MetadataCache::get()->clear();
        model.refreshModel();
        QCOMPARE(model.data(cell("xxx.prt.10"), Qt::DisplayRole).toString(), QString("Dialog value"));
        QCOMPARE(model.data(cell("xxx.revA.pdf"), Qt::DisplayRole).toString(), QString("Dotted name"));
    }

    void legacyFileParametersSurviveRefresh()
    {
        QTemporaryDir directory;
        touch(directory.path(), "xxx.pdf");
        touch(directory.path(), "xxx.prt.1");
        auto meta = MetadataCache::get()->metadata(directory.path());
        meta->setParameterHandles({"description"});
        // Reproduce records written by the broken inline editor.
        meta->setPartParam("xxx.prt.1", "description", "Existing value");
        PartCache::get()->parts(directory.path());
        PartCache::get()->refresh(directory.path());
        MetadataCache::get()->clear(directory.path());
        meta = MetadataCache::get()->metadata(directory.path());
        QCOMPARE(meta->partParam("xxx", "description"), QString("Existing value"));
        meta->setPartParam("xxx", "description", "");
        QCOMPARE(meta->partParam("xxx", "description"), QString());
        MetadataCache::get()->clear(directory.path());
        meta = MetadataCache::get()->metadata(directory.path());
        QCOMPARE(meta->partParam("xxx", "description"), QString());
        QCOMPARE(meta->partParam("xxx.prt.1", "description"), QString("Existing value"));
    }

    void proeParametersComeFromHighestNumericRevision()
    {
        QTemporaryDir directory;
        QFileInfoList files;
        for (const auto &entry : QList<QPair<QString, QString>>{
                {"xxx.prt.10", "Newest"}, {"xxx.prt.9", "Older"}, {"xxx.prt.1", "Oldest"}}) {
            QFile file(directory.filePath(entry.first));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(QByteArray("description\x15") + "DESCRIPTION'xx"
                       + entry.second.toUtf8() + "x\x14\n");
            file.close();
            files.append(QFileInfo(file.fileName()));
        }
        touch(directory.path(), "xxx.pdf");
        PtrReaderThread worker(files);
        QSignalSpy values(&worker, &PtrReaderThread::partParam);
        worker.start();
        QVERIFY(worker.wait(5000));
        QTRY_COMPARE(values.count(), 1);
        QCOMPARE(values.first().at(0).toString(), QString("xxx.prt.10"));
        QCOMPARE(values.first().at(2).toString(), QString("Newest"));
        auto meta = MetadataCache::get()->metadata(directory.path());
        meta->setParameterHandles({"description"});
        PrtReader reader;
        QSignalSpy loaded(&reader, &PrtReader::loaded);
        reader.load(directory.path(), files);
        QTRY_COMPARE(loaded.count(), 1);
        QCOMPARE(meta->partParam("xxx", "description"), QString("Newest"));
    }

    void modelRejectsStaleColumnsAfterMetadataChange()
    {
        QTemporaryDir directory;
        touch(directory.path(), "fixture.txt");
        auto metadata = MetadataCache::get()->metadata(directory.path());
        metadata->setParameterHandles({"description"});
        metadata->setParameterLabel("description", Settings::get()->LanguageMetadata, "Description");
        FileModel model;
        model.setDirectory(directory.path());
        const QModelIndex stale = model.index(0, 2);
        QVERIFY(stale.isValid());
        metadata->setParameterHandles({});
        model.refreshModel();
        QVERIFY(!model.headerData(-1, Qt::Horizontal, Qt::DisplayRole).isValid());
        QVERIFY(!model.headerData(2, Qt::Horizontal, Qt::DisplayRole).isValid());
        QVERIFY(!model.data(stale, Qt::DisplayRole).isValid());
    }
    void prototypeDirectoryCanBeRenamedAndDeletedWhileDisplayed()
    {
        QTemporaryDir directory;
        const QString prototypeName = "project-prototype";
        const QString prototype = directory.filePath("0000-index/prototypes/" + prototypeName);
        QVERIFY(QDir().mkpath(prototype + "/0000-index"));
        QVERIFY(QDir().mkpath(prototype + "/child/0000-index"));
        {
            QSettings metadata(prototype + "/0000-index/metadata.ini", QSettings::IniFormat);
            metadata.setValue("Directory/Version", 2);
            metadata.setValue("Directory/Label/en", "Project label from metadata");
            metadata.setValue("Directory/Label/cs", "Popis projektu");
            metadata.setValue("Directory/AutoIndex", true);
        }
        touch(prototype + "/child", "fixture.txt");
        DataSourceView view(directory.path());
        DirectoryWidget pane;
        DirectoryWidget secondPane;
        view.show();
        pane.show();
        QTRY_VERIFY(view.navigateToDirectory(directory.path()));
        const QString created = directory.filePath("created-project");
        const QString renamed = directory.filePath("renamed-project");
        QStringList warnings;
        QTimer responder;
        bool createdDialogAnswered = false;
        bool editedDialogAnswered = false;
        connect(&responder, &QTimer::timeout, this, [&] {
            for (QWidget *widget : QApplication::topLevelWidgets())
            {
                if (auto box = qobject_cast<QMessageBox *>(widget); box && box->isVisible())
                {
                    if (box->standardButtons().testFlag(QMessageBox::Yes)
                            && box->text().contains(renamed))
                        box->button(QMessageBox::Yes)->click();
                    else
                    {
                        warnings << box->text();
                        box->accept();
                    }
                }
                else if (auto dialog = qobject_cast<CreateDirectoryDialog *>(widget);
                         dialog && dialog->isVisible() && !createdDialogAnswered)
                {
                    createdDialogAnswered = true;
                    dialog->findChild<QLineEdit *>("nameLineEdit")->setText("created-project");
                    auto combo = dialog->findChild<QComboBox *>("prototypeComboBox");
                    combo->setCurrentIndex(combo->findData(prototypeName));
                    dialog->findChild<QDialogButtonBox *>("buttonBox")
                            ->button(QDialogButtonBox::Ok)->click();
                }
                else if (auto dialog = qobject_cast<DirectoryEditorDialog *>(widget);
                         dialog && dialog->isVisible() && !editedDialogAnswered)
                {
                    editedDialogAnswered = true;
                    dialog->findChild<QLineEdit *>("nameLineEdit")->setText("renamed-project");
                    dialog->accept();
                }
            }
        });
        responder.start(10);
        QVERIFY(QMetaObject::invokeMethod(&view, "createDirectory", Qt::DirectConnection));
        QVERIFY(QFileInfo::exists(created + "/child/fixture.txt"));
        QTRY_VERIFY(view.navigateToDirectory(created));
        view.expand(view.currentIndex());
        pane.setDirectory(created);
        secondPane.setDirectory(created);
        auto firstFiles = pane.findChild<FileView *>();
        auto secondFiles = secondPane.findChild<FileView *>();
        QVERIFY(firstFiles);
        QVERIFY(secondFiles);
        auto proxy = qobject_cast<QSortFilterProxyModel *>(view.model());
        QSignalSpy resets(proxy->sourceModel(), &QAbstractItemModel::modelReset);

        // A failed operation must restore watchers too, without changing either directory.
        QVERIFY(QDir(directory.path()).mkdir("occupied"));
        FileRenamer renamer;
        QVERIFY(!renamer.rename(directory.path(), QFileInfo(created), "occupied"));
        QCOMPARE(firstFiles->currentPath(), created);
        QCOMPARE(secondFiles->currentPath(), created);

        QVERIFY(QMetaObject::invokeMethod(&view, "editDirectory", Qt::DirectConnection));
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join("; ")));
        QVERIFY(!QFileInfo::exists(created));
        QVERIFY(QFileInfo::exists(renamed + "/child/fixture.txt"));
        QCOMPARE(firstFiles->currentPath(), renamed);
        QCOMPARE(secondFiles->currentPath(), renamed);
        QCOMPARE(MetadataCache::get()->metadata(renamed)->getLabel("en"),
                 QString("Project label from metadata"));
        QCOMPARE(MetadataCache::get()->metadata(renamed)->getLabel("cs"), QString("Popis projektu"));
        QTRY_VERIFY(view.navigateToDirectory(renamed));
        QVERIFY(QMetaObject::invokeMethod(&view, "deleteDirectory", Qt::DirectConnection));
        responder.stop();
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join("; ")));
        QVERIFY(!QFileInfo::exists(renamed));
        QCOMPARE(firstFiles->currentPath(), directory.path());
        QCOMPARE(secondFiles->currentPath(), directory.path());
        QCOMPARE(resets.size(), 0);
        QCoreApplication::processEvents();
        QVERIFY(!QFileInfo::exists(created));
        QVERIFY(!QFileInfo::exists(renamed));
        QVERIFY(QFileInfo::exists(prototype + "/child/fixture.txt"));
    }
    void moveDisplayedDirectoryAndBack()
    {
        QTemporaryDir directory;
        const QString source = directory.filePath("source/project");
        const QString destination = directory.filePath("destination");
        const QString moved = destination + "/project";
        QVERIFY(QDir().mkpath(source + "/0000-index"));
        QVERIFY(QDir().mkpath(source + "/child"));
        QVERIFY(QDir().mkpath(destination));
        {
            QSettings metadata(source + "/0000-index/metadata.ini", QSettings::IniFormat);
            metadata.setValue("Directory/Version", 2);
            metadata.setValue("Directory/AutoIndex", true);
            metadata.setValue("Directory/Label/en", "Project metadata");
        }
        touch(source + "/child", "fixture.txt");
        DirectoryWidget pane;
        DirectoryWidget secondPane;
        pane.setDirectory(source);
        secondPane.setDirectory(source);
        pane.show();
        auto firstFiles = pane.findChild<FileView *>();
        auto secondFiles = secondPane.findChild<FileView *>();
        QVERIFY(firstFiles);
        QVERIFY(secondFiles);
        QStringList warnings;
        QTimer responder;
        connect(&responder, &QTimer::timeout, this, [&] {
            for (QWidget *widget : QApplication::topLevelWidgets())
            {
                if (auto box = qobject_cast<QMessageBox *>(widget); box && box->isVisible())
                {
                    warnings << box->text();
                    box->accept();
                }
            }
        });
        responder.start(10);
        FileMover mover;
        mover.addSourceFile(QFileInfo(source));
        mover.setDestination(destination);
        mover.work();
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join("; ")));
        QVERIFY(!QFileInfo::exists(source));
        QVERIFY(QFileInfo::exists(moved + "/child/fixture.txt"));
        QCOMPARE(firstFiles->currentPath(), moved);
        QCOMPARE(secondFiles->currentPath(), moved);
        QCOMPARE(MetadataCache::get()->metadata(moved)->getLabel("en"), QString("Project metadata"));

        FileMover back;
        back.addSourceFile(QFileInfo(moved));
        back.setDestination(directory.filePath("source"));
        back.work();
        responder.stop();
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join("; ")));
        QVERIFY(!QFileInfo::exists(moved));
        QVERIFY(QFileInfo::exists(source + "/child/fixture.txt"));
        QCOMPARE(firstFiles->currentPath(), source);
        QCOMPARE(secondFiles->currentPath(), source);
    }
    void splashDoesNotSleepInWindowConstructor()
    {
        QTemporaryDir directory;
        auto settings = Settings::get();
        const auto oldTabs = settings->MainTabs;
        const auto oldDuration = settings->GUISplashDuration;
        const bool oldEnabled = settings->GUISplashEnabled;
        settings->MainTabs = {directory.path()};
        settings->ActiveMainTab = 0;
        settings->GUISplashEnabled = true;
        settings->GUISplashDuration = 5000;
        QTranslator translator;
        QElapsedTimer timer;
        timer.start();
        {
            MainWindow window(&translator);
            QVERIFY2(timer.elapsed() < 4000, "Splash duration blocked main-window construction");
            bool eventDelivered = false;
            QTimer::singleShot(0, &window, [&] { eventDelivered = true; });
            QTRY_VERIFY(eventDelivered);
        }
        for (QWidget *widget : QApplication::topLevelWidgets())
            if (auto splash = qobject_cast<QSplashScreen *>(widget))
                splash->close();
        settings->MainTabs = oldTabs;
        settings->GUISplashDuration = oldDuration;
        settings->GUISplashEnabled = oldEnabled;
    }

    void modelFilteringAndProgressiveUpdates()
    {
        QTemporaryDir directory, other;
        for (const QString &name : {"part.prt.2", "part.prt.10", "notes.unknown", ".hidden", "junk.bak"})
            touch(directory.path(), name);
        QImage picture(400, 200, QImage::Format_RGB32);
        picture.fill(Qt::green);
        QVERIFY(picture.save(directory.filePath("preview.png")));
        QDir(directory.path()).mkpath("0000-index");
        {
            QSettings filters(LocalFilters::filePath(directory.path()), QSettings::IniFormat);
            filters.setValue("Filters/Hide", QStringList{"*.bak"});
            filters.setValue("Filters/ShowVersions", false);
        }
        FileModel model;
        FileFilterModel proxy;
        proxy.setSourceModel(&model);
        QAbstractItemModelTester modelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QAbstractItemModelTester proxyTester(&proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
        model.setDirectory(directory.path());
        QCOMPARE(proxy.rowCount(), 4);
        QSignalSpy resets(&model, &QAbstractItemModel::modelReset);
        QSignalSpy changes(&model, &QAbstractItemModel::dataChanged);
        for (int row = 0; row < model.rowCount(); ++row) {
            if (model.data(model.index(row, 0), Qt::DisplayRole).toString() == "preview.png")
                model.data(model.index(row, 1), Qt::DecorationRole);
        }
        QTRY_VERIFY(changes.size() > 0);
        QCOMPARE(resets.size(), 0);
        QCOMPARE(proxy.rowCount(), 4);
        proxy.filterColumn(0, ".PRT.10");
        QTRY_COMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.data(proxy.index(0, 0)).toString(), QString("part.prt.10"));
        touch(other.path(), "new.type");
        model.setDirectory(other.path());
        proxy.resetFilters();
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.data(proxy.index(0, 0)).toString(), QString("new.type"));
    }
};
int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QStandardPaths::setTestModeEnabled(true);
    QApplication app(argc, argv);
    PartsIntegrationTest test;
    const int result = QTest::qExec(&test, argc, argv);
    BrowserProfileManager::shutdown();
    return result;
}
#include "tst_partsintegration.moc"
