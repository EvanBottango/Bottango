import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)

# Get available animations. Returns a list of strings. Index order is what is used
# to call play animation etc. in other requests
#
# response data:
# string[] animations

requestUrl = baseUrl + 'Animations/'
try:
	response = requests.get(requestUrl)
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