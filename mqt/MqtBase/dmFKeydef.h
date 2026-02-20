#ifndef DMFKEYDEF_H
#define DMFKEYDEF_H
#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QJsonObject>

#include "txkeyerCommonConstants.h"

using namespace TxKeyerCommon;

// defines fields saved to KeyVal json file
inline const QMap<TxKeyerId, QSet<QString>> keyerFieldMap = {
    { TxKeyerId::CW_RigControl, { "key", "label", "message", "messageDuration", "messageDurationEnable", "repeatDuration", "repeatEnable"}},
    { TxKeyerId::RigControl, { "key", "label", "messageDuration", "messageDurationEnable", "repeatDuration", "repeatEnable", "rigVoiceMemNum"}},
    { TxKeyerId::PcCwKeyer, { "key", "label", "message", "messageDuration", "messageDurationEnable", "repeatDuration", "repeatEnable"}},
    { TxKeyerId::DigitalModes, { "key", "label", "message"}}
};

class KeyVal {
public:
    KeyVal() = default;

    QString fk() const {return m_fk;}
    QString ktop() const {return m_ktop;}
    QString kval() const {return m_kval;}

    int rigVoiceMemNum() const {return m_rigVoiceMemNum;}
    bool rptEnable() const {return m_rptEnable;}
    int rptDur() const {return m_rptDur;}
    bool msgDurEnable() const {return m_msgDurEnable;}
    int msgDur() const {return m_msgDur;}

    void setFk(const QString &val) { if (m_fk != val) { m_fk = val; m_dirty = true; } }
    void setKtop(const QString &val) { if (m_ktop != val) { m_ktop = val; m_dirty = true; } }
    void setKval(const QString &val) { if (m_kval != val) { m_kval = val; m_dirty = true; } }
    void setRigVoiceMemNum(int val) { if (m_rigVoiceMemNum != val) { m_rigVoiceMemNum = val; m_dirty = true; } }
    void setRptEnable(bool val) { if (m_rptEnable != val) { m_rptEnable = val; m_dirty = true; } }
    void setRptDur(int val) { if (m_rptDur != val) { m_rptDur = val; m_dirty = true; } }
    void setMsgDurEnable(bool val) { if (m_msgDurEnable != val) { m_msgDurEnable = val; m_dirty = true; } }
    void setMsgDur(int val) { if (m_msgDur != val) { m_msgDur = val; m_dirty = true; } }


    void clearDirty() { m_dirty = false; }
    bool isDirty() const { return m_dirty; }

    bool operator==(const KeyVal &other) const
    {
        return m_fk == other.m_fk &&
               m_ktop == other.m_ktop &&
               m_kval == other.m_kval &&
               m_rigVoiceMemNum == other.m_rigVoiceMemNum &&
               m_rptEnable == other.m_rptEnable &&
               m_rptDur == other.m_rptDur &&
               m_msgDur == other.m_msgDur &&
               m_msgDurEnable == other.m_msgDurEnable;

    }

    void swapWith(KeyVal &other)
    {
        using std::swap;

        bool changed = false;

        if (m_fk != other.m_fk)
        {
            swap(m_fk, other.m_fk);
            changed = true;
        }
        if (m_ktop != other.m_ktop)
        {
            swap(m_ktop, other.m_ktop);
            changed = true;
        }
        if (m_kval != other.m_kval)
        {
            swap(m_kval, other.m_kval);
            changed = true;
        }
        if (m_rigVoiceMemNum != other.m_rigVoiceMemNum)
        {
            swap(m_rigVoiceMemNum, other.m_rigVoiceMemNum);
            changed = true;
        }
        if (m_rptEnable != other.m_rptEnable)
        {
            swap(m_rptEnable, other.m_rptEnable);
            changed = true;
        }
        if (m_rptDur != other.m_rptDur)
        {
            swap(m_rptDur, other.m_rptDur);
            changed = true;
        }
        if (m_msgDurEnable != other.m_msgDurEnable)
        {
            swap(m_msgDurEnable, other.m_msgDurEnable);
            changed = true;
        }
        if (m_msgDur != other.m_msgDur)
        {
            swap(m_msgDur, other.m_msgDur);
            changed = true;
        }



        if (changed)
        {
            m_dirty = true;
            other.m_dirty = true;
        }
    }

    QJsonObject toJson(const TxKeyerId &keyerId) const
    {
        QJsonObject obj;
        const auto fields = keyerFieldMap.value(keyerId);

        if (fields.contains("key")) obj["key"] = m_fk;
        if (fields.contains("label")) obj["label"] = m_ktop;
        if (fields.contains("message")) obj["message"] = m_kval;
        if (fields.contains("messageDuration")) obj["messageDuration"] = m_msgDur;
        if (fields.contains("messageDurationEnable")) obj["messageDurationEnable"] = m_msgDurEnable;
        if (fields.contains("repeatDuration")) obj["repeatDuration"] =  m_rptDur;
        if (fields.contains("repeatEnable")) obj["repeatEnable"] = m_rptEnable;
        if (fields.contains("rigVoiceMemNum")) obj["rigVoiceMemNum"] = m_rigVoiceMemNum;

        return obj;
    }

    void fromJson(const QJsonObject &obj)
    {
        m_fk = obj.value("key").toString();
        m_ktop = obj.value("label").toString();
        m_kval = obj.value("message").toString();

        // Use default values if fields are missing
        m_msgDur = obj.value("messageDuration").toInt(0);
        m_msgDurEnable = obj.value("messageDurationEnable").toBool(false);
        m_rptDur = obj.value("repeatDuration").toInt(0);
        m_rptEnable = obj.value("repeatEnable").toBool(false);
        m_rigVoiceMemNum = obj.value("rigVoiceMemNum").toInt(0);

        m_dirty = false;
    }

private:
    QString m_fk;
    QString m_ktop;
    QString m_kval;
    int m_rigVoiceMemNum = 0;
    bool m_msgDurEnable = false;
    int m_msgDur = 0;
    bool m_rptEnable = false;
    int m_rptDur = 0;
    bool m_dirty = false;
};


class CommonVal {

public:

    CommonVal() = default;

    int numButtons() const {return m_numButtons;}
    int endOfMessageType() const {return  m_endOfMessageType;}
    bool useCatForEom() const {return m_useCatForEom;}
    bool switchToCwMode() const {return  m_switchToCwMode;}

    void setNumButtons(int numButtons) { if (m_numButtons != numButtons){m_numButtons = numButtons; m_dirty = true;}}
    void setEndOfMessageType(int eomType) { if (m_endOfMessageType != eomType){m_endOfMessageType = eomType; m_dirty = true;}}
    void setUseCatForEom(bool useCatForEom) { if (m_useCatForEom != useCatForEom){m_useCatForEom = useCatForEom; m_dirty = true;}}
    void setSwitchToCwMode(bool switchToCwMode) {if (m_switchToCwMode != switchToCwMode){m_switchToCwMode = switchToCwMode; m_dirty = true;} }

    void clearDirty() { m_dirty = false; }
    bool isDirty() const { return m_dirty; }

    bool operator==(const CommonVal &other) const
    {
        return m_numButtons == other.m_numButtons &&
               m_endOfMessageType == other.m_endOfMessageType &&
               m_useCatForEom == other.m_useCatForEom &&
               m_switchToCwMode == other.m_switchToCwMode;

    }

    void swapWith(CommonVal &other)
    {
        using std::swap;

        bool changed = false;



        if (m_numButtons != other.m_numButtons)
        {
            swap(m_numButtons, other.m_numButtons);
            changed = true;
        }

        if (m_endOfMessageType != other.m_endOfMessageType)
        {
            swap(m_endOfMessageType, other.m_endOfMessageType);
            changed = true;
        }

        if (m_useCatForEom != other.m_useCatForEom)
        {
            swap(m_useCatForEom, other.m_useCatForEom);
            changed = true;
        }

        if (m_switchToCwMode != other.m_switchToCwMode)
        {
            swap(m_switchToCwMode, other.m_switchToCwMode);
            changed = true;
        }

        if (changed)
        {
            m_dirty = true;
            other.m_dirty = true;
        }
    }

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["numButtons"] = m_numButtons;
        obj["endOfMessageType"] = m_endOfMessageType;
        obj["useCatForEom"] = m_useCatForEom;
        obj["switchToCwMode"] = m_switchToCwMode;
        return obj;
    }

    void fromJson(const QJsonObject &obj)
    {
        m_numButtons = obj.value("numButtons").toInt(12);
        m_endOfMessageType = obj.value("endOfMessageType").toInt(0);
        m_useCatForEom = obj.value("useCatForEom").toBool(false);
        m_switchToCwMode = obj.value("switchToCwMode").toBool(true);

        m_dirty = false;
    }

private:

    int m_numButtons = 12;
    int m_endOfMessageType = 0;
    bool m_useCatForEom = false;
    bool m_switchToCwMode = true;
    bool m_dirty = false;
};



typedef QVector<KeyVal>KeySet;

struct ContestSection {
    KeySet run;
    KeySet sp;
    CommonVal common;
};

typedef QMap<QString, ContestSection> RigMap;         // rigModel or "default"
typedef QMap<QString, RigMap> ContestMap;             // contest name
typedef QMap<QString, ContestMap> KeyerMap;           // keyer type


//typedef QMap<QString, KeySet > Keys;

#endif // DMFKEYDEF_H
