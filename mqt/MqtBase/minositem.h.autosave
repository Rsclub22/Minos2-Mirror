#ifndef MINOSITEM_H
#define MINOSITEM_H
#include "XMPP_pch.h"

// templated class for items in contest and contact

template < class itemtype >
class MinosItem
{
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
      void setValue( itemtype t )
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
      void setInitialValue( itemtype t )
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
      MinosItem( const itemtype &d ) : val( d ), dirty( false )
      {}
      MinosItem<itemtype>(const MinosItem<itemtype> &s) : val(s.val), dirty(s.dirty)
      {}
      virtual ~MinosItem()
      {}
      MinosItem& operator = ( const MinosItem &rhs )
      {
          dirty = rhs.dirty;
          val = rhs.val;
          return *this;
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
    virtual void setValue( QString t )
    {
       t = t.trimmed();
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
    virtual void setInitialValue( QString t )
    {
       MinosItem<QString>::dirty = false;
       MinosItem<QString>::val = t;
    }
};
template < class Frequency >
class MinosFrequencyItem : public MinosItem<Frequency>
{
public:
    virtual void setValue( Frequency t )
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
