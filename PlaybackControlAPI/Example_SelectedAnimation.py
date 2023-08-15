import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

# Get currently selected animation index. Returns an error if not able to animate
#
# response data:
# int selectedAnimationIndex
# string selectedAnimationName

requestUrl = baseUrl + 'Animations/Selected/'
try:

	response = requests.get(requestUrl)
	response.raise_for_status()
	responseData = response.json()

	print ('Selected Animation: {} at index {}'.format(responseData['selectedAnimationName'], responseData['selectedAnimationIndex']))
	print ('------')

except requests.exceptions.RequestException as e:
	raise SystemExit(e)