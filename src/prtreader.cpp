#include "prtreader.h"
#include "file.h"
#include "metadata.h"

#include <QRegularExpression>
#include <QDebug>

PtrReaderThread::PtrReaderThread(QFileInfoList partList) :
	m_partList(partList)
{

}

void PtrReaderThread::run()
{
	QList<int> fileTypes;
	fileTypes << FileType::ASM
			  << FileType::DRW
			  << FileType::PRT_PROE;

	foreach (const QFileInfo &fi, m_partList)
	{
		if (isInterruptionRequested())
			return;

		FileMetadata fm(fi);

		if (!fileTypes.contains(fm.type))
			continue;

		qDebug() << "Parsing prt file" << fi.absoluteFilePath();
		parseFile(fi);
	}
}

void PtrReaderThread::parseFile(const QFileInfo &fi)
{
	QFile f(fi.absoluteFilePath());
	f.open(QIODevice::ReadOnly);
	QTextStream s(&f);
	s.setCodec("UTF-8"); // just guessing here... but it works somehow

	while (!s.atEnd())
	{
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

bool PrtReader::isRunning() const
{
	return m_thread != nullptr;
}

void PrtReader::load(const QString &dir, const QFileInfoList &partList)
{
	if (isRunning())
		stop();

	m_dir = dir;
	m_thread = new PtrReaderThread(partList);
	connect(m_thread, SIGNAL(partParam(QString,QString,QString)),
			this, SLOT(setPartParam(QString,QString,QString)));
	m_thread->start();
}

void PrtReader::stop()
{
	if (isRunning())
	{
		disconnect(m_thread, SIGNAL(partParam(QString,QString,QString)),
				   this, SLOT(setPartParam(QString,QString,QString)));

		m_thread->requestInterruption();
		m_thread->wait();
		m_thread->deleteLater();
		m_thread = nullptr;
	}
}

void PrtReader::setPartParam(const QString &part, const QString &param, const QString &value)
{
	Metadata *m = MetadataCache::get()->metadata(m_dir);

	if (m->parameterHandles().contains(param))
		m->setPartParam(part, param, value);
}
