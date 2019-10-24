// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/electrum-config.h"
#endif

#include "main.h"
#include "electrumgui.h"
#include "electrumunits.h"
#include "clientversion.h"
#include "clientmodel.h"
#include "coldstakingwizard.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "modaloverlay.h"
#include "networkstyle.h"
#include "notificator.h"
#include "openuridialog.h"
#include "optionsdialog.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "rpcconsole.h"
#include "utilitydialog.h"
#include "walletmodel.h"
#include "wallet/rpcwallet.h"

#ifdef ENABLE_WALLET
#include "walletframe.h"
#include "walletmodel.h"
#include "walletview.h"
#include "wallet/wallet.h"
#endif // ENABLE_WALLET

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include "chainparams.h"
#include "init.h"
#include "miner.h"
#include "ui_interface.h"
#include "util.h"
#include "pos.h"
#include "main.h"

#include <iostream>
#include <thread>

#include <curl/curl.h>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QDragEnterEvent>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QVariant>
#include <QVariantMap>
#include <QWidget>

const std::string ElectrumGUI::DEFAULT_UIPLATFORM =
#if defined(Q_OS_MAC)
        "macosx"
#elif defined(Q_OS_WIN)
        "windows"
#else
        "other"
#endif
        ;

const QString ElectrumGUI::DEFAULT_WALLET = "~Default";

ElectrumGUI::ElectrumGUI(const PlatformStyle *platformStyle, const NetworkStyle *networkStyle, QWidget *parent) :
    QMainWindow(parent),
    clientModel(0),
    walletFrame(0),
    unitDisplayControl(0),
    labelEncryptionIcon(0),
    labelWalletHDStatusIcon(0),
    labelConnectionsIcon(0),
    labelBlocksIcon(0),
    labelStakingIcon(0),
    labelPrice(0),
    progressBarLabel(0),
    progressBar(0),
    progressDialog(0),
    appMenuBar(0),
    overviewAction(0),
    historyAction(0),
    quitAction(0),
    sendCoinsAction(0),
    sendCoinsMenuAction(0),
    usedSendingAddressesAction(0),
    usedReceivingAddressesAction(0),
    repairWalletAction(0),
    importPrivateKeyAction(0),
    exportMasterPrivateKeyAction(0),
    signMessageAction(0),
    verifyMessageAction(0),
    aboutAction(0),
    webInfoAction(0),
    receiveCoinsAction(0),
    receiveCoinsMenuAction(0),
    optionsAction(0),
    cfundProposalsAction(0),
    cfundPaymentRequestsAction(0),
    toggleHideAction(0),
    encryptWalletAction(0),
    backupWalletAction(0),
    changePassphraseAction(0),
    aboutQtAction(0),
    openRPCConsoleAction(0),
    openAction(0),
    showHelpMessageAction(0),
    trayIcon(0),
    trayIconMenu(0),
    notificator(0),
    rpcConsole(0),
    helpMessageDialog(0),
    prevBlocks(0),
    spinnerFrame(0),
    fDontShowAgain(false),
    unlockWalletAction(0),
    unlockStakingAction(0),
    lockWalletAction(0),
    toggleStakingAction(0),
    generateColdStakingAction(0),
    lastDialogShown(0),
    platformStyle(platformStyle),
    updatePriceAction(0),
    fShowingVoting(0)
{
    GUIUtil::restoreWindowGeometry("nWindow", QSize(1152, 576), this);
    setMinimumSize(QSize(1152, 576));
    QString windowTitle = tr("Electrum Core") + "";
#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET
    if(enableWallet)
    {
        windowTitle += tr("");
    } else {
        windowTitle += tr("Node");
    }

    if(clientModel->isReleaseVersion() == false)
    {
         // default to test version
        QString titleExtra = tr("[TEST ONLY]");

        if(clientModel->isRCReleaseVersion() == true)
        {
            titleExtra = tr("[RELEASE CANDIDATE]");
        }

         windowTitle += " " + titleExtra;
    }

    windowTitle += " " + networkStyle->getTitleAddText();


#ifndef Q_OS_MAC
    QApplication::setWindowIcon(networkStyle->getTrayAndWindowIcon());
    setWindowIcon(networkStyle->getTrayAndWindowIcon());
#else
    MacDockIconHandler::instance()->setIcon(networkStyle->getAppIcon());
#endif
    setWindowTitle(windowTitle);

#if defined(Q_OS_MAC)
    // This property is not implemented in Qt 5. Setting it has no effect.
    // A replacement API (QtMacUnifiedToolBar) is available in QtMacExtras.
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    rpcConsole = new RPCConsole(platformStyle, 0);
    helpMessageDialog = new HelpMessageDialog(this, false);
#ifdef ENABLE_WALLET
    if(enableWallet)
    {
        /** Create wallet frame*/
        walletFrame = new WalletFrame(platformStyle, this);
    } else
#endif // ENABLE_WALLET
    {
        /* When compiled without wallet or -disablewallet is provided,
         * the central widget is the rpc console.
         */
        setCentralWidget(rpcConsole);
    }

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    // Needs walletFrame to be initialized
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create system tray icon and notification
    createTrayIcon(networkStyle);

    // Create status bar
    statusBar();

    // Disable size grip because it looks ugly and nobody needs it
    statusBar()->setSizeGripEnabled(false);

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    unitDisplayControl = new QComboBox();
    unitDisplayControl->setEditable(true);
    unitDisplayControl->setInsertPolicy(QComboBox::NoInsert);
    Q_FOREACH(ElectrumUnits::Unit u, ElectrumUnits::availableUnits())
    {
        unitDisplayControl->addItem(QString(ElectrumUnits::name(u)), u);
    }
    connect(unitDisplayControl,SIGNAL(currentIndexChanged(int)),this,SLOT(comboBoxChanged(int)));
    labelEncryptionIcon = new QLabel();
    labelStakingIcon = new QLabel();
    labelPrice = new QLabel();
    labelWalletHDStatusIcon = new QLabel();
    labelConnectionsIcon = new QPushButton();
    labelConnectionsIcon->setFlat(true); // Make the button look like a label, but clickable
    labelConnectionsIcon->setMaximumSize(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    // Jump to peers tab by clicking on connections icon
    connect(labelConnectionsIcon, SIGNAL(clicked()), this, SLOT(showPeers()));

    labelBlocksIcon = new GUIUtil::ClickableLabel();
    if(enableWallet)
    {
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelPrice);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(unitDisplayControl);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelEncryptionIcon);
        frameBlocksLayout->addWidget(labelWalletHDStatusIcon);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelStakingIcon);
    }
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();

    updatePrice(); // First price update

    int updateFiatPeriod = GetArg("-updatefiatperiod", PRICE_UPDATE_DELAY);
    if (updateFiatPeriod >= PRICE_UPDATE_DELAY)
    {
        QTimer *timerPrice = new QTimer(labelPrice);
        connect(timerPrice, SIGNAL(timeout()), this, SLOT(updatePrice()));
        timerPrice->start(updateFiatPeriod);
        info("Automatic price update set to " + std::to_string(updateFiatPeriod) + "ms");
    }
    else
    {
        info("Automatic price update turned OFF");
    }

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingStatus()));
        timerStakingIcon->start(45 * 1000);
        updateStakingStatus();
    }

    if (GetArg("-zapwallettxes",0) == 2 && GetArg("-repairwallet",0) == 1)
    {
        RemoveConfigFile("zapwallettxes","2");
        RemoveConfigFile("repairwallet","1");

        QMessageBox::information(this, tr("Repair wallet"),
            tr("Wallet has been repaired."),
            QMessageBox::Ok, QMessageBox::Ok);
    }

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(true);
    progressBar = new GUIUtil::ProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(true);

    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = QApplication::style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
      progressBar->setStyleSheet("QProgressBar { background-color: #F8F8F8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #00CCFF, stop: 1 #33CCFF); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);

    // Install event filter to be able to catch status tip events (QEvent::StatusTip)
    this->installEventFilter(this);

    // Initially wallet actions should be disabled
    setWalletActionsEnabled(false);

    // Subscribe to notifications from core
    subscribeToCoreSignals();

    modalOverlay = new ModalOverlay(this->centralWidget());
#ifdef ENABLE_WALLET
    if(enableWallet) {
        connect(walletFrame, &WalletFrame::requestedSyncWarningInfo, this, &ElectrumGUI::showModalOverlay);
        connect(labelBlocksIcon, &GUIUtil::ClickableLabel::clicked, this, &ElectrumGUI::showModalOverlay);
        connect(progressBar, &GUIUtil::ClickableProgressBar::clicked, this, &ElectrumGUI::showModalOverlay);
    }
#endif
}

ElectrumGUI::~ElectrumGUI()
{
    // Unsubscribe from notifications from core
    unsubscribeFromCoreSignals();

    GUIUtil::saveWindowGeometry("nWindow", this);
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
    MacDockIconHandler::cleanup();
#endif

    delete rpcConsole;
}

void ElectrumGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);
    overviewAction->setStatusTip(tr("Show general overview of wallet"));
    overviewAction->setToolTip(overviewAction->statusTip());
    overviewAction->setCheckable(true);
#ifdef Q_OS_MAC
    overviewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
#else
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
#endif
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("&Send"), this);
    sendCoinsAction->setStatusTip(tr("Send coins to a Electrum address"));
    sendCoinsAction->setToolTip(sendCoinsAction->statusTip());
    sendCoinsAction->setCheckable(true);
#ifdef Q_OS_MAC
    sendCoinsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
#else
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
#endif
    tabGroup->addAction(sendCoinsAction);

    sendCoinsMenuAction = new QAction(QIcon(":/icons/send"), sendCoinsAction->text(), this);
    sendCoinsMenuAction->setStatusTip(sendCoinsAction->statusTip());
    sendCoinsMenuAction->setToolTip(sendCoinsMenuAction->statusTip());

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Receive"), this);
    receiveCoinsAction->setStatusTip(tr("Request payments (generates QR codes and electrum: URIs)"));
    receiveCoinsAction->setToolTip(receiveCoinsAction->statusTip());
    receiveCoinsAction->setCheckable(true);
#ifdef Q_OS_MAC
    receiveCoinsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
#else
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
#endif
    tabGroup->addAction(receiveCoinsAction);

    receiveCoinsMenuAction = new QAction(QIcon(":/icons/receiving_addresses"), receiveCoinsAction->text(), this);
    receiveCoinsMenuAction->setStatusTip(receiveCoinsAction->statusTip());
    receiveCoinsMenuAction->setToolTip(receiveCoinsMenuAction->statusTip());

    toggleStakingAction = new QAction(QIcon(":/icons/staking"), tr("Toggle &Staking"), this);
    toggleStakingAction->setStatusTip(tr("Toggle Staking"));

    generateColdStakingAction = new QAction(QIcon(":/icons/verify"), tr("&Generate Cold Staking Address"), this);
    generateColdStakingAction->setStatusTip(tr("Generate Cold Staking Address"));

    historyAction = new QAction(QIcon(":/icons/history"), tr("&Transactions"), this);
    historyAction->setStatusTip(tr("Browse transaction history"));
    historyAction->setToolTip(historyAction->statusTip());
    historyAction->setCheckable(true);
#ifdef Q_OS_MAC
    historyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
#else
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
#endif
    tabGroup->addAction(historyAction);

    updatePriceAction  = new QAction(QIcon(":/icons/verify"), tr("Update exchange prices"), this);
    updatePriceAction->setStatusTip(tr("Update exchange prices"));

    connect(updatePriceAction, SIGNAL(triggered()), this, SLOT(updatePrice()));

#ifdef ENABLE_WALLET
    // These showNormalIfMinimized are needed because Send Coins and Receive Coins
    // can be triggered from the tray menu, and need to show the GUI to be useful.
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(sendCoinsMenuAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsMenuAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(receiveCoinsMenuAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsMenuAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(toggleStakingAction, SIGNAL(triggered()), this, SLOT(toggleStaking()));
    connect(generateColdStakingAction, SIGNAL(triggered()), this, SLOT(generateColdStaking()));
#endif // ENABLE_WALLET

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setStatusTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/about"), tr("&About %1").arg(tr(PACKAGE_NAME)), this);
    aboutAction->setStatusTip(tr("Show information about %1").arg(tr(PACKAGE_NAME)));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutAction->setEnabled(false);
    webInfoAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Electrum &Knowledge Base"), this);
    webInfoAction->setStatusTip(tr("Open the Electrum Knowledge Base in your browser"));
    webInfoAction->setMenuRole(QAction::NoRole);
    webInfoAction->setEnabled(false);
    aboutQtAction = new QAction(QIcon(":/icons/about_qt"), tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setStatusTip(tr("Modify configuration options for %1").arg(tr(PACKAGE_NAME)));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    optionsAction->setEnabled(false);
    cfundProposalsAction = new QAction(tr("Vote for Proposals"), this);
    cfundPaymentRequestsAction = new QAction(tr("Vote for Payment Requests"), this);
    toggleHideAction = new QAction(QIcon(":/icons/about"), tr("&Show / Hide"), this);
    toggleHideAction->setStatusTip(tr("Show or hide the main Window"));

    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setStatusTip(tr("Encrypt the private keys that belong to your wallet"));
    encryptWalletAction->setCheckable(true);
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet ..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet for Staking"));
    unlockStakingAction = new QAction(QIcon(":/icons/staking"), tr("Unlock Wallet for &Staking..."), this);
    unlockStakingAction->setToolTip(tr("Unlock wallet for Staking"));
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setStatusTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setStatusTip(tr("Change the passphrase used for wallet encryption"));

    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    signMessageAction->setStatusTip(tr("Sign messages with your Electrum addresses to prove you own them"));
    verifyMessageAction = new QAction(QIcon(":/icons/verify"), tr("&Verify message..."), this);
    verifyMessageAction->setStatusTip(tr("Verify messages to ensure they were signed with specified Electrum addresses"));

    openInfoAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Information"), this);
    openInfoAction->setStatusTip(tr("Show diagnostic information"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setStatusTip(tr("Open debugging and diagnostic console"));
    openGraphAction = new QAction(QIcon(":/icons/connect_4"), tr("&Network Monitor"), this);
    openGraphAction->setStatusTip(tr("Show network monitor"));
    openPeersAction = new QAction(QIcon(":/icons/connect_4"), tr("&Peers list"), this);
    openPeersAction->setStatusTip(tr("Show peers info"));
    //openRepairAction = new QAction(QIcon(":/icons/options"), tr("Wallet &Repair"), this);
    //openRepairAction->setStatusTip(tr("Show wallet repair options"));

    // initially disable the debug window menu item
    openInfoAction->setEnabled(false);
    openRPCConsoleAction->setEnabled(false);
    openGraphAction->setEnabled(false);
    openPeersAction->setEnabled(false);
    //openRepairAction->setEnabled(false);

    usedSendingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Sending addresses..."), this);
    usedSendingAddressesAction->setStatusTip(tr("Show the list of used sending addresses and labels"));
    usedReceivingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Receiving addresses..."), this);
    usedReceivingAddressesAction->setStatusTip(tr("Show the list of used receiving addresses and labels"));
    repairWalletAction = new QAction(QIcon(":/icons/options"), tr("&Repair wallet"), this);
    repairWalletAction->setToolTip(tr("Repair wallet transactions"));

    importPrivateKeyAction = new QAction(QIcon(":/icons/key"), tr("&Import private key"), this);
    importPrivateKeyAction->setToolTip(tr("Import private key"));

    exportMasterPrivateKeyAction = new QAction(QIcon(":/icons/key"), tr("Show &master private key"), this);
    exportMasterPrivateKeyAction->setToolTip(tr("Show master private key"));

    openAction = new QAction(QIcon(":/icons/open"), tr("Open &URI..."), this);
    openAction->setStatusTip(tr("Open a electrum: URI or payment request"));

    showHelpMessageAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Command-line options"), this);
    showHelpMessageAction->setMenuRole(QAction::NoRole);
    showHelpMessageAction->setStatusTip(tr("Show the %1 help message to get a list with possible Electrum command-line options").arg(tr(PACKAGE_NAME)));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(webInfoAction, SIGNAL(triggered()), this, SLOT(webInfoClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(cfundProposalsAction, SIGNAL(triggered()), this, SLOT(cfundProposalsClicked()));
    connect(cfundPaymentRequestsAction, SIGNAL(triggered()), this, SLOT(cfundPaymentRequestsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(showHelpMessageAction, SIGNAL(triggered()), this, SLOT(showHelpMessageClicked()));

    // Jump directly to tabs in RPC-console
    connect(openInfoAction, SIGNAL(triggered()), this, SLOT(showInfo()));
    connect(openRPCConsoleAction, SIGNAL(triggered()), this, SLOT(showDebugWindow()));
    connect(openGraphAction, SIGNAL(triggered()), this, SLOT(showGraph()));
    connect(openPeersAction, SIGNAL(triggered()), this, SLOT(showPeers()));
    //connect(openRepairAction, SIGNAL(triggered()), this, SLOT(showRepair()));

    // prevents an open debug window from becoming stuck/unusable on client shutdown
    connect(quitAction, SIGNAL(triggered()), rpcConsole, SLOT(hide()));

#ifdef ENABLE_WALLET
    if(walletFrame)
    {
        connect(encryptWalletAction, SIGNAL(triggered(bool)), walletFrame, SLOT(encryptWallet(bool)));
        connect(backupWalletAction, SIGNAL(triggered()), walletFrame, SLOT(backupWallet()));
        connect(changePassphraseAction, SIGNAL(triggered()), walletFrame, SLOT(changePassphrase()));
        connect(unlockWalletAction, SIGNAL(triggered()), walletFrame, SLOT(unlockWallet()));
        connect(unlockStakingAction, SIGNAL(triggered()), walletFrame, SLOT(unlockWalletStaking()));
        connect(lockWalletAction, SIGNAL(triggered()), walletFrame, SLOT(lockWallet()));
        connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
        connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
        connect(usedSendingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedSendingAddresses()));
        connect(usedReceivingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedReceivingAddresses()));
        connect(repairWalletAction, SIGNAL(triggered()), this, SLOT(repairWallet()));
        connect(importPrivateKeyAction, SIGNAL(triggered()), walletFrame, SLOT(importPrivateKey()));
        connect(exportMasterPrivateKeyAction, SIGNAL(triggered()), walletFrame, SLOT(exportMasterPrivateKeyAction()));
        connect(openAction, SIGNAL(triggered()), this, SLOT(openClicked()));
    }
#endif // ENABLE_WALLET

    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I), this, SLOT(showInfo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C), this, SLOT(showDebugWindowActivateConsole()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D), this, SLOT(showDebugWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G), this, SLOT(showGraph()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_P), this, SLOT(showPeers()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R), this, SLOT(showRepair()));
}

void ElectrumGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    if(walletFrame)
    {
        file->addAction(openAction);
        file->addAction(backupWalletAction);
        file->addAction(signMessageAction);
        file->addAction(verifyMessageAction);
        file->addSeparator();
        file->addAction(usedSendingAddressesAction);
        file->addAction(usedReceivingAddressesAction);
        file->addSeparator();
        file->addAction(importPrivateKeyAction);
        file->addAction(exportMasterPrivateKeyAction);
    }
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    if(walletFrame)
    {
        settings->addAction(encryptWalletAction);
        settings->addAction(changePassphraseAction);
        settings->addAction(unlockWalletAction);
        settings->addAction(lockWalletAction);
        settings->addSeparator();
        settings->addAction(unlockStakingAction);
        settings->addAction(toggleStakingAction);
        settings->addSeparator();
        settings->addAction(generateColdStakingAction);
        settings->addAction(updatePriceAction);
        settings->addSeparator();
    }
    settings->addAction(optionsAction);

    QMenu* tools = appMenuBar->addMenu(tr("&Tools"));
    if (walletFrame) {
        tools->addAction(openInfoAction);
        tools->addAction(openRPCConsoleAction);
        tools->addAction(openGraphAction);
        tools->addAction(openPeersAction);
        //tools->addAction(openRepairAction);
        tools->addAction(repairWalletAction);
        tools->addSeparator();
    }

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(webInfoAction);
    help->addAction(showHelpMessageAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

void ElectrumGUI::createToolBars()
{
#ifdef ENABLE_WALLET
    if(walletFrame)
    {
        QToolBar* toolbar = new QToolBar(tr("Tabs toolbar"));
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->addAction(overviewAction);
        toolbar->addAction(sendCoinsAction);
        toolbar->addAction(receiveCoinsAction);
        toolbar->addAction(historyAction);

        /** Create additional container for toolbar and walletFrame and make it the central widget.
            This is a workaround mostly for toolbar styling on Mac OS but should work fine for every other OSes too.
        */
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(toolbar);
        layout->addWidget(walletFrame);
        layout->setSpacing(0);
        layout->setContentsMargins(QMargins());
        QWidget* containerWidget = new QWidget();
        containerWidget->setLayout(layout);
        setCentralWidget(containerWidget);

/*
 *      topMenu5 = new QPushButton();
 *      walletFrame->menuLayout->addWidget(topMenu5);
 *      topMenu5->setFixedSize(215,94);
 *      topMenu5->setObjectName("topMenu5");
 *      connect(topMenu5, SIGNAL(clicked()), this, SLOT(gotoCommunityFundPage()));
 *      topMenu5->setStyleSheet(
 *                  "#topMenu5 { background: url(:/icons/menu_community_fund_ns) bottom center #eee; border: 0; }"
 *                  "#topMenu5:hover { background: url(:/icons/menu_community_fund_hover) bottom center #ddd; border: 0; }");
 */

    }
#endif // ENABLE_WALLET
}

void ElectrumGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Create system tray menu (or setup the dock menu) that late to prevent users from calling actions,
        // while the client has not yet fully loaded
        createTrayIconMenu();

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        modalOverlay->setKnownBestHeight(clientModel->getHeaderTipHeight(), QDateTime::fromTime_t(clientModel->getHeaderTipTime()));
        setNumBlocks(clientModel->getNumBlocks(), clientModel->getLastBlockDate(), clientModel->getVerificationProgress(NULL), false);
        connect(clientModel, SIGNAL(numBlocksChanged(int,QDateTime,double,bool)), this, SLOT(setNumBlocks(int,QDateTime,double,bool)));

        // Receive and report messages from client model
        connect(clientModel, SIGNAL(message(QString,QString,unsigned int)), this, SLOT(message(QString,QString,unsigned int)));

        // Show progress dialog
        connect(clientModel, SIGNAL(showProgress(QString,int)), this, SLOT(showProgress(QString,int)));

        rpcConsole->setClientModel(clientModel);
#ifdef ENABLE_WALLET
        if(walletFrame)
        {
            walletFrame->setClientModel(clientModel);
        }
#endif // ENABLE_WALLET

        OptionsModel* optionsModel = clientModel->getOptionsModel();
        if(optionsModel)
        {
            // be aware of the tray icon disable state change reported by the OptionsModel object.
            connect(optionsModel,SIGNAL(hideTrayIconChanged(bool)),this,SLOT(setTrayIconVisible(bool)));

            // initialize the disable state of the tray icon with the current value in the model.
            setTrayIconVisible(optionsModel->getHideTrayIcon());

            // be aware of a display unit change reported by the OptionsModel object.
            connect(optionsModel,SIGNAL(displayUnitChanged(int)),this,SLOT(updateDisplayUnit(int)));

            // initialize the display units label with the current value in the model.
            updateDisplayUnit(optionsModel->getDisplayUnit());
        }
    } else {
        // Disable possibility to show main window via action
        toggleHideAction->setEnabled(false);
        if(trayIconMenu)
        {
            // Disable context menu on tray icon
            trayIconMenu->clear();
        }
    }
}

#ifdef ENABLE_WALLET
bool ElectrumGUI::addWallet(const QString& name, WalletModel *walletModel)
{
    if(!walletFrame)
        return false;
    setWalletActionsEnabled(true);
    return walletFrame->addWallet(name, walletModel);
}

bool ElectrumGUI::setCurrentWallet(const QString& name)
{
    if(!walletFrame)
        return false;
    return walletFrame->setCurrentWallet(name);
}

void ElectrumGUI::removeAllWallets()
{
    if(!walletFrame)
        return;
    setWalletActionsEnabled(false);
    walletFrame->removeAllWallets();
}

void ElectrumGUI::repairWallet()
{
    QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Repair wallet"),
        tr("Client restart required to repair the wallet.") + "<br><br>" + tr("Client will be shut down. Do you want to proceed?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if(btnRetVal == QMessageBox::Cancel)
        return;

    WriteConfigFile("zapwallettxes","2");
    WriteConfigFile("repairwallet","1");

    QApplication::quit();
}

#endif // ENABLE_WALLET

void ElectrumGUI::setWalletActionsEnabled(bool enabled)
{
    overviewAction->setEnabled(enabled);
    sendCoinsAction->setEnabled(enabled);
    sendCoinsMenuAction->setEnabled(enabled);
    receiveCoinsAction->setEnabled(enabled);
    receiveCoinsMenuAction->setEnabled(enabled);
    historyAction->setEnabled(enabled);
    encryptWalletAction->setEnabled(enabled);
    backupWalletAction->setEnabled(enabled);
    changePassphraseAction->setEnabled(enabled);
    signMessageAction->setEnabled(enabled);
    verifyMessageAction->setEnabled(enabled);
    usedSendingAddressesAction->setEnabled(enabled);
    usedReceivingAddressesAction->setEnabled(enabled);
    repairWalletAction->setEnabled(enabled);
    importPrivateKeyAction->setEnabled(enabled);
    openAction->setEnabled(enabled);
}

void ElectrumGUI::createTrayIcon(const NetworkStyle *networkStyle)
{
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("Electrum client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getTrayAndWindowIcon());
    trayIcon->hide();
    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

void ElectrumGUI::createTrayIconMenu()
{
#ifndef Q_OS_MAC
    // return if trayIcon is unset (only on non-Mac OSes)
    if (!trayIcon)
        return;

    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsMenuAction);
    trayIconMenu->addAction(receiveCoinsMenuAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(overviewAction);
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addAction(historyAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openInfoAction);
    trayIconMenu->addAction(openRPCConsoleAction);
    trayIconMenu->addAction(openGraphAction);
    trayIconMenu->addAction(openPeersAction);
    //trayIconMenu->addAction(openRepairAction);
    trayIconMenu->addAction(repairWalletAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif
}

#ifndef Q_OS_MAC
void ElectrumGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHidden();
    }
}
#endif

void ElectrumGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;

    OptionsDialog dlg(this, enableWallet);
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

/*
 *  void ElectrumGUI::cfundProposalsClicked()
 *  {
 *      if(!clientModel || !clientModel->getOptionsModel())
 *          return;
 *  }
 */
    void ElectrumGUI::cfundProposalsOpen(bool fMode)
    {
        if(!clientModel || !clientModel->getOptionsModel())
            return;
    }

    void ElectrumGUI::cfundPaymentRequestsClicked()
    {
        if(!clientModel || !clientModel->getOptionsModel())
            return;
    }

void ElectrumGUI::aboutClicked()
{
    if(!clientModel)
        return;

    HelpMessageDialog dlg(this, true);
    dlg.exec();
}

void ElectrumGUI::webInfoClicked()
{
    if(!clientModel)
        return;

    QString link = QString("https://ankh.foundation/electrum");
    QDesktopServices::openUrl(QUrl(link));
}

void ElectrumGUI::showDebugWindow()
{
    rpcConsole->showNormal();
    rpcConsole->show();
    rpcConsole->raise();
    rpcConsole->activateWindow();
}

void ElectrumGUI::showInfo()
{
    rpcConsole->setTabFocus(RPCConsole::TAB_INFO);
    showDebugWindow();
}

void ElectrumGUI::showDebugWindowActivateConsole()
{
    rpcConsole->setTabFocus(RPCConsole::TAB_CONSOLE);
    showDebugWindow();
}

void ElectrumGUI::showGraph()
{
    rpcConsole->setTabFocus(RPCConsole::TAB_GRAPH);
    showDebugWindow();
}

void ElectrumGUI::showPeers()
{
    rpcConsole->setTabFocus(RPCConsole::TAB_PEERS);
    showDebugWindow();
}

void ElectrumGUI::showHelpMessageClicked()
{
    helpMessageDialog->show();
}

#ifdef ENABLE_WALLET
void ElectrumGUI::openClicked()
{
    OpenURIDialog dlg(this);
    if(dlg.exec())
    {
        Q_EMIT receivedURI(dlg.getURI());
    }
}

void ElectrumGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    if (walletFrame) walletFrame->gotoOverviewPage();
}

void ElectrumGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    if (walletFrame) walletFrame->gotoHistoryPage();
}

/*
 * void ElectrumGUI::gotoCommunityFundPage()
 * {
 *  topMenu1->setStyleSheet(
 *     "#topMenu1 { border-image: url(:/icons/menu_home_ns)  0 0 0 0 stretch stretch; border: 0px; }"
 *     "#topMenu1:hover { border-image: url(:/icons/menu_home_hover)  0 0 0 0 stretch stretch; border: 0px; }");
 *  topMenu2->setStyleSheet(
 *              "#topMenu2 { border-image: url(:/icons/menu_send_ns)  0 0 0 0 stretch stretch; border: 0px; }"
 *              "#topMenu2:hover { border-image: url(:/icons/menu_send_hover)  0 0 0 0 stretch stretch; border: 0px; }");
 *  topMenu3->setStyleSheet(
 *              "#topMenu3 { border-image: url(:/icons/menu_receive_ns)  0 0 0 0 stretch stretch; border: 0px; }"
 *              "#topMenu3:hover { border-image: url(:/icons/menu_receive_hover)  0 0 0 0 stretch stretch; border: 0px; }");
 *  topMenu4->setStyleSheet(
 *              "#topMenu4 { border-image: url(:/icons/menu_transaction_ns)  0 0 0 0 stretch stretch; border: 0px; }"
 *              "#topMenu4:hover { border-image: url(:/icons/menu_transaction_hover)  0 0 0 0 stretch stretch; border: 0px; }");
 *
 *  topMenu5->setStyleSheet(
 *              "#topMenu5 { border-image: url(:/icons/menu_community_fund_s)  0 0 0 0 stretch stretch; border: 0px; }");
 *
 *  historyAction->setChecked(true);
 *  if (walletFrame) walletFrame->gotoCommunityFundPage();
 *}
 */

void ElectrumGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoReceiveCoinsPage();
}

void ElectrumGUI::gotoSendCoinsPage(QString addr)
{
    sendCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoSendCoinsPage(addr);
}

void ElectrumGUI::gotoSignMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoSignMessageTab(addr);
}

void ElectrumGUI::gotoVerifyMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoVerifyMessageTab(addr);
}
#endif // ENABLE_WALLET

void ElectrumGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    QIcon connectionItem = QIcon(icon).pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    labelConnectionsIcon->setIcon(connectionItem);
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to Electrum network", "", count));
}

bool showingVotingDialog = false;

void ElectrumGUI::updateHeadersSyncProgressLabel()
{
    int64_t headersTipTime = clientModel->getHeaderTipTime();
    int headersTipHeight = clientModel->getHeaderTipHeight();
    int estHeadersLeft = (GetTime() - headersTipTime) / Params().GetConsensus().nTargetSpacing;
    if (estHeadersLeft > HEADER_HEIGHT_DELTA_SYNC)
        progressBarLabel->setText(tr("Syncing Headers (%1%)...").arg(QString::number(100.0 / (headersTipHeight+estHeadersLeft)*headersTipHeight, 'f', 1)));
}

void ElectrumGUI::setNumBlocks(int count, const QDateTime& blockDate, double nVerificationProgress, bool header)
{
    if (modalOverlay)
    {
        if (header)
            modalOverlay->setKnownBestHeight(count, blockDate);
        else
            modalOverlay->tipUpdate(count, blockDate, nVerificationProgress);
    }

    if(!clientModel)
        return;

    // Prevent orphan statusbar messages (e.g. hover Quit in main menu, wait until chain-sync starts -> garbelled text)
    statusBar()->clearMessage();

    // Acquire current block source
    enum BlockSource blockSource = clientModel->getBlockSource();
    switch (blockSource) {
        case BLOCK_SOURCE_NETWORK:
            if (header) {
                updateHeadersSyncProgressLabel();
                return;
            }
                progressBarLabel->setText(tr("Synchronizing with network..."));
                updateHeadersSyncProgressLabel();
            break;
        case BLOCK_SOURCE_DISK:
            if (header)
                    progressBarLabel->setText(tr("Importing blocks on disk..."));
            break;
        case BLOCK_SOURCE_REINDEX:
                progressBarLabel->setText(tr("Reindexing blocks on disk..."));
            break;
        case BLOCK_SOURCE_NONE:
            if (header) {
                return;
            }
            // Case: not Importing, not Reindexing and no network connection
                progressBarLabel->setText(tr("Connecting to peers..."));
            break;
    }

    QString tooltip;

    QDateTime currentDate = QDateTime::currentDateTime();
    qint64 secs = blockDate.secsTo(currentDate);

    tooltip = tr("Processed %n block(s) of transaction history.", "", count);

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

#ifdef ENABLE_WALLET
        if(walletFrame)
        {
            walletFrame->showOutOfSyncWarning(false);
            modalOverlay->showHide(true, true);
        }
#endif // ENABLE_WALLET

        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }
    else
    {
        // Represent time from last generated block in human readable text
        QString timeBehindText;

        progressBarLabel->setVisible(true);
        progressBar->setFormat(tr("%1 behind").arg(timeBehindText));
        progressBar->setMaximum(1000000000);
        progressBar->setValue(nVerificationProgress * 1000000000.0 + 0.5);
        progressBar->setVisible(true);

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        if(count != prevBlocks)
        {
            labelBlocksIcon->setPixmap(QIcon(QString(
                ":/movies/spinner-%1").arg(spinnerFrame, 3, 10, QChar('0')))
                .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
            spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;
        }
        prevBlocks = count;

#ifdef ENABLE_WALLET
        if(walletFrame)
        {
            walletFrame->showOutOfSyncWarning(true);
            modalOverlay->showHide();
        }
#endif // ENABLE_WALLET

        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void ElectrumGUI::showVotingDialog()
{

  if(showingVotingDialog)
    return;

  showingVotingDialog = true;

  bool showVoting = !ExistsKeyInConfigFile("votefunding") &&
      pindexBestHeader->nTime > 1508284800 &&
      pindexBestHeader->nTime < 1510704000 &&
      GetBoolArg("-votefunding",true);

  bool vote = false;

  if(showVoting)
  {

    QMessageBox msgBox;
    msgBox.setText(tr("Important network notice."));
    msgBox.setInformativeText(tr("The Nav Coin Network is currently voting on introducing changes on the consensus protocol. As a participant in our network, we value your input and the decision ultimately is yours. Please cast your vote. <br><br>For more information on the proposal, please visit <a href=\"https://electrum.org/community-fund\">this link</a><br><br>Would you like the Nav Coin Network to update the staking rewards to setup a decentralised community fund that will help grow the network?"));
    QAbstractButton *myYesButton = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    msgBox.addButton(trUtf8("No"), QMessageBox::NoRole);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.exec();

    if(msgBox.clickedButton() == myYesButton)
    {
        vote = true;
    }

    SoftSetArg("-votefunding",vote?"1":"0",true);

    RemoveConfigFile("votefunding",vote?"0":"1");
    WriteConfigFile("votefunding",vote?"1":"0");

  }

  showingVotingDialog = false;

}

void ElectrumGUI::message(const QString &title, const QString &message, unsigned int style, bool *ret)
{
    QString strTitle = tr("Electrum"); // default title
    // Default to information icon
    int nMBoxIcon = QMessageBox::Information;
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    // Prefer supplied title over style based title
    if (!title.isEmpty()) {
        msgType = title;
    }
    else {
        switch (style) {
        case CClientUIInterface::MSG_ERROR:
            msgType = tr("Error");
            break;
        case CClientUIInterface::MSG_WARNING:
            msgType = tr("Warning");
            break;
        case CClientUIInterface::MSG_INFORMATION:
            msgType = tr("Information");
            break;
        default:
            break;
        }
    }
    // Append title to "Electrum - "
    if (!msgType.isEmpty())
        strTitle += " - " + msgType;

    // Check for error/warning icon
    if (style & CClientUIInterface::ICON_ERROR) {
        nMBoxIcon = QMessageBox::Critical;
        nNotifyIcon = Notificator::Critical;
    }
    else if (style & CClientUIInterface::ICON_WARNING) {
        nMBoxIcon = QMessageBox::Warning;
        nNotifyIcon = Notificator::Warning;
    }

    // Display message
    if (style & CClientUIInterface::MODAL) {
        // Check for buttons, use OK as default, if none was supplied
        QMessageBox::StandardButton buttons;
        if (!(buttons = (QMessageBox::StandardButton)(style & CClientUIInterface::BTN_MASK)))
            buttons = QMessageBox::Ok;

        showNormalIfMinimized();
        QMessageBox mBox((QMessageBox::Icon)nMBoxIcon, strTitle, message, buttons, this);
        int r = mBox.exec();
        if (ret != NULL)
            *ret = r == QMessageBox::Ok;
    }
    else
        notificator->notify((Notificator::Class)nNotifyIcon, strTitle, message);
}

void ElectrumGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel() && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void ElectrumGUI::closeEvent(QCloseEvent *event)
{
#ifndef Q_OS_MAC // Ignored on Mac
    if(clientModel && clientModel->getOptionsModel())
    {
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            // close rpcConsole in case it was open to make some space for the shutdown window
            rpcConsole->close();

            QApplication::quit();
        }
    }
#endif
    QMainWindow::closeEvent(event);
}

void ElectrumGUI::showEvent(QShowEvent *event)
{
    // enable the debug window when the main window shows up
    openInfoAction->setEnabled(true);
    openRPCConsoleAction->setEnabled(true);
    openGraphAction->setEnabled(true);
    openPeersAction->setEnabled(true);
    //openRepairAction->setEnabled(true);
    aboutAction->setEnabled(true);
    webInfoAction->setEnabled(true);
    optionsAction->setEnabled(true);
}

#ifdef ENABLE_WALLET
void ElectrumGUI::incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address, const QString& label)
{
    // On new transaction, make an info balloon
    QString msg = tr("Date: %1\n").arg(date) +
                  tr("Amount: %1\n").arg(ElectrumUnits::formatWithUnit(unit, amount, true)) +
                  tr("Type: %1\n").arg(type);
    if (!label.isEmpty())
        msg += tr("Label: %1\n").arg(label);
    else if (!address.isEmpty())
        msg += tr("Address: %1\n").arg(address);
    message((amount)<0 ? tr("Sent transaction") : tr("Incoming transaction"),
             msg, CClientUIInterface::MSG_INFORMATION);
}
#endif // ENABLE_WALLET

void ElectrumGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ElectrumGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        Q_FOREACH(const QUrl &uri, event->mimeData()->urls())
        {
            Q_EMIT receivedURI(uri.toString());
        }
    }
    event->acceptProposedAction();
}

bool ElectrumGUI::eventFilter(QObject *object, QEvent *event)
{
    // Catch status tip events
    if (event->type() == QEvent::StatusTip)
    {
        // Prevent adding text from setStatusTip(), if we currently use the status bar for displaying other stuff
        if (progressBarLabel->isVisible() || progressBar->isVisible())
            return true;
    }
    return QMainWindow::eventFilter(object, event);
}

#ifdef ENABLE_WALLET
bool ElectrumGUI::handlePaymentRequest(const SendCoinsRecipient& recipient)
{
    // URI has to be valid
    if (walletFrame && walletFrame->handlePaymentRequest(recipient))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
        return true;
    }
    return false;
}

void ElectrumGUI::setHDStatus(int hdEnabled)
{
    labelWalletHDStatusIcon->setPixmap(platformStyle->SingleColorIcon(hdEnabled ? ":/icons/hd_enabled" : ":/icons/hd_disabled").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
    labelWalletHDStatusIcon->setToolTip(hdEnabled ? tr("HD key generation is <b>enabled</b>") : tr("HD key generation is <b>disabled</b>"));

    // eventually disable the QLabel to set its opacity to 50%
    labelWalletHDStatusIcon->setEnabled(hdEnabled);
}

void ElectrumGUI::setEncryptionStatus(int status)
{
    if(fWalletUnlockStakingOnly)
    {
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked for staking only</b>"));
        changePassphraseAction->setEnabled(false);
        encryptWalletAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        unlockStakingAction->setVisible(false);
        lockWalletAction->setVisible(false);
    }
    else
    {
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        encryptWalletAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        unlockStakingAction->setVisible(false);
        lockWalletAction->setVisible(false);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(platformStyle->SingleColorIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        unlockWalletAction->setVisible(false);
        unlockStakingAction->setVisible(false);
        lockWalletAction->setVisible(false);
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(platformStyle->SingleColorIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        unlockWalletAction->setVisible(false);
        unlockStakingAction->setVisible(true);
        lockWalletAction->setVisible(false);
        break;
    }
    }
    updateStakingStatus();
}
#endif // ENABLE_WALLET

void ElectrumGUI::showNormalIfMinimized(bool fToggleHidden)
{
    if(!clientModel)
        return;

    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void ElectrumGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void ElectrumGUI::detectShutdown()
{
    if (ShutdownRequested())
    {
        if(rpcConsole)
            rpcConsole->hide();
        qApp->quit();
    }
}

void ElectrumGUI::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}

void ElectrumGUI::setTrayIconVisible(bool fHideTrayIcon)
{
    if (trayIcon)
    {
        trayIcon->setVisible(!fHideTrayIcon);
    }
}

void ElectrumGUI::showModalOverlay()
{
    if (modalOverlay && (progressBar->isVisible() || modalOverlay->isLayerVisible()))
        modalOverlay->toggleVisibility();
}

static bool ThreadSafeMessageBox(ElectrumGUI *gui, const std::string& message, const std::string& caption, unsigned int style)
{
    bool modal = (style & CClientUIInterface::MODAL);
    // The SECURE flag has no effect in the Qt GUI.
    // bool secure = (style & CClientUIInterface::SECURE);
    style &= ~CClientUIInterface::SECURE;
    bool ret = false;
    // In case of modal message, use blocking connection to wait for user to click a button
    QMetaObject::invokeMethod(gui, "message",
                               modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                               Q_ARG(QString, QString::fromStdString(caption)),
                               Q_ARG(QString, QString::fromStdString(message)),
                               Q_ARG(unsigned int, style),
                               Q_ARG(bool*, &ret));
    return ret;
}

void ElectrumGUI::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.ThreadSafeMessageBox.connect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
    uiInterface.ThreadSafeQuestion.connect(boost::bind(ThreadSafeMessageBox, this, _1, _3, _4));
}

void ElectrumGUI::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.ThreadSafeMessageBox.disconnect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
    uiInterface.ThreadSafeQuestion.disconnect(boost::bind(ThreadSafeMessageBox, this, _1, _3, _4));
}

/** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
void ElectrumGUI::updateDisplayUnit(int unit)
{
    // Update the list value
    unitDisplayControl->setCurrentText(ElectrumUnits::name(unit));
}

/** Update the display currency **/
void ElectrumGUI::comboBoxChanged(int index)
{
    // Make sure we have a client model
    if (!clientModel)
        return;

    // Get the unit
    QVariant unit = unitDisplayControl->itemData(index);

    // Use the unit
    clientModel->getOptionsModel()->setDisplayUnit(unit);
}

void ElectrumGUI::toggleStaking()
{
    SetStaking(!GetStaking());

    Q_EMIT message(tr("Staking"), GetStaking() ? tr("Staking has been enabled") : tr("Staking has been disabled"),
                   CClientUIInterface::MSG_INFORMATION);
}

void ElectrumGUI::generateColdStaking()
{
  ColdStakingWizard wizard;
  wizard.exec();
}

#ifdef ENABLE_WALLET


void ElectrumGUI::updateWeight()
{
    if (!pwalletMain)
        return;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    nWeight = pwalletMain->GetStakeWeight();
}

void ElectrumGUI::updatePrice()
{
    info("Updating prices");

    std::thread pThread{[this]{
        try {
            CURL *curl;
            CURLcode curlCode;
            std::string response;
            std::string url(
                    "https://min-api.cryptocompare.com/data/price?fsym=0AE&tsyms="
                    "BTC,"
                    "EUR,"
                    "USD,"
                    "ARS,"
                    "AUD,"
                    "BRL,"
                    "CAD,"
                    "CHF,"
                    "CLP,"
                    "CZK,"
                    "DKK,"
                    "GBP,"
                    "HKD,"
                    "HUF,"
                    "IDR,"
                    "ILS,"
                    "INR,"
                    "JPY,"
                    "KRW,"
                    "MXN,"
                    "MYR,"
                    "NOK,"
                    "NZD,"
                    "PHP,"
                    "PKR,"
                    "PLN,"
                    "RUB,"
                    "SEK,"
                    "SGD,"
                    "THB,"
                    "TRY,"
                    "TWD,"
                    "ZAR"
                    );

            // Start curl
            curl = curl_easy_init();

            // Check that we started
            if(!curl) {
                error("Update prices could not init curl");
                return;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, priceUdateWriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curlCode = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            // Parse json
            // NOTE: Had to use boost json as Q5's json support would not work with
            //       the json data that I was getting from the API, IDK why ¯\_(ツ)_/¯
            boost::property_tree::ptree json;
            std::istringstream jsonStream(response);
            boost::property_tree::read_json(jsonStream, json);

            // Get an instance of settings
            QSettings settings;

            // Save the values
            settings.setValue("btcFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("BTC"))) * 100000000);
            settings.setValue("eurFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("EUR"))) * 100000000);
            settings.setValue("usdFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("USD"))) * 100000000);
            settings.setValue("arsFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("ARS"))) * 100000000);
            settings.setValue("audFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("AUD"))) * 100000000);
            settings.setValue("brlFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("BRL"))) * 100000000);
            settings.setValue("cadFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("CAD"))) * 100000000);
            settings.setValue("chfFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("CHF"))) * 100000000);
            settings.setValue("clpFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("CLP"))) * 100000000);
            settings.setValue("czkFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("CZK"))) * 100000000);
            settings.setValue("dkkFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("DKK"))) * 100000000);
            settings.setValue("gbpFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("GBP"))) * 100000000);
            settings.setValue("hkdFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("HKD"))) * 100000000);
            settings.setValue("hufFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("HUF"))) * 100000000);
            settings.setValue("idrFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("IDR"))) * 100000000);
            settings.setValue("ilsFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("ILS"))) * 100000000);
            settings.setValue("inrFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("INR"))) * 100000000);
            settings.setValue("jpyFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("JPY"))) * 100000000);
            settings.setValue("krwFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("KRW"))) * 100000000);
            settings.setValue("mxnFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("MXN"))) * 100000000);
            settings.setValue("myrFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("MYR"))) * 100000000);
            settings.setValue("nokFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("NOK"))) * 100000000);
            settings.setValue("nzdFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("NZD"))) * 100000000);
            settings.setValue("phpFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("PHP"))) * 100000000);
            settings.setValue("pkrFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("PKR"))) * 100000000);
            settings.setValue("plnFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("PLN"))) * 100000000);
            settings.setValue("rubFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("RUB"))) * 100000000);
            settings.setValue("sekFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("SEK"))) * 100000000);
            settings.setValue("sgdFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("SGD"))) * 100000000);
            settings.setValue("thbFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("THB"))) * 100000000);
            settings.setValue("tryFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("TRY"))) * 100000000);
            settings.setValue("twdFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("TWD"))) * 100000000);
            settings.setValue("zarFactor", (1.0 / boost::lexical_cast<double>(json.get<std::string>("ZAR"))) * 100000000);

            if(clientModel)
                clientModel->getOptionsModel()->setDisplayUnit(clientModel->getOptionsModel()->getDisplayUnit());

            info("Updated prices");
        }
        catch (const boost::property_tree::json_parser::json_parser_error& e)
        {
            error("Could not parse price data json 'boost::property_tree::json_parser::json_parser_error'");
        }
        catch (const std::runtime_error& e)
        {
            error("Could not parse price data json 'std::runtime_error'");
        }
        catch (...)
        {
            error("Could not parse price data json 'drunk'");
        }
    }};

    // Make sure we don't get in anyones way :D
    pThread.detach();
}

size_t ElectrumGUI::priceUdateWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*) userp)->append((char*) contents, size * nmemb);
    return size * nmemb;
}

void ElectrumGUI::updateStakingStatus()
{
    updateWeight();

    if(walletFrame){

        if (nLastCoinStakeSearchInterval && nWeight)
        {
            bool fFoundProposal = false;
            bool fFoundPaymentRequest = false;
            {
                LOCK(cs_main);


              CProposalMap mapProposals;

              if(pcoinsTip->GetAllProposals(mapProposals))
              {
                   for (CProposalMap::iterator it_ = mapProposals.begin(); it_ != mapProposals.end(); it_++)
                   {
                       CFund::CProposal proposal;
                       if (!pcoinsTip->GetProposal(it_->first, proposal))
                           continue;
                       if (proposal.fState != CFund::NIL)
                           continue;
                       auto it = std::find_if( vAddedProposalVotes.begin(), vAddedProposalVotes.end(),
                                               [&proposal](const std::pair<std::string, int>& element){ return element.first == proposal.hash.ToString();} );
                       if (it == vAddedProposalVotes.end()) {
                           fFoundProposal = true;
                           break;
                       }
                   }
              }

            }
            {
                CPaymentRequestMap mapPaymentRequests;

                if(pcoinsTip->GetAllPaymentRequests(mapPaymentRequests))
                {
                    for (CPaymentRequestMap::iterator it_ = mapPaymentRequests.begin(); it_ != mapPaymentRequests.end(); it_++)
                    {
                        CFund::CPaymentRequest prequest;

                        if (!pcoinsTip->GetPaymentRequest(it_->first, prequest))
                            continue;

                        if (prequest.fState != CFund::NIL)
                            continue;
                        auto it = std::find_if( vAddedPaymentRequestVotes.begin(), vAddedPaymentRequestVotes.end(),
                                                [&prequest](const std::pair<std::string, int>& element){ return element.first == prequest.hash.ToString();} );
                        if (it == vAddedPaymentRequestVotes.end()) {
                            fFoundPaymentRequest = true;
                            break;
                        }
                    }
                }
            }
            if ((fFoundPaymentRequest || fFoundProposal) && !this->fDontShowAgain && (this->lastDialogShown + (60*60*24)) < GetTimeNow()) {
                QCheckBox *cb = new QCheckBox("Don't show this notification again until wallet is restarted.");
                QMessageBox msgbox(this);
                msgbox.setWindowTitle("Community Fund Update");
                QString sWhat = fFoundProposal && fFoundPaymentRequest ? tr("Proposals and Payment Requests") : (fFoundProposal ? tr("Proposals") : tr("Payment Requests"));
                msgbox.setText(tr("There are new %1 in the Community Fund.<br><br>As a staker it's important to engage in the voting process.<br><br>Please cast your vote using the Community Fund tab!").arg(sWhat));
                msgbox.setIcon(QMessageBox::Icon::Information);
                msgbox.setCheckBox(cb);
                QAbstractButton* pButtonInfo = msgbox.addButton(tr("Read about the Community Fund"), QMessageBox::YesRole);
                QAbstractButton* pButtonOpen = msgbox.addButton(tr("Open Community Fund"), QMessageBox::YesRole);
                QAbstractButton* pButtonClose = msgbox.addButton(tr("Close"), QMessageBox::RejectRole);
                pButtonClose->setVisible(false);
                this->lastDialogShown = GetTimeNow();

                msgbox.exec();

                if(cb->isChecked()) {
                    this->fDontShowAgain = true;
                } else {
                    this->fDontShowAgain = false;
                }

                //  if (msgbox.clickedButton()==pButtonOpen) {
                //      gotoCommunityFundPage();
                //  }
                if (msgbox.clickedButton()==pButtonInfo) {
                    QString link = QString("https://electrum.org/en/community-fund/");
                    QDesktopServices::openUrl(QUrl(link));
                }
            }

            uint64_t nWeight = this->nWeight;
            uint64_t nNetworkWeight = GetPoSKernelPS();
            int nBestHeight = pindexBestHeader->nHeight;

            unsigned nEstimateTime = GetTargetSpacing(nBestHeight) * nNetworkWeight / nWeight;

            QString text;
            if (nEstimateTime < 60)
            {
                text = tr("Expected time to earn reward is %n seconds(s)", "", nEstimateTime);
            }
            else if (nEstimateTime < 60*60)
            {
                text = tr("Expected time to earn reward is %n minute(s)", "", nEstimateTime/60);
            }
            else if (nEstimateTime < 24*60*60)
            {
                text = tr("Expected time to earn reward is %n hour(s)", "", nEstimateTime/(60*60));
            }
            else
            {
                text = tr("Expected time to earn reward is %n day(s)", "", nEstimateTime/(60*60*24));
            }

            nWeight /= COIN;
            nNetworkWeight /= COIN;

            labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
            labelStakingIcon->setToolTip(tr("Staking: On"));
        }
        else
        {
            labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
            if (pwalletMain && pwalletMain->IsLocked())
                labelStakingIcon->setToolTip(tr("Staking: Off (because wallet is locked)"));
            else if (vNodes.empty())
                labelStakingIcon->setToolTip(tr("Staking: Off (because wallet is offline)"));
            else if (IsInitialBlockDownload())
                labelStakingIcon->setToolTip(tr("Staking: Off (because wallet is syncing)"));
            else if (!nWeight)
                labelStakingIcon->setToolTip(tr("Staking: Off (because you don't have mature coins)"));
            else
                labelStakingIcon->setToolTip(tr("Staking: Off"));
        }

//        vStakePeriodRange_T aRange = PrepareRangeForStakeReport();
//        int nItemCounted = GetsStakeSubTotal(aRange);
//        if(ARRAYLEN(aRange) > 32){
//            walletFrame->setStakingStats(FormatMoney(aRange[30].Total).c_str(),FormatMoney(aRange[31].Total).c_str(),FormatMoney(aRange[32].Total).c_str());
//        }
    }

}

#endif
