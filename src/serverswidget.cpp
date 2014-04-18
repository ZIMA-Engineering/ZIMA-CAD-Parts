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

    connect(deleteQueueBtn, SIGNAL(clicked()), m_downloadModel, SLOT(clear()));

    m_productView = new ProductView(tabWidget);

    serversToolBox->setStyleSheet("icon-size: 16px;");
	m_signalMapper = new QSignalMapper(this);

    m_servers = Utils::loadDataSources();

    thumbnailSizeSlider->setValue(settings.value("GUI/ThumbWidth", 32).toInt());

    downloadTreeView->setModel(m_downloadModel);
    downloadTreeView->setItemDelegate(new DownloadDelegate(this));

	connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(spawnZimaUtilityOnDir(int)));
    connect(serversToolBox, SIGNAL(currentChanged(int)), this, SLOT(serversToolBox_currentChanged(int)));

    connect(btnDownload, SIGNAL(clicked()), this, SLOT(downloadButton()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateClicked()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteSelectedParts()));
    connect(startStopDownloadBtn, SIGNAL(clicked()), this, SLOT(toggleDownload()));

    connect(filtersButton, SIGNAL(clicked()), this, SLOT(setFiltersDialog()));

    connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)), fm, SLOT(setThumbWidth(int)));
    connect(ui->thumbnailSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(adjustThumbColumnWidth(int)));

    connect(ui->serversWidget, SIGNAL(fileDownloaded(File*)),
            m_productView, SLOT(fileDownloaded(File*)));
    connect(partsTreeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(previewInProductView(QModelIndex)));
    connect(partsTreeView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(previewInProductView(QModelIndex)));
    connect(partsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(tree_doubleClicked(QModelIndex)));
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
    Q_ASSERT(m_downloadModel);

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

        m_downloadModel->registerHandler(DownloadModel::ServersModel, model);
        model->setDownloadQueue(m_downloadModel);

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
                tabWidget, SLOT(setPartsIndex(const QModelIndex&)));
		connect(view, SIGNAL(activated(const QModelIndex&)),
                this, SLOT(setPartsIndex(const QModelIndex&)));

		connect(view, SIGNAL(customContextMenuRequested(QPoint)),
		        this, SLOT(dirTreeContextMenu(QPoint)));

		addItem(view, ds->dataSourceIcon(), ds->label);
	}
}

void ServersWidget::retranslateMetadata()
{
    foreach (ServersModel *i, m_modelViews.keys())
    {
        i->retranslateMetadata();
    }
}

void ServersWidget::refresh(Item *item)
{
    // try to find proper model for given item
    foreach (ServersModel* i, m_modelViews.keys())
    {
        if (i->dataSource() == item->server)
        {
            i->refresh(item);
            return;
        }
    }
}

void ServersWidget::deleteFiles()
{
    QTreeView *w = qobject_cast<QTreeView*>(currentWidget());
    Q_ASSERT(w);
    QList<ServersModel*> l = m_modelViews.keys(w);
    Q_ASSERT(l.size());
    l.at(0)->deleteFiles();
}

void ServersWidget::uncheckAll()
{
    QTreeView *w = qobject_cast<QTreeView*>(currentWidget());
    Q_ASSERT(w);
    QList<ServersModel*> l = m_modelViews.keys(w);
    Q_ASSERT(l.size());
    l.at(0)->uncheckAll();
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

void ServersWidget::downloadButton()
{
#warning "TODO/FIXME serversModel"
//	serversModel->downloadFiles( ui->editDir->text() );
    serversWidget->uncheckAll(); //TODO/FIXME: maps

    tabWidget->setCurrentIndex(MainWindow::DOWNLOADS);

    downloadTreeView->resizeColumnToContents(0);
    downloadTreeView->resizeColumnToContents(2);
}

void ServersWidget::updateClicked()
{
    Item* i = fm->getRootItem();

    if( i == 0)
        return;

    fm->prepareForUpdate();

    refresh(i);//TODO/FIXME: maps
    requestTechSpecs(i);//TODO/FIXME: maps
}

void ServersWidget::deleteSelectedParts()
{
    if( QMessageBox::question(this,
                              tr("Do you really want to delete selected parts?"),
                              tr("Do you really want to delete selected parts? This action is irreversible."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            ==  QMessageBox::Yes)
    {
        deleteFiles(); //TODO/FIXME: maps
        uncheckAll(); //TODO/FIXME: maps
    }
}

void ServersWidget::toggleDownload()
{
    if (m_downloadModel->isEmpty())
        return;

    if (m_downloadModel->isDownloading())
        stopDownload();
    else
        resumeDownload();
}

void ServersWidget::resumeDownload()
{
    downloadModel->resume();
    startStopDownloadBtn->setText(tr("Stop"));
}

void ServersWidget::stopDownload()
{
    downloadModel->stop();
    startStopDownloadBtn->setText(tr("Resume"));
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

void ServersWidget::adjustThumbColumnWidth(int width)
{
    partsTreeView->setColumnWidth(1, width);
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

void ServersWidget::previewInProductView(const QModelIndex &index)
{
    QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(partsTreeView->model())->mapToSource(index);

    File *f = fm->getRootItem()->files.at(srcIndex.row());

    if (!m_productView->canHandle(f->type))
    {
        m_productView->hide();
        return;
    }

    m_productView->expectFile(f);

    // local files are not copied - just open the original
    // remote datasources are cached in ~/.cache/... or something similar
    if (f->parentItem->server->dataSource == LOCAL)
    {
        f->cachePath = f->path;
        m_productView->fileDownloaded(f);
    }
    else if (f->parentItem->server->dataSource == FTP)
    {
        QString pathForItem(f->parentItem->server->getPathForItem(f->parentItem));
        QDir d;
        d.mkpath(pathForItem);

        f->cachePath = pathForItem + "/" + f->name;
        // simulate "real cache" hit. Use local copy of the file (if it exists)
        if (QFile::exists(f->cachePath))
        {
            m_productView->fileDownloaded(f);
        }
        else
        {
            f->transferHandler = DownloadModel::ServersModel;
            f->parentItem->server->downloadFiles(QList<File*>() << f, pathForItem);
        }
    }
    else
    {
        Q_ASSERT_X(0, "business logic error", "Unhandled dataSource type");
    }

    m_productView->show();
    // keep focus on the main window - keyboard handling
    activateWindow();
}

#warning TODO/FIXME: rename tree_doubleClicked regarding GUI vs signal
void ServersWidget::tree_doubleClicked(const QModelIndex &index)
{
    QModelIndex srcIndex = static_cast<QSortFilterProxyModel*>(partsTreeView->model())->mapToSource(index);

    File *f = fm->getRootItem()->files.at(srcIndex.row());

    switch (f->type)
    {
    case File::PRT_PROE:
    case File::ASM:
    case File::DRW:
    case File::FRM:
    case File::NEU_PROE:
    {
        QString exe = settings.value("ExternalPrograms/ProE/Executable", "proe.exe").toString();
        qDebug() << "Starting ProE:" << exe << f->path << "; working dir" << ui->editDir->text();
        bool ret = QProcess::startDetached(exe, QStringList() << f->path, ui->editDir->text());
        if (!ret)
            QMessageBox::information(this, tr("ProE Startup Error"),
                                     tr("An error occured while ProE has been requested to start"),
                                     QMessageBox::Ok);
        break;
    }
    default:
        QDesktopServices::openUrl(QUrl::fromLocalFile(f->path));
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

void ServersWidget::setFiltersDialog()
{
    FiltersDialog *dlg = new FiltersDialog(this);

    if(dlg->exec() == QDialog::Accepted)
    {
        Utils::saveFilters();
        rebuildFilters();
    }
}
