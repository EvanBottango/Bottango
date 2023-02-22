from src.SocketDriverLerp import *

### Curve is evaluated over time to generate signal. This shouldn't need any changes.										###
### If you're building your own networked driver, you can choose to ignore all curve processing, and have the Bottango app 	###
### just send baked positions instead. That is the default behavior. The curve code below is only if you want to emulate 	###
### the full curve evaluation behavior.																					  	###


class Curve:

    def __init__(self, startTime, duration, startY, startControlX, startControlY, endY, endControlX, endControlY):
        self.startTime = startTime
        self.duration = duration
        self.startY = startY
        self.startControlX = startControlX
        self.startControlY = startControlY
        self.endY = endY
        self.endControlX = endControlX
        self.endControlY = endControlY
        self.endTime = self.startTime + self.duration
        self.lastU = 0.5

    def expired(self, time):
        return time > self.endTime

    def inRange(self, time):
        return time >= self.startTime and time <= self.endTime

    def evaluate(self, time):

        uLower = 0
        uUpper = 1
        u = self.lastU
        x = time

        while True:
            evaluatedX = 0
            evaluatedY = 0

            evaluatedX, evaluatedY = self.evaluateForU(u)

            if abs(evaluatedX - x) < 1:
                self.lastU = u
                return evaluatedY
            elif evaluatedX > x:
                uUpper = u
            elif evaluatedX < x:
                uLower = u

            u = (uUpper - uLower) / 2 + uLower

    def evaluateForU(self, u):
        p11x = lerp(self.startTime, self.startTime + self.startControlX, u)
        p11y = lerp(self.startY, self.startY + self.startControlY, u)

        p12x = lerp(self.startTime + self.startControlX,
                    self.endTime + self.endControlX, u)
        p12y = lerp(self.startY + self.startControlY,
                    self.endY + self.endControlY, u)

        p13x = lerp(self.endTime + self.endControlX, self.endTime, u)
        p13y = lerp(self.endY + self.endControlY, self.endY, u)

        p21x = lerp(p11x, p12x, u)
        p21y = lerp(p11y, p12y, u)

        p22x = lerp(p12x, p13x, u)
        p22y = lerp(p12y, p13y, u)

        evaluatedX = lerp(p21x, p22x, u)
        evaluatedY = lerp(p21y, p22y, u)

        return evaluatedX, evaluatedY
