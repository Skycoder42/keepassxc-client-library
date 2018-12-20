#ifndef KPXCCLIENT_SODIUMCRYPTOR_P_H
#define KPXCCLIENT_SODIUMCRYPTOR_P_H

#include <tuple>

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include "securebytearray.h"

namespace KPXCClient {

class SodiumCryptor : public QObject
{
	Q_OBJECT

public:
	explicit SodiumCryptor(QObject *parent = nullptr);

	SecureByteArray generateRandom(size_t bytes, SecureByteArray::State state = SecureByteArray::State::Readwrite) const;
	SecureByteArray generateRandomNonce(SecureByteArray::State state = SecureByteArray::State::Readwrite) const;

	bool createKeys();
	void dropKeys();
	SecureByteArray publicKey() const;

	QByteArray encrypt(const QByteArray &plain,
					   const SecureByteArray &publicKey,
					   const SecureByteArray &nonce);
	QByteArray decrypt(const QByteArray &cipher,
					   const SecureByteArray &publicKey,
					   const SecureByteArray &nonce);

private:
	SecureByteArray _secretKey;
	SecureByteArray _publicKey;
};

}

#endif // KPXCCLIENT_SODIUMCRYPTOR_P_H
