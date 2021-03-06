#include "ProposalDetailDialog.h"
#include "ui_ProposalDetailDialog.h"

#include "wallet.h"

ProposalDetailDialog::ProposalDetailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProposalDetailDialog)
{
    ui->setupUi(this);

    setParent(HXChain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->closeBtn->setStyleSheet(CANCELBTN_STYLE);

    ui->voteStateTableWidget->installEventFilter(this);
    ui->voteStateTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->voteStateTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->voteStateTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->voteStateTableWidget->setFrameShape(QFrame::NoFrame);
    ui->voteStateTableWidget->setMouseTracking(true);
    ui->voteStateTableWidget->setShowGrid(false);//隐藏表格线

    ui->voteStateTableWidget->horizontalHeader()->setSectionsClickable(true);
//    ui->voteStateTableWidget->horizontalHeader()->setFixedHeight(40);
    ui->voteStateTableWidget->horizontalHeader()->setVisible(true);
    ui->voteStateTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->voteStateTableWidget->setColumnWidth(0,300);
    ui->voteStateTableWidget->setColumnWidth(1,80);
    ui->voteStateTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->stackedWidget->setCurrentIndex(0);
    ui->typeStackedWidget->setCurrentIndex(0);

    ui->infoBtn->setCheckable(true);
    ui->voteStateBtn->setCheckable(true);
    ui->infoBtn->setChecked(true);
    ui->voteStateBtn->setChecked(false);
}

ProposalDetailDialog::~ProposalDetailDialog()
{
    delete ui;
}

void ProposalDetailDialog::pop()
{
    move(0,0);
    exec();
}

void ProposalDetailDialog::setProposal(QString _proposalId)
{
    ProposalInfo info = HXChain::getInstance()->proposalInfoMap.value(_proposalId);

    ui->proposalIdLabel->setText(info.proposalId);
    ui->proposerIdLabel->setText(info.proposer);
    ui->expirationTimeLabel->setText(info.expirationTime);

    int size = info.requiredAccounts.size();
    ui->voteStateTableWidget->setRowCount(0);
    ui->voteStateTableWidget->setRowCount(size);
    for(int i = 0; i < size; i++)
    {
        ui->voteStateTableWidget->setRowHeight(i,40);

        QString address = info.requiredAccounts.at(i);
        ui->voteStateTableWidget->setItem(i, 0, new QTableWidgetItem(QString("%1\n%2").
                                  arg(HXChain::getInstance()->guardAddressToName(address)).arg(address)));

        if(info.approvedKeys.contains(address))
        {
            ui->voteStateTableWidget->setItem(i, 1, new QTableWidgetItem(tr("approved")));
            ui->voteStateTableWidget->item(i,1)->setTextColor(QColor(0,255,0));
        }
        else if(info.disapprovedKeys.contains(address))
        {
            ui->voteStateTableWidget->setItem(i, 1, new QTableWidgetItem(tr("disapproved")));
            ui->voteStateTableWidget->item(i,1)->setTextColor(QColor(255,0,0));
        }
        else
        {
            ui->voteStateTableWidget->setItem(i, 1, new QTableWidgetItem(tr("not voted")));
        }

        tableWidgetSetItemZebraColor(ui->voteStateTableWidget);
    }


    QJsonObject object = QJsonDocument::fromJson(info.transactionStr.toLatin1()).object();
    switch (info.proposalOperationType)
    {
    case TRANSACTION_TYPE_COLDHOT:
    {
        ui->typeLabel->setText(tr("cold-hot trx"));
        ui->typeStackedWidget->setCurrentIndex(0);

        QJsonObject operationObject = object.value("operations").toArray().at(0).toArray().at(1).toObject();
        ui->assetLabel_hotCold->setText(operationObject.value("asset_symbol").toString());
        ui->amountLabel_hotCold->setText(operationObject.value("amount").toString());
        ui->sendLabel_hotCold->setText(operationObject.value("multi_account_withdraw").toString());
        ui->receiveLabel_hotCold->setText(operationObject.value("multi_account_deposit").toString());
        ui->memoLabel_hotCold->setText(operationObject.value("memo").toString());
    }
        break;
    case TRANSACTION_TYPE_CHANGE_ASSET_ACCOUNT:
    {
        ui->typeLabel->setText(tr("update multisig-address"));
        ui->typeStackedWidget->setCurrentIndex(1);

        QJsonObject operationObject = object.value("operations").toArray().at(0).toArray().at(1).toObject();
        ui->assetLabel_updateMS->setText(operationObject.value("chain_type").toString());
        ui->coldAddressLabel_updateMS->setText(operationObject.value("cold").toString());
        ui->hotAddressLabel_updateMS->setText(operationObject.value("hot").toString());
    }
        break;
    case TRANSACTION_TYPE_SET_PUBLISHER:
    {
        ui->typeLabel->setText(tr("set publisher"));
        ui->typeStackedWidget->setCurrentIndex(2);

        QJsonObject operationObject = object.value("operations").toArray().at(0).toArray().at(1).toObject();
        ui->assetLabel_publisher->setText(operationObject.value("asset_symbol").toString());
        ui->publisherLabel_publisher->setText(operationObject.value("publisher").toString());
    }
        break;
    case TRANSACTION_TYPE_COLDHOT_CANCEL:
    {
        ui->typeLabel->setText(tr("cancel cold-hot trx"));
        ui->typeStackedWidget->setCurrentIndex(3);

        QJsonObject operationObject = object.value("operations").toArray().at(0).toArray().at(1).toObject();
        ui->coldHotTrxIdLabel->setText(operationObject.value("trx_id").toString());
    }
        break;
    case TRANSACTION_TYPE_WITHDRAW_CANCEL:
    {
        ui->typeLabel->setText(tr("cancel withdraw trx"));
        ui->typeStackedWidget->setCurrentIndex(4);

        QJsonObject operationObject = object.value("operations").toArray().at(0).toArray().at(1).toObject();
        ui->withdrawTrxIdLabel->setText(operationObject.value("not_enough_sign_trx_id").toString());
    }
        break;
    default:
        break;
    }
}

void ProposalDetailDialog::on_infoBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);   
    ui->infoBtn->setChecked(true);
    ui->voteStateBtn->setChecked(false);
}

void ProposalDetailDialog::on_voteStateBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->infoBtn->setChecked(false);
    ui->voteStateBtn->setChecked(true);
}

void ProposalDetailDialog::on_closeBtn_clicked()
{
    close();
}
