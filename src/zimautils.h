#ifndef ZIMAUTILS_H
#define ZIMAUTILS_H

#include <QString>
#include <QStringList>


class ZimaUtils
{
public:
	enum ZimaUtility {
		ZimaPtcCleaner,
		ZimaCadSync,
		ZimaPs2Pdf,
		ZimaStepEdit,
		ZimaUtilsCount
	};

	static QString internalNameForUtility(int util);
	static QString internalNameForUtility(ZimaUtility util);
	static QString labelForUtility(int util);
	static QString labelForUtility(ZimaUtility util);
	static QHash<QString,QString> paths();
};

#endif // ZIMAUTILS_H
