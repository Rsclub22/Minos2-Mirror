#include "VKMixer.h"

//===================================================================
VKMixer * VKMixer::currentMixer = nullptr;

/*static*/ VKMixer *VKMixer::GetVKMixer()
{
    if (!currentMixer)
    {
        currentMixer = new VKMixer();
    }
    return currentMixer;
}

VKMixer::VKMixer():
    CurrMixerSet(emsUnloaded)

{
}
VKMixer::~VKMixer()
{
}

eMixerSets VKMixer::GetCurrentMixerSet()
{
    return CurrMixerSet;
}

void VKMixer::SetCurrentMixerSet( eMixerSets cms )
{
    CurrMixerSet = cms;

}
