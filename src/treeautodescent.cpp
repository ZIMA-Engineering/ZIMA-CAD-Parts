#include <QDebug>

#include "treeautodescent.h"
#include "serversmodel.h"

TreeAutoDescent::TreeAutoDescent(ServersModel *sm, Item *root, QString path, QObject *parent) :
	QObject(parent),
	m_sm(sm),
	m_root(root),
	m_path(path),
	m_done(0)
{
	m_pathParts = m_path.split("/");

	if(m_pathParts.last().isEmpty())
		m_pathParts.removeLast();
}

bool TreeAutoDescent::waitsFor(Item *item) const
{
	return m_currentItem == item;
}

QString TreeAutoDescent::path()
{
	return m_path;
}

void TreeAutoDescent::descend()
{
	qDebug() << "Descending to" << m_path;

	if(m_pathParts.isEmpty())
		return;

	qDebug() << "Descent path" << m_pathParts;

	QString dsName = m_pathParts.takeFirst();
	bool found = false;

	foreach(Item *dsItem, m_root->children)
	{
		if(dsName == dsItem->server->name())
		{
			found = true;
			m_ds = dsItem->server;

			qDebug() << "DataSource" << m_ds->label << "matches." << m_pathParts;

			if(m_pathParts.isEmpty())
			{
				emit completed(this, dsItem);
				return;
			}

			m_currentItem = dsItem;
			emit progress(this, dsItem);

			continueDescent();

			break;
		}
	}

	if(!found)
	{
		qDebug() << "Path not found" << m_path;
		emit notFound(this);
	}
}

void TreeAutoDescent::continueDescent(bool loaded)
{
	qDebug() << "Continuing descent" << m_currentItem->name;

	if(m_done)
	{
		emit completed(this, m_done);
		return;

	} else if(!loaded)
	{
		qDebug() << "Schedule load immediately" << m_currentItem->name;
		m_sm->loadItem(m_currentItem);
		return;

	} else if(m_currentItem->children.isEmpty()) {
		qDebug() << "Path not found (no children)" << m_path;
		emit progress(this, m_currentItem);
		emit notFound(this);
		return;

	} else if(m_pathParts.isEmpty()) {
		qDebug() << "Path not found (pathParts empty)" << m_path;
		emit notFound(this);

	} else if(m_pathParts.first().isEmpty()) {
		do {
			if(m_pathParts.first().isEmpty())
				m_pathParts.removeFirst();
			else break;
		} while(!m_pathParts.isEmpty());

		if(m_pathParts.isEmpty())
		{
			qDebug() << "Path not found" << m_path;
			emit notFound(this);
		}

	} else {
		bool found = false;

		foreach(Item *child, m_currentItem->children)
		{
			if(child->name == m_pathParts.first())
			{
				found = true;
				m_pathParts.removeFirst();

				emit progress(this, child);

				qDebug() << "child" << child->name << "found" << m_pathParts;

				if(m_pathParts.isEmpty())
				{
					qDebug() << "Descent completed";
					m_done = child;
					m_sm->loadItem(child);
					return;

				} else {
					m_currentItem = child;

					qDebug() << "Schedule load" << child->name;
					m_sm->loadItem(child);
					return;
				}
			}
		}

		if(!found)
		{
			qDebug() << "Path not found" << m_path;
			qDebug() << m_pathParts.first() << "not found, exiting search";

			emit progress(this, m_currentItem);
			emit notFound(this);
		}
	}
}
