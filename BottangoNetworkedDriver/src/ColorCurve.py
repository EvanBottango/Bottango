class ColorCurve:

	def __init__ (self, startTime, duration, startColor, endColor):
		self.startTime = startTime
		self.duration = duration 
		self.startColor = startColor
		self.endColor = endColor
		self.endTime = self.startTime + self.duration

	def expired (self, time):
		return time > self.endTime

	def inRange (self, time):
		return time >= self.startTime and time <= self.endTime

	def evaluate(self, time):
		if time < self.startTime:
			return self.startColor
		if time > self.endTime:
			return self.endColor
		if self.startTime == self.endTime:
			return self.endColor

		t = (time - self.startTime) / (self.endTime - self.startTime)

		r = (1 - t) * self.startColor[0] + t * self.endColor[0]
		g = (1 - t) * self.startColor[1] + t * self.endColor[1]
		b = (1 - t) * self.startColor[2] + t * self.endColor[2]

		return (r, g, b)

