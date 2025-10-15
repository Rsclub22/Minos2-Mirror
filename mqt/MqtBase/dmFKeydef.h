#ifndef DMFKEYDEF_H
#define DMFKEYDEF_H
#include <QString>
#include <QVector>

// vector of (map(key current set name) of vector of pairs (fkey name, fkey message) )
// added message repeat enable and repeat interval duration
// QVector is 24 entries, F1-F12, F1-F12
/*struct KeyVal {
            QString fk;
            QString ktop;
            QString kval;
            int rigVoiceMemNum = 0;
            bool rptEnable = false;
            int rptDur = 0;
};*/

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

typedef QVector<KeyVal>KeySet;

struct ContestSection {
    KeySet run;
    KeySet sp;
};

typedef QMap<QString, ContestSection> RigMap;         // rigModel or "default"
typedef QMap<QString, RigMap> ContestMap;             // contest name
typedef QMap<QString, ContestMap> KeyerMap;           // keyer type


//typedef QMap<QString, KeySet > Keys;

#endif // DMFKEYDEF_H
