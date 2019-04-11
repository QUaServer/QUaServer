### How Validation Works

Validation means that the client can be sure that it is connecting to the server it wants to connect, and not some other server posing as the real one, in an attempt to steal sensitive information. For example, if a malicious server poses as the real one, it can ask the client for passwords, credit card details, etc. Then use that information to access the real server and compromise it.

Validation of the server is based on [Public Key Infrasctructure (PKI)](https://en.wikipedia.org/wiki/Public_key_infrastructure). This document is no place to explain the whole details of PKI, but a minimal working set of concepts will do to get validation working.

* A [Cryptographic Hash](https://en.wikipedia.org/wiki/Cryptographic_hash_function) is a function that takes any data as input and creates a *fingerprint* of the data. Just as real finger prints, the *Hash* cannot be used to reconstruct the original data (or the person), but is useful to **identify** that such *fingerprint* belongs to that data (or that person).

* PKI makes use of [Asymmetric Encryption](https://www.youtube.com/watch?v=AQDCe585Lnc), which basically means that an entity creates for itself a **pair** of keys, a **public** key and a **private** key. The most important concept to understand is that *data encrypted with the **private** key can only be decrypted with the **public** key, and data encrypted with the **public** key can only be encrypted with the **private** key*. *Public* keys are meant to be send around to third parties, while *private* keys are meant to be stored securely by the owner.

* An [SSL Certificate](https://en.wikipedia.org/wiki/Public_key_certificate) is a piece of data containing information about the owner (its domain, server name, company, etc), the owners's **public** key. The certificate might be *signed* or *unsigned*. When *signed* it also contains a digital **signature** from a *Certificate Authority*.

* A [Certificate Authority (CA)](https://en.wikipedia.org/wiki/Certificate_authority) is a **trusted** entity which also has its own *public* and *private* keys and a *certificate* (signed by itself, or *self-signed*). By *"trusted"* it is meant that clients willingly and knowingly install the CA's certificate (CA's information and *public* key) in their software.

* A [Digital Signature](https://en.wikipedia.org/wiki/Cryptographic_hash_function#Signature_generation_and_verification) is when a CA takes an entity's *unsigned* certificate and creates a *cryptographic hash* of it, and *encrypts* that *hash* with the CA's *private* key. Such *encrypted hash* is the *Digital Signature*, which is appended to the certificate to create a *signed* certificate. Normally a *signed* certificate is given an *expiration* date by the CA. When *expired* the CA adds the certificate to its *Certificate Revocation List*

* A [Certificate Revocation List (CRL)](https://en.wikipedia.org/wiki/Certificate_revocation_list) contains the list of certificates revoked by a CA.

So validation works as follows:

* A client installs a trusted CA's certificate (contains CA's *public* key).

* A server creates its *public* key, *private* key and *unsigned certificate*.

* The server asks the trusted CA to *sign* its certificate. The CA's *signs* the server's certificate and gives it back the *signed certificate*.

* When a client connects to a server, the server presents its *signed certificate* to the client.

* The client uses the CA's *public* key (contained in the installed CA's certificate) to decrypt the signature of the server's certificate to obtain the unencrypted hash (calculated by the CA).

* The client calculates separately the hash of the rest of the server's certificate.

* Finally the client compares the unencrypted hash from the CA, with the hash calculated by itself. If they are the same, it means the server's certificate was actually signed by the CA. Therefore, the client can **trust** the server.

The action of *trusting* the server as a consecuence of *trusting* the CA is called the [Web of Trust](https://en.wikipedia.org/wiki/Web_of_trust).

This works because a malicious server would not be able to get his own certificate signed by the CA. So if a client connects to it, the *hashes* will not match. Therefore, the client will *not trust* the malicious server.

Since the server's certificate has an expiration date given by the CA, the client must once in a while check the CA's *Certificate Revocation List* to check whether the server's certificate is still valid.