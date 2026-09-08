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
