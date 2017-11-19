#ifndef DATASOURCEHISTORY_H
#define DATASOURCEHISTORY_H

#include <QObject>

class DataSourceHistory : public QObject
{
	Q_OBJECT
public:
	explicit DataSourceHistory(QObject *parent = nullptr);

public slots:
	bool canGoBack() const;
	void goBack();
	bool canGoForward() const;
	void goForward();
	void track(const QString &path);
	void goTo(const QString &path);
	void goToWorkingDirectory();
	void clear();

signals:
	void canGoBackChanged(bool can);
	void canGoForwardChanged(bool can);
	void openDirectory(const QString &path);

private:
	QStringList m_history;
	int m_currentIndex;
	bool m_canGoBack;
	bool m_canGoFwd;

	bool checkCanGoBack() const;
	bool checkCanGoForward() const;
	void update();
};

#endif // DATASOURCEHISTORY_H
