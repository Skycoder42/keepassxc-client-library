#include "securebytearray_p.h"
#include <sodium/utils.h>

#ifdef max
#undef max
#endif

SecureByteArray::SecureByteArray() = default;

SecureByteArray::SecureByteArray(size_t size, SecureByteArray::State state)
{
	reallocate(size, state);
}

SecureByteArray::SecureByteArray(const QByteArray &data, SecureByteArray::State state)
{
	reallocate(data.size(), State::Readwrite);
	memcpy(_data, data.constData(), _size);
	setState(state);
}

SecureByteArray::SecureByteArray(const SecureByteArray &other)
{
	reallocate(other._size, State::Readwrite);
	memcpy(_data, other._data, _size);
	setState(other._state);
}

SecureByteArray::SecureByteArray(SecureByteArray &&other) noexcept
{
	operator=(std::move(other));
}

SecureByteArray &SecureByteArray::operator=(const SecureByteArray &other)
{
	reallocate(other._size, State::Readwrite);
	memcpy(_data, other._data, _size);
	setState(other._state);
	return *this;
}

SecureByteArray &SecureByteArray::operator=(SecureByteArray &&other) noexcept
{
	std::swap(_data, other._data);
	std::swap(_size, other._size);
	std::swap(_state, other._state);
	return *this;
}

SecureByteArray::~SecureByteArray()
{
	deallocate();
}

QByteArray SecureByteArray::asByteArray() const
{
	if(_data) {
		Q_ASSERT(_size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return QByteArray::fromRawData(reinterpret_cast<char*>(_data), static_cast<int>(_size));
	} else
		return {};
}

SecureByteArray::operator QByteArray() const
{
	return asByteArray();
}

QByteArray SecureByteArray::copyToUnsafe() const
{
	if(_data) {
		Q_ASSERT(_size <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return QByteArray{reinterpret_cast<const char*>(_data), static_cast<int>(_size)};
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
	return !_data;
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
	if(_size == other._size) {
		if(_size == 0)
			return true;
		else
			return sodium_memcmp(_data, other._data, _size) == 0;
	} else
		return false;
}

bool SecureByteArray::operator!=(const SecureByteArray &other) const
{
	return !operator==(other);
}

size_t SecureByteArray::size() const
{
	return _size;
}

bool SecureByteArray::isEmpty() const
{
	return _size == 0;
}

SecureByteArray::State SecureByteArray::state() const
{
	return _state;
}

const quint8 *SecureByteArray::constData() const
{
	return _data;
}

const quint8 *SecureByteArray::data() const
{
	return _data;
}

quint8 *SecureByteArray::data()
{
	return _data;
}

void SecureByteArray::reallocate(size_t size, SecureByteArray::State state)
{
	deallocate();

	_data = reinterpret_cast<quint8*>(sodium_malloc(size));
	Q_ASSERT(_data);
	_size = size;
	_state = State::Readwrite;
	setState(state);
}

void SecureByteArray::deallocate()
{
	if(_data) {
		sodium_free(_data);
		_data = nullptr;
		_size = 0;
		_state = State::Unallocated;
	}
}

void SecureByteArray::increment(bool autoState)
{
	StateLocker _{this};
	if(autoState)
		setState(State::Readwrite);
	sodium_increment(_data, _size);
}

void SecureByteArray::add(const SecureByteArray &other, bool autoState)
{
	Q_ASSERT_X(_size == other._size, Q_FUNC_INFO, "To add two SecureByteArrays, they must be of the same size");
	StateLocker _{this};
	if(autoState)
		setState(State::Readwrite);
	sodium_add(_data, other._data, _size);
}

bool SecureByteArray::setState(SecureByteArray::State state)
{
	Q_ASSERT_X(state != State::Unallocated,
			   Q_FUNC_INFO,
			   "SecureByteArray::State::Unallocated cannot be set directly. "
			   "Use SecureByteArray::deallocate instead");
	Q_ASSERT_X(_state != State::Unallocated,
			   Q_FUNC_INFO,
			   "Cannot set change memory state if no memory has been allocated yet!");
	if(state == _state)
		return true;

	auto ok = false;
	switch (state) {
	case State::Noaccess:
		ok = sodium_mprotect_noaccess(_data) == 0;
		break;
	case State::Readonly:
		ok = sodium_mprotect_readonly(_data) == 0;
		break;
	case State::Readwrite:
		ok = sodium_mprotect_readwrite(_data) == 0;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	if(ok)
		_state = state;
	return ok;
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
