import src.SocketDriverTime
import src.CallbacksAndConfiguration
import math

from src.BezierCurve import *
from src.OnOffCurve import *
from src.TriggerCurve import *
from src.ColorCurve import *
from datetime import datetime

### All effectors (servos, steppers, etc.) are instances of the same Effector class.	###
### However, effectors do have an effector type string in case you want to keep track.	###

class Effector:
	
	def __init__(self, effectorType, identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal):		
		self.effectorType = effectorType
		self.identifier = identifier	
		self.minSignal = minSignal
		self.maxSignal = maxSignal
		self.maxSignalChangePerSecond = maxSignalChangePerSecond		
		
		self.currentSignal = startingSignal			

		self.curves = []

		self.register()
		self.lastSignalSetTime = 0

	def register(self):
		src.CallbacksAndConfiguration.handleEffectorRegistered(self.effectorType, self.identifier, self.minSignal, self.maxSignal, self.currentSignal)
		self.setSignal(self.currentSignal)

	def deregister(self):
		src.CallbacksAndConfiguration.handleEffectorDeregistered(self.identifier)
		self.stop()		

	def stop(self):		
		self.curves = []		

	def setSignal(self, signal):
		src.CallbacksAndConfiguration.handleEffectorSetSignal(self.effectorType, self.identifier, signal)		
		self.currentSignal = signal
		self.lastSignalSetTime = datetime.now().timestamp()

	def setOnOffSignal(self, on):
		src.CallbacksAndConfiguration.handleEffectorSetOnOff(self.effectorType, self.identifier, on)		
		self.currentSignal = on

	def setTrigger(self):
		src.CallbacksAndConfiguration.handleEffectorSetTrigger(self.effectorType, self.identifier)		

	def setColor(self, color):
		src.CallbacksAndConfiguration.handleEffectorSetColor(self.effectorType, self.identifier, color)
		self.currentSignal = color

	def update(self):

		timeOnServer = src.SocketDriverTime.getTimeOnServer()		
		
		expiredCurves = []		
		inRangeCurves = []

		for curve in self.curves:
			if curve.expired(timeOnServer):
				expiredCurves.append(curve)
			elif curve.inRange(timeOnServer):
				inRangeCurves.append(curve)
		
		curveToExcecute = None

		# curve in range to play
		if len(inRangeCurves) > 0:			
			# remove all in expired
			for curve in expiredCurves:
				self.curves.remove(curve)
			# play earliest start in range curve
			for curve in inRangeCurves:
				if curveToExcecute is None:
					curveToExcecute = curve				
				elif curve.startTime < curveToExcecute.startTime:
					curveToExcecute = curve
		# play latest finish expired curve
		elif len(expiredCurves) > 0:
			for curve in expiredCurves:
				if curveToExcecute is None:
					curveToExcecute = curve				
				elif curve.endTime > curveToExcecute.endTime:
					# remove previous
					if curveToExcecute in self.curves:
						self.curves.remove(curveToExcecute)					
					curveToExcecute = curve

		if curveToExcecute is not None:

			if isinstance(curveToExcecute, Curve):

				if curveToExcecute.expired(timeOnServer):
					# go to end if expired
					movement = curveToExcecute.endY
				else:
					# evaluate if in range
					movement = curveToExcecute.evaluate(timeOnServer)			
				
				signal = lerp(self.minSignal, self.maxSignal, movement)			
				signal = self.speedLimitSignal(lerp(self.minSignal, self.maxSignal, movement))

				if src.CallbacksAndConfiguration.roundSignalToInt:
					signal = round(signal)
				
				if not signal == self.currentSignal:				
					self.setSignal(signal)
			
			elif isinstance(curveToExcecute, OnOffCurve):
				on = curveToExcecute.evaluate(timeOnServer)
				if not on == self.currentSignal:				
					self.setOnOffSignal(on)

			elif isinstance(curveToExcecute, TriggerCurve):				
				self.setTrigger()
				self.curves.remove(curveToExcecute) # trigger curves should only fire once

			elif isinstance(curveToExcecute, ColorCurve):
				color = curveToExcecute.evaluate(timeOnServer)
				if not color == self.currentSignal:
					self.setColor(color)

					


	# limit signal change to max speed #
	def speedLimitSignal (self, newTarget):

		returnSignal = newTarget

		now = (datetime.now().timestamp())
		elapsedTime = now - self.lastSignalSetTime
		maxSignalInElapsedTime = self.maxSignalChangePerSecond * elapsedTime		
		if src.CallbacksAndConfiguration.roundSignalToInt:
			maxSignalInElapsedTime = math.floor(maxSignalInElapsedTime)

		if abs(self.currentSignal - returnSignal) > maxSignalInElapsedTime:
			if self.currentSignal < returnSignal: 								# move forward	        
				returnSignal = self.currentSignal + maxSignalInElapsedTime
			else:																# move backward
				returnSignal = self.currentSignal - maxSignalInElapsedTime        
		

		if (returnSignal > self.maxSignal):	    
			returnSignal = self.maxSignal	    
		elif returnSignal < self.minSignal:
			returnSignal = self.minSignal	    

		return returnSignal