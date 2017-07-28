#include "fileeditdialog.h"
#include "ui_fileeditdialog.h"
#include "metadata.h"

#include <QLineEdit>

FileEditDialog::FileEditDialog(QString dir, QString file, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::FileEditDialog),
	m_dir(dir),
	m_file(file),
	m_next(false)
{
	ui->setupUi(this);

	setWindowTitle(tr("Edit %1").arg(file));
	ui->nameLabel->setText(file);

	Metadata *meta = MetadataCache::get()->metadata(dir);
	int partColumn = 1; // parts are indexed from 1
	int layoutRow = 1; // first row contains part name

	foreach (const QString &label, meta->columnLabels().mid(2)) {
		QLineEdit *edit = new QLineEdit(meta->partParam(file, partColumn++));
		m_edits << edit;

		ui->gridLayout->addWidget(new QLabel(label), layoutRow, 0);
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

	for (int i = 0; i < cnt; i++) {
		meta->setPartParam(m_file, i+1, m_edits[i]->text());
	}
}
