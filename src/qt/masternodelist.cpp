#include "masternodelist.h"
#include "ui_masternodelist.h"
#include "newproposaldialog.h"
#include "governance-validators.h"
//#include "rpc/server.h"
//#include "rpc/client.h"
#include "masternodeconfig.h"
#include "messagesigner.h"

//#include "rpcconsole.h"


#include "activemasternode.h"
#include "clientmodel.h"
#include "init.h"
#include "guiutil.h"
#include "governance.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "sync.h"
#include "wallet/wallet.h"
#include "walletmodel.h"

//#include "governance-exceptions.h"

#include <QTimer>
#include <QMessageBox>
#include <QSettings>
#include <QStyledItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QToolTip>

const QSize FONT_RANGE(4, 40);
const char fontSizeSettingsKey[] = "msgFontSize";


int nFilterVoteRequiredOnly=1;
int nFilterActiveOnly = 1;
int nProposalCount=0;
int GetNextSuperblockTime();
std::string strSelectedProposalChannelAddress;

int GetOffsetFromUtc()
{
#if QT_VERSION < 0x050200
    const QDateTime dateTime1 = QDateTime::currentDateTime();
    const QDateTime dateTime2 = QDateTime(dateTime1.date(), dateTime1.time(), Qt::UTC);
    return dateTime1.secsTo(dateTime2);
#else
    return QDateTime::currentDateTime().offsetFromUtc();
#endif
}


class MessageSimpleViewDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

void MessageSimpleViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();
    QTextDocument doc;
    QString align(index.data(MessageModel::TypeRole) < 2 ? "right" : "left");//changed 20180831
    QString margin_left(index.data(MessageModel::TypeRole) < 2 ? "margin-left:250;" :"margin-left:0;");
    QString margin_right(index.data(MessageModel::TypeRole) < 2 ? "margin-right:0;" :"margin-right:200;");
    QString background_color("");
    //QString background_color(index.data(MessageModel::UnreadFlagRole).toBool() ? "background-color:rgb(245, 245, 245);" : "");
    QString html;
    html = "<hr align=\"left\" width=\"100%\">";
    html += "<p align=\"" + align + "\" style=\"font-size:8px;"+background_color+margin_left+margin_right
            +"margin-top:1px; margin-bottom:1px\">" + index.data(MessageModel::ReceivedDateRole).toString() + "</p>";
    html += "<p align=\"" + align + "\" style=\"font-size:8px;"+background_color+margin_left+margin_right
            +"margin-top:1px; margin-bottom:1px\">" + index.data(MessageModel::LabelRole).toString() + " (";
    html += index.data(MessageModel::FromAddressRole).toString() + ")</p>";
    html += "<p align=\"" + align + "\" style=\""+ background_color + "; margin-top:10px; "
            +margin_left+margin_right+"margin-bottom:12px\">" + index.data(MessageModel::ShortMessageRole).toString() + "</p>";


    //QString unreadFlagTag = index.data(MessageModel::UnreadFlagRole).toBool() ? "background-color:rgb(245, 245, 245)" : "";

    doc.setHtml(html);

    // Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText).lighter(190));


    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);

    doc.setTextWidth( textRect.width() );
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize MessageSimpleViewDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);
    QTextDocument doc;
    doc.setHtml(index.data(MessageModel::HTMLRole).toString());
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height()+40);//20);
}



MasternodeList::MasternodeList(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodeList),
    clientModel(0),
    walletModel(0),
    messageModel(0),
    msgdelegate (new MessageSimpleViewDelegate())
{
    ui->setupUi(this);

    ui->startButton->setEnabled(false);

    int columnAliasWidth = 100;
    int columnAddressWidth = 200;
    int columnProtocolWidth = 60;
    int columnStatusWidth = 80;
    int columnActiveWidth = 130;
    int columnLastSeenWidth = 130;

    ui->tableWidgetMasternodes->setColumnWidth(0, columnAddressWidth);
    ui->tableWidgetMasternodes->setColumnWidth(1, columnProtocolWidth);
    ui->tableWidgetMasternodes->setColumnWidth(2, columnStatusWidth);
    ui->tableWidgetMasternodes->setColumnWidth(3, columnActiveWidth);
    ui->tableWidgetMasternodes->setColumnWidth(4, columnLastSeenWidth);

    ui->tableWidgetMasternodes->setVisible(ui->checkboxShowAllNodes->isChecked());
    ui->tableWidgetMyMasternodes2->setColumnWidth(0, columnAliasWidth);
    ui->tableWidgetMyMasternodes2->setColumnWidth(1, columnAddressWidth);
    ui->tableWidgetMyMasternodes2->setColumnWidth(2, columnProtocolWidth);
    ui->tableWidgetMyMasternodes2->setColumnWidth(3, columnStatusWidth);
    ui->tableWidgetMyMasternodes2->setColumnWidth(4, columnActiveWidth);
    ui->tableWidgetMyMasternodes2->setColumnWidth(5, columnLastSeenWidth);


    ui->tableWidgetMyMasternodes2->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *startAliasAction = new QAction(tr("Start alias"), this);
    contextMenu = new QMenu();
    contextMenu->addAction(startAliasAction);
    connect(ui->tableWidgetMyMasternodes2, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(startAliasAction, SIGNAL(triggered()), this, SLOT(on_startButton_clicked()));

    ui->tableWidgetProposals->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *voteAction = new QAction(tr("Vote YES"), this);
    QAction *voteNoAction = new QAction(tr("Vote NO"), this);
    QAction *voteAbstainAction = new QAction(tr("Vote ABSTAIN"), this);
    //QAction *voteDeleteAction = new QAction(tr("Vote DELETE"), this);
    QAction *showDetailsAction = new QAction(tr("Show details..."), this);
    contextMenuProposalsTab = new QMenu();
    contextMenuProposalsTab->addAction(voteAction);
    contextMenuProposalsTab->addAction(voteNoAction);
    contextMenuProposalsTab->addAction(voteAbstainAction);
    //contextMenuProposalsTab->addAction(voteDeleteAction);
    contextMenuProposalsTab->addSeparator();
    contextMenuProposalsTab->addSeparator();
    contextMenuProposalsTab->addAction(showDetailsAction);

    connect(ui->tableWidgetProposals, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenuProposalTab(const QPoint&)));
    connect(voteAction, SIGNAL(triggered()), this, SLOT(voteYesAction()));
    connect(voteNoAction, SIGNAL(triggered()), this, SLOT(voteNoActionSLOT()));
    connect(voteAbstainAction, SIGNAL(triggered()), this, SLOT(voteAbstainActionSLOT()));
    //connect(voteDeleteAction, SIGNAL(triggered()), this, SLOT(voteDeleteActionSLOT()));
    connect(showDetailsAction, SIGNAL(triggered()), this, SLOT(showDetailsActionSLOT()));


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMyNodeList()));
    //connect(timer, SIGNAL(timeout()), this, SLOT(updateProposalList()));
    timer->start(1000);

    fFilterUpdated = false;
    nTimeFilterUpdated = GetTime();
    updateNodeList();
    updateProposalList();
    ui->tableWidgetProposals->installEventFilter(this);
    ui->lineeditMessage->installEventFilter(this);
    ui->bnFontBigger->setVisible(false);
    ui->bnFontSmaller->setVisible(false);
    ui->textBrowser->setVisible(false);
    ui->label_filter1->setVisible(false);
    ui->filterProposalLineEdit->setVisible(false);

    QSettings settings;

    ui->splitterMain->restoreState(settings.value("splitterMain_ProposalTab").toByteArray());
    ui->splitterHorizontal->restoreState(settings.value("splitterHorizontal_ProposalTab").toByteArray());
}

MasternodeList::~MasternodeList()
{
    QSettings settings;
    settings.setValue("splitterMain_ProposalTab", ui->splitterMain->saveState());
    settings.setValue("splitterHorizontal_ProposalTab", ui->splitterHorizontal->saveState());
    delete ui;
}

void MasternodeList::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model) {
        // try to update list when masternode count changes
        connect(clientModel, SIGNAL(strMasternodesChanged(QString)), this, SLOT(updateNodeList()));
        //TODO: Explore a reasonability connect to update proposals list.
    }
}

void MasternodeList::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void MasternodeList::showContextMenu(const QPoint &point)
{
    QTableWidgetItem *item = ui->tableWidgetMyMasternodes2->itemAt(point);
    if(item) contextMenu->exec(QCursor::pos());
}

void MasternodeList::showContextMenuProposalTab(const QPoint &point)
{
    QTableWidgetItem *item = ui->tableWidgetProposals->itemAt(point);
    if(item) contextMenuProposalsTab->exec(QCursor::pos());
}

void MasternodeList::voteAction(std::string vote, std::string strProposalHash = "")
{
    //std::string strProposalHash;
    
    {
        LOCK(cs_gobjectslist);
        // Find selected proposal hash
        QItemSelectionModel* selectionModel = ui->tableWidgetProposals->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();
        
        if(selected.count() == 0) return;
        
        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        if (strProposalHash == "")
            strProposalHash = ui->tableWidgetProposals->item(nSelectedRow, 8)->text().toStdString();
    }
    uint256 hash;
    //std::string strVote;

    hash = uint256S(strProposalHash);
    std::string strVoteSignal, strVoteOutcome;
    if (vote == "yes" || vote == "no" || vote =="abstain"){
        strVoteSignal = "funding";
        strVoteOutcome = vote;
    }
        else if (vote == "delete"){
        strVoteSignal = "delete";
        strVoteOutcome = "yes";
    };

//    std::string strVoteOutcome = vote;//"yes";

    vote_signal_enum_t eVoteSignal = CGovernanceVoting::ConvertVoteSignal(strVoteSignal);

    vote_outcome_enum_t eVoteOutcome = CGovernanceVoting::ConvertVoteOutcome(strVoteOutcome);

    int nSuccessful = 0;
    int nFailed = 0;

    std::vector<CMasternodeConfig::CMasternodeEntry> mnEntries;
    mnEntries = masternodeConfig.getEntries();

    UniValue resultsObj(UniValue::VOBJ);

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, mnEntries) {
        CPubKey pubKeyMasternode;
        CKey keyMasternode;

        UniValue statusObj(UniValue::VOBJ);

        if(!CMessageSigner::GetKeysFromSecret(mne.getPrivKey(), keyMasternode, pubKeyMasternode)){
            nFailed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "Masternode signing error, could not set key correctly"));
            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
            continue;
        }

        uint256 nTxHash;
        nTxHash.SetHex(mne.getTxHash());

        int nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        COutPoint outpoint(nTxHash, nOutputIndex);

        CMasternode mn;
        bool fMnFound = mnodeman.Get(outpoint, mn);

        if(!fMnFound) {
            nFailed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "Can't find masternode by collateral output"));
            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
            continue;
        }

        CGovernanceVote vote(mn.vin.prevout, hash, eVoteSignal, eVoteOutcome);
        if(!vote.Sign(keyMasternode, pubKeyMasternode)){
            nFailed++;
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "Failure to sign."));
            resultsObj.push_back(Pair(mne.getAlias(), statusObj));
            continue;
        }

        CGovernanceException exception;
        if(governance.ProcessVoteAndRelay(vote, exception, *g_connman)) {
            nSuccessful++;
            statusObj.push_back(Pair("result", "success"));
        }
        else {
            nFailed++;
            statusObj.push_back(Pair("result", "failed"));
//            statusObj.push_back(Pair("errorMessage", exception.GetMessage()));
        }

        resultsObj.push_back(Pair(mne.getAlias(), statusObj));
    }

    UniValue returnObj(UniValue::VOBJ);
    returnObj.push_back(Pair("overall", strprintf("Voted successfully %d time(s) and failed %d time(s).", nSuccessful, nFailed)));
    returnObj.push_back(Pair("detail", resultsObj));


    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(returnObj[0].get_str()));//  ..get_str()));
    //msgBox.setInformativeText(strErrorMessages.c_str());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
    updateProposalList(true);
}

void MasternodeList::StartAlias(std::string strAlias)
{
    std::string strStatusHtml;
    strStatusHtml += "<center>Alias: " + strAlias;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        if(mne.getAlias() == strAlias) {
            std::string strError;
            CMasternodeBroadcast mnb;

            bool fSuccess = CMasternodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

            if(fSuccess) {
                strStatusHtml += "<br>Successfully started masternode.";
                mnodeman.UpdateMasternodeList(mnb, *g_connman);
                mnb.Relay(*g_connman);
                mnodeman.NotifyMasternodeUpdates(*g_connman);
            } else {
                strStatusHtml += "<br>Failed to start masternode.<br>Error: " + strError;
            }
            break;
        }
    }
    strStatusHtml += "</center>";

    QMessageBox msg;
    msg.setText(QString::fromStdString(strStatusHtml));
    msg.exec();

    updateMyNodeList(true);
}

void MasternodeList::StartAll(std::string strCommand)
{
    int nCountSuccessful = 0;
    int nCountFailed = 0;
    std::string strFailedHtml;

    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        std::string strError;
        CMasternodeBroadcast mnb;

        int32_t nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        COutPoint outpoint = COutPoint(uint256S(mne.getTxHash()), nOutputIndex);

        if(strCommand == "start-missing" && mnodeman.Has(outpoint)) continue;

        bool fSuccess = CMasternodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

        if(fSuccess) {
            nCountSuccessful++;
            mnodeman.UpdateMasternodeList(mnb, *g_connman);
            mnb.Relay(*g_connman);
            mnodeman.NotifyMasternodeUpdates(*g_connman);
        } else {
            nCountFailed++;
            strFailedHtml += "\nFailed to start " + mne.getAlias() + ". Error: " + strError;
        }
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = strprintf("Successfully started %d masternodes, failed to start %d, total %d", nCountSuccessful, nCountFailed, nCountFailed + nCountSuccessful);
    if (nCountFailed > 0) {
        returnObj += strFailedHtml;
    }

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();

    updateMyNodeList(true);
}

void MasternodeList::updateMyMasternodeInfo(QString strAlias, QString strAddr, const COutPoint& outpoint)
{
    bool fOldRowFound = false;
    int nNewRow = 0;

    for(int i = 0; i < ui->tableWidgetMyMasternodes2->rowCount(); i++) {
            if(ui->tableWidgetMyMasternodes2->item(i, 0)->text() == strAlias) {
                fOldRowFound = true;
                nNewRow = i;
                break;
            }

    }

    if(nNewRow == 0 && !fOldRowFound) {
        nNewRow = ui->tableWidgetMyMasternodes2->rowCount();
        ui->tableWidgetMyMasternodes2->insertRow(nNewRow);
    }

    masternode_info_t infoMn;
    bool fFound = mnodeman.GetMasternodeInfo(outpoint, infoMn);

    QTableWidgetItem *aliasItem = new QTableWidgetItem(strAlias);
    QTableWidgetItem *addrItem = new QTableWidgetItem(fFound ? QString::fromStdString(infoMn.addr.ToString()) : strAddr);
    QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(fFound ? infoMn.nProtocolVersion : -1));
    QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(fFound ? CMasternode::StateToString(infoMn.nActiveState) : "MISSING"));
    QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(fFound ? (infoMn.nTimeLastPing - infoMn.sigTime) : 0)));
    QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M",
                                                                                                   fFound ? infoMn.nTimeLastPing + GetOffsetFromUtc() : 0)));
    QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(fFound ? CBitcoinAddress(infoMn.pubKeyCollateralAddress.GetID()).ToString() : ""));


    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 0, aliasItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 1, addrItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 2, protocolItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 3, statusItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 4, activeSecondsItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 5, lastSeenItem);
    ui->tableWidgetMyMasternodes2->setItem(nNewRow, 6, pubkeyItem);
}

void MasternodeList::updateMyNodeList(bool fForce)
{
    TRY_LOCK(cs_mymnlist, fLockAcquired);
    if(!fLockAcquired) {
        return;
    }
    static int64_t nTimeMyListUpdated = 0;

    // automatically update my masternode list only once in MY_MASTERNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t nSecondsTillUpdate = nTimeMyListUpdated + MY_MASTERNODELIST_UPDATE_SECONDS - GetTime();
    ui->secondsLabel->setText(QString::number(nSecondsTillUpdate));
    ui->labelProposalTabSeconds->setText(QString::number(nSecondsTillUpdate));


    if(nSecondsTillUpdate > 0 && !fForce) return;
    nTimeMyListUpdated = GetTime();

    ui->tableWidgetMasternodes->setSortingEnabled(false);
    BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
        int32_t nOutputIndex = 0;
        if(!ParseInt32(mne.getOutputIndex(), &nOutputIndex)) {
            continue;
        }

        updateMyMasternodeInfo(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), COutPoint(uint256S(mne.getTxHash()), nOutputIndex));
    }
    ui->tableWidgetMasternodes->setSortingEnabled(true);

    // reset "timer"
    ui->secondsLabel->setText("0");//TODO change label
}

void MasternodeList::updateNodeList()
{
    TRY_LOCK(cs_mnlist, fLockAcquired);
    if(!fLockAcquired) {
        return;
    }

    static int64_t nTimeListUpdated = GetTime();

    // to prevent high cpu usage update only once in MASTERNODELIST_UPDATE_SECONDS seconds
    // or MASTERNODELIST_FILTER_COOLDOWN_SECONDS seconds after filter was last changed
    int64_t nSecondsToWait = fFilterUpdated
                            ? nTimeFilterUpdated - GetTime() + MASTERNODELIST_FILTER_COOLDOWN_SECONDS
                            : nTimeListUpdated - GetTime() + MASTERNODELIST_UPDATE_SECONDS;

    if(fFilterUpdated) ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", nSecondsToWait)));
    if(nSecondsToWait > 0) return;

    nTimeListUpdated = GetTime();
    fFilterUpdated = false;

    QString strToFilter;
    ui->countLabel->setText("Updating...");
    ui->tableWidgetMasternodes->setSortingEnabled(false);
    ui->tableWidgetMasternodes->clearContents();
    ui->tableWidgetMasternodes->setRowCount(0);
    std::map<COutPoint, CMasternode> mapMasternodes = mnodeman.GetFullMasternodeMap();
    int offsetFromUtc = GetOffsetFromUtc();

    for(auto& mnpair : mapMasternodes)
    {
        CMasternode mn = mnpair.second;
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
        QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(mn.nProtocolVersion));
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(mn.GetStatus()));
        QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(mn.lastPing.sigTime - mn.sigTime)));
        QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", mn.lastPing.sigTime + offsetFromUtc)));
        QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(CBitcoinAddress(mn.pubKeyCollateralAddress.GetID()).ToString()));

        if (strCurrentFilter != "")
        {
            strToFilter =   addressItem->text() + " " +
                            protocolItem->text() + " " +
                            statusItem->text() + " " +
                            activeSecondsItem->text() + " " +
                            lastSeenItem->text() + " " +
                            pubkeyItem->text();
            if (!strToFilter.contains(strCurrentFilter)) continue;
        }

        ui->tableWidgetMasternodes->insertRow(0);
        ui->tableWidgetMasternodes->setItem(0, 0, addressItem);
        ui->tableWidgetMasternodes->setItem(0, 1, protocolItem);
        ui->tableWidgetMasternodes->setItem(0, 2, statusItem);
        ui->tableWidgetMasternodes->setItem(0, 3, activeSecondsItem);
        ui->tableWidgetMasternodes->setItem(0, 4, lastSeenItem);
        ui->tableWidgetMasternodes->setItem(0, 5, pubkeyItem);
    }

    ui->countLabel->setText(QString::number(ui->tableWidgetMasternodes->rowCount()));
    ui->tableWidgetMasternodes->setSortingEnabled(true);

    updateProposalList();
}

//force update list after vote.
void MasternodeList::updateProposalList(bool fForceUpdate)
{

    TRY_LOCK(cs_gobjectslist, fLockAcquired);
    if(!fLockAcquired) {
        return;
    }

    if (!fForceUpdate)
        if (governance.Count(GOVERNANCE_OBJECT_PROPOSAL) == nProposalCount)
            return;

    ui->tableWidgetProposals->setSortingEnabled(false);
    ui->tableWidgetProposals->clearContents();
    ui->tableWidgetProposals->setRowCount(0);
    std::vector<CGovernanceObject*> objs = governance.GetAllNewerThan(0);//nStartTime);


    governance.UpdateLastDiffTime(GetTime());
    //int offsetFromUtc = GetOffsetFromUtc();

    BOOST_FOREACH(CGovernanceObject* pGovObj, objs)
    {
        std::string strName, strData, strPayment_address, strUrl, strDescription, strSmsg_addr, strSmsg_pubkey, strSmsg_privkey;
        int nEnd_epoch, nPayment_amount, nStart_epoch;
        std::string strMyVote;
        if (pGovObj->GetObjectType() != GOVERNANCE_OBJECT_PROPOSAL) continue;
        uint256 hash = pGovObj->GetHash();
        COutPoint mnCollateralOutpoint;

        int nAbsoluteYesVoteCount = pGovObj->GetAbsoluteYesCount(VOTE_SIGNAL_FUNDING);

        BOOST_FOREACH(CMasternodeConfig::CMasternodeEntry mne, masternodeConfig.getEntries()) {
            int32_t vout = boost::lexical_cast<uint32_t>(mne.getOutputIndex());
            uint256 txid = uint256S(mne.getTxHash());
            mnCollateralOutpoint = COutPoint(txid, vout);
            LOCK(governance.cs);
            std::vector<CGovernanceVote> vecVotes = governance.GetCurrentVotes(hash, mnCollateralOutpoint);
            BOOST_FOREACH(CGovernanceVote vote, vecVotes) {
                strMyVote += vote.GetSignalString(); strMyVote +=":";
                strMyVote += vote.GetVoteString(); strMyVote +="(";
                strMyVote += GUIUtil::dateTimeStr(vote.GetTimestamp()).toStdString(); strMyVote +="); ";
            }
        }
        try
        {
            strData = pGovObj->GetDataAsString();
            UniValue objJSON = pGovObj->GetJSONObject();
            strName = objJSON["name"].get_str();
            nEnd_epoch = objJSON["end_epoch"].get_int();
            strPayment_address = objJSON["payment_address"].get_str();
            nPayment_amount = objJSON["payment_amount"].get_int();
            nStart_epoch = objJSON["start_epoch"].get_int();
            strUrl = objJSON["url"].get_str();
            strDescription = objJSON["description"].get_str();
            strSmsg_addr = objJSON["smsg_addr"].get_str();
            strSmsg_pubkey = objJSON["smsg_pubkey"].get_str();
            strSmsg_privkey = objJSON["smsg_privkey"].get_str();
        }//end try

        catch(std::exception& e) {
            std::string strErrorMessages = std::string(e.what()) + std::string(";");
            //error(strErrorMessages);
            break;
        }
        catch(...) {
            //strErrorMessages += "Unknown exception;";
            break;
        }

        if (!strMyVote.empty() && nFilterVoteRequiredOnly) continue;

        //if (nFilterActiveOnly && (nEnd_epoch-Params().GetConsensus().nBudgetPaymentsCycleBlocks/2 > GetNextSuperblockTime())) continue;
        if (nFilterActiveOnly && (GetNextSuperblockTime()-nEnd_epoch)>0) continue;



        QTableWidgetItem *proposalNameItem = new QTableWidgetItem(QString::fromStdString(strName));
        QTableWidgetItem *hashItem = new QTableWidgetItem(QString::fromStdString(pGovObj->GetHash().ToString()));

        QTableWidgetItem *collateralHashItem = new QTableWidgetItem(QString::fromStdString(pGovObj->GetCollateralHash().ToString()));

        //QTableWidgetItem *creationTimeItem = new QTableWidgetItem(GUIUtil::dateTimeStr(pGovObj->GetCreationTime()));
        QTableWidgetItem *creationTimeItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", (pGovObj->GetCreationTime()))));
        //DateTimeStrFormat("%Y-%m-%d", nStart_epoch)

//        QTableWidgetItem *startEpochItem = new QTableWidgetItem(GUIUtil::dateTimeStr(nStart_epoch+Params().GetConsensus().nBudgetPaymentsCycleBlocks/2));
//        QTableWidgetItem *endEpochItem = new QTableWidgetItem(GUIUtil::dateTimeStr(nEnd_epoch-Params().GetConsensus().nBudgetPaymentsCycleBlocks/2));

        QTableWidgetItem *startEpochItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", nStart_epoch+Params().GetConsensus().nBudgetPaymentsCycleBlocks/2)));
        QTableWidgetItem *endEpochItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", nEnd_epoch-Params().GetConsensus().nBudgetPaymentsCycleBlocks/2)));


        QTableWidgetItem *lenghtEpochItem = new QTableWidgetItem(QString::number((nEnd_epoch-nStart_epoch)
                                                                                 / (Params().GetConsensus().nPowTargetSpacing
                                                                                    * Params().GetConsensus().nBudgetPaymentsCycleBlocks)));
        QTableWidgetItem *paymentAmountItem = new QTableWidgetItem(QString::number(nPayment_amount));
        QTableWidgetItem *paymentAddressItem = new QTableWidgetItem(QString::fromStdString(strPayment_address));
        QTableWidgetItem *voteSignalItem = new QTableWidgetItem(QString::fromStdString(strMyVote));

        QTableWidgetItem *absoluteYesVotes = new QTableWidgetItem(QString::number((nAbsoluteYesVoteCount)));

//        if (strCurrentFilter != "")
//        {
//            strToFilter =   addressItem->text() + " " +
//                            protocolItem->text() + " " +
//                            statusItem->text() + " " +
//                            activeSecondsItem->text() + " " +
//                            lastSeenItem->text() + " " +
//                            pubkeyItem->text();
//            if (!strToFilter.contains(strCurrentFilter)) continue;
//        }

        ui->tableWidgetProposals->insertRow(0);

        CBitcoinAddress paymentAddress(strPayment_address);
        ui->tableWidgetProposals->setItem(0, 0, proposalNameItem);
        ui->tableWidgetProposals->setItem(0, 1, creationTimeItem);
        ui->tableWidgetProposals->setItem(0, 2, absoluteYesVotes);
        ui->tableWidgetProposals->setItem(0, 3, paymentAddressItem);
        ui->tableWidgetProposals->setItem(0, 4, paymentAmountItem);
        ui->tableWidgetProposals->setItem(0, 5, startEpochItem);
        ui->tableWidgetProposals->setItem(0, 6, lenghtEpochItem);
        ui->tableWidgetProposals->setItem(0, 7, endEpochItem);

        ui->tableWidgetProposals->setItem(0, 8, hashItem);
        ui->tableWidgetProposals->setItem(0, 9, collateralHashItem);
        ui->tableWidgetProposals->setItem(0, 10, voteSignalItem);

        if (IsMine(*pwalletMain, paymentAddress.Get()))
        {
            //ui->tableWidgetProposals->item(0,0)->setBackground(Qt::red);
            QFont font;
            font.setBold(true);
            ui->tableWidgetProposals->item(0,0)->setFont(font);
            //ui->tableWidgetProposals->item(0,2)->setBackground(QColor::fromRgb(255,0,0));
        }

        if (nAbsoluteYesVoteCount>10){
            ui->tableWidgetProposals->item(0,1)->setBackground(QColor::fromRgb(0,200,200));
            ui->tableWidgetProposals->item(0,2)->setBackground(QColor::fromRgb(0,200,200));
            ui->tableWidgetProposals->item(0,3)->setBackground(QColor::fromRgb(0,200,200));
            ui->tableWidgetProposals->item(0,4)->setBackground(QColor::fromRgb(0,200,200));
            ui->tableWidgetProposals->item(0,5)->setBackground(QColor::fromRgb(0,200,200));
            ui->tableWidgetProposals->item(0,6)->setBackground(QColor::fromRgb(0,200,200));
        }
    }



    //new proposal notify if voting power//TODO
    //if (masternodeConfig.getCount()>0)


    nProposalCount = governance.Count(GOVERNANCE_OBJECT_PROPOSAL);
    //ui->countProposalsLabel->setText(QString::number(ui->tableWidgetProposals->rowCount()));
    ui->countProposalsLabel->setText(QString::number(nProposalCount));
    ui->tableWidgetProposals->setSortingEnabled(true);
    // reset "timer"
    ui->secondsLabel->setText("0");
}


void MasternodeList::on_filterLineEdit_textChanged(const QString &strFilterIn)
{
    strCurrentFilter = strFilterIn;
    nTimeFilterUpdated = GetTime();
    fFilterUpdated = true;
    ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", MASTERNODELIST_FILTER_COOLDOWN_SECONDS)));
}


void MasternodeList::on_filterProposalLineEdit_textChanged(const QString &strFilterIn)
{
    strCurrentProposalFilter = strFilterIn;
    nTimeFilterUpdated = GetTime();
    fFilterProposalUpdated = true;
    ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", MASTERNODELIST_FILTER_COOLDOWN_SECONDS)));
}


void MasternodeList::showDetailsActionSLOT()
{
    std::string strProposalHash;
    {
        LOCK(cs_gobjectslist);
        // Find selected proposal hash
        QItemSelectionModel* selectionModel = ui->tableWidgetProposals->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if(selected.count() == 0) return;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strProposalHash = ui->tableWidgetProposals->item(nSelectedRow, 8)->text().toStdString();
        NewProposalDialog* dlg_NewProposalDialog = new NewProposalDialog(strProposalHash);//strProposalHash);//selection.at(0));
        dlg_NewProposalDialog->setWalletModel(walletModel);
        dlg_NewProposalDialog->showNormal();
        dlg_NewProposalDialog->show();
        dlg_NewProposalDialog->raise();
        dlg_NewProposalDialog->activateWindow();
    }
}


void MasternodeList::voteYesAction()
{
    voteAction("yes", "");
}

void MasternodeList::voteNoActionSLOT()
{
    voteAction("no", "");
}
void MasternodeList::voteAbstainActionSLOT()
{
    voteAction("abstain", "");
}
void MasternodeList::voteDeleteActionSLOT()
{
    voteAction("delete", "");
}


void MasternodeList::on_startButton_clicked()
{
    std::string strAlias;
    {
        LOCK(cs_mymnlist);
        // Find selected node alias
        QItemSelectionModel* selectionModel = ui->tableWidgetMyMasternodes2->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if(selected.count() == 0) return;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strAlias = ui->tableWidgetMyMasternodes2->item(nSelectedRow, 0)->text().toStdString();
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm masternode start"),
        tr("Are you sure you want to start masternode %1?").arg(QString::fromStdString(strAlias)),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if(!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAlias(strAlias);
        return;
    }

    StartAlias(strAlias);
}

void MasternodeList::on_startAllButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm all masternodes start"),
        tr("Are you sure you want to start ALL masternodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if(!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAll();
        return;
    }

    StartAll();
}

void MasternodeList::on_startMissingButton_clicked()
{
    if(!masternodeSync.IsMasternodeListSynced()) {
        QMessageBox::critical(this, tr("Command is not available right now"),
                              tr("You can't use this command until masternode list is synced"));
        return;
    }
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this,
                                                               tr("Confirm missing masternodes start"),
                                                               tr("Are you sure you want to start MISSING masternodes?"),
                                                               QMessageBox::Yes | QMessageBox::Cancel,
                                                               QMessageBox::Cancel);
    if(retval != QMessageBox::Yes) return;
    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForMixingOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if(!ctx.isValid()) return; // Unlock wallet was cancelled
        StartAll("start-missing");
        return;
    }
    StartAll("start-missing");
}

void MasternodeList::on_tableWidgetMyMasternodes2_itemSelectionChanged()
{
    if(ui->tableWidgetMyMasternodes2->selectedItems().count() > 0) {
        ui->startButton->setEnabled(true);
    }
}

void MasternodeList::on_UpdateButton_clicked()
{
    updateMyNodeList(true);
}

void MasternodeList::on_checkboxShowAllNodes_stateChanged(int i_checkboxShowAllNodes_state)
{
    //ui->startButton->setEnabled(!i_cbShowAllNodes_state);
    ui->tableWidgetMyMasternodes2->setVisible(!i_checkboxShowAllNodes_state);
    ui->tableWidgetMasternodes->setVisible(i_checkboxShowAllNodes_state);
}

void MasternodeList::on_buttonUpdateList_clicked()
{
    updateProposalList();
}

void MasternodeList::on_buttonNewProposal_clicked()
{
    NewProposalDialog* dlg_NewProposalDialog = new NewProposalDialog("");//selection.at(0));
    dlg_NewProposalDialog->setWalletModel(walletModel);
    dlg_NewProposalDialog->showNormal();
    dlg_NewProposalDialog->show();
    dlg_NewProposalDialog->raise();
    dlg_NewProposalDialog->activateWindow();
}

void MasternodeList::on_checkboxProposaslToVote_stateChanged(int arg1)
{
    nFilterVoteRequiredOnly = arg1;
    updateProposalList(true);
}

void MasternodeList::on_tableWidgetProposals_itemDoubleClicked()//QTableWidgetItem *item)
{
    showDetailsActionSLOT();
}

int GetSuperblockTime()
{
    //return 0;
    // Compute last/next superblock
    int nLastSuperblock, nNextSuperblock;

    // Get current block height
    int nBlockHeight = 0;
    {
        LOCK(cs_main);
        nBlockHeight = (int)chainActive.Height();
    }
   // getgovernanceinfo();

    // Get chain parameters
    int nSuperblockStartBlock = Params().GetConsensus().nSuperblockStartBlock;
    int nSuperblockCycle = Params().GetConsensus().nSuperblockCycle;

    // Get first superblock
    int nFirstSuperblockOffset = (nSuperblockCycle - nSuperblockStartBlock % nSuperblockCycle) % nSuperblockCycle;
    int nFirstSuperblock = nSuperblockStartBlock + nFirstSuperblockOffset;

    if(nBlockHeight < nFirstSuperblock){
        nLastSuperblock = 0;
        nNextSuperblock = nFirstSuperblock;
    } else {
        nLastSuperblock = nBlockHeight - nBlockHeight % nSuperblockCycle;
        nNextSuperblock = nLastSuperblock + nSuperblockCycle;
    }


    return nNextSuperblock*Params().GetConsensus().nPowTargetSpacing
            +chainActive.Genesis()->GetBlockTime();

}

void MasternodeList::on_cbActiveOnly_stateChanged(int arg1)
{
    nFilterActiveOnly = arg1;
    updateProposalList(true);
}

void MasternodeList::on_tableWidgetProposals_itemActivated()
{
}

void MasternodeList::on_tableWidgetProposals_itemSelectionChanged()
{
    //ui->textBrowser->setVisible(false);
//    labelProposalAmountAndDays
//    labelProposalFromToDate
//    labelProposalName
//    labelTotalProposalBudget
//    ProposalDescription_plainTextEdit
//    bnVoteAbstain
//    bnVoteNo
//    bnVoteYes
//    bnSendMessage
//    lineeditMessage

    std::string strProposalHash;
    {
        LOCK(cs_gobjectslist);
        // Find selected proposal hash
        QItemSelectionModel* selectionModel = ui->tableWidgetProposals->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();

        if(selected.count() == 0) return;

        QModelIndex index = selected.at(0);
        int nSelectedRow = index.row();
        strProposalHash = ui->tableWidgetProposals->item(nSelectedRow, 8)->text().toStdString();
        ui->ProposalHash_lineedit->setText(strProposalHash.c_str());
        uint256 hash = uint256S(strProposalHash);
        CGovernanceObject* pGovObj = governance.FindGovernanceObject(hash);

        if (!pGovObj) return; //only GOVERNANCE_OBJECT_PROPOSAL can be opened
        UniValue objJSON = pGovObj->GetJSONObject();
        std::string strName = objJSON["name"].get_str();
        //std::string strWindowTitle = "Aywa Core - Proposal "+strName;
        int nEnd_epoch = objJSON["end_epoch"].get_int();
        //std::string strPayment_address = objJSON["payment_address"].get_str();
        int nPayment_amount = objJSON["payment_amount"].get_int();
        int nStart_epoch = objJSON["start_epoch"].get_int();
        std::string strUrl = objJSON["url"].get_str();
        std::string strProposalChannelAddress, strProposalChannelPrivKey, strProposalChannelPubKey;
        try{//some proposals dont have a description
            strProposalChannelAddress = objJSON["smsg_addr"].get_str();
            strProposalChannelPubKey = objJSON["smsg_pubkey"].get_str();
            strProposalChannelPrivKey = objJSON["smsg_privkey"].get_str();
            std::string strDescription = objJSON["description"].get_str();
            std::vector<unsigned char> v = ParseHex(strDescription);
            std::string strProposalDescription(v.begin(), v.end());
            ui->ProposalDescription_plainTextEdit->setHtml(strProposalDescription.c_str());
        }
        catch(std::exception& e)
        {
            ui->ProposalDescription_plainTextEdit->setPlainText(e.what());
        };

        ui->labelProposalName->setText(strName.c_str());
        ui->ProposalHash_lineedit->setReadOnly(true);
        //ui->ProposalHash_lineedit->

        ui->labelProposalFromToDate->setText(QString::fromStdString(DateTimeStrFormat("%d/%m/%Y", nStart_epoch)+" - "+DateTimeStrFormat("%d/%m/%Y", nEnd_epoch)));

        ui->ProposalDescription_plainTextEdit->setReadOnly(true);

        ui->labelProposalAmountAndDays->setText(QString::number(nPayment_amount) + QString::fromStdString(" AYWA for ") + QString::number((nEnd_epoch - nStart_epoch)/86400) + QString::fromStdString(" day(s)"));

        ui->labelTotalProposalBudget->setText(QString::fromStdString("Total budget: ") + QString::number(nPayment_amount*((nEnd_epoch - nStart_epoch)/86400)) + QString::fromStdString(" AYWA"));
        //ui->ProposalDescription_plainTextEdit->setText(strPayment_address.c_str());
        //ui->lineeditPaymentAddress->setReadOnly(true);
        //ui->dateeditPaymentStartDate->setDateTime(QDateTime::fromTime_t(nStart_epoch+43200));// + Params().GetConsensus().nBudgetPaymentsCycleBlocks * Params().GetConsensus().nPowTargetSpacing / 2));
        //ui->dateeditPaymentStartDate->setEnabled(false);
        //ui->dateeditPaymentEndDate->setDateTime(QDateTime::fromTime_t(nEnd_epoch-43200));// - Params().GetConsensus().nBudgetPaymentsCycleBlocks * Params().GetConsensus().nPowTargetSpacing / 2));
        //ui->dateeditPaymentEndDate->setEnabled(false);
        //ui->spinboxAmount->setValue(nPayment_amount);
        //ui->spinboxAmount->setEnabled(false);
        //const Consensus::Params& consensusParams = Params().GetConsensus();
        //const int nBudgetPaymentsCycleBlocks = consensusParams.nBudgetPaymentsCycleBlocks;
        //ui->spinboxPeriod->setValue((nEnd_epoch - nStart_epoch)/86400);

        //ui->spinboxPeriod->setEnabled(false);

        //ui->lineeditProposalUrl->setText(strUrl.c_str());
        //ui->lineeditProposalUrl->setReadOnly(true);

        //ui->lineeditPrivateChatAddress->setText(strProposalChannelAddress.c_str());
        //ui->lineeditPrivateChatPubKey->setText(strProposalChannelPubKey.c_str());
        //ui->lineeditChannelPrivKey->setText(strProposalChannelPrivKey.c_str());

        //ui->bnJoinChannel->setEnabled(!GetIsChannelSubscribed(ui->lineeditPrivateChatAddress->text().toStdString()));

        //ui->lineeditPrivateChatAddress->setReadOnly(true);
        //ui->pushbuttonCheck->setEnabled(false);
        //ui->toolbuttonSelectPaymentAddress->setEnabled(false);

        if (!GetIsChannelSubscribed (strProposalChannelAddress))
            SetChannelSubscribtion(strProposalChannelAddress, strProposalChannelPubKey,
                                   strProposalChannelPrivKey, std::string("PR-") + strName);
        QListView * listViewConversation = ui->listViewConversation;
        auto proxyModelSelectedContactFilter = new QSortFilterProxyModel(this);
        proxyModelSelectedContactFilter->setSourceModel(messageModel);
        QString filter = QString::fromStdString(strProposalChannelAddress);
        proxyModelSelectedContactFilter->setFilterRole(false);
        proxyModelSelectedContactFilter->setFilterFixedString("");
        proxyModelSelectedContactFilter->sort(MessageModel::SentDateTime);
        proxyModelSelectedContactFilter->setFilterRole(MessageModel::FilterAddressRole);
        proxyModelSelectedContactFilter->setFilterFixedString(filter);
        listViewConversation->setItemDelegate(msgdelegate);
        listViewConversation->setModel(proxyModelSelectedContactFilter);
        listViewConversation->scrollToBottom();

        strSelectedProposalChannelAddress = strProposalChannelAddress;
    }

    if (walletModel->getEncryptionStatus() == WalletModel::Locked || walletModel->getEncryptionStatus() == WalletModel::UnlockedForMixingOnly) {
        //ui->lineeditProposalName->setFocus();
        QToolTip::showText(ui->tableWidgetProposals->mapToGlobal(QPoint()), tr("Unlock wallet to use a proposal chat."));
        return;
    }


    //ui->lineeditMessage->setFocus();
}


void MasternodeList::fontBigger()
{
    setFontSize(msgFontSize+1);
}

void MasternodeList::fontSmaller()
{
    setFontSize(msgFontSize-1);
}

void MasternodeList::setFontSize(int newSize)
{
    QSettings settings;

    //don't allow a insane font size
    if (newSize < FONT_RANGE.width() || newSize > FONT_RANGE.height())
        return;

    // temp. store content
    QString str = ui->ProposalDescription_plainTextEdit->toHtml();

    // replace font tags size in current content
    str.replace(QString("font-size:%1pt").arg(msgFontSize), QString("font-size:%1pt").arg(newSize));

    // store the new font size
    msgFontSize = newSize;
    settings.setValue(fontSizeSettingsKey, msgFontSize);

    // clear console (reset icon sizes, default stylesheet) and re-add the content
    //float oldPosFactor = 1.0 / ui->ProposalDescription_plainTextEdit->verticalScrollBar()->maximum() * ui->ProposalDescription_plainTextEdit->verticalScrollBar()->value();
    //clear(false);
    ui->ProposalDescription_plainTextEdit->setHtml(str);
    //ui->ProposalDescription_plainTextEdit->verticalScrollBar()->setValue(oldPosFactor * ui->ProposalDescription_plainTextEdit->verticalScrollBar()->maximum());
}

void MasternodeList::on_bnFontSmaller_clicked()
{
    fontBigger();
}

void MasternodeList::on_bnFontBigger_clicked()
{
    fontSmaller();
}



void MasternodeList::on_bnVoteYes_clicked()
{
  voteYesAction();
}

void MasternodeList::on_bnVoteNo_clicked()
{
    voteNoActionSLOT();
}

void MasternodeList::on_bnVoteAbstain_clicked()
{
    voteAbstainActionSLOT();
}

void MasternodeList::on_bnSendMessage_clicked()
{


    if(!messageModel)
        return;

    if (ui->lineeditMessage->text().isEmpty())
        return;

   if (ui->tableWidgetProposals->selectedItems().size() == 0)
       return;

    std::string sError;
    std::string sendTo  = strSelectedProposalChannelAddress;
    std::string message = ui->lineeditMessage->text().toStdString();

    std::string strSenderAccountAddress;
    for (std::vector<SecMsgAddress>::iterator it = smsgAddresses.begin(); it != smsgAddresses.end(); ++it)
    {
        CBitcoinAddress coinAddress(it->sAddress);
        strSenderAccountAddress = coinAddress.ToString();
        break;
    }

    if (sendTo.empty() || message.empty() || strSenderAccountAddress.empty())
        return;

    if (SecureMsgSend(strSenderAccountAddress, sendTo, message, sError) != 0)
    {
        QMessageBox::warning(NULL, tr("Send Secure Message"),
            tr("Send failed: %1.").arg(sError.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);

        return;
    };
    ui->lineeditMessage->clear();
    ui->lineeditMessage->setFocus();
}

void MasternodeList::setMessageModel(MessageModel *messageModel)
{
    this->messageModel = messageModel;
    if(!messageModel)
        return;

    //if (model->proxyModel)
    //    delete model->proxyModel;
    messageModel->proxyModel = new QSortFilterProxyModel(this);
    messageModel->proxyModel->setSourceModel(messageModel);
    messageModel->proxyModel->setDynamicSortFilter(true);
    messageModel->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    messageModel->proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    messageModel->proxyModel->sort(MessageModel::SentDateTime);
    messageModel->proxyModel->setFilterRole(MessageModel::Ambiguous);
    messageModel->proxyModel->setFilterFixedString("true");

    //connect (messageModel, SIGNAL(updateMessagePage()), this, SLOT(updateMessages()));
    connect(messageModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(incomingMessage()));


}

bool MasternodeList::eventFilter(QObject* obj, QEvent *event)
{
    if (obj != ui->lineeditMessage)
        if (obj !=ui->tableWidgetProposals)
        return QWidget::eventFilter(obj, event);

    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up:
            return QWidget::eventFilter(obj, event);
            //if(obj == ui->lineEdit) { browseHistory(-1); return true; }
            break;
        case Qt::Key_Down:
            return QWidget::eventFilter(obj, event);
            //if(obj == ui->lineEdit) { browseHistory(1); return true; }
            break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            //if(obj == ui->lineEdit)
            //{
            //    QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
            //    return true;
            //}
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // forward these events
            if (!mod && obj == ui->lineeditMessage) {
                event->ignore();
                //sendMessage();
                ui->bnSendMessage->animateClick();
                return true;
            }
            break;
        default:
           {
                ui->lineeditMessage->setFocus();
                //TODO:Skipping 1 symbol
                //return QWidget::eventFilter(ui->lineeditMessage, event);
                return false;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MasternodeList::on_listViewConversation_doubleClicked(const QModelIndex &index)
{
    QMessageBox::information(0, index.data(Qt::DisplayRole).toString()+" message", index.data(MessageModel::HTMLRole).toString());

}

void MasternodeList::incomingMessage()
{
    ui->listViewConversation->scrollToBottom();
}
