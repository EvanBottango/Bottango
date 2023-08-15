import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

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
	response = requests.get(requestUrl)
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