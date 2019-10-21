// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELECTRUM_QT_ELECTRUMADDRESSVALIDATOR_H
#define ELECTRUM_QT_ELECTRUMADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class ElectrumAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ElectrumAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Electrum address widget validator, checks for a valid electrum address.
 */
class ElectrumAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit ElectrumAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // ELECTRUM_QT_ELECTRUMADDRESSVALIDATOR_H
