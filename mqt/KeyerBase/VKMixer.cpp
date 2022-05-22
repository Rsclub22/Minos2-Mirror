#include <QString>
#include "VKMixer.h"

//===================================================================
VKMixer * VKMixer::currentMixer = nullptr;

// texts for displaying the current mixer set

static const char *msets[emsMaxMixerSet] = {QT_TRANSLATE_NOOP("VoiceKeyer", "Unloaded")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "No PTT")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "PassThrough")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "Replay")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "Pip")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "Replay Tone1")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "Replay Tone2")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "Voice Record")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "CW Transmit")
                                            , QT_TRANSLATE_NOOP("VoiceKeyer", "CW PassThrough")
                                            };
static const char *levelLabels[emsMaxMixerSet] = {QT_TRANSLATE_NOOP("VoiceKeyer", "none"), QT_TRANSLATE_NOOP("VoiceKeyer", "none")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "input")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                  , QT_TRANSLATE_NOOP("VoiceKeyer", "output")
                                                };


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

eMixerSets VKMixer::GetCurrentMixerSet() const
{
    return CurrMixerSet;
}

void VKMixer::SetCurrentMixerSet( eMixerSets cms )
{
    CurrMixerSet = cms;

}
const char *VKMixer::getCurrentLevelText() const
{
    return levelLabels[CurrMixerSet];
}

const char *VKMixer::getCurrentMixerText() const
{
    return msets[CurrMixerSet];
}
