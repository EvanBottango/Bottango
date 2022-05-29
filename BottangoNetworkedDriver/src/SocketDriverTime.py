from datetime import datetime

lastSyncTime = 0							# time sent at last time sync
lastSyncDT = None							# datetime of last time sync

def getTimeOnServer():
	if lastSyncDT is None:
		return None
	return lastSyncTime + (datetime.now().timestamp() * 1000) - (lastSyncDT.timestamp() * 1000)