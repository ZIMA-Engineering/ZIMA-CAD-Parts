/*
  ZIMA-CAD-Parts
  http://www.zima-construction.cz/software/ZIMA-Parts

  Copyright (C) 2011-2012 Jakub Skokan <aither@havefun.cz>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QRegExp>
#include <QDir>
#include <QProgressDialog>
#include <QDebug>

#include "metadata.h"
#include "settings.h"


MetadataCache * MetadataCache::m_instance = 0;


MetadataCache * MetadataCache::get()
{
	if (!m_instance)
		m_instance = new MetadataCache();
	return m_instance;
}

MetadataCache::MetadataCache()
{
}

MetadataCache::~MetadataCache()
{
	clear();
	delete m_instance;
	m_instance = 0;
}

void MetadataCache::clear()
{
	qDeleteAll(m_map);
	m_map.clear();
	emit cleared();
}

void MetadataCache::clear(const QString &path)
{
	delete m_map.take(path);
	emit cleared();
}

void MetadataCache::load(const QString &path)
{
	if (m_map.contains(path))
		m_map[path]->deleteLater();
	m_map[path] = new Metadata(path);
}

bool MetadataCache::showLabel(const QString &path)
{
	return !QDir().exists(path + "/" + TECHSPEC_DIR + "/" + LOGO_FILE);
}

QString MetadataCache::label(const QString &path)
{
	// avoid creation of empty Metadata instances
	// in ServersModel::data
	if (!QDir().exists(path + "/" + TECHSPEC_DIR))
		return QString();

	if (!m_map.contains(path))
		load(path);
	return m_map[path]->getLabel();
}

QStringList MetadataCache::columnLabels(const QString &path)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->columnLabels();
}

QString MetadataCache::partParam(const QString &path, const QString &fname, int column)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->partParam(fname, column);
}

MetadataVersionsMap MetadataCache::partVersions(const QString &path)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->partVersions();
}

void MetadataCache::deletePart(const QString &path, const QString &part)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->deletePart(part);
}

Metadata* MetadataCache::metadata(const QString &path)
{
    if (!m_map.contains(path))
        load(path);
    return m_map[path];
}

Metadata::Metadata(const QString &path, QObject *parent)
	: QObject(parent),
	  m_path(path),
	  m_loadedIncludes(0)
{
//    qDebug() << "Metadata constructor for" << path;
	m_settings = new QSettings(m_path + "/" + TECHSPEC_DIR + "/" + METADATA_FILE, QSettings::IniFormat);
	m_settings->setIniCodec("utf-8");
//    qDebug() << "ini file" << m_settings->fileName();

	m_settings->beginGroup("include");
	{
		QStringList toInclude;
		QStringList data = buildIncludePaths(m_settings->value("data").toStringList());
		QStringList thumbs = buildIncludePaths(m_settings->value("thumbnails").toStringList());

		toInclude << data << thumbs;

		toInclude.removeDuplicates();

		foreach(QString path, toInclude)
        m_includes << new Metadata(path, this);
	}
	m_settings->endGroup();
}

Metadata::~Metadata()
{
	delete m_settings;

	m_columnLabels.clear();
	m_versionsCache.clear();
}

QString Metadata::getLabel()
{
	if(label.count())
		return label;

	return (label = getLabel(Settings::get()->LanguageMetadata));
}

QString Metadata::getLabel(const QString &lang)
{
	return m_settings->value(QString("params/%1/label").arg(lang), QString()).toString();
}

void Metadata::setLabel(const QString &lang, const QString &newLabel)
{
	label.clear();

	m_settings->setValue(QString("params/%1/label").arg(lang), newLabel);
}

QStringList Metadata::columnLabels()
{
	if(m_columnLabels.count())
		return m_columnLabels;

	m_columnLabels << tr("Part Name") << tr("Thumbnail");

	QRegExp colRx("^\\d+$");
	int colIndex = 1;

	m_settings->beginGroup("params");
	{
		m_settings->beginGroup(Settings::get()->LanguageMetadata);
		{
			foreach(QString col, m_settings->childKeys())
			{
				if(colRx.exactMatch(col))
				{
					m_columnLabels << m_settings->value( QString("%1").arg(colIndex++) ).toString();
				}
			}
		}
		m_settings->endGroup();
	}
	m_settings->endGroup();

    if (m_includes.size())
	{
		m_columnLabels.clear();
        foreach(Metadata *include, m_includes)
		m_columnLabels << include->columnLabels();
	}

	return m_columnLabels;
}

QString Metadata::partParam(const QString &partName, int col)
{
	QString partGroup = partName.section('.', 0, 0);
	QString anyVal;
	QString val;

	m_settings->beginGroup(partGroup);

	foreach(QString group, m_settings->childGroups())
	{
		if(!(val = m_settings->value(QString("%1/%2").arg(group).arg(col)).toString()).isEmpty() && group == Settings::get()->LanguageMetadata)
			break;

		if(anyVal.isEmpty())
			anyVal = val;
	}

	m_settings->endGroup();

	if(!val.isEmpty())
	{
		return val;
	}
	else {

		QString ret = anyVal.isEmpty() ? m_settings->value(QString("%1/%2").arg(partGroup).arg(col), QString()).toString() : anyVal;

		if(ret.isEmpty())
		{
            foreach(Metadata *include, m_includes)
			{
				QString tmp = include->partParam(partName, col);

				if(!tmp.isEmpty())
					return tmp;
			}
		}

		return ret;
	}
}

void Metadata::setPartParam(const QString &partName, int col, const QString &value)
{
    QString partGroup = partName.section('.', 0, 0);

    m_settings->beginGroup(partGroup);

    foreach (QString lang, Settings::get()->Languages)
    {
        QString key = QString("%1/%2").arg(lang.left(2)).arg(col);
        m_settings->setValue(key, value);
    }

    m_settings->endGroup();
}

bool Metadata::partVersionType(FileType::FileType t, const QFileInfo &fi)
{
	QRegExp re(File::getRxForFileType(t));
	re.setCaseSensitivity(Qt::CaseInsensitive);
	if (re.exactMatch(fi.fileName()))
	{
		m_versionsCache[fi.completeBaseName()] = fi.fileName();
		return true;
	}
	return false;
}

MetadataVersionsMap Metadata::partVersions()
{
	if (m_versionsCache.size())
		return m_versionsCache;

	QDir d(m_path);
	QStringList files = d.entryList(QStringList(),
	                                QDir::Files | QDir::Readable,
	                                QDir::Name);

	QFileInfo fi;
	foreach (QString i, files)
	{
		fi.setFile(i);
		foreach(FileType::FileType t, File::versionedTypes())
		{
			if (partVersionType(t, fi.fileName()))
				break;
		}
	}

	return m_versionsCache;
}

void Metadata::deletePart(const QString &part)
{
	QString grp = part.section('.', 0, 0);

	if(grp.isEmpty())
		return;

	m_settings->remove(grp);
}

QString Metadata::buildIncludePath(const QString &raw)
{
	if(raw.startsWith('/'))
		return QDir::cleanPath(raw);
	else
	{
		return QDir::cleanPath(m_path + "/" + raw);
	}
}

QStringList Metadata::buildIncludePaths(const QStringList &raw)
{
	QStringList ret;

    if (raw.count())
    {
        Q_FOREACH(QString s, raw)
        {
            qDebug() << s;
            ret << buildIncludePath(s);
        }
    }

	return ret;
}

QString unaccent_helper(const QString &str)
{
    QString diacriticLetters = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
    QStringList noDiacriticLetters;
    noDiacriticLetters << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";

    QString output = "";
    for (int i = 0; i < str.length(); i++) {
        QChar c = str[i];
        int dIndex = diacriticLetters.indexOf(c);
        if (dIndex < 0) {
            output.append(c);
        } else {
            QString replacement = noDiacriticLetters[dIndex];
            output.append(replacement);
        }
    }

    return output;
}

void Metadata::reloadProe(const QFileInfoList &fil)
{
    qDebug() << "reloadProe" << m_path;

    QString txt = tr("Loading ProE metadata...");
    QProgressDialog dia(txt, tr("Abort"), 0, fil.size());
    dia.setWindowModality(Qt::WindowModal);

    int ix = 1;
    foreach (QFileInfo i, fil)
    {
        dia.setValue(ix++);

        if (dia.wasCanceled())
            break;

        FileMetadata fm(i);
        if (fm.type != FileType::ASM && fm.type != FileType::DRW && fm.type != FileType::PRT_PROE)
        {
            continue;
        }
        qDebug() << "PROE import for file" << i.absoluteFilePath();
        dia.setLabelText(txt + "\n" + i.fileName());

        qDebug() << fm.type << File::getInternalNameForFileType(fm.type);

        QFile f(i.absoluteFilePath());
        f.open(QIODevice::ReadOnly);
        QTextStream s(&f);
        s.setCodec("UTF-8"); // just guessing here... but it works somehow

        while (!s.atEnd())
        {
            QString line = s.readLine();
            if (line.startsWith("description") && !line.startsWith("descriptions"))
            {
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
                    s = s.replace(QRegExp("^.+\\r"), "");
                    QStringList vals = s.split("'");
                    if (vals.size() != 2)
                    {
                        qWarning() << "attribute unexpected:" << s << vals << "it needs to be split";
                        continue;
                    }
                    // user defined attributes are uppercased ASCII chars only
                    QString key = vals[0].replace(QRegExp("[^A-Z]"), "");
                    if (key.isEmpty())
                    {
                        qDebug() << "key is empty, skipping:" << s;
                        continue;
                    }
                    else
                    {
                        qDebug() << "found key:" << key;
                    }

                    QString val = vals[1].split("\x14")[0];
                    val.chop(1);
                    val.remove(0,2);
                    qDebug() << "    value:" << val << (val.isEmpty() ? "skipping" : "will be used") << "; original:" << vals[1];

                    // try to find metadata.ini index
                    QString fname = i.fileName();
                    QString key1 = key.toLower();
                    qDebug() << columnLabels();
                    foreach (QString key2, columnLabels())
                    {
                        QString unac = unaccent_helper(key2).toLower();
                        qDebug() << "UNAC" << unac << key2 << key1;
                        if (unac == key1)
                        {
                            setPartParam(fname, columnLabels().indexOf(key2) -1, val);
                            break;
                        }
                    }
                }
                break;
            }

        }

    }

    dia.setValue(fil.size());
}
