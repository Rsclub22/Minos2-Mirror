/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "XMPP_pch.h"
#include "tinyxml.h"
#include "TinyUtils.h"

//---------------------------------------------------------------------------
// RPC parameter structures
//---------------------------------------------------------------------------
RPCParam::RPCParam()
{}
RPCParam::~RPCParam()
{}
void RPCParam::addParam( TiXmlElement &node )
{
   TiXmlElement paramNode( "param" );
   addValue( paramNode );
   node.InsertEndChild( paramNode );
}
void RPCParam::addValue( TiXmlElement &node )
{
   TiXmlElement valueNode( "value" );
   addNode( valueNode );
   node.InsertEndChild( valueNode );
}
/*static*/ QSharedPointer<RPCParam> RPCParam::paramFactory(   TiXmlElement &node )
{
    // node points to the typed node we need to analyse

    if ( checkElementName( &node , "struct" ) )
    {
        RPCParam *p = new RPCParamStruct( node );
        return QSharedPointer<RPCParam>(p);
    }
    else
        if ( checkElementName( &node , "array" ) )
        {
            RPCParam *p = new RPCParamArray(node);
            return QSharedPointer<RPCParam>(p);
        }

        else
            if ( checkElementName( &node , "int" ) || checkElementName( &node , "i4" ) )
            {
                return QSharedPointer<RPCParam>(new RPCIntParam(node));
            }
            else
                if ( checkElementName( &node , "i8" ) )
                {
                    return QSharedPointer<RPCParam>(new RPCQint64Param(node));
                }
                else
                    if ( checkElementName( &node , "boolean" ) )
                    {
                        return QSharedPointer<RPCParam>(new RPCBooleanParam(node));
                    }
                    else
                        if ( checkElementName( &node , "double" ) )
                        {
                            return QSharedPointer<RPCParam>(new RPCDoubleParam(node));
                        }
                        else
                            if ( checkElementName( &node , "string" ) )
                            {
                                return QSharedPointer<RPCParam>(new RPCStringParam(node));
                            }
                            else
                                if ( checkElementName( &node , "base64" ) )
                                {
                                    return QSharedPointer<RPCParam>(new RPCBase64Param(node));
                                }
                                else
                                    if ( checkElementName( &node , "dateTime.iso8601" ) )
                                    {
                                        return QSharedPointer<RPCParam>(new RPCDtgParam(node));
                                    }
    return QSharedPointer<RPCParam>();
}
bool RPCParam::getBoolean( bool & )
{
   return false;
}
bool RPCParam::getInt(qint32 & )
{
   return false;
}
bool RPCParam::getQint64(qint64 & )
{
   return false;
}
bool RPCParam::getDouble( double & )
{
   return false;
}
bool RPCParam::getString( QString & )
{
   return false;
}
bool RPCParam::getBase64( QString & )
{
   return false;
}
bool RPCParam::getDtg( QString & )
{
   return false;
}
bool RPCParam::getMember( const QString &, QSharedPointer<RPCParam>& )
{
   return false;
}
bool RPCParam::getMember(int , QSharedPointer<RPCParam>& )
{
   return false;
}
bool RPCParam::getElements( int & )
{
   return false;
}
bool RPCParam::getElement(int , QSharedPointer<RPCParam>& )
{
   return false;
}
void RPCParam::addMember( QSharedPointer<RPCParam> , const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}

void RPCParam::addMember(qint32, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addMember(qint64, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addMember( bool, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addMember( double, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addMember( const QString &, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addBase64Member( const QString &, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
void RPCParam::addDtgMember( const QString &, const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// structure
RPCParamStruct::RPCParamStruct()
{}
RPCParamStruct::RPCParamStruct( TiXmlElement &aNode )
{
   // struct is list of members, each with name and value
   for ( TiXmlElement * e = aNode.FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      if ( checkElementName( e , "member" ) )
      {
         QString pname;
         TiXmlElement *valuenode = nullptr;
         for ( TiXmlElement * f = e->FirstChildElement(); f; f = f->NextSiblingElement() )
         {
            if ( checkElementName( f, "value" ) )
            {
               valuenode = f;
            }
            if ( checkElementName( f, "name" ) )
            {
               pname = f->GetText( );
            }
         }
         if ( valuenode && pname.size() )
         {
            TiXmlElement * vnode = valuenode->FirstChildElement();
            if ( vnode )
            {
               addMember( paramFactory( *vnode ), pname );
            }
         }
      }
   }
}
RPCParamStruct::~RPCParamStruct()
{
   elements.clear(); // clears the shared pointers
}

void RPCParamStruct::addMember( QSharedPointer<RPCParam>p, const QString &name )
{
   p->name = name;
   elements.push_back( p );
}
void RPCParamStruct::addMember(qint32 v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCIntParam( v )), name );
}
void RPCParamStruct::addMember(qint64 v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCQint64Param( v )), name );
}
void RPCParamStruct::addMember( bool v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCBooleanParam( v )), name );
}
void RPCParamStruct::addMember( double v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCDoubleParam( v )), name );
}
void RPCParamStruct::addMember( const QString &v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCStringParam( v )), name );
}
void RPCParamStruct::addBase64Member( const QString &v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCBase64Param( v )), name );
}
void RPCParamStruct::addDtgMember( const QString &v, const QString &name )
{
   addMember( QSharedPointer<RPCParam>(new RPCDtgParam( v )), name );
}

void RPCParamStruct::addNode( TiXmlElement &node )
{
   // struct
   // member
   // name

   // value as for individual
   TiXmlElement sNode( "struct" );
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      TiXmlElement mNode( "member" );
      TiXmlElement nNode( "name" );
      TiXmlText tNode( ( *i ) ->name.toStdString() );
      nNode.InsertEndChild( tNode );
      mNode.InsertEndChild( nNode );
      ( *i ) ->addValue( mNode );
      sNode.InsertEndChild( mNode );
   }
   node.InsertEndChild( sNode );

}
QString RPCParamStruct::print()
{
   QString s = "Struct\r\n";
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      s += "Member name " + ( *i ) ->name + "\r\n";
      s += ( *i ) ->print();
   }
   s += "End Struct\r\n";
   return s;
}
QString RPCParamStruct::analyse()
{
   QString s;
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      if (( *i ) ->name == "lseq" )
      {
         int n = 0;
         ( *i )->getInt(n);
         QString s1 = QString::number(n/65536);
         QString s2 = (n & 0x8000)?" 1":" 0";
         QString s3 = (n & 0x4000)?" 1":" 0";
         QString s4 = (n & 0x2000)?" 1":" 0";
         QString s5 = (n & 0x1000)?" 1":" 0";
         QString s6 = (n & 0x0800)?" 1":" 0";
         QString s7 = (n & 0x0400)?" 1":" 0";
         QString s8 = (n & 0x0200)?" 1":" 0";
         QString s9 = (n & 0x0100)?" 1":" 0";
         QString s10 = (n & 0x0080)?" 1":" 0";
         QString s11 = (n & 0x0040)?" 1":" 0";
         QString s12 = (n & 0x0020)?" 1":" 0";
         QString s13 = (n & 0x0010)?" 1":" 0";
         QString s14 = (n & 0x0008)?" 1":" 0";
         QString s15 = (n & 0x0004)?" 1":" 0";
         QString s16 = (n & 0x0002)?" 1":" 0";
         QString s17 = (n & 0x0001)?" 1":" 0";
         s += ( *i ) ->name + "<" + s1 + " : " + s2  + s3  + s4  + s5  + s6  + s7  + s8  + s9  + s10 + s11 + s12  + s13  + s14  + s15  + s16  + s17 + " (" +  ( *i ) ->analyse() + ")>";
      }
      else
      {
         s += ( *i ) ->name + "<" + ( *i ) ->analyse() + ">";
      }
   }
   return s;
}

bool RPCParamStruct::getMember( const QString &name, QSharedPointer<RPCParam>&p )
{
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      if ( ( *i ) ->name == name )
      {
         p = ( *i );
         return true;
      }
   }
   return false;
}
bool RPCParamStruct::getMember( int eleno, QSharedPointer<RPCParam>&p )
{
   if ( eleno < elements.size() )
   {
      p = elements[ eleno ];
      return true;
   }
   return false;
}
bool RPCParamStruct::getElements( int &size )
{
   size = elements.size();
   return true;
}
bool RPCParamStruct::getElement(int eleno, QSharedPointer<RPCParam>&p )
{
   if ( eleno < elements.size() )
   {
      p = elements[ eleno ];
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
// array - not being used at the moment...
RPCParamArray::RPCParamArray()
{}
RPCParamArray::RPCParamArray( TiXmlElement &aNode )
{
   // array is list of values
   // Array
   //    data
   //       value
   //       value
   //       value
   for ( TiXmlElement * e = aNode.FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      if ( checkElementName( e , "data" ) )
      {
         QString pname;
         for ( TiXmlElement * f = e->FirstChildElement(); f; f = f->NextSiblingElement() )
         {
            if ( checkElementName( f, "value" ) )
            {
               TiXmlElement * vnode = f->FirstChildElement();
               if ( vnode )
               {
                  addElement( paramFactory( *vnode ) );
               }
            }
         }
         break;
      }
   }

}
RPCParamArray::~RPCParamArray()
{
   elements.clear(); // clears shared pointers
}

void RPCParamArray::addElement( QSharedPointer<RPCParam> p )
{
   elements.push_back( p );
}
void RPCParamArray::addElement(qint32 v )
{
   addElement( QSharedPointer<RPCParam>(new RPCIntParam( v )) );
}
void RPCParamArray::addElement(qint64 v )
{
   addElement( QSharedPointer<RPCParam>(new RPCQint64Param( v )) );
}
void RPCParamArray::addElement( bool v )
{
   addElement( QSharedPointer<RPCParam>(new RPCBooleanParam( v )) );
}
void RPCParamArray::addElement( double v )
{
   addElement( QSharedPointer<RPCParam>(new RPCDoubleParam( v )) );
}
void RPCParamArray::addElement( const char *v )
{
   addElement( QSharedPointer<RPCParam>(new RPCStringParam( v )) );
}
void RPCParamArray::AddBase64Element( const char *v )
{
   addElement( QSharedPointer<RPCParam>(new RPCBase64Param( v )) );
}
void RPCParamArray::AddDtgElement( const char *v )
{
   addElement( QSharedPointer<RPCParam>(new RPCDtgParam( v )) );
}
void RPCParamArray::addElement( const QString &v )
{
   addElement( QSharedPointer<RPCParam>(new RPCStringParam( v )) );
}
void RPCParamArray::AddBase64Element( const QString &v )
{
   addElement( QSharedPointer<RPCParam>(new RPCBase64Param( v )) );
}
void RPCParamArray::AddDtgElement( const QString &v )
{
   addElement( QSharedPointer<RPCParam>(new RPCDtgParam( v )) );
}

void RPCParamArray::addNode( TiXmlElement &node )
{
   TiXmlElement aNode( "array" );
   TiXmlElement dNode( "data" );
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      ( *i ) ->addValue( dNode );
   }
   aNode.InsertEndChild( dNode );
   node.InsertEndChild( aNode );
}
QString RPCParamArray::print()
{
   QString s = "Array\r\n";
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      s += ( *i ) ->print();
   }
   s += "End Array\r\n";
   return s;
}
QString RPCParamArray::analyse()
{
   QString s = "[";
   for ( QVector<QSharedPointer<RPCParam> >::iterator i = elements.begin(); i != elements.end(); i++ )
   {
      s += ( *i ) ->analyse();
   }
   s += "]";
   return s;
}
bool RPCParamArray::getElements(int &size )
{
   size = elements.size();
   return true;
}
bool RPCParamArray::getElement(int eleno, QSharedPointer<RPCParam> &p )
{
   if ( elements.size() > eleno )
   {
      p = elements[ eleno ];
      return true;
   }
   return false;
}

//---------------------------------------------------------------------------
// int or i4
RPCIntParam::RPCIntParam( qint32 v ) : value( v )
{}
RPCIntParam::RPCIntParam() : value( 0 )
{}
RPCIntParam::RPCIntParam( TiXmlElement &aNode )
{
   const char * val = aNode.GetText();
   if ( val )
   {
      value = atoi( val );
   }
}
RPCIntParam::~RPCIntParam()
{}
void RPCIntParam::addNode( TiXmlElement &node )
{
   char buff[ 40 ];
   sprintf( buff, "%d", value );
   TiXmlElement vNode( "i4" );
   TiXmlText tNode( buff );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCIntParam::print()
{
   char buff[ 128 ];
   sprintf( buff, "Integer value %d\r\n", value );
   return QString( buff );
}
QString RPCIntParam::analyse()
{
   char buff[ 128 ];
   sprintf( buff, "%d", value );
   return QString( buff );
}
bool RPCIntParam::getInt(qint32 &res )
{
   res = value;
   return true;
}
//---------------------------------------------------------------------------
// long or i8
RPCQint64Param::RPCQint64Param( qint64 v ) : value( v )
{}
RPCQint64Param::RPCQint64Param() : value( 0 )
{}
RPCQint64Param::RPCQint64Param( TiXmlElement &aNode )
{
   const char * val = aNode.GetText();
   if ( val )
   {
      value = strtoll( val, nullptr, 10 );
   }
}
RPCQint64Param::~RPCQint64Param()
{}
void RPCQint64Param::addNode( TiXmlElement &node )
{
   char buff[ 40 ];
   sprintf( buff, "%lld", value );
   TiXmlElement vNode( "i8" );
   TiXmlText tNode( buff );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCQint64Param::print()
{
   char buff[ 128 ];
   sprintf( buff, "qint64 value %lld\r\n", value );
   return QString( buff );
}
QString RPCQint64Param::analyse()
{
   char buff[ 128 ];
   sprintf( buff, "%lld", value );
   return QString( buff );
}
bool RPCQint64Param::getQint64(qint64 &res )
{
   res = value;
   return true;
}


//---------------------------------------------------------------------------
// boolean
RPCBooleanParam::RPCBooleanParam( bool v ) : value( v )
{}
RPCBooleanParam::RPCBooleanParam() : value( false )
{}
RPCBooleanParam::RPCBooleanParam( TiXmlElement &aNode )
{
   const char * data = aNode.GetText();
   if ( data )
   {
      value = ( strlen( data ) && data[ 0 ] == '0' ) ? false : true;
   }
}
RPCBooleanParam::~RPCBooleanParam()
{}
void RPCBooleanParam::addNode( TiXmlElement &node )
{
   TiXmlElement vNode( "boolean" );
   TiXmlText tNode( value ? "1" : "0" );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );

}
QString RPCBooleanParam::print()
{
   if ( value )
      return "Boolean value true\r\n";
   return "Boolean value false\r\n";
}
QString RPCBooleanParam::analyse()
{
   if ( value )
      return "true";
   return "false";
}
bool RPCBooleanParam::getBoolean( bool &res )
{
   res = value;
   return true;
}

//---------------------------------------------------------------------------
// double
RPCDoubleParam::RPCDoubleParam( double v ) : value( v )
{}
RPCDoubleParam::RPCDoubleParam() : value( 0 )
{}
RPCDoubleParam::RPCDoubleParam( TiXmlElement &aNode )
{
   const char * data = aNode.GetText();
   if ( data )
   {
      char * endptr;
      value = strtod( data, &endptr );    // we may want to detect errors...
   }
}
RPCDoubleParam::~RPCDoubleParam()
{}
void RPCDoubleParam::addNode( TiXmlElement &node )
{
   TiXmlElement vNode( "double" );
   char buff[ 40 ];
   sprintf( buff, "%f", value );
   TiXmlText tNode( buff );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCDoubleParam::print()
{
   char buff[ 128 ];
   sprintf( buff, "double value %f\r\n", value );
   return QString( buff );
}
QString RPCDoubleParam::analyse()
{
   char buff[ 128 ];
   sprintf( buff, "%f", value );
   return QString( buff );
}
bool RPCDoubleParam::getDouble( double &res )
{
   res = value;
   return true;
}

//---------------------------------------------------------------------------
// string
RPCStringParam::RPCStringParam( QString v ) : value( v )
{}
RPCStringParam::RPCStringParam()
{}
RPCStringParam::RPCStringParam( TiXmlElement &aNode )
{
   const char * val = aNode.GetText();
   if ( val )
   {
      value = val;
   }
}
RPCStringParam::~RPCStringParam()
{}
void RPCStringParam::addNode( TiXmlElement &node )
{
   TiXmlElement vNode( "string" );

   TiXmlText tNode( value.toStdString().c_str() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCStringParam::print()
{
   QString p = "String value \"" + value + "\"\r\n";
   return p;
}
QString RPCStringParam::analyse()
{
   return "'" + value + "'";
}
bool RPCStringParam::getString( QString &res )
{
   res = value;
   return true;
}

//---------------------------------------------------------------------------
// <dateTime.iso8601>
RPCDtgParam::RPCDtgParam( QString v ) : value( v )
{}
RPCDtgParam::RPCDtgParam()
{}
RPCDtgParam::RPCDtgParam( TiXmlElement &aNode )
{
   const char * val = aNode.GetText();
   if ( val )
   {
      value = val;
   }
}
RPCDtgParam::~RPCDtgParam()
{}
void RPCDtgParam::addNode( TiXmlElement &node )
{
   TiXmlElement vNode( "dateTime.iso8601" );
   TiXmlText tNode( value.toStdString().c_str() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCDtgParam::print()
{
   return "dateTime.iso8601 value \"" + value + "\"\r\n";
}
QString RPCDtgParam::analyse()
{
   return value;
}
bool RPCDtgParam::getDtg( QString &res )
{
   res = value;
   return true;
}
//---------------------------------------------------------------------------
// base64
// This code assumes that we are base64'ing a string, rather than binary...
// NOT a correct assumption!

RPCBase64Param::RPCBase64Param( QString v ) : value( v )
{
   //   char * b64 = iks_base64_encode( v.c_str(), ( unsigned int ) ( v.length() ) );
   //   value = b64;
   //   iks_free( b64 );
}
RPCBase64Param::RPCBase64Param()
{}
RPCBase64Param::RPCBase64Param( TiXmlElement &aNode )
{

   const char * val = aNode.GetText();
   if ( val )
   {
      value = val;
      //   value = base64_decode( b64 );
   }

}
RPCBase64Param::~RPCBase64Param()
{}
void RPCBase64Param::addNode( TiXmlElement &node )
{
   TiXmlElement vNode( "base64" );
   TiXmlText tNode( value.toStdString().c_str() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCBase64Param::print()
{
   return "Base64 value \"" + value + "\"\r\n";
}
QString RPCBase64Param::analyse()
{
   return value;
}
bool RPCBase64Param::getBase64( QString &res )
{
   res = value;
   return true;
}
//---------------------------------------------------------------------------

RPCArgs::RPCArgs()
{}
RPCArgs::~RPCArgs()
{
   args.clear();// clears shared pointers
}
void RPCArgs::addParam( RPCParam *  )
{
   throw Exception("Add param by pointer to RPCArgs");
}
void RPCArgs::addParam( QSharedPointer<RPCParam> p )
{
   args.push_back( p );
}
void RPCArgs::addParam( qint32 v )
{
   addParam( QSharedPointer<RPCParam>(new RPCIntParam(v)));
}
void RPCArgs::addParam( qint64 v )
{
   addParam( QSharedPointer<RPCParam>(new RPCQint64Param(v)));
}
void RPCArgs::addParam( bool v )
{
   addParam( QSharedPointer<RPCParam>(new RPCBooleanParam( v )) );
}
void RPCArgs::addParam( double v )
{
   addParam( QSharedPointer<RPCParam>(new RPCDoubleParam( v )) );
}
void RPCArgs::addParam( const QString &v )
{
   addParam( QSharedPointer<RPCParam>(new RPCStringParam( v )) );
}
void RPCArgs::addBase64Param( const QString &v )
{
   addParam( QSharedPointer<RPCParam>(new RPCBase64Param( v )) );
}
void RPCArgs::addDtgParam( const QString &v )
{
   addParam( QSharedPointer<RPCParam>(new RPCDtgParam( v )) );
}

TiXmlElement *RPCArgs::makeParamsNode( )
{
   TiXmlElement * pNode = new TiXmlElement( "params" );

   for ( QVector<QSharedPointer<RPCParam> >::iterator i = args.begin(); i != args.end(); i++ )
   {
      ( *i ) ->addParam( *pNode );
   }

   return pNode;

}
QString RPCArgs::makeParamsString()
{
   TiXmlElement * params = makeParamsNode( );
   TIXML_STRING s;
   s << *params;
   delete params;
   return s.c_str();
}

void RPCArgs::addParams( TiXmlElement &paramsNode )
{
   if ( UTF8XML.length() )
   {
      TiXmlBase::SetCondenseWhiteSpace( false );
      TiXmlDocument paramdoc;
      paramdoc.Parse( UTF8XML.c_str(), nullptr );
      TiXmlElement *root = paramdoc.RootElement();
      paramsNode.InsertEndChild( *root );

   }
   else
   {
      TiXmlElement *pNode = makeParamsNode( );
      paramsNode.InsertEndChild( *pNode );     // which leaks pnode, as it gets cloned
      delete pNode;
   }
}
bool RPCArgs::parseParams( TIXML_STRING UTF8XML )   // parse from the string to args
{
   // And now we move to tinyXML for parsing...
   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument paramdoc;
   paramdoc.Parse( UTF8XML.c_str(), nullptr );
   if ( paramdoc.Error())
   {
       trace(QString("parse failed; ") + paramdoc.ErrorDesc());
       return false;
   }
   TiXmlElement *root = paramdoc.RootElement();
   if ( !root )
   {
      //      err = "No XML root element (does file exist?)";
      return false;
   }
   if ( !checkElementName( root, "params" ) )
   {
      //      err = "Invalid XML root element";
      return false;
   }
   for ( TiXmlElement * e = root->FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      // declaration, destination lines
      if ( checkElementName( e, "param" ) )
      {
         // value node is the child of <param />
         TiXmlElement * vnode = e->FirstChildElement();
         if ( vnode )
         {
            // type node is the child of <value />
            TiXmlElement * pnode = vnode->FirstChildElement();
            if ( pnode )
            {
               // and then pass the type node on to a param factory
               QSharedPointer<RPCParam> p = RPCParam::paramFactory( *pnode );
               // add each one to args
               if ( p )
                  args.push_back( p );
            }
         }
      }
   }
   return true;
}
QString RPCArgs::PrintArgs()
{
   QString s;

   for ( QVector<QSharedPointer<RPCParam> >::iterator i = args.begin(); i != args.end(); i++ )
   {
      s += ( *i ) ->print();
   }
   return s;
}
bool RPCArgs::getStructArgMember( int argno, const QString &name, QSharedPointer<RPCParam>&res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getMember( name, res );
   }
   return false;
}
bool RPCArgs::getStructArgMember(int argno, int eleno, QSharedPointer<RPCParam>&res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getMember( eleno, res );
   }
   return false;
}
bool RPCArgs::getArrayArgElements( int argno, int &size )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getElements( size );
   }
   return false;
}
bool RPCArgs::getArrayArgElement(int argno, int eleno, QSharedPointer<RPCParam>&res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getElement( eleno, res );
   }
   return false;
}
bool RPCArgs::getBooleanArg( int argno, bool &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getBoolean( res );
   }
   return false;
}
bool RPCArgs::getIntArg(int argno, qint32 &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getInt( res );
   }
   return false;
}
bool RPCArgs::getQint64Arg(int argno, qint64 &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getQint64( res );
   }
   return false;
}
bool RPCArgs::getDoubleArg( int argno, double &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getDouble( res );
   }
   return false;
}
bool RPCArgs::getStringArg(int argno, QString &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getString( res );
   }
   return false;
}
bool RPCArgs::getBase64Arg( int argno, QString &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getBase64( res );
   }
   return false;
}
bool RPCArgs::getDtgArg(int argno, QString &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getDtg( res );
   }
   return false;
}

