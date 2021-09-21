#include "metadatav2migration.h"

bool MetadataV2Migration::migrate()
{
	// First check if we're on the correct version, but the version
	// tag is simply missing
	if (hasGroup("Directory"))
	{
		m_settings->setValue("Directory/Version", 2);
		return true;
	}


	/*** Gather data from the old metadata ***/
	QRegExp colRx("^\\d+$");
	QStringList langs;
	langs << "cs" << "en" << "de" << "ru";
	QHash<QString, QString> dirLabels;
	QHash<QString, QHash<int, QString>> columns;
	QList<int> columnNumbers;
	QStringList includeDataPaths;
	QStringList includeThumbPaths;
	QHash<QString, QHash<int, QVariant>> directPartData;
	QHash<QString, QHash<QString, QHash<int, QVariant>>> localizedPartData;

	if (hasGroup("params"))
	{
		m_settings->beginGroup("params");

		// Try to find directory labels
		foreach (const QString &lang, langs)
		{
			QString label = m_settings->value(QString("%1/label").arg(lang)).toString();

			if (!label.isEmpty())
				dirLabels.insert(lang, label);
		}

		// Column labels
		foreach (const QString &lang, langs)
		{
			m_settings->beginGroup(lang);

			QHash<int, QString> colLabels;

			foreach (const QString &col, m_settings->childKeys())
			{
				if (!colRx.exactMatch(col))
					continue;

				int colNum = col.toInt();

				if (!columnNumbers.contains(colNum))
					columnNumbers << colNum;

				QString label = m_settings->value(col).toString();

				if (label.isEmpty())
					continue;

				colLabels.insert(colNum, label);
			}

			if (!colLabels.isEmpty())
				columns.insert(lang, colLabels);

			m_settings->endGroup();
		}

		m_settings->endGroup();
	}

	// Includes
	if (hasGroup("include"))
	{
		includeDataPaths = m_settings->value("include/data", QStringList()).toStringList();
		includeThumbPaths = m_settings->value("include/thumbnails", QStringList()).toStringList();
	}

	// Part data
	foreach (const QString &group, m_settings->childGroups())
	{
		if (group == "params" || group == "include")
			continue;

		m_settings->beginGroup(group);

		// Direct data
		QHash<int, QVariant> directColData;

		foreach (const QString &col, m_settings->childKeys())
		{
			if (!colRx.exactMatch(col))
				continue;

			QVariant v = m_settings->value(col);

			if (v.isNull())
				continue;

			directColData.insert(col.toInt(), v);
		}

		if (!directColData.isEmpty())
			directPartData.insert(group, directColData);

		// Localized data
		QHash<QString, QHash<int, QVariant>> partData;

		foreach (const QString &lang, langs)
		{
			QHash<int, QVariant> data;

			foreach (int col, columnNumbers)
			{
				QVariant v = m_settings->value(QString("%1/%2").arg(lang).arg(col));

				if (v.isNull())
					continue;

				data.insert(col, v);
			}

			if (!data.isEmpty())
				partData.insert(lang, data);
		}

		if (!partData.isEmpty())
			localizedPartData.insert(group, partData);

		m_settings->endGroup();
	}


	/*** Remove old metadata ***/
	foreach (const QString &group, m_settings->childGroups())
		m_settings->remove(group);


	/*** Write new ***/
	m_settings->setValue("Directory/Version", 2);

	// Directory labels
	QHashIterator<QString, QString> dirLabelsIterator(dirLabels);

	while (dirLabelsIterator.hasNext())
	{
		dirLabelsIterator.next();

		m_settings->setValue(
			QString("Directory/Label/%1").arg(dirLabelsIterator.key()),
			dirLabelsIterator.value()
		);
	}

	// Directory parameters
	QStringList paramHandles;

	std::sort(columnNumbers.begin(), columnNumbers.end());

	foreach (int col, columnNumbers)
		paramHandles << paramHandle(col);

	m_settings->setValue("Directory/Parameters", paramHandles);

	QHashIterator<QString, QHash<int, QString>> columnIterator(columns);

	while (columnIterator.hasNext())
	{
		columnIterator.next();

		QString lang = columnIterator.key();

		QHashIterator<int, QString> labelIterator(columnIterator.value());

		while (labelIterator.hasNext())
		{
			labelIterator.next();

			QString handle = paramHandle(labelIterator.key());

			m_settings->setValue(
				QString("Parameters/%1/Label/%2").arg(handle).arg(lang),
				labelIterator.value()
			);
		}
	}

	// Includes
	if (!includeDataPaths.isEmpty())
		m_settings->setValue("Directory/IncludeParameters", includeDataPaths);

	if (!includeThumbPaths.isEmpty())
		m_settings->setValue("Directory/IncludeThumbnails", includeThumbPaths);

	// Direct part data
	QHashIterator<QString, QHash<int, QVariant>> directPartdataIterator(directPartData);

	while (directPartdataIterator.hasNext())
	{
		directPartdataIterator.next();

		QString part = directPartdataIterator.key();

		QHashIterator<int, QVariant> colDataIterator(directPartdataIterator.value());

		while (colDataIterator.hasNext())
		{
			colDataIterator.next();

			QString param = paramHandle(colDataIterator.key());

			m_settings->setValue(
				QString("Parts/%1/%2").arg(part).arg(param),
				colDataIterator.value()
			);
		}
	}

	// Localized part data
	QHashIterator<QString, QHash<QString, QHash<int, QVariant>>> localizedPartDataIterator(localizedPartData);

	while (localizedPartDataIterator.hasNext())
	{
		localizedPartDataIterator.next();

		QString part = localizedPartDataIterator.key();
		QHashIterator<QString, QHash<int, QVariant>> partDataIterator(localizedPartDataIterator.value());

		while (partDataIterator.hasNext())
		{
			partDataIterator.next();

			QString lang = partDataIterator.key();

			QHashIterator<int, QVariant> colDataIterator(partDataIterator.value());

			while (colDataIterator.hasNext())
			{
				colDataIterator.next();

				QString param = paramHandle(colDataIterator.key());

				if (colDataIterator.value().toString().trimmed().isEmpty())
					continue;

				m_settings->setValue(
					QString("Parts/%1/%2/%3").arg(part).arg(param).arg(lang),
					colDataIterator.value()
				);
			}
		}
	}

	return true;
}

QString MetadataV2Migration::paramHandle(int col)
{
	return QString("param%1").arg(col, 2, 10, QLatin1Char('0'));
}
