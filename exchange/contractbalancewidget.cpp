#include "contractbalancewidget.h"
#include "ui_contractbalancewidget.h"

#include "wallet.h"
#include "depositexchangecontractdialog.h"
#include "withdrawexchangecontractdialog.h"
#include "ToolButtonWidget.h"

ContractBalanceWidget::ContractBalanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContractBalanceWidget)
{
    ui->setupUi(this);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->balancesTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->balancesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->balancesTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->balancesTableWidget->setFrameShape(QFrame::NoFrame);
    ui->balancesTableWidget->setMouseTracking(true);
    ui->balancesTableWidget->setShowGrid(false);//隐藏表格线
    ui->balancesTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->balancesTableWidget->horizontalHeader()->setFixedHeight(35);
    ui->balancesTableWidget->horizontalHeader()->setVisible(true);
    ui->balancesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->balancesTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->balancesTableWidget->verticalHeader()->setVisible(false);
    ui->balancesTableWidget->setColumnWidth(0,250);
    ui->balancesTableWidget->setColumnWidth(1,250);
    ui->balancesTableWidget->setColumnWidth(2,160);

    ui->balancesTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->depositBtn->setStyleSheet(TOOLBUTTON_STYLE_1);

    init();
}

ContractBalanceWidget::~ContractBalanceWidget()
{
    delete ui;
}

void ContractBalanceWidget::init()
{

}

void ContractBalanceWidget::setAccount(QString _accountName)
{
    accountName = _accountName;

    QString contractAddress = UBChain::getInstance()->getExchangeContractAddress(accountName);

    if(contractAddress.isEmpty())
    {
    }
    else
    {
        UBChain::getInstance()->postRPC( "id-invoke_contract_offline-owner_assets-" + accountName, toJsonFormat( "invoke_contract_offline",
                                                                               QJsonArray() << accountName << contractAddress
                                                                               << "owner_assets"  << ""));
    }

}

void ContractBalanceWidget::jsonDataUpdated(QString id)
{
    if( id == "id-invoke_contract_offline-owner_assets-" + accountName )
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonObject object = QJsonDocument::fromJson(jsonObject.take("result").toString().toLatin1()).object();

            ExchangeContractBalances balances;
            foreach (QString key, object.keys())
            {
                QJsonValue amountValue = object.take(key);
                unsigned long long amount = 0;

                if(amountValue.isString())
                {
                    amount = amountValue.toString().toULongLong();
                }
                else
                {
                    amount = QString::number(amountValue.toDouble(),'g',12).toULongLong();
                }

                balances.insert(key,amount);
            }

            UBChain::getInstance()->accountExchangeContractBalancesMap.insert(accountName,balances);
        }

        showContractBalances();

        return;
    }
}

void ContractBalanceWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(248,249,253),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(248,249,253),Qt::SolidPattern));

    painter.drawRect(rect());
}

void ContractBalanceWidget::showContractBalances()
{
    ExchangeContractBalances balances = UBChain::getInstance()->accountExchangeContractBalancesMap.value(accountName);

    QStringList keys = balances.keys();
    int size = keys.size();
    ui->balancesTableWidget->setRowCount(0);
    ui->balancesTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        QString key = keys.at(i);
        AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(UBChain::getInstance()->getAssetId(key));

        ui->balancesTableWidget->setItem(i,0, new QTableWidgetItem(key));
        ui->balancesTableWidget->setItem(i,1, new QTableWidgetItem(getBigNumberString(balances.value(key), assetInfo.precision)));
        ui->balancesTableWidget->setItem(i,2, new QTableWidgetItem(tr("withdraw")));

        for(int j = 0; j < 3; j++)
        {
            ui->balancesTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
            if(i%2)
            {
                ui->balancesTableWidget->item(i,j)->setBackgroundColor(QColor(255,255,255));
            }
            else
            {
                ui->balancesTableWidget->item(i,j)->setBackgroundColor(QColor(252,253,255));
            }
        }


        ToolButtonWidgetItem *toolButtonItem = new ToolButtonWidgetItem(i,2);
        toolButtonItem->setInitGray(false);
        toolButtonItem->setText(ui->balancesTableWidget->item(i,2)->text());
        ui->balancesTableWidget->setCellWidget(i,2,toolButtonItem);
        connect(toolButtonItem,SIGNAL(itemClicked(int,int)),this,SLOT(onItemClicked(int,int)));

    }

}

void ContractBalanceWidget::on_depositBtn_clicked()
{
    QString contractAddress = UBChain::getInstance()->getExchangeContractAddress(accountName);

    if(contractAddress.isEmpty())
    {

    }
    else
    {
        DepositExchangeContractDialog depositExchangeContractDialog;
        depositExchangeContractDialog.pop();
    }

}

void ContractBalanceWidget::onItemClicked(int _row, int _column)
{
    if(_column == 2)
    {
        // 合约提现
        WithdrawExchangeContractDialog withdrawExchangeContractDialog;
        withdrawExchangeContractDialog.setCurrentAsset(ui->balancesTableWidget->item(_row,0)->text());
        withdrawExchangeContractDialog.pop();
        return;
    }
}