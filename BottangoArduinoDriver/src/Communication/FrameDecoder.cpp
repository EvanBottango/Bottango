// 
// 
// 

#include "FrameDecoder.h"


bool FrameDecoder::hasFrame()
{
	return validFrameAvailable;
}

bool FrameDecoder::tryConsumeFrame(char** out)
{
	if (validFrameAvailable)
	{
		out = splitCommandBuffer;
		return true;
	}

	return false;
}