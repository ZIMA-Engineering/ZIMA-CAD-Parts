#include "serverswidget.h"
#include "zimautils.h"
#include "utils.h"

#include <QSignalMapper>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>


ServersWidget::ServersWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    serversToolBox->setStyleSheet("icon-size: 16px;");
	m_signalMapper = new QSignalMapper(this);

    m_servers = Utils::loadDataSources();

	connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(spawnZimaUtilityOnDir(int)));
    connect(serversToolBox, SIGNAL(currentChanged(int)), stackedWidget, SLOT(setCurrentIndex(int)));
}

void ServersWidget::setDataSources(QList<BaseDataSource*> datasources)
{
	// firstly delete all stuff used. Remember "the reset"
    foreach (ServersWidgetMap* i, m_map)
    {
        serversToolBox->removeItem(i->index);
        stackedWidget->removeWidget(i->tab);
    }

    qDeleteAll(m_map);
    m_map.clear();

	// now setup all item==group again
    int ix = 0;
	foreach(BaseDataSource *ds, datasources)
	{
        ServersWidgetMap *mapItem = new ServersWidgetMap();
        mapItem->index = ix;
        ix++;

		ServersModel *model = new ServersModel(ds, this);
		model->retranslateMetadata();

        mapItem->model = model;

		connect(model, SIGNAL(loadingItem(Item*)),
		        this, SLOT(loadingItem(Item*)));
		connect(model, SIGNAL(allItemsLoaded()),
		        this, SLOT(allItemsLoaded()));
		connect(model, SIGNAL(techSpecsIndexAlreadyExists(Item*)),
		        this, SIGNAL(techSpecsIndexAlreadyExists(Item*)));

        connect(model, SIGNAL(errorOccured(QString)),
                this, SIGNAL(errorOccured(QString)));
        connect(model, SIGNAL(filesDownloaded()),
                this, SIGNAL(filesDownloaded()));
        connect(model, SIGNAL(fileDownloaded(File*)),
                this, SIGNAL(fileDownloaded(File*)));
        connect(model, SIGNAL(techSpecAvailable(QUrl)),
                this, SIGNAL(techSpecAvailable(QUrl)));
        connect(model, SIGNAL(autoDescentProgress(QModelIndex)),
                this, SIGNAL(autoDescentProgress(QModelIndex)));
        connect(model, SIGNAL(autoDescentCompleted(QModelIndex)),
                this, SIGNAL(autoDescentComplete(QModelIndex)));
        connect(model, SIGNAL(autoDescentNotFound()),
                this, SIGNAL(autoDescentNotFound()));

		QTreeView *view = new QTreeView(this);
		view->header()->close();
        serversToolBox->addItem(view, ds->dataSourceIcon(), ds->label);

        mapItem->view = view;

        ServerTabWidget *tab = new ServerTabWidget(model, this);
        mapItem->tab = tab;
        stackedWidget->addWidget(tab);

        m_map.append(mapItem);

		view->setModel(model);
		view->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(view, SIGNAL(clicked(const QModelIndex&)),
		        model, SLOT(requestTechSpecs(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
		        model, SLOT(requestTechSpecs(const QModelIndex&)));
		connect(view, SIGNAL(clicked(const QModelIndex&)),
                stackedWidget, SLOT(setPartsIndex(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
                stackedWidget, SLOT(setPartsIndex(const QModelIndex&)));

		connect(view, SIGNAL(customContextMenuRequested(QPoint)),
		        this, SLOT(dirTreeContextMenu(QPoint)));

	}
}

void ServersWidget::retranslateMetadata()
{
    foreach (ServersWidgetMap *i, m_map)
    {
        i->model->retranslateMetadata();
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

	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PTC-Cleaner.png"), "Clean with ZIMA-PTC-Cleaner", m_signalMapper, SLOT(map())), ZimaUtils::ZimaPtcCleaner);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-CAD-Sync.png"), "Sync with ZIMA-CAD-Sync", m_signalMapper, SLOT(map())), ZimaUtils::ZimaCadSync);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PS2PDF.png"), "Convert postscript to PDF with ZIMA-PS2PDF", m_signalMapper, SLOT(map())), ZimaUtils::ZimaPs2Pdf);
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-STEP-Edit.png"), "Edit step files with ZIMA-STEP-Edit", m_signalMapper, SLOT(map())), ZimaUtils::ZimaStepEdit);

    menu->exec(serversToolBox->currentWidget()->mapToGlobal(point));
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
    qobject_cast<QTreeView*>(serversToolBox->currentWidget())->expand(index);
}

QModelIndex ServersWidget::currentIndex()
{
    return qobject_cast<QTreeView*>(serversToolBox->currentWidget())->currentIndex();
}

void ServersWidget::loadingItem(Item *i)
{
	emit statusUpdated(tr("Loading %1...").arg(i->getLabel()));
}

void ServersWidget::allItemsLoaded()
{
	emit statusUpdated(tr("All items loaded."));
}
