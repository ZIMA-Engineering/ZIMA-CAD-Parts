#include "servertabwidget.h"
#include "ui_servertabwidget.h"

ServerTabWidget::ServerTabWidget(ServersModel *serversModel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerTabWidget),
    m_serversModel(serversModel)
{
    ui->setupUi(this);

    ui->techSpecBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->techSpecForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->techSpecReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->techSpecGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

    connect(ui->techSpecBackButton, SIGNAL(clicked()),
            ui->techSpec, SLOT(back()));
    connect(ui->techSpecForwardButton, SIGNAL(clicked()),
            ui->techSpec, SLOT(forward()));
    connect(ui->techSpecReloadButton, SIGNAL(clicked()),
            ui->techSpec, SLOT(reload()));
    connect(ui->techSpecUrlLineEdit, SIGNAL(returnPressed()),
            this, SLOT(techSpecUrlLineEdit_returnPressed()));
    connect(ui->techSpecGoButton, SIGNAL(clicked()),
            this, SLOT(techSpecGoButton_clicked()));
    connect(ui->techSpecPinButton, SIGNAL(clicked()),
            this, SLOT(techSpecPinButton_clicked()));
    connect(ui->techSpec, SIGNAL(urlChanged(QUrl)),
            this, SLOT(techSpec_urlChanged(QUrl)));

    ui->partsIndexBackButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->partsIndexForwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->partsIndexReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->partsIndexGoButton->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

    connect(ui->partsIndexBackButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(back()));
    connect(ui->partsIndexForwardButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(forward()));
    connect(ui->partsIndexReloadButton, SIGNAL(clicked()),
            ui->partsWebView, SLOT(reload()));
    connect(ui->partsIndexUrlLineEdit, SIGNAL(returnPressed()),
            this, SLOT(partsIndexUrlLineEdit_returnPressed()));
    connect(ui->partsIndexGoButton, SIGNAL(clicked()),
            this, SLOT(partsIndexGoButton_clicked()));
    connect(ui->partsIndexPinButton, SIGNAL(clicked()),
            this, SLOT(partsIndexPinButton_clicked()));
    connect(ui->partsWebView, SIGNAL(urlChanged(QUrl)),
            this, SLOT(partsWebView_urlChanged(QUrl)));

    m_fileModel = new FileModel(this);

    m_proxyFileModel = new FileFilterModel(this);
    m_proxyFileModel->setSourceModel(m_fileModel);
    ui->partsTreeView->setModel(proxy);

    connect(m_fileModel, SIGNAL(requestColumnResize()),
            this, SLOT(fileModel_requestColumnResize()));

    connect(m_serversModel, SIGNAL(filesDeleted()),
            this, SLOT(filesDeleted()));
    connect(m_serversModel, SIGNAL(itemLoaded(const QModelIndex&)),
            this, SLOT(partsIndexLoaded(const QModelIndex&)));

    settingsChanged();
}

ServerTabWidget::~ServerTabWidget()
{
    delete ui;
}

void ServerTabWidget::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        if (ui->techSpec->url().path().startsWith("/data/zima-cad-parts") )
            ui->techSpec->loadAboutPage();
        break;
    default:
        break;
    }
}

void ServerTabWidget::settingsChanged()
{
    QSettings s;
    bool developer = s.value("Developer/Enabled", false).toBool();
    bool techSpecToolBarEnabled = developer && s.value("Developer/TechSpecToolBar", true).toBool();

    ui->techSpecDeveloperWidget->setVisible(techSpecToolBarEnabled);
    ui->partsIndexDeveloperWidget->setVisible(developer);

    ui->techSpec->loadAboutPage();
    ui->techSpec->setDownloadDirectory(s.value("WorkingDir", QDir::homePath() + "/ZIMA-CAD-Parts").toString());

    m_fileModel->setRootIndex(QModelIndex());
    m_fileModel->setThumbWidth( s.value("GUI/ThumbWidth", 32).toInt() );
    m_fileModel->setPreviewWidth( s.value("GUI/PreviewWidth", 256).toInt() );

    ui->thumbnailSizeSlider->setValue(settings.value("GUI/ThumbWidth", 32).toInt());
}

void ServerTabWidget::techSpecUrlLineEdit_returnPressed()
{
    techSpecGoButton_clicked();
}

void ServerTabWidget::techSpecGoButton_clicked()
{
    QString str = ui->techSpecUrlLineEdit->text();

    if (str == "ZIMA-CAD-Parts:about")
    {
        ui->techSpec->loadAboutPage();
    }
    else
        ui->techSpec->setUrl(QUrl(str));
}

void ServerTabWidget::techSpecPinButton_clicked()
{
    Item *it = fm->getRootItem();

    if (!it)
        return;

    QString lang = MainWindow::getCurrentMetadataLanguageCode().left(2);
    it->server->assignTechSpecUrlToItem(ui->techSpecUrlLineEdit->text(), it, lang, overwrite);
}

void ServerTabWidget::partsIndexUrlLineEdit_returnPressed()
{
    partsIndexGoButton_clicked();
}

void ServerTabWidget::partsIndexGoButton_clicked()
{
    QString str = ui->partsIndexUrlLineEdit->text();

    ui->partsWebView->load(QUrl(str));

    if (ui->partsWebView->isHidden())
        ui->partsWebView->show();
}

void ServerTabWidget::partsIndexPinButton_clicked()
{
    Item *it = fm->getRootItem();

    if (!it)
        return;

    QString lang = MainWindow::getCurrentMetadataLanguageCode().left(2);
    it->server->assignPartsIndexUrlToItem(ui->partsIndexUrlLineEdit->text(), it, lang, overwrite);
}

void ServerTabWidget::techSpec_urlChanged(const QUrl &url)
{
    ui->techSpecBackButton->setEnabled(ui->techSpec->history()->canGoBack());
    ui->techSpecForwardButton->setEnabled(ui->techSpec->history()->canGoForward());

    QString str = url.toString();

    if (str == "about:blank")
        return;

    ui->techSpecUrlLineEdit->setText(str);
}

void ServerTabWidget::partsWebView_urlChanged(const QUrl &url)
{
    ui->partsIndexBackButton->setEnabled(ui->partsWebView->history()->canGoBack());
    ui->partsIndexForwardButton->setEnabled(ui->partsWebView->history()->canGoForward());
    QString str = url.toString();
    ui->partsIndexUrlLineEdit->setText(str);
}

void ServerTabWidget::fileModel_requestColumnResize()
{
    int columnCnt = ui->partsTreeView->model()->columnCount(QModelIndex());
    for (int i = 0; i < columnCnt; i++)
        ui->partsTreeView->resizeColumnToContents(i);
}

void ServerTabWidget::rebuildFilters()
{
    QStringList expressions;

    int cnt = Utils::filterGroups.count();

    for(int i = 0; i < cnt; i++)
    {
        if(!filterGroups[i].enabled)
            continue;

        int filterCnt = filterGroups[i].filters.count();

        for(int j = 0; j < filterCnt; j++)
        {
            switch(filterGroups[i].filters[j]->filterType())
            {
            case FileFilter::Extension:
                if(filterGroups[i].filters[j]->enabled)
                    expressions << File::getRxForFileType(filterGroups[i].filters[j]->type);
                break;

            case FileFilter::Version:
                proxy->setShowProeVersions(filterGroups[i].filters[j]->enabled);
                break;
            }


        }
    }

    expressions.removeDuplicates();

    QRegExp rx( "^" + expressions.join("|") + "$" );
    proxy->setFilterRegExp(rx);
}

void ServerTabWidget::filesDeleted()
{
    if (m_serversModel->hasErrors(BaseDataSource::Delete))
    {
        ErrorDialog dlg;
        dlg.setError(tr("Unable to delete files:"));

        QString str = "<html><body><dl>";
        foreach(BaseDataSource::Error *e, serversModel->fileErrors(BaseDataSource::Delete))
        {
            str += "<dt>" + e->file->path + ":</dt>";
            str += "<dd>" + e->error + "</dd>";
        }
        str += "</dl></body></html>";

        dlg.setText(str);
        dlg.exec();
    } else {
        // FIXME: if the deletion should occur in another thread and take more time, it will
        // be neccessary to check if the reset is needed (user might be viewing something entirely
        // different by the time it's finished).

        ui->partsTreeView->reset();
    }
}

void ServerTabWidget::setPartsIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    qDebug() << "Set parts index" << static_cast<Item*>(index.internalPointer())->name;

    m_fileModel->setRootIndex(index);
    partsIndexLoaded(index);
}

void ServerTabWidget::partsIndexLoaded(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    Item *it = static_cast<Item*>(index.internalPointer());

    if (it == m_fileModel->getRootItem())
    {
        viewHidePartsIndex(it);
    }
}
