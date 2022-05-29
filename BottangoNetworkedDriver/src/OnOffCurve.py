class OnOffCurve:

	def __init__ (self, startTime, on):
		self.startTime = startTime
		self.endTime = startTime
		self.on = on

	def expired (self, time):
		return time >= self.startTime

	def inRange (self, time):
		return False

	def evaluate(self, time):
		return self.on