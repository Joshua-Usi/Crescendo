#pragma once

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    This tool uses RapidXML by Marcin Kalicinski.    *
 *        Though it is old, it still holds well        *
 *     <3 It took me a month to write my own parser    *
 *  (To be fair I didn't spend alot of my time on it,  * 
 *     and it wasn't even close to W3C compliant).     *
 *  But it was at this point I had decided reinventing *
 *          the wheel is such a waste of time!         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "core/core.h"

#include "datatypes/XMLDocument.h"

namespace Crescendo::Tools::XML
{
	/// <summary>
	/// Takes an XML string and creates a XML Node tree
	/// </summary>
	/// <param name="xmlDoc">A reference to the document to parse into</param>
	/// <param name="xmlString">A reference to the string to be parsed</param>
	void Parse(Document* xmlDoc, const char* xmlString);
	/// <summary>
	/// Parses an XML document directly from a file. Requires no explicit string declarations
	/// </summary>
	/// <param name="xmlDoc">A reference to the document to parse into</param>
	/// <param name="filePath">A file path to the file that is to be parsed</param>
	void ParseFromFile(Document* xmlDoc, const char* filePath);
	/// <summary>
	/// Takes an XML document and converts it into a serialised and saveable string
	/// </summary>
	/// <param name="xmlDoc">A reference of the XML document to build the serialised document</param>
	/// <param name="outputString">A reference to the string that will be output</param>
	void Stringify(Document* xmlDoc, std::string* outputString);
}