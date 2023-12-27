## !!! This should be the only file you need to edit / change, unless you want to
# make advanced changes, or you are building your own version of a Bottango client !!!

## Config variabls -- !! changes how this client works !! -- ##

import src.MainLoop

address = '127.0.0.1'  						# The server's hostname or IP address
port = 59225        						# The port used by the server
log = True									# enable logging
roundSignalToInt = True						# treat signal as an int (true) or as a float (false)
apiVersion = "CUSTOM7"						# api version to send in handshake response

## callbacks -- !! Put your custom code here !! -- ##

def handleEffectorRegistered(effectorType, identifier, minSignal, maxSignal, startingSignal):
	## !!! put your effector enable / turn on code here (if needed) !!! ##
	if log:
		print ("Register " + effectorType + " " + str(identifier))

def handleEffectorDeregistered(identifier):
	## !!! put your effector stop moving / turn off code here (if needed) !!! ##
	if log:
		print ("Deregister " + str(identifier))

def handleEffectorSetSignal(effectorType, identifier, signal):
	## !!! put your motor driving logic here. Signal will !!! ##
	## !!! Only called when the expected signal changes!!! ##
	if log:
		print ("Set signal on " + effectorType + " " + str(identifier) + ": " + str(signal))

def handleEffectorSetColor(effectorType, identifier, color):
	## !!! put your color/light callback logic here. !!! ##	
	r = color[0]
	g = color[1]
	b = color[2]
	if log:
		print ("Set color on " + effectorType + " " + str(identifier) + ": (" + str(r) + "," + str(g) +"," + str(b) + ")")

def handleEffectorSetOnOff(effectorType, identifier, on):
	## !!! put your on / off event callback logic here. Will !!! ##
	## !!! Only called when the expected on / off changes!!! ##
	if log:
		print ("Set on/off event on " + effectorType + " " + istr(identifier) + ": " + str(on))

def handleEffectorSetTrigger(effectorType, identifier):
	## !!! put your trigger event callback logic here. !!! ##
	if log:
		print ("Set trigger event on " + effectorType + " " + str(identifier))

def onMainLoop():		
	## !! convinent method if you want to do other things in the main loop ##		
	pass # delete this line if you add something to this callback
	# if you want to get the time on server, import src.SocketDriverTime
	# src.SocketDriverTime.getTimeOnServer() returns either time in MS on server or None if disconnected
	#
	#
	# to request stop and disconnect in app:
	# src.MainLoop.requestQueue.put('\nreqStop\n')
	#
	#
	# to request pause current playing animation:
	# src.MainLoop.requestQueue.put('\nreqPause\n')
	#
	#
	# to request start playing animation by index (index 2 in example), at time in MS (500ms from start in example):	
	# src.MainLoop.requestQueue.put('\nreqPlay,2,500\n')
	#
	#
	# to request start playing current animation at current time (-1 for index is current animation, -1 at time is current time in app):
	# src.MainLoop.requestQueue.put('\nreqPlay,-1,-1\n')
	