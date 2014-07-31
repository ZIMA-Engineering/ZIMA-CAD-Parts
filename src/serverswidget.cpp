#include "serverswidget.h"
#include "zimautils.h"
#include "settings.h"

#include <QSignalMapper>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QtDebug>


ServersWidget::ServersWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	serversToolBox->setStyleSheet(NavBar::loadStyle(":/styles/office2003gray.css"));

	m_signalMapper = new QSignalMapper(this);

	splitter->setSizes(Settings::get()->ServersSplitterSizes);

	connect(m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(spawnZimaUtilityOnDir(QString)));
	connect(serversToolBox, SIGNAL(currentChanged(int)), stackedWidget, SLOT(setCurrentIndex(int)));
	connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));
}

void ServersWidget::splitterMoved(int, int)
{
	Settings::get()->ServersSplitterSizes = splitter->sizes();
}

void ServersWidget::settingsChanged()
{
	if (Settings::get()->DataSourcesNeedsUpdate)
	{
		Settings::get()->DataSourcesNeedsUpdate = false;

		// Datasources
		// firstly delete all stuff used. Remember "the reset"
		foreach (ServersWidgetMap* i, m_map)
		{
			serversToolBox->removePage(i->index);
			stackedWidget->removeWidget(i->tab);
			i->model->deleteLater();
		}

		qApp->processEvents();
		qDeleteAll(m_map);
		m_map.clear();

		// now setup all item==group again
		int ix = 0;
		foreach(BaseDataSource *ds, Settings::get()->DataSources)
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
#warning "TODO/FIXME: autoDescentNotFound for implicit 'set home' "
//            connect(model, SIGNAL(autoDescentNotFound()),
//                    this, SIGNAL(autoDescentNotFound()));

			QTreeView *view = new QTreeView(this);
			view->header()->close();
			//serversToolBox->addItem(view, ds->dataSourceIcon(), ds->label);
			//serversToolBox->addItem(view, ds->itemIcon(model->rootItem()), ds->label);
			serversToolBox->addPage(view, ds->label, model->rootItem()->logo.isNull()
			                        ? ds->itemIcon(model->rootItem())
			                        : model->rootItem()->logo
			                       );

			mapItem->view = view;

			ServerTabWidget *tab = new ServerTabWidget(model, this);
			mapItem->tab = tab;
			stackedWidget->addWidget(tab);

			m_map.append(mapItem);

			view->setModel(model);
			view->setContextMenuPolicy(Qt::CustomContextMenu);

			connect(tab, SIGNAL(changeSettings()),
			        this, SLOT(settingsChanged()));
			connect(view, SIGNAL(clicked(const QModelIndex&)),
			        model, SLOT(requestTechSpecs(const QModelIndex&)));
			connect(view, SIGNAL(activated(const QModelIndex&)),
			        model, SLOT(requestTechSpecs(const QModelIndex&)));

			connect(view, SIGNAL(clicked(const QModelIndex&)),
			        this, SIGNAL(clicked(QModelIndex)));
			// track history
			connect(view, SIGNAL(activated(const QModelIndex&)),
			        this, SIGNAL(activated(const QModelIndex&)));
			connect(view, SIGNAL(clicked(const QModelIndex&)),
			        tab, SLOT(setPartsIndex(const QModelIndex&)));
			connect(view, SIGNAL(activated(const QModelIndex&)),
			        tab, SLOT(setPartsIndex(const QModelIndex&)));

			connect(view, SIGNAL(customContextMenuRequested(QPoint)),
			        this, SLOT(dirTreeContextMenu(QPoint)));

			connect(model, SIGNAL(techSpecsIndexAlreadyExists(Item*)),
			        tab, SLOT(techSpecsIndexOverwrite(Item*)));
		}

		serversToolBox->setVisibleRows(m_map.size());

	} // Settings::get()->DataSourcesNeedsUpdate

	foreach (ServersWidgetMap* i, m_map)
	i->tab->settingsChanged();
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

	QMenu *menu = new QMenu(this);

	menu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open"), this, SLOT(indexOpenPath()));
    menu->addAction(QIcon(":/gfx/gohome.png"), tr("Set as working directory"), this, SLOT(setWorkingDirectory()));

	menu->addSeparator();

	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PTC-Cleaner.png"), tr("Clean with ZIMA-PTC-Cleaner"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaPtcCleaner));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-CAD-Sync.png"), tr("Sync with ZIMA-CAD-Sync"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaCadSync));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-PS2PDF.png"), tr("Convert postscript to PDF with ZIMA-PS2PDF"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaPs2Pdf));
	m_signalMapper->setMapping(menu->addAction(QIcon(":/gfx/external_programs/ZIMA-STEP-Edit.png"), tr("Edit step files with ZIMA-STEP-Edit"), m_signalMapper, SLOT(map())),
	                           ZimaUtils::internalNameForUtility(ZimaUtils::ZimaStepEdit));

	menu->exec(serversToolBox->currentWidget()->mapToGlobal(point));
	menu->deleteLater();
}

void ServersWidget::spawnZimaUtilityOnDir(const QString &label)
{
	QString executable = Settings::get()->ExternalPrograms[label];

	if (executable.isEmpty())
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Please first configure path to %1 executable.").arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

	if (!QFile::exists(executable))
	{
		QMessageBox::warning(this, tr("Configure %1").arg(label), tr("Path '%1' to %2 executable does not exists!").arg(executable).arg(label));
		emit showSettings(SettingsDialog::ExternalPrograms);
		return;
	}

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

void ServersWidget::setModelindex(const QModelIndex &index)
{
	Item *item = static_cast<Item*>(index.internalPointer());

	foreach (ServersWidgetMap* i, m_map)
	{
		// just find appropriate objects for this item
		if (i->model->dataSource() != item->server)
			continue;

		serversToolBox->setCurrentIndex(i->index);
		stackedWidget->setCurrentIndex(i->index);
		i->model->requestTechSpecs(item);
		i->view->setCurrentIndex(index);
		i->tab->setPartsIndex(index);
	}

}

void ServersWidget::goToWorkingDirectory()
{
	foreach (ServersWidgetMap* i, m_map)
	{
        qDebug() << "gotow" << i->model << i->index;
		i->model->descentTo(Settings::get()->WorkingDir);
	}
}

void ServersWidget::retranslateMetadata(int langIndex)
{
	Settings::get()->setCurrentLanguageCode(Settings::get()->Languages[langIndex]);
	foreach (ServersWidgetMap* i, m_map)
	{
		i->model->retranslateMetadata();
	}
}

void ServersWidget::setWorkingDirectory()
{
	QTreeView *view = m_map[serversToolBox->currentIndex()]->view;
	Item *it = static_cast<Item*>(view->currentIndex().internalPointer());
	if (!it)
		return;
    Settings::get()->WorkingDir = it->pathWithDataSource();
	emit workingDirChanged();
}

void ServersWidget::indexOpenPath()
{
	QModelIndex index = currentIndex();
	if (!index.isValid())
		return;

	Item *i = static_cast<Item*>(index.internalPointer());
	QString path = i->server->getPathForItem(i);// pathWithDataSource();

	// Warning: it opens local file, even for remote datasources
#warning it opens local file, even for remote datasources
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
