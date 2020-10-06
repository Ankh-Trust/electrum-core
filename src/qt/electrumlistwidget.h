// Copyright (c) 2019 The NavCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELECTRUMLISTWIDGET_H
#define ELECTRUMLISTWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

typedef std::function<bool(QString)> ValidatorFunc;

class ElectrumListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ElectrumListWidget(QWidget *parent, QString title, ValidatorFunc validator);

    QStringList getEntries();

private:
    QListWidget* listWidget;
    QLineEdit* addInput;
    QPushButton* removeBtn;
    QLabel* warningLbl;

    ValidatorFunc validatorFunc;

private Q_SLOTS:
    void onInsert();
    void onRemove();
    void onSelect(QListWidgetItem*);
};

#endif // ELECTRUMLISTWIDGET_H
