#include <QDebug>

#include "treeautodescent.h"
#include "serversmodel.h"
#include "localdatasource.h"

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

	QString dsName = m_pathParts.takeFirst();
	bool found = false;

	foreach(Item *dsItem, m_root->children)
	{
		if(dsName == dsItem->server->name())
		{
			found = true;
			m_ds = dsItem->server;

			if(m_pathParts.isEmpty())
			{
				emit completed(this, dsItem);
				return;
			}

			m_currentItem = dsItem;
			emit progress(this, dsItem);

            if (m_currentItem->name == m_pathParts.first())
            {
                emit completed(this, m_currentItem);
                return;
            }
            else
            {
                continueDescent();
            }

            break;
		}
	}

	if(!found)
	{
		qDebug() << "Path not found" << m_path;
		emit notFound(this);
	}
}

void TreeAutoDescent::continueDescent()
{
    qDebug() << "Continuing descent" << m_currentItem->name << m_done;

    bool found = false;

    foreach(Item *child, m_currentItem->children)
    {
        if(child->name == m_pathParts.first())
        {
            found = true;
            m_pathParts.removeFirst();

            emit progress(this, child);

            if(m_pathParts.isEmpty())
            {
                qDebug() << "Descent completed";
                m_done = child;
                m_sm->loadItem(child);
                emit completed(this, m_done);
                return;
            } else {
                m_currentItem = child;

                qDebug() << "Schedule load" << child->name;
                m_sm->loadItem(child);
                continueDescent();
            }
        }
    }

    if(!found)
    {
        qDebug() << "Path not found" << m_path;
        qDebug() << m_pathParts.first() << "not found, exiting search";

        emit notFound(this);
    }

}
