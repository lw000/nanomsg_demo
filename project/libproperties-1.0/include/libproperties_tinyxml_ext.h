/*
  Project: libproperties
  Written by harite
  mailto:Harite.K@gmail.com
  date:2007/10/05
  libproperties's homepage:
    http://code.google.com/p/libproperties

  libproperties is released under the zlib license:

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.

*/

/*
  
  1.add a class TiXmlDoctype extend TiXmlComment date:2007/10/04
  2.add a class TiXmlDeclarationWithoutStandalone extend TiXmlDeclaration date:2007/10/04

	Because the class "Properties" in java need
	   <!DOCTYPE properties SYSTEM "http://java.sun.com/dtd/properties.dtd">
	   in xml file, its dtd is:

	   <?xml version="1.0" encoding="UTF-8"?>
             <!-- DTD for properties -->
             <!ELEMENT properties ( comment?, entry* ) >
             <!ATTLIST properties version CDATA #FIXED "1.0">
             <!ELEMENT comment (#PCDATA) >
             <!ELEMENT entry (#PCDATA) >
             <!ATTLIST entry key CDATA #REQUIRED>

	so if i want java class Properties can use the xml file which is created by libproperties,
	i must modify tinyxml.

!!!!!!!NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	1.The two class is JUST written for libproperties! 
	  If you don't know what you will do, please don't use them!
	
	2.The two class is OUT of tinyxml.
	  So you can use tinyxml in your wise.

	thank tinyxml :-)

*/
#ifndef LIBPROPERTIES_TINYXML_EXT_H
#define LIBPROPERTIES_TINYXML_EXT_H

#include "tinyxml.h"

/*
for libproperties
added by harite Harite.K@gmail.com
for libproperties project's debug
*/
// not used
//#define LIBPROPERTIES_DEBUG

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

//c++, ~~~
// not used
//class TiXmlParsingData;

/** 
    Class: TiXmlDeclarationWithoutStandalone
    Written by harite
    mailto:Harite.K@gmail.com
    date:2007/10/04
    for project libproperties on http://code.google.com/p/libproperties

    In correct XML,the declaration which is used by class Properties
    in java is the first entry in the file.
		<?xml version="1.0" encoding="UTF-8"?>

	In TinyXml, class TiXmlDeclaration has 3 possible attributes to 
	the declaration:version, encoding, and standalone.
	If i want tinyxml create a xml file with the declaration 
	"<?xml version="1.0" encoding="UTF-8"?>", i must kill god ^+^.
	So, I write a new class which is extend TiXmlDeclaration to do 
	this work.

	The implement of the class TiXmlDeclarationWithoutStandalone is simple.
	I just override some functions.

*/
class TiXmlDeclarationWithoutStandalone : public TiXmlDeclaration
{
public:
	TiXmlDeclarationWithoutStandalone()  : TiXmlDeclaration() {}

#ifdef TIXML_USE_STL
	TiXmlDeclarationWithoutStandalone(	const std::string& _version, const std::string& _encoding )
		: TiXmlDeclaration( _version, _encoding, "yes"//we don't use it
						  ){}
#endif

	TiXmlDeclarationWithoutStandalone(	const char* _version, const char* _encoding )
		: TiXmlDeclaration( _version, _encoding, "yes"//we don't use it
		                  ){}

	TiXmlDeclarationWithoutStandalone( const TiXmlDeclarationWithoutStandalone& copy )
		: TiXmlDeclaration( copy ){}

	virtual ~TiXmlDeclarationWithoutStandalone()	{}

	inline void operator=( const TiXmlDeclarationWithoutStandalone& copy )
	{
		Clear();
	    copy.CopyTo( this );
	}

	inline void Print( FILE* cfile, int /*not used*/ ) const
	{
		fprintf (cfile, "<?xml ");
		if ( this->Version()!=NULL )
		{fprintf (cfile, "version=\"%s\" ", this->Version());}
		if ( this->Encoding()!=NULL )
		{fprintf (cfile, "encoding=\"%s\"", this->Encoding());}
		fprintf (cfile, "?>");
	}
};


/** 
    Class: TiXmlDoctype
    Written by harite
    mailto:Harite.K@gmail.com
    date:2007/10/04
    for project libproperties on http://code.google.com/p/libproperties

    You can read "http://java.sun.com/dtd/properties.dtd" to know that 
	the class Properties in java need this DOCTYPE declaration
	<!DOCTYPE properties SYSTEM "http://java.sun.com/dtd/properties.dtd">
	in the xml files.
	First, i think that the class TiXmlUnknown in tinyxml can do it.But
	i am wrong, because TiXmlUnknown object can't be written into xml file.
	So, I write a new class which is extend TiXmlComment to do this work.

	The implement of the class TiXmlDoctype is simple.
	I just override some functions.

*/
/*
<!DOCTYPE value1 value2 ...>
*/
class TiXmlDoctype : public TiXmlComment
{
public:
	/// Constructs an empty comment.
	TiXmlDoctype() : TiXmlComment() {}

	TiXmlDoctype( const TiXmlDoctype& copy ) : TiXmlComment( copy ) { copy.CopyTo(this); }

	virtual ~TiXmlDoctype() {}

	inline void operator=( const TiXmlDoctype& base )
	{
		Clear();
		base.CopyTo(this);
	}

	inline void Print( FILE* cfile, int depth ) const
	{
		for ( int i=0; i<depth; i++ )
	    {
		    fputs( "    ", cfile );
	    }
	    fprintf( cfile, "<!DOCTYPE %s>", value.c_str() );
	}

	inline void StreamOut( TIXML_OSTREAM * stream ) const
	{
		(*stream) << "<!DOCTYPE ";
		(*stream) << value;
		(*stream) << ">";
	}

	/*
	  we don't use this functions.
	  Maybe they will exist in next version,but now they have errors.
	*/
	/*
	void CopyTo( TiXmlDoctype* target ) const
	{
		TiXmlNode::CopyTo( target );
	}*/

	/*
	TiXmlNode* Clone() const
	{
		TiXmlComment* clone = new TiXmlComment();

		if ( !clone )
			return 0;

		CopyTo( clone );
		return clone;
	}*/

	/*
	#ifdef TIXML_USE_STL
	void StreamIn( TIXML_ISTREAM * in, TIXML_STRING * tag )
	{
		while ( in->good() )
		{
			int c = in->get();	
			if ( c <= 0 )
			{
				TiXmlDocument* document = GetDocument();
				if ( document )
					document->SetError( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
				return;
			}
			
			(*tag) += (char) c;

			if ( c == '>' )//'>' in the end
			{
				// All is well.
				return;		
			}
		}
	}
    #endif
	*/

	/*
	const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
	{
		TiXmlDocument* document = GetDocument();
		value = "";
		p = SkipWhiteSpace( p, encoding );
		
		if ( data )
		{
			data->Stamp( p, encoding );
			location = data->Cursor();
		}
		const char* startTag = "<!DOCTYPE";
		const char* endTag   = ">";

		if ( !StringEqual( p, startTag, false, encoding ) )
		{
			document->SetError( TIXML_ERROR_PARSING_COMMENT, p, data, encoding );
			return 0;
		}
		p += strlen( startTag );
		p = ReadText( p, &value, false, endTag, false, encoding );
		return p;
	}
	*/

};


#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

