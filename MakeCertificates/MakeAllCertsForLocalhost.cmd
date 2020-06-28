@echo off

REM SET HOSTNAME=127.0.0.1
SET HOSTNAME=localhost
SET COUNTRY=RU
SET STATE=RU
SET CITY=Moscow
SET ORGANIZATION=DORS
SET ORGANIZATION_UNIT=IT Department
SET EMAIL=test@%HOSTNAME%

(
echo [req]
echo default_bits = 2048
echo prompt = no
echo default_md = sha256
echo x509_extensions = v3_req
echo distinguished_name = dn
echo:
echo [dn]
echo C = %COUNTRY%
echo ST = %STATE%
echo L = %CITY%
echo O = %ORGANIZATION%
echo OU = %ORGANIZATION_UNIT%
echo emailAddress = %EMAIL%
echo CN = %HOSTNAME%
echo:
echo [v3_req]
echo subjectAltName = @alt_names
echo:
echo [alt_names]
echo DNS.1 = *.%HOSTNAME%
echo DNS.2 = %HOSTNAME%
)>%HOSTNAME%.cnf

.\..\openssl\openssl req -new -x509 -newkey rsa:2048 -sha256 -nodes -keyout %HOSTNAME%.key -days 3560 -out %HOSTNAME%.crt -config %HOSTNAME%.cnf

.\..\openssl\openssl.exe pkcs12 -export -name "%HOSTNAME%" -out %HOSTNAME%.pfx -inkey %HOSTNAME%.key -in %HOSTNAME%.crt

.\..\openssl\openssl.exe x509 -in %HOSTNAME%.crt -outform der -out %HOSTNAME%.der.crt

.\..\openssl\openssl x509 -inform DER -in %HOSTNAME%.der.crt -out %HOSTNAME%.pem -text
pause
