#include "securebytearray.h"
#include "securebytearray_p.h"
#include <sodium/utils.h>

#ifdef max
#undef max
#endif

SecureByteArray::SecureByteArray() :
	d{new SecureByteArrayData{}}
{}

SecureByteArray::SecureByteArray(size_t size, SecureByteArray::State state) :
	SecureByteArray{}
{
	reallocate(size, state);
}

SecureByteArray::SecureByteArray(const QByteArray &data, SecureByteArray::State state) :
	SecureByteArray{}
{
	reallocate(data.size(), State::Readwrite);
	memcpy(d->data, data.constData(), d->size);
	setState(state);
}

SecureByteArray::SecureByteArray(const SecureByteArray &other) = default;

SecureByteArray::SecureByteArray(SecureByteArray &&other) noexcept = default;

SecureByteArray &SecureByteArray::operator=(const SecureByteArray &other) = default;

SecureByteArray &SecureByteArray::operator=(SecureByteArray &&other) noexcept = default;

SecureByteArray::~SecureByteArray() = default;

QByteArray SecureByteArray::asByteArray() const
{
	if(d->data) {
		Q_ASSERT(d->size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return QByteArray::fromRawData(reinterpret_cast<char*>(d->data), static_cast<int>(d->size));
	} else
		return {};
}

SecureByteArray::operator QByteArray() const
{
	return asByteArray();
}

QByteArray SecureByteArray::copyToUnsafe() const
{
	if(d->data) {
		Q_ASSERT(d->size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return QByteArray{reinterpret_cast<const char*>(d->data), static_cast<int>(d->size)};
	} else
		return {};
}

QString SecureByteArray::toBase64() const
{
	return QString::fromUtf8(asByteArray().toBase64());
}

SecureByteArray SecureByteArray::fromBase64(const QString &data, SecureByteArray::State state)
{
	return SecureByteArray{QByteArray::fromBase64(data.toUtf8()), state};
}

bool SecureByteArray::isNull() const
{
	return !d->data;
}

SecureByteArray::operator bool() const
{
	return !isNull();
}

bool SecureByteArray::operator!() const
{
	return isNull();
}

bool SecureByteArray::operator==(const SecureByteArray &other) const
{
	if(d->size == other.d->size) {
		if(d->size == 0)
			return true;
		else
			return sodium_memcmp(d->data, other.d->data, d->size) == 0;
	} else
		return false;
}

bool SecureByteArray::operator!=(const SecureByteArray &other) const
{
	return !operator==(other);
}

size_t SecureByteArray::size() const
{
	return d->size;
}

bool SecureByteArray::isEmpty() const
{
	return d->size == 0;
}

SecureByteArray::State SecureByteArray::state() const
{
	return d->state;
}

const quint8 *SecureByteArray::constData() const
{
	return d->data;
}

const quint8 *SecureByteArray::data() const
{
	return d->data;
}

quint8 *SecureByteArray::data()
{
	return d->data;
}

void SecureByteArray::reallocate(size_t size, SecureByteArray::State state)
{
	d->reallocate(size, state);
}

void SecureByteArray::deallocate()
{
	d->deallocate();
}

void SecureByteArray::increment(bool autoState)
{
	StateLocker _{this};
	if(autoState)
		setState(State::Readwrite);
	sodium_increment(d->data, d->size);
}

void SecureByteArray::add(const SecureByteArray &other, bool autoState)
{
	Q_ASSERT_X(d->size == other.d->size, Q_FUNC_INFO, "To add two SecureByteArrays, they must be of the same size");
	StateLocker _{this};
	if(autoState)
		setState(State::Readwrite);
	sodium_add(d->data, other.d->data, d->size);
}

bool SecureByteArray::setState(SecureByteArray::State state)
{
	return d->setState(state);
}

bool SecureByteArray::makeNoaccess()
{
	return setState(State::Noaccess);
}

bool SecureByteArray::makeReadonly()
{
	return setState(State::Readonly);
}

bool SecureByteArray::makeReadwrite()
{
	return setState(State::Readwrite);
}



SecureByteArray::StateLocker::StateLocker(SecureByteArray *data) :
	_data{data},
	_finalState{data->state()}
{}

SecureByteArray::StateLocker::StateLocker(SecureByteArray *data, SecureByteArray::State newState) :
	StateLocker{data, newState, data->state()}
{}

SecureByteArray::StateLocker::StateLocker(SecureByteArray *data, SecureByteArray::State newState, SecureByteArray::State finalState):
	_data{data},
	_finalState{finalState}
{
	_data->setState(newState);
}

SecureByteArray::StateLocker::~StateLocker()
{
	if(_data)
		_data->setState(_finalState);
}

void SecureByteArray::StateLocker::setFinalState(SecureByteArray::State finalState)
{
	_finalState = finalState;
}

void SecureByteArray::StateLocker::unlock()
{
	_data->setState(_finalState);
	_data = nullptr;
}



SecureByteArrayData::SecureByteArrayData(const SecureByteArrayData &other) :
	QSharedData{other}
{
	reallocate(other.size, SecureByteArray::State::Readwrite);
	memcpy(data, other.data, size);
	setState(other.state);
}

SecureByteArrayData::~SecureByteArrayData()
{
	deallocate();
}

void SecureByteArrayData::reallocate(size_t size, SecureByteArray::State state)
{
	deallocate();

	data = reinterpret_cast<quint8*>(sodium_malloc(size));
	Q_ASSERT(data);
	this->size = size;
	this->state = SecureByteArray::State::Readwrite;
	setState(state);
}

void SecureByteArrayData::deallocate()
{
	if(data) {
		sodium_free(data);
		data = nullptr;
		size = 0;
		state = SecureByteArray::State::Unallocated;
	}
}

bool SecureByteArrayData::setState(SecureByteArray::State newState)
{
	Q_ASSERT_X(newState != SecureByteArray::State::Unallocated,
			   Q_FUNC_INFO,
			   "SecureByteArray::State::Unallocated cannot be set directly. "
			   "Use SecureByteArray::deallocate instead");
	Q_ASSERT_X(state != SecureByteArray::State::Unallocated,
			   Q_FUNC_INFO,
			   "Cannot set change memory state if no memory has been allocated yet!");
	if(newState == state)
		return true;

	auto ok = false;
	switch (newState) {
	case SecureByteArray::State::Noaccess:
		ok = sodium_mprotect_noaccess(data) == 0;
		break;
	case SecureByteArray::State::Readonly:
		ok = sodium_mprotect_readonly(data) == 0;
		break;
	case SecureByteArray::State::Readwrite:
		ok = sodium_mprotect_readwrite(data) == 0;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	if(ok)
		state = newState;
	return ok;
}
