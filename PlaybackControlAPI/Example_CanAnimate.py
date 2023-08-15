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
	response = requests.get(requestUrl)
	response.raise_for_status()
	responseData = response.json()
	print ('Can Animate: {}'.format(responseData['canAnimate']))
	print ('------')
except requests.exceptions.RequestException as e:
	raise SystemExit(e)