import requests
import json

port = 59224
baseUrl = 'http://localhost:{}/'.format(port)


# Set master live off. Used as an API version of a stop button. The same as pressing "Escape" on your keyboard in Bottango,
# or toggling master to not live. By design there is no API to set back live, do that manually in Bottango. (Send as PUT)

# request data:
# None

requestUrl = baseUrl + 'Stop/'
try:
	response = requests.put(requestUrl)
	response.raise_for_status()
except requests.exceptions.RequestException as e:
	raise SystemExit(e)