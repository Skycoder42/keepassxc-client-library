#include "sodiumcryptor_p.h"
#include <sodium/crypto_box.h>
#include <sodium/utils.h>
#include <sodium/randombytes.h>

SodiumCryptor::SodiumCryptor(QObject *parent) :
	QObject{parent}
{}

SecureByteArray SodiumCryptor::generateRandom(size_t bytes, SecureByteArray::State state) const
{
	SecureByteArray data;
	data.reallocate(bytes);
	randombytes(data.data(), data.size());
	data.setState(state);
	return data;
}

SecureByteArray SodiumCryptor::generateRandomNonce(SecureByteArray::State state) const
{
	return generateRandom(crypto_box_NONCEBYTES, state);
}

bool SodiumCryptor::createKeys()
{
	_secretKey.reallocate(crypto_box_SECRETKEYBYTES);
	_publicKey.reallocate(crypto_box_PUBLICKEYBYTES);
	const auto ok = crypto_box_keypair(_publicKey.data(), _secretKey.data()) == 0;
	_secretKey.makeNoaccess();
	_publicKey.makeReadonly();
	return ok;
}

void SodiumCryptor::dropKeys()
{
	_secretKey.deallocate();
	_publicKey.deallocate();
}

SecureByteArray SodiumCryptor::publicKey() const
{
	return _publicKey;
}

QByteArray SodiumCryptor::encrypt(const QByteArray &plain, const SecureByteArray &publicKey, const SecureByteArray &nonce)
{
	QByteArray cipher{static_cast<int>(plain.size() + crypto_box_MACBYTES), Qt::Uninitialized};

	SecureByteArray::StateLocker _{&_secretKey, SecureByteArray::State::Readonly};
	const auto ok = crypto_box_easy(reinterpret_cast<quint8*>(cipher.data()),
									reinterpret_cast<const quint8*>(plain.constData()),
									plain.size(),
									nonce.constData(),
									publicKey.constData(),
									_secretKey.constData()) == 0;

	return ok ? cipher : QByteArray{};
}

QByteArray SodiumCryptor::decrypt(const QByteArray &cipher, const SecureByteArray &publicKey, const SecureByteArray &nonce)
{
	if(cipher.size() < static_cast<int>(crypto_box_MACBYTES))
		return {};

	QByteArray plain{static_cast<int>(cipher.size() - crypto_box_MACBYTES), Qt::Uninitialized};

	SecureByteArray::StateLocker _{&_secretKey, SecureByteArray::State::Readonly};
	const auto ok = crypto_box_open_easy(reinterpret_cast<quint8*>(plain.data()),
										 reinterpret_cast<const quint8*>(cipher.constData()),
										 cipher.size(),
										 nonce.constData(),
										 publicKey.constData(),
										 _secretKey.constData()) == 0;

	return ok ? plain : QByteArray{};
}
