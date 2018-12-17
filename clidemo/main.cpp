#include <QCoreApplication>
#include <kpxcclient.h>

#include <QDebug>
#include <sodiumcryptor_p.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	KPXCClient::init();

	SodiumCryptor alice;
	qDebug() << "alice.createKeys" << alice.createKeys();
	const auto keys = alice.storeKeys();
	qDebug() << "alice.storeKeys" << keys.toHex();
	qDebug() << "alice.loadKeys" << alice.loadKeys(keys);

	SodiumCryptor bob;
	qDebug() << "bob.createKeys" << bob.createKeys();

	QByteArray message = "Hello World";
	auto [cipher, nonce] = alice.encrypt(message, bob.publicKey());
	auto recover = bob.decrypt(cipher, alice.publicKey(), nonce);
	qDebug() << "bob received message:" << recover;

	return 0;
}
