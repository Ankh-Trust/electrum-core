Shared Libraries
================

## electrumconsensus

The purpose of this library is to make the verification functionality that is critical to Electrum's consensus available to other applications, e.g. to language bindings.

### API

The interface is defined in the C header `electrumconsensus.h` located in  `src/script/electrumconsensus.h`.

#### Version

`electrumconsensus_version` returns an `unsigned int` with the API version *(currently at an experimental `0`)*.

#### Script Validation

`electrumconsensus_verify_script` returns an `int` with the status of the verification. It will be `1` if the input script correctly spends the previous output `scriptPubKey`.

##### Parameters
- `const unsigned char *scriptPubKey` - The previous output script that encumbers spending.
- `unsigned int scriptPubKeyLen` - The number of bytes for the `scriptPubKey`.
- `const unsigned char *txTo` - The transaction with the input that is spending the previous output.
- `unsigned int txToLen` - The number of bytes for the `txTo`.
- `unsigned int nIn` - The index of the input in `txTo` that spends the `scriptPubKey`.
- `unsigned int flags` - The script validation flags *(see below)*.
- `electrumconsensus_error* err` - Will have the error/success code for the operation *(see below)*.

##### Script Flags
- `electrumconsensus_SCRIPT_FLAGS_VERIFY_NONE`
- `electrumconsensus_SCRIPT_FLAGS_VERIFY_P2SH` - Evaluate P2SH ([BIP16](https://github.com/electrum/bips/blob/master/bip-0016.mediawiki)) subscripts
- `electrumconsensus_SCRIPT_FLAGS_VERIFY_DERSIG` - Enforce strict DER ([BIP66](https://github.com/electrum/bips/blob/master/bip-0066.mediawiki)) compliance

##### Errors
- `electrumconsensus_ERR_OK` - No errors with input parameters *(see the return value of `electrumconsensus_verify_script` for the verification status)*
- `electrumconsensus_ERR_TX_INDEX` - An invalid index for `txTo`
- `electrumconsensus_ERR_TX_SIZE_MISMATCH` - `txToLen` did not match with the size of `txTo`
- `electrumconsensus_ERR_DESERIALIZE` - An error deserializing `txTo`

### Example Implementations
- [NElectrum](https://github.com/NicolasDorier/NElectrum/blob/master/NElectrum/Script.cs#L814) (.NET Bindings)
- [node-libelectrumconsensus](https://github.com/bitpay/node-libelectrumconsensus) (Node.js Bindings)
- [java-libelectrumconsensus](https://github.com/dexX7/java-libelectrumconsensus) (Java Bindings)
- [electrumconsensus-php](https://github.com/Bit-Wasp/electrumconsensus-php) (PHP Bindings)
