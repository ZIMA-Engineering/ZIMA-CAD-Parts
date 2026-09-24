#include "commandpanel.h"
#include "interactionstyle.h"
#include <QStyleFactory>
#include <QVBoxLayout>
#include "ai/aitools.h"
#include "ai/codexprovider.h"
#include "ai/systemcommanddialog.h"
#include "ai-fixture.h"
#include "updatespage.h"
#include "updateservice.h"
#include "settingsdialog.h"
#include <QTabWidget>
#include <QToolButton>
#include "core/partstools.h"
#include "partstoolsdialog.h"
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QDockWidget>
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
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QScopedValueRollback>
#include "maintabwidget.h"
#include <QComboBox>
#include <QCheckBox>
#include "mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
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
    void interactionColoursKeepNativeTreeMetrics()
    {
        for (const auto &name : QStyleFactory::keys())
        {
            PartsInteraction::Style style(QStyleFactory::create(name));
            const auto nativeIndent = style.baseStyle()->pixelMetric(QStyle::PM_TreeViewIndentation);
            QCOMPARE(style.pixelMetric(QStyle::PM_TreeViewIndentation), nativeIndent);
            for (const auto background : {QColor("#ffffff"), QColor("#202020")})
            {
                for (int state = 0; state < 3; ++state)
                {
                    QImage image(160, 30, QImage::Format_RGB32);image.fill(background);
                    QPainter painter(&image);
                    QStyleOptionViewItem option;option.rect = image.rect();
                    option.state = QStyle::State_Enabled;
                    if (state != 0) option.state |= QStyle::State_Selected;
                    if (state != 1) option.state |= QStyle::State_MouseOver;
                    style.drawControl(QStyle::CE_ItemViewItem, &option, &painter);
                    painter.end();
                    QCOMPARE(image.pixelColor(80, 15), state == 0
                        ? PartsInteraction::hover() : PartsInteraction::selection());
                }
            }
        }
        QWidget window;QVBoxLayout layout(&window);
        QTreeWidget tree;tree.setHeaderLabel("Parts");layout.addWidget(&tree);
        auto *root = new QTreeWidgetItem(&tree, {"Source"});
        auto *selected = new QTreeWidgetItem(root, {"Selected part"});
        auto *offered = new QTreeWidgetItem(root, {"Another part"});root->setExpanded(true);
        tree.setCurrentItem(selected);
        QTabWidget tabs;tabs.addTab(new QWidget, "Parts");tabs.addTab(new QWidget, "Preview");
        QFile css(":/gfx/navigation/tabs.css");QVERIFY(css.open(QIODevice::ReadOnly));
        tabs.tabBar()->setStyleSheet(QString::fromUtf8(css.readAll()));layout.addWidget(&tabs);
        QToolButton button;button.setText("Active command");button.setCheckable(true);button.setChecked(true);
        layout.addWidget(&button);window.resize(500, 400);window.show();QCoreApplication::processEvents();
        QVERIFY(tree.hasMouseTracking());QVERIFY(tree.viewport()->hasMouseTracking());
        QVERIFY(tree.styleSheet().isEmpty());QCOMPARE(tree.font(), QApplication::font(&tree));
        QCOMPARE(tree.palette().color(QPalette::Inactive,QPalette::Highlight),PartsInteraction::selection());
        // Native focus decoration may tint its own pixels; test the selection
        // fill independently while keeping platform focus painting intact.
        button.setFocus();QCoreApplication::processEvents();
        const auto offeredRect = tree.visualItemRect(offered);
        const auto selectedRect = tree.visualItemRect(selected);
        QTest::mouseMove(tree.viewport(), offeredRect.center());QCoreApplication::processEvents();
        const auto sample = [&](const QRect &rect) {
            const auto image = tree.viewport()->grab().toImage();
            return image.pixelColor(QPointF(rect.right()-4, rect.center().y()).toPoint() * image.devicePixelRatio());
        };
        QTRY_COMPARE(sample(offeredRect), PartsInteraction::hover());
        QCOMPARE(sample(selectedRect), PartsInteraction::selection());
        if (qEnvironmentVariableIsSet("PARTS_INTERACTION_SCREENSHOT"))
            QVERIFY(window.grab().save(qEnvironmentVariable("PARTS_INTERACTION_SCREENSHOT")));
        QTest::mouseMove(&button, button.rect().center());QCoreApplication::processEvents();
        QTRY_VERIFY(sample(offeredRect) != PartsInteraction::hover());
        QCOMPARE(tree.currentItem(), selected);
    }

    void initTestCase()
    {
        QCoreApplication::setOrganizationName("ZimaPartsTests");
        QCoreApplication::setApplicationName("PartsIntegration");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
        Settings::get()->UpdatesAutomatic = false;
    }

    void updateSettingsAreLocalizedAndDoNotStartDownload()
    {
        const QMap<QString, QString> expected{{"cs_CZ", "Nainstalovat aktualizaci"},
            {"de_DE", "Update installieren"}, {"fr_FR", "Installer la mise à jour"},
            {"ru_RU", "Установить обновление"}, {"en_US", "Install update"}};
        SettingsDialog dialog(nullptr);
        dialog.setSection(SettingsDialog::Updates);
        dialog.resize(900, 620); dialog.show();
        const auto tabs = dialog.findChild<QTabWidget *>("tabWidget");
        QVERIFY(tabs); QCOMPARE(tabs->currentIndex(), int(SettingsDialog::Updates));
        auto install = dialog.findChild<QPushButton *>("installUpdate");
        QVERIFY(install); QVERIFY(!install->isEnabled());
        QVERIFY(!UpdateService::get()->busy());
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            applyApplicationLanguage(it.key()); QCoreApplication::processEvents();
            QCOMPARE(install->text(), it.value());
            if (it.key() == "cs_CZ" && qEnvironmentVariableIsSet("PARTS_UPDATE_SCREENSHOT"))
                QVERIFY(dialog.grab().save(qEnvironmentVariable("PARTS_UPDATE_SCREENSHOT")));
        }
        applyApplicationLanguage("en_US");
        QVERIFY(!UpdateService::get()->busy());
        DirectoryWidget directory;
        auto indicator = directory.findChild<QToolButton *>("updateAvailableIndicator");
        QVERIFY(indicator); QVERIFY(indicator->isHidden());
    }

    void aiReadsUseCapturedRootsAndNeverSendAnAutomaticInventory()
    {
        QTemporaryDir root, other, working;
        touch(root.path(), "inside.txt"); touch(other.path(), "outside.txt"); touch(working.path(), "destination.txt");
        PartsCore::CommandContext context; context.directory = root.path(); context.workingDirectory = working.path();
        const auto sent = PartsAi::contextData(context);
        QCOMPARE(sent.size(), 4); QVERIFY(!sent.contains("entries")); QVERIFY(!sent.contains("selectedFiles"));
        auto read = PartsAi::runTool("read_text", {{"path", "inside.txt"}, {"offset", 0}}, context);
        QVERIFY(read.success); QCOMPARE(read.data["text"].toString(), QString("fixture"));
        QVERIFY(!PartsAi::runTool("read_text", {{"path", other.filePath("outside.txt")}, {"offset", 0}}, context).success);
        QVERIFY(PartsAi::runTool("read_text", {{"path", working.filePath("destination.txt")}, {"offset", 0}}, context).success);
        QVERIFY(!PartsAi::runTool("read_text", {{"path", "inside.txt"}, {"offset", -1}}, context).success);
        QVERIFY(!PartsAi::runTool("unknown", {}, context).success);
        QVERIFY(!PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"list", other.path()}}}, context).success);
        auto help = PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"help"}}}, context);
        QVERIFY(help.success); QVERIFY(help.data["text"].toString().contains("ps2pdf"));
        QVERIFY(!PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"update", "install", "--apply"}}}, context).success);
        for (int i = 0; i < 130; ++i) touch(root.path(), QString("file-%1").arg(i));
        auto page = PartsAi::runTool("directory_list", {{"path", "."}, {"offset", 0}}, context);
        QVERIFY(page.success); QCOMPARE(page.data["entries"].toArray().size(), 100);
        QCOMPARE(page.data["nextOffset"].toInt(), 100);
        auto second = PartsAi::runTool("directory_list", {{"path", "."}, {"offset", 100}}, context);
        QCOMPARE(second.data["entries"].toArray().size(), 31); QVERIFY(second.data["nextOffset"].isNull());
    }

    void aiPreviewHonorsLocksAndRejectsDirectApply()
    {
        QTemporaryDir root;
        touch(root.path(), "part.prt.1"); touch(root.path(), "part.prt.2");
        QDir(root.path()).mkpath("0000-index");
        const auto metadata = root.filePath("0000-index/metadata.ini");
        { QSettings s(metadata, QSettings::IniFormat); s.setValue("Directory/PreventRemoval", true); s.sync(); }
        PartsCore::CommandContext context; context.directory = root.path();
        auto locked = PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"ptc-clean"}}}, context);
        QVERIFY(locked.success); QVERIFY(locked.plan.items.isEmpty()); QCOMPARE(locked.plan.skipped.size(), 1);
        auto applied = PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"ptc-clean", "--apply"}}}, context);
        QVERIFY(!applied.success); QVERIFY(QFileInfo::exists(root.filePath("part.prt.1")));
        { QSettings s(metadata, QSettings::IniFormat); s.setValue("Directory/PreventRemoval", false); s.sync(); }
        auto preview = PartsAi::runTool("parts_command", {{"arguments", QJsonArray{"ptc-clean"}}}, context);
        QCOMPARE(preview.plan.items.size(), 1);
        QFile changed(root.filePath("part.prt.1")); QVERIFY(changed.open(QIODevice::Append)); changed.write("changed"); changed.close();
        auto stale = PartsCore::applyTool(preview.plan);
        QCOMPARE(stale["failed"].toArray().size(), 1); QVERIFY(QFileInfo::exists(changed.fileName()));
    }

    void aiReferencesGrantExactFilesAndDirectoryDescendants()
    {
        QTemporaryDir root, external;
        touch(external.path(), "žluťoučký part.txt"); touch(external.path(), "sibling.txt");
        QDir(external.path()).mkpath("attached/sub"); touch(external.filePath("attached/sub"), "notes.txt");
        PartsCore::CommandContext context; context.directory = root.path();
        const auto filePath = external.filePath("žluťoučký part.txt");
        const auto refs = PartsAi::referenceData({filePath, external.filePath("attached"), filePath});
        QCOMPARE(refs.size(), 2);
        QCOMPARE(refs[0].toObject()["type"].toString(), QString("file"));
        QCOMPARE(refs[1].toObject()["type"].toString(), QString("directory"));
        QVERIFY(PartsAi::runTool("read_text", {{"path", filePath}, {"offset", 0}}, context, refs).success);
        QVERIFY(!PartsAi::runTool("read_text", {{"path", external.filePath("sibling.txt")}, {"offset", 0}}, context, refs).success);
        QVERIFY(PartsAi::runTool("read_text", {{"path", external.filePath("attached/sub/notes.txt")}, {"offset", 0}}, context, refs).success);
        QVERIFY(!PartsAi::runTool("directory_list", {{"path", external.path()}, {"offset", 0}}, context, refs).success);
        QVERIFY(!PartsAi::runTool("read_text", {{"path", filePath}, {"offset", 0}}, context).success);
        const auto sent = PartsAi::contextData(context, refs);
        QVERIFY(!QJsonDocument(sent).toJson().contains("fixture"));
        QVERIFY(QFile::remove(filePath)); QVERIFY(QDir().mkpath(filePath)); touch(filePath, "new.txt");
        QVERIFY(!PartsAi::runTool("read_text", {{"path", filePath + "/new.txt"}, {"offset", 0}}, context, refs).success);
    }

    void aiDroppedPathsAreEditableTextAndUseTheRequestSnapshot()
    {
        QTemporaryDir root, external, directory;
        touch(external.path(), "notes with spaces.txt");
        PartsCore::CommandContext context; context.directory = root.path();
        FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai);
        panel.resize(1000, 360); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput"); input->setText("Look at this file");
        QMimeData mime; mime.setUrls({QUrl::fromLocalFile(external.filePath("notes with spaces.txt")), QUrl::fromLocalFile(directory.path())});
        QDragEnterEvent enter(QPoint(10, 10), Qt::CopyAction | Qt::MoveAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(input, &enter); QVERIFY(enter.isAccepted());
        QDropEvent drop(QPointF(10, 10), Qt::CopyAction | Qt::MoveAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(input, &drop); QVERIFY(drop.isAccepted()); QCOMPARE(drop.dropAction(), Qt::CopyAction);
        const auto filePath = QFileInfo(external.filePath("notes with spaces.txt")).canonicalFilePath();
        const auto directoryPath = QFileInfo(directory.path()).canonicalFilePath();
        const auto expected = "Look at this file " + PartsAi::quotedPath(filePath) + ' ' + PartsAi::quotedPath(directoryPath);
        QCOMPARE(ai.asks, 0); QCOMPARE(input->text(), expected);
        QVERIFY(panel.findChildren<QFrame *>("aiReferenceChip").isEmpty());
        QVERIFY(panel.addAiReferences({filePath})); QCOMPARE(input->text(), expected);
        if (qEnvironmentVariableIsSet("PARTS_AI_REFERENCES_SCREENSHOT")) {
            applyApplicationLanguage("cs_CZ"); QCoreApplication::processEvents(); QCoreApplication::processEvents();
            QVERIFY(panel.grab().save(qEnvironmentVariable("PARTS_AI_REFERENCES_SCREENSHOT")));
            applyApplicationLanguage("en_US"); QCoreApplication::processEvents();
        }
        // Removing a path from the draft must not silently retain it as an attachment.
        input->setText("Look at " + PartsAi::quotedPath(filePath));
        const auto sent = input->text();
        QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 1);
        QCOMPARE(ai.prompt, sent); QVERIFY(input->text().isEmpty());
        QCOMPARE(ai.context["references"].toArray().size(), 1);
        QVERIFY(panel.addAiReferences({directoryPath})); // Draft for the next request only.
        emit ai.toolRequested("not-sent", "directory_list", {{"path", directoryPath}, {"offset", 0}});
        QTRY_COMPARE(ai.lastId, QString("not-sent")); QVERIFY(!ai.lastSuccess);
        emit ai.toolRequested("captured", "read_text", {{"path", external.filePath("notes with spaces.txt")}, {"offset", 0}});
        QTRY_COMPARE(ai.lastId, QString("captured")); QVERIFY(ai.lastSuccess);
        ai.running = false; emit ai.answer("Done");
        QVERIFY(input->text().contains(PartsAi::quotedPath(directoryPath)));
        QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 2);
        QCOMPARE(ai.context["references"].toArray().size(), 2);
        // Paths already sent remain part of this conversation, like ordinary chat text.
        emit ai.toolRequested("follow-up", "read_text", {{"path", filePath}, {"offset", 0}});
        QTRY_COMPARE(ai.lastId, QString("follow-up")); QVERIFY(ai.lastSuccess);
        ai.running = false; emit ai.answer("Done");
        input->setText("/exit"); QTest::keyClick(input, Qt::Key_Return); QVERIFY(input->text().isEmpty());
        input->setText("codex"); QTest::keyClick(input, Qt::Key_Return);
        input->setText("/new"); QTest::keyClick(input, Qt::Key_Return);
        input->setText("A new question"); QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 3);
        QVERIFY(ai.context["references"].toArray().isEmpty());
        ai.running = false; emit ai.answer("Done");
        mime.setUrls({QUrl("https://example.com/not-a-local-file")});
        QDragEnterEvent remote(QPoint(10, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(input, &remote); QVERIFY(!remote.isAccepted());
    }

    void aiPathInsertionSupportsSelectionUndoAndManualPaths()
    {
        QTemporaryDir root;
        touch(root.path(), "česká poznámka.txt");
        const auto path = QFileInfo(root.filePath("česká poznámka.txt")).canonicalFilePath();
        PartsCore::CommandContext context; context.directory = root.path();
        FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        input->setText("Compare HERE with the original"); input->setSelection(8, 4);
        QVERIFY(panel.addAiReferences({path}));
        QCOMPARE(input->text(), "Compare " + PartsAi::quotedPath(path) + " with the original");
        QCOMPARE(PartsAi::promptReferences(input->text()).first().toObject()["path"].toString(), path);
        input->undo(); QCOMPARE(input->text(), QString("Compare HERE with the original"));
        QVERIFY(PartsAi::promptReferences(input->text()).isEmpty());
        const auto native = '"' + QDir::toNativeSeparators(path) + '"';
        QCOMPARE(PartsAi::promptReferences("Read " + native).first().toObject()["path"].toString(), path);
        QCOMPARE(ai.asks, 0);
    }

    void aiFileContextMenuUsesOnlySelectedRowsAndOpensPanel()
    {
        QTemporaryDir root; touch(root.path(), "first.txt"); touch(root.path(), "second.txt");
        FileView view; view.setDirectory(root.path()); view.resize(700, 400); view.show();
        QTRY_COMPARE(view.model()->rowCount(), 2);
        view.selectionModel()->select(view.model()->index(0, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        view.selectionModel()->select(view.model()->index(1, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        QCOMPARE(view.selectedReferencePaths().size(), 2);
        QSignalSpy requested(&view, &FileView::aiReferencesRequested);
        QTimer::singleShot(0, &view, [&view] {
            auto menu = view.findChild<QMenu *>(); QVERIFY(menu);
            auto action = menu->findChild<QAction *>("addToAiQuestion"); QVERIFY(action); action->trigger(); menu->close();
        });
        const auto point = view.visualRect(view.model()->index(0, 0)).center();
        QVERIFY(QMetaObject::invokeMethod(&view, "showContextMenu", Q_ARG(QPoint, point)));
        QCOMPARE(requested.size(), 1); QCOMPARE(requested.first()[0].toStringList().size(), 2);
        MainWindow window(nullptr); window.show();
        auto dock = window.findChild<QDockWidget *>("commandPanelDock"); dock->hide();
        auto tabs = window.findChild<MainTabWidget *>(); QVERIFY(tabs);
        emit tabs->currentDataSource()->aiReferencesRequested({root.path()});
        QVERIFY(dock->isVisible());
        QCOMPARE(window.findChild<CommandPanel *>()->findChild<QLineEdit *>("commandPanelInput")->text(),
            PartsAi::quotedPath(QFileInfo(root.path()).canonicalFilePath()));
        QVERIFY(!partsAiProvider()->busy());
    }

    void aiSystemCommandsRequireReviewAndReturnRealResults()
    {
        QTemporaryDir root;
        PartsCore::CommandContext context; context.directory = root.path();
        FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        input->setText("codex"); QTest::keyClick(input, Qt::Key_Return);
        input->setText("Create a note"); QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 1);
#ifdef Q_OS_WIN
        const QString command = "[IO.File]::WriteAllText((Join-Path (Get-Location) 'made.txt'), 'fixture'); Write-Output 'žluťoučký'";
#else
        const QString command = "printf fixture > made.txt; printf 'žluťoučký\\n'";
#endif
        QJsonObject args{{"command", command}, {"directory", "."}, {"reason", "Create the requested note"}, {"timeoutSeconds", 10}};
        emit ai.toolRequested("decline", "system_command", args);
        auto dialog = panel.findChild<SystemCommandDialog *>(); QVERIFY(dialog);
        QVERIFY(dialog->isVisible()); QVERIFY(!dialog->isWindow()); QVERIFY(!dialog->isModal());
        QVERIFY(!QApplication::activeModalWidget());
        QCOMPARE(dialog->findChild<QPlainTextEdit *>("systemCommandPreview")->toPlainText(), command);
        QVERIFY(dialog->findChild<QPlainTextEdit *>("systemCommandPreview")->parentWidget() != dialog);
        QVERIFY(!QFileInfo::exists(root.filePath("made.txt"))); QVERIFY(!dialog->wasStarted());
        QVERIFY(input->isEnabled()); QTest::keyClicks(input, "Draft during approval");
        QTest::keyClick(input, Qt::Key_Return);
        QCOMPARE(ai.asks, 1); QVERIFY(!dialog->wasStarted());
        dialog->findChild<QPushButton *>("denySystemCommand")->click();
        QTRY_COMPARE(ai.lastId, QString("decline")); QVERIFY(ai.lastResult["cancelled"].toBool());
        QVERIFY(!QFileInfo::exists(root.filePath("made.txt")));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        emit ai.toolRequested("approved", "system_command", args);
        dialog = panel.findChild<SystemCommandDialog *>(); QVERIFY(dialog);
        if (qEnvironmentVariableIsSet("PARTS_INLINE_REVIEW_SCREENSHOT")) {
            panel.resize(1000, 540); QCoreApplication::processEvents();
            QTest::qWait(100);
            QVERIFY(panel.grab().save(qEnvironmentVariable("PARTS_INLINE_REVIEW_SCREENSHOT")));
        }
        dialog->findChild<QPushButton *>("runSystemCommand")->click();
        QTRY_COMPARE_WITH_TIMEOUT(ai.lastId, QString("approved"), 15000);
        QVERIFY2(ai.lastSuccess, QJsonDocument(ai.lastResult).toJson().constData());
        QVERIFY(QFileInfo::exists(root.filePath("made.txt"))); QCOMPARE(ai.lastResult["exitCode"].toInt(), 0);
        QVERIFY(ai.lastResult["output"].toString().contains("žluťoučký"));
        QCOMPARE(input->text(), QString("Draft during approval"));
        args["timeoutSeconds"] = 0;
        emit ai.toolRequested("invalid", "system_command", args); QCOMPARE(ai.lastId, QString("invalid")); QVERIFY(!ai.lastSuccess);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        args["timeoutSeconds"] = 10;
        emit ai.toolRequested("stale", "system_command", args);
        dialog = panel.findChild<SystemCommandDialog *>(); QVERIFY(dialog);
        emit ai.failed("Fixture disconnect during approval");
        QVERIFY(!dialog->isVisible()); QVERIFY(!dialog->wasStarted());
        panel.findChild<QPushButton *>("commandPanelStop")->click();
    }

    void aiTreeAndRootMenusProvideDirectoryReferences()
    {
        QTemporaryDir root; QVERIFY(QDir(root.path()).mkpath("child"));
        DataSource source("Fixture", root.path());
        QScopedValueRollback<DataSourceList> restore(Settings::get()->DataSources, {&source});
        DataSourceWidget widget(root.path()); widget.resize(900, 650); widget.show();
        QTRY_VERIFY(widget.findChild<DataSourceView *>());
        auto view = widget.findChild<DataSourceView *>();
        QTRY_VERIFY(view->navigateToDirectory(root.filePath("child")));
        QVERIFY(view->dragEnabled());
        QScopedPointer<QMimeData> mime(view->model()->mimeData({view->currentIndex()}));
        QCOMPARE(mime->urls(), QList<QUrl>{QUrl::fromLocalFile(root.filePath("child"))});
        QSignalSpy requests(&widget, &DataSourceWidget::aiReferencesRequested);
        QTimer::singleShot(0, &widget, [view] {
            auto menu = view->findChild<QMenu *>(); QVERIFY(menu);
            auto action = menu->findChild<QAction *>("addToAiQuestion"); QVERIFY(action); action->trigger(); menu->close();
        });
        const auto point = view->visualRect(view->currentIndex()).center();
        QVERIFY(QMetaObject::invokeMethod(view, "showContextMenu", Q_ARG(QPoint, point)));
        QCOMPARE(requests.size(), 1); QCOMPARE(requests.first()[0].toStringList(), QStringList{root.filePath("child")});
        QTimer::singleShot(0, &widget, [&widget] {
            auto menu = qobject_cast<QMenu *>(QApplication::activePopupWidget()); QVERIFY(menu);
            auto action = menu->findChild<QAction *>("addToAiQuestion"); QVERIFY(action);
            // Native popup positioning can still be pending on Wayland.
            menu->setActiveAction(action);
            QTest::keyClick(menu, Qt::Key_Return);
        });
        QVERIFY(QMetaObject::invokeMethod(&widget, "showDataSourceContextMenu", Q_ARG(int, 0), Q_ARG(QPoint, widget.mapToGlobal(QPoint(20, 20)))));
        QCOMPARE(requests.size(), 2); QCOMPARE(requests.last()[0].toStringList(), QStringList{root.path()});
    }

    void systemCommandsReportFailureTimeoutAndCancellation()
    {
        QTemporaryDir root;
        SystemCommandDialog failure("exit 7", root.path(), "Fixture failure", 10);
        QSignalSpy failed(&failure, &QDialog::finished);
        failure.findChild<QPushButton *>("runSystemCommand")->click();
        QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000); QCOMPARE(failure.operationResult()["exitCode"].toInt(), 7);
#ifdef Q_OS_WIN
        const QString delayed = "Start-Sleep -Seconds 4; [IO.File]::WriteAllText((Join-Path (Get-Location) 'late.txt'), 'bad')";
#else
        const QString delayed = "sleep 4; printf bad > late.txt";
#endif
        SystemCommandDialog timeout(delayed, root.path(), "Fixture timeout", 1);
        QSignalSpy timed(&timeout, &QDialog::finished);
        timeout.findChild<QPushButton *>("runSystemCommand")->click();
        QTRY_COMPARE_WITH_TIMEOUT(timed.size(), 1, 10000); QVERIFY(timeout.operationResult()["timedOut"].toBool());
        SystemCommandDialog stopped(delayed, root.path(), "Fixture cancellation", 10);
        stopped.show();
        QSignalSpy cancelled(&stopped, &QDialog::finished);
        stopped.findChild<QPushButton *>("runSystemCommand")->click();
        QTest::qWait(500); stopped.close();
        QTRY_COMPARE_WITH_TIMEOUT(cancelled.size(), 1, 10000); QVERIFY(stopped.operationResult()["cancelled"].toBool());
        QVERIFY(!QFileInfo::exists(root.filePath("late.txt")));
#ifdef Q_OS_WIN
        QString childPath = root.filePath("child-late.txt"); childPath.replace("'", "''");
        const QString script = "Start-Sleep -Seconds 3; [IO.File]::WriteAllText('" + childPath + "', 'bad')";
        const auto encoded = QByteArray(reinterpret_cast<const char *>(script.utf16()), script.size() * 2).toBase64();
        const QString childCommand = "$child = Start-Process -FilePath ($env:SystemRoot + '\\System32\\WindowsPowerShell\\v1.0\\powershell.exe') "
            "-ArgumentList '-NoProfile','-NonInteractive','-EncodedCommand','" + QString::fromLatin1(encoded)
            + "' -PassThru -WindowStyle Hidden; [IO.File]::WriteAllText((Join-Path (Get-Location) 'child.pid'), [string]$child.Id); Wait-Process -Id $child.Id";
#else
        const QString childCommand = "(sleep 3; printf bad > child-late.txt) & child=$!; printf '%s' $child > child.pid; wait $child";
#endif
        SystemCommandDialog tree(childCommand, root.path(), "Fixture process tree", 10); tree.show();
        QSignalSpy treeStopped(&tree, &QDialog::finished);
        tree.findChild<QPushButton *>("runSystemCommand")->click();
        QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(root.filePath("child.pid")), 5000);
        tree.close(); QTRY_COMPARE_WITH_TIMEOUT(treeStopped.size(), 1, 5000);
        QVERIFY(tree.operationResult()["cancelled"].toBool());
        QTest::qWait(3300); QVERIFY(!QFileInfo::exists(root.filePath("child-late.txt")));
    }

    void aiDraftRemainsEditableUntilExplicitSubmission()
    {
        QTemporaryDir root;
        PartsCore::CommandContext context; context.directory = root.path();
        for (const auto ending : {0, 1, 2}) {
            FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
            auto input = panel.findChild<QLineEdit *>("commandPanelInput");
            auto send = panel.findChild<QPushButton *>("commandPanelRun");
            input->setText("codex"); QTest::keyClick(input, Qt::Key_Return);
            input->setText("First question"); QTest::keyClick(input, Qt::Key_Return);
            QVERIFY(input->isEnabled()); QVERIFY(!input->isReadOnly());
            QTest::keyClicks(input, "Next question"); // Also during local context preparation.
            QTRY_COMPARE(ai.asks, 1); QCOMPARE(ai.prompt, QString("First question"));
            QVERIFY(!send->isEnabled());
            emit ai.message("Still working");
            QTest::keyClick(input, Qt::Key_Return);
            QCOMPARE(ai.asks, 1); QCOMPARE(input->text(), QString("Next question"));
            if (ending == 2) panel.findChild<QPushButton *>("commandPanelStop")->click();
            else {
                ai.running = false;
                if (ending == 0) emit ai.answer("Done");
                else emit ai.failed("Fixture error");
            }
            QVERIFY(input->isEnabled()); QVERIFY(send->isEnabled());
            QCOMPARE(input->text(), QString("Next question")); QCOMPARE(ai.asks, 1);
            QTest::keyClicks(input, " edited");
            QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 2);
            QCOMPARE(ai.prompt, QString("Next question edited"));
            ai.cancel();
        }
    }

    void aiPanelStartsWithCodexAndRetainsRequestDirectory()
    {
        QTemporaryDir first, second;
        touch(first.path(), "first.txt"); touch(second.path(), "second.txt");
        PartsCore::CommandContext context; context.directory = first.path();
        FakeAiProvider ai;
        CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        auto output = panel.findChild<QPlainTextEdit *>("commandPanelOutput");
        input->setText("codex"); QTest::keyClick(input, Qt::Key_Return); QCOMPARE(ai.asks, 0);
        input->setText("What is here?"); QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(ai.asks, 1);
        QCOMPARE(ai.context["currentDirectory"].toString(), first.path()); QVERIFY(!ai.context.contains("entries"));
        context.directory = second.path();
        emit ai.toolRequested("read-first", "directory_list", {{"path", "."}, {"offset", 0}});
        QTRY_COMPARE(ai.lastId, QString("read-first")); QVERIFY(ai.lastSuccess);
        QVERIFY(QJsonDocument(ai.lastResult).toJson().contains("first.txt"));
        QVERIFY(!QJsonDocument(ai.lastResult).toJson().contains("second.txt"));
        ai.running = false; emit ai.message("Done"); emit ai.answer("Done");
        QVERIFY(input->isEnabled());
        input->setText("/exit"); QTest::keyClick(input, Qt::Key_Return);
        QSignalSpy finished(&panel, &CommandPanel::commandFinished);
        input->setText("list"); QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(finished.size(), 1); QVERIFY(output->toPlainText().contains("second.txt"));
    }

    void aiChangesRequireReviewAndCancelledPlansCannotReplay()
    {
        QTemporaryDir root;
        touch(root.path(), "part.prt.1"); touch(root.path(), "part.prt.2");
        PartsCore::CommandContext context; context.directory = root.path();
        FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        input->setText("codex"); QTest::keyClick(input, Qt::Key_Return);
        input->setText("Clean old versions"); QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 1);
        emit ai.toolRequested("preview", "parts_command", {{"arguments", QJsonArray{"ptc-clean"}}});
        QTRY_COMPARE(ai.lastId, QString("preview"));
        const auto plan = ai.lastResult["planId"].toString(); QVERIFY(!plan.isEmpty());
        QVERIFY(QFileInfo::exists(root.filePath("part.prt.1")));
        emit ai.toolRequested("review", "parts_apply", {{"planId", plan}});
        auto dialog = panel.findChild<PartsToolsDialog *>(); QVERIFY(dialog); QVERIFY(dialog->isVisible());
        QVERIFY(!dialog->isWindow()); QVERIFY(!dialog->isModal()); QVERIFY(!QApplication::activeModalWidget());
        QVERIFY(QFileInfo::exists(root.filePath("part.prt.1")));
        dialog->findChild<QPushButton *>("denyPartsOperation")->click();
        QTRY_COMPARE(ai.lastId, QString("review")); QVERIFY(ai.lastResult["cancelled"].toBool());
        emit ai.toolRequested("replay", "parts_apply", {{"planId", plan}});
        QCOMPARE(ai.lastId, QString("replay")); QVERIFY(!ai.lastSuccess);
        QVERIFY(QFileInfo::exists(root.filePath("part.prt.1")));
        panel.findChild<QPushButton *>("commandPanelStop")->click(); QVERIFY(input->isEnabled());
    }

    void aiInlinePartsApprovalAppliesOnlySelectedFiles()
    {
        QTemporaryDir root;
        const QByteArray original = "ISO-10303-21;\nHEADER;\nFILE_NAME('part','2026-09-15',('old'),('ZIMA'),'','','');\nENDSEC;\nDATA;\nENDSEC;\nEND-ISO-10303-21;\n";
        for (const auto &name : {"one.step", "two.step"}) {
            QFile file(root.filePath(name)); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(original);
        }
        PartsCore::CommandContext context; context.directory = root.path();
        FakeAiProvider ai; CommandPanel panel([&] { return context; }, nullptr, &ai); panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        input->setText("codex"); QTest::keyClick(input, Qt::Key_Return);
        input->setText("Update the author"); QTest::keyClick(input, Qt::Key_Return); QTRY_COMPARE(ai.asks, 1);
        emit ai.toolRequested("preview", "parts_command", {{"arguments", QJsonArray{"step-edit", "--set", "author=New author"}}});
        QTRY_COMPARE(ai.lastId, QString("preview"));
        const auto plan = ai.lastResult["planId"].toString(); QVERIFY(!plan.isEmpty());
        emit ai.toolRequested("apply", "parts_apply", {{"planId", plan}});
        auto review = panel.findChild<PartsToolsDialog *>(); QVERIFY(review); QVERIFY(!review->isWindow());
        auto files = review->findChild<QTreeWidget *>("toolFiles"); QCOMPARE(files->topLevelItemCount(), 2);
        const auto selected = files->topLevelItem(0)->text(0), excluded = files->topLevelItem(1)->text(0);
        files->topLevelItem(1)->setCheckState(0, Qt::Unchecked);
        if (qEnvironmentVariableIsSet("PARTS_INLINE_PARTS_SCREENSHOT")) {
            panel.resize(1000, 540); QCoreApplication::processEvents(); QTest::qWait(100);
            QVERIFY(panel.grab().save(qEnvironmentVariable("PARTS_INLINE_PARTS_SCREENSHOT")));
        }
        bool popup = false;
        QTimer modalGuard;
        connect(&modalGuard, &QTimer::timeout, &panel, [&] {
            if (auto modal = qobject_cast<QDialog *>(QApplication::activeModalWidget())) { popup = true; modal->reject(); }
        });
        modalGuard.start(20);
        review->findChild<QPushButton *>("toolApply")->click();
        QTRY_COMPARE(ai.lastId, QString("apply")); QVERIFY(!popup); QVERIFY(ai.lastSuccess);
        QFile changed(selected), unchanged(excluded);
        QVERIFY(changed.open(QIODevice::ReadOnly)); QVERIFY(changed.readAll().contains("New author"));
        QVERIFY(unchanged.open(QIODevice::ReadOnly)); QCOMPARE(unchanged.readAll(), original);
        panel.findChild<QPushButton *>("commandPanelStop")->click();
    }

    void aiSettingsAreLocalizedAndStayOfflineUntilConnect()
    {
        SettingsDialog dialog(nullptr); dialog.setSection(SettingsDialog::AI); dialog.resize(1000, 660); dialog.show();
        const auto login = dialog.findChild<QPushButton *>("aiLogin"); QVERIFY(login); QVERIFY(!login->isEnabled());
        const auto status = dialog.findChild<QLabel *>("aiStatus"); QVERIFY(status);
        const auto executableLabel = dialog.findChild<QLabel *>("aiExecutableLabel"); QVERIFY(executableLabel);
        const auto executable = dialog.findChild<QLineEdit *>("aiExecutable"); QVERIFY(executable);
        const QMap<QString, QString> expected{{"cs_CZ", "Přihlásit přes ChatGPT"}, {"de_DE", "Mit ChatGPT anmelden"},
            {"fr_FR", "Se connecter avec ChatGPT"}, {"ru_RU", "Войти через ChatGPT"}, {"en_US", "Sign in with ChatGPT"}};
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            applyApplicationLanguage(it.key()); QCoreApplication::processEvents(); QCOMPARE(login->text(), it.value());
            QCOMPARE(status->text(), QCoreApplication::translate("CodexProvider", "AI is disconnected."));
            QVERIFY(executableLabel->width() >= executableLabel->fontMetrics().horizontalAdvance(executableLabel->text()));
            QTRY_VERIFY(executableLabel->geometry().right() < executable->geometry().left());
            if (it.key() == "cs_CZ" && qEnvironmentVariableIsSet("PARTS_AI_SCREENSHOT"))
                QVERIFY(dialog.grab().save(qEnvironmentVariable("PARTS_AI_SCREENSHOT")));
            SystemCommandDialog review("echo example", QDir::tempPath(), "Example", 10);
            QCOMPARE(review.findChild<QPushButton *>("runSystemCommand")->text(), QCoreApplication::translate("SystemCommandDialog", "Run command"));
            review.setInlineReview();
            QCOMPARE(review.findChild<QPushButton *>("runSystemCommand")->text(), QCoreApplication::translate("SystemCommandDialog", "Allow"));
            QVERIFY(!review.wasStarted());
            if (it.key() == "cs_CZ" && qEnvironmentVariableIsSet("PARTS_SYSTEM_REVIEW_SCREENSHOT"))
                QVERIFY(review.grab().save(qEnvironmentVariable("PARTS_SYSTEM_REVIEW_SCREENSHOT")));
        }
        QVERIFY(!partsAiProvider()->busy()); QVERIFY(!partsAiProvider()->connected());
        applyApplicationLanguage("en_US");
    }

    void codexProtocolSupportsToolsAndChangingDirectories()
    {
        QTemporaryDir profile, first, second;
        CodexProvider provider(nullptr, profile.path());
        QSignalSpy errors(&provider, &AiProvider::failed), answers(&provider, &AiProvider::answer), calls(&provider, &AiProvider::toolRequested);
        provider.connectAccount(QCoreApplication::applicationFilePath());
        QTRY_VERIFY(provider.ready()); QTRY_VERIFY(!provider.models().isEmpty());
        provider.ask("List files", {{"currentDirectory", first.path()}}, "fixture");
        QTRY_COMPARE(calls.size(), 1); QCOMPARE(calls.first()[1].toString(), QString("directory_list"));
        provider.toolResult(calls.first()[0].toString(), {{"entries", QJsonArray{}}}, true);
        QTRY_COMPARE(answers.size(), 1); QCOMPARE(answers.first()[0].toString(), first.path());
        provider.ask("Now here", {{"currentDirectory", second.path()}}, "fixture");
        QTRY_COMPARE(calls.size(), 2);
        provider.toolResult(calls.last()[0].toString(), {}, true);
        QTRY_COMPARE(answers.size(), 2); QCOMPARE(answers.last()[0].toString(), second.path());
        QVERIFY(errors.isEmpty());
        provider.ask("WAIT_FOREVER", {{"currentDirectory", second.path()}}, "fixture");
        QVERIFY(provider.busy()); provider.cancel(); QVERIFY(!provider.busy()); QVERIFY(!provider.ready());
    }

    void codexProtocolRejectsNativeToolsAndMalformedMessages()
    {
        for (const auto &request : {QString("NATIVE_TOOL"), QString("INVALID_JSON")}) {
            QTemporaryDir profile;
            CodexProvider provider(nullptr, profile.path());
            QSignalSpy errors(&provider, &AiProvider::failed), answers(&provider, &AiProvider::answer);
            provider.connectAccount(QCoreApplication::applicationFilePath()); QTRY_VERIFY(provider.ready());
            provider.ask(request, {{"currentDirectory", profile.path()}}, "fixture");
            QTRY_COMPARE(errors.size(), 1); QVERIFY(answers.isEmpty()); QVERIFY(!provider.busy());
        }
    }

    void toolPlansRejectChangedFilesAndLocks()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prt.1"); touch(dir.path(), "part.prt.10");
        PartsCore::ToolRequest request; request.tool = "ptc-clean"; request.path = dir.path();
        auto plan = PartsCore::planTool(request);
        QCOMPARE(plan.items.size(), 1);
        QFile changed(dir.filePath("part.prt.1")); QVERIFY(changed.open(QIODevice::Append)); changed.write("changed"); changed.close();
        auto result = PartsCore::applyTool(plan);
        QCOMPARE(result["failed"].toArray().size(), 1);
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
        plan = PartsCore::planTool(request);
        QVERIFY(QFile::remove(dir.filePath("part.prt.10")));
        result = PartsCore::applyTool(plan);
        QCOMPARE(result["failed"].toArray().size(), 1);
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
        touch(dir.path(), "part.prt.10");
        plan = PartsCore::planTool(request);
        QDir().mkpath(dir.filePath("0000-index"));
        QSettings lock(dir.filePath("0000-index/metadata.ini"), QSettings::IniFormat);
        lock.setValue("Directory/PreventRemoval", true); lock.sync();
        result = PartsCore::applyTool(plan);
        QCOMPARE(result["failed"].toArray().size(), 1);
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
    }

    void toolDialogInvalidatesChangedPreview()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prt.1"); touch(dir.path(), "part.prt.10");
        PartsToolsDialog dialog("ptc-clean", dir.path());
        dialog.show();
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        auto old = dialog.findChild<QCheckBox *>("toolCleanOld");
        QVERIFY(!dialog.findChild<QPushButton *>("toolPreview"));
        QVERIFY(!dialog.findChild<QLineEdit *>("toolPath"));
        QCOMPARE(apply->text(), QString("Clean"));
        QVERIFY(!apply->isEnabled());
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->columnCount(), 1);
        QVERIFY(files->isHeaderHidden());
        QCOMPARE(files->topLevelItem(0)->text(0), dir.filePath("part.prt.1"));
        old->setChecked(false);
        QVERIFY(!apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 0);
        QTRY_VERIFY(old->isEnabled() && !apply->isEnabled());
        old->setChecked(true);
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 1);
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.10")));
    }

    void cleanerAutomaticallyRefreshesMasksAndRecursion()
    {
        QTemporaryDir dir;
        touch(dir.path(), "root.prt.1"); touch(dir.path(), "root.prt.2");
        touch(dir.path(), "trail.log");
        QVERIFY(QDir(dir.path()).mkdir("child"));
        touch(dir.filePath("child"), "child.prt.1");
        touch(dir.filePath("child"), "child.prt.2");
        PartsToolsDialog dialog("ptc-clean", dir.path());
        dialog.show();
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        auto masks = dialog.findChild<QLineEdit *>("toolCleanMasks");
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 1);
        dialog.findChild<QCheckBox *>("toolRecursive")->setChecked(true);
        QVERIFY(!apply->isEnabled());
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 2);
        masks->setText("*");
        masks->setText("*.log");
        QVERIFY(!apply->isEnabled());
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 3);
        for (int i = 0; i < files->topLevelItemCount(); ++i)
            QVERIFY(!files->topLevelItem(i)->text(0).endsWith(".2"));
        masks->clear();
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 2);
        QVERIFY(QFileInfo::exists(dir.filePath("root.prt.1")));
        QVERIFY(QFileInfo::exists(dir.filePath("child/child.prt.1")));
        QVERIFY(QFileInfo::exists(dir.filePath("trail.log")));
    }

    void cleanerRemovesOnlyCompletedRows()
    {
        QTemporaryDir dir;
        touch(dir.path(), "first.prt.1"); touch(dir.path(), "first.prt.2");
        touch(dir.path(), "second.prt.1"); touch(dir.path(), "second.prt.2");
        PartsToolsDialog dialog("ptc-clean", dir.path());
        dialog.show();
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QSignalSpy changed(&dialog, &PartsToolsDialog::filesChanged);
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 2);
        files->topLevelItem(1)->setCheckState(0, Qt::Unchecked);
        const auto confirm = [&dialog] {
            auto box = dialog.findChild<QMessageBox *>();
            QVERIFY(box);
            QCOMPARE(box->text(), QString("Move 1 selected files to the trash?"));
            QTest::mouseClick(box->button(QMessageBox::Yes), Qt::LeftButton);
        };
        QTimer::singleShot(0, &dialog, confirm);
        apply->click();
        QTRY_COMPARE(changed.count(), 1);
        QCOMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->topLevelItem(0)->checkState(0), Qt::Unchecked);
        QVERIFY(!QFileInfo::exists(dir.filePath("first.prt.1")));
        QVERIFY(QFileInfo::exists(dir.filePath("first.prt.2")));
        QVERIFY(QFileInfo::exists(dir.filePath("second.prt.1")));
        files->topLevelItem(0)->setCheckState(0, Qt::Checked);
        QTimer::singleShot(0, &dialog, confirm);
        apply->click();
        QTRY_COMPARE(changed.count(), 2);
        QCOMPARE(files->topLevelItemCount(), 0);
        QVERIFY(!apply->isEnabled());
        QVERIFY(!QFileInfo::exists(dir.filePath("second.prt.1")));
        QVERIFY(QFileInfo::exists(dir.filePath("second.prt.2")));
    }

    void cleanerKeepsFailedRowsAndShowsOptionsBelowFiles()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prt.1"); touch(dir.path(), "part.prt.2");
        PartsToolsDialog dialog("ptc-clean", dir.path());
        dialog.show();
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QTRY_VERIFY(apply->isEnabled());
        auto recursive = dialog.findChild<QCheckBox *>("toolRecursive");
        QVERIFY(recursive->mapTo(&dialog, QPoint()).y() > files->geometry().bottom());
        QFile changed(dir.filePath("part.prt.1"));
        QVERIFY(changed.open(QIODevice::Append)); changed.write("changed"); changed.close();
        QTimer::singleShot(0, &dialog, [&dialog] {
            auto box = dialog.findChild<QMessageBox *>();
            QVERIFY(box);
            QTest::mouseClick(box->button(QMessageBox::Yes), Qt::LeftButton);
        });
        apply->click();
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 1);
        QVERIFY(files->topLevelItem(0)->toolTip(0).contains("File changed since preview"));
        QVERIFY(dialog.findChild<QLabel *>("toolStatus")->text().contains("File changed since preview"));
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
    }

    void cleanerCoalescesDirectoryRefreshes()
    {
        QTemporaryDir dir, other;
        touch(dir.path(), "first.prt.1"); touch(dir.path(), "first.prt.2");
        touch(other.path(), "other.prt.1");
        auto cache = PartCache::get();
        QCOMPARE(cache->count(dir.path()), 2);
        QCOMPARE(cache->count(other.path()), 1);
        QSignalSpy changed(cache, &PartCache::directoryChanged);
        cache->beginFileChanges(dir.path());
        cache->beginFileChanges(dir.path());
        QVERIFY(QFile::remove(dir.filePath("first.prt.1")));
        QVERIFY(QMetaObject::invokeMethod(cache, "onDirectoryChange", Q_ARG(QString, dir.path())));
        QCOMPARE(cache->count(dir.path()), 2);
        QVERIFY(changed.isEmpty());
        touch(other.path(), "other.prt.2");
        QVERIFY(QMetaObject::invokeMethod(cache, "onDirectoryChange", Q_ARG(QString, other.path())));
        QCOMPARE(cache->count(other.path()), 2);
        QCOMPARE(changed.count(), 1);
        cache->endFileChanges(dir.path());
        QCOMPARE(cache->count(dir.path()), 2);
        cache->endFileChanges(dir.path());
        QCOMPARE(cache->count(dir.path()), 1);
        QCOMPARE(changed.count(), 2);
        cache->clear(dir.path()); cache->clear(other.path());
    }

    void cleanerLocalizedLayout()
    {
        QTemporaryDir dir;
        touch(dir.path(), "hydraulic_block.prt.1"); touch(dir.path(), "hydraulic_block.prt.12");
        touch(dir.path(), "hydraulic_block.prtz"); touch(dir.path(), "hydraulic_block.prtz.1");
        QFile drawing(dir.filePath("drawing.ps"));
        QVERIFY(drawing.open(QIODevice::WriteOnly)); drawing.write("%!PS\nshowpage\n"); drawing.close();
        applyApplicationLanguage("cs_CZ");
        {
            PartsToolsDialog dialog("ptc-clean", dir.path());
            dialog.show();
            auto apply = dialog.findChild<QPushButton *>("toolApply");
            QTRY_VERIFY(apply->isEnabled());
            QCOMPARE(apply->text(), QString::fromUtf8("Vyčistit"));
            if (qEnvironmentVariableIsSet("PARTS_CLEANER_SCREENSHOT"))
                QVERIFY(dialog.grab().save(qEnvironmentVariable("PARTS_CLEANER_SCREENSHOT")));
        }
        for (const auto &tool : {"zima-clean", "ps2pdf"}) {
            PartsToolsDialog dialog(tool, dir.path()); dialog.show();
            QTRY_VERIFY(dialog.findChild<QPushButton *>("toolApply")->isEnabled());
            if (qEnvironmentVariableIsSet("PARTS_TOOLS_SCREENSHOTS"))
                QVERIFY(dialog.grab().save(QDir(qEnvironmentVariable("PARTS_TOOLS_SCREENSHOTS")).filePath(QString(tool) + ".png")));
        }
        applyApplicationLanguage("en_US");
    }

    void cleanerPreparedReviewDoesNotRescan()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prt.1"); touch(dir.path(), "part.prt.2");
        PartsCore::ToolRequest request; request.tool = "ptc-clean"; request.path = dir.path();
        const auto plan = PartsCore::planTool(request);
        QCOMPARE(plan.items.size(), 1);
        touch(dir.path(), "other.prt.1"); touch(dir.path(), "other.prt.2");
        PartsToolsDialog dialog("ptc-clean", dir.path());
        dialog.setPreparedPlan(plan);
        dialog.show();
        QTest::qWait(350);
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QCOMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->topLevelItem(0)->text(0), dir.filePath("part.prt.1"));
        QCOMPARE(dialog.findChild<QPushButton *>("toolApply")->text(), QString("Allow selected"));
        QVERIFY(QFileInfo::exists(dir.filePath("part.prt.1")));
    }

    void otherToolDialogsKeepManualPreview()
    {
        QTemporaryDir dir;
        PartsToolsDialog dialog("step-edit", dir.path());
        dialog.show();
        QVERIFY(dialog.findChild<QLineEdit *>("toolPath"));
        QVERIFY(dialog.findChild<QPushButton *>("toolPreview")->isVisible());
        QCOMPARE(dialog.findChild<QPushButton *>("toolApply")->text(), QString("Apply selected"));
        QTest::qWait(350);
        QVERIFY(!dialog.findChild<QPushButton *>("toolApply")->isEnabled());
        QCOMPARE(dialog.findChild<QTreeWidget *>("toolFiles")->topLevelItemCount(), 0);
    }

    void pdfDialogUsesAutomaticCleanerStyleWorkflow()
    {
        QTemporaryDir dir;
        QFile source(dir.filePath("drawing.ps"));
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.write("%!PS-Adobe-3.0\nshowpage\n");
        source.close();
        QVERIFY(QDir(dir.path()).mkdir("child"));
        QVERIFY(QFile::copy(source.fileName(), dir.filePath("child/other.ps")));
        QScopedValueRollback<bool> recursiveDefault(Settings::get()->ToolsRecursive, true);
        PartsToolsDialog dialog("ps2pdf", dir.path());
        dialog.show();
        QVERIFY(!dialog.findChild<QLineEdit *>("toolPath"));
        QVERIFY(!dialog.findChild<QPushButton *>("toolPreview"));
        auto output = dialog.findChild<QLineEdit *>("toolOutputDirectory");
        auto remove = dialog.findChild<QCheckBox *>("toolDeleteSources");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QVERIFY(output); QVERIFY(remove); QVERIFY(files);
        QCOMPARE(output->text(), QString("pdf"));
        QVERIFY(remove->isChecked());
        QVERIFY(!dialog.findChild<QCheckBox *>("toolRecursive"));
        QCOMPARE(files->columnCount(), 1);
        QVERIFY(files->isHeaderHidden());
        QCOMPARE(dialog.findChild<QPushButton *>("toolApply")->text(), QString("Create PDF"));
        QVERIFY(output->mapTo(&dialog, QPoint()).y() > files->geometry().bottom());
        QTRY_COMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->topLevelItem(0)->toolTip(0), dir.filePath("pdf/drawing.pdf"));
    }

    void zimaCleanerRevalidatesAndReportsEachCompletedArchive()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prtz"); touch(dir.path(), "part.prtz.1");
        PartsCore::ToolRequest request; request.tool = "zima-clean"; request.path = dir.path();
        auto plan = PartsCore::planTool(request);
        QCOMPARE(plan.items.size(), 1);
        QFile changed(dir.filePath("part.prtz.1"));
        QVERIFY(changed.open(QIODevice::Append)); changed.write("changed"); changed.close();
        QCOMPARE(PartsCore::applyTool(plan)["failed"].toArray().size(), 1);
        QVERIFY(QFileInfo::exists(changed.fileName()));
        plan = PartsCore::planTool(request);
        QVERIFY(QDir().mkpath(dir.filePath("0000-index")));
        QSettings lock(dir.filePath("0000-index/metadata.ini"), QSettings::IniFormat);
        lock.setValue("Directory/PreventRemoval", true); lock.sync();
        QCOMPARE(PartsCore::applyTool(plan)["failed"].toArray().size(), 1);
        QVERIFY(QFileInfo::exists(changed.fileName()));
        lock.setValue("Directory/PreventRemoval", false); lock.sync();
        touch(dir.path(), "part.prtz.99");
        plan = PartsCore::planTool(request);
        QStringList completed;
        const auto result = PartsCore::applyTool(plan, [&completed](const QString &path) {
            QVERIFY(!QFileInfo::exists(path));
            completed.append(path);
        });
        QCOMPARE(completed.size(), 2);
        QCOMPARE(result["completed"].toArray().size(), 2);
        QVERIFY(QFileInfo::exists(dir.filePath("part.prtz")));
    }

    void zimaCleanerUsesSingleListAndKeepsUncheckedRows()
    {
        QTemporaryDir dir;
        touch(dir.path(), "part.prtz"); touch(dir.path(), "part.prtz.1"); touch(dir.path(), "part.prtz.99");
        PartsToolsDialog dialog("zima-clean", dir.path());
        dialog.show();
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        QVERIFY(!dialog.findChild<QPushButton *>("toolPreview"));
        QVERIFY(!dialog.findChild<QLineEdit *>("toolCleanMasks"));
        QVERIFY(!dialog.findChild<QCheckBox *>("toolCleanOld"));
        QCOMPARE(files->columnCount(), 1);
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 2);
        files->topLevelItem(0)->setCheckState(0, Qt::Unchecked);
        QSignalSpy changed(&dialog, &PartsToolsDialog::filesChanged);
        QTimer::singleShot(0, &dialog, [&dialog] {
            auto box = dialog.findChild<QMessageBox *>();
            QVERIFY(box);
            QTest::mouseClick(box->button(QMessageBox::Yes), Qt::LeftButton);
        });
        apply->click();
        QTRY_COMPARE(changed.count(), 1);
        QCOMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->topLevelItem(0)->checkState(0), Qt::Unchecked);
        QCOMPARE(files->topLevelItem(0)->text(0), dir.filePath("part.prtz.1"));
        QVERIFY(QFileInfo::exists(dir.filePath("part.prtz")));
        QVERIFY(!QFileInfo::exists(dir.filePath("part.prtz.99")));
    }

    void pdfPreparedReviewPreservesExplicitRecursiveScope()
    {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkdir("child"));
        QFile source(dir.filePath("child/drawing.ps"));
        QVERIFY(source.open(QIODevice::WriteOnly)); source.write("%!PS\nshowpage\n"); source.close();
        PartsCore::ToolRequest request; request.tool = "ps2pdf"; request.path = dir.path(); request.recursive = true;
        PartsToolsDialog dialog("ps2pdf", dir.path());
        dialog.setPreparedPlan(PartsCore::planTool(request));
        dialog.show();
        QTest::qWait(300);
        QVERIFY(dialog.findChild<QLabel *>("toolPreparedRecursive")->isVisible());
        QVERIFY(!dialog.findChild<QCheckBox *>("toolDeleteSources")->isChecked());
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QCOMPARE(files->topLevelItemCount(), 1);
        QCOMPARE(files->topLevelItem(0)->text(0), source.fileName());
    }

    void pdfDialogRemovesSuccessesAndKeepsFailuresAndUncheckedFiles()
    {
        QTemporaryDir dir;
        for (const auto &name : {"good.ps", "bad.ps", "unchecked.ps"}) {
            QFile source(dir.filePath(name));
            QVERIFY(source.open(QIODevice::WriteOnly));
            source.write(QString(name) == "bad.ps" ? "%!PS\nInvalidOperator\n" : "%!PS\nshowpage\n");
        }
        PartsToolsDialog dialog("ps2pdf", dir.path());
        dialog.show();
        auto apply = dialog.findChild<QPushButton *>("toolApply");
        auto files = dialog.findChild<QTreeWidget *>("toolFiles");
        QTRY_VERIFY(apply->isEnabled());
        QCOMPARE(files->topLevelItemCount(), 3);
        files->topLevelItem(2)->setCheckState(0, Qt::Unchecked);
        QSignalSpy changed(&dialog, &PartsToolsDialog::filesChanged);
        QTimer::singleShot(0, &dialog, [&dialog] {
            auto box = dialog.findChild<QMessageBox *>();
            QVERIFY(box);
            QTest::mouseClick(box->button(QMessageBox::Yes), Qt::LeftButton);
        });
        apply->click();
        QTRY_COMPARE_WITH_TIMEOUT(changed.count(), 1, 15000);
        QCOMPARE(files->topLevelItemCount(), 2);
        QCOMPARE(files->topLevelItem(0)->text(0), dir.filePath("bad.ps"));
        QVERIFY(!files->topLevelItem(0)->toolTip(0).isEmpty());
        QCOMPARE(files->topLevelItem(1)->checkState(0), Qt::Unchecked);
        QVERIFY(!QFileInfo::exists(dir.filePath("good.ps")));
        QVERIFY(QFileInfo::exists(dir.filePath("pdf/good.pdf")));
        QVERIFY(QFileInfo::exists(dir.filePath("bad.ps")));
        QVERIFY(QFileInfo::exists(dir.filePath("unchecked.ps")));
        QVERIFY(!QFileInfo::exists(dir.filePath("pdf/unchecked.pdf")));
    }

    void commandPanelUsesCapturedContextAndHistory()
    {
        QTemporaryDir first;
        QTemporaryDir second;
        touch(first.path(), "first.pdf");
        touch(second.path(), "second.pdf");
        PartsCore::CommandContext context;
        context.directory = first.path();
        CommandPanel panel([&context] { return context; });
        panel.show();
        auto input = panel.findChild<QLineEdit *>("commandPanelInput");
        auto output = panel.findChild<QPlainTextEdit *>("commandPanelOutput");
        QSignalSpy finished(&panel, &CommandPanel::commandFinished);
        input->setText("list");
        QTest::keyClick(input, Qt::Key_Return);
        context.directory = second.path();
        QTRY_COMPARE(finished.count(), 1);
        QCOMPARE(finished.last().first().toInt(), 0);
        QVERIFY(output->toPlainText().contains("first.pdf"));
        QVERIFY(!output->toPlainText().contains("second.pdf"));
        input->setText("draft");
        QTest::keyClick(input, Qt::Key_Up);
        QCOMPARE(input->text(), QString("list"));
        QTest::keyClick(input, Qt::Key_Down);
        QCOMPARE(input->text(), QString("draft"));
        input->setText("list");
        QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(finished.count(), 2);
        QVERIFY(output->toPlainText().contains("second.pdf"));
        input->setText("params \"second.pdf\"");
        QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(finished.count(), 3);
        QCOMPARE(finished.last().first().toInt(), 0);
        input->setText("params \"unfinished");
        QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(finished.count(), 4);
        QCOMPARE(finished.last().first().toInt(), 2);
        input->setText("help");
        QTest::keyClick(input, Qt::Key_Return);
        QTRY_COMPARE(finished.count(), 5);
        QVERIFY(output->toPlainText().contains("--language"));
        QTest::mouseClick(panel.findChild<QPushButton *>("commandPanelClear"), Qt::LeftButton);
        QVERIFY(output->toPlainText().isEmpty());
        QVERIFY(!QFileInfo::exists(first.filePath("0000-index")));
        QVERIFY(!QFileInfo::exists(second.filePath("0000-index")));
    }

    void commandPanelCanBeHiddenAndRestored()
    {
        MainWindow window(nullptr);
        window.show();
        auto dock = window.findChild<QDockWidget *>("commandPanelDock");
        auto toggle = window.findChild<QAction *>("toggleCommandPanel");
        QVERIFY(dock);
        QVERIFY(toggle);
        QCOMPARE(toggle->shortcut(), QKeySequence("Ctrl+Shift+P"));
        dock->show();
        toggle->trigger();
        QVERIFY(dock->isHidden());
        toggle->trigger();
        QVERIFY(!dock->isHidden());
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

    void zimaParametersUseCurrentFilesAndExistingMetadataHandles()
    {
        QTemporaryDir directory;
        QScopedValueRollback<QString> language(Settings::get()->LanguageMetadata, "cs");
        QFileInfoList files;
        const auto document = [](const QByteArray &type, const QByteArray &name) {
            return QByteArray("[Document]\ntype=") + type + "\nformat_version=41\n"
                "[CachedBodies]\ndata=" + QByteArray(300000, 'x') + "\n"
                "[UserParameters]\nOrder=name,stock,mass,quantity,empty,custom,first,second,nazev,description\n"
                "[UserParameterLabels]\nname\\cs=nazev\nname\\en=Name\nstock\\cs=polotovar\n"
                "mass\\cs=hmotnost\nquantity\\cs=mnozstvi\nfirst\\cs=ambiguous\nsecond\\cs=ambiguous\n"
                "[UserParameterValues]\nname=Shared\nname\\cs=" + name + "\nstock=RHS 40,20 = \"A\"\\B\n"
                "mass=1.250\nquantity=0\nempty=\ncustom=@String(unchanged)\nfirst=A\nsecond=B\n"
                "nazev=Explicit key wins\ndescription=" + name + "\n";
        };
        const auto write = [&](const QString &name, const QByteArray &bytes) {
            QFile file(directory.filePath(name));
            if (!file.open(QIODevice::WriteOnly)) return false;
            if (file.write(bytes) != bytes.size()) return false;
            file.close();files.append(QFileInfo(file.fileName()));return true;
        };
        const auto partBytes = document("part", QString::fromUtf8("Díl žluťoučký").toUtf8());
        QVERIFY(write("part.PRTZ", partBytes));
        QVERIFY(write("assembly.asmz", document("assembly", "Assembly current")));
        QVERIFY(write("part.PRTZ.999", document("part", "Archive must not win")));
        QVERIFY(write("orphan.prtz.1", document("part", "Orphan must not import")));
        QVERIFY(write("drawing.drwz", document("drawing", "Not a model")));
        QVERIFY(write("invalid.prtz", document("assembly", "Wrong type")));
        QVERIFY(write("broken.prtz", document("part", "Partial") + "[UserParameterValues]\nbroken line\n"));
        QVERIFY(write("part.prt.10", "description\x15" "DESCRIPTION'xxProE valuex\x14\n"));
        auto meta = MetadataCache::get()->metadata(directory.path());
        meta->setParameterHandles({"name","nazev","polotovar","hmotnost","mnozstvi","empty","custom","ambiguous","description"});
        meta->setPartParam("part", "empty", "Keep manual value");
        PrtReader reader;QSignalSpy loaded(&reader, &PrtReader::loaded);
        reader.load(directory.path(), files);
        QTRY_COMPARE(meta->partParam("part", "name"), QString::fromUtf8("Díl žluťoučký"));
        QTRY_VERIFY(!reader.isRunning());
        QVERIFY(!loaded.isEmpty());
        QCOMPARE(meta->partParam("part", "nazev"), QString("Explicit key wins"));
        QCOMPARE(meta->partParam("part", "polotovar"), QString("RHS 40,20 = \"A\"\\B"));
        QCOMPARE(meta->partParam("part", "hmotnost"), QString("1.250"));
        QCOMPARE(meta->partParam("part", "mnozstvi"), QString("0"));
        QCOMPARE(meta->partParam("part", "custom"), QString("@String(unchanged)"));
        QCOMPARE(meta->partParam("part", "empty"), QString("Keep manual value"));
        QCOMPARE(meta->partParam("part", "ambiguous"), QString());
        QCOMPARE(meta->partParam("part", "description"), QString::fromUtf8("Díl žluťoučký"));
        QCOMPARE(meta->partParam("assembly", "name"), QString("Assembly current"));
        for (const QString name : {"orphan","drawing","invalid","broken"})
            QCOMPARE(meta->partParam(name, "name"), QString());
        QFile unchanged(directory.filePath("part.PRTZ"));QVERIFY(unchanged.open(QIODevice::ReadOnly));
        QCOMPARE(unchanged.readAll(), partBytes);
        MetadataCache::get()->clear(directory.path());
        QCOMPARE(MetadataCache::get()->metadata(directory.path())->partParam("part", "hmotnost"), QString("1.250"));
    }

    void zimaParametersReadNativeCadSavedFixtures()
    {
        const QString fixtures=QFINDTESTDATA("fixtures/zima-metadata");QVERIFY(!fixtures.isEmpty());
        for (const QString language : {"cs", "en"})
        {
            QTemporaryDir directory;
            QScopedValueRollback<QString> selectedLanguage(Settings::get()->LanguageMetadata, language);
            QFileInfoList files;
            for (const QString file : {"native-part.prtz", "native-assembly.asmz"})
            {
                QVERIFY(QFile::copy(QDir(fixtures).filePath(file),directory.filePath(file)));
                files.append(QFileInfo(directory.filePath(file)));
            }
            auto meta=MetadataCache::get()->metadata(directory.path());
            meta->setParameterHandles({"nazev","polotovar","mnozstvi","mass"});
            PrtReader reader;reader.load(directory.path(),files);
            QTRY_COMPARE(meta->partParam("native-part","nazev"),language=="cs"?QString::fromUtf8("Držák A"):QString("Drzak A"));
            QTRY_VERIFY(!reader.isRunning());
            QCOMPARE(meta->partParam("native-assembly","nazev"),QString("Sestava A"));
            for (const QString part : {"native-part","native-assembly"})
            {
                QCOMPARE(meta->partParam(part,"polotovar"),QString("RHS 40,20 = \"A\""));
                QCOMPARE(meta->partParam(part,"mnozstvi"),QString("0"));
                QCOMPARE(meta->partParam(part,"mass"),QString("0.000"));
            }
        }
    }

    void zimaCzechLabelsShareProeColumnsAndRefreshFileModel()
    {
        QTemporaryDir directory;
        QScopedValueRollback<QString> language(Settings::get()->LanguageMetadata, "cs");
        const QByteArray bytes("[Document]\ntype=part\nformat_version=41\n[UserParameters]\nOrder=name,stock\n"
            "[UserParameterLabels]\nname\\cs=nazev\nstock\\cs=polotovar\n"
            "[UserParameterValues]\nname=Bracket\nstock=Plate 2 mm\n");
        QFile file(directory.filePath("bracket.prtz"));QVERIFY(file.open(QIODevice::WriteOnly));file.write(bytes);file.close();
        auto meta=MetadataCache::get()->metadata(directory.path());meta->setParameterHandles({"nazev","polotovar"});
        FileModel model;QSignalSpy updates(&model,&QAbstractItemModel::dataChanged);model.setDirectory(directory.path());model.reloadParts();
        QTRY_COMPARE(meta->partParam("bracket","nazev"),QString("Bracket"));
        QTRY_VERIFY(!updates.isEmpty());
        const auto parameter = [&](int column) {
            for (int row = 0; row < model.rowCount(); ++row)
                if (model.fileInfo(model.index(row, 0)).fileName() == "bracket.prtz")
                    return model.data(model.index(row, column), Qt::DisplayRole).toString();
            return QString();
        };
        QCOMPARE(parameter(2),QString("Bracket"));
        QCOMPARE(parameter(3),QString("Plate 2 mm"));
        QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));
        file.write(QByteArray(bytes).replace("Bracket","Edited bracket"));file.close();
        model.reloadParts();
        QTRY_COMPARE(meta->partParam("bracket","nazev"),QString("Edited bracket"));
        QCOMPARE(parameter(2),QString("Edited bracket"));
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
    void mainWindowStartsDirectlyWithSettingsLast()
    {
        QTemporaryDir directory;
        auto settings = Settings::get();
        const auto oldTabs = settings->MainTabs;
        settings->MainTabs = {directory.path()};
        settings->ActiveMainTab = 0;
        QTranslator translator;
        QElapsedTimer timer;
        timer.start();
        {
            MainWindow window(&translator);
            QVERIFY2(timer.elapsed() < 4000, "Main-window construction blocked startup");
            for (auto widget : QApplication::topLevelWidgets())
                QVERIFY(!widget->inherits("QSplashScreen"));
            const auto actions = window.findChild<MainToolBar *>()->actions();
            QCOMPARE(actions.last()->objectName(), QString("actionSettings"));
            QCOMPARE(actions.at(actions.size() - 2)->objectName(), QString("toggleCommandPanel"));
            bool eventDelivered = false;
            QTimer::singleShot(0, &window, [&] { eventDelivered = true; });
            QTRY_VERIFY(eventDelivered);
        }
        settings->MainTabs = oldTabs;
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
    if (argc > 1 && QString::fromLocal8Bit(argv[1]) == "app-server") return runAiServerFixture();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QStandardPaths::setTestModeEnabled(true);
    QApplication app(argc, argv);
    PartsInteraction::install(app);
    PartsIntegrationTest test;
    const int result = QTest::qExec(&test, argc, argv);
    BrowserProfileManager::shutdown();
    return result;
}
#include "tst_partsintegration.moc"
