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
void RPCParam::addParam( TiXmlElement &node ) const
{
   TiXmlElement paramNode( "param" );
   addValue( paramNode );
   node.InsertEndChild( paramNode );
}
void RPCParam::addValue( TiXmlElement &node ) const
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
bool RPCParam::getBoolean( bool & ) const
{
   return false;
}
bool RPCParam::getInt( int & ) const
{
   return false;
}
bool RPCParam::getDouble( double & ) const
{
   return false;
}
bool RPCParam::getString( QString & ) const
{
   return false;
}
bool RPCParam::getBase64( QString & ) const
{
   return false;
}
bool RPCParam::getDtg( QString & ) const
{
   return false;
}
bool RPCParam::getMember( const QString &, QSharedPointer<RPCParam>& ) const
{
   return false;
}
bool RPCParam::getMember(int , QSharedPointer<RPCParam>& ) const
{
   return false;
}
bool RPCParam::getElements( int & ) const
{
   return false;
}
bool RPCParam::getElement(int , QSharedPointer<RPCParam>& ) const
{
   return false;
}
void RPCParam::addMember( QSharedPointer<RPCParam> , const QString &/*name*/ )
{
   throw Exception("addMember to non-struct RPCParam");
}

void RPCParam::addMember( int, const QString &/*name*/ )
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

void RPCParamStruct::addMember( QSharedPointer<RPCParam>p, const QString &mname )
{
   p->name = mname;
   elements.push_back( p );
}
void RPCParamStruct::addMember( int v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCIntParam( v )), mname );
}
void RPCParamStruct::addMember( bool v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCBooleanParam( v )), mname );
}
void RPCParamStruct::addMember( double v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCDoubleParam( v )), mname );
}
void RPCParamStruct::addMember( const QString &v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCStringParam( v )), mname );
}
void RPCParamStruct::addBase64Member( const QString &v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCBase64Param( v )), mname );
}
void RPCParamStruct::addDtgMember( const QString &v, const QString &mname )
{
   addMember( QSharedPointer<RPCParam>(new RPCDtgParam( v )), mname );
}

void RPCParamStruct::addNode( TiXmlElement &node ) const
{
   // struct
   // member
   // name

   // value as for individual
   TiXmlElement sNode( "struct" );
   for ( auto const &i: elements )
   {
      TiXmlElement mNode( "member" );
      TiXmlElement nNode( "name" );
      TiXmlText tNode( i->name.toStdString() );
      nNode.InsertEndChild( tNode );
      mNode.InsertEndChild( nNode );
      i->addValue( mNode );
      sNode.InsertEndChild( mNode );
   }
   node.InsertEndChild( sNode );

}
QString RPCParamStruct::print() const
{
   QString s = "Struct\r\n";
   for ( auto const &i: elements )
   {
      s += "Member name " + i->name + "\r\n";
      s += i->print();
   }
   s += "End Struct\r\n";
   return s;
}
QString RPCParamStruct::analyse() const
{
   QString s;
   for ( auto const &i: elements )
   {
      if (i->name == "lseq" )
      {
         int n = 0;
         i->getInt(n);
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
         s += i->name + "<" + s1 + " : " + s2  + s3  + s4  + s5  + s6  + s7  + s8  + s9  + s10 + s11 + s12  + s13  + s14  + s15  + s16  + s17 + " (" +  i->analyse() + ")> ";
      }
      else
      {
         s += i->name + "<" + i->analyse() + "> ";
      }
   }
   return s;
}

bool RPCParamStruct::getMember( const QString &mname, QSharedPointer<RPCParam>&p ) const
{
    for ( auto const &i: elements )
   {
      if ( i->name == mname )
      {
         p = i;
         return true;
      }
   }
   return false;
}
bool RPCParamStruct::getMember( int eleno, QSharedPointer<RPCParam>&p ) const
{
   if ( eleno < elements.size() )
   {
      p = elements[ eleno ];
      return true;
   }
   return false;
}
bool RPCParamStruct::getElements( int &size ) const
{
   size = elements.size();
   return true;
}
bool RPCParamStruct::getElement(int eleno, QSharedPointer<RPCParam>&p ) const
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
void RPCParamArray::addElement( int v )
{
   addElement( QSharedPointer<RPCParam>(new RPCIntParam( v )) );
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

void RPCParamArray::addNode( TiXmlElement &node ) const
{
   TiXmlElement aNode( "array" );
   TiXmlElement dNode( "data" );
   for ( auto const &i: elements )
   {
      i->addValue( dNode );
   }
   aNode.InsertEndChild( dNode );
   node.InsertEndChild( aNode );
}
QString RPCParamArray::print() const
{
   QString s = "Array\r\n";
   for ( auto const &i: elements )
   {
      s += i->print();
   }
   s += "End Array\r\n";
   return s;
}
QString RPCParamArray::analyse() const
{
   QString s = "[";
   for ( auto const &i: elements )
   {
      s += i->analyse();
   }
   s += "]";
   return s;
}
bool RPCParamArray::getElements(int &size ) const
{
   size = elements.size();
   return true;
}
bool RPCParamArray::getElement(int eleno, QSharedPointer<RPCParam> &p ) const
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
RPCIntParam::RPCIntParam( int v ) : value( v )
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
void RPCIntParam::addNode( TiXmlElement &node ) const
{
   char buff[ 40 ];
   sprintf( buff, "%d", value );
   TiXmlElement vNode( "i4" );
   TiXmlText tNode( buff );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCIntParam::print() const
{
   char buff[ 128 ];
   sprintf( buff, "Integer value %d\r\n", value );
   return QString( buff );
}
QString RPCIntParam::analyse() const
{
   char buff[ 128 ];
   sprintf( buff, "%d", value );
   return QString( buff );
}
bool RPCIntParam::getInt( int &res ) const
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
void RPCBooleanParam::addNode( TiXmlElement &node ) const
{
   TiXmlElement vNode( "boolean" );
   TiXmlText tNode( value ? "1" : "0" );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );

}
QString RPCBooleanParam::print() const
{
   if ( value )
      return "Boolean value true\r\n";
   return "Boolean value false\r\n";
}
QString RPCBooleanParam::analyse() const
{
   if ( value )
      return "true";
   return "false";
}
bool RPCBooleanParam::getBoolean( bool &res ) const
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
void RPCDoubleParam::addNode( TiXmlElement &node ) const
{
   TiXmlElement vNode( "double" );
   char buff[ 40 ];
   sprintf( buff, "%f", value );
   TiXmlText tNode( buff );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCDoubleParam::print() const
{
   char buff[ 128 ];
   sprintf( buff, "double value %f\r\n", value );
   return QString( buff );
}
QString RPCDoubleParam::analyse() const
{
   char buff[ 128 ];
   sprintf( buff, "%f", value );
   return QString( buff );
}
bool RPCDoubleParam::getDouble( double &res ) const
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
void RPCStringParam::addNode( TiXmlElement &node ) const
{
   TiXmlElement vNode( "string" );

   TiXmlText tNode( value.toStdString() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCStringParam::print() const
{
   QString p = "String value \"" + value + "\"\r\n";
   return p;
}
QString RPCStringParam::analyse() const
{
   return "'" + value + "'";
}
bool RPCStringParam::getString( QString &res ) const
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
void RPCDtgParam::addNode( TiXmlElement &node ) const
{
   TiXmlElement vNode( "dateTime.iso8601" );
   TiXmlText tNode( value.toStdString() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCDtgParam::print() const
{
   return "dateTime.iso8601 value \"" + value + "\"\r\n";
}
QString RPCDtgParam::analyse() const
{
   return value;
}
bool RPCDtgParam::getDtg( QString &res ) const
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
void RPCBase64Param::addNode( TiXmlElement &node ) const
{
   TiXmlElement vNode( "base64" );
   TiXmlText tNode( value.toStdString() );
   vNode.InsertEndChild( tNode );
   node.InsertEndChild( vNode );
}
QString RPCBase64Param::print() const
{
   return "Base64 value \"" + value + "\"\r\n";
}
QString RPCBase64Param::analyse() const
{
   return value;
}
bool RPCBase64Param::getBase64( QString &res ) const
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
void RPCArgs::addParam( int v )
{
   addParam( QSharedPointer<RPCParam>(new RPCIntParam(v)));
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

   for ( auto const &i: args )
   {
      i->addParam( *pNode );
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

   for ( auto const &i: args )
   {
      s += i->print();
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
bool RPCArgs::getIntArg(int argno, int &res )
{
   if ( args.size() > argno )
   {
      return args[ argno ] ->getInt( res );
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

