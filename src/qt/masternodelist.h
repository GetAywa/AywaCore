#ifndef MASTERNODELIST_H
#define MASTERNODELIST_H

#include "primitives/transaction.h"
#include "platformstyle.h"
#include "sync.h"
#include "util.h"

//#include "rpcconsole.h" //???

#include <QMenu>
#include <QTimer>
#include <QWidget>

#define MY_MASTERNODELIST_UPDATE_SECONDS                 60
#define MASTERNODELIST_UPDATE_SECONDS                    15
#define MASTERNODELIST_FILTER_COOLDOWN_SECONDS            3

namespace Ui {
    class MasternodeList;
}



class ClientModel;
class WalletModel;
//class RPCExecutor;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Masternode Manager page widget */
class MasternodeList : public QWidget
{
    Q_OBJECT

public:
    explicit MasternodeList(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~MasternodeList();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void StartAlias(std::string strAlias);
    void StartAll(std::string strCommand = "start-all");
    void voteAction(std::string vote, std::string strProposalHash);

private:
    QMenu *contextMenu, *contextMenuProposalsTab;
    int64_t nTimeFilterUpdated;
    bool fFilterUpdated;
    bool fFilterProposalUpdated;

public Q_SLOTS:
    void updateMyMasternodeInfo(QString strAlias, QString strAddr, const COutPoint& outpoint);
    void updateMyNodeList(bool fForce = false);
    void updateNodeList();
    void updateProposalList(bool fForceUpdate = false);

Q_SIGNALS:

    /**  Fired when a message should be reported to the user */
    //void message(const QString &title, const QString &message, unsigned int style); //TODO notificator


private:
    QTimer *timer;
    Ui::MasternodeList *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

    // Protects tableWidgetMasternodes
    CCriticalSection cs_mnlist;

    // Protects tableWidgetMyMasternodes
    CCriticalSection cs_mymnlist;

    // Protects tableWidgetProposals
    CCriticalSection cs_gobjectslist;

    QString strCurrentFilter;
    QString strCurrentProposalFilter;

private Q_SLOTS:
    void showContextMenu(const QPoint &);
    void showContextMenuProposalTab(const QPoint &);
    void on_filterLineEdit_textChanged(const QString &strFilterIn);
    void on_filterProposalLineEdit_textChanged(const QString &strFilterIn);
    void voteYesAction();
    void voteNoActionSLOT();
    void voteAbstainActionSLOT();
    void voteDeleteActionSLOT();
    void showDetailsActionSLOT();
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_startMissingButton_clicked();
    void on_tableWidgetMyMasternodes2_itemSelectionChanged();
    void on_UpdateButton_clicked();
    void on_checkboxShowAllNodes_stateChanged(int i_checkboxShowAllNodes_state);
    void on_buttonUpdateList_clicked();
    void on_buttonNewProposal_clicked();
    void on_checkboxProposaslToVote_stateChanged(int arg1);
    void on_tableWidgetProposals_itemDoubleClicked();//QTableWidgetItem *item);
    void on_cbActiveOnly_stateChanged(int arg1);
    void on_tableWidgetProposals_itemActivated();
    void on_tableWidgetProposals_itemSelectionChanged();
};
#endif // MASTERNODELIST_H
