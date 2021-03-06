// Copyright (c) 2018 alex v
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/coldstakingwizard.h>

#include <QDebug>

ColdStakingWizard::ColdStakingWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new IntroPage);
    addPage(new GetAddressesPage);
    addPage(new ColdStakingAddressPage);

    setWindowTitle(tr("Create a Cold Staking address"));
}

void ColdStakingWizard::accept()
{
    QDialog::accept();
}

IntroPage::IntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    label = new QLabel(tr("This wizard will help you generate a cold staking<br>"
                          "address where you can safely store coins while<br>"
                          "staking them.<br>"
                          "You will need to provide two addresses from different wallets:<br>"
                          " - a staking address: this address will be authorised to stake the<br>"
                          "      coins sent to the cold staking address.<br>"
                          " - a spending address: this address will be authorised to spend the<br>"
                          "      coins sent to the cold staking address.<br>"
                          "You can optionally specify a third address, which would hold the<br>"
                          "voting rights. This address should belong to a wallet compatible<br>"
                          "with the light wallet voting features like NavCash.<br>"
                          "These addresses will be used to generate a cold staking address."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

GetAddressesPage::GetAddressesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Specify the addresses"));

    stakingAddressLabel = new QLabel(tr("&Staking Address:"));
    stakingAddressLineEdit = new QLineEdit;
    stakingAddressLabel->setBuddy(stakingAddressLineEdit);

    spendingAddressLabel = new QLabel(tr("S&pending Address:"));
    spendingAddressLineEdit = new QLineEdit;
    spendingAddressLabel->setBuddy(spendingAddressLineEdit);

    votingAddressLabel = new QLabel(tr("&Voting Address:"));
    votingAddressLineEdit = new QLineEdit;
    votingAddressLabel->setBuddy(votingAddressLineEdit);

    descriptionLabel = new QLabel(tr("Your spending address and staking address must be different. Specifying a voting address is optional."));
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("QLabel { color : red }");

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(stakingAddressLabel, 0, 0);
    layout->addWidget(stakingAddressLineEdit, 0, 1);
    layout->addWidget(spendingAddressLabel, 1, 0);
    layout->addWidget(spendingAddressLineEdit, 1, 1);
    layout->addWidget(votingAddressLabel, 2, 0);
    layout->addWidget(votingAddressLineEdit, 2, 1);
    layout->addWidget(descriptionLabel, 3, 0, 1, 2);
    layout->addWidget(errorLabel, 4, 0, 1, 2);
    setLayout(layout);

    registerField("stakingAddress*", stakingAddressLineEdit);
    registerField("spendingAddress*", spendingAddressLineEdit);
    registerField("votingAddress", votingAddressLineEdit);
}

bool GetAddressesPage::validatePage()
{
    QString stakingAddressStr = field("stakingAddress").toString();
    QString spendingAddressStr = field("spendingAddress").toString();
    QString votingAddressStr = field("votingAddress").toString();

    CElectrumAddress stakingAddress(stakingAddressStr.toStdString());
    CElectrumAddress spendingAddress(spendingAddressStr.toStdString());

    CKeyID stakingKeyID;
    CKeyID spendingKeyID;

    if (field("stakingAddress").toString() == field("spendingAddress").toString())  {
        errorLabel->setText(tr("The addresses can't be the same!"));
        return false;
    }
    if(!(stakingAddress.IsValid() && stakingAddress.GetKeyID(stakingKeyID))) {
        errorLabel->setText("The staking address is not valid.");
        return false;
    }
    if(!(spendingAddress.IsValid() && spendingAddress.GetKeyID(spendingKeyID))) {
        errorLabel->setText("The spending address is not valid.");
        return false;
    }
    if (votingAddressStr != "") {
        CElectrumAddress votingAddress(votingAddressStr.toStdString());
        CKeyID votingKeyID;
        if(!(votingAddress.IsValid() && votingAddress.GetKeyID(votingKeyID))) {
            errorLabel->setText("The voting address is not valid.");
            return false;
        }
    }

    errorLabel->setText("");

    return true;
}

ColdStakingAddressPage::ColdStakingAddressPage(QWidget *parent)
    : QWizardPage(parent)
{

    setTitle(tr("Generated address"));

    image = new QRImageWidget;
    image->setObjectName("lblQRCode");
    image->setAlignment(Qt::AlignCenter);

    address = new QLineEdit();
    address->setReadOnly(true);
    address->setAlignment(Qt::AlignCenter);

    button = new QPushButton("&Copy to Clipboard");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(image);
    layout->addWidget(address);
    layout->addWidget(button);
    setLayout(layout);

    connect(button, SIGNAL(released()), this, SLOT(copyToClipboard()));
}

void ColdStakingAddressPage::initializePage()
{
    QString stakingAddressStr = field("stakingAddress").toString();
    QString spendingAddressStr = field("spendingAddress").toString();
    QString votingAddressStr = field("votingAddress").toString();


    CElectrumAddress stakingAddress(stakingAddressStr.toStdString());
    CKeyID stakingKeyID;
    stakingAddress.GetKeyID(stakingKeyID);

    CElectrumAddress spendingAddress(spendingAddressStr.toStdString());
    CKeyID spendingKeyID;
    spendingAddress.GetKeyID(spendingKeyID);

    if (votingAddressStr != "")
    {
        CElectrumAddress votingAddress(votingAddressStr.toStdString());
        CKeyID votingKeyID;
        votingAddress.GetKeyID(votingKeyID);
        coldStakingAddress = QString::fromStdString(CElectrumAddress(stakingKeyID, spendingKeyID, votingKeyID).ToString());
    }
    else
    {
        coldStakingAddress = QString::fromStdString(CElectrumAddress(stakingKeyID, spendingKeyID).ToString());
    }

#ifdef USE_QRCODE
    QString uri = "electrum:" + coldStakingAddress;
    image->setText("");
    if(!uri.isEmpty())
    {
        // limit URI length
        if (uri.length() > MAX_URI_LENGTH)
        {
            image->setText(tr("Resulting URI too long, try to reduce the text for label / message."));
        } else {
            QRcode *code = QRcode_encodeString(uri.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
            if (!code)
            {
                image->setText(tr("Error encoding URI into QR Code."));
                return;
            }
            QImage qrImage = QImage(code->width + 8, code->width + 8, QImage::Format_RGB32);
            qrImage.fill(0xffffff);
            unsigned char *p = code->data;
            for (int y = 0; y < code->width; y++)
            {
                for (int x = 0; x < code->width; x++)
                {
                    qrImage.setPixel(x + 4, y + 4, ((*p & 1) ? 0x0 : 0xffffff));
                    p++;
                }
            }
            QRcode_free(code);

            QImage qrAddrImage = QImage(QR_IMAGE_SIZE*2, QR_IMAGE_SIZE+20, QImage::Format_RGB32);
            qrAddrImage.fill(0xffffff);
            QPainter painter(&qrAddrImage);
            painter.drawImage(QR_IMAGE_SIZE/2, 0, qrImage.scaled(QR_IMAGE_SIZE, QR_IMAGE_SIZE));
            QFont font = GUIUtil::fixedPitchFont();
            font.setPixelSize(12);
            painter.setFont(font);
            QRect paddedRect = qrAddrImage.rect();
            paddedRect.setHeight(QR_IMAGE_SIZE+12);
            painter.drawText(paddedRect, Qt::AlignBottom|Qt::AlignCenter, coldStakingAddress);
            painter.end();

            image->setPixmap(QPixmap::fromImage(qrAddrImage));
        }
    }
#endif
}


void ColdStakingAddressPage::copyToClipboard()
{
    GUIUtil::setClipboard(coldStakingAddress);
}
