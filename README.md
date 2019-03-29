# Compile

**IMPORTANT** : Requires `Qt 5.7` or higher and `C++ 11`.

The source code of the `open62541` library is contained in the files:

```
src/open62541.h
src/open62541.c
```

These files are **generated** by the *CMake* build of the [original library repository](https://github.com/open62541/open62541). They were generated and copied to this repo for convenience.

```bash
cd "C:\Users\User\Desktop\Repos\open62541.git"
mkdir build; cd build
cmake -DUA_ENABLE_AMALGAMATION=ON .. -G "Visual Studio 15 2017 Win64"
```

The library version of the `open62541` files used currently in this repo is `v0.3-rc4`.

---

# Compile Encryption

We need the `mbedtls` dependency:

* Clone [this repo](https://github.com/ARMmbed/mbedtls).

* Checkout a release tag (e.g. `mbedtls-2.17.0`).

Generate Solution:

```bash
cd "C:\Users\User\Desktop\Repos\mbedtls.git"
mkdir build; cd build
cmake .. -G "Visual Studio 15 2017 Win64"
```

* Build Release and Debug

You should have headers in `C:\Users\User\Desktop\Repos\mbedtls.git\build\include\mbedtls`.

And libs in `C:\Users\User\Desktop\Repos\mbedtls.git\build\library\Debug` and in `C:\Users\User\Desktop\Repos\mbedtls.git\build\library\Release`.

Now **Rebuild** the `open62541` with the following command:

```bash
cd "C:\Users\User\Desktop\Repos\open62541.git"
mkdir build; cd build
cmake -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_ENCRYPTION=ON .. -G "Visual Studio 15 2017 Win64" -DMBEDTLS_INCLUDE_DIRS="C:\Users\User\Desktop\Repos\mbedtls.git\build\include" -DMBEDTLS_LIBRARY="C:\Users\User\Desktop\Repos\mbedtls.git\build\library\Debug" -DMBEDX509_LIBRARY="C:\Users\User\Desktop\Repos\mbedtls.git\build\library\Debug" -DMBEDCRYPTO_LIBRARY="C:\Users\User\Desktop\Repos\mbedtls.git\build\library\Debug"
```

* Copy the amalgamated `open62541.h` and `open62541.c` to `./src` and enable the lines that mark `[ENCRYPTION]` in `open62541.pro` and `open62541.pri`.

---

# Server Certificate

Create self-signed certificate

```bash
# Create dirs somewhere
#rm -rf ca;
#rm -rf server;
mkdir ca
mkdir server

# Create CA key
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out ca/ca.key
# Create self-signed CA cert
openssl req -new -x509 -days 360 -key ca/ca.key -subj "/CN=juangburgos CA/O=juangburgos Organization" -out ca/ca.crt
# Convert cert to der format
openssl x509 -in ca/ca.crt -inform pem -out ca/ca.crt.der -outform der
# Create cert revocation list CRL file
# NOTE : need to have in relative path
#        - Empty file 'demoCA/index.txt'
#        - File 'crlnumber' with contents '1000'
openssl ca -keyfile ca/ca.key -cert ca/ca.crt -gencrl -out ca/ca.crl
# Convert CRL to der format
openssl crl -in ca/ca.crl -inform pem -out ca/ca.crl.der -outform der

# Create server key
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out server/server.key
# Convert server key to der format
openssl rsa -in server/server.key -inform pem -out server/server.key.der -outform der
# Create server cert sign request
openssl req -new -sha256 \
-key server/server.key \
-subj "/C=ES/ST=MAD/O=MyServer/CN=localhost" \
-out server/server.csr
# Sign cert sign request (NOTE: must provide exts.txt)
openssl x509 -days 700 -req \
-in server/server.csr \
-extensions v3_ca \
-extfile server/exts.txt \
-CAcreateserial -CA ca/ca.crt -CAkey ca/ca.key \
-out server/server.crt
# See all OPC UA extensions are present in cret contents
openssl x509 -text -noout -in server/server.crt
# Convert cert to der format
openssl x509 -in server/server.crt -inform pem -out server/server.crt.der -outform der
```

NOTE : `*.key` contains both **private** and **pubic** keys.

The `exts.txt` must look like:

```
[v3_ca]
subjectAltName=DNS:localhost,DNS:ppic09,IP:127.0.0.1,URI:urn:unconfigured:application
basicConstraints=CA:TRUE
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid,issuer
keyUsage=digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth,clientAuth,codeSigning
```

* NOTE : in `subjectAltName` we put ip address, machine name in windows network, domain name, etc.

* TODO : `URI:urn:unconfigured:application` is necesary as it is but I think is customizable in server library.

* Load `server/server.key.der` to server library as *server private key*.

* Load `server/server.crt.der` to server library as *server certificate*.

To make it work with UA Expert client:

* Copy `ca/ca.crt.der` to `C:\Users\User\AppData\Roaming\unifiedautomation\uaexpert\PKI\trusted\certs`.

* Copy `ca/ca.crl.der` to `C:\Users\User\AppData\Roaming\unifiedautomation\uaexpert\PKI\trusted\crl`. 

---

## TODO

1. Find out how to use OptionSets (BitMasks).

### 7.17 OptionSetType

The OptionSetType VariableType is used to represent a bit mask. Each array element of the OptionSetValues Property contains either the human-readable representation for the corresponding bit used in the option set or an empty LocalizedText for a bit that has no specific meaning. The order of the bits of the bit mask maps to a position of the array, i.e. the first bit (least significant bit) maps to the first entry in the array, etc.

In addition to this VariableType, the DataType OptionSet can alternatively be used to represent a bit mask. As a guideline the DataType would be used when the bit mask is fixed and applies to several Variables. The VariableType would be used when the bit mask is specific for only that Variable.

The DataType of this VariableType shall be capable of representing a bit mask. It shall be either a numeric DataType representing a signed or unsigned integer, or a ByteString. For example, it can be the BitFieldMaskDataType.

The optional BitMask Property provides the bit mask in an array of Booleans. This allows subscribing to individual entries of the bit mask. The order of the bits of the bit mask points to a position of the array, i.e. the first bit points to the first entry in the array, etc.

### 12.18 BitFieldMaskDataType

This simple DataType is a subtype of UInt64 and represents a bit mask up to 32 bits where individual bits can be written without modifying the other bits.

The first 32 bits (least significant bits) of the BitFieldMaskDataType represent the bit mask and the second 32 bits represent the validity of the bits in the bit mask. When the Server returns the value to the client, the validity provides information of which bits in the bit mask have a meaning. When the client passes the value to the Server, the validity defines which bits should be written. Only those bits defined in validity are changed in the bit mask, all others stay the same. The BitFieldMaskDataType can be used as DataType in the OptionSetType VariableType.


---

# References

* <https://blog.basyskom.com/2018/want-to-give-qt-opcua-a-try/>

* <https://github.com/open62541/open62541/issues/2584>

* <http://documentation.unified-automation.com/uasdkcpp/1.5.5/html/L2UaDiscoveryConnect.html#DiscoveryConnect_ConnectConfig>

* <https://www.openssl.org/docs/man1.0.2/man5/x509v3_config.html>

* <https://forum.unified-automation.com/topic1762.html#p3553>

* <https://serverfault.com/questions/823679/openssl-error-while-loading-crlnumber>