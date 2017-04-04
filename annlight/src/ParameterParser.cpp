#include "ParameterParser.h"
#include <sstream>
#include <fstream>
#include "ParserException.h"

const std::string ParameterParser::NO_DEFAULT_VALUE_STRING = "_#N/A#_";
const std::string ParameterParser::DEFAULT_COMMENT_STRING = "#";

/**
 * @brief Standard constructor for the <code>ParameterParser</code> class.
 */
ParameterParser::ParameterParser()
{
  this->commentString = ParameterParser::DEFAULT_COMMENT_STRING;
}

/**
 * @brief Standard destructor.
 */
ParameterParser::~ParameterParser()
{
  this->nameValueMap.clear();
}

/**
 * @brief Adds a new parameter to the pool of parameters known to the current <code>ParameterParser</code> instance.
 *
 * @param name The name of the parameter to add (the value can be fetched using that name)
 * @param defaultValue Optional parameter giving the a possible default value for the given parameter
 * @throw ParserException If a parameter with the given name already exists
 */
void ParameterParser::addParameter(std::string name, std::string defaultValue)
{
  std::map<std::string, std::string>::iterator nameFound = this->nameValueMap.find(name);
  if (nameFound != this->nameValueMap.end())
  {
    std::ostringstream errorMessage;
    errorMessage << "Parameter \"" << name << "\" already exists!";
    throw ParserException(errorMessage.str().c_str());
  }

  if (defaultValue == "")
    this->nameValueMap.insert(make_pair(name, ParameterParser::NO_DEFAULT_VALUE_STRING));
  else
    this->nameValueMap.insert(make_pair(name, defaultValue));
}

/**
 * Everything in a line that is beyond this string is ignored.
 * The default comment string used by the <code>ParameterParser</code> class is the one equal to the <code>ParameterParser::DEFAULT_COMMENT_STRING</code>
 * member variable.
 * @brief Allows the user to specify a string representing the begin of a command line.
 *
 * @param commentString The new comment string
 */
void ParameterParser::setCommentString(std::string commentString)
{
  this->commentString = commentString;
}

/**
 * @brief Reads the values of known parameters from a file.
 *
 * @param filename The name of the parameter file to read
 * @throw ParserException If the configuration file can not be opened or contains synthax errors
 */
void ParameterParser::readParameters(std::string filename)
{
  std::ifstream inFile(filename.c_str(), std::fstream::in);
  if (!inFile.is_open())
  {
    std::ostringstream errorMessage;
    errorMessage << "Could not open file \"" << filename << "\" for reading!";
    throw ParserException(errorMessage.str().c_str());
  }

  std::string line;
  unsigned lineCount = 1;
  while (getline(inFile, line, '\n'))
  {
    std::string::size_type commentStart = line.find(this->commentString, 0);
    std::string lineToParse = line.substr(0, commentStart);
    if (lineToParse.size() == 0)
    {
      lineCount++;
      continue;
    }
    std::string::size_type nameEnd = lineToParse.find(' ');
    if (nameEnd == std::string::npos)
    {
      std::ostringstream errorMessage;
      errorMessage << "Found parameter without value in line " << lineCount << " of configuration file \"" << filename << "\"!";
      throw ParserException(errorMessage.str().c_str());
    }
    std::string name = lineToParse.substr(0, nameEnd);
    std::string value = lineToParse.substr(nameEnd+1);
    this->trim(name);
    this->trim(value);
    if (value == "")
    {
      std::ostringstream errorMessage;
      errorMessage << "Found identifier without value in line " << lineCount << " of configuration file \"" << filename << "\"!";
      throw ParserException(errorMessage.str().c_str());
    }

    std::map<std::string, std::string>::iterator found = this->nameValueMap.find(name);
    if (found == this->nameValueMap.end())
    {
      std::cout << "Unknown parameter identifier \"" << name << "\" will be ignored!" << std::endl;
    }
    else
    {
      found->second = value;
    }
    lineCount++;
  }

  inFile.close();
}

/**
 * @brief Writes a default parameter file if <code>readParameters</code> has not been called yet or the current configuration otherwise.
 *
 * @param filename The name of the configuration file
 * @throw ParserException If the parameter file can not be created
 */
void ParameterParser::writeParameterFile(std::string filename)
{
  std::ofstream outFile(filename.c_str(), std::fstream::out | std::fstream::trunc);
  if (!outFile.is_open())
  {
    std::stringstream errorMessage;
    errorMessage << "Could not open file \"" << filename << "\" for writing!";
    throw ParserException(errorMessage.str().c_str());
  }
  outFile << this->commentString << " Default config file generated by ParameterParser" << std::endl;

  for (std::map<std::string, std::string>::iterator it = this->nameValueMap.begin(); it != this->nameValueMap.end(); it++)
  {
    if (it->second != ParameterParser::NO_DEFAULT_VALUE_STRING) outFile << it->first << " " << it->second << std::endl;
  }

  outFile.close();
}

/**
 * @brief Writes the current parameter values to a given stream.
 *
 * @param stream The stream to which the data should be written
 */
void ParameterParser::writeCurrentParameters(std::ostream& stream)
{
  stream << "Current parameters:" << std::endl;
  for (std::map<std::string, std::string>::iterator it = this->nameValueMap.begin(); it != this->nameValueMap.end(); it++)
  {
    if (it->second != ParameterParser::NO_DEFAULT_VALUE_STRING)
      stream << it->first << " = " << it->second << std::endl;
    else
      stream << it->first << " <no value set>" << std::endl;
  }
}

/**
 * @brief Gets the value of a parameter as a string.
 *
 * @param name The name of the parameter for which to get the value
 * @return The value of the requested parameter as a string
 * @throw ParserException If the parameter requested is unknown or has no value
 */
std::string ParameterParser::getParameterValue(std::string name) const
{
  std::map<std::string, std::string>::const_iterator found = this->nameValueMap.find(name);
  if (found == this->nameValueMap.end())
  {
    std::ostringstream errorMessage;
    errorMessage << "Unknown parameter name: \"" << name << "\"";
    throw ParserException(errorMessage.str().c_str());
  }

  std::string value = found->second;
  if (value == ParameterParser::NO_DEFAULT_VALUE_STRING)
  {
    std::ostringstream errorMessage;
    errorMessage << "No value for parameter \"" << name << "\" read and no default value defined.";
    throw ParserException(errorMessage.str().c_str());
  }

  return value;
}

/**
 * @brief Trims a given string removing all leading and trailing whitespaces.
 *
 * @param str The string to trim
 * @return A reference to the trimmed string
 */
std::string& ParameterParser::trim(std::string& str) const
{
  unsigned i = 0;
  while (i < str.length() && str[i] == ' ')
  {
    str.erase(i, 1);
    i++;
  }

  i = str.length() - 1;
  while (i >= 0 && str[i] == ' ')
  {
    str.erase(i, 1);
    i--;
  }

  return str;
}
