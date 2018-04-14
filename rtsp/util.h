#pragma once
#define PATTERN_SINGLETON_IMPLEMENT(className) \
className *className::instance = NULL;\
className *className::getInstance()\
{\
	if (!instance)\
	{\
		printf("im in new:%p\n", instance);\
		instance = new className();\
	}\
	return instance;\
}

#define PATTERN_SINGLETON_DECLARE(className)\
public:\
	static className *getInstance();\
private:\
	static className *instance;