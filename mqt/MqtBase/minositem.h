#ifndef MINOSITEM_H
#define MINOSITEM_H

#include "XMPPRPCParams.h"
#include <QString>

// templated class for items in contest and contact

template < class itemtype >
class MinosItem
{
private:
    virtual itemtype &operator=(const itemtype &r)
    {
        setValue(r);
        return val;
    }

   protected:
      bool dirty = false;
      itemtype val{};                          // C++ 11 default brace initialisation

   public:

      itemtype getValue() const
      {
         return val;
      }
      itemtype getValue( bool &d ) const
      {
         d |= dirty;
         return val;
      }
//      void setValue( itemtype t )
//      {
//         if ( val != t )        // so all item classes need != operator
//         {
//            dirty = true;
//            val = t;
//         }
//      }
      void setValue( const itemtype &t )
      {
         if ( val != t )        // so all item classes need != operator
         {
            dirty = true;
            val = t;
         }
      }
      void setValue( const MinosItem<itemtype> &t )
      {
         if ( val != t.getValue() )        // so all item classes need != operator
         {
            dirty = true;
            val = t.getValue();
         }
      }
      void setInitialValue( const itemtype &t )
      {
         dirty = false;
         val = t;
      }
      bool isDirty() const
      {
         return dirty;
      }
      void clearDirty()
      {
         dirty = false;
      }
      void setDirty()
      {
         dirty = true;
      }
      MinosItem()
      {}
      MinosItem( const itemtype &d ) : dirty( false ), val( d )
      {}
      MinosItem(const MinosItem &s) : dirty(s.dirty), val(s.val)
      {}
      virtual ~MinosItem()
      {}
      MinosItem& operator = ( const MinosItem &rhs )
      {
          dirty = rhs.dirty;
          val = rhs.val;
          return *this;
      }
      bool operator == ( const MinosItem &rhs )
      {
          return (rhs.val == val);
      }
      bool operator != ( const MinosItem &rhs )
      {
          return (rhs.val != val);
      }
      void addIfDirty( RPCParamStruct *st, const QString &stName, bool &d ) const
      {
         d |= isDirty();
         if ( isDirty() )
         {
            st->addMember( getValue(), stName );
         }
      }
};
template < class QString >
class MinosStringItem : public MinosItem<QString>
{
public:
    virtual void setValue( const QString &s )
    {
       QString t = s.trimmed();
       if ( MinosItem<QString>::val != t )        // so all item classes need != operator
       {
          MinosItem<QString>::dirty = true;
          MinosItem<QString>::val = t;
       }
    }
    void setValue( const MinosStringItem<QString> &t )
    {
       if ( MinosItem<QString>::val != t.getValue() )        // so all item classes need != operator
       {
          MinosItem<QString>::dirty = true;
          MinosItem<QString>::val = t.getValue();
       }
    }
    virtual void setInitialValue( const QString &t )
    {
       MinosItem<QString>::dirty = false;
       MinosItem<QString>::val = t;
    }
};
template < class Frequency >
class MinosFrequencyItem : public MinosItem<Frequency>
{
public:
    virtual void setValue( const Frequency &t )
    {
       if ( MinosItem<Frequency>::val != t )        // so all item classes need != operator
       {
          MinosItem<Frequency>::dirty = true;
          MinosItem<Frequency>::val = t;
       }
    }
    void setValue( const MinosFrequencyItem<Frequency> &t )
    {
       if ( MinosItem<Frequency>::val != t.getValue() )        // so all item classes need != operator
       {
          MinosItem<Frequency>::dirty = true;
          MinosItem<Frequency>::val = t.getValue();
       }
    }
    virtual void setInitialValue( const Frequency &t )
    {
       MinosItem<Frequency>::dirty = false;
       MinosItem<Frequency>::val = t;
    }
    void addIfDirty( RPCParamStruct *st, const QString &stName, bool &d ) const
    {
       d |= this->isDirty();
       if ( this->isDirty() )
       {
          st->addMember( this->getValue().str(), stName );
       }
    }
};


#endif // MINOSITEM_H
