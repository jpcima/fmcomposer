#include "midi.h"
#include "../input/noteInput.hpp"
#include "../views/instrument/instrEditor.hpp"
#include <RtMidi.h>
#include <memory>


static int selectedMidiDevice;
static RtMidiIn* midiStream;
static std::vector<unsigned char> midiBuffer;
static vector<string> midiDeviceNames;
static int midiReady, midiReceive, midiPedal;
static const char midiClientName[] = "FMComposer";
static const char midiVirtualPortName[] = "Virtual port";
static RtMidi::Api midiApi = RtMidi::UNSPECIFIED;
static const unsigned midiBufferSize = 8192;

static void midi_handleEvent(const unsigned char midiMsg[3])
{
	if (!midiReceive)
		return;

	switch (midiMsg[0] / 16)
	{
	case 8: // note off
		if (!midiPedal) {
			if (midiMsg[0] % 16 < instrList->text.size())
				previewNoteStop(midiMsg[1], 1,midiMsg[0]%16);
		}
		break;
	case 9: // note on
		if (midiMsg[0] % 16 < instrList->text.size())
			previewNote(midiMsg[0]%16, midiMsg[1], midiMsg[2] / 1.282828, 1);
		break;
	case 0xB: // cc
		switch (midiMsg[1])
		{
		case 07:{ // channel vol
			int midich=midiMsg[0]%16;
			for (int i = 0; i < noteActive[midich].size(); i++)
			{
				fm_setChannelVolume(fm, noteActive[midich][i].fmChannel, midiMsg[1]/ 1.282828);
			}
			break;
		}
		case 0x08: // channel balance
		case 0x0A: // (10) channel panning
		{
			int midich=midiMsg[0]%16;
			for (int i = 0; i < noteActive[midich].size(); i++)
			{
				fm_setChannelPanning(fm, noteActive[midich][i].fmChannel, midiMsg[1]*2);
			}
		}
		break;
		case 64: // sustain pedal
			midiPedal = (midiMsg[2] > 63);
			if (!midiPedal)
			{
				previewNoteStopAll(midiMsg[0]%16);
			}
			break;
		case 0x78: // (120) all sound off
			fm_stopSound(fm);
			break;
		case 0x7B: // all notes off
			for (unsigned i = 0; i < FM_ch; i++)
			{
				fm_stopNote(fm, i);
			}
			break;
		}
		break;
	case 0xC: // Program Change
		instrList->select(midiMsg[1]);

		break;
	case 14: // Pitch Bend
		if (midiMsg[0] % 16 < instrList->text.size())
			previewNoteBend(fm, 2 * midiMsg[2] + (midiMsg[1] > 63), midiMsg[0]%16);
		break;

	}
}

void midi_getEvents()
{
	if (!midiReady)
		return;

	while (midiStream->getMessage(&midiBuffer), !midiBuffer.empty())
	{
		if (midiBuffer.size() < 3)
			midiBuffer.resize(3);
		midi_handleEvent(midiBuffer.data());
	}
}

void midi_selectDevice(int id)
{
	delete midiStream;
	midiStream = nullptr;
	midiReady = 0;

	if (id <= 0 || --id >= midiDeviceNames.size())
		return;

	try
	{
		midiStream = new RtMidiIn(midiApi, midiClientName, midiBufferSize);

		midiApi = midiStream->getCurrentApi();

		if (midiDeviceNames[id] == midiVirtualPortName)
		{
			midiStream->openVirtualPort();
			midiReady = 1;
		}
		else
		{
			for (unsigned i = 0, n = midiStream->getPortCount() && !midiReady; i < n; ++i)
			{
				std::string name = midiStream->getPortName(i);
				if (name == midiDeviceNames[id])
				{
					midiStream->openPort(i);
					midiReady = 1;
				}
			}
		}

		if (midiReady)
			fprintf(stderr, "Opened MIDI In: %s\n", midiDeviceNames[id].c_str());
		else
			fprintf(stderr, "Cannot open MIDI In: %s\n", midiDeviceNames[id].c_str());
	}
	catch (RtMidiError &err)
	{
		fprintf(stderr, "%s\n", err.getMessage().c_str());
	}
}

vector<string>* midi_refreshDevices()
{
	midiDeviceNames.clear();

	try
	{
		std::unique_ptr<RtMidiIn> midiTemp{
			new RtMidiIn(midiApi, midiClientName, midiBufferSize)};

		midiApi = midiTemp->getCurrentApi();

		midiDeviceNames.push_back(midiVirtualPortName);

		for (unsigned i = 0, n = midiTemp->getPortCount(); i < n; ++i)
		{
			std::string name = midiTemp->getPortName(i);
			if (!name.empty())
				midiDeviceNames.push_back(name);
		}
	}
	catch (RtMidiError &err)
	{
		fprintf(stderr, "%s\n", err.getMessage().c_str());
	}

	return &midiDeviceNames;
}

void midiReceiveEnable(int enabled)
{
	midiReceive = enabled;
}
