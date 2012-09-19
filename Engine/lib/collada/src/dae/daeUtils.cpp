#include <cstdarg>
#include <algorithm>
#include <iterator>
#include <dae/daeUtils.h>
#include <dae/daeURI.h>

#ifdef _WIN32
#include <direct.h>  // for getcwd (windows)
#else
#include <unistd.h>  // for getcwd (linux)
#endif

using namespace std;

cdom::systemType cdom::getSystemType() {
#ifdef WIN32
	return Windows;
#else
	return Posix;
#endif
}

string cdom::replace(const string& s, const string& replace, const string& replaceWith) {
	if (replace.empty())
		return s;

	string result;
	size_t pos1 = 0, pos2 = s.find(replace);
	while (pos2 != string::npos) {
		result += s.substr(pos1, pos2-pos1);
		result += replaceWith;
		pos1 = pos2 + replace.length();
		pos2 = s.find(replace, pos1);
	}

	result += s.substr(pos1, s.length()-pos1);
	return result;
}

void cdom::tokenize(const string& s,
                    const string& separators,
                    /* out */ list<string>& tokens,
                    bool separatorsInResult) {
	size_t currentIndex = 0, nextTokenIndex = 0;
	while (currentIndex < s.length() &&
	       (nextTokenIndex = s.find_first_of(separators, currentIndex)) != string::npos) {
		if ((nextTokenIndex - currentIndex) > 0)
			tokens.push_back(s.substr(currentIndex, nextTokenIndex-currentIndex));
		if (separatorsInResult)
			tokens.push_back(string(1, s[nextTokenIndex]));
		currentIndex = nextTokenIndex+1;
	}

	if (currentIndex < s.length())
		tokens.push_back(s.substr(currentIndex, s.length()-currentIndex));
}

list<string> cdom::tokenize(const string& s,
                            const string& separators,
                            bool separatorsInResult) {
	list<string> result;
	tokenize(s, separators, result, separatorsInResult);
	return result;
}

vector<string> cdom::makeStringArray(const char* s, ...) {
	va_list args;
	va_start(args, s);
	vector<string> result;
	while (s) {
		result.push_back(s);
		s = va_arg(args, const char*);
	}
	va_end(args);
	return result;
}

list<string> cdom::makeStringList(const char* s, ...) {
	va_list args;
	va_start(args, s);
	list<string> result;
	while (s) {
		result.push_back(s);
		s = va_arg(args, const char*);
	}
	va_end(args);
	return result;
}

string cdom::getCurrentDir() {
#if defined(__CELLOS_LV2__) || defined(_XBOX_VER)
	// The PS3 has no getcwd call.
	// !!!steveT Should we return app_home instead?
	return "/";
#else
	char buffer[1024];
#ifdef _WIN32
	_getcwd(buffer, 1024);
#else
	getcwd(buffer, 1024);
#endif
	return buffer;
#endif
}

string cdom::getCurrentDirAsUri() {
	string result = string("file://") + cdom::nativePathToUri(getCurrentDir());
	// Make sure the last char is a /
	if (!result.empty()  &&  result[result.length()-1] != '/')
		result += "/";
	return result;
}

int cdom::strcasecmp(const char* str1, const char* str2) {
#ifdef _MSC_VER
	return _stricmp(str1, str2);
#else
	return ::strcasecmp(str1, str2);
#endif
}

string cdom::tolower(const string& s) {
	string result;
	transform(s.begin(), s.end(), back_inserter(result), ::tolower);
	return result;
}
