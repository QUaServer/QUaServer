# x509 Certificate Based Access Control



```bash
# Create client key
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out client_files/client.pem
# Convert client key to der format
openssl rsa -in client_files/client.pem -inform pem -out client_files/client.der -outform der
# Create client cert sign request
openssl req -new -sha256 \
-key client_files/client.pem \
-subj "/C=ES/ST=MAD/O=Myclient_files/CN=localhost" \
-out client_files/client.csr

# Sign cert sign request (NOTE: must provide exts.txt)
openssl x509 -days 3600 -req \
-in client_files/client.csr \
-extensions v3_ca \
-extfile client_files/exts.txt \
-CAcreateserial -CA ca_files/ca.crt -CAkey ca_files/ca.key \
-out client_files/client.crt
# Convert cert to der format
openssl x509 -in client_files/client.crt -inform pem -out client_files/client.crt.der -outform der
```