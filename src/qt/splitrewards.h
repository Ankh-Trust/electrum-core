// Copyright (c) 2019 The NavCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPLITREWARDS_H
#define SPLITREWARDS_H

#include "util.h"
#include "qjsonmodel.h"
#include "utilmoneystr.h"
#include "walletmodel.h"

#include <QComboBox>
#include <QDialog>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTreeView>
#include <QVBoxLayout>

static QMap<QString, QString> teamAddresses = {{"EPcKGcYZXXcgXANGQqWEjEte3xwiQUsRHA", "Ankh Trust Fund"},
                                               {"", "Custom"}};

class SplitRewardsDialog : public QDialog
{
    Q_OBJECT

public:
    SplitRewardsDialog(QWidget *parent);

    void setModel(WalletModel *model);
    void showFor(QString address);

private:
    WalletModel *model;
    QJsonObject jsonObject;
    QString currentAddress;
    QLabel *strDesc;
    QTreeView *tree;
    QComboBox *comboAddress;

    int availableAmount;

private Q_SLOTS:
    void onAdd();
    void onRemove();
    void onSave();
    void onEdit();
    void onQuit();
};

CAmount PercentageToElectrum(int percentage);

#endif // SPLITREWARDS_H
