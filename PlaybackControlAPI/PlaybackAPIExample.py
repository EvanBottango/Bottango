import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)


# Check if Bottango is currently able to animate
#
# response data:
# bool canAnimate

requestUrl = baseUrl + 'CanAnimate/'
try:
	response = requests.get(requestUrl);
	response.raise_for_status()
	responseData = response.json()
	print ('Can Animate: {}'.format(responseData['canAnimate']))
	print ('------')
except requests.exceptions.RequestException as e:
	raise SystemExit(e)


# Get available animations. Returns a list of strings. Index order is what is used
# to call play animation etc. in other requests
#
# response data:
# string[] animations

requestUrl = baseUrl + 'Animations/'
try:
	response = requests.get(requestUrl);
	response.raise_for_status()
	responseData = response.json()

	print ('Animations:')
	i = 0
	for animationName in responseData['animations']:
		print('Animation {}: {}'.format(i, animationName))
		i += 1

	print ('------')

except requests.exceptions.RequestException as e:
	raise SystemExit(e)


# Get currently selected animation index. Returns an error if not able to animate
#
# response data:
# int selectedAnimationIndex
# string selectedAnimationName

requestUrl = baseUrl + 'Animations/Selected/'
try:

	response = requests.get(requestUrl);
	response.raise_for_status()
	responseData = response.json()

	print ('Selected Animation: {} at index {}'.format(responseData['selectedAnimationName'], responseData['selectedAnimationIndex']))
	print ('------')

except requests.exceptions.RequestException as e:
	raise SystemExit(e)

# Get current state of animation playback. Returns an error if not able to animate
#
# response data:
# int selectedAnimationIndex
# string selectedAnimationName
# bool isPlaying
# int playbackTimeInMS
# int durationInMS
# obj[] effectors
#	string name
#	bool live
#	string identifier
#	string driverName
#	bool driverLive
#	float movement
#	float signal

requestUrl = baseUrl + 'PlaybackState/'
try:
	response = requests.get(requestUrl);
	response.raise_for_status()
	responseData = response.json()

	print ('playback state:')
	print ('Selected Animation: {} at index {}'.format(responseData['selectedAnimationName'], responseData['selectedAnimationIndex']))
	print ('is playing: {}'.format(responseData['isPlaying']))
	print ('playback time in MS: {}'.format(responseData['playbackTimeInMS']))
	print ('selected animation duration time in MS: {}'.format(responseData['durationInMS']))

	print ('effector status:')
	for effector in responseData['effectors']:
		print ('------')
		print ('name: {}'.format(effector['name']))
		print ('live: {}'.format(effector['live']))
		print ('identifier: {}'.format(effector['identifier']))
		print ('driverName: {}'.format(effector['driverName']))
		print ('driverLive: {}'.format(effector['driverLive']))
		print ('movement: {}'.format(effector['movement'])) # movement is normalized signal
		print ('signal: {}'.format(effector['signal']))

		# motors/curved events return signal / movement you would expect (0.0 - 1.0 as float for movement, direct signal value for signal)

		# On Off events return on off state as a bool cast to an int

		# Audio keyframes (when hardware playback is enabled) signal/movement is -1 if not playing, positive value if playing.
		# Audio signal is audio playback time from start of audio in MS
		# Audio movement is audio playback time / audio clip length.

	print ('------')

except requests.exceptions.RequestException as e:
	raise SystemExit(e)


# Set current state of animation playback. Returns an error if not able to animate (Send as PUT)

# request data:
# int selectedAnimationIndex
# string selectedAnimationName
# bool isPlaying
# int playbackTimeInMS

requestUrl = baseUrl + 'PlaybackState/'
try:
	requestParams = {}

	# change selected animation
	requestParams['selectedAnimationIndex'] = 0; 		# optional, if you want to change selected animation. Otherwise keeps existing animation
	#requestParams['selectedAnimationName'] = 'myAnim'; # optional, same as index, you can use either. Only index will be used if both are present.
														# if you switch animation while playing, Bottango will stop unless requested with isPlaying
														# returns an error if you try and select an animation with the same name as another animation.


	#change the current playhead time in Milliseconds
	requestParams['playbackTimeInMS'] = 2000; 			# optional
														# if you change time while playing, Bottango will stop unless requested with isPlaying
														# if starting play in the same request, will jog to requested time then start
														# if switching to new animation in the same request, will use this time for the jog instead of the current time


  	# change the desired playback state
	requestParams['isPlaying'] = True; 					# optional, if you want to change whether playing selected animation or not.
														# Also include as True if you want to maintain playing after changing time or clip
														# otherwise changing time and or clip stops playing.

	# change the desired recording input state
	requestParams['isRecording'] = False; 				# optional, if you want to change whether recording live controller input into selected animation or not.
														# Also include as True if you want to begin recording after changing time or clip
														# isPlaying will be ignored if isRecroding is set




	response = requests.put(requestUrl, json=requestParams);
	response.raise_for_status()

except requests.exceptions.RequestException as e:
	raise SystemExit(e)

# Set master live off. Used as an API version of a stop button. The same as pressing "Escape" on your keyboard in Bottango,
# or toggling master to not live. By design there is no API to set back live, do that manually in Bottango. (Send as PUT)

# request data:
# None

# commented out so you can run this script as is without stopping bottango, but the below call is supported

# requestUrl = baseUrl + 'Stop/'
# try:
# 	response = requests.put(requestUrl);
# 	response.raise_for_status()
# except requests.exceptions.RequestException as e:
# 	raise SystemExit(e)



print ('requests complete')