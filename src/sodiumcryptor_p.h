#ifndef SODIUMCRYPTOR_P_H
#define SODIUMCRYPTOR_P_H

#include <tuple>

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include "securebytearray_p.h"

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

	std::pair<QByteArray, SecureByteArray> encrypt(const QByteArray &plainText,
												   const SecureByteArray &publicKey); // (ciphertext, nonce)
	QByteArray decrypt(const QByteArray &cipher,
					   const SecureByteArray &publicKey,
					   const SecureByteArray &nonce);

private:
	SecureByteArray _secretKey;
	SecureByteArray _publicKey;
};

#endif // SODIUMCRYPTOR_P_H
