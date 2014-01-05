#ifndef TRANSFERQUEUEABLE_H
#define TRANSFERQUEUEABLE_H

class TransferHandler
{
public:
	TransferHandler();
	virtual void stopDownload() = 0;
	virtual void resumeDownload() = 0;
	virtual void clearQueue() = 0;
};

#endif // TRANSFERQUEUEABLE_H
