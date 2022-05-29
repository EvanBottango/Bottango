class TriggerCurve:

	def __init__ (self, startTime):
		self.startTime = startTime
		self.endTime = startTime

	def expired (self, time):
		return time >= self.startTime

	def inRange (self, time):
		return False