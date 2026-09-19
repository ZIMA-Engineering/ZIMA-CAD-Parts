#include "prtreader.h"
#include "file.h"
#include "metadata.h"
#include "localfilters.h"
#include "settings.h"

#include <QRegularExpression>
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QStringConverter>

PtrReaderThread::PtrReaderThread(QFileInfoList partList, QString language) :
    m_partList(partList), m_language(language.section('_', 0, 0).toLower())
{

}

void PtrReaderThread::run()
{
    QList<int> fileTypes;
    fileTypes << FileType::PROE_ASM
              << FileType::PROE_DRW
              << FileType::PROE_PRT;

    // Read the highest numeric revision independently of visible filters.
    LocalFilters versions;
    versions.showVersions = false;
    versions.showZimaVersions = false;
    const auto accepted = versions.accepted(m_partList, false);
    // A current native ZIMA document wins over a same-named Pro/E export.
    for (const bool native : {false, true})
    for (int row = 0; row < m_partList.size(); ++row)
    {
        if (!accepted.testBit(row))
            continue;
        const QFileInfo &fi = m_partList.at(row);
        if (isInterruptionRequested())
            return;

        FileMetadata fm(fi);

        const bool zima = fm.type == FileType::ZIMA_PRT || fm.type == FileType::ZIMA_ASM;
        if (native != zima || (!zima && !fileTypes.contains(fm.type)))
            continue;

        if (zima)
            parseZimaFile(fi);
        else
            parseFile(fi);
    }
}

void PtrReaderThread::parseZimaFile(const QFileInfo &fi)
{
    QFile file(fi.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;
    // Native CAD metadata is literal UTF-8 INI, not QSettings value syntax:
    // commas, quotes, backslashes and @ prefixes must survive unchanged.
    QMap<QString, QMap<QString, QString>> sections;
    QString section;
    const QSet<QString> wanted{"Document", "UserParameters", "UserParameterLabels", "UserParameterValues"};
    while (!file.atEnd())
    {
        if (isInterruptionRequested())
            return;
        QByteArray bytes = file.readLine(128 * 1024 + 1);
        if (bytes.isEmpty() && file.error() != QFileDevice::NoError)
            return;
        if (!bytes.endsWith('\n') && !file.atEnd())
        {
            // Geometry/cache records can be huge. Skip them with bounded memory.
            do {
                if (isInterruptionRequested()) return;
                bytes = file.readLine(128 * 1024 + 1);
                if (bytes.isEmpty() && file.error() != QFileDevice::NoError) return;
            } while (!bytes.endsWith('\n') && !file.atEnd());
            if (wanted.contains(section)) return;
            continue;
        }
        QStringDecoder decoder(QStringDecoder::Utf8);
        const QString decoded = decoder.decode(bytes);
        const QString line = decoded.trimmed();
        if (line.isEmpty() || line.startsWith(';') || line.startsWith('#')) continue;
        if (line.startsWith('[') && line.endsWith(']'))
        {
            section = line.mid(1, line.size() - 2).trimmed();
            continue;
        }
        if (!wanted.contains(section)) continue;
        const auto split = line.indexOf('=');
        if (decoder.hasError() || split <= 0) return;
        auto &values = sections[section];
        if (values.size() >= 4096 * 128) return;
        values.insert(line.left(split).trimmed(), line.mid(split + 1).trimmed());
    }
    const QString expected = FileMetadata(fi).type == FileType::ZIMA_PRT ? "part" : "assembly";
    if (sections.value("Document").value("type") != expected) return;
    const QStringList order = sections.value("UserParameters").value("Order").split(',', Qt::SkipEmptyParts);
    if (order.size() > 4096) return;
    QMap<QString, QString> parameters, aliases;
    QSet<QString> ambiguous;
    const auto &stored = sections["UserParameterValues"];
    const auto &labels = sections["UserParameterLabels"];
    for (const QString &entry : order)
    {
        const QString key = entry.trimmed();
        const QString localized = key + '\\' + m_language;
        const QString value = stored.contains(localized) ? stored.value(localized) : stored.value(key);
        if (key.isEmpty() || value.isEmpty()) continue;
        parameters.insert(key.toLower(), value);
        for (auto label = labels.lowerBound(key); label != labels.cend(); ++label)
        {
            if (label.key() != key && !label.key().startsWith(key + '\\')) break;
            const QString alias = label.value().trimmed().toLower();
            if (alias.isEmpty()) continue;
            if (aliases.contains(alias) && aliases.value(alias) != key) ambiguous.insert(alias);
            aliases.insert(alias, key);
        }
    }
    for (auto alias = aliases.cbegin(); alias != aliases.cend(); ++alias)
        if (!ambiguous.contains(alias.key()) && !parameters.contains(alias.key()))
            parameters.insert(alias.key(), parameters.value(alias.value().toLower()));
    for (auto value = parameters.cbegin(); value != parameters.cend(); ++value)
    {
        if (isInterruptionRequested()) return;
        emit partParam(fi.fileName(), value.key(), value.value());
    }
}

void PtrReaderThread::parseFile(const QFileInfo &fi)
{
    QFile f(fi.absoluteFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return;
    QTextStream s(&f);

    while (!s.atEnd())
    {
        if (isInterruptionRequested())
            return;
        QString line = s.readLine();

        if (!line.startsWith("description") || line.startsWith("descriptions"))
            continue;

        // some all-used separator or whatever. It seems it does not have any meaning
        line = line.replace("\xEF\xBF\xBD", "");
        // another all-arround used value
        line = line.replace("\x00", "");
        // then it seems like key and value is separated by "\x15"
        QStringList l = line.split("\x15");
        //qDebug() << l;
        QStringListIterator it(l);
        while (it.hasNext())
        {
            QString s = it.next();
            // \r is another strange char. It seems it used in all user defined attributes
            s = s.replace(QRegularExpression("^.+\\r"), "");
            QStringList vals = s.split("'");
            if (vals.size() != 2)
            {
                qWarning() << "attribute unexpected:" << s << vals << "it needs to be split";
                continue;
            }
            // user defined attributes are uppercased ASCII chars only
            QString key = vals[0].replace(QRegularExpression("[^A-Z]"), "");
            if (key.isEmpty())
            {
                qDebug() << "key is empty, skipping:" << s;
                continue;
            } else {
                qDebug() << "found key:" << key;
            }

            QString val = vals[1].split("\x14")[0];
            val.chop(1);
            val.remove(0,2);
            qDebug() << "    value:" << val << (val.isEmpty() ? "skipping" : "will be used") << "; original:" << vals[1];

            // try to find metadata.ini index
            QString fname = fi.fileName();
            QString key1 = key.toLower();

            if (!val.isEmpty())
                emit partParam(fname, key1, val);
        }

        break;
    }
}

PrtReader::PrtReader(QObject *parent)
    : QObject(parent),
      m_thread(nullptr)
{

}

PrtReader::~PrtReader()
{
    stop();
}

bool PrtReader::isRunning() const
{
    return m_thread != nullptr;
}

void PrtReader::load(const QString &dir, const QFileInfoList &partList)
{
    if (isRunning())
        stop();

    m_dir = dir;
    m_thread = new PtrReaderThread(partList, Settings::get()->LanguageMetadata);
    auto thread = m_thread.data();
    connect(thread, &PtrReaderThread::partParam, this,
            [this, thread](const QString &part, const QString &param, const QString &value) {
        if (m_thread == thread)
            setPartParam(part, param, value);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void PrtReader::stop()
{
    if (isRunning())
    {
        disconnect(m_thread, nullptr, this, nullptr);
        m_thread->requestInterruption();
        m_thread = nullptr;
    }
}

void PrtReader::setPartParam(const QString &part, const QString &param, const QString &value)
{
    Metadata *m = MetadataCache::get()->metadata(m_dir);

    if (m->parameterHandles().contains(param) && !value.isEmpty()) {
        const QFileInfo file(QDir(m_dir).filePath(part));
        m->setPartParam(File::partBaseName(file), param, value);
        emit loaded(file);
    }
}
