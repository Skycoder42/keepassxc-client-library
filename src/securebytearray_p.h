#ifndef SECUREBYTEARRAY_P_H
#define SECUREBYTEARRAY_P_H

#include "securebytearray.h"

class SecureByteArrayData : public QSharedData
{
public:
	quint8 *data = nullptr;
	size_t size = 0;
	SecureByteArray::State state = SecureByteArray::State::Unallocated;

	SecureByteArrayData() = default;
	SecureByteArrayData(const SecureByteArrayData &other);
	~SecureByteArrayData();

	void reallocate(size_t size, SecureByteArray::State state);
	void deallocate();
	bool setState(SecureByteArray::State newState);
};

#endif // SECUREBYTEARRAY_P_H
