#ifndef METADATAV2MIGRATION_H
#define METADATAV2MIGRATION_H

#include "../metadatamigration.h"

class MetadataV2Migration : public MetadataMigration
{
public:
	bool migrate();

private:
	QString paramHandle(int col);
};

#endif // METADATAV2MIGRATION_H
