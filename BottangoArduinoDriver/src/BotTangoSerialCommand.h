#ifndef BotTangoSerialCommand_h
#define BotTangoSerialCommand_h

class BotTangoSerialCommand
{
public:
	BotTangoSerialCommand();
	BotTangoSerialCommand(const char *, void (*function)(char *[]));
	void excecute(char *args[]);

	const char *commandName;
	bool initialized = false;

private:
	void (*function)(char *[]);
};

#endif