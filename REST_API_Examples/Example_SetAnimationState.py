import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

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
	requestParams['selectedAnimationIndex'] = 0 		# optional, if you want to change selected animation. Otherwise keeps existing animation
	#requestParams['selectedAnimationName'] = 'myAnim' # optional, same as index, you can use either. Only index will be used if both are present.
														# if you switch animation while playing, Bottango will stop unless requested with isPlaying
														# returns an error if you try and select an animation with the same name as another animation.


	#change the current playhead time in Milliseconds
	requestParams['playbackTimeInMS'] = 2000 			# optional
														# if you change time while playing, Bottango will stop unless requested with isPlaying
														# if starting play in the same request, will jog to requested time then start
														# if switching to new animation in the same request, will use this time for the jog instead of the current time


  	# change the desired playback state
	requestParams['isPlaying'] = True 					# optional, if you want to change whether playing selected animation or not.
														# Also include as True if you want to maintain playing after changing time or clip
														# otherwise changing time and or clip stops playing.

	# change the desired recording input state
	# requestParams['isRecording'] = False 				# optional, if you want to change whether recording live controller input into selected animation or not.
														# Also include as True if you want to begin recording after changing time or clip
														# isPlaying will be ignored if isRecroding is set




	response = requests.put(requestUrl, json=requestParams)
	response.raise_for_status()

except requests.exceptions.RequestException as e:
	raise SystemExit(e)