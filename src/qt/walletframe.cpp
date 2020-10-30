// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/walletframe.h>

#include <qt/electrumgui.h>
#include <qt/walletview.h>
#include <util.h>

#include <guiutil.h>

#include <cstdio>

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

WalletFrame::WalletFrame(const PlatformStyle *platformStyle, ElectrumGUI *_gui) :
    QFrame(_gui),
    gui(_gui),
    platformStyle(platformStyle)
{
    // Leave HBox hook for adding a list view later
    QHBoxLayout *frameLayout = new QHBoxLayout(this);
    frameLayout->setSpacing(0);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerLayout = new QVBoxLayout();
    headerLayout->setContentsMargins(headerMargin, headerMargin, headerMargin, headerMargin);
    headerLayout->setSpacing(headerMargin);

    QHBoxLayout* headLayout = new QHBoxLayout();
    headLayout->setContentsMargins(0, 0, 0, 0);
    headLayout->setSpacing(0);
    headerLayout->addLayout(headLayout);

    menuLayout = new QVBoxLayout();
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);

    walletStack = new QStackedWidget(this);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(walletStack);

    QLabel* noWallet = new QLabel(tr("No wallet has been loaded."));
    noWallet->setAlignment(Qt::AlignCenter);
    walletStack->addWidget(noWallet);

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(contentLayout);

    frameLayout->addLayout(menuLayout);
    frameLayout->addLayout(mainLayout);
}

WalletFrame::~WalletFrame()
{
}

void WalletFrame::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
}

bool WalletFrame::addWallet(const QString& name, WalletModel *walletModel)
{
    if (!gui || !clientModel || !walletModel || mapWalletViews.count(name) > 0)
        return false;

    WalletView *walletView = new WalletView(platformStyle, this);
    walletView->setElectrumGUI(gui);
    walletView->setClientModel(clientModel);
    walletView->setWalletModel(walletModel);

     /* TODO we should goto the currently selected page once dynamically adding wallets is supported */
    walletView->gotoOverviewPage();
    walletStack->addWidget(walletView);
    mapWalletViews[name] = walletView;

    // Ensure a walletView is able to show the main window
    connect(walletView, SIGNAL(showNormalIfMinimized()), gui, SLOT(showNormalIfMinimized()));
    connect(walletView, SIGNAL(openAddressHistory()), this, SLOT(usedReceivingAddresses()));
    connect(walletView, SIGNAL(daoEntriesChanged(int)), this, SLOT(onDaoEntriesChanged(int)));

    return true;
}

bool WalletFrame::setCurrentWallet(const QString& name)
{
    if (mapWalletViews.count(name) == 0)
        return false;

    WalletView *walletView = mapWalletViews.value(name);
    walletStack->setCurrentWidget(walletView);
    walletView->updateEncryptionStatus();
    return true;
}

bool WalletFrame::removeWallet(const QString &name)
{
    if (mapWalletViews.count(name) == 0)
        return false;

    WalletView *walletView = mapWalletViews.take(name);
    walletStack->removeWidget(walletView);
    return true;
}

void WalletFrame::removeAllWallets()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        walletStack->removeWidget(i.value());
    mapWalletViews.clear();
}

bool WalletFrame::handlePaymentRequest(const SendCoinsRecipient &recipient)
{
    WalletView *walletView = currentWalletView();
    if (!walletView)
        return false;

    return walletView->handlePaymentRequest(recipient);
}

void WalletFrame::gotoOverviewPage()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->gotoOverviewPage();
}

void WalletFrame::setStakingStats(QString day, QString week, QString month, QString year, QString all)
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->setStakingStats(day,week,month,year,all);
}

void WalletFrame::splitRewards()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->splitRewards();
}

void WalletFrame::gotoHistoryPage()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->gotoHistoryPage();
}

void WalletFrame::gotoCommunityFundPage()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->gotoCommunityFundPage();
}

void WalletFrame::gotoReceiveCoinsPage()
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->gotoReceiveCoinsPage();
}

void WalletFrame::gotoSendCoinsPage(QString addr)
{
    QMap<QString, WalletView*>::const_iterator i;
    for (i = mapWalletViews.constBegin(); i != mapWalletViews.constEnd(); ++i)
        i.value()->gotoSendCoinsPage(addr);
}

void WalletFrame::gotoSignMessageTab(QString addr)
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->gotoSignMessageTab(addr);
}

void WalletFrame::gotoVerifyMessageTab(QString addr)
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->gotoVerifyMessageTab(addr);
}

void WalletFrame::encryptWallet(bool status)
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->encryptWallet(status);
}

void WalletFrame::backupWallet()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->backupWallet();
}

void WalletFrame::changePassphrase()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->changePassphrase();
}

void WalletFrame::unlockWallet()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->unlockWallet();
}

void WalletFrame::unlockWalletStaking()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->unlockWalletStaking();
}

void WalletFrame::importPrivateKey()
{
    WalletView *walletView = currentWalletView();
    if(walletView)
        walletView->importPrivateKey();
}

void WalletFrame::exportMasterPrivateKeyAction()
{
    WalletView *walletView = currentWalletView();
    if(walletView)
        walletView->exportMasterPrivateKeyAction();
}

void WalletFrame::exportMnemonicAction()
{
    WalletView *walletView = currentWalletView();
    if(walletView)
        walletView->exportMnemonicAction();
}

void WalletFrame::lockWallet()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->lockWallet();
}

void WalletFrame::usedSendingAddresses()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->usedSendingAddresses();
}

void WalletFrame::usedReceivingAddresses()
{
    WalletView *walletView = currentWalletView();
    if (walletView)
        walletView->usedReceivingAddresses();
}

WalletView *WalletFrame::currentWalletView()
{
    return qobject_cast<WalletView*>(walletStack->currentWidget());
}

void WalletFrame::outOfSyncWarningClicked()
{
    Q_EMIT requestedSyncWarningInfo();
}

void WalletFrame::onDaoEntriesChanged(int count)
{
    Q_EMIT daoEntriesChanged(count);
}
