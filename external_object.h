#ifndef EXTERNAL_OBJECT_H
#define EXTERNAL_OBJECT_H

namespace UL {

	struct ExternalObject {
		void *io_ptr;
		template <typename T> ExternalObject(T);
		template <typename T> T& get() const;
		Types type() const;
	};

} // UL

#endif