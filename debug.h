#include <string>

char _GetFromStream(InputStream * stream, std::string & debug_str)
{
    char t = GetFromStream(stream);
    debug_str += t;
    return t;
}