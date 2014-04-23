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
    connect(serversToolBox, SIGNAL(currentChanged(int)), this, SLOT(serversToolBox_currentChanged(int)));
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

        connect(model, SIGNAL(techSpecAvailable(QUrl)),
                this, SLOT(loadTechSpec(QUrl)));

		connect(model, SIGNAL(loadingItem(Item*)),
		        this, SLOT(loadingItem(Item*)));
		connect(model, SIGNAL(allItemsLoaded()),
		        this, SLOT(allItemsLoaded()));
		connect(model, SIGNAL(partsIndexAlreadyExists(Item*)),
                this, SIGNAL(partsIndexOverwrite(Item*)));
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
                tabWidget, SLOT(setPartsIndex(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
                this, SLOT(setPartsIndex(const QModelIndex&)));

		connect(view, SIGNAL(customContextMenuRequested(QPoint)),
		        this, SLOT(dirTreeContextMenu(QPoint)));

	}
}

void ServersWidget::retranslateMetadata()
{
    foreach (ServersModel *i, m_modelViews.keys())
    {
        i->retranslateMetadata();
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

void ServersWidget::serversToolBox_currentChanged(int i)
{
	QTreeView *w = qobject_cast<QTreeView*>(widget(i));

    //emit groupChanged(w->rootIndex());
    tabWidet->setPartsIndex(w->rootIndex());
}

void ServersWidget::loadingItem(Item *i)
{
	emit statusUpdated(tr("Loading %1...").arg(i->getLabel()));
}

void ServersWidget::allItemsLoaded()
{
	emit statusUpdated(tr("All items loaded."));
}

void ServersWidget::loadTechSpec(QUrl url)
{
    // it *must* be ServersModel
    ServersModel *model = qobject_cast<ServersModel*>(sender());
    Q_ASSERT(model);

    Item *it = model->lastTechSpecRequest();

    if(it)
        ui->techSpec->setRootPath(it->server->pathToDataRoot());

    ui->techSpec->load(url);
}

void ServersWidget::assignPartsIndexUrlToDirectory(bool overwrite)
{
//    Item *it = fm->getRootItem();

//    if (!it)
//        return;

//    QString lang = MainWindow::getCurrentMetadataLanguageCode().left(2);
//    it->server->assignPartsIndexUrlToItem(partsIndexUrlBar->text(), it, lang, overwrite);
}

void ServersWidget::partsIndexOverwrite(Item *item)
{
    if (QMessageBox::warning(this,
                             tr("Parts index already exists"),
                             tr("Parts index %1 already exists, would you like to overwrite it?").arg(item->getLabel()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        assignPartsIndexUrlToDirectory(true);
    }
}

void ServersWidget::setWorkingDirectory()
{
    QSettings settings;
    Item *it = static_cast<Item*>(ui->serversWidget->currentIndex().internalPointer());

    settings.setValue("HomeDir", it->pathWithDataSource());
    settings.setValue("WorkingDir", it->path);

    ui->techSpec->setDownloadDirectory(it->path);

    ui->editDir->setText(it->path);
}

