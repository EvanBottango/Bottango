import src.SocketDriverTime
import src.CallbacksAndConfiguration

from src.Effector import *

log = src.CallbacksAndConfiguration.log

### Command handler methods ###
### See Bottango Arduino Readme for detailed Bottango API details ###
	
def sendHandshakeResponse(params):			## respond to a handshake request in order to start back and forth communication ##
	response = "btngoHSK,"					# add the handshake response identifier
	response += src.CallbacksAndConfiguration.apiVersion + ','			# add the current api version
	response += params[1] + ','				# return back the random code to confirm recipt
	response += "1"							# signal that live input is accepted
	response += '\n'						# add terminating char to response
	response += "OK\n"						# finally send ready for next command
	return response

def handleTimeSync(params):					## record sync time in MS with the Bottango server##	
	src.SocketDriverTime.lastSyncTime = int(params[1])
	src.SocketDriverTime.lastSyncDT = datetime.now()

def deregisterAllEffectors(params, effectors):			## deregister all effectors
	for effector in list(effectors.values()):
		effector.deregister()
	effectors = {}

def deregisterEffector(params, effectors):				## deregister sepecific effector
	effector = effectors.get(params[1])
	if effector:
		effector.deregister()
		del effectors[params[1]]

def clearAllCurves(params, effectors):					## stop all movement and clear all curves on all effectors
	for effector in list(effectors.values()):
		effector.stop()	

def clearEffectorCurves(params, effectors):			## stop all movement and clear all curves on a single effector
	effector = effectors.get(params[1])
	if effector:
		effector.stop()

def registerPinServo(params, effectors):				## register a pin servo
	identifier = params[1]
	minSignal = int(params[2])
	maxSignal = int(params[3])
	maxSignalChangePerSecond = int(params[4])
	startingSignal = int(params[5])
	effectors[identifier] = Effector("pinServo", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerI2cServo(params, effectors):				## register a pin servo
	identifier = str(params[1]) + str(params[2]) #identifier is i2c address + pin a string
	minSignal = int(params[3])
	maxSignal = int(params[4])
	maxSignalChangePerSecond = int(params[5])
	startingSignal = int(params[6])
	effectors[identifier] = Effector("i2cServo", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerPinStepper(params, effectors):				## register a pin stepper
	identifier = params[1]	# pin 0 is used as an identifier, the rest are ignored for now
	minSignal = int(params[5])
	maxSignal = int(params[6])
	maxSignalChangePerSecond = int(params[7])
	startingSignal = int(params[8])
	effectors[identifier] = Effector("pinStepper", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)	

def registerStepDirStepper(params, effectors):			## register a Step / Direction stepper
	identifier = params[1]
	minSignal = int(params[4])
	maxSignal = int(params[5])
	maxSignalChangePerSecond = int(params[6])
	startingSignal = int(params[7])
	effectors[identifier] = Effector("stepDirStepper", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)	

def registerCustomMotor(params, effectors):			## register a custom motor
	identifier = params[1]
	minSignal = float(params[2])
	maxSignal = float(params[3])
	maxSignalChangePerSecond = float(params[4])
	startingSignal = int(params[5])
	effectors[identifier] = Effector("customMotor", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerCurvedEvent(params, effectors):			## register a curved event
	identifier = params[1]	
	minSignal = 0.0
	maxSignal = 1.0
	maxSignalChangePerSecond = float(params[2])
	startingSignal = int(params[3]) / 8192
	effectors[identifier] = Effector("curvedEvent", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerOnOffEvent(params, effectors):				## register an on off event
	identifier = params[1]	
	minSignal = False
	maxSignal = True
	maxSignalChangePerSecond = 1
	startingSignal = bool(int(params[2]))
	effectors[identifier] = Effector("onOffEvent", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerColorEvent(params, effectors):				## register a color event
	identifier = params[1]	
	minSignal = (0,0,0)
	maxSignal = (255,255,255)
	maxSignalChangePerSecond = 1
	startingSignal = (int(params[2]), int(params[3]), int(params[4]))
	effectors[identifier] = Effector("colorEvent", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def registerTriggerEvent(params, effectors):			## register a trigger event
	identifier = params[1]	
	minSignal = 0
	maxSignal = 1
	maxSignalChangePerSecond = 1
	startingSignal = 0
	effectors[identifier] = Effector("onOffEvent", identifier, minSignal, maxSignal, maxSignalChangePerSecond, startingSignal)

def setCurve(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		startTime = src.SocketDriverTime.lastSyncTime + int(params[2])
		duration = int(params[3])
		startMovement = int(params[4]) / 8192
		
		startControlX = int(params[5])
		startControlY = int(params[6]) / 8192
		
		endMovement = int(params[7]) / 8192
		
		endControlX = int(params[8])
		endControlY = int(params[9]) / 8192

		newCurve = Curve(startTime, duration, startMovement, startControlX, startControlY, endMovement, endControlX, endControlY)
		effector.curves.append(newCurve)

def setInstantCurve(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		endMovement = int(params[2]) / 8192
		newCurve = Curve(src.SocketDriverTime.getTimeOnServer(), 0, endMovement, 0, 0, endMovement, 0, 0)
		effector.curves.append(newCurve)

def setOnOffCurve(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		startTime = src.SocketDriverTime.lastSyncTime + int(params[2])
		on = bool(int(params[3]))
		newCurve = OnOffCurve(startTime, on)
		effector.curves.append(newCurve)

def setTriggerCurve(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		startTime = src.SocketDriverTime.lastSyncTime + int(params[2])
		newCurve = TriggerCurve(startTime)
		effector.curves.append(newCurve)

def setColorCurve(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		startTime = src.SocketDriverTime.lastSyncTime + int(params[2])
		duration = int(params[3])
		startColor = (int(params[4]), int(params[5]), int(params[6]))
		endColor = (int(params[7]), int(params[8]), int(params[9]))		
		newCurve = ColorCurve(startTime, duration, startColor, endColor)
		effector.curves.append(newCurve)

def setColorCurveInstant(params, effectors):
	effector = effectors.get(params[1])
	if effector:
		endColor = (int(params[2]), int(params[3]), int(params[4]))
		newCurve = ColorCurve(src.SocketDriverTime.getTimeOnServer(), 0, endColor, endColor)
		effector.curves.append(newCurve)

def updateSignalBounds(params, effectors):			## update effector signal bounds
	effector = effectors.get(params[1])
	effector.minSignal = (int(params[2]))
	effector.maxSignal = (int(params[3]))
	effector.maxSignalChangePerSecond = (int(params[4]))

## Turn command into action, returns a string to send as a response
def parseCommand (command, effectors):

	if log:
		print ("recivied: " + command)	
			
	# split the command into paramaters using a ',' seperator char
	split = command.split(',')	

	commandParsed = False;

	# handshake request command has it's own special response
	if split[0] == "hRQ":
		if log:
			print ("<- handshake request")
		commandParsed = True
		return sendHandshakeResponse(split)		
	
	# all other commands just execute then send "OK\n" to signal ready for next command
	if split[0] == "tSYN":
		if log:
			print ("<- time sync request")
		commandParsed = True
		handleTimeSync(split)

	if split[0] == "xE":
		if log:
			print ("<- deregister all effectors")
		commandParsed = True
		deregisterAllEffectors(split, effectors)

	if split[0] == "xUE":
		if log:
			print ("<- deregister specific effector")
		commandParsed = True
		deregisterEffector(split, effectors)

	if split[0] == "xC":
		if log:
			print("<- clear all curves")
		commandParsed = True
		clearAllCurves(split, effectors)

	if split[0] == "STOP":
		if log:
			print ("<- Stop")
		commandParsed = True
		clearAllCurves(split, effectors)
		socketThreadAlive = False

	if split[0] == "xUC":
		if log:
			print ("<- clear effector curves")
		commandParsed = True
		clearEffectorCurves(split, effectors)

	if split[0] == "rSVPin":
		if log:
			print ("<- Register pin servo")
		commandParsed = True
		registerPinServo(split, effectors)
		

	if split[0] == "rSVI2C":
		if log:
			print ("<- Register i2c servo")
		commandParsed = True
		registerI2cServo(split, effectors)

	if split[0] == "rSTPin":
		if log:
			print ("<- Register 4 pin stepper")
		commandParsed = True
		registerPinStepper(split, effectors)

	if split[0] == "rSTDir":
		if log:
			print ("<- Register step/direction stepper")
		commandParsed = True
		registerStepDirStepper(split, effectors)

	if split[0] == "rECC":
		if log:
			print ("<- Register curved event")
		commandParsed = True
		registerCurvedEvent(split, effectors)

	if split[0] == "rECOnOff":
		if log:
			print ("<- Register on off event")
		commandParsed = True
		registerOnOffEvent(split, effectors)

	if split[0] == "rECTrig":
		if log:
			print ("<- Register trigger event")
		commandParsed = True
		registerTriggerEvent(split, effectors)

	if split[0] == "rECColor":
		if log:
			print ("<- Register color event")
		commandParsed = True
		registerColorEvent(split, effectors)

	if split[0] == "upE":
		if log:
			print ("<- Update effector signal bounds")
		commandParsed = True
		updateSignalBounds(split, effectors)

	if split[0] == "rMTR":
		if log:
			print ("<- Register custom motor")
		commandParsed = True
		registerCustomMotor(split, effectors)

	if split[0] == "sycM":
		if log:
			print ("<- Sync Motor Currently Unsupported")
		commandParsed = True

	if split[0] == "sC":
		if log:
			print ("<- Set Curve")
		commandParsed = True
		setCurve(split, effectors)

	if split[0] == "sCI":
		if log:
			print ("<- Set instant Curve")
		commandParsed = True
		setInstantCurve(split, effectors)

	if split[0] == "sCO":
		if log:
			print ("<- Set on off Curve")
		commandParsed = True
		setOnOffCurve(split, effectors)

	if split[0] == "sCT":
		if log:
			print ("<- Set trigger Curve")
		commandParsed = True
		setTriggerCurve(split, effectors)
	
	if split[0] == "sCC":
		if log:
			print ("<- Set Color Curve")
		commandParsed = True
		setColorCurve(split, effectors)

	if split[0] == "sCCI":
		if log:
			print ("<- Set Color Curve Instant")
		commandParsed = True
		setColorCurveInstant(split, effectors)

	if commandParsed:
		# finally return "OK\n" as the response
		if log:
			print ("-> OK\n")		
	else:
		if log:
			print ("Unable to parse command")
	
	return "OK\n"