# Responsible Disclosure Policy

Novo Bitcoin takes security very seriously.  We greatly appreciate any and
all disclosures of bugs and vulnerabilities that are done in a responsible
manner.  We will engage responsible disclosures according to this policy and put
forth our best effort to fix disclosed vulnerabilities as well as reaching out
to numerous node operators to deploy fixes in a timely manner.

This disclosure policy is also intended to conform to [this proposed standard](https://github.com/RD-Crypto-Spec/Responsible-Disclosure/blob/184391fcbc1bbf3c158c527a841e611ac9ae8388/README.md)
with some modifications (see below).

## Responsible Disclosure Guidelines

Do not disclose any security issue or vulnerability on public forums, message
boards, mailing lists, etc. prior to responsibly disclosing to Novo Bitcoin
and giving sufficient time for the issue to be fixed and deployed.
Do not execute on or exploit any vulnerability.  This includes testnet, as both
mainnet and testnet exploits are effectively public disclosure.  Regtest mode
may be used to test bugs locally.

## Reporting a Bug or Vulnerability

When reporting a bug or vulnerability, please provide the following to
novobitcoin@protonmail.com:

* A short summary of the potential impact of the issue (if known).
* Details explaining how to reproduce the issue or how an exploit may be formed.
* Your name (optional).  If provided, we will provide credit for disclosure.
  Otherwise, you will be treated anonymously and your privacy will be respected.
* Your email or other means of contacting you.
* A PGP key/fingerprint for us to provide encrypted responses to your disclosure.
  If this is not provided, we cannot guarantee that you will receive a response
  prior to a fix being made and deployed.

## Encrypting the Disclosure

We highly encourage all disclosures to be encrypted to prevent interception and
exploitation by third-parties prior to a fix being developed and deployed.
Please encrypt using the PGP public key with fingerprint:
``

It may be obtained via:

```
gpg --recv-keys ""
```

After this succeeded you can encrypt your prepared disclosure document with the
following line:

```
gpg --armor --encrypt --recipient novobitcoin@protonmail.com yourDisclosureDocument
```

This will create a copy of your document with the file extension `.asc`, you can
email this encrypted version of the document to Novo Bitcoins security team.

If you still have questions about how to do it, feel free to contact the Bitcoin
Cash Node project security team for more instructions.

## Backup PGP Keys

These PGP fingerprints and emails are provided only as backups in case you are
unable to contact Novo Bitcoin via the security email above.

## Disclosure Relationships

Neighboring projects that may be affected by bugs, potential exploits, or other
security vulnerabilities that are disclosed to Novo Bitcoin will be passed
along information regarding disclosures that we believe could impact them. As
per the standard referenced above, we are disclosing these relationships here:

We have approached several other projects and are waiting for responses from them.

If you feel your projects is closely related and would like to form a disclosure
relationship with Novo Bitcoin, please contact our security email address
to discuss.

## Bounty Payments

Novo Bitcoin cannot at this time commit to bounty payments ahead of time.
However, we will use our best judgement and do intend on rewarding those who
provide valuable disclosures (with a strong emphasis on easy to read and
reproduce disclosures).

## Deviations from the Standard

While Novo Bitcoin believes that strong cohesion among neighoring projects
and ethical behavior can be standardized to reduce poorly handled disclosure
incidents, we also believe that it's in the best interest of Novo Bitcoin for us
to deviate from the standard in the following ways:

* The standard calls for coordinated releases. While Novo Bitcoin will make
  attempts to coordinate releases when possible, it's not always feasible to
  coordinate urgent fixes for catastrophic exploits (ie. chain splitting events).
  For critical fixes, Novo Bitcoin will release them in the next release
  when possible.

## Making changes to this disclosure policy

Note that any changes to this disclosure policy should be mirrored in a pull
request to the [Novo Bitcoin repository](https://github.com/novobitcoin/novobitcoin).
