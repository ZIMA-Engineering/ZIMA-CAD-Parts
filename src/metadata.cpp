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
#include <QHash>
#include <QDebug>

#include "metadata.h"
#include "settings.h"
#include "metadata/metadatamigrator.h"


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

QStringList MetadataCache::parameterHandles(const QString &path)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->parameterHandles();
}

QStringList MetadataCache::parameterLabels(const QString &path)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->parameterLabels();
}

QString MetadataCache::partParam(const QString &path, const QString &fname, const QString &param)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->partParam(fname, param);
}

QString MetadataCache::partParam(const QString &path, const QString &fname, int index)
{
	if (!m_map.contains(path))
		load(path);
	return m_map[path]->partParam(fname, index);
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
	m_settings = new QSettings(
		m_path + "/" + TECHSPEC_DIR + "/" + METADATA_FILE,
		QSettings::IniFormat
	);
	m_settings->setIniCodec("utf-8");

	int v = version();

	if (v > METADATA_VERSION)
	{
		qDebug() << "Metadata file" << m_settings->fileName();
		qDebug() << "Detected version" << v;
		qDebug() << "This program supports only version" << METADATA_VERSION;
		return;
	}

	if (v < METADATA_VERSION)
	{
		qDebug() << "Metadata file" << m_settings->fileName();
		qDebug() << "Detected version" << v;

		if (isEmpty())
		{
			qDebug() << "File is empty, tagging version";
			m_settings->setValue("Directory/Version", METADATA_VERSION);

		} else {
			qDebug() << "Upgrading to version" << METADATA_VERSION;

			MetadataMigrator migrator(m_settings);

			if (!migrator.migrate(v, METADATA_VERSION))
			{
				qDebug() << "Migration failed";
				return;
			}

			qDebug() << "Migration successful";
		}
	}

	setup();
}

Metadata::~Metadata()
{
	delete m_settings;

	m_parameterLabels.clear();
	m_versionsCache.clear();
}

QString Metadata::getLabel()
{
	if (!label.isEmpty())
		return label;

	return (label = getLabel(Settings::get()->LanguageMetadata));
}

QString Metadata::getLabel(const QString &lang)
{
	return m_settings->value(
		QString("Directory/Label/%1").arg(lang),
		QString()
	).toString();
}

void Metadata::setLabel(const QString &lang, const QString &newLabel)
{
	label.clear();

	m_settings->setValue(QString("Directory/Label/%1").arg(lang), newLabel);
}

QStringList Metadata::parameterHandles()
{
	if (m_includes.isEmpty())
		return m_settings->value(
			"Directory/Parameters", QStringList()
		).toStringList();

	QStringList ret;

	foreach(Metadata *include, m_includes)
			ret << include->parameterHandles();

	return ret;
}

void Metadata::setParameterHandles(const QStringList &handles)
{
	// TODO: check that we're not removing no handles...
	// what is allowed is reordering and adding of new parameters
	m_settings->setValue("Directory/Parameters", handles);
	m_parameterLabels.clear();
}

QStringList Metadata::parameterLabels()
{
	if (!m_parameterLabels.isEmpty())
		return m_parameterLabels;

	return (m_parameterLabels = parameterLabels(Settings::get()->LanguageMetadata));
}

QStringList Metadata::parameterLabels(const QString &lang)
{
	QStringList ret;

	foreach (const QString &param, parameterHandles())
	{
		ret << m_settings->value(
			QString("Parameters/%1/Label/%2").arg(param).arg(lang),
			QString()
		).toString();
	}

	if (!m_includes.isEmpty())
	{
		ret.clear();

		foreach(Metadata *include, m_includes)
			ret << include->parameterLabels(lang);
	}

	return ret;
}

QHash<QString, QString> Metadata::parametersWithLabels(const QString &lang)
{
	QHash<QString, QString> ret;

	foreach (const QString &param, parameterHandles())
	{
		ret.insert(param, m_settings->value(
			QString("Parameters/%1/Label/%2").arg(param).arg(lang),
			QString()
		).toString());
	}

	if (!m_includes.isEmpty())
	{
		ret.clear();

		foreach(Metadata *include, m_includes)
			ret.unite(include->parametersWithLabels(lang));
	}

	return ret;
}

void Metadata::setParameterLabel(const QString &param, const QString &lang, const QString &value)
{
	m_settings->setValue(QString("Parameters/%1/Label/%2").arg(param).arg(lang), value);
}

void Metadata::renameParameter(const QString &handle, const QString &newHandle)
{
	QStringList handles = parameterHandles();
	handles.replace(handles.indexOf(handle), newHandle);
	m_settings->setValue("Directory/Parameters", handles);

	m_settings->beginGroup("Parameters");
	{
		rename(handle, newHandle);
	}
	m_settings->endGroup();

	m_settings->beginGroup("Parts");
	{
		foreach (const QString &part, m_settings->childGroups())
		{
			m_settings->beginGroup(part);
			rename(handle, newHandle);
			m_settings->endGroup();
		}
	}
	m_settings->endGroup();

	m_parameterLabels.clear();
}

void Metadata::removeParameter(const QString &handle)
{
	QStringList params = parameterHandles();
	params.removeOne(handle);

	m_settings->setValue("Directory/Parameters", params);

	// Parameter settings
	m_settings->remove(QString("Parameters/%1").arg(handle));

	// Part data
	m_settings->beginGroup("Parts");
	{
		foreach (const QString &part, m_settings->childGroups())
		{
			m_settings->beginGroup(part);
			m_settings->remove(handle);
			m_settings->endGroup();
		}
	}
	m_settings->endGroup();

	m_parameterLabels.clear();
}

QString Metadata::partParam(const QString &partName, const QString &param)
{
	QString partGroup = partName.section('.', 0, 0);
	QString anyVal;
	QString val;

	m_settings->beginGroup("Parts");
	m_settings->beginGroup(partGroup);
	m_settings->beginGroup(param);
	{
		foreach (const QString &lang, m_settings->childKeys())
		{
			val = m_settings->value(lang).toString();

			if (!(val).isEmpty() && lang == Settings::get()->LanguageMetadata)
				break;

			if (anyVal.isEmpty())
				anyVal = val;
		}
	}
	m_settings->endGroup();
	m_settings->endGroup();
	m_settings->endGroup();

	if (!val.isEmpty())
		return val;

	QString ret;

	if (anyVal.isEmpty())
	{
		ret = m_settings->value(
			QString("Parts/%1/%2").arg(partGroup).arg(param),
			QString()
		).toString();

	} else {
		ret = anyVal;
	}

	if (ret.isEmpty())
	{
		foreach(Metadata *include, m_includes)
		{
			QString tmp = include->partParam(partName, param);

			if (!tmp.isEmpty())
				return tmp;
		}
	}

	return ret;
}

QString Metadata::partParam(const QString &partName, int index)
{
	return partParam(partName, parameterHandles()[index]);
}

void Metadata::setPartParam(const QString &partName, const QString &param, const QString &value)
{
    QString partGroup = partName.section('.', 0, 0);

	m_settings->beginGroup("Parts");
    m_settings->beginGroup(partGroup);

	QString key = QString("%1/%2")
		.arg(param)
		.arg(Settings::get()->LanguageMetadata);

	m_settings->setValue(key, value);

    m_settings->endGroup();
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

void Metadata::rename(const QString &oldName, const QString &newName)
{
	QHash<QString, QVariant> settings;

	if (m_settings->childKeys().contains(oldName))
	{
		m_settings->setValue(newName, m_settings->value(oldName));
		m_settings->remove(oldName);
		return;
	}

	m_settings->beginGroup(oldName);
	recursiveRename(newName, settings);
	m_settings->endGroup();

	QHashIterator<QString, QVariant> i(settings);

	while (i.hasNext())
	{
		i.next();
		m_settings->setValue(i.key(), i.value());
	}

	m_settings->remove(oldName);
}

void Metadata::recursiveRename(const QString &path, QHash<QString, QVariant> &settings)
{
	foreach (const QString &group, m_settings->childGroups())
	{
		m_settings->beginGroup(group);
		recursiveRename(path + "/" + group, settings);
		m_settings->endGroup();
	}

	foreach (const QString &key, m_settings->childKeys())
		settings.insert(path + "/" + key, m_settings->value(key));
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

	m_settings->remove(QString("Parameters/%1").arg(grp));
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

					if (parameterHandles().contains(key1))
						setPartParam(fname, key1, val);

                }
                break;
            }

        }

    }

	dia.setValue(fil.size());
}

void Metadata::setup()
{
	m_settings->beginGroup("Directory");
	{
		QStringList toInclude;
		QStringList data = buildIncludePaths(m_settings->value("IncludeParameters").toStringList());
		QStringList thumbs = buildIncludePaths(m_settings->value("IncludeThumbnails").toStringList());

		toInclude << data << thumbs;

		toInclude.removeDuplicates();

		foreach(QString path, toInclude)
			m_includes << new Metadata(path, this);
	}
	m_settings->endGroup();
}

int Metadata::version()
{
	return m_settings->value("Directory/Version", 1).toInt();
}

bool Metadata::isEmpty()
{
	return m_settings->childGroups().empty() && m_settings->allKeys().empty();
}
