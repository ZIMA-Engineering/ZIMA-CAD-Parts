#ifndef FILEVIEWHEADER_H
#define FILEVIEWHEADER_H

#include <QHeaderView>
#include <QTreeView>
#include <QMap>
#include <QLineEdit>
#include <QSignalMapper>

class FileModel;

/*!
 * \brief QHeaderView for FileView with per-column filters
 *
 * The QHeaderView is extended with a filtering mode. When the search
 * mode is enabled, each column header will contain a line edit for
 * searched content.
 *
 * This solution is based on the following blog post:
 *
 *   http://blog.qt.io/blog/2012/09/28/qt-support-weekly-27-widgets-on-a-header/
 */
class FileViewHeader : public QHeaderView
{
    Q_OBJECT
public:
    FileViewHeader(FileModel *model, QWidget *parent = 0);

public slots:
    void newDirectory(const QString &path);
    void fixComboPositions();

signals:
    void filterColumn(int column, const QString &text);

protected:
    QSize sizeHint() const;
    void showEvent(QShowEvent *e);

private:
    FileModel *m_model;
    QMap<int, QLineEdit*> m_edits;
    QSignalMapper *m_mapper;

    void clearFields();
    void createFields();
    void setEditGeometry(QWidget *w, int index);
    void forceRedraw();

private slots:
    void handleSectionResized(int i);
    void handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex);
    void filter(int column);
    void sortIndicatorChange(int logicalIndex, Qt::SortOrder order);
};

#endif // FILEVIEWHEADER_H
