#include "fileview.h"
#include "filemodel.h"
#include "filefiltermodel.h"
#include "settings.h"

#include <QMessageBox>


FileView::FileView(QWidget *parent) :
    QTreeView(parent)
{
    m_model = new FileModel(this);
    m_proxy = new FileFilterModel(this);

    m_proxy->setSourceModel(m_model);
    setModel(m_proxy);
    //setModel(m_model);

    connect(m_model, SIGNAL(directoryLoaded(QString)),
            this, SLOT(resizeColumnToContents()));

    connect(MetadataCache::get(), SIGNAL(cleared()), this, SLOT(refreshModel()));
}

void FileView::setDirectory(const QString &path)
{
    if (path.isEmpty())
    {
        return;
    }
    m_path = path;
    // it has to be reset here because calling QFileSystemModel's reset
    // or begin/end alternatives results in "/" as a root path
    m_model->setDirectory(m_path);
    //setRootIndex(m_proxy->mapFromSource(m_model->setRootPath(m_path)));
    resizeColumnToContents();
}

void FileView::refreshModel()
{
    setDirectory(m_path);
}
void FileView::resizeColumnToContents()
{
    int columnCnt = m_model->columnCount(QModelIndex());
    for (int i = 0; i < columnCnt; i++)
    {
        QTreeView::resizeColumnToContents(i);
        // hack. Probably some QFontMetrics for header should be used. But not urgent for now.
        if (columnWidth(i) < 50)
            setColumnWidth(i, 50);
    }
}

void FileView::settingsChanged()
{
    m_model->settingsChanged();
    m_proxy->setShowProeVersions(Settings::get()->ShowProeVersions);
    refreshModel();
}

QFileInfo FileView::fileInfo(const QModelIndex &filteredIndex)
{
    QModelIndex ix = m_proxy->mapToSource(filteredIndex);
    return m_model->fileInfo(ix);
}

void FileView::createIndexHtmlFile(const QString &text, const QString &fileBase)
{
    QDir techSpecDir(m_path + "/" + TECHSPEC_DIR);

    if(!techSpecDir.exists())
        techSpecDir.mkdir(techSpecDir.absolutePath());

    QString lang = Settings::get()->getCurrentLanguageCode().left(2);
    QFile indexFile(techSpecDir + "/" + fileBase + "_" + lang + ".html");

    if(indexFile.exists())
    {
        if (QMessageBox::warning(0,
                                 tr("HTML index file already exists"),
                                 tr("HTML index for already exists, would you like to overwrite it?").arg(m_path),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    if(!indexFile.open(QIODevice::WriteOnly))
        return; // FIXME: Notify user on failure?

    QByteArray htmlIndex = QString("<html>\n"
                                   "	<head>\n"
                                   "		<meta http-equiv=\"refresh\" content=\"0;url=%1\">\n"
                                   "	</head>\n"
                                   "</html>\n").arg(text).toUtf8();

    indexFile.write(htmlIndex);
    indexFile.close();
}

void FileView::deleteParts()
{
    m_model->deleteParts();
}

void FileView::copyToWorkingDir()
{
    if (m_path == Settings::get()->WorkingDir)
        return;

    m_model->copyToWorkingDir();
}
