import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

# Set the input value on an API controlled input in a control scheme. Returns an error if not able to animate (Send as PUT)

# request data:
# string identifier
# float value

requestUrl = baseUrl + 'ControlInput/'	
try:
	requestParams = {}

	# set identifier for recording input
	requestParams['identifier'] = 'myIdentifier' 		# Required, provide the string identifier for the input as set up in the input settings menu															


	#change the current playhead time in Milliseconds
	requestParams['value'] = 0.5 						# Required, the value to set on the recording control
														# Movement parts (Joint, Motor, pose blend axis, etc) expect a value between 0.0 and 1.0
														# On Off events treat any non 0.0 value less than or equal to 1.0 as on, and 0.0 as off
														# Trigger events treat any non 0.0 value less than or equal to 1.0 as a new trigger



	response = requests.put(requestUrl, json=requestParams)
	response.raise_for_status()

except requests.exceptions.RequestException as e:
	raise SystemExit(e)	
