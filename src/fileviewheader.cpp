#include "fileviewheader.h"
#include "filemodel.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QDebug>

FileViewHeader::FileViewHeader(FileModel *model, QWidget *parent) :
	QHeaderView(Qt::Horizontal, parent),
	m_model(model)
{
	connect(m_model, SIGNAL(directoryLoaded(QString)),
			this, SLOT(newDirectory(QString)));
	connect(this, SIGNAL(sectionResized(int, int, int)),
			this, SLOT(handleSectionResized(int)));
	connect(this, SIGNAL(sectionMoved(int, int, int)),
			this, SLOT(handleSectionMoved(int, int, int)));
	connect(this, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
			this, SLOT(sortIndicatorChange(int,Qt::SortOrder)));

	setDefaultAlignment(Qt::AlignTop);

	m_mapper = new QSignalMapper(this);
	connect(m_mapper, SIGNAL(mappedInt(int)), this, SLOT(filter(int)));
}


void FileViewHeader::newDirectory(const QString &path)
{
	Q_UNUSED(path);

	clearFields();
	createFields();

	forceRedraw();
}

void FileViewHeader::fixComboPositions()
{
	QMap<int, QLineEdit*>::const_iterator i = m_edits.constBegin();
	while (i != m_edits.constEnd()) {
		setEditGeometry(i.value(), i.key());
		++i;
	}
}

QSize FileViewHeader::sizeHint() const
{
	QSize s = QHeaderView::sizeHint();
	s.setHeight(s.height() + 25);
	return s;
}

void FileViewHeader::showEvent(QShowEvent *e)
{
	QMap<int, QLineEdit*>::const_iterator i = m_edits.constBegin();
	while (i != m_edits.constEnd()) {
		int index = i.key();
		QLineEdit *edit = i.value();

		setEditGeometry(edit, index);
		edit->show();
		++i;
	}

	QHeaderView::showEvent(e);
}

void FileViewHeader::clearFields()
{
	foreach (QLineEdit *edit, m_edits)
		edit->deleteLater();

	m_edits.clear();
}

void FileViewHeader::createFields()
{
	if (!m_edits.empty())
		return;

	int cnt = m_model->columnCount();

	for (int i = 0; i < cnt; i++) {
		if (i == 1)
			continue;

		auto edit = new QLineEdit(this);
		edit->setPlaceholderText("Search...");

		connect(edit, SIGNAL(textChanged(QString)), m_mapper, SLOT(map()));
		m_mapper->setMapping(edit, i);

		m_edits[i] = edit;
	}
}

void FileViewHeader::setEditGeometry(QWidget *w, int index)
{
	w->setGeometry(
		sectionViewportPosition(index),
		20,
		sectionSize(index) - (index == sortIndicatorSection() ? 28 : 10),
		height() - 20
	);
}

void FileViewHeader::forceRedraw()
{
	// Force QShowEvent to be sent
	hide();
	show();

	emit geometriesChanged();
}

void FileViewHeader::handleSectionResized(int i)
{
	for (int j = visualIndex(i); j < count(); j++) {
		int logical = logicalIndex(j);

		if (logical == 1)
			continue;

		if (logical > m_edits.count())
			break;

		setEditGeometry(m_edits[logical], logical);
	}
}

void FileViewHeader::handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex)
{
	Q_UNUSED(logical)

	for (int i = qMin(oldVisualIndex, newVisualIndex); i < count(); i++){
		int logical = logicalIndex(i);

		if (logical == 1)
			continue;

		setEditGeometry(m_edits[logical], logical);
	}
}

void FileViewHeader::filter(int column)
{
	emit filterColumn(column, m_edits[column]->text());
}

void FileViewHeader::sortIndicatorChange(int logicalIndex, Qt::SortOrder order)
{
	Q_UNUSED(logicalIndex)
	Q_UNUSED(order)

	forceRedraw();
}
