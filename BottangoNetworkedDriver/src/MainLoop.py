import socket
import threading
from queue import Queue
import src.CommandParse
import src.CallbacksAndConfiguration
import atexit

effectors = {}								# effectors dictionary, key: identifier, value: Effector class instance		
cmdQueue = Queue()							# thread safe queue of recieved commands
socketThreadAlive = False

def threadedRead(s):

	global socketThreadAlive	
	cmdBuffer = ""								# buffer to write into from network

	while socketThreadAlive:
		# get data from network buffer		
		try:
			data = s.recv(1024);			
		except Exception as e:
			socketThreadAlive = False
			print("Unable to recieve data: ")
			print(e)
			exit()

		if data:			
			cmdBuffer = cmdBuffer + data.decode()	
			data = ""		

		# check if there's a command that was recieved in the buffer
		# new line is the terminating character for a command
		if '\n' in cmdBuffer:
			# handle the possibility that multiple commands came through
			commands = cmdBuffer.split('\n')
			[cmdQueue.put(command) for command in commands[:-1]]
			# last element of the split will either be an incomplete command or an empty string
			cmdBuffer = commands[-1]

def onExit(s):
	s.close()

def startLoop():		

	## open the socket connection
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		atexit.register(onExit, s)

		try:			
			s.connect((src.CallbacksAndConfiguration.address, src.CallbacksAndConfiguration.port))
		except Exception as e:
			print("Unable to connect: ")
			print(e)
			exit()
		
		if src.CallbacksAndConfiguration.log:
			print ('connected')
		
		global socketThreadAlive
		socketThreadAlive = True
		readThread = threading.Thread(target=threadedRead, args=(s, ))		
		readThread.start()

		try:
			# update, recieve, and respond loop
			while socketThreadAlive:			
				while not cmdQueue.empty():
					cmd = cmdQueue.get()
					response = src.CommandParse.parseCommand(cmd, effectors)
					
					if response:
						try:
							s.sendall(response.encode())
						except Exception as e:
							socketThreadAlive = False
							print("Unable to send data: ")
							print(e)
							exit()


				# update all effectors
				for effector in list(effectors.values()):
					effector.update()

				src.CallbacksAndConfiguration.onMainLoop()
		except Exception as e:
			socketThreadAlive = False
			print(e)
			exit()