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
#include <string.h>
#include <malloc.h>
#include "gui_sound.h"
#include "SoundHandler.hpp"
#include "WavDecoder.hpp"
#include "lz77.h"
#include "tools.h"

#define MAX_SND_VOICES	  16

static bool VoiceUsed[MAX_SND_VOICES] =
{
	true, false, false, false, false, false,
	false, false, false, false, false, false,
	false, false, false, false
};

static inline int GetFirstUnusedVoice()
{
	for(int i = 1; i < MAX_SND_VOICES; i++)
	{
		if(VoiceUsed[i] == false)
			return i;
	}

	return -1;
}

extern "C" void SoundCallback(s32 voice)
{
	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return;

	if(decoder->IsBufferReady())
	{
		if(ASND_AddVoice(voice, decoder->GetBuffer(), decoder->GetBufferSize()) == SND_OK)
		{
			decoder->LoadNext();
			SoundHandler::Instance()->ThreadSignal();
		}
	}
	else if(decoder->IsEOF())
	{
		ASND_StopVoice(voice);
		//if(voice == 0)
			//MusicPlayer::Instance()->SetPlaybackFinished(true); //see if next music must be played
	}
	else
	{
		SoundHandler::Instance()->ThreadSignal();
	}
}

GuiSound::GuiSound(const u8 * snd, s32 len, int vol, bool isallocated, int v)
{
	sound = NULL;
	length = 0;
	if(v < 0)
		voice = GetFirstUnusedVoice();
	else
		voice = v;

	if(voice > 0)
		VoiceUsed[voice] = true;

	volume = vol;
	SoundEffectLength = 0;
	loop = 0;
	allocated = false;
	Load(snd, len, isallocated);
}

GuiSound::~GuiSound()
{
	FreeMemory();
	if(voice > 0)
		VoiceUsed[voice] = false;
}

void GuiSound::FreeMemory()
{
	this->Stop();

	SoundHandler::Instance()->RemoveDecoder(voice);

	if(allocated && sound != NULL)
	{
		free(sound);
		sound = NULL;
		allocated = false;
	}

	SoundEffectLength = 0;
}

bool GuiSound::Load(const u8 * snd, s32 len, bool isallocated)
{
	FreeMemory();

	if(!snd)
		return false;

	sound = (u8 *) snd;
	length = len;
	allocated = isallocated;

	SoundHandler::Instance()->AddDecoder(voice, sound, length);

	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return false;

	if(!decoder->IsBufferReady())
	{
		SoundHandler::Instance()->RemoveDecoder(voice);
		return false;
	}

	SetLoop(loop);

	return true;
}

void GuiSound::Play()
{
	if(IsPlaying())
		return;

	if(voice < 0 || voice >= 16)
		return;

	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return;

	if(decoder->IsEOF())
	{
		ASND_StopVoice(voice);
		decoder->ClearBuffer();
		decoder->Rewind();
		decoder->Decode();
	}

	u8 * curbuffer = decoder->GetBuffer();
	int bufsize = decoder->GetBufferSize();
	decoder->LoadNext();
	SoundHandler::Instance()->ThreadSignal();

	ASND_SetVoice(voice, decoder->GetFormat(), decoder->GetSampleRate(), 0, curbuffer, bufsize, volume, volume, SoundCallback);
}

void GuiSound::Stop()
{
	if(voice < 0 || voice >= 16)
		return;

	ASND_StopVoice(voice);

	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return;

	decoder->ClearBuffer();
	Rewind();
	SoundHandler::Instance()->ThreadSignal();
}

void GuiSound::Pause()
{
	if(voice < 0 || voice >= 16)
		return;

	ASND_StopVoice(voice);
}

void GuiSound::Resume()
{
	Play();
}

bool GuiSound::IsPlaying()
{
	if(voice < 0 || voice >= 16)
		return false;

	int result = ASND_StatusVoice(voice);

	if(result == SND_WORKING || result == SND_WAITING)
		return true;

	return false;
}

void GuiSound::SetVolume(int vol)
{
	if(voice < 0 || voice >= 16)
		return;

	if(vol < 0)
		return;

	volume = 255*(vol/100.0);
	ASND_ChangeVolumeVoice(voice, volume, volume);
}

void GuiSound::SetLoop(u8 l)
{
	loop = l;

	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return;

	decoder->SetLoop(l == 1);
}

void GuiSound::Rewind()
{
	SoundDecoder * decoder = SoundHandler::Instance()->Decoder(voice);
	if(!decoder)
		return;

	decoder->Rewind();
}
