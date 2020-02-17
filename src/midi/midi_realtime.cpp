#include "midi.h"
#include "../input/noteInput.hpp"
#include "../views/instrument/instrEditor.hpp"

PmTimestamp midithru_time_proc(void *info);


int selectedMidiDevice;
PortMidiStream* midiStream;
PmEvent midiBuffer[512];
PmTimestamp current_timestamp;
vector<int> midiDeviceIds;
vector<string> midiDeviceNames;
int midiReady, midiReceive, midiPedal;

static void midi_handleEvent(const unsigned char midiMsg[3])
{
	switch (midiMsg[0] / 16)
	{
	case 8: // note off
		if (!midiPedal) {
			if (midiMsg[1] % 16 < instrList->text.size())
				previewNoteStop(midiMsg[1], 1,midiMsg[0]%16);
		}
		break;
	case 9: // note on
		if (midiMsg[1] % 16 < instrList->text.size())
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
		if (midiMsg[1] % 16 < instrList->text.size())
			previewNoteBend(fm, 2 * midiMsg[2] + (midiMsg[1] > 63), midiMsg[0]%16);
		break;

	}
}

void midi_getEvents()
{
	if (midiReady && Pm_Poll(midiStream))
	{
		int count = Pm_Read(midiStream, midiBuffer, 512);
		if (midiReceive)
		{
			for (int i = 0; i < count; i++)
			{
				unsigned char midiMsg[3];
				midiMsg[0] = Pm_MessageStatus(midiBuffer[i].message);
				midiMsg[1] = Pm_MessageData1(midiBuffer[i].message);
				midiMsg[2] = Pm_MessageData2(midiBuffer[i].message);
				midi_handleEvent(midiMsg);
			}
		}
	}
}

void midi_selectDevice(int id)
{

	if (midiStream || id < 0)
	{
		Pm_Close(midiStream);
		midiReady = 0;
	}
	if (id >= 0)
	{
		Pm_OpenInput(&midiStream, id, 0, 32, 0, 0);
		Pm_SetFilter(midiStream, PM_FILT_ACTIVE | PM_FILT_SYSEX | PM_FILT_CLOCK);
		midiReady = 1;
	}
}

vector<string>* midi_refreshDevices()
{
	Pm_Terminate();
	Pm_Initialize();
	int nbdevices = Pm_CountDevices();

	midiDeviceIds.clear();
	midiDeviceNames.clear();
	int nbInputDevices = 0;
	for (int i = 0; i < nbdevices; i++)
	{
		const PmDeviceInfo *device = Pm_GetDeviceInfo(i);

		//midi input devices
		if (device->input)
		{
			midiDeviceNames.push_back(device->name);
			midiDeviceIds.push_back(i);
			nbInputDevices++;
		}
	}
	if (nbInputDevices == 1)
	{
		midi_selectDevice(midiDeviceIds[0]);
	}

	return &midiDeviceNames;
}

void midiReceiveEnable(int enabled)
{
	midiReceive = enabled;
}
