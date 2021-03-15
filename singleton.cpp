#ifndef SINGLETON_CPP
#define SINGLETON_CPP

class Singleton {

};

bool operator ==(const Singleton& o1, const Singleton& o2) {
	return &o1 == &o2;
}

#endif
