#include "installationclient.h"
#include "../zima-cad-parts.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QUuid>
#include <memory>

QString partsUpdateExecutable()
{
    return QCoreApplication::applicationDirPath() + "/ZIMA-CAD-Parts-update"
#ifdef Q_OS_WIN
        ".exe"
#endif
        ;
}
QString partsInstallationRoot()
{
    QDir path(QCoreApplication::applicationDirPath());
#ifndef Q_OS_WIN
    if (path.dirName() == "bin") path.cdUp();
#endif
    if (!QRegularExpression("^[0-9]{10}$").match(path.dirName()).hasMatch()) return {};
    path.cdUp();
#ifdef Q_OS_WIN
    if (path.dirName() != "windows") return {};
#else
    if (path.dirName() != "linux") return {};
#endif
    path.cdUp();
    QFile file(path.filePath("installation.json"));
    if (!file.open(QIODevice::ReadOnly) || file.size() > 16384) return {};
    const auto marker = QJsonDocument::fromJson(file.readAll()).object();
    if (marker["product"] != "ZIMA-CAD-Parts" || marker["protocol"].toInt() != 1 || QFileInfo::exists(path.filePath(".git"))) return {};
    return path.canonicalPath();
}
bool registerPartsInstance(QString *error)
{
    static std::unique_ptr<QLockFile> instance;
    const auto root = partsInstallationRoot();
    if (root.isEmpty()) return true;
    const auto directory = root + "/.updates/instances";
    for (const auto &name : {root + "/.updates", directory}) {
        const QFileInfo info(name);
        if (info.isSymLink() || info.isJunction()) { *error = "Linked installation state"; return false; }
    }
    if (!QDir().mkpath(directory)) { *error = "Cannot register application instance"; return false; }
    QLockFile gate(root + "/.updates/install.lock"); gate.setStaleLockTime(0);
    const bool candidate = !qEnvironmentVariable("ZCP_UPDATE_SOCKET").isEmpty()
        && !qEnvironmentVariable("ZCP_UPDATE_TOKEN").isEmpty();
    if (!candidate && !gate.tryLock(5000)) { *error = "An update is being activated. Try again shortly."; return false; }
    instance = std::make_unique<QLockFile>(directory + '/' + VERSION + '-' + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".lock");
    instance->setStaleLockTime(0);
    if (!instance->tryLock()) { *error = "Cannot lock application registration"; return false; }
    return true;
}
