#ifndef ZIMAUTILS_H
#define ZIMAUTILS_H

#include <QString>

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
};

#endif // ZIMAUTILS_H
