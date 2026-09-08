#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include "localfilters.h"
#include "thumbnailmanager.h"
#include "file.h"

class PartsPerformanceTest : public QObject
{
    Q_OBJECT
private:
    static QString touch(const QString &dir, const QString &name)
    {
        const QString path = QDir(dir).filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            qFatal("Cannot create fixture");
        file.write("test");
        return path;
    }
    static QString picture(const QString &dir, const QString &name, const QColor &color)
    {
        const QString path = QDir(dir).filePath(name);
        QImage image(800, 400, QImage::Format_RGB32);
        image.fill(color);
        if (!image.save(path))
            qFatal("Cannot save fixture");
        return path;
    }
private slots:
    void exclusionsAndNumericVersions()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QDir(dir.path()).mkpath("0000-index");
        const QStringList names = {"odd.unknown", ".hidden", "dil.prt.2", "dil.prt.10",
            "dil.prt.9", "note.BAK", "readme", "name.with.dots.asm.1", "name.with.dots.asm.11"};
        QFileInfoList files;
        for (const auto &name : names)
            files.append(QFileInfo(touch(dir.path(), name)));
        files.append(QFileInfo(dir.filePath("0000-index")));
        LocalFilters filters;
        filters.showVersions = false;
        filters.hidden = {"*.bak"};
        auto accepted = filters.accepted(files, true);
        QCOMPARE(accepted.count(true), 5);
        QVERIFY(accepted.testBit(0));
        QVERIFY(accepted.testBit(1));
        QVERIFY(!accepted.testBit(2));
        QVERIFY(accepted.testBit(3));
        QVERIFY(!accepted.testBit(4));
        QVERIFY(!accepted.testBit(5));
        QVERIFY(accepted.testBit(8));
        QVERIFY(!accepted.testBit(9));
        filters.hidden.append("dil.prt.10");
        accepted = filters.accepted(files, true);
        QVERIFY(!accepted.testBit(2) && !accepted.testBit(3) && !accepted.testBit(4));
        filters.showVersions = true;
        QCOMPARE(filters.accepted(files, true).count(true), 7);
    }
    void localSettingsDoNotInherit()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("0000-index");
        QDir(dir.path()).mkpath("child");
        {
            QSettings settings(LocalFilters::filePath(dir.path()), QSettings::IniFormat);
            settings.setValue("Filters/Hide", QStringList{"*.tmp"});
            settings.setValue("Filters/ShowVersions", false);
        }
        LocalFilters parent, child;
        parent.load(dir.path(), true);
        child.load(dir.filePath("child"), true);
        QCOMPARE(parent.hidden, QStringList{"*.tmp"});
        QVERIFY(!parent.showVersions);
        QVERIFY(child.hidden.isEmpty());
        QVERIFY(child.showVersions);
    }
    void dottedCadNamesUseTheCorrectThumbnail()
    {
        QTemporaryDir dir;
        picture(dir.path(), "part.a.png", Qt::red);
        picture(dir.path(), "part.b.png", Qt::blue);
        ThumbnailManager manager;
        manager.setPath(dir.path(), 40);
        for (const QString &extension : {"prt.2", "prtz.3", "asmz", "drwz", "frmz.1", "tblz.2"}) {
            const QFileInfo file(dir.filePath("part.a." + extension));
            QCOMPARE(File::partBaseName(file), QString("part.a"));
            QCOMPARE(manager.path(file), dir.filePath("part.a.png"));
        }
        const QFileInfo file(dir.filePath("part.b.prtz.2"));
        QSignalSpy ready(&manager, &ThumbnailManager::thumbnailReady);
        manager.thumbnail(file);
        QTRY_COMPARE(ready.size(), 1);
        QCOMPARE(manager.thumbnail(file).toImage().pixelColor(0, 0), QColor(Qt::blue));
    }

    void thumbnailsAreLazyAndCached()
    {
        QTemporaryDir dir;
        const auto red = picture(dir.path(), "red.png", Qt::red);
        picture(dir.path(), "unused.png", Qt::green);
        ThumbnailManager manager;
        QSignalSpy ready(&manager, &ThumbnailManager::thumbnailReady);
        manager.setPath(dir.path(), 40);
        QTest::qWait(30);
        QCOMPARE(ready.size(), 0);
        manager.thumbnail(QFileInfo(red));
        manager.thumbnail(QFileInfo(red));
        QTRY_COMPARE(ready.size(), 1);
        const auto thumbnail = manager.thumbnail(QFileInfo(red));
        QCOMPARE(thumbnail.size(), QSize(40, 20));
        QCOMPARE(thumbnail.toImage().pixelColor(0, 0), QColor(Qt::red));
        QTest::qWait(30);
        QCOMPARE(ready.size(), 1);
    }
    void navigationDropsQueuedAndStaleResults()
    {
        QTemporaryDir oldDir, newDir;
        const auto oldFile = picture(oldDir.path(), "same.png", Qt::red);
        const auto newFile = picture(newDir.path(), "same.png", Qt::blue);
        ThumbnailManager manager;
        QSignalSpy ready(&manager, &ThumbnailManager::thumbnailReady);
        manager.setPath(oldDir.path(), 32);
        manager.thumbnail(QFileInfo(oldFile));
        // Block GUI delivery while the old image completes: its queued result
        // must still be rejected after a directory switch.
        QTest::qSleep(50);
        QElapsedTimer timer;
        timer.start();
        manager.setPath(newDir.path(), 32);
        QVERIFY(timer.elapsed() < 100);
        manager.thumbnail(QFileInfo(newFile));
        QTRY_COMPARE(ready.size(), 1);
        QCOMPARE(ready.first().first().toString(), newFile);
        QCOMPARE(manager.thumbnail(QFileInfo(newFile)).toImage().pixelColor(0, 0), QColor(Qt::blue));

        ThumbnailWorker worker;
        worker.setContext(1, oldDir.path(), 32);
        for (int i = 0; i < 1000; ++i)
            worker.enqueue(1, QFileInfo(oldFile));
        worker.setContext(2, newDir.path(), 32);
        worker.enqueue(2, QFileInfo(newFile));
        QSignalSpy images(&worker, &ThumbnailWorker::imageReady);
        worker.start();
        QTRY_COMPARE(images.size(), 1);
        QCOMPARE(images.first().at(0).toULongLong(), qulonglong(2));
    }
    void includesAndOperationsBeforePreview()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("0000-index/thumbnails");
        QDir(dir.path()).mkpath("included/0000-index");
        const auto local = picture(dir.path(), "part.png", Qt::red);
        picture(dir.filePath("0000-index/thumbnails"), "part.jpg", Qt::blue);
        const auto included = picture(dir.filePath("included"), "other.png", Qt::green);
        {
            QSettings settings(dir.filePath("0000-index/metadata.ini"), QSettings::IniFormat);
            settings.setValue("Directory/IncludeThumbnails", QStringList{"included"});
            QSettings child(dir.filePath("included/0000-index/metadata.ini"), QSettings::IniFormat);
            child.setValue("Directory/IncludeThumbnails", QStringList{".."});
        }
        ThumbnailManager manager;
        manager.setPath(dir.path());
        QCOMPARE(manager.path(QFileInfo(dir.filePath("part.prt.1"))), local);
        QCOMPARE(manager.path(QFileInfo(dir.filePath("other.prt.1"))), included);
        QVERIFY(ThumbnailWorker::discover(dir.path(), [] { return true; }).isEmpty());
    }
    void missingAndCorruptImagesAreNotRetried()
    {
        QTemporaryDir dir;
        const auto bad = touch(dir.path(), "bad.png");
        ThumbnailManager manager;
        manager.setPath(dir.path());
        QSignalSpy ready(&manager, &ThumbnailManager::thumbnailReady);
        manager.thumbnail(QFileInfo(bad));
        QTRY_COMPARE(ready.size(), 1);
        QVERIFY(manager.thumbnail(QFileInfo(bad)).isNull());
        QTest::qWait(30);
        QCOMPARE(ready.size(), 1);
    }
    void largeDirectoryFilterPass()
    {
        QFileInfoList files;
        for (int i = 0; i < 20000; ++i)
            files.append(QFileInfo(QString("part%1.prt.%2").arg(i / 10).arg(i % 10 + 1)));
        LocalFilters filters;
        filters.showVersions = false;
        QElapsedTimer timer;
        timer.start();
        QCOMPARE(filters.accepted(files, false).count(true), 2000);
        qInfo() << "20,000 filenames, numeric version grouping:" << timer.elapsed() << "ms";
    }
    void cancellationKeepsCompletedThumbnails()
    {
        QTemporaryDir dir;
        const auto file = picture(dir.path(), "cached.png", Qt::red);
        ThumbnailManager manager;
        manager.setPath(dir.path(), 32);
        QSignalSpy ready(&manager, &ThumbnailManager::thumbnailReady);
        manager.thumbnail(QFileInfo(file));
        QTRY_COMPARE(ready.size(), 1);
        manager.cancelPending();
        QVERIFY(!manager.thumbnail(QFileInfo(file)).isNull());
        QTest::qWait(30);
        QCOMPARE(ready.size(), 1);
        manager.clear();
        manager.thumbnail(QFileInfo(file));
        QTRY_COMPARE(ready.size(), 2);
    }

    void zimaArchivesAreIndependent()
    {
        QTemporaryDir dir;
        QFileInfoList files;
        for (const QString &ext : {"prtz", "asmz", "drwz", "frmz", "tblz"}) {
            for (const QString &suffix : {"", ".1", ".10", ".bak"})
                files.append(QFileInfo(touch(dir.path(), "item." + ext + suffix)));
        }
        files.append(QFileInfo(touch(dir.path(), "orphan.prtz.7")));
        files.append(QFileInfo(touch(dir.path(), "proe.prt.1")));
        files.append(QFileInfo(touch(dir.path(), "proe.prt.2")));
        LocalFilters filters;
        filters.showZimaVersions = false;
        QCOMPARE(filters.accepted(files, false).count(true), 12);
        filters.showVersions = false;
        QCOMPARE(filters.accepted(files, false).count(true), 11);
        filters.showZimaVersions = true;
        QCOMPARE(filters.accepted(files, false).count(true), 22);
        QCOMPARE(FileMetadata("item.prtz").type, FileType::ZIMA_PRT);
        QCOMPARE(FileMetadata("item.prtz.10").type, FileType::ZIMA_PRT);
        QCOMPARE(FileMetadata("item.ASMZ.2").type, FileType::ZIMA_ASM);
        QCOMPARE(FileMetadata("item.drwz").type, FileType::ZIMA_DRW);
        QCOMPARE(FileMetadata("item.frmz.1").type, FileType::ZIMA_FORMAT);
        QCOMPARE(FileMetadata("item.tblz").type, FileType::ZIMA_TITLE_BLOCK);
        QCOMPARE(FileMetadata("item.prtz.bak").type, FileType::UNDEFINED);
        QCOMPARE(FileMetadata("itemXprtz.1").type, FileType::UNDEFINED);
        QDir(dir.path()).mkpath("0000-index");
        QDir(dir.path()).mkpath("child");
        {
            QSettings settings(LocalFilters::filePath(dir.path()), QSettings::IniFormat);
            settings.setValue("Filters/ShowZimaVersions", false);
        }
        filters.load(dir.path(), true);
        QVERIFY(!filters.showZimaVersions);
        filters.load(dir.filePath("child"), true);
        QVERIFY(filters.showZimaVersions);
    }

    void fileTypesStillRecognized()
    {
        QCOMPARE(FileMetadata("part.prt.12").type, FileType::PROE_PRT);
        QCOMPARE(FileMetadata("photo.JPG").type, FileType::FILE_IMAGE);
        QCOMPARE(FileMetadata("readme.unknown").type, FileType::UNDEFINED);
    }
};
QTEST_MAIN(PartsPerformanceTest)
#include "tst_partsperformance.moc"
