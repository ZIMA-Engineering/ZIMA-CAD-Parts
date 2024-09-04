#include "fileeditdialog.h"
#include "ui_fileeditdialog.h"
#include "metadata.h"
#include "filemodel.h"
#include "settings.h"

#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>

FileEditDialog::FileEditDialog(QFileInfo file, FileModel *fileModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileEditDialog),
    m_fileInfo(file),
    m_fileModel(fileModel),
    m_next(false)
{
    ui->setupUi(this);

    m_dir = m_fileInfo.absoluteDir().path();
    m_fileName = m_fileInfo.baseName();

    setWindowTitle(tr("Edit %1").arg(m_fileName));
    ui->nameLabel->setText(m_fileName);

    setupThumbnail();
    connect(ui->removeThumbnailButton, SIGNAL(clicked()),
            this, SLOT(removeThumbnail()));
    connect(ui->setThumbnailButton, SIGNAL(clicked()),
            this, SLOT(openThumbnailDialog()));

    Metadata *meta = MetadataCache::get()->metadata(m_dir);
    QStringList paramHandles = meta->parameterHandles();
    QStringList paramLabels = meta->parameterLabels();
    int layoutRow = 2; // first row contains part name, second row is for thumbnail

    for (int i = 0; i < paramHandles.count(); i++)
    {
        QLineEdit *edit = new QLineEdit(meta->partParam(m_fileName, paramHandles[i]));
        m_edits << edit;

        ui->gridLayout->addWidget(new QLabel(paramLabels[i]), layoutRow, 0);
        ui->gridLayout->addWidget(edit, layoutRow++, 1);
    }

    connect(ui->saveAndNextButton, SIGNAL(clicked()),
            this, SLOT(saveAndNext()));
}

FileEditDialog::~FileEditDialog()
{
    delete ui;
}

bool FileEditDialog::editNext() const
{
    return m_next;
}

void FileEditDialog::saveAndNext()
{
    m_next = true;
    accept();
}

void FileEditDialog::save()
{
    int cnt = m_edits.count();
    Metadata *meta = MetadataCache::get()->metadata(m_dir);
    QStringList paramHandles = meta->parameterHandles();

    for (int i = 0; i < cnt; i++) {
        meta->setPartParam(m_fileName, paramHandles[i], m_edits[i]->text());
    }

    if (m_thumbnailPath != m_origThumbnailPath) {
        if (m_thumbnailPath.isEmpty()) {
            uninstallThumbnail();
        } else {
            installThumbnail();
        }
    }
}

void FileEditDialog::setupThumbnail()
{
    auto thumbPath = m_fileModel->findThumbnailPath(m_fileInfo);

    if (thumbPath.isNull()) {
        ui->thumbnailLabel->setText(tr("No thumbnail"));
        ui->removeThumbnailButton->setDisabled(true);
    } else {
        setThumbnail(QPixmap(thumbPath));
        m_thumbnailPath = thumbPath;
        m_origThumbnailPath = thumbPath;
    }
}

void FileEditDialog::installThumbnail()
{
    QFileInfo srcFile(m_thumbnailPath);
    QString dstPath = m_dir + "/" + THUMBNAILS_DIR + "/" + m_fileName + "." + srcFile.suffix();

    if (QFile::exists(dstPath))
        QFile::remove(dstPath);

    if (QFile::copy(m_thumbnailPath, dstPath)) {
        m_fileModel->reloadThumbnails();
        return;
    }

    QMessageBox::warning(
        this,
        tr("Unable to copy icon"),
        tr("Unable to copy '%1' to '%2'").arg(m_thumbnailPath).arg(dstPath)
    );
}

void FileEditDialog::uninstallThumbnail()
{
    if (QFile::remove(m_origThumbnailPath)) {
        m_fileModel->reloadThumbnails();
        return;
    }

    QMessageBox::warning(
        this,
        tr("Unable to remove thumbnail"),
        tr("Unable to remove thumbnail '%1'").arg(m_origThumbnailPath)
                );
}

void FileEditDialog::setThumbnail(const QPixmap &thumbnail)
{
    ui->thumbnailLabel->setPixmap(thumbnail.scaled(Settings::get()->GUIThumbWidth,
                                                   Settings::get()->GUIThumbWidth,
                                                   Qt::KeepAspectRatio));
}

void FileEditDialog::removeThumbnail()
{
    m_thumbnailPath.clear();
    ui->thumbnailLabel->setPixmap(QPixmap());
    ui->removeThumbnailButton->setEnabled(false);
}

void FileEditDialog::openThumbnailDialog()
{
    QString thumbFile = QFileDialog::getOpenFileName(
                           this,
                           tr("Select thumbnail"),
                           m_dir,
                           "Images (*.png *.jpg *.jpeg *.gif)"
                       );

    if (thumbFile.isNull())
        return;

    setThumbnail(QPixmap(thumbFile));
    m_thumbnailPath = thumbFile;
    ui->removeThumbnailButton->setEnabled(true);
}
