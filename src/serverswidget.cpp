#include "serverswidget.h"
#include "zimautils.h"

#include <QSignalMapper>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>


ServersWidget::ServersWidget(QWidget *parent)
	: QToolBox(parent)
{
	setStyleSheet("icon-size: 16px;");
	m_signalMapper = new QSignalMapper(this);

	connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(spawnZimaUtilityOnDir(int)));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(this_currentChanged(int)));
}

#if 0
void ServersWidget::setModel(ServersModel *model)
{
	m_model = model;

	setup();
	// Note: here we expect that ServersModel will send only the reset as in 2014-03-08
	connect(m_model, SIGNAL(reset()), this, SLOT(setup()));
}
#endif

void ServersWidget::setDataSources(QList<BaseDataSource*> datasources)
{
	// firstly delete all stuff used. Remember "the reset"
	for (int i = 0; i < count(); ++i)
	{
		removeItem(i);
	}

    qDeleteAll(m_modelViews);
    m_modelViews.clear();

	// now setup all item==group again
	foreach(BaseDataSource *ds, datasources)
	{
		ServersModel *model = new ServersModel(ds, this);
		model->retranslateMetadata();
		connect(model, SIGNAL(loadingItem(Item*)),
		        this, SLOT(loadingItem(Item*)));
		connect(model, SIGNAL(allItemsLoaded()),
		        this, SLOT(allItemsLoaded()));
		connect(model, SIGNAL(partsIndexAlreadyExists(Item*)),
		        this, SIGNAL(partsIndexAlreadyExists(Item*)));
		connect(model, SIGNAL(techSpecsIndexAlreadyExists(Item*)),
		        this, SIGNAL(techSpecsIndexAlreadyExists(Item*)));
		connect(model, SIGNAL(itemLoaded(const QModelIndex&)),
		        this, SIGNAL(itemLoaded(const QModelIndex&)));
        connect(model, SIGNAL(errorOccured(QString)),
                this, SIGNAL(errorOccured(QString)));
        connect(model, SIGNAL(filesDownloaded()),
                this, SIGNAL(filesDownloaded()));


		QTreeView *view = new QTreeView(this);
		view->header()->close();

        m_modelViews[model] = view;

		view->setModel(model);
		view->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(view, SIGNAL(clicked(const QModelIndex&)),
		        model, SLOT(requestTechSpecs(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
		        model, SLOT(requestTechSpecs(const QModelIndex&)));
		connect(view, SIGNAL(clicked(const QModelIndex&)),
		        this, SIGNAL(clicked(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
		        this, SIGNAL(activated(const QModelIndex&)));

		connect(view, SIGNAL(customContextMenuRequested(QPoint)),
		        this, SLOT(dirTreeContextMenu(QPoint)));

		addItem(view, ds->dataSourceIcon(), ds->label);
	}
}

void ServersWidget::dirTreeContextMenu(QPoint point)
{
	QModelIndex i = currentIndex();

	if (!i.isValid())
		return;

	Item *it = static_cast<Item*>(i.internalPointer());

	if (it->server->dataSource != LOCAL)
		return;

	QMenu *menu = new QMenu(this);

	menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));
	menu->addSeparator();

	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PTC-Cleaner.png"), "Clean with ZIMA-PTC-Cleaner", m_signalMapper, SLOT(map())), ZimaUtils::ZimaPtcCleaner);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-CAD-Sync.png"), "Sync with ZIMA-CAD-Sync", m_signalMapper, SLOT(map())), ZimaUtils::ZimaCadSync);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PS2PDF.png"), "Convert postscript to PDF with ZIMA-PS2PDF", m_signalMapper, SLOT(map())), ZimaUtils::ZimaPs2Pdf);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-STEP-Edit.png"), "Edit step files with ZIMA-STEP-Edit", m_signalMapper, SLOT(map())), ZimaUtils::ZimaStepEdit);

	menu->exec(currentWidget()->mapToGlobal(point));
	menu->deleteLater();
}

void ServersWidget::spawnZimaUtilityOnDir(int i)
{
	QString label = ZimaUtils::labelForUtility(i);
	QStringList paths = ZimaUtils::paths();

	if (paths[i].isEmpty())
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Please first configure path to %1 executable.").arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	QString executable = paths[i];

	if (!QFile::exists(executable))
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Path '%1' to %2 executable does not exists!").arg(executable).arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	qDebug() << "Spawn" << label;

	QStringList args;

	args << static_cast<Item*>(currentIndex().internalPointer())->path;

	QProcess::startDetached(executable, args);
}

void ServersWidget::expand(const QModelIndex & index)
{
	qobject_cast<QTreeView*>(currentWidget())->expand(index);
}

QModelIndex ServersWidget::currentIndex()
{
	return qobject_cast<QTreeView*>(currentWidget())->currentIndex();
}

void ServersWidget::setCurrentIndex(const QModelIndex &index)
{
    Item *item = static_cast<Item*>(index.internalPointer());

    // try to find proper model for given item
    foreach (ServersModel* i, m_modelViews.keys())
    {
        //qDebug() << "ds" << i->dataSource() << "it" << item->server << (i->dataSource() == item->server);
        if (i->dataSource() == item->server)
        {
            setCurrentWidget(m_modelViews[i]);
            m_modelViews[i]->setCurrentIndex(index);
            return;
        }
    }

    qWarning() << "ServersWidget::setCurrentIndex proper ServersModel not found";
}

void ServersWidget::requestTechSpecs(Item *item)
{
    // try to find proper model for given item
    foreach (ServersModel* i, m_modelViews.keys())
    {
        //qDebug() << "ds" << i->dataSource() << "it" << item->server << (i->dataSource() == item->server);
        if (i->dataSource() == item->server)
        {
            i->requestTechSpecs(item);
            return;
        }
    }

    qWarning() << "ServersWidget::requestTechSpecs proper ServersModel not found";
}

void ServersWidget::this_currentChanged(int i)
{
	QTreeView *w = qobject_cast<QTreeView*>(widget(i));

	emit groupChanged(w->rootIndex());
}

void ServersWidget::loadingItem(Item *i)
{
	emit statusUpdated(tr("Loading %1...").arg(i->getLabel()));
}

void ServersWidget::allItemsLoaded()
{
	emit statusUpdated(tr("All items loaded."));
}
