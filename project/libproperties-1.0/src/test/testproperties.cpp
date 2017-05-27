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
  Use libproperties
*/
#include "libproperties.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	Properties p;
	p.setProperty("sid","harite");
	p.setProperty("port","1521");
	p.setProperty("userpassword","12y3_aer65");
	p.setProperty("host","127.0.0.1");
	p.setProperty("username","system");
	p.storeToXML("connectsetting.xml");
	p.clear();
	
	if(!p.loadFromXML("connectsetting.xml"))
	{
		cout << "falue" << endl;
	}
	else
	{
		for(Properties::const_iterator it = p.begin()
			; it!=p.end() ;
			++it)
		{
			cout << (*it).first << "-->" << (*it).second << endl;
		}
		cout << "use getProperty" << endl;
		cout << p.getProperty("sid", "") << endl;
		cout << p.getProperty("username", "") << endl;
		cout << p.getProperty("port", "") << endl;
		cout << p.getProperty("notexist", "defaultvalue") << endl;
		p.clear();
	}

	return 0;
}

