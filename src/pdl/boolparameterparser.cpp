#include "boolparameterparser.h"

using namespace std;


BoolParameterParser::Data::Data(bool v, const std::string& d)
: ParserDataBase(d), value(v)
{}

void BoolParameterParser::Data::cppAddHeader(std::set<std::string>& headers) const
{
  headers.insert("\"base/parameters/simpleparameter.h\"");
}


std::string BoolParameterParser::Data::cppType(const std::string&) const
{
  return "bool";
}

std::string BoolParameterParser::Data::cppParamType(const std::string&) const
{
  return "insight::BoolParameter";
}

std::string BoolParameterParser::Data::cppValueRep(const std::string&, const std::string& thisscope) const
{
  return boost::lexical_cast<std::string>(value);
}
