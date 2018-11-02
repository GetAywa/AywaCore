#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include <QWidget>

namespace Ui {
    class MessagePage;
}
class MessageModel;
class WalletModel;
class ClientModel;
//class ItmDelegate;
//class ListDelegate;


QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
class QListWidgetItem;
class QTreeWidgetItem;
class MessageViewDelegate;
QT_END_NAMESPACE


/** Widget that shows a list of sending or receiving addresses.
  */
class MessagePage : public QWidget
{
    Q_OBJECT

public:

    explicit MessagePage(QWidget *parent = 0);
    ~MessagePage();

    void setModel(MessageModel *model);
//    void setWalletModel(WalletModel *walletModel);
//    void setClientModel(ClientModel *clientModel);

private:
    void setupTextActions();
    void updateContactList();
    void updateMessageList();
    bool IsContactInTheList(QString qstrAddress);
    QTreeWidgetItem * addContactToAddressList(QString qstdGroup, QString qstrAddress);
    virtual bool eventFilter(QObject* obj, QEvent *event);

public Q_SLOTS:
    void exportClicked();
    void on_bnEmoji_clicked();

private:
    Ui::MessagePage *ui;
    MessageModel *model;
    WalletModel *walletModel;
    ClientModel *clientModel;

    QMenu *contextMenu;
    QAction *replyAction;
    QAction *copyFromAddressAction;
    QAction *copyToAddressAction;
    QAction *deleteAction;
    QString replyFromAddress;
    QString replyToAddress;

    //ItmDelegate * msgdelegate;
    MessageViewDelegate * msgdelegate;
    //ListDelegate * msgdelegate;

private Q_SLOTS:
    void on_sendButton_clicked();
    void on_newButton_clicked();
    void on_copyFromAddressButton_clicked();
    void on_copyToAddressButton_clicked();
    void on_deleteButton_clicked();

    void on_replyToSenderOnly_clicked();//added

    void incomingMessage();
    void updateMessagePage();
    /** Spawn contextual menu (right mouse menu) for address book entry */
    void contextualMenu(const QPoint &point);
//signals:
    void on_listConversation_doubleClicked(const QModelIndex &index);
    //void on_pushButton_clicked();
    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_itemSelectionChanged();
    void on_treeWidget_itemActivated(QTreeWidgetItem *item, int column);

    void on_pushButton_clicked();

Q_SIGNALS:
    void doubleClicked(const QModelIndex&);
};
#endif // MESSAGEPAGE_H
