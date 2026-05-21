import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

# Get animation data as a JSON array of command strings
#
# response data:
# Json formated exported animation commands

requestUrl = baseUrl + 'AnimationCommands/'
try:
	requestParams = {}

	# desired animations
	# requestParams['animationIdexes'] = [0, 1] 						# optional array of ints, if you want to only include the given animations by index in the exported code
	# requestParams['animationNames'] = ['myAnim', 'myOtherAnim'] 		# optional array of strings, if you want to only include the given animations by name in the exported code
																		# names will be ignored if indexes are also sent.
																		# returns an error if you try and select invalid animations
																		# All animations will be included if not sent


	# starting animation for setup strings
	# requestParams['startingAnimationIndex'] = 0 						# optional, if you want to set which animation is used for the starting position of motors in the setup commands.
	# requestParams['startingAnimationName'] = 'myAnim' 				# optional string based, same as index, you can use either. Only index will be used if both are present.
																		# returns an error if you try and select invalid animation
																		# Index 0 animation will be used if not sent


  	# desired effectors
	# requestParams['effectorIdentifiers'] = [{"Name" : "Default Driver", "Identifiers" : [3, 5, 6]}]
																		# optional, if you want to only incldue certain drivers or effectors.		
																		# should be formated as an array of objects.
																		# Each object should have a 'Name' field, with the name of the driver as a string and
																		# an 'Identifiers' field which is an array of strings, each string the identifier of a desired effector to include
																		# defaults to all drivers and effectors if not sent.


	response = requests.get(requestUrl, json=requestParams)
	response.raise_for_status()
	responseData = response.json()
	print(responseData)

except requests.exceptions.RequestException as e:
	raise SystemExit(e)