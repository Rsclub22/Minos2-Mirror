/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef XMPPRPCParamsH
#define XMPPRPCParamsH
#include <QString>
#include <QSharedPointer>
#include <QVector>
#include "tinyxml.h"
//---------------------------------------------------------------------------
// RPC parameter structures

class TiXmlElement;
class RPCParam
{
   public:
      QString name;

      RPCParam();
      virtual ~RPCParam();
      virtual void addParam( TiXmlElement &node ) const;
      virtual void addValue( TiXmlElement &node ) const;
      virtual void addNode( TiXmlElement &node ) const = 0;
      virtual QString print() const = 0;
      virtual QString analyse() const = 0;

      static QSharedPointer<RPCParam> paramFactory( TiXmlElement &node );

      virtual bool getMember( const QString &name, QSharedPointer<RPCParam> &p ) const;
      virtual bool getMember( int eleno, QSharedPointer<RPCParam> &p ) const;
      virtual bool getElements(int &size ) const;
      virtual bool getElement( int eleno, QSharedPointer<RPCParam> &p ) const;
      virtual bool getBoolean( bool &res ) const;
      virtual bool getInt( int &res ) const;
      virtual bool getDouble( double &res ) const;
      virtual bool getString( QString &res ) const;
      virtual bool getBase64( QString &res ) const;
      virtual bool getDtg( QString &res ) const;

      virtual void addMember( QSharedPointer<RPCParam> , const QString &name );
      virtual void addMember( int, const QString &name );
      virtual void addMember( bool, const QString &name );
      virtual void addMember( double, const QString &name );
      virtual void addMember( const QString &, const QString &name );
      virtual void addBase64Member( const QString &, const QString &name );
      virtual void addDtgMember( const QString &, const QString &name );
};

// structure
class RPCParamStruct: public RPCParam
{
      void addMember( RPCParam * , const QString &name );
   protected:
      QVector <QSharedPointer<RPCParam> > elements;
   public:
      RPCParamStruct();
      RPCParamStruct( TiXmlElement &sNode );
      virtual ~RPCParamStruct() override;

      void addMember( QSharedPointer<RPCParam> , const QString &name ) override;
      void addMember( int, const QString &name)  override;
      void addMember( bool, const QString &name ) override;
      void addMember( double, const QString &name ) override;
      void addMember( const QString &, const QString &name ) override;
      void addBase64Member( const QString &, const QString &name ) override;
      void addDtgMember( const QString &, const QString &name ) override;

      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getElements( int &size ) const override;
      virtual bool getElement( int eleno, QSharedPointer<RPCParam> &p ) const override;
      virtual bool getMember( const QString &name, QSharedPointer<RPCParam> &p ) const override;
      virtual bool getMember( int eleno, QSharedPointer<RPCParam> &p ) const override;
};

// array
class RPCParamArray: public RPCParam
{
      void addElement( RPCParam * );
   protected:
      QVector <QSharedPointer<RPCParam> > elements;
   public:
      RPCParamArray();
      RPCParamArray( TiXmlElement &sNode );
      virtual ~RPCParamArray() override;

      void addElement( QSharedPointer<RPCParam>  );

      void addElement( int );
      void addElement( bool );
      void addElement( double );
      void addElement( const char * );
      void AddBase64Element( const char * );
      void AddDtgElement( const char * );
      void addElement( const QString & );
      void AddBase64Element( const QString & );
      void AddDtgElement( const QString & );

      virtual void addNode( TiXmlElement &node ) const override;
      static QSharedPointer<RPCParamArray> ParseArray( TiXmlElement &aNode );
      virtual QString print() const override;
      virtual QString analyse() const override ;
      virtual bool getElements( int &size ) const override;
      virtual bool getElement(int eleno, QSharedPointer<RPCParam> &p ) const override;
};

// int or i4
class RPCIntParam: public RPCParam
{
   protected:
      int value;
   public:
      RPCIntParam( int v );
      RPCIntParam( TiXmlElement &sNode );
      RPCIntParam();
      virtual ~RPCIntParam() override;
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getInt( int &res ) const override;
};

// boolean
class RPCBooleanParam: public RPCParam
{
   protected:
      bool value;
   public:
      RPCBooleanParam( bool v );
      RPCBooleanParam( TiXmlElement &sNode );
      RPCBooleanParam();
      virtual ~RPCBooleanParam() override;
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getBoolean( bool &res ) const override;
};

// double
class RPCDoubleParam: public RPCParam
{
   protected:
      double value;
   public:
      RPCDoubleParam( double v );
      RPCDoubleParam( TiXmlElement &sNode );
      RPCDoubleParam();
      virtual ~RPCDoubleParam();
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getDouble( double &res ) const override;
};

// string
class RPCStringParam: public RPCParam
{
   protected:
      QString value;
   public:
      RPCStringParam( QString v );
      RPCStringParam( TiXmlElement &sNode );
      RPCStringParam();
      virtual ~RPCStringParam();
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getString( QString &res ) const override;
};

// <dateTime.iso8601>
class RPCDtgParam: public RPCParam
{
   protected:
      QString value;   // not we want eventually, but...
   public:
      RPCDtgParam( QString v );
      RPCDtgParam( TiXmlElement &sNode );
      RPCDtgParam();
      virtual ~RPCDtgParam();
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getDtg( QString &res ) const override;
};

// base64
class RPCBase64Param: public RPCParam
{
   protected:
      QString value;
   public:
      RPCBase64Param( QString v );
      RPCBase64Param( TiXmlElement &sNode );
      RPCBase64Param();
      virtual ~RPCBase64Param() override;
      virtual void addNode( TiXmlElement &node ) const override;
      virtual QString print() const override;
      virtual QString analyse() const override;

      virtual bool getBase64( QString &res ) const override;
};

class RPCArgs
{
      void addParam( RPCParam *  );
   public:
      RPCArgs();
      virtual ~RPCArgs();

      virtual QString PrintArgs();

      QVector <QSharedPointer<RPCParam> > args;
      TIXML_STRING UTF8XML; // pre-built params structure

      void addParam( QSharedPointer<RPCParam> );
      void addParam( int );
      void addParam( bool );
      void addParam( double );
      void addParam( const QString & );
      void addBase64Param( const QString & );
      void addDtgParam( const QString & );

      TiXmlElement * makeParamsNode(  );
      QString makeParamsString();
      void addParams( TiXmlElement &paramsNode );       // add args to the node

      bool parseParams( TIXML_STRING ); // parse from the string to args

      bool getStructArgMember(int argno, const QString &name, QSharedPointer<RPCParam>  &res );
      bool getStructArgMember(int argno, int eleno, QSharedPointer<RPCParam>  &res );
      bool getArrayArgElements(int argno, int &size );
      bool getArrayArgElement(int argno, int eleno, QSharedPointer<RPCParam> &res );

      bool getBooleanArg(int argno, bool &res );
      bool getIntArg( int argno, int &res );
      bool getDoubleArg(int argno, double &res );
      bool getStringArg( int argno, QString &res );
      bool getBase64Arg(int argno, QString &res );
      bool getDtgArg( int argno, QString &res );
};
Q_DECLARE_METATYPE(RPCArgs);
//---------------------------------------------------------------------------
#endif
