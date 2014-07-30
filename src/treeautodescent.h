#ifndef TREEAUTODESCENT_H
#define TREEAUTODESCENT_H

#include <QObject>

#include "item.h"

class TreeAutoDescent : public QObject
{
	Q_OBJECT
public:
	explicit TreeAutoDescent(ServersModel *sm, Item *root, QString path, QObject *parent = 0);
	bool waitsFor(Item *item) const;
	QString path();

signals:
	void progress(TreeAutoDescent *descent, Item *item);
	void completed(TreeAutoDescent *descent, Item *item);
	void notFound(TreeAutoDescent *descent);

public slots:
	void descend();
    void continueDescent();

private:
	ServersModel *m_sm;
	Item *m_root;
	QString m_path;
	QStringList m_pathParts;
	Item *m_currentItem;
	BaseDataSource *m_ds;
	Item *m_done;

};

#endif // TREEAUTODESCENT_H
