#ifndef SECUREBYTEARRAY_H
#define SECUREBYTEARRAY_H

#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QSharedDataPointer>

#include "kpxcclient_global.h"

class SecureByteArrayData;
class SecureByteArray //TODO refactor name or use namespace
{
	Q_GADGET

public:
	enum class State {
		Noaccess,
		Readonly,
		Readwrite,

		Unallocated
	};
	Q_ENUM(State)

	class StateLocker {
		Q_DISABLE_COPY(StateLocker)
	public:
		StateLocker(SecureByteArray *data);
		StateLocker(SecureByteArray *data, State newState);
		StateLocker(SecureByteArray *data, State newState, State finalState);
		~StateLocker();

		void setFinalState(State finalState);

		void unlock();

	private:
		SecureByteArray *_data;
		State _finalState;
	};

	SecureByteArray();
	SecureByteArray(size_t size, State state = State::Readwrite);
	SecureByteArray(const QByteArray &data, State state = State::Readwrite);
	SecureByteArray(const SecureByteArray &other);
	SecureByteArray(SecureByteArray &&other) noexcept;
	SecureByteArray &operator=(const SecureByteArray &other);
	SecureByteArray &operator=(SecureByteArray &&other) noexcept;
	~SecureByteArray();

	QByteArray asByteArray() const;
	operator QByteArray() const;
	QByteArray copyToUnsafe() const;

	QString toBase64() const;
	static SecureByteArray fromBase64(const QString &data, State state = State::Readwrite);

	bool isNull() const;
	explicit operator bool() const;
	bool operator!() const;

	bool operator==(const SecureByteArray &other) const;
	bool operator!=(const SecureByteArray &other) const;

	size_t size() const;
	bool isEmpty() const;
	State state() const;
	const quint8 *constData() const;
	const quint8 *data() const;
	quint8 *data();

	void reallocate(size_t size, State state = State::Readwrite);
	void deallocate();

	void increment(bool autoState = false);
	void add(const SecureByteArray &other, bool autoState = false);

	bool setState(State state);
	bool makeNoaccess();
	bool makeReadonly();
	bool makeReadwrite();

private:
	QSharedDataPointer<SecureByteArrayData> d;
};

#endif // SECUREBYTEARRAY_H
