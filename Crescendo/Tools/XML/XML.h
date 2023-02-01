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

#include <vector>

#include "core/core.h"

#include "XMLDocument.h"

namespace Crescendo::Tools::XML
{
	void Parse(Document* xmlDoc, gt::string* xmlString);
	void ParseFromFile(Document* xmlDoc, gt::string filePath);
	
}