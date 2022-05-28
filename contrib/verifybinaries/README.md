### Verify Binaries

#### Preparation:

Make sure you obtain the proper release signing keys and verify the fingerprint with several independent sources.

```sh
$ gpg --fingerprint "Novo Bitcoin binary release signing key"
pub   4096R/6431FF95 2021-12-02 [expires: 2022-12-02]
      Key fingerprint =
uid                  renovoner (Novo Bitcoin binary release signing key) <novobitcoin@protonmail.com>
```

#### Usage:

This script attempts to download the signature file `SHA256SUMS.asc` from https://download.novobitcoin.org.

It first checks if the signature passes, and then downloads the files specified in the file, and checks if the hashes of these files match those that are specified in the signature file.

The script returns 0 if everything passes the checks. It returns 1 if either the signature check or the hash check doesn't pass. If an error occurs the return value is 2.


```sh
./verify.sh 0.2.1
./verify.sh 1.0.0.beta2
./verify.sh 1.0.3
```

If you only want to download the binaries of certain platform, add the corresponding suffix, e.g.:

```sh
./verify.sh 0.1.1-arm
./verify.sh 1.0.3-linux
```

If you do not want to keep the downloaded binaries, specify anything as the second parameter.

```sh
./verify.sh 1.0.3 delete
```
