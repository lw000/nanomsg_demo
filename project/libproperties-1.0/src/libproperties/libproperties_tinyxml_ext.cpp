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


#include "libproperties_tinyxml_ext.h"


