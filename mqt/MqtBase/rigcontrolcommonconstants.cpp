#include "rigcontrolcommonconstants.h"

const char * RIG_STATUS_CONNECTED = "Connected";
const char * RIG_STATUS_DISCONNECTED = "Disconnected";
const char * RIG_STATUS_ERROR = "Error";
const char * RIG_SWITCH_COMPLETED = "Radio Switch Complete";    // This doesn't need to be translated.


PresetFreq::PresetFreq()
{

};

PresetFreq::~PresetFreq()
{

};

void PresetFreq::clear()
{
    modePresetFreqList.clear();
}

Frequency PresetFreq::getPresetFreq(const QString mode, const QString band)
{
   QMap<QString, StoredPresetFreqs>*  mspf = modePresetFreqList[mode];
   return mspf->value(band).presetFreq;
}


Frequency PresetFreq::getLastFreq(const QString mode, const QString band)
{

    QMap<QString, StoredPresetFreqs>*  mspf = modePresetFreqList[mode];
    return mspf->value(band).lastFreq;
}



void PresetFreq::setPresetFreq(const QString mode, const QString band, const QString freq)
{
     QMap<QString, StoredPresetFreqs>* mspf = nullptr;

     if (mode == freqPresetData::PRESET_MODE_CW)
     {
         mspf = &cwFreqPresets;
     }
     else if (mode == freqPresetData::PRESET_MODE_PHONE)
     {
         mspf = &phoneFreqPresets;
     }
     else if (mode == freqPresetData::PRESET_MODE_MGM)
     {
         mspf = &mgmFreqPresets;
     }


     if (mspf)
     {
         StoredPresetFreqs spf{};
         spf = mspf->value(band);
         spf.presetFreq = Frequency(freq);
         mspf->insert(band, spf);
         modePresetFreqList.insert(mode, mspf);
     }



}

void PresetFreq::setPresetFreq(const QString mode, const QString band, const Frequency freq)
{
    QMap<QString, StoredPresetFreqs>* mspf = nullptr;

    if (mode == freqPresetData::PRESET_MODE_CW)
    {
        mspf = &cwFreqPresets;
    }
    else if (mode == freqPresetData::PRESET_MODE_PHONE)
    {
        mspf = &phoneFreqPresets;
    }
    else if (mode == freqPresetData::PRESET_MODE_MGM)
    {
        mspf = &mgmFreqPresets;
    }


    if (mspf)
    {

       StoredPresetFreqs spf{};
       spf = mspf->value(band);
       spf.presetFreq = freq;

       mspf->insert(band, spf);

       modePresetFreqList.insert(mode, mspf);
    }

}


void PresetFreq::setLastFreq(const QString mode, const QString band, const QString freq)
{
     if(!mode.isEmpty() ||!band.isEmpty())
     {
         QMap<QString, StoredPresetFreqs>* mspf = nullptr;

         if (mode == freqPresetData::PRESET_MODE_CW)
         {
             mspf = &cwFreqPresets;
         }
         else if (mode == freqPresetData::PRESET_MODE_PHONE)
         {
             mspf = &phoneFreqPresets;
         }
         else if (mode == freqPresetData::PRESET_MODE_MGM)
         {
             mspf = &mgmFreqPresets;
         }

         if (mspf)
         {
            StoredPresetFreqs spf;
            spf = mspf->value(band);
            spf.lastFreq = Frequency(freq);

            mspf->insert(band, spf);
         }


        modePresetFreqList.insert(mode, mspf);
     }

}

void PresetFreq::setLastFreq(const QString mode, const QString band, const Frequency freq)
{
    if (!mode.isEmpty() && !band.isEmpty())
    {
        QMap<QString, StoredPresetFreqs>* mspf = nullptr;

        if (mode == freqPresetData::PRESET_MODE_CW)
        {
            mspf = &cwFreqPresets;
        }
        else if (mode == freqPresetData::PRESET_MODE_PHONE)
        {
            mspf = &phoneFreqPresets;
        }
        else if (mode == freqPresetData::PRESET_MODE_MGM)
        {
            mspf = &mgmFreqPresets;
        }

        if (mspf)
        {
            StoredPresetFreqs spf;
            spf = mspf->value(band);
            spf.lastFreq = freq;

            mspf->insert(band, spf);
        }


       modePresetFreqList.insert(mode, mspf);
    }


}

void PresetFreq::copyAllPrevFreqToLastFreqByMode(const QString mode, const QStringList &listOfBands)
{
    for (auto &b:listOfBands)
    {
        Frequency f = getPresetFreq(mode, b);
        setLastFreq(mode, b, f);
    }
}



bool PresetFreq::contains(QString mode, QString band)
{
    bool modeTrue = modePresetFreqList.contains(mode);
    if (modeTrue)
    {
        if (mode == freqPresetData::PRESET_MODE_CW)
        {
            return modeTrue && cwFreqPresets.contains(band);
        }
        else if (mode == freqPresetData::PRESET_MODE_PHONE)
        {
            return modeTrue && phoneFreqPresets.contains(band);
        }
        else if (mode == freqPresetData::PRESET_MODE_MGM)
        {
            return modeTrue && mgmFreqPresets.contains(band);
        }
    }

    return modeTrue;
}

void PresetFreq::readSettings(const QStringList &listOfBands)
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    clear();


    config.beginGroup(freqPresetData::PRESET_MODE_CW);

    for (int i = 0; i < listOfBands.count(); i++)
    {
        setPresetFreq(freqPresetData::PRESET_MODE_CW, listOfBands[i], Frequency(config.value(listOfBands[i], freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();

    config.beginGroup(freqPresetData::PRESET_MODE_PHONE);

    for (int i = 0; i < listOfBands.count(); i++)
    {
        setPresetFreq(freqPresetData::PRESET_MODE_PHONE, listOfBands[i], Frequency(config.value(listOfBands[i], freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();

    config.beginGroup(freqPresetData::PRESET_MODE_MGM);

    for (int i = 0; i < listOfBands.count(); i++)
    {
        setPresetFreq(freqPresetData::PRESET_MODE_MGM, listOfBands[i], Frequency(config.value(listOfBands[i], freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();


}


