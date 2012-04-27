/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <unistd.h>
#include <malloc.h>
#include "SoundHandler.hpp"
#include "WavDecoder.hpp"
#include "AifDecoder.hpp"
#include "BNSDecoder.hpp"
#include "tools.h"

SoundHandler * SoundHandler::instance = NULL;

SoundHandler::SoundHandler()
{
	Decoding = false;
	ExitRequested = false;
	for(u32 i = 0; i < MAX_DECODERS; ++i)
		DecoderList[i] = NULL;

	ThreadStack = (u8 *) memalign(32, 32768);
	if(!ThreadStack)
		return;

	LWP_CreateThread(&SoundThread, UpdateThread, this, ThreadStack, 32768, 100);
}

SoundHandler::~SoundHandler()
{
	ExitRequested = true;
	ThreadSignal();
	LWP_JoinThread(SoundThread, NULL);
	SoundThread = LWP_THREAD_NULL;
	if(ThreadStack)
		free(ThreadStack);

	ClearDecoderList();
}

SoundHandler * SoundHandler::Instance()
{
	if (instance == NULL)
	{
		instance = new SoundHandler();
	}
	return instance;
}

void SoundHandler::DestroyInstance()
{
	if(instance)
	{
		delete instance;
	}
	instance = NULL;
}

void SoundHandler::AddDecoder(int voice, const u8 * snd, int len)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
		RemoveDecoder(voice);

	DecoderList[voice] = GetSoundDecoder(snd, len);
}

void SoundHandler::RemoveDecoder(int voice)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
		delete DecoderList[voice];

	DecoderList[voice] = NULL;
}

void SoundHandler::ClearDecoderList()
{
	for(u32 i = 0; i < MAX_DECODERS; ++i)
		RemoveDecoder(i);
}

SoundDecoder * SoundHandler::GetSoundDecoder(const u8 * sound, int length)
{
	const u8 * check = sound;
	int counter = 0;

	while(check[0] == 0 && counter < length)
	{
		check++;
		counter++;
	}

	if(counter >= length)
		return NULL;

	u32 * magic = (u32 *) check;

	if(magic[0] == MAKE_FOURCC('R','I','F','F'))
	{
		return new WavDecoder(sound, length);
	}
	else if(magic[0] == MAKE_FOURCC('B','N','S',' '))
	{
		return new BNSDecoder(sound, length);
	}
	else if(magic[0] == MAKE_FOURCC('F','O','R','M'))
	{
		return new AifDecoder(sound, length);
	}

	return new SoundDecoder(sound, length);
}

void * SoundHandler::UpdateThread(void *arg)
{
	((SoundHandler *) arg)->InternalSoundUpdates();
	return NULL;
}

void SoundHandler::InternalSoundUpdates()
{
	u16 i = 0;
	LWP_InitQueue(&ThreadQueue);
	while (!ExitRequested)
	{
		LWP_ThreadSleep(ThreadQueue);

		for(i = 0; i < MAX_DECODERS; ++i)
		{
			if(DecoderList[i] == NULL)
				continue;

			Decoding = true;
			DecoderList[i]->Decode();
		}
		Decoding = false;
	}
	LWP_CloseQueue(ThreadQueue);
	ThreadQueue = LWP_TQUEUE_NULL;
}
